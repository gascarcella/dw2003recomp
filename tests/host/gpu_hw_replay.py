#!/usr/bin/env python3
"""The hardware renderer's exactness at internal scale 1 (psxstack/runtime/render_gpu.c; issue #31): the gpu golden family's
715 command lists (tests/golden/gpu.json, as tests/host/gpu_replay.py replays them) through the software GPU with the
rasteriser listening, and after every case the whole 1024x512 VRAM target read back and compared with the software
VRAM, pixel for pixel. The reference is gpu.c, not the PS1: gpu.c's own differences from the emulator are
tests/host/known_mismatches.json's business. A case that differs is either in the ledger
(tests/host/gpu_hw_mismatches.json: case names, each with the reason) or a failure; after it the target is set to the
software VRAM again, so every case starts equal.

  tests/host/gpu_hw_replay.py [-v] [--out DIR] [--lavapipe] [--case NAME]

Needs a GPU device (Vulkan; --lavapipe uses Mesa's software driver) and SDL's offscreen video driver; without a device
it prints why and exits 0 (skipped), as CI does. It builds build/port-sdl first (the shaders' headers; tools/sdl3 and
tools/dxc), then the harness (tests/host/gpu_harness.c with -DGPU_HW, psxstack/runtime/render_gpu*.c, SDL3).
Exit codes: 0 pass or skipped (no device, no tools/sdl3 or tools/dxc: said so), 1 a difference the ledger does not
list (or a ledger entry that no longer differs), 2 --lavapipe without lavapipe."""
import argparse
import json
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/host"))
sys.path.insert(0, str(ROOT / "tests/port"))
import gpu_replay  # noqa: E402  (the golden's layout and call mirrors)
from run import Missing, build as build_port, tool_env  # noqa: E402

GOLDEN = ROOT / "tests/golden/gpu.json"
LEDGER = ROOT / "tests/host/gpu_hw_mismatches.json"
OUT_DEFAULT = ROOT / "build/host_gpu_hw"
LAVAPIPE = Path("/usr/share/vulkan/icd.d/lvp_icd.x86_64.json")


# The renderer's parts the harness stubs instead of compiling (tests/host/gpu_harness.c "texture keys").
HARNESS_STUBS = ("render_gpu_textures.c", "render_gpu_packs.c")

def tool(name):
    """A built tool: this checkout's tools/<name>, else the main checkout's."""
    here = ROOT / "tools" / name
    if here.exists():
        return here
    common = subprocess.run(["git", "-C", str(ROOT), "rev-parse", "--path-format=absolute", "--git-common-dir"],
                            capture_output=True, text=True).stdout.strip()
    return Path(common).parent / "tools" / name


def build(out, env):
    sdl = tool("sdl3")
    if not (sdl / "lib/libSDL3.a").exists():
        raise Missing("no tools/sdl3 (scripts/setup.sh sdl3)")
    if not (tool("dxc") / "bin/dxc").exists():
        raise Missing("no tools/dxc (scripts/setup.sh dxc)")
    if shutil.which("pkg-config") is None:
        raise Missing("no pkg-config (SDL3's static link flags)")
    build_port("build/port-sdl", ["-DPSXSTACK_SDL=ON"], 0, env)   # the shaders' generated headers
    out.mkdir(parents=True, exist_ok=True)
    inc = out / "include"
    inc.mkdir(exist_ok=True)
    (inc / "include_asm.h").write_text("#ifndef INCLUDE_ASM_H\n#define INCLUDE_ASM_H\n#define INCLUDE_ASM(FOLDER, NAME)\n"
                                       "#define INCLUDE_RODATA(FOLDER, NAME)\n#endif\n")
    libs = subprocess.run(["pkg-config", "--static", "--libs", "sdl3"], capture_output=True, text=True,
                          env=dict(env, PKG_CONFIG_PATH=str(sdl / "lib/pkgconfig")), check=True).stdout.split()
    binary = out / "gpu_hw_harness"
    psyq = gpu_replay.PSXSTACK / "psyq"
    cmd = (["gcc", "-std=gnu99", "-O1", "-fsigned-char", "-fwrapv", "-fno-strict-aliasing", "-DPC_PORT",
            "-DNON_MATCHING", "-DGPU_HW", "-DPSXSTACK_SDL", "-Wall", "-Wextra", "-Werror", f"-I{inc}",
            f"-I{ROOT / 'include'}", f"-I{ROOT}", f"-I{psyq}", f"-I{gpu_replay.PSXSTACK / 'include'}", f"-I{gpu_replay.PSXSTACK / 'include/psxstack'}", f"-I{gpu_replay.PSXSTACK / 'runtime'}", f"-I{ROOT / 'build/port-sdl/gen/include'}",
            f"-I{ROOT / 'build/port-sdl/gen/shaders'}", f"-I{sdl / 'include'}",
            str(ROOT / "tests/host/gpu_harness.c"), str(psyq / "libgpu.c"), str(psyq / "gpu.c"), str(psyq / "psyq.c"),
            str(psyq / "gte_shadow.c"),
            # the renderer's files (render_gpu.c and its render_gpu_<part>.c) and the filters' names they log; the
            # texture keys and packs are off here (gpu_harness.c stubs the renderer's calls into them)
            *[str(p) for p in sorted((gpu_replay.PSXSTACK / "runtime").glob("render_gpu*.c"))
              if p.name not in HARNESS_STUBS],
            str(gpu_replay.PSXSTACK / "runtime/video_filter.c"),
            "-ffunction-sections", "-Wl,--gc-sections", "-o", str(binary)]
           + libs)
    subprocess.run(cmd, check=True)
    return binary


