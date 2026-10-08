#!/usr/bin/env python3
"""The PC port's test: the port replays the layer-2 scripts and must reach what the emulator reached.

Usage: tests/port/run.py [SCRIPT ...] [--m32] [--sanitize] [--cd-speed instant|realistic] [--out DIR] [-j N]
       tests/port/run.py [SCRIPT ...] --exe build/port-win/dw2003.exe --wine     # the Windows build, under Wine

Builds the port if needed (cmake -S port -B build/port -G Ninja; cmake --build), and for each script (default: every
tests/replay/scripts/<name>.json with a tests/replay/expected/<name>.json; or the names given) runs
  build/port/dw2003 --disc iso/dw2003.cue --script tests/replay/scripts/<name>.json --log ... --record ...
twice and requires the two logs and the two records to be byte-identical (determinism), then compares the record's
cross-core view (tests/replay/replay.py cross_core_view, compare) with tests/replay/expected/<name>.json: the same
checkpoints (name, stage, map, stable gamestate hash) and the same overlay and map sequences, without frames.
The sound (tests/port/sound.py): once, LIBSND replayed on the emulator's timeline must give the committed emulator SPU
traces exactly (tests/sound/expected/*.trace, tests/port/sound/*.trace.gz); per script, the runs' SPU traces
(--spu-trace) must be identical too, the run's trace must equal LIBSND's replay of its own calls (skipped while
psxstack/runtime/audio.c is the stub that renders nothing), and where an emulator trace is committed the game must make the
same LIBSND calls (the segments between them are reported: docs/SOUND.md section 7).
  --m32       also build build/port-m32 (-DPSXSTACK_M32=ON, needs gcc-multilib) and require its cross-core view to equal
              the 64-bit build's, and for new_game its log and record byte for byte (the layout check: pointers are 4
              bytes there, as on the PS1; M32_LOG_EXACT says why the other scripts' frames may differ)
  --sanitize  also build build/port-san (-DPSXSTACK_SANITIZE=ON), run it once, and fail on any ASan/UBSan report
              (its log and record must equal the plain build's too)
  --cd-speed  the port's CD timing (default: the port's, realistic)
  --exe PATH  run this binary instead of building build/port; build/port is only configured (the run's own sound
              check compiles against its generated headers), and LIBSND's replay on the emulator's timeline is
              skipped (it tests the host's LIBSND, the plain run's job); --m32 and --sanitize do not apply to it
  --wine      run the binary (--exe, a Windows build: scripts/build_windows.sh) through `wine`, headless (SDL's
              dummy video and audio drivers, the prefix in build/wine-prefix/); its log and record must equal the
              Linux build's, which is what tests/replay/expected/ holds
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
sys.path.insert(0, str(ROOT / "tests/port"))
import sound  # noqa: E402  (the sound checks: tests/port/sound.py)

SCRIPTS = ROOT / "tests/replay/scripts"
EXPECTED = ROOT / "tests/replay/expected"
DISC = ROOT / "iso/dw2003.cue"
VENV_BIN = ROOT / "tools/venv/bin"
RUN_TIMEOUT = 600  # seconds; a plain run takes well under one, the sanitizer build a few
# Scripts whose -m32 log must equal the -m64 log byte for byte (the M1 criterion). In the others the game frees cached
# files by heap address (FIELDSTG's cdload free_above(0x8015C674)), and the host's heap layout differs between the two
# pointer widths (as both differ from the PS1's), so which files stay cached, and so the CD timing and the frames, can
# differ; their cross-core view must still be the same (DECISIONS "The port is checked against the emulator").
M32_LOG_EXACT = ("new_game",)
# Known out-of-bounds indexes of the game's own C that stay inside one struct whose layout is the same on the host
# (tests/host/FINDINGS.md 5 and 10): UBSan's bounds check reports them, the result is the PS1's.
UBSAN_SUPPRESSIONS = ROOT / "tests/port/ubsan.supp"
SANITIZER_MARKS = ("runtime error:", "ERROR: AddressSanitizer", "ERROR: LeakSanitizer", "SUMMARY: AddressSanitizer",
                   "SUMMARY: UndefinedBehaviorSanitizer")


class Missing(Exception):
    """Something the test needs is not there (exit 2)."""


RUNNER = []          # the command prefix of a run (--wine: ["wine"])
RUNNER_ENV = {}      # its environment additions (--wine: the prefix, no debug output, SDL's dummy drivers)


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


def configure(build_dir, options, env):
    """Configures the port into build_dir once (the generated headers are written then); returns the directory."""
    build_dir = ROOT / build_dir
    if not (build_dir / "CMakeCache.txt").exists():
        print(f"  configure {build_dir.relative_to(ROOT)} {' '.join(options)}".rstrip())
        subprocess.run(["cmake", "-S", str(ROOT / "port"), "-B", str(build_dir), "-G", "Ninja", *options],
                       check=True, env=env, stdout=subprocess.DEVNULL)
    return build_dir


def build(build_dir, options, jobs, env):
    """Configures (once) and builds the port into build_dir; returns the binary."""
    build_dir = configure(build_dir, options, env)
    cmd = ["cmake", "--build", str(build_dir)] + (["-j", str(jobs)] if jobs else [])
    proc = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if proc.returncode != 0:
        sys.stdout.write(proc.stdout[-4000:] + proc.stderr[-4000:])
        raise RuntimeError(f"the build of {build_dir.relative_to(ROOT)} failed")
    return build_dir / "dw2003"


def run_port(binary, script, out_dir, label, cd_speed, env=None):
    """One run of the script, with its SPU trace (<label>.spu.trace); returns (log bytes, record bytes, record, stderr
    text, SPU trace bytes)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    dumps = out_dir / f"{label}_checkpoints"
    if dumps.exists():
        shutil.rmtree(dumps)
    dumps.mkdir()
    log, record, err = out_dir / f"{label}.log", out_dir / f"{label}.json", out_dir / f"{label}.stderr"
    spu = out_dir / f"{label}.spu.trace"
    cmd = [*RUNNER, str(binary), "--disc", str(DISC), "--script", str(script), "--log", str(log), "--record",
           str(record), "--spu-trace", str(spu)]
    if cd_speed:
        cmd += ["--cd-speed", cd_speed]
    run_env = dict(env or os.environ, DW3_PORT_CHECKPOINT_DIR=str(dumps), **RUNNER_ENV)
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
    return log.read_bytes(), record.read_bytes(), rec, err_text, spu.read_bytes()


