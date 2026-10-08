#ifndef FIELDSTG_H
#define FIELDSTG_H

/* FIELDSTG.PRO (the field): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "gamestate.h"
#include "psyq/libgte.h"
#include "window_anim_type.h"

/* A position (copied as a whole). */
typedef struct FieldstgPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
} FieldstgPos; /* size 0x8 */

/* A record of the field's sprite list (fieldstg_stage.sprites, ended by present == 0); fieldstg_sprites_find_next
 * searches it by key. */
typedef struct FieldstgSprite {
    /* 0x00 */ u8 shown;  /* shown */
    /* 0x01 */ u8 type;   /* type (key); 0xFF: a sprite of the common bank (fieldstg_sprites_draw_entry) */
    /* 0x02 */ u8 present; /* 0: end of the list */
    /* 0x03 */ u8 ot_depth; /* ordering table depth */
    /* 0x04 */ u8 sprite; /* sprite */
    /* 0x05 */ u8 anim_mode; /* animation: 1 sprite loops, 2 frame loops, 3 frame goes back and forth */
    /* 0x06 */ u8 first_frame; /* first frame */
    /* 0x07 */ u8 last_frame; /* last frame */
    /* 0x08 */ u8 frame_time; /* frame time */
    /* 0x09 */ u8 frame;
    /* 0x0A */ s16 x;      /* x */
    /* 0x0C */ s16 y;      /* y */
    /* 0x0E */ s16 priority; /* drawn through the layer's list at this priority, 0: directly */
    /* 0x10 */ s16 frame_timer; /* frame timer (8.8; bit 15: going backwards) */
} FieldstgSprite; /* size 0x12 */

/* An entry of fieldstg_stage.map_events (fieldstg_loader_load_action_anims), 0-terminated. */
typedef struct FieldstgMapEvent {
    /* 0x00 */ u16 flag;   /* a flag that must have flag_value (gamestate_flags.get_flag(flag, flag_value)), 0xFFFF: none */
    /* 0x02 */ u16 flag_value; /* 1: set, 0: clear */
    /* 0x04 */ u16 flag_2; /* a second flag, with flag_2_value, 0xFFFF: none */
    /* 0x06 */ u16 flag_2_value;
    /* 0x08 */ u16 type;   /* type; 0: end */
    /* 0x0A */ u16 param;  /* parameter of the type */
    /* 0x0C */ u16 x;      /* x (types 1, 2, 3, 14) */
    /* 0x0E */ u16 y;      /* y */
    /* 0x10 */ u16 dir;    /* direction on arrival (types 1, 14) */
    /* 0x12 */ u16 hide_type; /* type 1: sprites of this type are hidden */
    /* 0x14 */ u16 route; /* type 1: the route (gamestate_data.route) and */
    /* 0x16 */ u16 room;  /* the room along it (gamestate_data.room) set on arrival (WstagExits) */
} FieldstgMapEvent; /* size 0x18 */

/* An event: a battle (fieldstg_start_battle) and two values for records_state. */
typedef struct FieldstgListedBattle {
    /* 0x0 */ s32 battle; /* index into fieldstg_battles */
    /* 0x4 */ s32 stage;
    /* 0x8 */ s32 music;
} FieldstgListedBattle; /* size 0xC */

/* A list of events: always 8 (the stages' data; fieldstg_encounter_start picks one by random & 7). */
typedef struct FieldstgBattleList {
    /* 0x0 */ s32 rate; /* encounter rate: index into fieldstg_encounter_rates (0 in stages without encounters) */
    /* 0x4 */ FieldstgListedBattle *events[8];
} FieldstgBattleList; /* size 0x24 */

/* What fieldstg_stage.battle_lists points to: the field's event lists. */
typedef struct FieldstgBattleLists {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 id; /* ID: stages with several pick theirs with fieldstg_stage.find_battle_lists */
    /* 0x08 */ s32 unk_08;
    /* 0x0C */ FieldstgBattleList *encounters[3]; /* random encounters per area (attribute layer 4; fieldstg_encounter_step) */
    /* 0x18 */ FieldstgBattleList *scripted;    /* battles that events start (fieldstg_start_listed_battle, fieldstg_start_battle_5) */
} FieldstgBattleLists; /* size 0x1C */

/* Event script commands (FieldstgEventDef.script; fieldstg_event_update, docs/FORMATS.md "Event scripts"): s16
 * words, the first one's high byte the command and its low byte the variant. Actor IDs are FieldstgPlacedActor.id. */
