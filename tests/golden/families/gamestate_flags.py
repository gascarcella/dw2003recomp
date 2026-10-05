"""gamestate_get_flag / gamestate_check_flags / gamestate_check_condition (src/main/gamestate.c, include/gamestate.h).

A flag is (type << 8 | index): types 0x02..0x40 are bit arrays in gamestate_data (type 0x00 is gamestate_flags.map_flags),
0x60 compares progress, 0x70 runs a condition of gamestate_conditions, 0x72 the party's stat sum, 0x7E route/room,
0x80..0x8E an item, 0x92 a card; any other type returns 1. docs/MECHANICS.md section 11.
"""
import re
from pathlib import Path

from oracle import Call, Case, Read, Write

COMMENT = ("Over one fixed gamestate_data (flag bytes patterned, progress 20, a few items, cards, route, party and Digimon set): "
           "gamestate_get_flag(flag, value) for a grid of types x indexes x values, gamestate_check_flags over lists, and "
           "gamestate_check_condition(id, value) for every id of gamestate_conditions whose type has no side effect outside "
           "gamestate_data (types 0x20, money, and 0x50, an object, are skipped). Each call is its own case: the fixture is "
           "rewritten and gamestate_data restored, so a condition that writes (digimon type 3 marks a Digimon joined) cannot "
           "leak into the next call; such writes are recorded as a SHA-1 of gamestate_data after the call.")

GS_SIZE = 0x275C
GS_FUNCS = 0x26FC   # .funcs: the function table; the bytes before it are pointer-free (the host replay can compare them)
# Offsets in GamestateData (include/gamestate.h).
OFF = dict(route=0x44, room=0x46, party=0x70, items=0x7C, items_equipped=0x20F, cards=0x3A2, digimon=0x75C,
           progress=0x263C, party_set=0x2640, flags=0x2644, flags_end=0x26C4)
DIGIMON_SIZE, JOINED, LEVEL = 0x3DC, 0x4, 0xC + 0x1C   # GamestateDigimon.joined; .record.stats.values[0]
TYPES = [0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x18, 0x1A, 0x1C, 0x20, 0x40, 0x50, 0x60, 0x70, 0x72,
         0x7E, 0x80, 0x8E, 0x92, 0xA0]
INDEXES = [0, 1, 7, 8, 13, 20, 31, 63, 100, 200, 511]
SKIP_COND_TYPES = {0x20: "gamestate_change_money (writes money)", 0x50: "gamestate_cond_object (needs a live object)"}


def flag_pattern():
    """Bytes of the flag arrays 0x2644..0x26C4: a fixed pattern with every byte distinct from its neighbours."""
    return bytes(((i * 37 + 11) ^ (i >> 3)) & 0xFF for i in range(OFF["flags_end"] - OFF["flags"]))


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0: the whole pointer-free part, so the "
               "fixture is complete (the game's state at CNTY_SEL and a host process then agree)"),
         Write("gamestate_flags", 0, bytes(0x1C), "gamestate_flags = 0"),
         Write("gamestate_data", OFF["flags"], flag_pattern(), "flags..flags_40 (all flag bit arrays)"),
         Write("gamestate_flags", 0, bytes([0xA5, 0x3C, 0x81]), "gamestate_flags.map_flags[3] (flag type 0x00)"),
         Write("gamestate_data", OFF["progress"], (20).to_bytes(4, "little"), "progress = 20"),
         Write("gamestate_data", OFF["party_set"], (1).to_bytes(4, "little"), "party_set = 1"),
         Write("gamestate_data", OFF["route"], (3).to_bytes(2, "little") + (2).to_bytes(2, "little"), "route = 3, room = 2"),
         Write("gamestate_data", OFF["party"], (1).to_bytes(4, "little") + (0xFFFFFFFF).to_bytes(4, "little") * 2,
               "party = {1, -1, -1}")]
    for idx, val in ((7, 1), (167, 2), (105, 1), (119, 1), (131, 3), (0, 1), (200, 1)):
        w.append(Write("gamestate_data", OFF["items"] + idx, bytes([val]), f"items[{idx}] = {val}"))
    for idx, val in ((89, 1), (144, 1), (1, 2)):
        w.append(Write("gamestate_data", OFF["items_equipped"] + idx, bytes([val]), f"items_equipped[{idx}] = {val}"))
    for idx, val in ((0, 1), (7, 3), (100, 9), (200, 1)):
        w.append(Write("gamestate_data", OFF["cards"] + idx, bytes([val]), f"cards[{idx}] = {val}"))
    for d, joined, level in ((1, 4, 50), (2, 5, 10), (5, 8, 45)):
        base = OFF["digimon"] + d * DIGIMON_SIZE
        w.append(Write("gamestate_data", base + JOINED, joined.to_bytes(4, "little"), f"digimon[{d}].joined = {joined}"))
        w.append(Write("gamestate_data", base + LEVEL, level.to_bytes(2, "little"), f"digimon[{d}].record.stats.values[0] = {level} (level)"))
    return w


