#!/usr/bin/env python3
"""Generate build.ninja and objdiff.json.

  tools/venv/bin/python configure.py             # split with splat, then write the files
  tools/venv/bin/python configure.py --no-split  # reuse the existing asm/ and linker scripts
  ninja                                          # build everything and check the SHA-1s

Each TARGET is one link unit (the EXE now, overlays later): a splat YAML, the linker script
splat writes, and the SHA-1 the output must have. The objects and their order come from
splat's linker script; sources map to objects by path:
    asm/<t>/X.s     -> build/<t>/asm/<t>/X.s.o
    assets/<t>/X.bin-> build/<t>/assets/<t>/X.bin.o
    src/<t>/X.c     -> build/<t>/src/<t>/X.c.o   (cpp -> cc1 -> maspsx -> as)
"""
import argparse
import concurrent.futures
import csv
import json
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent
TOOLS = "tools"
PYTHON = f"{TOOLS}/venv/bin/python"
BINUTILS = f"{TOOLS}/binutils/bin/mipsel-linux-gnu-"
SPLAT = f"{TOOLS}/venv/bin/splat"


@dataclass
class Target:
    name: str          # short name, also the asm/src/build subdirectory
    yaml: str          # splat config
    output: str        # final binary
    sha1: str          # file with "<sha1>  <basename>"
    ld_extra: tuple = ()  # extra linker scripts (e.g. the EXE's symbols, for overlays)


# The EXE's symbols (global, below the overlay area; nm prints 8 lowercase hex digits, so a string
# compare works) as a linker script, for the overlays to link
# against: their calls into the EXE resolve to the EXE's own definitions. Made from the EXE's ELF.
EXE_SYMS = "build/main/SLES_039.36.syms.ld"
OVERLAY_BASE = 0x80082CB0  # tier-1 overlays load here; the EXE's symbols end below it

TARGETS = [
    Target("main", "config/main.yaml", "build/SLES_039.36", "config/SLES_039.36.sha1"),
    # Tier-1 overlays (docs/DISC_LAYOUT.md "Overlays").
    *(Target(n.lower(), f"config/{n.lower()}.yaml", f"build/{n}.PRO", f"config/{n}.PRO.sha1", (EXE_SYMS,))
      for n in ("FIELDSTG", "FIGHTSTG", "CARDGAME", "CNTY_SEL", "SHOCKTST", "SOUNDTST", "STAGSLCT", "STCRDABM",
                "STCRDDEK", "STCRDSHP", "STDGNAME", "STDWTITL", "STFGTREP", "STGDGLAB", "STGMCARD", "STGTRAIN",
                "STITSHOP", "STPLNMET", "STSTATUS")),
]

# ---- Tier-2 overlays (docs/DISC_LAYOUT.md "Overlays", DECISIONS "Overlays link against the EXE and their parent") ----
# A tier-2 overlay is loaded at TIER2_BASE (overlay_load_file) while its parent, the tier-1 overlay that
# loads it, stays resident below TIER2_BASE; it calls and reads the parent as well as the EXE. So it links
# against the EXE's symbols (EXE_SYMS) and the parent's: the linker gets the parent ELF's global symbols in
# [OVERLAY_BASE, TIER2_BASE) (parent_syms_ld, made by ninja like EXE_SYMS), and splat gets the parent's
# names, written by split() from the parent's own splat run (parent_names; absolute:True like the EXE's).
TIER2_BASE = 0x800A5DE0
TIER2_PARENT = {"WFIGHTMN": "fightstg", "WFIGHTTS": "fightstg"}


def parent_syms_ld(parent: str) -> str:
    return f"build/{parent}/{parent.upper()}.PRO.syms.ld"


def parent_names(parent: str) -> str:
    return f"build/{parent}/names_for_tier2.txt"


# The other direction: a parent calls into its tier-2 overlays (FIGHTSTG into WFIGHTMN/WFIGHTTS) by their own
# names, which its symbol file declares `absolute:True` at TIER2_BASE+ (splat leaves absolute symbols to the
# linker). It can't link against their ELFs (they link against it), so write_tier2_calls defines them for its link.
def tier2_calls_ld(parent: str) -> str:
    return f"build/{parent}/tier2_calls.ld"


