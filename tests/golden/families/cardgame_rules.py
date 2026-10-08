"""The card game's match rules that run without the board display (CARDGAME.PRO; src/main/card.c for card_get_class):
the card classes and playability, the card data table, the conditions that void a card (cardgame_check_condition), the
card scripts' control effects and pure effects (cardgame_run_effect with no board), the script interpreter's steps
(cardgame_game_resolve_effect), the round's bonuses, sums and combinations, the round winner (cardgame_game_end_round),
the deck shuffle (pad_random) and the deck/hand bookkeeping. docs/MECHANICS.md section 10.

What is not here and why: every effect that animates (attack, discard, draw, points, stat changes, swaps of the
totals, combos) runs through CardgameBoard's methods and the frame timer, so it only runs inside the object update loop
with the GPU state; layer 2 is the place for those. The board argument is 0 in every call here, and the CardgameGameData
argument is a zeroed buffer (data->board = 0), so a step that would touch the board is never reached by these fixtures.
Card files 0x7F6, 0x7F7 and 0x7FA are loaded (0x7FA for the colour-6 Digimon, IDs 275..315)."""
import struct

from _cardgame import (CARD_FILE_7FA, CHOICE, Game, MARKED, OFF, PLAYER, READ_RNG, SAVES, SELECTABLE, SLOTS_SIZE, STATE, TURNS,
                       read, rng, setup_case)
from oracle import Call, Case, Read

COMMENT = ("CARDGAME.PRO in the slot, card files 0x7F6/0x7F7/0x7FA loaded. card_get_class per record class byte; "
           "cardgame_can_play_card per phase x class (phase 6: the colour's points against the level); "
           "cardgame_get_card_data per field; cardgame_check_condition per kind, true and false; cardgame_run_effect "
           "(board 0) for the script control effects 0x02-0x11 at their boundaries and the pure card effects (0x2A, the "
           "mark/choose effects, the draw marking); cardgame_game_resolve_effect's steps; cardgame_game_apply_bonuses for "
           "the five bonus cards and the 0..99 clamp; sum_slots; find_combo; cardgame_game_end_round's winner steps; "
           "cardgame_shuffle_deck at RNG indexes; sort_cards (a FAKE match), the CPU deck's stage bookkeeping, place_cards. "
           "Hand checks: bonus_111_hand4 4 x 10 + 10 = 50; bonus_68_cap 5 x 20 = 100 -> 99; place_cards card 60 -> "
           "(62, 70) from its record; shuffle_count2 takes 80 draws (index 100 -> 180); end_round_10_tie hp 6 = 6 -> the "
           "CPU (winner 1); card_data_36_script_0 = 0x0D. Appended: cardgame_deal_cards (effect 0x14) with a stub board: a "
           "fresh deal, and a held hand with a deck shorter than 6 (read past its end); cardgame_player_add_card_point (the "
           "deal's points: colours 1-5 give a point, capped at 99; colour 6 none); a slot card back to its owner's discard "
           "pile or hand.")

DATA = bytes(0x40)   # a CardgameGameData with data->board = 0 (0x1C bytes on the PS1, wider on the host)
EFFECT = read("effect_state", 1, "game.effect_state")
SCRIPT = read("script_pos", 6, "game.display.script_pos, repeat_count, repeat_pos")
EFFECT_IDS = read("effect", 4, "game.effect, new_effect, effect_step, next_step")
STEP = Read("buf:game", OFF["phase"], 8, "game.phase, step, (pad), timer")
ROUND = read("round", 6, "game.round, round_winner, winner, done, swap_card, swap_count")


def pread(side, name, size, extra=0):
    return Read("buf:game", OFF["players"] + side * PLAYER["size"] + PLAYER[name] + extra, size, f"game.players[{side}].{name}")


def call_case(name, g, func, args, ret, reads, comment, bufs=None):
    buffers = {"game": g.bytes()}
    if bufs:
        buffers.update(bufs)
    return Case(name, [Call(func, args, ret, reads + [STATE], comment=comment)], buffers=buffers, saves=SAVES)


def gcall(name, g, func, args, reads, comment, ret="s32"):
    return call_case(name, g, func, [("buf", "game")] + args, ret, reads, comment)


def class_cases():
    """card_init(pic); card_select(id + 1); card_get_class(): card_classes[record byte 3]."""
    out = []
    firsts = {1: 0, 2: 1, 3: 2, 4: 4, 5: 6, 6: 7, 7: 8, 8: 13, 9: 18, 0xA: 26, 0xB: 24, 0xC: 31, 0xD: 35, 0xE: 36, 0x10: 60}
    for b3, cid in firsts.items():
        out.append(Case(f"class_b3_{b3:02x}", [
            Call("card_init", [("buf", "pic")], "void"),
            Call("card_select", [cid + 1], "void"),
            Call("card_get_class", [], "s32", comment=f"card {cid}: record byte 3 = {b3:#x}")],
            buffers={"pic": bytes(0x100)}, saves=SAVES))
    out.append(Case("class_select_0", [
        Call("card_init", [("buf", "pic")], "void"),
        Call("card_select", [0], "void"),
        Call("card_get_class", [], "s32", comment="card_select(0) (n <= 0) selects the file's first record, card 0: class 1")],
        buffers={"pic": bytes(0x100)}, saves=SAVES))
    return out


