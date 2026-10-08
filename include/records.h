#ifndef RECORDS_H
#define RECORDS_H

#include "common.h"

typedef struct RecordsDigimon {
    /* 0x00 */ u16 id; /* Digimon ID (records_find_digimon) */
    /* 0x02 */ u16 stats[6];  /* base stats, copied to GamestateStats.stats by gamestate_init_records */
    /* 0x0E */ u16 resists[7]; /* base resistances, copied to GamestateStats.resists (FightstgStats.resists[0..6]) */
    /* 0x1C */ u16 techniques[7]; /* techniques it learns ([1..6], STFGTREP); [0] and [6] are read by FIGHTSTG
                               * (fightstg_counter_update, fightstg_status_update) */
    /* 0x2A */ u16 pair_technique; /* offered when its partner is switched in: level_thresholds[6] is that Digimon's
                                 * name_id (fightstg_switch_menu_update) */
    /* 0x2C */ u8 status_resists[5]; /* FightstgStats.resists[7..11] (fightstg_rules_get_stats) */
    /* 0x31 */ u8 technique_levels[6]; /* the level at which it learns techniques[i + 1] */
    /* 0x37 */ u8 level_thresholds[7]; /* level thresholds ([1..5], STFGTREP) */
    /* 0x3E */ u8 exp_curve; /* experience curve factor (STFGTREP) */
    /* 0x3F */ u8 start_hp;   /* HP and max HP at a new game (gamestate_init_records) */
    /* 0x40 */ u8 start_mp;   /* MP and max MP at a new game */
    /* 0x41 */ u8 hp_mp_gains[2]; /* max HP/MP gain base per level (STFGTREP) */
    /* 0x43 */ u8 stat_gains[6]; /* gain class of GamestateStats.stats per level (STFGTREP) */
    /* 0x49 */ u8 resist_gains[7]; /* gain class of GamestateStats.resists per level (STFGTREP) */
    /* 0x50 */ u8 blast_forms[5]; /* its blast forms by level band (<4, <19, <39, <70, more): records_digimon
                                   * index + 1 (WFIGHTMN) */
    /* 0x55 */ u8 name_id;    /* its entry in the Digimon names (?SDIGNAM) */
    /* 0x56 */ u8 type;   /* its type (FightstgStats.type: a technique or weapon whose strong_type is it does 1.5x) */
    /* 0x57 */ u8 pad_57;
} RecordsDigimon; /* size 0x58 */

/* RecordsItem.category (records_is_item_category; STITSHOP sells and buys categories 3-5). */
enum RecordsItemCategory {
    RECORDS_ITEM_KEY = 1,       /* key items (type 29): packs, DDNA, IDs, badges, ... */
    RECORDS_ITEM_USABLE = 2,    /* RecordsUsable (types 25-28) */
    RECORDS_ITEM_WEAPON = 3,    /* RecordsWeapon (types 2-14) */
    RECORDS_ITEM_ARMOR = 4,     /* RecordsArmor (types 15-20) */
    RECORDS_ITEM_ACCESSORY = 5  /* RecordsAccessory (types 21-24) */
};

typedef struct RecordsItem {
    /* 0x00 */ void *data;   /* its data, by type: RecordsWeapon, RecordsArmor, RecordsAccessory or RecordsUsable; NULL
                              * for key items (gamestate_unequip_item, gamestate_get_stats) */
    /* 0x04 */ u16 price;  /* price (STITSHOP) */
    /* 0x06 */ u16 sell_price; /* selling price (STITSHOP) */
    /* 0x08 */ u8 category; /* RecordsItemCategory (records_is_item_category) */
    /* 0x09 */ u8 type;     /* 2-14 weapons, 15-20 armour, 21-24 accessories, 25-28 usable, 29 key items; its icon is
                             * records_item_icons[type] */
    /* 0x0A */ u8 pad_0A[0x2];
} RecordsItem; /* size 0xC */

/* An equipment item's bonus stats (RecordsWeapon.bonus_stats, RecordsArmor.bonus_stats, RecordsAccessory.bonus_stat;
 * gamestate_add_stat_bonus): GamestateStats.stats[0..5] and resists[0..6], named after the items that raise them
 * (?SITMNAM: "Power Gem" raises 1, "Flame Ring" 8, ...). 17-21 (accessories only) are FightstgStats.resists[7..11]
 * (fightstg_rules_get_stats); "Antidote Ring" 17, "Revive Ring" 18, "Sober Ring" 19, "Awake Ring" 20, "Prayer Ring" 21. */
