"""The gamestate record functions (src/main/gamestate.c): stats, equipment bonuses, money, items, cards, forms, party,
playtime, and records_list_items (src/main/records.c), which filters an item list by gamestate_data.items.
docs/MECHANICS.md sections 6, 7, 9, 11.

One fixture: the whole pointer-free gamestate_data zeroed, then party {0, 3, 5} with Digimon 0, 1, 3, 5 joined, four
records with stats, penalties, equipment (a sword, a hat, a gem; a two-handed Claymore in both hand slots) and forms
(Digimon 1 owns all 44), item counts (some only equipped), cards and 5,000 money. Every call reads back what it may
have written (the record, the equipment, the items, the money, ...) and the SHA-1 of gamestate_data up to .funcs.
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE, conditions
from oracle import Call, Case, Read, Write

COMMENT = ("Over one gamestate_data fixture (party {0, 3, 5}; records with stats, penalties, equipment and forms; item counts; "
           "cards; 5,000 money): gamestate_set_stat and gamestate_add_stat on every stat index with the clamps and the s16 "
           "wrap; gamestate_add_stat_bonus for every bonus type; gamestate_get_stats with a weapon, armour, an accessory, "
           "penalties, a two-handed weapon in both slots, a key and a usable item in a slot, an item set and a near miss; "
           "gamestate_change_money for every table index and money value, and gamestate_check_condition for the money "
           "conditions (type 0x20, skipped by gamestate_flags); gamestate_change_item (counts, the equipped branches), "
           "gamestate_unequip_item, gamestate_change_card/gamestate_add_card; the form functions (duplicates, the 44 limit, "
           "an id below 3); party getters and gamestate_set_party; gamestate_tick_playtime at every carry; "
           "gamestate_init_cards; records_list_items for every list, owned and equipped.")

# Offsets in GamestateData and its records (include/gamestate.h).
OFF = dict(playtime_frames=0x48, playtime=0x4C, money=0x6C, party=0x70, items=0x7C, items_equipped=0x20F, cards=0x3A2,
           cards_obtained=0x4DF, decks=0x628, digimon=0x75C, progress=0x263C, party_set=0x2640)
DIGIMON_SIZE, JOINED, RECORD = 0x3DC, 0x4, 0xC
REC = dict(exp=0x18, stats=0x1C, chosen_forms=0x48, forms=0x50, equipment=0x3C0)
FORM_SIZE, FORMS, STATS_SIZE = 0x14, 44, 0x2C
DECKS_SIZE = 3 * 0x66

# Items (src/main/records.c): 92 Short Sword (weapon, power 14, charisma 15), 215 Bandanna (armour, guard 4, charisma 10,
# bonus +10 Power), 292 Power Gem (accessory, +10 Power, charisma 3), 293 Power Ring (+20 Power, charisma 5),
# 294 Guard Gem (+10 Guard), 217 Baseball Cap (guard 10, charisma 20), 176 Claymore (slot 7: both hands; power 110,
# charisma 30, Boost -2), 1 Balanced Pack (key item, no data), 43 Power Charge (usable).
SWORD, BANDANNA, POWER_GEM, POWER_RING, GUARD_GEM, CAP, CLAYMORE, KEY_ITEM, USABLE = 92, 215, 292, 293, 294, 217, 176, 1, 43
ITEM_SET_0 = (243, 269, 98, 284)     # gamestate_item_sets[0]: Digimon 0's set (+4 to the six stats, +20 Charisma)

STAT_VALUES = [-5, 0, 50, 99, 100, 150, 999, 1000, 9999, 10000, 32767, -32768]
ADD_VALUES = [5, -1000, 100, 2000, 20000]
MONEY_VALUES = [0, 799, 800, 5000, 9990000, 9999999]


def rec(i, off=0):
    return OFF["digimon"] + i * DIGIMON_SIZE + RECORD + off


def stats_bytes(level, lv1, hp, max_hp, mp, max_mp, stats, resists, penalties=(0, 0, 0)):
    return struct.pack("<6h6h7h3h", level, lv1, hp, max_hp, mp, max_mp, *stats, *resists, *penalties)


def form(id_, level=1, exp=0, techniques=(0,) * 6):
    return struct.pack("<hbBi6h", id_, level, 0, exp, *techniques)


def u32(v):
    return (v & 0xFFFFFFFF).to_bytes(4, "little")


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_flags", 0, bytes(0x1C), "gamestate_flags = 0"),
         Write("gamestate_data", OFF["money"], u32(5000), "money = 5000"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 3, 5), "party = {0, 3, 5}"),
         Write("gamestate_data", OFF["party_set"], u32(0), "party_set = 0")]
    for d in (0, 1, 3, 5):
        w.append(Write("gamestate_data", OFF["digimon"] + d * DIGIMON_SIZE + JOINED, u32(d + 3), f"digimon[{d}].joined = {d + 3}"))
    # Records.
    w.append(Write("gamestate_data", rec(0, REC["exp"]), u32(4000), "digimon[0].record.exp = 4000"))
    w.append(Write("gamestate_data", rec(0, REC["stats"]), stats_bytes(20, 7, 300, 300, 100, 100, (80, 60, 70, 90, 75, 10), (100, 101, 102, 103, 104, 105, 106)),
                   "digimon[0].record.stats: level 20, HP 300/300, MP 100/100, stats {80,60,70,90,75,10}, resists 100..106"))
    w.append(Write("gamestate_data", rec(1, REC["stats"]), stats_bytes(5, 1, 180, 180, 10, 10, (56, 58, 19, 17, 49, 1), (80, 115, 105, 120, 85, 115, 60)),
                   "digimon[1].record.stats: level 5, its base stats"))
    w.append(Write("gamestate_data", rec(3, REC["stats"]), stats_bytes(25, 3, 350, 350, 120, 120, (95, 70, 60, 80, 90, 20), (120, 80, 100, 100, 60, 150, 90), (5, 10, 15)),
                   "digimon[3].record.stats: level 25, stats {95,70,60,80,90,20}, penalties {5, 10, 15}"))
    w.append(Write("gamestate_data", rec(5, REC["stats"]), stats_bytes(40, 9, 1000, 1000, 500, 500, (990, 995, 10, 20, 30, 985), (100,) * 7),
                   "digimon[5].record.stats: level 40, stats {990,995,10,20,30,985} (near the 999 cap)"))
    w.append(Write("gamestate_data", rec(3, REC["equipment"]), struct.pack("<6h", SWORD, BANDANNA, POWER_GEM, 0, 0, 0),
                   "digimon[3].record.equipment = {92 Short Sword, 215 Bandanna, 292 Power Gem, 0, 0, 0}"))
    w.append(Write("gamestate_data", rec(5, REC["equipment"]), struct.pack("<6h", CAP, 0, CLAYMORE, CLAYMORE, GUARD_GEM, POWER_RING),
                   "digimon[5].record.equipment = {217 Baseball Cap, 0, 176 Claymore, 176 Claymore, 294 Guard Gem, 293 Power Ring}"))
    w.append(Write("gamestate_data", rec(1, REC["equipment"]), struct.pack("<6h", 0, 0, 0, 0, 0, POWER_RING),
                   "digimon[1].record.equipment = {0, 0, 0, 0, 0, 293 Power Ring} (joined, not in the party)"))
    # Forms: Digimon 0 owns 0x17F (slot 0), 0x3 (1), an id 1 (2: below 3, never found but the slot is taken), 0x91 (5).
    forms0 = bytearray(FORMS * FORM_SIZE)
    forms0[0:FORM_SIZE] = form(0x17F, 20, 500, (1, 2, 3, 0, 0, 0))
    forms0[FORM_SIZE:2 * FORM_SIZE] = form(0x3, 5)
    forms0[2 * FORM_SIZE:3 * FORM_SIZE] = form(1, 1)
    forms0[5 * FORM_SIZE:6 * FORM_SIZE] = form(0x91, 9, 77)
    w.append(Write("gamestate_data", rec(0, REC["chosen_forms"]), struct.pack("<3h", 0x3, 0x17F, -1), "digimon[0].record.chosen_forms = {0x3, 0x17F, -1}"))
    w.append(Write("gamestate_data", rec(0, REC["forms"]), bytes(forms0), "digimon[0].record.forms: [0] 0x17F lv 20 exp 500 techs {1,2,3}, [1] 0x3 lv 5, [2] id 1, [5] 0x91 lv 9 exp 77"))
    w.append(Write("gamestate_data", rec(1, REC["forms"]), b"".join(form(100 + j, 1) for j in range(FORMS)), "digimon[1].record.forms: all 44 slots, ids 100..143"))
    w.append(Write("gamestate_data", rec(1, REC["chosen_forms"]), struct.pack("<3h", 100, 143, 200), "digimon[1].record.chosen_forms = {100, 143, 200 (not owned)}"))
    # Items: counts, and equipped-only items (92 on Digimon 3, 176 on Digimon 5, 293 on Digimon 1 and 5, 70 on nobody).
    for idx, val in ((10, 1), (50, 98), (51, 99), (60, 2), (1, 1), (2, 1), (6, 1), (43, 1), (47, 1), (93, 1), (94, 1), (216, 1), (295, 1), (300, 3)):
        w.append(Write("gamestate_data", OFF["items"] + idx, bytes([val]), f"items[{idx}] = {val}"))
    for idx, val in ((SWORD, 1), (CLAYMORE, 1), (POWER_RING, 2), (70, 1), (BANDANNA, 1)):
        w.append(Write("gamestate_data", OFF["items_equipped"] + idx, bytes([val]), f"items_equipped[{idx}] = {val}"))
    for idx, val in ((0, 3), (7, 9), (100, 7), (316, 1)):
        w.append(Write("gamestate_data", OFF["cards"] + idx, bytes([val]), f"cards[{idx}] = {val}"))
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C)]
# gamestate_data after the call, in two reads around playtime_frames (0x48): the vsync handler (gfx) adds 0x133 to it
# when a call spans a vblank (a level-up to 99 does), which is the game's timer, not the function.
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]


def read_record(i):
    return Read("gamestate_data", rec(i, REC["exp"]), 0x30, f"digimon[{i}].record.exp and .stats after the call")


def read_equipment(i):
    return Read("gamestate_data", rec(i, REC["equipment"]), 12, f"digimon[{i}].record.equipment after the call")


def read_forms(i, n=8):
    return Read("gamestate_data", rec(i, REC["chosen_forms"]), 8 + n * FORM_SIZE, f"digimon[{i}].record.chosen_forms and forms[0..{n}) after the call")


def read_item(idx):
    return [Read("gamestate_data", OFF["items"] + idx, 1, f"items[{idx}]"), Read("gamestate_data", OFF["items_equipped"] + idx, 1, f"items_equipped[{idx}]")]


READ_MONEY = Read("gamestate_data", OFF["money"], 4, "money after the call")
READ_PARTY = Read("gamestate_data", OFF["party"], 12, "party after the call")
READ_PLAYTIME = Read("gamestate_data", OFF["playtime_frames"], 12, "playtime_frames and playtime[4] after the call")


def case(name, calls, buffers=None, comment=""):
    return Case(name, calls, fixture=FIXTURE, buffers=buffers or {}, saves=SAVES, comment=comment)


def stat_write(i, stat, value, note=""):
    return Write("gamestate_data", rec(i, REC["stats"]) + 2 * stat, struct.pack("<h", value), f"digimon[{i}].record.stats.values[{stat}] = {value}{note}")


def cases(sym):
    out = []
    # --- stats
    for stat in list(range(19)) + [19, 20]:
        calls = [Call("gamestate_set_stat", [0, stat, v], "void", [read_record(0)] + AFTER, comment=f"set_stat(0, {stat}, {v})") for v in STAT_VALUES]
        out.append(case(f"set_stat_{stat}", calls, comment="stat 0-1 cap 99, 2-5 cap 9999, 6-18 cap 999, negative -> 0; 19, 20 ignored"))
    for stat in list(range(19)) + [19]:
        calls = [Call("gamestate_add_stat", [0, stat, v], "void", [read_record(0)] + AFTER, comment=f"add_stat(0, {stat}, {v}) to the fixture's value")
                 for v in ADD_VALUES]
        for base, v in ((32000, 1000), (32000, 767), (32000, 768), (-30000, -5000), (0, 65536), (0, 65535 + 50)):
            calls.append(Call("gamestate_add_stat", [0, stat, v], "void", [read_record(0)] + AFTER,
                              writes=[stat_write(0, stat, base, " (before the add)")], comment=f"add_stat(0, {stat}, {v}) on {base}: the s16 sum wraps"))
        out.append(case(f"add_stat_{stat}", calls, comment="each add starts from the fixture's value (the case does not restore between calls); the last six set the value first"))
    base_stats = stats_bytes(20, 7, 300, 300, 100, 100, (80, 60, 70, 90, 75, 10), (100, 101, 102, 103, 104, 105, 106))
    for t in range(16):
        calls = []
        for v in (10, 990, -50):
            calls.append(Call("gamestate_add_stat_bonus", [("buf", "stats"), t, v], "void", [Read("buf:stats", 0, STATS_SIZE, "the stats buffer after the call")],
                              comment=f"add_stat_bonus(stats, {t}, {v}) (the buffer accumulates across the three calls)"))
        out.append(case(f"add_stat_bonus_{t}", calls, buffers={"stats": base_stats},
                        comment="type 7: all six stats; 1-6 one stat; 8-14 one resist; 0 and 15 nothing; cap 999 (no floor)"))
    # --- get_stats
    out_read = Read("buf:out", 0, STATS_SIZE, "the GamestateStats written")
    get = [(0, "no equipment", []), (3, "sword + bandanna + gem, penalties 5/10/15", []),
           (5, "cap 990 + cap + claymore x2 (both hand slots count) + guard gem + power ring; stats near 999", []),
           (1, "a Power Ring only", []), (7, "an empty record", []),
           (0, "the item set of Digimon 0 (243, 269, 98, 284): +4 x6, +20 charisma", [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", *ITEM_SET_0, 0, 0), "digimon[0].record.equipment = its item set")]),
           (0, "the set with slot 3 different: no set bonus", [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", 243, 269, 98, 285, 0, 0), "digimon[0].record.equipment = {243, 269, 98, 285}")]),
           (0, "the set in slots 1..4 (not 0..3): no set bonus", [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", 0, *ITEM_SET_0, 0), "digimon[0].record.equipment = {0, 243, 269, 98, 284, 0}")]),
           (3, "Digimon 3 wearing Digimon 0's set: no bonus (sets are per Digimon)", [Write("gamestate_data", rec(3, REC["equipment"]), struct.pack("<6h", *ITEM_SET_0, 0, 0), "digimon[3].record.equipment = Digimon 0's set")]),
           (0, "a key item and a usable item in slots: skipped (no data)", [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", KEY_ITEM, USABLE, 0, 0, 0, 0), "digimon[0].record.equipment = {1 Balanced Pack, 43 Power Charge}")]),
           (0, "penalties larger than the stats: floor 0", [Write("gamestate_data", rec(0, REC["stats"]) + 0x26, struct.pack("<3h", 200, 300, 400), "digimon[0].record.stats.penalties = {200, 300, 400}")]),
           (0, "a negative equipment id is skipped", [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", -1, SWORD, 0, 0, 0, 0), "digimon[0].record.equipment = {-1, 92}")])]
    for n, (i, desc, writes) in enumerate(get):
        out.append(case(f"get_stats_{n}_digimon{i}", [Call("gamestate_get_stats", [i, ("buf", "out")], "void", [out_read] + AFTER, writes=writes, comment=desc)],
                        buffers={"out": bytes(STATS_SIZE)}, comment=desc))
    # --- money
    for money in MONEY_VALUES:
        calls = []
        for t, n in ((0, 10), (1, 8), (2, 10)):
            for i in range(n):
                calls.append(Call("gamestate_change_money", [t, i], "s32", [READ_MONEY] + AFTER,
                                  writes=[Write("gamestate_data", OFF["money"], u32(money), f"money = {money}")],
                                  comment=f"type {t} ({'check' if t == 0 else 'add' if t == 1 else 'subtract'}) index {i} on money {money}"))
        out.append(case(f"change_money_{money}", calls, comment="type 0: money >= gamestate_money_check[i]; 1: += add[i] (cap 9,999,999); 2: -= sub[i] (floor 0)"))
    money_conds = [(cid, ct, arg) for cid, ct, arg in conditions() if ct & 0xF0 == 0x20]
    for money in (0, 5000, 9999999):
        calls = []
        for cid, ct, arg in money_conds:
            for v in (0, 1):
                calls.append(Call("gamestate_check_condition", [cid, v], "s32", [READ_MONEY] + AFTER,
                                  writes=[Write("gamestate_data", OFF["money"], u32(money), f"money = {money}")],
                                  comment=f"id {cid:#04x}: type {ct:#04x} arg {arg}; value {v}; money {money}"))
        out.append(case(f"check_condition_money_{money}", calls, comment="the type 0x20 conditions (gamestate_change_money): sub 0 checks, 1 adds (ret 0), 2 subtracts (ret 0)"))
    # --- items
    items = [("add_from_0", 10, 1), ("add_from_1", 10, 1), ("add_from_98", 50, 1), ("add_from_99", 51, 1), ("add_unowned", 200, 1),
             ("remove_from_2", 60, 0), ("remove_from_1", 10, 0), ("remove_none", 200, 0),
             ("remove_equipped_party", SWORD, 0), ("remove_equipped_both_hands", CLAYMORE, 0), ("remove_equipped_ring", POWER_RING, 0),
             ("remove_equipped_nowhere", 70, 0), ("remove_equipped_armour", BANDANNA, 0)]
    for name, idx, add in items:
        reads = read_item(idx) + [read_equipment(1), read_equipment(3), read_equipment(5)] + AFTER
        calls = [Call("gamestate_change_item", [idx, add], "void", reads, comment=f"change_item({idx}, {add})")]
        if name == "add_from_1":
            calls.insert(0, Call("gamestate_change_item", [idx, 1], "void", reads, comment="first add"))
        out.append(case(f"change_item_{name}", calls,
                        comment="add: ++count, 100 -> 99; remove: --count (floor 0), else if equipped: unequip from a party member, else from any joined Digimon, then --equipped"))
    # Removing 293 twice: Digimon 5 (party) first, then Digimon 1.
    out.append(case("change_item_remove_equipped_twice", [
        Call("gamestate_change_item", [POWER_RING, 0], "void", read_item(POWER_RING) + [read_equipment(1), read_equipment(5)] + AFTER, comment="first: off party member 5"),
        Call("gamestate_change_item", [POWER_RING, 0], "void", read_item(POWER_RING) + [read_equipment(1), read_equipment(5)] + AFTER, comment="second: off Digimon 1 (not in the party)"),
        Call("gamestate_change_item", [POWER_RING, 0], "void", read_item(POWER_RING) + [read_equipment(1), read_equipment(5)] + AFTER, comment="third: nothing left")]))
    for name, i, item in (("found", 3, SWORD), ("both_hands", 5, CLAYMORE), ("second_slot", 5, POWER_RING), ("absent", 3, CAP), ("empty_record", 7, SWORD)):
        out.append(case(f"unequip_{name}", [Call("gamestate_unequip_item", [i, item], "s32", [read_equipment(i)] + AFTER, comment=f"unequip_item({i}, {item})")]))
    # --- cards
    def read_card(idx):
        return [Read("gamestate_data", OFF["cards"] + idx, 1, f"cards[{idx}]"), Read("gamestate_data", OFF["cards_obtained"] + idx, 1, f"cards_obtained[{idx}]")]
    for name, idx, add in (("remove_from_3", 0, 0), ("remove_from_0", 5, 0), ("remove_from_1", 316, 0), ("add_from_0", 5, 1), ("add_from_3", 0, 1), ("add_from_9", 7, 1)):
        out.append(case(f"change_card_{name}", [Call("gamestate_change_card", [idx, add], "void", read_card(idx) + AFTER, comment=f"change_card({idx}, {add})")]))
    for name, idx, n in (("one", 5, 1), ("three", 100, 3), ("to_cap", 7, 1), ("zero", 5, 0), ("negative", 0, -2)):
        out.append(case(f"add_card_{name}", [Call("gamestate_add_card", [idx, n], "void", read_card(idx) + AFTER, comment=f"add_card({idx}, {n}): obtained = 1, count += n, 10 -> 9")]))
    out.append(case("init_cards", [Call("gamestate_init_cards", [], "void", [
        Read("gamestate_data", OFF["cards"], 0x13D, "cards[] after the call (SHA-1)"),
        Read("gamestate_data", OFF["cards_obtained"], 0x13D, "cards_obtained[] after the call (SHA-1)"),
        Read("gamestate_data", OFF["decks"], DECKS_SIZE, "decks[3] after the call (SHA-1)")] + AFTER,
        comment="the start deck added to the fixture's cards (cards[0] 3 -> ...), the three decks filled")]))
    # --- forms
    for name, i, fid in (("present", 0, 0x3), ("present_slot5", 0, 0x91), ("absent", 0, 0x100), ("id_below_3", 0, 1), ("zero", 0, 0), ("last_of_44", 1, 143), ("empty_record", 7, 0x3)):
        out.append(case(f"find_form_{name}", [Call("gamestate_find_form", [i, fid], "s32", comment=f"find_form({i}, {fid})")]))
    for name, i, fid in (("new", 0, 0x100), ("duplicate", 0, 0x3), ("duplicate_slot5", 0, 0x91), ("id_1_again", 0, 1), ("full", 1, 0x100), ("empty_record", 7, 0x17F)):
        out.append(case(f"add_form_{name}", [Call("gamestate_add_form", [i, fid], "s32", [read_forms(i)] + AFTER, comment=f"add_form({i}, {fid}): first slot with id 0; returns the new form's level 1, else 0")]))
    out.append(case("add_form_fill", [Call("gamestate_add_form", [0, 0x200 + k], "s32", [read_forms(0, 44)] + AFTER, comment=f"add_form(0, {0x200 + k:#x})") for k in range(42)],
                    comment="42 adds on a record with 4 slots taken fill it: the last two return 0"))
    for name, i in (("digimon0", 0), ("digimon1_full", 1), ("empty", 7)):
        out.append(case(f"list_forms_{name}", [Call("gamestate_list_forms", [i, ("buf", "list")], "s32", [Read("buf:list", 0, 88, "the 44 ids written")] + AFTER, comment=f"list_forms({i}, out): ids >= 3, the rest 0")],
                        buffers={"list": b"\xee" * 88}))
    for name, i in (("digimon0", 0), ("digimon1", 1), ("empty", 7)):
        out.append(case(f"get_chosen_forms_{name}", [Call("gamestate_get_chosen_forms", [i, ("buf", "ids")], "s32", [Read("buf:ids", 0, 6, "the 3 ids written")] + AFTER, comment=f"get_chosen_forms({i}, out): owned chosen forms, then -1")],
                        buffers={"ids": b"\xee" * 6}))
    for name, i, ids in (("owned", 0, (0x91, 0x3, 0x17F)), ("one_absent", 0, (0x3, 0x100, 0x17F)), ("id_1", 0, (1, 0, 0x3)), ("empty", 7, (0x3, 0x3, 0x3))):
        out.append(case(f"set_chosen_forms_{name}", [Call("gamestate_set_chosen_forms", [i, ("buf", "ids")], "void", [read_forms(i, 1)] + AFTER, comment=f"set_chosen_forms({i}, {ids}): -1 for an id not owned")],
                        buffers={"ids": struct.pack("<3h", *ids)}))
    for name, i, fid in (("present", 0, 0x17F), ("absent", 0, 0x100)):
        out.append(case(f"get_form_{name}", [Call("gamestate_get_form", [i, fid, ("buf", "form")], "s32", [Read("buf:form", 0, FORM_SIZE, "the form copied out")] + AFTER, comment=f"get_form({i}, {fid}, out): the slot index, out untouched when -1")],
                        buffers={"form": b"\xee" * FORM_SIZE}))
    new_form = form(0x3, 30, 12345, (7, 8, 9, 10, 11, 12))
    for name, i, fid in (("present", 0, 0x3), ("absent", 0, 0x100)):
        out.append(case(f"put_form_{name}", [Call("gamestate_put_form", [i, fid, ("buf", "form")], "s32", [read_forms(i)] + AFTER, comment=f"put_form({i}, {fid}, in): copies in over the slot")],
                        buffers={"form": new_form}))
    # --- party
    out.append(case("get_party_member", [Call("gamestate_get_party_member", [i], "s32", comment=f"get_party_member({i}): party[i], -1 for i >= 3") for i in (0, 1, 2, 3, 0xFFFFFFFF)]))
    out.append(case("get_party_digimon", [Call("gamestate_get_party_digimon", [i], "s32", comment=f"get_party_digimon({i}): digimon[party[i]].joined - 3") for i in (0, 1, 2, 3)]))
    for p in (0, 1, 2):
        out.append(case(f"set_party_{p}", [Call("gamestate_set_party", [p], "void", [READ_PARTY, Read("gamestate_data", OFF["party_set"], 4, "party_set")] +
                                                 [Read("gamestate_data", OFF["digimon"] + d * DIGIMON_SIZE + JOINED, 4, f"digimon[{d}].joined") for d in range(8)] + AFTER,
                                                 comment=f"set_party({p}): gamestate_parties[{p}], its members joined")]))
    # --- playtime
    def pt(frames, h, m, s, maxed=0):
        return [Write("gamestate_data", OFF["playtime_frames"], u32(frames), f"playtime_frames = {frames:#x}"),
                Write("gamestate_data", OFF["playtime"], struct.pack("<4h", h, m, s, maxed), f"playtime = {{{h}, {m}, {s}, {maxed}}}")]
    ticks = [("under_a_second", pt(59 << 8 | 0xFF, 0, 0, 0)), ("second", pt(60 << 8 | 0x12, 0, 0, 0)), ("second_61", pt(61 << 8 | 0x34, 0, 0, 58)),
             ("minute", pt(60 << 8, 0, 0, 59)), ("hour", pt(60 << 8, 0, 59, 59)), ("max", pt(60 << 8, 999, 59, 59)), ("past_max", pt(60 << 8, 999, 59, 59, 1)),
             ("large_frames", pt(0x7FFFFFFF, 1, 2, 3))]
    for name, writes in ticks:
        out.append(case(f"tick_playtime_{name}", [Call("gamestate_tick_playtime", [], "void", [READ_PLAYTIME] + AFTER, writes=writes, comment="tick_playtime")]))
    out.append(case("reset_playtime", [Call("gamestate_reset_playtime", [], "void", [READ_PLAYTIME] + AFTER, writes=pt(0x1234, 5, 6, 7, 1))]))
    # --- item lists (records_list_items reads gamestate_data.items / items_equipped)
    for t in list(range(6)) + [0x80000000 | x for x in range(6)]:
        out.append(case(f"list_items_{t:#x}", [Call("records_list_items", [t, ("buf", "out")], "s32", [Read("buf:out", 0, 256, "the ids written (u16)")] + AFTER,
                                                     comment=f"list_items({t:#x}, out): list {t & 0x7FFFFFFF}{' owned or equipped' if t >> 31 else ' owned'}; >= 5 returns 0")],
                        buffers={"out": bytes(256)}))
    return out
