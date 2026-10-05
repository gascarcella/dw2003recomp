"""What the event scripts do to the game state (src/main/gamestate.c): gamestate_set_flag, gamestate_set_flags,
gamestate_update_map_flags and gamestate_cond_object, the condition left out of gamestate_flags because it acts on a
live object. docs/MECHANICS.md section 11.

gamestate_set_flag(flag, value): type = (flag >> 8) & ~1, index = flag & 0x1FF (as gamestate_get_flag). The bit-array
types 0x00 (gamestate_flags.map_flags) and 0x02..0x40 set the bit for any non-zero value, else clear it; nothing checks
the index against the array (type 0x06 is one byte: index 8 is flags_08's bit 0). 0x70 runs the condition `index` with
value 1 (its side effect: condition 0x31 marks Digimon 3 joined = 6); 0x80..0x8E gamestate_change_item(index, value)
(+1 with a cap of 99, or -1), 0x92 gamestate_change_card. 0x60 (progress), 0x7E (route) and unknown types write nothing.
The types that start a stage or an event (0x74, 0x76, 0x78, 0x7A, 0x7C, 0x90, 0x94) call FIELDSTG's code and are not
cases (layer 2). gamestate_set_flags(list): (flag, value) pairs up to 0xFFFF.

gamestate_update_map_flags(): a new map (map_is_new) clears map_flags (all 24 type-0 flags); back from a card game
(prev_map 0x700) sets flags 0x11 and 0x12, flag 0x10 = card_game_won, and clears card_game_won.

gamestate_cond_object(sub, arg) ignores both arguments: heap_objects.find(0x16, -1, -1) (heap_find_object: the first
object of heap_objects.objects[0..99] whose kind is 0x16, keys and state not looked at; it leaves the filter and
cursor = slot + 1 behind), then obj->set_step(obj, 3) (object_set_step: step 3, substep and timer 0, state untouched),
and returns 1. Kind 0x16 is FIELDSTG's actor effect (fieldstg_actor_effect_create). gamestate_check_condition(0x13,
value) returns value == 1. heap_objects.objects[] is replaced by the case's list of scratch objects (`object_*`
buffers: an Object header with kind, keys, state, step, substep, timer and set_step = object_set_step; the other
methods 0, as nothing else is called), restored after the case. With no kind-0x16 object, find returns NULL and the
original calls through *(NULL + 0x2C) (the BIOS area): not a case, a crash (the only caller is condition 0x13, from the
event scripts).

The flag cases use gamestate_flags' fixture (pointer-free gamestate_data zeroed, the flag arrays patterned).
Hand checks: object_a (step 1, substep 2, timer 5) -> step 3, substep 0, timer 0, filter {0x16, -1, -1}, cursor 1;
back from a won card game: map_flags[2] 0x81 -> 0x87 (flags 0x10-0x12 are bits 0-2 of byte 2).
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE, OFF, fixture as flags_fixture, flag_pattern
from oracle import SCRATCH_BASE, Call, Case, Read, Write

COMMENT = ("gamestate_set_flag for every bit-array type (set and clear), an index past its array, a value of 2, the types "
           "that write nothing, a condition (0x70), items (0x80, 0x8E) and a card (0x92); gamestate_set_flags over a list "
           "and an empty one; gamestate_update_map_flags (new map, back from a won or lost card game); gamestate_cond_object "
           "(condition 0x13, type 0x50): the first kind-0x16 object of heap_objects gets set_step(3) and 1 is returned, "
           "directly and through gamestate_check_condition, among other kinds and empty slots, in the last slot. "
           "heap_objects.objects is a fixture of scratch Object headers.")

# The bit arrays: type -> (symbol, offset of the array, bytes).
ARRAYS = {0x00: ("gamestate_flags", 0, 3), 0x02: ("gamestate_data", 0x2644, 0x12), 0x04: ("gamestate_data", 0x2656, 2),
          0x06: ("gamestate_data", 0x2658, 1), 0x08: ("gamestate_data", 0x2659, 1), 0x0A: ("gamestate_data", 0x265A, 4),
          0x0C: ("gamestate_data", 0x265E, 8), 0x0E: ("gamestate_data", 0x2666, 0xC), 0x10: ("gamestate_data", 0x2672, 4),
          0x18: ("gamestate_data", 0x2676, 2), 0x1A: ("gamestate_data", 0x2678, 9), 0x1C: ("gamestate_data", 0x2681, 0xB),
          0x20: ("gamestate_data", 0x268C, 0x1E), 0x40: ("gamestate_data", 0x26AA, 0x1A)}
MAP_FLAGS = bytes([0xA5, 0x3C, 0x81])     # gamestate_flags' fixture
PREV_MAP, MAP_IS_NEW = 0x26CC, 0x26D8
OBJECT_SIZE, SLOTS, KIND = 0x50, 100, 0x16
SAVES = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C)]
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)"),
         Read("gamestate_flags", 0, 8, "gamestate_flags.map_flags[3], card_game_won after the call")]


def array_bytes(t):
    sym, off, n = ARRAYS[t]
    if sym == "gamestate_flags":
        return MAP_FLAGS
    p = flag_pattern()
    return p[off - OFF["flags"]:off - OFF["flags"] + n]


def pick_index(t, bit):
    """The highest index of type t's array whose bit is `bit` in the fixture (so the call changes it)."""
    b = array_bytes(t)
    for i in range(len(b) * 8 - 1, -1, -1):
        if (b[i >> 3] >> (i & 7)) & 1 == bit:
            return i
    raise ValueError(t)