def playable_cases():
    out = []
    for name, phase, cid, counts, comment in (
            ("p6_digimon_short", 6, 60, (2, 0, 0, 0, 0), "phase 6: Digimon 60 (colour 1, level 3) with 2 colour-1 points -> 0"),
            ("p6_digimon_exact", 6, 60, (3, 0, 0, 0, 0), "3 points = level 3 -> 1"),
            ("p6_digimon_colour2", 6, 103, (0, 3, 0, 0, 0), "Digimon 103 (colour 2, level 3): counts[1] = 3 -> 1"),
            ("p6_option", 6, 4, (9, 9, 9, 9, 9), "phase 6, option card 4 -> 0"),
            ("p5_class2", 5, 4, (), "phase 5: card 4 (class 2) -> 1"),
            ("p5_class1", 5, 1, (), "phase 5: card 1 (class 1) -> 0"),
            ("p5_digimon", 5, 60, (), "phase 5: a Digimon (class 0) -> 0"),
            ("p7_class1", 7, 1, (), "phase 7: class 1 -> 1"),
            ("p7_class2", 7, 4, (), "phase 7: class 2 -> 1"),
            ("p7_digimon", 7, 60, (), "phase 7: class 0 -> 0"),
            ("p8_class2", 8, 4, (), "phase 8 (round end): -> 0")):
        g = Game().put("phase", phase)
        c = g.card(cid, 0)
        out.append(call_case(f"playable_{name}", g, "cardgame_can_play_card", [("buf", "game"), ("buf", "counts"), c], "s32", [],
                             comment, bufs={"counts": bytes(counts) + bytes(8 - len(counts))}))
    return out


def card_data_cases():
    out = []
    for name, card, field, i, comment in (
            ("36_use", 36, 0, 0, "card 36's use effect 0x70"), ("36_condition", 36, 1, 0, "condition 0"),
            ("36_message", 36, 2, 0, "message 0"), ("36_target_kind", 36, 3, 0, "target kind 9"),
            ("36_script_0", 36, 4, 0, "script[0] = 0x0D"), ("36_script_31", 36, 4, 31, "script[31] = 0x11, the last entry"),
            ("36_script_32", 36, 4, 32, "script[32] = 0, the terminator"), ("36_field5", 36, 5, 0, "field 5: 0"),
            ("0_use", 0, 0, 0, "card 0 (the table's first): 0x91"), ("59_script_1", 59, 4, 1, "card 59 (the last): script[1] = 0x64")):
        out.append(call_case(f"card_data_{name}", Game(), "cardgame_get_card_data", [card, field, i], "u8", [], comment))
    return out


def cond_game(side=0, turn=1):
    g = Game().put("turn", turn)
    g.turn(turn - 1, g.card(4, side), side, 0, 0)
    return g


def full_deck(g, side, ids, pos):
    """deck[pos..40) = ids (padded to the end of the deck by repeating the last)."""
    ids = list(ids) + [ids[-1]] * (40 - pos - len(ids))
    return g.deck(side, ids, pos)


