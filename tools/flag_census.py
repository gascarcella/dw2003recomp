#!/usr/bin/env python3
"""Census of every gamestate flag word the game reads or writes (gamestate_get_flag / gamestate_set_flag).

  tools/venv/bin/python tools/flag_census.py               # per flag type: uses on the get and the set side
  tools/venv/bin/python tools/flag_census.py --type 0x72   # also list every use of that type, with its index
  tools/venv/bin/python tools/flag_census.py --json        # the same as JSON
  tools/venv/bin/python tools/flag_census.py --check       # every bit-array flag's index inside its array (one line)

A flag word is u16: type = (flag >> 8) & ~1, index = flag & 0x1FF (src/main/gamestate.c). The matching build is
byte-identical and all of its data is C, so the census reads the C (src/, comments stripped) instead of the disc:
- flag lists (u16 (flag, value) pairs ended by 0xFFFF) that FieldstgPlacedActor.flags_required and
  FieldstgTalk.flags_required (gamestate_check_flags: get side) and FieldstgTalk.flags_set (gamestate_set_flags) point at;
- FieldstgMapEvent.flag/flag_2 (get side, 0xFFFF = none) and FieldstgFlagEvent.flag_0/flag_1 (get side);
- calls gamestate_flags.get_flag/set_flag/check_flags/set_flags with a constant first argument. A call with a computed
  argument must be one of KNOWN_COMPUTED (each is accounted for there), else the census fails.
Checks that make it complete rather than a sample: every fieldstg_stage.actors/map_events root, every placed actor's
talk list and every list pointer must resolve to a parsed definition; every integer literal (hex or decimal) in
src/ and include/ whose value has the --type's type must be one of the uses counted, or is printed as unexplained; and
the residual asm (INCLUDE_ASM / INCLUDE_RODATA bodies, the `data` segments; needs configure.py's asm/) is swept the same.
"""
import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Calls whose flag argument is computed, and why each cannot carry another type than noted.
KNOWN_COMPUTED = {
    "fieldstg_flag_events[i].flag_0": "FieldstgFlagEvent data (counted)",
    "fieldstg_flag_events[i].flag_1": "FieldstgFlagEvent data (counted)",
    "obj->event->flag": "FieldstgMapEvent data (counted)",
    "obj->event->flag_2": "FieldstgMapEvent data (counted)",
    "placed->flags_required": "FieldstgPlacedActor lists (counted)",
    "talk->flags_required": "FieldstgTalk lists (counted)",
    "obj->talk_flags": "FieldstgTalk.flags_set lists (counted)",
    "gamestate_data.funcs.get_map() + 0x1E00": "set side only (the visited-map bit: type 0x20 for the field maps 0x200-0x2EE)",
    "(i & 0xFF) | 0x2000": "get side, type 0x20 only (STSTATUS)",
}
CALL = re.compile(r"gamestate_flags\.(get_flag|set_flag|check_flags|set_flags)\s*\(")
DEF = re.compile(r"^[ \t]*(?:static\s+)?(?:const\s+)?([A-Za-z_]\w*)\s*(\*?)\s*([A-Za-z_]\w*)\s*((?:\[[^\]]*\])*)\s*=\s*\{",
                 re.M)
INT = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)[uUlL]*(?![\w.])")
SIDE = {"get_flag": "get", "check_flags": "get", "set_flag": "set", "set_flags": "set"}


def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group()), t, flags=re.S)  # keep offsets
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group()), t)


def balanced(t, i, open_="{", close="}"):
    """t[i] is just after an opening bracket: return the index just after its match."""
    depth = 1
    while depth:
        depth += (t[i] == open_) - (t[i] == close)
        i += 1
    return i


def split_top(s, start):
    """Split s at top-level commas: [(text, absolute offset)]."""
    out, depth, cur = [], 0, start
    for k, c in enumerate(s):
        if c in "({":
            depth += 1
        elif c in ")}":
            depth -= 1
        elif c == "," and depth == 0:
            out.append((s[cur - start:k], cur))
            cur = start + k + 1
    out.append((s[cur - start:], cur))
    return [(x, o + len(x) - len(x.lstrip())) for x, o in out if x.strip()]


