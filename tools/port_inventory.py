#!/usr/bin/env python3
"""PC-port inventory for this game: psxstack's tools/port_inventory.py (the probe, the link check, the counts) configured
for this tree, plus the game's own commands (docs/PORT.md "Compiling the game C for the host").

  tools/venv/bin/python tools/port_inventory.py counts                 # the inventory's numbers on this tree
  tools/venv/bin/python tools/port_inventory.py counts --sites KIND    # file:line of every site of one kind
  tools/venv/bin/python tools/port_inventory.py probe [FILES...]       # host-compile gate at -m64 (exit 0 = clean)
  tools/venv/bin/python tools/port_inventory.py probe --warnings       # also count the non-gating -Wall warnings
  tools/venv/bin/python tools/port_inventory.py probe --m32            # the same at -m32 (compile only)
  tools/venv/bin/python tools/port_inventory.py probe --target windows # the same with llvm-mingw's clang (Windows x86_64)
  tools/venv/bin/python tools/port_inventory.py link                   # probe, then nm: duplicate / undefined globals
  tools/venv/bin/python tools/port_inventory.py structs                # sizeof every typedef'd struct, -m32 and -m64
  tools/venv/bin/python tools/port_inventory.py object-sizes           # literal object/data sizes (exit 0 = none)

The probe's objects go to build/port_inventory/<width>/ (its override headers to build/port_inventory/include/);
psxstack/tools/port_inventory.py's docstring describes the commands and the site kinds. `structs` (sizeof of every
typedef'd struct at -m32/-m64 against the `/* size 0xNN */` comments of include/) and `object-sizes`
(object_new/object_create calls with a bare literal size, FINDINGS 9d) are this game's.
"""
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PSXSTACK = ROOT / "psxstack"   # the stack: the submodule (scripts/worktree_init.sh checks it out)
MAIN = Path(subprocess.run(["git", "-C", str(ROOT), "rev-parse", "--path-format=absolute", "--git-common-dir"],
                           capture_output=True, text=True).stdout.strip() or str(ROOT / ".git")).parent
sys.path.insert(0, str(PSXSTACK / "tools"))
import port_inventory as inv  # noqa: E402
from port_inventory import (INT_LITERAL, PROBE_FLAGS, STACK_ROOT, Source, c_files, check_width,  # noqa: E402,F401
                            gcc_major, gcc_version, git_head, headers, strip_code, write_overrides)


def module_of(rel):
    parts = rel.split("/")
    if parts[0] == "include":
        return "include/" + parts[-1]
    return Path(parts[-1]).stem if parts[1] == "main" else parts[1]


CFG = inv.configure(
    root=ROOT, game_json=ROOT / "port/game/game.json",
    sources=lambda: sorted((ROOT / "src").rglob("*.c")),
    headers=lambda: sorted(p for p in (ROOT / "include").rglob("*.h") if "asm_generated" not in p.parts),
    include_dirs=[ROOT / "include", ROOT],
    symbol_files=[ROOT / "config" / "symbol_addrs.txt"] + sorted((ROOT / "config").rglob("*.symbols.txt")),
    module_of=module_of, gtemac=ROOT / "include/psyq/gtemac.h", include_asm=ROOT / "include/asm_generated/include_asm.h",
    port_h=ROOT / "include/port.h", psyq_dir=ROOT / "include/psyq", tool_dirs=[ROOT / "tools", MAIN / "tools"],
    defines=["NON_MATCHING"])


# ---------------------------------------------------------------------------------------------------------------------
# structs

TYPEDEF = re.compile(r"\btypedef\s+(struct|union)\b[^;{}]*\{")


