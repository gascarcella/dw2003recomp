#!/usr/bin/env python3
"""Candidate names for our unnamed symbols and struct fields, from a reference decomp's EU build (issue #56).

The reference is ReGame-Labs/dw3_decomp (the "US decomp" of DECISIONS), which builds the same EU executable and
overlays byte-identically. This tool only READS a built clone of it (outside the repo; nothing is copied in) and our
own build, and writes a report under build/ref_names/. It never edits src/ or include/: a candidate is a suggestion
(DECISIONS "Naming conventions": name only what the code confirms, in our snake_case).

  tools/venv/bin/python tools/ref_names.py --ref /path/to/dw3_decomp      # or DW3_REF_DECOMP=/path/to/dw3_decomp
  options: -j N (parallel clang runs), --clang PATH, --out DIR (default build/ref_names), --no-cache

Needs: our build (`scripts/build.sh`: build/<target>/ maps, ELFs, objects and the preprocessed `.c.o.i` beside each
object), the reference built for EU (its build/eu/: maps, ELFs, objects, `.c.i`), and a clang (on PATH, or the
pinned one from `scripts/setup.sh llvm-mingw`). clang only parses (-fsyntax-only, target mipsel so the layouts are the
PS1's): no code is generated.

How it pairs:
  * Symbols: both sides' link maps/objects/ELFs give every symbol's address per link unit; the same address in the
    same unit (the EXE, or one overlay) is the same function or global. -> symbols.tsv, and the report lists our
    remaining func_/D_ names that the C still uses.
  * Structs: each preprocessed unit is parsed by clang twice: the AST as JSON (every member access `a->b.c`, with
    the field it names, every global's and parameter's type) and the record layouts (-fdump-record-layouts-complete:
    each field's offset). A member access becomes (struct, offset from that struct, size) for every struct in its
    `.`-chain. Per address-paired function, our structs and theirs are matched one-to-one by how many of these
    (offset, size) accesses they share; votes add up over all functions, plus globals at the same address (3) and
    parameters at the same position (2) whose types are both a struct (or a pointer to one). Each of our structs
    takes the reference struct with the most votes (a tie: the one of the same name). Struct keys: the typedef or
    tag name; `Name@dir/file` for one defined in a .c file; `Name#dir` when a side has several layouts under one
    name (their per-overlay headers); `Parent.field` for an unnamed nested struct.
  * Fields: for each field of a paired struct, the reference field(s) at the same offset with the same size are
    the candidates, at any nesting depth and inside arrays (`vars[2]`, `cells[3].x`). Evidence = our functions in
    which both sides access that offset and size of the paired structs (`~`: only a wider access of theirs covers
    it). Flags: weak struct pairing (< 3 votes or < 60% of the struct's votes), struct conflict (another struct
    with >= half the votes), union (several candidates), size mismatch, ours spanning several of their fields,
    layout only (no access in a paired function).

Outputs (build/ref_names/, not committed): report.md (summary counts; remaining func_/D_ names; per struct of ours
with `unk_` fields: offset, our field, C uses, the candidate, its type, evidence, flags), fields.tsv / fields.json
(machine-readable, every field of every struct of ours, for the adoption batches), symbols.tsv (every
address-paired symbol), remaining.tsv (our default names the C uses, with the reference's). A cache of the clang
results is kept in build/ref_names/cache/ (keyed by each .i's size and mtime): a rerun takes seconds. The
reference's names stay out of the repo until adopted by hand (docs/THIRD_PARTY.md gets its row then).
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CACHE_VERSION = "2"
DEFAULT_NAME = re.compile(r"^(?:func|D|jtbl|jpt|L|lbl|switchD)_(?:[A-Za-z0-9]+_)?[0-9A-Fa-f]{8}$")
UNNAMED_FIELD = re.compile(r"^unk_?[0-9A-Fa-fx]*$")
# link-script and assembler bookkeeping symbols: not names of anything
LINKER_NAME = re.compile(r"(?:_VRAM|_ROM_?START|_ROM_?END|_(?:TEXT|DATA|RODATA|BSS|SDATA|SBSS)_(?:START|END|SIZE))$"
                         r"|^(?:__gnu_compiled_c|gcc2_compiled\.|_gp|HEAP_START|_end|end|_fbss|_fdata|_ftext)$|\.")
BUILTIN_SIZES = {
    "char": 1, "signed char": 1, "unsigned char": 1, "_Bool": 1,
    "short": 2, "unsigned short": 2, "short int": 2, "unsigned short int": 2,
    "int": 4, "unsigned int": 4, "long": 4, "unsigned long": 4, "long int": 4, "unsigned long int": 4,
    "unsigned": 4, "signed": 4, "float": 4,
    "long long": 8, "unsigned long long": 8, "double": 8, "long double": 8,
}
WEAK_VOTES = 3  # a struct pairing with fewer votes (or under 60% of its struct's votes) is flagged weak
CLANG_FLAGS = [
    "-target", "mipsel-unknown-linux-gnu", "-fsyntax-only", "-x", "c", "-std=gnu89", "-w", "-ferror-limit=0",
    "-Wno-error=int-conversion", "-Wno-error=incompatible-pointer-types", "-Wno-error=implicit-function-declaration",
    "-Wno-error=implicit-int", "-Wno-error=return-type", "-Wno-error=incompatible-function-pointer-types",
]


# ---------------------------------------------------------------------------------------------------------------
# ELF and link maps
# ---------------------------------------------------------------------------------------------------------------

def elf_symbols(path):
    """[(name, value, size, type, bind, section name)] of an ELF32 little-endian file (stdlib only)."""
    data = Path(path).read_bytes()
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
        raise ValueError(f"{path}: not an ELF32 LE file")
    shoff, = struct.unpack_from("<I", data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", data, 0x2E)
    secs = [struct.unpack_from("<IIIIIIIIII", data, shoff + i * shentsize) for i in range(shnum)]
    shstr = secs[shstrndx]

    def cstr(off):
        return data[off:data.index(b"\0", off)].decode("latin-1")

    names = [cstr(shstr[4] + s[0]) for s in secs]
    out = []
    for s in secs:
        if s[1] != 2:  # SHT_SYMTAB
            continue
        strtab = secs[s[6]]
        for i in range(s[5] // 16):
            st_name, value, size, info, _other, shndx = struct.unpack_from("<IIIBBH", data, s[4] + i * 16)
            name = cstr(strtab[4] + st_name) if st_name else ""
            typ, bind = info & 15, info >> 4
            if shndx == 0:
                sec = "UND"
            elif shndx == 0xFFF1:
                sec = "ABS"
            elif shndx < len(names):
                sec = names[shndx]
            else:
                sec = "?"
            out.append((name, value, size, typ, bind, sec))
    return out


MAP_SECTION = re.compile(r"^ (\.\w+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)\s*$")


def parse_map(path):
    """{object path: {section: address}} for the input sections of one link map."""
    out = defaultdict(dict)
    for line in Path(path).read_text(errors="replace").splitlines():
        m = MAP_SECTION.match(line)
        if m and int(m.group(3), 16):
            out[m.group(4)].setdefault(m.group(1), int(m.group(2), 16))
    return out


class Side:
    """One decomp's build: link units (targets), their maps and ELFs, and its preprocessed units."""

    def __init__(self, label, root, build, units):
        self.label, self.root, self.build = label, Path(root), Path(build)
        self.units = units          # [(object path relative to root, preprocessed path, source path)]
        self.targets = {}           # target -> (map path, elf path)
        self.obj_targets = defaultdict(list)  # object rel path -> [(target, {section: addr})]
        self.elf_syms = {}          # target -> {name: [(value, section, type)]}

    def load_links(self):
        for target, (mp, elf) in self.targets.items():
            for obj, secs in parse_map(mp).items():
                self.obj_targets[obj].append((target, secs))
            syms = defaultdict(list)
            for name, value, _size, typ, _bind, sec in elf_symbols(elf):
                if name and typ in (0, 1, 2) and sec != "UND" and not name.startswith((".", "$")):
                    syms[name].append((value, sec, typ))
            self.elf_syms[target] = syms

    def global_key(self, target, name):
        """(link unit, address) of a global as seen from a unit linked into `target`."""
        for value, sec, _typ in self.elf_syms.get(target, {}).get(name, []):
            if sec != "ABS":
                return (target, value)
        for value, sec, _typ in self.elf_syms.get("main", {}).get(name, []):
            if sec != "ABS":
                return ("main", value)
        return None


