#!/usr/bin/env python3
"""Find an overlay's .rodata / .text / .data boundaries (psylink layout: .rodata, .text, .data).

  tools/venv/bin/python tools/overlay_layout.py [NAME ...]     # default: every tier-1 overlay
  tools/venv/bin/python tools/overlay_layout.py --tier2        # every tier-2 overlay (WFIGHT*, WSTAG*)
  tools/venv/bin/python tools/overlay_layout.py --wstag-table > config/wstag.txt   # configure.py's WSTAG table

Each overlay is laid out at its load address (disc_survey.overlay_base: tier 1 0x80082CB0, tier 2 0x800A5DE0).

.text ends after the last `jr $ra` and its delay slot (data words equal to `jr $ra` would fool it; the
build and the disassembly check that). .rodata lies before the first function with a stack frame:
jump tables (runs of words pointing into .text), then other items the code reaches (strings,
constants); .text starts after them, or at that first function if no leaf function (`jr $ra`) lies
in between. Checks printed per overlay: every jump-table target and in-overlay `jal`
target lies inside .text, and the first .text word looks like a function start.

--wstag-table also checks each WSTAG against FIELDSTG's stage tables (12-byte records: stage, file ID, entry;
file IDs are matched to names through the EXE's LBA table and iso/dw2003.bin): the entry must be a function
start inside the .text found here. It fails if any check fails.
"""
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from disc_survey import PRO, TIER1_BASE, TIER2_BASE, hilo_addresses, overlay_base, words  # noqa: E402

JR_RA = 0x03E00008


def is_prologue(w):
    return w >> 16 == 0x27BD and w & 0x8000  # addiu $sp, $sp, -N


