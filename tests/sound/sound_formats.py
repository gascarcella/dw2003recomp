#!/usr/bin/env python3
"""Parse every sound bank on the disc the way docs/FORMATS.md "Sound" describes it, and check the description.

  tools/venv/bin/python tests/sound/sound_formats.py           # a summary per bank
  tools/venv/bin/python tests/sound/sound_formats.py --check   # exit 1 on the first claim that does not hold
  tools/venv/bin/python tests/sound/sound_formats.py --events  # also a census of the SEPs' MIDI events

What it checks, per bank of sound_banks (the 71 SoundBank records in the EXE, read from SLES_039.36):
  VH (VAB header, "pBAV"): version 7, the total size = 0x20 + 0x800 + programs x 0x200 + 0x200 + sum(VAG sizes) x 8;
     the 128 program records (16 bytes) of which `programs` have tones; each tone record (32 bytes) names its program
     and a VAG in 1..vags; the VAG size table (256 u16, sizes / 8, entry 0 unused).
  VB (VAB body): exactly sum(VAG sizes) bytes of SPU ADPCM; every 16-byte block has shift <= 12 and filter <= 4;
     each VAG ends with a block whose flags have bit 0 (loop end) set.
  SEP ("pQES", version 0): 16 sequences numbered 0, 1, ... each with resolution, tempo, rhythm and a data size that
     ends exactly after its end-of-track meta event (FF 2F 00); the event stream is MIDI with running status, meta
     events without a length byte (FF 51 + 3 bytes, FF 2F 00) that leave the running status alone.
Reads the disc through tools/disc_files.py; embeds no game data.
"""
import argparse
import struct
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import disc_files  # noqa: E402

SYMBOLS = ROOT / "config/symbol_addrs.txt"


def symbol(name):
    for line in SYMBOLS.read_text().splitlines():
        if line.startswith(name + " ="):
            return int(line.split("=")[1].split(";")[0], 16)
    raise KeyError(name)


def exe_word(exe, addr):
    return struct.unpack_from("<i", exe, addr - disc_files.EXE_OFF)[0]


def banks(exe):
    """[(id, SoundBank fields)] from sound_banks[1..71] (a NULL-terminated-by-count table of 72 pointers)."""
    table = symbol("sound_banks")
    out = []
    for i in range(1, 72):
        p = exe_word(exe, table + 4 * i) & 0xFFFFFFFF
        body, header, vh, vb = (exe_word(exe, p + 4 * k) for k in range(4))
        seps, k = [], 4
        while exe_word(exe, p + 4 * k) != 0:
            seps.append(exe_word(exe, p + 4 * k))
            k += 1
        out.append((i, dict(body_file=body, header_file=header, vab_header=vh, vab_body=vb, seps=seps)))
    return out


def sub(data, sub_id):
    """cdload_get_subfile on a loaded file: the sub-file's bytes from its offset to the file's end."""
    return data[disc_files.subfile(data, sub_id & 0xFFFF):]


def parse_vh(vh):
    magic, version, vab_id, size = struct.unpack_from("<4sIII", vh, 0)
    _, programs, tones, vags, mvol, pan, attr1, attr2 = struct.unpack_from("<HHHHBBBB", vh, 0x10)
    progs = [struct.unpack_from("<BBBBBBHII", vh, 0x20 + 16 * i) for i in range(128)]
    tone_base = 0x20 + 0x800
    # One block of 16 tone records per *non-empty* program, in program order (an empty program has no block).
    used = [i for i, p in enumerate(progs) if p[0] != 0]
    tone_recs = []
    for k, p in enumerate(used[:programs]):
        for t in range(16):
            o = tone_base + 0x200 * k + 32 * t
            f = struct.unpack_from("<16BHHhh4h", vh, o)
            tone_recs.append((p, t, f))
    table = tone_base + 0x200 * programs
    vag_sizes = [s * 8 for s in struct.unpack_from("<256H", vh, table)]
    return dict(magic=magic, version=version, vab_id=vab_id, size=size, programs=programs, tones=tones, vags=vags,
                mvol=mvol, pan=pan, attr=(attr1, attr2), progs=progs, tone_recs=tone_recs, vag_sizes=vag_sizes,
                header_size=table + 0x200)


