#!/usr/bin/env python3
"""The battle scripts on the disc, for the battle_animations mod (docs/LAUNCHER.md "Disable battle animations").

Usage: tests/port/battle.py [--out DIR] [-v]

Every model's script file (FightstgModelRecord.script_file of SMDLDATA.PRO, file 0x1CC) and every script in it is
read here with this file's own reading of src/fightstg/fightstg_8008B630.c's command readers, and by the port's
port/game/battle_scan.c (compiled alone with tests/port/battle_scan_driver.c); both must agree on every script (that it
ends, its length, whether it has a child command, a multi-hit child, its first hit-sound command), with the script's
stage 0 and -1 (command 4 reads no fades for a stage of -1). Then the facts the mod relies on, required:
  - every script reaches its end command;
  - scripts 1-4 (the target's reactions: flinch, heavy hit, KO, dodge) have no child command;
  - script 3 (KO) ends on model animation 10 (the KO pose: command 2 op 0 arg 10 is its last model animation);
and a summary of scripts 5 and up (attacks, techniques, items): with a child command, with a multi-hit child, without
one (the mod ends those at once), with a hit sound.
Needs the extracted disc and the host gcc. Exit 0 pass, 1 fail, 2 missing.
"""
import argparse
import collections
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import disc_files as df  # noqa: E402

SMDLDATA = 0x1CC


def s16(d, o):
    return struct.unpack_from("<h", d, o)[0]


def s32(d, o):
    return struct.unpack_from("<i", d, o)[0]


def script_files():
    """{script_file sub-file ID: [model IDs]} from SMDLDATA's entries."""
    d = df.read(SMDLDATA)
    entries, recs_a, recs_b = s32(d, 4), s32(d, 8), s32(d, 12)
    out = collections.defaultdict(list)
    o = entries
    while s16(d, o) != 0:
        mid, index, kind = s16(d, o), d[o + 2], d[o + 3]
        rec = recs_b + index * 0x48 if kind >= 0x3A else recs_a + index * 0xC4
        out[s32(d, rec + 8)].append(mid)
        o += 4
    return out


def scripts_of(sub_id):
    """[(script index, the stream's bytes up to the next sub-file or the container's end)]."""
    data = df.read(sub_id >> 16)
    base = df.subfile(data, sub_id & 0xFFFF)
    count = s32(data, base) // 4
    offs = [s32(data, base + 4 * k) for k in range(count)]
    limit = [df.subfile(data, (sub_id & 0xFFFF) + 1)] if (sub_id & 0xFFFF) + 1 < s32(data, 0) // 4 else []
    end = (limit[0] if limit and limit[0] > base else len(data))
    out = []
    for k, off in enumerate(offs):
        nxt = min([o for o in offs if o > off] + [end - base])
        out.append((k, data[base + off:base + nxt]))
    return out


