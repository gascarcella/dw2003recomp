#!/usr/bin/env python3
"""Write and read the PC port's text packs: translated entries laid over the English text files (docs/PORT.md
"Text packs").

  tools/venv/bin/python tools/text_pack.py --dump PACK      # the pack's files (ID, path, sizes) and entries, decoded
  tools/venv/bin/python tools/text_pack.py --selftest       # a test pack built, read back and rebuilt against the disc

A pack holds only translated entries, addressed (file_id, table, index): file_id the English file's ID (ES*.BIN,
JPN ID + 2), table 0xFFFF when the file is one counted string table, else the table's sub-file index in the ?SDMG###
container, index the i of font_get_entry(table, i), as tools/dump_text.py numbers them. When the game loads an English
file the pack has entries for, the port (port/game/text_pack.c) rebuilds it: every table keeps its entry count, each
entry is the pack's when it has one, else the disc's English bytes. The pack records each file's rebuilt size and the
CRC-32 of the English file it was built against (the user's disc, through tools/disc_files.py), which the port checks.

As a module (the translation tools call these):
  write_pack(path, entries)   entries {(file_id, table, index): bytes, each the game's encoded entry with its final 0}
  rebuild(file_id, entries)   the rebuilt file's bytes, by the algorithm port/game/text_pack.c runs
  read_pack(path)             -> (files, entries): the records, for checks
Reads iso/dw2003.bin and extracted/disc/SLES_039.36 (the user's disc); embeds no game data.
"""
import struct
import sys
import tempfile
import zlib
from collections import namedtuple
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import disc_files as df  # noqa: E402

MAGIC, VERSION, LANGUAGE = b"DW3T", 1, 2  # LANGUAGE: the records_language whose files the pack lays itself over
PLAIN = 0xFFFF                            # `table` of an entry in a file that is one table
HEADER = struct.Struct("<4sHHIII")        # magic, version, language, file count, entry count, blob size
FILE_REC = struct.Struct("<HHIIII")       # file ID, disc sectors, disc CRC-32, rebuilt size, first entry, entries
ENTRY_REC = struct.Struct("<HHHHII")      # file ID, table, index, 0, blob offset, length (with the final 0)
DMG_FIRST = 0x112                         # the ?SDMG### containers: JPN IDs 0x112 + 7k (docs/FORMATS.md "Text")
MAX_REBUILT = 0x80000                     # the port refuses a rebuilt file larger than this
MAX_COUNT = 5000                          # a table's entry count is 1..4999 (tools/dump_text.py table_offsets)

PackFile = namedtuple("PackFile", "file_id sectors crc size first count")
PackEntry = namedtuple("PackEntry", "file_id table index data")


class PackError(ValueError):
    pass


def is_english_text(fid):
    return fid in df.TEXT_IDS and (fid - 1) % 7 == LANGUAGE


def is_container(fid):
    return fid - LANGUAGE >= DMG_FIRST


def _s32(d, o):
    if o < 0 or o + 4 > len(d):
        raise PackError(f"offset {o:#x} outside the file")
    return struct.unpack_from("<i", d, o)[0]


def _table_count(d, base):
    """The entry count of the counted string table at `base` (shape 3), or None if it is not one."""
    if base < 0 or base + 8 > len(d):
        return None
    n = _s32(d, base)
    if not 0 < n < MAX_COUNT or base + 4 + 4 * n > len(d) or _s32(d, base + 4) != 4 + 4 * n:
        return None
    return n


def _entry(d, base, i):
    """Entry i of the table at `base`: its bytes up to and with its first 0 (the disc's entries hold no other 0)."""
    o = base + _s32(d, base + 4 + 4 * i)
    end = d.find(b"\0", o) if 0 <= o < len(d) else -1
    if end < 0:
        raise PackError(f"entry {i} of the table at {base:#x} has no end")
    return d[o:end + 1]


def _pad4(out):
    out += bytes(-len(out) % 4)


def _rebuild_table(d, base, repl):
    """The table at `base` rebuilt with repl {index: bytes}: the count, the offsets, then the entries in order, each
    starting 4-aligned (zero padding after the one before), as on the disc."""
    n = _table_count(d, base)
    if n is None:
        raise PackError(f"no string table at {base:#x}")
    bad = [i for i in repl if not 0 <= i < n]
    if bad:
        raise PackError(f"index {bad[0]} outside the table at {base:#x} ({n} entries)")
    out = bytearray(4 + 4 * n)
    struct.pack_into("<i", out, 0, n)
    for i in range(n):
        _pad4(out)
        struct.pack_into("<i", out, 4 + 4 * i, len(out))
        out += repl[i] if i in repl else _entry(d, base, i)
    return out


