#!/usr/bin/env python3
"""The PC port's test: the port replays the layer-2 scripts and must reach what the emulator reached.
The test is psxstack's (psxstack/tools/replay/port_test.py; GAME_CONTRACT.md "6. Tests"): this file is this game's
configuration of it (the disc, the scripts, the venv, the UBSan suppressions, the -m32 rule) plus the sound checks
(tests/port/sound.py) hooked in, and the names the other port tests import from here.

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
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "psxstack/tools/replay"))
import port_test  # noqa: E402  (psxstack's test)
from port_test import (RUNNER, RUNNER_ENV, SANITIZER_MARKS, Missing, audio_renders, build,  # noqa: E402,F401
                       compare, cross_core_view, run_port, same_output, sha1_file, summary, tool_env)
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


def before_scripts(variants, out):
    """LIBSND on the emulator's timeline, once (the committed emulator traces; tests/port/sound.py replay): the host's
    LIBSND through gcc, so not with --exe (another binary is under test; the plain run covers it)."""
    try:
        return sound.check_libsnd(variants, out / "sound")
    except (sound.Missing, RuntimeError) as e:
        return [str(e)]


def after_script(name, out, binary, run1):
    """The sound: the run's SPU trace against LIBSND's replay of its calls, and against the emulator's trace
    (tests/port/sound/<name>.trace.gz, where one is committed)."""
    emu_trace = sound.PORT_EXPECTED / f"{name}.trace.gz"
    return sound.check_port_run(out / "run1.spu.trace", emu_trace if emu_trace.exists() else None, out / "sound",
                                rendered=audio_renders(binary, out))


CFG = port_test.configure(
    root=ROOT, game_json=ROOT / "port/game/game.json", disc=DISC, scripts_dir=SCRIPTS, expected_dir=EXPECTED,
    venv_bin=VENV_BIN, ubsan_suppressions=UBSAN_SUPPRESSIONS, m32_log_exact=M32_LOG_EXACT,
    m32_note="FIELDSTG's free_above keeps other files cached at another heap layout: CD timing only",
    run_timeout=RUN_TIMEOUT, before_scripts=before_scripts, after_script=after_script)
configure = port_test.configure_build   # (build_dir, options, env): the port configured once, the headers written


def main():
    return port_test.main()


if __name__ == "__main__":
    sys.exit(main())
