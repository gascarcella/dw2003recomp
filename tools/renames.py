#!/usr/bin/env python3
"""Apply replayable rename lists (tools/renames/*.txt; issue #56): a branch that still uses old names catches up by
re-running the lists after merging, instead of fixing each conflict by hand. Idempotent: a list already applied
changes nothing.

  tools/venv/bin/python tools/renames.py tools/renames/<list>.txt [...]   # apply (default: every list, in name order)
  options: -n/--dry-run (report what would change), --no-compile (skip the compiler pass for qualified entries)

A list has one rename per line, `OLD NEW`, `#` starts a comment:
  * `OLD NEW`: OLD is replaced as a whole word in every tracked or new text file of src/, include/, config/, port/,
    tools/, tests/, launcher/, docs/ and the top-level *.md and *.py (not in tools/renames/ itself). For names that
    mean one thing repo-wide: symbols (functions, globals; an EXE symbol's address line in config/symbol_addrs.txt is
    added by hand) and fields whose name no other struct declares. A field name declared by several structs is refused:
    qualify it.
  * `Struct.field NEW`: the field of that struct (typedef or tag name) only. Its declaration in the struct's body is
    renamed (include/, src/, port/), then every C file of src/ is compiled with the host gcc (-fsyntax-only, once as
    the port's probe sees it: -DNON_MATCHING -DPC_PORT, once as the PS1 build's side: neither) and each
    "'Struct' has no member named 'field'" error is fixed at its line and column; repeated until none is left. In
    other text (docs, tests, comments) only the spelling `Struct.field` is replaced; other lines that mention both the
    struct and the old field are listed to check by hand. port/ is not compiled here: its build is the check.
NEW may end in an index (`vars[2]`) when fields become an array: the uses are renamed, the declaration is changed by
hand (the tool leaves declarations alone then, and leaves files other than C ones to the hand as well).
"""
import argparse
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LISTS = ROOT / "tools" / "renames"
SCAN_DIRS = ("src", "include", "config", "port", "tools", "tests", "launcher", "docs")
C_SUFFIXES = {".c", ".h", ".cpp", ".hpp", ".inc"}
IDENT = r"[A-Za-z_]\w*"
ENTRY = re.compile(rf"^(?:(?P<struct>{IDENT})\.)?(?P<old>{IDENT})\s+(?P<new>{IDENT}(?:\[\w+\])*)$")
NO_MEMBER = re.compile(r"^(?P<file>[^:\s][^:]*):(?P<line>\d+):(?P<col>\d+): error: '(?P<type>[^']+)'"
                       r"(?: \{aka '(?P<aka>[^']+)'\})? has no member named '(?P<field>\w+)'")
PROBE_FLAGS = ["-std=gnu99", "-fsyntax-only", "-fno-builtin", "-fsigned-char", "-w", "-fmax-errors=0",
               "-fdiagnostics-color=never", "-fno-diagnostics-show-caret", "-fmessage-length=0"]
PASSES = [["-DNON_MATCHING", "-DPC_PORT"], []]


def strip_code(t):
    """Comments, string and character literals blanked; offsets and newlines kept."""
    def blank(m):
        s = m.group()
        if s[0] == "/":
            return re.sub(r"[^\n]", " ", s)
        return s[0] + re.sub(r"[^\n]", " ", s[1:-1]) + s[-1]
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', blank, t, flags=re.S)


def read_lists(paths):
    entries = []
    for p in paths:
        for n, line in enumerate(Path(p).read_text().splitlines(), 1):
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            m = ENTRY.match(" ".join(line.split()))
            if not m:
                sys.exit(f"{p}:{n}: not `OLD NEW` or `Struct.field NEW`: {line}")
            entries.append((m.group("struct"), m.group("old"), m.group("new"), f"{Path(p).name}:{n}"))
    return entries


def text_files():
    out = subprocess.run(["git", "ls-files", "-co", "--exclude-standard", "-z"], cwd=ROOT, capture_output=True,
                         check=True).stdout.decode().split("\0")
    files = []
    for rel in out:
        if not rel or rel.startswith("tools/renames/"):
            continue
        top = rel.split("/", 1)[0]
        if top in SCAN_DIRS or ("/" not in rel and rel.endswith((".md", ".py"))):
            p = ROOT / rel
            if p.is_file() and not p.is_symlink():
                files.append(p)
    return files


class Files:
    """The scanned files' text, read once; writes are collected and flushed at the end (or not, on a dry run)."""

    def __init__(self, paths):
        self.text, self.changed = {}, set()
        for p in paths:
            try:
                self.text[p] = p.read_text()
            except (UnicodeDecodeError, OSError):
                pass

    def set(self, p, t):
        if t != self.text[p]:
            self.text[p] = t
            self.changed.add(p)

    def flush(self, dry):
        if not dry:
            for p in self.changed:
                p.write_text(self.text[p])


