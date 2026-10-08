#!/usr/bin/env python3
"""The hardware renderer's present filters (psxstack/runtime/render_gpu_present.c; issue #69): `--filter`,
video.filter. The options and the software renderer's refusal run everywhere (CI too); the pictures need a GPU device
and the disc and run locally.

Usage: tests/port/render_filters.py [--out DIR] [-j N] [--build DIR] [--lavapipe] [--filter NAME ...]

Builds build/port-sdl if needed (-DPSXSTACK_SDL=ON: tools/sdl3 and tools/dxc), then:
  - the options: a bad --filter exits 64 with the choices; the headless build accepts a filter; the software
    renderer's window (and the GPU renderer's fallback without a device) logs that the picture stays unfiltered and
    passes the input self-test;
  - with a GPU device and the disc (otherwise skipped, with the reason): new_game replayed with each filter, its
    --gpu-screenshot F@WxH pictures against a reference computed here from the unfiltered picture (--gpu-screenshot F,
    which tests/port/render_gpu.py checks against the software image), at internal scales 1, 2 and 3 (RUNS: integer
    and non-integer scales, letterboxing, a window smaller than the picture). A filter's reference mirrors its shader (psxstack/shaders/present_<filter>.frag.hlsl) in float64;
    the GPU computes in float32, so each filter has a tolerance per channel (TOLERANCE: the largest difference, and
    the share of channels that may differ at all). `smooth` filters the 1x software image at every internal scale,
    so its reference starts from --screenshot's picture.
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
# filter: (the largest difference per channel, the share of channels that may differ). The references compute in
# float64, the GPU in float32 (and its own cos, exp and pow). Measured on NVIDIA RTX 4070 Ti SUPER and lavapipe:
# - sharp: exact at integer scales; off by 1 where a weight is not exact in float32 (thirds at 3/2): NVIDIA 3.4 % of
#   the channels at 3/2, 0.16 % at 4/3; lavapipe 6.5 % at 3/2, none at 4/3;
# - scanlines: off by 1 in up to 5.3 % (NVIDIA) and 7.4 % (lavapipe) of the channels, at full strength;
# - crt: off by 1 in up to 0.06 % (NVIDIA) and 0.04 % (lavapipe), the curvature's edge included (it fades over a pixel:
#   a hard edge was a tie, 215 apart);
# - smooth: its decisions are integers; off by 1 in up to 0.75 % (NVIDIA) of the channels; lavapipe exact.
TOLERANCE = {"sharp": (1, 0.10), "scanlines": (1, 0.10), "crt": (1, 0.01), "smooth": (1, 0.02)}
# The filters as the runs set them (`--filter`): each with video.crt's defaults, and the scanlines filters at full
# strength (the CRT with its grille strong and the curvature on).
SPECS = ("sharp", "scanlines", "scanlines:scanlines=100", "crt", "crt:scanlines=100,mask=80,curvature=40", "smooth")
FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


class Frame:
    """The unfiltered picture (w x h at the internal scale) placed in an ow x oh output as video.c places it, with the
    present filters' sampling (psxstack/shaders/present_source.hlsli) in float64."""

    def __init__(self, image, scale, ow, oh):
        self.w, self.h, self.px = image
        self.lines = self.h // scale   # the display's lines
        self.x0, self.y0, self.dw, self.dh = dest(ow, oh, self.w // scale, self.lines)
        self.shrink = self.dw < self.w or self.dh < self.h

    def fetch(self, x, y):
        i = (min(max(y, 0), self.h - 1) * self.w + min(max(x, 0), self.w - 1)) * 3
        return self.px[i], self.px[i + 1], self.px[i + 2]

    def box(self, ox, oy):
        """box(): the average of the source pixels whose centres fall in output pixel (ox, oy) (at most 8 x 8)."""
        w, h, dw, dh = self.w, self.h, self.dw, self.dh
        lo_x = (2 * ox * w + dw - 1) // (2 * dw)
        hi_x = max(((2 * ox + 2) * w + dw - 1) // (2 * dw), lo_x + 1)
        lo_y = (2 * oy * h + dh - 1) // (2 * dh)
        hi_y = max(((2 * oy + 2) * h + dh - 1) // (2 * dh), lo_y + 1)
        s, n = [0, 0, 0], 0
        for y in range(lo_y, min(hi_y, lo_y + 8)):
            for x in range(lo_x, min(hi_x, lo_x + 8)):
                c = self.fetch(x, y)
                s = [s[i] + c[i] for i in range(3)]
                n += 1
        return [math.floor(c / n + 0.5) for c in s]

    @staticmethod
    def sharp_axis(s, k):
        """sharp_axis(): the two source pixels and the second one's weight."""
        sk = s * k - 0.5
        j = math.floor(sk)
        return j // k, (j + 1) // k, sk - j

    def sharp_at(self, qx, qy):
        """sharp_at(): sharp bilinear at output position (qx, qy) of the rectangle, unrounded."""
        ax, bx, fx = self.sharp_axis(qx * self.w / self.dw, max(self.dw // self.w, 1))
        ay, by, fy = self.sharp_axis(qy * self.h / self.dh, max(self.dh // self.h, 1))
        a, b, c, d = self.fetch(ax, ay), self.fetch(bx, ay), self.fetch(ax, by), self.fetch(bx, by)
        out = []
        for i in range(3):
            top = a[i] + (b[i] - a[i]) * fx
            bottom = c[i] + (d[i] - c[i]) * fx
            out.append(top + (bottom - top) * fy)
        return out

    def row_at(self, y, qx):
        """row_at(): source row y along x at output position qx (averaged where the rectangle is narrower)."""
        if self.dw < self.w:
            p = math.floor(qx)
            lo = (2 * p * self.w + self.dw - 1) // (2 * self.dw)
            hi = max(((2 * p + 2) * self.w + self.dw - 1) // (2 * self.dw), lo + 1)
            cols = [self.fetch(x, y) for x in range(lo, min(hi, lo + 8))]
            return [sum(c[i] for c in cols) / len(cols) for i in range(3)]
        a, b, f = self.sharp_axis(qx * self.w / self.dw, max(self.dw // self.w, 1))
        ca, cb = self.fetch(a, y), self.fetch(b, y)
        return [ca[i] + (cb[i] - ca[i]) * f for i in range(3)]

    def beam_lines(self):
        return self.lines // 2 if self.lines > 288 else self.lines

    def beam_strength(self, s):
        return s * min(max(self.dh / self.beam_lines() - 1.0, 0.0), 1.0)


def sharp(fr, params, ox, oy):
    """present_sharp.frag.hlsl."""
    if fr.shrink:
        return fr.box(ox, oy)
    return [math.floor(c + 0.5) for c in fr.sharp_at(ox + 0.5, oy + 0.5)]


def scanlines(fr, params, ox, oy):
    """present_scanlines.frag.hlsl: sharp, times a raised cosine per line."""
    c = fr.box(ox, oy) if fr.shrink else fr.sharp_at(ox + 0.5, oy + 0.5)
    s = fr.beam_strength(params["scanlines"] / 100)
    ly = (oy + 0.5) * fr.beam_lines() / fr.dh
    t = ly - math.floor(ly)
    w = (1.0 - s * (0.5 + 0.5 * math.cos(2 * math.pi * t))) * (1.0 + 0.5 * s)
    return [min(math.floor(v * w + 0.5), 255) for v in c]


def crt(fr, params, ox, oy):
    """present_crt.frag.hlsl: Gaussian beams in linear light, the aperture grille, the curvature."""
    qx, qy = ox + 0.5, oy + 0.5
    k = params["curvature"] / 100
    if k > 0:
        cx, cy = qx / fr.dw * 2 - 1, qy / fr.dh * 2 - 1
        cx, cy = cx * (1 + k * 0.12 * cy * cy), cy * (1 + k * 0.12 * cx * cx)
        qx, qy = (cx * 0.5 + 0.5) * fr.dw, (cy * 0.5 + 0.5) * fr.dh
    clamp01 = lambda v: min(max(v, 0.0), 1.0)  # noqa: E731
    edge = clamp01(min(qx, fr.dw - qx)) * clamp01(min(qy, fr.dh - qy)) if k > 0 else 1.0
    if edge <= 0:
        return [0, 0, 0]
    lines = fr.beam_lines()
    per = max(fr.h // lines, 1)
    s = fr.beam_strength(params["scanlines"] / 100)
    ly = qy * lines / fr.dh - 0.5
    l0 = math.floor(ly)
    d = ly - l0
    col = [0.0, 0.0, 0.0]
    for i in range(-1, 3):
        line = min(max(l0 + i, 0), lines - 1)
        a, b = fr.row_at(line * per + per // 4, qx), fr.row_at(line * per + (3 * per) // 4, qx)
        dd = d - i
        tent = max(0.0, 1.0 - abs(dd))
        for ch in range(3):
            c = ((a[ch] + b[ch]) * 0.5 / 255) ** 2.2
            sigma = 0.22 + (0.45 - 0.22) * math.sqrt(c)
            g = math.exp(-(dd * dd) / (2 * sigma * sigma)) / (sigma * 2.50662827463)
            col[ch] += (tent + (g - tent) * s) * c
    m = params["mask"] / 100
    x = (fr.x0 + ox) % 3
    out = []
    for ch in range(3):
        v = col[ch] * (1.0 if ch == x else 1.0 - 0.75 * m) / (1.0 - 0.5 * m) * edge
        out.append(math.floor(min(max(v, 0.0), 1.0) ** (1 / 2.2) * 255 + 0.5))
    return out


def yuv(c):
    r, g, b = c
    return 299 * r + 587 * g + 114 * b, -169 * r - 331 * g + 500 * b, 500 * r - 419 * g - 81 * b


def dist(a, b):
    return 48 * abs(a[0] - b[0]) + 7 * abs(a[1] - b[1]) + 6 * abs(a[2] - b[2])


def smooth_corners(fr, px, py, cache):
    """present_smooth.frag.hlsl cell()'s decisions for source pixel (px, py), integers: [(sx, sy, the colour beyond,
    shallow, steep)] for each corner an edge crosses."""
    key = (px, py)
    if key in cache:
        return cache[key]
    n = {(x, y): fr.fetch(px + x, py + y) for x in range(-2, 3) for y in range(-2, 3)}
    yv = {k: yuv(v) for k, v in n.items()}
    eq = lambda a, b: dist(a, b) < 15000  # noqa: E731
    corners = []
    for corner in range(4):
        sx, sy = (-1 if corner & 1 else 1), (-1 if corner & 2 else 1)
        N = lambda x, y: yv[(sx * x, sy * y)]  # noqa: E731
        E, F, H, I, B, D = N(0, 0), N(1, 0), N(0, 1), N(1, 1), N(0, -1), N(-1, 0)
        C, G, F4, I4, H5, I5 = N(1, -1), N(-1, 1), N(2, 0), N(2, 1), N(0, 2), N(1, 2)
        slash = dist(E, C) + dist(E, G) + dist(I, F4) + dist(I, H5) + 4 * dist(H, F)
        backslash = dist(H, D) + dist(H, I5) + dist(F, I4) + dist(F, B) + 4 * dist(E, I)
        restriction = (not eq(E, F) and not eq(E, H) and
                       ((not eq(F, B) and not eq(H, D)) or (eq(E, I) and not eq(F, I4) and not eq(H, I5))
                        or eq(E, G) or eq(E, C)))
        if slash < backslash and restriction:
            beyond = n[(sx, 0)] if dist(E, F) <= dist(E, H) else n[(0, sy)]
            shallow = 2 * dist(F, G) <= dist(H, C) and not eq(E, G) and not eq(D, G)
            steep = dist(F, G) >= 2 * dist(H, C) and not eq(E, C) and not eq(B, C)
            corners.append((sx, sy, beyond, shallow, steep))
    cache[key] = corners
    return corners


def smooth(fr, params, ox, oy):
    """present_smooth.frag.hlsl: xBR level 2 on the 1x image at cells k times smaller, bilinear between cells."""
    if fr.shrink:
        return fr.box(ox, oy)
    cache = params.setdefault("_cache", {})
    kx, ky = max(fr.dw // fr.w, 1), max(fr.dh // fr.h, 1)
    w = 1.0 / max(kx, ky)
    ramp = lambda x: min(max(x / w + 0.5, 0.0), 1.0)  # noqa: E731

    def cell(qx, qy):
        px, py = qx // kx, qy // ky
        u, v = (qx - px * kx + 0.5) / kx, (qy - py * ky + 0.5) / ky
        res = [float(c) for c in fr.fetch(px, py)]
        for sx, sy, beyond, shallow, steep in smooth_corners(fr, px, py, cache):
            uu, vv = (u if sx > 0 else 1.0 - u), (v if sy > 0 else 1.0 - v)
            a = ramp((uu + vv - 1.5) * 0.70710678)
            if shallow:
                a = max(a, ramp((0.5 * uu + vv - 1.0) * 0.89442719))
            if steep:
                a = max(a, ramp((uu + 0.5 * vv - 1.0) * 0.89442719))
            res = [res[i] + (beyond[i] - res[i]) * a for i in range(3)]
        return res

    sx = (ox + 0.5) * fr.w * kx / fr.dw - 0.5
    sy = (oy + 0.5) * fr.h * ky / fr.dh - 0.5
    ax, ay = math.floor(sx), math.floor(sy)
    fx, fy = sx - ax, sy - ay
    c00, c10, c01, c11 = cell(ax, ay), cell(ax + 1, ay), cell(ax, ay + 1), cell(ax + 1, ay + 1)
    out = []
    for i in range(3):
        top = c00[i] + (c10[i] - c00[i]) * fx
        bottom = c01[i] + (c11[i] - c01[i]) * fx
        out.append(math.floor(top + (bottom - top) * fy + 0.5))
    return out


# name: (the reference, the parameters it reads with video.crt's defaults, every how many rows it is compared)
REFERENCES = {"sharp": (sharp, {}, 1), "scanlines": (scanlines, {"scanlines": 50}, 1),
              "crt": (crt, {"scanlines": 50, "mask": 30, "curvature": 0}, 5), "smooth": (smooth, {}, 3)}


def reference(spec, image, scale, ow, oh):
    """{output row: its RGB bytes} of filter `spec` ("NAME[:KEY=V,...]") for the rows its entry compares."""
    name, _, rest = spec.partition(":")
    pixel, params, step = REFERENCES[name]
    params = dict(params, **{k: int(v) for k, v in (kv.split("=") for kv in rest.split(",") if kv)})
    if name == "smooth":   # the 1x software image whatever the internal scale (image is then --screenshot's)
        scale = 1
    fr = Frame(image, scale, ow, oh)
    rows = {}
    for y in range(0, oh, step):
        row = bytearray(ow * 3)
        oy = y - fr.y0
        if 0 <= oy < fr.dh:
            for x in range(max(fr.x0, 0), min(fr.x0 + fr.dw, ow)):
                row[x * 3:x * 3 + 3] = bytes(pixel(fr, params, x - fr.x0, oy))
        rows[y] = bytes(row)
    return rows


def compare(got, ow, want):
    """(the largest difference, the share of channels that differ) over want's rows."""
    worst, n, total = 0, 0, 0
    for y, row in want.items():
        for a, b in zip(got[y * ow * 3:(y + 1) * ow * 3], row):
            if a != b:
                n += 1
                worst = max(worst, abs(a - b))
        total += len(row)
    return worst, n / max(total, 1)


def options(sdl, headless, env):
    print("render_filters: the options and the software renderer")
    rc, out = run([sdl, "--filter", "blur"], env)
    check(rc == 64 and "--filter: none, sharp, scanlines, crt or smooth, not \"blur\"" in out,
          f"--filter blur: exit 64 with the choices (exit {rc})")
    for spec, message in (("sharp:scanlines=3", "sharp takes no parameters"),
                          ("scanlines:mask=3", "scanlines takes scanlines, not mask"),
                          ("crt:scanlines=101", "crt: scanlines from 0 to 100"),
                          ("crt:mask", "crt: KEY=V,... (scanlines, mask, curvature)"),
                          ("crt:glow=3", "crt takes scanlines, mask, curvature, not glow")):
        rc, out = run([sdl, "--filter", spec], env)
        check(rc == 64 and f"--filter: {message}" in out, f"--filter {spec}: exit 64, {message} (exit {rc})")
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


def pictures(sdl, env, out, spec):
    name = spec.split(":")[0]
    tag = spec.replace(":", "_").replace(",", "_").replace("=", "")
    worst_max, worst_share = TOLERANCE[name]
    print(f"render_filters: {spec}: new_game's pictures against the reference (at most {worst_max} per channel in "
          f"{worst_share:.0%} of the channels)")
    for scale, frames, sizes in RUNS:
        args = []
        for f in frames:
            args += ["--gpu-screenshot", f"{f}:{out / f'{tag}{scale}_{f}.ppm'}",
                     "--screenshot", f"{f}:{out / f'{tag}{scale}_{f}_sw.ppm'}"]
            for w, h in sizes:
                args += ["--gpu-screenshot", f"{f}@{w}x{h}:{out / f'{tag}{scale}_{f}_{w}x{h}.ppm'}"]
        rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / "new_game.json", "--max-frames",
                        str(max(frames) + 1), "--internal-scale", str(scale), "--filter", spec, *args], env)
        check(rc == 0 and f"renderer: gpu: filter {name}" in text, f"internal scale {scale}: the run (exit {rc})")
        if rc != 0:
            print("\n".join(text.splitlines()[-10:]))
            continue
        for f in frames:
            image = ppm(out / f"{tag}{scale}_{f}{'_sw' if name == 'smooth' else ''}.ppm")
            bad = []
            for w, h in sizes:
                got = ppm(out / f"{tag}{scale}_{f}_{w}x{h}.ppm")
                if got[:2] != (w, h):
                    bad.append(f"{w}x{h}: the size is {got[0]}x{got[1]}")
                    continue
                worst, share = compare(got[2], w, reference(spec, image, scale, w, h))
                if worst > worst_max or share > worst_share:
                    bad.append(f"{w}x{h}: up to {worst} in {share:.2%}")
            check(not bad, f"internal scale {scale}, vsync {f} ({image[0]}x{image[1]}): "
                  f"{', '.join(f'{w}x{h}' for w, h in sizes)}" + (f" ({'; '.join(bad)})" if bad else ""))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(ROOT / "build/render-filters-test"), help="where the pictures go")
    ap.add_argument("-j", "--jobs", type=int, default=0, help="build jobs (default: ninja's)")
    ap.add_argument("--build", default="build/port-sdl", help="the SDL build directory (default: build/port-sdl)")
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
        sdl = build(args.build, ["-DPSXSTACK_SDL=ON"], args.jobs, env)
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
            for spec in SPECS:
                if not args.filter or spec.split(":")[0] in args.filter:
                    pictures(sdl, env, out, spec)
    print(f"render_filters test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
