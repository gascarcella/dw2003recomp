#ifndef FIGHTSTG_H
#define FIGHTSTG_H

/* FIGHTSTG.PRO (the battle): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "gfx.h"
#include "records.h"
#include "wfightmn.h" /* the tier-2 battle overlays FIGHTSTG calls into */
#include "psyq/libgs.h"
#include "psyq/libgte.h"

/* A VRAM position, passed by value (fightstg_slots_vram, D_FIGHTSTG_80082CCC). */
typedef struct FightstgPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} FightstgPos; /* size 0x8 */

/* One of the two layers a model is drawn on. */
typedef struct FightstgModelLayer {
    /* 0x0 */ s32 shown; /* drawn on it */
    /* 0x4 */ s32 layer_id; /* layer ID (gfx_get_layer) */
    /* 0x8 */ s32 edges; /* 0: mesh->queue_draw, else mesh->queue_edges */
} FightstgModelLayer; /* size 0xC */

typedef struct FightstgVec {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
} FightstgVec; /* size 0x6 */

/* Parameters of a model: the model keeps a pointer to them (FightstgModel.params); the object that creates the
 * model owns them. */
typedef struct FightstgModelParams {
    /* 0x00 */ s32 in_use; /* slot in use (fightstg_slots_create's objects) */
    /* 0x04 */ s32 id; /* ID of the slot */
    /* 0x08 */ s32 anim;
    /* 0x0C */ s32 restart;
    /* 0x10 */ s32 anim_done; /* animation ended */
    /* 0x14 */ s32 model_id; /* model ID (fightstg_models.get) */
    /* 0x18 */ s32 idle_anim;
    /* 0x1C */ FightstgVec pos; /* position */
    /* 0x22 */ FightstgVec rot;
    /* 0x28 */ FightstgVec start_pos; /* pos as set */
    /* 0x2E */ FightstgVec start_rot; /* rot as set */
    /* 0x34 */ FightstgModelLayer layers[2];
} FightstgModelParams; /* size 0x4C */

/* A part of a model: its transform relative to its parent and its world matrix. */
typedef struct FightstgModelPart {
    /* 0x00 */ s32 parent; /* parent part */
    /* 0x04 */ s32 mesh_file; /* sub-file ID of its mesh */
    /* 0x08 */ s32 keys_file;
    /* 0x0C */ s32 visible;
    /* 0x10 */ SVECTOR trans;
    /* 0x18 */ SVECTOR rot;
    /* 0x20 */ SVECTOR scale;
    /* 0x28 */ MATRIX local; /* local matrix */
    /* 0x48 */ MATRIX *parent_world; /* parent's world */
    /* 0x4C */ MATRIX world; /* world matrix */
    /* 0x6C */ SVECTOR saved_trans; /* trans, rot, scale as saved by fightstg_model_save_pose */
    /* 0x74 */ SVECTOR saved_rot;
    /* 0x7C */ SVECTOR saved_scale;
} FightstgModelPart; /* size 0x84 */

/* A model (fightstg_model_new): its parts and meshes, and a table of its functions. */
typedef struct FightstgModel {
    /* 0x0000 */ Object base;
    /* 0x0050 */ s32 part_count;
    /* 0x0054 */ FightstgModelPart *parts;
    /* 0x0058 */ SVECTOR move; /* moves the root part (in its axes), then cleared */
    /* 0x0060 */ s32 has_idle;
    /* 0x0064 */ FightstgModelParams *params;
    /* 0x0068 */ FightstgPos texture_pos; /* texture position in VRAM */
    /* 0x0070 */ s32 texture_file;
    /* 0x0074 */ s32 anim_file;
    /* 0x0078 */ s32 anim; /* animation (sub-file anim - 1 of file anim_file) */
    /* 0x007C */ s32 anim_done; /* the animation reached its end */
    /* 0x0080 */ s32 frame; /* entry of frames.. being played */
    /* 0x0084 */ s32 key; /* key frame shown */
    /* 0x0088 */ s32 idle_keys[2];
    /* 0x0090 */ s32 blend_to; /* key frame blended to */
    /* 0x0094 */ s32 blending;
    /* 0x0098 */ s32 blend; /* blend factor (0x1000 = 1) */
    /* 0x009C */ s32 to_idle;
    /* 0x00A0 */ s32 frame_count; /* entries in frames */
    /* 0x00A4 */ u16 frames[0x640]; /* per frame: key frame, or 0x8000 | blend factor (0x1000 = 1);
                                     * 0x8000: jump to entry frames_to, 0xFFFF: end */
    /* 0x0D24 */ u16 frames_to[0x640]; /* per frame: the key frame blended to */
    /* 0x19A4 */ u16 frames_from[0x640]; /* per frame: the key frame blended from (0xFFFF: the current pose) */
    /* 0x2624 */ void (*play_anim)(struct FightstgModel *, s32, s32);       /* fightstg_model_play_anim */
    /* 0x2628 */ s32 (*is_anim_done)(struct FightstgModel *);                  /* fightstg_model_is_anim_done */
    /* 0x262C */ void (*set_color)(struct FightstgModel *, s32, CVECTOR *); /* fightstg_model_set_color */
    /* 0x2630 */ void (*set_part_unclipped)(struct FightstgModel *, s32, s32);       /* fightstg_model_set_part_unclipped */
} FightstgModel; /* size 0x2634 */

