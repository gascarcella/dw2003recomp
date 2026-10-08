#!/usr/bin/env python3
"""Save states (psxstack/runtime/savestate.c; docs/PORT.md "Save states"): the resume test and the battle fixture.

Usage:
  tests/port/savestate.py [test] [--m32] [--sanitize] [--exe BIN [--wine]] [--out DIR] [-j N]
  tests/port/savestate.py path [CHECKPOINT] [--exe BIN]   # a state at CHECKPOINT (default battle_start), made if missing

test: first_battle_save from boot with `--save-state battle_start:FILE` (the record's checkpoints must still match
tests/replay/expected/, the emulator's), then the same script from that state (`--load-state FILE`): the resumed run's
record must equal the straight run's byte for byte, its frame log every line of the straight log after the saved
frame, and its audio (--wav) the straight run's samples from that frame on. --m32 and --sanitize do the same with
build/port-m32 and build/port-san (ASAN_OPTIONS=detect_stack_use_after_return=0: a state cannot hold ASan's fake
stacks; no ASan/UBSan report allowed); --exe tests another binary (--wine: a Windows build under Wine).

path: prints the path of a state of `--exe` (default build/port/dw2003) at the end of the frame in which
first_battle_save's checkpoint CHECKPOINT ran, and makes it first when it is missing (about 10 s headless; the run
exits right after the save). A state belongs to the exact binary that saved it, so it lives under
build/states/<the binary's SHA-1, 12 digits>/: a rebuilt binary gets its own. States hold game data: never commit one.
  build/port-sdl/dw2003 --disc iso/dw2003.cue --window --renderer gpu \\
      --load-state "$(tools/venv/bin/python tests/port/savestate.py path --exe build/port-sdl/dw2003)"

Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
import run  # noqa: E402  (the port test's builds, runner and comparisons)
from run import Missing  # noqa: E402

SCRIPT = ROOT / "tests/replay/scripts/first_battle_save.json"
EXPECTED = ROOT / "tests/replay/expected/first_battle_save.json"
CHECKPOINT = "battle_start"
STATES = ROOT / "build/states"
SPU_RATE = 44100
WAV_HEADER = 44  # audio.c's RIFF header

FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def sha1_of(path):
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def port_run(binary, out, label, extra, env=None, script=SCRIPT, outputs=True):
    """One run of the port headless; returns its stderr text (raises on a failed run)."""
    out.mkdir(parents=True, exist_ok=True)
    err = out / f"{label}.stderr"
    cmd = [*run.RUNNER, str(binary), "--disc", str(run.DISC), "--script", str(script)]
    if outputs:
        cmd += ["--log", str(out / f"{label}.log"), "--record", str(out / f"{label}.json"), "--wav",
                str(out / f"{label}.wav")]
    cmd += extra
    with open(err, "w") as f:
        proc = subprocess.run(cmd, cwd=ROOT, env=dict(env or os.environ, **run.RUNNER_ENV), stdout=f,
                              stderr=subprocess.STDOUT, timeout=run.RUN_TIMEOUT)
    text = err.read_text(errors="replace")
    if proc.returncode != 0:
        tail = "\n    ".join(text.splitlines()[-8:])
        raise RuntimeError(f"{label}: exit {proc.returncode} ({err}):\n    {tail}")
    return text


def log_after(path, frame):
    """The frame log's lines after `frame` (every line but the header carries its frame second)."""
    lines = []
    for line in path.read_text().splitlines():
        if not line.startswith("#") and int(line.split()[1]) > frame:
            lines.append(line)
    return lines


def wav_data(path):
    data = path.read_bytes()
    if data[:4] != b"RIFF" or data[36:40] != b"data":
        raise RuntimeError(f"{path}: not the port's WAV")
    return data[WAV_HEADER:]


def check_variant(label, binary, out, env=None):
    """Straight with a save at battle_start, then resumed from it: the same record, log tail and audio."""
    print(f"--- {label}: {binary.relative_to(ROOT) if binary.is_relative_to(ROOT) else binary}"
          f"{' under wine' if run.RUNNER else ''}")
    state = out / f"{label}.state"
    state.unlink(missing_ok=True)
    try:
        err = port_run(binary, out, f"{label}_straight", ["--save-state", f"{CHECKPOINT}:{state}"], env)
        rec = json.loads((out / f"{label}_straight.json").read_text())
        saved = next((c["frame"] for c in rec["checkpoints"] if c["name"] == CHECKPOINT), None)
        check(state.exists() and saved is not None, f"saved at {CHECKPOINT} (frame {saved}): "
              + next((l.split("(", 1)[1].rstrip(")") for l in err.splitlines() if "state: frame" in l), "?"))
        expected = json.loads(EXPECTED.read_text())
        diffs = run.compare(run.cross_core_view(expected), run.cross_core_view(rec))
        check(not diffs, "the straight run (with the save) matches the emulator's checkpoints"
              + (f": {diffs[0]}" if diffs else ""))
        err = port_run(binary, out, f"{label}_resumed", ["--load-state", str(state)], env)
        check("script: first_battle_save resumed" in err, "the script resumed from the state's step")
        a, b = (out / f"{label}_straight.json").read_bytes(), (out / f"{label}_resumed.json").read_bytes()
        check(a == b, f"record: the resumed run's equals the straight run's ({len(a)} bytes)")
        la, lb = log_after(out / f"{label}_straight.log", saved), log_after(out / f"{label}_resumed.log", -1)
        first = next((i for i, (x, y) in enumerate(zip(la, lb)) if x != y), None)
        check(la == lb, f"log: {len(lb)} lines after frame {saved}, equal to the straight log's {len(la)}"
              + (f"; first difference: {la[first]!r} vs {lb[first]!r}" if first is not None else ""))
        wa, wb = wav_data(out / f"{label}_straight.wav"), wav_data(out / f"{label}_resumed.wav")
        skip = saved * SPU_RATE // rec_rate(err) * 4
        check(wa[skip:] == wb, f"audio: {len(wb) // 4} samples from frame {saved + 1}, equal to the straight run's")
        if env is not None:
            reports = [l for p in out.glob(f"{label}_*.stderr") for l in p.read_text(errors="replace").splitlines()
                       if any(m in l for m in run.SANITIZER_MARKS)]
            check(not reports, "no ASan/UBSan report" + (f": {reports[0]}" if reports else ""))
    except (RuntimeError, subprocess.TimeoutExpired) as e:
        check(False, str(e))


