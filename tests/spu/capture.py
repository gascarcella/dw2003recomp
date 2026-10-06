#!/usr/bin/env python3
"""Our SPU's rendering of the committed CNTY_SEL trace against what the emulator played for the same run.

  tools/venv/bin/python tests/spu/capture.py [--keep DIR]      # ~1 min: one emulator run at speed 1, then the compare

PCSX-Redux has no audio dump, but it plays through SDL3, and SDL3's `disk` driver writes what it plays (F32LE stereo,
44,100 Hz; docs/SOUND.md section 4). The capture is paced by the host, so it has gaps and is not a golden; this script
cuts our render (tests/spu/render_trace.py's WAV of tests/sound/expected/cnty_sel.trace: the same script, new_game to
cnty_sel + 300 frames) into windows and finds each window in the capture by cross-correlation
(tests/spu/capture_compare.c): per window the best lag, the normalized correlation, and the level ratio. Redux's SPU is
an emulation itself: this checks that the two agree on what the trace sounds like (pitch, samples, envelopes, volumes,
reverb), not bit-exactness. The run is the dynarec's (frame timing differs a little from the interpreter's trace; the
alignment absorbs it).
"""
import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "tests/replay"))
sys.path.insert(0, str(ROOT / "tests/sound"))
import replay  # noqa: E402
import spu_trace  # noqa: E402

RATE = 877.4


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--keep", help="keep the emulator run's files (and the capture) here")
    ap.add_argument("--capture", help="use this capture (F32LE stereo) instead of running the emulator")
    args = ap.parse_args()
    build = ROOT / "build/spu_test"
    # Redux renders ~877.4 samples per PAL vsync (tests/spu/envelope_oracle.py's fit), not 882: our rendering takes the
    # emulator's pace so that a window's notes start where the capture's do.
    ours = build / "cnty_sel_877.wav"
    subprocess.run([sys.executable, str(HERE / "render_trace.py"), "--frames-per-tick", str(RATE), "--out", str(ours)],
                   check=True, stdout=subprocess.DEVNULL)
    tool = build / "capture_compare"
    subprocess.run(["gcc", "-std=gnu99", "-O2", "-Wall", "-Wextra", "-Werror", str(HERE / "capture_compare.c"),
                    "-o", str(tool), "-lm"], check=True)
    if args.capture:
        capture = Path(args.capture)
    else:
        replay.check_tools()
        out = Path(args.keep) if args.keep else Path(tempfile.mkdtemp(prefix="dw3_spu_capture_"))
        out.mkdir(parents=True, exist_ok=True)
        capture = out / "capture.f32"
        script_path = ROOT / "tests/replay/scripts/new_game.json"
        script = spu_trace.cut_script(replay.load_script(script_path), "cnty_sel", 300)
        os.environ["SDL_AUDIO_DRIVER"] = "disk"
        os.environ["SDL_AUDIO_DISK_OUTPUT_FILE"] = str(capture)
        replay.run_once(script_path, script, replay.bios_path("openbios"), out, speed=1)
        print(f"capture: {capture} ({capture.stat().st_size} bytes)")
    # The trace starts at tick 513; CNTY_SEL's music keys on at tick 869: compare from there to the end.
    start = int((869 - 513) * RATE)
    r = subprocess.run([str(tool), str(ours), str(capture), str(start)])
    return r.returncode


if __name__ == "__main__":
    sys.exit(main())
