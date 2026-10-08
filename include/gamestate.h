#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

/* Element of GamestateRecord.forms (gamestate_find_form searches it; gamestate_get_form/gamestate_put_form copy it). */
typedef struct GamestateForm {
    /* 0x00 */ s16 id;
    /* 0x02 */ s8 level;   /* level; set to 1 by gamestate_add_form; read as s8 by FIGHTSTG (fightstg_status_update) */
    /* 0x03 */ u8 pad_03;
    /* 0x04 */ s32 exp;  /* the form's experience, <= 9999999 (STFGTREP adds it and raises level from it) */
    /* 0x08 */ s16 techniques[6]; /* techniques learnt (STFGTREP); flags 0x2000, 0x4000 (FIGHTSTG
                                 * fightstg_status_update), 0x8000 */
} GamestateForm; /* size 0x14 */

/* GamestateRecord.stats, copied out whole by gamestate_get_stats: 19 stats (gamestate_set_stat: stat 0 <= 99,
 * 2-5 <= 9999, the rest <= 999), in three arrays: STGTRAIN raises stats[type - 1] and resists[type - 8] and only
 * matches with them (their offsets fold into the loads). Code that picks a stat by a table index (0-18) reads
 * the whole block through values. */
typedef struct GamestateStats {
    /* 0x00 */ s16 values[6]; /* level, ?, HP, max HP, MP, max MP */
    /* 0x0C */ s16 stats[6];  /* stats 6-11 */
    /* 0x18 */ s16 resists[7]; /* stats 12-18 */
    /* 0x26 */ s16 penalties[3]; /* taken off stats[0], [1] and [4] (gamestate_get_stats); cleared by inn_heal_party */
} GamestateStats; /* size 0x2C */

/* What gamestate_get_record returns: gamestate_data.digimon[i].record. */
typedef struct GamestateRecord {
    /* 0x000 */ char name[0x18];    /* name (gamestate_init_records) */
    /* 0x018 */ s32 exp;     /* experience (STFGTREP) */
    /* 0x01C */ GamestateStats stats;
    /* 0x048 */ s16 chosen_forms[3];
    /* 0x04E */ u8 pad_04E[0x2];
    /* 0x050 */ GamestateForm forms[44];
    /* 0x3C0 */ s16 equipment[6]; /* equipment: item IDs (gamestate_unequip_item, gamestate_get_stats) */
    /* 0x3CC */ u8 last_bonus_training; /* the training whose bonus round it won last (STGTRAIN: no bonus for the
                                       * same training twice in a row), 0: none */
    /* 0x3CD */ u8 pad_3CD[0x3];
} GamestateRecord; /* size 0x3D0 */

/* A Digimon's record: gamestate_data.digimon[i] (0x80049490). The overlays take the record's address as
 * gamestate_data + 0x75C + i * 0x3DC and read the form at +8 (STGDGLAB, STSTATUS), so the record starts at 0x75C,
 * not 0x760, and GamestateRecord is 0x3D0 bytes (session 6). */
typedef struct GamestateDigimon {
    /* 0x000 */ s32 unk_000;
    /* 0x004 */ s32 joined;  /* i + 3 once Digimon i has joined (gamestate_cond_digimon type 3), 0: not yet;
                              * gamestate_get_party_digimon returns it - 3 */
    /* 0x008 */ s32 shown_form; /* the form it is shown as (a Digimon ID; STGDGLAB, STSTATUS), 0: its own */
    /* 0x00C */ GamestateRecord record;
} GamestateDigimon; /* size 0x3DC */

/* Entry of the array at gamestate_data + 0x628 (gamestate_init_records, gamestate_init_cards). */
typedef struct GamestateDeck {
    /* 0x00 */ char name[0x16];
    /* 0x16 */ u16 cards[40];
} GamestateDeck; /* size 0x66 */

