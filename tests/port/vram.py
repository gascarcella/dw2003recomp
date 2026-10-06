#!/usr/bin/env python3
"""The port's VRAM in the first battle against the emulator's (issue #7; M2's VRAM checks covered new_game only).

Usage: tests/port/vram.py [--out DIR] [--frames N] [--keep]

Runs first_battle_save cut after its checkpoint battle_start (FIGHTSTG loaded), then a `vram` step ("start": the
whole VRAM on that frame), `--frames` frames without input (default 300: the battle's loading and its opening camera
swing, up to the command menu waiting for a button) and a second `vram` step ("end"), in the emulator
(tests/replay/replay.py run_once, ~30 s) and in the port (build/port/dw2003, DW3_PORT_CHECKPOINT_DIR, ~10 s).

What can be compared, and what cannot: the displayed frames cannot. The PS1 is CPU-bound in the battle (a game frame
takes 2 or 3 vsyncs, the port's one), and the game advances its animations by the ticks elapsed (gfx_get_frame_ticks),
so the two runs pass through different frames; even with the ticks forced equal they diverge (the vsync-based time,
the random index and the CD loads differ too). The battle's camera is checked by the layer-1 family libgs_view instead.
What the battle puts in VRAM besides the frames is comparable: its textures and CLUTs. So, outside the two display
buffers (x < 320, both halves: SetDefDispEnv's buffers at y 0 and 256), every pixel that either run changed between
"start" and "end" must be equal in the two "end" images. Pixels neither run touched were there before the battle; their
differences are reported, not failed. They come from between the checkpoints asuka_lobby (the whole VRAM equal) and
battle_start: the field picture kept at x 640..959 differs where the sprites stand (the runs leave the field on
different frames), and a few areas drawn off screen before the battle differ in content (not explained yet: issue #23).
The battle's frames before it draws (its loading screens) are equal too, and are not checked.

Writes DIR/report.txt with a 64x64-tile map of the differences. Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import array
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
sys.path.insert(0, str(ROOT / "tests/replay"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)
import replay  # noqa: E402  (the emulator's runner)

OUT_DEFAULT = ROOT / "build/port-vram"
W, H = 1024, 512
DISPLAY_W = 320  # the battle's two display buffers: x 0..319, y 0..255 and 256..511
TILE = 64


def battle_script(frames, out):
    """first_battle_save up to battle_start, then the two VRAM dumps `frames` frames apart."""
    script = json.loads((SCRIPTS / "first_battle_save.json").read_text())
    steps = script["steps"]
    idx = next(i for i, s in enumerate(steps) if s.get("type") == "checkpoint" and s.get("name") == "battle_start")
    script["name"] = "first_battle_save@battle_vram"
    script["steps"] = steps[:idx + 1] + [
        {"type": "vram", "name": "start", "comment": "FIGHTSTG just loaded"},
        {"type": "wait_frames", "frames": frames - 1},
        {"type": "vram", "name": "end", "comment": "the battle's loading and opening camera done"},
    ]
    path = out / f"{script['name']}.json"
    path.write_text(json.dumps(script, indent=1) + "\n")
    return path, script


def load(path):
    data = array.array("H")
    data.frombytes(path.read_bytes())
    if len(data) != W * H:
        raise RuntimeError(f"{path}: {len(data) * 2} bytes, not one VRAM image")
    if sys.byteorder != "little":
        data.byteswap()
    return data


def compare(emu, port):
    """Returns (failures, leftovers): per 64x64 tile, the pixels outside the display buffers that differ at the end
    and that either run changed during the battle (failures), or that neither did (leftovers)."""
    es, ee, ps, pe = emu["start"], emu["end"], port["start"], port["end"]
    failures, leftovers = {}, {}
    for y in range(H):
        row = y * W
        for x in range(DISPLAY_W, W):
            i = row + x
            if ee[i] == pe[i]:
                continue
            tile = (x // TILE, y // TILE)
            touched = ee[i] != es[i] or pe[i] != ps[i]
            target = failures if touched else leftovers
            target[tile] = target.get(tile, 0) + 1
    return failures, leftovers


def tile_map(failures, leftovers):
    lines = ["differing pixels per 64x64 tile at the end (F: changed during the battle, a failure; l: left over from "
             "before it; ...: the display buffers, not compared)"]
    for ty in range(H // TILE):
        cells = []
        for tx in range(W // TILE):
            if tx * TILE < DISPLAY_W:
                cells.append("  ... ")
            elif (tx, ty) in failures:
                cells.append(f"F{failures[(tx, ty)]:5d}")
            elif (tx, ty) in leftovers:
                cells.append(f"l{leftovers[(tx, ty)]:5d}")
            else:
                cells.append("     .")
        lines.append(" ".join(cells))
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("--frames", type=int, default=300, help="frames from FIGHTSTG's load to the second dump")
    ap.add_argument("--keep", action="store_true", help="keep the VRAM dumps (4 MB)")
    args = ap.parse_args()
    out = Path(args.out).resolve()
    try:
        if not DISC.exists():
            raise Missing(f"no disc image ({DISC.relative_to(ROOT)})")
        if not replay.REDUX.exists():
            raise Missing("no emulator (scripts/setup.sh redux)")
        env = tool_env()
        binary = build("build/port", [], os.cpu_count() or 1, env)
    except Missing as e:
        print(f"vram: {e}")
        return 2
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    script_path, script = battle_script(args.frames, out)
    print(f"vram: {script['name']} ({args.frames} frames from FIGHTSTG's load)")

    print("  emulator")
    replay.run_once(script_path, script, replay.bios_path("openbios"), out / "emu")
    port_dir = out / "port"
    port_dir.mkdir()
    print("  port")
    proc = subprocess.run([str(binary), "--disc", str(DISC), "--script", str(script_path)],
                          env=dict(os.environ, DW3_PORT_CHECKPOINT_DIR=str(port_dir)), stdout=subprocess.DEVNULL,
                          stderr=subprocess.PIPE, text=True, timeout=600)
    if proc.returncode != 0:
        print(proc.stderr[-2000:])
        print(f"vram: FAIL: the port exited {proc.returncode}")
        return 1
    images = {}
    for side, d in (("emu", out / "emu"), ("port", port_dir)):
        images[side] = {n: load(d / f"vram_{n}.bin") for n in ("start", "end")}
    failures, leftovers = compare(images["emu"], images["port"])
    report = tile_map(failures, leftovers)
    (out / "report.txt").write_text(report + "\n")
    emu_start, emu_end = images["emu"]["start"], images["emu"]["end"]
    changed = sum(1 for i in range(W * H) if i % W >= DISPLAY_W and emu_end[i] != emu_start[i])
    if not args.keep:
        for side in ("emu", "port"):
            for n in ("start", "end"):
                (out / side / f"vram_{n}.bin").unlink()
    nf, nl = sum(failures.values()), sum(leftovers.values())
    print(f"  outside the display buffers: the emulator's battle changed {changed} pixels; {nf} differ at the end, "
          f"{nl} left over from before the battle differ (not a failure)")
    if nf:
        print(report)
        print(f"vram: FAIL ({out / 'report.txt'})")
        return 1
    print("vram: pass")
    return 0


if __name__ == "__main__":
    sys.exit(main())