/* fightstg_slots_create's object (on the object list as 0x14): four model slots and a table of its
 * functions. */
typedef struct FightstgSlots {
    /* 0x000 */ Object base;
    /* 0x050 */ FightstgModelParams slots[4];
    /* 0x180 */ void (*remove)(struct FightstgSlots *, s32);                /* fightstg_slots_remove */
    /* 0x184 */ void (*add)(struct FightstgSlots *, s32, s32, s32);      /* fightstg_slots_add */
    /* 0x188 */ FightstgModelParams *(*get_params)(struct FightstgSlots *, s32); /* fightstg_slots_get_params */
    /* 0x18C */ void (*set_id)(struct FightstgSlots *, s32, s32);           /* fightstg_slots_set_id */
    /* 0x190 */ s32 (*get_model_id)(struct FightstgSlots *, s32);                 /* fightstg_slots_get_model_id: model ID */
    /* 0x194 */ void (*reset_pos)(struct FightstgSlots *, s32);                /* fightstg_slots_reset_pos */
    /* 0x198 */ void (*set_idle_anim)(struct FightstgSlots *, s32, s32);           /* fightstg_slots_set_idle_anim */
} FightstgSlots; /* size 0x19C */

/* A lighting setup: three flat lights and the ambient colour (FightstgStageRecord.lights). */
typedef struct FightstgLights {
    /* 0x00 */ GsF_LIGHT lights[3];
    /* 0x30 */ s32 ambient_r; /* ambient r, g, b (SetBackColor) */
    /* 0x34 */ s32 ambient_g;
    /* 0x38 */ s32 ambient_b;
} FightstgLights; /* size 0x3C */

/* fightstg_lights_create's object (on the object list as 0x13): the battle's lighting, set on layer
 * layer_id and faded from fade_from to fade_to. */
typedef struct FightstgLighting {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer_id;
    /* 0x054 */ FightstgLights lights; /* current */
    /* 0x090 */ FightstgLights fade_to;
    /* 0x0CC */ FightstgLights fade_from;
    /* 0x108 */ s32 fade_pos; /* fade position (0x1000 = done) */
    /* 0x10C */ s32 fade_step;
    /* 0x110 */ void (*set)(struct FightstgLighting *, FightstgLights *); /* fightstg_lights_set */
    /* 0x114 */ void (*fade)(struct FightstgLighting *, FightstgLights *, FightstgLights *, s32); /* fightstg_lights_fade */
    /* 0x118 */ FightstgLights *(*get_stage)(struct FightstgLighting *, s32);   /* fightstg_lights_get_stage */
} FightstgLighting; /* size 0x11C */

/* A 0x58-byte record of file 0x1CB (cdload_get_file): a stage (model, sound, colours, lighting). */
typedef struct FightstgStageRecord {
    /* 0x00 */ s32 model_file; /* the model's sub-file ID */
    /* 0x04 */ s32 anim_file;
    /* 0x08 */ s32 sound; /* sound (index into fightstg_stage_sounds), or -1 */
    /* 0x0C */ u8 background[3]; /* background colour */
    /* 0x0F */ u8 unk_0F;
    /* 0x10 */ u8 unclipped_parts[8]; /* parts set unclipped (FightstgModel.set_part_unclipped), 0-terminated */
    /* 0x18 */ FightstgLights lights;
    /* 0x54 */ u8 unk_54[0x4];
} FightstgStageRecord; /* size 0x58 */

/* The object fightstg_stage_create creates (on the object list as 0x15: the stage; type function
 * fightstg_stage_update). */