def write_tier2_calls(parent: str):
    out = []
    for line in (ROOT / f"config/{parent}.symbols.txt").read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);.*\babsolute:True", line)
        if m and int(m.group(2), 16) >= TIER2_BASE:
            out.append(f"{m.group(1)} = 0x{m.group(2)};\n")
    path = ROOT / tier2_calls_ld(parent)
    path.parent.mkdir(parents=True, exist_ok=True)
    write_if_changed(path, "".join(out))


TARGETS += [Target(n.lower(), f"config/{n.lower()}.yaml", f"build/{n}.PRO", f"config/{n}.PRO.sha1",
                   (EXE_SYMS, parent_syms_ld(p)))
            for n, p in TIER2_PARENT.items()]

# WSTAG###: the 293 stage overlays (tier 2, parent FIELDSTG). Their splat configs and SHA-1 files are generated
# into GEN_CONFIG (write_wstag_configs) from WSTAG_TEMPLATE and one line each of WSTAG_TABLE (size, .text and
# .data offsets: tools/overlay_layout.py --wstag-table) and config/overlays.sha1. objdiff category "stage".
# A file listed in WSTAG_C (one name per line) is a C unit: its .rodata (if any) and .text are the C file
# src/wstag/wstag###.c (one src directory for all of them; splat writes the INCLUDE_ASM stub the first time).
# Its .data stays generated asm unless the line says `data` (WSTAG### data): then the .data is the C file's too
# (tools/wstag_data.py writes it; the file is DATA_IN_C). Every other file is asm units only. tools/wstag_groups.py groups the files by
# code shape. unit_diff: `tools/unit_diff.py wstag### -t wstag` (expected objects go to build/expected/<src dir>/).
# A file's own names and sizes, if any, are in WSTAG_SYMBOLS (the files share addresses, so one each).
# A `split=R:T:D` flag makes the file one more C unit src/wstag/wstag###_<.text vram>.c from those offsets
# (wstag_c_splits; WSTAG924, whose jump tables start 4 mod 8 after the setup's CVECTOR).
WSTAG_TABLE = "config/wstag.txt"
WSTAG_C = "config/wstag_c.txt"
WSTAG_SYMBOLS = "config/wstag/{n}.symbols.txt"
GEN_CONFIG = "build/config"
WSTAG_TEMPLATE = """\
# Generated by configure.py from {table} (do not edit): the tier-2 stage overlay AAA/PRO/{N}.PRO, loaded at
# 0x800A5DE0 by FIELDSTG (its stage table, entry 0x{entry}) and linked against the EXE and FIELDSTG.
name: {N}.PRO
sha1: {sha1}
options:
  basename: {N}.PRO
  base_path: ../..
  target_path: extracted/disc/AAA/PRO/{N}.PRO
  elf_path: build/{n}/{N}.PRO.elf
  platform: psx
  compiler: GCC
  build_path: build/{n}
  asm_path: asm/{n}
  src_path: {src_path}
  asset_path: assets/{n}
  ld_script_path: build/{n}/{N}.PRO.ld
  symbol_addrs_path: [build/exe_symbols.txt, {parent_names}{own_names}]
  create_undefined_funcs_auto: True
  undefined_funcs_auto_path: build/{n}/undefined_funcs_auto.txt
  create_undefined_syms_auto: True
  undefined_syms_auto_path: build/{n}/undefined_syms_auto.txt
  gp_value: 0x8005CB50
  section_order: [".rodata", ".text", ".data", ".bss"]
  subalign: 4
  ld_align_section_vram_end: False
  ld_align_segment_vram_end: False
  find_file_boundaries: True
  migrate_rodata_to_functions: True
  use_legacy_include_asm: False
  asm_function_macro: glabel
  asm_jtbl_label_macro: jlabel
  asm_data_macro: dlabel
  make_full_disasm_for_code: True
  generate_asm_macros_files: False  # the tier-1 configs write include/asm_generated; these split in parallel
segments:
  - name: {n}
    type: code
    start: 0x0
    vram: 0x800A5DE0
    symbol_name_format: {N}_$VRAM
    subsegments:
{rodata}      - [0x{text:X}, {text_seg}]
{data_seg}{tail}  - [0x{size:X}]
"""


