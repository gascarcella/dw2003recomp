#!/usr/bin/env python3
"""Print the game's text tables as readable text (docs/FORMATS.md "Text").

  tools/venv/bin/python tools/dump_text.py ITMNAM              # ENG item names (file name without the ?S prefix)
  tools/venv/bin/python tools/dump_text.py SKLNAM -l FRA       # another language: JPN USA ENG FRA ITA GER SPN
  tools/venv/bin/python tools/dump_text.py 0x112 -l GER        # by the Japanese file's ID (+ language, like the game)
  tools/venv/bin/python tools/dump_text.py DMG900 -s 3         # one table of a container file (the DMG event texts)
  tools/venv/bin/python tools/dump_text.py TALK00 -r           # raw: the game's bytes per entry, in hex
  tools/venv/bin/python tools/dump_text.py --check             # decode every text file of every language, report

Text is the game's 1-byte glyph code (message_decode_char with MessageLine.unk_A == 0): 0 ends, 1 xx is an extra
glyph, 2 c ... a control code (message_code_lengths), 4..0xE9 a glyph. font_chars / font_chars_ext give each glyph's
Shift-JIS code; they are read from the user's EXE (extracted/disc/SLES_039.36), not embedded. The European fonts put
accented letters on the glyphs whose Shift-JIS code is a kanji (font_chars_ext 0x3B..); LATIN is our reading of
them from word context (not checked against the font image yet). Control codes print as <code,args>.
"""
import argparse
import signal
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import disc_files as df  # noqa: E402

FONT_CHARS, FONT_CHARS_EXT = 0x8004E010, 0x8004E3BC
CODE_LENGTHS = [1, 2, 3, 2, 5, 3, 3, 2, 3, 2, 0]  # message_code_lengths
LANGS = ["JPN", "USA", "ENG", "FRA", "ITA", "GER", "SPN"]  # records_language 0..6
# Extra glyphs (01 xx) that the European fonts draw as other letters: inferred from the words they occur in.
LATIN = {
    0x3B: "¡", 0x3F: "¿", 0x41: "Ä", 0x45: "Ñ", 0x47: "Ö", 0x49: "Ü", 0x4A: "ß", 0x4B: "à", 0x4C: "â", 0x4E: "ä",
    0x4F: "ç", 0x50: "è", 0x51: "é", 0x52: "ê", 0x54: "ì", 0x55: "î", 0x56: "ï", 0x57: "ñ", 0x58: "ò", 0x59: "ô",
    0x5B: "ö", 0x5C: "ù", 0x5D: "û", 0x5E: "ü", 0x5F: "á", 0x60: "í", 0x61: "ó", 0x62: "ú", 0x69: "É", 0x6A: "Í",
    0x6B: "Ó", 0x6C: "Ú", 0x72: "º",
}


def char_tables():
    exe = df.EXE.read_bytes()

    def read(addr):
        out, o = {}, addr - df.EXE_OFF
        while True:
            code, glyph = struct.unpack_from("<HB", exe, o)
            if code == 0xFFFF:
                return out
            out.setdefault(glyph, code)
            o += 4
    chars = read(FONT_CHARS)
    ext = read(FONT_CHARS_EXT)
    chars = {g: c for g, c in chars.items() if g >= 4}  # entries 0-3 are the lead bytes
    ext.pop(0, None)
    return chars, ext


def sjis(code):
    s = struct.pack(">H", code).decode("shift_jis", "replace")
    if 0xFF01 <= ord(s) <= 0xFF5E:  # full-width ASCII
        return chr(ord(s) - 0xFEE0)
    return " " if s == "　" else s


def decode(d, o, tables, latin=True):
    """One entry from offset o; returns (text, unknown bytes seen)."""
    chars, ext = tables
    out, bad = [], 0
    while o < len(d) and d[o] != 0:
        b = d[o]
        if b == 1:
            g = d[o + 1]
            if latin and g in LATIN:
                out.append(LATIN[g])
            elif g in ext:
                out.append(sjis(ext[g]))
            else:
                out.append(f"{{01 {g:02X}}}")
                bad += 1
            o += 2
        elif b == 2:
            c = d[o + 1]
            n = CODE_LENGTHS[c] if c < len(CODE_LENGTHS) and CODE_LENGTHS[c] >= 2 else 2
            out.append("<" + ",".join(str(x) for x in d[o + 1:o + n]) + ">")
            if c == 1:
                out.append("\n    ")
            o += n
        elif b == 3:
            out.append(f"{{03 {d[o + 1]:02X}}}")
            bad += 1
            o += 2
        elif b in chars:
            out.append(sjis(chars[b]))
            o += 1
        else:
            out.append(f"{{{b:02X}}}")
            bad += 1
            o += 1
    return "".join(out), bad


