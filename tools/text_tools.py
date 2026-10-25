#!/usr/bin/env python3
"""Translation work files for the English text (docs/FORMATS.md "Text", "Font"): export, fit, check, build.

  tools/venv/bin/python tools/text_tools.py export DIR                # write/merge one JSON work file per English text file
  tools/venv/bin/python tools/text_tools.py export DIR --file TALK00  # only these files (repeatable)
  tools/venv/bin/python tools/text_tools.py check DIR                 # every translated entry: codes, speaker, width, lines, characters
  tools/venv/bin/python tools/text_tools.py check DIR -w              # also print the warnings
  tools/venv/bin/python tools/text_tools.py build DIR OUT.pack        # fit + encode every non-empty pt, write the text pack
  tools/venv/bin/python tools/text_tools.py fit TALK00:14 'Texto...'  # fit one translation of one entry, print the result
  tools/venv/bin/python tools/text_tools.py limits                    # each file's kind, layout and limits
  tools/venv/bin/python tools/text_tools.py --selftest                # round trip, fitting, checker, merge, build

Work file (DIR/<FILE>.json, UTF-8): {file, file_id, path, kind, layout, font, fit, notes, entries: [{key, speaker?, en,
pt, fit?, note?}]}. key is FILE:index or FILE:table:index (the ?SDMG### event texts), index font_get_entry's i (entry 0
is the empty string), so the pack address is (file_id, 0xFFFF or table, index). en and pt use tools/text_codec.py's
notation without the speaker tag: the build puts the English <7>Name<7> back in front (speaker "<9>" is the player).
An empty pt keeps the English. Entries with no Latin letter, the empty/"a" placeholders and leftover Japanese are not
exported. kind "names" marks the files of proper names (they stay English; a pt there is still built).

Re-export merges: pt and any field the translator added are kept; if an entry's English changed, it gets
"stale": true and "en_was" (the English its pt was written against); entries that no longer exist stay, "orphan": true.

Fitting (what check and build run on each pt): limits come from the English (max_width and max_lines per page) with
the 14 px font's widths (font_get_text_width: advance + x offset; every text window of these files uses font 1, the
14 px one; a <5,n> insert counts INSERT_WIDTH). Dialogue (TALK, DMG) with a speaker is the speech bubble, whose fill is
at most 139 px wide (message_create_dialog) with the text starting 10 px left of it, so lines of up to 149 px; other
dialogue entries whose English is wider than that are the box (message_box_create; limit: its widest English line),
the rest is held to the bubble's 149 px, 3 lines a page. In the other files a page holds as many lines as the file's
(uniform) or the entry's (mixed: menus and messages) English, a line as wide as the file's widest English line; in a
mixed file a line wider than its entry's English is a warning (its window may be narrower). MAX_LINES raises a file's
lines per page. Lines end at <1>; a page ends at a run of <1>/<2,n>/<3> that holds a
wait or a <3>. The fitter keeps the translator's breaks that fit, re-flows a line that does not (its remainder joins the
next line), and when a page has too many lines splits it into pages joined by the English's own separator (dialogue:
<2,2><3>), preferring page ends at a sentence end. A missing final wait is copied from the English. What still does
not fit is an error, never truncated.
"""
import argparse
import json
import math
import re
import sys
import tempfile
from collections import Counter, namedtuple
from functools import lru_cache
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import disc_files as df  # noqa: E402
import dump_text as dt  # noqa: E402
import text_codec as tc  # noqa: E402

# message_create_dialog: the bubble's fill is the text's width clamped to 0x5F..0x8B, and the text starts 10 px left of
# the fill (message_dialog_layouts: text_pos.x - fill_pos.x), so a line up to 0x8B + 10 px stays inside the fill.
BUBBLE_WIDTH = 0x8B + 10
DIALOGUE_LINES = 3   # message_create_dialog, message_box_create: set_page_lines(3)
SPEECH = 6           # px per character, for the translator's rough "~N characters" hint (lowercase advance)

