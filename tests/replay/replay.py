#!/usr/bin/env python3
"""Layer-2 replay tests: boot the disc in PCSX-Redux, feed a pad script, hash gamestate_data at named checkpoints.

Usage:
  tests/replay/replay.py run <script.json> [--record] [--repeat N] [--bios openbios|retail] [--speed S] [--interpreter]
                             [--iso CUE] [--prelude LUA] [--expected-dir DIR] [--out DIR] [-v]
  tests/replay/replay.py check [--interpreter] [--iso CUE] [-j N] [script.json ...]   # every script with an expected file (the test entry point), N at a time

A script (tests/replay/scripts/<name>.json) is a list of steps (see tests/README.md); the runner tests/replay/run.lua
executes it in the emulator and writes result.json plus one gamestate_data dump per checkpoint. This driver hashes
the dumps (SHA-1), builds the record and compares it with tests/replay/expected/<name>.json, or writes that file
with --record. --repeat N runs the script N times and requires identical records (determinism).

Exit codes: 0 pass, 1 mismatch or emulator failure, 2 usage / missing tool.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REDUX = ROOT / "tools/redux/pcsx-redux"
REDUX_VERSION = ROOT / "tools/redux/app/usr/share/pcsx-redux/resources/version.json"
OPENBIOS = ROOT / "tools/redux/app/usr/share/pcsx-redux/resources/openbios.bin"


def gamedata_dir():
    """The optional data checkout (scripts/gamedata_dir.sh, same lookup): $DW3_GAMEDATA, tools/local.env, this
    checkout if it has gamedata/, or the sibling ../dw2003-gamedata. None when there is none."""
    d = os.environ.get("DW3_GAMEDATA")
    if not d and (ROOT / "tools/local.env").is_file():
        for line in (ROOT / "tools/local.env").read_text().splitlines():
            if line.startswith("DW3_GAMEDATA="):
                d = line.split("=", 1)[1].strip().strip("\"'")
                break
    if not d and (ROOT / "gamedata").is_dir():
        d = str(ROOT)
    if not d and (ROOT.parent / "dw2003-gamedata/gamedata").is_dir():
        d = str(ROOT.parent / "dw2003-gamedata")
    return Path(d) if d and Path(d).is_dir() else None


GD = gamedata_dir()
RETAIL_BIOS = (GD or ROOT) / "gamedata/bios/scph7502.bin"   # cross-check only: committed goldens come from OpenBIOS
ISO = ROOT / "iso/dw2003.cue"
RUN_LUA = ROOT / "tests/replay/run.lua"
SCRIPTS = ROOT / "tests/replay/scripts"
EXPECTED = ROOT / "tests/replay/expected"
# Extra emulator flags of --interpreter (the core tools/coverage.py needs); scripts must reach the same checkpoints on it.
EMU_INTERPRETER = ("-interpreter",)
# Fields of a checkpoint / sequence entry that the record keeps (in this order).
CHECKPOINT_FIELDS = ("name", "frame", "stage", "map", "random_index", "gamestate_sha1", "gamestate_sha1_stable")
# Byte ranges of gamestate_data (include/gamestate.h) that change every frame, with the frame of arrival, or with the
# pad_random index (which advances about once per frame in the field); they are
# zeroed for gamestate_sha1_stable so the port can compare a checkpoint reached a few frames earlier or later.
VOLATILE_RANGES = ((0x00, 0x01),   # checksum: 0 until a save is loaded, then the slot's, which covers the playtime
                   (0x30, 0x34),   # encounter_timer
                   (0x48, 0x54),   # playtime_frames, playtime[4]
                   (0x26EC, 0x26F0))  # spot_target: drawn from pad_random on entering a map with spots (0x21D), and the
                                      # RNG index depends on the frame count (core- and port-dependent); 0 before that


def sha1_file(path):
    return hashlib.sha1(Path(path).read_bytes()).hexdigest()


def lua_literal(v, indent=""):
    """A JSON value as a Lua literal (the step table run.lua loads)."""
    if isinstance(v, bool):
        return "true" if v else "false"
    if v is None:
        return "nil"
    if isinstance(v, (int, float)):
        return repr(v)
    if isinstance(v, str):
        return '"' + v.replace("\\", "\\\\").replace('"', '\\"') + '"'
    inner = indent + "  "
    if isinstance(v, list):
        return "{\n" + ",\n".join(inner + lua_literal(x, inner) for x in v) + "\n" + indent + "}"
    if isinstance(v, dict):
        parts = []
        for k, x in v.items():
            if isinstance(x, str) and k in ("addr", "map", "value", "word0") and x.startswith("0x"):
                x = int(x, 16)
            parts.append(f"{inner}[{lua_literal(k)}] = {lua_literal(x, inner)}")
        return "{\n" + ",\n".join(parts) + "\n" + indent + "}"
    raise TypeError(type(v))


def parse_ints(obj):
    """Hex strings in the script ("0x2D7") become ints for the Lua side."""
    if isinstance(obj, dict):
        return {k: (int(v, 16) if isinstance(v, str) and v.startswith("0x") and k in ("addr", "map", "value", "word0")
                    else parse_ints(v)) for k, v in obj.items()}
    if isinstance(obj, list):
        return [parse_ints(x) for x in obj]
    return obj


def tree_commit():
    try:
        # The last commit that touched the matching tree (not HEAD: test and doc commits don't change the game).
        rev = subprocess.run(["git", "-C", str(ROOT), "log", "-1", "--format=%H", "--", "src", "include", "config"],
                             check=True, capture_output=True, text=True).stdout.strip()
        dirty = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain", "--", "src", "config", "include"],
                               check=True, capture_output=True, text=True).stdout.strip() != ""
        return rev + ("-dirty" if dirty else "")
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown"


def emulator_info():
    info = json.loads(REDUX_VERSION.read_text())
    return {"name": "pcsx-redux", "version": info["version"], "build_id": info["buildId"],
            "changeset": info["changeset"]}


def bios_path(name):
    if name == "openbios":
        return OPENBIOS
    if name == "retail":
        return RETAIL_BIOS
    return Path(name)


def run_once(script_path, script, bios, out_dir, verbose=False, lua=None, emu_args=(), speed=0, iso=None):
    """Runs the script in the emulator once; returns the record (without the checkpoint dumps) or raises. `lua` replaces
    run.lua as the -dofile chunk and `emu_args` are extra emulator flags (tools/coverage.py: a wrapper chunk that loads
    tools/coverage.lua first, and -debugger -interpreter); `speed` is PCSX-Redux's spu.Speed (0: unthrottled); `iso` is
    another disc image's .cue (tests/holdouts/run.py: the NON_MATCHING build) instead of iso/dw2003.cue."""
    # Absolute: PCSX-Redux resolves a relative -memcard path elsewhere (not the cwd), and a card that is not the fresh
    # one changes the boot (CNTY_SEL 144 frames earlier with `--out` relative).
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    lua_script = out_dir / "script.lua"
    lua_script.write_text("return " + lua_literal(parse_ints(script)) + "\n")
    env = dict(os.environ, DW3_REPLAY_SCRIPT=str(lua_script), DW3_REPLAY_OUT=str(out_dir))
    if verbose:
        env["DW3_REPLAY_VERBOSE"] = "1"
    # PCSX-Redux's spu.Speed paces the host (the audio sink is the master clock): 0 = unthrottled, ~10x real time here.
    # The emulated machine does not see it: the records are identical at speed 1 (checked on new_game).
    env["DW3_REPLAY_SPEED"] = str(speed)
    # Fresh (empty) memory cards per run: the user's ~/.config cards must not leak into a replay.
    mcd1, mcd2 = out_dir / "memcard1.mcd", out_dir / "memcard2.mcd"
    for m in (mcd1, mcd2):
        if m.exists():
            m.unlink()
    cmd = [str(REDUX), "-no-ui", "-stdout", "-testmode", "-run", *emu_args, "-iso", str(iso or ISO), "-bios", str(bios),
           "-memcard1", str(mcd1), "-memcard2", str(mcd2), "-dofile", str(lua or RUN_LUA)]
    max_frames = script.get("max_frames", 20000)
    # real time (--speed 1) is ~50 frames/s here; unthrottled ~500 (dynarec) or ~250 (interpreter): 10x margins
    timeout = max_frames / (10 if speed and speed > 0 else 25) + 120
    log_path = out_dir / "emulator.log"
    t0 = time.time()
    with open(log_path, "w") as log:
        proc = subprocess.run(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=timeout)
    elapsed = time.time() - t0
    result_path = out_dir / "result.json"
    log_text = log_path.read_text(errors="replace")
    if verbose:
        sys.stdout.write("".join(l + "\n" for l in log_text.splitlines() if l.startswith("replay:")))
    if not result_path.exists():
        tail = "\n".join(log_text.splitlines()[-15:])
        raise RuntimeError(f"emulator exited {proc.returncode} without result.json (log {log_path}):\n{tail}")
    result = json.loads(result_path.read_text())
    if result.get("status") != "ok" or proc.returncode != 0:
        raise RuntimeError(f"replay failed (exit {proc.returncode}): {result.get('message')} (log {log_path})")
    checkpoints = []
    for cp in result["checkpoints"]:
        entry = {k: cp[k] for k in CHECKPOINT_FIELDS if k in cp}
        data = (out_dir / cp["gamestate_file"]).read_bytes()
        entry["gamestate_sha1"] = hashlib.sha1(data).hexdigest()
        stable = bytearray(data)
        for lo, hi in VOLATILE_RANGES:
            stable[lo:hi] = bytes(hi - lo)
        entry["gamestate_sha1_stable"] = hashlib.sha1(stable).hexdigest()
        checkpoints.append({k: entry[k] for k in CHECKPOINT_FIELDS})
    record = {
        "script": script["name"],
        "script_sha1": sha1_file(script_path),
        "emulator": emulator_info(),
        "bios": {"name": "openbios" if bios == OPENBIOS else ("retail" if bios == RETAIL_BIOS else bios.name),
                 "sha1": sha1_file(bios)},
        "tree_commit": tree_commit(),
        "frames": result["frames"],
        "checkpoints": checkpoints,
        "overlay_sequence": result["overlay_sequence"],
        "map_sequence": result["map_sequence"],
        "inputs": result["inputs"],
    }
    print(f"  run: {result['frames']} frames, {len(checkpoints)} checkpoints, {elapsed:.0f} s wall")
    return record


