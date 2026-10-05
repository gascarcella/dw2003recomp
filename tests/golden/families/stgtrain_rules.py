"""Training (STGTRAIN, src/stgtrain/stgtrain_800861BC.c and stgtrain_80088100.c): the stat gains and losses of a
training round, the accessories' success bonus, and the training menus. docs/MECHANICS.md section 6.

A session (StgtrainSession) trains party Digimon `digimon` at menu entry `training` with the trainer level
StgtrainMain.level (0..2, costing stgtrain_tp_costs {1, 5, 10} of GamestateStats.values[1], the TP); after each won
round stgtrain_session_apply_round calls:
- stgtrain_session_raise_stat(type 1..5): stats[type - 1] += stgtrain_stat_gains[row] with row = (training >= 13 ? 6 : 0)
  + bonus * 3 + level; a gain is base + next() % rand (rand 0: base, no draw); cap 999. Other types: 0, no draw.
- stgtrain_session_lower_stat(type 1..5): one draw; odd: no loss; even: stats[type - 1] -= stgtrain_stat_losses[level];
  floor 0.
- stgtrain_session_raise_stat2(type 8..14): resists[type - 8] += stgtrain_resist_gain_tables[(class - 1) * 3 + size][row]
  with class = records_digimon[digimon].resist_gains[type - 8] (1..5), size 0 below 100, 1 below 300, else 2, and row =
  level * 3 + (bonus ? (training < 13 ? 1 : 2) : 0); cap 999.
- stgtrain_session_raise_max(15 max HP, 16 max MP): stgtrain_max_gains[same row]; cap 9999.
- stgtrain_session_get_bonus: success chance bonus 3 for item 0x151 in equipment slot 4 or 5, else 6 for 0x152, else 0.
- stgtrain_get_menu(menu): the menu's 16 entries; menu 1..14 kept, anything else becomes 0; counts the non-zero IDs into
  stgtrain_module.count. stgtrain_menus has 14 rows (0..13), so menu 14 reads the 16 entries past the table (the start of
  stgtrain_trainings): the golden records what the original counts there.

STGTRAIN.PRO goes in the overlay slot (a kept setup case); its tables and stgtrain_module are inside the file. Fixture:
pointer-free gamestate_data zeroed, party {0, 1, 7}, records of Digimon 0 and 1 (level 20, stats and resists as each
call writes). Buffers: train_session (0x134 bytes: main 0x50, digimon 0x5C, training 0x60, bonus 0xD8) whose `main`
points at train_main (0x114 bytes: level 0x7C). The RNG index is set before each call and read back.

Hand checks: raise_stat row 0 (training 12, no bonus, level 0) = 1 + next() % 2; at RNG index 1 the draw is 1151 (odd)
-> 2. raise_stat2 of Digimon 0's Fire (class 2) at 50 with the bonus, training 13, level 0: table 3 = resist_gains_0,
row 2 -> +2; Machine of Digimon 0 is class 3 -> resist_gains_1 row 2 -> +3; Thunder (class 4) -> resist_gains_2 -> +4.
raise_max row 8 (level 2, bonus, training 13) = 40: max HP 9990 -> 9999. get_bonus with 0x151 and 0x152 both worn -> 3.

stgtrain_session_apply_round(session, round) (appended cases): the entry stgtrain_find_menu_entry(map, training) (map
outside 1..14: menu 0) decides; when results[round] != 0 (a won round, or -1, "not played") and the entry's stat != 0:
stat 1..5 -> gains[round] = raise_stat(stat); else gains[round] = raise_stat2(stat) and, with stat_2 != 0, losses[round]
= lower_stat(stat_2) for 1..5, else raise_max(stat_2) (the Fire, Ice... trainings raise max HP or MP as their "loss").
The result windows are stubs (a `session_data` block whose every word points at `window`, every method heap_nop); the
text file (0x10B + records_language = 0x10D, ESTRAINI.BIN) is loaded by a second kept setup case, since get_file is still
called. A training the menu does not have gives a NULL entry, read at once: not a case.

Not tested here: the round's success roll (`chance >= next() % 100` with chance 75 + bonus, 50 for the bonus round, 0
when the same training's bonus was won last time) is inside stgtrain_trainee_update and stgtrain_session_run, which
need sprites and windows (docs/MECHANICS.md section 6 has it [C]); stgtrain_find_menu_entry returns a pointer, which
the host cannot compare (stgtrain_get_menu covers the menu index rule).
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import DIGIMON_SIZE, JOINED, OFF, REC, rec, stats_bytes, u32
from oracle import SCRATCH_BASE, Call, Case, Read, Write

COMMENT = ("STGTRAIN.PRO in the slot. stgtrain_session_raise_stat on every gain row (training below/at 13, bonus, trainer "
           "level) with RNG residues, the 999 cap and out-of-range types; stgtrain_session_lower_stat's odd/even draw, every "
           "loss row and the floor; stgtrain_session_raise_stat2 on all 15 gain-table slots (5 classes x 3 stat sizes), the "
           "size edges 99/100 and 299/300, every row, the cap; stgtrain_session_raise_max on every row, max MP, the 9999 cap; "
           "stgtrain_session_get_bonus for the two accessories; stgtrain_get_menu's index rule and entry counts (menu 14 "
           "reads past the table); stgtrain_session_apply_round's dispatch (a stat, a resist with a stat loss or a max HP/MP "
           "raise, a lost and an unplayed round, a map outside the menus) with stubbed result windows.")

OVERLAY_SLOT = 0x80082CB0
STGTRAIN_PRO = "extracted/disc/AAA/PRO/STGTRAIN.PRO"
SESSION_SIZE, MAIN_SIZE = 0x134, 0x114
MAIN_ADDR = SCRATCH_BASE + ((SESSION_SIZE + 15) & ~15)   # where the oracle places train_main (after train_session)

# records_digimon rows 0, 1: resist gain classes (Fire, Water, Ice, Wind, Thunder, Machine, Dark).
CLASSES = {0: (2, 5, 2, 2, 4, 3, 3), 1: (2, 4, 3, 4, 2, 4, 1)}
BASE_STATS = (60, 60, 60, 60, 60, 1)
BASE_RESISTS = (100, 100, 100, 100, 100, 100, 100)


def stats_write(d, stats=BASE_STATS, resists=BASE_RESISTS, max_hp=500, max_mp=200):
    return Write("gamestate_data", rec(d, REC["stats"]), stats_bytes(20, 30, max_hp, max_hp, max_mp, max_mp, stats, resists),
                 f"digimon[{d}].record.stats: level 20, TP 30, max HP {max_hp}, max MP {max_mp}, stats {stats}, resists {resists}")


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_flags", 0, bytes(0x1C), "gamestate_flags = 0"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 1, 7), "party = {0, 1, 7}"),
         Write("pad_random", 0, bytes(4), "pad_random.index = 0")]
    for d in (0, 1):
        w.append(Write("gamestate_data", OFF["digimon"] + d * DIGIMON_SIZE + JOINED, u32(d + 3), f"digimon[{d}].joined = {d + 3}"))
        w.append(stats_write(d))
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C), ("pad_random", 4), ("stgtrain_module", 4)]
READ_RNG = Read("pad_random", 0, 4, "pad_random.index after the call (the draws taken)")
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]


def rng(index):
    return Write("pad_random", 0, u32(index), f"pad_random.index = {index}")


def buffers(digimon, training, bonus, level):
    s = bytearray(SESSION_SIZE)
    struct.pack_into("<I", s, 0x50, MAIN_ADDR)
    struct.pack_into("<2i", s, 0x5C, digimon, training)
    struct.pack_into("<i", s, 0xD8, bonus)
    m = bytearray(MAIN_SIZE)
    struct.pack_into("<i", m, 0x7C, level)
    return {"train_session": bytes(s), "train_main": bytes(m)}


def session_case(name, func, d, type_, training, bonus, level, writes, desc, idx=0):
    reads = [Read("gamestate_data", rec(d, REC["stats"]), 0x2C, f"digimon[{d}].record.stats after the call"), READ_RNG] + AFTER
    return Case(name, [Call(func, [("buf", "train_session"), type_], "s32", reads, writes=writes + [rng(idx)], comment=desc)],
                fixture=FIXTURE, saves=SAVES, buffers=buffers(d, training, bonus, level),
                comment=f"session: Digimon {d}, training {training}, bonus {bonus}, trainer level {level}; RNG index {idx}")


STAT_GAINS = [(1, 2), (7, 2), (14, 3), (2, 0), (10, 0), (20, 0), (1, 2), (6, 3), (11, 5), (4, 0), (22, 0), (48, 0)]
RESIST_ARRAYS = [[(1, 0), (1, 0), (2, 0), (4, 2), (8, 0), (12, 0), (8, 2), (20, 0), (30, 0)],
                 [(1, 2), (2, 0), (3, 0), (6, 3), (12, 0), (18, 0), (12, 4), (30, 0), (40, 0)],
                 [(2, 0), (2, 0), (4, 0), (8, 4), (12, 0), (25, 0), (16, 5), (30, 0), (50, 0)]]
RESIST_TABLES = [0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 2, 1, 2, 2, 1]   # stgtrain_resist_gain_tables -> resist_gains_0/1/2
MAX_GAINS = [(1, 2), (2, 0), (4, 0), (6, 4), (10, 0), (20, 0), (13, 5), (20, 0), (40, 0)]
# RNG indexes by the residue of the next draw (pad_random_table): mod 2, 3, 4, 5.
RES = {2: [0, 1], 3: [0, 6, 1], 4: [0, 6, 4, 1], 5: [2, 1, 7, 0, 3]}


def raise_stat_cases():
    out = []
    for row, (base, rand) in enumerate(STAT_GAINS):
        training = 13 if row >= 6 else 12
        bonus, level = (row % 6) // 3, row % 3
        t = row % 5 + 1
        for idx in (RES[rand] if rand else [0]):
            desc = f"type {t}, row {row} (training {training}, bonus {bonus}, level {level}): {base} + next() % {rand}" if rand else \
                f"type {t}, row {row} (training {training}, bonus {bonus}, level {level}): {base}, no draw"
            out.append(session_case(f"raise_stat_row{row}_rng{idx}", "stgtrain_session_raise_stat", 0, t, training, bonus, level, [], desc, idx))
    out.append(session_case("raise_stat_cap", "stgtrain_session_raise_stat", 0, 2, 13, 1, 2, [stats_write(0, stats=(60, 990, 60, 60, 60, 1))],
                            "Guard 990 + 48 (row 11) -> 999"))
    for t in (0, 6):
        out.append(session_case(f"raise_stat_type{t}", "stgtrain_session_raise_stat", 0, t, 12, 0, 0, [],
                                f"type {t}: outside 1..5 -> 0, no draw (6 would be Charisma: never trained)"))
    return out


def lower_stat_cases():
    out = [session_case("lower_stat_odd", "stgtrain_session_lower_stat", 0, 1, 12, 0, 1, [], "first draw odd (1151): no loss -> 0, one draw", 1)]
    for level, (base, rand) in enumerate([(1, 0), (2, 3), (4, 3)]):
        for idx in ([13] if rand == 0 else ([13, 5, 0] if level == 2 else [0])):
            out.append(session_case(f"lower_stat_lv{level}_rng{idx}", "stgtrain_session_lower_stat", 0, 3, 12, 0, level, [],
                                    f"first draw even: Spirit -= {base}" + (f" + next() % {rand}" if rand else ""), idx))
    out.append(session_case("lower_stat_floor", "stgtrain_session_lower_stat", 0, 5, 12, 0, 2, [stats_write(0, stats=(60, 60, 60, 60, 3, 1))],
                            "Boost 3 - 6 -> 0 (floor)", 0))
    for t in (0, 6):
        out.append(session_case(f"lower_stat_type{t}", "stgtrain_session_lower_stat", 0, t, 12, 0, 0, [], f"type {t}: -> 0 before the draw"))
    return out


def resists_with(i, v):
    r = list(BASE_RESISTS)
    r[i] = v
    return tuple(r)


def raise_stat2_cases():
    out = []
    # All 15 table slots: class 1..5 (Digimon 1 Dark is class 1; Digimon 0 Fire 2, Machine 3, Thunder 4, Water 5) x size,
    # at row 2 (bonus, training 13, level 0): resist_gains_0/1/2 give 2/3/4 there, no draw.
    who = {1: (1, 6), 2: (0, 0), 3: (0, 5), 4: (0, 4), 5: (0, 1)}
    for cls, (d, i) in who.items():
        for size, v in enumerate((50, 150, 400)):
            arr = RESIST_TABLES[(cls - 1) * 3 + size]
            out.append(session_case(f"raise_stat2_class{cls}_size{size}", "stgtrain_session_raise_stat2", d, 8 + i, 13, 1, 0,
                                    [stats_write(d, resists=resists_with(i, v))],
                                    f"class {cls}, resist {v} (size {size}): table {(cls - 1) * 3 + size} = resist_gains_{arr}, row 2 -> +{RESIST_ARRAYS[arr][2][0]}"))
    # Size edges at row 2: class 2 (Fire of Digimon 0; tables 3, 4, 5 = arrays 0, 1, 1: +2, +3, +3) shows 99/100, class 4
    # (Thunder; tables 9, 10, 11 = arrays 2, 2, 1: +4, +4, +3) shows 299/300.
    for i, v, gain in ((0, 99, 2), (0, 100, 3), (4, 299, 4), (4, 300, 3)):
        out.append(session_case(f"raise_stat2_edge_{8 + i}_{v}", "stgtrain_session_raise_stat2", 0, 8 + i, 13, 1, 0, [stats_write(0, resists=resists_with(i, v))],
                                f"class {CLASSES[0][i]}, resist {v}: size {0 if v < 100 else 1 if v < 300 else 2} -> +{gain}"))
    # Every row on class 3 (resist_gains_1, whose rows all differ).
    for level in range(3):
        for bonus, training in ((0, 12), (1, 12), (1, 13)):
            row = level * 3 + (0 if not bonus else 1 if training < 13 else 2)
            base, rand = RESIST_ARRAYS[1][row]
            for idx in (RES[rand] if rand and level == 2 else ([1] if rand else [0])):
                out.append(session_case(f"raise_stat2_row{row}_rng{idx}", "stgtrain_session_raise_stat2", 0, 13, training, bonus, level, [],
                                        f"Machine (class 3) 100: row {row} = {base}" + (f" + next() % {rand}" if rand else ""), idx))
    out.append(session_case("raise_stat2_nobonus_t13", "stgtrain_session_raise_stat2", 0, 13, 13, 0, 1, [],
                            "no bonus, training 13: row 3 (training only matters with the bonus) = 6 + next() % 3", 1))
    out.append(session_case("raise_stat2_cap", "stgtrain_session_raise_stat2", 0, 13, 13, 1, 2, [stats_write(0, resists=resists_with(5, 995))],
                            "Machine 995 (size 2: resist_gains_1) row 8 + 40 -> 999"))
    for t in (7, 15):
        out.append(session_case(f"raise_stat2_type{t}", "stgtrain_session_raise_stat2", 0, t, 12, 0, 0, [], f"type {t}: outside 8..14 -> 0"))
    return out


def raise_max_cases():
    out = []
    for level in range(3):
        for bonus, training in ((0, 12), (1, 12), (1, 13)):
            row = level * 3 + (0 if not bonus else 1 if training < 13 else 2)
            base, rand = MAX_GAINS[row]
            for idx in (RES[rand] if rand and row == 6 else ([1] if rand else [0])):
                out.append(session_case(f"raise_max_row{row}_rng{idx}", "stgtrain_session_raise_max", 0, 15, training, bonus, level, [],
                                        f"max HP 500: row {row} = {base}" + (f" + next() % {rand}" if rand else ""), idx))
    out.append(session_case("raise_max_mp", "stgtrain_session_raise_max", 0, 16, 13, 1, 1, [], "max MP (type 16), row 5 -> +20"))
    out.append(session_case("raise_max_cap", "stgtrain_session_raise_max", 0, 15, 13, 1, 2, [stats_write(0, max_hp=9990)], "max HP 9990 + 40 -> 9999"))
    for t in (14, 17):
        out.append(session_case(f"raise_max_type{t}", "stgtrain_session_raise_max", 0, t, 12, 0, 0, [], f"type {t}: -> 0"))
    return out


def bonus_cases():
    out = []
    for name, equip, desc in (("none", (0,) * 6, "no accessory -> 0"),
                              ("151_slot4", (0, 0, 0, 0, 0x151, 0), "0x151 in slot 4 -> 3"),
                              ("152_slot5", (0, 0, 0, 0, 0, 0x152), "0x152 in slot 5 -> 6"),
                              ("both", (0, 0, 0, 0, 0x152, 0x151), "0x152 and 0x151: 0x151 is checked first -> 3"),
                              ("151_slot3", (0, 0, 0, 0x151, 0, 0), "0x151 in slot 3: not looked at -> 0")):
        w = Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", *equip), f"digimon[0].record.equipment = {equip}")
        out.append(Case(f"bonus_{name}", [Call("stgtrain_session_get_bonus", [("buf", "train_session")], "s32", writes=[w], comment=desc)],
                        fixture=FIXTURE, saves=SAVES, buffers=buffers(0, 1, 0, 0), comment="session: Digimon 0"))
    return out


def menu_cases():
    out = []
    for menu in (-1, 0, 1, 2, 3, 13, 14, 15):
        desc = {-1: "menu -1 -> menu 0", 0: "menu 0 (5 entries)", 14: "menu 14 is kept but stgtrain_menus has rows 0..13: "
                "counts 16 words past the table (stgtrain_trainings)", 15: "menu 15 -> menu 0"}.get(menu, f"menu {menu}")
        out.append(Case(f"menu_{menu & 0xFF}", [Call("stgtrain_get_menu", [menu], "void",
                                                     [Read("stgtrain_module", 0, 4, "stgtrain_module.count after the call")], comment=desc)],
                        fixture=FIXTURE, saves=SAVES, comment="the returned pointer is not compared (host addresses differ); the count is"))
    return out


def setup_case():
    return Case("setup_stgtrain", [Call("stgtrain_get_menu", [1], "void", comment="smoke: the overlay answers")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STGTRAIN.PRO", file=STGTRAIN_PRO)],
                keep=True, comment="setup, kept for the family: the overlay in the slot; nothing is restored")


TEXT_ID, TEXT_FILE, TEXT_ADDR = 0x10D, "extracted/disc/AAA/DAT/COUNTRY/ENG/ESTRAINI.BIN", 0x80190000
CDLOAD_ENTRY = 4 + 63 * 0x10     # cdload_module.entries[63]
WINDOW_SIZE, DATA_SIZE = 0x174, 0x1C   # MessageWindow, StgtrainSessionData (seven pointers)
ROUND_SAVES = SAVES + [("records_language", 4)]


def text_setup_case():
    return Case("setup_stgtrain_text", [Call("stgtrain_get_menu", [1], "void", comment="smoke")],
                fixture=[Write(TEXT_ADDR, 0, b"", "the training text ESTRAINI.BIN (0x10D)", file=TEXT_FILE),
                         Write("cdload_module", CDLOAD_ENTRY, struct.pack("<hhiiI", 3, 0, TEXT_ID, 0, TEXT_ADDR),
                               "cdload_module.entries[63] = {loaded, id 0x10D, buffer -> the file}")],
                keep=True, comment="setup, kept: the text file apply_round's windows are given (get_file(0x10D))")


def round_case(name, sym, training, round_, result, desc, map_=1, idx=1, level=0):
    b = buffers(0, training, 0, level)
    s = bytearray(b["train_session"])
    window = SCRATCH_BASE + ((SESSION_SIZE + 15) & ~15) + ((MAIN_SIZE + 15) & ~15)
    data = window + ((WINDOW_SIZE + 15) & ~15)
    struct.pack_into("<I", s, 0x24, data)                 # Object.children: the session's data block
    struct.pack_into("<i", s, 0x64 + 4 * round_, result)  # results[round]
    struct.pack_into("<i", s, 0xA0, map_)
    b["train_session"] = bytes(s)
    b["window"] = struct.pack("<I", sym["heap_nop"]) * (WINDOW_SIZE // 4)
    b["session_data"] = struct.pack("<I", window) * (DATA_SIZE // 4)
    reads = [Read("buf:train_session", 0x64, 0x3C, "results[5], gains[5], losses[5] after the call"),
             Read("gamestate_data", rec(0, REC["stats"]), 0x2C, "digimon[0].record.stats after the call"), READ_RNG] + AFTER
    writes = [rng(idx), Write("records_language", 0, u32(2), "records_language = 2 (English)")]
    return Case(f"round_{name}", [Call("stgtrain_session_apply_round", [("buf", "train_session"), round_], "void", reads,
                                       writes=writes, comment=desc)],
                fixture=FIXTURE, saves=ROUND_SAVES, buffers=b,
                comment=f"Digimon 0, map {map_:#x}, training {training}, round {round_}, results[round] {result}, level {level}; RNG index {idx}")


def round_cases(sym):
    return [
        text_setup_case(),
        round_case("power", sym, 1, 2, 1, "map 1, training 1 (stat 1, Power): gains[2] = raise_stat(1); losses untouched"),
        round_case("fire_max_mp", sym, 6, 2, 1, "training 6 (stat 8 Fire, stat_2 16): gains[2] = raise_stat2(8), "
                   "losses[2] = raise_max(16): max MP goes up"),
        round_case("water_guard", sym, 7, 2, 1, "training 7 (stat 9 Water, stat_2 2): gains[2] = raise_stat2(9), losses[2] = "
                   "lower_stat(2) (Guard, its own odd/even draw)", idx=13),
        round_case("lost", sym, 1, 2, 0, "results[2] 0: nothing applied, no draw (only the failure text)"),
        round_case("unplayed", sym, 1, 2, -1, "results[2] -1 (not played) is non-zero: applied like a win"),
        round_case("map_outside", sym, 3, 0, 1, "map 0x20 is outside 1..14: menu 0, training 3 (Spirit) -> raise_stat(3)", map_=0x20),
    ]


def cases(sym):
    return ([setup_case()] + raise_stat_cases() + lower_stat_cases() + raise_stat2_cases() + raise_max_cases() + bonus_cases()
            + menu_cases() + round_cases(sym))