def condition_cases():
    out = []

    def c(name, kind, g, comment):
        out.append(gcall(f"cond_{name}", g, "cardgame_check_condition", [0, kind], [MARKED], comment))

    c("k0", 0, cond_game(), "kind 0: no condition -> 0")
    g = cond_game(); c("k1_empty", 1, g, "kind 1: the other side's hand is empty -> 1 (the card fails)")
    g = cond_game(); g.hand(1, [60], kinds=[0]); c("k1_card", 1, g, "the other side holds a card -> 0")
    g = cond_game(); g.hand(1, [60, 39], kinds=[0, 0]); c("k2_no_colour6_digimon", 2, g, "kind 2: other's hand: Digimon colour 1, option colour 6 -> 1")
    g = cond_game(); g.hand(1, [60, 276], kinds=[0, 0]); c("k2_colour6_digimon", 2, g, "Digimon 276 (colour 6) in other's hand -> 0")
    g = cond_game(); g.hand(1, [24, 27], kinds=[0, 0]); c("k3_all_colour5", 3, g, "kind 3: other's hand all colour 5 -> 1")
    g = cond_game(); g.hand(1, [24, 60], kinds=[0, 0]); c("k3_one_not5", 3, g, "one card not colour 5 -> 0")
    g = cond_game(); c("k3_empty", 3, g, "empty hand -> 1")
    g = cond_game(); c("k4_no_discard", 4, g, "kind 4: side's discard pile empty -> 1")
    g = cond_game(); g.discard(0, [5]); c("k4_discard", 4, g, "one card in it -> 0")
    g = cond_game(); c("k5_no_deck", 5, g, "kind 5: side's deck_count 0 -> 1")
    g = cond_game(); g.deck(0, [5], 39); c("k5_deck", 5, g, "deck_count 1 -> 0")
    g = cond_game(); g.deck(0, [5], 39); c("k6_other_no_deck", 6, g, "kind 6: the other side's deck_count 0 -> 1")
    g = cond_game(); g.deck(1, [5], 39); c("k6_other_deck", 6, g, "other's deck_count 1 -> 0")
    g = cond_game(); full_deck(g, 0, [5, 6], 35); c("k7_no_digimon", 7, g, "kind 7: deck[35..40) options only -> 1")
    g = cond_game(); full_deck(g, 0, [5, 6, 60, 6], 35); c("k7_digimon", 7, g, "a Digimon at 37 -> 0")
    g = cond_game(); full_deck(g, 0, [60, 61], 35); c("k8_all_digimon", 8, g, "kind 8: deck[35..40) Digimon only -> 1")
    g = cond_game(); full_deck(g, 0, [60, 61, 5, 61], 35); c("k8_option", 8, g, "an option card at 37 -> 0")

    def target(name, kind, tk, p, cpu, tgt, comment, side=0):
        g = Game().put("turn", 1)
        if p:
            g.slots(0, p)
        if cpu:
            g.slots(1, cpu)
        g.turn(0, g.card(4, side), side, tk, tgt)
        g.flags("marked", [1] * 12)
        c(name, kind, g, comment)

    S2 = [(60, 5, 5), (61, 5, 5)]
    target("k9_target_gone", 9, 0, S2, S2, 0x30, "kind 9: target id 0x30 is no slot -> 1; marked[0..12) cleared")
    target("k9_target_cpu", 9, 0, S2, S2, 0x21, "target 0x21 (the CPU's slot 1) exists -> 0; marked cleared up to it")
    target("k10_tk1_own", 10, 1, S2, [], 0, "kind 10, target kind 1 (side's own row), side 0 with slots -> 0")
    target("k10_tk1_other_only", 10, 1, [], S2, 0, "target kind 1, only the other row has slots -> 1")
    target("k10_tk1_cpu", 10, 1, [], S2, 0, "target kind 1 played by the CPU, the CPU's row has slots -> 0", side=1)
    target("k10_tk2_other", 10, 2, [], S2, 0, "target kind 2 (the other row) -> 0")
    target("k10_tk3_empty", 10, 3, [], [], 0, "target kind 3 (both rows), no slots -> 1")
    target("k10_tk3_any", 10, 3, [], S2, 0, "target kind 3 with slots -> 0")
    target("k10_tk9", 10, 9, S2, S2, 0, "target kind 9: no arm -> 1")
    g = cond_game(); h = g.hand(0, [5, 6]); g.turn(0, g.card(4, 0), 0, 9, h[1])
    c("k11_in_hand", 11, g, "kind 11: the turn's target (a hand card index) is still in side's hand -> 0")
    g = cond_game(); g.hand(0, [5, 6]); g.turn(0, g.card(4, 0), 0, 9, 77)
    c("k11_gone", 11, g, "target 77 not in the hand -> 1")
    c("k12", 12, cond_game(), "kind 12: no arm -> 0")
    return out


def effect_game(effect, side=0, new=0):
    g = Game().put("turn", 1).put("effect", effect).put("new_effect", new)
    g.turn(0, g.card(7, side), side, 9, 0)
    g.turn(1, 0, side ^ 1)
    return g


