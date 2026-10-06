"""The enemy's choice of action (src/fightstg/fightstg_80086A00.c fightstg_enemy_check_condition, fightstg_enemy_get_action,
fightstg_enemy_turn_find_target; run by fightstg_enemy_turn_update on the enemy's turn). docs/MECHANICS.md section 5.

The fightstg_rules battle fixture (tests/golden/families/fightstg_rules.py: FIGHTSTG.PRO in the slot, SDIGIEDT.PRO as
the enemy records; party member 0 Kotemon 300/300 HP, enemies 0x20 400 HP (current) and 0xD9 800 HP). Added by sweep4
(2026-10-05): no golden ran the enemy's choice. A family of its own named to run after gamestate_flags (DECISIONS "Replay and golden contracts": cases added before it can move a vblank into its whole-struct read).

The rule (fightstg_enemy_turn_update): the enemy's record (file 0x1CF) has actions[3] {type, condition, value}; the first
whose condition holds is taken, and when none holds the loop ends at i = 3 and reads the 4 bytes after the array (record
+0x3E, every record has {1 or 2, 0, 0} there: a default action, FINDINGS 5). fightstg_enemy_get_action maps the type:
1 attack (1), 2/3 the record's tech_2/tech_3, 4 flee (-1), 5..8 call member 0/1/2/any (-2..-5), else 0.
- fightstg_enemy_check_condition(type, value), on the enemy's current member unless said otherwise:
  1 next() % 128 < value (one draw); 2 HP under: (s16)(max_hp / 100) * (value * 100 / 128) > hp, both divisions
  truncated: value 26 is rate 20, max_hp 450 gives 4 * 20 = 80 (not 450 * 26 / 128 = 91 nor 450 * 20 / 100 = 90): hp
  79 -> 1, hp 80 -> 0; max_hp 99 gives 0, so even hp 0 is not under; 3 the complement (<=): hp 80 -> 1; 6/7 the same on
  the party's current member (value 64 is rate 50: 300 -> 3 * 50 = 150); 8 the party member's Digimon is one of
  records_digimon[0..7] (index 7 0x1F -> 1, index 8 0x182 -> 0); 10 another enemy member (not the current slot) alive
  with Digimon value (0: any); 18 the current member's turns % value == 0 (turn 0 -> 1; the data's values are 2..4,
  never 0). 4/5 MP under/over, 9 the party's status 8, 11 field element, 12 records_state.battle, 13 .unk_3D, 14
  power_up, 15..17 modifiers[0..2] < 0 are one comparison each; 0 and > 18 are 1.
- fightstg_enemy_turn_find_target(obj) (obj->action at +0x78): -2/-3/-4 name member 0/1/2, refused (-1) when it is dead
  or has the same Digimon as the current member (an ID compare, not a slot compare: a second 0x20 cannot be called by a
  0x20); -5: the other live members, the only one without a draw, of two found[next() & 1] (RNG index 1: draw 1151,
  odd: the second)."""
import struct

from _battle import BATTLE_STATE, STATE, setup_case
from fightstg_rules import FIXTURE, READ_RNG, SAVES, case, member_write, rng
from oracle import Call, Case, Read, Write

COMMENT = ("The enemy's choice on the fightstg_rules fixture: fightstg_enemy_check_condition's conditions at their "
           "boundaries (the RNG roll, the truncated HP thresholds, the party's Digimon list, another member, turns), "
           "fightstg_enemy_get_action's mapping and record lookup, fightstg_enemy_turn_find_target's call rules.")

PARTY0 = dict(digimon=0x17F, max_hp=300, max_mp=100)
ENEMY0 = dict(digimon=0x20, max_hp=400, max_mp=50)
ENEMY1 = dict(digimon=0xD9, max_hp=800, max_mp=100, item=357)
TURN_SIZE = 0x80          # FightstgEnemyTurn (local to fightstg_80086A00.c): Object (0x50), args, unk_5C, action, called
TURN_ACTION = 0x78


def current(enemy):
    return Write("fightstg_battle", BATTLE_STATE + STATE["current"], struct.pack("<2i", 0, enemy), f"state.current = {{0, {enemy}}}")


