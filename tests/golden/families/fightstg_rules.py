"""fightstg_rules_* (src/fightstg/fightstg_8008D3B4.c 7101-8260): the battle rules. docs/MECHANICS.md sections 2-5.

One battle fixture: party member 0 = Kotemon (records_digimon[0], ID 0x17F, type 1) at level 20, member 1 =
records_digimon[3] (ID 0x3, type 2) at level 25; enemies 0 = ID 0x20 (type 8, no item) and 1 = ID 0xD9 (type 3, holds
item 357, drop rate 128), with records_state.enemies scaling them. Every case varies it through per-call writes
(technique, side, RNG index, statuses, equipment, field) and reads back fightstg_rules.stats (what get_stats built), the
RNG index after the call (the draws it took) and, where the call writes, the battle state."""
from _battle import (BATTLE_STATE, GS, RS, STATE, battle_state, enemy, fightstg_stats, member, record_writes, setup_case)
from oracle import Call, Case, Read, Write

COMMENT = ("The battle rules on one fixture (two party members, two enemies, FIGHTSTG.PRO in the slot, SDIGIEDT.PRO as the "
           "enemy records). Techniques: 1 and 10 basic (kind 1, power 60/100), 150 Ice elemental (defense_stat 4, "
           "element_power 32), 225 (defense_stat 6), 131 (strong type), 55 (anim script 0xB), 230 (kind 2), 72 (kind 9, "
           "elemental), 403 (kind 0xB, elemental). Side 0 is the party attacking enemy 0, side 1 enemy 0 attacking "
           "member 0. Each case is one call; its own writes change the fixture first. Reads: fightstg_rules.stats[2], "
           "pad_random.index (draws taken), fightstg_battle.state when the call changes it.")

TECHS = [1, 10, 150, 225, 131, 55, 230, 72, 403]
ELEM_TECHS = [150, 225, 72, 403, 259]          # defense_stat >= 2: roll_special_critical divides by the resist
STATUS_TECHS = [1, 55, 230, 72, 403]
INDEXES = [0, 777, 1500, 3333]
PARTY = [dict(i=0, level=20, hp=300, max_hp=300, mp=100, max_mp=100, stats=(80, 60, 70, 90, 75, 10), resists=(100,) * 7),
         dict(i=3, level=25, hp=350, max_hp=350, mp=120, max_mp=120, stats=(95, 70, 60, 80, 90, 20), resists=(120, 80, 100, 100, 60, 150, 90))]
DIGIMON_IDS = {0: 0x17F, 3: 0x3}
ENEMIES = [dict(digimon=0x20, level=18, hp=400, mp=50, stat_scale=16), dict(digimon=0xD9, level=30, hp=800, mp=100, stat_scale=24)]


def base_fixture():
    w = [Write("gamestate_data", GS["party"], (0).to_bytes(4, "little") + (3).to_bytes(4, "little") + (0xFFFFFFFF).to_bytes(4, "little"),
               "party = {0, 3, -1} (records_digimon indexes)")]
    for p in PARTY:
        w += record_writes(p["i"], p["level"], p["hp"], p["max_hp"], p["mp"], p["max_mp"], p["stats"], p["resists"])
    party = [member(DIGIMON_IDS[p["i"]], p["max_hp"], max_mp=p["max_mp"]) for p in PARTY]
    enemies = [member(e["digimon"], e["hp"], max_mp=e["mp"], item=357 if e["digimon"] == 0xD9 else -1) for e in ENEMIES]
    w.append(Write("fightstg_battle", BATTLE_STATE, battle_state((0, 0), party, enemies),
                   "fightstg_battle.state: current {0,0}; members[0] Kotemon 300 HP, #3 350 HP; members[1] 0x20 400 HP, 0xD9 800 HP (item 357)"))
    w.append(Write("records_state", RS["enemies"], b"".join(enemy(**e) for e in ENEMIES) + enemy(0, 0, 0, 0, 0),
                   "records_state.enemies: 0x20 level 18 scale 16; 0xD9 level 30 scale 24"))
    w.append(Write("records_state", RS["blocked"], bytes(12), "records_state.blocked = all 0"))
    w.append(Write("fightstg_rules", 0, bytes(0x80), "fightstg_rules.stats[2] = 0 (the function table after them stays)"))
    w.append(Write("pad_random", 0, bytes(4), "pad_random.index = 0"))
    return w


