#!/usr/bin/env python3
"""The port's 60 Hz mode against the 60 Hz game in the emulator (docs/LAUNCHER_MODS_PLAN.md 5.5).

Usage: tests/port/hz60.py [check] [--out DIR] [-j N]
       tests/port/hz60.py record [--out DIR]          # needs the emulator (scripts/setup.sh redux); ~8 min

The 60 Hz game is the EU disc with the NTSC patch's two data words (records_60hz 1, main_screen_pos 0); the emulator
runs it from the unpatched disc with tests/port/ntsc_patch.lua, which writes them at main's entry. The port's 60 Hz
mode is `--refresh 60` (or the settings' video.refresh): records_60hz 1 before the reset's snapshot, the pace, the
audio's and the CD's rate at 60 (main_screen_pos stays 1: the port's video ignores the screen offset).

`record` writes, from the emulator, into tests/port/hz60/:
  new_game.json                              the layer-2 record of new_game (replay.py run --prelude, two runs agreeing)
  first_battle_save@card_shop_left.json      the same for first_battle_save cut after its checkpoint card_shop_left:
                                             the script's next walk (step 219, LEFT for 20 frames to the trainer) is
                                             timed for 50 Hz and stops short at 60, on the emulator as in the port
  new_game.trace.gz                          new_game's SPU trace with the game's LIBSND calls (spu_trace.py --calls)
`check` (the default; no emulator) runs the port at 60 Hz with DW3_PORT_RESET_CHECK=1 and requires: new_game twice
with the same log and record; each script's cross-core view (checkpoint names, stages, maps, stable gamestate hashes,
the overlay and map sequences) equal to the emulator's; new_game's SPU trace equal to LIBSND's replay of its own calls
at 735 samples per vsync with the NTSC tick, and the same LIBSND calls as the emulator's (tests/port/sound.py `port`).
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import gzip
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
sys.path.insert(0, str(ROOT / "tests/replay"))
sys.path.insert(0, str(ROOT / "tests/sound"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)
from replay import compare, cross_core_view, sha1_file  # noqa: E402
import sound  # noqa: E402

HZ60 = ROOT / "tests/port/hz60"
PRELUDE = ROOT / "tests/port/ntsc_patch.lua"
# (script, the checkpoint it is cut after, or None)
CASES = (("new_game", None), ("first_battle_save", "card_shop_left"))
RATE_60 = 735.0  # the port's samples per vsync at 60 Hz (SPU_RATE / 60)


def case_script(name, until, out):
    """The script file of a case: the committed one, or the cut copy (written the same way every time)."""
    path = SCRIPTS / f"{name}.json"
    if until is None:
        return path
    script = json.loads(path.read_text())
    steps = script["steps"]
    idx = next(i for i, s in enumerate(steps) if s.get("type") == "checkpoint" and s.get("name") == until)
    script["name"] = f"{name}@{until}"
    script["steps"] = steps[:idx + 1]
    cut = out / f"{script['name']}.json"
    cut.parent.mkdir(parents=True, exist_ok=True)
    cut.write_text(json.dumps(script, indent=1) + "\n")
    return cut


def cmd_record(out):
    py = sys.executable
    HZ60.mkdir(exist_ok=True)
    for name, until in CASES:
        script = case_script(name, until, out / "scripts")
        subprocess.run([py, str(ROOT / "tests/replay/replay.py"), "run", str(script), "--prelude", str(PRELUDE),
                        "--expected-dir", str(HZ60), "--record", "--repeat", "2", "--out", str(out / f"emu_{name}")],
                       check=True)
    trace_out = out / "emu_trace"
    subprocess.run([py, str(ROOT / "tests/sound/spu_trace.py"), "run", str(SCRIPTS / "new_game.json"), "--calls",
                    "--repeat", "2", "--prelude", str(PRELUDE), "--out", str(trace_out)], check=True)
    with gzip.open(HZ60 / "new_game.trace.gz", "wb", mtime=0) as f:
        f.write((trace_out / "run1/spu.trace").read_bytes())
    print(f"hz60: recorded {', '.join(p.name for p in sorted(HZ60.iterdir()))}")
    return 0


def run_port(binary, script, out, label, spu=False):
    log, record, err = out / f"{label}.log", out / f"{label}.json", out / f"{label}.stderr"
    cmd = [str(binary), "--disc", str(DISC), "--refresh", "60", "--script", str(script), "--log", str(log),
           "--record", str(record)]
    if spu:
        cmd += ["--spu-trace", str(out / f"{label}.spu.trace")]
    with open(err, "w") as f:
        proc = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, DW3_PORT_RESET_CHECK="1"), stdout=f,
                              stderr=subprocess.STDOUT, timeout=600)
    if proc.returncode != 0 or not record.exists():
        tail = "\n    ".join(err.read_text(errors="replace").splitlines()[-6:])
        raise RuntimeError(f"{label}: exit {proc.returncode} ({err}):\n    {tail}")
    return log.read_bytes(), record.read_bytes(), json.loads(record.read_text())


def cmd_check(out, jobs):
    try:
        if not DISC.exists():
            raise Missing(f"no disc image ({DISC.relative_to(ROOT)})")
        env = tool_env()
    except Missing as e:
        print(f"hz60: {e}", file=sys.stderr)
        return 2
    binary = build("build/port", [], jobs, env)
    failures = []
    for name, until in CASES:
        script = case_script(name, until, out / "scripts")
        label = script.stem
        expected_path = HZ60 / f"{label}.json"
        expected = json.loads(expected_path.read_text())
        print(f"hz60: {label} at 60 Hz (the emulator's record: {expected_path.relative_to(ROOT)})")
        try:
            a = run_port(binary, script, out, f"{label}_1", spu=name == "new_game")
            if name == "new_game":
                b = run_port(binary, script, out, f"{label}_2")
                if a[:2] != b[:2]:
                    failures.append(f"{label}: two runs differ (log or record)")
        except (RuntimeError, subprocess.TimeoutExpired) as e:
            failures.append(str(e))
            continue
        if expected.get("script_sha1") != sha1_file(script):
            failures.append(f"{label}: the script is not the one {expected_path.name} was recorded from "
                            f"(tests/port/hz60.py record)")
        diffs = compare(cross_core_view(expected), cross_core_view(a[2]))
        if diffs:
            failures.append(f"{label}: the cross-core view differs from the emulator's:\n    " + "\n    ".join(diffs))
        else:
            print(f"  ok: {len(expected['checkpoints'])} checkpoints, the stages and maps as in the emulator "
                  f"(port {a[2]['frames']} frames, emulator {expected['frames']})")
    trace = out / "new_game_1.spu.trace"
    if trace.exists():
        failures += sound.check_port_run(trace, HZ60 / "new_game.trace.gz", out / "sound", RATE_60, ntsc=True)
    print(f"hz60: {'FAIL' if failures else 'pass'}")
    for f in failures:
        print(f"  {f}")
    return 1 if failures else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("mode", nargs="?", default="check", choices=("check", "record"))
    ap.add_argument("--out", help="scratch directory (default build/port-test/hz60)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None)
    args = ap.parse_args()
    out = (Path(args.out) if args.out else ROOT / "build/port-test/hz60").resolve()
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    return cmd_record(out) if args.mode == "record" else cmd_check(out, args.jobs)


if __name__ == "__main__":
    sys.exit(main())