def cases(sym):
    out = [setup_case()]
    cond = "fightstg_enemy_check_condition"
    for name, type_, value, writes, comment in (
            ("cond1_roll_under", 1, 32, [rng(75)], "next() % 128 = 31 < 32: 1"),
            ("cond1_roll_equal", 1, 32, [rng(25)], "next() % 128 = 32: 0"),
            ("cond2_hp_under", 2, 26, [member_write(1, 0, **dict(ENEMY0, max_hp=450), hp=79)], "450 / 100 * (26 * 100 / 128) = 80 > 79: 1"),
            ("cond2_hp_at", 2, 26, [member_write(1, 0, **dict(ENEMY0, max_hp=450), hp=80)], "80 > 80 is false: 0 (an exact 91 or 90 would say 1)"),
            ("cond2_small_max", 2, 26, [member_write(1, 0, **dict(ENEMY0, max_hp=99), hp=0)], "max_hp 99: 99 / 100 = 0, 0 > 0 is false even at hp 0"),
            ("cond3_hp_at", 3, 26, [member_write(1, 0, **dict(ENEMY0, max_hp=450), hp=80)], "80 <= 80: 1"),
            ("cond6_party_hp", 6, 64, [member_write(0, 0, **PARTY0, hp=149)], "the party's member: 300 / 100 * 50 = 150 > 149: 1 (the enemy's 400/400 would be 0)"),
            ("cond8_index7", 8, 0, [member_write(0, 0, **dict(PARTY0, digimon=0x1F))], "party Digimon 0x1F = records_digimon[7].id: 1"),
            ("cond8_index8", 8, 0, [member_write(0, 0, **dict(PARTY0, digimon=0x182))], "party Digimon 0x182 = records_digimon[8].id: past the 8 searched, 0"),
            ("cond10_other", 10, 0xD9, [], "enemy 1 is 0xD9, alive: 1"),
            ("cond10_self", 10, 0x20, [], "0x20 is the current member itself (slot 0), not counted: 0"),
            ("cond10_any_dead", 10, 0, [member_write(1, 1, **ENEMY1, hp=0)], "value 0 (any), the only other member at 0 HP: 0"),
            ("cond18_turn0", 18, 2, [], "turns 0: 0 % 2 == 0: 1"),
            ("cond18_turn3", 18, 2, [member_write(1, 0, **ENEMY0, turns=3)], "turns 3: 3 % 2 = 1: 0")):
        out.append(case(name, cond, [type_, value], writes, reads=(READ_RNG,), comment=comment))

    act = "fightstg_enemy_get_action"
    out.append(case("action_tech2_current1", act, [2], [current(1)], reads=(), comment="type 2, current enemy 1 (0xD9): its record's tech_2, 182"))
    out.append(case("action_call_any", act, [8], [], reads=(), comment="type 8: -5 (call any member); 4..8 are -1..-5"))

    def turn(action):
        b = bytearray(TURN_SIZE)
        struct.pack_into("<i", b, TURN_ACTION, action)
        return {"enemy_turn": bytes(b)}
    find = "fightstg_enemy_turn_find_target"
    third = member_write(1, 2, digimon=0x3, max_hp=200, max_mp=10)
    for name, action, writes, comment in (
            ("call_member1", -3, [], "action -3: member 1 (0xD9) alive, another Digimon: 1"),
            ("call_same_digimon", -3, [member_write(1, 1, **dict(ENEMY1, digimon=0x20))], "action -3: member 1 is a second 0x20, the current member's Digimon: -1"),
            ("call_any_one", -5, [rng(1)], "action -5: one other live member (1): 1, no draw"),
            ("call_any_two", -5, [third, rng(1)], "action -5: members 1 and 2: found[next() & 1], draw 1151: 2")):
        out.append(Case(name, [Call(find, [("buf", "enemy_turn")], "s32", [READ_RNG], comment=comment, writes=writes)],
                        fixture=FIXTURE, saves=SAVES, buffers=turn(action), comment=f"FightstgEnemyTurn.action = {action}"))
    return out