def run_effect_cases():
    out = []
    reads = [EFFECT, SCRIPT, TURNS]

    def c(name, g, comment, extra=()):
        out.append(gcall(f"effect_{name}", g, "cardgame_run_effect", [0], reads + list(extra), comment, ret="void"))

    def pos(g, script_pos, repeat_count=0, repeat_pos=0):
        struct.pack_into("<hhh", g.b, OFF["script_pos"], script_pos, repeat_count, repeat_pos)
        return g

    c("02_skip2", pos(effect_game(0x02), 5), "0x02: script_pos 5 -> 7, effect_state 2")
    for op, n in ((0x03, 2), (0x04, 3), (0x05, 5)):
        c(f"{op:02x}_repeat{n}", pos(effect_game(op), 3), f"{op:#04x}: repeat_count {n}, repeat_pos = script_pos 3")
    c("06_loop", pos(effect_game(0x06), 9, 2, 3), "0x06 with repeat_count 2: count 1, back to repeat_pos 3")
    c("06_last", pos(effect_game(0x06), 9, 1, 3), "repeat_count 1: count 0, no jump (stays 9)")
    c("07_cpu", pos(effect_game(0x07, side=1), 4), "0x07, the CPU played the card: skip 1 (4 -> 5)")
    c("07_player", pos(effect_game(0x07, side=0), 4), "the player played it: no skip")
    c("08_cpu", pos(effect_game(0x08, side=1), 4), "0x08, CPU: skip 2")
    for op, n, sign, hand in ((0x09, 10, 9, (10, 11)), (0x0A, 10, -9, (10, 11)), (0x0B, 3, 9, (3, 4)),
                              (0x0C, 3, -9, (3, 4)), (0x0D, 2, 4, (2, 3))):
        for h in hand:
            g = pos(effect_game(op), 20)
            g.pfield(0, "hand_count", h)
            c(f"{op:02x}_hand{h}", g, f"{op:#04x}: side's hand {h} (bound {n}, move {sign:+d})")
    for d in (0, 1):
        g = pos(effect_game(0x0E), 4)
        g.pfield(1, "deck_count", d)
        c(f"0e_deck{d}", g, f"0x0E: other's deck_count {d} (skip 2 if > 0)")
    for name, ids in (("all5", [24, 27]), ("one_not5", [24, 60]), ("empty", [])):
        g = pos(effect_game(0x0F), 8)
        if ids:
            g.hand(1, ids, kinds=[0] * len(ids))
        c(f"0f_{name}", g, f"0x0F: other's hand {ids}: back 4 if a card is not colour 5")
    for n in (5, 6):
        g = pos(effect_game(0x10), 2)
        g.slots(0, [(60, 1, 1)] * n)
        c(f"10_slots{n}", g, f"0x10: side's slots {n} (skip 5 if < 6)")
    c("11_swap_side", effect_game(0x11), "0x11: turns[0].side 0 -> 1")
    c("2a_queue_swap", effect_game(0x2A), "0x2A: swap_count + 1, swap_card = turns[0].card + 1", [ROUND])
    g = effect_game(0x4A)
    g.hand(1, [5, 6, 7], kinds=[0, 0, 0])
    g.flags("selectable", (0, 1, 1))
    c("4a_first_selectable", g, "0x4A: the first selectable card of other's hand -> choice 1", [CHOICE])
    g = effect_game(0x4B, side=1)
    g.deck(0, [5] * 7, 33)
    c("4b_other_player", g, "0x4B played by the CPU: other = the player: choice = its deck_pos 33", [CHOICE])
    g = effect_game(0x4B, side=0).put("cpu_deck_last", 37)
    c("4b_other_cpu", g, "0x4B played by the player: choice = cpu_deck_last 37", [CHOICE])
    g = effect_game(0x5B)
    g.slots(0, [(60, 1, 1)]); g.slots(1, [(61, 1, 1), (62, 1, 1)])
    g.turn(0, g.card(7, 0), 0, 0, 0x21)
    c("5b_mark_target", g, "0x5B: target 0x21 -> marked[7]", [MARKED])
    for name, tk, side in (("tk1_player", 1, 0), ("tk2_cpu", 2, 1), ("tk3", 3, 0)):
        g = effect_game(0x5C, side=side)
        g.slots(0, [(60, 1, 1), (61, 1, 1)]); g.slots(1, [(62, 1, 1)])
        g.turn(0, g.card(7, side), side, tk, 0)
        c(f"5c_{name}", g, f"0x5C: target kind {tk}, side {side}: the area's existing slots", [MARKED])
    # The selectable marks (cardgame_mark_selectable_cards / _slots through their script effects).
    for op, comment in ((0x34, "side's deck from deck_pos: Digimon only"), (0x35, "side's deck: options only"),
                        (0x39, "other's hand: not colour 5"), (0x38, "other's hand: colour-6 Digimon"),
                        (0x3A, "side's discard pile"), (0x3B, "both rows (mode = side 0)"),
                        (0x3C, "the other row"), (0x3D, "side's own row")):
        g = effect_game(op)
        g.deck(0, [60, 5, 276, 24], 36)
        g.hand(1, [24, 60, 276, 39], kinds=[0] * 4)
        g.discard(0, [5, 60])
        g.slots(0, [(60, 1, 1), (24, 1, 1)]); g.slots(1, [(276, 1, 1)])
        c(f"{op:02x}_mark", g, f"{op:#04x}: {comment}", [read("target_rows", 17, "game.target_rows, selectable[0..16)")])
    # Draw marking: new_effect 0x45-0x47 runs cardgame_mark_draw_start, then the update for the side that played.
    reads_draw = [MARKED, read("selected", 2, "game.selected, target_rows")]
    g = effect_game(0, new=0x45); g.deck(0, [5, 6, 7, 8], 36)
    c("45_draw2", g, "0x45, player: 2 from deck_pos 36 -> marked[36], [37]", reads_draw)
    g = effect_game(0, new=0x45); g.deck(0, [5], 39)
    c("45_draw2_end", g, "deck_pos 39: one card, then i = 40 stops it: selected 1, target_rows 1", reads_draw)
    g = effect_game(0, new=0x46); g.pfield(0, "hand_count", 1); g.deck(0, [5, 6, 7, 8], 36)
    c("46_to3_hand1", g, "0x46: 3 - hand 1 = 2 cards", reads_draw)
    g = effect_game(0, new=0x46); g.pfield(0, "hand_count", 4); g.deck(0, [5, 6, 7, 8], 36)
    c("46_to3_hand4", g, "hand 4: max(3 - 4, 0) = 0 cards: nothing marked", reads_draw)
    for name, stages, deck_n in (("stage_front_then_end", [2, 2, 1, 1, 2, 3, 3, 7, 7, 7], 10),
                                 ("short_deck", [2, 2, 2, 2], 4)):
        g = effect_game(0, side=1, new=0x47)
        g.deck(1, [60] * deck_n, 40 - deck_n)
        for k, st in enumerate(stages):
            g.b[OFF["cpu_deck_info"] + 2 * (40 - deck_n + k) + 1] = st
        c(f"47_cpu_{name}", g, f"0x47, CPU, round 0: 6 cards, from the front while the stage is 2, else from 39 down "
          f"(stages {stages})", reads_draw)
    return out


