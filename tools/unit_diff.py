#!/usr/bin/env python3
"""Compile one C unit on its own and score it per function against the original, without ninja.

Compiles src/<target>/<unit>.c the way build.ninja does (compiler and -G from configure.py's
CC_OVERRIDES/G_OVERRIDES, via tools/cc_psx.sh) into build/unit_diff/, then runs objdiff against
build/expected/<target>/<unit>.s.o (made by ninja; `scripts/build.sh` once). Safe to run while
other sessions edit other units: it touches no ninja state. Also compares .rodata/.data/.sdata word by
word with relocations resolved (jump tables, string literals), which objdiff's section score can't.
A function objdiff scores below 100% only because a relocation names another symbol for the same address
(`D_80042BCA` vs `records_techniques - 18`, a label vs its struct + offset) is listed as "OK (by address)": its
instructions match once every relocation is resolved to an address (EXE symbols, symbol files, splat's
undefined_*_auto lists, names like D_8004xxxx; the object's own sections from the last link's map) or else
to the symbol + offset it falls in.

  tools/venv/bin/python tools/unit_diff.py heap                 # every function's match %
  tools/venv/bin/python tools/unit_diff.py heap func_80017FDC   # + side-by-side instruction diff
  options: -G N / -V VER to override the file's -G or compiler; --all to list INCLUDE_ASM ones too;
           -D NON_MATCHING (repeatable) to score the WIP C instead of its INCLUDE_ASM
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
import configure  # noqa: E402  (CC_OVERRIDES, g_for, DEFAULT_CC, CC_INCLUDES)

OBJDUMP = ROOT / "tools/binutils/bin/mipsel-linux-gnu-objdump"


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)


NAME_ADDR = re.compile(r"^(?:D|func|jtbl)_(?:[A-Z0-9]+_)?([0-9A-F]{8})$")  # splat's default names


def known_addresses(target: str) -> dict:
    """Symbol name -> address: the EXE's linked symbols, the symbol files and splat's undefined_*_auto lists.
    Names given to more than one address (static helpers copied per file) are left out."""
    seen = {}
    for f in ("build/main/SLES_039.36.syms.ld", "config/symbol_addrs.txt", f"config/{target}.symbols.txt",
              f"build/{target}/undefined_syms_auto.txt", f"build/{target}/undefined_funcs_auto.txt"):
        path = ROOT / f
        if not path.exists():
            continue
        for ln in path.read_text().splitlines():
            m = re.match(r"\s*([A-Za-z_.$][\w.$]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", ln)
            if m:
                seen.setdefault(m.group(1), set()).add(int(m.group(2), 16))
    return {name: addrs.pop() for name, addrs in seen.items() if len(addrs) == 1}


def section_bases(target: str, unit: str) -> dict:
    """Where the last link put this unit's sections (build/<target>/*.map), so that references into the
    object's own sections (`.rodata` + 0x40 in C, jtbl_80012345 in asm) compare as addresses."""
    bases = {}
    for path in sorted((ROOT / f"build/{target}").glob("*.map")):
        for ln in path.read_text(errors="replace").splitlines():
            m = re.match(r"^ (\.\w+)\s+0x([0-9a-f]+)\s+0x[0-9a-f]+ \S*/" + re.escape(unit) + r"\.[cs]\.o$", ln)
            if m and int(m.group(2), 16):
                bases.setdefault(m.group(1), int(m.group(2), 16))
    return bases


class ObjSymbols:
    """An object's sections and defined symbols (readelf); `bases`: the sections' addresses, when known."""

    def __init__(self, obj: Path, bases: dict = None):
        self.bases = bases or {}
        readelf = str(OBJDUMP).replace("objdump", "readelf")
        secnames = {}
        for ln in run([readelf, "-SW", str(obj)]).stdout.splitlines():
            m = re.match(r"^\s*\[\s*(\d+)\] (\S+)", ln)
            if m:
                secnames[m.group(1)] = m.group(2)
        self.sections = set(secnames.values())
        self.syms = {name: (name, 0) for name in self.sections}  # name -> (section, offset)
        self.by_sec = {}  # section -> [(offset, name)], named symbols only
        for ln in run([readelf, "-sW", str(obj)]).stdout.splitlines():
            f = ln.split()
            if len(f) == 8 and f[6].isdigit():
                sec, val = secnames.get(f[6], f[6]), int(f[1], 16)
                self.syms[f[7]] = (sec, val)
                if f[3] != "SECTION" and not f[7].endswith(".NON_MATCHING") and not f[7].startswith("."):
                    self.by_sec.setdefault(sec, []).append((val, f[7]))
        for v in self.by_sec.values():
            v.sort()

    def defined(self, name: str) -> bool:
        return name in self.syms and self.syms[name][0] != name or name in self.sections

    def resolve(self, addrs: dict, sym: str, off: int, depth: int = 0) -> str:
        """A relocation target: its address when that can be known, else symbol + offset."""
        if sym in addrs:
            return f"0x{addrs[sym] + off:08X}"
        m = NAME_ADDR.match(sym)
        if m:
            return f"0x{int(m.group(1), 16) + off:08X}"
        if self.defined(sym) and depth < 2:
            sec, base = self.syms[sym]
            pos = base + off
            if sec in self.bases:
                return f"0x{self.bases[sec] + pos:08X}"
            inside = [(v, n) for v, n in self.by_sec.get(sec, []) if v <= pos]
            if inside and inside[-1][1] != sym:
                v, n = inside[-1]
                return self.resolve(addrs, n, pos - v, depth + 1)
        return f"{sym}+0x{off:X}"