/* The function table at gamestate_data.funcs (0x8004B430). */
typedef struct GamestateFuncs {
    /* 0x00 */ void (*new_game)();        /* gamestate_new_game */
    /* 0x04 */ void (*change_map)();      /* gamestate_change_map */
    /* 0x08 */ s32 (*get_map)();          /* gamestate_get_map */
    /* 0x0C */ s32 (*get_map_entry)();    /* gamestate_get_map_entry */
    /* 0x10 */ void (*set_next_map)();    /* gamestate_set_next_map */
    /* 0x14 */ s32 (*is_map_changing)();  /* gamestate_is_map_changing */
    /* 0x18 */ s32 (*get_prev_map)();     /* gamestate_get_prev_map */
    /* 0x1C */ s32 (*get_party_member)(s32 i); /* gamestate_get_party_member: party[i], or -1 */
    /* 0x20 */ void (*set_party)(s32);    /* gamestate_set_party */
    /* 0x24 */ void (*add_card)(s32, s32); /* gamestate_add_card */
    /* 0x28 */ void (*init_cards)();      /* gamestate_init_cards */
    /* 0x2C */ s32 (*get_party_digimon)(); /* gamestate_get_party_digimon: party member i's Digimon, or -1 */
    /* 0x30 */ void (*set_stat)();        /* gamestate_set_stat */
    /* 0x34 */ void (*add_stat)();        /* gamestate_add_stat */
    /* 0x38 */ void (*get_stats)(s32 i, GamestateStats *out); /* gamestate_get_stats */
    /* 0x3C */ s32 (*get_chosen_forms)(s32 i, s16 *out); /* gamestate_get_chosen_forms */
    /* 0x40 */ void (*set_chosen_forms)(); /* gamestate_set_chosen_forms */
    /* 0x44 */ s32 (*list_forms)();       /* gamestate_list_forms */
    /* 0x48 */ s32 (*add_form)();         /* gamestate_add_form */
    /* 0x4C */ s32 (*get_form)(s32 i, s32 id, GamestateForm *out); /* gamestate_get_form: copies Digimon i's form
                                                                * record */
    /* 0x50 */ s32 (*put_form)();         /* gamestate_put_form: writes it back */
    /* 0x54 */ GamestateRecord *(*get_record)(s32 i); /* gamestate_get_record */
    /* 0x58 */ void (*reset_playtime)();  /* gamestate_reset_playtime */
    /* 0x5C */ void (*tick_playtime)();   /* gamestate_tick_playtime */
} GamestateFuncs;

/* An s32 position (24.8): gamestate_data.player_pos, where FIELDSTG saves the player's position (copied whole from
 * its actor's unk_50). */
typedef struct GamestatePos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} GamestatePos; /* size 0x8 */

/* gamestate_data: this file's state and its function table (funcs); inn_heal_party reaches table
 * entries through gamestate_data's address. */