#define FIELDSTG_EVENT_END 0 /* ends the script, as does an unknown command */
/* puts the actor at (x, y) (pixels) */
#define FIELDSTG_EVENT_PLACE(actor, x, y) 0x100, actor, x, y
/* id < 0x320: the actor plays anim facing dir (FieldstgActor.play_anim); else sends (anim, dir) to the script object
 * id, started if needed (fieldstg_event_send_message) */
#define FIELDSTG_EVENT_ANIM(id, anim, dir) 0x101, id, anim, dir
/* the actor walks to (x, y), then faces dir */
#define FIELDSTG_EVENT_WALK(actor, x, y, dir) 0x102, actor, x, y, dir
/* opens dialog slot 0-2 with a message of the event's text, spoken by the actor (mode 4: no speaker;
 * fieldstg_dialog_create) */
#define FIELDSTG_EVENT_DIALOG(slot, message, actor, mode) 0x200, slot, message, actor, mode
#define FIELDSTG_EVENT_WAIT(frames) 0x300, frames
#define FIELDSTG_EVENT_WAIT_DIALOG 0x301 /* until dialog 0 closes */
#define FIELDSTG_EVENT_WAIT_WALK(actor) 0x302, actor /* until the actor stops walking */
#define FIELDSTG_EVENT_WAIT_ANIM(actor) 0x303, actor /* until its animation ends */
/* goes to another map (fieldstg_goto_map) and ends the event */
#define FIELDSTG_EVENT_GOTO_MAP(map, x, y, dir) 0x304, map, x, y, dir
#define FIELDSTG_EVENT_CAMERA_FOLLOW(snap, actor) 0x600, snap, actor /* fieldstg_camera_follow */
#define FIELDSTG_EVENT_CAMERA_MOVE(snap, x, y) 0x601, snap, x, y   /* fieldstg_camera_move_to */

/* An event (fieldstg_stage.events points to a table of them, ended by ID -1). */
typedef struct FieldstgEventDef {
    /* 0x00 */ s32 id;     /* -1 ends the list; 8000-8999 don't stop the player */
    /* 0x04 */ s16 *script; /* docs/FORMATS.md "Event scripts" */
    /* 0x08 */ s32 text;   /* text sub-file ID (high half: file ID), 0: none */
    /* 0x0C */ Object *(*start)(void); /* run once after the script (FieldstgEvent.start); it returns an object (or 0)
                                     * that the event keeps as a child and waits for */
    /* 0x10 */ void (*end)(void);    /* run at the end (FieldstgEvent.end) */
} FieldstgEventDef; /* size 0x14 */

/* A sprite's place in VRAM: fieldstg_stage.vram_places points to an array of them ([0] the actors' shadow,
 * [1] the background, [2 + i] actor sprites, FieldstgActor.vram). Only what is known. */
typedef struct FieldstgVramPlace {
    /* 0x0 */ s16 tpage_x; /* texture page x */
    /* 0x2 */ s16 tpage_y; /* texture page y */
    /* 0x4 */ s16 image_x; /* image x (where fieldstg_actor_update_sprite loads it) */
    /* 0x6 */ s16 image_y; /* image y */
    /* 0x8 */ s16 u;     /* u */
    /* 0xA */ s16 v;     /* v */
    /* 0xC */ s16 clut_x; /* CLUT x */
    /* 0xE */ s16 clut_y; /* CLUT y */
} FieldstgVramPlace; /* size 0x10 */

/* What an actor says when talked to: the first entry whose flags are set (fieldstg_actor_run_state). */
typedef struct FieldstgTalk {
    /* 0x0 */ u16 *flags_required; /* flags that must be set (gamestate_flags.check_flags), NULL: none */
    /* 0x4 */ u16 *flags_set; /* flags set afterwards (gamestate_flags.set_flags), NULL: none */
    /* 0x8 */ s32 message; /* message (fieldstg_dialog_create_talk) */
} FieldstgTalk; /* size 0xC */

/* An actor placed on the field (fieldstg_stage.actors; fieldstg_manager_update). */
typedef struct FieldstgPlacedActor {
    /* 0x00 */ u16 *flags_required; /* flags that must be set (gamestate_flags.check_flags), NULL: none */
    /* 0x04 */ FieldstgTalk *talks; /* what it says (the first entry whose flags are set) */
    /* 0x08 */ s16 id;     /* ID (fieldstg_actor_create); 1, 0x6A, 0x146, 0x147: the player */
    /* 0x0A */ s16 vram_place; /* VRAM place */
    /* 0x0C */ s16 x;      /* x */
    /* 0x0E */ s16 y;      /* y */
    /* 0x10 */ s16 dir;    /* direction */
} FieldstgPlacedActor; /* size 0x14 */