typedef struct FightstgStage {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 stage; /* record of file 0x1CB */
    /* 0x54 */ s32 prev_stage; /* previous stage */
    /* 0x58 */ FightstgModelParams params; /* the model's parameters */
    /* 0xA4 */ s32 fade_pos; /* fade position (0x1000 = done) */
    /* 0xA8 */ s32 fade_step;
    /* 0xAC */ s32 fade_in; /* fade-in frames */
    /* 0xB0 */ s32 fade_out; /* fade-out frames */
    /* 0xB4 */ SVECTOR color_from; /* colour faded from */
    /* 0xBC */ SVECTOR color_to; /* colour faded to */
    /* 0xC4 */ SVECTOR color; /* colour */
    /* 0xCC */ SVECTOR background_from; /* background colour faded from */
    /* 0xD4 */ SVECTOR background_to; /* background colour faded to */
    /* 0xDC */ SVECTOR background; /* background colour */
    /* 0xE4 */ s16 sound_key;
    /* 0xE6 */ s16 pad_E6;
    /* 0xE8 */ void (*change)(struct FightstgStage *, s32, s32, s32); /* fightstg_stage_change */
} FightstgStage; /* size 0xEC */

/* A camera setting (fightstg_camera_setting; what the camera object copies, fightstg_camera_set). */
typedef struct FightstgCameraSetting {
    /* 0x00 */ s32 eye[3];
    /* 0x0C */ s32 target[3];
    /* 0x18 */ s32 trans[3];
    /* 0x24 */ s16 rot[3];
    /* 0x2A */ u8 pad_2A[0x2];
    /* 0x2C */ s32 roll;
    /* 0x30 */ s32 projection;
} FightstgCameraSetting; /* size 0x34 */

extern FightstgCameraSetting fightstg_camera_setting;

/* fightstg_camera_create's object (on the object list as 0x12: the camera), with a table of its
 * functions. */
typedef struct FightstgCamera {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 layer_id;
    /* 0x054 */ FightstgCameraSetting setting;
    /* 0x088 */ FightstgCameraSetting move_to; /* moved to (move) */
    /* 0x0BC */ FightstgCameraSetting move_from; /* moved from */
    /* 0x0F0 */ s32 move_pos; /* move position (0x1000 = done) */
    /* 0x0F4 */ s32 move_step; /* move step, 24.8 */
    /* 0x0F8 */ void (*set)(struct FightstgCamera *, FightstgCameraSetting *);           /* fightstg_camera_set */
    /* 0x0FC */ void (*move)(struct FightstgCamera *, FightstgCameraSetting *from, FightstgCameraSetting *to, s32 frames); /* fightstg_camera_move */
    /* 0x100 */ FightstgCameraSetting *(*get_default)(struct FightstgCamera *);                 /* fightstg_camera_get_default */
    /* 0x104 */ FightstgCameraSetting *(*get_preset)(struct FightstgCamera *, s32, s32);      /* fightstg_camera_get_preset */
} FightstgCamera; /* size 0x108 */

/* fightstg_script_create's object: runs a battle script (sub-file `script` of the model's script file),
 * one s16 command at a time. */
typedef struct FightstgScript {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 side; /* side: slot 0 or 0x10 of the 0x14 object */
    /* 0x54 */ s32 script;
    /* 0x58 */ s32 results[4];
    /* 0x68 */ s32 stage; /* stage */
    /* 0x6C */ s32 effect;
    /* 0x70 */ s32 hit_sound;
    /* 0x74 */ s32 unk_74; /* set by fightstg_player_reaction_update (its unk_5C); nothing reads it */
    /* 0x78 */ s32 model_id; /* model ID */
    /* 0x7C */ s32 slot;
    /* 0x80 */ s32 file; /* script file's sub-file ID */
    /* 0x84 */ s32 unk_84;
    /* 0x88 */ FightstgSlots *slots;
    /* 0x8C */ s16 *next; /* next command */
    /* 0x90 */ s32 child;
    /* 0x94 */ s32 sound_index;
    /* 0x98 */ s32 waiting; /* waiting (command 11) */
    /* 0x9C */ s32 wait_frames; /* frames left to wait */
    /* 0xA0 */ s32 load_state; /* state of command 4's op 1 */
    /* 0xA4 */ s32 image_file; /* command 4: image sub-file ID (fightstg_effect_get_files) */
    /* 0xA8 */ s32 model_file; /* command 4: model sub-file ID */
    /* 0xAC */ FightstgPos image_pos; /* command 4: VRAM position of the image */
} FightstgScript; /* size 0xB4 */

/* An action of a Digimon (FightstgEnemyRecord.actions): its type (fightstg_enemy_get_action) and its condition
 * (fightstg_enemy_check_condition). */
