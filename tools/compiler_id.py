#!/usr/bin/env python3
"""Compiler-ID experiment: which old GCC (and -G) built the game code?

For a set of game functions: m2c drafts C from the original asm (only mechanical changes:
`--valid-syntax` plus m2c_macros.h; for -G8, `extern`s of the variables the original reaches
through $gp become definitions, see do_function), the draft is compiled with every GCC in tools/gcc via
tools/cc_psx.sh at -G0 and -G8, and objdiff scores each object against the original function
assembled from asm/main/. Scores come from natural m2c C only, never from permuter output
(DECISIONS 2026-10-01, Milestone 1).

Needs a split tree (scripts/build.sh); reads every game unit in asm/main/. Work files go to build/compiler_id/; the results are
build/compiler_id/results.csv and a summary on stdout.

  tools/venv/bin/python tools/compiler_id.py [--max N] [--min-insns N] [--max-insns N] [-j N]
  tools/venv/bin/python tools/compiler_id.py --target fieldstg --rodata-end 0x80083784   # an overlay
  tools/venv/bin/python tools/compiler_id.py --target wfightmn --rodata-start 0x800A5DE0 --rodata-end 0x800A5ECC  # tier 2
"""
import argparse
import csv
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
T = ROOT / "tools"
PY = str(T / "venv/bin/python")
AS = str(T / "binutils/bin/mipsel-linux-gnu-as")
GCCS = ["2.7.2", "2.8.0", "2.8.1", "2.91.66", "2.95.2"]
GS = [0, 8]
RODATA_START, RODATA_END = 0x80010000, 0x80010E88  # .rodata is 0x80010000-0x80010E88 (EXE; --rodata-end for other targets)
WORK = ROOT / "build/compiler_id"  # + /<target> for targets other than main
NON_GAME_UNITS = {"header", "crt0", "psyq"}

FUNC_RE = re.compile(r"^nonmatching (\w+), 0x([0-9A-Fa-f]+)\n(.*?)^endlabel \1\n", re.M | re.S)


def parse_functions(path: Path):
    text = path.read_text()
    for m in FUNC_RE.finditer(text):
        name, size, body = m.group(1), int(m.group(2), 16), m.group(3)
        insns = len(re.findall(r"^\s+/\* [0-9A-F]+ [0-9A-F]{8} [0-9A-F]{8} \*/", body, re.M))
        refs = set(re.findall(r"%(?:hi|lo|gp_rel)\((\w+)", body))
        rodata = any(r.startswith("jtbl_") or (re.match(r"D_(\w+_)?[0-9A-F]{8}$", r) and
                                                 int(r[-8:], 16) < RODATA_END and int(r[-8:], 16) >= RODATA_START)
                     for r in refs)
        yield {
            "name": name, "size": size, "insns": insns, "block": m.group(0),
            "split_case": bool(re.search(rf"^jlabel {name}\b", body, re.M)),
            # jump tables / computed jumps (jr through a register other than $ra)
            "jtbl": "jtbl_" in body or re.search(r"\bjr\s+\$(?!ra)", body) is not None,
            "rodata": rodata,
            "gp": "%gp_rel" in body,
            "unit": path.stem,
        }


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def match_percent(target_o, base_o, name):
    r = run([str(T / "bin/objdiff-cli"), "diff", "-1", str(target_o), "-2", str(base_o), "-o", "-", name])
    if r.returncode != 0:
        return None
    d = json.loads(r.stdout)
    for sym in d.get("left", {}).get("symbols", []):
        if sym.get("name") == name and sym.get("kind") == "SYMBOL_FUNCTION":
            return float(sym.get("match_percent", 0.0))
    return 0.0