def read_varlen(d, o):
    v = 0
    while True:
        b = d[o]
        o += 1
        v = (v << 7) | (b & 0x7F)
        if not b & 0x80:
            return v, o


def parse_events(d, o, end, census):
    """Walks one sequence's MIDI stream from o; returns the offset after FF 2F 00 (or raises)."""
    status = None
    while o < end:
        _, o = read_varlen(d, o)
        b = d[o]
        if b == 0xFF:
            # A meta event; it leaves the running status of the channel events as it was (the SEPs rely on this: a
            # tempo change can be followed by data bytes of the previous status).
            # LIBSND's metas have no length byte: FF 51 t t t (tempo, us per quarter note, big-endian), FF 2F 00 (end).
            mtype = d[o + 1]
            census[f"meta {mtype:02X}"] += 1
            if mtype == 0x51:
                o += 5
                continue
            if mtype == 0x2F:
                if d[o + 2] != 0:
                    raise ValueError(f"FF 2F {d[o + 2]:02X} at {o:#x}")
                return o + 3
            raise ValueError(f"meta {mtype:#x} at {o:#x}")
        if b & 0x80:
            status = b
            o += 1
        elif status is None:
            raise ValueError(f"running status with no status at {o:#x}")
        kind = status & 0xF0
        if kind in (0x80, 0x90, 0xA0, 0xE0):
            if kind == 0x90:
                census["note on" if d[o + 1] else "note on vel 0 (off)"] += 1
            else:
                census[{0x80: "note off", 0xA0: "poly pressure", 0xE0: "pitch bend"}[kind]] += 1
            o += 2
        elif kind == 0xB0:
            census[f"cc {d[o]}"] += 1
            if d[o] in (98, 99, 100, 101, 6):
                census[f"cc {d[o]} = {d[o + 1]}"] += 1
            o += 2
        elif kind in (0xC0, 0xD0):
            census["program change" if kind == 0xC0 else "channel pressure"] += 1
            o += 1
        else:
            raise ValueError(f"status {status:#x} at {o:#x}")
    raise ValueError("no end-of-track")


def parse_sep(sep, census):
    magic, version = struct.unpack_from(">4sH", sep, 0)
    seqs, o, n = [], 6, 0
    while True:
        num, res = struct.unpack_from(">HH", sep, o)
        if num != n:
            break
        tempo = int.from_bytes(sep[o + 4:o + 7], "big")
        rhythm = (sep[o + 7], sep[o + 8])
        size = struct.unpack_from(">I", sep, o + 9)[0]
        start = o + 13
        end = parse_events(sep, start, start + size, census)
        seqs.append(dict(num=num, resolution=res, tempo=tempo, rhythm=rhythm, size=size, parsed=end - start))
        o = start + size
        n += 1
        if o + 13 > len(sep):
            break
    return magic, version, seqs, o