typedef struct FightstgEnemyAction {
    /* 0x0 */ u8 type; /* action type */
    /* 0x1 */ u8 condition; /* condition type */
    /* 0x2 */ s16 value; /* condition value */
} FightstgEnemyAction; /* size 0x4 */

/* A record of file 0x1CF (fightstg_enemy_find_record searches them by ID): a Digimon's data. */
typedef struct FightstgEnemyRecord {
    /* 0x00 */ s16 id; /* ID; 0 ends the list */
    /* 0x02 */ s16 item; /* item it holds (WFIGHTMN copies it to FightstgMember.item), 0 = none */
    /* 0x04 */ s16 drop_rate;
    /* 0x06 */ s16 name; /* name: line of text 0x4E */
    /* 0x08 */ s16 tech; /* its own action (records_techniques record) */
    /* 0x0A */ s16 tech_2;
    /* 0x0C */ s16 tech_3;
    /* 0x0E */ s16 stats[5]; /* stats (x records_state.enemies[].stat_scale / 16), as FightstgStats.stats */
    /* 0x18 */ s16 resists[12]; /* as FightstgStats.resists */
    /* 0x30 */ u8 type; /* as FightstgStats.type */
    /* 0x31 */ u8 pad_31;
    /* 0x32 */ FightstgEnemyAction actions[3]; /* the first whose condition holds is taken */
    /* 0x3E */ u8 unk_3E[0x4];
    /* 0x42 */ u8 counter_action;
    /* 0x43 */ u8 counter_condition;
    /* 0x44 */ s16 counter_value;
} FightstgEnemyRecord; /* size 0x46 */

/* fightstg_enemy_records: a one-entry function table (a struct: WFIGHTMN's wfightmn_init_members only matches when
 * the call's load waits for its struct-field stores, which GCC 2.8 does for a struct, not a scalar). */
typedef struct FightstgEnemyRecords {
    /* 0x0 */ FightstgEnemyRecord *(*get)(s32 id); /* fightstg_enemy_get_record */
} FightstgEnemyRecords; /* size 0x4 */

extern FightstgEnemyRecords fightstg_enemy_records;

/* An entry of the battle event queue (fightstg_events.events): an event type, the frames
 * before it runs, and its arguments. fightstg_events_take_next takes the entry with the smallest
 * delay and subtracts that delay from the others; WFIGHTMN runs a handler per type.
 * Types: 1 battle end (fightstg_events.result), 2 party turn, 3 enemy turn, 4 escape, 5 regeneration ends,
 * 6 regeneration (every 1000 frames; technique 0xBD, or item 0x140 equipped: fightstg_rules_get_regen), 7 field
 * effect ends, 8 counter stance (never runs: a kind-10 technique kept for fightstg_counter_update), 9..12 a status
 * ends (poison, paralysis, confusion, sleep), 13..15 a stat modifier ends, 16/17 seals (status 0x10 no switching,
 * 0x20 no digivolving or team technique), 0x12 gauge full, 0x13 blast ends, 0x14 knock-out, 0x15 boost ends,
 * 0x16 revert to the base form (technique 0x187), 0x17 the boss enters (battle type 5 becomes 6), 0x18 the boss's
 * weakened phase ends (fightstg_events_start_final_phase) (inferred from the handlers and the callers). */
typedef struct FightstgEvent {
    /* 0x00 */ s16 type; /* type; 0 = free */
    /* 0x02 */ s16 delay;
    /* 0x04 */ s32 args[6];
} FightstgEvent; /* size 0x1C */

/* An event as the callers build it (fightstg_new_event) before fightstg_events_add queues it. */
typedef struct FightstgNewEvent {
    /* 0x00 */ s32 type;
    /* 0x04 */ s32 delay;
    /* 0x08 */ s32 args[6];
} FightstgNewEvent; /* size 0x20 */

/* fightstg_new_event (an FightstgNewEvent) is defined in fightstg_8008D3B4.c, its only user. */