def compare(expected, actual, ignore=("tree_commit",)):
    """Lists the differences between two records (empty when identical)."""
    diffs = []
    for key in sorted(set(expected) | set(actual)):
        if key in ignore:
            continue
        if expected.get(key) != actual.get(key):
            if key == "checkpoints":
                for i, (e, a) in enumerate(zip(expected[key], actual[key])):
                    for f in CHECKPOINT_FIELDS:
                        if e.get(f) != a.get(f):
                            diffs.append(f"checkpoint {i} ({e.get('name')}) {f}: expected {e.get(f)!r}, got {a.get(f)!r}")
                if len(expected[key]) != len(actual[key]):
                    diffs.append(f"checkpoints: expected {len(expected[key])}, got {len(actual[key])}")
            else:
                diffs.append(f"{key}: expected {json.dumps(expected.get(key))[:200]}, got {json.dumps(actual.get(key))[:200]}")
    return diffs


def cross_core_view(record):
    """The part of a record that must not depend on the CPU core (dynarec or -interpreter): checkpoint names, stages,
    maps and stable hashes, and the overlay and map sequences without frames. Frames, RNG draw counts, the full hash
    (timers) and the input trace follow the emulated timing, which the two cores do not share (CD and MDEC)."""
    return {
        "checkpoints": [{k: cp[k] for k in ("name", "stage", "map", "gamestate_sha1_stable")} for cp in record["checkpoints"]],
        "overlay_sequence": [(o["stage"], o["file"]) for o in record["overlay_sequence"]],
        "map_sequence": [m["map"] for m in record["map_sequence"]],
    }


