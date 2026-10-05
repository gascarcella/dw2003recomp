"""stfgtrep_add_exp / stfgtrep_raise_stats (src/stfgtrep/stfgtrep_80082E70.c): experience, level-ups and the stat gains
of the battle report. docs/MECHANICS.md section 6.

A kept setup case puts STFGTREP.PRO in the overlay slot (its tables are in the file). The records are
gamestate_data.digimon[i].record with records_digimon[i] as the growth data (the eight party Digimon are the first
eight rows): Digimon 1 (exp_curve 10, so the thresholds are exactly lv^3 + 5 lv - 6 + bonus[band]) carries the
threshold cases; Digimon 0 (exp_curve 8) and 7 (exp_curve 9) the gain grids. The RNG index is set before every
call and read back (the draws taken: 2 for HP/MP, 6 for the stats, 7 for the resists while lv <= 40).

Two reads past a table are part of the recorded behaviour: stfgtrep_stat_gains[k][class + next() % 5] with class 5
(Digimon 1's Power and Guard) and residue 4 reads index 9 of a 9-entry row (the next row's first entry; past the last
row for levels >= 80), and stfgtrep_resist_gains[class + next() % 4] with class 5 and residue 3 reads index 8 of an
8-entry table. The golden records what the original did; a host build may differ there (tests/host/FINDINGS.md).
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import DIGIMON_SIZE, JOINED, OFF, REC, rec, stats_bytes, u32
from oracle import Call, Case, Read, Write

COMMENT = ("STFGTREP.PRO in the slot. stfgtrep_add_exp on Digimon 1 (exp_curve 10) at level 1: exp around the thresholds 12 "
           "and 36, several levels in one call, the 999,999 cap, level 98 -> 99, level 99 (nothing), and on Digimon 0 and 7 with "
           "two RNG indexes; stfgtrep_raise_stats for Digimon 0, 1, 7 x 13 levels at the band edges x 3 RNG indexes, plus a "
           "record at the caps (stats 998, HP/MP 9998).")

OVERLAY_SLOT = 0x80082CB0
STFGTREP_PRO = "extracted/disc/AAA/PRO/STFGTREP.PRO"
LEVELS = [1, 4, 5, 19, 20, 39, 40, 41, 59, 60, 79, 80, 99]
RNG = [0, 1234, 4000]
EXPS = [0, 11, 12, 13, 35, 36, 37, 100, 1000, 5000, 100000, 999999, 1000000, 50000000]

# Base records (records_digimon rows): level 1, HP/MP the start values, the base stats and resists.
BASE = {0: dict(hp=150, mp=0, stats=(48, 44, 41, 34, 33, 1), resists=(85, 125, 80, 80, 115, 100, 95)),
        1: dict(hp=180, mp=10, stats=(56, 58, 19, 17, 49, 1), resists=(80, 115, 105, 120, 85, 115, 60)),
        7: dict(hp=130, mp=170, stats=(20, 45, 48, 58, 29, 1), resists=(80, 115, 100, 115, 60, 80, 130))}


def record_bytes(d, level=1, lv1=0, exp=0):
    b = BASE[d]
    return u32(exp) + stats_bytes(level, lv1, b["hp"], b["hp"], b["mp"], b["mp"], b["stats"], b["resists"])


def fixture():
    w = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
         Write("gamestate_flags", 0, bytes(0x1C), "gamestate_flags = 0"),
         Write("gamestate_data", OFF["party"], struct.pack("<3i", 0, 1, 7), "party = {0, 1, 7}"),
         Write("pad_random", 0, bytes(4), "pad_random.index = 0")]
    for d in BASE:
        w.append(Write("gamestate_data", OFF["digimon"] + d * DIGIMON_SIZE + JOINED, u32(d + 3), f"digimon[{d}].joined = {d + 3}"))
        w.append(Write("gamestate_data", rec(d, REC["exp"]), record_bytes(d), f"digimon[{d}].record: exp 0, level 1, its start HP/MP and base stats"))
    return w


FIXTURE = fixture()
SAVES = [("gamestate_data", GS_SIZE), ("gamestate_flags", 0x1C), ("pad_random", 4)]
# gamestate_data after the call, in two reads around playtime_frames (0x48): the vsync handler (gfx) adds 0x133 to it
# when a call spans a vblank (a level-up to 99 does), which is the game's timer, not the function.
AFTER = [Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48) after the call"),
         Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) after the call (SHA-1)")]
READ_RNG = Read("pad_random", 0, 4, "pad_random.index after the call (the draws taken)")


def read_record(i):
    return Read("gamestate_data", rec(i, REC["exp"]), 0x30, f"digimon[{i}].record.exp and .stats after the call")


def rng(index):
    return Write("pad_random", 0, u32(index), f"pad_random.index = {index}")


def record_write(d, level, lv1=0, exp=0, note=""):
    return Write("gamestate_data", rec(d, REC["exp"]), record_bytes(d, level, lv1, exp), f"digimon[{d}].record: level {level}, values[1] {lv1}, exp {exp}{note}")


def setup_case():
    """The kept case: STFGTREP.PRO into the overlay slot (its tables are inside the file). A kept case restores nothing,
    so it writes nothing else; the smoke call runs on the resident state (its result is not compared on the host)."""
    return Case("setup_stfgtrep", [Call("stfgtrep_add_exp", [1, 0], "void",
                                        comment="smoke: the overlay answers (no experience added; the resident record decides the rest)")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STFGTREP.PRO", file=STFGTREP_PRO)],
                keep=True, comment="setup, kept for the family: the overlay in the slot; nothing is restored")


def case(name, calls, comment=""):
    return Case(name, calls, fixture=FIXTURE, saves=SAVES, comment=comment)


def cases(sym):
    out = [setup_case()]
    reads = lambda d: [read_record(d), READ_RNG] + AFTER
    # add_exp thresholds on Digimon 1 at level 1 (exp_curve 10): level 2 needs exp > 12, level 3 exp > 36.
    for exp in EXPS:
        for idx in (0, 2000):
            out.append(case(f"add_exp_d1_{exp}_rng{idx}", [Call("stfgtrep_add_exp", [1, exp], "s32", reads(1), writes=[rng(idx)],
                                                                 comment=f"add_exp(1, {exp}) at level 1, exp 0, RNG {idx}")],
                            comment="returns 1 if a level was gained; exp capped 999,999; each level raises values[1] by 5 (cap 99) and rolls the gains"))
    for d in (0, 7):
        for exp in (50, 5000, 999999):
            for idx in (0, 2000):
                out.append(case(f"add_exp_d{d}_{exp}_rng{idx}", [Call("stfgtrep_add_exp", [d, exp], "s32", reads(d), writes=[rng(idx)],
                                                                      comment=f"add_exp({d}, {exp}) at level 1, RNG {idx}")]))
    # Near the level cap, and with exp already at the cap.
    for level, lv1, exp, add, desc in ((98, 90, 900000, 999999, "level 98 -> 99: one level, the stats still raised (values[0] = 99 < 100)"),
                                       (99, 99, 999999, 1, "level 99: the loop breaks at lv 100, nothing changes but the capped exp"),
                                       (97, 95, 0, 999999, "level 97 -> 99: two levels"),
                                       (1, 0, 999999, 1, "exp at the cap: stays 999,999, then every level up to 99 in one call"),
                                       (39, 50, 0, 100000, "level 39: the band changes during the call"),
                                       (1, 0, 0, -100, "negative exp: nothing"),
                                       (1, 0, 500, -600, "exp goes negative: no level (the thresholds are positive)")):
        out.append(case(f"add_exp_d1_from_{level}_{exp}_{add & 0xFFFFFFFF:x}", [Call("stfgtrep_add_exp", [1, add], "s32", reads(1),
                                                                                  writes=[record_write(1, level, lv1, exp), rng(0)], comment=desc)], comment=desc))
    # raise_stats grids.
    for d in BASE:
        for lv in LEVELS:
            for idx in RNG:
                out.append(case(f"raise_stats_d{d}_lv{lv}_rng{idx}", [Call("stfgtrep_raise_stats", [d, lv], "void", reads(d), writes=[rng(idx)],
                                                                            comment=f"raise_stats({d}, {lv}) RNG {idx}: HP/MP band by lv (<5, <20, <40), stat band (<5, <20, <40, <60, <80), resists only for lv <= 40")]))
    capped = u32(0) + stats_bytes(50, 50, 9998, 9998, 9998, 9998, (998,) * 6, (998,) * 7)
    for lv in (10, 50):
        for idx in (0, 1234):
            out.append(case(f"raise_stats_capped_lv{lv}_rng{idx}", [Call("stfgtrep_raise_stats", [0, lv], "void", reads(0),
                                                                          writes=[Write("gamestate_data", rec(0, REC["exp"]), capped, "digimon[0].record: HP/MP 9998, stats and resists 998"), rng(idx)],
                                                                          comment="caps: max HP/MP 9999, stats and resists 999")]))
    return out
