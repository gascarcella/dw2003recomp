"""The card game CPU's decisions (src/cardgame/cardgame_cpu.c, and its choices inside effects in cardgame_80085DE8.c):
cardgame_cpu_choose_card and the helpers it runs (cardgame_cpu_can_use_effect, _choose_target, _choose_counter_target,
_keep_best_target, _filter_targets, _set_target, _set_target_kind_3/_4, _set_cheapest_target), and the CPU's picks
when an effect asks it to choose a card (cardgame_cpu_choose_card_by_value, _choose_own_deck_card,
_choose_player_deck_card), and two choices of the effect interpreter: cardgame_mark_best_cpu_slot (effect 0xAE on the
CPU's turn) and cardgame_cpu_select_cards (effect 0x9D). docs/MECHANICS.md section 10.

None of them draws a random number and none touches the board display (the board argument is 0): each is a function
of the CardgameGame alone, so a case is one fixture and one call. Card facts the cases rely on (card files 0x7F6/0x7F7,
cardgame_card_data): ID 1 colour 1 class 1 use 0x70; ID 3 colour 1 class 1 use 0x85, heal 30; ID 4 colour 1 class 2
use 0x73; ID 8 colour 2 class 2 use 0x83; ID 12 colour 3 (option); ID 13 colour 3 class 2 use 0x81; ID 18 colour 4
record kind 9; ID 21 colour 4 record kind 9 use 0x88, damage 30; ID 24 colour 5; ID 39 colour 6 use 0x96; ID 40
colour 6 use 0x87; ID 46 colour 6; ID 47 colour 6 class 2 use 0x84; Digimon 60 (v8 7), 61 (17), 62 (29), 64 (44),
65 (45), all colour 1. The card files hold Digimon of colours 1 and 2 only, so where a slot needs colour 3-6 it holds
an option card of that colour (the slot code reads only the colour byte)."""
import struct

from _cardgame import (CHOICE, Game, MARKED, OFF, PLAYER, SAVES, SELECTABLE, STATE, TURNS, read, setup_case)
from oracle import Call, Case

COMMENT = ("CARDGAME.PRO in the slot, the card files 0x7F6/0x7F7 loaded. cardgame_cpu_can_use_effect for every use effect "
           "0x70-0x97 (and 0) on a full and an empty board, the colour masks on two masked boards; _choose_target per "
           "card arm (found / not found); the targeting helpers' ties and fallbacks; _choose_counter_target per target kind "
           "and side; cardgame_cpu_choose_card per decision (phase 5 kind 1, phase 7 kind 3 behind the swap and score "
           "gates, the counter list, the record-kind 3/9 answers); the CPU's picks inside effects. Reads: the return "
           "value, the fields written (marked, selectable, turns, choice) and a SHA-1 of the whole CardgameGame state. Appended: "
           "cardgame_mark_best_cpu_slot (the CPU slot whose loss costs the least score, the first of a tie, an empty "
           "slot when the CPU has fewer than six, none on an empty side) and cardgame_cpu_select_cards in phase 6 (greedy in hand order against the points left, kind 5 "
           "skipped, the cap of 6 selected, option cards not playable; the board is a stub of heap_nop methods). Hand "
           "checks: target_7_best removes the slot worth 65 of 123 and targets it (0x11); counter_k0_cpu1_saved: slot hp 12, "
           "damage 30, heal 30 -> 12 <= 30 and not 12 <= 0 -> targeted (0x20); by_value_cpu_hand_highest_tie: values 44, "
           "45, 45 -> the later 45 (2).")

USE_EFFECTS = [0x00, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
               0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98]
CONDITIONAL = [0x71, 0x72, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8E, 0x8F,
               0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96]
SEL_ROWS = read("target_rows", 17, "game.target_rows and selectable[0..16)")
P_SLOTS = [(60, 20, 25), (61, 30, 35), (62, 5, 8)]   # player's slots 0x10-0x12: a+h 45, 65, 13 (score 123)


VOID = {"cardgame_cpu_keep_best_target", "cardgame_cpu_choose_card_by_value", "cardgame_cpu_choose_own_deck_card",
        "cardgame_cpu_choose_player_deck_card"}


def case(name, g, func, args, reads, comment):
    return Case(name, [Call(func, [("buf", "game")] + args, "void" if func in VOID else "s32", reads + [STATE], comment=comment)],
                buffers={"game": g.bytes()}, saves=SAVES)