def resolve_cases():
    out = []
    reads = [read("resolve_step", 2, "game.display.resolve_step, cancelled"), EFFECT_IDS, EFFECT, SCRIPT,
             read("turn", 1, "game.turn")]

    def c(name, g, comment, extra=()):
        out.append(call_case(f"resolve_{name}", g, "cardgame_game_resolve_effect", [("buf", "game"), ("buf", "data")], "u8",
                             reads + list(extra), comment, bufs={"data": DATA}))

    def rg(step, card=4, turn=1, side=0):
        g = Game().put("resolve_step", step).put("turn", turn)
        g.turn(turn - 1, g.card(card, side), side, 9, 0)
        return g

    g = rg(0); g.slots(0, [(68, 5, 5)])
    c("0_bonus", g, "step 0: a bonus card (68) on the table -> its values set, new_effect 0x5D, step 1",
      [read("slots", 16, "game.slots[0] head")])
    c("1", rg(1), "step 1 -> new_effect 0x4C (close the gaps), step 2")
    c("2", rg(2), "step 2 -> new_effect 0x5A (recount), step 3")
    c("6_no_condition", rg(6), "step 6, card 4 (condition 0): script_pos 0, step 7")
    g = rg(7, card=36); struct.pack_into("<h", g.b, OFF["script_pos"], 0)
    c("7_first", g, "step 7, card 36 at 0: new_effect 0x0D, script_pos 1, effect_state 1")
    g = rg(7, card=36); struct.pack_into("<h", g.b, OFF["script_pos"], 31)
    c("7_last", g, "card 36 at 31: its last entry 0x11, script_pos 32")
    c("10", rg(10), "step 10 -> new_effect 0x5A, step 11")
    c("11", rg(11), "step 11 -> step 12")
    g = rg(12, side=1); g.discard(1, [5])
    c("12_discard", g, "step 12: card 4 played by the CPU goes on its discard pile (count 2), turn 1 -> 0",
      [pread(1, "discard_count", 2), pread(1, "discard", 4)])
    g = rg(12, card=13)
    c("12_card13_kept", g, "card 13 is not discarded; turn 1 -> 0", [pread(0, "discard_count", 2)])
    g = rg(12, turn=2); g.put("cancelled", 1)
    c("12_cancelled", g, "cancelled: turn 2 -> 0", [pread(0, "discard_count", 2)])
    c("14_done", rg(14), "step 14 with no slots: returns 1")
    g = rg(14); g.slots(1, [(111, 5, 5)])
    c("14_bonus", g, "step 14, bonus card 111 on the CPU's side: new_effect 0x5D, step 15")
    c("15", rg(15), "step 15 -> 0x4C, step 16")
    c("16", rg(16), "step 16 -> 0x5A, step 17")
    c("17", rg(17), "step 17: returns 1")
    return out


def bonus_cases():
    out = []
    reads = [read("slots", 2 * 0x72, "game.slots[2]")]

    def c(name, g, comment):
        out.append(gcall(f"bonus_{name}", g, "cardgame_game_apply_bonuses", [], reads, comment))

    def slot_bonus(g, side, i, attack_bonus, hp_bonus):
        struct.pack_into("<hh", g.b, OFF["slots"] + side * 0x72 + 2 + i * 0xE + 2, attack_bonus, hp_bonus)

    g = Game(); g.slots(1, [(68, 1, 1), (60, 1, 1), (61, 1, 1)])
    c("68_slots3", g, "68 (step 0): side's slots 3 x 20 = 60/60; the others keep (1, 1)")
    g = Game(); g.slots(1, [(68, 1, 1)] + [(60, 1, 1)] * 4)
    slot_bonus(g, 1, 0, 0, 5)
    c("68_cap", g, "5 slots: 100 -> 99, hp 100 + 5 -> 99")
    g = Game(); g.slots(0, [(111, 1, 1)]); g.pfield(0, "hand_count", 4)
    c("111_hand4", g, "111 (step 1): hand 4 x 10 + 10 = 50")
    g = Game(); g.slots(0, [(154, 1, 1)]); g.pfield(0, "discard_count", 3)
    c("154_discard3", g, "154 (step 2): discard 3 x 20 + 10 = 70")
    g = Game(); g.slots(0, [(197, 1, 1), (60, 1, 1)]); g.slots(1, [(60, 1, 1)] * 3)
    c("197_slots5", g, "197 (step 3): all slots 2 + 3 = 5 x 10 = 50")
    g = Game(); g.slots(1, [(240, 1, 1)]); g.pfield(0, "discard_count", 2); g.pfield(1, "discard_count", 3)
    c("240_discards5", g, "240 (step 4): discards 2 + 3 = 5 x 10 + 10 = 60")
    g = Game(); g.slots(0, [(111, 1, 1)]); slot_bonus(g, 0, 0, -100, 30)
    c("111_negative", g, "111 with hand 0 and attack_bonus -100: 10 - 100 -> 0; hp 10 + 30 = 40")
    g = Game(); g.slots(0, [(60, 7, 8)])
    c("no_bonus_card", g, "no bonus card: values kept, returns 1 (a slot exists)")
    c("no_slots", Game(), "no slots: returns 0")
    g = Game(); g.slots(0, [(60, 30, 40), (61, 25, 5)])
    out.append(gcall("sum_slots", g, "cardgame_game_sum_slots", [0], [pread(0, "attack", 4)], "attack 55, hp 45", ret="void"))
    g = Game(); g.pfield(1, "attack", 9).pfield(1, "hp", 9)
    out.append(gcall("sum_slots_empty", g, "cardgame_game_sum_slots", [1], [pread(1, "attack", 4)], "no slots: 0, 0", ret="void"))
    return out