FIXTURE = base_fixture()
SAVES = [("gamestate_data", 0x275C), ("gamestate_flags", 0x1C), ("fightstg_battle", 0xF8), ("fightstg_rules", 0xF0),
         ("records_state", 0x68), ("pad_random", 4), ("cdload_module", 0x434), ("fightstg_events", 0xB1C)]
READ_STATS = Read("fightstg_rules", 0, 0x80, "fightstg_rules.stats[0..1] after the call (attacker, target)")
READ_RNG = Read("pad_random", 0, 4, "pad_random.index after the call")
READ_STATE = Read("fightstg_battle", BATTLE_STATE, 0xD4, "fightstg_battle.state after the call")


def rng(index):
    return Write("pad_random", 0, index.to_bytes(4, "little"), f"pad_random.index = {index}")


def member_write(side, i, **kw):
    off = BATTLE_STATE + STATE["members"] + (side * 3 + i) * STATE["member_size"]
    return Write("fightstg_battle", off, member(**kw), f"members[{side}][{i}] = {kw}")


def equip(i, slots):
    base = GS["digimon"] + i * GS["digimon_size"] + GS["record"] + GS["equipment"]
    import struct
    return Write("gamestate_data", base, struct.pack("<6h", *slots), f"digimon[{i}].record.equipment = {list(slots)}")


def case(name, func, args, writes=(), reads=(READ_STATS, READ_RNG), comment="", ret="s32"):
    return Case(name, [Call(func, list(args), ret, list(reads), comment=comment, writes=list(writes))], fixture=FIXTURE, saves=SAVES)