def board_full():
    """Every pile non-empty, slots of colours 6 and 3 (player) and 6 and 1 (CPU), turn 1 after a colour-6 card."""
    g = Game().put("phase", 7).put("turn", 1)
    g.hand(0, [60, 1]); g.deck(0, [61, 2]); g.discard(0, [3])
    g.hand(1, [64, 4], kinds=[2, 1]); g.deck(1, [62, 5]); g.discard(1, [6])
    g.slots(0, [(46, 5, 5), (12, 5, 5)]); g.slots(1, [(40, 5, 5), (64, 5, 5)])
    played = g.card(47, 0)
    g.turn(0, played, 0, 9, 0)
    return g


def board_empty():
    return Game().put("phase", 7)


def board_mask():
    """Player's slot only colour 5, CPU's only colour 4; the CPU's hand without Digimon; turn 1 after a colour-1 card."""
    g = Game().put("phase", 7).put("turn", 1)
    g.hand(0, [60]); g.hand(1, [4, 1], kinds=[1, 1])
    g.slots(0, [(24, 5, 5)]); g.slots(1, [(18, 5, 5)])
    g.turn(0, g.card(1, 0), 0, 9, 0)
    return g


def board_mask4():
    g = Game().put("phase", 7)
    g.slots(0, [(18, 5, 5)]); g.slots(1, [(18, 5, 5)])
    return g


def can_use_cases():
    out = []
    for name, build, effects in (("empty", board_empty, USE_EFFECTS), ("full", board_full, CONDITIONAL),
                                 ("mask", board_mask, [0x71, 0x83, 0x84, 0x85, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x91,
                                                       0x92, 0x93, 0x94, 0x96]),
                                 ("mask4", board_mask4, [0x91, 0x92, 0x93, 0x94])):
        for e in effects:
            out.append(case(f"can_use_{name}_{e:02x}", build(), "cardgame_cpu_can_use_effect", [0, e], [SEL_ROWS],
                            f"use effect {e:#x} on the {name} board"))
    return out


def target_board(flags=(1, 1, 1), cpu_flags=(0, 0), cpu_slots=((64, 10, 12), (65, 15, 40))):
    g = Game().put("phase", 7)
    g.slots(0, P_SLOTS)
    if cpu_slots:
        g.slots(1, list(cpu_slots))
    g.flags("selectable", flags).flags("selectable", cpu_flags, 6)
    return g


def choose_target_cases():
    out = []
    t = lambda name, g, card, comment: out.append(case(f"target_{name}", g, "cardgame_cpu_choose_target", [0, card],
                                                       [TURNS, SELECTABLE], comment))
    g = target_board(flags=(1, 0, 1, 0))
    g.hand(1, [64, 60, 65])
    t("39_cheapest", g, 39, "card 39: the flagged CPU hand card with the lowest record value: 64 (44) beats 65 (45); 60 (7) is not flagged -> target 40")
    g = target_board(flags=(0, 0, 0))
    g.hand(1, [64, 60, 65])
    t("39_none", g, 39, "card 39 with no flagged hand card -> 0")
    t("7_best", target_board(), 7, "card 7: keep the slot whose removal leaves the lowest score: 123 - 65 = 58 -> slot 1 (0x11)")
    t("7_none", target_board(flags=(0, 0, 0)), 7, "card 7, nothing flagged -> 0")
    t("57_best", target_board(), 57, "card 57 shares card 7's arm -> 0x11")
    t("21_filter30", target_board(), 21, "card 21: drop hp > 30 (slot 1), then the best of 0 (78 left) and 2 (110) -> 0x10")
    t("21_all_survive", target_board(flags=(0, 1, 0)), 21, "card 21, only slot 1 (hp 35) flagged: filtered out, then re-flagged as the last -> 0x11")
    t("56_filter10", target_board(), 56, "card 56: hp <= 10 only: slot 2 -> 0x12")
    t("40_cpu_slot", target_board(cpu_flags=(0, 1)), 40, "card 40: the first flagged CPU slot -> 0x21")
    t("40_none", target_board(), 40, "card 40, no CPU slot flagged -> 0")
    t("3_cpu_slot0", target_board(), 3, "card 3: the CPU's first slot (0x20), flags not read")
    t("3_no_cpu_slots", target_board(cpu_slots=()), 3, "card 3 with no CPU slot -> 0")
    t("4_default", target_board(flags=(0, 0, 0)), 4, "card 4: no target needed -> 1, turns untouched")
    return out


