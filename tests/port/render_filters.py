#!/usr/bin/env python3
"""The hardware renderer's present filters (psxstack/runtime/render_gpu_present.c; issue #69): `--filter`,
video.filter. The options and the software renderer's refusal run everywhere (CI too); the pictures need a GPU device
and the disc and run locally.

Usage: tests/port/render_filters.py [--out DIR] [-j N] [--lavapipe] [--filter NAME ...]

Builds build/port-sdl if needed (-DPSXSTACK_SDL=ON: tools/sdl3 and tools/dxc), then:
  - the options: a bad --filter exits 64 with the choices; the headless build accepts a filter; the software
    renderer's window (and the GPU renderer's fallback without a device) logs that the picture stays unfiltered and
    passes the input self-test;
  - with a GPU device and the disc (otherwise skipped, with the reason): new_game replayed with each filter, its
    --gpu-screenshot F@WxH pictures against a reference computed here from the unfiltered picture (--gpu-screenshot F,
    which tests/port/render_gpu.py checks against the software image), at internal scales 1, 2 and 3 (RUNS: integer
    and non-integer scales, letterboxing, a window smaller than the picture). A filter's reference mirrors its shader (psxstack/shaders/present_<filter>.frag.hlsl) in float64;
    the GPU computes in float32, so each filter has a tolerance per channel (TOLERANCE: the largest difference, and
    the share of channels that may differ at all).
--lavapipe runs on Mesa's software Vulkan driver (VK_DRIVER_FILES) instead of the default device.
Exit codes: 0 pass (parts may be skipped), 1 fail, 2 something missing for the build.
"""
import argparse
import math
import os
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)
from render_gpu import LAVAPIPE, dest, ppm, run  # noqa: E402  (video.c's placement, the PPM reader)

# (internal scale, vsyncs, output sizes). The pictures: the title's menu (900) and the field (1800), both 320x240
# 15-bit displays from the VRAM target, and the disabled display (2: black, the 32-bit image's path). At scale 1 every
# 4:3 window is an integer multiple (a filter differs from nearest only in a window smaller than the picture, 300x200);
# at scale 2 a 640x480 picture in 960x720 is a scale of 3/2, at scale 3 a 960x720 one in 1920x1080 of 4/3.
RUNS = ((1, (2, 900, 1800), ((960, 720), (1366, 768), (300, 200))),
        (2, (900, 1800), ((960, 720), (1366, 768), (1000, 700))),
        (3, (1800,), ((1920, 1080), (300, 200))))