def struct_bodies(text):
    """-> [(names, start, end)]: each struct/union body's span ({ to }) and its tag and typedef/declarator names."""
    code = strip_code(text)
    out = []
    for m in re.finditer(rf"\b(?:struct|union)\s*({IDENT})?\s*\{{", code):
        depth, i = 0, m.end() - 1
        while i < len(code):
            if code[i] == "{":
                depth += 1
            elif code[i] == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        names = {m.group(1)} if m.group(1) else set()
        after = re.match(rf"\s*({IDENT})", code[i + 1:])
        if after:
            names.add(after.group(1))
        out.append((names, m.end(), i))
    return out


def declarator(old):
    """A field's declarator in a struct body: the name, then array bounds, then `;`, `,` or a bit-field's `:`."""
    return re.compile(rf"\b{re.escape(old)}\b(?=\s*(?:\[[^\]]*\]\s*)*[;,:])")


def declarations(files, old):
    """-> [(path, struct names)] of the struct bodies that declare a field named old (C files of include/src/port)."""
    found, pat = [], declarator(old)
    for p, t in files.text.items():
        if p.suffix not in C_SUFFIXES or old not in t:
            continue
        code = strip_code(t)
        for names, s, e in struct_bodies(t):
            if pat.search(code, s, e):
                found.append((p, names))
    return found


def rename_declaration(files, struct, old, new):
    n, pat = 0, declarator(old)
    for p, t in list(files.text.items()):
        if p.suffix not in C_SUFFIXES or old not in t or struct not in t:
            continue
        code = strip_code(t)
        spans = [(s, e) for names, s, e in struct_bodies(t) if struct in names]
        hits = [m for s, e in spans for m in pat.finditer(code, s, e)]
        for m in sorted(hits, key=lambda m: -m.start()):
            t = t[:m.start()] + new + t[m.end():]
            n += 1
        files.set(p, t)
    return n


def compile_errors(files, units):
    """Compiles the units as they are in `files` (written to disk first) -> list of NO_MEMBER matches."""
    def one(args):
        unit, flags = args
        r = subprocess.run(["gcc"] + PROBE_FLAGS + flags + [f"-I{ROOT / 'build/port_inventory/include'}",
                            f"-I{ROOT / 'include'}", f"-I{ROOT}", str(unit.relative_to(ROOT))],
                           cwd=ROOT, capture_output=True, text=True, errors="replace",
                           env=dict(os.environ, LC_ALL="C"))
        return [m for m in map(NO_MEMBER.match, r.stderr.splitlines()) if m]
    jobs = [(u, f) for u in units for f in PASSES]
    with ThreadPoolExecutor() as ex:
        return [m for ms in ex.map(one, jobs) for m in ms]


def ensure_overrides():
    """The port probe's override headers (INCLUDE_ASM empty, gte_* macros no-ops), made by tools/port_inventory.py."""
    sys.path.insert(0, str(ROOT / "tools"))
    import port_inventory
    port_inventory.write_overrides(port_inventory.OUT)


def type_names(m):
    names = set()
    for t in (m.group("type"), m.group("aka")):
        if t:
            t = re.sub(r"\b(?:const|volatile|struct|union)\b", "", t).strip()
            names.add(t)
    return names