def cases(sym):
    out = [setup_case()]
    sides = [(0, "party -> enemy"), (1, "enemy -> party")]
    # Damage pipelines.
    for func in ("fightstg_rules_get_damage", "fightstg_rules_get_special_damage"):
        short = func.split("_")[-2] if "special" in func else "damage"
        for t in TECHS:
            for side, sdesc in sides:
                for idx in INDEXES:
                    out.append(case(f"{short}_t{t}_s{side}_r{idx}", func, [side, t], [rng(idx)], comment=f"technique {t}, {sdesc}, RNG index {idx}"))
    # Modifiers of the damage: power_up, field, equipment, statuses, boss, boosted.
    mods = [
        ("power_up32", [member_write(0, 0, digimon=0x17F, max_hp=300, max_mp=100, power_up=32)], "attacker power_up 32 (+50%)"),
        ("field_ice64", [Write("fightstg_battle", BATTLE_STATE + STATE["field"], (4).to_bytes(2, "little") + (64).to_bytes(2, "little"), "field = {element 4 (Ice), power 64}")], "Ice field: +50% for Ice, -25% for Fire techniques"),
        ("field_fire64", [Write("fightstg_battle", BATTLE_STATE + STATE["field"], (2).to_bytes(2, "little") + (64).to_bytes(2, "little"), "field = {element 2 (Fire), power 64}")], "Fire field"),
        ("multi_crest", [equip(0, (0, 0, 0, 0, 0x13C, 0))], "Multi Crest 0x13C: kind < 2 damage * 4 / 10"),
        ("guard_crest", [equip(0, (0, 0, 0, 0, 0x145, 0))], "crest 0x145: the party member's guard subtracts flat damage when it is the target (side 1)"),
        ("fire_crest", [equip(0, (0, 0, 0, 0, 0x153, 0))], "Fire Power 1 (0x153): attack_element 2 on non-elemental techniques"),
        ("crit_crest", [equip(0, (0, 0, 0, 0, 0x13D, 0))], "crest 0x13D: the attacker's critical rate"),
        ("target_asleep", [member_write(1, 0, digimon=0x20, max_hp=400, max_mp=50, status=8, sleep_power=80)], "enemy asleep, sleep_power 80: +10 critical chance"),
        ("target_paralysed", [member_write(1, 0, digimon=0x20, max_hp=400, max_mp=50, status=2, paralysis_power=64)], "enemy paralysed, paralysis_power 64: +8"),
        ("target_confused", [member_write(1, 0, digimon=0x20, max_hp=400, max_mp=50, status=4, confusion_power=50)], "enemy confused, confusion_power 50: +25"),
        ("enemy1_scaled", [Write("fightstg_battle", BATTLE_STATE + STATE["current"], (0).to_bytes(4, "little") + (1).to_bytes(4, "little"), "current = {0, 1}: enemy 0xD9, stat_scale 24")], "the second enemy (scale 24/16, type 3)"),
        ("member1_attacks", [Write("fightstg_battle", BATTLE_STATE + STATE["current"], (1).to_bytes(4, "little") + (0).to_bytes(4, "little"), "current = {1, 0}: party member 1 (records_digimon[3])")], "the second party member"),
        ("boss", [Write("fightstg_battle", BATTLE_STATE + STATE["type"], (6).to_bytes(2, "little"), "state.type = 6 (boss)")], "boss battle: the party's hit roll is a flat 85/128"),
        ("mod_power64", [member_write(0, 0, digimon=0x17F, max_hp=300, max_mp=100, modifiers=(40, -30, 0, 0))], "member modifiers +40 Power, -30 Guard"),
    ]
    for mname, writes, desc in mods:
        for t in (1, 150, 131):
            for side, sdesc in sides:
                for idx in (0, 1500):
                    for func, short in (("fightstg_rules_get_damage", "damage"), ("fightstg_rules_roll_hit", "hit"), ("fightstg_rules_roll_critical", "crit")):
                        out.append(case(f"{short}_{mname}_t{t}_s{side}_r{idx}", func, [side, t], writes + [rng(idx)], comment=f"{desc}; technique {t}, {sdesc}, RNG {idx}"))
    # Hit and critical rolls over the techniques.
    for func, short, techs in (("fightstg_rules_roll_hit", "hit", TECHS), ("fightstg_rules_roll_special_hit", "shit", TECHS),
                               ("fightstg_rules_roll_critical", "crit", TECHS), ("fightstg_rules_roll_special_critical", "scrit", ELEM_TECHS)):
        for t in techs:
            for side, sdesc in sides:
                for idx in INDEXES:
                    out.append(case(f"{short}_t{t}_s{side}_r{idx}", func, [side, t], [rng(idx)], comment=f"technique {t}, {sdesc}, RNG {idx}"))
    # Status rolls.
    for roll in ("poison", "paralysis", "confusion", "sleep", "knockout", "drain", "revert", "lower_stat", "switch_seal", "digivolve_seal"):
        for t in STATUS_TECHS:
            for side, sdesc in sides:
                for idx in (0, 1500):
                    out.append(case(f"{roll}_t{t}_s{side}_r{idx}", f"fightstg_rules_roll_{roll}", [side, t], [rng(idx)], comment=f"technique {t}, {sdesc}, RNG {idx}"))
    for i, slot in enumerate(("poison", "paralysis", "confusion", "sleep", "knockout", "drain")):
        blocked = bytes(12); blocked = blocked[:i] + b"\x01" + blocked[i + 1:]
        out.append(case(f"{slot}_blocked_s0", f"fightstg_rules_roll_{slot}", [0, 230],
                        [Write("records_state", RS["blocked"], blocked, f"blocked[{i}] = 1"), rng(0)], comment=f"records_state.blocked[{i}]: the party's roll fails without a draw"))
    # Steal: the second enemy holds item 357 (drop rate 128).
    cur01 = Write("fightstg_battle", BATTLE_STATE + STATE["current"], (0).to_bytes(4, "little") + (1).to_bytes(4, "little"), "current = {0, 1}")
    for idx in INDEXES:
        out.append(case(f"steal_s0_r{idx}", "fightstg_rules_roll_steal", [0, 230], [cur01, rng(idx)], comment=f"against enemy 0xD9 holding item 357; RNG {idx}"))
    out.append(case("steal_s0_noitem", "fightstg_rules_roll_steal", [0, 230], [rng(0)], comment="enemy 0 holds nothing: 0 without a draw"))
    out.append(case("steal_s0_blocked", "fightstg_rules_roll_steal", [0, 230], [cur01, Write("records_state", RS["blocked"] + 7, b"\x01", "blocked[7] = 1"), rng(0)], comment="blocked[7]: 0 without a draw"))
    out.append(case("steal_s1", "fightstg_rules_roll_steal", [1, 230], [cur01, rng(0)], comment="the enemy never steals: 0"))
    # Escape.
    for side, sdesc in sides:
        for esc in (0, 3):
            for status, sdesc2 in ((0, "healthy"), (2, "paralysed"), (8, "asleep")):
                for idx in (0, 1500):
                    w = [Write("fightstg_battle", BATTLE_STATE + STATE["escapes"], esc.to_bytes(2, "little"), f"escapes = {esc}"),
                         member_write(side, 0, digimon=0x17F if side == 0 else 0x20, max_hp=300 if side == 0 else 400, max_mp=100 if side == 0 else 50, status=status), rng(idx)]
                    out.append(case(f"escape_s{side}_e{esc}_st{status}_r{idx}", "fightstg_rules_roll_escape", [side], w, comment=f"{sdesc}, escapes {esc}, {sdesc2}, RNG {idx}"))
    out.append(case("escape_s0_blocked", "fightstg_rules_roll_escape", [0], [Write("records_state", RS["blocked"] + 11, b"\x01", "blocked[11] = 1"), rng(0)], comment="blocked[11]: 0"))
    out.append(case("escape_s1_noescape_crest", "fightstg_rules_roll_escape", [1], [equip(0, (0, 0, 0, 0, 0x13F, 0)), rng(0)], comment="the party wears crest 0x13F: the enemy's base chance 32 instead of 64"))
    # Counter, wake, confused, paralysed, regen.
    for dmg in (10, 100, 299, 300):
        for idx in (0, 1500):
            out.append(case(f"counter_d{dmg}_r{idx}", "fightstg_rules_roll_counter", [0, dmg], [rng(idx)], comment=f"damage {dmg} on member 0 (300 HP), RNG {idx}"))
    for side in (0, 1):
        for amount in (0, 50, 200):
            out.append(case(f"wake_s{side}_a{amount}", "fightstg_rules_roll_wake", [side, amount],
                            [member_write(side, 0, digimon=0x17F if side == 0 else 0x20, max_hp=300 if side == 0 else 400, max_mp=100 if side == 0 else 50, status=8, sleep_power=40), rng(0)],
                            comment=f"sleeping member (sleep_power 40) hit for {amount}; no sleep event in fightstg_events (its find_member returns -1)"))
    target_stats = fightstg_stats(level=20, stats=(80, 60, 70, 90, 75), resists=(100, 100, 100, 50, 60, 100, 100, 100, 40, 30, 100, 100))
    for roll, power_field in (("confused", "confusion_power"), ("paralyzed", "paralysis_power")):
        for side in (0, 1):
            for power in (20, 60, 127):
                for idx in (0, 1500):
                    kw = {power_field: power, "status": 4 if roll == "confused" else 2}
                    w = [Write("fightstg_rules", 0x40, target_stats, "fightstg_rules.stats[1] (read directly): resists[3] 50, [4] 60, [8] 40, [9] 30"),
                         member_write(side, 0, digimon=0x17F if side == 0 else 0x20, max_hp=300 if side == 0 else 400, max_mp=100 if side == 0 else 50, **kw), rng(idx)]
                    out.append(case(f"{roll}_s{side}_p{power}_r{idx}", f"fightstg_rules_roll_{roll}", [side], w, comment=f"{power_field} {power}, RNG {idx}"))
    for side in (0, 1):
        for flag in (0, 1):
            for idx in (0, 777, 1500):
                out.append(case(f"regen_s{side}_f{flag}_r{idx}", "fightstg_rules_get_regen", [side, 0, flag], [rng(idx)], comment=f"member 0 of side {side}, flag {flag}, RNG {idx}"))
    # Heal, poison, counter damage.
    for t in (200, 150, 10):
        for side in (0, 1):
            out.append(case(f"heal_t{t}_s{side}", "fightstg_rules_get_heal", [side, t], [], comment=f"technique {t} (power {dict(((200, 0), (150, 200), (10, 100)))[t]}), side {side}"))
    import struct
    for side in (0, 1):
        for amount in (20, 50, 200):
            buf = struct.pack("<B3xii", side, 0, amount)
            out.append(Case(f"poison_dmg_s{side}_a{amount}", [Call("fightstg_rules_get_poison_damage", [("buf", "args")], "s32", [READ_STATS], comment=f"args {{side {side}, member 0, amount {amount}}}")],
                            fixture=FIXTURE, buffers={"args": buf}, saves=SAVES))
    out.append(Case("poison_dmg_other_member", [Call("fightstg_rules_get_poison_damage", [("buf", "args")], "s32", [READ_STATS], comment="member 1 is not the acting member: 0")],
                    fixture=FIXTURE, buffers={"args": struct.pack("<B3xii", 0, 1, 50)}, saves=SAVES))
    for t in (1, 150):
        for amount in (50, 200):
            for side in (0, 1):
                for boosted in (0, 1):
                    w = [member_write(side, 0, digimon=0x17F if side == 0 else 0x20, max_hp=300 if side == 0 else 400, max_mp=100 if side == 0 else 50, boosted=boosted), rng(0)]
                    out.append(case(f"counterdmg_t{t}_a{amount}_s{side}_b{boosted}", "fightstg_rules_get_counter_damage", [side, t, amount], w, comment=f"technique {t}, amount {amount}, side {side}, boosted {boosted}"))
    # Field bonus (pure over the field), modifiers, gauge, cost.
    for felem, fpow in ((4, 64), (2, 128), (0, 64)):
        fw = Write("fightstg_battle", BATTLE_STATE + STATE["field"], felem.to_bytes(2, "little") + fpow.to_bytes(2, "little"), f"field = {{{felem}, {fpow}}}")
        for elem in range(9):
            for value in (200, 201):
                out.append(case(f"fieldbonus_f{felem}p{fpow}_e{elem}_v{value}", "fightstg_rules_get_field_bonus", [value, elem], [fw], reads=(), comment=f"field element {felem} power {fpow}; technique element {elem}, value {value}"))
    for side in (0, 1):
        for kind in range(4):
            for amount in (128, -256, 64, 0):
                out.append(case(f"modifier_s{side}_k{kind}_a{amount}", "fightstg_rules_change_modifier", [side, 0, kind, amount & 0xFFFFFFFF], [],
                                reads=(READ_STATS, READ_STATE), ret="void", comment=f"side {side}, member 0, kind {kind}, amount {amount}"))
    out.append(case("modifier_twice_cap", "fightstg_rules_change_modifier", [0, 0, 0, 128], [member_write(0, 0, digimon=0x17F, max_hp=300, max_mp=100, modifiers=(80, 0, 0, 0))],
                    reads=(READ_STATS, READ_STATE), ret="void", comment="a second +128 on Power (modifier already 80): capped at the stat"))
    out.append(case("modifier_dead_member", "fightstg_rules_change_modifier", [0, 0, 0, 128], [member_write(0, 0, digimon=0x17F, max_hp=300, hp=0, max_mp=100)],
                    reads=(READ_STATS, READ_STATE), ret="void", comment="hp 0: nothing changes"))
    for crest in (0, 0x149, 0x14A):
        for amount in (0, 30, 150, 300, 1000):
            out.append(case(f"gauge_c{crest:x}_a{amount}", "fightstg_rules_get_gauge_gain", [amount], [equip(0, (0, 0, 0, 0, crest, 0))], reads=(), comment=f"damage {amount} on 300 max HP, crest {crest:#x}"))
    for t in (150, 100, 200, 1):
        for flag in (0, 0x4000):
            for side in (0, 1):
                out.append(case(f"cost_t{t}_f{flag:x}_s{side}", "fightstg_rules_get_tech_cost", [side, t | flag], [], reads=(), comment=f"technique {t} (mp {dict(((150, 48), (100, 200), (200, 42), (1, 0)))[t]}), flag {flag:#x}, side {side}"))
            for crest in (0x143, 0x144):
                out.append(case(f"cost_t{t}_f{flag:x}_s0_c{crest:x}", "fightstg_rules_get_tech_cost", [0, t | flag], [equip(0, (0, 0, 0, 0, crest, 0))], reads=(), comment=f"technique {t}, flag {flag:#x}, MP crest {crest:#x}"))
    out += appended_cases(cur01)
    return out