def target_name(stem):
    t = stem.lower().split(".pro")[0]
    return "main" if t.startswith("sles_") else t


def our_side():
    side = Side("ours", ROOT, ROOT / "build", [])
    for mp in sorted((ROOT / "build").glob("*/*.map")):
        elf = mp.with_suffix(".elf")
        if elf.exists():
            side.targets[mp.parent.name] = (mp, elf)
    for i in sorted((ROOT / "build").glob("*/src/**/*.c.o.i")):
        obj = i.with_suffix("")  # strip .i
        rel = obj.relative_to(ROOT)
        src = Path(*rel.parts[2:]).with_suffix("")  # build/<t>/src/x/y.c.o -> src/x/y.c
        side.units.append((str(rel), i, src))
    return side


def ref_side(ref):
    build = ref / "build/eu"
    side = Side("ref", ref, build, [])
    for mp in sorted(build.glob("*.map")):
        elf = mp.with_suffix(".elf")
        if elf.exists():
            side.targets[target_name(mp.stem)] = (mp, elf)
    for i in sorted(build.glob("src/**/*.c.i")):
        obj = i.with_suffix(".o")
        rel = obj.relative_to(ref)
        src = Path(*rel.parts[2:]).with_suffix("")  # build/eu/src/x/y.c.o -> src/x/y.c
        side.units.append((str(rel), i, src))
    return side


# ---------------------------------------------------------------------------------------------------------------
# One preprocessed unit through clang: records (layouts), functions (member accesses, parameters), globals
# ---------------------------------------------------------------------------------------------------------------

def parse_layouts(text):
    """clang -fdump-record-layouts output -> [{"header", "size", "entries": [(off, bits, depth, type, name)]}]."""
    recs, cur = [], None
    for line in text.splitlines():
        if line.startswith("*** Dumping AST Record Layout"):
            cur = None
            continue
        left, sep, right = line.partition(" | ")
        if not sep:
            continue
        left = left.strip()
        if right.startswith("[sizeof="):
            if cur is not None:
                cur["size"] = int(re.match(r"\[sizeof=(\d+)", right).group(1))
                recs.append(cur)
                cur = None
            continue
        depth = (len(right) - len(right.lstrip(" "))) // 2
        body = right.strip()
        if depth == 0:
            cur = {"header": body, "size": None, "entries": []}
            continue
        if cur is None:
            continue
        bits = None
        if ":" in left:
            byte, _, rng = left.partition(":")
            off = int(byte)
            lo, _, hi = rng.partition("-")
            bits = (int(lo), int(hi))
        else:
            off = int(left)
        if body.endswith(")"):
            typ, name = body, ""
        else:
            typ, _, name = body.rpartition(" ")
        cur["entries"].append((off, bits, depth, typ.strip(), name))
    return recs


def norm_header(h):
    return h.replace("(anonymous at ", "(unnamed at ")


def strip_quals(t):
    t = re.sub(r"\b(const|volatile|register|restrict|__restrict)\b", " ", t)
    return " ".join(t.split())