def conditions():
    """(id, type, arg) of gamestate_conditions, parsed from src/main/gamestate.c (the table the game reads)."""
    src = (Path(__file__).resolve().parents[3] / "src/main/gamestate.c").read_text()
    body = src[src.index("gamestate_conditions[] = {"):]
    body = body[:body.index("};")]
    rows = [(int(a, 16), int(b, 16), int(c)) for a, b, c in
            re.findall(r"\{\s*(0x[0-9A-Fa-f]+),\s*(0x[0-9A-Fa-f]+),\s*(\d+)\s*\}", body)]
    return [r for r in rows if r[0] != 0xFF]


def cases(sym):
    fx = fixture()
    saves = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C)]
    # two reads around playtime_frames (0x48), which the vsync handler counts (DECISIONS "Session 10 oracle lessons")
    after = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
             Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]
    out = []
    for t in TYPES:
        for i in INDEXES:
            for v in (0, 1):
                flag = (t << 8) | i
                out.append(Case(f"get_flag_{flag:04x}_{v}",
                                [Call("gamestate_get_flag", [flag, v], "s32", after, comment=f"type {t:#04x} index {i} value {v}")],
                                fixture=fx, saves=saves))
    lists = {
        "empty": [],
        "one_true": [(0x0203, 1)],
        "one_false": [(0x0203, 0)],
        "two_mixed": [(0x0203, 1), (0x0205, 0)],
        "progress_20": [(0x6014, 1), (0x0203, 1)],
        "unknown_type": [(0xA005, 0), (0x0203, 1)],
        "item_7": [(0x8007, 1), (0x8008, 0)],
    }
    for name, pairs in lists.items():
        data = b"".join(x.to_bytes(2, "little") for p in pairs for x in p) + b"\xff\xff"
        out.append(Case(f"check_flags_{name}", [Call("gamestate_check_flags", [("buf", "list")], "s32", after,
                                                     comment=f"list {pairs} then 0xFFFF")],
                        fixture=fx, buffers={"list": data}, saves=saves))
    for cid, ctype, arg in conditions():
        if ctype & 0xF0 in SKIP_COND_TYPES:
            continue
        for v in (0, 1):
            out.append(Case(f"check_condition_{cid:02x}_{v}",
                            [Call("gamestate_check_condition", [cid, v], "s32", after,
                                  comment=f"id {cid:#04x}: type {ctype:#04x} arg {arg}; value {v}")],
                            fixture=fx, saves=saves))
    for v in (0, 1):
        out.append(Case(f"check_condition_fe_{v}", [Call("gamestate_check_condition", [0xFE, v], "s32", after,
                                                        comment="an id that is not in the table: ret stays 0")],
                        fixture=fx, saves=saves))
    return out
