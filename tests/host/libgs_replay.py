#!/usr/bin/env python3
"""The libgs_view golden family (tests/golden/libgs_view.json, made by tests/golden/families/libgs_view.py) through the
port's LIBGS and LIBGTE (psxstack/psyq/libgs.c, libgte.c, gte.c): GsSetRefView2's world-screen matrix and its copy,
GsGetLw's coordinate systems, the LIBGTE functions' outputs, v0 and the GTE registers each leaves must equal what the PS1 left in
the emulator (issue #7: the battle camera). tests/host/replay.py runs it for the libgs_view family; standalone:

  tests/host/libgs_replay.py [-v] [--out DIR] [--m32] [--cflags "..."]   exit 1 on any mismatch

The harness (tests/host/libgs_harness.c) is built with the port's flags, as gte_replay.py builds its own."""
import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PSXSTACK = ROOT / "psxstack"   # the stack: the submodule, or the sibling clone linked there
GOLDEN = ROOT / "tests/golden/libgs_view.json"
OUT_DEFAULT = ROOT / "build/host_libgs"
sys.path.insert(0, str(ROOT / "tests/golden"))
sys.path.insert(0, str(ROOT / "tests/golden/families"))
import gte  # noqa: E402  (the wrapper's scratch addresses)
import libgs_view as family  # noqa: E402
from oracle import LARGE_READ, Symbols  # noqa: E402

WRAPPED = ("GsSetRefView2", "GsGetLw", "GsSetFlatLight", "MulMatrix", "MulMatrix2", "ApplyMatrixLV", "TransposeMatrix",
           "SquareRoot0")


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
    binary = out / ("libgs_harness_m32" if m32 else "libgs_harness")
    cmd = (["gcc", "-m32" if m32 else "-m64", "-std=gnu99", "-O1", "-fsigned-char", "-fwrapv", "-fno-strict-aliasing",
            "-DPC_PORT", "-DNON_MATCHING", "-Wall", "-Wextra", "-Werror", f"-I{inc}", f"-I{ROOT / 'include'}", f"-I{ROOT}", f"-I{PSXSTACK / 'include'}", f"-I{PSXSTACK / 'include/psxstack'}",
            f"-I{PSXSTACK / 'psyq'}"] + cflags.split()
           + [str(ROOT / "tests/host/libgs_harness.c"), str(PSXSTACK / "psyq/libgs.c"), str(PSXSTACK / "psyq/libgte.c"),
              str(PSXSTACK / "psyq/gte.c"), "-o", str(binary)])
    subprocess.run(cmd, check=True)
    return binary


def shown(read, hx):
    """A read as the golden keeps it: hex, or the SHA-1 of a large one."""
    return hashlib.sha1(bytes.fromhex(hx)).hexdigest() if read["size"] > LARGE_READ else hx


def expected_of(read):
    return read.get("sha1") or read["hex"]


def replay(golden, binary, verbose=False):
    """Returns (calls, mismatches): mismatches are (case, what, original, host) tuples."""
    sym = Symbols()
    names = {sym[n]: n for n in WRAPPED}
    wrap = gte.assemble()[1]["wrap"]
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    def ask(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()
        return proc.stdout.readline().split()

    state_base, state_psdcnt = ask("S")
    calls, mismatches = 0, []
    for case in golden["cases"]:
        bufs = case["buffers"]
        for k, call in enumerate(case["calls"]):
            calls += 1
            if int(call["func"], 16) != wrap:
                raise ValueError(f"{case['name']}: not the wrapper ({call['func']})")
            fn = names[call["args"][3]]
            regs = call["writes"][0]["hex"]
            got = {}
            if fn in ("GsSetRefView2", "GsGetLw"):
                a0, a1 = call["args"][0], call["args"][1]
                ws, ws_copy, view, out_regs, v0 = ask(f"V {fn} {regs} {bufs['view']} {a0:#x} {a1:#x}")
                got = {f"0x{family.WS_ADDR:08X}": ws, f"0x{family.WS_COPY_ADDR:08X}": ws_copy, "buf:view": view,
                       f"0x{family.VIEW_BASE_ADDR:08X}": state_base, f"0x{family.PSDCNT_ADDR:08X}": state_psdcnt}
            elif fn == "GsSetFlatLight":
                # the case's calls build on each other (a stage's three lights); the first starts from zero matrices
                light = bufs[call["args"][1]["buf"]]
                lm, cm, out_regs, v0 = ask(f"L {regs} {call['args'][0]:#x} {light} {1 if k == 0 else 0}")
                got = {f"0x{family.LIGHT_ADDR:08X}": lm, f"0x{family.COLOR_ADDR:08X}": cm}
            else:
                args, distinct = [], []
                for i, a in enumerate(call["args"][:3]):
                    if isinstance(a, dict):
                        prev = [j for j in range(i) if call["args"][j] == a]
                        if prev:
                            args.append(f"={prev[0]}")
                        else:
                            args.append(bufs[a["buf"]])
                            distinct.append(a["buf"])
                    else:
                        args.append(f"#{a}")
                answer = ask(f"F {fn} {regs} " + " ".join(args))
                *outs, out_regs, v0 = answer
                got = {f"buf:{n}": hx for n, hx in zip(distinct, outs)}
            got[f"0x{gte.OUT_ADDR:08X}"] = out_regs
            for read in call["reads"]:
                key = read["symbol"]
                hx = got.get(key)
                host = shown(read, hx[2 * read["offset"]:2 * (read["offset"] + read["size"])]) if hx else None
                if host != expected_of(read):
                    mismatches.append((case["name"], f"{fn}: {read['field']}", expected_of(read), host))
            if call["ret_type"] != "void" and int(v0, 16) != call["v0"]:
                mismatches.append((case["name"], f"{fn}: v0", f"{call['v0']:08x}", v0))
    proc.stdin.close()
    proc.wait()
    if verbose:
        for name, what, exp, got in mismatches[:20]:
            print(f"    {name} ({what})\n      PS1  {exp}\n      host {got}")
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
    print(f"  libgs_view: {len(golden['cases'])} cases, {calls} calls: "
          + ("matches the PS1" if not mismatches else f"{len(mismatches)} MISMATCH(ES)"))
    if mismatches and not args.verbose:
        for name, what, _, _ in mismatches[:10]:
            print(f"    {name} ({what})")
    return 1 if mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
