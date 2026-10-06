#!/usr/bin/env python3
"""Cross-reference table of an overlay's functions, for finding its file boundaries.

  tools/venv/bin/python tools/overlay_xref.py fieldstg     # needs asm/<target>/ (scripts/build.sh)

One line per function in address order:
  ADDR SIZE J[jump tables: addr%8] R[other .rodata] D[.data it references] P[.data tables pointing at it]
  cl[callers in the overlay] =DUP (byte-identical to an earlier function, relocations masked)
Evidence, as in the EXE (DECISIONS "File boundaries come from evidence"): psylink packs objects at 4 bytes
and GCC aligns jump tables to 8 relative to the object's start, so a table whose address mod 8
differs from the previous table's starts a new object; .rodata and .data are each in link order, so
a function whose data comes before the previous function's data starts a new file; duplicated
static helpers sit at the top of their files. Lines starting with '|' mark rodata-parity breaks.
"""
import hashlib
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INS = re.compile(r"^\s*/\* \w+ ([0-9A-F]{8}) ([0-9A-F]{8}) \*/\s+(\S+)\s*(.*)")
SYM_ADDR = re.compile(r"(?:D|jtbl|func)_(?:[A-Z0-9_]+?_)?([0-9A-F]{8})$")


def addr_of_sym(s, known):
    if s in known:
        return known[s]
    m = SYM_ADDR.search(s)
    return int(m.group(1), 16) if m else None


def main():
    target = sys.argv[1] if len(sys.argv) > 1 else "fieldstg"
    asm = ROOT / "asm" / target
    if not (asm / "text.s").exists():
        sys.exit(f"{asm}/text.s missing: run scripts/build.sh")
    known = {}
    for p in (ROOT / "build/exe_symbols.txt", ROOT / f"config/{target}.symbols.txt"):
        if p.exists():
            for line in p.read_text().splitlines():
                m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", line)
                if m:
                    known[m.group(1)] = int(m.group(2), 16)

    # Functions (jlabel switch pieces folded into the function that owns them).
    funcs, cur = [], None
    for line in (asm / "text.s").read_text().splitlines():
        m = re.match(r"^(glabel|jlabel)\s+(\w+)", line)
        if m:
            if m.group(1) == "glabel" or cur is None:
                cur = dict(name=m.group(2), addr=None, end=None, refs=set(), calls=set(), words=[])
                funcs.append(cur)
            continue
        m = INS.match(line)
        if m and cur is not None:
            a = int(m.group(1), 16)
            cur["addr"] = cur["addr"] or a
            cur["end"] = a + 4
            op, args = m.group(3), m.group(4)
            w = int(m.group(2), 16)
            # mask immediates/targets of relocated instructions for duplicate detection
            if "%hi(" in args or "%lo(" in args:
                w &= 0xFFFF0000
            if op in ("jal", "j"):
                w &= 0xFC000000
            cur["words"].append(w)
            cur["refs"].update(re.findall(r"%(?:hi|lo)\(([A-Za-z_]\w*)", args))
            if op == "jal":
                cur["calls"].add(args.split()[0])
    for f in funcs:
        known.setdefault(f["name"], f["addr"])
    lo = min(f["addr"] for f in funcs)
    hi = max(f["end"] for f in funcs)

    # .rodata jump tables and other labels; .data labels and the function pointers stored in them.
    def labels(path):
        out, sym = [], None
        for line in path.read_text().splitlines():
            m = re.match(r"^dlabel\s+(\w+)", line)
            if m:
                sym = m.group(1)
                out.append((sym, []))
                continue
            m = re.search(r"\.word\s+([A-Za-z_]\w*)", line)
            if m and out:
                out[-1][1].append(m.group(1))
        return out
    rod = labels(asm / "data/rodata.rodata.s")
    dat = labels(asm / "data/data.data.s")
    base = addr_of_sym(rod[0][0], known) if rod else lo
    pointed_by = defaultdict(set)
    for sym, words in dat:
        for w in words:
            pointed_by[w].add(sym)

    callers = defaultdict(set)
    for f in funcs:
        for c in f["calls"]:
            callers[c].add(f["name"])
    seen = {}
    prev_parity = None
    for f in sorted(funcs, key=lambda f: f["addr"]):
        refs = [(addr_of_sym(s, known), s) for s in f["refs"]]
        jt = sorted(a for a, s in refs if s.startswith("jtbl_") and a is not None)
        ro = sorted(a for a, s in refs if a is not None and base <= a < lo and not s.startswith("jtbl_"))
        da = sorted(a for a, s in refs if a is not None and a >= hi and a < 0x80100000)
        pt = sorted(addr_of_sym(t, known) for t in pointed_by[f["name"]])
        cl = sorted(known[c] for c in callers[f["name"]] if c in known)
        h = hashlib.sha1(str(f["words"]).encode()).hexdigest()
        dup = f" =DUP({seen[h]:05X})" if h in seen and len(f["words"]) > 3 else ""
        seen.setdefault(h, f["addr"] & 0xFFFFF)
        mark = " "
        for a in jt:
            par = (a - base) % 8
            if prev_parity is not None and par != prev_parity:
                mark = "|"
            prev_parity = par
        fmt = lambda xs: ",".join(f"{x & 0xFFFFF:05X}" for x in xs)
        print(f"{mark}{f['addr'] & 0xFFFFF:05X} {f['end'] - f['addr']:5X}"
              f" J[{','.join(f'{a & 0xFFFFF:05X}%{(a - base) % 8}' for a in jt)}] R[{fmt(ro)}]"
              f" D[{fmt(da)}] P[{fmt(pt)}] cl[{fmt(cl)}]{dup}")


if __name__ == "__main__":
    main()