def _rebuild(fid, d, repl):
    """File `fid` (its loaded bytes `d`) rebuilt with repl {(table, index): bytes}."""
    if not is_container(fid):
        bad = [t for t, _ in repl if t != PLAIN]
        if bad:
            raise PackError(f"file {fid:#x} is one table: table {bad[0]:#x} (want 0xFFFF)")
        return bytes(_rebuild_table(d, 0, {i: v for (_, i), v in repl.items()}))
    # an offset table (shape 1) of sub-files: n = the header's size / 4; 0 an empty slot; the rest increasing
    n = _s32(d, 0) // 4
    if not 0 < n < MAX_COUNT or 4 * n > len(d):
        raise PackError(f"file {fid:#x}: no container header")
    subs = [_s32(d, 4 * k) for k in range(n)]
    used = [s for s in subs if s != 0]
    if any(s < 4 * n or s >= len(d) for s in used) or any(a >= b for a, b in zip(used, used[1:])):
        raise PackError(f"file {fid:#x}: the sub-file offsets are not increasing within the file")
    bad = [t for t, _ in repl if not 0 <= t < n or subs[t] == 0 or _table_count(d, subs[t]) is None]
    if bad:
        raise PackError(f"file {fid:#x}: sub-file {bad[0]} is no string table")
    out = bytearray(4 * n)
    for k, s in enumerate(subs):
        if s == 0:
            continue
        _pad4(out)
        struct.pack_into("<i", out, 4 * k, len(out))
        if _table_count(d, s) is not None:
            out += _rebuild_table(d, s, {i: v for (t, i), v in repl.items() if t == k})
        else:  # not a table: copied as it is, to the next sub-file or the buffer's end
            nxt = [u for u in used if u > s]
            out += d[s:nxt[0] if nxt else len(d)]
    _pad4(out)  # the container ends 4-aligned too
    return bytes(out)


def _norm(file_id, entries):
    """{(table, index): bytes} of `file_id` from either key form rebuild() takes."""
    out = {}
    for k, v in entries.items():
        if len(k) == 3:
            if k[0] != file_id:
                continue
            k = k[1:]
        out[(int(k[0]), int(k[1]))] = bytes(v)
    return out


def rebuild(file_id, entries):
    """The rebuilt file bytes for one English text file with these entries replaced (the same algorithm as the C).
    entries: {(file_id, table, index): bytes} as write_pack takes (other files' keys ignored), or {(table, index):
    bytes}. The result has no sector padding: its length is the size the pack records."""
    if not is_english_text(file_id):
        raise PackError(f"file {file_id:#x} is not an English text file (JPN ID + 2, IDs 3..0x15A)")
    return _rebuild(file_id, df.read(file_id), _norm(file_id, entries))


def build_pack(entries):
    """The pack's bytes for entries {(file_id, table, index): bytes}, every key checked against the disc."""
    by_file = {}
    for k, v in entries.items():
        if len(k) != 3:
            raise PackError(f"key {k!r}: want (file_id, table, index)")
        fid, table, index = (int(x) for x in k)
        v = bytes(v)
        if not is_english_text(fid):
            raise PackError(f"{k!r}: file {fid:#x} is not an English text file (JPN ID + 2, IDs 3..0x15A)")
        if not 0 <= table <= 0xFFFF or not 0 <= index <= 0xFFFF:
            raise PackError(f"{k!r}: table or index out of range")
        if not v or v[-1] != 0:
            raise PackError(f"{k!r}: an entry must end in 0")
        by_file.setdefault(fid, {})[(table, index)] = v
    files, recs, blob = [], [], bytearray()
    for fid in sorted(by_file):
        repl = by_file[fid]
        disc = df.read(fid)
        try:
            size = len(_rebuild(fid, disc, repl))
        except PackError as e:
            raise PackError(f"file {fid:#x} ({df.files()[fid].path}): {e}") from None
        if size > MAX_REBUILT:
            raise PackError(f"file {fid:#x}: rebuilt to {size} bytes, over the port's {MAX_REBUILT}")
        files.append(FILE_REC.pack(fid, df.files()[fid].sectors, zlib.crc32(disc), size, len(recs), len(repl)))
        for (table, index) in sorted(repl):
            v = repl[(table, index)]
            recs.append(ENTRY_REC.pack(fid, table, index, 0, len(blob), len(v)))
            blob += v
    head = HEADER.pack(MAGIC, VERSION, LANGUAGE, len(files), len(recs), len(blob))
    return head + b"".join(files) + b"".join(recs) + bytes(blob)


