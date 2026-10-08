#!/usr/bin/env python3
"""The hardware renderer (psxstack/runtime/render_gpu.c; issue #31): the device, the present, the fallback, and the rasteriser
at internal scale 1. CI only compiles the renderer and runs the fallback part; the rest needs a GPU device and runs
locally.

Usage: tests/port/render_gpu.py [--out DIR] [-j N] [--lavapipe]
       tests/port/render_gpu.py --exe build/port-win/dw2003.exe --wine [--lavapipe]   # the Windows build, under Wine

Builds build/port-sdl if needed (-DPSXSTACK_SDL=ON: tools/sdl3 and tools/dxc), then:
  - the fallback (no device, no disc needed): `--window --renderer gpu --input-test` with a Vulkan loader that finds
    no driver (VK_DRIVER_FILES naming none) must log why and pass the input self-test on SDL_Renderer; the options'
    errors (a bad --renderer, a bad --gpu-screenshot, --gpu-screenshot in the headless build);
  - the window through the renderer, headless, where the device can present to SDL's offscreen video driver (Mesa's
    lavapipe can: VK_EXT_headless_surface; NVIDIA's driver cannot, and the window then falls back: skipped): the
    input self-test with `--renderer gpu` must pass with the window presenting through SDL_GPU;
  - with a GPU device and the disc (otherwise skipped, with the reason): new_game replayed headless with --screenshot
    and --gpu-screenshot at fixed vsyncs: the hardware renderer's picture (drawn by its rasteriser for a 15-bit
    display) must be the software image byte for byte, and its present into outputs of several sizes (integer scales,
    letterboxing, a non-integer 4:3 width, a window smaller than the image) must be the reference below pixel for
    pixel; then new_game and first_battle_save with the rasteriser's whole VRAM target compared with the software VRAM
    every 10 vsyncs (DW3_PORT_GPU_VRAM_CHECK): no difference anywhere (~40 s); then internal scales 2 and 4: the
    pictures of field and battle frames are the display's size times the scale and, averaged back over each N x N
    block, close to the software image (SCALED_BUDGET: the 3D's edges and textures sampled between texels differ, the
    2D does not); the debug channel's screenshot with "renderer": "gpu" at the internal scale;
  - with the disc: SDL_Renderer's own present (a 960x720 window on the offscreen driver, read back through
    DW3_PORT_PRESENT_READBACK) must be the same reference: the two present paths agree.
The reference is video.c's placement (video_dest: 4:3, as tall as an integer multiple of the image's lines allows,
centred) with target pixel p reading image pixel floor((p - x0 + 0.5) * w / dw) (render_gpu.c's present shader).
--lavapipe runs on Mesa's software Vulkan driver (VK_DRIVER_FILES) instead of the default device.
--exe BINARY tests that binary instead of building build/port-sdl: the device, the pictures, the VRAM checks and the
internal scales (the fallback, the window, the debug channel and SDL_Renderer's present are the Linux build's parts).
--wine runs it through `wine` (a Windows build: scripts/build_windows.sh), headless: SDL_GPU on Direct3D 12 there
(Wine's vkd3d, or vkd3d-proton in a prefix that has it), Vulkan with SDL_GPU_DRIVER=vulkan. The prefix is
build/wine-prefix unless WINEPREFIX names one; VKD3D_SHADER_CACHE_PATH=0 keeps vkd3d-proton from writing its cache
into the working directory. Without a device under Wine (CI's Wine 9.0 has none: issue #67) the pictures are skipped.
Exit codes: 0 pass (parts may be skipped), 1 fail, 2 something missing for the build.
"""
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/port"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)

