"""The battle's event queue and the copy rule (src/fightstg/fightstg_8008D3B4.c fightstg_events_*, run by WFIGHTMN's
wfightmn_run_events; src/wfightmn/wfightmn_800A6440.c wfightmn_note_copied_tech). docs/MECHANICS.md section 5.

The fightstg_rules battle fixture (tests/golden/families/fightstg_rules.py: FIGHTSTG.PRO in the slot, SDIGIEDT.PRO as
the enemy records, two party members, two enemies). A family of its own (added by sweep3, 2026-10-05) and named to run
last: appending these cases to fightstg_rules moved a vblank into gamestate_flags' check_condition_70_1 (the vsync
handler's playtime word is inside that case's whole-struct read; DECISIONS "Session 10 oracle lessons"), so a family
that runs after every existing one leaves their goldens byte-identical."""
from _battle import BATTLE_STATE, STATE, setup_case
from fightstg_rules import (FIXTURE, GS, READ_RNG, READ_STATE, READ_STATS, SAVES, WFIGHTMN_PRO, WFIGHTMN_SLOT, case, rng)
from oracle import Call, Case, Read, Write

COMMENT = ("The battle's event queue on the fightstg_rules fixture: fightstg_events_get_delay per kind (the turn formula "
           "and its Newton root, the bounds, the random kinds), fightstg_events_take_next's tie rule and take modes, "
           "add_first, a full queue, the boss's final phase; and battle type 4's copy rule wfightmn_note_copied_tech "
           "(WFIGHTMN.PRO in the tier-2 slot).")


def cases(sym):
    import struct
    stats0 = GS["digimon"] + GS["record"] + GS["stats"]

    def party0(level, boost):
        return (Write("gamestate_data", stats0, struct.pack("<h", level), f"digimon[0].record level {level}"),
                Write("gamestate_data", stats0 + 12 + 4 * 2, struct.pack("<h", boost), f"digimon[0].record Boost {boost}"))
    return [setup_case()] + event_queue_cases(party0)