def table_offsets(d, base=0):
    """font_get_entry's table: s32 count, then count offsets (from the table's start). None if it isn't one."""
    if base + 8 > len(d):
        return None
    n = struct.unpack_from("<i", d, base)[0]
    if not 0 < n < 5000 or base + 4 + 4 * n > len(d):
        return None
    offs = struct.unpack_from(f"<{n}i", d, base + 4)
    if offs[0] != 4 + 4 * n:
        return None
    return [base + x for x in offs]


def tables_of(d):
    """[(sub-file or None, offsets)]: a table, or a cdload container of tables (the ?SDMG### event texts)."""
    offs = table_offsets(d)
    if offs:
        return [(None, offs)]
    n = struct.unpack_from("<i", d, 0)[0] // 4 if len(d) >= 4 else 0
    out = []
    for i in range(n):
        t = table_offsets(d, df.subfile(d, i))
        if t:
            out.append((i, t))
    return out


def find_file(name, lang):
    if name.lower().startswith("0x") or name.isdigit():
        return df.files()[int(name, 0) + LANGS.index(lang)]
    for e in df.files():
        p = e.path.split("/")
        if len(p) == 5 and p[2] == "COUNTRY" and p[3] == lang and p[4][2:-4] == name.upper():
            return e
    sys.exit(f"no text file {name!r} for {lang}")


def check(tables):
    """Decode every entry of every text file; also count the tables and check that entry 0 of each is the empty
    string (docs/FORMATS.md "Containers", shape 3)."""
    total = bad = ntables = 0
    for e in df.files():
        p = e.path.split("/")
        if len(p) != 5 or p[2] != "COUNTRY":
            continue
        d = (df.DISC / e.path).read_bytes()
        found = tables_of(d)
        if not found and any(d):
            print(f"{e.path}: not a text table")
            bad += 1
        for sub, offs in found:
            ntables += 1
            if d[offs[0]]:
                print(f"{e.path}{'' if sub is None else f' table {sub}'}: entry 0 is not the empty string")
                bad += 1
            for o in offs:
                _, b = decode(d, o, tables)
                total += 1
                bad += b
    print(f"{total} entries in {ntables} tables decoded, {bad} unknown bytes, non-table files or non-empty entries 0")
    return bad == 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("file", nargs="?", help="file name without the language prefix (ITMNAM), or the JPN file ID")
    ap.add_argument("-l", "--lang", default="ENG", choices=LANGS)
    ap.add_argument("-s", "--sub", type=int, help="table (sub-file) of a container file")
    ap.add_argument("-r", "--raw", action="store_true", help="print each entry's bytes in hex")
    ap.add_argument("--sjis", action="store_true", help="print extra glyphs as their Shift-JIS character")
    ap.add_argument("--check", action="store_true", help="decode every text file, report unknown bytes")
    a = ap.parse_args()
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # allow `| head`
    tables = char_tables()
    if a.check:
        sys.exit(0 if check(tables) else 1)
    if not a.file:
        ap.error("give a file (or --check)")
    e = find_file(a.file, a.lang)
    d = (df.DISC / e.path).read_bytes()
    print(f"# {e.path} (file ID 0x{e.id:03X})")
    for sub, offs in tables_of(d):
        if a.sub is not None and sub != a.sub:
            continue
        if sub is not None:
            print(f"## table {sub}")
        for i, o in enumerate(offs):
            if a.raw:
                end = d.index(0, o) if 0 in d[o:] else len(d)
                print(f"{i:4}: {d[o:end].hex(' ')}")
            else:
                print(f"{i:4}: {decode(d, o, tables, latin=not a.sjis and a.lang != 'JPN')[0]}")


if __name__ == "__main__":
    main()