def groups(s, start):
    """The brace groups of an array initializer: [(inner text, absolute offset)]."""
    out, k = [], 0
    while True:
        k = s.find("{", k)
        if k < 0:
            return out
        e = balanced(s, k + 1)
        out.append((s[k + 1:e - 1], start + k + 1))
        k = e


def ival(x):
    return int(x.strip().rstrip("uUlL"), 0)


class Census:
    def __init__(self):
        self.defs = {}  # name -> (type, is_ptr, dims, file, inner text, inner offset)
        self.texts = {}
        self.uses = []  # dicts: type, index, flag, value, side, source, file, where
        self.counted = set()  # (file, offset) of every flag literal counted
        self.problems = []

    def load(self):
        for f in sorted((ROOT / "src").rglob("*.c")):
            rel = str(f.relative_to(ROOT))
            t = strip_comments(f.read_text(errors="replace"))
            self.texts[rel] = t
            for m in DEF.finditer(t):
                e = balanced(t, m.end())
                self.defs[m.group(3)] = (m.group(1), m.group(2), m.group(4), rel, t[m.end():e - 1], m.end())

    def add(self, flag, value, side, source, file, off, where):
        if (file, off) in self.counted:  # a list that several talks share is one use
            return
        self.uses.append({"type": (flag >> 8) & ~1, "index": flag & 0x1FF, "flag": flag, "value": value,
                          "side": side, "source": source, "file": file, "where": where})
        self.counted.add((file, off))

    def walk_list(self, ref, side, source, where):
        """A u16 * initializer (NULL, name, &name[k], name + k): the (flag, value) pairs up to 0xFFFF."""
        ref = ref.strip()
        if ref in ("NULL", "0"):
            return
        m = re.fullmatch(r"(?:\(\s*u16\s*\*\s*\)\s*)?&?\s*(\w+)\s*(?:\[\s*(\w+)\s*\]|\+\s*(\w+))?", ref)
        if not m or m.group(1) not in self.defs or self.defs[m.group(1)][0] not in ("u16", "s16"):
            self.problems.append(f"{where}: flag list {ref!r} does not resolve to a u16 array")
            return
        name = m.group(1)
        skip = ival(m.group(2) or m.group(3) or "0")
        _, _, _, file, inner, off = self.defs[name]
        words = [(ival(x), o) for x, o in split_top(inner, off)][skip:]
        for k in range(0, len(words), 2):
            flag, foff = words[k]
            if flag == 0xFFFF:
                return
            if k + 1 >= len(words):
                break
            self.add(flag, words[k + 1][0], side, source, file, foff, f"{name}[{skip + k}]")
        self.problems.append(f"{where}: flag list {name} has no 0xFFFF end inside its definition")

    def scan_data(self):
        talks_seen = set()
        for name, (typ, ptr, dims, file, inner, off) in self.defs.items():
            elems = groups(inner, off) if dims else [(inner, off)]
            if typ == "FieldstgTalk" and not ptr:
                talks_seen.add(name)
                for k, (e, eo) in enumerate(elems):
                    f = split_top(e, eo)
                    self.walk_list(f[0][0], "get", "FieldstgTalk.flags_required", f"{name}[{k}]")
                    self.walk_list(f[1][0], "set", "FieldstgTalk.flags_set", f"{name}[{k}]")
            elif typ == "FieldstgPlacedActor" and not ptr:
                for k, (e, eo) in enumerate(elems):
                    f = split_top(e, eo)
                    self.walk_list(f[0][0], "get", "FieldstgPlacedActor.flags_required", f"{name}[{k}]")
                    talks = f[1][0].strip()
                    if talks != "NULL" and (talks not in self.defs or self.defs[talks][0] != "FieldstgTalk"):
                        self.problems.append(f"{name}: talk list {talks!r} is not a FieldstgTalk definition")
            elif typ == "FieldstgMapEvent" and not ptr:
                for k, (e, eo) in enumerate(elems):
                    f = split_top(e, eo)
                    if ival(f[4][0]) == 0:  # type 0 ends the list
                        continue
                    for fi in (0, 2):
                        flag = ival(f[fi][0])
                        if flag != 0xFFFF:
                            self.add(flag, ival(f[fi + 1][0]), "get", "FieldstgMapEvent", file, f[fi][1],
                                     f"{name}[{k}]")
            elif typ == "FieldstgFlagEvent" and not ptr:
                for k, (e, eo) in enumerate(elems):
                    f = split_top(e, eo)
                    if ival(f[0][0]) == -1:
                        continue
                    for fi, value in ((1, 0), (2, 1)):
                        self.add(ival(f[fi][0]), value, "get", "FieldstgFlagEvent", file, f[fi][1], f"{name}[{k}]")
        # Roots: what the stage setups give FIELDSTG.
        for file, t in self.texts.items():
            for m in re.finditer(r"fieldstg_stage\.(map_events|actors)\s*=\s*([^;]+);", t):
                want = {"map_events": ("FieldstgMapEvent", ""), "actors": ("FieldstgPlacedActor", "*")}[m.group(1)]
                for name in re.findall(r"[A-Za-z_]\w*", m.group(2)):
                    if name == "NULL":
                        continue
                    d = self.defs.get(name)
                    if not d or (d[0], d[1]) != want:
                        self.problems.append(f"{file}: fieldstg_stage.{m.group(1)} = {name}: not a parsed {want}")
                    elif m.group(1) == "actors":
                        for x, _ in split_top(d[4], d[5]):
                            x = re.sub(r"\s*\[.*$", "", x.strip().lstrip("&").strip())
                            if x not in ("NULL", "0") and (x not in self.defs or self.defs[x][0] != "FieldstgPlacedActor"):
                                self.problems.append(f"{name}: actor {x!r} is not a FieldstgPlacedActor definition")

    def scan_code(self):
        for file, t in self.texts.items():
            for m in CALL.finditer(t):
                e = balanced(t, m.end(), "(", ")")
                args = split_top(t[m.end():e - 1], m.end())
                a0, o0 = re.sub(r"\s+", " ", args[0][0]).strip(), args[0][1]
                fn = m.group(1)
                if fn in ("get_flag", "set_flag") and re.fullmatch(r"0[xX][0-9A-Fa-f]+|\d+", a0):
                    value = args[1][0].strip()
                    value = ival(value) if re.fullmatch(r"0[xX][0-9A-Fa-f]+|\d+", value) else value
                    line = t.count("\n", 0, o0) + 1
                    self.add(ival(a0), value, SIDE[fn], f"code {fn}", file, o0, f"{file}:{line}")
                elif a0 not in KNOWN_COMPUTED:
                    self.problems.append(f"{file}: gamestate_flags.{fn}({a0}, ...): computed flag not in KNOWN_COMPUTED")

    def sweep(self, ftype):
        """Integer literals of type ftype that the census did not count (C sources, headers, residual asm)."""
        lo, hi = ftype << 8, (ftype << 8) | 0x1FF

        def hit(v):  # the value, or either half of a 32-bit word (a list typed as s32 by content)
            return lo <= v <= hi or (v > 0xFFFF and (lo <= (v & 0xFFFF) <= hi or lo <= (v >> 16 & 0xFFFF) <= hi))
        out = []
        files = dict(self.texts)
        for f in sorted((ROOT / "include").rglob("*.h")):
            files[str(f.relative_to(ROOT))] = strip_comments(f.read_text(errors="replace"))
        for file, t in files.items():
            for m in INT.finditer(t):
                v = ival(m.group(1))
                if hit(v) and (file, m.start()) not in self.counted:
                    line = t.count("\n", 0, m.start()) + 1
                    out.append(f"{file}:{line}: {m.group(1)}  | {t.splitlines()[line - 1].strip()[:100]}")
        asm, n = self.residual_asm()
        for path in asm:
            for k, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
                code = line.split("*/", 1)[-1] if "*/" in line else line
                for m in INT.finditer(code):
                    v = ival(m.group(1))
                    if hit(v):
                        out.append(f"{path.relative_to(ROOT)}:{k}: {m.group(1)}  | {code.strip()[:100]}")
        return out, n

    def residual_asm(self):
        paths = []
        for file, t in self.texts.items():
            for m in re.finditer(r'INCLUDE_(?:ASM|RODATA)\(\s*"([^"]+)"\s*,\s*(\w+)\s*\)', t):
                paths.append(ROOT / m.group(1) / f"{m.group(2)}.s")
        for cfg in sorted((ROOT / "config").glob("*.yaml")):
            for m in re.finditer(r"- \[0x[0-9A-Fa-f]+,\s*data,\s*([\w/]+)\]", cfg.read_text()):
                if not m.group(1).startswith("psyq/"):
                    paths += list((ROOT / "asm" / cfg.stem / "data").glob(f"{Path(m.group(1)).name}*.s"))
        missing = [p for p in paths if not p.exists()]
        if missing:
            self.problems.append(f"{len(missing)} residual asm files missing (run configure.py first), "
                                 f"e.g. {missing[0].relative_to(ROOT)}")
        return [p for p in paths if p.exists()], len(paths)


