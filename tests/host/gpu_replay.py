#!/usr/bin/env python3
"""The gpu golden family (tests/golden/gpu.json, made by tests/golden/families/gpu.py) through the port's LIBGPU and
software GPU (port/psyq/libgpu.c, gpu.c): every VRAM rectangle a case reads back, every packet a LIBGPU builder wrote
and every return value must equal what the PS1 (the emulator) left, or be a known difference
(tests/host/known_mismatches.json, "gpu/<case>:": the emulator rules gpu.c does not reproduce, and the hardware rules it
keeps where the emulator differs; port/psyq/gpu.c's header). tests/host/replay.py runs it for the gpu family (exit 1 on a
new mismatch); standalone:

  tests/host/gpu_replay.py [-v] [--out DIR] [--m32] [--cflags "..."] [--results RUN/results.json] [--case NAME]

The golden keeps large reads as a SHA-1; --results takes an oracle run's raw results (tests/golden/oracle.py gen|check
--out RUN) instead, so that -v can show the differing pixels. The harness (tests/host/gpu_harness.c) is built with the
port's flags and the shim's psyq.c (its psyq_reset, which needs the other libraries, is dropped by --gc-sections)
(-std=gnu99 -fsigned-char -fwrapv -fno-strict-aliasing -DPC_PORT -Wall -Wextra -Werror)."""
import argparse
import hashlib
import json
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GOLDEN = ROOT / "tests/golden/gpu.json"
OUT_DEFAULT = ROOT / "build/host_gpu"
KNOWN = ROOT / "tests/host/known_mismatches.json"
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(ROOT / "tests/golden/families"))
import gpu as family   # noqa: E402
from oracle import SCRATCH_BASE  # noqa: E402


def build(out, m32=False, cflags=""):
    out.mkdir(parents=True, exist_ok=True)
    inc = out / "include"
    inc.mkdir(exist_ok=True)
    (inc / "include_asm.h").write_text("#ifndef INCLUDE_ASM_H\n#define INCLUDE_ASM_H\n#define INCLUDE_ASM(FOLDER, NAME)\n"
                                       "#define INCLUDE_RODATA(FOLDER, NAME)\n#endif\n")
    binary = out / ("gpu_harness_m32" if m32 else "gpu_harness")
    psyq = ROOT / "port/psyq"
    cmd = (["gcc", "-m32" if m32 else "-m64", "-std=gnu99", "-O1", "-fsigned-char", "-fwrapv", "-fno-strict-aliasing",
            "-DPC_PORT", "-DNON_MATCHING", "-Wall", "-Wextra", "-Werror", f"-I{inc}", f"-I{ROOT / 'include'}", f"-I{ROOT}",
            f"-I{psyq}"] + cflags.split()
           + [str(ROOT / "tests/host/gpu_harness.c"), str(psyq / "libgpu.c"), str(psyq / "gpu.c"), str(psyq / "psyq.c"),
              "-ffunction-sections", "-Wl,--gc-sections", "-o", str(binary)])
    subprocess.run(cmd, check=True)
    return binary