def appended_cases(cur01):
    """Branches the cases above never took (appended; every case above is unchanged). Each pair sits on both sides of a
    threshold, at an RNG index chosen for its draw, so it pins the threshold's exact value:
    - a successful steal: technique 230 (effect_chance 72), Boost 75 against 0xD9's 104 * 24 / 16 = 156: ratio 7500 / 156
      = 48, percent (72 + 0) * 100 / 64 = 112, chance 128 * 48 * 112 / 10000 = 68 against next() % 1024 (67 steals, 68
      does not); Boost 999: ratio 640 capped at 200, chance 286 (285 / 286); Hack Sticker (0x147, value 32): percent 162,
      chance 99 (98 / 99);
    - the enemy's hit floor 0x20: party member 0 at level 99 with Boost 999 against enemy 0x20 (level 18, Boost 48):
      diff (48 - 999) / 8 = -118, level -81, chance 110 + 110 * -199 / 128 = -61, floored at 32 (residue 31 hits, 32 not);
      the party has no floor: level 1, Boost 1 against 0x20 at level 99 and scale 186 (Boost 558): chance
      110 + 110 * (-69 - 98) / 128 = -33, so even residue 0 misses;
    - waking with a sleep event (fightstg_events.events[0] = {12, delay, side, member}): wait = delay / 100; member 0
      (Guard 60, sleep_power 40) hit for 50: 6400 / 60 + 44 - wait = 150 - wait against residue 96 (RNG 0): delay 5399 ->
      97 wakes, 5400 -> 96 does not; the enemy (Guard 42, side 0x10 as the game passes it): 196 - 100 = 96, does not."""
    import struct
    out = []
    stats0 = GS["digimon"] + GS["record"] + GS["stats"]

    def party0(level, boost):
        return (Write("gamestate_data", stats0, struct.pack("<h", level), f"digimon[0].record level {level}"),
                Write("gamestate_data", stats0 + 12 + 4 * 2, struct.pack("<h", boost), f"digimon[0].record Boost {boost}"))

    boost999 = party0(20, 999)[1]
    hack = equip(0, (0, 0, 0, 0, 0x147, 0))
    for name, writes, idx, want in (("steal_s0_chance68_hit", [], 559, "67 < 68: steals"), ("steal_s0_chance68_miss", [], 53, "68: no"),
                                    ("steal_s0_ratio_cap_hit", [boost999], 6, "ratio capped 200, chance 286; 285: steals"),
                                    ("steal_s0_ratio_cap_miss", [boost999], 1152, "286: no (640 uncapped would steal)"),
                                    ("steal_s0_crest_hit", [hack], 374, "Hack Sticker: chance 99; 98: steals"),
                                    ("steal_s0_crest_miss", [hack], 822, "99: no")):
        out.append(case(name, "fightstg_rules_roll_steal", [0, 230], [cur01] + writes + [rng(idx)],
                        comment=f"technique 230 against 0xD9; RNG {idx}: next() % 1024 = {want}"))
    floor = list(party0(99, 999))
    out.append(case("hit_s1_floor_hit", "fightstg_rules_roll_hit", [1, 1], floor + [rng(75)],
                    comment="enemy 0x20 against member 0 (level 99, Boost 999): chance -61 floored at 32; residue 31 hits"))
    out.append(case("hit_s1_floor_miss", "fightstg_rules_roll_hit", [1, 1], floor + [rng(25)],
                    comment="the same; residue 32 misses (the floor is 32)"))
    out.append(case("hit_s0_no_floor", "fightstg_rules_roll_hit", [0, 1], list(party0(1, 1)) + [
        Write("records_state", RS["enemies"], enemy(0x20, 99, 400, 50, 186), "records_state.enemies[0]: 0x20 level 99 scale 186"), rng(14)],
        comment="member 0 (level 1, Boost 1) against 0x20 at level 99, Boost 558: chance -33, no floor for the party; residue 0 misses"))
    for name, side, delay, sleeper in (("wake_event_s0_d5399", 0, 5399, "member 0"), ("wake_event_s0_d5400", 0, 5400, "member 0"),
                                       ("wake_event_s16_d10000", 0x10, 10000, "enemy 0")):
        enemy_side = int(side != 0)
        ev = struct.pack("<hh6i", 12, delay, side, 0, 0, 0, 0, 0)
        w = [member_write(enemy_side, 0, digimon=0x20 if enemy_side else 0x17F, max_hp=400 if enemy_side else 300,
                          max_mp=50 if enemy_side else 100, status=8, sleep_power=40),
             Write("fightstg_events", 0, ev, f"fightstg_events.events[0] = sleep event {{12, delay {delay}, side {side:#x}, member 0}}"), rng(0)]
        out.append(case(name, "fightstg_rules_roll_wake", [side, 50], w,
                        comment=f"{sleeper} asleep (sleep_power 40) hit for 50, its sleep event's delay {delay}: wait {delay // 100}"))
    out += wfightmn_cases(party0)
    out += wfightmn_state_cases()
    return out