def combo_cases():
    out = []
    reads = [SELECTABLE, read("effect_vars", 4, "game.effect_vars[4] (the combination's card + 1)", 0x10)]

    def c(name, cards, start, comment, side=1):
        g = Game()
        g.slots(side, [(cid, 10, 10) for cid in cards])
        g.flags("selectable", [1] * 6)
        out.append(gcall(f"combo_{name}", g, "cardgame_game_find_combo", [side, start], reads, comment))

    c("none", [60, 61, 62], 0, "no combinable card -> -1, flags untouched")
    c("pair", [69, 69, 60], 0, "a pair of 69 (combinable): j = 1 -> -1")
    c("triple", [69, 60, 69, 69], 0, "three 69 split by 60: sorted 60, 69 x 3: flags the three, effect_vars[4] 193, returns 4")
    c("four", [69, 69, 69, 69], 0, "four 69: all four flagged, returns 4")
    c("two_groups_first", [72, 69, 72, 69, 72, 69], 0, "69 x 3 and 72 x 3: the first group (69s), returns 3")
    c("two_groups_second", [72, 69, 72, 69, 72, 69], 3, "start 3: the 72s, effect_vars[4] 62, returns 6")
    c("noncombinable_triple", [60, 60, 60], 0, "three 60 (record +10 = 0): no combination -> -1")
    c("player_side", [69, 69, 69], 0, "side 0", side=0)
    return out


def end_round_cases():
    out = []
    reads = [STEP, ROUND, EFFECT_IDS, EFFECT]

    def c(name, step, comment, hp=(0, 0), slots=(0, 0), swap=(0, 0), winner=0, combo=False):
        g = Game().put("step", step).put("round_winner", winner).put("swap_card", swap[0]).put("swap_count", swap[1])
        g.pfield(0, "hp", hp[0]).pfield(1, "hp", hp[1])
        for side, n in enumerate(slots):
            if n:
                g.slots(side, [(69 if combo else 60 + k, 5, 5) for k in range(n)])
        out.append(call_case(f"end_round_{name}", g, "cardgame_game_end_round", [("buf", "game"), ("buf", "data")], "s32",
                             reads, comment, bufs={"data": DATA}))

    c("1_combo", 1, "step 1: the player's triple of 69 -> timer 3, new_effect 0x19 (combo)", slots=(3, 0), combo=True)
    c("1_none", 1, "step 1: no combination -> step 2, timer 0", slots=(2, 0))
    c("2_none", 2, "step 2: none for the CPU -> step 8", slots=(0, 2))
    c("8_no_swap", 8, "step 8, swap_card 0, swap_count 0 -> step 9")
    c("8_swap", 8, "swap_card 5, swap_count 2: new_effect 0x1B, count 1, stay in step 8", swap=(5, 2))
    c("8_last_swap", 8, "swap_count 1 -> 0, step 9", swap=(5, 1))
    c("9_player_empty_wins", 9, "step 9, the player has no slots, hp 5 > 3: winner 0, and the winner's row is empty "
      "while the CPU's is not -> step 10", hp=(5, 3), slots=(0, 2))
    c("9_player_empty_loses", 9, "hp 3 < 5: winner 1 -> step 11", hp=(3, 5), slots=(0, 2))
    c("9_both_empty_tie", 9, "no slots, hp 4 = 4: the tie goes to the CPU (winner 1) -> step 11", hp=(4, 4))
    c("9_cpu_empty", 9, "the CPU has no slots, hp 6 > 2: winner 0 -> step 11", hp=(6, 2), slots=(2, 0))
    c("10_player", 10, "step 10, hp 7 > 6: new_effect 0x18 (the CPU's slots go), winner 0", hp=(7, 6))
    c("10_tie", 10, "hp 6 = 6: new_effect 0x17, winner 1", hp=(6, 6))
    c("10_cpu", 10, "hp 2 < 9: 0x17, winner 1", hp=(2, 9))
    c("18_player", 18, "step 18, winner 0: new_effect 0x1C, step 26", winner=0)
    c("18_cpu", 18, "winner 1: 0x1D", winner=1)
    c("25", 25, "step 25: returns 2 (the match is over)")
    c("26", 26, "step 26: returns 1 (next round)")
    return out


