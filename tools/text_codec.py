#!/usr/bin/env python3
"""Encode text in tools/dump_text.py's notation back to the game's bytes, and measure and lay it out the way the
game's message windows do (docs/FORMATS.md "Text", "Font").

  tools/venv/bin/python tools/text_codec.py --roundtrip                 # every entry of every text file: decode, encode, compare
  tools/venv/bin/python tools/text_codec.py --encode 'Olá, <8,1><5,1>!'  # the bytes of a text, in hex
  tools/venv/bin/python tools/text_codec.py --width 'Não sei.<1>Você sabe?' --font 11   # each line's width in pixels
  tools/venv/bin/python tools/text_codec.py --chars                     # every character the encoder takes

Notation (what dump_text.py prints, without the line breaks it adds for display):
  <c,args>   control code c with its argument bytes: <1> new line, <2,2> wait for a button, <3> the next page,
             <5,n> insert text line n, <7>Name<7> speaker, <8,n> player's name into line n, <9> (in a speaker tag)
             the player. message_code_lengths gives each code's length; a wrong argument count is an error.
  {XX}, {01 XX}  raw bytes in hex (what dump_text prints for a byte it cannot read).
  characters  each a glyph: font_chars (codes 4..0xE9: digits, letters, kana, ・?!ー〜) or font_chars_ext (01 xx),
             with the European letters of dump_text.LATIN on 01 3B..01 72. A character with both a 1-byte and an
             01 xx glyph encodes as the 1-byte one, as on the disc. On input only, ASCII stand-ins are taken for the
             font's own characters (ALIASES: ' " - ~ ° and typographic quotes and dashes). A newline right after <1> is
             ignored; any other newline is a <1>.
JPN text (Shift-JIS readings, no LATIN) is Codec(latin=False).

As a module: Codec(latin=True) .decode(bytes) / .encode(text, where="") / .tokens(text); Metrics(height) for glyph
widths; layout helpers split_head / split_pages / line_width (tools/text_tools.py fits and checks translations).
Reads extracted/disc/SLES_039.36 and the disc's font file; embeds no game data.
"""
import argparse
import re
import sys
import unicodedata
from functools import lru_cache
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import disc_files as df  # noqa: E402
import dump_text as dt  # noqa: E402

# Input-only stand-ins: what a translator types -> the character the font has (all checked against the font image)
ALIASES = {"'": "’", "‘": "’", "`": "’", "´": "’", '"': "”", "“": "”", "„": "”", "-": "−", "‐": "−", "–": "−",
           "—": "―", "~": "〜", "•": "・", "°": "º", "　": " ", " ": " "}
SPACE = b"\x01\x01"
INSERT_WIDTH = 48  # nominal width of a <5,n> insert (player's name, a number, an item name): same for EN and PT
LAYOUT_CODES = {1, 2, 3}  # new line, wait for a button, next page: the fitter may add or move these
HEAD_RE = re.compile(r"<7>(.*?)<7>")


class EncodeError(ValueError):
    pass


