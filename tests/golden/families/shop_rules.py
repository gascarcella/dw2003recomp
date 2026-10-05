"""The shops' rules that live in pure helpers: STITSHOP (the item shop, src/stitshop/stitshop_800859C0.c, its
stitshop_funcs table) and STCRDSHP (the card shop, src/stcrdshp/stcrdshp_80088E24.c, stcrdshp_stock).
docs/MECHANICS.md section 9.

STITSHOP:
- stitshop_get_items(shop): shop `shop`'s goods (stitshop_shops, 31 shops 0..30); sets stitshop_funcs.count. shop < 0
  or a NULL list: NULL, count untouched. The table is not bounds-checked: shop 31 reads stitshop_funcs itself (it follows
  the table) as a shop, so the count is set to itself.
- stitshop_can_equip(digimon, item): bit `digimon` of the item's RecordsEquip.members.
- stitshop_get_slot(digimon, item): the equipment slot by the item's slot kind: 4 head -> 0, 5 body -> 1, 2 shield -> 3,
  7 two hands -> 2; 3 one hand: slot 2 if empty, else 3 if empty, else 2 if slot 2 holds a two-handed weapon or slot 3 a
  shield (type 20), -1 if either is not a weapon, 2 if slot 2's weapon is weaker (power), else 3 (equal too: the code
  falls through to case 2); 6 ring: 4 if empty, else 5 if empty, else the weaker bonus_value of the two (4 on a tie
  goes to 5); 8 crest: 4/5 if empty, else the slot holding a crest of the same group (4 first), else the weaker.
- stitshop_equip(digimon, slot, item, take): puts the item in, taking out what was there (a two-handed weapon empties
  both hand slots; a two-handed item also takes out the other hand's item; a crest takes out crests of its group); with
  `take` every item taken out goes back to the bag (items_equipped--, items++) and the new one leaves it. Item 0 only
  takes out.
- stitshop_info_get_stats(i, out): the stats shown for "with the item": as gamestate_get_stats but without the item-set
  bonus (gamestate_item_sets); stitshop_info_add_stat is gamestate_add_stat_bonus (type 7 all six stats, 1..6 a stat,
  8..14 a resist, each capped at 999, no floor).
STCRDSHP:
- stcrdshp_get_price(card): its price in stcrdshp_prices (44 cards), 1 if absent (also card 0, the terminator).
- stcrdshp_find_shop(id): the shop with that ID (49..67, 70..74), shop 0 if none; counts its cards (up to the first 0)
  into its count.

Not here (UI state machines, docs/MECHANICS.md section 9 [C]): the buy/sell counts and the money (stitshop_buy_run,
stitshop_sell_run, stcrdshp_run_buy) and the booster draw (stcrdshp_run_booster, still asm).

Fixture: pointer-free gamestate_data zeroed, Digimon 0's record (level 20, stats 60, Charisma 10, no equipment), bag
counts 3 for the items used. Each overlay goes in the slot by a kept setup case (STITSHOP first, then STCRDSHP).

Hand checks: shop 18 (the largest regular one) has 32 goods, shop 30 all 52; the Short Sword (92, power 14) in slot 2
and the Zanden Sword (93, power 26) in slot 3: a third sword goes to slot 2. Ronin set (243, 269, 98, 284) on Digimon
0: Power 60 + 120 = 180 and no +4 (gamestate_get_stats gives 184). Card 235 costs 13,000, card 1 (not sold) 1.
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import OFF, REC, rec, stats_bytes, u32
from oracle import Call, Case, Read, Write

COMMENT = ("STITSHOP.PRO then STCRDSHP.PRO in the slot. stitshop_get_items (counts, a negative shop, shop 31 past the "
           "table), stitshop_can_equip, stitshop_get_slot for every slot kind and every branch of the hand, ring and crest "
           "choices, stitshop_equip (bag counts with and without take, two-handed weapons, crest groups, unequip), "
           "stitshop_info_get_stats (no set bonus, caps, penalties), stitshop_info_add_stat; stcrdshp_get_price (table "
           "edges, absent cards), stcrdshp_find_shop (first, last, unknown ID -> shop 0, the count).")

OVERLAY_SLOT = 0x80082CB0
STITSHOP_PRO = "extracted/disc/AAA/PRO/STITSHOP.PRO"
STCRDSHP_PRO = "extracted/disc/AAA/PRO/STCRDSHP.PRO"
ITEMS_EQUIPPED = 0x20F
BASE_STATS, BASE_RESISTS = (60, 60, 60, 60, 60, 10), (100,) * 7
# Items (src/main/records.c): weapons 92 Short Sword (power 14), 93 Zanden Sword (26), 98 Ronin Blade (120), 176
# Claymore (two hands, power 110, members 0x39), 205 Mega Guard (members 0xB8); armour 215 Bandanna (head), 243 Ronin
# Helmet, 269 Ronin Armor (body), 275 Buckler (shield, type 20), 284 Ronin Shield; rings 292 Power Gem (+10), 293
# Power Ring (+20), 299 Wisdom Ring; crests 317 (group 2, 32), 318 (group 3, 64), 325 (group 9, 10), 326 (group 9, 20).
ITEMS = (43, 92, 93, 98, 176, 205, 215, 243, 269, 275, 284, 292, 293, 299, 317, 318, 325, 326)


def stats_write(d, stats=BASE_STATS, penalties=(0, 0, 0)):
    return Write("gamestate_data", rec(d, REC["stats"]), stats_bytes(20, 50, 300, 500, 100, 200, stats, BASE_RESISTS, penalties),
                 f"digimon[{d}].record.stats: level 20, stats {stats}, resists 100, penalties {penalties}")


def equip_write(d, equipment):
    return Write("gamestate_data", rec(d, REC["equipment"]), struct.pack("<6h", *equipment), f"digimon[{d}].record.equipment = {equipment}")


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 1, 7), "party = {0, 1, 7}"),
         stats_write(0)]
    for i in ITEMS:
        w.append(Write("gamestate_data", OFF["items"] + i, bytes([3]), f"items[{i}] = 3"))
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("stitshop_funcs", 4)]
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]


def get_items_cases():
    out = []
    for shop, desc in ((0, "shop 0: 11 goods"), (4, "shop 4: 2 goods"), (18, "shop 18: 32 goods"), (30, "shop 30: all 52"),
                       (-1, "shop -1: NULL, count untouched (77)"),
                       (31, "shop 31, past the 31-entry table: stitshop_funcs read as a shop (count = itself, 77)")):
        out.append(Case(f"get_items_{shop & 0xFF}", [Call("stitshop_get_items", [shop], "void",
                                                          [Read("stitshop_funcs", 0, 4, "stitshop_funcs.count after the call")],
                                                          writes=[Write("stitshop_funcs", 0, u32(77), "stitshop_funcs.count = 77")], comment=desc)],
                        fixture=FIXTURE, saves=SAVES, comment="the returned list is not compared (an address); the count is"))
    return out


def can_equip_cases():
    calls = [Call("stitshop_can_equip", [d, item], "s32", comment=desc) for d, item, desc in (
        (0, 92, "Short Sword (members 0x01), Digimon 0 -> 1"), (1, 92, "Digimon 1 -> 0"),
        (3, 176, "Claymore (members 0x39), Digimon 3 -> 1"), (1, 176, "Digimon 1 -> 0"), (7, 205, "Mega Guard (0xB8), Digimon 7 -> 1"),
        (8, 215, "Bandanna (0xFF), Digimon 8 (no such bit) -> 0"))]
    return [Case("can_equip", calls, fixture=FIXTURE, saves=SAVES, comment="RecordsEquip.members >> digimon & 1")]


def slot_case(name, item, equipment, desc):
    return Case(name, [Call("stitshop_get_slot", [0, item], "s32", writes=[equip_write(0, equipment)], comment=desc)],
                fixture=FIXTURE, saves=SAVES, comment="Digimon 0")


def get_slot_cases():
    E = (0, 0, 0, 0, 0, 0)

    def eq(**kw):
        e = list(E)
        for k, v in kw.items():
            e[int(k[1:])] = v
        return tuple(e)
    return [
        slot_case("slot_head", 215, E, "Bandanna (head) -> 0"),
        slot_case("slot_body", 269, E, "Ronin Armor (body) -> 1"),
        slot_case("slot_shield", 275, E, "Buckler (shield) -> 3"),
        slot_case("slot_two_hands", 176, eq(s2=92, s3=93), "Claymore (two hands) -> 2 whatever is held"),
        slot_case("slot_hand_empty", 92, E, "sword, hands empty -> 2"),
        slot_case("slot_hand_second", 92, eq(s2=93), "sword, slot 2 held -> 3"),
        slot_case("slot_hand_two_handed", 92, eq(s2=176, s3=176), "slot 2 holds a two-handed weapon -> 2"),
        slot_case("slot_hand_shield", 93, eq(s2=92, s3=275), "slot 3 holds a shield (type 20) -> 2"),
        slot_case("slot_hand_weaker2", 98, eq(s2=92, s3=93), "Short Sword (14) in 2, Zanden (26) in 3: 2 is weaker -> 2"),
        slot_case("slot_hand_weaker3", 98, eq(s2=93, s3=92), "Zanden in 2, Short Sword in 3 -> 3"),
        slot_case("slot_hand_equal", 98, eq(s2=92, s3=92), "equal power -> 3 (falls through to the shield case)"),
        slot_case("slot_hand_not_weapon", 92, eq(s2=215, s3=92), "slot 2 holds armour (a hand-made state): -> -1"),
        slot_case("slot_ring_empty", 292, E, "ring, both empty -> 4"),
        slot_case("slot_ring_second", 292, eq(s4=293), "ring, slot 4 held -> 5"),
        slot_case("slot_ring_weaker4", 292, eq(s4=292, s5=293), "Power Gem (10) in 4, Power Ring (20) in 5 -> 4"),
        slot_case("slot_ring_weaker5", 292, eq(s4=293, s5=292), "Ring (20) in 4, Gem (10) in 5 -> 5"),
        slot_case("slot_ring_equal", 293, eq(s4=292, s5=292), "equal bonus -> 5"),
        slot_case("slot_ring_vs_crest", 293, eq(s4=292, s5=318), "Gem (10) vs a crest's bonus_value (64): compared alike -> 4"),
        slot_case("slot_crest_empty", 325, E, "crest, both empty -> 4"),
        slot_case("slot_crest_second", 325, eq(s4=317), "crest, slot 4 held -> 5"),
        slot_case("slot_crest_group4", 325, eq(s4=326, s5=317), "slot 4 holds a crest of its group (9) -> 4"),
        slot_case("slot_crest_group5", 325, eq(s4=317, s5=326), "slot 5 holds a crest of its group -> 5"),
        slot_case("slot_crest_weaker4", 325, eq(s4=317, s5=318), "no group match: 32 in 4 < 64 in 5 -> 4"),
        slot_case("slot_crest_weaker5", 325, eq(s4=318, s5=317), "no group match: 64 in 4, 32 in 5 -> 5"),
    ]


def equip_reads(items):
    reads = [Read("gamestate_data", rec(0, REC["equipment"]), 12, "digimon[0].record.equipment after the call")]
    for i in items:
        reads.append(Read("gamestate_data", OFF["items"] + i, 1, f"items[{i}]"))
        reads.append(Read("gamestate_data", ITEMS_EQUIPPED + i, 1, f"items_equipped[{i}]"))
    return reads + AFTER


def equip_case(name, slot, item, take, equipment, involved, desc):
    writes = [equip_write(0, equipment)]
    for i in set(equipment) - {0}:
        writes.append(Write("gamestate_data", ITEMS_EQUIPPED + i, bytes([1]), f"items_equipped[{i}] = 1"))
    return Case(name, [Call("stitshop_equip", [0, slot, item, take], "void", equip_reads(involved), writes=writes, comment=desc)],
                fixture=FIXTURE, saves=SAVES, comment=f"Digimon 0, equipment {equipment}; bag 3 of each")


def equip_cases():
    return [
        equip_case("equip_empty", 0, 215, 1, (0,) * 6, (215,), "Bandanna into empty slot 0: bag 3 -> 2, equipped 0 -> 1"),
        equip_case("equip_replace", 0, 243, 1, (215, 0, 0, 0, 0, 0), (215, 243), "Ronin Helmet over the Bandanna: Bandanna back to the bag"),
        equip_case("equip_no_take", 0, 243, 0, (215, 0, 0, 0, 0, 0), (215, 243), "take 0: the slot changes, no count moves"),
        equip_case("equip_two_hands", 2, 176, 1, (0, 0, 92, 275, 0, 0), (92, 275, 176),
                   "Claymore into slot 2 over sword + Buckler: both back, Claymore in 2 and 3, one equipped"),
        equip_case("equip_over_two_hands", 2, 93, 1, (0, 0, 176, 176, 0, 0), (176, 93),
                   "a sword into slot 2 over the Claymore: both hand slots emptied, Claymore back once"),
        equip_case("equip_crest_group", 5, 325, 1, (0, 0, 0, 0, 326, 317), (326, 317, 325),
                   "crest 325 (group 9) into slot 5: 317 out of 5, 326 (group 9) out of 4"),
        equip_case("equip_ring_no_group", 4, 293, 1, (0, 0, 0, 0, 0, 292), (292, 293), "rings have no group rule: the Power Gem in 5 stays"),
        equip_case("equip_remove", 1, 0, 1, (0, 269, 0, 0, 0, 0), (269,), "item 0 into slot 1: Ronin Armor back to the bag"),
    ]


def get_stats_case(name, writes, desc, d=0):
    return Case(name, [Call("stitshop_info_get_stats", [d, ("buf", "out")], "void",
                            [Read("buf:out", 0, 0x2C, "out (GamestateStats)")], writes=writes, comment=desc)],
                fixture=FIXTURE, saves=SAVES, buffers={"out": bytes(0x2C)}, comment=f"Digimon {d}")


def get_stats_cases():
    return [
        get_stats_case("stats_ronin_set", [equip_write(0, (243, 269, 98, 284, 0, 0))],
                       "the Ronin set (gamestate_item_sets[0]): Power 60 + 120 = 180, Guard + 28 + 38 + 30; no set bonus"),
        get_stats_case("stats_caps_penalties", [stats_write(0, stats=(950, 60, 60, 60, 60, 990), penalties=(5, 100, 70)),
                                                 equip_write(0, (0, 0, 98, 0, 0, 0))],
                       "Power 950 + 120 -> 999, then - 5 = 994; Charisma 990 + 55 -> 999; Guard 60 - 100 -> 0, Boost 60 - 70 -> 0"),
        get_stats_case("stats_accessories", [equip_write(0, (0, 0, 0, 0, 292, 43))],
                       "Power Gem: Power + 10 (bonus pair), Charisma + 3; a usable item (43) in slot 5 adds nothing"),
    ]


def add_stat_cases():
    vals = struct.pack("<19h", 20, 50, 300, 500, 100, 200, 60, 60, 60, 60, 997, 10, *(100,) * 7)
    calls = [Call("stitshop_info_add_stat", [("buf", "stats"), t, v], "void", [Read("buf:stats", 0, 0x26, "values after the call")], comment=desc)
             for t, v, desc in ((7, 5, "type 7: the six stats + 5, Boost 997 -> 999"), (1, -100, "type 1: Power - 100 -> -35 (no floor)"),
                                (6, 1000, "type 6: Charisma + 1000 -> 999"), (8, 3, "type 8: Fire + 3"), (14, 3, "type 14: Dark + 3"),
                                (0, 50, "type 0: nothing"), (15, 50, "type 15: nothing"))]
    return [Case("add_stat", calls, fixture=FIXTURE, saves=SAVES, buffers={"stats": vals},
                 comment="GamestateStats.values: Power..Charisma 60, 60, 60, 60, 997, 10; the calls add up")]


def price_cases():
    calls = [Call("stcrdshp_get_price", [card], "s16", comment=desc) for card, desc in (
        (10, "first entry -> 2200"), (285, "last entry -> 9000"), (235, "the dearest -> 13000"), (96, "the cheapest -> 500"),
        (1, "not sold -> 1"), (0, "card 0 (the terminator) -> 1"), (314, "the last card ID, not sold -> 1"))]
    return [Case("card_prices", calls, fixture=FIXTURE, saves=SAVES, comment="stcrdshp_prices")]


SHOP_SAVES = SAVES + [("stcrdshp_shops", 25 * 0xC), ("stcrdshp_stock_0", 14), ("stcrdshp_stock_23", 14)]


def find_shop_case(name, id_, entry, writes, desc):
    return Case(name, [Call("stcrdshp_find_shop", [id_], "void", [Read("stcrdshp_shops", entry * 0xC + 4, 4, f"stcrdshp_shops[{entry}].count")],
                            writes=writes, comment=desc)],
                fixture=FIXTURE, saves=SHOP_SAVES, comment="the returned pointer is not compared; the shop's count is")


def find_shop_cases():
    cut0 = Write("stcrdshp_stock_0", 4, b"\0\0", "stcrdshp_stock_0[2] = 0 (shop 49 now lists 2 cards)")
    cut23 = Write("stcrdshp_stock_23", 6, b"\0\0", "stcrdshp_stock_23[3] = 0 (shop 74 now lists 3)")
    return [
        find_shop_case("find_shop_first", 49, 0, [], "shop 49 = entry 0: 6 cards"),
        find_shop_case("find_shop_last", 74, 23, [cut23], "shop 74 = entry 23, list cut to 3: count 3"),
        find_shop_case("find_shop_unknown", 68, 0, [cut0], "no shop 68 (67 and 70 exist): entry 0, count 2"),
    ]


def setup_case(name, pro, smoke):
    return Case(name, [smoke], fixture=[Write(OVERLAY_SLOT, 0, b"", f"overlay slot: {pro.rsplit('/', 1)[1]}", file=pro)],
                keep=True, comment="setup, kept for what follows: the overlay in the slot; nothing is restored")


def cases(sym):
    return ([setup_case("setup_stitshop", STITSHOP_PRO, Call("stitshop_can_equip", [0, 92], "void", comment="smoke: the overlay answers"))]
            + get_items_cases() + can_equip_cases() + get_slot_cases() + equip_cases() + get_stats_cases() + add_stat_cases()
            + [setup_case("setup_stcrdshp", STCRDSHP_PRO, Call("stcrdshp_get_price", [10], "void", comment="smoke: the overlay answers"))]
            + price_cases() + find_shop_cases())