def write_pack(path, entries):
    """entries: {(file_id, table, index): bytes} — each value the game's encoded entry bytes INCLUDING the final 0.
    file_id is the English file's ID, table 0xFFFF for a plain table, index as font_get_entry's i.
    Rebuilds every touched file against the user's disc (disc_files.read) to compute and record its rebuilt size,
    validates every key exists on the disc, writes the pack."""
    data = build_pack(entries)
    Path(path).write_bytes(data)
    return len(data)


def parse_pack(data):
    """(files, entries) of a pack's bytes, checked as port/game/text_pack.c checks them (PackError otherwise)."""
    if len(data) < HEADER.size:
        raise PackError("shorter than the header")
    magic, version, lang, nf, ne, nb = HEADER.unpack_from(data, 0)
    if magic != MAGIC or version != VERSION or lang != LANGUAGE:
        raise PackError(f"magic {magic!r}, version {version}, language {lang} (want {MAGIC!r}, {VERSION}, {LANGUAGE})")
    eo = HEADER.size + FILE_REC.size * nf
    bo = eo + ENTRY_REC.size * ne
    if bo + nb != len(data):
        raise PackError(f"{len(data)} bytes, the header says {bo + nb}")
    files = [PackFile(*FILE_REC.unpack_from(data, HEADER.size + FILE_REC.size * k)) for k in range(nf)]
    entries, nxt = [], 0
    for k, f in enumerate(files):
        if not is_english_text(f.file_id) or (k and f.file_id <= files[k - 1].file_id):
            raise PackError(f"file record {k}: ID {f.file_id:#x} not an English text file or not increasing")
        if f.first != nxt or f.count == 0 or f.size == 0 or f.size > MAX_REBUILT:
            raise PackError(f"file {f.file_id:#x}: entries {f.first}+{f.count} or size {f.size} wrong")
        nxt += f.count
        last = None
        for j in range(f.first, f.first + f.count):
            fid, table, index, zero, off, n = ENTRY_REC.unpack_from(data, eo + ENTRY_REC.size * j)
            if fid != f.file_id or zero != 0 or (last is not None and (table, index) <= last):
                raise PackError(f"entry record {j}: not file {f.file_id:#x}'s or not sorted")
            if n == 0 or off + n > nb or data[bo + off + n - 1] != 0:
                raise PackError(f"entry record {j}: blob range {off}+{n} wrong or not ending in 0")
            last = (table, index)
            entries.append(PackEntry(fid, table, index, data[bo + off:bo + off + n]))
    if nxt != ne:
        raise PackError(f"{ne} entry records, the files own {nxt}")
    return files, entries


def read_pack(path):
    return parse_pack(Path(path).read_bytes())


def dump(path):
    import dump_text as dt
    tables = dt.char_tables()
    files, entries = read_pack(path)
    print(f"# {path}: {len(files)} file(s), {len(entries)} entr{'y' if len(entries) == 1 else 'ies'}")
    for f in files:
        e = df.files()[f.file_id]
        crc = zlib.crc32(df.read(f.file_id))
        print(f"0x{f.file_id:03X} {e.path}: {f.count} entries, disc {f.sectors} sectors (crc {f.crc:08X}"
              f"{'' if crc == f.crc else f', this disc {crc:08X}: MISMATCH'}), rebuilt {f.size} bytes "
              f"({(f.size + 2047) // 2048} sectors)")
        for x in entries[f.first:f.first + f.count]:
            where = "" if x.table == PLAIN else f"table {x.table} "
            print(f"    {where}{x.index:4}: {dt.decode(x.data, 0, tables)[0]}")


# ---- the self-test
def _encoder():
    """str -> the game's glyph bytes (dump_text's tables inverted; the European letters by dump_text.LATIN)."""
    import dump_text as dt
    chars, ext = dt.char_tables()
    to = {}
    for g, c in ext.items():
        to.setdefault(dt.sjis(c), b"\x01" + bytes([g]))
    for g, c in chars.items():
        to.setdefault(dt.sjis(c), bytes([g]))
    for g, ch in dt.LATIN.items():
        to[ch] = b"\x01" + bytes([g])
    return lambda s: b"".join(to[ch] for ch in s) + b"\0"


