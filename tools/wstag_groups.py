#!/usr/bin/env python3
"""Group the WSTAG### stage overlays by code shape, and copy matched C between files that share a function.

  tools/venv/bin/python tools/wstag_groups.py              # every group of 2+ files, largest first
  tools/venv/bin/python tools/wstag_groups.py --all        # also the files with a unique shape
  tools/venv/bin/python tools/wstag_groups.py WSTAG219     # the group of one file, with its functions
  tools/venv/bin/python tools/wstag_groups.py --funcs      # functions shared by 2+ files (identical up to local names)
  tools/venv/bin/python tools/wstag_groups.py --add WSTAG290 ...          # make files C units (wstag_c.txt + splat)
  tools/venv/bin/python tools/wstag_groups.py --propagate [WSTAG290 ...]  # copy matched C to the other members
  tools/venv/bin/python tools/wstag_groups.py --rename FUNC SUFFIX        # name FUNC's --funcs group wstag###_SUFFIX
  options: --min N (groups of at least N files or functions), --names (each group's function names per member)

The shape of a file is its .text words (config/wstag.txt's .text and .data offsets, from the original
extracted/disc/AAA/PRO/WSTAG###.PRO) with every immediate masked except branch offsets: jal/j targets and the
16-bit immediates of I-type instructions (addresses, but also constants: file IDs, positions, flags). Opcodes,
registers and control flow stay, so members of a group have the same C up to constants and symbols. Each group
is split once more into variants: the same words with only the fields that splat's disassembly (asm/wstag###/)
relocates masked (%hi/%lo, jal/j), plus the external symbols referenced (EXE and FIELDSTG, in order). Members
of one variant have identical C up to their own local names. `(C)` marks files listed in config/wstag_c.txt
(they have src/wstag/wstag###.c).

--funcs groups single functions the same way as variants (relocation-masked words and external symbols): a
function decompiled once can be copied to every member, renaming its local symbols. `C n/m` counts the members
already in C (a definition in src/wstag/, not INCLUDE_ASM or NON_MATCHING).

--rename names every member of a --funcs group (FUNC is any member, by its current name) `wstag###_SUFFIX` in its
file: the C (src/wstag/wstag###*.c), mentions in include/ and docs/, and config/wstag/wstag###.symbols.txt. A
member whose file already has that name is skipped. Then `scripts/build.sh --check` (the symbol files changed).
Named functions are local symbols like func_WSTAG###_ ones (grouping, C counts); --propagate pairs named symbols
by name.

--add lists files in config/wstag_c.txt, writes their configs, runs splat on them (it writes src/wstag/<n>.c with
INCLUDE_ASM), regenerates build.ninja and builds their expected objects (for unit_diff). --propagate replaces each
top-level INCLUDE_ASM in the C units (or the ones named) whose function has a matched C member elsewhere with that
C, its local symbols renamed through the two functions' relocations (in order), and adds the externs and
prototypes it needs from the member's file. Check the result: tools/unit_diff.py wstag### -t wstag.
"""
import argparse
import hashlib
import re
import subprocess
import sys
import textwrap
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
import configure  # noqa: E402  (wstag_rows, wstag_c_names, write_wstag_configs, split)

WSTAG_SYMBOLS_FMT = configure.WSTAG_SYMBOLS

PRO = ROOT / "extracted/disc/AAA/PRO"
BASE = configure.TIER2_BASE
INSN = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ([0-9A-F]{8}) \*/\s+(\S+)\s*(.*)$")
RELOC = re.compile(r"%(?:hi|lo)\(([^)]+)\)")
FUNC = re.compile(r"^glabel (\S+)", re.M)
BRANCHES = (1, 4, 5, 6, 7)  # regimm, beq, bne, blez, bgtz: their offsets are the control flow
DECL = re.compile(r"^(?:extern\s|\w[\w\s*]*\b(?:func_WSTAG\d+_[0-9A-F]+|wstag\d+_\w+)\([^)]*\);)")
LOCAL = re.compile(r"\b(?:(?:func|D|jtbl)_WSTAG\d+_[0-9A-F]+|wstag\d+_\w+)\b")
SYMBOLS_HEADER = "// {N} (configure.py WSTAG_SYMBOLS): the file's own names, and sizes of the data the C reads inside of."


def file_of(sym: str) -> str:
    """"wstag201" from func_WSTAG201_800A5E2C, D_WSTAG201_..., wstag201_start."""
    return "wstag" + re.search(r"(?i)wstag(\d+)", sym).group(1)