typedef struct GamestateData {
    /* 0x0000 */ u8 checksum; /* of a saved slot: memcard_get_checksum of bytes 4..0x26C3 (STGMCARD) */
    /* 0x0001 */ u8 unk_0001;
    /* 0x0002 */ u8 version;  /* of a saved slot: 4 (STGMCARD) */
    /* 0x0003 */ u8 unk_0003;
    /* 0x0004 */ s8 digivolve_demo; /* the battle Digivolve demo is shown (STSTATUS's option "Digivolve Demo": ON/OFF);
                                   * 1 at a new game */
    /* 0x0005 */ u8 unk_0005[0x7];
    /* 0x000C */ s32 unk_000C;
    /* 0x0010 */ u8 unk_0010[0x18];
    /* 0x0028 */ s32 stage_select_first; /* STAGSLCT (the debug stage select): first stage shown */
    /* 0x002C */ s32 stage_select_cursor; /* STAGSLCT: stage under the cursor (from stage_select_first) */
    /* 0x0030 */ s32 encounter_timer;
    /* 0x0034 */ s32 field_map;
    /* 0x0038 */ GamestatePos player_pos; /* the player's field position (FIELDSTG) */
    /* 0x0040 */ s32 player_dir;         /* its direction */
    /* 0x0044 */ u16 route; /* a position along a route of maps: the route (1..30) and the room along it, set on arrival
                          * by the exit taken (FIELDSTG: FieldstgMapEvent.route/room, or a warp's); the maze stages */
    /* 0x0046 */ u16 room;  /* pick their exits by them (WstagExits); flag type 0x7E tests them (gamestate_check_route) */
    /* 0x0048 */ s32 playtime_frames; /* 24.8 frames of 1/60 s: gfx adds 0x100 per frame at 60 Hz, 0x133 at 50 Hz */
    /* 0x004C */ s16 playtime[4]; /* hours (<= 999), minutes, seconds, 1: maxed (gamestate_tick_playtime) */
    /* 0x0054 */ char name[0x18];     /* name (gamestate_init_records) */
    /* 0x006C */ s32 money;    /* 0..9999999 (gamestate_change_money) */
    /* 0x0070 */ s32 party[3];    /* indices into digimon (gamestate_get_party_member, gamestate_set_party) */
    /* 0x007C */ s8 items[0x193];
    /* 0x020F */ s8 items_equipped[0x193];
    /* 0x03A2 */ s8 cards[0x13D];    /* counts, 0..9 (gamestate_change_card, gamestate_add_card) */
    /* 0x04DF */ u8 cards_obtained[0x13D]; /* set by gamestate_add_card; cards the player owns (STCRDABM reads it as u8) */
    /* 0x061C */ u8 unk_061C[0xC];
    /* 0x0628 */ GamestateDeck decks[3];
    /* 0x075A */ u8 pad_075A[0x2];
    /* 0x075C */ GamestateDigimon digimon[8];  /* the eight Digimon's records */
    /* 0x263C */ s32 progress; /* a progress value (gamestate_cond_progress, gamestate_check_progress; set by STAGSLCT's stage list,
                                * compared by FIELDSTG and STDWTITL); was "record 7's unk_3D0" */
    /* 0x2640 */ s32 party_set; /* the party chosen by gamestate_set_party (index into gamestate_parties) */
    /* 0x2644 */ u8 flags[0x12];    /* bit arrays of the flag types (gamestate_get_flag, gamestate_set_flag: type = flag >> 8
                                  * & ~1, index = flag & 0x1FF): this one type 0x02, flags_04..flags_40 the others */
    /* 0x2656 */ u8 flags_04[0x2];
    /* 0x2658 */ u8 flags_06[0x1];
    /* 0x2659 */ u8 flags_08[0x1];
    /* 0x265A */ u8 flags_0A[0x4];
    /* 0x265E */ u8 flags_0C[0x8];
    /* 0x2666 */ u8 flags_0E[0xC];
    /* 0x2672 */ u8 flags_10[0x4];
    /* 0x2676 */ u8 flags_18[0x2];
    /* 0x2678 */ u8 flags_1A[0x9];
    /* 0x2681 */ u8 flags_1C[0xB];
    /* 0x268C */ u8 flags_20[0x1E];
    /* 0x26AA */ u8 flags_40[0x1A];
    /* 0x26C4 */ s32 map;      /* the current map (gamestate_get_map); 0x1600 at a new game */
    /* 0x26C8 */ s32 next_map; /* set by gamestate_set_next_map, taken by gamestate_change_map (0: none) */
    /* 0x26CC */ s32 prev_map; /* the map before (gamestate_get_prev_map) */
    /* 0x26D0 */ s32 map_entry; /* entry point on the next map (gamestate_get_map_entry) */
    /* 0x26D4 */ u8 countdown[4]; /* a countdown (WSTAG795/800): three decimal digits, then frames */
    /* 0x26D8 */ s32 map_is_new; /* FIELDSTG entered another map than field_last_map (gamestate_update_map_flags then clears
                              * the map flags) */
    /* 0x26DC */ s32 field_last_map; /* the map (funcs.get_map) FIELDSTG last saw; map_is_new: it changed */
    /* 0x26E0 */ s32 attr_layer; /* attribute layer (FIELDSTG) */
    /* 0x26E4 */ s32 alt_layout; /* WSTAG810: its second layout is set (0x20: attribute file 0x06ED0004, its second
                               * map events), kept when the same map is entered again */
    /* 0x26E8 */ s32 player_depth; /* the player actor's depth: set by FIELDSTG's map events (type 5,
                                 * fieldstg_map_events_enter), 4 on a new map */
    /* 0x26EC */ s32 spot_target;  /* the spot to find (fieldstg_spots_pick_target) */
    /* 0x26F0 */ s32 screen_white; /* WSTAG745/746: their white flash is showing (it stays white across the map change) */
    /* 0x26F4 */ s32 player_height; /* the player actor's height, kept for the next map unless it is new (FIELDSTG) */
    /* 0x26F8 */ s32 meter_random_count; /* FIELDSTG: 16 on entering a new map; while > 0 each new meter (map event 7)
                                       * takes a random pattern and counts it down, else pattern 8 */
    /* 0x26FC */ GamestateFuncs funcs;
} GamestateData;

extern GamestateData gamestate_data;

/* gamestate_flags: the first gamestate module (gamestate_test_bit-gamestate_update_map_flags): its state and function
 * table, which game code calls through (the overlays as &gamestate_flags + 0xC). */
typedef struct GamestateFlags {
    /* 0x00 */ u8 map_flags[3]; /* bit array: flags of type 0 (gamestate_get_flag); cleared by gamestate_update_map_flags */
    /* 0x04 */ s32 card_game_won; /* the card game just played was won (CARDGAME); flag type 0x90 starts an event (a card
                                 * game) and copies it to flag 0x10, then clears it */
    /* 0x08 */ s32 (*check_flags)(u16 *list); /* gamestate_check_flags */
    /* 0x0C */ void (*set_flag)(s32, s32);    /* gamestate_set_flag */
    /* 0x10 */ s32 (*get_flag)(u16, u16);     /* gamestate_get_flag */
    /* 0x14 */ void (*set_flags)(u16 *list);  /* gamestate_set_flags */
    /* 0x18 */ void (*update_map_flags)(void); /* gamestate_update_map_flags */
} GamestateFlags; /* size 0x1C */

extern GamestateFlags gamestate_flags;

#endif /* GAMESTATE_H */
