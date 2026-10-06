#!/usr/bin/env python3
"""The PC port's M1 test: the port replays the layer-2 script new_game and must reach what the emulator reached.

Usage: tests/port/run.py [--m32] [--sanitize] [--cd-speed instant|realistic] [--out DIR] [-j N]

Builds the port if needed (cmake -S port -B build/port -G Ninja; cmake --build), runs
  build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/new_game.json --log ... --record ...
twice and requires the two logs and the two records to be byte-identical (determinism), then compares the record's
cross-core view (tests/replay/replay.py cross_core_view, compare) with tests/replay/expected/new_game.json: the same
checkpoints (name, stage, map, stable gamestate hash) and the same overlay and map sequences, without frames.
  --m32       also build build/port-m32 (-DDW3_PORT_M32=ON, needs gcc-multilib) and require its log and record to equal
              the 64-bit build's (the layout check: pointers are 4 bytes there, as on the PS1)
  --sanitize  also build build/port-san (-DDW3_PORT_SANITIZE=ON), run it once, and fail on any ASan/UBSan report
              (its log and record must equal the plain build's too)
  --cd-speed  the port's CD timing (default: the port's, realistic)
  --out DIR   where the logs, records and checkpoint dumps go (default build/port-test/)
Needs only the disc, the host gcc, CMake and Ninja (on PATH, or in tools/venv: scripts/setup.sh cmake) and the venv.
Exit codes: 0 pass, 1 fail, 2 something missing. tests/port/README.md says what is compared and why.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/replay"))
from replay import compare, cross_core_view, sha1_file  # noqa: E402  (the layer-2 test's own comparison)

SCRIPT = ROOT / "tests/replay/scripts/new_game.json"
EXPECTED = ROOT / "tests/replay/expected/new_game.json"
DISC = ROOT / "iso/dw2003.cue"
VENV_BIN = ROOT / "tools/venv/bin"
RUN_TIMEOUT = 600  # seconds; a plain run takes well under one, the sanitizer build a few
SANITIZER_MARKS = ("runtime error:", "ERROR: AddressSanitizer", "ERROR: LeakSanitizer", "SUMMARY: AddressSanitizer",
                   "SUMMARY: UndefinedBehaviorSanitizer")


class Missing(Exception):
    """Something the test needs is not there (exit 2)."""


def tool_env():
    """The environment for cmake/ninja: the system's first, then tools/venv/bin (scripts/setup.sh cmake)."""
    env = dict(os.environ)
    env["PATH"] = env.get("PATH", "") + os.pathsep + str(VENV_BIN)
    for tool in ("cmake", "ninja"):
        if shutil.which(tool, path=env["PATH"]) is None:
            raise Missing(f"no {tool} on PATH or in tools/venv (scripts/setup.sh cmake)")
    if shutil.which("gcc") is None and shutil.which("cc") is None:
        raise Missing("no host C compiler (gcc)")
    return env


def build(build_dir, options, jobs, env):
    """Configures (once) and builds the port into build_dir; returns the binary."""
    build_dir = ROOT / build_dir
    if not (build_dir / "CMakeCache.txt").exists():
        print(f"  configure {build_dir.relative_to(ROOT)} {' '.join(options)}".rstrip())
        subprocess.run(["cmake", "-S", str(ROOT / "port"), "-B", str(build_dir), "-G", "Ninja", *options],
                       check=True, env=env, stdout=subprocess.DEVNULL)
    cmd = ["cmake", "--build", str(build_dir)] + (["-j", str(jobs)] if jobs else [])
    proc = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if proc.returncode != 0:
        sys.stdout.write(proc.stdout[-4000:] + proc.stderr[-4000:])
        raise RuntimeError(f"the build of {build_dir.relative_to(ROOT)} failed")
    return build_dir / "dw2003"


def run_port(binary, out_dir, label, cd_speed, env=None):
    """One run of the script; returns (log bytes, record bytes, record, stderr text)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    dumps = out_dir / f"{label}_checkpoints"
    if dumps.exists():
        shutil.rmtree(dumps)
    dumps.mkdir()
    log, record, err = out_dir / f"{label}.log", out_dir / f"{label}.json", out_dir / f"{label}.stderr"
    cmd = [str(binary), "--disc", str(DISC), "--script", str(SCRIPT), "--log", str(log), "--record", str(record)]
    if cd_speed:
        cmd += ["--cd-speed", cd_speed]
    run_env = dict(env or os.environ, DW3_PORT_CHECKPOINT_DIR=str(dumps))
    with open(err, "w") as f:
        proc = subprocess.run(cmd, cwd=ROOT, env=run_env, stdout=f, stderr=subprocess.STDOUT, timeout=RUN_TIMEOUT)
    err_text = err.read_text(errors="replace")
    if not record.exists() or record.stat().st_size == 0:
        tail = "\n    ".join(err_text.splitlines()[-15:])
        raise RuntimeError(f"{label}: exit {proc.returncode} without a record ({err}):\n    {tail}")
    rec = json.loads(record.read_text())
    if proc.returncode != 0 or rec.get("status") != 0:
        tail = "\n    ".join(err_text.splitlines()[-8:])
        raise RuntimeError(f"{label}: exit {proc.returncode}, status {rec.get('status')}: {rec.get('reason')} "
                           f"({err}):\n    {tail}")
    return log.read_bytes(), record.read_bytes(), rec, err_text


def same_output(a, b, what):
    """Byte comparison of two runs' (log, record); returns the differences as text."""
    diffs = []
    for i, name in ((0, "log"), (1, "record")):
        if a[i] != b[i]:
            la, lb = a[i].decode(errors="replace").splitlines(), b[i].decode(errors="replace").splitlines()
            first = next((n for n, (x, y) in enumerate(zip(la, lb)) if x != y), min(len(la), len(lb)))
            diffs.append(f"{what}: the {name}s differ from line {first + 1}: "
                         f"{la[first] if first < len(la) else '<end>'!r} vs {lb[first] if first < len(lb) else '<end>'!r}")
    return diffs


