"""cardgame_cpu_* (src/cardgame/cardgame_cpu.c): the card game's CPU helpers that are pure over a CardgameGame.
docs/MECHANICS.md section 10. The CardgameGame (0x824 bytes, include/cardgame.h) is a scratch buffer passed as the
first argument; card records come from the card files 0x7F6/0x7F7 (64 records of 0x62C each), placed in free RAM and
registered as loaded cdload entries like the enemy file of the battle family."""
import struct

from oracle import Call, Case, Read, Write

COMMENT = ("cardgame_cpu_get_score over boards of 2-6 slot cards (non-combinable option cards, combinable Digimon cards in "
           "pairs, triples and a run of four, masks); cardgame_cpu_get_card_damage/_get_card_heal for every card ID 0..64; "
           "cardgame_cpu_is_target_within and cardgame_cpu_find_kind over a hand. CARDGAME.PRO is in the slot, the card "
           "files 0x7F6/0x7F7 are loaded. Combinable cards (record s16 at +10 non-zero): IDs 69, 72, 75 (records 70, 73, 76 "
           "of file 0x7F7); IDs 0..59 are option cards, 60..63 Digimon cards with that field 0.")

OVERLAY_SLOT = 0x80082CB0
CARDGAME_PRO = "extracted/disc/AAA/PRO/CARDGAME.PRO"
CARD_FILES = [(0x7F6, 0x80100000, "extracted/disc/AAA/DAT/CARD/CARDPAK0.BIN"), (0x7F7, 0x80119000, "extracted/disc/AAA/DAT/CARD/CARDPAK1.BIN")]
G = dict(card_ids=0x50, cpu_cards=0x35C, selectable=0x446, players=0x59C, player_size=0xC8, hand_count=0xA, hand=0x64,
         slots=0x72C, slots_size=0x72, size=0x824)


def game(slots=((), ()), hand=(), kinds=(), selectable=()):
    """A CardgameGame: card_ids[i] = i, slots[side] = [(card, attack, hp), ...], the CPU's hand (card indexes, 40..)
    with cpu_cards[card - 40].kind, selectable[i] flags."""
    b = bytearray(G["size"])
    for i in range(189):
        struct.pack_into("<h", b, G["card_ids"] + 2 * i, i)
    for side, cards in enumerate(slots):
        base = G["slots"] + side * G["slots_size"]
        b[base] = len(cards)
        for i, (card, attack, hp) in enumerate(cards):
            struct.pack_into("<hhhhhBBBB", b, base + 2 + i * 0xE, card, 0, 0, attack, hp, side, side, i + 1, 0)
    p1 = G["players"] + G["player_size"]
    struct.pack_into("<h", b, p1 + G["hand_count"], len(hand))
    for i, card in enumerate(hand):
        struct.pack_into("<h", b, p1 + G["hand"] + 2 * i, card)
        b[G["cpu_cards"] + (card - 40) * 4] = kinds[i]
    for i, v in enumerate(selectable):
        b[G["selectable"] + i] = v
    return bytes(b)


def setup_case():
    writes = [Write(OVERLAY_SLOT, 0, b"", "overlay slot: CARDGAME.PRO", file=CARDGAME_PRO)]
    for n, (fid, addr, path) in enumerate(CARD_FILES):
        writes.append(Write(addr, 0, b"", f"card file {fid:#x} (64 records of 0x62C)", file=path))
        writes.append(Write("cdload_module", 4 + (61 + n) * 0x10, struct.pack("<hhiiI", 3, 0, fid, 0, addr),
                            f"cdload_module.entries[{61 + n}] = {{loaded, id {fid:#x}, buffer -> the file}}"))
    return Case("setup_cardgame", [Call("cardgame_cpu_get_card_damage", [0, 0x12], "s32", comment="smoke: card 0x12 does 60")],
                fixture=writes, keep=True, comment="setup, kept for the family: the overlay and the card files")


SAVES = [("cdload_module", 0x434), ("card_module", 4)]


