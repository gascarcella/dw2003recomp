#!/usr/bin/env python3
"""The texture dump (psxstack/runtime/render_gpu_textures.c; issue #70, psxstack docs/RUNTIME.md "Texture dump"):
`--dump-textures DIR` writes every texture a primitive samples, the first time, as a PNG named by its key (the SHA-1 of
the transfer it was loaded by, the SHA-1 of its CLUT, the depth, the size), and DIR/index.json. It runs under either
renderer and needs no GPU device, so CI runs it.

Usage: tests/port/textures.py [--out DIR] [-j N] [--update]

Builds build/port-sdl if needed (-DPSXSTACK_SDL=ON: tools/sdl3 and tools/dxc), then:
  - the option's errors: `--dump-textures` in the headless build (no SDL: exit 64);
  - with the disc: new_game under the software renderer, headless, with and without the dump: the per-frame log and the
    record byte for byte the same (the dump changes nothing the game sees); the dump's files exactly the names in
    tests/port/textures/new_game.keys (hashes only; --update rewrites it), each a PNG of its key's size (an indexed
    PNG for 4- and 8-bit textures, RGBA for 15-bit), index.json listing every file once with sampled ranges inside it;
  - the dump continued: a second run into the same directory reads index.json back, writes no PNG again and adds its
    draws to the counts;
  - with a GPU device (else skipped): the same run with the hardware renderer's rasteriser on (a --gpu-screenshot
    headless) gives the same files and the same index.json.
Exit codes: 0 pass (parts may be skipped), 1 fail, 2 something missing for the build.
"""
import argparse
import json
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)

KEYS = ROOT / "tests/port/textures/new_game.keys"
FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def run(cmd, env, timeout=600):
    proc = subprocess.run([str(c) for c in cmd], cwd=ROOT, env=env, capture_output=True, text=True, timeout=timeout)
    return proc.returncode, proc.stdout + proc.stderr


def png_info(path):
    """(width, height, colour type) from a PNG's IHDR: 3 indexed, 6 RGBA."""
    data = path.read_bytes()[:33]
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    w, h, _depth, ctype = struct.unpack(">IIBB", data[16:26])
    return w, h, ctype


def pngs(d):
    return sorted(str(p.relative_to(d)) for p in d.rglob("*.png"))


def new_game(sdl, env, out, extra):
    return run([sdl, "--disc", DISC, "--script", SCRIPTS / "new_game.json", *extra], env)


