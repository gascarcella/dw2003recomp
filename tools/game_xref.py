#!/usr/bin/env python3
"""Cross-reference table of the EXE's game functions, for finding file boundaries.

  tools/venv/bin/python tools/game_xref.py            # needs asm/main/ (scripts/build.sh splits it)

One line per game function (0x80010F4C-0x80020D8C) in address order:
  ADDR SIZE cl[game callers] pt[data tables that point at it] refs  ext:[calls outside the game code]
Refs are shortened by region: rXXXX .rodata, dXXXXX .data, sXXXX .sdata, SXXXX .sbss, bXXXXX .bss,
with gp: in front for $gp-relative accesses. Reading guide: docs/DECISIONS.md "Game code file split".
"""
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM = ROOT / "asm/main"
GAME = (0x80010F4C, 0x80020D8C)
INS = re.compile(r"^\s*/\* \w+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+(\S+)\s*(.*)")


def in_game(a):
    return a is not None and GAME[0] <= a < GAME[1]


def main():
    if not ASM.is_dir():
        sys.exit("asm/main missing: run scripts/build.sh first")
    addr_of = {}
    for line in (ROOT / "config/symbol_addrs.txt").read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", line)
        if m:
            addr_of[m.group(1)] = int(m.group(2), 16)

    # Functions: switch cases that spimdisasm split off (jlabel) are folded into their parent.
    funcs, cur = [], None
    for f in sorted(ASM.glob("*.s")):
        for line in f.read_text().splitlines():
            m = re.match(r"^(glabel|jlabel)\s+(\w+)", line)
            if m:
                if m.group(1) == "glabel" or cur is None:
                    cur = dict(name=m.group(2), addr=None, end=None, refs=set(), gp=set(), calls=set())
                    funcs.append(cur)
                continue
            m = INS.match(line)
            if m and cur is not None:
                a = int(m.group(1), 16)
                cur["addr"] = cur["addr"] or a
                cur["end"] = a + 4
                op, args = m.group(2), m.group(3)
                cur["gp"].update(re.findall(r"%gp_rel\((\w+)", args))
                cur["refs"].update(re.findall(r"%(?:hi|lo)\((\w+)", args))
                if op in ("jal", "j") and not args.startswith(".L"):
                    cur["calls"].add(args.split()[0])
    for fn in funcs:
        addr_of.setdefault(fn["name"], fn["addr"])

    # Data symbols and the symbols stored in them (pointer tables).
    pointed_by = defaultdict(set)
    for f in sorted((ASM / "data").glob("*.s")):
        sym = None
        for line in f.read_text().splitlines():
            m = re.match(r"^dlabel\s+(\w+)", line)
            if m:
                sym = m.group(1)
                if sym.startswith("D_") and sym not in addr_of:
                    addr_of[sym] = int(sym[2:], 16)
                continue
            m = re.search(r"\.word\s+([A-Za-z_]\w*)", line)
            if m and sym:
                pointed_by[m.group(1)].add(sym)

    game = sorted((f for f in funcs if in_game(f["addr"])), key=lambda f: f["addr"])
    names = {f["name"] for f in game}
    callers = defaultdict(set)
    for f in funcs:
        for c in f["calls"]:
            callers[c].add(f["name"])

    def short(a):
        if a < 0x80010E88:
            return f"r{a & 0xFFFF:04X}"
        if a < 0x8005CB50:
            return f"d{a & 0xFFFFF:05X}"
        if a < 0x8005CCE8:
            return f"s{a & 0xFFFF:04X}"
        if a < 0x8005CD38:
            return f"S{a & 0xFFFF:04X}"
        return f"b{a & 0xFFFFF:05X}"

    def data_ref(s):
        a = addr_of.get(s)
        return a is not None and not in_game(a) and not (0x80020D8C <= a < 0x8003EDCC) and a >= 0x80010000

    for f in game:
        refs = sorted({short(addr_of[s]) for s in f["refs"] if data_ref(s)})
        refs += [f"gp:{short(addr_of[s])}" for s in sorted(f["gp"]) if s in addr_of]
        cl = ",".join(f"{addr_of[c] & 0xFFFFF:05X}" for c in sorted(callers[f["name"]] & names, key=addr_of.get))
        pt = ",".join(f"{addr_of[t] & 0xFFFFF:05X}" for t in sorted(pointed_by[f["name"]], key=lambda t: addr_of.get(t, 0)))
        ext = ",".join(sorted(c for c in f["calls"] if c not in names))
        print(f"{f['addr'] & 0xFFFFF:05X} {f['end'] - f['addr']:5X} cl[{cl}] pt[{pt}] {' '.join(refs)}  ext:[{ext}]")


if __name__ == "__main__":
    main()