# File (name without the E S prefix) -> (kind, layout). layout: "dialogue" (TALK/DMG: bubble or box, pages may be
# added), "uniform" (one window for every entry: the file's widest English line and most lines are the limits),
# "mixed" (menus and messages of several windows: the file's widest line is the hard limit, a line wider than the
# entry's own English a warning; no more lines than the entry's English has, no new pages unless the entry has one).
FILES = {
    "AMTMAP": ("names", "uniform"), "ASKMAP": ("names", "uniform"), "CARDNM": ("names", "uniform"),
    "DIGNAM": ("names", "uniform"), "ITMNAM": ("names", "uniform"), "SKLNAM": ("names", "uniform"),
    "SHPNAM": ("names", "uniform"), "STNAME": ("names", "uniform"), "STAREA": ("names", "uniform"),
    "CRDDEK": ("names", "uniform"), "HTLNAM": ("names", "mixed"),
    "ITMINF": ("translate", "uniform"), "SKLINF": ("translate", "uniform"), "DIGINF": ("translate", "uniform"),
    "CARDST": ("translate", "uniform"),
    "CARDGM": ("translate", "mixed"), "CRDABM": ("translate", "mixed"), "CRDSHP": ("translate", "mixed"),
    "DGLABO": ("translate", "mixed"), "DGNMET": ("translate", "mixed"), "FGTRPT": ("translate", "mixed"),
    "ITSHOP": ("translate", "mixed"), "MEMCRD": ("translate", "mixed"), "MFIGHT": ("translate", "mixed"),
    "NAMEDT": ("translate", "mixed"), "NAMEET": ("translate", "mixed"), "STATUS": ("translate", "mixed"),
    "TRAINI": ("translate", "mixed"), "SYSTEM": ("unused", "mixed"),
}
for _k in range(10):
    FILES[f"TALK0{_k}"] = ("translate", "dialogue")
for _n in (200, 260, 270, 300, 305, 400, 500, 600, 700, 800, 900):
    FILES[f"DMG{_n}"] = ("translate", "dialogue")

# Entries of "names" files that are messages, not names (translated like a mixed file's entries).
NAME_FILE_TEXTS = {"HTLNAM": {17, 18, 19, 20}}
# Files with labels drawn in the 9 px font (font 3: digits and A-Z a-z only, no space, no accents).
SMALL_LABEL_FILES = {"STATUS", "TRAINI", "DGLABO", "MEMCRD", "ITSHOP", "CRDABM", "CRDSHP", "NAMEET", "FGTRPT",
                     "HTLNAM", "MFIGHT"}
NOTES = {
    "MEMCRD": "Entry 39 is the save's title on the memory card: leave it empty.",
    "HTLNAM": "Inn names stay English; only entries 17-20 are messages.",
    "SYSTEM": "Only a debug overlay reads this file.",
}
DIALOGUE_NOTE = ("Write each page's text as running text or with your own <1> line breaks; the tool re-flows lines and "
                 "adds pages (<2,2><3>) when a page is too long. Keep <8,n><5,n> (the player's name) and other <5,n> "
                 "inserts. speaker \"<9>\" is the player.")

# More lines per page than the English uses, per file: for a window that draws them (checked on screen first). The
# one-line description files are the tight ones: ITMINF, SKLINF, DIGINF (an instant window with page_lines 1 draws
# 2 lines, but whether a 2nd line overlaps something else is not checked).
MAX_LINES = {"ITMINF": 2, "SKLINF": 2, "DIGINF": 2}  # a second line allowed (2026-10-24); to be checked in game

Limits = namedtuple("Limits", "width lines sep soft_width en_lines")  # sep: tokens joining added pages, or None


class FitError(ValueError):
    pass


def stem(path):
    return path.split("/")[-1][2:-4]


@lru_cache(None)
def codec():
    return tc.Codec(True)


def english_files():
    """[(stem, FileEntry)] of the 50 English text files, in ID order."""
    return [(stem(e.path), e) for e in df.files() if e.id in df.TEXT_IDS and (e.id - 1) % 7 == 2]


def entry_key(name, sub, i):
    return f"{name}:{i}" if sub is None else f"{name}:{sub}:{i}"


def parse_key(key):
    p = key.split(":")
    return (p[0], None, int(p[1])) if len(p) == 2 else (p[0], int(p[1]), int(p[2]))


def is_placeholder(text):
    return text in ("", "a")


def has_japanese(text):
    return any("぀" <= ch <= "ヿ" or "一" <= ch <= "鿿" for ch in text)


@lru_cache(None)
def english():
    """{key: (file stem, file ID, table or None, index, decoded text)} of every English entry."""
    out = {}
    for name, e in english_files():
        d = (df.DISC / e.path).read_bytes()
        for sub, offs in dt.tables_of(d):
            for i, o in enumerate(offs):
                out[entry_key(name, sub, i)] = (name, e.id, sub, i, codec().decode(d, o))
    return out