# filter: (the largest difference per channel, the share of channels that may differ). Measured, sharp: exact at
# integer scales on both devices; off by 1 where a weight is not exact in float32 (thirds at 3/2): NVIDIA RTX 4070 Ti
# SUPER 3.4 % of the channels at 3/2, 0.16 % at 4/3; lavapipe 6.5 % at 3/2, none at 4/3.
TOLERANCE = {"sharp": (1, 0.10)}
FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def box(px, w, h, ox, oy, dw, dh):
    """present_source.hlsli box(): output pixel (ox, oy) of the rectangle as the average of the source pixels whose
    centres fall in it (at most 8 x 8)."""
    lo_x = (2 * ox * w + dw - 1) // (2 * dw)
    hi_x = max(((2 * ox + 2) * w + dw - 1) // (2 * dw), lo_x + 1)
    lo_y = (2 * oy * h + dh - 1) // (2 * dh)
    hi_y = max(((2 * oy + 2) * h + dh - 1) // (2 * dh), lo_y + 1)
    s, n = [0, 0, 0], 0
    for y in range(lo_y, min(hi_y, lo_y + 8)):
        row = min(max(y, 0), h - 1) * w
        for x in range(lo_x, min(hi_x, lo_x + 8)):
            i = (row + min(max(x, 0), w - 1)) * 3
            s[0] += px[i]
            s[1] += px[i + 1]
            s[2] += px[i + 2]
            n += 1
    return [math.floor(c / n + 0.5) for c in s]


def sharp_axis(s, k):
    """present_source.hlsli sharp_axis(): the two source pixels and the second one's weight."""
    sk = s * k - 0.5
    j = math.floor(sk)
    return j // k, (j + 1) // k, sk - j


def present_filter(image, scale, ow, oh, pixel):
    """The ow x oh output of a filter: black around video.c's rectangle, pixel(px, w, h, ox, oy, dw, dh) inside,
    the average where the rectangle is smaller than the picture (present_source.hlsli shrinking())."""
    w, h, px = image
    x0, y0, dw, dh = dest(ow, oh, w // scale, h // scale)
    out = bytearray(ow * oh * 3)
    shrink = dw < w or dh < h
    for y in range(max(y0, 0), min(y0 + dh, oh)):
        for x in range(max(x0, 0), min(x0 + dw, ow)):
            ox, oy = x - x0, y - y0
            c = box(px, w, h, ox, oy, dw, dh) if shrink else pixel(px, w, h, ox, oy, dw, dh)
            o = (y * ow + x) * 3
            out[o:o + 3] = bytes(c)
    return bytes(out)


def sharp(image, scale, ow, oh):
    """present_sharp.frag.hlsl: nearest by the largest integer multiple per axis, then bilinear."""
    w, h, _ = image
    x0, y0, dw, dh = dest(ow, oh, w // scale, h // scale)
    kx, ky = max(dw // w, 1), max(dh // h, 1)
    cols = [sharp_axis((ox + 0.5) * w / dw, kx) for ox in range(max(dw, 1))]
    rows = [sharp_axis((oy + 0.5) * h / dh, ky) for oy in range(max(dh, 1))]

    def pixel(px, w, h, ox, oy, dw, dh):
        ax, bx, fx = cols[ox]
        ay, by, fy = rows[oy]
        ax, bx = min(max(ax, 0), w - 1), min(max(bx, 0), w - 1)
        ay, by = min(max(ay, 0), h - 1), min(max(by, 0), h - 1)
        a, b, c, d = (ay * w + ax) * 3, (ay * w + bx) * 3, (by * w + ax) * 3, (by * w + bx) * 3
        out = []
        for i in range(3):
            top = px[a + i] + (px[b + i] - px[a + i]) * fx
            bottom = px[c + i] + (px[d + i] - px[c + i]) * fx
            out.append(math.floor(top + (bottom - top) * fy + 0.5))
        return out

    return present_filter(image, scale, ow, oh, pixel)


REFERENCES = {"sharp": sharp}


def compare(got, want):
    """(the largest difference, the share of channels that differ)."""
    worst, n = 0, 0
    for a, b in zip(got, want):
        if a != b:
            n += 1
            worst = max(worst, abs(a - b))
    return worst, n / max(len(want), 1)


def options(sdl, headless, env):
    print("render_filters: the options and the software renderer")
    rc, out = run([sdl, "--filter", "blur"], env)
    check(rc == 64 and "--filter: none or sharp" in out, f"--filter blur: exit 64 with the choices (exit {rc})")
    rc, out = run([sdl, "--filter", "sharp:strength=3"], env)
    check(rc == 64 and "--filter: sharp takes no parameters" in out, f"--filter sharp:strength=3: exit 64 (exit {rc})")
    if headless.exists():
        rc, out = run([headless, "--filter", "sharp", "--max-frames", "2"], env)
        check(rc == 0, f"the headless build takes --filter sharp (exit {rc})")
    rc, out = run([sdl, "--input-test", "--fps", "0", "--filter", "sharp"], env)
    check(rc == 0 and "input test passed" in out and "filter: sharp needs the GPU renderer" in out,
          f"--window --filter sharp on the software renderer: logged, the input self-test passes (exit {rc})")
    rc, out = run([sdl, "--input-test", "--fps", "0", "--renderer", "gpu", "--filter", "sharp"],
                  dict(env, VK_DRIVER_FILES=str(ROOT / "build/no-such-vulkan-driver.json")))
    check(rc == 0 and "; software" in out and "filter: sharp needs the GPU renderer" in out,
          f"--renderer gpu --filter sharp without a device: the fallback, logged (exit {rc})")


def device(sdl, env, out):
    """None when a device opened, else the reason."""
    rc, text = run([sdl, "--max-frames", "2", "--gpu-screenshot", f"2:{out / 'probe.ppm'}"], env)
    line = next((l for l in text.splitlines() if "renderer: gpu" in l), "")
    if rc != 0 or "for the screenshots" not in line:
        return line.split("port: ", 1)[-1] or f"exit {rc}"
    print(f"render_filters: the device: {line.split('(', 1)[1][:-1]}")
    return None


def pictures(sdl, env, out, name):
    worst_max, worst_share = TOLERANCE[name]
    print(f"render_filters: {name}: new_game's pictures against the reference (at most {worst_max} per channel in "
          f"{worst_share:.0%} of the channels)")
    for scale, frames, sizes in RUNS:
        args = []
        for f in frames:
            args += ["--gpu-screenshot", f"{f}:{out / f'{name}{scale}_{f}.ppm'}"]
            for w, h in sizes:
                args += ["--gpu-screenshot", f"{f}@{w}x{h}:{out / f'{name}{scale}_{f}_{w}x{h}.ppm'}"]
        rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / "new_game.json", "--max-frames",
                        str(max(frames) + 1), "--internal-scale", str(scale), "--filter", name, *args], env)
        check(rc == 0 and f"renderer: gpu: filter {name}" in text, f"internal scale {scale}: the run (exit {rc})")
        if rc != 0:
            print("\n".join(text.splitlines()[-10:]))
            continue
        for f in frames:
            image = ppm(out / f"{name}{scale}_{f}.ppm")
            bad = []
            for w, h in sizes:
                got = ppm(out / f"{name}{scale}_{f}_{w}x{h}.ppm")
                if got[:2] != (w, h):
                    bad.append(f"{w}x{h}: the size is {got[0]}x{got[1]}")
                    continue
                worst, share = compare(got[2], REFERENCES[name](image, scale, w, h))
                if worst > worst_max or share > worst_share:
                    bad.append(f"{w}x{h}: up to {worst} in {share:.2%}")
            check(not bad, f"internal scale {scale}, vsync {f} ({image[0]}x{image[1]}): "
                  f"{', '.join(f'{w}x{h}' for w, h in sizes)}" + (f" ({'; '.join(bad)})" if bad else ""))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(ROOT / "build/render-filters-test"), help="where the pictures go")
    ap.add_argument("-j", "--jobs", type=int, default=0, help="build jobs (default: ninja's)")
    ap.add_argument("--lavapipe", action="store_true",
                    help="Mesa's software Vulkan driver instead of the default device")
    ap.add_argument("--filter", action="append", choices=sorted(REFERENCES), help="only these filters' pictures")
    args = ap.parse_args()
    out = Path(args.out)
    try:
        env = tool_env()
        for tool in ("sdl3", "dxc"):
            if not (ROOT / "tools" / tool).exists():
                print(f"render_filters: no tools/{tool} (scripts/setup.sh {tool}): skipped")
                return 0
        sdl = build("build/port-sdl", ["-DPSXSTACK_SDL=ON"], args.jobs, env)
    except Missing as e:
        print(f"render_filters: {e}")
        return 2
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_AUDIO_DRIVER="dummy")
    if args.lavapipe:
        if not LAVAPIPE.exists():
            print(f"render_filters: --lavapipe: no {LAVAPIPE}")
            return 2
        env["VK_DRIVER_FILES"] = str(LAVAPIPE)
    options(sdl, ROOT / "build/port/dw2003", env)
    if not DISC.exists():
        print("render_filters: no disc (iso/dw2003.cue): the pictures are skipped")
    else:
        why = device(sdl, env, out)
        if why is not None:
            print(f"render_filters: no GPU device ({why}): the pictures are skipped")
        else:
            for name in args.filter or sorted(REFERENCES):
                pictures(sdl, env, out, name)
    print(f"render_filters test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
