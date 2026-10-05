"""The battle's spoils: the item drop and HP write-back at the battle's end (src/wfightmn/wfightmn_800A6440.c
wfightmn_battle_end, substep 0) and the special enemies' prize chosen at its start (src/fieldstg/fieldstg_80087DB0.c
fieldstg_start_battle; see prize_case). docs/MECHANICS.md section 5. Added by sweep5 (2026-10-05); a family of its own named to
run after gamestate_flags (DECISIONS "Third sweep": cases added before it can move a vblank into its whole-struct read).

The fightstg_rules battle fixture (FIGHTSTG.PRO in the slot, SDIGIEDT.PRO as the enemy records; party {0, 3, -1}:
member 0 Kotemon 300 HP / 100 MP, member 1 Digimon 3 350 HP / 120 MP), WFIGHTMN.PRO copied into the tier-2 slot per case.
The function creates its fade object in the same step (fightstg_fade_create -> object_new -> heap_funcs.alloc_zero): the
oracle points heap_funcs.first/end at a 0x100-byte scratch arena (one free block and its terminator), so the object is
allocated there and both are restored after the case; the host skips those writes and allocates with libc. The main
object (`obj`, only base.substep read and written) and its data block (`data`: intro_camera NULL) are zeroed buffers.

The rule (substep 0, result = fightstg_events.result):
- result != 0 (1 won, 2 lost): results.battle = records_state.battle, results.member = state.current[0];
  count = the enemies with digimon != 0 and item != 0 (a stolen item is -1 and counts; an empty slot does not, whatever
  its item); pick = next() % count; the holder of rank `pick` is found, but the item and record read are enemies[rank],
  not that holder's slot (rank used as a slot index); item > 0: it drops when next() % 1024 < drop_rate + 1, else (or
  item <= 0) results.item = 0, no second draw; records_state.has_prize == 1 replaces results.item with prize_item.
  count 0 (no holder): the R3000A's division leaves the draw, no item (FINDINGS 7; the x86 host traps).
- always: each party slot with a Digimon (id >= 0) writes HP and MP back to its record; HP <= 0 comes back as 1 and
  clears results.members[k] (took_part and forms[3]).
Hand checks (pad_random_table): index 0 draws 3168, 1151 (1151 % 1024 = 127); index 1 draws 1151, 3080 (8); index 744's
second draw % 1024 = 128, index 798's = 129; 0xD9 has drop rate 128 (chance 129: residue 128 drops, 129 does not), 0x20
rate 2 (chance 3)."""
import struct

from _battle import BATTLE_STATE, GS, STATE, setup_case
from fightstg_rules import FIXTURE, SAVES, WFIGHTMN_PRO, WFIGHTMN_SLOT, member_write, rng
from oracle import Call, Case, Read, Write, layout_buffers

COMMENT = ("The battle's spoils on the fightstg_rules fixture: wfightmn_battle_end's item drop (the holder count and pick, "
           "rank used as a slot, drop_rate + 1 out of 1024, a stolen item, the prize override, result 0/1/2, no holder) and "
           "its HP/MP write-back (a knocked-out member back at 1 HP, its results cleared); the fade object it creates is "
           "allocated in a scratch heap arena. fieldstg_start_battle (FIELDSTG.PRO in the slot, a scratch field manager): "
           "the battle copied into records_state, the manager's next map, the prize range 0x1C9..0x1D0 and the prize item "
           "by parity (& 0x1F / & 0xF), map and progress.")

RESULT = 0xAF4                 # fightstg_events.result (u8)
HAS_PRIZE = 0x4C               # records_state.has_prize, prize_item
BATTLE = 0x10                  # records_state.battle
ARENA = 0x100
D9 = dict(digimon=0xD9, max_hp=800, max_mp=100)        # drop rate 128
E20 = dict(digimon=0x20, max_hp=400, max_mp=50)        # drop rate 2
STATS = lambda i: GS["digimon"] + i * GS["digimon_size"] + GS["record"] + GS["stats"]