def structure(text):
    """(head, pages, seps, tail) of a text in the notation."""
    head, body = tc.split_head(codec().tokens(text))
    pages, seps, tail = tc.split_pages(body)
    return head, pages, seps, tail


@lru_cache(None)
def file_stats():
    """{stem: (widest line, most lines in a page, most common page separator or None)} over the English; plus
    "BOX": the widest dialogue line of an entry with no speaker (the box)."""
    m = tc.metrics(14)
    out, box = {}, 0
    for key, (name, _, _, _, text) in english().items():
        if is_placeholder(text) or has_japanese(text):
            continue
        head, pages, seps, _ = structure(text)
        w = max(tc.line_width(ln, m) for pg in pages for ln in pg)
        lines = max(len(pg) for pg in pages)
        mw, ml, sc = out.get(name, (0, 0, Counter()))
        sc.update(tc.join(s) for s in seps)
        out[name] = (max(mw, w), max(ml, lines), sc)
        if FILES[name][1] == "dialogue" and not head:
            box = max(box, w)
    res = {k: (w, n, sc.most_common(1)[0][0] if sc else None) for k, (w, n, sc) in out.items()}
    res["BOX"] = box
    return res


def limits_for(key):
    """Limits of one entry, from its file's layout and the English."""
    name, _, _, _, text = english()[key]
    layout = FILES[name][1]
    m = tc.metrics(14)
    head, pages, seps, _ = structure(text)
    en_w = max(tc.line_width(ln, m) for pg in pages for ln in pg)
    en_lines = max(len(pg) for pg in pages)
    fw, fl, fsep = file_stats()[name]
    if layout == "dialogue":
        sep = seps[0] if seps else codec().tokens(fsep or "<2,2><3>")
        width = BUBBLE_WIDTH if head or en_w <= BUBBLE_WIDTH else file_stats()["BOX"]
        return Limits(width, DIALOGUE_LINES, sep, None, en_lines)
    if layout == "uniform":
        return Limits(fw, MAX_LINES.get(name, fl), seps[0] if seps else None, None, en_lines)
    return Limits(fw, max(en_lines, MAX_LINES.get(name, 0)), seps[0] if seps else None, en_w, en_lines)


def fit_hint(lim, layout):
    """The limits in words, for the translator (px and a rough character count)."""
    lines = f"{lim.lines} line{'s' if lim.lines > 1 else ''}"
    if lim.soft_width is not None:  # mixed
        s = (f"{lines}, best no longer than the English ({lim.soft_width} px, ~{lim.soft_width // SPEECH} characters"
             f"{' per line' if lim.lines > 1 else ''}); at most {lim.width} px")
    else:
        s = f"{lines} of {lim.width} px (~{lim.width // SPEECH} characters)"
    if lim.sep is not None:
        s += " per page" + ("; pages are added as needed" if layout == "dialogue" else "")
    return s


# --- fitting --------------------------------------------------------------------------------------------------------

SPACE_TOK = ("g", tc.SPACE, " ")
NL_TOK = (("c", 1, ()), b"\x02\x01", "<1>")


def _words(line):
    """Split at spaces: [word tokens]; empty words dropped."""
    words, cur = [], []
    for t in line:
        if t[0] == "g" and t[1] == tc.SPACE:
            if cur:
                words.append(cur)
            cur = []
        else:
            cur.append(t)
    if cur:
        words.append(cur)
    return words


def _wrap(tokens, width, m):
    """Greedy word wrap of one run of text into lines of at most `width` px."""
    lines, cur = [], []
    for w in _words(tokens):
        cand = cur + [SPACE_TOK] + w if cur else w
        if m.width(cand) <= width:
            cur = cand
            continue
        if not cur or m.width(w) > width:
            raise FitError(f"“{tc.join(w)}” is {m.width(w)} px wide, more than a line ({width} px)")
        lines.append(cur)
        cur = w
    lines.append(cur)
    return lines


def reflow(lines, width, m):
    """Keep each line that fits; a line that does not is wrapped and its remainder joins the next line."""
    out, carry = [], []
    for line in lines:
        line = tc.strip_trailing_spaces(line)
        if carry and not _words(line):  # an empty line (layout) after a carried remainder: the remainder first
            out.extend(_wrap(carry, width, m))
            carry = []
        cur = carry + [SPACE_TOK] + line if carry else line
        carry = []
        if tc.line_width(cur, m) <= width:
            out.append(cur)
            continue
        wrapped = _wrap(cur, width, m)
        out.extend(wrapped[:-1])
        carry = wrapped[-1]
    if carry:
        out.extend(_wrap(carry, width, m))
    return out