class Codec:
    def __init__(self, latin=True):
        self.latin = latin
        self.tables = dt.char_tables()
        chars, ext = self.tables
        enc = {}
        for g, code in ext.items():  # 01 xx first, so the 1-byte glyphs win (as the disc has them)
            if latin and (g in dt.LATIN or g in dt.LATIN_RAW):
                continue
            enc.setdefault(dt.sjis(code), bytes([1, g]))
        for g, code in chars.items():
            enc[dt.sjis(code)] = bytes([g])
        if latin:
            for g, ch in dt.LATIN.items():
                enc[ch] = bytes([1, g])
        enc.pop("\n", None)
        self.enc = enc

    def decode(self, data, o=0):
        """One entry (from offset o, up to its 0) in the notation, with no display line breaks."""
        return dt.decode(data, o, self.tables, latin=self.latin, nl="")[0]

    def tokens(self, text, where=""):
        """[(kind, bytes, text)]: kind "g" a glyph (text its character), or ("c", code, args) for a control code."""
        out, i, n = [], 0, len(text)
        while i < n:
            ch = text[i]
            if ch == "<":
                m = re.match(r"<(\d+)((?:,\d+)*)>", text[i:])
                if m:
                    code = int(m.group(1))
                    args = [int(x) for x in m.group(2).split(",")[1:]]
                    if code >= len(dt.CODE_LENGTHS) - 1:
                        raise EncodeError(f"{where}: no control code {code} ({m.group(0)}; codes are 0-9)")
                    want = max(dt.CODE_LENGTHS[code], 2) - 2
                    if len(args) != want or any(a > 255 for a in args):
                        raise EncodeError(f"{where}: bad control code {m.group(0)} (code {code} takes {want} "
                                          f"argument byte{'s' if want != 1 else ''})")
                    out.append((("c", code, tuple(args)), bytes([2, code, *args]), m.group(0)))
                    i += m.end()
                    if code == 1 and i < n and text[i] == "\n":
                        i += 1
                    continue
            if ch == "{":
                m = re.match(r"\{([0-9A-Fa-f]{2}(?: [0-9A-Fa-f]{2})*)\}", text[i:])
                if m:
                    b = bytes.fromhex(m.group(1))
                    if 0 in b:
                        raise EncodeError(f"{where}: {m.group(0)}: a 00 byte would end the entry")
                    out.append(("g", b, m.group(0)))
                    i += m.end()
                    continue
            if ch == "\n":
                out.append((("c", 1, ()), b"\x02\x01", "<1>"))
                i += 1
                continue
            b = self.enc.get(ch)
            if b is None and self.latin:
                b = self.enc.get(ALIASES.get(ch, ""))
            if b is None:
                name = unicodedata.name(ch, f"U+{ord(ch):04X}")
                near = text[max(0, i - 15):i + 5]
                raise EncodeError(f"{where}: the font has no {ch!r} ({name}) at position {i}: …{near}…")
            out.append(("g", b, ch))
            i += 1
        return out

    def encode(self, text, where=""):
        """The entry's bytes, with the final 0."""
        return b"".join(t[1] for t in self.tokens(text, where)) + b"\0"


def is_code(tok, *codes):
    return tok[0] != "g" and (not codes or tok[0][1] in codes)


def join(tokens):
    return "".join(t[2] for t in tokens)


class Metrics:
    """Glyph widths of one font, as font_get_text_width adds them (advance + x offset)."""

    def __init__(self, height=14, insert_width=INSERT_WIDTH):
        self.height = height
        glyphs, ext = dt.font_glyphs(height)
        self.glyphs, self.ext = glyphs, ext
        self.insert_width = insert_width

    def has(self, b):
        """Does this font draw the glyph (a missing one draws as the font's first glyph, '0')?"""
        if b[0] == 1:
            return 0 < b[1] <= len(self.ext) and self.ext[b[1] - 1][0] != 0xFF
        return 4 <= b[0] < 0xEA and self.glyphs[b[0] - 4][0] != 0xFF

    def width(self, tokens):
        w = 0
        for kind, b, _ in tokens:
            if kind == "g":
                if b[0] == 1:
                    w += dt.glyph_width(self.ext[b[1] - 1]) if 0 < b[1] <= len(self.ext) else 0
                elif 4 <= b[0] < 0xEA:
                    w += dt.glyph_width(self.glyphs[b[0] - 4])
            elif kind[1] == 5:
                w += self.insert_width
        return w


@lru_cache(None)
def metrics(height=14):
    return Metrics(height)


def split_head(tokens):
    """(head, body): head the leading speaker tag <7>Name<7> (as tokens), or [] if there is none."""
    if tokens and is_code(tokens[0], 7):
        for k in range(1, len(tokens)):
            if is_code(tokens[k], 7):
                return tokens[:k + 1], tokens[k + 1:]
    return [], tokens


def strip_trailing_spaces(line):
    while line and line[-1][0] == "g" and line[-1][1] == SPACE:
        line = line[:-1]
    return line