def short(sym: str) -> str:
    """A function's name without its file: 800A5E2C, start."""
    return re.sub(r"^(?:func_WSTAG\d+_|wstag\d+_)", "", sym)


def asm_file(name: str) -> Path:
    """splat's disassembly of the file's .text: text.s (asm unit) or the C unit's full disassembly."""
    n = name.lower()
    for p in (ROOT / f"asm/{n}/{n}.s", ROOT / f"asm/{n}/text.s"):
        if p.exists():
            return p
    sys.exit(f"no disassembly for {name} in asm/{n}/: run configure.py")


def shape(name: str, text: int, data: int, per_func=None):
    """(shape hash, variant hash, function names) of one file. per_func: a dict that gets
    {function name: (variant hash of the function, size, its local symbol references in order)}."""
    raw = (PRO / f"{name}.PRO").read_bytes()[text:data]
    words = [int.from_bytes(raw[i:i + 4], "little") for i in range(0, len(raw), 4)]
    src = asm_file(name).read_text()
    masks, ext = {}, []
    func_ext, func_local, starts = defaultdict(list), defaultdict(list), []
    for ln in src.splitlines():
        g = FUNC.match(ln)
        if g:
            starts.append([g.group(1), None])
        m = INSN.match(ln)
        if not m:
            continue
        addr, op, args = int(m.group(1), 16), m.group(3), m.group(4)
        if starts and starts[-1][1] is None:
            starts[-1][1] = addr
        sym = None
        r = RELOC.search(args)
        if r:
            masks[addr] = 0xFFFF0000
            sym = r.group(1)
        elif op in ("jal", "j"):  # absolute targets: relocated even to a local label
            masks[addr] = 0xFC000000
            sym = None if args.startswith(".") else args
        if sym and f"_{name}_" not in sym and not sym.startswith(f"{name.lower()}_"):
            ext.append(sym.split(" ")[0])
            func_ext[starts[-1][0]].append(sym.split(" ")[0])
        elif sym:
            func_local[starts[-1][0]].append(sym)  # "D_WSTAG###_800A6360 + 0x2"
    if per_func is not None:
        ends = [a for _, a in starts[1:]] + [BASE + data]
        for (f, start), end in zip(starts, ends):
            fh = hashlib.sha1(repr(func_ext[f]).encode())
            for a in range(start, end, 4):
                w = words[(a - BASE - text) // 4]
                fh.update((w & masks.get(a, 0xFFFFFFFF)).to_bytes(4, "little"))
            per_func[f] = (fh.hexdigest(), end - start, func_local[f])
    h, v = hashlib.sha1(), hashlib.sha1(repr(ext).encode())
    for i, w in enumerate(words):
        op = w >> 26
        h.update((w & (0xFC000000 if op in (2, 3) else 0xFFFF0000 if op and op not in BRANCHES
                       else 0xFFFFFFFF)).to_bytes(4, "little"))
        v.update((w & masks.get(BASE + text + i * 4, 0xFFFFFFFF)).to_bytes(4, "little"))
    return h.hexdigest(), v.hexdigest(), FUNC.findall(src)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("file", nargs="?", help="show the group of this WSTAG### file")
    ap.add_argument("--all", action="store_true", help="also list files with a unique shape")
    ap.add_argument("--min", type=int, default=2, help="smallest group to list (default 2)")
    ap.add_argument("--names", action="store_true", help="list each member's functions")
    ap.add_argument("--funcs", action="store_true", help="group single functions instead of files")
    ap.add_argument("--add", nargs="+", metavar="WSTAG###", help="make these files C units")
    ap.add_argument("--propagate", nargs="*", metavar="WSTAG###", help="copy matched C to other members")
    ap.add_argument("--rename", nargs=2, metavar=("FUNC", "SUFFIX"), help="name FUNC's group wstag###_SUFFIX")
    args = ap.parse_args()
    if args.add:
        return add_files([n.upper() for n in args.add])

    in_c = set(configure.wstag_c_names())
    groups, info, per_func = defaultdict(list), {}, {}
    for name, size, text, data, entry in configure.wstag_rows():
        key, ext, funcs = shape(name, text, data, per_func)
        groups[key].append(name)
        info[name] = (ext, funcs, data - text)
    if args.funcs:
        return list_funcs(per_func, args.min)
    if args.propagate is not None:
        return propagate(per_func, [n.upper() for n in args.propagate] or sorted(in_c))
    if args.rename:
        return rename_group(per_func, *args.rename)
    order = sorted(groups.values(), key=lambda g: (-len(g), g[0]))
    print(f"# {len(info)} files, {len(groups)} shapes, {sum(len(g) == 1 for g in order)} unique; "
          f"{sum(len(g) for g in order if len(g) > 1)} files in {sum(len(g) > 1 for g in order)} groups")
    for i, g in enumerate(order, 1):
        if args.file and args.file.upper() not in g:
            continue
        if not args.file and len(g) < (1 if args.all else args.min):
            continue
        ext, funcs, size = info[g[0]]
        variants = defaultdict(list)
        for n in g:
            variants[info[n][0]].append(n)
        done = sum(n in in_c for n in g)
        print(f"G{i:03} {len(g):2} files  {len(funcs):2} functions  0x{size:04X} B .text  "
              f"{len(variants)} variant(s)  C {done}/{len(g)}")
        # One line per variant (files with identical words up to relocations); "|" separates variants.
        print(textwrap.fill(" | ".join(" ".join(n + ("(C)" if n in in_c else "") for n in members)
                                       for members in sorted(variants.values(), key=lambda m: (-len(m), m))),
                            116, initial_indent="      ", subsequent_indent="      "))
        if args.names or args.file:
            for n in g:
                print(f"      {n}: " + " ".join(short(f) for f in info[n][1]))


def c_functions():
    """Functions defined in C under src/wstag/ (outside INCLUDE_ASM; a NON_MATCHING definition counts as asm)."""
    done = set()
    for p in (ROOT / "src/wstag").glob("*.c"):
        text = re.sub(r"#ifdef NON_MATCHING.*?#else", "", p.read_text(), flags=re.S)
        done.update(re.findall(r"^\S.*\b(func_WSTAG\d+_[0-9A-F]+|wstag\d+_\w+)\(.*\)\s*\{\s*$", text, re.M))
    return done


def list_funcs(per_func, minimum):
    groups = defaultdict(list)
    for f, (key, size, _) in per_func.items():
        groups[key].append(f)
    done = c_functions()
    order = sorted(groups.values(), key=lambda g: (-len(g), g[0]))
    shared = [g for g in order if len(g) >= max(minimum, 2)]
    print(f"# {len(per_func)} functions, {len(groups)} distinct; {sum(len(g) for g in shared)} in "
          f"{len(shared)} groups of {max(minimum, 2)}+; {len(done)} in C")
    for i, g in enumerate(order, 1):
        if len(g) < max(minimum, 2):
            continue
        size = per_func[g[0]][1]
        print(f"F{i:03} {len(g):3} x 0x{size:03X} B  C {sum(f in done for f in g)}/{len(g)}  {g[0]}")
        print(textwrap.fill(" ".join(f.removeprefix("func_") + ("(C)" if f in done else "") for f in g[1:]), 116,
                            initial_indent="      ", subsequent_indent="      "))


def add_files(names):
    unknown = set(names) - set(configure.WSTAG_NAMES)
    if unknown:
        sys.exit(f"not WSTAG files: {' '.join(sorted(unknown))}")
    path = ROOT / configure.WSTAG_C
    lines = path.read_text().splitlines()
    have = set(configure.wstag_c_names())
    entries = [ln for ln in lines if ln.strip() and not ln.startswith("#")] + sorted(set(names) - have)
    path.write_text("\n".join([ln for ln in lines if ln.startswith("#")] + sorted(entries)) + "\n")
    configure.write_wstag_configs()
    for t in configure.WSTAG_TARGETS:
        if t.name.upper() in names:  # splat keeps an existing C file
            configure.split(t, quiet=True)
    subprocess.run([configure.PYTHON, "configure.py", "--no-split"], cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
    subprocess.run(["ninja"] + [f"build/expected/wstag/{n.lower()}.s.o" for n in names], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)


def split_ref(ref: str):
    """("D_WSTAG###_800A6360", 2) from a relocation's "D_WSTAG###_800A6360 + 0x2"."""
    m = re.match(r"^(\w+)(?:\s*([+-])\s*(0x[0-9A-Fa-f]+|\d+))?$", ref.strip())
    if not m:
        sys.exit(f"can't parse relocation {ref!r}")
    off = int(m.group(3), 0) if m.group(3) else 0
    return m.group(1), -off if m.group(2) == "-" else off


def c_definition(text: str, func: str):
    """The C definition of func in text (signature line to the closing brace), or None."""
    m = re.search(rf"^\S[^\n]*\b{func}\([^\n]*\)\s*\{{\s*\n.*?^\}}\n", text, re.M | re.S)
    return m.group(0) if m else None


def stage_comment(name: str) -> str:
    for line in (ROOT / configure.WSTAG_TABLE).read_text().splitlines():
        f = line.split()
        if f and f[0] == name:
            table = {"A884": "fieldstg_stages", "A5F0": "fieldstg_stages_2d"}[f[4]]
            return f"/* {name}: stage {f[5]} ({table}). */\n"
    return ""


def symbol_sizes(name: str):
    """{symbol: line} of a file's WSTAG_SYMBOLS entries that have a size."""
    path = ROOT / configure.WSTAG_SYMBOLS.format(n=name.lower())
    out = {}
    for ln in path.read_text().splitlines() if path.exists() else []:
        m = re.match(r"^(\w+) = 0x[0-9A-Fa-f]+;.*\bsize:", ln)
        if m:
            out[m.group(1)] = ln
    return out


def add_sizes(name: str, syms):
    """Give a file's data symbols a size as their counterparts have one (the C reads inside them, e.g. a table's
    second half-word, which splat would otherwise label): up to the next label at least 4 bytes further."""
    path = ROOT / configure.WSTAG_SYMBOLS.format(n=name.lower())
    have = symbol_sizes(name)
    data = (ROOT / f"asm/{name.lower()}/data/data.data.s").read_text()
    labels = sorted(int(a, 16) for a in re.findall(rf"^dlabel D_{name}_([0-9A-F]+)$", data, re.M))
    new = []
    for sym in syms:
        if sym in have:
            continue
        addr = int(sym[-8:], 16)
        end = next((a for a in labels if a >= addr + 4), None)
        if end is None:
            sys.exit(f"{sym}: no label after it to size it by")
        new.append(f"{sym} = 0x{addr:08X}; // size:0x{end - addr:X}")
    if not new:
        return False
    old = path.read_text().splitlines() if path.exists() else [
        f"// {name} (configure.py WSTAG_SYMBOLS): sizes of the data the C reads inside of."]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(old + new) + "\n")
    return True


def propagate(per_func, files):
    done = c_functions()
    groups = defaultdict(list)
    for f, (key, _, _) in per_func.items():
        groups[key].append(f)
    resplit = []
    for name in files:
        n = name.lower()
        path = ROOT / f"src/wstag/{n}.c"
        if not path.exists():
            print(f"{name}: no src/wstag/{n}.c (--add it first)")
            continue
        text = path.read_text()
        stub = '#include "common.h"\n'
        if text.startswith(stub):
            text = '#include "wstag.h"\n\n' + stage_comment(name) + text[len(stub):]
        filled = []
        for target in re.findall(rf'^INCLUDE_ASM\("asm/{n}/nonmatchings/{n}", (\w+)\);$', text, re.M):
            if f'#else\nINCLUDE_ASM("asm/{n}/nonmatchings/{n}", {target});' in text:
                continue  # a NON_MATCHING WIP
            key, _, target_syms = per_func[target]
            refs = [f for f in groups[key] if f in done and f != target]
            if not refs:
                continue
            ref = refs[0]
            m_text = (ROOT / f"src/wstag/{file_of(ref)}.c").read_text()
            body = c_definition(m_text, ref)
            ren = {ref: target}
            # Pair the two functions' references by address: "sym + off" in one may be a label of its own in
            # the other (a symbol with a size in one file only).
            for a, b in zip(per_func[ref][2], target_syms):
                base_a, off_a = split_ref(a)
                base_b, off_b = split_ref(b)
                if re.search(r"_[0-9A-F]{8}$", base_b):
                    mapped = f"{base_b[:-8]}{int(base_b[-8:], 16) + off_b - off_a:08X}"
                elif off_a == off_b:
                    mapped = base_b  # a named symbol
                else:
                    sys.exit(f"{target}: {b} against {a}: a named symbol at another offset")
                if ren.setdefault(base_a, mapped) != mapped:
                    sys.exit(f"{target}: inconsistent symbols against {ref} ({a})")
            used = sorted(set(LOCAL.findall(body)) - {ref})
            if any(u not in ren for u in used):
                print(f"{target}: {ref} uses symbols its asm doesn't reference; skipped")
                continue
            rename = lambda s: LOCAL.sub(lambda mm: ren.get(mm.group(0), mm.group(0)), s)  # noqa: E731
            body = rename(body)
            decls = []  # what the body needs: the reference file's extern/prototype lines, renamed
            for u in used:
                if any(DECL.match(ln) and re.search(rf"\b{ren[u]}\b", ln) for ln in text.splitlines()):
                    continue  # already declared
                own = c_definition(text, ren[u]) if u.startswith("func_") else None
                if own and text.index(own) < text.index(f"{target});"):
                    continue  # defined above
                if u.startswith("func_") and per_func[u][0] != per_func[ren[u]][0]:
                    decls.append(f"void {ren[u]}();")  # a different function: its type is unknown yet
                    continue
                line = next((ln for ln in m_text.splitlines() if DECL.match(ln) and re.search(rf"\b{u}\b", ln)
                             and ln.rstrip().endswith(";")), None)
                if line is None and u.startswith("func_"):
                    line = c_definition(m_text, u).splitlines()[0].rstrip(" {") + ";"
                if line is None:
                    sys.exit(f"{target}: no declaration of {u} in the file of {ref}")
                decls.append(rename(line))
            sized = symbol_sizes(file_of(ref).upper())
            if add_sizes(name, [ren[u] for u in used if u in sized]) and name not in resplit:
                resplit.append(name)
            text = text.replace(f'INCLUDE_ASM("asm/{n}/nonmatchings/{n}", {target});\n', body)
            if decls:
                text = insert_decls(text, decls)
            filled.append(f"{target.removeprefix('func_')} (from {ref.removeprefix('func_')})")
        path.write_text(text)
        print(f"{name}: " + (", ".join(filled) if filled else "nothing to copy"))
    if resplit:  # new symbol sizes: split those files again (splat keeps the C files)
        print(f"symbol sizes added, splitting again: {' '.join(resplit)}")
        add_files(resplit)


def name_function(old: str, new: str) -> bool:
    """Rename one WSTAG function in its C file(s), include/, docs/, and add it to the file's symbol file."""
    n = file_of(old)
    paths = sorted((ROOT / "src/wstag").glob(f"{n}*.c")) + sorted((ROOT / "include").rglob("*.h")) \
        + sorted((ROOT / "docs").glob("*.md"))
    texts = {p: p.read_text() for p in paths}
    if any(re.search(rf"\b{new}\b", t) for p, t in texts.items() if p.parent.name == "wstag"):
        print(f"{old}: {new} exists; skipped")
        return False
    for p, t in texts.items():
        t2 = re.sub(rf"\b{old}\b", new, t)
        if t2 != t:
            p.write_text(t2)
    sym = ROOT / WSTAG_SYMBOLS_FMT.format(n=n)
    lines = sym.read_text().rstrip("\n").split("\n") if sym.exists() else [SYMBOLS_HEADER.format(N=n.upper())]
    if re.search(r"_[0-9A-F]{8}$", old):
        lines.append(f"{new} = 0x{int(old[-8:], 16):08X}; // type:func")
    else:  # renaming a named function: replace its line
        lines = [re.sub(rf"^{old} =", f"{new} =", ln) for ln in lines]
    head, entries = lines[0], [ln for ln in lines[1:] if ln.strip()]
    entries.sort(key=lambda ln: int(re.search(r"= 0x([0-9A-Fa-f]+);", ln).group(1), 16))
    sym.parent.mkdir(parents=True, exist_ok=True)
    sym.write_text("\n".join([head] + entries) + "\n")
    return True


def rename_group(per_func, func: str, suffix: str):
    groups = defaultdict(list)
    for f, (key, _, _) in per_func.items():
        groups[key].append(f)
    if func not in per_func:
        sys.exit(f"{func}: not a WSTAG function (splat's names: run configure.py after renaming)")
    members = groups[per_func[func][0]]
    done = sum(name_function(f, f"{file_of(f)}_{suffix}") for f in members if f != f"{file_of(f)}_{suffix}")
    print(f"{done} of {len(members)} named {suffix}; now: scripts/build.sh --check")


def insert_decls(text: str, decls):
    """Add declarations to the block before the first function: externs by address, then prototypes."""
    lines = text.split("\n")
    first = next(i for i, ln in enumerate(lines) if ln.startswith(("INCLUDE_ASM", "#ifdef NON_MATCHING"))
                 or re.match(r"^\w.*\)\s*\{$", ln))
    start = next((i for i in range(first) if DECL.match(lines[i])), first)
    block = [ln for ln in lines[start:first] if ln.strip()] + decls

    def key(ln):
        syms = LOCAL.findall(ln)
        return (not ln.startswith("extern"), syms[-1][-8:] if syms else ln)
    block = sorted(dict.fromkeys(block), key=key)
    return "\n".join(lines[:start] + block + [""] + lines[first:])


if __name__ == "__main__":
    main()