/* fightstg_events: the battle event queue and its function table. */
typedef struct FightstgEvents {
    /* 0x000 */ FightstgEvent events[99];
    /* 0xAD4 */ u8 unk_AD4[0x1C];
    /* 0xAF0 */ s8 taken_type; /* type of the event fightstg_events_take_next took */
    /* 0xAF1 */ s8 taken; /* its index */
    /* 0xAF2 */ s8 find_type; /* event type searched by fightstg_events_find_from */
    /* 0xAF3 */ s8 found; /* index found, -1 = none */
    /* 0xAF4 */ u8 result; /* the battle's end: 0 escaped, 1 won, 2 lost (fightstg_events_end_battle); WFIGHTMN compares it unsigned */
    /* 0xAF5 */ u8 status_power;
    /* 0xAF6 */ u8 pad_AF6[0x2];
    /* 0xAF8 */ void (*add)(FightstgNewEvent *event); /* fightstg_events_add */
    /* 0xAFC */ void (*add_first)(FightstgNewEvent *event); /* fightstg_events_add_first */
    /* 0xB00 */ s32 (*take_next)(void);                /* fightstg_events_take_next */
    /* 0xB04 */ s32 (*find)(s32 type);            /* fightstg_events_find */
    /* 0xB08 */ s32 (*find_next)(void);                /* fightstg_events_find_next */
    /* 0xB0C */ s32 (*find_member)(s32 type, u8 side, s32 arg2); /* fightstg_events_find_member */
    /* 0xB10 */ void (*remove_member)(s32 *arg0);          /* fightstg_events_remove_member */
    /* 0xB14 */ s32 (*get_delay)(u8 side, s32 kind);   /* fightstg_events_get_delay: a delay */
    /* 0xB18 */ void (*cure_status)();                   /* fightstg_events_cure_status */
} FightstgEvents; /* size 0xB1C */

extern FightstgEvents fightstg_events;

/* A combatant's battle state (fightstg_battle.state.members[side][i]). */
typedef struct FightstgMember {
    /* 0x00 */ s16 digimon; /* Digimon (fightstg_enemy_records), 0 = none */
    /* 0x02 */ s16 base_digimon; /* Digimon shown instead while blasted is set */
    /* 0x04 */ s16 turns;
    /* 0x06 */ s16 max_hp; /* max HP (maximum of hp) */
    /* 0x08 */ s16 hp; /* HP; 0 = out of the battle */
    /* 0x0A */ s16 max_mp;
    /* 0x0C */ s16 mp;
    /* 0x0E */ s16 power_up; /* cleared with sh (fightstg_counter_update); read into a u8 by fightstg_rules_get_stats (lbu) */
    /* 0x10 */ s16 modifiers[4]; /* stat modifiers (fightstg_rules_change_modifier), for stats fightstg_rules_modifier_stats[] */
    /* 0x18 */ s16 item; /* item taken (gamestate_data.items), -1 = none */
    /* 0x1A */ u8 blasted; /* the gauge reached 1000: shown as another level's form until event 0x13 */
    /* 0x1B */ u8 boosted;
    /* 0x1C */ u8 status; /* flags: 1 poison, 2 paralysis, 4 confusion, 8 sleep, 0x10/0x20 seals (inferred) */
    /* 0x1D */ u8 paralysis_power;
    /* 0x1E */ u8 sleep_power;
    /* 0x1F */ u8 confusion_power;
} FightstgMember; /* size 0x20 */

/* fightstg_battle.state.field (fightstg_rules_get_field_bonus reaches both fields from its address). */
typedef struct FightstgField {
    /* 0x0 */ s16 element;
    /* 0x2 */ s16 power;
} FightstgField; /* size 0x4 */

/* fightstg_battle.speed: the battle speed. */
typedef struct FightstgSpeed {
    /* 0x0 */ s32 mode; /* 0: frames as counted, 1: none, 2: about a quarter, 3: double */
    /* 0x4 */ s32 frames; /* frames counted (2) */
} FightstgSpeed; /* size 0x8 */

/* fightstg_battle.state: the state of one battle, cleared as one block when a battle starts
 * (wfightmn_main_create bzeroes its 0xD4 bytes). */
typedef struct FightstgBattleState {
    /* 0x00 */ s32 current[2]; /* per side (0 = the party): the acting member */
    /* 0x08 */ FightstgMember members[2][3]; /* per side: the members */
    /* 0xC8 */ FightstgField field;
    /* 0xCC */ s16 escapes;
    /* 0xCE */ s16 type; /* 0 normal, 1 the enemy may flee, 4 the enemy copies techniques, 5 scripted, 6 boss */
    /* 0xD0 */ s16 copied_tech;
    /* 0xD2 */ s8 hits; /* a counter (wfightmn_count_boss_hits compares it signed) */
    /* 0xD3 */ u8 final_phase;
} FightstgBattleState; /* size 0xD4 */

