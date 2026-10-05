"""The digivolution rules of the battle report (src/stfgtrep/stfgtrep_80082E70.c): which new form ("Digivolution") a
Digimon gains, how a form levels up, the techniques a form learns and marks, the form experience a battle gives, and
the experience bonus item. docs/MECHANICS.md section 7.

The C calls a form a "technique" (StfgtrepLearn.technique, stfgtrep_learn_technique): it is a records_digimon row
(1-based) whose ID gamestate_add_form stores in GamestateRecord.forms. Functions:
- stfgtrep_learn_technique(d): walks stfgtrep_learn_lists[d] (44 entries, one per form row 9..52) and gains the first
  form not owned whose requirements hold: up to two other forms at a minimum form level, and one stat minimum (1-6 the six
  stats, 7 the Digimon's level, 8-14 the seven resists). The new form gets level 1 and the first free chosen slot (after
  gamestate_get_chosen_forms packed them). One form per call; 0 when none qualifies.
- stfgtrep_add_technique_exp(d, form, exp): form level L goes up while exp >= L * 10, or past the form's
  level_thresholds[5] (t5) x * 10 + (L - x) * 50 with x = t5 - 1; cap 99; exp capped 9,999,999; a form already at 99
  returns 0 without storing the exp.
- stfgtrep_learn_skill(d, form): the first empty slot i (0..5) whose records technique [i + 1] exists and whose
  technique_levels[i] <= the form's level gets it (slot 5 with 0x8000); later slots may fill before earlier ones.
- stfgtrep_mark_skill(d, form): the first slot i < 5 holding a positive technique without 0x2000 whose
  level_thresholds[i] <= the form's level gets 0x2000; returns the technique & 0x1FFF.
- stfgtrep_get_technique_exp(d, form, exp, n): a = exp * 10 / level (level < 51) or exp / 5; n users: 0 and 1 a,
  2 a * 6 / 10, else a / n; then at least 1, at most 10 while the form's level < t5, else at most 50.
- stfgtrep_member_get_exp(member): + exp / 5 once when item 0x141 is in equipment slot 4 or 5.

A kept setup case puts STFGTREP.PRO in the overlay slot. Fixture: pointer-free gamestate_data zeroed, party {0, 1, 7},
records of Digimon 0 and 7 (level 10, no forms, chosen forms -1); each call writes the record parts it needs.

Hand checks: learn_technique(0) at level 5 with no forms gains row 9 = ID 386 (Digimon 0's list entry 0: level >= 5) and
puts it in chosen slot 0; add_technique_exp of 100 to form 386 at level 1 reaches level 11 (needs 10, 20, ..., 100);
form 150 (row 49, t5 = 60) at level 60 needs 590 + 50 = 640; get_technique_exp(Digimon level 20, exp 100, n 2) =
50 * 6 / 10 = 30; member_get_exp of 103 with item 0x141 = 103 + 20 = 123.

Not tested: a form the Digimon does not own in add_technique_exp/learn_skill/mark_skill/get_technique_exp (the
GamestateForm on the stack is left unset by gamestate_get_form: undefined, and unreachable since the report only passes
chosen forms that went up a level); a Digimon level of 0 (division by zero, unreachable).
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import DIGIMON_SIZE, FORM_SIZE, FORMS, JOINED, OFF, REC, form, rec, stats_bytes, u32
from oracle import Call, Case, Read, Write
from stfgtrep_exp import AFTER, OVERLAY_SLOT, READ_RNG, STFGTREP_PRO

COMMENT = ("STFGTREP.PRO in the slot. stfgtrep_learn_technique on Digimon 0's list: the level, form-level, stat and resist "
           "requirements at their boundaries, two requirements, the first eligible entry, every form owned, the chosen-slot "
           "packing, a full form table, Digimon 7's list; stfgtrep_add_technique_exp at the x10 and x50 thresholds, several "
           "levels, level 99 and the exp cap; stfgtrep_learn_skill and stfgtrep_mark_skill slot order and flags; "
           "stfgtrep_get_technique_exp over the level bands, user counts and caps; stfgtrep_member_get_exp's item bonus.")

# Form IDs (records_digimon[row - 1].id for the 1-based rows StfgtrepLearn names) and their level_thresholds[5] (t5).
F386, F387, F388, F389 = 386, 387, 388, 389   # rows 9, 10, 11, 20 (t5 100)
F5, F259, F260, F254 = 5, 259, 260, 254       # rows 12, 13, 18, 25
F367, F375, F392, F150 = 367, 375, 392, 150   # rows 14 (t5 100), 27 (t5 95), 33, 49 (t5 60)
F20 = 20                                      # row 16: Digimon 7's level-5 form
ALL_D0 = [386, 387, 388, 5, 259, 367, 374, 20, 6, 260, 234, 389, 390, 391, 12, 26, 254, 368, 375, 211, 66, 27, 19, 56,
          392, 393, 394, 213, 148, 369, 376, 214, 196, 144, 267, 359, 378, 372, 230, 59, 150, 381, 377, 151]  # rows 9..52

BASE_STATS = {0: ((48, 44, 41, 34, 33, 1), (85, 125, 80, 80, 115, 100, 95)), 7: ((20, 45, 48, 58, 29, 1), (80, 115, 100, 115, 60, 80, 130))}


def stats_write(d, level=10, boost=None, fire=None):
    s, r = BASE_STATS[d]
    s, r = list(s), list(r)
    note = f"level {level}"
    if boost is not None:
        s[4] = boost
        note += f", Boost {boost}"
    if fire is not None:
        r[0] = fire
        note += f", Fire resist {fire}"
    return Write("gamestate_data", rec(d, REC["stats"]), stats_bytes(level, 0, 300, 300, 50, 50, s, r), f"digimon[{d}].record.stats: {note}")


def forms_write(d, forms):
    """forms: list of (id, level[, exp[, techniques]]) in slots 0.."""
    data = bytearray(FORMS * FORM_SIZE)
    for i, f in enumerate(forms):
        f = list(f)
        if len(f) > 3:
            f[3] = tuple(t - 0x10000 if t >= 0x8000 else t for t in f[3])   # s16: 0x8000 flags as negative
        data[i * FORM_SIZE:(i + 1) * FORM_SIZE] = form(*f)
    desc = ", ".join(f"{f[0]} lv {f[1]}" for f in forms[:6]) + (" ..." if len(forms) > 6 else "")
    return Write("gamestate_data", rec(d, REC["forms"]), bytes(data), f"digimon[{d}].record.forms: {desc or 'none'}")


def chosen_write(d, ids):
    return Write("gamestate_data", rec(d, REC["chosen_forms"]), struct.pack("<3h", *ids), f"digimon[{d}].record.chosen_forms = {tuple(ids)}")


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_flags", 0, bytes(0x1C), "gamestate_flags = 0"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 1, 7), "party = {0, 1, 7}"),
         Write("pad_random", 0, bytes(4), "pad_random.index = 0")]
    for d in (0, 7):
        w.append(Write("gamestate_data", OFF["digimon"] + d * DIGIMON_SIZE + JOINED, u32(d + 3), f"digimon[{d}].joined = {d + 3}"))
        w.append(stats_write(d))
        w.append(chosen_write(d, (-1, -1, -1)))
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C), ("pad_random", 4)]


def read_forms(d, n=8):
    return [Read("gamestate_data", rec(d, REC["chosen_forms"]), 6, f"digimon[{d}].record.chosen_forms after the call"),
            Read("gamestate_data", rec(d, REC["forms"]), n * FORM_SIZE, f"digimon[{d}].record.forms[0..{n}) after the call")]


def case(name, calls, comment=""):
    return Case(name, calls, fixture=FIXTURE, saves=SAVES, comment=comment)


def setup_case():
    """The kept case: STFGTREP.PRO into the overlay slot; nothing else (a kept case restores nothing)."""
    return Case("setup_stfgtrep", [Call("stfgtrep_add_exp", [1, 0], "void",
                                        comment="smoke: the overlay answers (the resident record decides the result)")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STFGTREP.PRO", file=STFGTREP_PRO)],
                keep=True, comment="setup, kept for the family: the overlay in the slot; nothing is restored")


def learn(name, d, writes, desc):
    reads = read_forms(d) + AFTER
    return case(f"learn_{name}", [Call("stfgtrep_learn_technique", [d], "s32", reads, writes=writes, comment=desc)], comment=desc)


def learn_cases():
    out = []
    lv = lambda level, **k: stats_write(0, level, **k)
    f = lambda *forms: forms_write(0, list(forms))
    out.append(learn("level4", 0, [lv(4)], "level 4, no forms: nothing (entry 0, form 386, needs level >= 5) -> 0"))
    out.append(learn("level5", 0, [lv(5)], "level 5, no forms: gains 386 at form level 1, put in chosen slot 0 -> 386"))
    out.append(learn("level19", 0, [lv(19), f((F386, 1))], "level 19 owning 386: nothing (entry 11, form 389, needs level >= 20) -> 0"))
    out.append(learn("level20", 0, [lv(20), f((F386, 1))], "level 20 owning 386: gains 389 (stat 7 = the level, >= 20) -> 389"))
    out.append(learn("form_level19", 0, [f((F386, 1), (F367, 19))], "367 at form level 19: 387 (needs 367 >= 20) not yet -> 0"))
    out.append(learn("form_level20", 0, [f((F386, 1), (F367, 20))], "367 at form level 20: gains 387 -> 387"))
    out.append(learn("boost279", 0, [lv(10, boost=279), f((F386, 1), (F387, 1), (F367, 30))],
                     "388 needs 367 >= 30 and Boost (stat 5) >= 280: Boost 279 -> 0"))
    out.append(learn("boost280", 0, [lv(10, boost=280), f((F386, 1), (F387, 1), (F367, 30))], "Boost 280 -> 388"))
    out.append(learn("fire199", 0, [lv(4, fire=199), f((F375, 50))],
                     "367 needs 375 >= 50 and Fire resist (stat 8) >= 200: Fire 199 -> 0 (level 4: entry 0 not met)"))
    out.append(learn("fire200", 0, [lv(4, fire=200), f((F375, 50))], "Fire resist 200 -> 367"))
    for a, b in ((5, 4), (4, 5), (5, 5)):
        out.append(learn(f"two_req_{a}_{b}", 0, [lv(4), f((F259, a), (F260, b))],
                         f"254 needs 259 >= 5 and 260 >= 5: 259 at {a}, 260 at {b} -> {254 if (a, b) == (5, 5) else 0}"))
    out.append(learn("first_eligible", 0, [lv(20), f((F386, 1), (F367, 20))],
                     "level 20 with 367 at 20: both 387 (entry 1) and 389 (entry 11) qualify; the earlier entry wins -> 387"))
    out.append(learn("all_owned", 0, [lv(99, boost=999, fire=999), f(*[(i, 99) for i in ALL_D0])],
                     "every form of the list owned (level 99, stats 999): -> 0"))
    out.append(learn("chosen_packed", 0, [lv(20), f((F386, 1), (F367, 1)), chosen_write(0, (F386, -1, F367))],
                     "chosen {386, -1, 367}: get_chosen_forms packs it to {386, 367, -1}; 389 goes to slot 2 -> chosen {386, 367, 389}"))
    out.append(learn("chosen_full", 0, [lv(20), f((F386, 1), (F367, 1), (F5, 1)), chosen_write(0, (F386, F367, F5))],
                     "chosen {386, 367, 5} full: 389 is gained but not chosen"))
    out.append(learn("forms_full", 0, [lv(5), f(*[(1, 1)] * FORMS)],
                     "44 form slots taken by ID 1 (below 3: never found): add_form fails, yet the call returns 386 and "
                     "set_chosen_forms writes -1 (386 is not found)"))
    out.append(learn("digimon7_level5", 7, [stats_write(7, 5)], "Digimon 7, level 5: its own list (entry 7: row 16, ID 20) -> 20"))
    return out


def form_cases():
    out = []
    reads = read_forms(0, 2) + AFTER
    for name, fm, exp, add, desc in (
            ("exp_9", (F386, 1, 0), 0, 9, "level 1, exp 0 + 9: 9 < 10 -> 0, exp 9 stored"),
            ("exp_10", (F386, 1, 0), 0, 10, "level 1 + 10: 10 >= 1 * 10 -> level 2, returns 1"),
            ("exp_100", (F386, 1, 0), 0, 100, "level 1 + 100: thresholds 10, 20, ..., 100 -> level 11"),
            ("lv98", (F386, 98, 980), 980, 10, "level 98, 990 >= 980 -> level 99, stops there"),
            ("lv99", (F386, 99, 990), 990, 100, "level 99: returns 0 before put_form, the 100 exp are not stored"),
            ("exp_cap", (F386, 50, 9999990), 9999990, 100, "exp capped 9,999,999; every level to 99 in one call"),
            ("t5_lv59", (F150, 59, 589), 589, 1, "form 150 (t5 60), level 59: 60 <= t5 so needs 590 -> level 60; then 640 not met"),
            ("t5_lv60_639", (F150, 60, 590), 590, 49, "level 60: past t5, needs 59 * 10 + 1 * 50 = 640; 639 -> 0"),
            ("t5_lv60_640", (F150, 60, 639), 639, 1, "level 60: 640 -> level 61 (next needs 690)"),
            ("t5_range", (F150, 59, 0), 0, 1000, "level 59 + 1000: 590, 640, ..., 990 met, 1040 not -> level 68"),
            ("t5_95", (F375, 94, 940), 940, 0, "form 375 (t5 95), level 94 with exp 940: 95 <= t5 needs 940 -> level 95; then 990 not met")):
        fid = fm[0]
        out.append(case(f"form_exp_{name}", [Call("stfgtrep_add_technique_exp", [0, fid, add], "s32", reads,
                                                  writes=[forms_write(0, [fm, (F367, 7, 70)])], comment=desc)], comment=desc))
    return out


def skill_cases():
    out = []
    reads = read_forms(0, 2) + AFTER
    t375 = (157, 158, 150, 151, 217)  # records_digimon row 27 (ID 375): techniques[1..6] 157 158 150 151 217 79,
    # technique_levels 5 25 10 45 55 70, level_thresholds 15 65 35 80 90 95
    for name, lv, techs, desc in (
            ("lv4", 4, (0,) * 6, "375 at level 4: technique 157 needs 5 -> 0"),
            ("lv5", 5, (0,) * 6, "level 5 -> 157 in slot 0"),
            ("skip_slot1", 10, (157, 0, 0, 0, 0, 0), "level 10: slot 1 (158) needs 25, slot 2 (150) needs 10 -> 150 in slot 2"),
            ("one_per_call", 99, (0,) * 6, "level 99, all empty: one technique per call -> 157 only"),
            ("slot5", 70, t375 + (0,), "level 70, slots 0..4 full -> slot 5 = 79 | 0x8000, returns 79"),
            ("all_full", 99, t375 + (79 | 0x8000,), "all six full -> 0")):
        out.append(case(f"skill_{name}", [Call("stfgtrep_learn_skill", [0, F375], "u16", reads,
                                               writes=[forms_write(0, [(F375, lv, 0, techs)])], comment=desc)], comment=desc))
    out.append(case("skill_no_tech5", [Call("stfgtrep_learn_skill", [0, F386], "u16", reads,
                                            writes=[forms_write(0, [(F386, 60, 0, (105, 107, 184, 198, 0, 0))])],
                                            comment="form 386 has no technique [5]: slot 4 stays empty, slot 5 = 61 | 0x8000 at level 60")],
                    comment="a missing records technique leaves its slot empty"))
    for name, lv, techs, desc in (
            ("lv14", 14, (157, 0, 0, 0, 0, 0), "375 at level 14: slot 0 needs threshold 15 -> 0"),
            ("lv15", 15, (157, 0, 0, 0, 0, 0), "level 15 -> slot 0 | 0x2000, returns 157"),
            ("order", 35, (157 | 0x2000, 158, 150, 0, 0, 0), "level 35: slot 0 marked, slot 1 needs 65, slot 2 needs 35 -> 150"),
            ("flag4000", 15, (157 | 0x4000, 0, 0, 0, 0, 0), "a 0x4000 technique is positive: marked (0x6000 | 157), returns 157"),
            ("negative", 99, (157 | 0x8000, 0, 0, 0, 0, 79 | 0x8000), "0x8000 makes the s16 negative: never marked -> 0"),
            ("all_marked", 99, tuple(t | 0x2000 for t in t375) + (79 | 0x8000,), "slots 0..4 marked, slot 5 not looked at -> 0")):
        out.append(case(f"mark_{name}", [Call("stfgtrep_mark_skill", [0, F375], "s32", reads,
                                              writes=[forms_write(0, [(F375, lv, 0, techs)])], comment=desc)], comment=desc))
    return out


def tech_exp_cases():
    out = []
    for name, lv, fm, exp, n, desc in (
            ("lv1", 1, (F386, 1), 12, 1, "Digimon level 1: a = 120 -> capped 10 (form level 1 < t5 100)"),
            ("lv20", 20, (F386, 1), 12, 1, "level 20: a = 120 / 20 = 6"),
            ("lv50", 50, (F150, 60), 14, 1, "level 50: a = 140 / 50 = 2"),
            ("lv51", 51, (F150, 60), 14, 1, "level 51: a = 14 / 5 = 2 (the other formula, same value)"),
            ("lv99", 99, (F150, 60), 300, 1, "level 99: a = 300 / 5 = 60 -> capped 50 (form at t5)"),
            ("n0", 20, (F150, 60), 100, 0, "a = 50; n 0 -> 50"),
            ("n2", 20, (F150, 60), 100, 2, "n 2 -> 50 * 6 / 10 = 30"),
            ("n3", 20, (F150, 60), 100, 3, "n 3 -> 50 / 3 = 16"),
            ("n4", 20, (F150, 60), 100, 4, "n 4 -> 12"),
            ("zero", 20, (F150, 60), 0, 1, "exp 0 -> at least 1"),
            ("floor", 20, (F150, 60), 1, 1, "exp 1: a = 10 / 20 = 0 -> 1"),
            ("cap10", 20, (F150, 59), 120, 1, "a = 60, form level 59 < t5 60 -> 10"),
            ("cap50", 20, (F150, 60), 120, 1, "a = 60, form level 60 = t5 -> 50")):
        out.append(case(f"tech_exp_{name}", [Call("stfgtrep_get_technique_exp", [0, fm[0], exp, n], "s32", AFTER,
                                                  writes=[stats_write(0, lv), forms_write(0, [fm])], comment=desc)], comment=desc))
    return out


# StfgtrepMember (file-local in stfgtrep_80082E70.c, size 0x174): slot 0x5C, exp_gained 0x60, bonus_applied 0x64.
MEMBER_SIZE = 0x174


def member_buf(exp, applied=0):
    b = bytearray(MEMBER_SIZE)
    struct.pack_into("<3i", b, 0x5C, 0, exp, applied)
    return bytes(b)


def member_cases():
    out = []
    for name, equip, applied, desc in (
            ("none", (0,) * 6, 0, "no item: 103"),
            ("slot4", (0, 0, 0, 0, 0x141, 0), 0, "item 0x141 in slot 4: 103 + 103 / 5 = 123"),
            ("slot5", (0, 0, 0, 0, 0, 0x141), 0, "item 0x141 in slot 5: 123"),
            ("slot3", (0, 0, 0, 0x141, 0, 0), 0, "item 0x141 in slot 3: not looked at -> 103"),
            ("applied", (0, 0, 0, 0, 0x141, 0), 1, "bonus already applied: 103, unchanged")):
        w = Write("gamestate_data", rec(0, REC["equipment"]), struct.pack("<6h", *equip), f"digimon[0].record.equipment = {equip}")
        out.append(Case(f"member_exp_{name}", [Call("stfgtrep_member_get_exp", [("buf", "rep_member")], "s32",
                                                    [Read("buf:rep_member", 0x5C, 12, "member: slot, exp_gained, bonus_applied after the call")],
                                                    writes=[w], comment=desc)],
                        fixture=FIXTURE, saves=SAVES, buffers={"rep_member": member_buf(103, applied)},
                        comment="StfgtrepMember: slot 0 (Digimon 0), exp_gained 103"))
    return out


def cases(sym):
    return [setup_case()] + learn_cases() + form_cases() + skill_cases() + tech_exp_cases() + member_cases()