def header_typedefs(path):
    """-> [(name, documented size or None)] for every `typedef struct/union {...} Name;` of a header."""
    raw = path.read_text(errors="replace")
    text = strip_code(raw)
    out = []
    for m in TYPEDEF.finditer(text):
        depth, i = 1, m.end()
        while depth and i < len(text):
            depth += (text[i] == "{") - (text[i] == "}")
            i += 1
        t = re.match(r"\s*(\w+)\s*;", text[i:i + 200])
        if not t:
            continue
        tail = raw[i + t.end():raw.find("\n", i + t.end())]
        s = re.match(r"\s*/\*\s*size\s+(0x[0-9A-Fa-f]+|\d+)", tail)
        out.append((t.group(1), int(s.group(1), 0) if s else None))
    return out


def measure_sizes(width, jobs):
    """-> ({(header, name): size}, {header: error}) by compiling one translation unit per header."""
    out = CFG.out / f"structs{width}"
    out.mkdir(parents=True, exist_ok=True)
    inc = write_overrides(CFG.out)
    env = dict(os.environ, LC_ALL="C")
    flags = ["gcc", f"-m{width}"] + PROBE_FLAGS + [f"-D{d}" for d in CFG.defines] + ["-w", "-fno-common", f"-I{inc}"]
    flags += [f"-I{d}" for d in CFG.include_dirs] + [f"-I{STACK_ROOT / 'include'}", f"-I{STACK_ROOT / 'include/psxstack'}"]
    if gcc_major() >= 14:
        flags.append("-fpermissive")

    def one(h):
        rel = h.relative_to(ROOT / "include").as_posix()
        names = [n for n, _ in header_typedefs(h)]
        if not names:
            return rel, {}, None
        stem = rel.replace("/", "__")[:-2]
        err = ""
        for prefix in ("", '#include "common.h"\n'):
            c = out / f"{stem}.c"
            c.write_text(prefix + f'#include "{rel}"\n'
                         + "".join(f"char size__{n}[sizeof({n})];\n" for n in names))
            o = c.with_suffix(".o")
            r = subprocess.run(flags + ["-c", str(c), "-o", str(o)], capture_output=True, text=True, env=env)
            if r.returncode == 0:
                break
            err = next((line for line in r.stderr.splitlines() if "error" in line), "gcc failed")
        else:
            return rel, {}, err
        sizes = {}
        for line in subprocess.run(["nm", "-S", str(o)], capture_output=True, text=True).stdout.splitlines():
            p = line.split()
            if len(p) == 4 and p[3].startswith("size__"):
                sizes[p[3][6:]] = int(p[1], 16)
        return rel, sizes, None

    sizes, errors = {}, {}
    with ThreadPoolExecutor(max_workers=jobs or os.cpu_count() or 1) as ex:
        for rel, s, err in ex.map(one, headers()):
            if err:
                errors[rel] = err
            for n, v in s.items():
                sizes[(rel, n)] = v
    return sizes, errors


def cmd_structs(args):
    CFG.out.mkdir(parents=True, exist_ok=True)
    t0 = time.time()
    doc = {}
    for h in headers():
        rel = h.relative_to(ROOT / "include").as_posix()
        for n, s in header_typedefs(h):
            doc[(rel, n)] = s
    documented = {k: v for k, v in doc.items() if v is not None}
    print(f"port inventory: struct sizes at {git_head()}, {gcc_version()}")
    print(f"typedef'd structs/unions in include/: {len(doc)}; with a `/* size 0xNN */` comment: {len(documented)}")
    sizes = {}
    for width in (32, 64):
        why = check_width(width, CFG.out)
        if why:
            print(f"-m{width}: the host gcc cannot compile at this width: {why}")
            continue
        sizes[width], errors = measure_sizes(width, args.jobs)
        for rel, err in sorted(errors.items()):
            print(f"-m{width}: include/{rel} does not compile on its own: {err}")
        s = sizes[width]
        match = [k for k in documented if s.get(k) == documented[k]]
        differ = [k for k in documented if k in s and s[k] != documented[k]]
        print(f"-m{width}: measured {len(s)} of {len(doc)}; documented sizes: {len(match)} match the PS1, "
              f"{len(differ)} differ")
        if width == 32 or not args.brief:
            for k in sorted(differ):
                print(f"    {k[1]:<32} PS1 0x{documented[k]:X}  -m{width} 0x{s[k]:X}  ({k[0]})")
    if 32 in sizes and 64 in sizes:
        common = [k for k in sizes[32] if k in sizes[64]]
        changed = [k for k in common if sizes[32][k] != sizes[64][k]]
        print(f"-m32 -> -m64: {len(changed)} of {len(common)} change size "
              f"({sum(1 for k in changed if k not in documented)} of them undocumented)")
        if args.verbose:
            for k in sorted(changed):
                if k not in documented:
                    print(f"    {k[1]:<32} -m32 0x{sizes[32][k]:X}  -m64 0x{sizes[64][k]:X}  ({k[0]})")
    print(f"({time.time() - t0:.1f} s)")
    return 0