class TypeSizer:
    def __init__(self, typedefs, header_sizes):
        self.typedefs = typedefs        # name -> ("record", size) | ("type", qualType)
        self.header_sizes = header_sizes  # normalized layout header -> size

    def size(self, t, depth=0):
        if depth > 20:
            return None
        t = strip_quals(t)
        if "(*" in t:  # pointer to function/array, or an array of them: (*[N])
            m = re.search(r"\(\*((?:\[\d+\])+)\)", t)
            n = 4
            for d in re.findall(r"\[(\d+)\]", m.group(1)) if m else ():
                n *= int(d)
            return n
        dims = [int(d) if d else 0 for d in re.findall(r"\[(\d*)\]", t)]
        t = re.sub(r"\s*(\[\d*\])+$", "", t)
        if t.endswith("*"):
            base = 4
        elif t in BUILTIN_SIZES:
            base = BUILTIN_SIZES[t]
        elif t.startswith("enum "):
            base = 4
        elif norm_header(t) in self.header_sizes:
            base = self.header_sizes[norm_header(t)]
        elif t in self.typedefs:
            kind, v = self.typedefs[t]
            base = v if kind == "record" else self.size(v, depth + 1)
        else:
            return None
        if base is None:
            return None
        for d in dims:
            base *= d
        return base


def type_record(qt, tag_keys, typedef_keys, typedef_types, depth=0):
    """A type string -> (record key, shape) where shape is '', '*', '**', '[]' ...; or None."""
    t = strip_quals(qt)
    shape = ""
    m = re.match(r"^(.*?)\s*((?:\[\d*\])+)$", t)
    if m:
        t, shape = m.group(1), "[]"
    while t.endswith("*") and "(" not in t:
        t, shape = t[:-1].rstrip(), shape + "*"
    m = re.match(r"^(?:struct|union) (\w+)$", t)
    if m and m.group(1) in tag_keys:
        return tag_keys[m.group(1)], shape
    if t in typedef_keys:
        return typedef_keys[t], shape
    if t in typedef_types and depth < 10:
        r = type_record(typedef_types[t], tag_keys, typedef_keys, typedef_types, depth + 1)
        if r:
            return r[0], shape + r[1]
    return None


def walk(node, parent=None):
    """Iterative pre-order walk yielding (node, parent)."""
    stack = [(node, parent)]
    while stack:
        n, p = stack.pop()
        yield n, p
        for c in reversed(n.get("inner", ()) or ()):
            if isinstance(c, dict):
                stack.append((c, n))


def strip_paren(n):
    while n.get("kind") in ("ParenExpr",) and n.get("inner"):
        n = n["inner"][0]
    return n


def defines_record(src_text, name):
    return bool(re.search(r"\b(?:struct|union)\s+" + re.escape(name) + r"\s*\{", src_text)
                or re.search(r"\}\s*" + re.escape(name) + r"\s*;", src_text))


