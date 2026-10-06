#!/usr/bin/env python3
"""Generators for the PC port's build (port/CMakeLists.txt runs them; docs/PC_PORT_PLAN.md 2.5, "Build-system changes").

  tools/port_gen.py units --out build/port/gen/units.cmake          # the game's C units, as CMake variables
  tools/port_gen.py overrides --out build/port/gen/include            # include_asm.h and psyq/gtemac.h for the host
  tools/port_gen.py ldscript --out build/port/gen/overlays.ld         # per-overlay .data/.bss sections, the arena symbols
  tools/port_gen.py tables --nm nm --objects objs.rsp --out build/port/gen/overlay_tables.c   # address -> function
  tools/port_gen.py state --nm nm --cc cc --gen-include build/port/gen/include --objects objs.rsp \
      --out build/port/gen/port_state_tables.c     # the EXE's functions and data symbols (port/src/state.c)

Everything comes from tracked sources only (no disc, no splat output), so the port configures from a fresh clone:
  - the unit list: src/<target>/*.c for the EXE and the tier-1/tier-2 overlays (the same set configure.py compiles:
    every C file under src/ is a unit of its directory's target), and src/wstag/<unit>.c for the WSTAG files in
    config/wstag_c.txt (configure.py's wstag_c_units; WSTAG260 is data-only and has no unit);
  - the file IDs: tier 1 from overlay_files in src/main/overlay.c, WFIGHTMN/WFIGHTTS from fightstg's load_file
    calls (0x208/0x209, docs/DISC_LAYOUT.md), the WSTAG files from FIELDSTG's stage tables (fieldstg_stages,
    fieldstg_stages_2d: `{ stage, file, WSTAG_ENTRY(entry) }`) joined with config/wstag.txt (name, table, stage, entry);
  - the addresses: `type:func` lines of config/<overlay>.symbols.txt and config/wstag/<n>.symbols.txt;
  - which of those functions exist on the host: `nm` of the compiled objects (a `static` function, or one still
    INCLUDE_ASM, has no global symbol and cannot be in a table: `tables` prints how many were skipped and why).
`tables` also checks every tag site in the C (WSTAG_ENTRY, OVERLAY_ENTRY, SLOT_FUNC, LATE_FUNC) against the tables and
fails when an address no overlay defines is used where the overlay is known.
`state` lists the EXE's functions (`type:func` lines of config/symbol_addrs.txt) and its data symbols with a `size:`
there, as nm finds them in the EXE's objects, for the game-state probes and the checkpoint hash (port/src/state.c);
the stable hash's VOLATILE_RANGES come from tests/replay/replay.py.
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SLOT1_BASE, SLOT2_BASE, HEAP_START, HEAP_END = 0x80082CB0, 0x800A5DE0, 0x800AB800, 0x801FF000
# The arena's regions (port/src/arena.c, PC_PORT_PLAN 2.4) mirror the PS1's layout: slot 1, slot 2 and the heap are
# contiguous at the PS1's distances, so a pointer's PS1-style address is SLOT1_BASE + its offset into the arena (a
# sector-rounded copy that runs past a slot's end lands where it does on the PS1: in the next region). The heap is
# larger than the PS1's (64-bit structs); the whole arena stays under 16 MB (PTR_TO_U32's 24-bit offsets).
SLOT1_SIZE = SLOT2_BASE - SLOT1_BASE   # 0x23130
SLOT2_SIZE = HEAP_START - SLOT2_BASE   # 0x5A20
HEAP_SIZE = 4 << 20
ARENA_ALIGN = 1 << 24
assert SLOT1_SIZE + SLOT2_SIZE + HEAP_SIZE < ARENA_ALIGN

TIER1 = ["FIELDSTG", "FIGHTSTG", "CARDGAME", "CNTY_SEL", "SHOCKTST", "SOUNDTST", "STAGSLCT", "STCRDABM", "STCRDDEK",
         "STCRDSHP", "STDGNAME", "STDWTITL", "STFGTREP", "STGDGLAB", "STGMCARD", "STGTRAIN", "STITSHOP", "STPLNMET",
         "STSTATUS"]
TIER2_FIXED = {"WFIGHTMN": 0x208, "WFIGHTTS": 0x209}   # FIGHTSTG's overlay_module.load_file(0x208/0x209)
WSTAG_TABLE, WSTAG_C = ROOT / "config/wstag.txt", ROOT / "config/wstag_c.txt"
STAGE_TABLES = ROOT / "src/fieldstg/fieldstg_80087DB0.c"
OVERLAY_C = ROOT / "src/main/overlay.c"


# ---------------------------------------------------------------------------------------------------------------- units

def wstag_c_lines():
    """(name, flags) per line of config/wstag_c.txt (configure.py wstag_c_lines)."""
    out = []
    for ln in WSTAG_C.read_text().splitlines():
        f = ln.split("#")[0].split()
        if f:
            out.append((f[0], f[1:]))
    return out


def wstag_c_units(name, flags):
    """The C units of a WSTAG file: <n>, then <n>_<.text vram> per `split=R:T:D` flag (configure.py wstag_c_units)."""
    n = name.lower()
    units = [n]
    for f in flags:
        if f.startswith("split="):
            _, t, _ = (int(x, 16) for x in f[len("split="):].split(":"))
            units.append(f"{n}_{SLOT2_BASE + t:08X}")
    return units


def units():
    """-> [(src path relative to ROOT, overlay name or 'main')] in a stable order."""
    out = []
    for d in ["main"] + [n.lower() for n in TIER1] + [n.lower() for n in TIER2_FIXED]:
        for p in sorted((ROOT / "src" / d).glob("*.c")):
            out.append((p.relative_to(ROOT).as_posix(), d.upper()))
    for name, flags in wstag_c_lines():
        for u in wstag_c_units(name, flags):
            out.append((f"src/wstag/{u}.c", name))
    return out


def unit_overlay(path):
    """The overlay of a unit (or object) path: src/<dir>/<unit>.c -> 'MAIN', 'FIELDSTG', ..., 'WSTAG200'."""
    parts = Path(path).as_posix().split("/")
    i = len(parts) - 1 - parts[::-1].index("src")
    d, unit = parts[i + 1], parts[i + 2]
    if d == "wstag":
        return re.match(r"(wstag\d+)", unit).group(1).upper()
    return d.upper()


def cmd_units(args):
    us = units()
    missing = [u for u, _ in us if not (ROOT / u).exists()]
    if missing:
        sys.exit(f"port_gen units: missing {' '.join(missing)}")
    extra = sorted(set(p.relative_to(ROOT).as_posix() for p in (ROOT / "src").rglob("*.c")) - set(u for u, _ in us))
    if extra:
        sys.exit(f"port_gen units: C files under src/ that no target compiles: {' '.join(extra)}")
    text = ["# Generated by tools/port_gen.py units; do not edit.",
            f"set(DW3_GAME_UNITS", *(f"    \"${{DW3_ROOT}}/{u}\"" for u, _ in us), ")",
            f"set(DW3_GAME_UNIT_COUNT {len(us)})", ""]
    write(args.out, "\n".join(text))
    print(f"port_gen units: {len(us)} units -> {args.out}")


# ------------------------------------------------------------------------------------------------------------ overrides

def include_guard(path):
    m = re.search(r"^\s*#\s*ifndef\s+(\w+)", path.read_text(), flags=re.M)
    return m.group(1) if m else None


def cmd_overrides(args):
    """The override headers (first on the include path): INCLUDE_ASM/INCLUDE_RODATA empty, every gte_* macro of
    include/psyq/gtemac.h a no-op (M5 implements the GTE). They carry the real headers' include guards, so the real
    ones are skipped wherever they are included from (what tools/port_inventory.py's probe does)."""
    out = Path(args.out)
    (out / "psyq").mkdir(parents=True, exist_ok=True)
    gen = ROOT / "include/asm_generated/include_asm.h"
    guard = (include_guard(gen) if gen.exists() else None) or "INCLUDE_ASM_H"
    write(out / "include_asm.h", f"/* Generated by tools/port_gen.py overrides; do not edit. */\n#ifndef {guard}\n"
          f"#define {guard}\n#define INCLUDE_ASM(FOLDER, NAME)\n#define INCLUDE_RODATA(FOLDER, NAME)\n#endif\n")
    real = ROOT / "include/psyq/gtemac.h"
    text = re.sub(r"/\*.*?\*/", "", real.read_text(), flags=re.S)
    text = re.sub(r"\\\n", " ", text)
    macros = re.findall(r"^[ \t]*#[ \t]*define[ \t]+(gte_\w+)(\([^)]*\))", text, flags=re.M)
    body = "".join(f"#define {n}{a} ((void)0)\n" for n, a in macros)
    write(out / "psyq/gtemac.h", f"/* Generated by tools/port_gen.py overrides from include/psyq/gtemac.h; do not edit.\n"
          f" * Every GTE macro is a no-op until M5 (docs/PC_PORT_PLAN.md 1.3). */\n#ifndef "
          f"{include_guard(real) or 'PSYQ_GTEMAC_H'}\n#define {include_guard(real) or 'PSYQ_GTEMAC_H'}\n{body}#endif\n")
    print(f"port_gen overrides: include_asm.h, psyq/gtemac.h ({len(macros)} gte_* no-ops) -> {out}")


# ------------------------------------------------------------------------------------------------------------- overlays

def overlay_file_ids():
    """{overlay name: file ID} for the tier-1 overlays (overlay_files in src/main/overlay.c), WFIGHTMN/WFIGHTTS, and
    the WSTAG files (FIELDSTG's stage tables joined with config/wstag.txt)."""
    ids = {}
    text = OVERLAY_C.read_text()
    m = re.search(r"overlay_files\[\]\s*=\s*\{(.*?)\};", text, flags=re.S)
    for fid, name in re.findall(r"0x([0-9A-Fa-f]+),\s*/\*\s*\d+\s+(\w+)\.PRO\s*\*/", m.group(1)):
        ids[name] = int(fid, 16)
    missing = set(TIER1) - set(ids)
    if missing:
        sys.exit(f"port_gen: overlay_files in {OVERLAY_C.name} lacks {' '.join(sorted(missing))}")
    ids.update(TIER2_FIXED)
    # FIELDSTG's two tables: {table symbol: {stage: (file, entry)}}
    text = STAGE_TABLES.read_text()
    records = {}
    for sym, body in re.findall(r"FieldstgStageEntry\s+(fieldstg_stages(?:_2d)?)\[\d*\]\s*=\s*\{(.*?)\};", text,
                                flags=re.S):
        records[sym] = {int(s): (int(f), int(e, 16))
                       for s, f, e in re.findall(r"\{\s*(\d+),\s*(\d+),\s*WSTAG_ENTRY\(0x([0-9A-Fa-f]+)\)\s*\}", body)}
    table_of = {"A884": "fieldstg_stages", "A5F0": "fieldstg_stages_2d"}   # the symbols' addresses 0x8009xxxx
    for ln in WSTAG_TABLE.read_text().splitlines():
        if ln.strip() and not ln.startswith("#"):
            name, _, _, _, table, stage, entry = ln.split()[:7]
            rec = records.get(table_of[table], {}).get(int(stage, 16))
            if rec is None:
                sys.exit(f"port_gen: {name}: stage {stage} not in {table_of[table]} ({STAGE_TABLES.name})")
            if rec[1] != int(entry, 16):
                sys.exit(f"port_gen: {name}: entry 0x{rec[1]:08X} in {table_of[table]}, 0x{int(entry, 16):08X} in wstag.txt")
            ids[name] = rec[0]
    return ids


def overlay_funcs(name):
    """{address: symbol} of the `type:func` lines of the overlay's symbol file (none for the EXE)."""
    if name.startswith("WSTAG"):
        path = ROOT / f"config/wstag/{name.lower()}.symbols.txt"
    else:
        path = ROOT / f"config/{name.lower()}.symbols.txt"
    funcs = {}
    if path.exists():
        for ln in path.read_text().splitlines():
            m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;\s*//(.*)", ln)
            if m and "type:func" in m.group(3):
                funcs.setdefault(int(m.group(2), 16), m.group(1))
    return funcs


def tier_of(name):
    return 0 if name == "MAIN" else 1 if name in TIER1 else 2


def all_overlays():
    """Every overlay name, tier 1 first, then WFIGHT*, then the WSTAG files in config/wstag.txt order."""
    names = list(TIER1) + list(TIER2_FIXED)
    names += [ln.split()[0] for ln in WSTAG_TABLE.read_text().splitlines() if ln.strip() and not ln.startswith("#")]
    return names


def section_name(name):
    return name.lower()


def cmd_ldscript(args):
    """A GNU ld script (-T, augmenting the default one with INSERT) that collects each overlay's .data/.bss input
    sections (its objects', with -fdata-sections; plus any object's .data.dw3.<ovl>/.bss.dw3.<ovl>, port/src/asmdata.c)
    into .dw3.data.<ovl> / .dw3.bss.<ovl>, bracketed by __start_dw3_{data,bss}_<ovl> / __stop_... symbols for the
    overlay manager's snapshot (port/src/overlay.c), and defines the arena's symbols (port_slot1, port_slot2,
    port_heap_start, port_heap_end; port/src/arena.c's port_arena is the storage). The objects are matched by path:
    *src/<dir>/<unit>.c.o (CMake keeps the source path under its object directory). The statements go BEFORE the
    default .data/.bss ones: ld gives an input section to the first statement that matches it."""
    data, bss = [], []
    for name in all_overlays():
        s = section_name(name)
        if name.startswith("WSTAG"):
            # a WSTAG file's units: wstag###.c and wstag###_<vram>.c (one directory for all of them)
            files = [f"*src/wstag/{s}.c.o", f"*src/wstag/{s}_*.c.o"]
        else:
            files = [f"*src/{s}/*.c.o"]
        # each file pattern needs its own section list: a bare pattern would take every section of the file
        d = " ".join(f"{f}(.data .data.* .sdata .sdata.*)" for f in files)
        b = " ".join(f"{f}(.bss .bss.* .sbss .sbss.* COMMON)" for f in files)
        data.append(f"  .dw3.data.{s} : {{ __start_dw3_data_{s} = .; *(.data.dw3.{s}) {d} . = ALIGN(16); "
                    f"__stop_dw3_data_{s} = .; }}")
        bss.append(f"  .dw3.bss.{s} : {{ __start_dw3_bss_{s} = .; *(.bss.dw3.{s}) {b} . = ALIGN(16); "
                   f"__stop_dw3_bss_{s} = .; }}")
    text = ["/* Generated by tools/port_gen.py ldscript; do not edit. port/README.md explains. */",
            "SECTIONS {", *data, "} INSERT BEFORE .data;", "",
            "SECTIONS {", *bss, "} INSERT BEFORE .bss;", "",
            "/* The memory arena (include/port.h, PC_PORT_PLAN 2.4): one 16 MB-aligned block, port_arena (arena.c). */",
            "port_slot1 = port_arena;",
            f"port_slot2 = port_arena + 0x{SLOT1_SIZE:X};",
            f"port_heap_start = port_arena + 0x{SLOT1_SIZE + SLOT2_SIZE:X};",
            f"port_heap_end = port_arena + 0x{SLOT1_SIZE + SLOT2_SIZE + HEAP_SIZE:X};", ""]
    write(args.out, "\n".join(text))
    print(f"port_gen ldscript: {len(data)} overlays -> {args.out}")


def cmd_arena_header(args):
    """port_arena.h: the arena's layout for port/src/arena.c (one source of the numbers: this file)."""
    text = ["/* Generated by tools/port_gen.py arena-header; do not edit. */", "#ifndef PORT_ARENA_GEN_H",
            "#define PORT_ARENA_GEN_H", f"#define PORT_ARENA_ALIGN 0x{ARENA_ALIGN:X}",
            f"#define PORT_SLOT1_SIZE 0x{SLOT1_SIZE:X}", f"#define PORT_SLOT2_SIZE 0x{SLOT2_SIZE:X}",
            f"#define PORT_HEAP_SIZE 0x{HEAP_SIZE:X}",
            f"#define PORT_ARENA_SIZE 0x{SLOT1_SIZE + SLOT2_SIZE + HEAP_SIZE:X}", "#endif", ""]
    write(args.out, "\n".join(text))


# --------------------------------------------------------------------------------------------------------------- tables

def nm_globals(nm, objects):
    """{object path: set of global function (T) symbols}."""
    out = {}
    for obj in objects:
        r = subprocess.run([nm, "--defined-only", "-g", obj], capture_output=True, text=True, check=True)
        out[obj] = {ln.split()[2] for ln in r.stdout.splitlines() if len(ln.split()) == 3 and ln.split()[1] in "TtWw"}
    return out


def tag_sites():
    """Every tag in the C: [(file:line, tier, overlay or None, address)]. The overlay is known for WSTAG_ENTRY
    (the record's file), OVERLAY_ENTRY (the comment names it) and LATE_FUNC tier 1 (FIELDSTG, gamestate.c's comment);
    a SLOT_FUNC/LATE_FUNC of tier 2 is for whatever WSTAG file is loaded."""
    sites = []
    for path in sorted((ROOT / "src").rglob("*.c")):
        text = path.read_text(errors="replace")
        rel = path.relative_to(ROOT).as_posix()
        for m in re.finditer(r"\{\s*\d+,\s*(\d+),\s*WSTAG_ENTRY\(0x([0-9A-Fa-f]+)\)", text):
            sites.append((f"{rel}:{text.count(chr(10), 0, m.start()) + 1}", 2, int(m.group(1)), int(m.group(2), 16)))
        for m in re.finditer(r"OVERLAY_ENTRY\(0x([0-9A-Fa-f]+)\),\s*/\*\s*\d+\s+(\w+)\s*\*/", text):
            sites.append((f"{rel}:{text.count(chr(10), 0, m.start()) + 1}", 1, m.group(2), int(m.group(1), 16)))
        for m in re.finditer(r"SLOT_FUNC\([^,]+,\s*0x([0-9A-Fa-f]+)\)", text):
            sites.append((f"{rel}:{text.count(chr(10), 0, m.start()) + 1}", 2, None, int(m.group(1), 16)))
        for m in re.finditer(r"LATE_FUNC\(\s*([12]),\s*0x([0-9A-Fa-f]+)", text):
            tier = int(m.group(1))
            sites.append((f"{rel}:{text.count(chr(10), 0, m.start()) + 1}", tier, "FIELDSTG" if tier == 1 else None,
                          int(m.group(2), 16)))
    return sites


def cmd_tables(args):
    objects = [ln.strip() for ln in Path(args.objects).read_text().replace(";", "\n").splitlines() if ln.strip()]
    globals_of = nm_globals(args.nm, objects)
    by_overlay = {}
    for obj, syms in globals_of.items():
        by_overlay.setdefault(unit_overlay(obj), set()).update(syms)
    ids = overlay_file_ids()
    names = all_overlays()
    stats = {"funcs": {1: 0, 2: 0}, "skipped_static_or_asm": 0, "unnamed_global": 0}
    tables, skipped = [], []
    for name in names:
        funcs = overlay_funcs(name)
        host = by_overlay.get(name, set())
        rows = sorted((a, s) for a, s in funcs.items() if s in host)
        for a, s in sorted(funcs.items()):
            if s not in host:
                stats["skipped_static_or_asm"] += 1
                skipped.append((name, s, a))
        stats["unnamed_global"] += len(host - set(funcs.values()) - {"main"})
        stats["funcs"][tier_of(name)] += len(rows)
        tables.append((name, rows))
    # every tag site resolves?
    defined = {name: {a for a, _ in rows} for name, rows in tables}
    by_file = {ids[name]: name for name in names}
    any_wstag = {}
    for name in names:
        if name.startswith("WSTAG"):
            for a in defined[name]:
                any_wstag[a] = any_wstag.get(a, 0) + 1
    bad, checked, loose = [], 0, 0
    for where, tier, ovl, addr in tag_sites():
        if isinstance(ovl, int):
            ovl = by_file.get(ovl)
        if ovl is not None:
            checked += 1
            if addr not in defined.get(ovl, set()):
                bad.append(f"{where}: 0x{addr:08X} not a host function of {ovl}")
        else:
            loose += 1
            if addr not in any_wstag and addr not in {a for n in TIER2_FIXED for a in defined[n]}:
                bad.append(f"{where}: 0x{addr:08X} (tier {tier}, overlay unknown) is in no tier-2 table")
    out = ["/* Generated by tools/port_gen.py tables from the overlays' symbol files (config/) and nm of the game's",
           " * objects; do not edit. The only place that maps a PS1 address to a host function (port.h SLOT_FUNC). */",
           "#include <stddef.h>", "#include \"port_runtime.h\"", ""]
    for name, rows in tables:
        out += [f"void {s}(void);" for _, s in rows]  # the real prototypes are in the game's headers; any one will do
    out.append("")
    for name, rows in tables:
        out.append(f"static const PortOverlayFunc port_funcs_{section_name(name)}[] = {{")
        out += [f"    {{ 0x{a:08X}u, (PortFn){s} }}," for a, s in rows]
        out += ["    { 0, NULL },", "};"]
    out.append("")
    for name, _ in tables:
        s = section_name(name)
        out += [f"extern char __start_dw3_data_{s}[], __stop_dw3_data_{s}[], __start_dw3_bss_{s}[], __stop_dw3_bss_{s}[];"]
    out += ["", "const PortOverlay port_overlays[] = {"]
    for name, rows in tables:
        s = section_name(name)
        out.append(f"    {{ {tier_of(name)}, 0x{ids[name]:X}, \"{name}\", port_funcs_{s}, {len(rows)}, "
                   f"__start_dw3_data_{s}, __stop_dw3_data_{s}, __start_dw3_bss_{s}, __stop_dw3_bss_{s} }},")
    out += ["};", f"const int port_overlay_count = {len(tables)};", ""]
    write(args.out, "\n".join(out))
    print(f"port_gen tables: {len(tables)} overlays ({len(TIER1)} tier 1, {len(tables) - len(TIER1)} tier 2); "
          f"host functions: tier 1 {stats['funcs'][1]}, tier 2 {stats['funcs'][2]}; "
          f"skipped {stats['skipped_static_or_asm']} symbol-file functions with no global symbol on the host (static, "
          f"or still INCLUDE_ASM); {stats['unnamed_global']} global functions without a symbol-file address (not in "
          f"any table); tag sites: {checked} checked against their overlay, {loose} against every tier-2 table")
    if args.report:
        for name, s, a in skipped:
            print(f"  skipped {name} {s} 0x{a:08X}")
    for b in bad:
        print(f"  UNRESOLVED {b}")
    if bad:
        sys.exit(f"port_gen tables: {len(bad)} tag site(s) do not resolve")


# ---------------------------------------------------------------------------------------------------------------- state

SYMBOL_ADDRS = ROOT / "config/symbol_addrs.txt"
REPLAY_PY = ROOT / "tests/replay/replay.py"


def exe_symbols():
    """-> (funcs {name: address}, data {name: (address, PS1 size)}) from config/symbol_addrs.txt: the `type:func`
    lines, and the other lines that carry a `size:`."""
    funcs, data = {}, {}
    for ln in SYMBOL_ADDRS.read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;\s*//(.*)", ln)
        if not m:
            continue
        name, addr, attrs = m.group(1), int(m.group(2), 16), m.group(3)
        if "type:func" in attrs:
            funcs.setdefault(name, addr)
        else:
            s = re.search(r"\bsize:(0x[0-9A-Fa-f]+|\d+)", attrs)
            if s:
                data.setdefault(name, (addr, int(s.group(1), 0)))
    return funcs, data


def volatile_ranges():
    """VOLATILE_RANGES of tests/replay/replay.py (the bytes of gamestate_data zeroed for the stable hash), read from its
    source: one definition for the emulator's records and the port's."""
    import ast
    for node in ast.parse(REPLAY_PY.read_text()).body:
        if isinstance(node, ast.Assign) and any(getattr(t, "id", None) == "VOLATILE_RANGES" for t in node.targets):
            ranges = ast.literal_eval(node.value)
            if not all(len(r) == 2 and 0 <= r[0] < r[1] for r in ranges):
                sys.exit(f"port_gen state: VOLATILE_RANGES in {REPLAY_PY.name} is not a list of (lo, hi) pairs")
            return [tuple(r) for r in ranges]
    sys.exit(f"port_gen state: no VOLATILE_RANGES in {REPLAY_PY}")


def nm_defined(nm, objects):
    """{object: {symbol: (type letter, size or None)}} of the defined global symbols (nm -S)."""
    out = {}
    for obj in objects:
        r = subprocess.run([nm, "--defined-only", "-g", "-S", obj], capture_output=True, text=True, check=True)
        syms = {}
        for ln in r.stdout.splitlines():
            p = ln.split()
            if len(p) == 4:
                syms[p[3]] = (p[2], int(p[1], 16))
            elif len(p) == 3:
                syms[p[2]] = (p[1], None)
        out[obj] = syms
    return out


def object_source(obj):
    """The unit an object was compiled from: .../src/<dir>/<unit>.c.o -> ROOT/src/<dir>/<unit>.c."""
    parts = Path(obj).as_posix().split("/")
    i = len(parts) - 1 - parts[::-1].index("src")
    return ROOT.joinpath(*parts[i:]).with_suffix("")


def probe_sizes_m64(cc, gen_include, by_unit):
    """{symbol: sizeof at -m64}: compiles each unit wrapped with `char dw3_size__<sym>[sizeof(<sym>)];` lines at
    -m64 (to assembly only) and reads the arrays' .size directives. The -m64 size is the pointer test of the
    layout-identical rule (cmd_state); measuring it the same way in every build keeps the -m32 build's table equal to
    the -m64 build's."""
    from concurrent.futures import ThreadPoolExecutor
    ver = subprocess.run([cc, "-dumpversion"], capture_output=True, text=True).stdout.strip()
    flags = [cc, "-m64", "-S", "-o", "-", "-w", "-x", "c", "-std=gnu99", "-DPC_PORT", "-DNON_MATCHING",
             "-fsigned-char", "-fno-builtin", "-fno-common", f"-I{gen_include}", f"-I{ROOT / 'include'}", f"-I{ROOT}"]
    if ver.split(".")[0].isdigit() and int(ver.split(".")[0]) >= 14:
        flags.append("-fpermissive")

    def one(item):
        src, syms = item
        text = f'#include "{src}"\n' + "".join(f"char dw3_size__{s}[sizeof({s})];\n" for s in sorted(syms))
        r = subprocess.run(flags + ["-"], input=text, capture_output=True, text=True)
        if r.returncode != 0:
            sys.exit(f"port_gen state: the -m64 size probe of {src} failed:\n{r.stderr[-2000:]}")
        return {m.group(1): int(m.group(2)) for m in re.finditer(r"\.size\s+dw3_size__(\w+),\s*(\d+)", r.stdout)}

    sizes = {}
    with ThreadPoolExecutor(max_workers=2) as ex:
        for s in ex.map(one, sorted(by_unit.items())):
            sizes.update(s)
    return sizes


def cmd_state(args):
    """port_state_tables.c for port/src/state.c and framelog.c: the EXE's functions ({PS1 address, host function}:
    gamestate_data.funcs's PS1 image), the EXE's sized data symbols ({PS1 address, PS1 size, host object, the length of
    the layout-identical prefix by the default rule}: port_state_read), and VOLATILE_RANGES (the stable hash).
    The default rule: a whole object is layout-identical when its -m64 sizeof equals its PS1 size (any pointer, or
    long, makes it larger at -m64); its prefix is 0 otherwise (port/src/state.c lists the pointer-bearing objects'
    identical prefixes, with _Static_asserts). Only symbols that nm shows as globals of the EXE's objects are listed."""
    objects = [ln.strip() for ln in Path(args.objects).read_text().replace(";", "\n").splitlines() if ln.strip()]
    objects = [o for o in objects if unit_overlay(o) == "MAIN"]
    defined = nm_defined(args.nm, objects)
    funcs, data = exe_symbols()
    host_funcs = {s for syms in defined.values() for s, (t, _) in syms.items() if t in "TW"}
    frows = sorted((a, s) for s, a in funcs.items() if s in host_funcs)
    by_unit, owner = {}, {}
    for obj, syms in defined.items():
        for s, (t, size) in syms.items():
            if s in data and t in "DdBbRrGgSsVv":
                by_unit.setdefault(str(object_source(obj)), set()).add(s)
                owner[s] = size
    m64 = probe_sizes_m64(args.cc, args.gen_include, by_unit)
    drows, bad = [], []
    for s in sorted(owner, key=lambda s: data[s][0]):
        addr, ps1 = data[s]
        identical = m64.get(s) == ps1
        if identical and owner[s] != ps1:
            bad.append(f"{s}: -m64 size 0x{m64[s]:X} is the PS1 size but this build's is 0x{owner[s]:X}")
        drows.append((addr, ps1, s, ps1 if identical else 0))
    if bad:
        sys.exit("port_gen state: layout differs between -m32 and -m64 for a pointer-free object:\n  " + "\n  ".join(bad))
    vol = volatile_ranges()
    out = ["/* Generated by tools/port_gen.py state from config/symbol_addrs.txt, nm of the EXE's objects and",
           " * tests/replay/replay.py's VOLATILE_RANGES; do not edit. port/src/state.c uses it. */",
           "#include <stddef.h>", "#include \"port_runtime.h\"", ""]
    out += [f"void {s}(void);" for _, s in frows]
    out += [f"extern char {s}[];" for _, _, s, _ in drows]
    out += ["", "const PortExeFunc port_exe_funcs[] = {"]
    out += [f"    {{ 0x{a:08X}u, (PortFn){s} }}," for a, s in frows]
    out += ["};", f"const int port_exe_func_count = {len(frows)};", "", "const PortExeData port_exe_data[] = {"]
    out += [f"    {{ 0x{a:08X}u, 0x{n:X}, \"{s}\", {s}, 0x{p:X} }}," for a, n, s, p in drows]
    out += ["};", f"const int port_exe_data_count = {len(drows)};", "",
            "/* tests/replay/replay.py VOLATILE_RANGES: [lo, hi) byte ranges of gamestate_data's PS1 image */",
            "const PortRange port_gamestate_volatile[] = {"]
    out += [f"    {{ 0x{lo:X}, 0x{hi:X} }}," for lo, hi in vol]
    out += ["};", f"const int port_gamestate_volatile_count = {len(vol)};", ""]
    write(args.out, "\n".join(out))
    print(f"port_gen state: {len(frows)} EXE functions, {len(drows)} sized EXE data symbols "
          f"({sum(1 for r in drows if r[3])} layout-identical by size), {len(vol)} volatile ranges -> {args.out}")


# ----------------------------------------------------------------------------------------------------------------- main

def write(path, text):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("units", help="the game's C units as CMake variables")
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_units)
    p = sub.add_parser("overrides", help="include_asm.h and psyq/gtemac.h for the host build")
    p.add_argument("--out", required=True, help="directory")
    p.set_defaults(fn=cmd_overrides)
    p = sub.add_parser("ldscript", help="per-overlay .data/.bss sections and the arena's symbols (GNU ld)")
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_ldscript)
    p = sub.add_parser("arena-header", help="the arena's sizes as a header")
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_arena_header)
    p = sub.add_parser("tables", help="the overlay address tables (needs the compiled objects)")
    p.add_argument("--nm", default=os.environ.get("NM", "nm"))
    p.add_argument("--objects", required=True, help="a file listing the game's objects (one per line or ;-separated)")
    p.add_argument("--out", required=True)
    p.add_argument("--report", action="store_true", help="list every skipped function")
    p.set_defaults(fn=cmd_tables)
    p = sub.add_parser("state", help="the EXE's function and data tables for the game-state probes (needs the objects)")
    p.add_argument("--nm", default=os.environ.get("NM", "nm"))
    p.add_argument("--cc", default=os.environ.get("CC", "cc"), help="the C compiler (the -m64 size probe)")
    p.add_argument("--gen-include", required=True, help="the generated override headers (overrides --out)")
    p.add_argument("--objects", required=True, help="a file listing the game's objects (one per line or ;-separated)")
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_state)
    args = ap.parse_args()
    args.fn(args)


if __name__ == "__main__":
    main()