WFIGHTMN_SLOT = 0x800A5DE0
WFIGHTMN_PRO = "extracted/disc/AAA/PRO/WFIGHTMN.PRO"


def wfightmn_cases(party0):
    """Two battle rules of WFIGHTMN (the battle's tier-2 overlay, linked against FIGHTSTG): each case also copies
    WFIGHTMN.PRO into the tier-2 slot (restored after the case, like every non-kept write).
    - wfightmn_roll_first_strike(): 0 without a draw when records_state.first_strike_chance is 0; else chance =
      first_strike_chance * (32 - the leader's level - enemy 0's level) / 32 and r = next() % 128: r == 0 always strikes
      first, else r < chance. Hand checks: chance 128, levels 1 and 1: 128 * 30 / 32 = 120 (residue 119 yes, 120 no);
      the fixture's levels 20 and 18: 128 * -6 / 32 = -24, only residue 0 strikes first.
    - wfightmn_cap_damage(side, value, count): battle types 1 and 2 keep the enemy hit by the party above
      limit = (s16)(max_hp / 11) (count hits: (hp - limit) / count each; nothing at or below the limit); type 3: the party
      deals 0; the enemy's damage and other types unchanged. Enemy 0x20 has 400/400 HP: limit 36; 380 -> 364; 3 x 150
      -> 364 / 3 = 121."""
    import struct
    wf = Write(WFIGHTMN_SLOT, 0, b"", "tier-2 slot: WFIGHTMN.PRO (restored after the case)", file=WFIGHTMN_PRO)
    fixture = FIXTURE + [wf]
    saves = SAVES   # the file write itself is undone after the case (oracle.lua apply_write keeps the bytes)
    out = []

    def wcase(name, func, args, writes, comment, reads=(READ_RNG,)):
        return Case(name, [Call(func, list(args), "s32", list(reads), comment=comment, writes=list(writes))], fixture=fixture, saves=saves)

    def fsc(v):
        return Write("records_state", 0x3C, bytes([v]), f"records_state.first_strike_chance = {v}")
    lv1 = [party0(1, 75)[0], Write("records_state", RS["enemies"], enemy(0x20, 1, 400, 50, 16), "records_state.enemies[0]: 0x20 level 1")]
    for name, writes, comment in (
            ("first_strike_chance0", [fsc(0), rng(14)], "first_strike_chance 0: 0, no draw (index stays 14)"),
            ("first_strike_hit119", [fsc(128)] + lv1 + [rng(266)], "chance 128 * (32 - 1 - 1) / 32 = 120; residue 119: 1"),
            ("first_strike_miss120", [fsc(128)] + lv1 + [rng(158)], "chance 120; residue 120: 0"),
            ("first_strike_residue0", [fsc(128), rng(14)], "levels 20 and 18: chance -24; residue 0 strikes first anyway"),
            ("first_strike_residue1", [fsc(128), rng(68)], "chance -24; residue 1: 0")):
        out.append(wcase(name, "wfightmn_roll_first_strike", [], writes, comment))

    def btype(t):
        return Write("fightstg_battle", BATTLE_STATE + STATE["type"], struct.pack("<h", t), f"state.type = {t}")
    hp36 = member_write(1, 0, digimon=0x20, max_hp=400, hp=36, max_mp=50)
    for name, t, side, value, count, writes, comment in (
            ("cap_type0", 0, 0, 500, 0, [], "type 0: 500 unchanged"),
            ("cap_type1_below", 1, 0, 100, 0, [], "type 1, 400 HP: 300 left >= 36, 100 unchanged"),
            ("cap_type1_capped", 1, 0, 380, 0, [], "type 1: 400 - 380 < 36 -> 400 - 36 = 364"),
            ("cap_type2_at_limit", 2, 0, 50, 0, [hp36], "type 2, enemy at 36 HP = the limit: 0"),
            ("cap_type1_hits3", 1, 0, 150, 3, [], "type 1, 3 hits of 150: 400 - 450 < 36 -> (400 - 36) / 3 = 121 each"),
            ("cap_type1_hits3_at_limit", 1, 0, 150, 3, [hp36], "type 1, 3 hits, enemy at the limit: 0"),
            ("cap_type1_enemy", 1, 1, 500, 0, [], "type 1, the enemy's damage: 500 unchanged"),
            ("cap_type3", 3, 0, 500, 0, [], "type 3: the party deals 0"),
            ("cap_type3_enemy", 3, 1, 500, 0, [], "type 3, the enemy's damage: 500 unchanged")):
        out.append(wcase(name, "wfightmn_cap_damage", [side, value, count], [btype(t)] + writes, comment, reads=()))
    return out