def analyze_unit(args):
    """Parse one preprocessed unit; returns a JSON-able dict (cached by the caller)."""
    clang, ipath, src_text, src_label = args
    def run(extra):
        return subprocess.run([clang, *CLANG_FLAGS, *extra, str(ipath)], capture_output=True, text=True)

    lay = run(["-Xclang", "-fdump-record-layouts-complete"])
    ast = run(["-Xclang", "-ast-dump=json"])
    errors = len(re.findall(r"\berror:", ast.stderr))
    layouts = [r for r in parse_layouts(lay.stdout) if "__NSConstantString" not in r["header"]]
    try:
        tu = json.loads(ast.stdout)
    except json.JSONDecodeError:
        return {"error": "clang produced no AST", "stderr": ast.stderr[-2000:]}

    # Complete record definitions in completion (post-) order, matching the layout dump's order.
    records = []          # [node]
    rec_parent = {}       # record id -> enclosing record id
    field_owner = {}      # FieldDecl id -> (record id, index among the record's FieldDecls)
    typedef_of = {}       # record id -> first typedef name owning it
    typedef_types = {}    # typedef name -> qualType
    typedef_rec = {}      # typedef name -> record id
    stack = [(tu, None, False)]
    while stack:
        n, prec, done = stack.pop()
        kind = n.get("kind")
        if done:
            records.append(n)
            continue
        if kind == "RecordDecl" and n.get("completeDefinition") and not n.get("isImplicit"):
            if prec is not None:
                rec_parent[n["id"]] = prec
            stack.append((n, prec, True))
            fields = [c for c in n.get("inner", ()) if c.get("kind") == "FieldDecl"]
            for idx, f in enumerate(fields):
                field_owner[f["id"]] = (n["id"], idx)
            for c in reversed(n.get("inner", ())):
                stack.append((c, n["id"], False))
            continue
        if kind == "TypedefDecl" and not n.get("isImplicit"):
            typedef_types.setdefault(n["name"], n.get("type", {}).get("qualType", ""))
            for c, _ in walk(n):
                d = c.get("decl")
                if isinstance(d, dict) and d.get("kind") == "RecordDecl":
                    typedef_rec.setdefault(n["name"], d["id"])
                    typedef_of.setdefault(d["id"], n["name"])
                    break
        for c in reversed(n.get("inner", ()) or ()):
            if isinstance(c, dict):
                stack.append((c, prec, False))

    # Pair JSON records with layout records (same order); check the top-level field names agree.
    mismatches = 0
    rec_layout = {}
    if len(records) == len(layouts):
        for n, lay_rec in zip(records, layouts):
            jn = [f.get("name", "") for f in n.get("inner", ()) if f.get("kind") == "FieldDecl"]
            ln = [e[4] for e in lay_rec["entries"] if e[2] == 1]
            if jn != ln:
                mismatches += 1
                continue
            rec_layout[n["id"]] = lay_rec
    else:
        mismatches = abs(len(records) - len(layouts)) or 1

    # Record keys: typedef name, else tag, else parent.field; a struct defined in this unit's own .c gets @source.
    by_id = {n["id"]: n for n in records}
    keys = {}

    def key_of(rid):
        if rid in keys:
            return keys[rid]
        n = by_id[rid]
        name = typedef_of.get(rid) or n.get("name")
        if name:
            k = name + (f"@{src_label}" if src_text and defines_record(src_text, name) else "")
        elif rid in rec_parent and rec_parent[rid] in by_id:
            parent = by_id[rec_parent[rid]]
            inner = parent.get("inner", ())
            pos = next(i for i, c in enumerate(inner) if c.get("id") == rid)
            fname = ""
            for c in inner[pos + 1:]:
                if c.get("kind") == "FieldDecl":
                    fname = c.get("name", "")
                    break
            k = key_of(parent["id"]) + "." + (fname or f"anon{pos}")
        else:
            k = "<anon " + ",".join(f.get("name", "") for f in n.get("inner", ()) if f.get("kind") == "FieldDecl") + ">"
        keys[rid] = k
        return k

    header_sizes = {norm_header(lr["header"]): lr["size"] for lr in layouts}
    typedefs = {}
    for name, qt in typedef_types.items():
        rid = typedef_rec.get(name)
        if rid in rec_layout:
            typedefs[name] = ("record", rec_layout[rid]["size"])
        else:
            typedefs[name] = ("type", qt)
    sizer = TypeSizer(typedefs, header_sizes)

    out_records = {}
    field_info = {}  # (record id, index) -> (name, off, size, bits, type)
    for n in records:
        rid = n["id"]
        lr = rec_layout.get(rid)
        if lr is None:
            continue
        k = key_of(rid)
        top = [e for e in lr["entries"] if e[2] == 1]
        fields = []
        for idx, (off, bits, _d, typ, name) in enumerate(top):
            size = sizer.size(typ)
            if size is None:
                nxt = top[idx + 1][0] if idx + 1 < len(top) else lr["size"]
                size = max(nxt - off, 0)
            if bits:
                size = 0
            fields.append([name, off, size, typ, bits])
            field_info[(rid, idx)] = (name, off, size, bits, typ)
        flat, path = [], []
        for off, bits, d, typ, name in lr["entries"]:
            path[d - 1:] = [name or "<anon>"]
            size = 0 if bits else sizer.size(typ)
            flat.append([".".join(p for p in path if p != "<anon>"), off, size, typ, d])
        out_records[k] = {"size": lr["size"], "kind": n.get("tagUsed"), "fields": fields, "flat": flat}

    tag_keys = {n["name"]: key_of(n["id"]) for n in records if n.get("name") and n["id"] in rec_layout}
    typedef_keys = {t: key_of(r) for t, r in typedef_rec.items() if r in rec_layout}

    def rec_of(qt):
        return type_record(qt, tag_keys, typedef_keys, typedef_types)

    functions, globals_ = [], []
    for top_node in tu.get("inner", ()):
        kind = top_node.get("kind")
        if kind == "VarDecl" and top_node.get("name"):
            r = rec_of(top_node.get("type", {}).get("qualType", ""))
            if r:
                globals_.append([top_node["name"], r[0], r[1]])
            continue
        if kind != "FunctionDecl" or top_node.get("isImplicit"):
            continue
        inner = top_node.get("inner", ())
        if not any(c.get("kind") == "CompoundStmt" for c in inner):
            continue
        params = []
        for c in inner:
            if c.get("kind") == "ParmVarDecl":
                r = rec_of(c.get("type", {}).get("qualType", ""))
                params.append(list(r) if r else None)
        views = []
        for node, parent in walk(top_node):
            if node.get("kind") != "MemberExpr":
                continue
            if parent is not None and parent.get("kind") == "MemberExpr" and not parent.get("isArrow") \
                    and parent.get("inner") and strip_paren(parent["inner"][0]) is node:
                continue  # not the end of a `.` chain
            chain, cur = [], node
            while True:
                fo = field_owner.get(cur.get("referencedMemberDecl"))
                if fo is None or fo not in field_info:
                    chain = None
                    break
                chain.append((fo[0], field_info[fo]))
                if cur.get("isArrow") or not cur.get("inner"):
                    break
                base = strip_paren(cur["inner"][0])
                if base.get("kind") != "MemberExpr":
                    break
                cur = base
            if not chain:
                continue
            leaf = chain[0][1]
            off, names = 0, []
            for rid, (fname, foff, _fs, _bits, _t) in chain:
                off += foff
                if fname:
                    names.insert(0, fname)
                views.append([key_of(rid), off, leaf[2], ".".join(names), 1 if leaf[3] else 0])
        functions.append({"name": top_node["name"], "static": top_node.get("storageClass") == "static",
                          "params": params, "views": views})
    return {"records": out_records, "functions": functions, "globals": globals_,
            "errors": errors, "layout_mismatches": mismatches}


def load_units(side, clang, cache_dir, jobs, use_cache):
    cache_dir.mkdir(parents=True, exist_ok=True)
    todo, results = [], {}
    for rel, ipath, src in side.units:
        st = ipath.stat()
        h = hashlib.sha1(f"{CACHE_VERSION}|{ipath}|{st.st_size}|{st.st_mtime_ns}".encode()).hexdigest()[:20]
        cpath = cache_dir / f"{h}.json"
        if use_cache and cpath.exists():
            results[rel] = json.loads(cpath.read_text())
        else:
            srcp = side.root / src
            text = srcp.read_text(errors="replace") if srcp.exists() else ""
            label = str(Path(*src.parts[1:]).with_suffix(""))  # src/main/inn.c -> main/inn
            todo.append((rel, ipath, text, label, cpath))
    if todo:
        print(f"[{side.label}] parsing {len(todo)} units with clang ({jobs} jobs)...", file=sys.stderr)
        with ProcessPoolExecutor(jobs) as ex:
            jobs_ = [(clang, i, t, lab) for _r, i, t, lab, _c in todo]
            for (rel, _i, _t, _l, cpath), res in zip(todo, ex.map(analyze_unit, jobs_, chunksize=1)):
                cpath.write_text(json.dumps(res))
                results[rel] = res
    return results


# ---------------------------------------------------------------------------------------------------------------
# Aggregation and pairing
# ---------------------------------------------------------------------------------------------------------------