def score_case(name, side, mask, slots, comment):
    return Case(name, [Call("cardgame_cpu_get_score", [("buf", "game"), side, mask], "s32", comment=comment)],
                buffers={"game": game(slots=slots)}, saves=SAVES)


def cases(sym):
    out = [setup_case()]
    plain = [(1, 5, 5), (2, 4, 6), (3, 3, 3), (4, 2, 2)]
    for mask in (0, 1, 0b0101, 0b1111, 0b1000):
        out.append(score_case(f"score_plain4_m{mask:x}", 1, mask, ((), plain), f"option cards 1-4 (5,5) (4,6) (3,3) (2,2), mask {mask:#x}: no combinations"))
    out.append(score_case("score_side0", 0, 0, (plain, ()), "the same board on side 0"))
    for n in (2, 3, 4, 5, 6):
        board = [(69, 3, 3)] * n
        for mask in (0, 1, 0b11, 0b100):
            out.append(score_case(f"score_same{n}_c69_m{mask:x}", 1, mask, ((), board), f"{n} x card 69 (combinable) (3,3), mask {mask:#x}"))
    out.append(score_case("score_pair_plus", 1, 0, ((), [(69, 3, 3), (2, 4, 6), (69, 5, 1)]), "a pair of 69 split by card 2: the sort groups them"))
    out.append(score_case("score_triple_plus", 1, 0, ((), [(69, 3, 3), (2, 4, 6), (69, 5, 1), (69, 1, 1)]), "a triple of 69 with card 2 between"))
    out.append(score_case("score_two_groups", 1, 0, ((), [(72, 2, 2), (69, 3, 3), (72, 2, 2), (69, 3, 3), (72, 2, 2), (69, 3, 3)]), "two triples (69 and 72)"))
    out.append(score_case("score_two_groups_masked", 1, 0b010101, ((), [(72, 2, 2), (69, 3, 3), (72, 2, 2), (69, 3, 3), (72, 2, 2), (69, 3, 3)]), "two triples, slots 0, 2, 4 masked (the 72s)"))
    out.append(score_case("score_digimon60_x3", 1, 0, ((), [(60, 4, 4)] * 3), "three Digimon cards with a zero combination field: no combo"))
    out.append(score_case("score_quad_masked_mid", 1, 0b0100, ((), [(75, 2, 3)] * 4 + [(1, 9, 9)]), "four 75s and card 1, slot 2 masked: the skip path"))
    out.append(score_case("score_one", 1, 0, ((), [(69, 7, 8)]), "one card"))
    for card in range(0x41):
        out.append(Case(f"card_damage_{card:02x}", [Call("cardgame_cpu_get_card_damage", [("buf", "game"), card], "s32")], buffers={"game": game()}, saves=SAVES))
        out.append(Case(f"card_heal_{card:02x}", [Call("cardgame_cpu_get_card_heal", [("buf", "game"), card], "s32")], buffers={"game": game()}, saves=SAVES))
    g = game(slots=((), [(1, 5, 5), (2, 4, 6), (3, 3, 3)]), selectable=(1, 0, 1))
    for i in range(3):
        for mx in (2, 3, 6, 10):
            out.append(Case(f"within_i{i}_m{mx}", [Call("cardgame_cpu_is_target_within", [("buf", "game"), 1, i, mx], "s32", comment=f"slot {i} (hp {[5, 6, 3][i]}, selectable {[1, 0, 1][i]}), max {mx}")],
                            buffers={"game": g}, saves=SAVES))
    g = game(hand=(40, 41, 42, 43, 44), kinds=(1, 3, 3, 4, 1))
    for kind in range(6):
        out.append(Case(f"find_kind_{kind}", [Call("cardgame_cpu_find_kind", [("buf", "game"), kind], "s32", comment=f"hand kinds {{1,3,3,4,1}}: first index of kind {kind}, or 5")],
                        buffers={"game": g}, saves=SAVES))
    return out
