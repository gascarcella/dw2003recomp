"""Shared fixture code for the card game families (cardgame_cpu_choice, cardgame_rules): a CardgameGame builder that
places cards by card ID, the kept setup case (CARDGAME.PRO in the slot, the card files 0x7F6/0x7F7 loaded) and the
reads. Offsets are the PS1 layout of include/cardgame.h; the host replay remaps the buffer and the buf:game reads past
the Object header (tests/host/replay.py BUFFER_LAYOUTS).

Card indexes follow the game's own split (CardgameBoard.load_cards): 0..39 the player's cards, 40..79 the CPU's
(cpu_cards[index - 40]), 80.. other cards (slots placed by the fixtures); card_ids[index] is the card ID, and a card ID
n is record n & 63 of card file card_files[n >> 6] (card_select(n + 1)). The fixtures use IDs < 128 (files 0x7F6/0x7F7).
Card record bytes (docs/FORMATS.md): 0 the colour 1-6 ("kind" in the C), 1 attack, 2 hp, 3 the class index (0x10 a
Digimon card, 1-14 option cards; card_classes), 5 the level, s16 at 8 a value, s16 at 10 the combination result."""
import struct

from oracle import Call, Case, Read, Write

OVERLAY_SLOT = 0x80082CB0
CARDGAME_PRO = "extracted/disc/AAA/PRO/CARDGAME.PRO"
CARD_FILES = [(0x7F6, 0x80100000, "extracted/disc/AAA/DAT/CARD/CARDPAK0.BIN"),
              (0x7F7, 0x80119000, "extracted/disc/AAA/DAT/CARD/CARDPAK1.BIN")]
SAVES = [("cdload_module", 0x434), ("card_module", 4), ("pad_random", 4)]

SIZE = 0x824
OBJECT = 0x50   # the Object header (pointers): left zero, the host remaps the rest
OFF = dict(card_ids=0x50, card_count=0x244, effect_state=0x2F4, phase=0x2F8, step=0x2F9, timer=0x2FC, round=0x300,
           round_winner=0x301, winner=0x302, swap_card=0x304, swap_count=0x305, next_slot_id=0x308,
           cpu_deck_info=0x30A, cpu_cards=0x35C, cpu_counter_ids=0x400, cpu_deck_end=0x41B, cpu_deck_last=0x41C,
           effect=0x420, new_effect=0x421, effect_step=0x422, next_step=0x423, unk_424=0x424, unk_438=0x438,
           cursor=0x43C, choice=0x440, selected=0x444, target_rows=0x445, selectable=0x446, marked=0x46F,
           resolve_step=0x4DC, cancelled=0x4DD, script_pos=0x4E0, repeat_count=0x4E2, repeat_pos=0x4E4,
           turn=0x575, side=0x579, answer=0x57A, turns=0x580, players=0x59C, slots=0x72C, end=0x810)
PLAYER = dict(attack=0, hp=2, deck_pos=4, discard_count=6, deck_count=8, hand_count=0xA, points=0xC, side=0x11,
              wins=0x12, deck=0x14, hand=0x64, discard=0x78, size=0xC8)
SLOTS_SIZE = 0x72
SLOT_SIZE = 0xE
FMT = {1: "<B", 2: "<h", 4: "<i"}