/* fieldstg_stage: the field module's state (the stage setup fills it) and its function table (find_stage ...). */
typedef struct FieldstgStageState {
    /* 0x00 */ s32 code_file; /* file of the map's code (overlay_module.load_file), from fieldstg_stages_2d/fieldstg_stages */
    /* 0x04 */ void *(*entry)(); /* its entry, called with the field manager (fieldstg_manager_update) */
    /* 0x08 */ s32 background_file;
    /* 0x0C */ s32 sprite_file;
    /* 0x10 */ FieldstgSprite *sprites; /* the field's sprites */
    /* 0x14 */ FieldstgMapEvent *map_events;
    /* 0x18 */ s32 mask_subfile; /* a TIM list sub-file ID loaded like mask_file (no stage sets it) */
    /* 0x1C */ s32 mask_file;    /* S###MASK: a TIM list loaded at (0x140, 0x100) and freed (fieldstg_loader_run) */
    /* 0x20 */ FieldstgBattleLists *battle_lists; /* event lists */
    /* 0x24 */ FieldstgEventDef *events; /* events */
    /* 0x28 */ FieldstgVramPlace *vram_places; /* VRAM places of the sprites */
    /* 0x2C */ GamestatePos start_pos; /* the player's start position (24.8) */
    /* 0x34 */ s32 start_dir;         /* and direction */
    /* 0x38 */ CVECTOR color; /* colour of the background and sprites, used when cd != 0 */
    /* 0x3C */ s32 music;
    /* 0x40 */ s32 sound;
    /* 0x44 */ s32 talk_file;
    /* 0x48 */ s32 event_text; /* text sub-file of the current event (fieldstg_event_start, fieldstg_dialog_create) */
    /* 0x4C */ FieldstgPlacedActor **actors; /* the actors to place (NULL-terminated) */
    /* 0x50 */ s32 menu_open;
    /* 0x54 */ s32 title_shown; /* set while fieldstg_map_title_create's object exists */
    /* 0x58 */ s32 event_running; /* an event runs (fieldstg_event_start) */
    /* 0x5C */ s32 battle_starting;
    /* 0x60 */ s32 actor_busy;
    /* 0x64 */ GamestatePos return_pos; /* the player's position when gamestate_data.funcs.get_map_entry() is -1 */
    /* 0x6C */ s32 return_dir;         /* and direction */
    /* 0x70 */ void (*find_stage)(void);   /* fieldstg_find_stage */
    /* 0x74 */ s32 (*get_actor_sprite_file)(s32 i);   /* fieldstg_get_actor_sprite_file: an actor's sprite file */
    /* 0x78 */ s32 (*get_actor_width)(s32 i);   /* fieldstg_get_actor_width (returns u8; callers use s32): its width */
    /* 0x7C */ FieldstgBattleLists *(*find_battle_lists)(FieldstgBattleLists *lists, s32 id); /* fieldstg_find_battle_lists: the lists with that ID */
} FieldstgStageState; /* size 0x80 */

extern FieldstgStageState fieldstg_stage;

/* fieldstg_stage_funcs: a function table. Reached as a struct: the store before the call isn't moved past
 * the load. Its second and third entries are FIELDSTG's copy of the EXE's window_anim pair (window_anim.h), used
 * here as a fade (level 0..0x1000). */
typedef struct FieldstgStageFuncs {
    /* 0x0 */ void (*setup)(void);                     /* fieldstg_stage_setup */
    /* 0x4 */ void (*fade_start)(WindowAnim *fade, s32 in); /* fieldstg_window_anim_start */
    /* 0x8 */ s32 (*fade_update)(WindowAnim *fade);          /* fieldstg_window_anim_update */
} FieldstgStageFuncs; /* size 0xC */

extern FieldstgStageFuncs fieldstg_stage_funcs;