def scan(words, stage):
    """This file's reading of the command readers (see port/game/battle_scan.c's table): (ok, length, child, multi,
    sound, sound_arg, model animations set by command 2 op 0)."""
    i, child, multi, sound, sound_arg, anims = 0, 0, 0, 0, 0, []
    n = len(words)
    try:
        while i < n:
            cmd = words[i]
            i += 1
            if cmd in (0, 0xFF):
                return 1, i, child, multi, sound, sound_arg, anims
            if cmd == 1:
                op = words[i]
                i += 1
                if op in (0, 5):
                    child, multi = 1, multi | (op == 5)
            elif cmd == 2:
                op, i = words[i], i + 2
                if op in (3, 4):
                    i += 4
                elif op in (7, 8):
                    i += 1
                elif op not in (1, 2, 9):
                    arg = words[i]
                    i += 1
                    if op == 0 and arg not in (0, 1):
                        anims.append(arg)
                    if op == 5 and arg == 4:
                        i += 1
            elif cmd in (3, 6):
                op, i = words[i], i + 2
                if op != 1:
                    i += 3 if cmd == 3 else 6
            elif cmd == 4:
                op, sid, i = words[i], words[i + 1], i + 2
                if (stage if sid == 0x38 else sid) != -1 and op != 1:
                    i += 2
            elif cmd == 5:
                i += 16 if words[i + 1] == 1 else 3
            elif cmd in (7,):
                i += 2
            elif cmd == 10:
                sid, arg, i = words[i], words[i + 1], i + 2
                if sid in (0x62, 0x63) and not sound:
                    sound, sound_arg = sid, arg
            elif cmd == 11:
                i += 1
    except IndexError:
        pass
    return 0, 0, child, multi, sound, sound_arg, anims


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", help="scratch directory (default build/port-test/battle)")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if not df.EXE.exists() or shutil.which("gcc") is None:
        print("battle: needs the extracted disc (scripts/extract.sh) and gcc", file=sys.stderr)
        return 2
    out = (Path(args.out) if args.out else ROOT / "build/port-test/battle").resolve()
    out.mkdir(parents=True, exist_ok=True)
    driver = out / "battle_scan_driver"
    subprocess.run(["gcc", "-std=gnu99", "-O2", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT / "port/game"),
                    str(ROOT / "port/game/battle_scan.c"), str(ROOT / "tests/port/battle_scan_driver.c"), "-o",
                    str(driver)], check=True)
    files = script_files()
    items, records = [], bytearray()
    for sub_id in sorted(files):
        for k, raw in scripts_of(sub_id):
            words = list(struct.unpack_from(f"<{len(raw) // 2}h", raw))
            for stage in (0, -1):
                items.append((sub_id, k, stage, scan(words, stage)))
                records += struct.pack("<ii", len(words), stage) + raw[:len(words) * 2]
    (out / "records.bin").write_bytes(records)
    got = subprocess.run([str(driver), str(out / "records.bin")], capture_output=True, text=True, check=True)
    lines = got.stdout.split("\n")
    failures = []
    disagree = [(s, k, st) for (s, k, st, r), l in zip(items, lines) if tuple(map(int, l.split())) != r[:6]]
    if disagree:
        failures.append(f"port/game/battle_scan.c and this reading disagree on {len(disagree)} script(s): "
                        f"{', '.join(f'{s:#x}/{k} stage {st}' for s, k, st in disagree[:8])}")
    pal = [(s, k, r) for s, k, st, r in items if st == 0]
    nscripts = collections.Counter(len(scripts_of(s)) for s in files)
    print(f"battle: {len(files)} script files ({sum(len(v) for v in files.values())} model records), "
          f"scripts per file {dict(nscripts)}; {len(pal)} scripts, the C scanner and this reading "
          f"{'agree on all' if not disagree else 'DISAGREE'} (stages 0 and -1)")
    bad = [(s, k) for s, k, r in pal if not r[0]]
    if bad:
        failures.append(f"{len(bad)} script(s) do not reach their end: {bad[:8]}")
    react_child = [(s, k) for s, k, r in pal if 1 <= k <= 4 and r[2]]
    if react_child:
        failures.append(f"reaction scripts with a child command: {react_child[:8]}")
    ko = [(s, r[6][-1] if r[6] else None) for s, k, r in pal if k == 3]
    ko_bad = [(hex(s), a) for s, a in ko if a != 10]
    if ko_bad:
        failures.append(f"KO scripts (3) whose last model animation is not 10: {ko_bad[:8]}")
    print(f"  every script ends: {'yes' if not bad else 'NO'}; reactions 1-4 without a child command: "
          f"{'yes' if not react_child else 'NO'}; every KO script (3) ends on animation 10: "
          f"{'yes' if not ko_bad else 'NO'} ({len(ko)} files)")
    attacks = [(s, k, r) for s, k, r in pal if k >= 5 and r[1] > 1]
    empty = [(s, k) for s, k, r in pal if k >= 5 and r[1] <= 1]
    with_child = [(s, k, r) for s, k, r in attacks if r[2]]
    without = [(s, k) for s, k, r in attacks if not r[2]]
    print(f"  scripts 5-17: {len(attacks)} with commands ({len(empty)} empty), {len(with_child)} with a child command "
          f"({sum(1 for *_, r in with_child if r[3])} multi-hit), {len(without)} without (the mod ends them at once), "
          f"{sum(1 for *_, r in attacks if r[4])} with a hit sound")
    by_index = collections.Counter(k for s, k in without)
    print(f"    without a child command, by script index: {dict(sorted(by_index.items()))}")
    if args.verbose:
        for s, k in without:
            print(f"      {s:#x} script {k} (models {files[s]})")
    print(f"battle: {'FAIL' if failures else 'pass'}")
    for f in failures:
        print(f"  {f}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
