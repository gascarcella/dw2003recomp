#!/usr/bin/env python3
"""The texture dump and texture packs (psxstack/runtime/render_gpu_textures.c, render_gpu_packs.c; issue #70, psxstack
docs/RUNTIME.md "Texture dump", "Texture packs"): `--dump-textures DIR` writes every texture a primitive samples, the
first time, as a PNG named by its key (the SHA-1 of the transfer it was loaded by, the SHA-1 of its CLUT, the depth, the
size), and DIR/index.json; `--texture-pack DIR` loads PNGs named so, which the hardware renderer draws instead. The
dump runs under either renderer and needs no GPU device, so CI runs it; the packs' pictures need a device.

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
    headless) gives the same files and the same index.json;
  - `--texture-pack` in the headless build (exit 64) and a directory that is not a pack (exit 1);
  - packs made from that dump, with a GPU device (else skipped), new_game at internal scale 1: the identity pack (the
    dump's own PNGs, `nearest`) keeps the rasteriser's whole VRAM target equal to the software VRAM every 10 vsyncs
    and the pictures unchanged; a pack of 1x1 magenta PNGs (every key) changes them; the same as sub-rectangle files
    (each key's sampled range from index.json) gives the same pictures; the identity pack given before the magenta one
    wins; at internal scale 2 the identity pack's pictures are within SCALED_BUDGET of those without it (the colour is
    8-bit there, the texture coordinates exact).
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
FRAMES = (900, 1300, 1800)   # new_game: the title's menus, the field
# At internal scale 2 an identity pack's picture may differ from the one without it by at most this mean per channel,
# and by more than 8 in at most this share of the channels. Above scale 1 a replacement's colour is its 8 bits, the
# dump's widening c << 3 | c >> 2 where the VRAM's texel gives c << 3 (up to 7 more), and its texture coordinates are
# exact where gpu.c's are rounded (a texel's edge). Measured on NVIDIA: new_game's title 3.8 mean, none over 8;
# first_battle_save's field 2.9, none over 7; its battle 1.9, 0.015 % over 8.
SCALED_BUDGET = (6.0, 0.001)
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


def write_png(path, w, h, rgba):
    """An 8-bit RGBA PNG."""
    import zlib
    raw = b"".join(b"\0" + rgba[y * w * 4:(y + 1) * w * 4] for y in range(h))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
                     + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


def make_pack(d, pid, filt, files):
    """A pack at d: mod.json and files {relative path: (w, h, rgba) or a source PNG to copy}."""
    (d / "textures").mkdir(parents=True)
    (d / "mod.json").write_text(json.dumps({"schema": 1, "id": pid, "name": pid, "kind": "data",
                                            "textures": {"dir": "textures", "filter": filt}}))
    for rel, content in files.items():
        if isinstance(content, Path):
            (d / "textures" / rel).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(content, d / "textures" / rel)
        else:
            write_png(d / "textures" / rel, *content)


def ppm(path):
    data = path.read_bytes()
    _magic, size, _depth, pixels = data.split(b"\n", 3)
    return tuple(map(int, size.split())), pixels


def shots(sdl, env, out, tag, extra, scale=1, check_every=0):
    """new_game's hardware pictures at FRAMES with these options; (exit, output, {frame: ppm})."""
    args = ["--gpu-screenshot", "1:/dev/null", "--internal-scale", str(scale)]
    for f in FRAMES:
        args += ["--gpu-screenshot", f"{f}:{out / f'{tag}_{f}.ppm'}"]
    rc, text = new_game(sdl, dict(env, DW3_PORT_GPU_VRAM_CHECK=str(check_every)) if check_every else env, out,
                        args + extra)
    return rc, text, {f: ppm(out / f"{tag}_{f}.ppm") for f in FRAMES if (out / f"{tag}_{f}.ppm").exists()}