FRAMES = (150, 400, 900, 1300, 1800)
SCALED_FRAMES = (10000, 15000, 19900, 20300)   # first_battle_save: the field, a field event, the first battle
# Averaged back to 1x, a frame at a higher internal scale may differ from the software image by at most this mean
# per channel, and by more than 32 in at most this share of the channels (measured on NVIDIA: field 0.00 / 0.00 %,
# a field event's translucent bands in 8-bit colour 1.1 / 0.00 %, the battle's 3D up to 9.5 / 9.1 %).
SCALED_BUDGET = (12.0, 0.12)   # new_game: CNTY_SEL's 320x576 screen, the title's menus, the field
SIZES = ((960, 720), (1366, 768), (1920, 1080), (1280, 1024), (1000, 700), (300, 200))
LAVAPIPE = Path("/usr/share/vulkan/icd.d/lvp_icd.x86_64.json")
FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def ppm(path):
    """(w, h, RGB bytes) of a binary PPM as the port writes it."""
    data = path.read_bytes()
    magic, size, depth, pixels = data.split(b"\n", 3)
    assert magic == b"P6" and depth == b"255", path
    w, h = map(int, size.split())
    return w, h, pixels


def dest(ow, oh, w, h):
    """video.c video_dest, in whole pixels as video_dest_rect rounds it: (x, y, w, h)."""
    f = oh // h
    while f > 0 and (h * f * 4 + 2) // 3 > ow:
        f -= 1
    if f > 0:
        dh, dw = h * f, (h * f * 4 + 2) // 3
    else:
        dh = min(float(oh), ow * 3.0 / 4.0)
        dw = dh * 4.0 / 3.0
    return int((ow - dw) / 2), int((oh - dh) / 2), int(dw + 0.5), int(dh + 0.5)