def replay(golden, binary, env, only=None, verbose=False):
    """{case: hardware answer} for every case that differs; None when there is no device (the reason printed)."""
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, env=env)
    first = proc.stdout.readline().strip()
    if not first.startswith("device "):
        proc.wait()
        print(f"  gpu_hw: no GPU device ({first or 'the harness exited ' + str(proc.returncode)}): skipped")
        return None
    print(f"  gpu_hw: {first[7:]}")

    def ask(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()
        return proc.stdout.readline().strip()

    differs = {}
    for case in golden["cases"]:
        if only and case["name"] not in only:
            continue
        places = gpu_replay.layout(case)
        mem = gpu_replay.Mem(case, places)
        for call in case["calls"]:
            fn = call["func"]
            args = [gpu_replay.arg_value(places, a) for a in call["args"]]
            if fn == f"0x{gpu_replay.family.CODE_ADDR:08X}":
                px = gpu_replay.family.pattern(args[1], args[2], args[3])
                mem.put(args[0], struct.pack(f"<{len(px)}H", *px))
            elif fn == "LoadImage":
                x, y, w, h = mem.rect(args[0])
                ask(f"L {x} {y} {w} {h} {mem.get(args[1], 2 * w * h).hex()}")
            elif fn == "DrawOTag":
                name = next(n for n, a in places.items() if a == args[0])
                ask(f"O {args[0]:x} {case['buffers'][name]}")
            elif fn == "StoreImage":
                x, y, w, h = mem.rect(args[0])
                mem.put(args[1], bytes.fromhex(ask(f"S {x} {y} {w} {h}")))
            elif fn == "BreakDraw":
                ask("B")
            elif fn == "MoveImage":
                x, y, w, h = mem.rect(args[0])
                ask(f"M {x} {y} {w} {h} {args[1]} {args[2]}")
            elif fn in ("ClearImage", "ClearImage2"):
                x, y, w, h = mem.rect(args[0])
                ask(f"{'C2' if fn == 'ClearImage2' else 'C'} {x} {y} {w} {h} {args[1]} {args[2]} {args[3]}")
            elif fn == "SetDrawEnv":
                mem.put(args[0], bytes.fromhex(ask(f"E {mem.get(args[0], 64).hex()} {mem.get(args[1], 92).hex()}")))
            elif fn == f"0x{gpu_replay.family.CALL5_ADDR:08X}" and args[0] == "SetDefDrawEnv":
                words = struct.unpack("<I4i", mem.get(args[1], 20))
                got = ask(f"D {mem.get(words[0], 92).hex()} {words[1]} {words[2]} {words[3]} {words[4]}")
                mem.put(words[0], bytes.fromhex(got))
            elif fn == "SetDrawMove":
                x, y, w, h = mem.rect(args[1])
                got = ask(f"V {mem.get(args[0], 24).hex()} {x} {y} {w} {h} {args[2]} {args[3]}")
                mem.put(args[0], bytes.fromhex(got))
            elif fn != "DrawSync":
                raise ValueError(f"{case['name']}: no host mirror for {fn}")
        answer = ask("H")
        if answer != "ok":
            differs[case["name"]] = answer
            if verbose:
                print(f"    {case['name']}: {answer}")
    proc.stdin.close()
    proc.wait()
    return differs


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-v", "--verbose", action="store_true", help="every differing case as it comes")
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("--lavapipe", action="store_true", help="Mesa's software Vulkan driver")
    ap.add_argument("--case", action="append", help="only this case (repeatable)")
    args = ap.parse_args()
    env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen")
    if args.lavapipe:
        if not LAVAPIPE.exists():
            print(f"gpu_hw: --lavapipe: no {LAVAPIPE}")
            return 2
        env["VK_DRIVER_FILES"] = str(LAVAPIPE)
    try:
        binary = build(Path(args.out), tool_env())
    except Missing as e:
        print(f"  gpu_hw: {e}: skipped")
        return 0
    golden = json.loads(GOLDEN.read_text())
    differs = replay(golden, binary, env, args.case, args.verbose)
    if differs is None:
        return 0
    ledger = {k["case"]: k["reason"] for k in json.loads(LEDGER.read_text())}
    new = {c: a for c, a in differs.items() if c not in ledger}
    gone = [c for c in ledger if c not in differs and (not args.case or c in args.case)]
    cases = len(args.case) if args.case else len(golden["cases"])
    print(f"  gpu_hw: {cases} cases, {len(differs)} differ from the software VRAM "
          f"({len(differs) - len(new)} in the ledger), {len(new)} NEW" + (f", {len(gone)} ledger entries no longer "
                                                                        f"differ" if gone else ""))
    for c, a in list(new.items())[:20]:
        print(f"    {c}: {a} (count, first x y, software, hardware)")
    for c in gone[:20]:
        print(f"    {c}: in the ledger, now equal")
    return 1 if new or gone else 0


if __name__ == "__main__":
    sys.exit(main())