def helper_cases():
    out = []
    g = target_board(flags=(1, 0, 1))
    g.b[OFF["slots"] + 2 + 6:OFF["slots"] + 2 + 10] = (5).to_bytes(2, "little") + (8).to_bytes(2, "little")
    out.append(case("keep_best_tie", g, "cardgame_cpu_keep_best_target", [0], [SELECTABLE],
                    "slots 0 and 2 both (5, 8), both flagged: removing either leaves 78; the later one (2) is kept"))
    out.append(case("keep_best_cpu_side", target_board(cpu_flags=(1, 1)), "cardgame_cpu_keep_best_target", [1], [SELECTABLE],
                    "side 1 uses selectable[6..]: CPU slots (10, 12) and (15, 40): removing slot 1 leaves 22 -> keep slot 1"))
    out.append(case("filter_keep1_none", target_board(flags=(0, 1, 0)), "cardgame_cpu_filter_targets", [0, 30, 1], [SELECTABLE],
                    "keep = 1: slot 1 (hp 35) filtered, nothing re-flagged, returns 1"))
    out.append(case("filter_no_flags", target_board(flags=(0, 0, 0)), "cardgame_cpu_filter_targets", [0, 30, 0], [SELECTABLE],
                    "nothing flagged at all: returns 1 and flags slot 0 (last starts at 0)"))
    out.append(case("filter_boundary", target_board(), "cardgame_cpu_filter_targets", [0, 25, 0], [SELECTABLE],
                    "max 25: slot 0 (hp 25) stays (hp > max is dropped), slot 1 dropped, slot 2 stays -> 0"))
    for k, func in ((3, "cardgame_cpu_set_target_kind_3"), (4, "cardgame_cpu_set_target_kind_4")):
        g = Game().put("phase", 7)
        g.slots(1, [(60, 5, 5), (12, 5, 5), (18, 5, 5)])
        g.flags("selectable", (1, 1, 1), 6)
        out.append(case(f"set_target_kind_{k}", g, func, [1], [TURNS, SELECTABLE],
                        f"CPU slots of colours 1, 3, 4, all flagged: the colour-{k} one (0x2{k - 2})"))
        g = Game().put("phase", 7)
        g.slots(1, [(60, 5, 5)])
        g.flags("selectable", (1,), 6)
        out.append(case(f"set_target_kind_{k}_none", g, func, [1], [TURNS, SELECTABLE], f"no colour-{k} slot -> 0"))
    return out


def counter_board(kind, target, cpu_flags=(1, 1), flags=(1, 1, 1), cpu_slots=((64, 10, 12), (65, 15, 40))):
    g = target_board(flags=flags, cpu_flags=cpu_flags, cpu_slots=cpu_slots).put("turn", 1)
    return g, kind, target


