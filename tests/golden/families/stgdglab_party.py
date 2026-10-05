"""stgdglab_main_pack_party (STGDGLAB, src/stgdglab/stgdglab_8008EB30.c): entering the Digivolution Lab compacts the party.
docs/MECHANICS.md section 7.

member_count = the number of gamestate_data.party[0..2] that are >= 0; then for each slot i holding a negative value, the
first later slot j holding a member moves into i (party[i] = party[j], party[j] = -1): the members keep their order and
the empty slots (any negative value) end up last, as -1 only where a member left. It writes the save data (party).

STGDGLAB.PRO goes in the overlay slot (a kept setup case). Fixture: pointer-free gamestate_data zeroed, the party per
case; buffer `dglab_main` (StgdglabMain, 0x7C bytes: member_count at 0x60, preset to 9).
Hand check: {-1, 3, 5} -> {3, 5, -1}, member_count 2.
"""
import struct

from gamestate_flags import GS_FUNCS, GS_SIZE
from gamestate_records import OFF
from oracle import Call, Case, Read, Write

COMMENT = ("STGDGLAB.PRO in the slot. stgdglab_main_pack_party on a full party, holes at the front, in the middle and at "
           "both ends, a value other than -1 (-2) as a hole, and an empty party: member_count and gamestate_data.party.")

OVERLAY_SLOT = 0x80082CB0
STGDGLAB_PRO = "extracted/disc/AAA/PRO/STGDGLAB.PRO"
MAIN_SIZE, MEMBER_COUNT = 0x7C, 0x60
SAVES = [("gamestate_data", GS_SIZE)]


def setup_case():
    return Case("setup_stgdglab", [Call("stgdglab_get_sprite", [0], "void", comment="smoke: the overlay answers")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: STGDGLAB.PRO", file=STGDGLAB_PRO)],
                keep=True, comment="setup, kept for the family: the overlay in the slot; nothing is restored")


def pack_case(name, party, comment):
    main = bytearray(MAIN_SIZE)
    struct.pack_into("<i", main, MEMBER_COUNT, 9)
    fixture = [Write("gamestate_data", 0, bytes(GS_FUNCS), "gamestate_data[0..funcs) = 0"),
               Write("gamestate_data", OFF["party"], struct.pack("<3i", *party), f"party = {party}")]
    reads = [Read("buf:dglab_main", MEMBER_COUNT, 4, "dglab_main.member_count"),
             Read("gamestate_data", OFF["party"], 12, "gamestate_data.party[3] after the call"),
             Read("gamestate_data", 0, 0x48, "gamestate_data[0..0x48): nothing else written"),
             Read("gamestate_data", 0x4C, GS_FUNCS - 0x4C, "gamestate_data[0x4C..funcs) (SHA-1): nothing else written")]
    return Case(f"pack_{name}", [Call("stgdglab_main_pack_party", [("buf", "dglab_main")], "void", reads, comment=comment)],
                fixture=fixture, saves=SAVES, buffers={"dglab_main": bytes(main)}, comment=f"party {party}")


def cases(sym):
    return [
        setup_case(),
        pack_case("full", (1, 2, 3), "{1, 2, 3}: unchanged, member_count 3"),
        pack_case("front", (-1, 3, 5), "{-1, 3, 5} -> {3, 5, -1}, 2 (the order kept)"),
        pack_case("middle", (4, -1, 6), "{4, -1, 6} -> {4, 6, -1}, 2"),
        pack_case("ends", (-1, 7, -1), "{-1, 7, -1} -> {7, -1, -1}, 1"),
        pack_case("minus2", (-2, 0, -1), "-2 is a hole too: {-2, 0, -1} -> {0, -1, -1}, 1 (Digimon 0 is a member)"),
        pack_case("empty", (-1, -1, -1), "no member: unchanged, member_count 0"),
    ]