def check_adpcm(body, sizes, problems, name):
    o = 0
    blocks = flags = loops = 0
    for i, s in enumerate(sizes):
        if i == 0 or s == 0:
            continue
        vag = body[o:o + s]
        for b in range(0, s, 16):
            sf, fl = vag[b], vag[b + 1]
            blocks += 1
            if sf & 0xF > 12 or (sf >> 4) & 0xF > 4:
                problems.append(f"{name}: VAG {i} block {b // 16}: shift/filter byte {sf:#x}")
                break
            if fl & 4:
                loops += 1
        if not vag[s - 16 + 1] & 1:
            problems.append(f"{name}: VAG {i} does not end with a loop-end flag ({vag[s - 15]:#x})")
        flags += 1
        o += s
    return blocks, loops


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--events", action="store_true")
    args = ap.parse_args()
    exe = disc_files.EXE.read_bytes()
    paths = {e.id: e.path for e in disc_files.files()}
    problems = []
    census = Counter()
    totals = Counter()
    for bank_id, b in banks(exe):
        name = paths[b["header_file"]].split("/")[-2] if "/" in paths[b["header_file"]] else str(bank_id)
        hdr = disc_files.read(b["header_file"])
        body_file = disc_files.read(b["body_file"])
        vh_bytes = sub(hdr, b["vab_header"])
        vh = parse_vh(vh_bytes)
        if vh["magic"] != b"pBAV" or vh["version"] != 7:
            problems.append(f"{name}: VH magic {vh['magic']!r} version {vh['version']}")
        vag_bytes = sum(vh["vag_sizes"][1:vh["vags"] + 1])
        expect = vh["header_size"] + vag_bytes
        if vh["size"] != expect:
            problems.append(f"{name}: VH size {vh['size']:#x}, expected {expect:#x}")
        if any(vh["vag_sizes"][vh["vags"] + 1:]) or vh["vag_sizes"][0] != 0:
            problems.append(f"{name}: VAG size table has entries past {vh['vags']} or at 0")
        used = [p for p in vh["progs"] if p[0] != 0]
        if len(used) != vh["programs"] or sum(p[0] for p in vh["progs"]) != vh["tones"]:
            problems.append(f"{name}: programs {vh['programs']} tones {vh['tones']}: program records disagree")
        for p, t, f in vh["tone_recs"]:
            if t < vh["progs"][p][0]:
                prog, vag = f[18], f[19]
                if prog != p or not 1 <= vag <= vh["vags"]:
                    problems.append(f"{name}: program {p} tone {t}: prog {prog} vag {vag}")
                if f[6] > f[7]:
                    problems.append(f"{name}: program {p} tone {t}: key range {f[6]}..{f[7]}")
        body = sub(body_file, b["vab_body"])
        if len(body) < vag_bytes or any(body[vag_bytes:]) and len(body) - vag_bytes >= 2048:
            problems.append(f"{name}: body {len(body)} bytes for {vag_bytes} bytes of VAGs")
        blocks, loops = check_adpcm(body, vh["vag_sizes"][:vh["vags"] + 1], problems, name)
        seq_summary = []
        for s in b["seps"]:
            sep = sub(hdr, s)
            try:
                magic, version, seqs, end = parse_sep(sep, census)
            except (ValueError, IndexError, struct.error) as e:
                problems.append(f"{name}: SEP {s & 0xFFFF}: {e}")
                continue
            if magic != b"pQES" or version != 0:
                problems.append(f"{name}: SEP magic {magic!r} version {version}")
            if len(seqs) != 16:
                problems.append(f"{name}: SEP {s & 0xFFFF}: {len(seqs)} sequences (SsSepOpen opens 16)")
            for q in seqs:
                if q["parsed"] != q["size"]:
                    problems.append(f"{name}: SEP seq {q['num']}: size {q['size']} but end-of-track after {q['parsed']}")
                totals[f"resolution {q['resolution']}"] += 1
            seq_summary.append(f"{len(seqs)} seqs (tempo {min(q['tempo'] for q in seqs)}..{max(q['tempo'] for q in seqs)}"
                               f" us/qn)" if seqs else "0 seqs")
            totals["sequences"] += len(seqs)
        totals["banks"] += 1
        totals["programs"] += vh["programs"]
        totals["tones"] += vh["tones"]
        totals["vags"] += vh["vags"]
        totals["vag bytes"] += vag_bytes
        totals["adpcm blocks"] += blocks
        totals["loop-start blocks"] += loops
        if not args.check:
            print(f"bank {bank_id:2d} {name:10s} VH {vh['size']:#7x}: {vh['programs']:3d} programs {vh['tones']:4d} tones"
                  f" {vh['vags']:3d} VAGs ({vag_bytes:6d} bytes, {loops} loop starts); SEPs: " + "; ".join(seq_summary))
    print("totals: " + ", ".join(f"{k} {v}" for k, v in sorted(totals.items())))
    if args.events:
        for k, v in sorted(census.items()):
            print(f"  {k}: {v}")
    if problems:
        print(f"{len(problems)} problem(s):")
        for p in problems[:50]:
            print("  " + p)
        return 1
    print("sound formats: all claims hold")
    return 0


if __name__ == "__main__":
    sys.exit(main())
