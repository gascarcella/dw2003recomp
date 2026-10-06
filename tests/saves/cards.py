#!/usr/bin/env python3
"""Format checks of memory card images (.mcd) holding the game's save (docs/FORMATS.md "Save data", "The card image").

Usage: tests/saves/cards.py CARD.mcd [OTHER.mcd]
  one card:  its directory (frame checksums, the save file's chain of blocks, every other block free), the save file's
             Sony header, the game's header and the saved slot (version, magic, both XOR checksums exact)
  two cards: both as above, then byte for byte equal except MASKS (each named, with its reason)
tests/saves/run.py calls check_pair on the port's and the emulator's card after their saves. Exit 0 pass, 1 fail.
"""
import sys
from functools import reduce
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/replay"))
from replay import VOLATILE_RANGES  # noqa: E402

CARD_SIZE = 0x20000
BLOCK, FRAME = 0x2000, 0x80
FILE_NAME = b"BESLES-03936DMW3-EUR"      # memcard_set_file_name, records_language 2 (EU)
FILE_BLOCKS = 4                           # memcard_create_file: 0x8000 bytes
# The save file's parts (memcard_read/_write: 3 icon frames after the Sony header, then part 1, then a slot per part).
HEADER_AT, HEADER_USED, HEADER_SIZE = 0x200, 0xD4, 0x100     # StgmcardSaveHeader
SLOT_AT, SLOT_USED, SLOT_SIZE = 0x300, 0x26C4, 0x2700         # StgmcardSaveData: gamestate_data's first 0x26C4 bytes
SUMMARY_AT, SUMMARY_SIZE, SUMMARY_PLAYTIME = 0x08, 0x44, (0x24, 0x30)  # StgmcardSlotSummary.playtime (StgmcardPlayInfo)
VERSION = 4
MAGIC = b"DMW3"

# The fields of a saved slot that depend on timing: replay.py's VOLATILE_RANGES inside the slot (the rest of them,
# spot_target, is runtime state after 0x26C4 and is not saved). Reasons in the masks below.
SLOT_VOLATILE = ((0x00, 0x01), (0x30, 0x34), (0x48, 0x54))
assert set(SLOT_VOLATILE) == {r for r in VOLATILE_RANGES if r[1] <= SLOT_USED}, "replay.VOLATILE_RANGES changed"


def masks(file_at, slot):
    """(start, end, name, reason) of every card range two saves of the same game state may differ in, for a save file
    at card offset file_at whose slot `slot` was saved last."""
    hdr = file_at + HEADER_AT
    summary = hdr + SUMMARY_AT + slot * SUMMARY_SIZE
    part = file_at + SLOT_AT + slot * SLOT_SIZE
    return [
        (63 * FRAME, 64 * FRAME, "directory frame 63",
         "the emulator's BIOS writes a buffer of its own there when it clears a new card's flag (OpenBIOS: not a "
         "directory frame, no valid checksum); the port leaves the formatted card's copy of frame 0"),
        (hdr, hdr + 1, "header checksum", "the XOR of header bytes 4..0xD3, which hold the play time"),
        (summary + SUMMARY_PLAYTIME[0], summary + SUMMARY_PLAYTIME[1], f"header slots[{slot}].playtime",
         "the play time at the save (frames since boot: the port's CD and movie timing differ from the emulator's)"),
        (hdr + HEADER_USED, hdr + HEADER_SIZE, "header tail 0xD4..0xFF",
         "written in 0x80-byte calls from a 0x180-byte heap buffer that is never cleared: stale RAM, never read back"),
        (part + 0x00, part + 0x01, "slot checksum", "the XOR of slot bytes 4..0x26C3, which hold the timers below"),
        (part + 0x30, part + 0x34, "slot encounter_timer",
         "steps every 8th moving frame, so it follows the frames the walk took (VOLATILE_RANGES)"),
        (part + 0x48, part + 0x54, "slot playtime_frames, playtime[4]", "the play time again (VOLATILE_RANGES)"),
        (part + SLOT_USED, part + SLOT_SIZE, "slot tail 0x26C4..0x26FF",
         "written in 0x80-byte calls from a 0x2780-byte heap buffer that is never cleared: stale RAM"),
    ]


