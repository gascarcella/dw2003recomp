#!/usr/bin/env python3
"""Print the bytes of an EXE data symbol as a C initializer (for moving .data into C files).

  tools/venv/bin/python tools/data_to_c.py filetable_lba u32            # whole symbol, ELF size
  tools/venv/bin/python tools/data_to_c.py 0x800474A4 u16 -n 2382 -w 12 # address, count, per line
  tools/venv/bin/python tools/data_to_c.py filetable_funcs ptr          # pointers become names
  tools/venv/bin/python tools/data_to_c.py D_8005CCC0 str               # NUL-terminated strings
  tools/venv/bin/python tools/data_to_c.py -t shocktst D_SHOCKTST_80084AE8 ptr --deref   # an overlay

Types: u8 s8 u16 s16 u32 s32 (hex, or decimal with --dec), ptr (a word: 0, a symbol name, or
`sym + 0xN`; with --deref, the strings the words point at), str (strings, one per line), or a
struct as a comma list of those, `sz` being a pointer to a string (e.g. `sz,sz,s32,s32`): one
`{ ... },` per element (no padding between fields is assumed: give it as a field).
Reads the original EXE (extracted/disc/SLES_039.36) or, with -t, the overlay's file; names come
from the linked ELFs (build/main/SLES_039.36.elf, and build/<t>/<T>.PRO.elf for overlay
addresses, so build first). Only the values are printed; the declaration around them is up to you.
"""
import argparse
import bisect
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXE = ROOT / "extracted/disc/SLES_039.36"
ELF = ROOT / "build/main/SLES_039.36.elf"
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
VRAM, ROM = 0x80010000, 0x800
FMT = {"u8": "<B", "s8": "<b", "u16": "<H", "s16": "<h", "u32": "<I", "s32": "<i", "ptr": "<I"}


OVERLAY_BASE = 0x80082CB0


def symbols(elves=((ELF, 0, 1 << 32),)):
    """Names from each (elf, lo, hi): its symbols in [lo, hi) (an overlay's ELF also has the EXE's
    names, as absolute symbols without sizes, so those come from the EXE's ELF)."""
    by_name, by_addr = {}, []
    for elf, lo, hi in elves:
        out = subprocess.run([str(NM), "-S", "-n", str(elf)], capture_output=True, text=True, check=True).stdout
        for line in out.splitlines():
            p = line.split()
            if len(p) < 3 or p[-1].endswith(".NON_MATCHING") or p[-1].endswith(("_START", "_END", "_SIZE")):
                continue
            addr = int(p[0], 16)
            if not lo <= addr < hi:
                continue
            size = int(p[1], 16) if len(p) == 4 else 0
            by_name[p[-1]] = (addr, size)
            if p[-2] in "TtDdBbRrA" and addr >= VRAM:
                by_addr.append((addr, size, p[-1]))
    by_addr.sort()
    return by_name, by_addr