/* fightstg_battle: the battle module: its state, its speed and its function table (fightstg_battle_update_speed's). */
typedef struct FightstgBattle {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 frames;    /* frames this tick */
    /* 0x08 */ FightstgBattleState state;
    /* 0xDC */ FightstgSpeed speed; /* the speed (fightstg_battle_update_speed) */
    /* 0xE4 */ void (*update_speed)(void);                                       /* fightstg_battle_update_speed */
    /* 0xE8 */ void (*set_speed)(s32 state);                                  /* fightstg_battle_set_speed */
    /* 0xEC */ void (*to_screen)(GfxLayer *layer, SVECTOR *in, SVECTOR *out); /* fightstg_battle_to_screen: to screen */
    /* 0xF0 */ void (*draw_quad)(s32 layer_id, s32 depth, DVECTOR *pos, CVECTOR *colors); /* fightstg_battle_draw_quad_opaque: opaque quad */
    /* 0xF4 */ void (*draw_quad_semi)(s32 layer_id, s32 depth, DVECTOR *pos, CVECTOR *colors); /* fightstg_battle_draw_quad_semi: semi-transparent */
} FightstgBattle; /* size 0xF8 */

extern FightstgBattle fightstg_battle;

/* The records fightstg_models_get returns from file 0x1CC: type A (0xC4 bytes) when the entry's
 * byte 3 is below 0x3A, else type B (0x48 bytes). Both start with the same fields (up to 0x1A). */
typedef struct FightstgModelRecordA {
    /* 0x00 */ s32 model_file; /* the model's sub-file ID */
    /* 0x04 */ s32 anim_file;
    /* 0x08 */ s32 script_file; /* sub-file ID of its scripts */
    /* 0x0C */ s32 texture_anims;
    /* 0x10 */ s16 depth;
    /* 0x12 */ u8 unk_12[0x6];
    /* 0x18 */ s16 height;
    /* 0x1A */ s16 camera_eyes[12][3];
    /* 0x62 */ s16 camera_targets[12][3];
    /* 0xAA */ s16 camera_projections[12];
    /* 0xC2 */ u8 pad_C2[0x2];
} FightstgModelRecordA; /* size 0xC4 */

typedef struct FightstgModelRecordB {
    /* 0x00 */ s32 model_file;
    /* 0x04 */ s32 anim_file;
    /* 0x08 */ s32 script_file;
    /* 0x0C */ s32 texture_anims;
    /* 0x10 */ s16 depth;
    /* 0x12 */ u8 unk_12[0x6];
    /* 0x18 */ s16 height;
    /* 0x1A */ s16 camera_eyes[3][3];
    /* 0x2C */ s16 camera_targets[3][3];
    /* 0x3E */ s16 camera_projections[3];
    /* 0x44 */ s16 default_camera;
    /* 0x46 */ u8 pad_46[0x2];
} FightstgModelRecordB; /* size 0x48 */

/* fightstg_models: the record cache of file 0x1CC and its function table. */
typedef struct FightstgModels {
    /* 0x00 */ s32 id;            /* ID of the cached record */
    /* 0x04 */ s32 is_b;            /* 1 = type B */
    /* 0x08 */ s32 index;            /* record index */
    /* 0x0C */ s32 kind;            /* the entry's byte 3 */
    /* 0x10 */ FightstgModelRecordA *record;
    /* 0x14 */ FightstgModelRecordB *record_b; /* the same record */
    /* 0x18 */ void *(*get)(s32 id);                        /* fightstg_models_get */
    /* 0x1C */ void (*select)(s32 i);                          /* fightstg_models_select */
    /* 0x20 */ void (*get_range)(s32 type, s32 *min, s32 *max);   /* fightstg_models_get_range */
    /* 0x24 */ void *(*get_texture_anim)(s32 id);                        /* fightstg_models_get_texture_anim */
} FightstgModels; /* size 0x28 */

extern FightstgModels fightstg_models;

/* fightstg_math: a table of functions. */
typedef struct FightstgMath {
    /* 0x0 */ void (*nop)();                                              /* fightstg_math_nop */
    /* 0x4 */ void (*lerp)(SVECTOR *a, SVECTOR *b, s32 t, SVECTOR *out); /* fightstg_math_lerp: a + (b - a) * t */
    /* 0x8 */ s32 (*wave)(s32 mode, s32 angle, s32 scale);               /* fightstg_math_wave */
} FightstgMath; /* size 0xC */

extern FightstgMath fightstg_math;