/* The objects of type 5 on the object list, the field's actors (fieldstg_actor_update), created by
 * fieldstg_actor_create: position and methods (the event scripts use them). base.key1 is the actor ID,
 * base.key2 its type (0: the player, 2/4/8: the partners following it, 1: placed by the map).
 * States (base.step, fieldstg_actor_run_state): 0 idle, 1 stand, 2 walk, 3 run (footsteps, random
 * encounters), 4 stop, 5 walk out of the map; ladders: 0x43/0x44 grab it from the bottom/top, 0x40 hold, 0x41/0x42
 * climb up/down, 0x45/0x46 get off at the top/bottom; 0x47 jump down a ledge, 0x48 the meter (FieldstgMeter),
 * 0x49 open a spot (FieldstgSpots), 0x4A talked to (dialog, then the talk flags), 0x4B/0x4C speed up/slow down
 * (fieldstg_player_control_height), 0x4D/0x4E pick up an object (actors 0x148, 0x15F, 0x160: flags 8-10),
 * 0x4F/0x50 carried along and stopping (map events 11/12). The methods (walk_out ... launch) are what
 * the map events and the event scripts call. */
typedef struct FieldstgActor {
    /* 0x000 */ Object base;
    /* 0x050 */ GamestatePos pos; /* position, 24.8 */
    /* 0x058 */ FieldstgPos pixel_pos; /* pos >> 8, minus climb_height */
    /* 0x060 */ s32 dir; /* direction (fieldstg_pad_dirs; index into fieldstg_background_order_by_dir) */
    /* 0x064 */ s32 height; /* height, 24.8 */
    /* 0x068 */ s32 speed; /* speed (fieldstg_actor_collide) */
    /* 0x06C */ struct FieldstgVramPlace *vram; /* VRAM place of the sprite */
    /* 0x070 */ FieldstgVramPlace *shadow_vram; /* fieldstg_stage.vram_places ([0]: the shadow) */
    /* 0x074 */ s32 has_shadow; /* the shadow is drawn */
    /* 0x078 */ s32 depth; /* ordering-table depth (map event 5) */
    /* 0x07C */ FieldstgPlacedActor *placed; /* how it was placed (fieldstg_actor_create) */
    /* 0x080 */ s32 width; /* width (fieldstg_actor_find_at) */
    /* 0x084 */ s32 height_control; /* actor 0x147: fieldstg_player_control_height */
    /* 0x088 */ struct FieldstgActor *talker; /* the actor talking to this one (fieldstg_actor_talk_to) */
    /* 0x08C */ s32 ladder_side; /* the ladder is on the right (sign of the x offsets) */
    /* 0x090 */ s32 climb_height; /* height on a ladder, 24.8 */
    /* 0x094 */ s32 climb_top;    /* its top */
    /* 0x098 */ u8 unk_98[0x4];
    /* 0x09C */ s32 sprite_file; /* sprite sub-file ID (high half: file ID), 0: none */
    /* 0x0A0 */ s32 anim; /* animation (fieldstg_actor_set_anim) */
    /* 0x0A4 */ s32 loaded_anim;    /* anim when anim_scripts was filled */
    /* 0x0A8 */ u32 anim_scripts[5]; /* animation script sub-file IDs, per direction 0-4 (5-7 are mirrored) */
    /* 0x0BC */ s32 walk; /* moving walks (state 2) instead of running (3) */
    /* 0x0C0 */ s32 reload; /* reload the sprite */
    /* 0x0C4 */ s32 vertical_speed; /* vertical speed (fieldstg_player_control_height) */
    /* 0x0C8 */ s16 voice; /* voice of a looping sound, or -1 */
    /* 0x0CA */ u8 unk_CA[0x2];
    /* 0x0CC */ s32 anim_pos;  /* position in the animation script */
    /* 0x0D0 */ s32 anim_time; /* time left of the frame */
    /* 0x0D4 */ u32 frame_file; /* sprite sub-file ID */
    /* 0x0D8 */ u32 loaded_frame_file; /* sprite sub-file ID loaded into VRAM */
    /* 0x0DC */ s32 frame_x; /* x offset */
    /* 0x0E0 */ s32 frame_y; /* y offset */
    /* 0x0E4 */ s16 frame_w; /* width */
    /* 0x0E6 */ u16 frame_h; /* height */
    /* 0x0E8 */ s32 anim_done; /* the script reached its end (-1) */
    /* 0x0EC */ s32 walking;   /* walks to target_x/y (fieldstg_actor_walk_to_target), then faces target_dir */
    /* 0x0F0 */ s32 target_x;
    /* 0x0F4 */ s32 target_y;
    /* 0x0F8 */ s32 target_dir;
    /* 0x0FC */ u16 *talk_flags; /* flags to set when the talk ends (FieldstgTalk.flags_set) */
    /* 0x100 */ s32 no_turn; /* doesn't turn to the talker (actors 0x28-0x2A, 0x3E, 0x11A) */
    /* 0x104 */ struct FieldstgTrail *trail;
    /* 0x108 */ void (*control)(); /* run every frame: pad control, following, scripted walk, ... */
    /* 0x10C */ s32 unk_10C; /* only ever cleared (fieldstg_player_clear_unk_10C); nothing reads it */
    /* 0x110 */ void (*walk_out)(struct FieldstgActor *obj, s32 dir);
    /* 0x114 */ void (*climb_from_bottom)(struct FieldstgActor *obj, s32 dir, s32 x, s32 y, s32 height);
    /* 0x118 */ void (*climb_from_top)(struct FieldstgActor *obj, s32 dir, s32 x, s32 y, s32 height);
    /* 0x11C */ void (*jump_down)(struct FieldstgActor *obj, s32 dir, FieldstgPos pos, s32 height);
    /* 0x120 */ void (*start_meter)(struct FieldstgActor *obj, s32 dir, FieldstgPos offset);
    /* 0x124 */ void (*warp)(struct FieldstgActor *obj, u16 *params, s32 type);
    /* 0x128 */ u8 unk_128[0x4];
    /* 0x12C */ void (*start_walk_to)(struct FieldstgActor *obj);
    /* 0x130 */ void (*reset_control)(struct FieldstgActor *obj);
    /* 0x134 */ void (*set_dir)();
    /* 0x138 */ s32 (*is_anim_done)(struct FieldstgActor *obj);
    /* 0x13C */ void (*set_walk_target)(struct FieldstgActor *obj, s32 x, s32 y, s32 dir);
    /* 0x140 */ s32 (*is_walking)(struct FieldstgActor *obj);
    /* 0x144 */ void (*set_anim)();
    /* 0x148 */ void (*play_anim)(struct FieldstgActor *obj, s32 anim, s32 dir);
    /* 0x14C */ void (*get_front)(struct FieldstgActor *obj, FieldstgPos *out); /* the position in front */
    /* 0x150 */ void (*start_carry)(struct FieldstgActor *obj, s32 dir);
    /* 0x154 */ void (*stop_carry)(struct FieldstgActor *obj);
    /* 0x158 */ void (*launch)(); /* fieldstg_actor_launch(obj, s16 *params); callers pass the u16 map event fields */
} FieldstgActor; /* size 0x15C */