def split_pages(body):
    """(pages, separators, tail): a page ends at a run of layout codes holding a wait (<2,n>) or a <3>; the run is
    its separator, the run that ends the text is the tail. Each page is a list of lines (token lists, split at <1>)."""
    pages, seps, cur, k, n = [], [], [], 0, len(body)
    while k < n:
        if is_code(body[k], *LAYOUT_CODES):
            j = k
            while j < n and is_code(body[j], *LAYOUT_CODES):
                j += 1
            run = body[k:j]
            if any(is_code(t, 2, 3) for t in run):
                pages.append(cur)
                if j == n:
                    return [_lines(p) for p in pages], seps, run
                seps.append(run)
                cur = []
            else:
                cur.extend(run)
            k = j
        else:
            cur.append(body[k])
            k += 1
    pages.append(cur)
    return [_lines(p) for p in pages], seps, []


def _lines(page):
    lines, cur = [], []
    for t in page:
        if is_code(t, 1):
            lines.append(cur)
            cur = []
        else:
            cur.append(t)
    lines.append(cur)
    return lines


def line_width(line, m):
    return m.width(strip_trailing_spaces(line))


def roundtrip(verbose=False):
    """Decode and re-encode every entry of every text file (latin readings for the European languages)."""
    codecs = {True: Codec(True), False: Codec(False)}
    total = bad = 0
    per_lang = {}
    for e in df.files():
        p = e.path.split("/")
        if len(p) != 5 or p[2] != "COUNTRY":
            continue
        c = codecs[p[3] != "JPN"]
        d = (df.DISC / e.path).read_bytes()
        for sub, offs in dt.tables_of(d):
            for i, o in enumerate(offs):
                raw = d[o:d.index(0, o) + 1]
                where = f"{p[4][:-4]}{'' if sub is None else f':{sub}'}:{i}"
                try:
                    again = c.encode(c.decode(d, o), where)
                except EncodeError as err:
                    again = None
                    print(f"  {err}")
                total += 1
                per_lang[p[3]] = per_lang.get(p[3], 0) + 1
                if again != raw:
                    bad += 1
                    if again is not None and (verbose or bad <= 20):
                        print(f"  {where}: {raw.hex(' ')} -> {again.hex(' ')}")
    langs = ", ".join(f"{k} {v}" for k, v in per_lang.items())
    print(f"round trip: {total} entries ({langs}) decoded and re-encoded, {bad} differ")
    return bad == 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--roundtrip", action="store_true", help="decode and re-encode every entry of every text file")
    ap.add_argument("--encode", metavar="TEXT", help="print a text's bytes")
    ap.add_argument("--width", metavar="TEXT", help="print each line's width in pixels")
    ap.add_argument("--font", type=int, default=14, choices=sorted(dt.FONTS))
    ap.add_argument("--jpn", action="store_true", help="Japanese readings (no European letters)")
    ap.add_argument("--chars", action="store_true", help="list the characters the encoder takes")
    a = ap.parse_args()
    c = Codec(not a.jpn)
    try:
        run(a, c)
    except EncodeError as e:
        sys.exit(str(e))


def run(a, c):
    if a.roundtrip:
        sys.exit(0 if roundtrip() else 1)
    if a.encode is not None:
        print(c.encode(a.encode, "text").hex(" "))
    if a.width is not None:
        m = metrics(a.font)
        pages, _, _ = split_pages(split_head(c.tokens(a.width, "text"))[1])
        for k, page in enumerate(pages):
            for line in page:
                missing = "".join(t[2] for t in line if t[0] == "g" and not m.has(t[1]))
                print(f"page {k} {line_width(line, m):4} px  {join(line)}" + (f"   (no glyph in this font: {missing})"
                                                                            if missing else ""))
    if a.chars:
        m = metrics(14)
        for ch, b in sorted(c.enc.items(), key=lambda x: (len(x[1]), x[1])):
            print(f"{b.hex(' '):6} {ch}  w{m.width([('g', b, ch)])}")
        print("aliases: " + " ".join(f"{k!r}->{v}" for k, v in ALIASES.items()))


if __name__ == "__main__":
    main()
