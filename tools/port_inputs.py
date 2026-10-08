#!/usr/bin/env python3
"""The PC port's build inputs (psxstack's GAME_CONTRACT.md "5. The build inputs"; port/CMakeLists.txt runs it at
configure time): what psxstack's generators need to know about this game, read from the tracked sources only.

  tools/port_inputs.py --out build/port/gen/inputs      # writes units.txt, overlays.txt, tag_sites.txt, volatile.txt

  units.txt      every C unit (src/<target>/*.c of the EXE and the tier-1/tier-2 overlays, the same set configure.py
                 compiles; src/wstag/<unit>.c for the WSTAG files in config/wstag_c.txt) and its overlay (MAIN for
                 the EXE's)
  overlays.txt   every overlay: name, its slot (1: the 19 stage overlays; 2: WFIGHTMN/WFIGHTTS and the WSTAG files),
                 its file ID (tier 1 from overlay_files in src/main/overlay.c, WFIGHTMN/WFIGHTTS from fightstg's
                 load_file calls (0x208/0x209, docs/DISC_LAYOUT.md), the WSTAG files from FIELDSTG's stage tables
                 joined with config/wstag.txt) and its symbol file (config/<overlay>.symbols.txt,
                 config/wstag/<n>.symbols.txt)
  tag_sites.txt  the tag sites whose overlay this game knows: WSTAG_ENTRY (the record's file), OVERLAY_ENTRY (the
                 comment names it), LATE_FUNC tier 1 (FIELDSTG); psxstack checks them against that overlay's table
  volatile.txt   tests/replay/replay.py's VOLATILE_RANGES: one definition for the emulator's records and the port's
"""
import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GAME_JSON = ROOT / "port/game/game.json"
TIER1 = ["FIELDSTG", "FIGHTSTG", "CARDGAME", "CNTY_SEL", "SHOCKTST", "SOUNDTST", "STAGSLCT", "STCRDABM", "STCRDDEK",
         "STCRDSHP", "STDGNAME", "STDWTITL", "STFGTREP", "STGDGLAB", "STGMCARD", "STGTRAIN", "STITSHOP", "STPLNMET",
         "STSTATUS"]
TIER2_FIXED = {"WFIGHTMN": 0x208, "WFIGHTTS": 0x209}   # FIGHTSTG's overlay_module.load_file(0x208/0x209)
WSTAG_TABLE, WSTAG_C = ROOT / "config/wstag.txt", ROOT / "config/wstag_c.txt"
STAGE_TABLES = ROOT / "src/fieldstg/fieldstg_80087DB0.c"
OVERLAY_C = ROOT / "src/main/overlay.c"
REPLAY_PY = ROOT / "tests/replay/replay.py"


def _addr(v):
    return int(v, 0) if isinstance(v, str) else int(v)


SLOT2_BASE = _addr(json.loads(GAME_JSON.read_text())["memory"]["slots"][1]["base"])


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


def units():
    """-> [(src path relative to ROOT, overlay name or 'MAIN')] in a stable order."""
    out = []
    for d in ["main"] + [n.lower() for n in TIER1] + [n.lower() for n in TIER2_FIXED]:
        for p in sorted((ROOT / "src" / d).glob("*.c")):
            out.append((p.relative_to(ROOT).as_posix(), d.upper()))
    for name, flags in wstag_c_lines():
        for u in wstag_c_units(name, flags):
            out.append((f"src/wstag/{u}.c", name))
    return out


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


def symbols_file(name):
    if name.startswith("WSTAG"):
        return ROOT / f"config/wstag/{name.lower()}.symbols.txt"
    return ROOT / f"config/{name.lower()}.symbols.txt"


def tier_of(name):
    return 1 if name in TIER1 else 2


def all_overlays():
    """Every overlay name, tier 1 first, then WFIGHT*, then the WSTAG files in config/wstag.txt order."""
    names = list(TIER1) + list(TIER2_FIXED)
    names += [ln.split()[0] for ln in WSTAG_TABLE.read_text().splitlines() if ln.strip() and not ln.startswith("#")]
    return names


# ------------------------------------------------------------------------------------------------------------ tag sites

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


# ------------------------------------------------------------------------------------------------------------- volatile

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


# ----------------------------------------------------------------------------------------------------------------- main

def write(path, text):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True, help="the directory for the four files")
    args = ap.parse_args()
    out = Path(args.out)
    us = units()
    missing = [u for u, _ in us if not (ROOT / u).exists()]
    if missing:
        sys.exit(f"port_inputs: missing {' '.join(missing)}")
    extra = sorted(set(p.relative_to(ROOT).as_posix() for p in (ROOT / "src").rglob("*.c")) - set(u for u, _ in us))
    if extra:
        sys.exit(f"port_inputs: C files under src/ that no target compiles: {' '.join(extra)}")
    write(out / "units.txt", "# Generated by tools/port_inputs.py; do not edit. <source>\t<overlay>\n"
          + "".join(f"{ROOT / u}\t{o}\n" for u, o in us))
    ids = overlay_file_ids()
    names = all_overlays()
    write(out / "overlays.txt", "# Generated by tools/port_inputs.py; do not edit. <name>\t<tier>\t<file id>\t<symbols>\n"
          + "".join(f"{n}\t{tier_of(n)}\t0x{ids[n]:X}\t{symbols_file(n)}\n" for n in names))
    sites = tag_sites()
    write(out / "tag_sites.txt", "# Generated by tools/port_inputs.py; do not edit. <where>\t<tier>\t<overlay>\t<addr>\n"
          + "".join(f"{w}\t{t}\t{'file:0x%X' % o if isinstance(o, int) else (o or '-')}\t0x{a:08X}\n"
                    for w, t, o, a in sites))
    vol = volatile_ranges()
    write(out / "volatile.txt", "# Generated by tools/port_inputs.py from tests/replay/replay.py VOLATILE_RANGES; do not edit.\n"
          + "".join(f"0x{lo:X} 0x{hi:X}\n" for lo, hi in vol))
    print(f"port_inputs: {len(us)} units, {len(names)} overlays ({len(TIER1)} tier 1), {len(sites)} known tag sites, "
          f"{len(vol)} volatile ranges -> {out}")


if __name__ == "__main__":
    main()