def arena(base):
    """A heap of one free block (prev = itself, next = the terminator) and the terminator (state 1) at its end."""
    b = bytearray(ARENA)
    term = base + ARENA - 0xC
    struct.pack_into("<IIi", b, 0, base, term, 0)
    struct.pack_into("<IIi", b, ARENA - 0xC, base, base + ARENA, 1)
    return bytes(b)


def end_case(name, result, writes, comment):
    buffers = {"obj": bytes(0x100), "data": bytes(0x40), "heap": bytes(ARENA)}
    probe = Case(name, [], buffers=buffers)
    base = layout_buffers(probe)["heap"]
    buffers["heap"] = arena(base)
    fixture = FIXTURE + [
        Write(WFIGHTMN_SLOT, 0, b"", "tier-2 slot: WFIGHTMN.PRO (restored after the case)", file=WFIGHTMN_PRO),
        Write("heap_funcs", 4, struct.pack("<II", base, base + ARENA), "heap_funcs.first/end -> the scratch arena (buf heap)"),
        Write("records_state", BATTLE, struct.pack("<i", 42), "records_state.battle = 42"),
        Write("records_state", HAS_PRIZE, struct.pack("<ii", 0, 0), "records_state.has_prize = 0, prize_item = 0"),
        Write("records_battle_results", 0, struct.pack("<hhh", -1, 0x55, -1) + bytes([1] * 12) + bytes(2),
              "records_battle_results = {battle -1, item 0x55, member -1, members all 1}"),
        Write("fightstg_events", RESULT, bytes([result]), f"fightstg_events.result = {result}")]
    reads = [Read("records_battle_results", 0, 0x14, "records_battle_results"),
             Read("gamestate_data", STATS(0), 12, "digimon[0].record.stats.values[0..5] (level, TP, HP, max HP, MP, max MP)"),
             Read("gamestate_data", STATS(3), 12, "digimon[3].record.stats.values[0..5]"),
             Read("pad_random", 0, 4, "pad_random.index (draws taken)"),
             Read("buf:obj", 0x14, 4, "obj.base.substep")]
    return Case(name, [Call("wfightmn_battle_end", [("buf", "obj"), ("buf", "data")], "void", reads, comment=comment,
                            writes=list(writes))],
                fixture=fixture, buffers=buffers, saves=SAVES + [("records_battle_results", 0x14)])


def enemies(e0, e1):
    return [member_write(1, 0, **e0), member_write(1, 1, **e1)]