class Model:
    """One side's records, functions keyed by (link unit, address), globals keyed the same way."""

    def __init__(self, side, units):
        self.side = side
        self.records = {}                 # key -> record (first seen)
        self.record_variants = defaultdict(set)
        self.funcs = {}                   # (target, addr) -> {"name", "unit", "params", "views"}
        self.globals = {}                 # (target, addr) -> (name, record key, shape)
        self.problems = []
        self.uses = Counter()             # (record key, field path) -> member accesses in address-known functions
        self.renamed = {}                 # key -> [variant keys] for a name with several layouts on this side
        rename = self._variants(units)
        for rel, res in units.items():
            if "error" in res:
                self.problems.append(f"{rel}: {res['error']}")
                continue
            if res["errors"] or res["layout_mismatches"]:
                self.problems.append(f"{rel}: {res['errors']} clang errors, {res['layout_mismatches']} layout mismatches")
            umap = {k: rename[(k, sig)] for k, r in res["records"].items() if (k, sig := record_sig(r)) in rename}
            if umap:
                res = rekey_unit(res, umap)
            for k, r in res["records"].items():
                self.record_variants[k].add(record_sig(r))
                self.records.setdefault(k, r)
            links = side.obj_targets.get(rel, [])
            text_syms = {}
            objpath = side.root / rel
            if objpath.exists():
                for name, value, _size, typ, _bind, sec in elf_symbols(objpath):
                    if typ == 2 and sec == ".text":
                        text_syms[name] = value
            for f in res["functions"]:
                if f["name"] not in text_syms:
                    continue
                for target, secs in links:
                    if ".text" in secs:
                        key = (target, secs[".text"] + text_syms[f["name"]])
                        self.funcs[key] = dict(f, unit=rel)
                # uses: each access counted once per struct it names directly (views whose path is one field)
                for v in f["views"]:
                    if v[3] and "." not in v[3]:
                        self.uses[(v[0], v[3])] += 1
            for name, rk, shape in res["globals"]:
                for target, _secs in links:
                    gk = side.global_key(target, name)
                    if gk:
                        self.globals[gk] = (name, rk, shape)
        self.conflicting_records = {k for k, v in self.record_variants.items() if len(v) > 1}
        self.by_base = defaultdict(list)  # name without @/# -> keys
        for k in self.records:
            self.by_base[re.split(r"[@#]", k)[0]].append(k)

    def _variants(self, units):
        """Header structs of the same name but different layouts (one per overlay, say): key#<src dir>."""
        sigs, labels = defaultdict(Counter), defaultdict(lambda: defaultdict(Counter))
        for rel, res in units.items():
            parts = Path(rel).parts
            label = parts[parts.index("src") + 1] if "src" in parts[:-1] else "?"
            for k, r in res.get("records", {}).items():
                sig = record_sig(r)
                sigs[k][sig] += 1
                labels[k][sig][label] += 1
        rename = {}
        for k, c in sigs.items():
            if len(c) < 2:
                continue
            used = set()
            for sig, _n in c.most_common():
                lab = labels[k][sig].most_common(1)[0][0]
                nk, i = f"{k}#{lab}", 2
                while nk in used:
                    nk, i = f"{k}#{lab}{i}", i + 1
                used.add(nk)
                rename[(k, sig)] = nk
                self.renamed.setdefault(k, []).append(nk)
        return rename


def record_sig(r):
    return (r["size"], tuple((f[0], f[1]) for f in r["fields"]))


def rekey_unit(res, umap):
    """One unit's result with its record keys renamed (records, nested keys, views, parameters, globals)."""
    def m(k):
        if k in umap:
            return umap[k]
        head, dot, rest = k.partition(".")
        return umap[head] + dot + rest if dot and head in umap else k
    out = dict(res)
    out["records"] = {m(k): r for k, r in res["records"].items()}
    out["functions"] = [dict(f, views=[[m(v[0]), *v[1:]] for v in f["views"]],
                             params=[[m(pr[0]), pr[1]] if pr else None for pr in f["params"]])
                        for f in res["functions"]]
    out["globals"] = [[g[0], m(g[1]), g[2]] for g in res["globals"]]
    return out


def pair_structs(ours, ref):
    votes = defaultdict(Counter)           # our key -> {ref key: votes}
    field_evidence = defaultdict(set)      # (our key, ref key, off) -> {our function names}
    cover_evidence = defaultdict(set)      # same, their access only covers that offset
    kinds = defaultdict(Counter)           # (our key, ref key) -> {"access"/"param"/"global": n}
    paired_funcs = 0
    for addr, of in ours.funcs.items():
        rf = ref.funcs.get(addr)
        if rf is None:
            continue
        paired_funcs += 1
        osets, rsets = defaultdict(set), defaultdict(set)
        for k, off, size, _p, _b in of["views"]:
            osets[k].add((off, size))
        for k, off, size, _p, _b in rf["views"]:
            rsets[k].add((off, size))
        scored = []
        for ok, os_ in osets.items():
            for rk, rs in rsets.items():
                common = os_ & rs
                if common:
                    same_name = re.split(r"[@#]", ok)[0] == re.split(r"[@#]", rk)[0]
                    scored.append((len(common), same_name, ok, rk, common))
        scored.sort(key=lambda s: (-s[0], not s[1], s[2], s[3]))
        used_o, used_r = set(), set()
        for n, _same, ok, rk, common in scored:
            if ok in used_o or rk in used_r:
                continue
            used_o.add(ok)
            used_r.add(rk)
            votes[ok][rk] += n
            kinds[(ok, rk)]["access"] += n
            for off, _size in common:
                field_evidence[(ok, rk, off)].add(of["name"])
            # weaker: their access covers our offset (an array element, or a wider field)
            for off, _size in osets[ok] - common:
                if any(roff <= off < roff + rsize for roff, rsize in rsets[rk]):
                    cover_evidence[(ok, rk, off)].add(of["name"])
        for op, rp in zip(of["params"], rf["params"]):
            if op and rp and op[1] == rp[1]:
                votes[op[0]][rp[0]] += 2
                kinds[(op[0], rp[0])]["param"] += 1
    for addr, (_oname, ok, oshape) in ours.globals.items():
        g = ref.globals.get(addr)
        if g and g[2] == oshape:
            votes[ok][g[1]] += 3
            kinds[(ok, g[1])]["global"] += 1
    return votes, (field_evidence, cover_evidence), kinds, paired_funcs