def wstag_rows():
    """(name, size, .text offset, .data offset, entry) per line of WSTAG_TABLE."""
    rows = []
    for line in (ROOT / WSTAG_TABLE).read_text().splitlines():
        if line.strip() and not line.startswith("#"):
            f = line.split()
            rows.append((f[0], int(f[1], 16), int(f[2], 16), int(f[3], 16), f[6]))
    return rows


def wstag_c_lines():
    """(name, flags) per line of WSTAG_C."""
    path = ROOT / WSTAG_C
    lines = path.read_text().splitlines() if path.exists() else []
    return [(f[0], f[1:]) for f in (ln.split("#")[0].split() for ln in lines) if f]


def wstag_c_names():
    """The WSTAG files that are C units (WSTAG_C)."""
    return [n for n, _ in wstag_c_lines()]


def wstag_c_data_names():
    """The C units whose .data is in C too (`data` in WSTAG_C)."""
    return {n for n, flags in wstag_c_lines() if "data" in flags}


def wstag_c_splits(flags):
    """A WSTAG C file made of more than one object: `split=R:T:D` in its WSTAG_C line gives the file offsets of
    the next object's .rodata, .text and .data (one flag per extra object, in file order). Returns
    [(rodata, text, data)] of the extra objects; their unit is <n>_<.text vram> (src/wstag/<n>_<vram>.c)."""
    return [tuple(int(x, 16) for x in f[len("split="):].split(":")) for f in flags if f.startswith("split=")]


def wstag_c_units(name):
    """The C units (src/wstag/<unit>.c) of a WSTAG C file: <n>, then one <n>_<vram> per split."""
    flags = dict(wstag_c_lines()).get(name, [])
    n = name.lower()
    return [n] + [f"{n}_{0x800A5DE0 + t:08X}" for _, t, _ in wstag_c_splits(flags)]


WSTAG_NAMES = [r[0] for r in wstag_rows()]
TIER2_PARENT.update({n: "fieldstg" for n in WSTAG_NAMES})
WSTAG_TARGETS = [Target(n.lower(), f"{GEN_CONFIG}/{n.lower()}.yaml", f"build/{n}.PRO", f"{GEN_CONFIG}/{n}.PRO.sha1",
                        (EXE_SYMS, parent_syms_ld("fieldstg")))
                 for n in WSTAG_NAMES]
TARGETS += WSTAG_TARGETS
for _t in TARGETS:
    if _t.name in TIER2_PARENT.values():
        _t.ld_extra += (tier2_calls_ld(_t.name),)


def write_if_changed(path: Path, text: str):
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def write_wstag_configs():
    sha1 = {Path(f).stem: h for h, f in (line.split() for line in (ROOT / "config/overlays.sha1").read_text()
                                        .splitlines() if line.strip())}
    out = ROOT / GEN_CONFIG
    out.mkdir(parents=True, exist_ok=True)
    in_c = set(wstag_c_names())
    data_in_c = wstag_c_data_names()
    unknown = in_c - set(WSTAG_NAMES)
    if unknown:
        sys.exit(f"{WSTAG_C}: not in {WSTAG_TABLE}: {' '.join(sorted(unknown))}")
    splits = {n: wstag_c_splits(flags) for n, flags in wstag_c_lines()}
    for name, size, text, data, entry in wstag_rows():
        n = name.lower()
        units = list(zip(wstag_c_units(name), [(0, text, data)] + splits.get(name, []))) if name in in_c else []
        if name in in_c and len(units) > 1:  # C units src/wstag/<unit>.c (wstag_c_splits)
            src_path = "src/wstag"
            rodata = "".join(f"      - [0x{r:X}, .rodata, {u}]\n" for u, (r, t, d) in units if r < text)
            text_seg = f"c, {n}]\n" + "".join(f"      - [0x{t:X}, c, {u}]\n" for u, (r, t, d) in units[1:])
            text_seg = text_seg[:-2]  # the template closes the last .text subsegment
        elif name in in_c:  # C unit src/wstag/<n>.c: its .rodata and .text
            src_path, text_seg = "src/wstag", f"c, {n}"
            rodata = f"      - [0x0, .rodata, {n}]\n" if text else ""
        else:
            src_path, text_seg = f"src/{n}", "asm, text"
            rodata = "      - [0x0, rodata, rodata]\n" if text else ""
        # A file ending inside a word (WSTAG925): splat drops a data section's partial last word, so those
        # bytes stay binary (like CNTY_SEL's data_tail).
        tail = f"      - [0x{size & ~3:X}, bin, data_tail]\n" if size % 4 else ""
        if name in data_in_c:  # the C file's .data, to the end of the file (its last bytes too)
            data_seg, tail = f"      - [0x{data:X}, .data, {n}]\n", ""
            data_seg += "".join(f"      - [0x{d:X}, .data, {u}]\n" for u, (r, t, d) in units[1:])
        else:
            data_seg = f"      - [0x{data:X}, data, data]\n"
        own = WSTAG_SYMBOLS.format(n=n)
        own_names = f", {own}" if (ROOT / own).exists() else ""
        write_if_changed(out / f"{name.lower()}.yaml", WSTAG_TEMPLATE.format(
            table=WSTAG_TABLE, N=name, n=n, sha1=sha1[name], entry=entry[2:], src_path=src_path,
            parent_names=parent_names("fieldstg"), rodata=rodata, text=text, text_seg=text_seg, data_seg=data_seg,
            tail=tail, size=size, own_names=own_names))
        write_if_changed(out / f"{name}.PRO.sha1", f"{sha1[name]}  {name}.PRO\n")