def ps1_only_lines(src):
    """Per line of src: True inside the PS1-only side of a PC_PORT conditional (`#ifndef PC_PORT`, or the #else of
    `#ifdef PC_PORT` / `#if defined(PC_PORT)`)."""
    out, stack = [], []
    for line in src.text.split("\n"):
        m = re.match(r"\s*#\s*(ifdef|ifndef|if|else|elif|endif)\b\s*(.*)", line)
        if m:
            kind, rest = m.groups()
            if kind in ("ifdef", "ifndef", "if"):
                pc = re.search(r"\bPC_PORT\b", rest) is not None
                neg = kind == "ifndef" or (kind == "if" and re.search(r"!\s*defined", rest) is not None)
                stack.append((pc, neg))
            elif kind in ("else", "elif") and stack:
                pc, neg = stack[-1]
                stack[-1] = (pc, not neg)
            elif kind == "endif" and stack:
                stack.pop()
        out.append(any(pc and neg for pc, neg in stack))
    return out


def cmd_object_sizes(args):
    """FINDINGS 9d: object_new/object_create calls with a bare literal object or data size (exit 1 if any)."""
    bad, total = [], 0
    for path in c_files():
        src = Source(path)
        ps1 = None
        for m in re.finditer(r"\bobject_(?:new|create)\b", src.text):
            if not src.is_use(m):
                continue
            total += 1
            ps1 = ps1 or ps1_only_lines(src)
            ln = src.line(m.start())
            if ps1[ln - 1]:
                continue
            i = src.text.index("(", m.end())
            a = src.args(i)
            if len(a) < 3:
                continue
            lits = [(what, v) for what, v in (("size", a[1]), ("data", a[2]))
                    if INT_LITERAL.match(v) and (what == "size" or int(v.strip("()uUlL ").rstrip("uUlL"), 0) != 0)]
            if not lits:
                continue
            end = src.line(src.close_paren(i))
            lo = src.line_starts[max(ln - 2, 0)]
            hi = src.line_starts[end] if end < len(src.line_starts) else len(src.raw)
            if "PC_PORT: bytes" in src.raw[lo:hi]:
                continue
            bad.append(f"{src.rel}:{ln}: {m.group()} " + ", ".join(f"{what} {v}" for what, v in lits))
    for b in bad:
        print(b)
    print(f"object-sizes: {total} object_new/object_create calls in src/, {len(bad)} with a bare literal size")
    return 1 if bad else 0


def main():
    ap, sub = inv.build_parser()
    p = sub.add_parser("structs", help="sizeof of every typedef'd struct at -m32/-m64 vs the documented PS1 size")
    p.add_argument("-j", "--jobs", type=int)
    p.add_argument("--brief", action="store_true", help="do not list the documented structs that differ at -m64")
    p.add_argument("-v", "--verbose", action="store_true", help="also list the undocumented structs that change")
    p.set_defaults(func=cmd_structs)
    p = sub.add_parser("object-sizes", help="object_new/object_create calls with a bare literal size (exit 0 = none)")
    p.set_defaults(func=cmd_object_sizes)
    return inv.main(ap)


if __name__ == "__main__":
    sys.exit(main())