def elem_record(model, typ):
    """For an array field's type "T[N]...": (element type, its record if known, element count)."""
    m = re.match(r"^(.*?)\s*((?:\[\d+\])+)$", strip_quals(typ))
    if not m:
        return None
    base = m.group(1)
    count = 1
    for d in re.findall(r"\[(\d+)\]", m.group(2)):
        count *= int(d)
    name = re.sub(r"^(?:struct|union) ", "", base)
    return base, [model.records[k] for k in model.by_base.get(name, ())], count


def candidates_at(model, rrec, off, size, prefix="", depth0=0, guard=0):
    """The reference record's fields at `off`: (exact-size matches, other overlapping fields), each
    (depth, path, type, size, starts at off). Array fields are entered: an element (or a field of a struct
    element) at the offset is a candidate `path[i]` / `path[i].x`."""
    exact, other = [], []
    for path, foff, fsize, typ, depth in rrec["flat"]:
        d = depth0 + depth
        full = prefix + path
        if foff == off and fsize == size and size:
            exact.append((d, full, typ, fsize, True))
            continue
        if not fsize or not (foff <= off < foff + fsize):
            continue
        arr = elem_record(model, typ) if guard < 4 else None
        if arr and arr[2]:
            base, erecs, count = arr
            esize = fsize // count
            idx, rel = divmod(off - foff, esize) if esize else (0, 0)
            if rel == 0 and esize == size:
                exact.append((d, f"{full}[{idx}]", base, esize, True))
                continue
            erec = next((r for r in erecs if r["size"] == esize), None)
            if erec:
                e2, o2 = candidates_at(model, erec, rel, size, f"{full}[{idx}].", d, guard + 1)
                if e2:
                    exact.extend(e2)
                    continue
                other.extend(o2)
        other.append((d, full, typ, fsize, foff == off))
    exact.sort()
    # mismatches: a field starting at the offset (outermost first), else the innermost one containing it
    other.sort(key=lambda c: (0, c[0]) if c[4] else (1, -c[0]))
    return exact, other


def hexo(n):
    return f"0x{n:X}"


def find_definitions(root, dirs):
    """Our record/typedef name -> the file that defines it (first match), for grouping the report."""
    out = {}
    pats = [re.compile(r"\}\s*(\w+)\s*;"), re.compile(r"\b(?:struct|union)\s+(\w+)\s*\{")]
    for d in dirs:
        for p in sorted((root / d).rglob("*.[ch]")):
            text = p.read_text(errors="replace")
            for pat in pats:
                for m in pat.finditer(text):
                    out.setdefault(m.group(1), str(p.relative_to(root)))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--ref", default=os.environ.get("DW3_REF_DECOMP"),
                    help="the reference decomp's checkout, built for EU (env DW3_REF_DECOMP)")
    ap.add_argument("--clang", default=os.environ.get("CLANG"))
    ap.add_argument("--out", default=str(ROOT / "build/ref_names"))
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--no-cache", action="store_true")
    args = ap.parse_args()
    if not args.ref:
        ap.error("--ref (or DW3_REF_DECOMP) is required: a checkout of ReGame-Labs/dw3_decomp built for EU")
    ref_root = Path(args.ref).resolve()
    if not (ref_root / "build/eu").is_dir():
        ap.error(f"{ref_root}/build/eu not found: build the reference for EU first")
    clang = args.clang or shutil.which("clang") or str(ROOT / "tools/llvm-mingw/bin/clang")
    if not Path(clang).exists() and not shutil.which(clang):
        ap.error("no clang: put one on PATH, pass --clang, or run scripts/setup.sh llvm-mingw")
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    sides = [our_side(), ref_side(ref_root)]
    if not sides[0].units:
        ap.error("no build/<target>/src/**/*.c.o.i: run scripts/build.sh first")
    models = []
    for side in sides:
        side.load_links()
        units = load_units(side, clang, out / "cache" / side.label, args.jobs, not args.no_cache)
        models.append(Model(side, units))
    ours, ref = models

    write_symbols(ours, ref, out)
    votes, evidence, kinds, paired_funcs = pair_structs(ours, ref)
    write_fields(ours, ref, votes, evidence, kinds, paired_funcs, out)


def write_symbols(ours, ref, out):
    def table(side):
        t = defaultdict(set)
        for target, syms in side.elf_syms.items():
            for name, entries in syms.items():
                for value, sec, typ in entries:
                    if sec == "ABS" or LINKER_NAME.search(name):
                        continue
                    t[(target, value)].add(name)
        return t

    ot, rt = table(ours.side), table(ref.side)
    rows = []
    for key in sorted(ot.keys() & rt.keys()):
        for on in sorted(ot[key]):
            rn = sorted(rt[key])
            rows.append((key[0], key[1], on, rn))
    with open(out / "symbols.tsv", "w") as f:
        f.write("target\taddress\tours\tref\n")
        for target, addr, on, rn in rows:
            f.write(f"{target}\t{addr:08X}\t{on}\t{','.join(rn)}\n")
    used = set()
    for d in ("src", "include"):
        for p in (ROOT / d).rglob("*.[ch]"):
            for line in p.read_text(errors="replace").splitlines():
                if "INCLUDE_ASM" not in line and "INCLUDE_RODATA" not in line:
                    used.update(re.findall(r"\b(?:func|D)_\w+", line))
    remaining = []
    for target, addr, on, rn in rows:
        if on in used and DEFAULT_NAME.match(on):
            named = [n for n in rn if not DEFAULT_NAME.match(n)]
            remaining.append((on, target, addr, named))
    keys = ot.keys() & rt.keys()
    both = sum(1 for k in keys if any(not DEFAULT_NAME.match(n) for n in ot[k])
               and any(not DEFAULT_NAME.match(n) for n in rt[k]))
    only_ref = sum(1 for k in keys if all(DEFAULT_NAME.match(n) for n in ot[k])
                   and any(not DEFAULT_NAME.match(n) for n in rt[k]))
    ours.symbol_stats = (len(ot), len(rt), len(keys), both, only_ref)
    ours.symbol_rows, ours.remaining = rows, remaining