def deck_cases():
    out = []
    deck = [5 + (k % 50) for k in range(40)]
    DECK0 = pread(0, "deck", 80)

    def shuffle(name, start, count, index, comment):
        g = Game()
        g.deck(0, deck, 0)
        out.append(Case(f"shuffle_{name}", [Call("cardgame_shuffle_deck", [("buf", "game"), start, count], "void",
                                                 [DECK0, READ_RNG], comment=comment, writes=[rng(index)])],
                        buffers={"game": g.bytes()}, saves=SAVES))

    shuffle("count0", 0, 0, 100, "count 0: no draws, deck unchanged")
    shuffle("count1", 39, 1, 100, "count 1: no draws")
    shuffle("count2", 38, 2, 100, "deck[38..40): 40 swaps, 80 draws (index 100 -> 180)")
    shuffle("full_rng0", 0, 40, 0, "the whole deck at RNG index 0")
    shuffle("full_rng4090", 0, 40, 4090, "at index 4090: the index wraps past 4095")
    shuffle("tail", 30, 10, 777, "deck[30..40) only")
    g = Game()
    cards = [g.card(cid) for cid in (69, 5, 300, 60, 5, 1)]
    for n in range(6):
        g.b[OFF["cpu_deck_info"] + 2 * n] = 10 + n
    g.flags("selectable", (1, 0, 1, 0, 0, 1))
    buf = struct.pack("<6h", *cards)
    for name, rng_, flags, comment in (("all_f0", 6 << 16, 0, "cards with IDs 69 5 300 60 5 1, sorted by ID; the sort swaps (not stable): the two 5s come out reversed"),
                                       ("all_f1", 6 << 16, 1, "flags 1: cpu_deck_info[i] swapped along"),
                                       ("all_f3", 6 << 16, 3, "flags 3: cpu_deck_info and selectable swapped along"),
                                       ("range_2_5", 2 | 5 << 16, 2, "cards[2..5) only; selectable indexed from start")):
        out.append(Case(f"sort_{name}", [Call("cardgame_sort_cards", [("buf", "game"), ("buf", "cards"), rng_, flags], "void",
                                              [Read("buf:cards", 0, 12, "the cards"), STATE], comment=comment)],
                        buffers={"game": g.bytes(), "cards": buf}, saves=SAVES))
    # The CPU's hand order (cardgame_game_sort_cpu_hand: kind, then order).
    g = Game()
    idx = g.hand(1, [5, 6, 7, 8], kinds=[3, 1, 3, 1])
    for k, o in zip(idx, (305, 210, 301, 205)):
        struct.pack_into("<h", g.b, OFF["cpu_cards"] + (k - 40) * 4 + 2, o)
    out.append(gcall("sort_cpu_hand", g, "cardgame_game_sort_cpu_hand", [], [pread(1, "hand", 8)],
                     "kinds 3 1 3 1, orders 305 210 301 205 -> 43 41 42 40", ret="void"))
    # The CPU deck's stages: limits per round, the reset between rounds, the restore after a choice.
    def cpu_deck(stages, pos=0, rnd=0, hand=()):
        g = Game().put("round", rnd)
        g.deck(1, [60 + (k % 8) for k in range(40 - pos)], pos)
        for k, st in enumerate(stages):
            g.b[OFF["cpu_deck_info"] + 2 * k] = k
            g.b[OFF["cpu_deck_info"] + 2 * k + 1] = st
        if hand:
            g.hand(1, list(hand), kinds=[0] * len(hand))
        return g

    stages = [1] * 6 + [2] * 6 + [3] * 8 + [4] * 8 + [5] * 8 + [7] * 4
    deck_reads = [read("cpu_deck_info", 80, "game.cpu_deck_info"), read("cpu_deck_end", 2, "cpu_deck_end, cpu_deck_last"),
                  pread(1, "deck_pos", 8), pread(1, "deck", 80)]
    for rnd in (0, 1):
        g = cpu_deck(stages, 6, rnd)
        out.append(call_case(f"cpu_deck_limits_round{rnd}", g, "cardgame_game_set_cpu_deck_limits",
                             [("buf", "game"), ("buf", "data")], "void", deck_reads,
                             f"round {rnd}: end = first stage > {rnd * 2 + 2} from deck_pos 6; last = 40 - four stage-7 cards = 36",
                             bufs={"data": DATA}))
    g = cpu_deck(stages, 12, 1, hand=(100, 101)).put("cpu_deck_last", 36)
    out.append(call_case("cpu_deck_reset", g, "cardgame_game_reset_cpu_deck", [("buf", "game"), ("buf", "data")], "void",
                         deck_reads + [pread(1, "hand_count", 2)],
                         "round 1, hand of 2 back on the deck (deck_pos 10, stage 3), then every card with stage <= 4 at "
                         "deck_pos rotates to cpu_deck_last - 1 as stage 7; pos renumbered", bufs={"data": DATA}))
    g = cpu_deck([7 - (k % 3) for k in range(40)], 30).put("cpu_deck_last", 36).put("choice", 33, 4)
    for k in range(40):
        g.b[OFF["cpu_deck_info"] + 2 * k] = 39 - k
    out.append(gcall("cpu_deck_restore", g, "cardgame_restore_cpu_deck", [], deck_reads + [CHOICE],
                     "deck[30..40) sorted back by pos (here reversed), then the chosen card 33 swapped with cpu_deck_last 36",
                     ret="void"))
    # Placing the chosen hand cards (cardgame_game_place_cards): at most `selected`, every marked card leaves the hand.
    for name, sel in (("all", 3), ("capped", 2)):
        g = Game().put("selected", sel).put("next_slot_id", 0x40)
        g.hand(0, [60, 5, 61, 62, 6])
        g.flags("marked", (1, 0, 1, 1, 0))
        out.append(gcall(f"place_cards_{name}", g, "cardgame_game_place_cards", [0],
                         [read("slots", 0x72, "game.slots[0]"), pread(0, "hand_count", 2), pread(0, "hand", 10),
                          read("next_slot_id", 1)],
                         f"hand 60 5 61 62 6, marked 1 0 1 1 0, selected {sel}: slots from the records' attack/hp, ids from "
                         f"0x40; marked cards leave the hand" + (" (the third is lost)" if sel == 2 else ""), ret="void"))
    return out