def drop_cases():
    hurt = [member_write(0, 0, digimon=0x17F, max_hp=300, hp=123, max_mp=100, mp=45),
            member_write(0, 1, digimon=0x3, max_hp=350, hp=350, max_mp=120, mp=7)]
    ko = [member_write(0, 0, digimon=0x17F, max_hp=300, hp=0, max_mp=100, mp=30),
          member_write(0, 1, digimon=0x3, max_hp=350, hp=-5, max_mp=120, mp=0)]
    cur1 = Write("fightstg_battle", BATTLE_STATE + STATE["current"], struct.pack("<2i", 1, 0), "state.current = {1, 0}")
    prize = Write("records_state", HAS_PRIZE, struct.pack("<ii", 1, 300), "records_state.has_prize = 1, prize_item = 300")
    one_d9 = enemies(dict(D9, item=357), dict(E20, item=0))
    return [
        end_case("escaped", 0, hurt + [rng(0)],
                 "result 0: no item logic, no draw, results untouched; HP 123 / MP 45 and 350 / 7 written back"),
        end_case("won_drop_128", 1, one_d9 + [rng(744)],
                 "won; one holder (0xD9 in slot 0, 357): pick 0, then 128 < 129: results.item 357, 2 draws"),
        end_case("won_no_drop_129", 1, one_d9 + [rng(798)], "the same at residue 129: item 0, 2 draws"),
        end_case("won_rank_as_slot", 1, enemies(dict(E20, item=0), dict(D9, item=357)) + [rng(0)],
                 "the one holder is slot 1, pick (rank) 0 reads enemies[0]: item 0, no second draw (slot 1's 357 would "
                 "drop: 127 < 129)"),
        end_case("won_two_pick0", 1, enemies(dict(D9, item=357), dict(E20, item=100)) + [rng(0)],
                 "two holders: 3168 % 2 = 0 -> enemies[0] 357, 127 < 129: 357"),
        end_case("won_two_pick1", 1, enemies(dict(D9, item=357), dict(E20, item=100)) + [rng(1)],
                 "two holders: 1151 % 2 = 1 -> enemies[1] (0x20, chance 3): 8 >= 3, item 0 (0xD9's rate would give 100)"),
        end_case("won_stolen", 1, enemies(dict(D9, item=-1), dict(E20, item=0)) + [rng(0)],
                 "slot 0's item stolen (-1) still counts: pick 0, item -1 <= 0: item 0, one draw"),
        end_case("won_prize", 1, one_d9 + [rng(798), prize], "has_prize 1: the failed drop is replaced by prize_item 300"),
        end_case("won_ko_member1_current", 1, one_d9 + ko + [cur1, rng(744)],
                 "member 0 at 0 HP and member 1 at -5 HP: HP 1 each, MP 30 / 0 written, results.members[0..1] cleared, "
                 "[2] kept; results.member = current[0] = 1; the drop as won_drop_128"),
        end_case("lost", 2, one_d9 + ko + [rng(744)], "result 2 (lost) runs the same drop: 357 (STFGTREP never reads it)"),
        end_case("no_holder", 1, enemies(dict(D9, item=0), dict(E20, item=0)) + [rng(0)],
                 "no holder (slot 2 empty with item -1 is not counted): next() % 0 -- the R3000A gives the draw, no "
                 "trap; no holder found, enemies[0].item 0: item 0, one draw (FINDINGS 7: the x86 host traps)"),
    ]


OVERLAY_SLOT = 0x80082CB0
FIELDSTG_PRO = "extracted/disc/AAA/PRO/FIELDSTG.PRO"
PROGRESS, MAP = 0x263C, 0x26C4                 # GamestateData.progress, .map
MANAGER = 0x80                                  # FieldstgManager: Object, fade, window_w/h, next_map (0x5C), map_entry


def prize_case(sym, name, battle, writes, comment, manager=True):
    """fieldstg_start_battle(battle) with FIELDSTG.PRO in the slot (restored after the case: FIGHTSTG comes back) and a
    scratch field manager (kind 7, state 1, set_state = object_set_state) as heap_objects.objects[0]. The rule: no
    manager on the object list: return; else event_running/battle_starting, the manager's next map (0x600, 0xE0A at
    progress 0x2B), map_entry 0, set_state(2); records_state.battle, first strike, unk_3D, blocked[12], enemies[3] from
    fieldstg_battles[battle]; has_prize = enemy 0 in 0x1C9..0x1D0 (battles 327..334; 269 is 0x1C8, 322 is 0x1D1); a prize
    battle draws r = next() & 0x1F (odd enemy) or & 0xF (even) and picks by map (and progress 0x2D on 0x28C..0x299) the
    common item (r != 0) or the rarer one (r == 0). Hand checks: RNG index 55 draws 0xC61 (& 0x1F = 1), index 0 draws
    0xC60 (& 0x1F = 0), index 34 draws 0xED0 (& 0x1F = 16): odd 0x177, even (& 0xF = 0) 0x186; the odd table on 0x235
    gives 0x17A, the even one 0x178."""
    mgr = bytearray(MANAGER)
    struct.pack_into("<7i", mgr, 0, 7, 0, 0, 1, 1, 2, 5)
    struct.pack_into("<I", mgr, 0x28, sym["object_set_state"])
    struct.pack_into("<ii", mgr, 0x5C, -1, -1)
    buffers = {"object_mgr": bytes(mgr)}
    place = layout_buffers(Case(name, [], buffers=buffers))["object_mgr"]
    fixture = [Write(OVERLAY_SLOT, 0, b"", "overlay slot: FIELDSTG.PRO (restored after the case)", file=FIELDSTG_PRO),
               Write("heap_objects", 0, struct.pack("<100I", place if manager else 0, *([0] * 99)),
                     "heap_objects.objects[0] = object_mgr (kind 7), the rest empty" if manager else "heap_objects.objects all empty"),
               Write("records_state", BATTLE, struct.pack("<i", -1), "records_state.battle = -1"),
               Write("records_state", HAS_PRIZE, struct.pack("<ii", 1, 0x55), "records_state.has_prize = 1, prize_item = 0x55"),
               Write("gamestate_data", PROGRESS, struct.pack("<i", 0x20), "progress = 0x20"),
               Write("gamestate_data", MAP, struct.pack("<i", 0x21D), "map = 0x21D"),
               Write("pad_random", 0, bytes(4), "pad_random.index = 0")]
    reads = [Read("records_state", BATTLE, 0x44, "records_state.battle .. prize_item (enemies, first strike, unk_3D, blocked, has_prize)"),
             Read("pad_random", 0, 4, "pad_random.index (draws taken)"),
             Read("buf:object_mgr", 0x0C, 0x10, "manager: state, step, substep, timer"),
             Read("buf:object_mgr", 0x5C, 8, "manager: next_map, map_entry")]
    return Case(name, [Call("fieldstg_start_battle", [battle], "void", reads, comment=comment, writes=list(writes))],
                fixture=fixture, buffers=buffers,
                saves=[("records_state", 0x58), ("heap_objects", 0x1A0), ("fieldstg_stage", 0x80), ("pad_random", 4)])