def flag_case(name, calls, comment, extra=()):
    return Case(name, calls, fixture=flags_fixture() + list(extra), saves=SAVES, comment=comment)


def set_flag_cases():
    out = []
    for t in ARRAYS:
        for v in (1, 0):
            i = pick_index(t, 1 - v)
            flag = (t << 8) | i
            out.append(flag_case(f"set_flag_{flag:04x}_{v}", [Call("gamestate_set_flag", [flag, v], "void", AFTER,
                                 comment=f"type {t:#04x} index {i}: bit {'set' if v else 'cleared'} (was {1 - v})")],
                                 f"{ARRAYS[t][0]} + {ARRAYS[t][1]:#x}: byte {i >> 3}, mask {1 << (i & 7):#04x}"))
    out.append(flag_case("set_flag_0608_overrun", [Call("gamestate_set_flag", [0x0608, 1], "void", AFTER,
                         comment="flags_06 is one byte: index 8 sets flags_08 bit 0 (no bound check)")],
                         "an index past its array writes the next one (the game's data never does: flag_census)",
                         [Write("gamestate_data", 0x2659, bytes([0x00]), "flags_08 = 0")]))
    out.append(flag_case("set_flag_value2", [Call("gamestate_set_flag", [0x0200 | pick_index(0x02, 0), 2], "void", AFTER,
                         comment="value 2: set (any non-zero value sets)")], "the value is a truth value"))
    for flag, what in ((0x6005, "type 0x60 (progress): nothing, progress stays 20"),
                       (0x7E05, "type 0x7E (route): nothing"),
                       (0xA005, "type 0xA0 (unknown): nothing")):
        out.append(flag_case(f"set_flag_{flag:04x}_noop", [Call("gamestate_set_flag", [flag, 1], "void", AFTER, comment=what)],
                             "types gamestate_get_flag tests but gamestate_set_flag does not write"))
    out.append(flag_case("set_flag_7031", [Call("gamestate_set_flag", [0x7031, 0], "void", AFTER,
                         comment="condition 0x31 (type 0x13, arg 3) run with value 1: digimon[3].joined = 6 (the value 0 is not passed)")],
                         "type 0x70 runs a condition for its side effect"))
    for flag, v, what in ((0x802B, 1, "type 0x80, item 43: 0 -> 1"),
                          (0x8EA7, 1, "type 0x8E, item 167: 2 -> 3 (every type 0x80..0x8E is the item index)"),
                          (0x8007, 0, "type 0x80, item 7, value 0: 1 -> 0")):
        out.append(flag_case(f"set_flag_{flag:04x}_{v}", [Call("gamestate_set_flag", [flag, v], "void", AFTER, comment=what)],
                             "items go through gamestate_change_item (gamestate_records has its branches)"))
    out.append(flag_case("set_flag_9207_0", [Call("gamestate_set_flag", [0x9207, 0], "void", AFTER, comment="card 7: 3 -> 2")],
                         "cards go through gamestate_change_card"))
    return out


def list_buffer(pairs):
    return b"".join(struct.pack("<HH", f, v) for f, v in pairs) + struct.pack("<H", 0xFFFF)


def set_flags_cases():
    pairs = [(0x0200 | pick_index(0x02, 0), 1), (0x0400 | pick_index(0x04, 1), 0), (0x802B, 1)]
    return [
        Case("set_flags_list", [Call("gamestate_set_flags", [("buf", "list")], "void", AFTER,
                                     comment="three pairs: a flags bit set, a flags_04 bit cleared, item 43 + 1")],
             fixture=flags_fixture(), saves=SAVES, buffers={"list": list_buffer(pairs)},
             comment="(flag, value) pairs ending with 0xFFFF"),
        Case("set_flags_empty", [Call("gamestate_set_flags", [("buf", "list")], "void", AFTER, comment="just the 0xFFFF end: nothing")],
             fixture=flags_fixture(), saves=SAVES, buffers={"list": list_buffer([])}, comment="an empty list"),
    ]