def check_ranges(c):
    """Flag types that gamestate_get_flag/set_flag keep as bit arrays (gamestate_flags.map_flags for type 0,
    GamestateData.flags for 0x02, flags_XX for 0xXX; sizes from include/gamestate.h): every counted use's index must
    lie inside its array, else the game reads or writes the next array (part of the save, docs/FORMATS.md "Save data").
    Returns the number of violations."""
    h = strip_comments((ROOT / "include/gamestate.h").read_text())
    size = {0x00 if m.group(1) == "map_" else 0x02 if not m.group(2) else int(m.group(2), 16): int(m.group(3), 0) * 8
            for m in re.finditer(r"\bu8\s+(map_)?flags(?:_([0-9A-F]{2}))?\s*\[\s*(\w+)\s*\]", h)}
    top, bad = {}, 0
    for u in c.uses:
        if u["type"] in size:
            top[u["type"]] = max(top.get(u["type"], -1), u["index"])
            if u["index"] >= size[u["type"]]:
                bad += 1
                print(f"  0x{u['flag']:04X}: index {u['index']} >= {size[u['type']]} bits  {u['source']} "
                      f"{u['file']} {u['where']}")
    n = sum(u["type"] in size for u in c.uses)
    edges = ", ".join(f"0x{t:02X} {i}/{size[t]}" for t, i in sorted(top.items()))
    print(f"{n} uses of {len(top)} bit-array flag types (of {len(size)} arrays) inside their arrays, {bad} past the end "
          f"(highest index/bits: {edges})")
    for p in c.problems:
        print("PROBLEM:", p)
    return bad + len(c.problems)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--type", type=lambda x: int(x, 0), help="list every use of this flag type (e.g. 0x72)")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--check", action="store_true", help="only check the bit-array flags' ranges (tests/formats)")
    args = ap.parse_args()
    c = Census()
    c.load()
    c.scan_data()
    c.scan_code()
    if args.check:
        return 1 if check_ranges(c) else 0
    per = defaultdict(Counter)
    for u in c.uses:
        per[u["type"]][(u["side"], u["source"])] += 1
    unexplained, nasm = c.sweep(args.type) if args.type is not None else ([], 0)
    if args.json:
        json.dump({"uses": c.uses, "problems": c.problems, "unexplained_literals": unexplained}, sys.stdout, indent=1)
        print()
        return 1 if c.problems else 0
    print(f"{len(c.uses)} flag uses ({sum(u['side'] == 'get' for u in c.uses)} get, "
          f"{sum(u['side'] == 'set' for u in c.uses)} set) in {len(c.defs)} parsed definitions")
    print(f"{'type':>6} {'get':>5} {'set':>5}  sources")
    for t in sorted(per):
        g = sum(v for (s, _), v in per[t].items() if s == "get")
        s_ = sum(v for (s, _), v in per[t].items() if s == "set")
        src = ", ".join(f"{k[1]} {k[0]} {v}" for k, v in sorted(per[t].items()))
        print(f"  0x{t:02X} {g:5} {s_:5}  {src}")
    if args.type is not None:
        sel = [u for u in c.uses if u["type"] == args.type]
        print(f"\ntype 0x{args.type:02X}: {len(sel)} uses, indexes {sorted({u['index'] for u in sel})}")
        for u in sel:
            print(f"  {u['side']} 0x{u['flag']:04X} index {u['index']:3} value {u['value']}  {u['source']}  "
                  f"{u['file']} {u['where']}")
        print(f"literals of type 0x{args.type:02X} not counted above (C, headers, {nasm} residual asm files): "
              f"{len(unexplained)}")
        for x in unexplained:
            print("  " + x)
    for p in c.problems:
        print("PROBLEM:", p)
    return 1 if c.problems else 0


if __name__ == "__main__":
    sys.exit(main())
