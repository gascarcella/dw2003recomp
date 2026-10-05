#!/usr/bin/env python3
"""Move a WSTAG### C unit's .data into its C file (DECISIONS "WSTAG .data in C").

  tools/venv/bin/python tools/wstag_data.py WSTAG201 [...]        # print the C definitions of each file's .data
  tools/venv/bin/python tools/wstag_data.py --survey [WSTAG201 ...] # what each data symbol is typed as (? = unknown)
  tools/venv/bin/python tools/wstag_data.py --apply WSTAG201 [...]  # append them to src/wstag/<n>.c, mark the file
                                                                     # `data` in config/wstag_c.txt
  options: --all (every C unit whose data is still asm), --limit N (the first N of them)

Reads the original bytes (extracted/disc/AAA/PRO/WSTAG###.PRO), splat's data asm (asm/wstag###/data/data.data.s:
labels and relocations, so run it while the file's .data is still asm) and the file's code: the C file's extern
declarations and the stage setup's stores to fieldstg_stage (asm/wstag###/nonmatchings/.../*.s) give the root
types; pointers in typed records type their targets (FieldstgEventDef -> script, FieldstgPlacedActor -> flags and
FieldstgTalk, FieldstgBattleLists -> FieldstgBattleList -> FieldstgListedBattle, ...). Struct layouts come from the
headers (include/fieldstg.h, include/wstag.h, ...) and the C file. Every data label becomes one C definition, in
address order, appended after the file's functions; untyped symbols become u16/s32/pointer arrays by content.
The extern declarations the functions use stay (they now refer to the definitions below them).
"""
import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
import configure  # noqa: E402  (wstag_rows, wstag_c_names)

BASE = configure.TIER2_BASE
PRO = ROOT / "extracted/disc/AAA/PRO"
HEADERS = ["include/psyq/libgte.h", "include/gamestate.h", "include/object.h", "include/fieldstg.h",
           "include/wstag.h"]

SCALAR = {"u8": ("<B", 1), "s8": ("<b", 1), "u16": ("<H", 2), "s16": ("<h", 2), "u32": ("<I", 4), "s32": ("<i", 4),
          "char": ("<b", 1), "short": ("<h", 2), "int": ("<i", 4), "long": ("<i", 4),
          "unsigned char": ("<B", 1), "unsigned short": ("<H", 2), "unsigned int": ("<I", 4),
          "unsigned long": ("<I", 4), "ptr": ("<I", 4)}
SIGNED = {"s8", "s16", "s32", "char", "short", "int", "long"}

# fieldstg_stage (FieldstgStageState) fields the setup stores a data address into: offset -> element type, array.
STAGE_FIELDS = {0x10: ("FieldstgSprite", True), 0x14: ("FieldstgMapEvent", True), 0x20: ("FieldstgBattleLists", False),
                0x24: ("FieldstgEventDef", True), 0x28: ("FieldstgVramPlace", True),
                0x4C: ("FieldstgPlacedActor *", True), 0x38: ("CVECTOR", False)}


def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", " ", t, flags=re.S)
    return re.sub(r"//[^\n]*", " ", t)