def split_page(lines, max_lines):
    """Split lines into the fewest pages of at most max_lines, ending pages at sentence ends where possible, then as
    evenly as possible. Returns [lines]."""
    n = len(lines)
    k = math.ceil(n / max_lines)
    ends = [bool(re.search(r"[.!?…)”]$", tc.join(tc.strip_trailing_spaces(ln)))) for ln in lines]
    best = None

    def rec(start, left, cuts):
        nonlocal best
        if left == 1:
            if n - start <= max_lines:
                parts = cuts + [n]
                sizes = [b - a for a, b in zip([0] + parts[:-1], parts)]
                score = (sum(ends[c - 1] for c in parts[:-1]), -(max(sizes) - min(sizes)))
                if best is None or score > best[0]:
                    best = (score, parts)
            return
        for c in range(start + 1, min(start + max_lines, n - (left - 1)) + 1):
            rec(c, left - 1, cuts + [c])
    rec(0, k, [])
    parts = best[1]
    return [lines[a:b] for a, b in zip([0] + parts[:-1], parts)]


def fit(key, pt):
    """(fitted text in the notation, the encoded bytes, [(severity, message)]). Raises nothing: what cannot be fitted
    is an "error" problem (and the text is None)."""
    name, _, _, _, en = english()[key]
    layout = FILES[name][1]
    lim = limits_for(key)
    m = tc.metrics(14)
    probs = []
    try:
        toks = codec().tokens(pt, key)
    except tc.EncodeError as e:
        return None, None, [("error", str(e).split(": ", 1)[1])]
    en_head, en_pages, en_seps, en_tail = structure(en)
    head, body = tc.split_head(toks)
    if head and tc.join(head) != tc.join(en_head):
        probs.append(("error", f"speaker tag {tc.join(head)} is not the English {tc.join(en_head) or '(none)'}"))
    pages, seps, tail = tc.split_pages(body)
    if not tail and en_tail:
        tail = en_tail
    if tc.join(tail) != tc.join(en_tail):
        probs.append(("error", f"ends with {tc.join(tail) or '(no code)'}, "
                               f"the English with {tc.join(en_tail) or '(no code)'}"))
    # non-layout control codes: same multiset (error), same order (warning)
    def codes(ts):
        return [t[2] for t in ts if t[0] != "g" and t[0][1] not in tc.LAYOUT_CODES]
    ec, pc = codes(tc.split_head(codec().tokens(en))[1]), codes(body)
    if Counter(ec) != Counter(pc):
        probs.append(("error", f"control codes {' '.join(pc) or '(none)'}, the English has {' '.join(ec) or '(none)'}"))
    elif ec != pc:
        probs.append(("warning", f"control codes in another order than the English ({' '.join(ec)})"))
    out_pages, out_seps = [], []
    for k, page in enumerate(pages):
        try:
            lines = reflow(page, lim.width, m)
        except FitError as e:
            probs.append(("error", f"page {k + 1}: {e}"))
            lines = page
        if len(lines) > lim.lines:
            if lim.sep is None and lim.lines == 1:
                w = m.width([t for ln in page for t in ln])
                probs.append(("error", f"{w} px, more than its one line holds ({lim.width} px)"))
                chunks = [lines]
            elif lim.sep is None:
                probs.append(("error", f"page {k + 1}: {len(lines)} lines, at most {lim.lines}"
                              + ("" if layout != "mixed" else " (as the English)")))
                chunks = [lines]
            else:
                chunks = split_page(lines, lim.lines)
        else:
            chunks = [lines]
        for j, ch in enumerate(chunks):
            out_pages.append(ch)
            if j < len(chunks) - 1:
                out_seps.append(lim.sep)
        if k < len(seps):
            out_seps.append(seps[k])
    toks_out = list(en_head)
    for k, page in enumerate(out_pages):
        for j, ln in enumerate(page):
            if j:
                toks_out.append(NL_TOK)
            toks_out.extend(ln)
        if k < len(out_seps):
            toks_out.extend(out_seps[k])
    toks_out.extend(tail)
    # the result, measured again (a page or line over its limit here was already reported above)
    for ln in (ln for page in tc.split_pages(toks_out[len(en_head):])[0] for ln in page):
        w = tc.line_width(ln, m)
        if lim.soft_width is not None and lim.soft_width < w <= lim.width:
            probs.append(("warning", f"line “{tc.join(ln)}” is {w} px, the English {lim.soft_width} px: "
                                     "check that it fits its window"))
    if name in SMALL_LABEL_FILES:
        small = tc.metrics(9)
        en_ok = all(small.has(t[1]) for t in codec().tokens(en) if t[0] == "g")
        lack = sorted({t[2] for t in toks if t[0] == "g" and not small.has(t[1])})
        if en_ok and lack:
            probs.append(("warning", "if this label is drawn in the 9 px font (LV, HP labels), it has no "
                                     + " ".join(repr(ch) for ch in lack)))
    text = tc.join(toks_out)
    if any(p[0] == "error" for p in probs):
        return text, None, probs
    return text, b"".join(t[1] for t in toks_out) + b"\0", probs