def check_dump(d, update):
    names = pngs(d)
    if update:
        KEYS.parent.mkdir(parents=True, exist_ok=True)
        KEYS.write_text("".join(n + "\n" for n in names))
        print(f"  wrote {KEYS.relative_to(ROOT)} ({len(names)} keys)")
    want = KEYS.read_text().split() if KEYS.exists() else []
    missing, extra = sorted(set(want) - set(names)), sorted(set(names) - set(want))
    check(names == sorted(want), f"the dump's {len(names)} files are {KEYS.relative_to(ROOT)}'s {len(want)}"
          + (f" (missing {missing[:3]}, extra {extra[:3]})" if missing or extra else ""))
    index = json.loads((d / "index.json").read_text())
    entries = index["textures"]
    check(index["schema"] == 1 and sorted(e["file"] for e in entries) == names,
          f"index.json lists each file once ({len(entries)} entries)")
    bad = []
    for e in entries:
        info = png_info(d / e["file"])
        w, h = e["size"]
        u0, v0, u1, v1 = e["uv"]
        name = Path(e["file"]).name
        stem = f"{e['image']}-15bpp" if e["depth"] == 15 else f"{e['image']}-{e['clut']}-{e['depth']}bpp"
        if name != f"{stem}-{w}x{h}.png":
            bad.append(f"{e['file']}: the name is not its key")
        elif info != (w, h, 6 if e["depth"] == 15 else 3):
            bad.append(f"{e['file']}: PNG {info}, expected {w}x{h}")
        elif not (0 <= u0 <= u1 < w and 0 <= v0 <= v1 < h) or e["draws"] < 1:
            bad.append(f"{e['file']}: uv {e['uv']} outside {w}x{h} or no draws")
    check(not bad, "every PNG has its key's name, size and format; every sampled range is inside it"
          + (f" ({'; '.join(bad[:3])})" if bad else ""))
    return entries


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(ROOT / "build/textures-test"), help="work directory (wiped)")
    ap.add_argument("-j", "--jobs", type=int, default=0, help="build jobs (default: ninja's)")
    ap.add_argument("--update", action="store_true", help=f"rewrite {KEYS.relative_to(ROOT)} from this run")
    args = ap.parse_args()
    out = Path(args.out)
    try:
        env = tool_env()
        for tool in ("sdl3", "dxc"):
            if not (ROOT / "tools" / tool).exists():
                print(f"textures: no tools/{tool} (scripts/setup.sh {tool}): skipped")
                return 0
        sdl = build("build/port-sdl", ["-DPSXSTACK_SDL=ON"], args.jobs, env)
    except Missing as e:
        print(f"textures: {e}")
        return 2
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_AUDIO_DRIVER="dummy")

    print("textures: the option")
    headless = ROOT / "build/port/dw2003"
    if headless.exists():
        rc, text = run([headless, "--dump-textures", out / "x"], env)
        check(rc == 64 and "--dump-textures: this build has no texture dump" in text,
              f"--dump-textures in the headless build: exit 64 (exit {rc})")
    if not DISC.exists():
        print("textures: no disc (iso/dw2003.cue): the dump is skipped")
    else:
        print("textures: new_game dumped under the software renderer")
        rc, text = new_game(sdl, env, out, ["--log", out / "plain.log", "--record", out / "plain.json"])
        check(rc == 0, f"new_game without the dump (exit {rc})")
        dump = out / "dump"
        rc, text = new_game(sdl, env, out, ["--log", out / "dump.log", "--record", out / "dump.json",
                                            "--dump-textures", dump])
        check(rc == 0 and "textures: dumping to" in text, f"new_game with --dump-textures (exit {rc})")
        check((out / "plain.log").read_bytes() == (out / "dump.log").read_bytes()
              and (out / "plain.json").read_bytes() == (out / "dump.json").read_bytes(),
              "the per-frame log and the record are the run's without the dump, byte for byte")
        entries = check_dump(dump, args.update)

        print("textures: the dump continued")
        stamps = {p: p.stat().st_mtime_ns for p in dump.rglob("*.png")}
        rc, text = new_game(sdl, env, out, ["--dump-textures", dump])
        again = json.loads((dump / "index.json").read_text())["textures"]
        check(rc == 0 and f"{len(entries)} keys from earlier dumps" in text, "index.json read back")
        check(all(p.stat().st_mtime_ns == m for p, m in stamps.items()) and pngs(dump) == sorted(map(str, (
              p.relative_to(dump) for p in stamps))), "no PNG written again, none added")
        before = {e["file"]: e["draws"] for e in entries}
        check(len(again) == len(entries) and all(e["draws"] == 2 * before[e["file"]] for e in again),
              "the draws of the second run added to the counts")

        print("textures: the same with the hardware renderer's rasteriser")
        gpu = out / "dump-gpu"
        rc, text = new_game(sdl, env, out, ["--dump-textures", gpu, "--gpu-screenshot", f"1800:{out / 'g.ppm'}"])
        if "renderer: gpu for the screenshots" not in text:
            why = next((l.split("unavailable ", 1)[1] for l in text.splitlines() if "gpu unavailable" in l), "?")
            print(f"  skipped: no GPU device ({why})")
        else:
            check(rc == 0 and pngs(gpu) == pngs(dump), "the same files")
            check(json.loads((gpu / "index.json").read_text())["textures"] == entries, "the same index.json")
    print(f"textures test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