def selftest():
    import dump_text as dt
    enc, bad = _encoder(), []
    talk00, dmg200, system = 0xC5 + LANGUAGE, 0x112 + LANGUAGE, 0xBE + LANGUAGE

    def ok(cond, what):
        print(f"  {'ok  ' if cond else 'FAIL'} {what}")
        if not cond:
            bad.append(what)

    # 1. nothing replaced: every English file rebuilds to the disc's bytes (the 4-aligned entries, the sub-files)
    same = [f for f in range(LANGUAGE + 1, 0x15F, 7)
            if rebuild(f, {}) != (df.DISC / df.files()[f].path).read_bytes()]
    ok(not same, f"the 50 English files rebuilt with no entry equal the disc's files"
       f"{': not ' + ', '.join(hex(f) for f in same) if same else ''}")
    # 2. a pack: entry 1 of ESTALK00, one entry of a DMG table, one entry long enough to grow ESSYSTEM past a sector
    long = enc("Teste " * 400)
    entries = {(talk00, PLAIN, 1): enc("Olá, mundo! Pacote de teste."), (dmg200, 1, 2): enc("Ação no DMG200."),
               (system, PLAIN, 3): long}
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "test.pack"
        size = write_pack(path, entries)
        files, recs = read_pack(path)
    ok([f.file_id for f in files] == sorted({k[0] for k in entries}) and len(recs) == 3,
       f"the pack ({size} bytes) reads back: {len(files)} files, {len(recs)} entries, sorted")
    ok({(r.file_id, r.table, r.index): r.data for r in recs} == entries, "its entries are the ones written")
    for f in files:
        new, old = rebuild(f.file_id, entries), (df.DISC / df.files()[f.file_id].path).read_bytes()
        ok(len(new) == f.size and f.crc == zlib.crc32(df.read(f.file_id)),
           f"0x{f.file_id:03X}: rebuilt size {len(new)} and the disc's CRC recorded")
        tn, to = dt.tables_of(new), dt.tables_of(old)
        ok([(s, len(o)) for s, o in tn] == [(s, len(o)) for s, o in to],
           f"0x{f.file_id:03X}: dump_text reads {len(tn)} table(s) with the disc's entry counts")
        diff = []
        for (sub, on), (_, oo) in zip(tn, to):
            for i, (a, b) in enumerate(zip(on, oo)):
                got, was = new[a:new.index(0, a) + 1], old[b:old.index(0, b) + 1]
                key = (f.file_id, PLAIN if sub is None else sub, i)
                if got != entries.get(key, was):
                    diff.append(key)
                if a % 4:
                    diff.append(("unaligned", key))
        ok(not diff, f"0x{f.file_id:03X}: every entry the pack's or the disc's, 4-aligned{': ' + str(diff[:3]) if diff else ''}")
    ok((len(rebuild(system, entries)) + 2047) // 2048 > df.files()[system].sectors,
       f"0x{system:03X}: the long entry grows the file past its {df.files()[system].sectors} sector(s)")
    # 3. refusals
    for what, ents in (("a key not on the disc", {(talk00, PLAIN, 9999): b"\0"}),
                       ("a DMG entry with table 0xFFFF", {(dmg200, PLAIN, 1): b"\0"}),
                       ("a plain file with a table", {(talk00, 0, 1): b"\0"}),
                       ("a non-English file", {(talk00 + 1, PLAIN, 1): b"\0"}),
                       ("an entry without its final 0", {(talk00, PLAIN, 1): b"x"})):
        try:
            build_pack(ents)
            ok(False, f"refused: {what}")
        except PackError as e:
            ok(True, f"refused: {what} ({e})")
    print(f"text_pack self-test: {'FAIL (' + str(len(bad)) + ')' if bad else 'pass'}")
    return not bad


def main(argv):
    if argv[:1] == ["--selftest"]:
        sys.exit(0 if selftest() else 1)
    if argv[:1] == ["--dump"] and len(argv) == 2:
        try:
            dump(argv[1])
        except PackError as e:
            sys.exit(f"{argv[1]}: not a valid pack: {e}")
        return
    sys.exit(__doc__.split("\n\n")[1])


if __name__ == "__main__":
    main(sys.argv[1:])