# --- work files -----------------------------------------------------------------------------------------------------

# Entries not exported: the name-entry keyboard (its layout and letters)
NOT_EXPORTED = {"NAMEDT": set(range(2, 11))}


def exported(key, text):
    name, _, _, i, _ = english()[key]
    if is_placeholder(text) or has_japanese(text) or not re.search(r"[A-Za-z]", text):
        return False
    return FILES[name][0] != "unused" and i not in NOT_EXPORTED.get(name, ())


def entry_doc(key, file_fit):
    name, _, _, i, text = english()[key]
    head, body = tc.split_head(codec().tokens(text))
    e = {"key": key}
    if head:
        e["speaker"] = tc.join(head[1:-1])
    e["en"] = tc.join(body)
    e["pt"] = ""
    hint = fit_hint(limits_for(key), FILES[name][1])
    if hint != file_fit:
        e["fit"] = hint
    if name in NAME_FILE_TEXTS and i in NAME_FILE_TEXTS[name]:
        e["note"] = "a message, not a name: translate"
    return e


def file_doc(name, e):
    kind, layout = FILES[name]
    keys = [k for k, v in english().items() if v[0] == name and exported(k, v[4])]
    notes = [n for n in (NOTES.get(name), DIALOGUE_NOTE if layout == "dialogue" else None) if n]
    hints = Counter(fit_hint(limits_for(k), layout) for k in keys)
    file_fit = hints.most_common(1)[0][0] if hints and layout != "mixed" else None
    doc = {"file": name, "file_id": e.id, "path": e.path, "kind": kind, "layout": layout, "font": "14 px"}
    if file_fit:
        doc["fit"] = file_fit + " (unless an entry's own fit says otherwise)"
    doc["notes"] = notes
    doc["entries"] = [entry_doc(k, file_fit) for k in keys]
    return doc


def merge(new, old):
    """new file doc with the old doc's pt and extra fields; returns (doc, counts)."""
    olds = {e["key"]: e for e in old.get("entries", [])}
    kept = stale = 0
    for e in new["entries"]:
        o = olds.pop(e["key"], None)
        if o is None:
            continue
        for k, v in o.items():
            if k not in e or k == "pt":
                e[k] = v
        if o.get("pt"):
            kept += 1
        was = o.get("en_was", o.get("en"))
        if o.get("en") != e["en"] or o.get("stale"):
            e["stale"] = True
            e["en_was"] = was
        else:
            e.pop("stale", None)
            e.pop("en_was", None)
        stale += bool(e.get("stale"))
        e.pop("orphan", None)
    orphans = [dict(o, orphan=True) for o in olds.values()]
    new["entries"].extend(orphans)
    return new, {"kept": kept, "stale": stale, "orphans": len(orphans)}


def dump_json(doc):
    return json.dumps(doc, ensure_ascii=False, indent=1) + "\n"


def export(outdir, only=None):
    outdir = Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    total = Counter()
    for name, e in english_files():
        if only and name not in only:
            continue
        if FILES[name][0] == "unused":
            continue
        doc = file_doc(name, e)
        path = outdir / f"{name}.json"
        counts = {"kept": 0, "stale": 0, "orphans": 0}
        if path.exists():
            doc, counts = merge(doc, json.loads(path.read_text("utf-8")))
        path.write_text(dump_json(doc), "utf-8")
        total.update(counts)
        total["entries"] += len(doc["entries"])
        total["files"] += 1
        flag = "".join(f", {v} {k}" for k, v in counts.items() if v and k != "kept")
        print(f"{path.name}: {len(doc['entries'])} entries ({doc['kind']}), {counts['kept']} translations kept{flag}")
    print(f"{outdir}: {total['files']} files, {total['entries']} entries, {total['kept']} translations kept, "
          f"{total['stale']} stale, {total['orphans']} orphans")