def present(image, ow, oh):
    """The reference present of image (w, h, RGB) into an ow x oh output, on black."""
    w, h, px = image
    x0, y0, dw, dh = dest(ow, oh, w, h)
    out = bytearray(ow * oh * 3)
    xs = [((2 * (x - x0) + 1) * w) // (2 * dw) for x in range(ow)]
    for y in range(max(y0, 0), min(y0 + dh, oh)):
        row = ((2 * (y - y0) + 1) * h) // (2 * dh) * w
        for x in range(max(x0, 0), min(x0 + dw, ow)):
            i, o = (row + xs[x]) * 3, (y * ow + x) * 3
            out[o:o + 3] = px[i:i + 3]
    return bytes(out)


def differing(a, b):
    return sum(1 for i in range(0, len(a), 3) if a[i:i + 3] != b[i:i + 3])


RUNNER = []   # the command prefix of a run (--wine: ["wine"])
SLOWER = 1    # the timeouts' factor (--wine: 3)


def run(cmd, env, timeout=300):
    proc = subprocess.run([*RUNNER, *[str(c) for c in cmd]], cwd=ROOT, env=env, capture_output=True, text=True,
                          timeout=timeout * SLOWER)
    return proc.returncode, proc.stdout + proc.stderr


def fallback(sdl, headless, env):
    print("render_gpu: the fallback (a Vulkan loader without drivers) and the options")
    rc, out = run([sdl, "--input-test", "--fps", "0", "--renderer", "gpu"],
                  dict(env, VK_DRIVER_FILES=str(ROOT / "build/no-such-vulkan-driver.json")))
    check(rc == 0 and "renderer: gpu unavailable (" in out and "; software" in out and "renderer gpu (" not in out,
          f"--window --renderer gpu without a device: the fallback logged, the input self-test passes (exit {rc})")
    rc, out = run([sdl, "--renderer", "vulkan"], env)
    check(rc == 64 and "--renderer: software or gpu" in out, f"--renderer vulkan: exit 64 (exit {rc})")
    for spec in ("0:x.ppm", "5@640:x.ppm", "5@0x480:x.ppm", "5", "5:"):
        rc, out = run([sdl, "--gpu-screenshot", spec], env)
        check(rc == 64 and "--gpu-screenshot: FRAME[@WxH]:PATH" in out, f"--gpu-screenshot {spec}: exit 64 (exit {rc})")
    if headless.exists():
        rc, out = run([headless, "--gpu-screenshot", "5:x.ppm"], env)
        check(rc == 64 and "this build has no GPU renderer" in out,
              f"--gpu-screenshot in the headless build: exit 64 (exit {rc})")


def window(sdl, env):
    rc, out = run([sdl, "--input-test", "--fps", "0", "--renderer", "gpu"], env)
    line = next((l for l in out.splitlines() if "window: SDL" in l), "")
    if "renderer gpu (" not in line:
        why = next((l.split("unavailable ", 1)[1] for l in out.splitlines() if "gpu unavailable" in l), "?")
        print(f"render_gpu: the window through SDL_GPU, headless: skipped (the device cannot present to the offscreen "
              f"driver's window: {why})")
        return
    print("render_gpu: the window through SDL_GPU, headless (the offscreen driver's window)")
    check(rc == 0 and "input test passed" in out, f"--window --renderer gpu: {line.split('renderer ', 1)[1]}: the "
          f"input self-test passes (exit {rc})")


def device(sdl, env, out):
    """None when a device opened (its description printed), else the reason."""
    rc, text = run([sdl, "--max-frames", "2", "--gpu-screenshot", f"2:{out / 'probe.ppm'}"], env)
    line = next((l for l in text.splitlines() if "renderer: gpu" in l), "")
    if rc != 0 or "for the screenshots" not in line:
        return line.split("port: ", 1)[-1] or f"exit {rc}"
    print(f"render_gpu: the device: {line.split('(', 1)[1][:-1]}")
    return None


def screenshots(sdl, env, out):
    print("render_gpu: new_game, the hardware renderer's pictures against the software image and the reference")
    args = []
    for f in FRAMES:
        args += ["--screenshot", f"{f}:{out / f'sw{f}.ppm'}", "--gpu-screenshot", f"{f}:{out / f'hw{f}.ppm'}"]
        for w, h in SIZES:
            args += ["--gpu-screenshot", f"{f}@{w}x{h}:{out / f'hw{f}_{w}x{h}.ppm'}"]
    rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / "new_game.json", *args], env)
    check(rc == 0, f"the run: exit {rc}")
    if rc != 0:
        print("\n".join(text.splitlines()[-10:]))
        return
    for f in FRAMES:
        sw, hw = out / f"sw{f}.ppm", out / f"hw{f}.ppm"
        image = ppm(sw)
        check(sw.read_bytes() == hw.read_bytes(), f"vsync {f}: the image ({image[0]}x{image[1]}) byte for byte")
        bad = []
        for w, h in SIZES:
            got = ppm(out / f"hw{f}_{w}x{h}.ppm")
            n = differing(got[2], present(image, w, h)) if got[:2] == (w, h) else w * h
            if n:
                bad.append(f"{w}x{h}: {n} pixels")
        check(not bad, f"vsync {f}: the present into {', '.join(f'{w}x{h}' for w, h in SIZES)}"
              + (f" ({'; '.join(bad)} differ)" if bad else ""))


def vram_checks(sdl, env, every=10):
    print(f"render_gpu: the rasteriser's whole VRAM against the software VRAM every {every} vsyncs")
    for name in ("new_game", "first_battle_save"):
        rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / f"{name}.json", "--gpu-screenshot", "1:/dev/null"],
                       dict(env, DW3_PORT_GPU_VRAM_CHECK=str(every)), timeout=900)
        total = next((l.split("gpu vram check: ", 1)[1] for l in text.splitlines() if " checks, " in l), "no checks")
        bad = [l.split("port: ", 1)[1] for l in text.splitlines() if "pixels differ" in l]
        check(rc == 0 and total.endswith(" 0 with differences") and "no rasteriser" not in text,
              f"{name}: {total}" + (f"; the first: {bad[0]}" if bad else ""))


def reduced(hi, n, w, h):
    """The n x n blocks of a (w n) x (h n) RGB picture averaged: (mean |difference| per channel, share > 32) against
    the w x h picture `lo`, as a function."""
    big = list(hi)
    W = w * n

    def against(lo):
        total, far = 0.0, 0
        for y in range(h):
            rows = [((y * n + j) * W) * 3 for j in range(n)]
            for x in range(w):
                for c in range(3):
                    acc = 0
                    for r0 in rows:
                        base = r0 + x * n * 3 + c
                        acc += sum(big[base:base + n * 3:3])
                    d = abs(acc / (n * n) - lo[(y * w + x) * 3 + c])
                    total += d
                    far += d > 32
        return total / (w * h * 3), far / (w * h * 3)
    return against