enum RecordsBonusStat {
    RECORDS_BONUS_NONE = 0,
    RECORDS_BONUS_POWER = 1,    /* stats[0] (penalties[0] lowers it) */
    RECORDS_BONUS_GUARD = 2,    /* stats[1] (penalties[1]) */
    RECORDS_BONUS_SPIRIT = 3,   /* stats[2] */
    RECORDS_BONUS_WISDOM = 4,   /* stats[3] */
    RECORDS_BONUS_BOOST = 5,    /* stats[4] (penalties[2]) */
    RECORDS_BONUS_CHARISMA = 6, /* stats[5] */
    RECORDS_BONUS_ALL = 7,      /* stats[0..5] (no item uses it) */
    RECORDS_BONUS_FIRE = 8,     /* resists[0] */
    RECORDS_BONUS_WATER = 9,    /* resists[1] */
    RECORDS_BONUS_ICE = 10,     /* resists[2] */
    RECORDS_BONUS_WIND = 11,    /* resists[3] */
    RECORDS_BONUS_THUNDER = 12, /* resists[4] */
    RECORDS_BONUS_MACHINE = 13, /* resists[5] */
    RECORDS_BONUS_DARK = 14     /* resists[6] */
};

/* The start of every equipment item's data (RecordsWeapon, RecordsArmor, RecordsAccessory). */
typedef struct RecordsEquip {
    /* 0x0 */ u16 charisma; /* added to GamestateStats.stats[5] (gamestate_get_stats) */
    /* 0x2 */ u8 slot;      /* slot kind, as GamestateRecord.equipment indices (stitshop_get_slot): 4 head [0], 5 body
                             * [1], 3 hand [2] or [3], 1 [2], 2 shield [3], 7 both hands [2] and [3]
                             * (gamestate_unequip_item), 6 ring [4] or [5], 8 crest [4] or [5] (replaces one of its
                             * group) */
    /* 0x3 */ u8 group;     /* group: two of a group can't be worn together (STITSHOP, STSTATUS) */
    /* 0x4 */ u8 members;   /* bit i: gamestate_data.digimon[i] can wear it (stitshop_can_equip) */
    /* 0x5 */ u8 pad_05;
} RecordsEquip; /* size 0x6 */

/* RecordsItem.data of a weapon (types 2-14). */
typedef struct RecordsWeapon {
    /* 0x00 */ u16 charisma;
    /* 0x02 */ u8 slot;
    /* 0x03 */ u8 group;
    /* 0x04 */ u8 members;
    /* 0x05 */ u8 pad_05;
    /* 0x06 */ s16 bonus_values[2]; /* added to bonus_stats[i] (gamestate_add_stat_bonus) */
    /* 0x0A */ u16 power;           /* added to GamestateStats.stats[0] */
    /* 0x0C */ u8 bonus_stats[2];   /* RecordsBonusStat */
    /* 0x0E */ u8 accuracy;         /* added to FightstgStats.accuracy */
    /* 0x0F */ u8 unk_0F;
    /* 0x10 */ u8 status_chance;    /* chance of its status effect, for the items fightstg_rules_get_stats knows */
    /* 0x11 */ u8 status_power;     /* power of that effect (or the critical-hit rate) */
    /* 0x12 */ u8 strong_type;      /* FightstgStats.strong_types when >= 2 */
    /* 0x13 */ u8 pad_13;
} RecordsWeapon; /* size 0x14 */

/* RecordsItem.data of armour (types 15-20). */
typedef struct RecordsArmor {
    /* 0x00 */ u16 charisma;
    /* 0x02 */ u8 slot;
    /* 0x03 */ u8 group;
    /* 0x04 */ u8 members;
    /* 0x05 */ u8 pad_05;
    /* 0x06 */ s16 bonus_values[2]; /* added to bonus_stats[i] (gamestate_add_stat_bonus) */
    /* 0x0A */ u8 bonus_stats[2];   /* RecordsBonusStat */
    /* 0x0C */ u16 guard;           /* added to GamestateStats.stats[1] */
    /* 0x0E */ u8 unk_0E[0x2];
    /* 0x10 */ u8 evasion;          /* added to FightstgStats.evasion */
    /* 0x11 */ u8 unk_11[0x3];
} RecordsArmor; /* size 0x14 */

/* RecordsItem.data of an accessory (types 21-24): a ring raises one stat, a crest has the effect FIGHTSTG gives
 * its item ID. */