/* fightstg_action: an action's state and one function. */
typedef struct FightstgAction {
    /* 0x00 */ u8 unk_00[0x20];
    /* 0x20 */ u8 side; /* side */
    /* 0x21 */ u8 pad_21[0x3];
    /* 0x24 */ s32 tech; /* record of the EXE's table records_techniques (from 1) */
    /* 0x28 */ s32 damage;
    /* 0x2C */ s32 drained;
    /* 0x30 */ u8 hits[0x4];
    /* 0x34 */ u16 hit_count;
    /* 0x36 */ s16 strikes;
    /* 0x38 */ u8 effects[0x28]; /* per kind (records_techniques's kind) */
    /* 0x60 */ s32 damages[2];
    /* 0x68 */ void (*run)(); /* fightstg_action_run */
} FightstgAction; /* size 0x6C */

extern FightstgAction fightstg_action;

/* A combatant's battle stats (fightstg_rules.stats[], filled by fightstg_rules_get_stats). */
typedef struct FightstgStats {
    /* 0x00 */ s16 level;
    /* 0x02 */ s16 stats[5];
    /* 0x0C */ s16 resists[12];
    /* 0x24 */ u8 status; /* the member's status (flags) */
    /* 0x25 */ u8 type;
    /* 0x26 */ u8 power_up;
    /* 0x27 */ u8 guard;
    /* 0x28 */ u8 strong_types[3]; /* filled from item byte 0x12 when >= 2 */
    /* 0x2B */ u8 attack_element;
    /* 0x2C */ u8 attack_element_power;
    /* 0x2D */ u8 poison_chance;
    /* 0x2E */ u8 poison_power;
    /* 0x2F */ u8 paralysis_chance;
    /* 0x30 */ u8 paralysis_power;
    /* 0x31 */ u8 confusion_chance;
    /* 0x32 */ u8 confusion_power;
    /* 0x33 */ u8 knockout_chance;
    /* 0x34 */ u8 knockout_power;
    /* 0x35 */ u8 drain_chance;
    /* 0x36 */ u8 drain_power;
    /* 0x37 */ u8 multi_hit;
    /* 0x38 */ u8 critical;
    /* 0x39 */ u8 counter;
    /* 0x3A */ u8 accuracy;
    /* 0x3B */ u8 evasion;
    /* 0x3C */ u8 escape;
    /* 0x3D */ u8 no_escape;
    /* 0x3E */ u8 steal;
    /* 0x3F */ u8 pad_3F;
} FightstgStats; /* size 0x40 */

struct FightstgPoisonArgs;

/* fightstg_rules: two combatants' stats and a table of 28 functions. */
typedef struct FightstgRules {
    /* 0x00 */ FightstgStats stats[2]; /* [0]: fightstg_rules_get_stats's arg1 = 1, [1]: arg1 = 0 */
    /* 0x80 */ FightstgStats *(*get_stats)(u8 side, s32 arg1, s32 member); /* fightstg_rules_get_stats */
    /* 0x84 */ s32 (*get_damage)(u8 side, s32 id);   /* fightstg_rules_get_damage */
    /* 0x88 */ s32 (*get_special_damage)(u8 side, s32 id);   /* fightstg_rules_get_special_damage */
    /* 0x8C */ s32 (*get_poison_damage)(struct FightstgPoisonArgs *args); /* fightstg_rules_get_poison_damage */
    /* 0x90 */ s32 (*get_counter_damage)(u8 side, s32 id, s32 amount); /* fightstg_rules_get_counter_damage */
    /* 0x94 */ s32 (*get_heal)(u8 side, s32 id);   /* fightstg_rules_get_heal */
    /* 0x98 */ s32 (*get_regen)(u8 side, s32 member, s32 tech); /* fightstg_rules_get_regen */
    /* 0x9C */ s32 (*roll_hit)(u8 side, s32 id);   /* fightstg_rules_roll_hit */
    /* 0xA0 */ s32 (*roll_special_hit)(u8 side, s32 id);   /* fightstg_rules_roll_special_hit */
    /* 0xA4 */ s32 (*roll_poison)(u8 side, s32 id);   /* fightstg_rules_roll_poison */
    /* 0xA8 */ s32 (*roll_paralysis)(u8 side, s32 id);   /* fightstg_rules_roll_paralysis */
    /* 0xAC */ s32 (*roll_confusion)(u8 side, s32 id);   /* fightstg_rules_roll_confusion */
    /* 0xB0 */ s32 (*roll_sleep)(u8 side, s32 id);   /* fightstg_rules_roll_sleep */
    /* 0xB4 */ s32 (*roll_knockout)(u8 side, s32 id);   /* fightstg_rules_roll_knockout */
    /* 0xB8 */ s32 (*roll_steal)(u8 side, s32 id);   /* fightstg_rules_roll_steal */
    /* 0xBC */ s32 (*roll_drain)(u8 side, s32 id);   /* fightstg_rules_roll_drain */
    /* 0xC0 */ s32 (*roll_revert)(u8 side, s32 id);   /* fightstg_rules_roll_revert */
    /* 0xC4 */ s32 (*roll_lower_stat)(u8 side, s32 id);   /* fightstg_rules_roll_lower_stat */
    /* 0xC8 */ s32 (*roll_switch_seal)(u8 side, s32 id);   /* fightstg_rules_roll_switch_seal */
    /* 0xCC */ s32 (*roll_digivolve_seal)(u8 side, s32 id);   /* fightstg_rules_roll_digivolve_seal */
    /* 0xD0 */ s32 (*roll_counter)(u8 side, s32 arg1); /* fightstg_rules_roll_counter */
    /* 0xD4 */ s32 (*roll_escape)(u8 side);           /* fightstg_rules_roll_escape */
    /* 0xD8 */ s32 (*roll_wake)(u8 side, s32 amount); /* fightstg_rules_roll_wake */
    /* 0xDC */ s32 (*roll_confused)(u8 side);           /* fightstg_rules_roll_confused */
    /* 0xE0 */ s32 (*roll_paralyzed)(u8 side);           /* fightstg_rules_roll_paralyzed */
    /* 0xE4 */ void (*change_modifier)(u8 side, s32 member, s32 arg2, s32 amount); /* fightstg_rules_change_modifier */
    /* 0xE8 */ s32 (*get_gauge_gain)(s32 amount);        /* fightstg_rules_get_gauge_gain */
    /* 0xEC */ s32 (*get_tech_cost)(u8 side, s32 id);   /* fightstg_rules_get_tech_cost: cost */
} FightstgRules; /* size 0xF0 */