def scaled(sdl, env, out):
    print(f"render_gpu: internal scales 2 and 4, {len(SCALED_FRAMES)} frames of first_battle_save against the "
          f"software image (block averages)")
    for n in (2, 4):
        args = []
        for f in SCALED_FRAMES:
            args += ["--screenshot", f"{f}:{out / f'sc_sw{f}.ppm'}", "--gpu-screenshot", f"{f}:{out / f'sc{n}_{f}.ppm'}"]
        # No watchdog: software Vulkan (--lavapipe) falls far behind at these scales, and a screenshot's readback
        # then waits on the backlog for longer than the default 10 s without a vsync.
        rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / "first_battle_save.json", "--max-frames",
                        str(SCALED_FRAMES[-1] + 1), "--internal-scale", str(n), "--watchdog", "0", *args], env,
                       timeout=1800)
        if rc != 0:
            check(False, f"scale {n}: the run (exit {rc})")
            continue
        for f in SCALED_FRAMES:
            w, h, lo = ppm(out / f"sc_sw{f}.ppm")
            W, H, hi = ppm(out / f"sc{n}_{f}.ppm")
            if (W, H) != (w * n, h * n):
                check(False, f"scale {n}, vsync {f}: {W}x{H}, not {w * n}x{h * n}")
                continue
            mean, far = reduced(hi, n, w, h)(lo)
            check(mean <= SCALED_BUDGET[0] and far <= SCALED_BUDGET[1],
                  f"scale {n}, vsync {f}: {W}x{H}, averaged back: mean |diff| {mean:.2f}, {100 * far:.2f} % of the "
                  f"channels off by more than 32")


def debug_channel(sdl, env, out):
    print("render_gpu: the debug channel's screenshot, renderer gpu (a headless --renderer gpu --internal-scale 2)")
    sys.path.insert(0, str(ROOT / "tools"))
    from mcp_game import Game, GameError  # noqa: E402  (psxstack's MCP client of the channel, configured for this game)
    old = dict(os.environ)
    os.environ.update(env)
    try:
        g = Game.spawn([str(sdl), "--disc", str(DISC), "--cd-speed", "instant", "--renderer", "gpu",
                        "--internal-scale", "2"], cwd=str(ROOT))
    finally:
        os.environ.clear()
        os.environ.update(old)
    try:
        g.step(400)
        sw = g.screenshot(str(out / "debug_sw.ppm"))
        hw = g.screenshot(str(out / "debug_hw.ppm"), "gpu")
        check((sw["w"], sw["h"]) == (320, 240) and (hw["w"], hw["h"]) == (640, 480) and
              ppm(out / "debug_hw.ppm")[:2] == (640, 480), f"software {sw['w']}x{sw['h']}, gpu {hw['w']}x{hw['h']}")
        try:
            g.screenshot(str(out / "debug_bad.ppm"), "vulkan")
            check(False, "renderer \"vulkan\": an error")
        except GameError as e:
            check("software or gpu" in str(e), f"renderer \"vulkan\": {e}")
    finally:
        g.quit()


