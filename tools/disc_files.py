#!/usr/bin/env python3
"""Map the game's file IDs to disc paths, and read files by ID (docs/FORMATS.md "Disc access: the file table and cdload").

  tools/venv/bin/python tools/disc_files.py                 # every file ID: id, LBA, sectors, path
  tools/venv/bin/python tools/disc_files.py 0x1CB 520       # the given IDs only
  tools/venv/bin/python tools/disc_files.py --path CARDDATA # IDs whose path contains the text
  tools/venv/bin/python tools/disc_files.py --check         # check the table against the disc (tests/formats/run.sh)

The game addresses files only by ID: an index into the EXE's filetable_lba / filetable_sectors
(src/main/filetable.c). The names come from the disc's ISO 9660 directories, reached through the path
table (the root directory record hides AAA/, docs/DISC_LAYOUT.md), and are matched by LBA.
Reads iso/dw2003.bin and extracted/disc/SLES_039.36 (the user's disc); embeds no game data.

As a module: `files()` -> list of FileEntry(id, lba, sectors, path), `read(id)` -> bytes (the file's
sectors as cdload_read loads them: whole 2048-byte sectors, so up to 2047 bytes past the file's end).
"""
import struct
import sys
from collections import namedtuple
from functools import lru_cache
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ISO = ROOT / "iso" / "dw2003.bin"
EXE = ROOT / "extracted" / "disc" / "SLES_039.36"
DISC = ROOT / "extracted" / "disc"
EXE_OFF = 0x80010000 - 0x800  # file offset of an EXE address
FILETABLE_LBA, FILETABLE_SECTORS, FILE_COUNT = 0x80044F6C, 0x800474A4, 2382
RAW, USER = 2352, 24  # MODE2/2352 sector; the 2048 user bytes start after 24 bytes (sync, header, subheader)

FileEntry = namedtuple("FileEntry", "id lba sectors path")


def _sector(iso, lba, n=1):
    return b"".join(iso[(lba + i) * RAW + USER:(lba + i) * RAW + USER + 2048] for i in range(n))


@lru_cache(None)
def _iso():
    import mmap
    f = open(ISO, "rb")
    return mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)