def counter_cases():
    """choose_counter_target(game, card1 = the player's played card, card2 = the CPU's answer, cpu): the damage and heal
    come from get_card_damage/_heal applied to the card INDEXES, not the card IDs (0..39 the player's, 40..79 the CPU's):
    by index the CPU's cards can only do 30 (index 46) or 10 (56) damage and heal 30 (40) or 10 (58)."""
    out = []

    def c(name, kind, target, card1, id1, card2, id2, cpu, comment, **kw):
        g = target_board(**kw).put("turn", 1)
        g.card(id1, at=card1)
        g.card(id2, at=card2)
        g.turn(0, card1, 0, kind, target)
        out.append(case(f"counter_{name}", g, "cardgame_cpu_choose_counter_target", [card1, card2, cpu], [TURNS, SELECTABLE], comment))

    c("k0_cpu0_kill", 0, 0x10, 3, 3, 46, 21, 0, "player healed slot 0x10 (hp 25); damage(46) = 30 >= 25 -> target 0x10")
    c("k0_cpu0_survives", 0, 0x11, 3, 3, 46, 21, 0, "target 0x11 (hp 35) > 30 -> 0")
    c("k0_cpu0_index_not_id", 0, 0x10, 3, 3, 41, 18, 0, "the CPU card is ID 18 (60 damage by ID) at index 41: damage(41) = 0 -> 0")
    c("k0_cpu1_saved", 0, 0x20, 21, 21, 40, 3, 1,
      "player hits CPU slot 0x20 (hp 12) for damage(21) = 30; heal(40) = 30: 12 <= 30 and not 12 <= 0 -> target 0x20 "
      "(selectable[0], not [6], is the flag read)", flags=(1, 0, 0), cpu_flags=(0, 0))
    c("k0_cpu1_heal_too_small", 0, 0x20, 21, 21, 58, 15, 1,
      "ID 15 heals 20 by ID but its index 58 gives 10: 12 <= 30 - 10 -> not saved -> 0", flags=(1, 0, 0), cpu_flags=(0, 0))
    c("k0_cpu1_flag_row", 0, 0x20, 21, 21, 40, 3, 1,
      "as k0_cpu1_saved but only the CPU row flagged (what use effect 0x85 marks): selectable[0] = 0 -> 0", flags=(0, 0, 0), cpu_flags=(1, 1))
    c("k1_cpu0", 1, 0, 2, 2, 46, 21, 0, "kind 1 (a side): filter <= 30, keep the best -> 0x10")
    c("k1_cpu1", 1, 0, 21, 21, 40, 3, 1, "kind 1 for the CPU's answer: no branch -> 0")
    c("k3_cpu1_one_dies", 3, 0, 20, 20, 40, 3, 1, "kind 3 (both sides), damage(20) = 15: CPU slot 0x20 (hp 12) would "
      "die, so a flag survives the filter -> 0", cpu_flags=(1, 1))
    c("k3_cpu1_none_die", 3, 0, 20, 20, 40, 3, 1, "kind 3, damage 15, CPU slots hp 40 and 50 survive: re-flag the last -> 0x21",
      cpu_flags=(1, 1), cpu_slots=((64, 10, 40), (65, 15, 50)))
    c("k3_cpu0", 3, 0, 12, 12, 56, 21, 0, "kind 3 for cpu 0, damage(56) = 10: only slot 2 (hp 8) -> 0x12")
    c("k6_cpu0", 6, 0, 12, 12, 46, 21, 0, "kind 6: filter <= 30, keep the best, then a colour-3 slot: none among the "
      "player's (colours 1) -> 0")
    c("k7_cpu1", 7, 0, 18, 18, 40, 3, 1, "kind 7, damage(18) = 60: keep the best CPU slot (removing slot 1 leaves 80), "
      "filter: hp 80 > 60 so nothing left, slot 1 re-flagged; it is colour 4 (ID 18) -> 0x21",
      cpu_flags=(1, 1), cpu_slots=((64, 10, 70), (18, 15, 80)))
    c("k7_cpu0", 7, 0, 18, 18, 46, 21, 0, "kind 7 for cpu 0: no branch -> 0")
    c("k2_cpu0", 2, 0, 2, 2, 46, 21, 0, "kind 2: no branch -> 0")
    return out


def effect_pick_cases():
    out = []

    def by_value(name, which, mode, ids, sel, comment):
        g = Game().put("effect_vars", which, 4, 0x10)
        side, pile = {5: (0, "hand"), 11: (1, "hand"), 15: (1, "discard"), 0: (0, "hand")}[which]
        if pile == "hand":
            g.hand(side, ids, kinds=[0] * len(ids))
        else:
            g.discard(side, ids)
        g.flags("selectable", sel)
        out.append(case(f"by_value_{name}", g, "cardgame_cpu_choose_card_by_value", [0, mode], [CHOICE], comment))

    by_value("player_hand_lowest", 5, 1, [64, 60, 61], (1, 1, 1), "0xA9: the player's hand, lowest value: 60 (7) -> 1")
    by_value("player_hand_flagged", 5, 1, [64, 60, 61], (1, 0, 1), "60 not selectable: 61 (17) -> 2")
    by_value("cpu_hand_highest_tie", 11, 0, [64, 65, 65], (1, 1, 1), "0xAA: the CPU's hand, highest value, ties to the later: 2")
    by_value("cpu_discard_lowest_tie", 15, 1, [62, 62, 64], (1, 1, 1), "0xAB: the CPU's discard pile, lowest, ties to the first: 0")
    by_value("none_selectable", 5, 1, [64, 60], (0, 0), "nothing selectable -> choice -1")
    by_value("unknown_source", 0, 1, [64, 60], (1, 1), "effect_vars[4] 0 (no source): n = 0 -> -1")

    def own_deck(name, pos, end, sel, comment):
        g = Game().put("cpu_deck_end", end)
        g.deck(1, [60 + (i % 6) for i in range(40 - pos)], pos)
        g.flags("selectable", sel)
        out.append(case(f"own_deck_{name}", g, "cardgame_cpu_choose_own_deck_card", [0], [CHOICE, MARKED], comment))

    own_deck("in_range", 30, 35, (0, 0, 1, 1), "0xAC: deck from 30, end 35: the first selectable in [30, 35) -> 32")
    own_deck("from_end", 30, 32, (0, 0, 0, 0, 0, 1, 0, 1), "none in [30, 32): the last selectable from 39 down -> 37")
    own_deck("none", 30, 32, (), "nothing selectable: the loop stops at deck_pos -> 30")

    def player_deck(name, pos, ids, comment):
        g = Game()
        g.deck(0, ids, pos)
        out.append(case(f"player_deck_{name}", g, "cardgame_cpu_choose_player_deck_card", [0], [CHOICE], comment))

    player_deck("lowest", 36, [64, 61, 60, 60], "0xAD: the lowest value in deck[36..40): 60 (7) first at 38")
    return out