def write_fields(ours, ref, votes, evidences, kinds, paired_funcs, out):
    evidence, cover = evidences
    defs = find_definitions(ROOT, ["include", "src"])
    rows = []
    struct_pairs = {}
    for ok in sorted(ours.records):
        rec = ours.records[ok]
        vs = votes.get(ok)
        if not vs:
            best, alts = None, []
        else:
            # most votes; a tie goes to the reference struct of the same name (DRAWENV, RECT, ...)
            base = re.split(r"[@#]", ok)[0]
            ranked = sorted(vs.items(), key=lambda kv: (-kv[1], re.split(r"[@#]", kv[0])[0] != base, kv[0]))
            best = ranked[0][0]
            alts = [(rk, n) for rk, n in ranked[1:] if n * 2 >= ranked[0][1]]
            struct_pairs[ok] = (best, ranked[0][1], sum(vs.values()), alts)
        rrec = ref.records.get(best) if best else None
        for name, off, size, typ, bits in rec["fields"]:
            src_file = f"src/{ok.split('@')[1].split('.')[0].split('#')[0]}.c" if "@" in ok else ""
            row = {"struct": ok, "file": src_file or defs.get(re.split(r"[@#.]", ok)[0], ""), "offset": off, "size": size,
                   "field": name, "type": typ, "unnamed": bool(UNNAMED_FIELD.match(name or "x_")) or not name,
                   "uses": ours.uses.get((ok, name), 0), "ref_struct": best, "candidates": [], "ref_type": "",
                   "evidence": [], "cover_evidence": [], "flags": []}
            if bits:
                row["flags"].append("bitfield")
            if ok in ours.conflicting_records:
                row["flags"].append("our struct has several layouts")
            if rrec:
                exact, other = candidates_at(ref, rrec, off, size)
                if exact:
                    shallow = [c for c in exact if c[0] == exact[0][0]]
                    row["candidates"] = [c[1] for c in shallow]
                    row["ref_type"] = shallow[0][2]
                    if len(shallow) > 1:
                        row["flags"].append("union: several candidates")
                elif other:
                    c = other[0]
                    inside = [e for e in rrec["flat"] if off <= e[1] < off + size and e[2]]
                    if c[4] and size > (c[3] or 0) and len(inside) > 1:
                        # ours is wider: list their fields inside our range (the outermost ones)
                        md = min(e[4] for e in inside)
                        inside = [e for e in inside if e[4] == md]
                        row["candidates"] = [e[0] for e in inside]
                        row["ref_type"] = inside[0][3]
                        row["flags"].append(f"ours ({size} bytes) spans {len(inside)} of their fields")
                    else:
                        row["candidates"] = [c[1]]
                        row["ref_type"] = c[2]
                        row["flags"].append(f"size mismatch (ours {size}, theirs {c[3]})")
                row["evidence"] = sorted(evidence.get((ok, best, off), ()))
                row["cover_evidence"] = sorted(cover.get((ok, best, off), ()))
                if not row["evidence"]:
                    row["flags"].append("covered by a wider access" if row["cover_evidence"] else "layout only")
                bv, bt = struct_pairs[ok][1], struct_pairs[ok][2]
                if bv < WEAK_VOTES or bv < 0.6 * bt:
                    row["flags"].append(f"weak struct pairing ({bv}/{bt} votes)")
                if alts:
                    row["flags"].append("struct conflict: also " + ", ".join(f"{a} ({n})" for a, n in alts[:3]))
                if best in ref.conflicting_records:
                    row["flags"].append("ref struct has several layouts")
            rows.append(row)

    with open(out / "fields.json", "w") as f:
        json.dump({"struct_pairs": {k: {"ref": v[0], "votes": v[1], "total": v[2],
                                        "alternatives": v[3], "evidence": dict(kinds[(k, v[0])])}
                                    for k, v in struct_pairs.items()},
                   "fields": rows}, f, indent=1)
    with open(out / "fields.tsv", "w") as f:
        f.write("file\tstruct\toffset\tsize\tfield\tuses\tref_struct\tcandidate\tref_type\tevidence\tflags\n")
        for r in rows:
            f.write("\t".join([r["file"], r["struct"], hexo(r["offset"]), str(r["size"]), r["field"] or "-",
                                               str(r["uses"]), r["ref_struct"] or "", "|".join(r["candidates"]), r["ref_type"],
                               ",".join(r["evidence"][:5] or ["~" + n for n in r["cover_evidence"][:5]]),
                               "; ".join(r["flags"])]) + "\n")
    write_report(ours, ref, rows, struct_pairs, kinds, paired_funcs, out)


