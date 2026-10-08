"""Shared fixture builders for the battle (FIGHTSTG) families: struct packers for include/fightstg.h, include/records.h
and include/gamestate.h, and the setup case that puts FIGHTSTG.PRO in the overlay slot with the enemy records file."""
import struct

from oracle import Call, Case, Write

OVERLAY_SLOT = 0x80082CB0                      # tier-1 overlays run here (docs/DISC_LAYOUT.md)
ENEMY_FILE_ADDR = 0x80190000                   # SDIGIEDT.PRO (file 0x1CF, 14 KB): free heap, above the scratch buffers
FIGHTSTG_PRO = "extracted/disc/AAA/PRO/FIGHTSTG.PRO"
SDIGIEDT_PRO = "extracted/disc/AAA/PRO/SDIGIEDT.PRO"
CDLOAD_ENTRY = 4 + 63 * 0x10                   # cdload_module.entries[63]

# Offsets (include/gamestate.h, include/fightstg.h, include/records.h).
GS = dict(party=0x70, digimon=0x75C, digimon_size=0x3DC, record=0xC, stats=0x1C, equipment=0x3C0)
BATTLE_STATE = 0x08                            # FightstgBattle.state
STATE = dict(current=0x00, members=0x08, member_size=0x20, field=0xC8, escapes=0xCC, type=0xCE)
RS = dict(enemies=0x18, enemy_size=0xC, blocked=0x3E)


def member(digimon=0, max_hp=0, hp=None, max_mp=0, mp=None, power_up=0, modifiers=(0, 0, 0, 0), item=-1, blasted=0,
           boosted=0, status=0, paralysis_power=0, sleep_power=0, confusion_power=0, base_digimon=None, turns=0):
    """FightstgMember (0x20 bytes)."""
    return struct.pack("<8h4hh6B", digimon, digimon if base_digimon is None else base_digimon, turns, max_hp,
                       max_hp if hp is None else hp, max_mp, max_mp if mp is None else mp, power_up, *modifiers, item,
                       blasted, boosted, status, paralysis_power, sleep_power, confusion_power)


def battle_state(current=(0, 0), party=(), enemies=(), field=(0, 0), escapes=0, type_=0):
    """FightstgBattleState (0xD4 bytes): members[0] = party, members[1] = enemies (3 each, missing ones empty)."""
    empty = member()
    mem = list(party) + [empty] * (3 - len(party)) + list(enemies) + [empty] * (3 - len(enemies))
    return struct.pack("<2i", *current) + b"".join(mem) + struct.pack("<hhhhhbB", field[0], field[1], escapes, type_, 0, 0, 0)


def enemy(digimon, level, hp, mp, stat_scale=16):
    """RecordsEnemy (0xC bytes): records_state.enemies[i]."""
    return struct.pack("<ihhhh", digimon, level, hp, mp, stat_scale)


def gs_stats(level, hp, max_hp, mp, max_mp, stats, resists, penalties=(0, 0, 0)):
    """GamestateStats (0x2C bytes): values {level, ?, hp, max hp, mp, max mp}, stats[6], resists[7], penalties[3]."""
    return struct.pack("<6h6h7h3h", level, 0, hp, max_hp, mp, max_mp, *stats, *resists, *penalties)


def fightstg_stats(level=0, stats=(1, 1, 1, 1, 1), resists=(100,) * 12, status=0, type_=0, power_up=0, guard=0,
                   strong_types=(0, 0, 0), **u8s):
    """FightstgStats (0x40 bytes); the trailing u8 fields by name (attack_element, critical, escape, ...)."""
    names = ["attack_element", "attack_element_power", "poison_chance", "poison_power", "paralysis_chance",
             "paralysis_power", "confusion_chance", "confusion_power", "knockout_chance", "knockout_power",
             "drain_chance", "drain_power", "multi_hit", "critical", "counter", "accuracy", "evasion", "escape",
             "no_escape", "steal", "pad_3F"]
    tail = [u8s.pop(n, 0) for n in names]
    assert not u8s, u8s
    return struct.pack("<h5h12h4B3B21B", level, *stats, *resists, status, type_, power_up, guard, *strong_types, *tail)


def record_writes(i, level, hp, max_hp, mp, max_mp, stats, resists, equipment=(0,) * 6, who=""):
    """Writes for gamestate_data.digimon[i].record: its stats and equipment."""
    base = GS["digimon"] + i * GS["digimon_size"] + GS["record"]
    return [Write("gamestate_data", base + GS["stats"], gs_stats(level, hp, max_hp, mp, max_mp, stats, resists),
                  f"digimon[{i}].record.stats{who}: level {level}, HP {hp}/{max_hp}, MP {mp}/{max_mp}, stats {list(stats)}, resists {list(resists)}"),
            Write("gamestate_data", base + GS["equipment"], struct.pack("<6h", *equipment),
                  f"digimon[{i}].record.equipment = {list(equipment)}")]


def setup_case(name="setup_fightstg"):
    """The kept case: FIGHTSTG.PRO into the overlay slot (its .data/.bss are inside the file), SDIGIEDT.PRO (the enemy
    records fightstg_enemy_records.get reads through cdload_module.files.get_file(0x1CF)) into free RAM, registered as a
    loaded cdload entry so no CD read happens. One smoke call checks the overlay answers."""
    entry = struct.pack("<hhiiI", 3, 0, 0x1CF, 0, ENEMY_FILE_ADDR)
    return Case(name, [Call("fightstg_rules_get_field_bonus", [1000, 4], "s32",
                            comment="smoke: no field element yet (the slot's data is the file's), so 0")],
                fixture=[Write(OVERLAY_SLOT, 0, b"", "overlay slot: FIGHTSTG.PRO", file=FIGHTSTG_PRO),
                         Write(ENEMY_FILE_ADDR, 0, b"", "SDIGIEDT.PRO, the enemy records (file 0x1CF)", file=SDIGIEDT_PRO),
                         Write("cdload_module", CDLOAD_ENTRY, entry,
                               "cdload_module.entries[63] = {state 3 (loaded), id 0x1CF, buffer -> the file above}")],
                keep=True, comment="setup, kept for the family: the overlay and the enemy file; nothing is restored")