def choose_board(phase, hand, kinds, flags=None, at=None, turn=0, p_slots=((60, 10, 10),), c_slots=((64, 10, 10),)):
    g = Game().put("phase", phase).put("turn", turn)
    idx = g.hand(1, hand, kinds=kinds, flags=flags, at=at)
    g.hand(0, [60])
    if p_slots:
        g.slots(0, list(p_slots))
    if c_slots:
        g.slots(1, list(c_slots))
    return g, idx


def empty_score_cases():
    """cardgame_cpu_get_score with count 0: the sort is skipped and list[0] (a stack slot never written) indexes the
    slots once; the value read is discarded, so the score is 0. The CPU calls it so in phase 7 whenever a side has no
    slot (a player who placed nothing)."""
    out = []
    for side in (0, 1):
        g = Game().put("phase", 7)
        g.slots(side ^ 1, [(60, 10, 10)])
        out.append(case(f"score_empty_side{side}", g, "cardgame_cpu_get_score", [side, 0], [],
                        f"side {side} has no slot -> 0"))
    return out


def choose_card_cases():
    out = []
    reads = [MARKED, TURNS, SELECTABLE]

    def c(name, g, comment):
        out.append(case(f"choose_{name}", g, "cardgame_cpu_choose_card", [0], reads, comment))

    g, _ = choose_board(5, [1, 1, 4], [3, 1, 1])
    c("p5_kind1", g, "phase 5: from the first kind-1 card (1): ID 1 is class 1 (not playable in phase 5), ID 4 is -> marked[2]")
    g, _ = choose_board(5, [4], [3])
    c("p5_no_kind1", g, "phase 5, no kind-1 card in hand -> 0")
    g, _ = choose_board(5, [13, 4], [1, 1])
    c("p5_unusable_then_next", g, "ID 13 (use 0x81) with the CPU's discard pile empty is not usable; ID 4 -> marked[1]")
    g, _ = choose_board(5, [13, 4], [1, 2])
    c("p5_kind_checked_each", g, "after the start every card must still be kind 1: ID 4 is kind 2 -> 0")
    g, _ = choose_board(7, [4], [3])
    g.put("swap_count", 1)
    c("p7_swap_pending", g, "phase 7 with an odd swap_count -> 0 before looking at the hand")
    g, _ = choose_board(7, [4], [3], c_slots=((64, 10, 11),))
    c("p7_cpu_ahead", g, "phase 7, CPU score 21 > player's 20 -> 0")
    g, _ = choose_board(7, [4], [3], c_slots=((64, 10, 9),))
    g.put("swap_count", 2)
    c("p7_behind_swap_even", g, "phase 7, CPU 19 < 20 and swap_count 2 (even): ID 4 -> marked[0]")
    g, _ = choose_board(7, [40], [3], c_slots=((64, 5, 5), (40, 5, 5)))
    c("p7_card40", g, "phase 7, scores equal (20): ID 40 (use 0x87) marks the colour-6 CPU slot 0x21 and targets it")
    g, _ = choose_board(7, [3, 4], [3, 3], c_slots=((64, 5, 5), (65, 5, 5)))
    c("p7_card3_first", g, "ID 3 (use 0x85) is usable and targets the CPU's slot 0 (0x20): marked[0], ID 4 not reached")
    g, _ = choose_board(7, [21], [3], p_slots=P_SLOTS, c_slots=((64, 10, 10),))
    c("p7_card21", g, "ID 21 (use 0x88): the player's slots, hp <= 30, best -> target 0x10")
    g, idx = choose_board(7, [39, 64, 60], [3, 2, 2])
    c("p7_card39", g, "ID 39 (use 0x96) marks the CPU's Digimon in hand (64, 60) and targets the cheapest, 60 (index 42)")
    g, _ = choose_board(7, [40], [3], c_slots=((64, 10, 10),))
    c("p7_card40_unusable", g, "ID 40 with no colour-6 CPU slot -> 0")
    g, _ = choose_board(7, [4], [3], p_slots=(), c_slots=())
    c("p7_no_slots", g, "phase 7 with no slot on either side: both scores read an empty board (get_score's count 0 "
      "reads its uninitialised list[0]; the result is 0) -> equal -> ID 4 marked")

    # Reply turns: turn 1 after the player's card.
    def reply(name, played_id, counters, hand, kinds, flags, comment, at=None, played_at=None, kind=0, target=0, **kw):
        g, _ = choose_board(7, hand, kinds, flags=flags, at=at, turn=1, **kw)
        p = g.card(played_id, 0, played_at)
        g.turn(0, p, 0, kind, target)
        g.flags("cpu_counter_ids", list(counters) + [0xFF])
        c(f"reply_{name}", g, comment)

    reply("counter_kind4", 1, [1], [4, 8], [1, 4], None, "ID 1 is in the counter list: from the first kind-4 card: ID 8 "
          "(use 0x83, turn != 0) -> marked[1]")
    reply("counter_any_after_first4", 1, [1], [47, 4], [4, 1], None, "counter list: ID 47 (0x84 needs a colour-6 card "
          "played; ID 1 is colour 1) fails, then ID 4 (kind 1) is taken: the kind is not checked after the start")
    reply("counter_not_listed", 1, [5], [8], [4], None, "ID 1 not in the list and its record kind is 2 (not 3 or 9) -> 0")
    reply("listed_fallthrough_b3", 2, [2], [21, 47], [3, 4], [1, 0], "ID 2 (record kind 3) listed; the kind-4 card 47 "
          "is not usable, so the record path: ID 21 (kind 9, flagged) answers with cpu 0; its index 46 does 30: "
          "the player's slots hp <= 30, best -> 0x10", at=[46, 47], played_at=2, kind=1, p_slots=P_SLOTS)
    reply("b9_answer_b3_dead_branch", 21, [], [3], [3], [1], "ID 21 (kind 9) hit CPU slot 0x20 (hp 12): ID 3 at index "
          "40 (heal 30) would save it, but use effect 0x85 flags only the CPU row and the counter reads selectable[0] -> 0",
          at=[40], played_at=21, kind=0, target=0x20, c_slots=((64, 10, 12),))
    reply("b3_unflagged", 2, [], [21], [3], [0], "ID 21 answers a kind-3 card but its cpu_cards flag is 0 -> 0",
          at=[46], played_at=2, kind=1, p_slots=P_SLOTS)
    reply("b3_wrong_kind", 2, [], [21], [1], [1], "ID 21's CPU kind is 1 (not 3 or 4) -> 0",
          at=[46], played_at=2, kind=1, p_slots=P_SLOTS)
    reply("b3_answer_not_b9", 2, [], [3], [3], [1], "the answer must be record kind 9 for a kind-3 card: ID 3 is 3 -> 0",
          at=[46], played_at=2, kind=1, p_slots=P_SLOTS)
    return out