def xor(data):
    return reduce(lambda a, b: a ^ b, data, 0)


def u16(c, at):
    return int.from_bytes(c[at:at + 2], "little")


def u32(c, at):
    return int.from_bytes(c[at:at + 4], "little")


def check_card(c, name):
    """The format checks of one card; returns (failures, file offset, saved slot) (the offset None if no file)."""
    if len(c) != CARD_SIZE:
        return [f"{name}: {len(c)} bytes, not a raw 128 KB card image"], None, None
    fail = []
    if c[0:2] != b"MC":
        fail.append(f"{name}: directory frame 0 does not start with 'MC'")
    # Frames 0..62 (frame 63 is the BIOS's: masks()): every directory frame's byte 0x7F is the XOR of its bytes 0..0x7E,
    # the unused frames 36..62 are zeros, whose checksum is 0 too.
    bad = [i for i in range(63) if xor(c[i * FRAME:i * FRAME + FRAME - 1]) != c[i * FRAME + FRAME - 1]]
    if bad:
        fail.append(f"{name}: directory frame checksum (byte 0x7F) wrong in frame(s) {bad}")
    entries = [c[i * FRAME:(i + 1) * FRAME] for i in range(1, 16)]
    first = [i for i, e in enumerate(entries) if e[0] == 0x51]
    files = [i for i in first if entries[i][0x0A:0x0A + len(FILE_NAME) + 1] == FILE_NAME + b"\0"]
    if len(files) != 1 or len(first) != 1:
        fail.append(f"{name}: {len(files)} save file(s) named {FILE_NAME.decode()} and {len(first)} file(s) in all "
                    "in the directory (expected the save alone)")
        return fail, None, None
    block, chain = files[0], []
    if u32(entries[block], 4) != FILE_BLOCKS * BLOCK:
        fail.append(f"{name}: the save file's size is {u32(entries[block], 4):#x}, not {FILE_BLOCKS * BLOCK:#x}")
    while block is not None and len(chain) <= 15:
        chain.append(block)
        nxt = u16(entries[block], 8)
        block = None if nxt == 0xFFFF else nxt
    states = [entries[b][0] for b in chain]
    want = [0x51] + [0x52] * (FILE_BLOCKS - 2) + [0x53]
    if states != want:
        fail.append(f"{name}: the save file's blocks {[b + 1 for b in chain]} have states "
                    f"{[hex(s) for s in states]}, not {[hex(s) for s in want]}")
    used = [i for i, e in enumerate(entries) if e[0] != 0xA0]
    if sorted(used) != sorted(chain):
        fail.append(f"{name}: directory entries {[i + 1 for i in used]} in use, the save file is "
                    f"{[b + 1 for b in chain]}")
    if chain != list(range(chain[0], chain[0] + FILE_BLOCKS)):
        fail.append(f"{name}: the save file's blocks {[b + 1 for b in chain]} are not contiguous")
        return fail, None, None
    at = (chain[0] + 1) * BLOCK
    f = c[at:at + FILE_BLOCKS * BLOCK]
    if f[0:2] != b"SC" or f[2] != 0x13 or f[3] != FILE_BLOCKS:
        fail.append(f"{name}: Sony header {f[0:4].hex()}, not 'SC', 0x13 (three icon frames), {FILE_BLOCKS} blocks")
    hdr = f[HEADER_AT:HEADER_AT + HEADER_USED]
    slot = hdr[1]
    if hdr[2] != VERSION or hdr[4:8] != MAGIC or slot > 2:
        fail.append(f"{name}: game header version {hdr[2]}, magic {hdr[4:8]!r}, last slot {slot} (expected "
                    f"{VERSION}, {MAGIC!r}, 0..2)")
        return fail, at, None
    if xor(hdr[4:]) != hdr[0]:
        fail.append(f"{name}: game header checksum {hdr[0]:#04x}, the XOR of bytes 4..0xD3 is {xor(hdr[4:]):#04x}")
    part = f[SLOT_AT + slot * SLOT_SIZE:SLOT_AT + slot * SLOT_SIZE + SLOT_USED]
    if part[2] != VERSION:
        fail.append(f"{name}: slot {slot} version {part[2]}, not {VERSION}")
    if xor(part[4:]) != part[0]:
        fail.append(f"{name}: slot {slot} checksum {part[0]:#04x}, the XOR of bytes 4..0x26C3 is {xor(part[4:]):#04x}")
    return fail, at, slot