class Types:
    """Struct layouts parsed from C: name -> [(field, kind, dims, pointee)]; kind is a scalar, a struct name or
    "ptr" (pointee: the pointed-to type, "func" for function pointers)."""

    def __init__(self, texts):
        self.t, self.alias = {}, {}
        for text in texts:
            self.parse(text)

    def parse(self, text):
        text = strip_comments(text)
        text = re.sub(r"^\s*#.*$", "", text, flags=re.M)
        for m in re.finditer(r"typedef\s+struct\s*(\w*)\s*\{([^{}]*)\}\s*(\w+)\s*;", text, re.S):
            fields = []
            for d in m.group(2).split(";"):
                d = " ".join(d.split())
                if not d:
                    continue
                decls = [d]
                if "," in d and "(" not in d:  # `s16 vx, vy;`
                    first, *rest = [x.strip() for x in d.split(",")]
                    m0 = re.match(r"(.*?)(\**\s*\w+\s*(?:\[\s*\w+\s*\])*)$", first)
                    base = re.sub(r"\*+\s*$", "", m0.group(1)).strip()
                    decls = [first] + [f"{base} {r}" for r in rest]
                for d1 in decls:
                    f = self.field(d1)
                    if f is None:
                        fields = None
                        break
                    fields.append(f)
                if fields is None:
                    break
            if fields is not None:
                self.t[m.group(3)] = fields
                if m.group(1):
                    self.alias["struct " + m.group(1)] = m.group(3)
        for m in re.finditer(r"typedef\s+([\w ]+?)\s+(\w+)\s*;", text):
            self.alias[m.group(2)] = m.group(1).strip()

    @staticmethod
    def field(d):
        if ":" in d:
            return None
        fm = re.match(r".*?\(\s*\*\s*(\w+)\s*((?:\[\w+\])*)\s*\)\s*\(", d)
        if fm:
            return (fm.group(1), "ptr", [int(x, 0) for x in re.findall(r"\[(\w+)\]", fm.group(2))], "func:" + d)
        m = re.match(r"(.*?)(\w+)\s*((?:\[\s*\w+\s*\])*)$", d)
        if not m:
            return None
        typ, name = m.group(1).strip(), m.group(2)
        dims = [int(x, 0) for x in re.findall(r"\[\s*(\w+)\s*\]", m.group(3))]
        typ = typ.replace("const ", "").replace("volatile ", "").strip()
        if "*" in typ:
            pointee = typ[:typ.rindex("*")].strip()
            return (name, "ptr", dims, pointee)
        return (name, typ.replace("signed ", "").strip() if typ != "signed" else "int", dims, None)

    def resolve(self, k):
        for _ in range(10):
            if k in SCALAR or k in self.t or k not in self.alias:
                break
            k = self.alias[k]
        return k

    def known(self, k):
        k = self.resolve(k)
        return k in SCALAR or k in self.t or k.endswith("*")

    def align(self, k):
        k = self.resolve(k)
        if k.endswith("*"):
            return 4
        if k in SCALAR:
            return SCALAR[k][1]
        return max(self.align(f[1]) for f in self.t[k])

    def layout(self, k):
        """[(name, kind, dims, offset, pointee)], size."""
        k = self.resolve(k)
        off, out = 0, []
        for name, kind, dims, pointee in self.t[k]:
            al = self.align(kind)
            off = (off + al - 1) // al * al
            out.append((name, kind, dims, off, pointee))
            n = 1
            for x in dims:
                n *= x
            off += self.size(kind) * n
        al = self.align(k)
        return out, (off + al - 1) // al * al

    def size(self, k):
        k = self.resolve(k)
        if k.endswith("*"):
            return 4
        if k in SCALAR:
            return SCALAR[k][1]
        return self.layout(k)[1]


class Sym:
    def __init__(self, name, addr):
        self.name, self.addr, self.extent = name, addr, 0
        self.type = None      # element type (C type name, "T *" for pointer arrays)
        self.array = True     # array of type (count from the extent) or one object
        self.count = None     # fixed element count (else from the extent)
        self.decl = None      # existing extern declaration (kept as the definition's type)
        self.why = ""
        self.inner = []       # inner dimensions (an extern `T x[][2]`)


