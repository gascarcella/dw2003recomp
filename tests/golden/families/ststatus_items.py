"""Items and healing techniques used from the status menu (STSTATUS, src/ststatus/ststatus_8008E94C.c and
ststatus_800937A4.c). docs/MECHANICS.md sections 8 and 9.

ststatus_items_use(page, windows) uses item page.item (0x78) on party member page.member (0x3B0), by its RecordsUsable
effect (records_items[item - 1].data):
- 1 (the Charges 43-46, amount 500/2000/5000/9999): only if HP < max HP; HP += amount, at most max HP. HP 0 is not
  special (there is no knockout outside battle).
- 17 (the Train Chips 62-65, +1/+2/+3/+5): only if values[1] (the TP) < 99; += amount, a result >= 100 becomes 99.
- 2..16 (the Chips 47-61, amount 30 for HP/MP, else 10): ststatus_stat_raises maps the effect to a values[] index and a
  cap (2 max HP and 3 max MP: 9999; 4..9 Power..Charisma, 10..16 the seven resists: 999); only if the stat is below the
  cap: one draw, += next() % amount + 1, at most the cap. The HP Chip raises max HP, not HP.
- anything else (0: the battle items, the boosters): nothing.
When it did something the bag loses one (gamestate_data.items[item]--, an s8 with no floor), else nothing changes.
The windows and the sound are the page's UI: the goldens give every window method an empty function (heap_nop) and
read what the function wrote to gamestate_data and the RNG index.

The techniques page heals with the forms' healing techniques 0xB8..0xBC:
- ststatus_calc_heal(digimon, tech) = power * 64 + power * Wisdom / 8, power = records_techniques[tech - 1].effect_power,
  Wisdom from gamestate_get_stats (equipment included).
- ststatus_tech_use(page, windows, sound): target 3 (0xB8-0xBA) heals page.target if its HP < max HP (capped) and
  takes the MP cost from the user, else fails; target 4 (0xBB, 0xBC) heals every party member below max HP (party
  members -1 skipped), takes the cost once if any was healed, and fails if all were full. The MP is not checked here
  (ststatus_tech_get_usable does: the technique if cost <= MP, else 0), so it can go negative.
- ststatus_tech_collect(page, member): the healing techniques (id & 0x1FFF in 0xB8..0xBC) of the member's chosen forms
  with an ID >= 3, in form order, each once, at most 5; the dedup compares with all five slots of the page's list,
  which it never clears (a stale entry past the new count hides that technique).
The equipment page (ststatus_80099B6C.c): ststatus_can_equip(digimon, slot, item) = the members bit, and a shield (kind
2) not in slot 2 (kind 1 not in slot 3; no item has kind 1); ststatus_list_items(category, out): 5 the owned weapons and
armour of kinds 1, 2, 3, 7 (hands), 6 of kind 4 (head), 7 of kind 5 (body); ststatus_equip_item is stitshop_equip with
the bag counts always moved (shop_rules has every branch).

STSTATUS.PRO goes in the overlay slot and the menu text file (0xB0 + records_language = 0xB2, ESSTATUS.BIN) in RAM as a
loaded cdload entry (a kept setup case): the window methods are stubs, but cdload_module.files.get_file(0xB2) is still
called and would otherwise start a CD read. Fixture: pointer-free gamestate_data zeroed, party {0, 1, 7},
records_language 2, the records of Digimon 0, 1 and 7, five of every item used.

Hand checks: Power Charge (500) on HP 300/1000 -> 800; Super Charge (2000) on 300/500 -> 500. Train Chip V on TP 97 ->
99. Power Chip (10) at RNG index 2 (the draw 3080, residue 0) -> +1, at index 6 (1309, residue 9) -> +10. calc_heal
0xBA (power 127) with Wisdom 60: 8128 + 952 = 9080; 0xB8 with a Wisdom Ring (+20): 512 + 80 = 592.
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import OFF, REC, rec, stats_bytes, u32, form
from oracle import SCRATCH_BASE, Call, Case, Read, Write

COMMENT = ("STSTATUS.PRO in the slot, the menu text file loaded. ststatus_items_use for every effect class: HP heal (partial, "
           "capped, exactly full, at max HP, above it, from 0 HP), TP (+5, the 99 cap, at 99, above it), the 15 stat-raise "
           "effects of ststatus_stat_raises with their RNG residues and caps, a battle-only item, the bag count; "
           "ststatus_calc_heal for the five healing techniques and Wisdom (equipment, 0); ststatus_tech_get_usable's MP "
           "check; ststatus_tech_use on one member and on the party (capped, full, one member full, MP not checked); "
           "ststatus_tech_collect (flags stripped, duplicates, IDs below 3, the range edges, the limit of 5, a stale list); "
           "the equipment page's ststatus_can_equip, ststatus_list_items (hand, head, body) and ststatus_equip_item. "
           "Window methods are stubbed with heap_nop.")

OVERLAY_SLOT = 0x80082CB0
STSTATUS_PRO = "extracted/disc/AAA/PRO/STSTATUS.PRO"
TEXT_ID, TEXT_FILE = 0xB2, "extracted/disc/AAA/DAT/COUNTRY/ENG/ESSTATUS.BIN"   # 0xB0 + records_language 2
TEXT_ADDR = 0x80190000           # free heap above the scratch buffers (the battle family's data file goes here too)
CDLOAD_ENTRY = 4 + 63 * 0x10     # cdload_module.entries[63]

WINDOW_SIZE = 0x174              # MessageWindow
ITEMS_DATA_SIZE, ITEMS_PAGE_SIZE = 0xD4, 0x45C   # StstatusItemsPageData, StstatusItemsPage
TECH_DATA_SIZE, TECH_PAGE_SIZE = 0xB4, 0x1F4     # StstatusTechPageData, StstatusTechPage
TECH_MEMBER, TECH_MEMBER_SIZE = 0x8C, 0x54       # StstatusTechPage.members[3]: forms 0, form_records 8, techniques 0x44

BASE_STATS, BASE_RESISTS = (60, 60, 60, 60, 60, 10), (100,) * 7
USED_ITEMS = list(range(43, 67))                  # the Charges, Chips, Train Chips and the Antidote Disk (66)
CHIPS = {2: (47, "HP Chip", 3, 9999, 30), 3: (48, "MP Chip", 5, 9999, 30)}   # effect -> item, name, values[] index, cap, amount
CHIPS.update({e: (e + 45, n, e + 2, 999, 10) for e, n in zip(range(4, 17), (
    "Power Chip", "Armor Chip", "Mind Chip", "Wisdom Chip", "Boost Chip", "Charisma Chip", "Fire Chip", "Water Chip",
    "Ice Chip", "Wind Chip", "Thunder Chip", "Metal Chip", "Devil Chip"))})
# RNG indexes by the residue of the next draw (pad_random_table[index + 1]): mod 10 and mod 30.
RES10 = {0: 2, 9: 6, 5: 8}
RES30 = {0: 52, 29: 23}


def stats_write(d, level=20, tp=50, hp=300, max_hp=500, mp=100, max_mp=200, stats=BASE_STATS, resists=BASE_RESISTS):
    return Write("gamestate_data", rec(d, REC["stats"]), stats_bytes(level, tp, hp, max_hp, mp, max_mp, stats, resists),
                 f"digimon[{d}].record.stats: TP {tp}, HP {hp}/{max_hp}, MP {mp}/{max_mp}, stats {stats}, resists {resists}")


def count_write(item, n):
    return Write("gamestate_data", OFF["items"] + item, struct.pack("<b", n), f"items[{item}] = {n}")


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 1, 7), "party = {0, 1, 7}"),
         Write("gamestate_data", OFF["items"] + USED_ITEMS[0], bytes([5] * len(USED_ITEMS)), "items[43..66] = 5"),
         Write("records_language", 0, u32(2), "records_language = 2 (English)"),
         Write("pad_random", 0, bytes(4), "pad_random.index = 0"),
         stats_write(0),
         stats_write(1, hp=200, max_hp=400, mp=50, max_mp=100, stats=(40, 40, 40, 80, 40, 5)),
         stats_write(7, hp=100, max_hp=300, mp=30, max_mp=60)]
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("pad_random", 4), ("records_language", 4), ("cdload_module", 0x434),
         ("sound_module", 0x4284)]
READ_RNG = Read("pad_random", 0, 4, "pad_random.index after the call (the draws taken)")
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]


def rng(index):
    return Write("pad_random", 0, u32(index), f"pad_random.index = {index}")


def ui_buffers(data_size, page_name, page):
    """The stubbed UI: `window` (every word heap_nop's address), the page's data block `windows` (every word -> window)
    and the page; placed in this order from SCRATCH_BASE as the oracle does (16-aligned)."""
    window_addr = SCRATCH_BASE
    return {"window": struct.pack("<I", NOP) * (WINDOW_SIZE // 4),
            "windows": struct.pack("<I", window_addr) * (data_size // 4),
            page_name: bytes(page)}


NOP = None   # heap_nop's address, set by cases(sym)


def items_page(item, member):
    p = bytearray(ITEMS_PAGE_SIZE)
    struct.pack_into("<i", p, 0x78, item)
    struct.pack_into("<i", p, 0x3B0, member)
    return p


def use_case(name, item, writes, desc, member=0, idx=0):
    d = (0, 1, 7)[member]
    reads = [Read("gamestate_data", rec(d, REC["stats"]), 0x2C, f"digimon[{d}].record.stats after the call"),
             Read("gamestate_data", OFF["items"] + item, 1, f"items[{item}] after the call"), READ_RNG] + AFTER
    return Case(name, [Call("ststatus_items_use", [("buf", "items_page"), ("buf", "windows")], "void", reads,
                            writes=writes + [rng(idx)], comment=desc)],
                fixture=FIXTURE, saves=SAVES, buffers=ui_buffers(ITEMS_DATA_SIZE, "items_page", items_page(item, member)),
                comment=f"item {item} on party member {member} (Digimon {d}); RNG index {idx}")


def heal_cases():
    return [
        use_case("heal_partial", 43, [stats_write(0, hp=300, max_hp=1000)], "Power Charge (500): HP 300/1000 -> 800, one used"),
        use_case("heal_capped", 44, [stats_write(0, hp=300, max_hp=500)], "Super Charge (2000): HP 300/500 -> 500 (capped), one used"),
        use_case("heal_exact_from_0", 43, [stats_write(0, hp=0, max_hp=500)], "Power Charge: HP 0/500 -> 500 exactly; 0 HP is not special"),
        use_case("heal_full", 46, [stats_write(0, hp=500, max_hp=500)], "Max Charge on HP 500/500: nothing, none used"),
        use_case("heal_above_max", 43, [stats_write(0, hp=600, max_hp=500)], "HP 600 above max 500: nothing (HP not lowered)"),
        use_case("heal_member2", 45, [], "Ultra Charge (5000) on party member 2 (Digimon 7, HP 100/300) -> 300", member=2),
        use_case("count_last", 43, [count_write(43, 1)], "the last one: items[43] 1 -> 0"),
        use_case("count_zero", 43, [count_write(43, 0)], "items[43] 0 (not offered by the menu): still used, -> -1 (s8, no floor)"),
    ]


def tp_cases():
    return [
        use_case("tp_add", 65, [], "Train Chip V (+5): TP 50 -> 55"),
        use_case("tp_cap", 65, [stats_write(0, tp=97)], "Train Chip V: TP 97 + 5 -> 99 (>= 100 becomes 99)"),
        use_case("tp_exact_99", 62, [stats_write(0, tp=98)], "Train Chip 1 (+1): TP 98 -> 99"),
        use_case("tp_at_99", 64, [stats_write(0, tp=99)], "Train Chip 3 on TP 99: nothing, none used"),
        use_case("tp_above_99", 63, [stats_write(0, tp=120)], "TP 120 (above the cap): nothing"),
    ]


def with_value(index, v):
    """stats_write kwargs that put v at GamestateStats.values[index] (3 max HP, 5 max MP, 6..11 stats, 12..18 resists)."""
    if index == 3:
        return dict(max_hp=v)
    if index == 5:
        return dict(max_mp=v)
    if index < 12:
        s = list(BASE_STATS)
        s[index - 6] = v
        return dict(stats=tuple(s))
    r = list(BASE_RESISTS)
    r[index - 12] = v
    return dict(resists=tuple(r))


def raise_cases():
    out = []
    # Every effect of ststatus_stat_raises at RNG index 2 (draw 3080: +1 for amount 10, +21 for amount 30).
    for effect, (item, name, index, cap, amount) in CHIPS.items():
        out.append(use_case(f"raise_effect{effect}", item, [],
                            f"{name} (effect {effect}, amount {amount}): values[{index}] += 3080 % {amount} + 1 = {3080 % amount + 1}", idx=2))
    # Residues: the largest and smallest draws, both amounts.
    out.append(use_case("raise_power_res9", 49, [], "Power Chip at RNG index 6 (1309 % 10 = 9): Power 60 -> 70", idx=RES10[9]))
    out.append(use_case("raise_maxhp_res0", 47, [], "HP Chip at RNG index 52 (residue 0 mod 30): max HP 500 -> 501; HP stays 300", idx=RES30[0]))
    out.append(use_case("raise_maxhp_res29", 47, [], "HP Chip at RNG index 23 (residue 29): max HP 500 -> 530", idx=RES30[29]))
    # Caps: below, at, above.
    out.append(use_case("raise_power_capped", 49, [stats_write(0, **with_value(6, 995))], "Power 995 + 10 -> 999 (capped), one used", idx=RES10[9]))
    out.append(use_case("raise_power_at_cap", 49, [stats_write(0, **with_value(6, 999))], "Power 999: nothing, no draw, none used", idx=RES10[9]))
    out.append(use_case("raise_power_above_cap", 49, [stats_write(0, **with_value(6, 1005))], "Power 1005 (above 999): nothing"))
    out.append(use_case("raise_maxhp_capped", 47, [stats_write(0, max_hp=9990)], "max HP 9990 + 30 -> 9999", idx=RES30[29]))
    out.append(use_case("raise_maxmp_at_cap", 48, [stats_write(0, max_mp=9999)], "max MP 9999: nothing"))
    out.append(use_case("raise_dark_at_cap", 61, [stats_write(0, **with_value(18, 999))], "Devil Chip on Dark 999: nothing"))
    out.append(use_case("raise_dark_capped", 61, [stats_write(0, **with_value(18, 998))], "Devil Chip on Dark 998 + 10 -> 999", idx=RES10[9]))
    out.append(use_case("battle_item", 66, [], "Antidote Disk (effect 0, battle only): nothing, none used"))
    return out


def tech_page(user=0, target=0, cursor=0, member_count=3, techniques=None):
    p = bytearray(TECH_PAGE_SIZE)
    struct.pack_into("<i", p, 0x5C, member_count)
    struct.pack_into("<i", p, 0x7C, user)
    struct.pack_into("<i", p, 0x84, target)
    struct.pack_into("<i", p, 0x88, cursor)
    for m, techs in (techniques or {}).items():
        struct.pack_into("<5h", p, TECH_MEMBER + m * TECH_MEMBER_SIZE + 0x44, *(list(techs) + [0] * (5 - len(techs))))
    return p


def calc_cases():
    out = []
    for tech, power in ((0xB8, 8), (0xB9, 32), (0xBA, 127), (0xBB, 32), (0xBC, 64)):
        out.append(Case(f"calc_heal_{tech:x}", [Call("ststatus_calc_heal", [0, tech], "s32", comment=f"power {power}, Wisdom 60: {power * 64} + {power * 60 // 8}")],
                        fixture=FIXTURE, saves=SAVES, comment="Digimon 0"))
    ring = Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", 0, 0, 0, 0, 299, 0), "digimon[0].record.equipment[4] = 299 Wisdom Ring (+20)")
    out.append(Case("calc_heal_equipment", [
        Call("ststatus_calc_heal", [0, 0xB8], "s32", writes=[ring], comment="Wisdom 60 + 20 (ring): 512 + 80 = 592"),
        Call("ststatus_calc_heal", [0, 0xBA], "s32", writes=[stats_write(0, stats=(60, 60, 60, 0, 60, 10))], comment="Wisdom 0 (and the ring): 8128 + 127 * 20 / 8 = 8445"),
        Call("ststatus_calc_heal", [1, 0xBA], "s32", comment="Digimon 1, Wisdom 80: 8128 + 1270 = 9398")],
        fixture=FIXTURE, saves=SAVES, comment="Wisdom from gamestate_get_stats: equipment counts"))
    return out


def usable_cases():
    page = tech_page(techniques={0: [0xB8, 0xBA]})
    return [Case("get_usable", [
        Call("ststatus_tech_get_usable", [("buf", "tech_page")], "s32", writes=[stats_write(0, mp=16)], comment="0xB8 costs 16, MP 16 -> 0xB8"),
        Call("ststatus_tech_get_usable", [("buf", "tech_page")], "s32", writes=[stats_write(0, mp=15)], comment="MP 15 -> 0")],
        fixture=FIXTURE, saves=SAVES, buffers=ui_buffers(TECH_DATA_SIZE, "tech_page", page), comment="user 0, cursor 0: 0xB8")]


def tech_use_case(name, tech, writes, desc, user=0, target=0, member_count=3):
    page = tech_page(user=user, target=target, member_count=member_count, techniques={user: [tech]})
    reads = [Read("gamestate_data", rec(d, REC["stats"]), 0x2C, f"digimon[{d}].record.stats after the call") for d in (0, 1, 7)]
    return Case(name, [Call("ststatus_tech_use", [("buf", "tech_page"), ("buf", "windows"), 0x4001C], "s32", reads + AFTER,
                            writes=writes, comment=desc)],
                fixture=FIXTURE, saves=SAVES, buffers=ui_buffers(TECH_DATA_SIZE, "tech_page", page),
                comment=f"technique {tech:#x}, user {user}, target {target}, {member_count} members")


def tech_use_cases():
    full1 = stats_write(1, hp=400, max_hp=400, mp=50, max_mp=100, stats=(40, 40, 40, 80, 40, 5))
    full7 = stats_write(7, hp=300, max_hp=300, mp=30, max_mp=60)
    return [
        tech_use_case("use_one_capped", 0xB8, [], "0xB8 (572) on member 1 (Digimon 1, HP 200/400) -> 400; user MP 100 - 16 -> 84", target=1),
        tech_use_case("use_one_partial", 0xB8, [stats_write(1, hp=200, max_hp=1000, stats=(40, 40, 40, 80, 40, 5))],
                      "0xB8 on HP 200/1000 -> 772", target=1),
        tech_use_case("use_one_full", 0xB9, [full1], "target full: fails (0), no MP taken", target=1),
        tech_use_case("use_self", 0xBA, [], "user heals itself: HP 300/500 -> 500, MP 100 - 120 -> -20 (the MP is not checked)", target=0),
        tech_use_case("use_party", 0xBB, [full7], "0xBB (2288) on the party: Digimon 0 -> 500, 1 -> 400, 7 already full; MP 100 - 188 once"),
        tech_use_case("use_party_all_full", 0xBC, [stats_write(0, hp=500), full1, full7], "everyone full: fails (0), no MP taken"),
        tech_use_case("use_party_two_members", 0xBC, [], "member_count 2: Digimon 7 (member 2) is not healed", member_count=2),
        tech_use_case("use_party_empty_slot", 0xBB, [Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, -1, 7), "party = {0, -1, 7}")],
                      "a party slot of -1 is skipped"),
    ]


def forms_write(d, chosen, forms):
    w = [Write("gamestate_data", rec(d, REC["chosen_forms"]), struct.pack("<3h", *chosen), f"digimon[{d}].record.chosen_forms = {chosen}")]
    blob = b"".join(form(f, 1, 0, tuple(x - 0x10000 if x >= 0x8000 else x for x in t) + (0,) * (6 - len(t))) for f, t in forms)
    w.append(Write("gamestate_data", rec(d, REC["forms"]), blob, f"digimon[{d}].record.forms[0..{len(forms)}) = {forms}"))
    return w


def collect_case(name, writes, desc, member=0, stale=None):
    page = tech_page(techniques={member: stale} if stale else None)
    off = TECH_MEMBER + member * TECH_MEMBER_SIZE
    return Case(name, [Call("ststatus_tech_collect", [("buf", "tech_page"), member], "s32",
                            [Read("buf:tech_page", off, TECH_MEMBER_SIZE, f"tech_page.members[{member}] after the call")],
                            writes=writes, comment=desc)],
                fixture=FIXTURE, saves=SAVES, buffers=ui_buffers(TECH_DATA_SIZE, "tech_page", page), comment=f"member {member}")


def collect_cases():
    forms = [(20, [0xB8, 0x20B9, 5]), (21, [0x80B8, 0xBC, 0xB7]), (22, [0xBD, 0xBA])]
    return [
        collect_case("collect_basic", forms_write(0, (20, 21, 22), forms),
                     "forms 20 {0xB8, 0x20B9}, 21 {0x80B8 dup, 0xBC, 0xB7 below the range}, 22 {0xBD above, 0xBA}: {B8, B9, BC, BA} -> 4"),
        collect_case("collect_low_ids", forms_write(0, (2, 21, 22), [(2, [0xBB])] + forms[1:]),
                     "chosen form 2 (< 3) is skipped: {B8, BC, BA}"),
        collect_case("collect_limit", forms_write(0, (20, 21, 22), [(20, [0xB8, 0xB9, 0xBA, 0xBB]), (21, [0xBC, 0xB8]), (22, [0xBA])]),
                     "all five by the second form: returns 5 there"),
        collect_case("collect_stale", forms_write(0, (20, 21, 22), forms), "stale list {0, 0, 0xB9}: 0xB9 is taken as found",
                     stale=[0, 0, 0xB9]),
        collect_case("collect_member2", forms_write(7, (20, 0, 0), [(20, [0xBB])]), "member 2 = Digimon 7: {BB}", member=2),
    ]


def equip_cases():
    """The equipment page's helpers (ststatus_80099B6C.c): the same rules as STITSHOP's (shop_rules), so one case per
    rule, the bag counts always kept."""
    out = [Case("can_equip", [Call("ststatus_can_equip", [d, slot, item], "s32", comment=desc) for d, slot, item, desc in (
        (0, 2, -1, "no item (-1) -> 1"), (0, 2, 92, "Short Sword (members 0x01), Digimon 0 -> 1"), (1, 2, 92, "Digimon 1 -> 0"),
        (0, 2, 275, "Buckler (kind 2, a shield) in slot 2 -> 0"), (0, 3, 275, "in slot 3 -> 1"), (0, 3, 92, "a sword in slot 3 -> 1"))],
        fixture=FIXTURE, saves=SAVES, comment="ststatus_can_equip(digimon, slot, item)")]
    bag = [Write("gamestate_data", OFF["items"] + i, bytes([2]), f"items[{i}] = 2") for i in (92, 93, 176, 215, 243, 269, 275, 284, 292)]
    for cat, desc in ((5, "hand items (kinds 1, 2, 3, 7): weapons 92, 93, 176, then shields 275, 284"), (6, "head (kind 4): 215, 243"),
                      (7, "body (kind 5): 269")):
        out.append(Case(f"list_items_{cat}", [Call("ststatus_list_items", [cat, ("buf", "list")], "s32",
                                                    [Read("buf:list", 0, 0x20, "the list's first 16 entries")], writes=bag, comment=desc)],
                        fixture=FIXTURE, saves=SAVES + [("ststatus_module", 0x690)], buffers={"list": bytes(0x328)},
                        comment="the owned items of lists 2 and 3 (weapons, armour) of the slot's kinds"))

    def equip(name, slot, item, equipment, involved, desc):
        writes = [Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", *equipment), f"digimon[0].record.equipment = {equipment}")]
        writes += [Write("gamestate_data", 0x20F + i, bytes([1]), f"items_equipped[{i}] = 1") for i in set(equipment) - {0}]
        writes += [Write("gamestate_data", OFF["items"] + i, bytes([2]), f"items[{i}] = 2") for i in involved]
        reads = [Read("gamestate_data", rec(0, REC["equipment"]), 12, "digimon[0].record.equipment after the call")]
        for i in involved:
            reads += [Read("gamestate_data", OFF["items"] + i, 1, f"items[{i}]"), Read("gamestate_data", 0x20F + i, 1, f"items_equipped[{i}]")]
        return Case(name, [Call("ststatus_equip_item", [0, slot, item], "void", reads + AFTER, writes=writes, comment=desc)],
                    fixture=FIXTURE, saves=SAVES, comment="Digimon 0")
    out += [equip("equip_two_hands", 2, 176, (0, 0, 92, 275, 0, 0), (92, 275, 176), "Claymore over sword + Buckler: both back to the bag"),
            equip("equip_crest_group", 4, 325, (0, 0, 0, 0, 317, 326), (317, 326, 325), "crest 325 (group 9) into slot 4: 317 out of 4, 326 (group 9) out of 5"),
            equip("equip_remove", 2, 0, (0, 0, 176, 176, 0, 0), (176,), "item 0 into slot 2 over the Claymore: both hands emptied, one back")]
    return out


def setup_case():
    entry = struct.pack("<hhiiI", 3, 0, TEXT_ID, 0, TEXT_ADDR)
    return Case("setup_ststatus", [Call("ststatus_calc_heal", [0, 0xB8], "void", comment="smoke: the overlay answers")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STSTATUS.PRO", file=STSTATUS_PRO),
                         Write(TEXT_ADDR, 0, b"", "ESSTATUS.BIN, the menu text (file 0xB2)", file=TEXT_FILE),
                         Write("cdload_module", CDLOAD_ENTRY, entry,
                               "cdload_module.entries[63] = {state 3 (loaded), id 0xB2, buffer -> the file above}")],
                keep=True, comment="setup, kept for the family: the overlay and the text file; nothing is restored")


def cases(sym):
    global NOP
    NOP = sym["heap_nop"]
    return ([setup_case()] + heal_cases() + tp_cases() + raise_cases() + calc_cases() + usable_cases() + tech_use_cases()
            + collect_cases() + equip_cases())