def describe(off, file_at):
    """Where a card offset is, in words."""
    if off < BLOCK:
        return f"directory frame {off // FRAME} +{off % FRAME:#x}"
    if file_at is None or not file_at <= off < file_at + FILE_BLOCKS * BLOCK:
        return f"block {off // BLOCK} +{off % BLOCK:#x} (no file)"
    o = off - file_at
    if o < FRAME:
        return f"save file Sony header +{o:#x}"
    if o < HEADER_AT:
        return f"save file icon frame {o // FRAME - 1} +{o % FRAME:#x}"
    if o < SLOT_AT:
        return f"game header +{o - HEADER_AT:#x}"
    s, r = divmod(o - SLOT_AT, SLOT_SIZE)
    return f"slot {s} +{r:#x}" if s < 3 else f"save file's unused end +{o - SLOT_AT - 3 * SLOT_SIZE:#x}"


def check_pair(a, b, name_a, name_b, indent=""):
    """Both cards' format checks, then byte equality outside masks(); prints one line per check, returns failures."""
    fa, at_a, slot_a = check_card(a, name_a)
    fb, at_b, slot_b = check_card(b, name_b)
    failures = fa + fb
    print(f"{indent}{name_a}: {'ok' if not fa else 'FAIL'}; {name_b}: {'ok' if not fb else 'FAIL'} "
          "(directory checksums, the save file's 4 blocks, Sony header, game header and slot: version, both XOR sums)")
    if at_a is None or at_b is None or slot_a is None or slot_b is None:
        return failures
    if (at_a, slot_a) != (at_b, slot_b):
        return failures + [f"the save file is at {at_a:#x} slot {slot_a} on {name_a}, at {at_b:#x} slot {slot_b} on "
                           f"{name_b}"]
    ms = masks(at_a, slot_a)
    masked = bytearray(CARD_SIZE)
    for lo, hi, _, _ in ms:
        masked[lo:hi] = b"\1" * (hi - lo)
    diff = [i for i in range(CARD_SIZE) if a[i] != b[i] and not masked[i]]
    if diff:
        runs = []
        for i in diff:
            if runs and i == runs[-1][1]:
                runs[-1][1] = i + 1
            else:
                runs.append([i, i + 1])
        for lo, hi in runs[:8]:
            failures.append(f"{name_a} vs {name_b}: {hi - lo} byte(s) differ at card {lo:#07x} "
                            f"({describe(lo, at_a)}): {a[lo:min(hi, lo + 16)].hex()} vs {b[lo:min(hi, lo + 16)].hex()}")
        if len(runs) > 8:
            failures.append(f"{name_a} vs {name_b}: ... {len(runs) - 8} more differing range(s)")
    differing = [f"{n} ({sum(1 for i in range(lo, hi) if a[i] != b[i])})" for lo, hi, n, _ in ms if a[lo:hi] != b[lo:hi]]
    print(f"{indent}equal byte for byte but the {len(ms)} masked fields: "
          f"{'ok' if not diff else f'FAIL ({len(diff)} bytes)'}; masked fields that differ (bytes): "
          f"{', '.join(differing) or 'none'}")
    return failures


def main():
    if len(sys.argv) not in (2, 3):
        print(__doc__, file=sys.stderr)
        return 2
    cards = [Path(p) for p in sys.argv[1:]]
    if len(cards) == 1:
        failures, at, slot = check_card(cards[0].read_bytes(), cards[0].name)
        if not failures:
            print(f"{cards[0].name}: ok (save file at {at:#x}, slot {slot})")
    else:
        failures = check_pair(cards[0].read_bytes(), cards[1].read_bytes(), cards[0].name, cards[1].name)
    for f in failures:
        print("FAIL: " + f)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