def rec_rate(stderr_text):
    """The run's rate (the start line: '... 50 Hz')."""
    for line in stderr_text.splitlines():
        if line.startswith("port: start:"):
            return int(line.rsplit(",", 1)[1].split()[0])
    return 50


def state_path(binary, checkpoint):
    """build/states/<binary sha1[:12]>/<checkpoint>.state, made when missing."""
    path = STATES / sha1_of(binary)[:12] / f"{checkpoint}.state"
    if path.exists():
        return path
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.parent / f"{checkpoint}.making"
    port_run(binary, tmp, checkpoint, ["--save-state", f"{checkpoint}:{path}", "--save-state-exit"],
             outputs=False)
    shutil.rmtree(tmp, ignore_errors=True)
    if not path.exists():
        raise RuntimeError(f"{SCRIPT.name} ended without reaching its checkpoint {checkpoint}")
    return path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("command", nargs="?", default="test", choices=("test", "path"))
    ap.add_argument("checkpoint", nargs="?", default=CHECKPOINT, help="path: the script's checkpoint")
    ap.add_argument("--m32", action="store_true", help="also the -m32 build")
    ap.add_argument("--sanitize", action="store_true", help="also the ASan/UBSan build")
    ap.add_argument("--exe", help="this binary instead of build/port/dw2003")
    ap.add_argument("--wine", action="store_true", help="run --exe through wine (a Windows build), headless")
    ap.add_argument("--out", help="output directory (default build/port-test/savestate)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None)
    args = ap.parse_args()
    try:
        if not run.DISC.exists():
            raise Missing(f"no disc image ({run.DISC.relative_to(ROOT)}; scripts/setup.sh disc)")
        if args.exe and not Path(args.exe).is_file():
            raise Missing(f"no binary at {args.exe}")
        if args.wine:
            if not args.exe or shutil.which("wine") is None:
                raise Missing("--wine needs --exe (scripts/build_windows.sh) and wine on PATH")
            prefix = ROOT / "build/wine-prefix"
            prefix.mkdir(parents=True, exist_ok=True)
            run.RUNNER[:] = ["wine"]
            run.RUNNER_ENV.update(WINEPREFIX=str(prefix), WINEDEBUG="-all", SDL_VIDEO_DRIVER="dummy",
                                  SDL_AUDIO_DRIVER="dummy")
        if args.exe and (args.m32 or args.sanitize):
            raise Missing("--m32 and --sanitize build their own binaries: not with --exe")
        env = run.tool_env()
    except Missing as e:
        print(f"savestate: {e}", file=sys.stderr)
        return 2

    if args.command == "path":
        try:
            binary = Path(args.exe).resolve() if args.exe else run.build("build/port", [], args.jobs, env)
            print(state_path(binary, args.checkpoint))
        except (RuntimeError, subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
            print(f"savestate: {e}", file=sys.stderr)
            return 1
        return 0

    out = (Path(args.out) if args.out else ROOT / "build/port-test/savestate").resolve()
    out.mkdir(parents=True, exist_ok=True)
    try:
        variants = [("m64", Path(args.exe).resolve() if args.exe else run.build("build/port", [], args.jobs, env),
                     None)]
        if args.m32:
            variants.append(("m32", run.build("build/port-m32", ["-DPSXSTACK_M32=ON"], args.jobs, env), None))
        if args.sanitize:
            san_env = dict(os.environ, UBSAN_OPTIONS=f"print_stacktrace=1:suppressions={run.UBSAN_SUPPRESSIONS}",
                           ASAN_OPTIONS="detect_leaks=1:detect_stack_use_after_return=0")
            variants.append(("san", run.build("build/port-san", ["-DPSXSTACK_SANITIZE=ON"], args.jobs, env), san_env))
    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(f"savestate: FAIL: {e}")
        return 1
    for label, binary, venv in variants:
        check_variant(label, binary, out, venv)
    if FAILURES:
        print(f"savestate: {len(FAILURES)} failure(s)")
        for f in FAILURES:
            print(f"  {f}")
        return 1
    print("savestate: pass")
    return 0


if __name__ == "__main__":
    sys.exit(main())