@lru_cache(None)
def iso_files():
    """{LBA: (path, size)} for every file on the disc, from the path table's directories."""
    iso = _iso()
    pvd = _sector(iso, 16)
    pt_size, pt_lba = struct.unpack_from("<II", pvd, 132)[0], struct.unpack_from("<I", pvd, 140)[0]
    pt = _sector(iso, pt_lba, (pt_size + 2047) // 2048)[:pt_size]
    dirs, o = [], 0
    while o < pt_size:  # path table: name length, ext attr, extent, parent (1-based), name
        n, lba, parent = pt[o], struct.unpack_from("<I", pt, o + 2)[0], struct.unpack_from("<H", pt, o + 6)[0]
        name = pt[o + 8:o + 8 + n].decode()
        path = "" if name == "\0" else (dirs[parent - 1][0] + "/" + name).lstrip("/")
        dirs.append((path, lba))
        o += 8 + n + (n & 1)
    out = {}
    for path, lba in dirs:
        first = _sector(iso, lba)
        size = struct.unpack_from("<I", first, 10)[0]  # the "." record holds the directory's size
        data = _sector(iso, lba, (size + 2047) // 2048)
        o = 0
        while o < size:
            n = data[o]
            if n == 0:  # records don't cross sectors
                o = (o // 2048 + 1) * 2048
                continue
            flags, nl = data[o + 25], data[o + 32]
            name = data[o + 33:o + 33 + nl]
            if not flags & 2 and name not in (b"\0", b"\1"):
                ext, fsize = struct.unpack_from("<I", data, o + 2)[0], struct.unpack_from("<I", data, o + 10)[0]
                out[ext] = ((path + "/" if path else "") + name.decode().split(";")[0], fsize)
            o += n
    return out


@lru_cache(None)
def files():
    exe = EXE.read_bytes()
    lba = struct.unpack_from(f"<{FILE_COUNT}I", exe, FILETABLE_LBA - EXE_OFF)
    secs = struct.unpack_from(f"<{FILE_COUNT}H", exe, FILETABLE_SECTORS - EXE_OFF)
    names = iso_files()
    return [FileEntry(i, lba[i], secs[i], names.get(lba[i], ("?", 0))[0]) for i in range(FILE_COUNT)]


def read(fid):
    """File `fid` as the game loads it: filetable_sectors[fid] whole sectors (zero-padded past the end)."""
    e = files()[fid]
    p = DISC / e.path
    d = p.read_bytes() if p.exists() else b""
    return d + bytes(e.sectors * 2048 - len(d)) if len(d) < e.sectors * 2048 else d


def by_path(text):
    return [e for e in files() if text.upper() in e.path.upper()]


def subfile(data, index):
    """cdload_get_subfile: word `index` of the file is the byte offset of a sub-file."""
    return struct.unpack_from("<i", data, index * 4)[0]


LANGS, LANG_PREFIX = ("JPN", "USA", "ENG", "FRA", "ITA", "GER", "SPN"), "MUEFIDS"  # records_language 0..6
TEXT_IDS = range(1, 0x15F)  # 50 text files x 7 languages, the language at consecutive IDs


def check():
    """docs/FORMATS.md "Disc access": the table covers every file under AAA/ exactly once, each size is its file's
    size rounded up to whole sectors, the bytes past a file's end in its last sector are zero (cdload loads them),
    and the text files sit at JPN ID + records_language. Returns the number of violations."""
    iso, names, fs, bad = _iso(), iso_files(), files(), []
    for e in fs:
        if e.lba not in names:
            bad.append(f"0x{e.id:03X}: LBA {e.lba} is no file")
            continue
        size = names[e.lba][1]
        if e.sectors != (size + 2047) // 2048:
            bad.append(f"0x{e.id:03X} {e.path}: {e.sectors} sectors for {size} bytes")
        elif any(_sector(iso, e.lba + e.sectors - 1)[size - (e.sectors - 1) * 2048:]):
            bad.append(f"0x{e.id:03X} {e.path}: non-zero bytes after the end in its last sector")
    aaa = {lba for lba, (p, _) in names.items() if p.startswith("AAA/")}
    if sorted(e.lba for e in fs) != sorted(aaa):
        bad.append(f"the table's {len({e.lba for e in fs})} distinct LBAs are not the {len(aaa)} files under AAA/")
    for e in fs:
        p = e.path.split("/")
        is_text = len(p) == 5 and p[2] == "COUNTRY"
        if is_text != (e.id in TEXT_IDS):
            bad.append(f"0x{e.id:03X} {e.path}: text file outside IDs 1-0x15E, or the reverse")
        elif is_text:
            k, jpn = (e.id - 1) % 7, fs[e.id - (e.id - 1) % 7].path.split("/")[4]
            if p[3] != LANGS[k] or p[4] != LANG_PREFIX[k] + jpn[1:]:
                bad.append(f"0x{e.id:03X} {e.path}: not language {k} of {jpn}")
    for b in bad:
        print("  " + b)
    print(f"{len(fs)} file IDs against the ISO directories (sectors, zero tail, every AAA/ "
          f"file once, {len(TEXT_IDS)} text files at JPN ID + language), {len(bad)} violations")
    return len(bad)


def main(argv):
    if argv[:1] == ["--check"]:
        sys.exit(1 if check() else 0)
    if argv[:1] == ["--path"]:
        sel = by_path(argv[1])
    elif argv:
        sel = [files()[int(a, 0)] for a in argv]
    else:
        sel = files()
    for e in sel:
        print(f"0x{e.id:03X} {e.id:4}  lba {e.lba:6}  {e.sectors:5} sec  {e.path}")


if __name__ == "__main__":
    main(sys.argv[1:])