typedef struct RecordsAccessory {
    /* 0x0 */ u16 charisma;
    /* 0x2 */ u8 slot;
    /* 0x3 */ u8 group;
    /* 0x4 */ u8 members;
    /* 0x5 */ u8 pad_05;
    /* 0x6 */ s16 bonus_value; /* added to bonus_stat; a crest's strength (FIGHTSTG) */
    /* 0x8 */ u8 bonus_stat;   /* RecordsBonusStat (17-21 too) */
    /* 0x9 */ u8 unk_09[0x3];
} RecordsAccessory; /* size 0xC */

/* RecordsItem.data of a usable item (types 25-28). */
typedef struct RecordsUsable {
    /* 0x0 */ u8 flags;   /* bit 0: usable from the menu (STSTATUS), bit 1: in battle (FIGHTSTG); the boosters have 4 */
    /* 0x1 */ u8 effect;  /* menu effect: 1 heals HP, 17 raises GamestateStats.values[1], others raise a stat
                           * (ststatus_items_use) */
    /* 0x2 */ u16 amount; /* the amount healed or raised (FIGHTSTG, STSTATUS) */
} RecordsUsable; /* size 0x4 */

/* records_state.enemies: an enemy of the battle, copied from FIELDSTG's battle table (fieldstg_start_battle). */
typedef struct RecordsEnemy {
    /* 0x0 */ s32 digimon; /* (D_FIGHTSTG_800A37FC) */
    /* 0x4 */ s16 level;
    /* 0x6 */ s16 hp;    /* (WFIGHTMN's wfightmn_init_members) */
    /* 0x8 */ s16 mp;
    /* 0xA */ s16 stat_scale; /* / 16 (fightstg_rules_get_stats) */
} RecordsEnemy; /* size 0xC */

/* records_state: this file's state, then records_battle_results and its function table records_funcs (separate objects:
 * the overlays reach each one through its own %hi/%lo). */
typedef struct RecordsState {
    /* 0x00 */ s32 encounters; /* non-zero: random battles on (fieldstg_encounter_step; STAGSLCT's debug menu
                                * toggles it) */
    /* 0x04 */ s32 debug_up_down;    /* -1..3: STAGSLCT's debug menu steps it (pad 2 up/down); nothing reads it */
    /* 0x08 */ s32 debug_left_right; /* -1..7: the same with pad 2's right/left */
    /* 0x0C */ s32 stage;  /* the battle stage (SFSTDATA record; fightstg_stage_create) */
    /* 0x10 */ s32 battle; /* index into FIELDSTG's fieldstg_battles */
    /* 0x14 */ s32 music;  /* the battle music (a sound key, played by FIGHTSTG) */
    /* 0x18 */ RecordsEnemy enemies[3];
    /* 0x3C */ u8 first_strike_chance; /* from FIELDSTG's battle table, like the next two: the party's chance to strike
                                     * first, 0: never (WFIGHTMN) */
    /* 0x3D */ u8 battle_kind; /* FieldstgBattle.kind (1-5); FIGHTSTG's script condition 13 compares it */
    /* 0x3E */ u8 blocked[12]; /* set: the effect fails against this battle's enemy (FIGHTSTG): [0] poison, [1] paralysis,
                             * [2] confusion, [3] sleep, [4] knockout, [5] drain and item 0x55, [7] steal, [8] power down,
                             * [9] guard down, [10] item 0x59, [11] escape (fightstg_rules_roll_*, the item effects) */
    /* 0x4C */ s32 has_prize; /* the first enemy is 0x1C9..0x1D0: a prize battle */
    /* 0x50 */ s32 prize_item; /* then the item won (records_battle_results.item): chosen from the map and a random
                                * value (fieldstg_start_battle) */
    /* 0x54 */ void (*clear_gauges)(void); /* records_clear_gauges */
    /* 0x58 */ s16 gauges[8]; /* per Digimon, 0..1000: raised by fightstg_rules.get_gauge_gain (WFIGHTMN) and items
                               * (FIGHTSTG) */
} RecordsState; /* size 0x68 */

/* records_battle_results: the last battle's results (read by STFGTREP, the battle report). */
typedef struct RecordsBattleResults {
    /* 0x0 */ s16 battle; /* index into STFGTREP's reward table */
    /* 0x2 */ s16 item;   /* item won (ITMNAM; STFGTREP adds it), 0: none */
    /* 0x4 */ s16 member; /* party member */
    /* 0x6 */ struct {
        u8 took_part;
        u8 forms[3]; /* per chosen form (gamestate_get_chosen_forms): fought as it */
    } members[3];
    /* 0x12 */ u8 unk_12[0x2];
} RecordsBattleResults; /* size 0x14 */