/* An actor taking part in an event (FieldstgEvent.actors). */
typedef struct FieldstgEventActor {
    /* 0x0 */ s32 id;    /* its ID (base.key1), 0: end of the list */
    /* 0x4 */ FieldstgActor *actor;
} FieldstgEventActor; /* size 0x8 */

/* Object of fieldstg_event_update, created by fieldstg_event_start: runs an event's script. */
typedef struct FieldstgEvent {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 id;      /* event ID */
    /* 0x054 */ s16 *script_pos; /* script position, NULL when done */
    /* 0x058 */ Object *(*start)(void);
    /* 0x05C */ void (*end)(void);
    /* 0x060 */ s32 wait;    /* wait counter */
    /* 0x064 */ FieldstgEventActor actors[30]; /* the actors it uses, found by ID (fieldstg_event_find_actor) */
} FieldstgEvent; /* size 0x154 */

/* An entry of fieldstg_script_objects: an event ID, the function that starts it and the one that gets its
 * messages (fieldstg_find_script_object searches it). */
typedef struct FieldstgScriptObject {
    /* 0x0 */ s32 id;
    /* 0x4 */ Object *(*start)(s32 id);
    /* 0x8 */ void (*message)(void *obj, s32 arg1, s32 arg2);
} FieldstgScriptObject; /* size 0xC */

extern FieldstgScriptObject fieldstg_script_objects[];

/* fieldstg_attr: the map attribute module (fieldstg_attr_load_layer loads a layer, fieldstg_attr_get
 * looks a position up), and its function table. */