def load_script(path):
    script = json.loads(Path(path).read_text())
    script.setdefault("name", Path(path).stem)
    return script


def check_tools():
    missing = [str(p) for p in (REDUX, ISO, RUN_LUA) if not p.exists()]
    if missing:
        print("replay: missing " + ", ".join(missing) + " (scripts/setup.sh redux; the disc)", file=sys.stderr)
        sys.exit(2)


def shown(path):
    """A path for messages: relative to the repository when inside it."""
    path = Path(path).resolve()
    return path.relative_to(ROOT) if path.is_relative_to(ROOT) else path


def cmd_run(args):
    check_tools()
    script_path = Path(args.script)
    script = load_script(script_path)
    bios = bios_path(args.bios)
    base_out = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="dw3_replay_"))
    records = []
    emu_args, lua = EMU_INTERPRETER if args.interpreter else (), None
    if args.prelude:
        # a Lua chunk run before run.lua (tests/port/ntsc_patch.lua: the NTSC patch's bytes at main's entry); its exec
        # breakpoints need the debugger and the interpreter core
        emu_args = ("-debugger", "-interpreter")
        base_out.mkdir(parents=True, exist_ok=True)
        lua = base_out.resolve() / "prelude_wrapper.lua"
        lua.write_text(f"dofile({json.dumps(str(Path(args.prelude).resolve()))})\ndofile({json.dumps(str(RUN_LUA))})\n")
    for i in range(args.repeat):
        print(f"replay {script['name']} (bios {args.bios}), run {i + 1}/{args.repeat}")
        try:
            records.append(run_once(script_path, script, bios, base_out / f"run{i + 1}", verbose=args.verbose,
                                    lua=lua, emu_args=emu_args, speed=args.speed, iso=args.iso))
        except (RuntimeError, subprocess.TimeoutExpired) as e:
            print(f"  FAIL: {e}")
            return 1
    status = 0
    for i, rec in enumerate(records[1:], start=2):
        diffs = compare(records[0], rec, ignore=())
        if diffs:
            status = 1
            print(f"  run 1 vs run {i} DIFFER (non-deterministic):\n    " + "\n    ".join(diffs))
    if args.repeat > 1 and status == 0:
        print(f"  determinism: {args.repeat} runs identical")
    if args.prelude:
        records[0]["prelude"] = Path(args.prelude).name
    expected_path = (Path(args.expected_dir) if args.expected_dir else EXPECTED) / f"{script['name']}.json"
    if args.record and args.interpreter and not args.prelude:
        print("  not recording: expected files come from the default core (dynarec)")
    elif args.record:
        if status:
            print("  not recording: runs differ")
        else:
            expected_path.parent.mkdir(exist_ok=True)
            expected_path.write_text(json.dumps(records[0], indent=1) + "\n")
            print(f"  recorded {shown(expected_path)}")
    elif expected_path.exists():
        expected = json.loads(expected_path.read_text())
        diffs = (compare(cross_core_view(expected), cross_core_view(records[0])) if args.interpreter or args.prelude
                 else compare(expected, records[0]))
        if diffs:
            status = 1
            print(f"  MISMATCH vs {shown(expected_path)}:\n    " + "\n    ".join(diffs))
        else:
            print(f"  matches {shown(expected_path)}" + (" (cross-core view)" if args.interpreter else ""))
    else:
        print(f"  no expected file ({shown(expected_path)}); use --record")
    print(f"  outputs: {base_out}")
    return status