def same_output(a, b, what):
    """Byte comparison of two runs' (log, record, SPU trace); returns the differences as text."""
    diffs = []
    for i, name in ((0, "log"), (1, "record"), (4, "SPU trace")):
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


def check_script(name, args, env, out):
    """The test of one script; returns its failures."""
    script, expected_path = SCRIPTS / f"{name}.json", EXPECTED / f"{name}.json"
    expected = json.loads(expected_path.read_text())
    if expected.get("script_sha1") != sha1_file(script):
        return [f"{script.relative_to(ROOT)} changed since {expected_path.relative_to(ROOT)} was recorded "
                "(re-record it with tests/replay/replay.py run --record)"]
    out = out / name
    out.mkdir(parents=True, exist_ok=True)
    failures = []
    print(f"port test: {script.relative_to(ROOT)} vs {expected_path.relative_to(ROOT)} (cross-core view)")
    try:
        binary = Path(args.exe).resolve() if args.exe else build("build/port", [], args.jobs, env)
        if args.exe:
            print(f"  binary: {binary}{' under wine' if args.wine else ''}")
        run1 = run_port(binary, script, out, "run1", args.cd_speed)
        run2 = run_port(binary, script, out, "run2", args.cd_speed)
        rec = run1[2]
        print(f"  runs: {rec['frames']} frames (emulator {expected['frames']}), {rec['reason']}; exit 0")
        diffs = same_output(run1, run2, "run 1 vs run 2")
        failures += diffs
        if not diffs:
            print("  determinism: two runs, identical logs, records and SPU traces")
        summary(expected, rec)
        diffs = compare(cross_core_view(expected), cross_core_view(rec))
        if diffs:
            failures += diffs
            print(f"  checkpoint dumps (gamestate_data's PS1 image): {out / 'run1_checkpoints'}; the emulator's: "
                  f"tests/replay/replay.py run {script.relative_to(ROOT)} -v --out DIR")
        else:
            print(f"  cross-core view: matches {expected_path.relative_to(ROOT)}")
        if args.m32:
            m32 = build("build/port-m32", ["-DPSXSTACK_M32=ON"], args.jobs, env)
            run = run_port(m32, script, out, "m32", args.cd_speed)
            diffs = compare(cross_core_view(rec), cross_core_view(run[2]))
            failures += [f"-m32: {d}" for d in diffs]
            log_diffs = same_output(run1, run, "-m64 vs -m32")
            if name in M32_LOG_EXACT:
                failures += log_diffs
            if not diffs and not log_diffs:
                print("  -m32: log, record and SPU trace identical to the 64-bit build's")
            elif not diffs:
                print("  -m32: the same cross-core view; the frames differ (FIELDSTG's free_above keeps other files "
                      "cached at another heap layout: CD timing only): " + log_diffs[0].split(": ", 1)[1][:120])
        if args.sanitize:
            san = build("build/port-san", ["-DPSXSTACK_SANITIZE=ON"], args.jobs, env)
            san_env = dict(os.environ, UBSAN_OPTIONS=f"print_stacktrace=1:suppressions={UBSAN_SUPPRESSIONS}",
                           ASAN_OPTIONS=os.environ.get("ASAN_OPTIONS", "detect_leaks=1"))
            run = run_port(san, script, out, "san", args.cd_speed, san_env)
            reports = [l for l in run[3].splitlines() if any(m in l for m in SANITIZER_MARKS)]
            if reports:
                failures.append(f"sanitizer: {len(reports)} report line(s) in {out / 'san.stderr'}: {reports[0]}")
            else:
                print("  sanitizer: no ASan/UBSan report")
            diffs = same_output(run1, run, "plain vs sanitizer build")
            failures += diffs
            if not diffs:
                print("  sanitizer build: log, record and SPU trace identical to the plain build's")
        # The sound: the run's SPU trace against LIBSND's replay of its calls, and against the emulator's trace
        # (tests/port/sound/<name>.trace.gz, where one is committed).
        emu_trace = sound.PORT_EXPECTED / f"{name}.trace.gz"
        failures += sound.check_port_run(out / "run1.spu.trace", emu_trace if emu_trace.exists() else None,
                                         out / "sound", rendered=audio_renders(binary, out))
    except (RuntimeError, subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
        failures.append(str(e))
    return failures


def audio_renders(binary, out):
    """Whether the port's audio output renders the SPU (psxstack/runtime/audio.c past its M3 step-0 stub, which refuses --wav):
    LIBSND's replay of a run is exact only if the run rendered 882 samples a vsync, as the replay does."""
    proc = subprocess.run([*RUNNER, str(binary), "--max-frames", "1", "--wav", str(out / "audio_probe.wav")], cwd=ROOT,
                          env=dict(os.environ, **RUNNER_ENV), capture_output=True, text=True, timeout=120)
    return proc.returncode == 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("scripts", nargs="*", help="script names (default: every script with an expected file)")
    ap.add_argument("--m32", action="store_true", help="also the -m32 build; its log and record must be the same")
    ap.add_argument("--sanitize", action="store_true", help="also an ASan/UBSan build; no report allowed")
    ap.add_argument("--cd-speed", choices=("instant", "realistic"), help="the port's CD timing")
    ap.add_argument("--out", help="output directory (default build/port-test/)")
    ap.add_argument("--exe", help="run this binary instead of build/port/dw2003 (no --m32/--sanitize for it)")
    ap.add_argument("--wine", action="store_true", help="run --exe through wine (a Windows build), headless")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None,
                    help="build jobs (default: $DW3_JOBS, else Ninja's)")
    args = ap.parse_args()

    try:
        if not DISC.exists():
            raise Missing(f"no disc image ({DISC.relative_to(ROOT)}; scripts/setup.sh disc)")
        env = tool_env()
        if args.exe and not Path(args.exe).is_file():
            raise Missing(f"no binary at {args.exe}")
        if args.wine:
            if not args.exe:
                raise Missing("--wine needs --exe (the Windows build: scripts/build_windows.sh)")
            if shutil.which("wine") is None:
                raise Missing("no wine on PATH")
            prefix = ROOT / "build/wine-prefix"
            prefix.mkdir(parents=True, exist_ok=True)
            RUNNER[:] = ["wine"]
            RUNNER_ENV.update(WINEPREFIX=str(prefix), WINEDEBUG="-all", SDL_VIDEO_DRIVER="dummy",
                              SDL_AUDIO_DRIVER="dummy")
        if args.exe and (args.m32 or args.sanitize):
            raise Missing("--m32 and --sanitize build their own binaries: not with --exe")
    except Missing as e:
        print(f"port test: {e}", file=sys.stderr)
        return 2
    names = args.scripts or sorted(p.stem for p in EXPECTED.glob("*.json") if (SCRIPTS / p.name).exists())
    for name in names:
        if not (SCRIPTS / f"{name}.json").exists() or not (EXPECTED / f"{name}.json").exists():
            print(f"port test: no script or no expected file for {name}", file=sys.stderr)
            return 2
    out = Path(args.out) if args.out else ROOT / "build/port-test"
    out.mkdir(parents=True, exist_ok=True)
    out = out.resolve()

    failed = []
    try:  # the sound replays below compile against the port's generated headers (build/port/gen): build it first
        if args.exe:
            configure("build/port", [], env)  # the headers only: the binary under test is --exe's
        else:
            build("build/port", [], args.jobs, env)
    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(f"port test: FAIL: {e}")
        return 1
    # LIBSND on the emulator's timeline, once (the committed emulator traces; tests/port/sound.py replay): the host's
    # LIBSND through gcc, so not with --exe (another binary is under test; the plain run covers it)
    if not args.exe:
        try:
            variants = ["m64"] + (["m32"] if args.m32 else []) + (["san"] if args.sanitize else [])
            failures = sound.check_libsnd(variants, out / "sound")
        except (sound.Missing, RuntimeError) as e:
            failures = [str(e)]
        if failures:
            print("port test: sound: FAIL\n  " + "\n  ".join(failures))
            failed.append("sound")
    for name in names:
        failures = check_script(name, args, env, out)
        if failures:
            print(f"port test: {name}: FAIL\n  " + "\n  ".join(failures))
            failed.append(name)
    if failed:
        print(f"port test: FAIL ({', '.join(failed)}; outputs: {out})")
        return 1
    print(f"port test: pass ({', '.join(names)}; outputs: {out})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