class Game:
    """A CardgameGame fixture. Cards are given by card ID; each gets a fresh index in its side's range."""

    def __init__(self):
        self.b = bytearray(SIZE)
        self.next = {0: 0, 1: 40, 2: 80}
        self.player_n = {0: {}, 1: {}}

    def put(self, name, value, size=1, extra=0):
        struct.pack_into(FMT[size], self.b, OFF[name] + extra, value)
        return self

    def card(self, cid, owner=2, at=None):
        """A new card index of owner (0 player, 1 CPU, 2 other) holding card ID cid, or index at."""
        if at is None:
            at = self.next[owner]
            self.next[owner] += 1
        struct.pack_into("<h", self.b, OFF["card_ids"] + 2 * at, cid)
        return at

    def pfield(self, side, name, value, size=2, extra=0):
        struct.pack_into(FMT[size], self.b, OFF["players"] + side * PLAYER["size"] + PLAYER[name] + extra, value)
        return self

    def pile(self, side, name, ids, start=0, at=None):
        """Puts cards (IDs) into a player's hand / deck / discard pile from position start; returns their indexes
        (fresh ones, or the indexes at)."""
        idx = [self.card(c, side, at[k] if at else None) for k, c in enumerate(ids)]
        for k, i in enumerate(idx):
            self.pfield(side, name, i, 2, 2 * (start + k))
        return idx

    def hand(self, side, ids, kinds=None, flags=None, at=None):
        """The hand (hand_count set); for the CPU, cpu_cards[index - 40] = {kind, flag}."""
        idx = self.pile(side, "hand", ids, 0, at)
        self.pfield(side, "hand_count", len(ids))
        if side == 1:
            for k, i in enumerate(idx):
                self.b[OFF["cpu_cards"] + (i - 40) * 4] = kinds[k] if kinds else 0
                self.b[OFF["cpu_cards"] + (i - 40) * 4 + 1] = flags[k] if flags else 0
        return idx

    def deck(self, side, ids, pos=0):
        """deck[pos..pos + len) = the cards, deck_pos = pos, deck_count = len."""
        idx = self.pile(side, "deck", ids, pos)
        self.pfield(side, "deck_pos", pos).pfield(side, "deck_count", len(ids))
        return idx

    def discard(self, side, ids):
        idx = self.pile(side, "discard", ids)
        self.pfield(side, "discard_count", len(ids))
        return idx

    def slots(self, side, cards, first_id=None):
        """slots[side] = [(card ID, attack, hp), ...]; slot ids 0x10 * (side + 1) + i unless given."""
        base = OFF["slots"] + side * SLOTS_SIZE
        self.b[base] = len(cards)
        idx = []
        for i, (cid, attack, hp) in enumerate(cards):
            c = self.card(cid)
            idx.append(c)
            sid = (first_id if first_id is not None else 0x10 * (side + 1)) + i
            struct.pack_into("<hhhhhBBBB", self.b, base + 2 + i * SLOT_SIZE, c, 0, 0, attack, hp, side, side, sid, 0)
        return idx

    def turn(self, n, card, side, target_kind=0, target=0, mark=0):
        """turns[n] (the n + 1-th card of the round)."""
        struct.pack_into("<hhBBBB", self.b, OFF["turns"] + 8 * n, card, mark, side, target_kind, target, 0)
        return self

    def flags(self, name, values, start=0):
        for i, v in enumerate(values):
            self.b[OFF[name] + start + i] = v
        return self

    def bytes(self):
        return bytes(self.b)


# The last card file (IDs 256..315: the colour-5 and colour-6 Digimon), for cardgame_rules; entry 63 is also the battle
# family's, which sets it again in its own setup.
CARD_FILE_7FA = (0x7FA, 0x80132000, "extracted/disc/AAA/DAT/CARD/CARDPAK4.BIN", 63)


def setup_case(name, func, args, comment, extra=()):
    """The kept case of a card family: the overlay and the card files, registered as loaded cdload entries; a smoke call
    with no reads (it runs on the resident state)."""
    writes = [Write(OVERLAY_SLOT, 0, b"", "overlay slot: CARDGAME.PRO", file=CARDGAME_PRO)]
    files = [(fid, addr, path, 61 + n) for n, (fid, addr, path) in enumerate(CARD_FILES)] + list(extra)
    for fid, addr, path, entry in files:
        writes.append(Write(addr, 0, b"", f"card file {fid:#x} (records of 0x62C)", file=path))
        writes.append(Write("cdload_module", 4 + entry * 0x10, struct.pack("<hhiiI", 3, 0, fid, 0, addr),
                            f"cdload_module.entries[{entry}] = {{loaded, id {fid:#x}, buffer -> the file}}"))
    return Case(name, [Call(func, args, "s32", comment=comment)], fixture=writes, keep=True,
                comment="setup, kept for the family: the overlay and the card files")


def read(name, size, field=None, extra=0):
    return Read("buf:game", OFF[name] + extra, size, field or f"game.{name}")


STATE = Read("buf:game", OBJECT, OFF["end"] - OBJECT, "CardgameGame 0x50..0x810 (everything but the Object header and methods)")
MARKED = read("marked", 41, "game.marked[41]")
SELECTABLE = read("selectable", 16, "game.selectable[0..16)")
TURNS = read("turns", 24, "game.turns[3]")
CHOICE = read("choice", 4)


def rng(index):
    return Write("pad_random", 0, struct.pack("<i", index), f"pad_random.index = {index}")


READ_RNG = Read("pad_random", 0, 4, "pad_random.index after the call (the draws taken)")