def load_dir(d):
    docs = []
    for p in sorted(Path(d).glob("*.json")):
        doc = json.loads(p.read_text("utf-8"))
        if "entries" in doc and "file" in doc:
            docs.append((p, doc))
    return docs


def check(d, warnings=False, out=print):
    """Fit every translated entry; print a summary per file and the problems. Returns {key: bytes} of the entries
    that fit (no error) and the error count."""
    good, nerr, nwarn = {}, 0, 0
    problems = []
    for p, doc in load_dir(d):
        n = done = err = warn = 0
        for e in doc["entries"]:
            n += 1
            key, pt = e["key"], e.get("pt", "")
            if not pt:
                continue
            done += 1
            if key not in english():
                problems.append((key, "error", "no such entry in the English files"))
                err += 1
                continue
            if e.get("stale"):
                problems.append((key, "warning", "the English changed since this was translated (en_was)"))
                warn += 1
            text, data, probs = fit(key, pt)
            for sev, msg in probs:
                problems.append((key, sev, msg))
                err += sev == "error"
                warn += sev == "warning"
            if data is not None:
                good[key] = data
        nerr += err
        nwarn += warn
        out(f"{doc['file']:7} {doc['kind']:9} {done:5}/{n:<5} translated  {err:4} errors  {warn:4} warnings")
    for key, sev, msg in problems:
        if sev == "error" or warnings:
            out(f"  {key}: {sev}: {msg}")
    more = "" if warnings or not nwarn else " (-w lists them)"
    out(f"{len(good)} entries fit, {nerr} errors, {nwarn} warnings{more}")
    return good, nerr


def pack_entries(good):
    out = {}
    for key, data in good.items():
        _, fid, sub, i, _ = english()[key]
        out[(fid, 0xFFFF if sub is None else sub, i)] = data
    return out


def build(d, pack, allow_errors=False):
    good, nerr = check(d)
    if nerr and not allow_errors:
        print(f"not built: {nerr} errors (--allow-errors builds the entries that fit)")
        return 1
    import text_pack
    size = text_pack.write_pack(pack, pack_entries(good))
    print(f"{pack}: {len(good)} entries, {size} bytes")
    return 0


def show_limits():
    for name, e in english_files():
        kind, layout = FILES[name]
        w, n, sep = file_stats()[name]
        print(f"{name:7} 0x{e.id:03X} {kind:9} {layout:8} widest {w:3} px, {n} lines/page, separator {sep or '-'}")
    print(f"dialogue: speaker bubble {BUBBLE_WIDTH} px, box {file_stats()['BOX']} px (widest English line of an entry "
          f"with no speaker that is wider than the bubble), {DIALOGUE_LINES} lines per page")


# --- self test ------------------------------------------------------------------------------------------------------