def wfightmn_state_cases():
    """Two more WFIGHTMN rules that write the battle's state (appended 2026-10-05, sweep2; WFIGHTMN.PRO in the tier-2 slot
    as above):
    - wfightmn_add_gauge(side, value), on every hit: only a hit on the party (side != 0) with value != 0, on an acting
      member alive and not blasted, adds get_gauge_gain(value) to records_state.gauges[the member's Digimon index] (not
      its party slot); at >= 1000 the gauge is set to 1000 and a gauge-full event (type 0x12, delay 0) is queued.
      Hand checks: 150 on member 0 (300 max HP): 50% -> 2500 / 20 = 125 into gauges[0]; on member 1 (Digimon 3, 350 max
      HP): 42% -> 88 into gauges[3]; 874 + 125 = 999 (no event), 875 + 125 = 1000 (event), 900 + 125 -> 1000 (event).
    - wfightmn_mark_took_part(): records_battle_results.members[slot].took_part = 1 for the acting member, and when it
      fights as one of its chosen forms, forms[i] = 1 with i its place in get_chosen_forms' packed list: chosen
      {386, -1, 367} fighting as 367 marks forms[1] (not 2); its own form or an unchosen one marks no form."""
    import struct
    wf = Write(WFIGHTMN_SLOT, 0, b"", "tier-2 slot: WFIGHTMN.PRO (restored after the case)", file=WFIGHTMN_PRO)
    fixture = FIXTURE + [wf]
    saves = SAVES + [("records_battle_results", 0x14)]
    read_gauges = Read("records_state", 0x58, 16, "records_state.gauges[8]")
    read_event = Read("fightstg_events", 0, 4, "fightstg_events.events[0].type, .delay")
    read_results = Read("records_battle_results", 0, 0x14, "records_battle_results")
    out = []

    def gauges(*kv):
        g = [0] * 8
        for k, v in kv:
            g[k] = v
        return Write("records_state", 0x58, struct.pack("<8h", *g), f"records_state.gauges = {g}")
    no_event = Write("fightstg_events", 0, bytes(0x1C), "fightstg_events.events[0] empty")
    cur10 = Write("fightstg_battle", BATTLE_STATE + STATE["current"], (1).to_bytes(4, "little") + (0).to_bytes(4, "little"), "current = {1, 0}")
    for name, side, value, writes, comment in (
            ("gauge_party_hits", 0, 150, [], "the party's hit (side 0): no gauge"),
            ("gauge_value0", 0x10, 0, [], "value 0: no gauge"),
            ("gauge_add", 0x10, 150, [], "150 on member 0 (300 max HP): gauges[0] += 125"),
            ("gauge_member1", 0x10, 150, [cur10], "member 1 (Digimon 3, 350 max HP) acting: gauges[3] += 88 (by Digimon index)"),
            ("gauge_999", 0x10, 150, [gauges((0, 874))], "874 + 125 = 999: no event"),
            ("gauge_1000", 0x10, 150, [gauges((0, 875))], "875 + 125 = 1000: set to 1000, gauge-full event queued"),
            ("gauge_cap", 0x10, 150, [gauges((0, 900))], "900 + 125 capped at 1000, event queued"),
            ("gauge_knocked_out", 0x10, 150, [member_write(0, 0, digimon=0x17F, max_hp=300, hp=0, max_mp=100)], "member at 0 HP: no gauge"),
            ("gauge_blasted", 0x10, 150, [member_write(0, 0, digimon=0x17F, max_hp=300, max_mp=100, blasted=1)], "member blasted: no gauge")):
        w = [gauges(), no_event] + writes   # a case's own gauges write comes later and wins
        out.append(Case(name, [Call("wfightmn_add_gauge", [side, value], "void", [read_gauges, read_event], comment=comment, writes=w)],
                        fixture=fixture, saves=saves))

    rec0 = GS["digimon"] + GS["record"]
    form = lambda i: struct.pack("<hbBi6h", i, 1, 0, 0, 0, 0, 0, 0, 0, 0)
    forms = [Write("gamestate_data", rec0 + 0x48, struct.pack("<3h", 386, -1, 367), "digimon[0].record.chosen_forms = {386, -1, 367}"),
             Write("gamestate_data", rec0 + 0x50, form(386) + form(367), "digimon[0].record.forms[0..1] = 386, 367"),
             Write("records_battle_results", 0, bytes(0x14), "records_battle_results = 0")]
    for name, writes, comment in (
            ("took_part_own", [], "member 0 fights as itself (0x17F): took_part only"),
            ("took_part_form", [member_write(0, 0, digimon=367, base_digimon=0x17F, max_hp=300, max_mp=100)],
             "member 0 fights as 367, chosen slot 2 but packed place 1: forms[1]"),
            ("took_part_unchosen", [member_write(0, 0, digimon=389, base_digimon=0x17F, max_hp=300, max_mp=100)],
             "member 0 fights as 389, not chosen: took_part only"),
            ("took_part_member1", [cur10], "member 1 (Digimon 3) acting as itself: members[1].took_part")):
        out.append(Case(name, [Call("wfightmn_mark_took_part", [], "void", [read_results], comment=comment, writes=forms + writes)],
                        fixture=fixture, saves=saves))
    return out