def sdl_renderer(sdl, env, out):
    print("render_gpu: SDL_Renderer's present (a 960x720 window, offscreen) against the same reference")
    for f in (FRAMES[0], FRAMES[-1]):
        shot, back = out / f"sdl_sw{f}.ppm", out / f"sdl{f}.ppm"
        rc, text = run([sdl, "--disc", DISC, "--script", SCRIPTS / "new_game.json", "--window", "--scale", "3",
                        "--fps", "0", "--mute", "--screenshot", f"{f}:{shot}"],
                       dict(env, DW3_PORT_PRESENT_READBACK=f"{f}:{back}"))
        renderer = next((l.split("renderer ", 1)[1].split(",")[0] for l in text.splitlines() if "window: SDL" in l),
                        "?")
        if rc != 0 or not back.exists():
            check(False, f"vsync {f}: the run (exit {rc})")
            continue
        image, got = ppm(shot), ppm(back)
        n = differing(got[2], present(image, 960, 720))
        check(got[:2] == (960, 720) and n == 0,
              f"vsync {f}: SDL_Renderer ({renderer}) is the reference ({image[0]}x{image[1]} into 960x720"
              + (f"; {n} pixels differ)" if n else ")"))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(ROOT / "build/render-gpu-test"), help="where the pictures go")
    ap.add_argument("-j", "--jobs", type=int, default=0, help="build jobs (default: ninja's)")
    ap.add_argument("--lavapipe", action="store_true",
                    help="Mesa's software Vulkan driver instead of the default device")
    ap.add_argument("--exe", help="test this binary instead of building build/port-sdl (its GPU parts only)")
    ap.add_argument("--wine", action="store_true", help="run --exe through wine (a Windows build), headless")
    args = ap.parse_args()
    out = Path(args.out)
    if args.exe:
        return main_exe(args, out)
    try:
        env = tool_env()
        for tool in ("sdl3", "dxc"):
            if not (ROOT / "tools" / tool).exists():
                print(f"render_gpu: no tools/{tool} (scripts/setup.sh {tool}): skipped")
                return 0
        sdl = build("build/port-sdl", ["-DPSXSTACK_SDL=ON"], args.jobs, env)
    except Missing as e:
        print(f"render_gpu: {e}")
        return 2
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_AUDIO_DRIVER="dummy")
    if args.lavapipe:
        if not LAVAPIPE.exists():
            print(f"render_gpu: --lavapipe: no {LAVAPIPE}")
            return 2
        env["VK_DRIVER_FILES"] = str(LAVAPIPE)
    fallback(sdl, ROOT / "build/port/dw2003", env)
    if not DISC.exists():
        print("render_gpu: no disc (iso/dw2003.cue): the pictures are skipped")
    else:
        why = device(sdl, env, out)
        if why is not None:
            print(f"render_gpu: no GPU device ({why}): the hardware renderer's pictures are skipped")
        else:
            window(sdl, env)
            screenshots(sdl, env, out)
            vram_checks(sdl, env)
            scaled(sdl, env, out)
            debug_channel(sdl, env, out)
        sdl_renderer(sdl, env, out)
    print(f"render_gpu test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


def main_exe(args, out):
    """--exe [--wine]: the device, the pictures, the VRAM checks and the internal scales of a given binary."""
    exe = Path(args.exe).resolve()
    if not exe.is_file():
        print(f"render_gpu: no binary at {args.exe}")
        return 2
    env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_AUDIO_DRIVER="dummy")
    if args.wine:
        global SLOWER
        if shutil.which("wine") is None:
            print("render_gpu: --wine: no wine on PATH")
            return 2
        prefix = Path(os.environ.get("WINEPREFIX", ROOT / "build/wine-prefix"))
        prefix.mkdir(parents=True, exist_ok=True)
        RUNNER[:] = ["wine"]
        SLOWER = 3
        env.update(WINEPREFIX=str(prefix), WINEDEBUG="-all", SDL_VIDEO_DRIVER="dummy", VKD3D_SHADER_CACHE_PATH="0")
    if args.lavapipe:
        if not LAVAPIPE.exists():
            print(f"render_gpu: --lavapipe: no {LAVAPIPE}")
            return 2
        env["VK_DRIVER_FILES"] = str(LAVAPIPE)
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    print(f"render_gpu: {exe}{' under wine (' + str(env['WINEPREFIX']) + ')' if args.wine else ''}")
    if not DISC.exists():
        print("render_gpu: no disc (iso/dw2003.cue): skipped")
        return 0
    why = device(exe, env, out)
    if why is not None:
        print(f"render_gpu: no GPU device ({why}): the hardware renderer's pictures are skipped")
    else:
        screenshots(exe, env, out)
        vram_checks(exe, env)
        scaled(exe, env, out)
    print(f"render_gpu test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