def _s16(x: int) -> int:
    return x - 0x10000 if x & 0x8000 else x


def disasm(obj: Path, func: str, addrs: dict = None, osyms: "ObjSymbols" = None):
    """Instructions of `func` in `obj` as (text, key): text shows each relocation's symbol, key has it resolved
    (ObjSymbols.resolve) and branch targets relative to the function, so equal keys mean equal linked code."""
    r = run([str(OBJDUMP), "-drz", "-m", "mips:3000", f"--disassemble={func}", str(obj)])
    insns = []  # [offset, word, asm, [(type, symbol)]]
    for ln in r.stdout.splitlines():
        m = re.match(r"^\t+[0-9a-f]+: (R_MIPS_\w+)\t(\S+)$", ln)
        if m and insns:
            insns[-1][3].append((m.group(1), m.group(2)))
            continue
        m = re.match(r"^\s*([0-9a-f]+):\t([0-9a-f]{8}) \t(.*)$", ln)
        if m:
            insns.append([int(m.group(1), 16), int(m.group(2), 16),
                          " ".join(p.strip() for p in m.group(3).split("\t")), []])
    start = insns[0][0] if insns else 0
    text_base = osyms.bases.get(".text") if osyms else None

    def target(m):  # a branch target: its address, or its offset in the function, not the label objdump picks
        t = int(m.group(1), 16)
        return f"<0x{text_base + t:08X}>" if text_base is not None else f"<+0x{t - start:x}>"

    out, last_hi = [], {}
    for i, (_, word, asm, relocs) in enumerate(insns):
        text = asm + "".join(f"  ; {t[7:]} {sym}" for t, sym in relocs)
        key = re.sub(r"\b([0-9a-f]+) <[^>]*>", target, asm)
        if relocs and addrs is not None:
            typ, sym = relocs[0]
            imm = word & 0xFFFF
            if typ == "R_MIPS_HI16":
                last_hi[sym] = imm
                lo = next((w & 0xFFFF for _, w, _, rl in insns[i + 1:] if ("R_MIPS_LO16", sym) in rl), 0)
                ahl = (imm << 16) + _s16(lo)
            elif typ == "R_MIPS_LO16":
                ahl = (last_hi.get(sym, 0) << 16) + _s16(imm)
            elif typ == "R_MIPS_26":
                ahl = (word & 0x3FFFFFF) << 2
            elif typ == "R_MIPS_PC16":  # the branch goes to S + A + 4
                ahl = (_s16(imm) << 2) + 4
            else:  # R_MIPS_GPREL16 and the like
                ahl = _s16(imm)
            where = osyms.resolve(addrs, sym, ahl)
            if typ == "R_MIPS_PC16":  # as a branch without a relocation
                key = re.sub(r"\b([0-9a-f]+) <[^>]*>", lambda m: f"<{where}>", asm)
            else:
                mnemonic, _, ops = asm.partition(" ")
                regs = [o for o in re.split(r"[,()]", ops) if o and not re.match(r"^-?(0x[0-9a-f]+|\d+)$|.*<", o)]
                key = f"{mnemonic} {','.join(regs)} {typ[7:]}({where})"
        out.append((text, key))
    return out