GET_SCORE = 0x820          # CardgameGame.get_score: cardgame_cpu_get_score (cardgame_game_init sets it)
BOARD_SIZE = 0xF48         # CardgameBoard
CPU_POINTS = read("players", 5, "players[1].points[5]", PLAYER["size"] + PLAYER["points"])
SELECTED = read("selected", 1, "game.selected")


def best_slot_cases(sym):
    """cardgame_mark_best_cpu_slot(game): marked[0..11] cleared; for i = 6..11 the score of the CPU's side without slot
    i - 6 (game->get_score(game, 1, 1 << (i - 6)): the mask is the slots left out); a strictly higher one moves the mark
    and sets choice = i (start: best 0, slot 6). So the CPU picks the slot whose loss costs least, the first of a tie;
    `selectable` is not looked at. With fewer than six slots, leaving out a slot that does not exist costs nothing: the
    pick is slot count + 6, an empty one (the effect then acts on nothing). With no slot every score is 0: nothing is
    marked, choice is kept."""
    out = []

    def c(name, cpu_slots, comment):
        g = Game().put("phase", 7).put("choice", 0x55, 4).flags("marked", [1] * 12)
        if cpu_slots:
            g.slots(1, list(cpu_slots))
        g.slots(0, [(60, 10, 10)])
        b = bytearray(g.bytes())
        struct.pack_into("<I", b, GET_SCORE, sym["cardgame_cpu_get_score"])
        out.append(Case(f"best_slot_{name}", [Call("cardgame_mark_best_cpu_slot", [("buf", "game")], "void",
                                                   [MARKED, CHOICE, STATE], comment=comment)],
                        buffers={"game": bytes(b)}, saves=SAVES))

    six = [(60, 10, 10), (61, 20, 25), (62, 5, 5), (63, 15, 15), (64, 10, 15), (65, 20, 20)]
    c("six", six, "six slots worth 20, 45, 10, 30, 25, 40 (no combination): leaving out slot 2 (10) keeps the most -> "
      "choice 8, marked[8] only (marked[0..11] were 1)")
    c("six_tie", [six[0], (61, 5, 5)] + six[2:3] + six[3:4] + [(64, 5, 5)] + six[5:],
      "slots 1, 2 and 4 all worth 10: the first of them (7) keeps the mark")
    c("three", six[:3], "three slots (20, 45, 10): leaving out slot 3, which does not exist, keeps all 75 -> choice 9, an "
      "empty slot")
    c("empty_side", (), "no CPU slot: every score 0, nothing marked (not even slot 6), choice stays 0x55")
    return out