extern FightstgRules fightstg_rules;

/* fightstg_model */
FightstgModel *fightstg_model_new(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params, s32 has_idle);
FightstgModel *fightstg_model_create_idle(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params);
FightstgModel *fightstg_model_create(s32 id, s32 anim_file, FightstgPos pos, FightstgModelParams *params);

/* fightstg_stage */
s32 fightstg_stage_get_random(void);
s32 fightstg_stage_get_file(s32 index);

/* fightstg_80086A00 */
s32 fightstg_prop_get_file(s32 id);
struct FightstgProp *fightstg_prop_create(s32 id, SVECTOR *pos, SVECTOR *rot);

/* fightstg_8008B630 */
FightstgScript *fightstg_script_create(void);

/* fightstg_8008D3B4 */
struct FightstgFlash *fightstg_flash_create(s32 frames);
void fightstg_flash_end(struct FightstgFlash *obj, s32 frames);
void *fightstg_sound_play(s32 id, s32 delay);

/* fightrest */
/* fightstg_8008B118 */
struct FightstgTextureAnim *fightstg_texture_anim_create(FightstgModel *model, s32 id);

/* fightstg_80086A00 */
s32 fightstg_effect_get_files(s32 id, s32 *image_file, s32 *file, FightstgPos *pos);
struct FightstgEffect *fightstg_effect_create(s32 id, SVECTOR *pos);

/* fightcore-lo */
struct FightstgMessage *fightstg_message_create(void);
void fightstg_events_add_enemy_turn(s32 delay);
void fightstg_events_add_boss_entrance(void);
void fightstg_events_add_knockout(u8 side);
void fightstg_events_end_final_phase(void);
struct FightstgResults *fightstg_results_create(u8 side);
void fightstg_events_end_battle(s8 result);
void fightstg_events_start_poison(u8 side, s32 member, s32 power);
void fightstg_events_start_paralysis(u8 side, s32 arg1, u8 power);
void fightstg_events_start_confusion(u8 side, s32 arg1, u8 power);
void fightstg_events_start_sleep(u8 side, s32 arg1, u8 power);
void fightstg_events_start_seal(s32 id);

/* fightcore-hi */
/* fightstg_rules_get_poison_damage's argument. */
typedef struct FightstgPoisonArgs {
    /* 0x0 */ u8 side;
    /* 0x1 */ u8 pad_1[0x3];
    /* 0x4 */ s32 member; /* member */
    /* 0x8 */ s32 amount;
} FightstgPoisonArgs; /* size 0xC */

FightstgStats *fightstg_rules_get_stats(u8 side, s32 first, s32 member);

#endif /* FIGHTSTG_H */
