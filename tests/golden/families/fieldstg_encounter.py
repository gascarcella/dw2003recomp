"""fieldstg_encounter_reset (src/fieldstg/fieldstg_80087DB0.c): the random-encounter timer's redraw after each encounter
step that reaches 0. docs/MECHANICS.md section 1.

One draw: r = pad_random.next() % 2304; encounter_timer = r below 256, else (r + 256) / 2 (256..1279). The two branches
agree at r = 255 and 256, so the threshold is pinned by r = 254 (254, not 255) and r = 257 (256, not 257); the top is r =
2303 (1279); a draw of 2304 or more wraps (2304 -> 0, 4095 -> r 1791 -> 1023). Each case picks the RNG index whose next
value is the draw named (the table is a permutation). Each case copies FIELDSTG.PRO into the tier-1 overlay slot (a
non-kept file write, restored after the case); the function reads nothing of FIELDSTG's data.
The rest of the encounter rules (the step every 8th moving frame, the rate table, the encounter list) needs the field's
player object and attribute layer: layer 2 (`first_battle_save`, the timer 926 -> 809 on the walk to the trainer).
"""
from oracle import Call, Case, Read, Write

COMMENT = ("fieldstg_encounter_reset at RNG indexes whose next draw is 254, 257, 2303, 2304 and 4095: "
           "encounter_timer = r < 256 ? r : (r + 256) / 2 with r = draw % 2304; FIELDSTG.PRO in the overlay slot.")

OVERLAY_SLOT = 0x80082CB0
FIELDSTG_PRO = "extracted/disc/AAA/PRO/FIELDSTG.PRO"
TIMER = 0x30   # GamestateData.encounter_timer (s32)
# (RNG index before the call, the draw it gives, expected timer).
DRAWS = [(4, 254, 254), (1092, 257, 256), (4083, 2303, 1279), (3028, 2304, 0), (8, 4095, 1023)]


def cases(sym):
    out = []
    for idx, draw, want in DRAWS:
        writes = [Write(OVERLAY_SLOT, 0, b"", "overlay slot: FIELDSTG.PRO (restored after the case)", file=FIELDSTG_PRO),
                  Write("gamestate_data", TIMER, (-1 & 0xFFFFFFFF).to_bytes(4, "little"), "encounter_timer = -1"),
                  Write("pad_random", 0, idx.to_bytes(4, "little"), f"pad_random.index = {idx}")]
        out.append(Case(f"reset_draw{draw}", [Call("fieldstg_encounter_reset", [], "void",
                                                   [Read("gamestate_data", TIMER, 4, "encounter_timer"),
                                                    Read("pad_random", 0, 4, "pad_random.index (one draw)")],
                                                   comment=f"draw {draw}: r {draw % 2304}, timer {want}")],
                        fixture=writes, saves=[("gamestate_data", TIMER + 4), ("pad_random", 4)]))
    return out