# Compiler per C file; anything not listed uses DEFAULT_CC (tools/cc_psx.sh picks the matching
# ASPSX version for maspsx). GCC 2.8.1 is confirmed by tools/compiler_id.py (docs/TOOLCHAIN.md).
DEFAULT_CC = "2.8.1"
CC_OVERRIDES: dict[str, str] = {}      # "src/main/foo.c": "2.7.2"
# -G per target (docs/DECISIONS.md): the EXE's game files are -G8 (all match at -G8, 10 need it, and
# its .data/.bss hold no variable of 8 bytes or less); overlay code is -G0 (it keeps %hi of the EXE's
# small .sdata variables in registers and puts their lui in delay slots, which -G8 can't produce; an
# overlay can't have .sdata). Exceptions by C path in G_OVERRIDES.
DEFAULT_G = {"main": 8}  # targets not listed: 0
G_OVERRIDES: dict[str, int] = {}


def g_for(src: str) -> int:
    """-G for a C file src/<target>/...: G_OVERRIDES, else its target's default."""
    return G_OVERRIDES.get(src, DEFAULT_G.get(Path(src).parts[1], 0))

# C files that define all their own data (every .data/.sdata/.sbss/.bss subsegment of theirs in the
# YAML is the C object's): compiled without maspsx's --use-comm-section, so their variables without
# an initializer get storage in the object (DECISIONS "Data in C, split per object"). The other C
# files' tentative definitions stay COMMON and resolve to the data asm.
DATA_IN_C: set[str] = {
    "src/main/card.c",
    "src/main/cdload.c",
    "src/main/filetable.c",
    "src/main/font.c",
    "src/main/gamestate.c",
    "src/main/gfx.c",
    "src/main/heap.c",
    "src/main/inn.c",
    "src/main/main.c",
    "src/main/message.c",
    "src/main/overlay.c",
    "src/main/pad.c",
    "src/main/records.c",
    "src/main/sound.c",
    "src/main/sprite.c",
    "src/main/tim.c",
    # Tier-1 overlays (DECISIONS "Data in C, split per object").
    "src/cardgame/cardgame_80083E34.c",
    "src/cardgame/cardgame_80085DE8.c",
    "src/cardgame/cardgame_800954F8.c",
    "src/cardgame/cardgame_80096950.c",
    "src/cardgame/cardgame_8009D6E0.c",
    "src/cardgame/cardgame_800A32D8.c",
    "src/cnty_sel/cnty_sel_80082CE8.c",
    "src/fieldstg/fieldstg_80083784.c",
    "src/fieldstg/fieldstg_80083D08.c",
    "src/fieldstg/fieldstg_80085590.c",
    "src/fieldstg/fieldstg_80087DB0.c",
    "src/fightstg/fightstg_stage.c",
    "src/fightstg/fightstg_80086A00.c",
    "src/fightstg/fightstg_8008B630.c",
    "src/fightstg/fightstg_8008D3B4.c",
    "src/fightstg/fightstg_800A1FE0.c",
    "src/shocktst/shocktst_80082DA0.c",
    "src/soundtst/soundtst_80084370.c",
    "src/stagslct/stagslct_800849CC.c",
    "src/stcrdabm/stcrdabm_80082D88.c",
    "src/stcrddek/stcrddek_800831F0.c",
    "src/stcrddek/stcrddek_800867E8.c",
    "src/stcrddek/stcrddek_8008A100.c",
    "src/stcrdshp/stcrdshp_8008300C.c",
    "src/stcrdshp/stcrdshp_800853B4.c",
    "src/stcrdshp/stcrdshp_80088E24.c",
    "src/stdgname/stdgname_80082F8C.c",
    "src/stdgname/stdgname_80086184.c",
    "src/stdwtitl/stdwtitl_80082D70.c",
    "src/stdwtitl/stdwtitl_80083F80.c",
    "src/stdwtitl/stdwtitl_800852F0.c",
    "src/stdwtitl/stdwtitl_80085CD4.c",
    "src/stdwtitl/stdwtitl_80086DF8.c",
    "src/stfgtrep/stfgtrep_80082E70.c",
    "src/stgdglab/stgdglab_80082F48.c",
    "src/stgdglab/stgdglab_800886DC.c",
    "src/stgdglab/stgdglab_8008A92C.c",
    "src/stgdglab/stgdglab_8008EB30.c",
    "src/stgmcard/stgmcard_80082CD0.c",
    "src/stgmcard/stgmcard_80087C18.c",
    "src/stgtrain/stgtrain_80083058.c",
    "src/stgtrain/stgtrain_800861BC.c",
    "src/stgtrain/stgtrain_80088100.c",
    "src/stitshop/stitshop_800859C0.c",
    "src/stplnmet/stplnmet_80082F58.c",
    "src/stplnmet/stplnmet_80083D70.c",
    "src/stplnmet/stplnmet_80085ECC.c",
    "src/ststatus/ststatus_80083558.c",
    "src/ststatus/ststatus_80084F54.c",
    "src/ststatus/ststatus_8008833C.c",
    "src/ststatus/ststatus_8008E94C.c",
    "src/ststatus/ststatus_800937A4.c",
    "src/ststatus/ststatus_80096590.c",
    "src/ststatus/ststatus_80099B6C.c",
    # Tier-2 overlays (wfight2).
    "src/wfightmn/wfightmn_800A6440.c",
    "src/wfightts/wfightts_800A67B8.c",
    "src/wfightts/wfightts_800A7A90.c",
    "src/wfightts/wfightts_800A8018.c",
    "src/wfightts/wfightts_800A8484.c",
    "src/wfightts/wfightts_800A8798.c",
    "src/wfightts/wfightts_800A8D34.c",
}