def layout(case):
    """name -> address of the case's buffers (tests/golden/oracle.py layout_buffers: in order, 16-aligned)."""
    addr, places = SCRATCH_BASE, {}
    for name, hx in case["buffers"].items():
        places[name] = addr
        addr += (max(len(hx) // 2, 1) + 15) & ~15
    return places


def arg_value(places, a):
    if isinstance(a, dict):
        return places[a["buf"]] if "buf" in a else a["symbol"]   # a symbol: call5's function, by name
    return a


class Mem:
    """The bytes the case's calls write and read: the buffers, the pattern area and StoreImage's output."""

    def __init__(self, case, places):
        self.blocks = {places[n]: bytearray.fromhex(hx) for n, hx in case["buffers"].items()}

    def put(self, addr, data):
        self.blocks[addr] = bytearray(data)

    def get(self, addr, size):
        for base, data in self.blocks.items():
            if base <= addr and addr + size <= base + len(data):
                return bytes(data[addr - base:addr - base + size])
        raise KeyError(f"no data at {addr:#x}+{size}")

    def rect(self, addr):
        return struct.unpack("<4h", self.get(addr, 8))


def replay(golden, binary, verbose=False, results=None, only=None):
    """Returns (calls, mismatches): mismatches are (case, what, original, host) tuples."""
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    def ask(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()
        return proc.stdout.readline().strip()

    raw = {}
    if results:
        raw = {j["name"]: j for j in json.loads(Path(results).read_text())["jobs"]}
    calls, mismatches = 0, []
    for case in golden["cases"]:
        if only and case["name"] not in only:
            continue
        places = layout(case)
        mem = Mem(case, places)
        job = raw.get(f"gpu/{case['name']}")
        for ci, call in enumerate(case["calls"]):
            calls += 1
            fn = call["func"]
            args = [arg_value(places, a) for a in call["args"]]
            ret = None
            if fn == f"0x{family.CODE_ADDR:08X}":
                px = family.pattern(args[1], args[2], args[3])
                mem.put(args[0], struct.pack(f"<{len(px)}H", *px))
                ret = 0
            elif fn == "LoadImage":
                x, y, w, h = mem.rect(args[0])
                ask(f"L {x} {y} {w} {h} {mem.get(args[1], 2 * w * h).hex()}")
            elif fn == "DrawOTag":
                name = next(n for n, a in places.items() if a == args[0])
                ask(f"O {args[0]:x} {case['buffers'][name]}")
            elif fn == "StoreImage":
                x, y, w, h = mem.rect(args[0])
                mem.put(args[1], bytes.fromhex(ask(f"S {x} {y} {w} {h}")))
            elif fn == "DrawSync":
                ret = 0
            elif fn == "BreakDraw":
                ret = int(ask("B"))
            elif fn == "MoveImage":
                x, y, w, h = mem.rect(args[0])
                ret = int(ask(f"M {x} {y} {w} {h} {args[1]} {args[2]}"))
            elif fn in ("ClearImage", "ClearImage2"):
                x, y, w, h = mem.rect(args[0])
                ret = int(ask(f"{'C2' if fn == 'ClearImage2' else 'C'} {x} {y} {w} {h} {args[1]} {args[2]} {args[3]}"))
            elif fn == "SetDrawEnv":
                got = ask(f"E {mem.get(args[0], 64).hex()} {mem.get(args[1], 92).hex()}")
                mem.put(args[0], bytes.fromhex(got))
            elif fn == f"0x{family.CALL5_ADDR:08X}" and args[0] == "SetDefDrawEnv":
                words = struct.unpack("<I4i", mem.get(args[1], 20))
                got = ask(f"D {mem.get(words[0], 92).hex()} {words[1]} {words[2]} {words[3]} {words[4]}")
                mem.put(words[0], bytes.fromhex(got))
                ret = words[0]
            elif fn == "SetDrawMove":
                x, y, w, h = mem.rect(args[1])
                got = ask(f"V {mem.get(args[0], 24).hex()} {x} {y} {w} {h} {args[2]} {args[3]}")
                mem.put(args[0], bytes.fromhex(got))
            else:
                raise ValueError(f"{case['name']}: no host mirror for {fn}")
            if ret is not None and call["ret_type"] != "void" and ret != call["ret"]:
                mismatches.append((case["name"], f"{fn} return", call["ret"], ret))
            for ri, rd in enumerate(call.get("reads", [])):
                if rd["symbol"].startswith("buf:"):
                    addr = places[rd["symbol"][4:]] + rd["offset"]
                else:
                    addr = int(rd["symbol"], 16) + rd["offset"]
                got = mem.get(addr, rd["size"])
                if job is not None:
                    exp_hex = job["calls"][ci]["reads"][ri]
                    ok = got.hex() == exp_hex
                    expected = exp_hex
                else:
                    ok = (hashlib.sha1(got).hexdigest() == rd["sha1"]) if "sha1" in rd else got.hex() == rd["hex"]
                    expected = rd.get("sha1") or rd.get("hex")
                if not ok:
                    mismatches.append((case["name"], f"{fn} read {rd['field']}", expected, got.hex()))
    proc.stdin.close()
    proc.wait()
    if verbose:
        show(mismatches, golden)
    return calls, mismatches


def show(mismatches, golden, limit=20):
    for name, what, exp, got in mismatches[:limit]:
        print(f"    {name}: {what}")
        if len(exp) == len(got) and "VRAM" in what:
            field = what.split("VRAM ")[1]
            pos, size = field.split(" ")
            x0, y0 = map(int, pos.split(","))
            w = int(size.split("x")[0])
            e = struct.unpack(f"<{len(exp) // 4}H", bytes.fromhex(exp))
            g = struct.unpack(f"<{len(got) // 4}H", bytes.fromhex(got))
            diffs = [i for i in range(len(e)) if e[i] != g[i]]
            print(f"      {len(diffs)} pixel(s) differ; first: " + ", ".join(
                f"({x0 + i % w},{y0 + i // w}) PS1 {e[i]:04x} host {g[i]:04x}" for i in diffs[:6]))
        else:
            print(f"      PS1 {str(exp)[:120]}\n      host {str(got)[:120]}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("--m32", action="store_true", help="build the harness -m32")
    ap.add_argument("--cflags", default="", help="extra compiler flags (e.g. -fsanitize=address,undefined)")
    ap.add_argument("--results", help="an oracle run's results.json: compare raw pixels instead of SHA-1s")
    ap.add_argument("--case", action="append", help="only this case (repeatable)")
    args = ap.parse_args()
    golden = json.loads(GOLDEN.read_text())
    binary = build(Path(args.out), args.m32, args.cflags)
    calls, mismatches = replay(golden, binary, args.verbose, args.results, args.case)
    known = [k["where"] for k in json.loads(KNOWN.read_text()) if k["where"].startswith("gpu/")]
    new = [m for m in mismatches if not any(f"gpu/{m[0]}: {m[1]}".startswith(k) for k in known)]
    print(f"  gpu: {len(golden['cases'])} cases, {calls} calls: "
          + ("matches the PS1" if not mismatches else f"{len(new)} NEW MISMATCH(ES), {len(mismatches) - len(new)} known"))
    if new and not args.verbose:
        for name, what, _, _ in new[:10]:
            print(f"    {name} ({what})")
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