def deal_cases(sym):
    """cardgame_deal_cards(game, board) (effect 0x14, the deal): effect_time, effect_vars[0], [3] = 0, effect_vars[1], [2] = the two deck counts;
    for each side, 6 times: hand[k] = deck[deck_pos], hand_count++, deck_pos++, deck_count-- (k from 0 whatever the hand
    held; no check of the deck's end); then the board shows the 12 cards (a stub board: every method heap_nop)."""
    board = struct.pack("<I", sym["heap_nop"]) * (0xF48 // 4)
    reads = [pread(0, "deck_pos", 8), pread(0, "hand", 12), pread(1, "deck_pos", 8), pread(1, "hand", 12),
             read("effect_time", 0x14, "game.effect_time..effect_vars[3]")]
    out = []
    g = Game().put("effect_time", 7, 4)
    g.deck(0, list(range(60, 70)))
    g.deck(1, list(range(70, 80)))
    out.append(call_case("deal_round", g, "cardgame_deal_cards", [("buf", "game"), ("buf", "board")], "void", reads,
                         "decks of 10 from position 0: hands = the first 6, deck_pos 6, deck_count 4, hand_count 6; "
                         "effect_vars[1], [2] = 10", {"board": board}))
    g = Game()
    g.deck(0, [60 + k % 10 for k in range(40)])
    g.pfield(0, "deck_pos", 30).pfield(0, "deck_count", 10)
    g.hand(0, [61, 62])
    g.deck(1, [70 + k % 10 for k in range(40)])
    g.pfield(1, "deck_pos", 37).pfield(1, "deck_count", 3)
    out.append(call_case("deal_held_and_short", g, "cardgame_deal_cards", [("buf", "game"), ("buf", "board")], "void", reads,
                         "player: 2 cards already held: hand[0..5] overwritten, hand_count 8; CPU: 3 cards left at 37: "
                         "deck[40..42] are past the deck: they are hand[0..2], which the deal "
                         "has just written, so the three cards come again; deck_count -3", {"board": board}))
    return out


def points_cases(sym):
    """cardgame_player_add_card_point(game, board, side, card) (the deal's points step: each of the 6 dealt cards of each
    side, cardgame_80085DE8.c): a card of colour 1-5 (record byte 0) gives its owner a point of that colour, capped at 99,
    and returns 1 (the board flashes it); a colour-6 card gives nothing and returns 0 (stub board, as deal_cases).
    Hand checks: card 60 (colour-1 Digimon) points[0] 0 -> 1; card 24 (colour-5 option) at 99 stays 99 and still
    returns 1; card 30 (colour-6 option) returns 0, nothing changes."""
    board = struct.pack("<I", sym["heap_nop"]) * (0xF48 // 4)
    out = []
    for name, side, cid, points, comment in (
            ("card_point_colour1", 0, 60, (0, 0, 0, 0, 0), "card 60, colour 1: points[0] 0 -> 1, returns 1"),
            ("card_point_cap99", 1, 24, (5, 0, 0, 0, 99), "the CPU's card 24, colour 5, points[4] at 99: stays 99, returns 1"),
            ("card_point_colour6", 0, 30, (3, 3, 3, 3, 3), "card 30, colour 6: no point, returns 0")):
        g = Game()
        idx = g.card(cid, side)
        for k, v in enumerate(points):
            g.pfield(side, "points", v, 1, k)
        out.append(call_case(name, g, "cardgame_player_add_card_point", [("buf", "game"), ("buf", "board"), side, idx], "s32",
                             [pread(side, "points", 5)], comment, {"board": board}))
    # A slot card leaves the board for its owner's pile (CardgameSlot.owner), not the pile of the side it sits on.
    g = Game()
    g.discard(0, [61, 62])
    g.slots(1, [(60, 5, 5)])
    g.b[OFF["slots"] + 1 * SLOTS_SIZE + 2 + 0xA] = 0
    out.append(call_case("slot_to_discard_by_owner", g, "cardgame_slot_to_discard", [("buf", "game"), ("buf", "board"), 1, 0], "void",
                         [pread(0, "discard_count", 2), pread(0, "discard", 6), pread(1, "discard_count", 2)],
                         "the CPU's slot 0 holds the player's card (owner 0): it goes to the player's discard pile (count 2 -> 3)",
                         {"board": board}))
    g = Game()
    g.hand(1, [70])
    g.slots(0, [(60, 5, 5)])
    g.b[OFF["slots"] + 2 + 0xA] = 1
    out.append(call_case("slot_to_hand_by_owner", g, "cardgame_slot_to_hand", [("buf", "game"), ("buf", "board"), 0, 0], "void",
                         [pread(1, "hand_count", 2), pread(1, "hand", 4), pread(0, "hand_count", 2)],
                         "the player's slot 0 holds a CPU card (owner 1): it goes back to the CPU's hand (count 1 -> 2)",
                         {"board": board}))
    return out


def cases(sym):
    out = [setup_case("setup_cardgame_rules", "cardgame_cpu_get_card_damage", [0, 0x12], "smoke: card 0x12 does 60",
                      extra=[CARD_FILE_7FA])]
    out += class_cases()
    out += playable_cases()
    out += card_data_cases()
    out += condition_cases()
    out += run_effect_cases()
    out += resolve_cases()
    out += bonus_cases()
    out += combo_cases()
    out += end_round_cases()
    out += deck_cases()
    out += deal_cases(sym)
    out += points_cases(sym)
    return out