# The WSTAG C units whose .data is in C (`data` in WSTAG_C).
DATA_IN_C |= {f"src/wstag/{u}.c" for n in wstag_c_data_names() for u in wstag_c_units(n)}


def cc_flags(src: str) -> list[str]:
    """Extra tools/cc_psx.sh flags for a C file."""
    flags = ["--data-in-c"] if src in DATA_IN_C else []
    return flags

AS_FLAGS = "-EL -march=r3000 -mtune=r3000 -no-pad-sections -G0 -Iinclude -Iinclude/asm_generated -I."
# C files go through tools/cc_psx.sh (cpp -> cc1 -O2 -> maspsx -> as), shared with the permuter.
CC_INCLUDES = "-Iinclude -Iinclude/asm_generated -I."

# Progress categories for objdiff: "game" (the EXE's game code), "overlay" (overlay code) and "sdk"
# (Psy-Q library code: crt0 and one unit per library object under psyq/, excluded from progress).
# Unit names are <target>/<path under asm/<t>/ or src/<t>/>.
SDK_UNIT_PREFIXES = {"main": ("crt0", "psyq/"), "stdwtitl": ("psyq/",)}


# The EXE's names for the overlays' splat configs: config/symbol_addrs.txt with `absolute:True`
# added, because splat ignores user symbols outside the segments it is splitting otherwise.
EXE_NAMES_FOR_OVERLAYS = "build/exe_symbols.txt"