/* records_funcs: this file's function table, right after records_battle_results. A separate object: callers that
 * keep its address in a register hold records_state + 0x7C, never records_state (FIGHTSTG's item menu
 * fightstg_item_menu_draw, gamestate_get_stats, STITSHOP). */
typedef struct RecordsFuncs {
    /* 0x0 */ RecordsItem *(*get_item)(s32 i);              /* records_get_item */
    /* 0x4 */ s32 (*get_item_icon)(s32 i);                  /* records_get_item_icon */
    /* 0x8 */ s32 (*is_item_category)(s32 i, s32 category); /* records_is_item_category */
    /* 0xC */ s32 (*list_items)(s32 arg0, u16 *out);        /* records_list_items */
} RecordsFuncs; /* size 0x10 */

extern RecordsState records_state;
extern RecordsBattleResults records_battle_results;
extern RecordsFuncs records_funcs;
extern RecordsDigimon records_digimon[];

/* records_techniques: the techniques, 0x12-byte records from ID 1 (records_techniques[id - 1]). GCC folds the
 * - 1 into the address, records_techniques - 0x12, which splat's asm (and m2c's drafts) in the overlays shows as
 * records_item_lists + 0x2: that is records_techniques[id - 1], not the item lists. */
typedef struct RecordsTechnique {
    /* 0x00 */ u16 mp_cost; /* MP cost */
    /* 0x02 */ u16 power;  /* power (FIGHTSTG) */
    /* 0x04 */ u8 element; /* element: icon 0x37 + element (STGDGLAB, STSTATUS) */
    /* 0x05 */ u8 target;  /* target: 3 one member (STSTATUS's healing techniques) */
    /* 0x06 */ u8 accuracy; /* hit chance, out of 128, before stats and levels (fightstg_rules_roll_hit) */
    /* 0x07 */ u8 defense_stat; /* 2..: the target's stat GamestateStats.stats[defense_stat - 2] divides the damage
                               * (FIGHTSTG) */
    /* 0x08 */ u8 element_power; /* with defense_stat >= 2, as FightstgStats.attack_element_power: damage += damage *
                                * element_power * 2 / the target's resists[defense_stat - 2] (fightstg_rules) */
    /* 0x09 */ u8 strong_type;   /* >= 2: 1.5x damage against a target of this type (FightstgStats.type) */
    /* 0x0A */ u8 kind;    /* kind (FIGHTSTG): < 2 uses the attacker's item effects instead of effect_chance/power */
    /* 0x0B */ u8 effect_chance; /* chance of its status effect, out of 128 (fightstg_rules_roll_poison, ...,
                                * fightstg_rules_roll_switch_seal) */
    /* 0x0C */ u8 effect_power; /* its effect's power: poison damage (fightstg_rules_roll_poison), STSTATUS's healing
                                * (power * 64 + power * stat / 8) */
    /* 0x0D */ u8 anim_stage;  /* its animation: FightstgScript.stage, */
    /* 0x0E */ u8 anim_effect; /* .effect, */
    /* 0x0F */ u8 hit_sound;   /* .hit_sound */
    /* 0x10 */ u8 anim_script; /* and .script (5, 6, 0xB, 0xC change how it hits: FIGHTSTG, WFIGHTMN) */
    /* 0x11 */ u8 hit_count;   /* kind >= 2: the number of hits (fightstg_action_hit_again; else 3) */
} RecordsTechnique; /* size 0x12 */

extern RecordsTechnique records_techniques[];

/* records_get_digimon, in this file's .sdata. */
extern RecordsDigimon *(*records_get_digimon_func)(s32 id);

/* Settings in .sdata: records_language and records_60hz end this file's .sdata (main reads CCAC with %hi/%lo,
 * so it is not main's); main_screen_pos starts main's (main.c). DECISIONS "Data in C, split per object". */
extern s32 records_language; /* language: 0 Japanese, 1 USA, 2-6 Europe; offsets text file IDs */
extern s32 records_60hz; /* non-zero: 60 Hz (the NTSC patch sets it) */
extern s32 main_screen_pos; /* non-zero: NTSC screen position (the NTSC patch sets it) */

#endif /* RECORDS_H */