def selftest():
    fails = []

    def expect(cond, what):
        print(("ok   " if cond else "FAIL ") + what)
        if not cond:
            fails.append(what)

    expect(tc.roundtrip(), "round trip of every entry of every text file")
    c = codec()
    # encoder
    expect(c.encode("Olá") == bytes([0x1C, 0x33, 1, 0x5F, 0]), "encode Olá")
    expect(c.encode("'-~") == c.encode("’−〜"), "ASCII stand-ins")
    expect(c.encode("a<1>\nb") == c.encode("a\nb") == c.encode("a<1>b"),
           "newline after <1> ignored, bare newline a <1>")
    for bad in ("ª", "<2>", "<12>", "{00}", "€"):
        try:
            c.encode(bad, "TEST:1")
            expect(False, f"rejects {bad!r}")
        except tc.EncodeError as e:
            expect("TEST:1" in str(e), f"rejects {bad!r}: {e}")
    m = tc.metrics(14)
    expect(m.width(c.tokens("Ação")) == sum(m.width(c.tokens(ch)) for ch in "Ação"), "width is the sum of glyphs")
    # fitting: a dialogue entry with a speaker (bubble 139 px, 3 lines)
    key = "TALK00:14"
    en = english()[key][4]
    expect(en.startswith("<7>Energetic Yuki<7>"), f"{key} has its speaker")
    long = ("O Digimon Online é muito divertido! Eu estou mesmo me dedicando a ele todos os dias, "
            "de manhã até a noite, sem parar nem um minuto.")
    text, data, probs = fit(key, long)
    head, pages, seps, tail = structure(text)
    expect(data is not None and not probs, f"long running text fits: {probs}")
    expect(tc.join(head) == "<7>Energetic Yuki<7>" and tc.join(tail) == "<2,2>", "speaker and final wait copied")
    expect(len(pages) >= 2 and all(len(p) <= 3 for p in pages), f"split into {len(pages)} pages of <= 3 lines")
    expect(all(tc.join(s) == "<2,2><3>" for s in seps), "pages joined by the English separator <2,2><3>")
    expect(all(tc.line_width(ln, m) <= BUBBLE_WIDTH for p in pages for ln in p), "every line within 149 px")
    expect(c.decode(data) == text, "the encoded bytes decode to the fitted text")
    text, data, probs = fit(key, "Digimon Online é<1>divertido!<2,2>")
    expect(text == "<7>Energetic Yuki<7>Digimon Online é<1>divertido!<2,2>" and not probs, "author breaks kept")
    text, _, _ = fit(key, "Digimon Online é muito divertido demais para todos<1>nós!")
    expect("<1>" in text and tc.join(structure(text)[1][0][-1]).endswith("nós!"), f"overflowing line re-flowed: {text}")
    text, _, probs = fit(key, "Um.<1>Dois.<1>Três.<1>Quatro.")
    expect(len(structure(text)[1]) == 2 and not probs, f"4 lines become 2 pages: {text}")
    # checker: hand-made bad translations
    k2 = "DMG200:3:3"  # <7>Teddy<7>...oh!<1>Hey <8,1><5,1>!<2,2>
    expect("<8,1><5,1>" in english()[k2][4], f"{k2} names the player")
    cases = [
        (k2, "...oh!<1>Ei!", "control codes"),                                 # player name dropped
        (k2, "<7>Ted<7>...oh!<1>Ei <8,1><5,1>!", "speaker tag"),               # speaker renamed
        (k2, "...oh!<1>Ei <8,1><5,1>!<3>", "ends with"),                       # wrong tail
        (k2, "Ei <8,1><5,1> Supercalifragilisticexpialidocioso-extraordinário!", "more than a line"),
        ("MFIGHT:76", "Força, defesa e velocidade foram amaldiçoadas!", "one line holds"),  # one-line message too wide
        ("CARDGM:33", "Você não pode comprar 6 cartas, então você perde<1>este set.<1>Que pena.", "at most 2"),
        ("ITMINF:5", "Item para pescar à beira d’água, ª", "no 'ª'"),
        ("MFIGHT:35", "<5,2> X <5,1> = <5,3>!", "another order"),
    ]
    for k, pt, want in cases:
        _, _, probs = fit(k, pt)
        expect(any(want in msg for _, msg in probs), f"{k} {pt!r}: reports {want!r} ({probs})")
    _, data, probs = fit("MFIGHT:1", "Lutar")
    expect(data is not None and [s for s, _ in probs] == ["warning"],
           f"a menu label wider than the English: a warning ({probs})")
    _, data, probs = fit("MFIGHT:1", "Luta")
    expect(data is not None and not probs, "a menu label as wide as the English passes")
    # export, merge, check, build
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        import io
        import contextlib
        with contextlib.redirect_stdout(io.StringIO()):
            export(tmp, only={"TALK00", "MFIGHT"})
        doc = json.loads((tmp / "TALK00.json").read_text("utf-8"))
        ent = {e["key"]: e for e in doc["entries"]}
        expect(key in ent and ent[key]["speaker"] == "Energetic Yuki" and ent[key]["pt"] == "",
               "export: speaker, empty pt")
        expect("TALK00:1" not in ent, "export skips the 'a' placeholders")
        ent[key]["pt"] = long
        ent[key]["comment"] = "kept"
        ent["TALK00:15"]["en"] = "an older English"
        doc["entries"].append({"key": "TALK00:9999", "en": "gone", "pt": "sumiu"})
        (tmp / "TALK00.json").write_text(dump_json(doc), "utf-8")
        with contextlib.redirect_stdout(io.StringIO()):
            export(tmp, only={"TALK00"})
        doc2 = json.loads((tmp / "TALK00.json").read_text("utf-8"))
        ent2 = {e["key"]: e for e in doc2["entries"]}
        expect(ent2[key]["pt"] == long and ent2[key]["comment"] == "kept", "re-export keeps pt and added fields")
        expect(ent2["TALK00:15"].get("stale") and ent2["TALK00:15"]["en_was"] == "an older English"
               and ent2["TALK00:15"]["en"] != "an older English", "re-export flags a changed English")
        expect(ent2.get("TALK00:9999", {}).get("orphan") and ent2["TALK00:9999"]["pt"] == "sumiu", "orphans kept")
        ent2.pop("TALK00:9999")
        doc2["entries"] = [e for e in doc2["entries"] if e["key"] != "TALK00:9999"]
        mdoc = json.loads((tmp / "MFIGHT.json").read_text("utf-8"))
        for e in mdoc["entries"]:
            if e["key"] == "MFIGHT:1":
                e["pt"] = "Lutar"
        (tmp / "MFIGHT.json").write_text(dump_json(mdoc), "utf-8")
        (tmp / "TALK00.json").write_text(dump_json(doc2), "utf-8")
        lines = []
        good, nerr = check(tmp, out=lines.append)
        expect(nerr == 0 and set(good) == {key, "MFIGHT:1"}, f"check: 2 entries fit ({nerr} errors)")
        try:
            import text_pack
        except ImportError:
            text_pack = None
        if text_pack is None or not hasattr(text_pack, "write_pack"):
            print("skip build: tools/text_pack.py has no write_pack")
        else:
            pack = tmp / "pt.pack"
            with contextlib.redirect_stdout(io.StringIO()):
                rc = build(tmp, pack)
            expect(rc == 0 and pack.exists(), "build writes the pack")
            if hasattr(text_pack, "read_pack"):
                _, entries = text_pack.read_pack(pack)
                got = {(e.file_id, e.table, e.index): bytes(e.data) for e in entries}
                fid = english()[key][1]
                expect(got.get((fid, 0xFFFF, 14)) == good[key], "the pack holds TALK00:14's bytes")
                expect(c.decode(got[(fid, 0xFFFF, 14)]).startswith("<7>Energetic Yuki<7>O Digimon"), "and they decode")
    print(f"selftest: {len(fails)} failures")
    return not fails


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--selftest", action="store_true")
    sp = ap.add_subparsers(dest="cmd")
    p = sp.add_parser("export", help="write or merge the work files")
    p.add_argument("dir")
    p.add_argument("--file", action="append", help="only this file (TALK00, DMG200, ...)")
    p = sp.add_parser("check", help="check the translations")
    p.add_argument("dir")
    p.add_argument("-w", "--warnings", action="store_true", help="also list the warnings")
    p = sp.add_parser("build", help="fit, encode and pack the translations")
    p.add_argument("dir")
    p.add_argument("pack")
    p.add_argument("--allow-errors", action="store_true", help="build the entries that fit despite errors elsewhere")
    p = sp.add_parser("fit", help="fit one translation")
    p.add_argument("key")
    p.add_argument("pt")
    sp.add_parser("limits", help="each file's kind, layout and limits")
    a = ap.parse_args()
    if a.selftest:
        sys.exit(0 if selftest() else 1)
    if a.cmd == "export":
        export(a.dir, set(x.upper() for x in a.file) if a.file else None)
    elif a.cmd == "check":
        sys.exit(1 if check(a.dir, a.warnings)[1] else 0)
    elif a.cmd == "build":
        sys.exit(build(a.dir, a.pack, a.allow_errors))
    elif a.cmd == "fit":
        if a.key not in english():
            sys.exit(f"no entry {a.key}")
        text, data, probs = fit(a.key, a.pt)
        print(f"limits: {fit_hint(limits_for(a.key), FILES[english()[a.key][0]][1])}")
        m = tc.metrics(14)
        if text is not None:
            head, pages, seps, tail = structure(text)
            for k, page in enumerate(pages):
                for ln in page:
                    print(f"  page {k + 1} {tc.line_width(ln, m):4} px  {tc.join(ln)}")
            print(text)
        for sev, msg in probs:
            print(f"{sev}: {msg}")
        sys.exit(1 if data is None else 0)
    elif a.cmd == "limits":
        show_limits()
    else:
        ap.print_help()


if __name__ == "__main__":
    main()