def select_cards_cases(sym):
    """cardgame_cpu_select_cards(game, board): for each card of the CPU's hand: marked[i] = 0, then while selected < 6, a
    card cardgame_can_play_card allows against players[1].points (phase 6: a Digimon card whose colour's points >= its
    level) and whose cpu_cards kind is not 5 is selected: its level is taken from the points (cardgame_player_change_points;
    the board's panel update is a stub), marked[i] = 1, selected++. Returns 1."""
    nop = sym["heap_nop"]
    out = []

    def c(name, hand, kinds, points, comment, selected=0, marked=()):
        g = Game().put("phase", 6).put("selected", selected)
        g.hand(1, hand, kinds=kinds)
        g.hand(0, [60])
        for colour, n in points.items():
            g.pfield(1, "points", n, 1, colour - 1)
        if marked:
            g.flags("marked", list(marked))
        out.append(Case(f"select_{name}", [Call("cardgame_cpu_select_cards", [("buf", "game"), ("buf", "board")], "s32",
                                                [MARKED, SELECTED, CPU_POINTS, STATE], comment=comment)],
                        buffers={"game": g.bytes(), "board": struct.pack("<I", nop) * (BOARD_SIZE // 4)}, saves=SAVES))

    c("greedy", [60, 68, 69], [1, 1, 1], {1: 5}, "colour 1 points 5: Digimon 60 (level 3) -> 2 left, 68 (level 4) not "
      "playable, 69 (level 2) -> 0 left; marked 1 0 1, selected 2")
    c("kind5_skipped", [60, 61], [5, 1], {1: 6}, "60 is kind 5: skipped though playable; 61 selected (6 -> 3)")
    c("cap", [69, 70, 71], [1, 1, 1], {1: 10}, "selected starts at 5: only 69 (selected 6), the rest unmarked "
      "(marked[0..2] were 1)", selected=5, marked=(1, 1, 1))
    c("option_and_exact", [1, 103], [1, 1], {2: 3}, "card 1 is an option card (not playable in phase 6); 103 (colour 2, "
      "level 3) with exactly 3 colour-2 points -> selected, 0 left")
    return out


def cases(sym):
    out = [setup_case("setup_cardgame_choice", "cardgame_cpu_get_card_damage", [0, 0x12], "smoke: card 0x12 does 60")]
    out += can_use_cases()
    out += choose_target_cases()
    out += helper_cases()
    out += counter_cases()
    out += effect_pick_cases()
    out += empty_score_cases()
    out += choose_card_cases()
    out += best_slot_cases(sym)
    out += select_cards_cases(sym)
    return out