class StageData:
    def __init__(self, name):
        self.name, self.n = name, name.lower()
        row = {r[0]: r for r in configure.wstag_rows()}[name]
        self.size, self.text, self.dstart = row[1], row[2], row[3]
        self.bytes = (PRO / f"{name}.PRO").read_bytes()
        self.c_path = ROOT / f"src/wstag/{self.n}.c"
        self.c_text = self.c_path.read_text()
        self.headers = "".join((ROOT / h).read_text() for h in HEADERS)
        self.types = Types([self.headers, self.c_text])
        self.syms, self.relocs = self.parse_data_asm()
        self.by_name = {s.name: s for s in self.syms}
        self.by_addr = {s.addr: s for s in self.syms}

    # ---- input
    def parse_data_asm(self):
        path = ROOT / f"asm/{self.n}/data/data.data.s"
        if not path.exists():
            sys.exit(f"{path} missing: {self.name}'s .data is not asm (already in C?) or configure.py hasn't run")
        syms, relocs = [], {}
        cur = None
        for line in path.read_text().splitlines():
            m = re.match(r"dlabel (\S+)", line)
            if m:
                cur = m.group(1)
                continue
            m = re.match(r"\s*/\* \w+ ([0-9A-F]{8}) ", line)
            if m and cur:  # a label's address: its first datum (spimdisasm prints some words as strings)
                syms.append(Sym(cur, int(m.group(1), 16)))
                cur = None
            m = re.match(r"\s*/\* \w+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+\.word ([A-Za-z_]\w*)(?: \+ (0x[0-9A-Fa-f]+))?$",
                         line)
            if m:
                relocs[int(m.group(1), 16)] = (m.group(2), int(m.group(3) or "0", 16))
        end = BASE + self.size  # splat drops a partial last word; the C covers it
        # A name the C file declares in the dropped bytes (WSTAG925's D_WSTAG925_800A67C4) gets its label back.
        have = {s.name for s in syms}
        for name in re.findall(rf"^extern\s[^;]*?\b(D_{self.name}_([0-9A-F]{{8}}))\b", self.c_text, re.M):
            if name[0] not in have and BASE + self.dstart <= int(name[1], 16) < end:
                syms.append(Sym(name[0], int(name[1], 16)))
        syms.sort(key=lambda s: s.addr)
        for s, nxt in zip(syms, syms[1:] + [None]):
            s.extent = (nxt.addr if nxt else end) - s.addr
        if syms and syms[0].addr != BASE + self.dstart:
            sys.exit(f"{self.name}: .data starts without a label")
        return syms, relocs

    def u(self, a, fmt):
        return struct.unpack_from(fmt, self.bytes, a - BASE)[0]

    def zero(self, a, n):
        return not any(self.bytes[a - BASE:a - BASE + n]) and not any(a <= r < a + n for r in self.relocs)

    # ---- types
    def set_type(self, name, typ, array, why, count=None):
        s = self.by_name.get(name)
        if s is None:
            return False
        if s.type is None:
            s.type, s.array, s.why, s.count = typ, array, why, count
            return True
        if s.type == typ and array and not s.array:  # more evidence: one of several records
            s.array = True
            return True
        return False


    def root_types(self):
        text = strip_comments(self.c_text)
        for m in re.finditer(EXTERN, text, re.M):
            s = self.by_name.get(m.group(3))
            if not s:
                continue
            dims = re.findall(r"\[\s*(\w*)\s*\]", m.group(4))
            s.decl = m.group(0)
            s.type, s.array, s.why = (m.group(1).strip() + " " + m.group(2)).strip(), bool(dims), "extern"
            s.inner = [int(d, 0) for d in dims[1:]]
            if dims and dims[0]:
                s.count = int(dims[0], 0)
        for f in self.asm_functions():
            self.track_stores(f.read_text())
        # The last word of .data: the stage's function table.
        last = self.syms[-1]
        if last.extent == 4 and (last.addr in self.relocs):
            self.set_type(last.name, "WstagFuncs", False, "function table")

    def asm_functions(self):
        """The functions of the file that stay asm (INCLUDE_ASM in the C file)."""
        out = []
        for f in sorted((ROOT / f"asm/{self.n}/nonmatchings").glob("*/*.s")):
            if f"INCLUDE_ASM(\"asm/{self.n}/nonmatchings/{f.parent.name}\", {f.stem})" in self.c_text:
                out.append(f)
        return out

    def track_stores(self, asm):
        """Type the symbols whose addresses the function stores into fieldstg_stage fields: registers hold sets of
        %lo() symbols, merged at labels from the jumps and branches to them (after their delay slot)."""
        regs, at_label, pending, dead, call_7c = {}, {}, None, False, False

        def merge(dst, src):
            for r, v in src.items():
                dst[r] = dst.get(r, set()) | v

        for line in asm.splitlines():
            lm = re.match(r"\s*(\.L\w+):", line)
            if lm:
                cur = {} if dead else {r: set(v) for r, v in regs.items()}
                merge(cur, at_label.get(lm.group(1), {}))
                regs, dead = cur, False
                continue
            m = re.match(r"\s*/\* [^*]+\*/\s+(\w+)\s*(.*)$", line)
            if not m:
                continue
            op, args = m.group(1), m.group(2)
            ops = [x.strip() for x in args.split(",")]
            lo = re.search(r"%lo\(([^)]+)\)", args)
            lw7c = re.match(r"0x7C\(\$(\w+)\)", ops[1]) if op == "lw" and len(ops) > 1 else None
            if op == "addiu" and lo:
                regs[ops[0][1:]] = {lo.group(1).strip()}
            elif op == "addiu" and len(ops) == 3 and ops[1][1:] in regs and re.match(r"-?(0x)?[0-9A-Fa-f]+$", ops[2]):
                # sym + n: the setup picks one of several records (WSTAG340's two FieldstgBattleLists)
                regs[ops[0][1:]] = {f"{v}+{int(ops[2], 0)}" for v in regs[ops[1][1:]]}
            elif lw7c and regs.get(lw7c.group(1)) == {"fieldstg_stage"}:
                regs[ops[0][1:]] = {"@unk_7C"}
            elif op == "jalr" and regs.get(ops[0][1:]) == {"@unk_7C"}:
                call_7c = True  # its arguments are complete after the delay slot
            elif op == "sw":
                sm = re.match(r"(0x[0-9A-Fa-f]+|\d+)?\(\$(\w+)\)", ops[1])
                base = regs.get(sm.group(2), set()) if sm else set()
                if base == {"fieldstg_stage"}:
                    off = int(sm.group(1) or "0", 0)
                    for sym in regs.get(ops[0][1:], set()):
                        if off in STAGE_FIELDS:
                            typ, arr = STAGE_FIELDS[off]
                            sym, _, plus = sym.partition("+")
                            self.set_type(sym, typ, arr or bool(plus), f"fieldstg_stage+0x{off:X}")
            elif ops and ops[0].startswith("$") and op not in ("sw", "sh", "sb", "swl", "swr", "jr", "jalr", "j",
                                                                  "beq", "bne", "beqz", "bnez", "bltz", "bgez",
                                                                  "blez", "bgtz", "mult", "multu", "div", "divu"):
                regs.pop(ops[0][1:], None)
            if call_7c and op != "jalr":
                # fieldstg_stage.unk_7C(entries, id) (func_FIELDSTG_800921E4) searches FieldstgBattleLists by ID
                for sym in regs.get("a0", set()):
                    self.set_type(sym, "FieldstgBattleLists", True, "fieldstg_stage.unk_7C")
                call_7c = False
            if pending is not None:  # this was the delay slot
                target, uncond = pending
                merge(at_label.setdefault(target, {}), regs)
                dead = uncond
                pending = None
            tm = re.search(r"(\.L\w+)$", args)
            if tm and (op in ("j", "b") or op.startswith("b")):
                pending = (tm.group(1), op in ("j", "b"))

    def propagate(self):
        changed = True
        while changed:
            changed = False
            for s in self.syms:
                if s.type is None:
                    continue
                for a, pointee in self.pointer_slots(s):
                    if a not in self.relocs:
                        continue
                    tname, off = self.relocs[a]
                    t = self.by_name.get(tname)
                    if t is None or t.type is not None or pointee is None or pointee.startswith("func") \
                            or pointee == "void" or off:
                        continue
                    pointee = self.types.alias.get(pointee, pointee) if pointee.startswith("struct ") else pointee
                    single = self.types.resolve(pointee) in SINGLE and t.extent < 2 * self.types.size(pointee)
                    changed |= self.set_type(tname, pointee, not single, f"from {s.name}")

    def esize(self, s):
        if s.type == "@funcptr":
            return 4
        n = self.types.size(s.type)
        for x in s.inner:
            n *= x
        return n

    def elements(self, s):
        """(count, element size) of a typed symbol."""
        esz = self.esize(s)
        if not s.array:
            return 1, esz
        if s.count:
            return s.count, esz
        return max(1, s.extent // esz), esz

    def pointer_slots(self, s):
        """(address, pointee) of every pointer in a typed symbol."""
        out = []
        if s.type == "@funcptr":
            return out
        n, esz = self.elements(s)
        inner = 1
        for x in s.inner:
            inner *= x
        step = self.types.size(s.type)
        for i in range(n * inner):
            out += self.slots_of(s.type, s.addr + i * step)
        return out

    def slots_of(self, typ, a):
        typ = self.types.resolve(typ)
        if typ.endswith("*"):
            return [(a, typ[:-1].strip())]
        if typ in SCALAR:
            return []
        lay, _ = self.types.layout(typ)
        out = []
        for name, kind, dims, off, pointee in lay:
            cnt = 1
            for x in dims:
                cnt *= x
            step = self.types.size(kind)
            for i in range(cnt):
                if kind == "ptr":
                    out.append((a + off + i * step, pointee))
                elif kind not in SCALAR:
                    out += self.slots_of(kind, a + off + i * step)
        return out

    def referenced(self):
        """Names the remaining code or data uses: the C file, the asm functions, the relocations."""
        names = set(re.findall(r"\bD_\w+", self.c_text))
        for f in self.asm_functions():
            names |= set(re.findall(r"\bD_\w+", f.read_text()))
        names |= {n for n, _ in self.relocs.values()}
        return names

    def absorb(self):
        """An untyped label nothing names inside a typed array (splat labels each address the code reads, e.g. a
        field of an element, WSTAG805's D_WSTAG805_800A72BA) becomes part of that array."""
        refs, out = self.referenced(), []
        self.alias = {}
        for s in self.syms:
            prev = out[-1] if out else None
            if prev and s.type is None and s.name not in refs and prev.type and prev.array and not prev.count:
                prev.extent += s.extent
                self.alias[s.name] = (prev.name, s.addr - prev.addr)
                continue
            out.append(s)
        self.syms = out
        self.by_name = {s.name: s for s in self.syms}

    def fallback(self, s):
        """Type an unreferenced-by-type symbol by its content."""
        words = list(range(s.addr, s.addr + s.extent - 3, 4))
        rel = [a for a in words if a in self.relocs]
        if s.extent == 4 and rel and self.relocs[s.addr][0].startswith("func_"):
            # a function pointer variable (WSTAG210's fade pair, read one by one)
            s.type, s.array, s.why = "@funcptr", False, "content"
            return
        if rel and s.extent % 4 == 0 and all(a in self.relocs or self.u(a, "<I") == 0 for a in words):
            kinds = {self.target_type(*self.relocs[a]) for a in rel}
            kind = kinds.pop() if len(kinds) == 1 else None
            s.type = f"{kind} *" if kind and not kind.startswith("func") else "void *"
        elif rel:
            raise Gen(f"{s.name}: words and pointers mixed (needs a type)")
        elif s.extent % 4 == 0:
            s.type = "s32"
        else:
            s.type = "s16" if s.extent % 2 == 0 else "u8"
        s.array, s.why = True, "content"

    def target_type(self, name, off):
        name, off = self.resolve_alias(name, off)
        t = self.by_name.get(name)
        if t is None:
            return "func" if name.startswith("func_") else None
        if t.inner or off % self.esize(t):
            return None
        return t.type

    def resolve_alias(self, name, off):
        if name in self.alias:
            base, delta = self.alias[name]
            return base, off + delta
        return name, off

    def infer(self):
        self.root_types()
        self.propagate()
        self.absorb()
        for s in self.syms:
            if s.type is None:
                self.fallback(s)
        self.propagate()

    # ---- C
    def layout_objects(self):
        """[(sym, count or None, size)]: every symbol as an object that GCC places 4-aligned (arrays and structs:
        mips DATA_ALIGNMENT), with leftover bytes as an extra object (an unreferenced word)."""
        out = []
        for s in list(self.syms):
            if s.addr % 4:
                raise Gen(f"{s.name}: label not 4-aligned")
            n, esz = self.elements(s)
            size = n * esz
            rest = s.extent - size
            if rest < 0:
                raise Gen(f"{s.name}: {s.type} x {n} = 0x{size:X} > extent 0x{s.extent:X}")
            if self.types.resolve(s.type) in SCALAR and not s.array and rest:
                raise Gen(f"{s.name}: scalar {s.type} with 0x{rest:X} bytes after it")
            if rest >= 4 or (rest and not self.zero(s.addr + size, rest)):
                if not s.array or s.count:
                    raise Gen(f"{s.name}: 0x{rest:X} bytes after {s.type} x {n}")
                extra = Sym(f"D_{self.name}_{s.addr + size:08X}", s.addr + size)
                extra.extent = rest
                self.fallback(extra)
                s.extent = size
                out.append((s, n if s.array else None, size))
                idx = self.syms.index(s)
                self.syms.insert(idx + 1, extra)
                self.by_name[extra.name] = extra
                n2, e2 = self.elements(extra)
                out.append((extra, n2, n2 * e2))
                continue
            out.append((s, n if s.array else None, size))
        return out

    def scalar(self, kind, a, hexa):
        kind = self.types.resolve(kind)
        fmt, sz = SCALAR[kind]
        if a in self.relocs:
            raise Gen(f"pointer at 0x{a:08X} in a {kind}")
        v = struct.unpack_from(fmt, self.bytes, a - BASE)[0]
        if kind in SIGNED and not hexa:
            return f"0x{v & 0xFFFFFFFF:08X}" if sz == 4 and abs(v) > 0xFFFF else str(v)
        if v < 0:
            return str(v)
        return f"0x{v:X}" if v > 9 else str(v)

    def pointer(self, a, pointee):
        v = self.u(a, "<I")
        if a not in self.relocs:
            if v == 0:
                return "NULL"
            raise Gen(f"pointer value 0x{v:08X} at 0x{a:08X} without a relocation")
        name, off = self.resolve_alias(*self.relocs[a])
        t = self.by_name.get(name)
        if t is None:
            if off:
                raise Gen(f"pointer {name} + 0x{off:X}")
            if name.startswith("func_") or (pointee and pointee.startswith("func:")):
                if pointee and pointee.startswith("func:"):
                    if not self.declared(name):
                        self.protos[name] = pointee[5:]
                    else:  # a function whose own type differs from the field's (the callers'): cast, as FIELDSTG
                        field = signature(pointee[5:])
                        own = self.function_type(name)
                        if field and own and normalize(own) != normalize(field):
                            cast = f"({field[0]} (*)({field[1]}))"
                            self.casts.append((name, cast))
                            return cast + name
                return name
            raise Gen(f"pointer to {name} (not in this file's .data)")
        if t.array:
            esz = self.esize(t)
            k, r = divmod(off, esz)
            if r:
                raise Gen(f"pointer into an element: {name} + 0x{off:X}")
            expr, typ = (name if k == 0 else f"&{name}[{k}]"), t.type
        else:
            if off:
                raise Gen(f"pointer {name} + 0x{off:X}")
            expr, typ = f"&{name}", t.type
        if t.inner:
            typ = None
        self.uses.append(name)
        if pointee in (None, "void") or (typ and self.types.resolve(typ) == self.types.resolve(pointee)):
            return expr
        self.casts.append((expr, pointee))
        return f"({pointee} *){expr}"

    def declared(self, name):
        """A function defined or declared in the C file (not just INCLUDE_ASM or a NON_MATCHING draft)."""
        plain = re.sub(r"^#ifdef NON_MATCHING\n.*?^#else\n", "", self.c_text, flags=re.M | re.S)
        for line in (plain + self.headers).splitlines():
            if re.search(rf"\b{name}\s*\(", line) and "INCLUDE_ASM" not in line:
                return True
        return False

    def value(self, kind, dims, a, hexa=False, pointee=None):
        kind = self.types.resolve(kind)
        if dims:
            step = self.types.size(kind)
            for x in dims[1:]:
                step *= x
            return "{ " + ", ".join(self.value(kind, dims[1:], a + i * step, hexa, pointee)
                                    for i in range(dims[0])) + " }"
        if kind.endswith("*"):
            return self.pointer(a, kind[:-1].strip())
        if kind == "ptr":
            return self.pointer(a, pointee)
        if kind in SCALAR:
            return self.scalar(kind, a, hexa)
        lay, size = self.types.layout(kind)
        parts, prev = [], a
        for fname, fk, fd, fo, fp in lay:
            if not self.zero(prev, a + fo - prev):
                raise Gen(f"non-zero padding in {kind} at 0x{prev:08X}")
            n = self.types.size(fk)
            for x in fd:
                n *= x
            parts.append(self.value(fk, fd, a + fo, hexa, fp))
            prev = a + fo + n
        if not self.zero(prev, a + size - prev):
            raise Gen(f"non-zero tail padding in {kind} at 0x{prev:08X}")
        return "{ " + ", ".join(parts) + " }"

    def function_type(self, name):
        """(return type, parameters) of a function defined or declared in the C file, else None."""
        m = re.search(rf"^(\w[\w \t*]*?)\s*\b{name}\s*\(([^)]*)\)", strip_comments(self.c_text), re.M)
        return (m.group(1).strip(), m.group(2).strip()) if m else None

    def definition(self, s, n):
        if s.type == "@funcptr":
            func = self.relocs[s.addr][0]
            ret, params = self.function_type(func) or ("void", "")
            star = "*" if ret.endswith("*") else ""
            return f"{ret.rstrip('*').strip()} {star}(*{s.name})({params}) = {func};\n"
        hexa = s.why.startswith("from") and self.types.resolve(s.type) in ("s16", "u16")
        inner = "".join(f"[{x}]" for x in s.inner)
        typ = s.type
        star = ""
        if typ.endswith("*"):
            typ, star = typ[:-1].strip(), "*"
        declarator = f"{typ} {star}{s.name}" + (f"[{n}]" if n is not None else "") + inner
        if n is None:
            init = self.value(s.type, [], s.addr, hexa)
            if len(declarator) + len(init) + 4 <= 116 or not init.startswith("{"):
                return f"{declarator} = {init};\n"
            items = top_level_items(init)
        else:
            esz = self.esize(s)
            items = [self.value(s.type, s.inner, s.addr + i * esz, hexa) for i in range(n)]
        if n is not None and self.types.resolve(s.type) in SCALAR and not s.inner:
            w = 16 if self.types.size(s.type) == 1 else 8
            rows = [", ".join(items[i:i + w]) + "," for i in range(0, len(items), w)]
        else:
            rows, cur = [], ""
            for it in items:
                if cur and len(cur) + len(it) + 2 > 112:
                    rows.append(cur)
                    cur = ""
                cur = (cur + " " if cur else "") + it + ","
            rows.append(cur)
            wrapped = []
            for r in rows:  # one item longer than a line: break it after commas
                while len(r) > 112:
                    cut = r.rfind(", ", 0, 112)
                    wrapped.append(r[:cut + 1])
                    r = "    " + r[cut + 2:]
                wrapped.append(r)
            rows = wrapped
        one = f"{declarator} = {{ " + " ".join(rows).rstrip(",") + " };"
        if len(rows) == 1 and len(one) <= 116:
            return one + "\n"
        return f"{declarator} = {{\n" + "\n".join("    " + r for r in rows) + "\n};\n"

    def generate(self):
        """The C text appended to the file: prototypes, forward declarations, the definitions."""
        self.infer()
        self.protos, self.casts = {}, []
        objs = self.layout_objects()
        defs, defined, forward = [], set(), []
        declared = set(re.findall(r"^extern\s[^;]*?\b(D_\w+)", strip_comments(self.c_text), re.M))
        for s, n in [(o[0], o[1]) for o in objs]:
            self.uses = []
            defs.append(self.definition(s, n))
            for u in self.uses:
                if u not in defined and u not in declared and u != s.name and u not in [f[0] for f in forward]:
                    forward.append((u, self.by_name[u]))
            defined.add(s.name)
        end = BASE + self.size
        last = objs[-1]
        if last[0].addr + last[0].extent != end:
            raise Gen("objects don't end at the end of the file")
        out = ["\n/* The stage's .data (tools/wstag_data.py). */\n"]
        for name, decl in self.protos.items():
            out.append(re.sub(r"\(\s*\*\s*\w+\s*\)", name, decl, count=1) + ";\n")
        for name, s in forward:
            typ, star = (s.type[:-1].strip(), "*") if s.type.endswith("*") else (s.type, "")
            arr = "[]" if s.array else ""
            out.append(f"extern {typ} {star}{name}{arr}{''.join(f'[{x}]' for x in s.inner)};\n")
        if self.protos or forward:
            out.append("\n")
        out += defs
        return "".join(out)


def signature(decl):
    """(return type, parameters) of a function pointer declaration `T (*name)(params)`."""
    m = re.match(r"\s*(.*?)\s*\(\s*\*\s*\w+\s*\)\s*\((.*)\)\s*$", decl)
    return (m.group(1).strip(), m.group(2).strip()) if m else None


def normalize(sig):
    """A signature without parameter names or spacing."""
    ret, params = sig
    out = []
    for p in params.split(","):
        p = " ".join(p.split())
        m = re.match(r"(.*?[\w*])\s*\b(?!void\b|s8\b|u8\b|s16\b|u16\b|s32\b|u32\b|int\b)\w+$", p)
        out.append((m.group(1) if m and m.group(1).strip() else p).replace(" *", "*"))
    return " ".join(ret.split()).replace(" *", "*"), ",".join(out)


def top_level_items(init):
    """The items of a brace initializer `{ a, { b, c }, d }`: a, { b, c }, d."""
    items, depth, cur = [], 0, ""
    for ch in init.strip()[1:-1].strip():
        if ch == "," and depth == 0:
            items.append(cur.strip())
            cur = ""
            continue
        depth += {"{": 1, "}": -1}.get(ch, 0)
        cur += ch
    if cur.strip():
        items.append(cur.strip())
    return items


EXTERN = r"^extern\s+([\w ]+?)\s*(\**)\s*(D_\w+)\s*((?:\[\s*\w*\s*\])*)\s*;"
# Record types a pointer usually points to one of (a single object when its extent holds one).
SINGLE = {"FieldstgBattleList", "FieldstgListedBattle", "FieldstgPlacedActor", "FieldstgBattleLists", "WstagExit", "WstagExits"}


class Gen(Exception):
    pass


def survey(names):
    tot = 0
    for name in names:
        d = StageData(name)
        try:
            d.infer()
            d.layout_objects()
        except Gen as e:
            print(f"{name}: {e}")
            continue
        for s in d.syms:
            tot += 1
            if s.why == "content":
                first = d.bytes[s.addr - BASE:s.addr - BASE + min(s.extent, 16)].hex()
                print(f"{name} {s.name} ext=0x{s.extent:X} {s.type} {first}")


def mark_data(names):
    path = ROOT / configure.WSTAG_C
    lines = path.read_text().splitlines()
    for i, ln in enumerate(lines):
        f = ln.split("#")[0].split()
        if f and f[0] in names and "data" not in f[1:]:
            lines[i] = ln.replace(f[0], f"{f[0]} data", 1)
    path.write_text("\n".join(lines) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*")
    ap.add_argument("--survey", action="store_true")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--limit", type=int)
    args = ap.parse_args()
    in_c = configure.wstag_c_data_names()
    names = args.files
    if args.all:
        names = [n for n in configure.wstag_c_names() if n not in in_c]
    names = [n.upper() for n in names]
    if args.limit:
        names = names[:args.limit]
    if args.survey:
        survey(names)
        return
    done, failed = [], []
    for name in names:
        if name in in_c:
            print(f"{name}: its .data is already in C", file=sys.stderr)
            continue
        d = StageData(name)
        try:
            text = d.generate()
        except Gen as e:
            print(f"{name}: {e}", file=sys.stderr)
            failed.append(name)
            continue
        if args.apply:
            d.c_path.write_text(d.c_text.rstrip("\n") + "\n" + text)
            done.append(name)
        else:
            print(f"// ---- {name}\n{text}")
        for expr, cast in d.casts:
            print(f"{name}: cast {cast if cast.startswith('(') else '(' + cast + ' *)'}{expr}", file=sys.stderr)
    if done:
        mark_data(set(done))
        # Their new configs (the .data subsegment of the C file), split, build.ninja (like wstag_groups.py --add).
        configure.write_wstag_configs()
        for t in configure.WSTAG_TARGETS:
            if t.name.upper() in done:
                configure.split(t, quiet=True)
        subprocess.run([configure.PYTHON, "configure.py", "--no-split"], cwd=ROOT, check=True,
                       stdout=subprocess.DEVNULL)
        print(f"{len(done)} file(s) done; {len(failed)} failed: {' '.join(failed)}", file=sys.stderr)


if __name__ == "__main__":
    main()