def check_one(script_path, args):
    """One script's replay against its expected file: (status, the lines to print)."""
    script = load_script(script_path)
    expected_path = EXPECTED / f"{script['name']}.json"
    if not expected_path.exists():
        return 0, [f"replay {script['name']}: no expected file, skipped"]
    expected = json.loads(expected_path.read_text())
    if expected.get("script_sha1") != sha1_file(script_path):
        return 1, [f"replay {script['name']}: FAIL (the script changed since it was recorded; re-record)"]
    bios = bios_path(expected["bios"]["name"])
    out = Path(tempfile.mkdtemp(prefix=f"dw3_replay_{script['name']}_"))
    lines = [f"replay {script['name']} (bios {expected['bios']['name']})"]
    try:
        record = run_once(script_path, script, bios, out, emu_args=EMU_INTERPRETER if args.interpreter else (),
                          iso=args.iso)
    except (RuntimeError, subprocess.TimeoutExpired) as e:
        return 1, lines + [f"  FAIL: {e}"]
    diffs = compare(cross_core_view(expected), cross_core_view(record)) if args.interpreter else compare(expected, record)
    if diffs:
        return 1, lines + [f"  FAIL: mismatch vs {expected_path.relative_to(ROOT)}:\n    " + "\n    ".join(diffs)]
    shutil.rmtree(out, ignore_errors=True)
    return 0, lines + [f"  pass ({record['frames']} frames, {len(record['checkpoints'])} checkpoints)"]