def map_case(name, is_new, prev, won, comment):
    extra = [Write("gamestate_data", MAP_IS_NEW, struct.pack("<i", is_new), f"map_is_new = {is_new}"),
             Write("gamestate_data", PREV_MAP, struct.pack("<i", prev), f"prev_map = {prev:#x}"),
             Write("gamestate_flags", 4, struct.pack("<i", won), f"card_game_won = {won}")]
    return flag_case(name, [Call("gamestate_update_map_flags", [], "void", AFTER, comment=comment)], comment, extra)


def map_cases():
    return [
        map_case("map_flags_new_map", 1, 0x200, 0, "a new map: map_flags 0xA5 0x3C 0x81 -> 0"),
        map_case("map_flags_same_map", 0, 0x200, 1, "the same map, not from a card game: nothing (card_game_won kept)"),
        map_case("map_flags_card_won", 0, 0x700, 1, "back from a won card game: flags 0x10, 0x11, 0x12 set, card_game_won 0"),
        map_case("map_flags_card_lost", 0, 0x700, 0, "back from a lost card game: 0x10 cleared, 0x11 and 0x12 set"),
        map_case("map_flags_new_and_card", 1, 0x700, 1, "both: cleared first, then 0x10-0x12 set"),
    ]


def obj(sym, kind, key1=0, key2=0, state=1, step=1, substep=2, timer=5):
    """An Object header: kind, key1, key2, state, step, substep, timer, paused 0, children 0; set_step (0x2C) =
    object_set_step, the other methods 0."""
    b = bytearray(OBJECT_SIZE)
    struct.pack_into("<7i", b, 0, kind, key1, key2, state, step, substep, timer)
    struct.pack_into("<I", b, 0x2C, sym["object_set_step"])
    return bytes(b)


def objects_case(name, objects, slots, call, comment):
    """objects: [(buffer name, Object bytes)]; slots: {slot: buffer name}. The buffers are placed from SCRATCH_BASE in
    order (16-aligned, as the oracle does): every Object is 0x50 bytes."""
    places = {n: SCRATCH_BASE + i * OBJECT_SIZE for i, (n, _) in enumerate(objects)}
    words = [0] * SLOTS
    for s, n in slots.items():
        words[s] = places[n]
    fixture = [Write("heap_objects", 0, struct.pack(f"<{SLOTS}I", *words),
                     "heap_objects.objects[100] = " + ", ".join(f"[{s}] {n}" for s, n in sorted(slots.items())))]
    call.reads = [Read(f"buf:{n}", 0, 0x20, f"{n}: kind, key1, key2, state, step, substep, timer, paused") for n, _ in objects]
    call.reads.append(Read("heap_objects", 0x190, 0x10, "heap_objects.filter[3], cursor"))
    return Case(name, [call], fixture=fixture, buffers=dict(objects), saves=[("heap_objects", 0x1A0)], comment=comment)


def object_cases(sym):
    a = obj(sym, KIND, key1=7, key2=9)
    return [
        objects_case("object_slot0", [("object_a", a)], {0: "object_a"},
                     Call("gamestate_cond_object", [0, 0], "s32", comment="object_a: step 1 -> 3, substep 2 -> 0, timer 5 -> 0; returns 1"),
                     "one actor effect in slot 0 (keys 7, 9: not matched); the arguments are ignored"),
        objects_case("object_condition_1", [("object_a", a)], {0: "object_a"},
                     Call("gamestate_check_condition", [0x13, 1], "s32", comment="condition 0x13 (type 0x50), value 1: returns 1"),
                     "gamestate_check_condition dispatches id 0x13 to gamestate_cond_object"),
        objects_case("object_condition_0", [("object_a", a)], {0: "object_a"},
                     Call("gamestate_check_condition", [0x13, 0], "s32", comment="value 0: returns 0, the object still stepped"),
                     "the side effect does not depend on the value compared"),
        objects_case("object_first_match", [("object_b", obj(sym, 0x15, step=4)), ("object_a", a), ("object_c", obj(sym, KIND, step=6))],
                     {0: "object_b", 2: "object_a", 3: "object_c"},
                     Call("gamestate_cond_object", [0, 0], "s32", comment="object_b (kind 0x15) skipped, slot 1 empty, object_a "
                          "stepped, object_c (also kind 0x16) untouched; cursor 3"),
                     "the first kind-0x16 object in slot order, nothing past it"),
        objects_case("object_last_slot_state2", [("object_a", obj(sym, KIND, state=2, step=0, substep=1, timer=0))], {99: "object_a"},
                     Call("gamestate_cond_object", [0, 0], "s32", comment="slot 99 found (cursor 100); state 2 (done) not checked, kept"),
                     "the search covers all 100 slots; an object that has finished its job is still found"),
    ]


def cases(sym):
    return set_flag_cases() + set_flags_cases() + map_cases() + object_cases(sym)