def c_string(s: bytes) -> str:
    """A C string literal: both bytes of a Shift-JIS character as hex escapes, ASCII as is; a hex escape
    followed by a hex digit is split ("\\x82" "A")."""
    out, esc, i = "", False, 0
    while i < len(s):
        c = s[i]
        if 0x81 <= c <= 0x9F or 0xE0 <= c <= 0xFC:  # a Shift-JIS lead byte: escape the pair
            out += "".join(f"\\x{b:02X}" for b in s[i:i + 2])
            esc, i = True, i + 2
            continue
        if 32 <= c < 127 and c not in b'"\\':
            out += ('" "' if esc and chr(c) in "0123456789abcdefABCDEF" else "") + chr(c)
            esc = False
        else:
            out += {10: "\\n", 9: "\\t", 34: '\\"', 92: "\\\\"}.get(c, f"\\x{c:02X}")
            esc = c not in (10, 9, 34, 92)
        i += 1
    return f'"{out}"'


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("what", help="symbol name or 0xADDRESS")
    ap.add_argument("type", help="u8 s8 u16 s16 u32 s32 ptr str, or a struct: a comma list (also sz)")
    ap.add_argument("-n", "--count", type=lambda x: int(x, 0), help="number of elements (default: the symbol's size)")
    ap.add_argument("-w", "--width", type=int, default=8, help="elements per line")
    ap.add_argument("--dec", action="store_true", help="decimal instead of hex")
    ap.add_argument("-t", "--target", help="a tier-1 overlay (e.g. shocktst) instead of the EXE")
    ap.add_argument("--deref", action="store_true", help="ptr: print the strings the words point at")
    args = ap.parse_args()

    exe, base, rom = EXE, VRAM, ROM
    if args.target:
        import yaml
        t = args.target.lower()
        cfg = yaml.safe_load((ROOT / f"config/{t}.yaml").read_text())
        opts = cfg["options"]
        exe = (ROOT / "config" / opts["base_path"] / opts["target_path"]).resolve()
        # the segment's vram: OVERLAY_BASE for tier-1 overlays, TIER2_BASE for WFIGHTMN/WFIGHTTS
        base, rom = cfg["segments"][0].get("vram", OVERLAY_BASE), 0
        by_name, by_addr = symbols(((ELF, 0, OVERLAY_BASE), (ROOT / opts["elf_path"], base, 1 << 32)))
    else:
        by_name, by_addr = symbols()
    if args.what.startswith("0x"):
        addr, size = int(args.what, 16), 0
    elif args.what in by_name:
        addr, size = by_name[args.what]
    else:
        sys.exit(f"unknown symbol {args.what}")
    data = exe.read_bytes()
    off = addr - base + rom

    if args.type == "str":
        end = off + size if size else off + 4096
        for s in data[off:end].split(b"\0"):
            if s:
                print(c_string(s) + ",")
        return

    fields = args.type.split(",")
    if any(f not in FMT and not (f == "sz" and len(fields) > 1) for f in fields):
        sys.exit(f"unknown type {args.type}")
    esize = sum(struct.calcsize(FMT.get(f, "<I")) for f in fields)
    count = args.count or (size // esize)
    if not count:
        sys.exit("no size: pass -n")
    starts = [a for a, _, _ in by_addr]

    def name_of(v):
        if v == 0:
            return "0"
        i = bisect.bisect_right(starts, v) - 1
        if i >= 0:
            a, s, n = by_addr[i]
            if v == a:
                return n
            # no size (an asm label): up to the next symbol
            end = a + s if s else (starts[i + 1] if i + 1 < len(starts) else a + 1)
            if v < end:
                return f"(u8 *)&{n} + 0x{v - a:X}"
        return f"0x{v:08X}"

    def c_str_at(v):
        if base <= v < base + len(data):
            return c_string(data[v - base + rom:data.index(b"\0", v - base + rom)])
        return name_of(v)

    if len(fields) > 1:
        pos = off
        for _ in range(count):
            items = []
            for f in fields:
                v = struct.unpack_from(FMT.get(f, "<I"), data, pos)[0]
                pos += struct.calcsize(FMT.get(f, "<I"))
                if f == "sz":
                    items.append(c_str_at(v) if v else "NULL")
                elif f == "ptr":
                    items.append(name_of(v))
                elif args.dec:
                    items.append(str(v))
                else:
                    items.append(str(v) if -10 < v < 10 else f"-0x{-v:X}" if v < 0 else f"0x{v:X}")
            print("    { " + ", ".join(items) + " },")
        return

    vals = [struct.unpack_from(FMT[args.type], data, off + i * esize)[0] for i in range(count)]
    if args.type == "ptr" and args.deref:
        items = [c_str_at(v) for v in vals]
    elif args.type == "ptr":
        items = [name_of(v) for v in vals]
    elif args.dec:
        items = [str(v) for v in vals]
    else:
        digits = esize * 2
        items = [(f"-0x{-v:X}" if v < 0 else f"0x{v:0{digits}X}") for v in vals]
    for i in range(0, len(items), args.width):
        print("    " + ", ".join(items[i:i + args.width]) + ",")


if __name__ == "__main__":
    main()