def do_function(f):
    d = WORK / f["name"]
    d.mkdir(parents=True, exist_ok=True)
    # Target: the original function, assembled on its own.
    s = d / "target.s"
    s.write_text('.include "macro.inc"\n.set noat\n.set noreorder\n.section .text\n\n' + f["block"])
    r = run([AS, "-EL", "-march=r3000", "-mtune=r3000", "-no-pad-sections", "-G0",
             f"-I{ROOT}/include/asm_generated", "-o", str(d / "target.o"), str(s)])
    if r.returncode != 0:
        return [{**f, "gcc": "-", "g": "-", "status": "target_as_failed"}]
    # Draft C from m2c, unedited.
    r = run([PY, str(T / "ext/m2c/m2c.py"), "--target", "mips-gcc-c", "--valid-syntax", str(s)])
    if r.returncode != 0 or not r.stdout.strip():
        return [{**f, "gcc": "-", "g": "-", "status": "m2c_failed"}]
    prelude = '#include "common.h"\n#include "m2c_macros.h"\n\n'
    c = d / "draft.c"
    c.write_text(prelude + r.stdout)
    # For -G8: ASPSX (and maspsx) only use $gp for symbols defined earlier in the same file, never
    # for `.extern`s, so the original must have defined its $gp variables in the same file. Turn
    # m2c's `extern` declarations of the symbols the original reaches through $gp into definitions.
    gp_syms = set(re.findall(r"%gp_rel\((\w+)\)", f["block"]))
    c8 = d / "draft_G8.c"
    c8.write_text(prelude + re.sub(
        r"^extern ([^;\[]*\b(\w+));$",
        lambda m: m.group(1) + ";" if m.group(2) in gp_syms else m.group(0),
        r.stdout, flags=re.M))
    m2c_error = "M2C_ERROR" in r.stdout
    rows = []
    for gcc in GCCS:
        for g in GS:
            o = d / f"{gcc}_G{g}.o"
            src = c8 if g else c
            r = run([str(T / "cc_psx.sh"), "-V", gcc, "-G", str(g), f"-I{ROOT}/include",
                     f"-I{ROOT}/include/asm_generated", f"-I{T}/ext/m2c", str(src), "-o", str(o)])
            if r.returncode != 0:
                rows.append({**f, "gcc": gcc, "g": g, "status": "cc_failed", "m2c_error": m2c_error})
                continue
            pct = match_percent(d / "target.o", o, f["name"])
            rows.append({**f, "gcc": gcc, "g": g, "status": "ok" if pct is not None else "diff_failed",
                         "pct": pct, "m2c_error": m2c_error})
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--max", type=int, default=150, help="number of functions (evenly spread by size)")
    ap.add_argument("--min-insns", type=int, default=4)
    ap.add_argument("--max-insns", type=int, default=150)
    ap.add_argument("-j", type=int, default=os.cpu_count())
    ap.add_argument("--target", default="main", help="splat target: asm/<target>/ (default main)")
    ap.add_argument("--rodata-end", type=lambda x: int(x, 0),
                    help="end of the target's .rodata (overlays: its .text start); the target's base is its start")
    ap.add_argument("--rodata-start", type=lambda x: int(x, 0), default=0x80082CB0,
                    help="start of an overlay's .rodata = its base (default tier 1; tier 2: 0x800A5DE0)")
    args = ap.parse_args()
    global RODATA_START, RODATA_END, WORK
    if args.target != "main":
        RODATA_START, RODATA_END = args.rodata_start, args.rodata_end or args.rodata_start
        WORK = WORK / args.target

    # Every game unit (asm units and the full disassembly of C units), not the SDK or the header.
    units = [p for p in sorted((ROOT / "asm" / args.target).glob("*.s")) if p.stem not in NON_GAME_UNITS]
    funcs = [f for p in units for f in parse_functions(p)]
    print(f"{len(funcs)} functions in {len(units)} game units")
    excl = defaultdict(int)
    cands = []
    for f in funcs:
        for k in ("split_case", "jtbl", "rodata"):
            if f[k]:
                excl[k] += 1
                break
        else:
            if args.min_insns <= f["insns"] <= args.max_insns:
                cands.append(f)
            else:
                excl["size"] += 1
    print(f"excluded: {dict(excl)}; candidates: {len(cands)}")
    cands.sort(key=lambda f: f["insns"])
    if len(cands) > args.max:  # spread evenly over the size range
        step = len(cands) / args.max
        cands = [cands[int(i * step)] for i in range(args.max)]
    print(f"selected {len(cands)} (insns {cands[0]['insns']}-{cands[-1]['insns']}, "
          f"{sum(f['gp'] for f in cands)} use $gp)")

    WORK.mkdir(parents=True, exist_ok=True)
    with ThreadPoolExecutor(args.j) as ex:
        rows = [r for rs in ex.map(do_function, cands) for r in rs]

    keys = ["name", "unit", "insns", "gp", "gcc", "g", "status", "pct", "m2c_error"]
    with open(WORK / "results.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, keys, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)

    bad = defaultdict(int)
    for r in rows:
        if r["status"] in ("target_as_failed", "m2c_failed"):
            bad[r["status"]] += 1
    print(f"setup failures: {dict(bad) or 'none'}")

    ok = [r for r in rows if r["status"] == "ok"]
    names = sorted({r["name"] for r in ok})
    clean = {r["name"] for r in ok if not r["m2c_error"]}
    print(f"\n{len(names)} functions scored ({len(clean)} without M2C_ERROR)\n")

    def best(name, gcc, gs=GS):
        ps = [r["pct"] for r in ok if r["name"] == name and r["gcc"] == gcc and r["g"] in gs]
        return max(ps) if ps else 0.0

    print(f"{'gcc':8} {'exact (best G)':>14} {'exact -G0':>9} {'exact -G8':>9} {'mean %':>7} "
          f"{'cc fails':>8}   {'exact, clean m2c':>16}")
    for gcc in GCCS:
        b = [best(n, gcc) for n in names]
        e0 = sum(best(n, gcc, [0]) == 100.0 for n in names)
        e8 = sum(best(n, gcc, [8]) == 100.0 for n in names)
        fails = sum(1 for r in rows if r["gcc"] == gcc and r["status"] == "cc_failed")
        ec = sum(best(n, gcc) == 100.0 for n in clean)
        print(f"{gcc:8} {sum(p == 100.0 for p in b):14} {e0:9} {e8:9} {sum(b) / len(b):7.2f} {fails:8}"
              f"   {ec:16}")

    # Functions only some compilers match: the evidence that separates them.
    print("\nfunctions matched exactly by some but not all compilers:")
    for n in names:
        m = [g for g in GCCS if best(n, g) == 100.0]
        if m and len(m) < len(GCCS):
            print(f"  {n:16} {next(r['insns'] for r in ok if r['name'] == n):4} insns  matched by {', '.join(m)}")

    # -G: for exact matches, which -G was needed, vs whether the original uses $gp.
    print("\n-G vs $gp use (functions matched exactly by the best compiler):")
    top = max(GCCS, key=lambda g: sum(best(n, g) == 100.0 for n in names))
    for gp in (False, True):
        sel = [n for n in names if next(r["gp"] for r in ok if r["name"] == n) == gp]
        e0 = sum(best(n, top, [0]) == 100.0 for n in sel)
        e8 = sum(best(n, top, [8]) == 100.0 for n in sel)
        print(f"  {'uses $gp' if gp else 'no $gp':9} {len(sel):4} functions: {top} exact at -G0 {e0}, at -G8 {e8}")
    print(f"\nper-row results: {WORK / 'results.csv'}")


if __name__ == "__main__":
    main()