def packs(sdl, env, out, dump, entries):
    print("textures: packs (the hardware renderer at internal scale 1, then 2)")
    magenta = (1, 1, bytes((255, 0, 255, 255)))
    make_pack(out / "identity", "identity", "nearest",
              {str(p.relative_to(dump)): p for p in dump.rglob("*.png")})
    make_pack(out / "magenta", "magenta", "linear", {Path(e["file"]).name: magenta for e in entries})
    subs = {}
    for e in entries:
        u0, v0, u1, v1 = e["uv"]
        subs[Path(e["file"]).name[:-4] + f"@{u0},{v0},{u1 - u0 + 1}x{v1 - v0 + 1}.png"] = magenta
    make_pack(out / "magenta-sub", "magenta_sub", "linear", subs)
    rc, text, plain = shots(sdl, env, out, "plain", [])
    if "renderer: gpu for the screenshots" not in text:
        why = next((l.split("unavailable ", 1)[1] for l in text.splitlines() if "gpu unavailable" in l), "?")
        print(f"  skipped: no GPU device ({why})")
        return
    check(rc == 0 and len(plain) == len(FRAMES), f"new_game's pictures without a pack (exit {rc})")
    rc, text, ident = shots(sdl, env, out, "identity", ["--texture-pack", out / "identity"], check_every=10)
    total = next((l.split("gpu vram check: ", 1)[1] for l in text.splitlines() if " checks, " in l), "no checks")
    used = next((l.split("texture packs: ", 1)[1] for l in text.splitlines() if "textures uploaded" in l), "none used")
    check(rc == 0 and total.endswith(" 0 with differences") and "uploaded" in used,
          f"the identity pack: the whole VRAM target equal to the software VRAM ({total}; {used})")
    check(ident == plain, "the identity pack: the pictures unchanged")
    rc, text, mag = shots(sdl, env, out, "magenta", ["--texture-pack", out / "magenta"])
    changed = sum(1 for f in FRAMES if mag.get(f) != plain[f])
    check(rc == 0 and changed == len(FRAMES), f"the magenta pack changes the pictures ({changed} of {len(FRAMES)})")
    rc, text, sub = shots(sdl, env, out, "magenta-sub", ["--texture-pack", out / "magenta-sub"])
    check(rc == 0 and sub == mag, "the magenta pack as sub-rectangle files: the same pictures")
    rc, text, both = shots(sdl, env, out, "both", ["--texture-pack", out / "identity", "--texture-pack",
                                                   out / "magenta"])
    check(rc == 0 and both == plain, "the identity pack given first wins over the magenta one")
    rc, text, plain2 = shots(sdl, env, out, "plain2", [], scale=2)
    rc2, text, ident2 = shots(sdl, env, out, "identity2", ["--texture-pack", out / "identity"], scale=2)
    worst = (0.0, 0.0)
    for f in FRAMES:
        a, b = plain2.get(f), ident2.get(f)
        if a is None or b is None or a[0] != b[0]:
            worst = (float("inf"), 1.0)
            continue
        diffs = [abs(x - y) for x, y in zip(a[1], b[1])]
        worst = (max(worst[0], sum(diffs) / len(diffs)), max(worst[1], sum(1 for d in diffs if d > 8) / len(diffs)))
    check(rc == 0 and rc2 == 0 and worst[0] <= SCALED_BUDGET[0] and worst[1] <= SCALED_BUDGET[1],
          f"internal scale 2: the identity pack within the budget (mean {worst[0]:.2f}, over 8: "
          f"{100 * worst[1]:.3f} %)")


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
        rc, text = run([headless, "--texture-pack", out / "x"], env)
        check(rc == 64 and "--texture-pack: this build has no hardware renderer" in text,
              f"--texture-pack in the headless build: exit 64 (exit {rc})")
    (out / "not-a-pack").mkdir()
    rc, text = run([sdl, "--max-frames", "1", "--texture-pack", out / "not-a-pack"], env)
    check(rc == 1 and "no mod.json" in text, f"--texture-pack with a directory that is not a pack: exit 1 (exit {rc})")
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
        first = out / "dump-first"
        rc, text = new_game(sdl, env, out, ["--dump-textures", first])
        packs(sdl, env, out, first, json.loads((first / "index.json").read_text())["textures"])
    print(f"textures test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