def cmd_check(args):
    """Every script (or the ones named), -j at a time (default $DW3_JOBS, else all at once: each is one emulator,
    single-threaded); each one's lines are printed together, in the scripts' order."""
    check_tools()
    scripts = [Path(s) for s in args.scripts] or sorted(SCRIPTS.glob("*.json"))
    status = 0
    jobs = max(1, min(args.jobs or len(scripts), len(scripts)))
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for rc, lines in pool.map(lambda s: check_one(s, args), scripts):
            print("\n".join(lines), flush=True)
            status |= rc
    return status


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run", help="run one script (record or compare)")
    r.add_argument("script")
    r.add_argument("--record", action="store_true", help="write tests/replay/expected/<name>.json")
    r.add_argument("--repeat", type=int, default=1, help="run N times and require identical records")
    r.add_argument("--bios", default="openbios", help="openbios (default), retail, or a BIOS file")
    r.add_argument("--out", help="output directory (default: a temp dir)")
    r.add_argument("--speed", type=float, default=0,
                   help="emulation speed (PCSX-Redux spu.Speed): 0 = unthrottled (default), 1 = real time")
    r.add_argument("--interpreter", action="store_true",
                   help="run on the interpreter core; compares the cross-core view with the expected file (no --record)")
    r.add_argument("--iso", help="another disc image (.cue) instead of iso/dw2003.cue (tests/holdouts/run.py)")
    r.add_argument("--prelude", help="a Lua chunk run before run.lua (e.g. tests/port/ntsc_patch.lua); implies the "
                                     "debugger and the interpreter core; compares the cross-core view")
    r.add_argument("--expected-dir", help="where the expected file is read or --record writes it "
                                          "(default tests/replay/expected; e.g. tests/port/hz60)")
    r.add_argument("-v", "--verbose", action="store_true")
    r.set_defaults(func=cmd_run)
    c = sub.add_parser("check", help="run every recorded script and compare")
    c.add_argument("scripts", nargs="*")
    c.add_argument("--interpreter", action="store_true", help="run on the interpreter core; compare the cross-core view")
    c.add_argument("--iso", help="another disc image (.cue) instead of iso/dw2003.cue (tests/holdouts/run.py)")
    c.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None,
                   help="scripts replayed at once (default: $DW3_JOBS, else all)")
    c.set_defaults(func=cmd_check)
    args = ap.parse_args()
    sys.exit(args.func(args))


if __name__ == "__main__":
    main()