def fix_uses(files, qualified, dry):
    """Compile-driven: fixes "'S' has no member named 'old'" for each qualified (S, old) -> new until none is left."""
    units = sorted(p for p in files.text if p.suffix == ".c" and p.relative_to(ROOT).parts[0] == "src")
    ensure_overrides()
    fixed, unresolved = 0, []
    for _ in range(6):
        if files.changed:                     # the compiler reads the disk: write what changed so far
            files.flush(False)
        errs = compile_errors(files, units)
        todo = {}
        for m in errs:
            for s in type_names(m):
                if (s, m.group("field")) in qualified:
                    p = (ROOT / m.group("file")).resolve()
                    todo.setdefault(p, set()).add((int(m.group("line")), int(m.group("col")), m.group("field"),
                                                   qualified[(s, m.group("field"))]))
        if not todo:
            # what is left: a new name the struct does not have (a wrong site, or a declaration not renamed yet)
            news = {re.match(r"\w+", n).group() for n in qualified.values()}
            unresolved = [m.group(0) for m in errs if m.group("field") in news]
            break
        for p, sites in todo.items():
            if p not in files.text:
                unresolved += [f"{p}:{ln}:{col}: not a scanned file" for ln, col, _, _ in sites]
                continue
            lines = files.text[p].split("\n")
            for ln, col, old, new in sorted(sites, key=lambda s: (s[0], -s[1])):
                line = lines[ln - 1]
                at = col - 1
                if line[at:at + len(old)] != old or re.match(r"\w", line[at + len(old):at + len(old) + 1] or " "):
                    cands = [m.start() for m in re.finditer(rf"(?:->|\.)\s*\b{re.escape(old)}\b", line)]
                    cands = [line.index(old, c) for c in cands]
                    after = [c for c in cands if c >= at]
                    if not cands:
                        unresolved.append(f"{p.relative_to(ROOT)}:{ln}:{col}: no `{old}` access on the line")
                        continue
                    at = after[0] if after else cands[-1]
                lines[ln - 1] = line[:at] + new + line[at + len(old):]
                fixed += 1
            files.set(p, "\n".join(lines))
    return fixed, unresolved


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("lists", nargs="*", help="rename lists (default: tools/renames/*.txt)")
    ap.add_argument("-n", "--dry-run", action="store_true")
    ap.add_argument("--no-compile", action="store_true")
    args = ap.parse_args()
    paths = args.lists or sorted(LISTS.glob("*.txt"))
    entries = read_lists(paths)
    files = Files(text_files())
    originals = dict(files.text)
    report, check = [], []

    qualified = {}
    for struct, old, new, where in entries:
        plain = re.fullmatch(IDENT, new) is not None
        if struct is None:
            decls = declarations(files, old)
            if len(decls) > 1:
                sys.exit(f"{where}: `{old}` is a field of several structs "
                         f"({', '.join('/'.join(sorted(n)) for _, n in decls)}): qualify it as Struct.{old}")
            word = re.compile(rf"\b{re.escape(old)}\b")
            n = 0
            for p, t in list(files.text.items()):
                if old not in t:
                    continue
                if not plain:
                    if p.suffix not in C_SUFFIXES:
                        check += [f"{p.relative_to(ROOT)}:{i}: `{old}` (-> {new}, by hand)"
                                  for i, l in enumerate(t.split("\n"), 1) if word.search(l)]
                        continue
                    code = strip_code(t)       # leave the declaration: it is changed by hand
                    keep = {m.start() for _, s, e in struct_bodies(t) for m in declarator(old).finditer(code, s, e)}
                    t2 = "".join(seg if i == 0 else (new if pos not in keep else old) + seg
                                 for i, (pos, seg) in enumerate(_split_words(t, word)))
                else:
                    t2 = word.sub(new, t)
                n += t != t2
                files.set(p, t2)
            report.append(f"{where}: {old} -> {new}: {n} file(s)")
        else:
            n = rename_declaration(files, struct, old, new) if plain else 0
            if plain and n == 0 and not declarations(files, new):
                report.append(f"{where}: {struct}.{old}: no declaration found (nothing renamed)")
            dotted = re.compile(rf"\b{re.escape(struct)}\.{re.escape(old)}\b")
            for p, t in list(files.text.items()):
                if dotted.search(t):
                    files.set(p, dotted.sub(f"{struct}.{new}", t))
            qualified[(struct, old)] = new
            report.append(f"{where}: {struct}.{old} -> {new}: declaration {'renamed' if n else 'unchanged'}")

    if qualified and not args.no_compile:
        if args.dry_run:
            report.append("dry run: the compiler pass for qualified entries is skipped")
        else:
            fixed, unresolved = fix_uses(files, qualified, args.dry_run)
            report.append(f"compiler pass: {fixed} member access(es) renamed")
            check += [f"unresolved: {u}" for u in unresolved]
        for (struct, old), new in qualified.items():
            word = re.compile(rf"\b{re.escape(old)}\b")
            for p, t in files.text.items():
                if p.suffix == ".c" and p.relative_to(ROOT).parts[0] == "src" and not args.no_compile:
                    continue
                if struct in t and word.search(t):
                    other = set()             # the same name declared by another struct: not a use
                    if p.suffix in C_SUFFIXES:
                        code = strip_code(t)
                        other = {t.count("\n", 0, m.start()) + 1 for names, s, e in struct_bodies(t)
                                 if struct not in names for m in declarator(old).finditer(code, s, e)}
                    check += [f"{p.relative_to(ROOT)}:{i}: `{old}` (if {struct}'s: -> {new})"
                              for i, l in enumerate(t.split("\n"), 1) if word.search(l) and i not in other]

    files.flush(args.dry_run)
    changed = sorted(p.relative_to(ROOT).as_posix() for p in files.text if files.text[p] != originals[p])
    for r in report:
        print(r)
    print(f"{'would change' if args.dry_run else 'changed'}: {len(changed)} file(s)")
    for c in changed:
        print(f"  {c}")
    if check:
        print("check by hand:")
        for c in check:
            print(f"  {c}")


def _split_words(t, word):
    """[(start, text)]: the text before the first match, then each match's start with the text after it."""
    out, last, pos = [], 0, None
    for m in word.finditer(t):
        out.append((pos, t[last:m.start()]))
        pos, last = m.start(), m.end()
    out.append((pos, t[last:]))
    return out


if __name__ == "__main__":
    main()
