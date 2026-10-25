#!/usr/bin/env python3
"""Print the game's text tables as readable text (docs/FORMATS.md "Text").

  tools/venv/bin/python tools/dump_text.py ITMNAM              # ENG item names (file name without the ?S prefix)
  tools/venv/bin/python tools/dump_text.py SKLNAM -l FRA       # another language: JPN USA ENG FRA ITA GER SPN
  tools/venv/bin/python tools/dump_text.py 0x112 -l GER        # by the Japanese file's ID (+ language, like the game)
  tools/venv/bin/python tools/dump_text.py DMG900 -s 3         # one table of a container file (the DMG event texts)
  tools/venv/bin/python tools/dump_text.py TALK00 -r           # raw: the game's bytes per entry, in hex
  tools/venv/bin/python tools/dump_text.py --check             # decode every text file of every language, report
  tools/venv/bin/python tools/dump_text.py --font-png f.png    # draw the 14 px font's extra glyphs (01 xx), labelled
  tools/venv/bin/python tools/dump_text.py --font-png f.png --font 11 --base   # the 11 px font's codes 4..0xE9

Text is the game's 1-byte glyph code (message_decode_char with MessageLine.is_sjis == 0): 0 ends, 1 xx is an extra
glyph, 2 c ... a control code (message_code_lengths), 4..0xE9 a glyph. font_chars / font_chars_ext give each glyph's
Shift-JIS code; they are read from the user's EXE (extracted/disc/SLES_039.36), not embedded. The European fonts put
accented letters on the glyphs whose Shift-JIS code is a kanji (font_chars_ext 0x3B..0x72); LATIN gives them, read
off the font image (--font-png). Control codes print as <code,args>; a byte with no reading prints as {XX} or {01 XX}.
tools/text_codec.py encodes this notation back to bytes.
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
# Extra glyphs (01 xx) that the European fonts draw as other letters, read off the font image (dump_text.py --font-png,
# the 14 and 11 px fonts; the 9 px font has none of them). docs/FORMATS.md "Text", "European letters".
LATIN = dict(zip(range(0x3B, 0x73), "¡£§µ¿ÃÄÅÆÇÑÕÖØÜßàâãäçèéêëìîïñòôõöùûüáíóúÂÊÔËÏÁÉÍÓÚÀÈÌÒÙº"))
# Extra glyphs whose Shift-JIS reading collides with a LATIN letter; the European fonts draw icons there.
LATIN_RAW = {0x29, 0x2C}  # £ and § in font_chars_ext


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


def decode(d, o, tables, latin=True, nl="\n    "):
    """One entry from offset o; returns (text, unknown bytes seen). `nl` follows each <1> (display only)."""
    chars, ext = tables
    out, bad = [], 0
    while o < len(d) and d[o] != 0:
        b = d[o]
        if b == 1:
            g = d[o + 1]
            if latin and g in LATIN:
                out.append(LATIN[g])
            elif g in ext and not (latin and g in LATIN_RAW):
                out.append(sjis(ext[g]))
            else:
                out.append(f"{{01 {g:02X}}}")
                bad += 1
            o += 2
        elif b == 2:
            c = d[o + 1]
            n = CODE_LENGTHS[c] if c < len(CODE_LENGTHS) and CODE_LENGTHS[c] >= 2 else 2
            out.append("<" + ",".join(str(x) for x in d[o + 1:o + n]) + ">")
            if c == 1 and nl:
                out.append(nl)
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


FONT_FILE = 0x287  # COUNTRY/CMFONTTM.BIN: sub-file 0 is the TIM list message_load_font loads at (0x140, 0)
FONT_VRAM_X = 0x140
# message_fonts[1..3]: (height, glyphs for codes 4..0xE9, glyphs_ext for 01 01..01 72)
FONTS = {14: (0x8004E5A4, 0x8004EF88), 11: (0x8004F470, 0x8004FE54), 9: (0x8005033C, 0x80050D20)}
FONT_NUMBER = {1: 14, 2: 11, 3: 9}  # message_create_window / message_set_font argument -> height
Glyph = struct.Struct("<BBBBBBBbbBB")  # FontGlyph: tpage_x, u, v, clut_x, clut_y, w, h, x_off, y_off, advance, ?


def font_glyphs(height=14):
    """(glyphs, glyphs_ext) of a font as lists of FontGlyph tuples: glyphs[code - 4], glyphs_ext[xx - 1]."""
    exe = df.EXE.read_bytes()
    a, x = FONTS[height]
    return ([Glyph.unpack_from(exe, a - df.EXE_OFF + i * 11) for i in range(230)],
            [Glyph.unpack_from(exe, x - df.EXE_OFF + i * 11) for i in range(114)])


def glyph_width(g):
    """What font_get_text_width adds per glyph (advance + x offset); 0 for a missing glyph (tpage_x 0xFF)."""
    return 0 if g[0] == 0xFF else g[9] + g[7]


def unpack_rlen(b):
    """docs/FORMATS.md "Containers", shape 4."""
    if b[:4] != b"RLEN":
        return b
    n, o, out = struct.unpack_from("<i", b, 4)[0], 8, bytearray()
    while b[o]:
        c = b[o]
        if c < 0x80:
            out += b[o + 1:o + 1 + c]
            o += 1 + c
        else:
            out += bytes([b[o + 1]]) * (c & 0x7F)
            o += 2
    return bytes(out[:n])


def font_vram():
    """The font page as message_load_font leaves it: {(vram x, y): u16}, each TIM 64 halfwords right of the last."""
    d = df.read(FONT_FILE)
    sub = d[df.subfile(d, 0):]
    vram, x, i = {}, FONT_VRAM_X, 0
    while True:
        off = struct.unpack_from("<i", sub, i * 4)[0]
        if off == 0:
            return vram
        t = unpack_rlen(sub[off:])
        p = 8 + (struct.unpack_from("<I", t, 8)[0] if struct.unpack_from("<I", t, 4)[0] & 8 else 0)
        w, h = struct.unpack_from("<HH", t, p + 8)
        px = struct.unpack_from(f"<{w * h}H", t, p + 12)
        for y in range(h):
            for xx in range(w):
                vram[(x + xx, y)] = px[y * w + xx]
        x, i = x + 0x40, i + 1


# 3x5 hex digits for the cell labels of --font-png (drawn by us, not the game's)
_HEX_ROWS = {  # rows top to bottom, 3 bits each (4 = left pixel)
    "0": (7, 5, 5, 5, 7), "1": (2, 6, 2, 2, 7), "2": (7, 1, 7, 4, 7), "3": (7, 1, 3, 1, 7), "4": (5, 5, 7, 1, 1),
    "5": (7, 4, 7, 1, 7), "6": (7, 4, 7, 5, 7), "7": (7, 1, 1, 1, 1), "8": (7, 5, 7, 5, 7), "9": (7, 5, 7, 1, 7),
    "A": (2, 5, 7, 5, 5), "B": (6, 5, 6, 5, 6), "C": (7, 4, 4, 4, 7), "D": (6, 5, 5, 5, 6), "E": (7, 4, 6, 4, 7),
    "F": (7, 4, 6, 4, 4),
}


def write_png(path, rows):
    """rows: list of lists of (r, g, b). Plain zlib PNG (the venv has no PIL)."""
    import zlib
    h, w = len(rows), len(rows[0])
    raw = b"".join(b"\0" + bytes(c for px in row for c in px) for row in rows)

    def chunk(t, b):
        return struct.pack(">I", len(b)) + t + b + struct.pack(">I", zlib.crc32(t + b) & 0xFFFFFFFF)
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                           + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def font_png(path, height=14, ext=True, scale=4):
    """Draw a font's glyphs as the game does (4-bit texels through the glyph's CLUT, palette 0; colour 0x0000 is
    transparent) in a grid of 16 per row, each cell labelled with its code: `xx` of `01 xx` with ext, else the
    1-byte code. A red bar under each glyph shows its advance + x offset (the width the game measures)."""
    vram = font_vram()
    glyphs, gext = font_glyphs(height)
    sel = [(i + 1, g) for i, g in enumerate(gext)] if ext else [(i + 4, g) for i, g in enumerate(glyphs)]
    cw, ch = 20, 6 + height + 4
    cols, nrows = 16, (len(sel) + 15) // 16
    bg, grid = (40, 44, 80), (90, 90, 120)
    img = [[bg] * (cols * cw * scale) for _ in range(nrows * ch * scale)]

    def put(x, y, col):
        for a in range(scale):
            img[y * scale + a][x * scale:(x + 1) * scale] = [col] * scale
    for n, (code, g) in enumerate(sel):
        cx, cy = (n % cols) * cw, (n // cols) * ch
        for x in range(cw):
            put(cx + x, cy, grid)
        for y in range(ch):
            put(cx, cy + y, grid)
        for k, digit in enumerate(f"{code:02X}"):
            for yy, bits in enumerate(_HEX_ROWS[digit]):
                for xx in range(3):
                    if bits & (4 >> xx):
                        put(cx + 2 + k * 4 + xx, cy + 1 + yy, (255, 255, 160))
        tx, u, v, clx, cly, w, h, xo, yo = g[:9]
        if tx == 0xFF:
            continue
        clut = [vram.get((FONT_VRAM_X + clx + i, cly), 0) for i in range(16)]
        for yy in range(h):
            for xx in range(w):
                hw = vram.get((FONT_VRAM_X + tx * 64 + (u + xx) // 4, v + yy), 0)
                c = clut[(hw >> ((u + xx) % 4 * 4)) & 0xF]
                if c & 0x7FFF or c & 0x8000:
                    put(cx + 2 + xo + xx, cy + 7 + yo + yy,
                        ((c & 31) * 255 // 31, (c >> 5 & 31) * 255 // 31, (c >> 10 & 31) * 255 // 31))
        for xx in range(glyph_width(g)):
            put(cx + 2 + xx, cy + ch - 2, (220, 60, 60))
    write_png(path, img)


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
    ap.add_argument("--font-png", metavar="PNG", help="draw a font's glyphs as the game does, labelled with codes")
    ap.add_argument("--font", type=int, default=14, choices=sorted(FONTS), help="with --font-png: the font (height)")
    ap.add_argument("--base", action="store_true", help="with --font-png: the 1-byte codes 4..0xE9, not the 01 xx")
    ap.add_argument("--scale", type=int, default=4, help="with --font-png: pixels per texel")
    a = ap.parse_args()
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # allow `| head`
    if a.font_png:
        font_png(a.font_png, a.font, not a.base, a.scale)
        print(f"{a.font_png}: {a.font} px font, {'codes 4..0xE9' if a.base else 'extra glyphs 01 01..01 72'}")
        return
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
