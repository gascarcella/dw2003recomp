#!/usr/bin/env python3
"""Find functions that spimdisasm cut short at a switch case, and print the symbol lines that fix them.

spimdisasm sometimes takes a jump-table target for a function start: the piece shows up in the
split asm as `jlabel func_XXXXXXXX` (a "function" entered only through a jump table) right after
the real function's `endlabel`. Giving the real function its full size in the symbol file merges
the pieces back. This prints one line per affected function, in splat's symbol_addrs format:

  tools/venv/bin/python tools/split_case_sizes.py asm/fightstg/text.s [more.s ...]

Paste the lines into the target's symbol file (config/symbol_addrs.txt for the EXE,
config/<overlay>.symbols.txt for an overlay) and re-split.
"""
import re
import sys

START = re.compile(r"^(glabel|jlabel) (func_(?:[A-Z0-9]+_)?([0-9A-F]{8}))\b")
INSN = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ")


def scan(path):
    """Functions in file order: (kind, name, start, end) with end = last instruction + 4."""
    funcs, cur = [], None
    for line in open(path):
        m = START.match(line)
        if m:
            cur = [m.group(1), m.group(2), int(m.group(3), 16), None]
            funcs.append(cur)
            continue
        m = INSN.match(line)
        if m and cur:
            cur[3] = int(m.group(1), 16) + 4
    return funcs


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for path in sys.argv[1:]:
        owner = None
        merged = {}
        for kind, name, start, end in scan(path):
            if kind == "glabel":
                owner = (name, start)
            elif owner:  # a case piece: extend the function before it
                merged[owner] = end
        for (name, start), end in sorted(merged.items(), key=lambda kv: kv[0][1]):
            print(f"{name} = 0x{start:08X}; // type:func size:0x{end - start:X}")


if __name__ == "__main__":
    main()