def summary(expected, rec):
    """The checkpoints side by side, and the sequences."""
    exp_cps = {cp["name"]: cp for cp in expected["checkpoints"]}
    print(f"  {'checkpoint':<16} {'stage':>5} {'map':>7} {'port frame':>10} {'emulator':>8}  stable hash")
    for cp in rec["checkpoints"]:
        e = exp_cps.get(cp["name"], {})
        mark = "ok" if e.get("gamestate_sha1_stable") == cp["gamestate_sha1_stable"] else \
            f"DIFFERS (emulator {e.get('gamestate_sha1_stable', '?')[:12]})"
        print(f"  {cp['name']:<16} {cp['stage']:>5} {cp['map']:>#7x} {cp['frame']:>10} {e.get('frame', '-'):>8}  "
              f"{cp['gamestate_sha1_stable'][:12]} {mark}")
    view = cross_core_view(rec)
    print("  overlay sequence: " + " ".join(f"({s},{f})" for s, f in view["overlay_sequence"]))
    print("  map sequence:     " + " ".join(f"{m:#x}" for m in view["map_sequence"]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--m32", action="store_true", help="also the -m32 build; its log and record must be the same")
    ap.add_argument("--sanitize", action="store_true", help="also an ASan/UBSan build; no report allowed")
    ap.add_argument("--cd-speed", choices=("instant", "realistic"), help="the port's CD timing")
    ap.add_argument("--out", help="output directory (default build/port-test/)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None,
                    help="build jobs (default: $DW3_JOBS, else Ninja's)")
    args = ap.parse_args()

    try:
        if not DISC.exists():
            raise Missing(f"no disc image ({DISC.relative_to(ROOT)}; scripts/setup.sh disc)")
        env = tool_env()
    except Missing as e:
        print(f"port test: {e}", file=sys.stderr)
        return 2
    expected = json.loads(EXPECTED.read_text())
    if expected.get("script_sha1") != sha1_file(SCRIPT):
        print(f"port test: FAIL: {SCRIPT.relative_to(ROOT)} changed since {EXPECTED.relative_to(ROOT)} was recorded "
              "(re-record it with tests/replay/replay.py run --record)")
        return 1
    out = Path(args.out) if args.out else ROOT / "build/port-test"
    out.mkdir(parents=True, exist_ok=True)
    out = out.resolve()

    failures = []
    print(f"port test: {SCRIPT.relative_to(ROOT)} vs {EXPECTED.relative_to(ROOT)} (cross-core view)")
    try:
        binary = build("build/port", [], args.jobs, env)
        run1 = run_port(binary, out, "run1", args.cd_speed)
        run2 = run_port(binary, out, "run2", args.cd_speed)
        rec = run1[2]
        print(f"  runs: {rec['frames']} frames (emulator {expected['frames']}), {rec['reason']}; exit 0")
        diffs = same_output(run1, run2, "run 1 vs run 2")
        failures += diffs
        if not diffs:
            print("  determinism: two runs, identical logs and records")
        summary(expected, rec)
        diffs = compare(cross_core_view(expected), cross_core_view(rec))
        if diffs:
            failures += diffs
            print(f"  checkpoint dumps (gamestate_data's PS1 image): {out / 'run1_checkpoints'}; the emulator's: "
                  "tests/replay/replay.py run tests/replay/scripts/new_game.json -v --out DIR")
        else:
            print(f"  cross-core view: matches {EXPECTED.relative_to(ROOT)}")
        if args.m32:
            m32 = build("build/port-m32", ["-DDW3_PORT_M32=ON"], args.jobs, env)
            run = run_port(m32, out, "m32", args.cd_speed)
            diffs = same_output(run1, run, "-m64 vs -m32")
            failures += diffs
            if not diffs:
                print("  -m32: log and record identical to the 64-bit build's")
        if args.sanitize:
            san = build("build/port-san", ["-DDW3_PORT_SANITIZE=ON"], args.jobs, env)
            san_env = dict(os.environ, UBSAN_OPTIONS="print_stacktrace=1",
                           ASAN_OPTIONS=os.environ.get("ASAN_OPTIONS", "detect_leaks=1"))
            run = run_port(san, out, "san", args.cd_speed, san_env)
            reports = [l for l in run[3].splitlines() if any(m in l for m in SANITIZER_MARKS)]
            if reports:
                failures.append(f"sanitizer: {len(reports)} report line(s) in {out / 'san.stderr'}: {reports[0]}")
            else:
                print("  sanitizer: no ASan/UBSan report")
            diffs = same_output(run1, run, "plain vs sanitizer build")
            failures += diffs
            if not diffs:
                print("  sanitizer build: log and record identical to the plain build's")
    except (RuntimeError, subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
        failures.append(str(e))
    if failures:
        print("port test: FAIL\n  " + "\n  ".join(failures))
        print(f"  outputs: {out}")
        return 1
    print(f"port test: pass (outputs: {out})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