def section_words(obj: Path, sec: str, addrs: dict):
    """`sec`'s bytes as 4-byte words, each R_MIPS_32 resolved to (target section, offset), or None if absent.

    asm objects relocate jump tables against their `.L` labels, C objects against `.text` + an in-place
    addend, so raw bytes and objdiff's section score differ even when the linked output is identical."""
    r = run([str(OBJDUMP), "-s", "-j", sec, str(obj)])
    if f"Contents of section {sec}:" not in r.stdout:
        return None
    data = bytearray()
    for ln in r.stdout.split(f"Contents of section {sec}:")[1].splitlines()[1:]:
        m = re.match(r"^ [0-9a-f]+ ((?:[0-9a-f]{2,8} ?){1,4})", ln)
        if not m:
            break
        data += bytes.fromhex(m.group(1).replace(" ", ""))
    osyms = ObjSymbols(obj)
    relocs = {}
    for ln in run([str(OBJDUMP), "-r", "-j", sec, str(obj)]).stdout.splitlines():
        m = re.match(r"^([0-9a-f]{8}) (R_MIPS_\w+)\s+(\S+)$", ln)
        if m:
            relocs[int(m.group(1), 16)] = (m.group(2), m.group(3))
    words = []
    for off in range(0, len(data), 4):
        w = int.from_bytes(data[off:off + 4].ljust(4, b"\0"), "little")
        if off in relocs:
            typ, sym = relocs[off]
            if osyms.defined(sym):
                base_sec, val = osyms.syms[sym]
                words.append((typ, base_sec, val + w))
            else:  # an extern: by address when it is known
                words.append((typ, osyms.resolve(addrs, sym, w)))
        else:
            words.append(w)
    return words


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("unit")
    ap.add_argument("func", nargs="?")
    ap.add_argument("-t", "--target", default="main")
    ap.add_argument("-G", type=int)
    ap.add_argument("-V")
    ap.add_argument("--all", action="store_true", help="also list functions still in INCLUDE_ASM")
    ap.add_argument("-D", action="append", default=[], metavar="DEF", help="extra preprocessor define")
    args = ap.parse_args()

    src = f"src/{args.target}/{args.unit}.c"
    expected = ROOT / f"build/expected/{args.target}/{args.unit}.s.o"
    if not expected.exists():
        sys.exit(f"{expected} missing: run scripts/build.sh once")
    out = ROOT / f"build/unit_diff/{args.target}/{args.unit}.c.o"
    out.parent.mkdir(parents=True, exist_ok=True)
    cc = args.V or configure.CC_OVERRIDES.get(src, configure.DEFAULT_CC)
    g = args.G if args.G is not None else configure.g_for(src)
    r = run(["tools/cc_psx.sh", "-V", cc, "-G", str(g), *configure.cc_flags(src), *(f"-D{d}" for d in args.D),
             *configure.CC_INCLUDES.split(),
             src, "-o", str(out)])
    if r.returncode != 0:
        sys.exit(r.stdout + r.stderr)

    r = run([str(ROOT / "tools/bin/objdiff-cli"), "diff", "-1", str(expected), "-2", str(out), "-o", "-"])
    if r.returncode != 0:
        sys.exit(r.stderr)
    d = json.loads(r.stdout)
    funcs = [s for s in d["left"]["symbols"] if s.get("kind") == "SYMBOL_FUNCTION"]
    done = [s for s in funcs if s.get("match_percent") is not None]
    addrs = known_addresses(args.target)
    bases = section_bases(args.target, args.unit)
    syms_a, syms_b = ObjSymbols(expected, bases), ObjSymbols(out, bases)

    def same_by_address(name):
        a, b = disasm(expected, name, addrs, syms_a), disasm(out, name, addrs, syms_b)
        return bool(a) and [k for _, k in a] == [k for _, k in b]

    ok = by_addr = 0
    for s in funcs:
        pct = s.get("match_percent")
        if pct is None:
            mark = "asm"
        elif pct == 100.0:
            mark, ok = "OK", ok + 1
        elif same_by_address(s["name"]):
            mark, ok, by_addr = "OK (by address)", ok + 1, by_addr + 1
        else:
            mark = f"{pct:.2f}%"
        if pct is not None or args.all:
            print(f"  {s['name']:24} {mark}")
    print(f"{args.unit} (gcc {cc}, -G{g}): {ok}/{len(done)} C functions match"
          + (f" ({by_addr} by address)" if by_addr else "") + f", {len(funcs) - len(done)} still asm")

    def as_addr(w):
        """A word as the value the link gives it, when known: splat leaves pointers it has no label for as
        plain words (string tables), where C has relocations against its own sections."""
        if isinstance(w, tuple) and len(w) == 3 and w[1] in bases:
            return bases[w[1]] + w[2]
        if isinstance(w, tuple) and len(w) == 2 and str(w[1]).startswith("0x"):
            return int(w[1], 16)
        return w

    for sec in (".rodata", ".data", ".sdata"):
        a, b = section_words(expected, sec, addrs), section_words(out, sec, addrs)
        if a is None and b is None:
            continue
        if a == b:
            print(f"  {sec:24} OK ({len(a) * 4} bytes)")
        elif a and b and [as_addr(w) for w in a] == [as_addr(w) for w in b]:
            print(f"  {sec:24} OK by address ({len(a) * 4} bytes)")
        else:
            a, b = a or [], b or []
            first = next((i for i in range(max(len(a), len(b)))
                          if i >= len(a) or i >= len(b) or a[i] != b[i]), None)
            print(f"  {sec:24} DIFFERS: {len(a) * 4} vs {len(b) * 4} bytes, first difference at 0x{first * 4:X}")

    if args.func:
        a, b = disasm(expected, args.func, addrs, syms_a), disasm(out, args.func, addrs, syms_b)
        w = max([len(x) for x, _ in a] + [10])
        print(f"\n{'original':{w}}   C   (| differs; ~ same address, other symbol)")
        for i in range(max(len(a), len(b))):
            x, kx = a[i] if i < len(a) else ("", None)
            y, ky = b[i] if i < len(b) else ("", None)
            print(f"{x:{w}} {'  ' if x == y else ' ~' if kx == ky else ' |'} {y}")


if __name__ == "__main__":
    main()