typedef struct FieldstgAttr {
    /* 0x00 */ s32 files[8]; /* sub-file IDs, per layer (fieldstg_attr_set_file) */
    /* 0x20 */ s32 width;    /* width in 128-unit blocks */
    /* 0x24 */ s32 height;    /* height */
    /* 0x28 */ u8 *blocks;       /* the sub-file's 6 parts (docs/FORMATS.md): per 128-unit block */
    /* 0x2C */ u8 *quarters_64;  /* 64-unit quarters */
    /* 0x30 */ s16 *quarters_32; /* 32 */
    /* 0x34 */ s16 *quarters_16; /* 16 */
    /* 0x38 */ s16 *quarters_8;  /* 8 */
    /* 0x3C */ u8 *cells;        /* the 8x8 attribute bytes */
    /* 0x40 */ void (*set_file)(s32 i, s32 id);              /* fieldstg_attr_set_file */
    /* 0x44 */ u8 (*get)(s32 layer, FieldstgPos *pos); /* fieldstg_attr_get */
    /* 0x48 */ void (*get_step)();                           /* fieldstg_attr_get_step */
    /* 0x4C */ void (*get_flat_step)();                           /* fieldstg_attr_get_flat_step */
    /* 0x50 */ void (*init_layer)(s32);                        /* fieldstg_attr_init_layer */
    /* 0x54 */ void (*set_layer)(s32);                        /* fieldstg_attr_set_layer */
    /* 0x58 */ u8 (*is_free)(FieldstgPos *pos);          /* fieldstg_attr_is_free */
} FieldstgAttr; /* size 0x5C */

extern FieldstgAttr fieldstg_attr;

/* fieldstg_timer: a timer's state and its two functions. */
typedef struct FieldstgTimer {
    /* 0x0 */ s32 frames; /* frames left */
    /* 0x4 */ s32 running; /* running */
    /* 0x8 */ void (*reset)(void);             /* fieldstg_timer_reset */
    /* 0xC */ FieldstgActor *(*find_actor)(s32 id); /* fieldstg_find_actor */
} FieldstgTimer; /* size 0x10 */

extern FieldstgTimer fieldstg_timer;

/* fieldstg_event_funcs: functions for event conditions. */
typedef struct FieldstgEventFuncs {
    /* 0x00 */ void (*to_screen_pos)(FieldstgPos *pos); /* fieldstg_to_screen_pos */
    /* 0x04 */ void (*wait_frames)(s32 frames, s32 *count); /* fieldstg_wait_frames */
    /* 0x08 */ void (*wait_anim_done)(s32 id, s32 *count);     /* fieldstg_wait_anim_done */
    /* 0x0C */ void (*wait_walk_done)(s32 id, s32 *count);     /* fieldstg_wait_walk_done */
    /* 0x10 */ void (*player_clear_unk_10C)(void);     /* fieldstg_player_clear_unk_10C (nothing calls it) */
} FieldstgEventFuncs; /* size 0x14 */

extern FieldstgEventFuncs fieldstg_event_funcs;

/* A battle (fieldstg_start_battle). */
typedef struct FieldstgBattle {
    /* 0x00 */ struct RecordsEnemy *enemies[3]; /* its enemies (copied into records_state.enemies) */
    /* 0x0C */ u8 first_strike; /* the party's chance to strike first (records_state.first_strike_chance, wfightmn_roll_first_strike) */
    /* 0x0D */ u8 kind;        /* 1-5: a class of battle the enemies' scripts can test (records_state.battle_kind) */
    /* 0x0E */ u8 blocked[12]; /* records_state.blocked: the effects that fail against its enemies */
} FieldstgBattle; /* size 0x1C */

/* fieldstg_80083784.c: the battles (fieldstg_battles) and their enemies. */
extern FieldstgBattle fieldstg_battles[];

/* fieldstg_event.c */
FieldstgEvent *fieldstg_event_start(s32 id);

/* fieldstg_80085590 */
void fieldstg_background_fill_layer(s32 layer_id, s32 alpha);

/* fieldstg_80087DB0 */
struct FieldstgDialog *fieldstg_dialog_create(FieldstgActor *actor, s32 message, s32 type, s32 fixed);
struct FieldstgManager *fieldstg_manager_create(void);
void fieldstg_goto_map(s32 map, s32 entry, s32 x, s32 y, s32 dir);
void fieldstg_spots_open_scripted(void);
void fieldstg_camera_follow(s32 snap, s32 id);
void fieldstg_camera_move_to(s32 snap, s32 x, s32 y);
void fieldstg_camera_set_shake(s32 on);
void fieldstg_partners_hide(void);
Object *fieldstg_start_script_object(s32 id);
void fieldstg_send_script_object(void *obj, s32 id, s32 arg2, s32 arg3);

#endif /* FIELDSTG_H */
