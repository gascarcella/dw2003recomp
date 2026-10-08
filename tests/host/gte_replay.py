#!/usr/bin/env python3
"""The gte golden family (tests/golden/gte.json, made by tests/golden/families/gte.py) through the port's software GTE
and LIBGTE (psxstack/psyq/gte.c, libgte.c): every case's 64 registers (or sweep, or LIBGTE output) must equal what the PS1
left in the emulator. tests/host/replay.py runs it for the gte family (its cases call MIPS routines, not game
functions); standalone:

  tests/host/gte_replay.py [-v] [--out DIR] [--m32] [--cflags "..."]   exit 1 on any mismatch

-v lists every differing register of the first mismatches. The harness (tests/host/gte_harness.c) is built with the
port's flags (-std=gnu99 -fsigned-char -fwrapv -fno-strict-aliasing -DPC_PORT -Wall -Wextra -Werror)."""
import argparse
import json
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PSXSTACK = ROOT / "psxstack"   # the stack: the submodule (scripts/worktree_init.sh checks it out)
GOLDEN = ROOT / "tests/golden/gte.json"
OUT_DEFAULT = ROOT / "build/host_gte"
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(ROOT / "tests/golden/families"))
import gte as family   # noqa: E402  (the family's routine table: which address runs which command)
from oracle import Symbols  # noqa: E402

DATA_NAMES = ["VXY0", "VZ0", "VXY1", "VZ1", "VXY2", "VZ2", "RGBC", "OTZ", "IR0", "IR1", "IR2", "IR3", "SXY0", "SXY1",
              "SXY2", "SXYP", "SZ0", "SZ1", "SZ2", "SZ3", "RGB0", "RGB1", "RGB2", "RES1", "MAC0", "MAC1", "MAC2", "MAC3",
              "IRGB", "ORGB", "LZCS", "LZCR"]
CTRL_NAMES = ["RT11_12", "RT13_21", "RT22_23", "RT31_32", "RT33", "TRX", "TRY", "TRZ", "L11_12", "L13_21", "L22_23",
              "L31_32", "L33", "RBK", "GBK", "BBK", "LR1_2", "LR3_G1", "LG2_3", "LB1_2", "LB3", "RFC", "GFC", "BFC", "OFX",
              "OFY", "H", "DQA", "DQB", "ZSF3", "ZSF4", "FLAG"]
REG_NAMES = DATA_NAMES + CTRL_NAMES


def build(out, m32=False, cflags=""):
    out.mkdir(parents=True, exist_ok=True)
    inc = out / "include"
    inc.mkdir(exist_ok=True)
    (inc / "include_asm.h").write_text("#ifndef INCLUDE_ASM_H\n#define INCLUDE_ASM_H\n#define INCLUDE_ASM(FOLDER, NAME)\n"
                                       "#define INCLUDE_RODATA(FOLDER, NAME)\n#endif\n")
    # psxstack_game_gen.h: the game's description, which psxstack's hooks.h includes (the shim's psyq.h includes
    # that): the same header the port's build generates (psxstack/tools/game_gen.py).
    subprocess.run([sys.executable, str(PSXSTACK / "tools/game_gen.py"), str(ROOT / "port/game/game.json"),
                    "--out", str(inc / "psxstack_game_gen.h")], check=True)
    binary = out / ("gte_harness_m32" if m32 else "gte_harness")
    cmd = (["gcc", "-m32" if m32 else "-m64", "-std=gnu99", "-O1", "-fsigned-char", "-fwrapv", "-fno-strict-aliasing",
            "-DPC_PORT", "-DNON_MATCHING", "-Wall", "-Wextra", "-Werror", f"-I{inc}", f"-I{ROOT / 'include'}", f"-I{ROOT}", f"-I{PSXSTACK / 'include'}", f"-I{PSXSTACK / 'include/psxstack'}",
            f"-I{PSXSTACK / 'psyq'}"] + cflags.split()
           + [str(ROOT / "tests/host/gte_harness.c"), str(PSXSTACK / "psyq/gte.c"), str(PSXSTACK / "psyq/libgte.c"),
              str(PSXSTACK / "psyq/gte_shadow.c"), "-o", str(binary)])
    subprocess.run(cmd, check=True)
    return binary


def words(hx):
    return struct.unpack(f"<{len(hx) // 8}I", bytes.fromhex(hx))


def replay(golden, binary, verbose=False):
    """Returns (calls, mismatches): mismatches are (case, what, original, host) tuples."""
    code, labels = family.assemble()
    by_addr = {addr: name for name, addr in labels.items()}
    words_of = dict(family.stub_list())
    sym = Symbols()
    lib_names = {sym[n]: n for n in ("rsin", "rcos", "RotMatrixYXZ_gte", "RotMatrixZYX_gte", "ScaleMatrix", "ApplyMatrixSV")}
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    def ask(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()
        return proc.stdout.readline().strip()

    calls, mismatches = 0, []
    for case in golden["cases"]:
        for call in case["calls"]:
            calls += 1
            label = by_addr[int(call["func"], 16)]
            args = call["args"]
            bufs = case["buffers"]
            expected = "".join(r["hex"] for r in call["reads"])
            if label.startswith("cmd:"):
                word = words_of[label[4:]]
                got = ask(f"G {'-' if word is None else hex(word)} {bufs['in']}")
            elif label == "rtps_sweep":
                got = ask(f"W {bufs['in']} {args[2]} {args[3]}")
            elif label == "sweep":
                got = ask(f"S {lib_names[args[0]]} {args[2]} {args[3]}")
            elif label == "wrap":
                fn = lib_names[args[3]]
                names = [a["buf"] for a in args[:3] if isinstance(a, dict)]
                regs = call["writes"][0]["hex"]
                got = ask(f"F {fn} {regs} " + " ".join(bufs[n] for n in names))
            else:
                raise ValueError(f"{case['name']}: unknown routine {label}")
            if got != expected:
                mismatches.append((case["name"], label, expected, got))
    proc.stdin.close()
    proc.wait()
    if verbose:
        for name, label, exp, got in mismatches[:20]:
            print(f"    {name} ({label})")
            if label.startswith("cmd:") and len(exp) == len(got) == 512:
                inp = words(next(c for c in golden["cases"] if c["name"] == name)["buffers"]["in"])
                for i, (e, g) in enumerate(zip(words(exp), words(got))):
                    if e != g:
                        print(f"      {REG_NAMES[i]:8} in {inp[i]:08x}  PS1 {e:08x}  host {g:08x}")
            else:
                for i in range(0, min(len(exp), len(got)), 8):
                    if exp[i:i + 8] != got[i:i + 8]:
                        print(f"      byte {i // 2:#x}: PS1 {exp[i:i + 8]} host {got[i:i + 8]}")
                        break
    return calls, mismatches


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("--m32", action="store_true", help="build the harness -m32")
    ap.add_argument("--cflags", default="", help="extra compiler flags (e.g. -fsanitize=address,undefined)")
    args = ap.parse_args()
    golden = json.loads(GOLDEN.read_text())
    binary = build(Path(args.out), args.m32, args.cflags)
    calls, mismatches = replay(golden, binary, args.verbose)
    print(f"  gte: {len(golden['cases'])} cases, {calls} calls: "
          + ("matches the PS1" if not mismatches else f"{len(mismatches)} MISMATCH(ES)"))
    if mismatches and not args.verbose:
        for name, label, _, _ in mismatches[:10]:
            print(f"    {name} ({label})")
    return 1 if mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