def prize_cases(sym):
    def gs(field, off, v):
        return Write("gamestate_data", off, struct.pack("<i", v), f"{field} = {v:#x}")
    return [prize_case(sym, name, battle, writes, comment, manager) for name, battle, writes, comment, manager in (
        ("start_no_manager", 3, [], "no field manager (kind 7) on the object list: returns before writing anything", False),
        ("start_plain", 3, [], "battle 3 (enemies 0xAC, 0x8, 0xCF): copied, first strike 0, unk_3D 2, blocked {0, 0, 1, 1, 1, 0, ..., 1}; "
                               "has_prize 0 (prize_item kept); manager next_map 0x600, map_entry 0, set_state(2); no draw", True),
        ("start_progress_2b", 3, [gs("progress", PROGRESS, 0x2B)], "progress 0x2B: next_map 0xE0A", True),
        ("start_1c8", 269, [], "battle 269, enemy 0x1C8 (below the range): has_prize 0, no draw", True),
        ("start_1d1", 322, [], "battle 322, enemy 0x1D1 (0x1D1 - 0x1C9 = 8, past the range): has_prize 0, no draw", True),
        ("prize_odd_common", 327, [rng(55)], "battle 327, 0x1C9 (odd): next() & 0x1F = 1 on map 0x21D: 0x177", True),
        ("prize_odd_rare", 327, [], "0x1C9, draw 3168 & 0x1F = 0: 0x186", True),
        ("prize_odd_16", 327, [rng(34)], "0x1C9, a draw with & 0x1F = 16 (& 0xF = 0): odd takes 5 bits, 0x177", True),
        ("prize_even_16", 328, [rng(34), gs("map", MAP, 0x201)], "battle 328, 0x1CA (even): the same draw & 0xF = 0: 0x186", True),
        ("prize_even_235", 328, [rng(55), gs("map", MAP, 0x235)], "0x1CA on map 0x235: the even table's 0x178 (the odd one has 0x17A)", True),
        ("prize_odd_28c_2d", 327, [rng(55), gs("map", MAP, 0x28C), gs("progress", PROGRESS, 0x2D)],
         "0x1C9 on map 0x28C at progress 0x2D: 0x17F", True),
        ("prize_odd_28c", 327, [rng(55), gs("map", MAP, 0x28C), gs("progress", PROGRESS, 0x2C)],
         "0x1C9 on map 0x28C at progress 0x2C: 0x17C", True))]


def cases(sym):
    return [setup_case()] + drop_cases() + prize_cases(sym)