def write_report(ours, ref, rows, struct_pairs, kinds, paired_funcs, out):
    unk = [r for r in rows if r["unnamed"] and r["field"]]
    unk_used = [r for r in unk if r["uses"]]
    with_cand = [r for r in unk if r["candidates"]]
    conflicts = [r for r in unk if any(f.startswith(("struct conflict", "union", "size mismatch", "weak", "ours ("))
                                       for f in r["flags"])]
    L = []
    L.append("# Reference names: candidates for our unnamed fields and symbols\n")
    L.append("Generated by `tools/ref_names.py` (issue #56) from our build and the reference decomp's EU build. "
             "Candidates are suggestions in the reference's spelling; adopt them in our conventions "
             "(DECISIONS \"Naming conventions\"), and only where our code confirms them.\n")
    L.append("## Summary\n")
    L.append(f"- Units parsed: ours {len(ours.side.units)}, reference {len(ref.side.units)}; "
             f"functions with C on both sides at the same address: {paired_funcs}")
    L.append(f"- Our structs: {len(ours.records)}, paired with a reference struct: {len(struct_pairs)}")
    L.append(f"- Our `unk_`-style fields: {len(unk)} ({len(unk_used)} accessed by C); "
             f"with a candidate: {len(with_cand)} ({sum(1 for r in with_cand if r['uses'])} accessed), "
             f"without: {len(unk) - len(with_cand)}; flagged (weak or conflicting struct pairing, union, size "
             f"mismatch, ours spanning several of theirs): {len(conflicts)}")
    ev = [r for r in with_cand if r["evidence"]]
    cov = [r for r in with_cand if not r["evidence"] and r["cover_evidence"]]
    L.append(f"- Candidates backed by a paired function accessing that offset with that size on both sides: {len(ev)}; "
             f"by their wider access covering it (array element, bigger field): {len(cov)}; layout only (struct "
             f"paired, offset not accessed in a paired function): {len(with_cand) - len(ev) - len(cov)}")
    so, sr, sb, sboth, sref = ours.symbol_stats
    L.append(f"- Symbols (link unit + address): ours {so}, reference {sr}, same address {sb}; named on both sides "
             f"{sboth}; a default name of ours that the reference names: {sref} (all in symbols.tsv)")
    L.append(f"- Remaining `func_`/`D_` names our C uses, at an address the reference names: "
             f"{sum(1 for r in ours.remaining if r[3])} of {len(ours.remaining)}")
    if ours.problems or ref.problems:
        L.append(f"- Parse problems: ours {len(ours.problems)}, reference {len(ref.problems)} (listed at the end)")
    L.append("")
    L.append("## Remaining func_/D_ names\n")
    stage = [r for r in ours.remaining if r[1].startswith("wstag")]
    L.append(f"The WSTAG stage scripts' data (`D_WSTAG<n>_...`, {len(stage)} names in {len({r[1] for r in stage})} "
             f"units, {sum(1 for r in stage if r[3])} with a reference name) are in remaining.tsv; the others:\n")
    L.append("| Ours | Unit | Address | Reference |")
    L.append("|---|---|---|---|")
    for on, target, addr, named in sorted(r for r in ours.remaining if not r[1].startswith("wstag")):
        L.append(f"| `{on}` | {target} | {addr:08X} | {', '.join(f'`{n}`' for n in named) or '-'} |")
    L.append("")
    with open(out / "remaining.tsv", "w") as f:
        f.write("ours\ttarget\taddress\tref\n")
        for on, target, addr, named in sorted(ours.remaining, key=lambda r: (r[1], r[2])):
            f.write(f"{on}\t{target}\t{addr:08X}\t{','.join(named)}\n")
    L.append("## Fields, per struct\n")
    L.append("Structs with at least one `unk_` field, grouped by our defining file. Columns: offset, size, our field, "
             "C uses (member accesses in functions with an address), the candidate (their path at that offset), their "
             "type, evidence (our functions where both sides access that offset and size; `~` = only their wider "
             "access covers it), flags. Only `unk_` fields are listed; fields.tsv has every field.\n")
    by_struct = defaultdict(list)
    for r in rows:
        by_struct[r["struct"]].append(r)
    groups = defaultdict(list)
    for sk, rs in by_struct.items():
        if any(r["unnamed"] and r["field"] for r in rs):
            groups[rs[0]["file"] or "(unknown file)"].append(sk)
    for file in sorted(groups, key=lambda f: -sum(r["uses"] for sk in groups[f] for r in by_struct[sk]
                                                    if r["unnamed"])):
        L.append(f"### {file}\n")
        for sk in sorted(groups[file]):
            rs = by_struct[sk]
            sp = struct_pairs.get(sk)
            rec = ours.records[sk]
            if sp:
                k = kinds[(sk, sp[0])]
                how = ", ".join(f"{v} {n}" for n, v in sorted(k.items()))
                head = f"`{sk}` (0x{rec['size']:X} bytes) -> `{sp[0]}` (votes {sp[1]}/{sp[2]}: {how})"
                if sp[3]:
                    head += "; also " + ", ".join(f"`{a}` ({n})" for a, n in sp[3][:3])
            else:
                head = f"`{sk}` (0x{rec['size']:X} bytes) -> no reference struct"
            L.append(f"#### {head}\n")
            L.append("| Off | Size | Ours | Uses | Candidate | Their type | Evidence | Flags |")
            L.append("|---|---|---|---|---|---|---|---|")
            for r in rs:
                if not (r["unnamed"] and r["field"]):
                    continue
                e = r["evidence"] or r["cover_evidence"]
                evs = ("" if r["evidence"] else "~") * bool(e) + ", ".join(e[:3]) + (f" +{len(e) - 3}" if len(e) > 3 else "")
                L.append(f"| {hexo(r['offset'])} | {r['size']} | `{r['field'] or '(anon)'}` | {r['uses']} | "
                         f"{', '.join(f'`{c}`' for c in r['candidates']) or '-'} | {r['ref_type'] or '-'} | "
                         f"{evs or '-'} | {'; '.join(r['flags'])} |")
            L.append("")
    if ours.problems or ref.problems:
        L.append("## Parse problems\n")
        for p in ours.problems:
            L.append(f"- ours: {p}")
        for p in ref.problems:
            L.append(f"- reference: {p}")
    (out / "report.md").write_text("\n".join(L) + "\n")
    print(f"wrote {out}/report.md, fields.tsv, fields.json, symbols.tsv, remaining.tsv", file=sys.stderr)
    print("\n".join(L[3:12]))


if __name__ == "__main__":
    main()