def layout(path: Path):
    d = path.read_bytes()
    ws = words(d)
    base = overlay_base(path.name)
    end = base + len(d)
    last_jr = max(i for i, w in enumerate(ws) if w == JR_RA)
    text_end = base + (last_jr + 2) * 4
    in_text = lambda w: base <= w < text_end  # noqa: E731
    # The first function with a stack frame; .rodata lies before it.
    p = next(i for i, w in enumerate(ws) if is_prologue(w))
    # Jump tables: runs of pointers into the overlay before that function.
    tables, i = [], 0
    while i < p:
        if in_text(ws[i]):
            j = i
            while j < p and in_text(ws[j]):
                j += 1
            tables.append((i, j))
            i = j
        else:
            i += 1
    ro_end = tables[-1][1] if tables else 0
    # Other .rodata the code reaches (strings, constants) between the last table and that function.
    refs = sorted({(a - base) // 4 for a in hilo_addresses(ws) if base + ro_end * 4 <= a < base + p * 4
                   and a % 4 == 0})
    for r in refs:
        raw = d[r * 4:p * 4]
        z = raw.find(b"\0")
        # A string (printable or Shift-JIS bytes up to a NUL) or else one word (WSTAG960's 0x01966754).
        is_str = z > 0 and all(c >= 0x20 for c in raw[:z])
        n = (z + 4) // 4 if is_str else 1
        ro_end = max(ro_end, r + n)
    # Leaf functions (no frame) between .rodata and the first frame: they end in jr $ra.
    gap = ws[ro_end:p]
    text_start = base + (ro_end if JR_RA in gap else p) * 4
    jal_in = sorted({((w & 0x03FFFFFF) << 2) | (base & 0xF0000000) for w in ws[(text_start - base) // 4:last_jr + 2]
                     if w >> 26 == 3} & set(range(base, text_end, 4)))
    bad_tbl = [hex(ws[k]) for a, b in tables for k in range(a, b) if not text_start <= ws[k] < text_end]
    bad_jal = [hex(a) for a in jal_in if a < text_start]
    first = ws[(text_start - base) // 4]
    return dict(name=path.name, base=base, size=len(d), text_start=text_start, text_end=text_end, end=end,
                rodata_refs=len(refs), tables=len(tables), bad_tbl=bad_tbl, bad_jal=bad_jal,
                first_word=first, prologue=is_prologue(first))


EXE_OFF = 0x80010000 - 0x800                 # SLES_039.36 file offset of an EXE address
FILETABLE_LBA, FILETABLE_SECTORS, FILE_COUNT = 0x80044F6C, 0x800474A4, 2382
# FIELDSTG's two stage tables of (stage, file ID, entry) records, back to back (fieldstg_get_actor_width picks one):
# fieldstg_stages_2d (55 records) when gamestate_data.unk_263C == 0x2D, else fieldstg_stages (239 records).
FIELDSTG_STAGES = (0x8009A5F0, 0x8009B3B8)
FIELDSTG_STAGES_2 = 0x8009A884
ISO = ROOT / "iso" / "dw2003.bin"


def wstag_entries():
    """{WSTAG name: (table, stage, entry)} from FIELDSTG's stage tables, for the records that point into tier 2."""
    import mmap
    exe = (PRO.parent.parent / "SLES_039.36").read_bytes()
    lba = struct.unpack_from(f"<{FILE_COUNT}I", exe, FILETABLE_LBA - EXE_OFF)
    secs = struct.unpack_from(f"<{FILE_COUNT}H", exe, FILETABLE_SECTORS - EXE_OFF)
    fs = (PRO / "FIELDSTG.PRO").read_bytes()
    recs = [(a, *struct.unpack_from("<3I", fs, a - TIER1_BASE)) for a in range(*FIELDSTG_STAGES, 12)]
    names = {}
    with open(ISO, "rb") as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as iso:
        for p in PRO.glob("WSTAG*.PRO"):
            head, n = p.read_bytes()[:2048], (p.stat().st_size + 2047) // 2048
            for i in range(FILE_COUNT):
                o = lba[i] * 2352 + 24  # MODE2/2352 sector: 24 header bytes before the user data
                if secs[i] == n and iso[o:o + len(head)] == head:
                    names[i] = p.name
                    break
    return {names[f]: ("A884" if a >= FIELDSTG_STAGES_2 else "A5F0", s, e)
            for a, s, f, e in recs if e >= TIER2_BASE and f in names}


def wstag_table():
    entries = wstag_entries()
    print("# WSTAG### stage overlays (tier 2, loaded at 0x800A5DE0 by FIELDSTG), made by")
    print("# tools/overlay_layout.py --wstag-table; configure.py generates one splat config per line.")
    print("# name      size     .text   .data    table and stage (FIELDSTG's table at 0x8009<table>), entry")
    bad = 0
    for p in sorted(PRO.glob("WSTAG*.PRO")):
        if overlay_base(p.name) is None:
            continue
        r = layout(p)
        ws = words(p.read_bytes())
        table, stage, entry = entries.get(p.name, ("----", None, None))
        i = (entry - r["base"]) // 4 if entry else 0
        ok = (entry is not None and r["text_start"] <= entry < r["text_end"] and not r["bad_tbl"]
              and not r["bad_jal"] and (is_prologue(ws[i]) or entry == r["text_start"] or ws[i - 2] == JR_RA))
        bad += not ok
        print(f"{p.stem:9} 0x{r['size']:05X}  0x{r['text_start'] - r['base']:04X}  0x{r['text_end'] - r['base']:04X}"
              f"   {table} 0x{stage or 0:03X}  0x{entry or 0:08X}{'' if ok else '  # CHECK'}")
    if bad:
        sys.exit(f"{bad} WSTAG overlay(s) failed a check")


def main():
    args = sys.argv[1:]
    if args == ["--wstag-table"]:
        return wstag_table()
    tier = TIER2_BASE if "--tier2" in args else TIER1_BASE
    names = [a for a in args if a != "--tier2"] or sorted(p.name for p in PRO.glob("*.PRO") if overlay_base(p.name) == tier)
    for n in names:
        if not n.endswith(".PRO"):
            n += ".PRO"
        r = layout(PRO / n)
        ok = not r["bad_tbl"] and not r["bad_jal"]
        print(f"{r['name']:13} size 0x{r['size']:05X}  .rodata 0x{r['base']:08X} ({r['tables']} tables, "
              f"{r['rodata_refs']} refs)  .text 0x{r['text_start']:08X}-0x{r['text_end']:08X}  "
              f".data -0x{r['end']:08X}  first word {r['first_word']:08X}{' (prologue)' if r['prologue'] else ''}"
              f"{'' if ok else '  CHECK: ' + str(r['bad_tbl'][:3] + r['bad_jal'][:3])}")


if __name__ == "__main__":
    main()