def event_queue_cases(party0):
    """The battle's event queue and its delays (no golden ran them before; MECHANICS section 5):
    - fightstg_events_get_delay(side, kind): kind 0 (a turn; 1 escape and 3..7 blast use the same formula) = base *
      other.Boost / root, root = 10 Newton steps from 999 on own.Boost * other.Boost (not an exact square root: at a
      product k*k - 1 it ends on k, an isqrt gives k - 1), bounded by fightstg_events_delay_formulas[kind] = {base, min,
      max} (0 = no bound): kind 0 {1000, 707, 1414}, 2 {2001, 1001, -}, 3 {2000, -, -}, 8 {-, 2000, 6000}, 9/10 {500,
      1000, -}, 11 {500, 500, -}, 12 {2001, -, -}. Hand checks: member 0 (Boost 75) against 0x20 (Boost 48): root 60,
      1000 * 48 / 60 = 800; the enemy's turn 1000 * 75 / 60 = 1250; Boost 50: product 2400 = 49 * 49 - 1, root 49, 979
      (isqrt: 1000); Boost 999: 1000 * 48 / 218 = 220 -> 707; Boost 1: root 7 (isqrt 6), 6857 -> 1414; kind 3 with Boost
      999: 2000 * 48 / 218 = 440, no bound; kind 2 (regen) next() % 2001 + Spirit * 10: RNG 0 1167 + 700 = 1867, RNG 3333
      38 + 700 -> 1001; kind 8 next() % 8001 (draws are < 4096, so 6000 is never reached): RNG 777 1936 -> 2000, RNG 3333
      2039; kind 12: RNG 0 1167 + 2000 + 700 = 3867; kinds 9, 10, 11 (paralysis, confusion, sleep): next() % 500 +
      (Spirit + status_power) * 8 + 3000 (sleep 1000) - the target's (status + element resist) * 8: RNG 0, status_power
      40 against 0x20 (resists[8] 50 + [4] 160, [9] 50 + [3] 60, [10] 50 + [2] 90): 2368, 3168, 928.
    - fightstg_events_take_next(): the smallest delay, the first one on a tie, except that a turn event (type 2 or 3)
      loses a tie to any later event; every delay drops by the taken one; take mode -1 (types 4, 6, 9) keeps the event,
      1 frees it; an empty queue returns 0. fightstg_events_add_first (through _add_knockout): the new event's delay is
      raised to 2 and added to every queued delay, then it is queued with delay 1. fightstg_events_add with 99 events
      queued drops the new one.
    - fightstg_events_start_final_phase(): event 0x18 at 3000, final_phase 1; enemy 0's modifiers[1] and [3] are set
      (not added) to -stat >> 1 of record 0x1D3's stats[1] and [2]: -750 >> 1 = -375, -730 >> 1 = -365 (both even, so
      the shift's rounding of an odd stat never shows in the game's data).
    fightstg_new_event (the builder the add functions fill) is zeroed first where the call leaves some of its fields.
    - wfightmn_note_copied_tech(side, id) (WFIGHTMN.PRO in the tier-2 slot): in battle type 4 only, for the party
      (side 0), a technique whose anim_script is not 5 or 12 and that has kind 2..8, 11 or >= 13, or defense_stat >= 2,
      becomes state.copied_tech."""
    import struct
    out = []
    EV = 0x1C
    read_events = Read("fightstg_events", 0, 4 * EV, "fightstg_events.events[0..3]")
    read_taken = Read("fightstg_events", 0xAF0, 2, "fightstg_events.taken_type, .taken")

    def queue(*evs):
        b = b"".join(struct.pack("<hh6i", t, d, *(list(a) + [0] * (6 - len(a)))) for t, d, *a in evs)
        return Write("fightstg_events", 0, b + bytes(99 * EV - len(b)), f"fightstg_events.events = {[e[:2] for e in evs]} (type, delay), rest empty")

    def status_power(v):
        return Write("fightstg_events", 0xAF5, bytes([v]), f"fightstg_events.status_power = {v}")

    boost = lambda b: party0(20, b)[1]
    for name, side, kind, writes, comment in (
            ("delay_turn_s0", 0, 0, [], "member 0 (Boost 75) against 0x20 (Boost 48): 1000 * 48 / 60 = 800"),
            ("delay_turn_s16", 0x10, 0, [], "the enemy's turn: 1000 * 75 / 60 = 1250"),
            ("delay_turn_newton", 0, 0, [boost(50)], "Boost 50: product 2400 = 49^2 - 1, the 10 Newton steps end on 49: 979 (isqrt 48: 1000)"),
            ("delay_turn_min", 0, 0, [boost(999)], "Boost 999: 1000 * 48 / 218 = 220, raised to 707"),
            ("delay_turn_max", 0, 0, [boost(1)], "Boost 1: root 7 (isqrt 6), 6857 lowered to 1414"),
            ("delay_blast_unbounded", 0, 3, [boost(999)], "kind 3 (blast): 2000 * 48 / 218 = 440, no bounds"),
            ("delay_regen_r0", 0, 2, [rng(0)], "kind 2: 1167 + Spirit 70 * 10 = 1867"),
            ("delay_regen_min", 0, 2, [rng(3333)], "kind 2: 38 + 700 = 738, raised to 1001"),
            ("delay_kind8_min", 0, 8, [rng(777)], "kind 8: draw 1936, raised to 2000"),
            ("delay_kind8", 0, 8, [rng(3333)], "kind 8: draw 2039"),
            ("delay_paralysis", 0, 9, [status_power(40), rng(0)], "kind 9: 168 + (70 + 40) * 8 + 3000 - 0x20's (50 + 160) * 8 = 2368"),
            ("delay_confusion", 0, 10, [status_power(40), rng(0)], "kind 10: resists[9] + [3] = 50 + 60: 4048 - 880 = 3168"),
            ("delay_sleep", 0, 11, [status_power(40), rng(0)], "kind 11: + 1000, resists[10] + [2] = 50 + 90: 2048 - 1120 = 928"),
            ("delay_modifier_tech", 0, 12, [rng(0)], "kind 12: 1167 + 2000 + 700 = 3867")):
        out.append(case(name, "fightstg_events_get_delay", [side, kind], writes, comment=comment))

    for name, writes, comment in (
            ("take_turn_loses_tie", [queue((9, 500, 0, 0, 20), (2, 300), (3, 300))],
             "poison 500, party turn 300, enemy turn 300: the later enemy turn wins the tie (3, freed); delays 200, 0"),
            ("take_turn_then_status", [queue((2, 300), (9, 300, 0, 0, 20))],
             "party turn 300, poison 300: the poison wins (9, kept: mode -1); delays 0, 0"),
            ("take_status_then_turn", [queue((9, 300, 0, 0, 20), (2, 300))],
             "poison 300, party turn 300: the first (9, kept), the turn does not take a tie from a status event"),
            ("take_empty", [queue()], "empty queue: 0")):
        out.append(case(name, "fightstg_events_take_next", [], writes, reads=(read_events, read_taken), comment=comment))
    new0 = Write("fightstg_new_event", 0, bytes(0x20), "fightstg_new_event = 0 (its unset args would be stale)")
    out.append(case("add_first_knockout", "fightstg_events_add_knockout", [0x10], [new0, queue((2, 500), (3, 800))], reads=(read_events,), ret="void",
                    comment="knockout of the enemy (add_first): queued delays + 2 (502, 802), the event 0x14 at delay 1 in slot 2"))
    full = Write("fightstg_events", 0, struct.pack("<hh6i", 7, 9000, 0, 0, 0, 0, 0, 0) * 99, "99 events {7, 9000}: the queue is full")
    out.append(case("add_full_queue", "fightstg_events_add_party_turn", [100], [full],
                    reads=(Read("fightstg_events", 0, 99 * EV, "fightstg_events.events (hashed)"),), ret="void",
                    comment="a full queue drops the new party turn (no slot, nothing overwritten)"))
    out.append(case("final_phase_start", "fightstg_events_start_final_phase", [], [new0, queue()], reads=(read_events, READ_STATE), ret="void",
                    comment="event 0x18 at 3000, final_phase 1, enemy 0's modifiers[1] and [3] = -(record 0x1D3's stats[1], [2]) >> 1"))

    wf = Write(WFIGHTMN_SLOT, 0, b"", "tier-2 slot: WFIGHTMN.PRO (restored after the case)", file=WFIGHTMN_PRO)
    t4 = Write("fightstg_battle", BATTLE_STATE + STATE["type"], struct.pack("<hh", 4, 0), "state.type = 4 (the enemy copies), copied_tech = 0")
    for name, side, tech, writes, comment in (
            ("copy_kind1", 0, 55, [t4], "technique 55: kind 1, defense_stat < 2: not copied"),
            ("copy_kind1_elemental", 0, 53, [t4], "technique 53: kind 1, defense_stat >= 2: copied"),
            ("copy_anim5", 0, 222, [t4], "technique 222: defense_stat >= 2 but anim_script 5/12: not copied"),
            ("copy_kind2", 0, 94, [t4], "technique 94: kind 2: copied"),
            ("copy_kind9", 0, 61, [t4], "technique 61: kind 9: not copied"),
            ("copy_kind10", 0, 63, [t4], "technique 63: kind 10: not copied"),
            ("copy_kind11", 0, 54, [t4], "technique 54: kind 11: copied"),
            ("copy_kind12", 0, 127, [t4], "technique 127: kind 12: not copied"),
            ("copy_kind13", 0, 211, [t4], "technique 211: kind 13: copied"),
            ("copy_kind9_elemental", 0, 72, [t4], "technique 72: kind 9 but defense_stat >= 2: copied"),
            ("copy_type0", 0, 94, [], "battle type 0: not copied"),
            ("copy_enemy", 0x10, 94, [t4], "the enemy's technique: not copied")):
        out.append(Case(name, [Call("wfightmn_note_copied_tech", [side, tech], "void", [READ_STATE], comment=comment, writes=writes)],
                        fixture=FIXTURE + [wf], saves=SAVES))
    return out