def write_exe_names():
    out = []
    for line in (ROOT / "config/symbol_addrs.txt").read_text().splitlines():
        m = re.match(r"^(\w+ = 0x[0-9A-Fa-f]+;)\s*(?://\s*(.*))?$", line.strip())
        if m:
            out.append(f"{m.group(1)} // {(m.group(2) or '').strip()} absolute:True".replace("//  ", "// "))
    path = ROOT / EXE_NAMES_FOR_OVERLAYS
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("// Generated by configure.py from config/symbol_addrs.txt; do not edit.\n" + "\n".join(out) + "\n")


def split(t: Target, quiet: bool = False):
    if not quiet:
        print(f"splat: {t.yaml}")
    if t.name not in TIER2_PARENT.values():
        subprocess.run([SPLAT, "split", t.yaml], cwd=ROOT, check=True, stdout=subprocess.DEVNULL)
        align_small_data(ROOT / splat_paths(t)["ld_script_path"])
        return
    # A tier-2 parent: also dump splat's symbol context (an extra config merged into t.yaml) and keep the
    # parent's names for its tier-2 overlays' splat configs.
    dump_yaml = ROOT / "build/splat_dump_symbols.yaml"
    dump_yaml.parent.mkdir(parents=True, exist_ok=True)
    dump_yaml.write_text("# Generated by configure.py: merged into a tier-2 parent's config.\n"
                         "options:\n  dump_symbols: True\n")
    subprocess.run([SPLAT, "split", t.yaml, str(dump_yaml.relative_to(ROOT))], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    align_small_data(ROOT / splat_paths(t)["ld_script_path"])
    write_parent_names(t.name)


def write_parent_names(parent: str):
    """The parent's symbols as splat sees them (spimdisasm's context, base_path/.splat/spim_context.csv):
    every named function, jump table and data symbol in [OVERLAY_BASE, TIER2_BASE), with `absolute:True`
    (splat ignores user symbols outside the segments it splits otherwise) and the user-declared sizes."""
    splat_dir = ROOT / ".splat"
    out = []
    with open(splat_dir / "spim_context.csv", newline="") as f:
        for s in csv.DictReader(f):
            addr, name = int(s["address"], 16), s["getName"]
            if (s["category"] != "symbol" or not OVERLAY_BASE <= addr < TIER2_BASE or name.startswith(".")
                    or s["getType"] in ("@branchlabel", "@jumptablelabel")):
                continue
            attrs = ["type:func"] if s["getType"] == "@function" else []
            if s["userDeclaredSize"] not in ("", "None"):
                attrs.append(f"size:{s['userDeclaredSize']}")
            out.append(f"{name} = 0x{addr:08X}; // {' '.join(attrs + ['absolute:True'])}")
    shutil.rmtree(splat_dir)
    path = ROOT / parent_names(parent)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(f"// Generated by configure.py from {parent}'s splat run; do not edit.\n" + "\n".join(out) + "\n")


def align_small_data(ld_script: Path):
    """psylink starts every game object's .sdata, .sbss and .bss 8-aligned (.data and .text only
    4-aligned; DECISIONS "Data in C, split per object"). splat's linker script (SUBALIGN(4)) gets
    `. = ALIGN(8);` in front of those sections of the EXE's game units, C or asm (an asm subsegment
    already ends with the padding, so it is a no-op there). Psy-Q objects are left alone."""
    text = ld_script.read_text()
    pat = re.compile(r"^(\s*)(build/main/(?:src/main/\w+\.c|asm/main/data/\w+\.\w+\.s)\.o\(\.(?:sdata|sbss|bss)\);)$",
                     re.M)
    new = pat.sub(lambda m: f"{m.group(1)}. = ALIGN(8);\n{m.group(1)}{m.group(2)}", text)
    if new != text:
        ld_script.write_text(new)


def splat_paths(t: Target):
    opts = yaml.safe_load((ROOT / t.yaml).read_text())["options"]
    return {k: opts[k] for k in ("ld_script_path", "elf_path", "undefined_funcs_auto_path",
                                 "undefined_syms_auto_path", "asm_path", "src_path")}


def objects_from_ld(ld_script: Path):
    seen, objs = set(), []
    for m in re.finditer(r"^\s*(build/\S+?\.o)\(", ld_script.read_text(), re.M):
        if m.group(1) not in seen:
            seen.add(m.group(1))
            objs.append(m.group(1))
    return objs


def source_of(obj: str, t: Target):
    rel = obj[len(f"build/{t.name}/"):-len(".o")]   # asm/main/X.s, src/main/X.c, ...
    return rel


class Ninja:
    def __init__(self):
        self.lines = []

    def w(self, s=""):
        self.lines.append(s)

    def rule(self, name, command, description, **kw):
        self.w(f"rule {name}")
        self.w(f"  command = {command}")
        self.w(f"  description = {description}")
        for k, v in kw.items():
            self.w(f"  {k} = {v}")
        self.w()

    def build(self, outs, rule, ins=(), implicit=(), variables=None):
        outs = " ".join(outs) if isinstance(outs, (list, tuple)) else outs
        ins = " ".join(ins) if isinstance(ins, (list, tuple)) else ins
        imp = (" | " + " ".join(implicit)) if implicit else ""
        self.w(f"build {outs}: {rule} {ins}{imp}".rstrip())
        for k, v in (variables or {}).items():
            self.w(f"  {k} = {v}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--no-split", action="store_true", help="don't run splat")
    args = ap.parse_args()

    write_exe_names()
    write_wstag_configs()
    for parent in sorted(set(TIER2_PARENT.values())):
        write_tier2_calls(parent)
    if not args.no_split:
        for t in TARGETS:
            if t not in WSTAG_TARGETS:
                split(t)
        # The stage overlays are small and independent once FIELDSTG's names exist: split them in parallel.
        print(f"splat: {len(WSTAG_TARGETS)} WSTAG configs ({GEN_CONFIG}/wstag*.yaml)")
        # DW3_JOBS limits it (parallel agents on one machine share its memory; docs/MATCHING.md "Working in parallel").
        with concurrent.futures.ThreadPoolExecutor(int(os.environ.get("DW3_JOBS", os.cpu_count()))) as pool:
            list(pool.map(split, WSTAG_TARGETS, [True] * len(WSTAG_TARGETS)))

    n = Ninja()
    n.w("# Generated by configure.py; do not edit.")
    n.w("ninja_required_version = 1.10")
    n.w()
    n.rule("as", f"{BINUTILS}as {AS_FLAGS} --MD $out.d -o $out $in",
           "as $in", depfile="$out.d", deps="gcc")
    n.rule("bin", f"{BINUTILS}objcopy -I binary -O elf32-tradlittlemips -B mips $in $out", "bin $in")
    n.rule("cc", f"tools/cc_psx.sh -V $cc -G $g $flags -MD $out.d {CC_INCLUDES} $in -o $out",
           "cc $in (gcc $cc)", depfile="$out.d", deps="gcc")
    n.rule("ld", f"{BINUTILS}ld -EL -nostdlib $ldscripts -Map $map -o $out", "ld $out")
    # truncate: the file's size is the config's last segment end; a section padded to 4 bytes
    # (CNTY_SEL ends 3 bytes into a word) would otherwise leave extra bytes.
    n.rule("objcopy", f"{BINUTILS}objcopy -O binary $in $out && truncate -s $size $out", "objcopy $out")
    n.rule("sha1", f"echo \"$$(cut -d' ' -f1 $sha1)  $in\" | sha1sum --quiet -c - && touch $out",
           "sha1 $in")
    n.rule("syms", f"{BINUTILS}nm --defined-only -g $in | "
           f"awk '$$1 < \"{OVERLAY_BASE:08x}\" {{ printf \"%s = 0x%s;\\n\", $$3, $$1 }}' > $out",
           "syms $out")
    n.rule("configure", f"{PYTHON} configure.py", "configure", generator="1")
    n.build(EXE_SYMS, "syms", "build/main/SLES_039.36.elf")
    # Tier-2 parents: their ELF's global symbols in the tier-1 area, for their tier-2 overlays to link against.
    n.rule("parent_syms", f"{BINUTILS}nm --defined-only -g $in | "
           f"awk '$$1 >= \"{OVERLAY_BASE:08x}\" && $$1 < \"{TIER2_BASE:08x}\" "
           f"{{ printf \"%s = 0x%s;\\n\", $$3, $$1 }}' > $out", "parent_syms $out")
    for parent in sorted(set(TIER2_PARENT.values())):
        n.build(parent_syms_ld(parent), "parent_syms", f"build/{parent}/{parent.upper()}.PRO.elf")
    n.w()

    units = []
    finals = []
    expected_objs = []  # objdiff target objects for C units (built by default; objdiff doesn't build them)
    for t in TARGETS:
        p = splat_paths(t)
        ld_script = ROOT / p["ld_script_path"]
        if not ld_script.exists():
            sys.exit(f"{ld_script} missing: run without --no-split")
        objs = objects_from_ld(ld_script)
        n.w(f"# ---- {t.name} ({t.yaml}) ----")
        for obj in objs:
            src = source_of(obj, t)
            if src.endswith(".s"):
                n.build(obj, "as", src)
            elif src.endswith(".bin"):
                n.build(obj, "bin", src)
            elif src.endswith(".c"):
                cc = CC_OVERRIDES.get(src, DEFAULT_CC)
                n.build(obj, "cc", src, variables={"cc": cc, "g": g_for(src), "flags": " ".join(cc_flags(src))})
            else:
                sys.exit(f"don't know how to build {obj} (source {src})")

            # objdiff: every code unit. asm units only have a target (the original bytes);
            # C units diff their object against the assembled original asm of the same unit.
            stem = str(Path(*Path(src).with_suffix("").parts[2:]))  # asm/<t>/X.s -> X
            if src.endswith((".s", ".c")) and "/data/" not in src and stem != "header":
                sdk = stem.startswith(SDK_UNIT_PREFIXES.get(t.name, ()))
                cat = "sdk" if sdk else ("game" if t.name == "main" else
                                         "stage" if t in WSTAG_TARGETS else "overlay")
                unit = {"name": f"{t.name}/{stem}", "metadata": {"progress_categories": [cat]}}
                if src.endswith(".c"):
                    # build/expected/<src dir>/: the target's name, except for the WSTAG C units (src/wstag/)
                    expected = f"build/expected/{Path(src).parts[1]}/{stem}.s.o"
                    full_asm = f"{p['asm_path']}/{stem}.s"
                    n.build(expected, "as", full_asm)
                    expected_objs.append(expected)
                    unit.update(target_path=expected, base_path=obj)
                    unit["metadata"]["source_path"] = src
                else:
                    unit.update(target_path=obj)
                units.append(unit)

        ldscripts = [p["ld_script_path"], p["undefined_syms_auto_path"],
                     p["undefined_funcs_auto_path"], *t.ld_extra]
        elf, mapf = p["elf_path"], p["elf_path"][:-len(".elf")] + ".map"
        n.build(elf, "ld", objs, implicit=ldscripts,
                variables={"ldscripts": " ".join(f"-T {s}" for s in ldscripts), "map": mapf})
        size = yaml.safe_load((ROOT / t.yaml).read_text())["segments"][-1][0]
        n.build(t.output, "objcopy", elf, variables={"size": str(size)})
        n.build(t.output + ".ok", "sha1", t.output, implicit=[t.sha1], variables={"sha1": t.sha1})
        finals.append(t.output + ".ok")
        n.w()

    deps = ["configure.py", "config/symbol_addrs.txt", WSTAG_TABLE, WSTAG_C, "config/overlays.sha1"]
    deps += [t.yaml for t in TARGETS if t not in WSTAG_TARGETS]
    deps += [str(p.relative_to(ROOT)) for p in sorted((ROOT / "config").glob("*.symbols.txt"))]
    deps += [str(p.relative_to(ROOT)) for p in sorted((ROOT / "config/wstag").glob("*.symbols.txt"))]
    n.build("build.ninja", "configure", implicit=deps)
    n.w()
    n.w(f"default {' '.join(finals + expected_objs)}")
    (ROOT / "build.ninja").write_text("\n".join(n.lines) + "\n")

    objdiff = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "min_version": "3.0.0",
        "custom_make": "ninja",
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.c", "*.h", "*.s", "*.inc"],
        "progress_categories": [{"id": "game", "name": "Game code (EXE)"},
                                {"id": "overlay", "name": "Game code (overlays)"},
                                {"id": "stage", "name": "Game code (WSTAG stage overlays)"},
                                {"id": "sdk", "name": "Psy-Q SDK (not decompiled)"}],
        "units": units,
    }
    (ROOT / "objdiff.json").write_text(json.dumps(objdiff, indent=2) + "\n")
    print(f"wrote build.ninja ({len(TARGETS)} target(s)) and objdiff.json ({len(units)} units)")


if __name__ == "__main__":
    main()
