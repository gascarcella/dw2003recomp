#include "common.h"
#include "object.h"
#include "heap.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "inn.h"
#include "fieldmenu.h"
#include "overlay.h"
#include "psyq/libgte.h"
#include "overlay_common.h"
#include "fieldstg.h"

/* A frame of a sprite animation (fieldstg_actor_effect_anims, fieldstg_spots_open_anim, fieldstg_spots_effect_anims):
 * the list ends with sprite 0xFF. */
typedef struct FieldstgAnimFrame {
    /* 0x0 */ u8 sprite; /* sprite ID; 0xFF ends the list */
    /* 0x1 */ u8 time;   /* ticks it is shown; at the end: the frame the actor effects loop to */
} FieldstgAnimFrame; /* size 0x2 */

/* An icon over an actor (fieldstg_icon_update, created by fieldstg_icon_create; script objects 803-806 and the
 * map events' prompt): base.key2 picks the animation. States: 0 find the player, 1 open (frame_start up to
 * frame_shown, then animate), 2 close (up to frame_end), 3 gone. */
typedef struct FieldstgIcon {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgActor *actor; /* object of type 5 found by the message 0x325/0x327 */
    /* 0x54 */ s32 frame_start; /* start of frame */
    /* 0x58 */ s32 frame_shown; /* its end in state 1 */
    /* 0x5C */ s32 frame_end; /* its end in state 2 */
    /* 0x60 */ s32 frame; /* sprite frame << 2 (fieldstg_icon_draw) */
    /* 0x64 */ s32 anim_pos; /* index into fieldstg_icon_anims[base.key2]: frame, then its time */
    /* 0x68 */ s32 anim_time; /* time left of the frame */
} FieldstgIcon; /* size 0x6C */

/* The map events (fieldstg_map_events_update, created by fieldstg_map_events_create): the entries of
 * fieldstg_stage.map_events the player stands on (attribute layer 7: index | direction << 5). States: 0 find the
 * player, 1 watch: sub-state 0 enter an event (show the icon or run it), 1 wait for the button (then
 * fieldstg_map_events_run), 2 wait until the player is back to a normal state. Types (FieldstgMapEvent.type): 1 exit
 * to a map, 2/3 ladder from the bottom/top, 4 jump down, 5 depth, 6 attribute layer, 7 the meter, 8 an event,
 * 9/10 warp, 11/12 carried along/stop, 13/14 jump to the nearest object 0x17. */
typedef struct FieldstgMapEvents {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 sprite_file; /* fieldstg_stage.sprite_file (fieldstg_map_events_create; not read) */
    /* 0x54 */ FieldstgMapEvent *events;
    /* 0x58 */ FieldstgActor *player; /* fieldstg_map_events_check */
    /* 0x5C */ s32 index; /* index into events */
    /* 0x60 */ s32 dir; /* the attribute's high bits: direction the event wants (fieldstg_map_event_facing) */
    /* 0x64 */ FieldstgMapEvent *event; /* &events[index] */
} FieldstgMapEvents; /* size 0x68 */

/* Data block of FieldstgMapEvents (base.children). */
typedef struct FieldstgMapEventsData {
    /* 0x0 */ FieldstgIcon *icon;
    /* 0x4 */ FieldstgEvent *event; /* the event started (fieldstg_event_start) */
} FieldstgMapEventsData; /* size 0x8 */

/* A dialog window (fieldstg_dialog_update, created by fieldstg_dialog_create or fieldstg_dialog_create_talk): a
 * speech window next to the actor, or a fixed one. States: 0 open, 1 follow the actor until the window is gone. */
typedef struct FieldstgDialog {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgActor *actor;
    /* 0x54 */ s32 message;
    /* 0x58 */ s32 type; /* message window type */
    /* 0x5C */ s32 fixed; /* 0: a window at the object, else message_box_create */
    /* 0x60 */ void *messages; /* the messages: sub-file fieldstg_stage.event_text */
} FieldstgDialog; /* size 0x64 */

/* The object fieldstg_sprites_create creates (fieldstg_sprites_update). */
typedef struct FieldstgSprites {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 file; /* sub-file ID of the sprites */
    /* 0x54 */ FieldstgSprite *sprites;
} FieldstgSprites; /* size 0x58 */

/* The object of type 0x16 (fieldstg_actor_effect_update), created by fieldstg_actor_effect_create. */
typedef struct FieldstgActorEffect {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgActor *actor;
    /* 0x54 */ s32 sprite;       /* sprite ID */
    /* 0x58 */ s32 frame;       /* animation frame */
    /* 0x5C */ s32 time;       /* time in the frame */
    /* 0x60 */ FieldstgAnimFrame *anim; /* animation (fieldstg_actor_effect_anims[base.step]) */
} FieldstgActorEffect; /* size 0x64 */

/* The field manager object (type 7: fieldstg_manager_update), created by fieldstg_manager_create. States: 0 load
 * (sub-states: set up the layers and the stage, start the music, run the loader, create the sprites, map events,
 * actors and camera, wait for the background and the map title), 1 play (sub-states: 0 watch for the menu
 * button, 1 menu open, 2 inn open, 3 warp), 2 leave (the battle shatter when fieldstg_stage.battle_starting, else
 * close the window and change the map), 3 end. */
typedef struct FieldstgManager {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 fade; /* fade level (fieldstg_background_fill_layer) */
    /* 0x54 */ s32 window_w; /* width of the window being opened/closed */
    /* 0x58 */ s32 window_h; /* its height */
    /* 0x5C */ s32 next_map; /* gamestate_data.funcs.set_next_map's arguments */
    /* 0x60 */ s32 map_entry; /* entry point on the next map (gamestate_data.map_entry; fieldstg_goto_map) */
    /* 0x64 */ void *buffer; /* a heap block (0x9615C bytes) held while the field loads */
    /* 0x68 */ s32 delay; /* time left before the window closes (state 2) */
    /* 0x6C */ s32 close_on_player; /* the window closes on the player (else on the screen centre) */
    /* 0x70 */ s32 warp_type; /* a map event (fieldstg_start_warp): its ID */
    /* 0x74 */ s32 warp_x; /* x */
    /* 0x78 */ s32 warp_y; /* y */
    /* 0x7C */ u16 *warp_params; /* its parameters (FieldstgMapEvent.param): map, x, y, dir, hide_type, route, room */
} FieldstgManager; /* size 0x80 */

/* Data block of FieldstgManager (base.children): the objects of the field. */
typedef struct FieldstgManagerData {
    /* 0x00 */ Object *warp_effect;  /* fieldstg_warp_effect_create's object (a map event) */
    /* 0x04 */ Fade *fade;         /* the fade (inn_fade_create) */
    /* 0x08 */ Object *warp_picture;  /* fieldstg_warp_picture_create's object */
    /* 0x0C */ FieldstgEvent *event;  /* the event running (fieldstg_start_indexed_event) */
    /* 0x10 */ void *unk_10;         /* inn_create's object (fieldstg_open_inn), fieldstg_loader_create's */
    /* 0x14 */ Object *map_title;  /* fieldstg_map_title_create's object */
    /* 0x18 */ FieldstgActor *party[4]; /* the player, then the partners */
    /* 0x28 */ struct FieldstgCamera *camera; /* fieldstg_camera_create's object */
    /* 0x2C */ FieldstgActor *actors[15]; /* the other actors */
    /* 0x68 */ void *map_events;         /* fieldstg_map_events_create's object */
    /* 0x6C */ void *stage;         /* what the map's code returned (fieldstg_stage.entry) */
    /* 0x70 */ Object *background;  /* fieldstg_background_create's object */
    /* 0x74 */ struct Fieldmenu *menu;   /* fieldmenu_create's object */
    /* 0x78 */ Object *sprites;  /* fieldstg_sprites_create's object */
} FieldstgManagerData; /* size 0x7C */

/* A launch (fieldstg_launch_update, created by fieldstg_launch_create; map events 13/14 through the actor's
 * launch method): the actor walks onto the nearest object of type 0x17 (a stage object, the pad), triggers it, then
 * flies spinning along a sine ease to (params[1], params[2]); the partners hide meanwhile. Sub-states of state 1:
 * 0 walk to the pad, 1 trigger it and wait, 2 fly. */
typedef struct FieldstgLaunch {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgActor *actor;
    /* 0x54 */ s16 *params; /* parameters of a map event (fieldstg_actor_launch): [1] x, [2] y */
    /* 0x58 */ Object *pad; /* the nearest object of type 0x17 (position in base.key1/key2) */
    /* 0x5C */ s32 start_x; /* start x */
    /* 0x60 */ s32 start_y; /* start y */
    /* 0x64 */ s32 dist_x; /* |dx| */
    /* 0x68 */ s32 dist_y; /* |dy| */
    /* 0x6C */ s32 neg_x; /* dx < 0 */
    /* 0x70 */ s32 neg_y; /* dy < 0 */
} FieldstgLaunch; /* size 0x74 */

/* An entry of FieldstgSpots.spots. */
typedef struct FieldstgSpot {
    /* 0x00 */ s32 frame;  /* its frame (0x38: closed) */
    /* 0x04 */ s32 sprite; /* index in fieldstg_stage.sprites */
    /* 0x08 */ FieldstgPos pos;
    /* 0x10 */ s32 target; /* the one to find */
} FieldstgSpot; /* size 0x14 */

/* The spots (type 0xB: fieldstg_spots_update, created by fieldstg_spots_create): the map's sprites of type 0xFF
 * (the common bank), one of them the target (gamestate_data.spot_target, picked again on a new map). Pressing the button
 * at one (with flag 0x8004) opens it (state 2): the target starts a battle, any other one shows a hint of the
 * target's distance (FieldstgSpotsHint). States: 0 load, 1 wait, 2 open, 3 free. */
typedef struct FieldstgSpots {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 count; /* entries in spots */
    /* 0x54 */ FieldstgSpot *spots;
    /* 0x58 */ s32 selected; /* selected entry */
    /* 0x5C */ s32 scripted; /* opened by fieldstg_spots_open_scripted (no battle roll) */
    /* 0x60 */ FieldstgPos target_pos;
} FieldstgSpots; /* size 0x68 */

/* The object fieldstg_spots_hint_create creates (fieldstg_spots_hint_update). */
typedef struct FieldstgSpotsHint {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 time; /* time */
    /* 0x54 */ s32 blink; /* frames per digit step: 3, 6 or 12 by distance */
    /* 0x58 */ s32 sprite; /* sprite ID */
    /* 0x5C */ FieldstgPos from;
    /* 0x64 */ FieldstgPos to;
} FieldstgSpotsHint; /* size 0x6C */

/* Data block of FieldstgSpots (base.children). */
typedef struct FieldstgSpotsData {
    /* 0x0 */ struct FieldstgSpotsEffect *effect;
    /* 0x4 */ FieldstgSpotsHint *hint;
} FieldstgSpotsData; /* size 0x8 */

/* The object fieldstg_spots_effect_create creates (fieldstg_spots_effect_update). */
typedef struct FieldstgSpotsEffect {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 x; /* x */
    /* 0x54 */ s32 y; /* y */
    /* 0x58 */ s32 dir; /* direction */
    /* 0x5C */ s32 sprite; /* sprite ID */
    /* 0x60 */ FieldstgAnimFrame *anim; /* animation (fieldstg_spots_effect_anims[dir]) */
} FieldstgSpotsEffect; /* size 0x64 */

/* The meter (fieldstg_meter_update, created by fieldstg_meter_create, map event 7): after 90 frames a cursor runs
 * back and forth; the button stops it, and the cell it stops on (fieldstg_meter_patterns) starts battle 4 or 7, or
 * nothing. */
typedef struct FieldstgMeter {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgPos pos;
    /* 0x58 */ s32 pattern; /* meter pattern (fieldstg_meter_patterns) */
    /* 0x5C */ s32 cursor; /* cursor, 0..0x3000 */
    /* 0x60 */ s32 speed; /* cursor speed */
    /* 0x64 */ s32 back; /* cursor moves back */
} FieldstgMeter; /* size 0x68 */

/* The camera (type 0x10: fieldstg_camera_update, created by fieldstg_camera_create): follows an actor
 * (sub-state 0) or a point (1), moving there smoothly while base.substep is 1, clamped to the map, with an
 * optional shake. */
typedef struct FieldstgCamera {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgActor *actor; /* the object of type 5 followed (fieldstg_camera_follow) */
    /* 0x54 */ FieldstgPos center; /* fieldstg_camera_apply centres the screen on it */
    /* 0x5C */ s32 shake; /* shake the screen (fieldstg_camera_set_shake) */
    /* 0x60 */ s32 shake_frame; /* shake frame, 0..3 */
    /* 0x64 */ s16 voice; /* voice of the shake sound, or -1 */
    /* 0x66 */ u8 unk_66[0x2];
    /* 0x68 */ FieldstgPos pos; /* the smoothed centre */
    /* 0x70 */ s32 has_map_size; /* map_size is known */
    /* 0x74 */ FieldstgPos map_size; /* map size */
    /* 0x7C */ s32 unk_7C;
    /* 0x80 */ s32 snap;     /* jump there instead of moving smoothly */
    /* 0x84 */ s32 actor_id; /* the actor followed (fieldstg_camera_follow) */
    /* 0x88 */ s32 target_x; /* the point followed (fieldstg_camera_move_to) */
    /* 0x8C */ s32 target_y;
} FieldstgCamera; /* size 0x90 */

/* The object of type 4 that fieldstg_camera_apply asks for the map size. */
typedef struct FieldstgBackgroundView {
    /* 0x000 */ Object base;
    /* 0x050 */ u8 unk_050[0xE0];
    /* 0x130 */ FieldstgPos *(*get_size)(struct FieldstgBackgroundView *obj);
} FieldstgBackgroundView;

/* An entry of FieldstgTrail.positions. */
typedef struct FieldstgTrailPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
    /* 0x8 */ s32 dir;
} FieldstgTrailPos; /* size 0xC */

/* FieldstgActor.trail: a ring buffer of 64 positions of the leader, which a partner replays behind it
 * (fieldstg_actor_follow; fieldstg_actor_follow_hide feeds it zeros to hide the partner). */
typedef struct FieldstgTrail {
    /* 0x000 */ FieldstgActor *leader; /* the actor followed */
    /* 0x004 */ s32 write;          /* write index */
    /* 0x008 */ s32 read;          /* read index */
    /* 0x00C */ FieldstgTrailPos positions[64];
} FieldstgTrail; /* size 0x30C */

/* A box around an actor (fieldstg_actor_boxes, rebuilt once per frame by fieldstg_attr_is_free). */
typedef struct FieldstgActorBox {
    /* 0x0 */ s32 x0;
    /* 0x4 */ s32 x1;
    /* 0x8 */ s32 y0;
    /* 0xC */ s32 y1;
} FieldstgActorBox; /* size 0x10 */

/* An entry of fieldstg_stages_2d/fieldstg_stages (fieldstg_find_stage): per stage ID, the values of
 * fieldstg_stage.code_file and entry. */
typedef struct FieldstgStageEntry {
    /* 0x0 */ s32 id;
    /* 0x4 */ s32 file; /* the stage's code (a WSTAG file), 0: none */
    /* 0x8 */ void *(*entry)(); /* entry of the map's code */
} FieldstgStageEntry; /* size 0xC */

/* A step of the screen-shatter transition (fieldstg_manager_shatter): move one tile coordinate. */
typedef struct FieldstgShatterStep {
    /* 0x0 */ s32 *coord; /* the coordinate (in fieldstg_shatter_tiles) */
    /* 0x4 */ s32 speed;  /* its speed (0: the last step) */
    /* 0x8 */ s32 limit;  /* its limit */
} FieldstgShatterStep;

extern s16 fieldstg_indexed_events[];
extern s32 fieldstg_actor_sprite_files[];
extern u8 fieldstg_actor_widths[];
extern u8 fieldstg_icon_anims[][9];
extern s32 fieldstg_encounter_rates[];
extern u8 fieldstg_map_event_facing[][8];
extern FieldstgStageEntry fieldstg_stages_2d[];
extern FieldstgStageEntry fieldstg_stages[];
extern s16 fieldstg_inn_maps[];
extern s32 fieldstg_pad_dirs[];
extern s32 fieldstg_meter_partner_types[];
extern s32 fieldstg_hide_partner_types[];
extern s32 fieldstg_follow_partner_types[];
extern FieldstgPos fieldstg_camera_shake_offsets[];
extern FieldstgPos fieldstg_front_offsets[];
extern FieldstgPos fieldstg_slope_steps[][8];
extern u8 fieldstg_slope_mirror_dirs[];
extern void (*fieldstg_encounter_step_func)(void);
extern void (*fieldstg_start_listed_battle_func)();
extern FieldstgAnimFrame fieldstg_spots_open_anim[];
extern u8 *fieldstg_meter_patterns[];
extern FieldstgAnimFrame *fieldstg_actor_effect_anims[];
extern s16 fieldstg_ladder_offsets[][2];
extern s32 fieldstg_ladder_step;
extern FieldstgSprite *fieldstg_sprites_search_next;
extern s32 fieldstg_sprites_search_key;
extern u8 fieldstg_collision_probes[][5];
extern FieldstgPos fieldstg_collision_probe_pos[];
extern u8 fieldstg_collision_probe_push[][2];
extern s16 fieldstg_talk_redirects[][2];
extern FieldstgAnimFrame *fieldstg_spots_effect_anims[];
extern s32 fieldstg_spots_effect_depths[];
extern s32 fieldstg_actor_boxes_frame;
FieldstgActorBox fieldstg_actor_boxes[20]; /* .bss */
s32 fieldstg_actor_box_count;
extern FieldstgPos fieldstg_shatter_tiles[5][6];
extern s32 fieldstg_shatter_state;
extern s16 fieldstg_carry_voice;
extern RECT fieldstg_shatter_rect;
extern FieldstgShatterStep fieldstg_shatter_steps[];
extern FieldstgSprite D_FIELDSTG_800996C4[];
extern FieldstgMapEvent D_FIELDSTG_800997C0[];
extern FieldstgVramPlace D_FIELDSTG_80097D0C[];
extern FieldstgPlacedActor *D_FIELDSTG_800994F4[];
extern FieldstgEventDef D_FIELDSTG_80099844[];

void fieldstg_icon_update(FieldstgIcon *obj);
void fieldstg_map_events_update();
void fieldstg_dialog_update();
void fieldstg_sprites_update();
FieldstgSprite *fieldstg_sprites_find_next(void);
s32 fieldstg_loader_run(Object *obj);
void fieldstg_manager_update();
void fieldstg_start_battle(s32 battle);
void fieldstg_actor_effect_update();
void fieldstg_launch_update();
void fieldstg_spots_hint_update();
FieldstgSpots *fieldstg_spots_create(s32 count);
void fieldstg_spots_update();
void fieldstg_spots_pick_target(FieldstgSpots *obj);
void fieldstg_goto_map_delayed(s32 map, s32 entry, s32 x, s32 y, s32 dir, s32 delay);
struct FieldstgSpotsHint *fieldstg_spots_hint_create(FieldstgPos from, FieldstgPos to);
struct FieldstgSpotsEffect *fieldstg_spots_effect_create(s32 arg0);
void fieldstg_spots_effect_update();
void fieldstg_meter_update();
LATE_FUNC(2, 0x800A6024, void, func_800A6024, (void));
void fieldstg_camera_update();
void fieldstg_player_control();
void fieldstg_player_control_height();
void fieldstg_actor_update();
void fieldstg_partners_follow(void);
void fieldstg_actor_follow();
void fieldstg_actor_follow_hide();
void fieldstg_actor_walk_to_target();
void fieldstg_actor_set_move(FieldstgActor *obj, s32 pad_dir);
FieldstgActor *fieldstg_actor_find_at(FieldstgPos *pos);
s32 fieldstg_actor_talk_to(FieldstgActor *obj, FieldstgPos *pos);
void fieldstg_actor_draw(void *arg0, void *arg1, s32 arg);
s32 fieldstg_map_events_enter(FieldstgMapEvents *obj, FieldstgMapEventsData *data);
void fieldstg_map_events_run(FieldstgMapEvents *obj);
void fieldstg_actor_update_sprite(FieldstgActor *obj);
void fieldstg_actor_run_state(FieldstgActor *obj, void **data);
Object *fieldstg_warp_picture_create(s32 type);
/* Defined with an s16 arg2 (fieldstg_80085590.c); this file passes an int (lw, no truncation). */
Object *fieldstg_warp_effect_create(s32 x, s32 y, s32 type);
Object *fieldstg_background_create(s32 file);
Object *fieldstg_map_title_create(s32 show);
FieldstgCamera *fieldstg_camera_create(void);
FieldstgActor *fieldstg_actor_create(s32 id, s32 type, s32 vram, FieldstgPlacedActor *placed);
void fieldstg_encounter_start(void);
void fieldstg_encounter_reset(void);
s32 fieldstg_attr_load_layer(s32 i);
u8 fieldstg_attr_get(s32 layer, FieldstgPos *pos);

s32 fieldstg_icon_step_anim(FieldstgIcon *obj) {
    obj->anim_time -= gfx_module.funcs.get_frame_ticks();
    if (obj->anim_time < 0) {
        obj->anim_pos += 2;
        if (fieldstg_icon_anims[obj->base.key2][obj->anim_pos] == 0xFF) {
            obj->anim_pos = 0;
        }
        obj->anim_time = fieldstg_icon_anims[obj->base.key2][obj->anim_pos + 1];
    }
    return fieldstg_icon_anims[obj->base.key2][obj->anim_pos];
}

void fieldstg_icon_draw(FieldstgIcon *obj) {
    Sprite sprite;
    FieldstgPos pos;
    s32 frame;

    pos.x = obj->actor->pixel_pos.x;
    pos.y = obj->actor->pixel_pos.y - (obj->actor->height >> 8);
    sprite_init(&sprite);
    sprite.set_layer_id(0x1002, 1);
    sprite.set_vram_pos(0x200, 0x100);
    if (obj->base.step == 2) {
        frame = fieldstg_icon_step_anim(obj);
        sprite.draw(cdload_module.get_subfile_by_id(0x1600000), frame, pos.x, pos.y - 0x1B);
    }
    sprite.draw(cdload_module.get_subfile_by_id(0x1600000), obj->frame >> 2, pos.x, pos.y - 0x1B);
}

void fieldstg_icon_update(FieldstgIcon *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->actor == NULL) {
            obj->actor = (FieldstgActor *)heap_objects.find(5, -1, 0);
            if (obj->actor == NULL) {
                break;
            }
        }
        if (obj->base.key1 == 0) {
            obj->frame_start = 200;
            obj->frame_shown = 212;
            obj->frame_end = 220;
        } else {
            obj->frame_start = 260;
            obj->frame_shown = 268;
            obj->frame_end = 276;
        }
        if (obj->base.key2 != 1) {
            sound_module.play(0x40007);
        }
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        if (fieldstg_stage.menu_open != 0) {
            break;
        }
        switch (obj->base.step) {
        case 0:
        default:
            obj->frame = obj->frame_start;
            obj->base.next_step(obj);
        case 1:
            obj->frame += gfx_module.funcs.get_frame_ticks();
            if (obj->frame >= obj->frame_shown) {
                obj->frame = obj->frame_shown;
                obj->base.next_step(obj);
            }
            break;
        case 2:
            break;
        }
        fieldstg_icon_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->frame += gfx_module.funcs.get_frame_ticks();
        if (obj->frame >= obj->frame_end) {
            obj->frame = obj->frame_end;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        fieldstg_icon_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FieldstgIcon *fieldstg_icon_create(s32 style, s32 type, s32 id) {
    FieldstgIcon *obj = object_create(fieldstg_icon_update, sizeof(FieldstgIcon), 0, id);

    obj->base.key1 = style;
    obj->base.key2 = type;
    return obj;
}

OBJECT_V0(FieldstgIcon *) fieldstg_icon_start(s32 id) {
    OBJECT_V0_TAIL(fieldstg_icon_create(0, 0, id)) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

void fieldstg_icon_message(FieldstgIcon *obj, s32 msg, s32 actor_id) {
    if (obj != NULL) {
        switch (msg) {
        case 0x325:
            obj->base.key2 = 0;
            break;
        case 0x327:
            obj->base.key2 = 1;
            break;
        case 0x326:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            break;
        }
        if (msg == 0x325 || msg == 0x327) {
            obj->actor = (FieldstgActor *)heap_objects.find(5, actor_id, -1);
        }
    }
}

s32 fieldstg_map_events_check(FieldstgMapEvents *obj) {
    FieldstgActor *player = obj->player;
    FieldstgPos pos = player->pixel_pos;
    u32 attr = fieldstg_attr.get(7, &pos);

    if (attr == 0) {
        return 0;
    }
    obj->dir = attr >> 5;
    obj->index = attr & 0x1F;
    obj->event = &obj->events[obj->index];
    switch (obj->event->type) {
    case 5:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
        return 1;
    }
    return fieldstg_map_event_facing[obj->dir][player->dir] != 0;
}

s32 fieldstg_map_events_enter(FieldstgMapEvents *obj, FieldstgMapEventsData *data) {
    s32 type;

    if (obj->event->flag != 0xFFFF && !gamestate_flags.get_flag(obj->event->flag, obj->event->flag_value)) {
        return 0;
    }
    if (obj->event->flag_2 != 0xFFFF && !gamestate_flags.get_flag(obj->event->flag_2, obj->event->flag_2_value)) {
        return 0;
    }
    switch (obj->event->type) {
    case 5:
        obj->player->depth = obj->event->param;
        gamestate_data.player_depth = obj->event->param;
        return 0;
    case 6:
        fieldstg_attr.set_layer(obj->event->param);
        return 0;
    case 8:
        if (data->event == NULL) {
            data->event = fieldstg_event_start(obj->event->param);
        }
        return 0;
    case 11:
        obj->player->start_carry(obj->player, obj->dir);
        return 0;
    case 12:
        obj->player->stop_carry(obj->player);
        return 0;
    case 13:
        obj->player->launch(obj->player, &obj->event->param);
        return 0;
    }
    switch (obj->event->type) {
    default:
        type = 0;
        break;
    case 2:
    case 3:
        type = 2;
        break;
    case 4:
        type = 4;
        break;
    case 7:
        type = 5;
        break;
    case 1:
        if (obj->dir == 4) {
            type = 10;
        } else {
            type = (obj->dir >> 1) + 6;
        }
        break;
    }
    if (data->icon != NULL) {
        data->icon->base.set_state(data->icon, OBJECT_STATE_RUN);
        data->icon->base.key2 = type;
        data->icon->anim_pos = 0;
        data->icon->anim_time = 0;
    } else {
        data->icon = fieldstg_icon_create(0, type, 6);
    }
    return 1;
}

void fieldstg_map_events_run(FieldstgMapEvents *obj) {
    static const FieldstgPos jump_pos = { 0, 0 };
    FieldstgSprite *sprite;
    s32 key;
    FieldstgMapEvent *entry;

    switch (obj->event->type) {
    case 1:
        obj->player->walk_out(obj->player, obj->dir);
        fieldstg_goto_map(obj->event->param, -1, obj->event->x << 8, obj->event->y << 8,
                               obj->event->dir);
        entry = obj->event;
        if (entry->hide_type != 0) {
            key = entry->hide_type;
            for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
                if (sprite->type == key) {
                    sprite->shown = 0;
                }
            }
        }
        gamestate_data.route = obj->event->route;
        gamestate_data.room = obj->event->room;
        break;
    case 14:
        obj->player->launch(obj->player, &obj->event->param);
        fieldstg_goto_map_delayed(obj->event->param, -1, obj->event->x << 8, obj->event->y << 8,
                               obj->event->dir, 60);
        break;
    case 2:
        obj->player->climb_from_bottom(obj->player, obj->dir, obj->event->x, obj->event->y,
                             (obj->event->param - 1) * 16);
        break;
    case 3:
        obj->player->climb_from_top(obj->player, obj->dir == 1 ? 5 : 3, obj->event->x, obj->event->y,
                             (obj->event->param - 1) * 16);
        break;
    case 4:
        obj->player->jump_down(obj->player, obj->dir, jump_pos, obj->event->param * 16);
        break;
    case 7:
        obj->player->start_meter(obj->player, obj->dir,
                             (FieldstgPos){ obj->event->param, obj->event->x });
        break;
    case 10:
        obj->player->warp(obj->player, &obj->event->param, 0);
        break;
    case 9:
        obj->player->warp(obj->player, &obj->event->param, 1);
        break;
    }
}

void fieldstg_map_events_update(FieldstgMapEvents *obj, FieldstgMapEventsData *data) {
    obj->events = fieldstg_stage.map_events;
    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        obj->player = (FieldstgActor *)heap_objects.find(5, -1, 0);
        if (obj->player != NULL) {
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        if (fieldstg_stage.event_running == 0 && fieldstg_stage.title_shown == 0) {
            switch (obj->base.step) {
            case 0:
            default:
                if (fieldstg_map_events_check(obj) && fieldstg_map_events_enter(obj, data)) {
                    obj->base.next_step(obj);
                }
                break;
            case 1:
                if (!fieldstg_map_events_check(obj)) {
                    obj->base.set_step(obj, 0);
                    data->icon->base.set_state(data->icon, OBJECT_STATE_DONE);
                } else if ((pad_state.get_pressed(0) & 0x2000) && fieldstg_stage.title_shown == 0) {
                    data->icon->base.set_state(data->icon, OBJECT_STATE_END);
                    fieldstg_map_events_run(obj);
                    obj->base.next_step(obj);
                }
                break;
            case 2:
                if (obj->player->base.step < 5) {
                    obj->base.set_step(obj, 0);
                }
                break;
            }
        }
        break;
    }
}

FieldstgMapEvents *fieldstg_map_events_create(s32 arg0, FieldstgMapEvent *arg1) {
    FieldstgMapEvents *obj = object_new(fieldstg_map_events_update, sizeof(FieldstgMapEvents), sizeof(FieldstgMapEventsData));

    obj->sprite_file = arg0;
    return obj;
}

void fieldstg_dialog_get_pos(FieldstgDialog *obj, FieldstgPos *out) {
    FieldstgPos pos;

    pos.x = obj->actor->pixel_pos.x;
    pos.y = obj->actor->pixel_pos.y;
    if (obj->actor->base.key1 == 0xD6) {
        pos.y -= 0x15;
    }
    fieldstg_event_funcs.to_screen_pos(&pos);
    if (obj->type == 2 || obj->type == 3) {
        pos.x -= 11;
    } else {
        pos.x += 11;
    }
    if (obj->type == 0 || obj->type == 2) {
        pos.y -= 0x13;
    } else {
        pos.y -= 7;
    }
    *out = pos;
}

void fieldstg_dialog_update(FieldstgDialog *obj, MessageDialog **data) {
    FieldstgPos pos;
    FieldstgPos pos2;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        if (obj->fixed != 0) {
            *data = (MessageDialog *)message_box_create(0x1004, obj->messages, obj->message);
        } else {
            fieldstg_dialog_get_pos(obj, &pos);
            *data = message_create_dialog(0x1004, pos.x, pos.y, obj->messages, obj->message, obj->type);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (*data == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        } else if (obj->fixed == 0) {
            fieldstg_dialog_get_pos(obj, &pos2);
            (*data)->move(*data, pos2.x, pos2.y);
        }
        break;
    }
}

FieldstgDialog *fieldstg_dialog_create(FieldstgActor *actor, s32 message, s32 type, s32 fixed) {
    FieldstgDialog *obj = object_new(fieldstg_dialog_update, sizeof(FieldstgDialog), sizeof(MessageDialog *));

    obj->actor = actor;
    obj->message = message;
    obj->type = type;
    obj->fixed = fixed;
    obj->messages = cdload_module.get_subfile_by_id(fieldstg_stage.event_text);
    return obj;
}

FieldstgDialog *fieldstg_dialog_create_talk(FieldstgActor *actor, s32 message) {
    FieldstgDialog *obj = object_new(fieldstg_dialog_update, sizeof(FieldstgDialog), sizeof(MessageDialog *));
    FieldstgPos pos;

    obj->actor = actor;
    obj->message = message;
    obj->messages = cdload_module.files.get_file(fieldstg_stage.talk_file);
    pos = actor->pixel_pos;
    fieldstg_event_funcs.to_screen_pos(&pos);
    switch (actor->dir) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 7:
    default:
        if (pos.x < 160) {
            obj->type = 2;
        } else {
            obj->type = 0;
        }
        break;
    case 3:
    case 4:
    case 5:
        if (pos.x < 160) {
            obj->type = 3;
        } else {
            obj->type = 1;
        }
        break;
    }
    switch (obj->type) {
    case 0:
        if (pos.y < 121) {
            obj->type = 1;
        }
        break;
    case 2:
        if (pos.y < 121) {
            obj->type = 3;
        }
        break;
    case 1:
        if (pos.y >= 172) {
            obj->type = 0;
        }
        break;
    case 3:
        if (pos.y >= 172) {
            obj->type = 2;
        }
        break;
    }
    obj->fixed = 0;
    return obj;
}

void fieldstg_sprites_draw_entry(FieldstgSprites *obj, GfxLayer *layer, s32 i) {
    FieldstgSprite *entry = &obj->sprites[i];
    Sprite sprite;

    if (obj->base.state == OBJECT_STATE_RUN) {
        sprite_init(&sprite);
        sprite.set_clut8_pos(0, 0x1F0);
        sprite.set_layer(layer, entry->ot_depth);
        if (entry->type != 0xFF) {
            sprite.set_vram_pos(0x140, 0x100);
            sprite.set_palette(entry->frame);
            if (fieldstg_stage.color.cd != 0) {
                sprite.set_color(&fieldstg_stage.color);
            }
            sprite.draw(cdload_module.get_subfile_by_id(obj->file), entry->sprite, entry->x, entry->y);
        } else {
            sprite.set_vram_pos(0x200, 0x100);
            sprite.set_palette(entry->frame);
            sprite.draw(cdload_module.get_subfile_by_id(0x1600000), entry->sprite, entry->x, entry->y);
        }
    }
}

void fieldstg_sprites_update(FieldstgSprites *obj, FieldstgSpots **data) {
    Sprite sprite;
    GfxRect view;
    FieldstgSprite *entry;
    FieldstgSprite *other;
    GfxLayer *layer;
    s32 n;
    s32 i;
    s32 x;
    s32 y;
    s32 back;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        n = 0;
        for (other = obj->sprites; other->y != 0; other++) {
            if (other->type == 0xFF) {
                n++;
            }
        }
        if (n != 0) {
            data[0] = fieldstg_spots_create(n);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        entry = obj->sprites;
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->get_view_rect(layer, &view);
        sprite_init(&sprite);
        sprite.set_vram_pos(0x140, 0x100);
        sprite.set_clut8_pos(0, 0x1F0);
        for (i = 0; entry->present != 0; entry++, i++) {
            if (entry->shown != 0) {
                x = entry->type == 0xFF ? entry->x - 50 : entry->x;
                y = entry->type == 0xFF ? entry->y - 100 : entry->y;
                if (x >= view.x - entry->present && view.x + view.w >= x && y >= view.y - entry->present
                    && view.y + view.h >= y) {
                    switch (entry->anim_mode) {
                    case 1:
                        if (entry->frame_timer < 0x100) {
                            entry->sprite++;
                            if (entry->last_frame < entry->sprite) {
                                entry->sprite = entry->first_frame;
                            }
                            entry->frame_timer = entry->frame_time << 8;
                        } else {
                            entry->frame_timer -= 0x100;
                        }
                        break;
                    case 2:
                        if (entry->frame_timer < 0x100) {
                            entry->frame++;
                            if (entry->last_frame < entry->frame) {
                                entry->frame = entry->first_frame;
                            }
                            entry->frame_timer = entry->frame_time << 8;
                        } else {
                            entry->frame_timer -= 0x100;
                        }
                        break;
                    case 3:
                        if (entry->frame_timer & 0x8000) {
                            back = 0x8000;
                            entry->frame_timer &= 0x7FFF;
                            if (entry->frame_timer < 0x100) {
                                entry->frame--;
                                entry->frame_timer = entry->frame_time << 8;
                                if (entry->frame == (u8)(entry->first_frame - 1)) {
                                    entry->frame = entry->first_frame + 1;
                                    back = 0;
                                }
                            } else {
                                entry->frame_timer -= 0x100;
                            }
                            entry->frame_timer |= back;
                        } else if (entry->frame_timer < 0x100) {
                            entry->frame++;
                            entry->frame_timer = entry->frame_time << 8;
                            if (entry->frame == entry->last_frame + 1) {
                                entry->frame = entry->last_frame - 1;
                                entry->frame_timer |= 0x8000;
                            }
                        } else {
                            entry->frame_timer -= 0x100;
                        }
                        break;
                    }
                    if (entry->priority != 0) {
                        layer->add_callback(layer, (void (*)(void *, GfxLayer *, s32))fieldstg_sprites_draw_entry, obj, entry->priority, i);
                    } else {
                        sprite.set_layer_id(0x1002, entry->ot_depth);
                        sprite.set_palette(entry->frame);
                        if (fieldstg_stage.color.cd != 0) {
                            sprite.set_color(&fieldstg_stage.color);
                        }
                        sprite.draw(cdload_module.get_subfile_by_id(obj->file), entry->sprite, entry->x, entry->y);
                    }
                }
            }
        }
        break;
    }
}

FieldstgSprites *fieldstg_sprites_create(s32 file, FieldstgSprite *sprites) {
    FieldstgSprites *obj = object_new(fieldstg_sprites_update, sizeof(FieldstgSprites), sizeof(FieldstgSpots *));

    obj->sprites = sprites;
    obj->file = file;
    return obj;
}

FieldstgSprite *fieldstg_sprites_find_next(void) {
    FieldstgSprite *entry = fieldstg_sprites_search_next;
    s32 found = 0;

    while (entry->present != 0) {
        if (entry->type == fieldstg_sprites_search_key) {
            found = 1;
            break;
        }
        entry++;
    }
    fieldstg_sprites_search_next = entry + 1;
    if (found) {
        return entry;
    }
    return NULL;
}

OBJECT_V0(FieldstgSprite *) fieldstg_sprites_find_first(s32 arg0) {
    fieldstg_sprites_search_key = arg0;
    fieldstg_sprites_search_next = fieldstg_stage.sprites;
    OBJECT_V0_TAIL(fieldstg_sprites_find_next()) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

void fieldstg_loader_load_inn_names(Object *obj) {
    s16 id = gamestate_data.funcs.get_map();
    s32 found = 0;
    s32 i;

    for (i = 0; fieldstg_inn_maps[i] != 0; i++) {
        if (fieldstg_inn_maps[i] == id) {
            found = 1;
            break;
        }
    }
    if (found) {
        cdload_module.queue_file(records_language + 0x5C);
    }
}

void fieldstg_loader_load_action_anims(Object *obj) {
    FieldstgMapEvent *entry = fieldstg_stage.map_events;
    s32 a = 0;
    s32 b = 0;
    s32 c = 0;

    for (; entry->type != 0; entry++) {
        switch (entry->type) {
        case 2:
        case 3:
            a = 1;
            break;
        case 4:
            b = 1;
            break;
        case 7:
            c = 1;
            break;
        }
    }
    if (a) {
        cdload_module.queue_file(0x3CC);
    }
    if (b) {
        cdload_module.queue_file(0x3C9);
    }
    if (c) {
        cdload_module.queue_file(0x3CA);
    }
}

s32 fieldstg_loader_run(Object *obj) {
    Tim tim;
    Tim bg;

    switch (obj->substep) {
    case 0:
        switch (obj->timer) {
        case 0:
        default:
            cdload_module.queue_file(0x160);
            obj->next_timer(obj);
        case 1:
            if (cdload_module.is_loading(0x160) == 0) {
                tim_init(&tim);
                tim.set_image_pos(0x200, 0x100);
                tim.load_all(cdload_module.get_subfile_by_id(0x1600002));
                tim.set_image_pos(0x240, 0x100);
                tim.load_all(cdload_module.get_subfile_by_id(0x1600003));
                obj->next_substep(obj);
            }
            break;
        }
        break;
    case 1:
        switch (obj->timer) {
        case 0:
        default:
            if (fieldstg_stage.mask_subfile == 0 && fieldstg_stage.mask_file == 0) {
                obj->next_substep(obj);
                break;
            }
            if (fieldstg_stage.mask_subfile != 0) {
                cdload_module.queue_file(fieldstg_stage.mask_subfile >> 16);
            }
            if (fieldstg_stage.sprite_file != 0) {
                cdload_module.queue_file(fieldstg_stage.sprite_file >> 16);
            }
            if (fieldstg_stage.mask_file != 0) {
                cdload_module.queue_file(fieldstg_stage.mask_file);
            }
            obj->next_timer(obj);
        case 1:
            if (fieldstg_stage.mask_subfile != 0) {
                if (cdload_module.is_loading(fieldstg_stage.mask_subfile >> 16) != 0) {
                    break;
                }
                tim_init(&bg);
                bg.set_clut_pos(0, 0x1F0);
                bg.set_image_pos(0x140, 0x100);
                bg.load_all(cdload_module.get_subfile_by_id(fieldstg_stage.mask_subfile));
            }
            obj->next_timer(obj);
        case 2:
            if (fieldstg_stage.mask_file != 0) {
                if (cdload_module.is_loading(fieldstg_stage.mask_file) != 0) {
                    break;
                }
                tim_init(&bg);
                bg.set_clut_pos(0, 0x1F0);
                bg.set_image_pos(0x140, 0x100);
                bg.load_all((s32 *)cdload_module.files.get_file(fieldstg_stage.mask_file));
            }
            obj->next_timer(obj);
        case 3:
            if (fieldstg_stage.sprite_file != 0 && cdload_module.is_loading(fieldstg_stage.sprite_file >> 16) != 0) {
                break;
            }
            obj->next_substep(obj);
            break;
        }
        break;
    case 2:
        if (fieldstg_stage.mask_file != 0) {
            cdload_module.files.free_file(fieldstg_stage.mask_file);
        }
        obj->next_substep(obj);
    case 3:
        cdload_module.queue_file(0x286);
        cdload_module.queue_file(fieldstg_stage.talk_file);
        cdload_module.queue_file(records_language + 0xB0);
        fieldstg_loader_load_action_anims(obj);
        fieldstg_loader_load_inn_names(obj);
        obj->next_substep(obj);
        return 1;
    default:
        return 1;
    }
    return 0;
}

void fieldstg_loader_update(Object *obj) {
    switch (obj->state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        obj->step = obj->key2;
    case OBJECT_STATE_RUN:
        if (fieldstg_loader_run(obj) != 0) {
            obj->set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

Object *fieldstg_loader_create(s32 step) {
    Object *obj = object_new(fieldstg_loader_update, sizeof(Object) + 4, 0); /* 4 unused bytes past the header */

    obj->key2 = step;
    return obj;
}

void fieldstg_actor_effect_update(FieldstgActorEffect *obj) {
    Sprite sprite;
    FieldstgPos pos;
    FieldstgActor *actor;
    s32 frame;
    s32 time;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        if (obj->base.substep == 0) {
            obj->anim = fieldstg_actor_effect_anims[obj->base.step];
            obj->frame = 0;
            obj->time = 0;
            if (obj->base.step == 1 || obj->base.step == 3) {
                sound_module.play(0x40009);
            }
            obj->base.next_substep(obj);
        }
        if (obj->actor != NULL && obj->actor->base.state == OBJECT_STATE_RUN) {
            frame = obj->frame;
            time = obj->time;
            time += gfx_module.funcs.get_frame_ticks();
            if (obj->anim[frame].time < time) {
                time -= obj->anim[frame].time;
                frame++;
                if (obj->anim[frame].sprite == 0xFF) {
                    frame = obj->anim[frame].time;
                }
                obj->sprite = obj->anim[frame].sprite;
                obj->frame = frame;
            }
            obj->time = time;
            actor = obj->actor;
            switch (actor->base.step) {
            case 0x45:
                if (actor->ladder_side != 0) {
                    pos.x = actor->pixel_pos.x + fieldstg_ladder_offsets[fieldstg_ladder_step][0];
                } else {
                    pos.x = actor->pixel_pos.x - fieldstg_ladder_offsets[fieldstg_ladder_step][0];
                }
                pos.y = actor->pixel_pos.y + fieldstg_ladder_offsets[fieldstg_ladder_step][1];
                if (fieldstg_ladder_offsets[fieldstg_ladder_step + 1][0] != 0) {
                    fieldstg_ladder_step++;
                }
                break;
            case 0x44:
                if (fieldstg_ladder_step == 0) {
                    fieldstg_ladder_step = 14;
                }
                if (actor->ladder_side != 0) {
                    pos.x = actor->pixel_pos.x + fieldstg_ladder_offsets[fieldstg_ladder_step][0];
                } else {
                    pos.x = actor->pixel_pos.x - fieldstg_ladder_offsets[fieldstg_ladder_step][0];
                }
                pos.y = actor->pixel_pos.y + fieldstg_ladder_offsets[fieldstg_ladder_step][1];
                if (fieldstg_ladder_step != 1) {
                    fieldstg_ladder_step--;
                }
                break;
            default:
                pos.x = actor->pixel_pos.x;
                pos.y = actor->pixel_pos.y;
                fieldstg_ladder_step = 0;
                break;
            }
            sprite_init(&sprite);
            sprite.set_vram_pos(0x200, 0x100);
            sprite.set_layer_id(0x1002, 2);
            sprite.draw(cdload_module.get_subfile_by_id(0x1600000), obj->sprite, pos.x, pos.y);
        }
        break;
    }
}

FieldstgActorEffect *fieldstg_actor_effect_create(FieldstgActor *arg0) {
    FieldstgActorEffect *obj;

    if (gamestate_data.funcs.get_map() < 0x2D7) {
        obj = object_create(fieldstg_actor_effect_update, sizeof(FieldstgActorEffect), 0, 0x16);
        obj->actor = arg0;
        return obj;
    }
    return NULL;
}

void fieldstg_manager_shatter(FieldstgManager *obj, FieldstgManagerData *data) {
    GfxLayer *layer;
    u32 *ot;
    FieldstgShatterStep *step;
    s32 k;
    s32 done;
    POLY_FT4 *ft4;
    s32 dt;
    s32 i;
    s32 j;
    s32 x;
    s32 y;

    switch (obj->base.step) {
    case 0:
        cdload_module.mark_loaded();
        fieldstg_shatter_rect.x = 0;
        fieldstg_shatter_rect.y = 0;
        fieldstg_shatter_rect.w = 320;
        fieldstg_shatter_rect.h = 240;
        MoveImage(&fieldstg_shatter_rect, 0x280, 0);
        layer = gfx_module.funcs.get_layer(0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->set_bg_color(layer, 0, 0, 0);
        gamestate_data.field_map = gamestate_data.funcs.get_map();
        gamestate_data.player_pos = data->party[0]->pos;
        gamestate_data.player_dir = data->party[0]->dir;
        for (j = 0; j < 6; j++) {
            for (i = 0; i < 5; i++) {
                fieldstg_shatter_tiles[i][j].x = i << 6;
                fieldstg_shatter_tiles[i][j].y = j * 40;
            }
        }
        sound_module.stop_all();
        sound_module.play(0x40005);
        for (k = 0; k < obj->base.child_count; k++) {
            if (((Object **)data)[k] != NULL) {
                ((Object **)data)[k]->set_state(((Object **)data)[k], OBJECT_STATE_END);
            }
        }
        fieldstg_shatter_state = 0;
        obj->base.next_step(obj);
    case 1:
        switch (fieldstg_shatter_state) {
        case 0:
            if (cdload_reader.is_busy() == 0) {
                fieldstg_shatter_state++;
            }
            break;
        case 1:
            if (cdload_reader.is_busy() == 0) {
                cdload_module.queue_file(0x1CB);
                cdload_module.queue_file(0x1CC);
                cdload_module.queue_file(0x1CF);
                cdload_module.queue_file(0x208);
                cdload_module.queue_file(0x167);
                fieldstg_shatter_state++;
            }
            break;
        }
        layer = gfx_module.funcs.get_layer(0x1002);
        ot = layer->get_ot_entry(layer, 0);
        dt = gfx_module.funcs.get_frame_ticks() * 40;
        /* Evidence (class A2, sched2 barrier; DECISIONS "LOOP_BLOCK audit"): the original loads gfx_module.buffer only
         * after step, leaving two load-delay nops. */
        LOOP_BARRIER();
        step = &fieldstg_shatter_steps[obj->base.timer];
        if (gfx_module.buffer == 0) {
            if (step->speed != 0) {
                *step->coord += dt * step->speed;
                done = step->speed >= 0 ? step->limit > *step->coord : *step->coord > step->limit;
                if (!done) {
                    *step->coord = 0x200;
                    obj->base.next_timer(obj);
                }
            } else {
                fieldstg_shatter_tiles[2][3].x = 0x200;
                if (++obj->base.substep >= 16) {
                    if (obj->base.substep < 0x1000) {
                        gamestate_data.funcs.set_next_map(obj->next_map, obj->map_entry);
                    }
                    obj->base.substep = 0x1000;
                }
            }
        }
        if (obj->base.substep != 0x1000) {
            ft4 = gfx_module.funcs.get_packet();
            setRGB0((POLY_F4 *)ft4, 0x10, 0x10, 0x10);
            setPolyF4((POLY_F4 *)ft4);
            setSemiTrans((POLY_F4 *)ft4, 1);
            ((POLY_F4 *)ft4)->x0 = 0;
            ((POLY_F4 *)ft4)->x1 = 320;
            ((POLY_F4 *)ft4)->x2 = 0;
            ((POLY_F4 *)ft4)->x3 = 320;
            ((POLY_F4 *)ft4)->y0 = 0;
            ((POLY_F4 *)ft4)->y1 = 0;
            ((POLY_F4 *)ft4)->y2 = 240;
            ((POLY_F4 *)ft4)->y3 = 240;
            addPrim(ot, (POLY_F4 *)ft4);
            ft4 = (POLY_FT4 *)((POLY_F4 *)ft4 + 1);
            setDrawTPage((DR_TPAGE *)ft4, 0, 1, getTPage(0, 2, 320, 0));
            addPrim(ot, (DR_TPAGE *)ft4);
            ft4 = (POLY_FT4 *)((DR_TPAGE *)ft4 + 1);
            for (y = 0; y < 6; y++) {
                for (x = 0; x < 5; x++) {
                    if (fieldstg_shatter_tiles[x][y].x != 0x200 && fieldstg_shatter_tiles[x][y].y != 0x200) {
                        setPolyFT4(ft4);
                        setSemiTrans(ft4, 1);
                        setRGB0(ft4, 0x80, 0x80, 0x80);
                        ft4->x0 = ft4->x2 = fieldstg_shatter_tiles[x][y].x;
                        ft4->x1 = ft4->x3 = ft4->x0 + 64;
                        ft4->y0 = ft4->y1 = fieldstg_shatter_tiles[x][y].y;
                        ft4->y2 = ft4->y3 = ft4->y0 + 40;
                        ft4->u0 = 0;
                        ft4->u1 = 64;
                        ft4->u2 = 0;
                        ft4->u3 = 64;
                        ft4->v0 = y * 40;
                        ft4->v1 = y * 40;
                        ft4->v2 = y * 40 + 40;
                        ft4->v3 = y * 40 + 40;
                        ft4->tpage = getTPage(2, 0, 0x280 + x * 64, 0);
                        addPrim(ot, ft4);
                        ft4++;
                    }
                }
            }
            gfx_module.funcs.set_packet(ft4);
        } else {
            obj->base.next_step(obj);
        }
        break;
    case 2:
        break;
    }
}

void fieldstg_manager_leave_map(FieldstgManager *obj, FieldstgManagerData *data) {
    GfxLayer *layer;
    FieldstgActor *actor;
    FieldstgActor *player;
    s32 pos[2];
    s32 w;
    s32 h;

    switch (obj->base.step) {
    case 0:
    default:
        cdload_module.mark_loaded();
        actor = (FieldstgActor *)heap_objects.find(5, -1, 0);
        if (actor != NULL && actor->pixel_pos.x != 0) {
            obj->close_on_player = 1;
        } else {
            obj->close_on_player = 0;
        }
        obj->window_w = 320;
        obj->fade = 0;
        obj->window_h = 240;
        obj->base.next_step(obj);
    case 1:
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->set_bg_color(layer, 1, 1, 1);
        obj->window_w -= 10;
        obj->window_h -= 7;
        if (obj->window_w <= 0) {
            layer->set_bg_color(layer, 0, 0, 0);
            obj->window_w = 0;
            obj->window_h = 0;
            obj->base.next_step(obj);
        }
        player = (FieldstgActor *)heap_objects.find(5, -1, 0);
        layer->get_scroll(layer, pos);
        if (obj->close_on_player != 0) {
            pos[0] = player->pixel_pos.x - pos[0];
            pos[1] = player->pixel_pos.y - pos[1];
        } else {
            pos[0] = 160;
            pos[1] = 120;
        }
        pos[0] -= obj->window_w / 2;
        if (pos[0] < 0) {
            pos[0] = 0;
        }
        pos[1] -= obj->window_h / 2;
        if (pos[1] < 0) {
            pos[1] = 0;
        }
        layer->set_clip_pos(layer, pos[0], pos[1]);
        w = obj->window_w;
        h = obj->window_h;
        if (pos[0] + w > 320) {
            w = 320 - pos[0];
        }
        if (pos[1] + h > 240) {
            h = 240 - pos[1];
        }
        layer->set_clip_size(layer, w, h);
        fieldstg_background_fill_layer(0x1001, obj->fade);
        if (obj->fade != 0x8000) {
            obj->fade += 0x400;
        }
        break;
    case 3:
        break;
    case 2:
        layer = gfx_module.funcs.get_layer(0x1001);
        switch (obj->base.substep) {
        case 0:
        default:
            obj->window_w = 0;
            obj->window_h = 0;
            obj->base.substep++;
        case 1:
            break;
        }
        obj->window_w += 8;
        layer->set_clip_pos(layer, obj->window_w, obj->window_h);
        layer->set_clip_size(layer, (160 - obj->window_w) * 2, (120 - obj->window_h) * 2);
        if (obj->window_w > 160) {
            gamestate_data.funcs.set_next_map(obj->next_map, obj->map_entry);
            gamestate_data.field_map = gamestate_data.funcs.get_map();
            gamestate_data.player_pos = data->party[0]->pos;
            gamestate_data.player_dir = data->party[0]->dir;
            obj->base.next_step(obj);
        }
        fieldstg_background_fill_layer(0x1001, 0x8000);
        break;
    }
}

s32 fieldstg_map_has_no_tiles(void) {
    if (gamestate_data.funcs.get_map() == 0x22D) {
        return 1;
    }
    return gamestate_data.funcs.get_map() == 0x2DE;
}

#ifdef NON_MATCHING
/* 99.48%: register allocation only (obj in s3 instead of s4; the constant 1, the actor-list loops and the
 * fieldstg_stage/gamestate_data bases get other saved registers); declaration orders tried. The second actor loop has
 * its own list variable. By -dg: the original needs the first loop's hoisted constant 1 (5 refs/52) above obj
 * (54/1168), list above placed in that loop, and the top constant 1 (8/92) below both case-1 bases (6/44, 6/48);
 * a placed per loop, for/index loops, segvar --fresh and cmpswap change none of them. wip-13: 30 min of permuter
 * (420 -> 250) only splits the data parameter into a copy for some cases; block-local placed/n, list reused for the
 * second loop, a for/while(id == 0) first loop: no gain. US decomp (func_8008A154): still INCLUDE_ASM.
 * final-rest: per-case copies of the first loop's test (merged by cross-jumping), a default case: no gain.
 * last-rest (final): the permuter (25 min, boosted weights) found `do { switch (obj->base.step) {...} } while (0)`
 * around case 0's inner switch (99.80: obj and the second loop right); with `placed` block-local in the first loop
 * too, 99.87: left only case 1's fieldstg_stage/gamestate_data bases and the top constant 1 (here stage s0,
 * constant s1, gamestate s2; the original gamestate s0, stage s1, constant s2). Not adopted (forced and not 100%);
 * a block-local leader pointer in case 1: worse. */
void fieldstg_manager_update(FieldstgManager *obj, FieldstgManagerData *data) {
    RECT rect;
    GfxLayer *layer;
    FieldstgPlacedActor **list;
    FieldstgPlacedActor *placed;
    s32 id;
    s32 map;
    s32 i;
    s32 n;
    FieldstgPlacedActor **list2;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            if (fieldstg_map_has_no_tiles() == 0) {
                cdload_module.files.free_above(HEAP_ADDR(0x8015C674));
            }
            gfx_module.reset();
            gfx_module.alloc_packet_buffers(0x6400);
            gfx_module.funcs.init_display(320, 240, 0, 0);
            rect.x = 0;
            rect.y = 0;
            rect.w = 320;
            rect.h = 240;
            layer = gfx_module.funcs.create_layer(&rect, 1, 0x1000);
            layer->set_bg_color(layer, 1, 1, 1);
            gfx_module.funcs.create_layer(&rect, 1, 0x1001);
            layer = gfx_module.funcs.create_layer(&rect, 4, 0x1002);
            layer->alloc_callbacks(layer, 50);
            gfx_module.funcs.create_layer(&rect, 1, 0x1004);
            layer = gfx_module.funcs.create_layer(&rect, 1, 0x1003);
            layer->set_bg_color(layer, 1, 1, 1);
            if (fieldstg_map_has_no_tiles() == 0) {
                obj->buffer = heap_funcs.alloc_top(0x9615C, 2);
            }
            if (gamestate_data.map_is_new == 0) {
                cdload_module.age_marked();
            }
            fieldstg_attr.set_file(4, 0);
            fieldstg_stage.find_stage();
            if (fieldstg_stage.code_file != 0) {
                overlay_module.load_file(fieldstg_stage.code_file);
            }
            if (fieldstg_stage.entry != NULL) {
                data->stage = OVERLAY_FN(2, fieldstg_stage.entry)(obj);
            }
            gamestate_flags.update_map_flags();
            gamestate_flags.set_flag(gamestate_data.funcs.get_map() + 0x1E00, 1);
            map = gamestate_data.funcs.get_prev_map();
            if ((map & 0xFF00) != 0x200 && (map & 0xFF00) != 0x300 && (map & 0xFF00) != 0xE00 && map != 0x1500
                && map != 0x500) {
                gamestate_data.map_entry = -1;
                fieldstg_stage.return_pos = gamestate_data.player_pos;
                fieldstg_stage.return_dir = gamestate_data.player_dir;
            }
            data->map_title = fieldstg_map_title_create(1);
            obj->base.next_step(obj);
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                if (fieldstg_stage.music != 0) {
                    sound_module.load_extra_bank(fieldstg_stage.music);
                }
                obj->base.next_substep(obj);
            case 1:
                break;
            }
            if (sound_module.is_loading() == 0) {
                if (fieldstg_stage.sound != 0) {
                    sound_module.play(fieldstg_stage.sound);
                } else {
                    sound_module.stop(sound_module.current);
                }
                obj->base.next_step(obj);
            }
            break;
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                data->unk_10 = fieldstg_loader_create(0);
                obj->base.next_substep(obj);
                break;
            case 1:
                if (data->unk_10 == NULL) {
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 3:
            if (fieldstg_stage.sprites != NULL) {
                data->sprites = (Object *)fieldstg_sprites_create(fieldstg_stage.sprite_file, fieldstg_stage.sprites);
            }
            if (fieldstg_stage.map_events != NULL) {
                data->map_events = fieldstg_map_events_create(fieldstg_stage.sprite_file, fieldstg_stage.map_events);
            }
            list = fieldstg_stage.actors;
            id = 0;
            if (list != NULL) {
                while (*list != NULL) {
                    placed = *list;
                    switch (placed->id) {
                    case 1:
                    case 0x6A:
                    case 0x146:
                    case 0x147:
                        if (placed->flags_required == NULL || gamestate_flags.check_flags(placed->flags_required) == 1) {
                            id = placed->id;
                        }
                        break;
                    }
                    if (id != 0) {
                        break;
                    }
                    list++;
                }
            }
            if (id != 0) {
                data->party[0] = fieldstg_actor_create(id, 0, 0, NULL);
            } else {
                data->party[0] = fieldstg_actor_create(2, 0, 0, NULL);
                if (gamestate_data.progress >= 3) {
                    if (gamestate_data.funcs.get_party_digimon(0) >= 0) {
                        data->party[1] = fieldstg_actor_create(gamestate_data.funcs.get_party_digimon(0) + 3, 2, 1, NULL);
                    }
                    if (gamestate_data.funcs.get_party_digimon(1) >= 0) {
                        data->party[2] = fieldstg_actor_create(gamestate_data.funcs.get_party_digimon(1) + 3, 4, 2, NULL);
                    }
                    if (gamestate_data.funcs.get_party_digimon(2) >= 0) {
                        data->party[3] = fieldstg_actor_create(gamestate_data.funcs.get_party_digimon(2) + 3, 8, 3, NULL);
                    }
                }
            }
            list2 = fieldstg_stage.actors;
            if (list2 != NULL) {
                n = 0;
                while (*list2 != NULL) {
                    placed = *list2;
                    switch (placed->id) {
                    case 1:
                    case 0x6A:
                    case 0x146:
                    case 0x147:
                        break;
                    default:
                        if (placed->flags_required == NULL || gamestate_flags.check_flags(placed->flags_required) != 0) {
                            data->actors[n] = fieldstg_actor_create(placed->id, 1, placed->vram_place, placed);
                            data->actors[n]->pos.x = placed->x << 8;
                            data->actors[n]->pos.y = placed->y << 8;
                            data->actors[n]->dir = placed->dir;
                            n++;
                        }
                        break;
                    }
                    list2++;
                }
            }
            data->camera = fieldstg_camera_create();
            obj->base.next_step(obj);
            break;
        case 4:
            if (fieldstg_map_has_no_tiles() == 0) {
                switch (obj->base.substep) {
                case 0:
                default:
                    if (cdload_reader.is_busy() != 0) {
                        break;
                    }
                    if (obj->buffer != NULL) {
                        heap_funcs.free(obj->buffer);
                    }
                    data->background = fieldstg_background_create(fieldstg_stage.background_file);
                    obj->base.next_substep(obj);
                case 1:
                    if (data->background->state == OBJECT_STATE_RUN && data->map_title->state == OBJECT_STATE_DONE) {
                        data->map_title->set_step(data->map_title, 1);
                        obj->base.next_state(obj);
                    }
                    break;
                }
            } else if (data->map_title->state == OBJECT_STATE_DONE) {
                data->map_title->set_step(data->map_title, 1);
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (fieldstg_stage.menu_open != 1 && fieldstg_stage.event_running == 0 && fieldstg_stage.actor_busy == 0
                && gamestate_data.progress >= 4 && (pad_state.get_pressed(0) & 8)) {
                fieldstg_stage.menu_open = 1;
                fieldstg_stage.event_running = 1;
                data->menu = fieldmenu_create(0x1002, 0);
                gamestate_data.field_map = gamestate_data.funcs.get_map();
                gamestate_data.player_pos = data->party[0]->pos;
                gamestate_data.player_dir = data->party[0]->dir;
                data->party[0]->base.set_step(data->party[0], 1);
                cdload_module.mark_loaded();
                obj->base.next_step(obj);
            }
            break;
        case 1:
            if (data->menu == NULL) {
                fieldstg_stage.menu_open = 0;
                fieldstg_stage.event_running = 0;
                obj->base.set_step(obj, 0);
            }
            break;
        case 2:
            if (data->unk_10 == NULL) {
                fieldstg_stage.menu_open = 0;
                fieldstg_stage.event_running = 0;
                obj->base.set_step(obj, 0);
            }
            break;
        case 3:
            switch (obj->base.substep) {
            case 0:
            default:
                data->warp_effect = fieldstg_warp_effect_create(obj->warp_x, obj->warp_y, obj->warp_type);
                sound_module.play(0x40004);
                obj->base.next_substep(obj);
            case 1:
                if (obj->base.timer < 60) {
                    obj->base.timer += gfx_module.funcs.get_frame_ticks();
                    break;
                }
                data->fade = inn_fade_create(0x1002);
                data->fade->start(data->fade, 0, 20);
                obj->base.next_substep(obj);
            case 2:
                if (data->fade->base.state == OBJECT_STATE_DONE) {
                    for (i = 0; i < 4; i++) {
                        if (data->party[i] != NULL) {
                            data->party[i]->base.set_state(data->party[i], OBJECT_STATE_END);
                        }
                    }
                    for (i = 0; i < 15; i++) {
                        if (data->actors[i] != NULL) {
                            data->actors[i]->base.set_state(data->actors[i], OBJECT_STATE_END);
                        }
                    }
                    data->background->set_state(data->background, OBJECT_STATE_END);
                    data->sprites->set_state(data->sprites, OBJECT_STATE_END);
                    data->warp_picture = fieldstg_warp_picture_create(obj->warp_type);
                    data->fade->start(data->fade, 1, 20);
                    obj->base.next_substep(obj);
                }
                break;
            case 3:
                if (data->warp_picture->state == OBJECT_STATE_DONE) {
                    gamestate_data.route = obj->warp_params[5];
                    gamestate_data.room = obj->warp_params[6];
                    fieldstg_goto_map((s16)obj->warp_params[0], -1, (s16)obj->warp_params[1] << 8, (s16)obj->warp_params[2] << 8,
                                           (s16)obj->warp_params[3]);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (fieldstg_stage.battle_starting != 0) {
            fieldstg_manager_shatter(obj, data);
        } else if (obj->delay <= 0) {
            fieldstg_stage.title_shown = 1;
            fieldstg_manager_leave_map(obj, data);
        } else {
            obj->delay -= gfx_module.funcs.get_frame_ticks();
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}
#else
INCLUDE_ASM("asm/fieldstg/nonmatchings/fieldstg_80087DB0", fieldstg_manager_update);
#endif

FieldstgManager *fieldstg_manager_create(void) {
    return object_create(fieldstg_manager_update, sizeof(FieldstgManager), sizeof(FieldstgManagerData), 7);
}

void fieldstg_goto_map_delayed(s32 map, s32 entry, s32 x, s32 y, s32 dir, s32 delay) {
    FieldstgManager *obj = (FieldstgManager *)heap_objects.find(7, -1, -1);

    if (obj != NULL) {
        obj->next_map = map;
        obj->map_entry = entry;
        obj->delay = delay;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
        fieldstg_stage.return_pos.x = x;
        fieldstg_stage.return_pos.y = y;
        fieldstg_stage.return_dir = dir;
    }
}

void fieldstg_goto_map(s32 map, s32 entry, s32 x, s32 y, s32 dir) {
    fieldstg_goto_map_delayed(map, entry, x, y, dir, 0);
}

void fieldstg_start_battle(s32 battle) {
    FieldstgManager *mgr;
    s32 i;
    s32 map;
    s32 r;

    mgr = (FieldstgManager *)heap_objects.find(7, -1, -1);
    if (mgr == NULL) {
        return;
    }
    fieldstg_stage.event_running = 1;
    fieldstg_stage.battle_starting = 1;
    mgr->next_map = gamestate_data.progress != 0x2B ? 0x600 : 0xE0A;
    mgr->map_entry = 0;
    mgr->base.set_state(mgr, OBJECT_STATE_DONE);
    records_state.battle = battle;
    records_state.first_strike_chance = fieldstg_battles[battle].first_strike;
    records_state.unk_3D = fieldstg_battles[battle].unk_0D;
    for (i = 0; i < 12; i++) {
        records_state.blocked[i] = fieldstg_battles[battle].unk_0E[i];
    }
    for (i = 0; i < 3; i++) {
        records_state.enemies[i] = *fieldstg_battles[battle].enemies[i];
    }
    records_state.has_prize = 0;
    if ((u32)(records_state.enemies[0].digimon - 0x1C9) >= 8) {
        return;
    }
    records_state.has_prize = 1;
    map = gamestate_data.funcs.get_map();
    if (records_state.enemies[0].digimon & 1) {
        r = pad_random.next() & 0x1F;
        switch (map) {
        case 0x21D:
        default:
            if (r != 0) {
                records_state.prize_item = 0x177;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x22A:
            if (r != 0) {
                records_state.prize_item = 0x179;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x233:
        case 0x235:
        case 0x237:
        case 0x23A:
        case 0x23B:
        case 0x23C:
        case 0x24A:
        case 0x24C:
            if (r != 0) {
                records_state.prize_item = 0x17A;
            } else {
                records_state.prize_item = 0x187;
            }
            break;
        case 0x28C:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17F;
            } else {
                records_state.prize_item = 0x188;
            }
            break;
        case 0x28D:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x179;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x28E:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x178;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x28F:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17E;
            } else {
                records_state.prize_item = 0x188;
            }
            break;
        case 0x290:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x180;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x291:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x181;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x296:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x182;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x298:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17C;
                } else {
                    records_state.prize_item = 0x187;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x183;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        case 0x299:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17F;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x185;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        case 0x2A1:
        case 0x2A3:
        case 0x2A4:
        case 0x2A7:
        case 0x2A8:
        case 0x2A9:
            if (r != 0) {
                records_state.prize_item = 0x180;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x261:
        case 0x262:
        case 0x265:
        case 0x266:
            if (r != 0) {
                records_state.prize_item = 0x181;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x2B4:
        case 0x2B6:
        case 0x2C9:
        case 0x2CA:
        case 0x2CD:
        case 0x2CE:
            if (r != 0) {
                records_state.prize_item = 0x184;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        }
    } else {
        r = pad_random.next() & 0xF;
        switch (map) {
        case 0x201:
        default:
            if (r != 0) {
                records_state.prize_item = 0x177;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x234:
        case 0x235:
        case 0x237:
        case 0x23A:
        case 0x23B:
        case 0x23C:
        case 0x23D:
            if (r != 0) {
                records_state.prize_item = 0x178;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x247:
        case 0x249:
        case 0x24B:
            if (r != 0) {
                records_state.prize_item = 0x17B;
            } else {
                records_state.prize_item = 0x187;
            }
            break;
        case 0x271:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x177;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x28C:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17A;
            } else {
                records_state.prize_item = 0x187;
            }
            break;
        case 0x28D:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x179;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x28E:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x178;
            } else {
                records_state.prize_item = 0x186;
            }
            break;
        case 0x28F:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17B;
            } else {
                records_state.prize_item = 0x187;
            }
            break;
        case 0x290:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x184;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        case 0x296:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17C;
            } else {
                records_state.prize_item = 0x187;
            }
            break;
        case 0x299:
            if (gamestate_data.progress != 0x2D) {
                if (r != 0) {
                    records_state.prize_item = 0x17D;
                } else {
                    records_state.prize_item = 0x188;
                }
            } else if (r != 0) {
                records_state.prize_item = 0x17D;
            } else {
                records_state.prize_item = 0x188;
            }
            break;
        case 0x2A2:
        case 0x2A3:
        case 0x2A4:
        case 0x2A7:
        case 0x2A8:
        case 0x2A9:
        case 0x2AA:
            if (r != 0) {
                records_state.prize_item = 0x17E;
            } else {
                records_state.prize_item = 0x188;
            }
            break;
        case 0x266:
            if (r != 0) {
                records_state.prize_item = 0x182;
            } else {
                records_state.prize_item = 0x189;
            }
            break;
        case 0x2B1:
        case 0x2B3:
        case 0x2B5:
            if (r != 0) {
                records_state.prize_item = 0x183;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        case 0x2CE:
            if (r != 0) {
                records_state.prize_item = 0x185;
            } else {
                records_state.prize_item = 0x18A;
            }
            break;
        }
    }
}

s32 fieldstg_start_battle_5(void) {
    FieldstgListedBattle *event = fieldstg_stage.battle_lists->scripted->events[5];

    records_state.stage = event->stage;
    records_state.music = event->music;
    fieldstg_start_battle(event->battle);
    gamestate_flags.set_flag(0xF, 1);
    return 0;
}

void fieldstg_start_indexed_event(s32 i) {
    void **data = heap_objects.find(7, -1, -1)->children;

    data[3] = fieldstg_event_start(fieldstg_indexed_events[i]);
}

void fieldstg_open_inn(void) {
    FieldstgManager *obj = (FieldstgManager *)heap_objects.find(7, -1, -1);
    void **data = obj->base.children;

    fieldstg_stage.menu_open = 1;
    fieldstg_stage.event_running = 1;
    data[4] = inn_create(0x1002);
    obj->base.set_step(obj, 2);
}

void fieldstg_start_warp(s32 type, FieldstgPos *pos, u16 *params) {
    FieldstgManager *obj = (FieldstgManager *)heap_objects.find(7, -1, -1);

    obj->warp_type = type;
    obj->warp_x = pos->x;
    obj->warp_y = pos->y;
    obj->warp_params = params;
    obj->base.set_step(obj, 3);
}

s32 fieldstg_sin_scale(s32 angle, s32 radius) {
    return rsin(angle >> 2) * radius / 4096;
}

void fieldstg_launch_update(FieldstgLaunch *obj) {
    FieldstgPos pos;
    FieldstgPos best_pos;
    Object *other;
    Object *best;
    s32 best_dist;
    s32 dx;
    s32 dy;
    s32 dist;
    s32 t;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        best_dist = 0x8000;
        best = NULL;
        pos.x = obj->actor->pixel_pos.x;
        pos.y = obj->actor->pixel_pos.y;
        for (other = heap_objects.find(0x17, -1, -1); other != NULL; other = heap_objects.find_next()) {
            dx = pos.x - other->key1;
            if (dx < 0) {
                dx = -dx;
            }
            dy = pos.y - other->key2;
            if (dy < 0) {
                dy = -dy;
            }
            dist = dx + dy * 2;
            if (dist < best_dist) {
                best = other;
                best_pos.x = other->key1;
                best_dist = dist;
                best_pos.y = best->key2;
            }
        }
        if (best == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        obj->pad = best;
        obj->start_x = best_pos.x + 20;
        obj->start_y = best_pos.y + 13;
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            switch (obj->base.substep) {
            case 0:
                obj->actor->set_walk_target(obj->actor, obj->start_x, obj->start_y, 0);
                fieldstg_partners_hide();
                obj->base.next_substep(obj);
            case 1:
                if (obj->actor->is_walking(obj->actor) == 0) {
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                obj->pad->set_state(obj->pad, OBJECT_STATE_DONE);
                sound_module.play(0x4001D);
                obj->base.next_substep(obj);
            case 1:
                break;
            }
            if (obj->pad->state != OBJECT_STATE_DONE) {
                obj->base.next_step(obj);
            }
            break;
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                obj->neg_x = 0;
                obj->dist_x = obj->params[1] - obj->start_x;
                if (obj->dist_x < 0) {
                    obj->dist_x = -obj->dist_x;
                    obj->neg_x = 1;
                }
                obj->neg_y = 0;
                obj->dist_y = obj->params[2] - obj->start_y;
                if (obj->dist_y < 0) {
                    obj->dist_y = -obj->dist_y;
                    obj->neg_y = 1;
                }
                obj->base.next_substep(obj);
            case 1:
                break;
            }
            obj->base.timer += gfx_module.funcs.get_frame_ticks() * 24;
            if (obj->base.timer > 0x1000) {
                obj->base.timer = 0x1000;
                obj->actor->pixel_pos.x = obj->params[1];
                obj->actor->pos.x = obj->actor->pixel_pos.x << 8;
                obj->actor->pixel_pos.y = obj->params[2];
                obj->actor->pos.y = obj->actor->pixel_pos.y << 8;
                obj->base.next_step(obj);
                break;
            }
            t = fieldstg_sin_scale(obj->base.timer, obj->dist_x);
            if (obj->neg_x != 0) {
                obj->actor->pixel_pos.x = obj->start_x - t;
            } else {
                obj->actor->pixel_pos.x = obj->start_x + t;
            }
            obj->actor->pos.x = obj->actor->pixel_pos.x << 8;
            t = fieldstg_sin_scale(obj->base.timer, obj->dist_y);
            if (obj->neg_y != 0) {
                obj->actor->pixel_pos.y = obj->start_y - t;
            } else {
                obj->actor->pixel_pos.y = obj->start_y + t;
            }
            obj->actor->pos.y = obj->actor->pixel_pos.y << 8;
            obj->actor->dir = (gfx_module.funcs.get_time() >> 1) & 7;
            obj->actor->has_shadow = 0;
            break;
        default:
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        obj->actor->dir = 0;
        obj->actor->has_shadow = 1;
        fieldstg_stage.event_running = 0;
        obj->actor->reset_control(obj->actor);
        fieldstg_partners_follow();
        break;
    }
}

FieldstgLaunch *fieldstg_launch_create(FieldstgActor *arg0, s16 *arg1) {
    FieldstgLaunch *obj = object_new(fieldstg_launch_update, sizeof(FieldstgLaunch), 0);

    obj->actor = arg0;
    obj->params = arg1;
    if (gamestate_data.funcs.get_map() == 0x26C) {
        LATE_CALL(func_800A6024)();
    }
    if (gamestate_data.funcs.get_map() == 0x2D4) {
        LATE_CALL(func_800A6024)();
    }
    return obj;
}

void fieldstg_spots_hint_update(FieldstgSpotsHint *obj) {
    Sprite sprite;
    s32 *bank;
    s32 dx;
    s32 dy;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        dx = obj->from.x - obj->to.x;
        if (dx < 0) {
            dx = -dx;
        }
        dy = obj->from.y - obj->to.y;
        if (dy < 0) {
            dy = -dy;
        }
        if (dx < 0x80 && dy < 0x80) {
            obj->blink = 3;
            obj->sprite = 0x52;
        } else if (dx < 0x100 && dy < 0x100) {
            obj->blink = 6;
            obj->sprite = 0x51;
        } else {
            obj->blink = 12;
            obj->sprite = 0x50;
        }
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        bank = cdload_module.get_subfile_by_id(0x1600001);
        sprite_init(&sprite);
        sprite.set_layer_id(0x1002, 0);
        sprite.set_vram_pos(0x240, 0x100);
        sprite.set_follow_scroll(0);
        sprite.set_palette(obj->time / obj->blink % 10);
        sprite.draw(bank, obj->sprite, 0xF8, 0xA8);
        sprite.draw(bank, 0x4F, 0xF8, 0xA8);
        obj->time += gfx_module.funcs.get_frame_ticks();
        if (obj->time >= 120) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

FieldstgSpotsHint *fieldstg_spots_hint_create(FieldstgPos from, FieldstgPos to) {
    FieldstgSpotsHint *obj = object_new(fieldstg_spots_hint_update, sizeof(FieldstgSpotsHint), 0);

    obj->from = from;
    obj->to = to;
    return obj;
}

void fieldstg_spots_pick_target(FieldstgSpots *obj) {
    s32 i = pad_random.next() % obj->count;

    gamestate_data.spot_target = i;
    obj->spots[i].target = 1;
    obj->target_pos = obj->spots[i].pos;
}

void fieldstg_spots_update(FieldstgSpots *obj, FieldstgSpotsData *data) {
    FieldstgSprite *sprites;
    s32 frame;
    s32 time;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        cdload_module.queue_file(0x3CB);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            data->effect = fieldstg_spots_effect_create(obj->scripted);
            obj->base.next_step(obj);
        } else if (obj->base.step != 0x80) {
            if (obj->base.step < 0x14) {
                obj->base.step = obj->base.step + gfx_module.funcs.get_frame_ticks() + 1;
            } else {
                if (obj->scripted == 0) {
                    if (obj->spots[obj->selected].target != 0) {
                        if ((pad_random.next() & 0x7F) < 0x66) {
                            fieldstg_start_listed_battle_func(3);
                        } else {
                            fieldstg_start_listed_battle_func(6);
                        }
                        fieldstg_spots_pick_target(obj);
                    } else {
                        if (data->hint != NULL) {
                            data->hint->base.destroy(data->hint);
                        }
                        data->hint = fieldstg_spots_hint_create(obj->target_pos, obj->spots[obj->selected].pos);
                    }
                }
                obj->base.set_step(obj, 0x80);
            }
        }
        frame = obj->base.substep;
        time = obj->base.timer;
        time += gfx_module.funcs.get_frame_ticks();
        if (fieldstg_spots_open_anim[frame].time < time) {
            time -= fieldstg_spots_open_anim[frame].time;
            frame++;
            if (fieldstg_spots_open_anim[frame].sprite == 0xFF) {
                obj->base.set_state(obj, OBJECT_STATE_RUN);
                break;
            }
            obj->spots[obj->selected].frame = fieldstg_spots_open_anim[frame].sprite;
            obj->base.substep = frame;
        }
        obj->base.timer = time;
        sprites = fieldstg_stage.sprites;
        for (i = 0; i < obj->count; i++) {
            if (obj->spots[i].frame != 0) {
                if (obj->selected == i) {
                    sprites[obj->spots[i].sprite].sprite = obj->spots[i].frame;
                } else {
                    sprites[obj->spots[i].sprite].sprite = 0x38;
                }
            }
        }
        break;
    case OBJECT_STATE_END:
        if (obj->spots != NULL) {
            heap_funcs.free(obj->spots);
        }
        break;
    }
}

FieldstgSpots *fieldstg_spots_create(s32 count) {
    FieldstgSpots *obj = object_create(fieldstg_spots_update, sizeof(FieldstgSpots), sizeof(FieldstgSpotsData), 0xB);
    FieldstgSprite *sprite;
    s32 i;
    s32 n;
    s32 sel;

    obj->count = count;
    sprite = fieldstg_stage.sprites;
    obj->spots = heap_funcs.alloc(count * sizeof(FieldstgSpot), 2);
    for (i = 0, n = 0; sprite->y != 0; i++, sprite++) {
        if (sprite->type == 0xFF) {
            obj->spots[n].pos.x = sprite->x;
            obj->spots[n].pos.y = sprite->y;
            obj->spots[n].sprite = i;
            obj->spots[n].frame = 0x38;
            obj->spots[n].target = 0;
            n++;
        }
    }
    if (n != 0) {
        if (gamestate_data.map_is_new != 0) {
            fieldstg_spots_pick_target(obj);
        } else {
            sel = gamestate_data.spot_target;
            obj->spots[sel].target = 1;
            obj->target_pos = obj->spots[sel].pos;
        }
    }
    return obj;
}

FieldstgSpots *fieldstg_spots_find(s32 *pos, s32 select) {
    FieldstgSpots *obj = (FieldstgSpots *)heap_objects.find(0xB, -1, -1);
    FieldstgSpot *entry;
    s32 i;

    if (obj != NULL) {
        for (i = 0; i < obj->count; i++) {
            entry = &obj->spots[i];
            if (pos[0] >= entry->pos.x - 10 && entry->pos.x + 10 >= pos[0]
                && pos[1] >= entry->pos.y - 10 && entry->pos.y + 10 >= pos[1]) {
                if (select) {
                    obj->selected = i;
                    obj->scripted = 0;
                }
                return obj;
            }
        }
    }
    return NULL;
}

void fieldstg_spots_open_scripted(void) {
    FieldstgSpots *obj = (FieldstgSpots *)heap_objects.find(0xB, -1, -1);
    s32 i;

    if (obj != NULL) {
        for (i = 0; i < obj->count; i++) {
            if (obj->spots[i].pos.x >= 1000) {
                obj->selected = i;
                obj->scripted = 1;
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
    }
}

void fieldstg_spots_effect_draw(FieldstgSpotsEffect *obj, GfxLayer *layer) {
    Sprite sprite;

    if (obj->base.state == OBJECT_STATE_RUN) {
        sprite_init(&sprite);
        sprite.set_vram_pos(0x200, 0x100);
        sprite.set_layer(layer, 4);
        sprite.draw(cdload_module.get_subfile_by_id(0x1600000), obj->sprite, obj->x, obj->y);
    }
}

void fieldstg_spots_effect_update(FieldstgSpotsEffect *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    FieldstgActor *actor;
    s32 frame;
    s32 time;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        if (obj->base.key1 != 0) {
            actor = (FieldstgActor *)heap_objects.find(5, 0x11B, -1);
        } else {
            actor = (FieldstgActor *)heap_objects.find(5, -1, 0);
        }
        if (actor == NULL) {
            break;
        }
        obj->x = actor->pixel_pos.x;
        obj->y = actor->pixel_pos.y;
        obj->dir = actor->dir;
        obj->anim = fieldstg_spots_effect_anims[obj->dir];
        obj->base.next_state(obj);
        sound_module.play(0x80045C44);
    case OBJECT_STATE_RUN:
        frame = obj->base.substep;
        time = obj->base.timer;
        time += gfx_module.funcs.get_frame_ticks();
        if (obj->anim[frame].time < time) {
            time -= obj->anim[frame].time;
            frame++;
            if (obj->anim[frame].sprite == 0xFF) {
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            obj->sprite = obj->anim[frame].sprite;
            obj->base.substep = frame;
        }
        obj->base.timer = time;
        if (obj->sprite != 0) {
            layer->add_callback(layer, (void (*)(void *, GfxLayer *, s32))fieldstg_spots_effect_draw, obj, obj->y + fieldstg_spots_effect_depths[obj->dir], 0);
        }
        break;
    }
}

FieldstgSpotsEffect *fieldstg_spots_effect_create(s32 arg0) {
    FieldstgSpotsEffect *obj = object_new(fieldstg_spots_effect_update, sizeof(FieldstgSpotsEffect), 0);

    obj->base.key1 = arg0;
    return obj;
}

void fieldstg_meter_update(FieldstgMeter *obj) {
    Sprite sprite;
    Sprite meter;
    s32 *bank;
    s32 *bank2;
    u8 cell;
    s32 x;
    s32 pos;

    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.meter_random_count > 0) {
            obj->pattern = pad_random.next() & 7;
            gamestate_data.meter_random_count--;
        } else {
            obj->pattern = 8;
        }
        obj->speed = 0x100;
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep > 90) {
                obj->base.next_step(obj);
            }
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                if (pad_state.get_pressed(0) & 0x2000) {
                    if (!(pad_random.next() & 3)) {
                        obj->base.set_substep(obj, 2);
                    } else {
                        obj->base.set_substep(obj, 1);
                    }
                    sound_module.play(0x4001B);
                }
                break;
            case 1:
                obj->speed -= 0x10;
                if (obj->speed == 0) {
                    obj->base.set_substep(obj, 3);
                }
                break;
            case 2:
                obj->speed -= 4;
                if (obj->speed == 0) {
                    obj->base.set_substep(obj, 3);
                }
                break;
            case 3:
                pos = obj->cursor;
                x = pos >> 10;
                cell = fieldstg_meter_patterns[obj->pattern][x];
                pos >>= 7;
                cell = (cell >> (pos & 6)) & 3;
                if (obj->base.timer < 60) {
                    if (obj->base.timer == 0 && cell == 1) {
                        sound_module.play(0x80045341);
                    }
                    obj->base.timer += gfx_module.funcs.get_frame_ticks();
                } else {
                    switch (cell) {
                    case 0:
                    default:
                        obj->base.set_state(obj, OBJECT_STATE_END);
                        break;
                    case 1:
                        fieldstg_start_listed_battle_func(4, x);
                        obj->base.set_state(obj, OBJECT_STATE_DONE);
                        break;
                    case 2:
                        fieldstg_start_listed_battle_func(7, x);
                        obj->base.set_state(obj, OBJECT_STATE_DONE);
                        break;
                    }
                }
                break;
            }
            if (obj->back != 0) {
                obj->cursor -= obj->speed;
                if (obj->cursor <= 0) {
                    obj->cursor = 0;
                    obj->back = 0;
                }
            } else {
                obj->cursor += obj->speed;
                if (obj->cursor >= 0x3000) {
                    obj->cursor = 0x3000;
                    obj->back = 1;
                }
            }
            bank = cdload_module.get_subfile_by_id(0x1600000);
            sprite_init(&sprite);
            sprite.set_layer_id(0x1002, 4);
            sprite.set_vram_pos(0x200, 0x100);
            sprite.draw(bank, gfx_module.funcs.get_time() % 48 / 12 + 0x60, obj->pos.x, obj->pos.y);
            bank2 = cdload_module.get_subfile_by_id(0x1600001);
            sprite_init(&meter);
            meter.set_layer_id(0x1002, 0);
            meter.set_vram_pos(0x240, 0x100);
            meter.set_follow_scroll(0);
            meter.draw(bank2, 0x3D, (obj->cursor >> 8) + 0x18, 0xC0);
            meter.draw(bank2, obj->pattern + 0x3E, 0x18, 0xC0);
            meter.draw(bank2, 0x3C, 0x18, 0xC0);
            break;
        }
        break;
    }
}

FieldstgMeter *fieldstg_meter_create(FieldstgPos pos) {
    FieldstgMeter *obj = object_new(fieldstg_meter_update, sizeof(FieldstgMeter), 0);

    obj->pos = pos;
    return obj;
}

void fieldstg_camera_apply(FieldstgCamera *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 x = obj->center.x - 160;
    s32 y = obj->center.y - 140;
    FieldstgBackgroundView *map;
    s32 shake;

    if (obj->has_map_size == 0) {
        map = (FieldstgBackgroundView *)heap_objects.find(4, -1, -1);
        if (map != NULL) {
            if (map->base.state == OBJECT_STATE_RUN) {
                obj->map_size = *map->get_size(map);
                obj->has_map_size = 1;
            }
        } else if (gamestate_data.funcs.get_map() != 0x2DE) {
            obj->map_size.x = 0x7FFF;
            obj->map_size.y = 0x7FFF;
        } else {
            obj->map_size.x = 0x500;
            obj->map_size.y = 0x400;
        }
    }
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x > obj->map_size.x - 320) {
        x = obj->map_size.x - 320;
    }
    if (y > obj->map_size.y - 240) {
        y = obj->map_size.y - 240;
    }
    shake = 0;
    if (obj->shake != 0) {
        obj->shake_frame = (obj->shake_frame + 1) & 3;
        shake = obj->shake_frame + 1;
        if (obj->voice == -1) {
            obj->voice = sound_module.play(0xA00431BF);
        }
    } else if (obj->voice != -1) {
        sound_module.key_off(0xA00431BF, obj->voice);
        obj->voice = -1;
    }
    layer->set_scroll(layer, (fieldstg_camera_shake_offsets[shake].x + x) << 8, (fieldstg_camera_shake_offsets[shake].y + y) << 8);
}

void fieldstg_camera_update(FieldstgCamera *obj) {
    FieldstgPos dist;
    FieldstgPos sign;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->actor = (FieldstgActor *)heap_objects.find(5, -1, 0);
        obj->snap = 1;
        if (obj->actor != NULL) {
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            obj->center.x = obj->actor->pixel_pos.x;
            obj->center.y = obj->actor->pixel_pos.y - (obj->actor->height >> 8);
            if ((obj->base.substep == 0) & (obj->snap == 0)) {
                obj->base.next_substep(obj);
            }
            break;
        case 1:
            obj->center.x = obj->target_x;
            obj->center.y = obj->target_y;
            if ((obj->base.substep == 0) & (obj->snap == 0)) {
                obj->base.next_substep(obj);
            }
            break;
        }
        if (obj->base.substep == 1) {
            sign.x = 1;
            sign.y = 1;
            dist.x = obj->center.x - obj->pos.x;
            if (dist.x < 0) {
                sign.x = -1;
                dist.x = -dist.x;
            }
            dist.y = obj->center.y - obj->pos.y;
            if (dist.y < 0) {
                sign.y = -1;
                dist.y = -dist.y;
            }
            if (dist.x != 0 && dist.y != 0) {
                if (dist.x > 4) {
                    dist.x /= 4;
                } else if (dist.x > 2) {
                    dist.x /= 2;
                } else {
                    dist.x = 1;
                }
                obj->center.x = obj->pos.x += dist.x * sign.x;
                if (dist.y > 4) {
                    dist.y /= 4;
                } else if (dist.y > 2) {
                    dist.y /= 2;
                } else {
                    dist.y = 1;
                }
                obj->center.y = obj->pos.y += dist.y * sign.y;
            } else {
                obj->base.next_substep(obj);
            }
        }
        fieldstg_camera_apply(obj);
        break;
    case OBJECT_STATE_END:
        if (obj->voice != -1) {
            sound_module.key_off(0xA00431BF, obj->voice);
            obj->voice = -1;
        }
        break;
    }
}

FieldstgCamera *fieldstg_camera_create(void) {
    FieldstgCamera *obj = object_create(fieldstg_camera_update, sizeof(FieldstgCamera), 0, 0x10);

    obj->voice = -1;
    return obj;
}

void fieldstg_camera_follow(s32 snap, s32 id) {
    FieldstgCamera *obj = (FieldstgCamera *)heap_objects.find(0x10, -1, -1);

    if (obj != NULL) {
        obj->unk_7C = 0;
        obj->snap = snap;
        obj->actor_id = id;
        obj->actor = (FieldstgActor *)heap_objects.find(5, id, -1);
        obj->pos = obj->center;
        obj->base.set_step(obj, 0);
    }
}

void fieldstg_camera_move_to(s32 snap, s32 x, s32 y) {
    FieldstgCamera *obj = (FieldstgCamera *)heap_objects.find(0x10, -1, -1);

    if (obj != NULL) {
        obj->unk_7C = 0;
        obj->snap = snap;
        obj->target_x = x;
        obj->target_y = y;
        obj->pos = obj->center;
        obj->base.set_step(obj, 1);
    }
}

void fieldstg_camera_set_shake(s32 on) {
    FieldstgCamera *obj = (FieldstgCamera *)heap_objects.find(0x10, -1, -1);

    if (obj != NULL) {
        obj->shake = on;
    }
}

s32 fieldstg_actor_collide_point(FieldstgActor *obj, s32 x, s32 y, FieldstgPos d) {
    FieldstgPos pos;
    u8 attr;
    s32 ret = 0;

    pos.x = (obj->pos.x >> 8) + x;
    pos.y = (obj->pos.y >> 8) + y;
    attr = fieldstg_attr.is_free(&pos);
    if (attr) {
        attr = fieldstg_attr.get(gamestate_data.attr_layer, &pos);
    }
    if (obj->height != 0 && attr != 1) {
        switch (attr) {
        case 2:
            if (obj->height < 0x2000) {
                attr = 0;
            }
            break;
        case 3:
            if (obj->height < 0x3000) {
                attr = 0;
            }
            break;
        case 4:
            if (obj->height < 0x4000) {
                attr = 0;
            }
            break;
        case 5:
            if (obj->height < 0x5000) {
                attr = 0;
            }
            break;
        case 6:
            if (obj->height < 0x6000) {
                attr = 0;
            }
            break;
        }
        switch (attr) {
        case 18:
            if (obj->height > 0x6000) {
                attr = 0;
            }
            break;
        case 19:
            if (obj->height > 0x5000) {
                attr = 0;
            }
            break;
        case 20:
            if (obj->height > 0x4000) {
                attr = 0;
            }
            break;
        case 21:
            if (obj->height > 0x3000) {
                attr = 0;
            }
            break;
        case 22:
            if (obj->height > 0x2000) {
                attr = 0;
            }
            break;
        }
        if (attr == 0) {
            ret = 1;
        }
    }
    if (attr == 0) {
        obj->pos.x -= d.x;
        obj->pos.y -= d.y;
    }
    return ret;
}

s32 fieldstg_actor_collide(FieldstgActor *obj) {
    s32 ret = 0;
    s32 i;
    u8 k;
    u8 *dir;
    FieldstgPos d;

    for (i = 0; i < 5; i++) {
        k = fieldstg_collision_probes[obj->dir][i];
        dir = fieldstg_collision_probe_push[k];
        d.x = (dir[0] & 1) * obj->speed / 2;
        if (dir[0] & 0x80) {
            d.x = -d.x;
        }
        d.y = (dir[1] & 1) * obj->speed / 4;
        if (dir[1] & 0x80) {
            d.y = -d.y;
        }
        if (fieldstg_actor_collide_point(obj, fieldstg_collision_probe_pos[k].x, fieldstg_collision_probe_pos[k].y, d)) {
            ret = 1;
        }
    }
    return ret;
}

void fieldstg_actor_set_move(FieldstgActor *obj, s32 pad_dir) {
    if (pad_dir != 0 && fieldstg_stage.menu_open == 0) {
        obj->dir = fieldstg_pad_dirs[pad_dir];
        if (obj->walk != 0) {
            if (obj->base.step != 2) {
                obj->base.set_step(obj, 2);
            }
        } else if (obj->base.step != 3) {
            obj->base.set_step(obj, 3);
        }
    } else {
        if (obj->base.step == 2) {
            obj->base.set_step(obj, 1);
        }
        if (obj->base.step == 3) {
            obj->base.set_step(obj, 4);
        }
    }
}

FieldstgActor *fieldstg_actor_find_at(FieldstgPos *pos) {
    HeapObjects *list = &heap_objects;
    FieldstgActor *obj;
    s32 w;

    for (obj = (FieldstgActor *)list->find(5, -1, 1); obj != NULL; obj = (FieldstgActor *)list->find_next()) {
        w = obj->width;
        if (pos->x >= obj->pixel_pos.x - w && obj->pixel_pos.x + w >= pos->x) {
            w >>= 1;
            if (pos->y >= obj->pixel_pos.y - w && obj->pixel_pos.y + w >= pos->y) {
                return obj;
            }
        }
    }
    return NULL;
}

s32 fieldstg_actor_talk_to(FieldstgActor *obj, FieldstgPos *pos) {
    FieldstgActor *other = fieldstg_actor_find_at(pos);
    s32 ret = 0;
    s32 state;
    s32 i;

    if (other != NULL && other->base.state == OBJECT_STATE_RUN && other->base.key1 != 0x180 && other->base.key1 != 0x181
        && other->base.key1 != 0x182) {
        for (i = 0; fieldstg_talk_redirects[i][0] != 0; i++) {
            if (other->base.key1 == fieldstg_talk_redirects[i][0]) {
                other = (FieldstgActor *)heap_objects.find(5, fieldstg_talk_redirects[i][1], -1);
                break;
            }
        }
        switch (other->base.key1) {
        case 0x148:
        case 0x15F:
        case 0x160:
            if (other->base.step != 0x4E) {
                other->base.set_step(other, 0x4E);
                ret = 1;
                other->talker = obj;
                obj->base.set_step(obj, 0x4D);
                obj->control = NULL;
            }
            break;
        default:
            other->base.set_step(other, 0x4A);
            other->talker = obj;
            state = 1;
            if (obj->height_control != 0) {
                state = 0x4C;
            }
            obj->base.set_step(obj, state);
            obj->control = NULL;
            fieldstg_stage.actor_busy = 1;
            ret = 1;
            break;
        }
    }
    return ret;
}

/* held is first the held buttons (copied after the second pad call), later the landing height. */
void fieldstg_player_control_height(FieldstgActor *obj) {
    FieldstgPos front;
    FieldstgPos pos;
    s32 buttons = pad_state.get_held(0);
    s32 pressed = (pad_state.get_pressed(0) & 0x2000) != 0;
    s32 held = buttons;
    s32 hit;
    u8 attr;

    if (fieldstg_stage.menu_open == 0 && fieldstg_stage.event_running == 0 && fieldstg_stage.battle_starting == 0) {
        if (pressed && obj->height < 0x2000) {
            obj->get_front(obj, &front);
            if (fieldstg_actor_talk_to(obj, &front)) {
                obj->vertical_speed = 0;
                obj->speed = 0;
                if (obj->voice != -1) {
                    sound_module.key_off(0xA0045F4A, obj->voice);
                    obj->voice = -1;
                }
                return;
            }
        }
        if (held & 0x4000) {
            if (obj->base.step != 0x4B) {
                obj->base.set_step(obj, 0x4B);
            }
            if (obj->voice == -1) {
                obj->voice = sound_module.play(0xA0045F4A);
            }
        } else {
            if (obj->base.step == 0x4B) {
                obj->base.set_step(obj, 0x4C);
            }
            if (obj->voice != -1) {
                sound_module.key_off(0xA0045F4A, obj->voice);
                obj->voice = -1;
            }
        }
        if ((gfx_module.funcs.get_frames() & 7) == 0) {
            if (held & 0x20) {
                obj->reload = 1;
                obj->dir = (obj->dir + 1) & 7;
            } else if (held & 0x80) {
                obj->reload = 1;
                obj->dir = (obj->dir - 1) & 7;
            }
        }
        if (obj->base.step == 0x4B) {
            if (obj->vertical_speed > -0xC0) {
                obj->vertical_speed -= 4;
            } else {
                obj->vertical_speed = -0xC0;
            }
        } else {
            if (obj->vertical_speed < 0x80) {
                obj->vertical_speed += 4;
            } else {
                obj->vertical_speed = 0x80;
            }
        }
        hit = 0;
        pos.x = obj->pos.x >> 8;
        pos.y = obj->pos.y >> 8;
        attr = fieldstg_attr.get(gamestate_data.attr_layer, &pos);
        obj->height += obj->vertical_speed;
        held = 0;
        if (obj->vertical_speed > 4) {
            switch (attr) {
            case 18:
                if (obj->height > 0x6000) {
                    hit = 1;
                }
                break;
            case 19:
                if (obj->height > 0x5000) {
                    hit = 1;
                }
                break;
            case 20:
                if (obj->height > 0x4000) {
                    hit = 1;
                }
                break;
            case 21:
                if (obj->height > 0x3000) {
                    hit = 1;
                }
                break;
            case 22:
                if (obj->height > 0x2000) {
                    hit = 1;
                }
                break;
            }
        } else {
            switch (attr) {
            case 2:
                if (obj->height < 0x2000) {
                    hit = 1;
                    held = 0x200C;
                }
                break;
            case 3:
                if (obj->height < 0x3000) {
                    hit = 1;
                    held = 0x300C;
                }
                break;
            case 4:
                if (obj->height < 0x4000) {
                    hit = 1;
                    held = 0x400C;
                }
                break;
            case 5:
                if (obj->height < 0x5000) {
                    hit = 1;
                    held = 0x500C;
                }
                break;
            case 6:
                if (obj->height < 0x6000) {
                    hit = 1;
                    held = 0x600C;
                }
                break;
            }
        }
        if (hit || obj->height > 0x7000 || obj->height < 0x1800) {
            if (held != 0) {
                obj->height = held;
            }
            obj->vertical_speed = 0;
        }
        if (obj->height >= 0x7000) {
            obj->height = 0x7000;
        }
        if (obj->height <= 0x1800) {
            obj->height = 0x1800;
        }
    }
}

void fieldstg_player_control(FieldstgActor *obj) {
    s32 dir;
    s32 pressed;
    s32 flag;
    FieldstgPos pos;
    FieldstgSpots *sel;

    if (fieldstg_stage.menu_open == 0 && fieldstg_stage.event_running == 0 && fieldstg_stage.battle_starting == 0) {
        dir = (pad_state.get_held(0) >> 4) & 0xF;
        pressed = (pad_state.get_pressed(0) & 0x2000) != 0;
        flag = gamestate_flags.get_flag(0x12, 1);
        if (pressed || flag) {
            obj->get_front(obj, &pos);
            /* Evidence (class B, register priority only; DECISIONS "LOOP_BLOCK audit"): the block's loop-weighted
             * references give the original's saved registers (with scheduling off only registers differ). */
            LOOP_BLOCK(
                if (gamestate_flags.get_flag(0x8004, 1) == 0 || flag || (sel = fieldstg_spots_find(&pos.x, 1)) == NULL) {
                    if (fieldstg_actor_talk_to(obj, &pos) && flag) {
                        gamestate_flags.set_flag(0x12, 0);
                    }
                } else {
                    fieldstg_stage.actor_busy = 1;
                    obj->base.set_step(obj, 0x49);
                    sel->base.set_state(sel, OBJECT_STATE_DONE);
                }
            );
        } else {
            fieldstg_actor_set_move(obj, dir);
        }
    }
}

void fieldstg_actor_control_ladder(FieldstgActor *obj) {
    s32 buttons = pad_state.get_held(0);

    if (buttons & 0x10) {
        if (obj->base.step != 0x41) {
            obj->base.set_step(obj, 0x41);
        }
    } else if (buttons & 0x40) {
        if (obj->base.step != 0x42) {
            obj->base.set_step(obj, 0x42);
        }
    } else if (obj->base.step != 0x40) {
        obj->base.set_step(obj, 0x40);
    }
}

void fieldstg_actor_follow(FieldstgActor *obj) {
    FieldstgTrail *trail;
    FieldstgActor *leader;

    if (obj->trail->leader == NULL) {
        obj->trail->leader = (FieldstgActor *)heap_objects.find(5, -1, 0);
    }
    leader = obj->trail->leader;
    if (leader != NULL) {
        trail = obj->trail;
        switch (leader->base.step) {
        case 2:
        case 3:
        case 5:
        case 0x4F:
        case 0x50:
            trail->positions[trail->write].x = leader->pos.x;
            trail->positions[trail->write].y = leader->pos.y;
            trail->positions[trail->write].dir = leader->dir;
            trail->write = (trail->write + 1) & 0x3F;
            obj->pos.x = trail->positions[trail->read].x;
            obj->pos.y = trail->positions[trail->read].y;
            obj->dir = trail->positions[trail->read].dir;
            trail->read = (trail->read + 1) & 0x3F;
            break;
        }
        switch (leader->base.step) {
        case 2:
        case 3:
        case 5:
            if (obj->base.step != 3) {
                obj->base.set_step(obj, 3);
            }
            break;
        case 0x4F:
            if (obj->base.step != 1) {
                obj->base.set_step(obj, 1);
            }
            break;
        default:
            if (obj->base.step == 3) {
                obj->base.set_step(obj, 4);
            }
            break;
        }
        obj->depth = leader->depth;
    }
}

void fieldstg_actor_follow_hide(FieldstgActor *obj) {
    FieldstgTrail *trail;
    FieldstgActor *leader;
    s32 i;

    if (obj->trail->leader == NULL) {
        obj->trail->leader = (FieldstgActor *)heap_objects.find(5, -1, 0);
    }
    leader = obj->trail->leader;
    trail = obj->trail;
    if (leader != NULL) {
        for (i = 0; i < 2; i++) {
            trail->positions[trail->write].x = 0;
            trail->positions[trail->write].y = 0;
            trail->positions[trail->write].dir = 0;
            trail->write = (trail->write + 1) & 0x3F;
            obj->pos.x = trail->positions[trail->read].x;
            obj->pos.y = trail->positions[trail->read].y;
            obj->dir = trail->positions[trail->read].dir;
            trail->read = (trail->read + 1) & 0x3F;
        }
        if (obj->base.step != 3) {
            obj->base.set_step(obj, 3);
        }
        if (obj->pos.x == 0) {
            for (i = 0; i < 64; i++) {
                trail->positions[i].dir = 0;
                trail->positions[i].x = 0;
                trail->positions[i].y = 0;
            }
            obj->control = NULL;
        }
    }
}

void fieldstg_actor_walk_to_target(FieldstgActor *obj) {
    s32 x, y, tx, ty, buttons;

    if (obj->walking != 0) {
        x = obj->pos.x >> 8;
        y = obj->pos.y >> 8;
        tx = obj->target_x;
        ty = obj->target_y;
        if (x >> 1 != tx >> 1 || y >> 1 != ty >> 1) {
            buttons = 0;
            if (x < tx) {
                buttons = 0x20;
            } else if (tx < x) {
                buttons = 0x80;
            }
            if (y < ty) {
                buttons |= 0x40;
            } else if (ty < y) {
                buttons |= 0x10;
            }
            obj->walk = 1;
            fieldstg_actor_set_move(obj, buttons >> 4);
        } else {
            obj->pos.x = obj->target_x << 8;
            obj->pos.y = obj->target_y << 8;
            obj->walking = 0;
            obj->dir = obj->target_dir;
            obj->base.set_step(obj, 1);
        }
    }
}

void fieldstg_actor_set_walk_target(FieldstgActor *obj, s32 x, s32 y, s32 dir) {
    obj->walking = 1;
    obj->target_x = x;
    obj->target_y = y;
    obj->target_dir = dir;
}

s32 fieldstg_actor_is_walking(FieldstgActor *obj) {
    return obj->walking;
}

void fieldstg_actor_start_walk_to(FieldstgActor *obj) {
    obj->control = fieldstg_actor_walk_to_target;
}

void fieldstg_actor_reset_control(FieldstgActor *obj) {
    switch (obj->base.key2) {
    case 0:
        obj->control = fieldstg_player_control;
        obj->walk = 0;
        break;
    case 1:
        obj->control = NULL;
        break;
    case 2:
    case 4:
    case 8:
        obj->control = fieldstg_actor_follow;
        break;
    }
}

void fieldstg_actor_walk_out(FieldstgActor *obj, s32 dir) {
    obj->control = NULL;
    obj->base.set_step(obj, 5);
    obj->dir = dir;
}

void fieldstg_actor_start_carry(FieldstgActor *obj, s32 dir) {
    if (obj->base.step != 0x4F) {
        obj->control = NULL;
        obj->base.set_step(obj, 0x4F);
        obj->dir = dir;
    }
}

void fieldstg_actor_stop_carry(FieldstgActor *obj) {
    if (obj->base.step == 0x4F) {
        obj->base.set_step(obj, 0x50);
    }
}

void fieldstg_actor_climb_from_bottom(FieldstgActor *obj, s32 dir, s32 x, s32 y, s32 height) {
    obj->control = NULL;
    obj->base.set_step(obj, 0x43);
    obj->dir = dir;
    obj->pos.x = x << 8;
    obj->pos.y = y << 8;
    obj->climb_height = 0;
    obj->climb_top = height << 8;
    if (dir != 5) {
        obj->ladder_side = 1;
    } else {
        obj->ladder_side = 0;
    }
    fieldstg_partners_hide();
    fieldstg_stage.actor_busy = 1;
}

void fieldstg_actor_climb_from_top(FieldstgActor *obj, s32 dir, s32 x, s32 y, s32 height) {
    obj->control = NULL;
    obj->base.set_step(obj, 0x44);
    obj->dir = dir;
    obj->pos.x = x << 8;
    obj->pos.y = y << 8;
    obj->climb_height = obj->climb_top = height << 8;
    if (dir != 5) {
        obj->ladder_side = 1;
    } else {
        obj->ladder_side = 0;
    }
    obj->has_shadow = 0;
    fieldstg_partners_hide();
    fieldstg_stage.actor_busy = 1;
}

void fieldstg_actor_jump_down(FieldstgActor *obj, s32 dir, FieldstgPos pos, s32 height) {
    obj->control = NULL;
    obj->base.set_step(obj, 0x47);
    obj->dir = dir;
    if (dir != 7) {
        obj->ladder_side = 0;
    } else {
        obj->ladder_side = 1;
    }
    obj->climb_height = obj->climb_top = height << 8;
    obj->has_shadow = 0;
    fieldstg_partners_hide();
    fieldstg_stage.actor_busy = 1;
}

void fieldstg_actor_start_meter(FieldstgActor *obj, s32 dir, FieldstgPos offset) {
    FieldstgActor *other;
    FieldstgPos pos;
    void **data;
    s32 i;

    fieldstg_stage.event_running = 1;
    obj->control = NULL;
    obj->base.set_step(obj, 0x48);
    obj->dir = dir;
    for (i = 0; i < 3; i++) {
        other = (FieldstgActor *)heap_objects.find(5, -1, fieldstg_meter_partner_types[i]);
        if (other != NULL) {
            other->dir = dir;
        }
    }
    data = obj->base.children;
    pos.x = obj->pixel_pos.x + offset.x;
    pos.y = obj->pixel_pos.y + offset.y;
    data[2] = fieldstg_meter_create(pos);
}

void fieldstg_actor_warp(FieldstgActor *obj, u16 *params, s32 type) {
    obj->control = NULL;
    obj->base.set_step(obj, 1);
    obj->dir = 0;
    fieldstg_stage.event_running = 1;
    fieldstg_start_warp(type, &obj->pixel_pos, params);
}

void fieldstg_actor_launch(FieldstgActor *obj, s16 *arg1) {
    void **data;

    obj->control = fieldstg_actor_walk_to_target;
    obj->base.set_step(obj, 1);
    obj->dir = 0;
    fieldstg_stage.event_running = 1;
    data = obj->base.children;
    data[2] = fieldstg_launch_create(obj, arg1);
}

void fieldstg_actor_set_anim(FieldstgActor *obj, s32 anim) {
    obj->anim = anim;
    obj->anim_pos = 0;
    obj->anim_time = 0;
    obj->anim_done = 0;
}

void fieldstg_actor_play_anim(FieldstgActor *obj, s32 anim, s32 dir) {
    obj->walking = 0;
    obj->base.set_step(obj, 0);
    obj->dir = dir;
    fieldstg_actor_set_anim(obj, anim);
}

s32 fieldstg_actor_is_anim_done(FieldstgActor *obj) {
    return obj->anim_done;
}

/* A layer draw callback (gfx_add_callback): obj and layer come in as void * (the form the US decomp found). */
void fieldstg_actor_draw(void *arg0, void *arg1, s32 arg) {
    FieldstgActor *obj = arg0;
    GfxLayer *layer = arg1;
    FieldstgVramPlace *vram = obj->vram;
    FieldstgVramPlace *shadow;
    u32 *ot;
    FieldstgPos pos;
    s32 ofs[2];
    POLY_FT4 *poly;
    SPRT *sprt;
    DR_TPAGE *tpage;
    s32 dx;

    if (obj->base.state == OBJECT_STATE_RUN) {
        ot = layer->get_ot_entry(layer, obj->depth);
        pos = obj->pixel_pos;
        layer->get_scroll(layer, ofs);
        pos.x -= ofs[0];
        pos.y -= ofs[1];
        poly = gfx_module.funcs.get_packet();
        pos.y -= obj->height >> 8;
        setPolyFT4(poly);
        setRGB0(poly, 0x80, 0x80, 0x80);
        dx = obj->frame_x;
        if (obj->dir >= 5) {
            dx = -(obj->frame_w + dx);
        }
        poly->x0 = pos.x + dx;
        poly->x1 = obj->frame_w + (pos.x + dx);
        poly->x2 = pos.x + dx;
        poly->x3 = obj->frame_w + (pos.x + dx);
        poly->y0 = obj->frame_y + pos.y;
        poly->y1 = obj->frame_y + pos.y;
        poly->y2 = obj->frame_h + (obj->frame_y + pos.y);
        poly->y3 = obj->frame_h + (obj->frame_y + pos.y);
        if (obj->dir < 5) {
            poly->u0 = vram->u;
            poly->u1 = obj->frame_w + vram->u;
            poly->u2 = vram->u;
            poly->u3 = obj->frame_w + vram->u;
        } else {
            poly->x1--;
            poly->x3--;
            poly->u0 = obj->frame_w + vram->u - 1;
            poly->u1 = vram->u;
            poly->u2 = obj->frame_w + vram->u - 1;
            poly->u3 = vram->u;
        }
        poly->v0 = vram->v;
        poly->v1 = vram->v;
        poly->v2 = obj->frame_h + vram->v;
        poly->v3 = obj->frame_h + vram->v;
        poly->clut = getClut(vram->clut_x, vram->clut_y);
        poly->tpage = getTPage(0, 0, vram->tpage_x, vram->tpage_y);
        addPrim(ot, poly);
        pos.y += obj->height >> 8;
        poly++;
        if (obj->has_shadow != 0) {
            ot = layer->get_ot_entry(layer, obj->depth + 1);
            shadow = obj->shadow_vram;
            sprt = (SPRT *)poly;
            setSprt(sprt);
            setRGB0(sprt, 0x80, 0x80, 0x80);
            sprt->x0 = pos.x - 16;
            sprt->y0 = pos.y - 8 + (obj->climb_height >> 8);
            sprt->u0 = shadow->u;
            sprt->v0 = shadow->v;
            sprt->w = 32;
            sprt->h = 16;
            sprt->clut = getClut(shadow->clut_x, shadow->clut_y);
            addPrim(ot, sprt);
            poly = (POLY_FT4 *)(sprt + 1);
            tpage = (DR_TPAGE *)poly;
            SetDrawTPage(tpage, 0, 1, GetTPage(0, 0, shadow->tpage_x, shadow->tpage_y));
            addPrim(ot, tpage);
            poly = (POLY_FT4 *)(tpage + 1);
        }
        gfx_module.funcs.set_packet(poly);
    }
}

void fieldstg_actor_set_dir(FieldstgActor *obj, s32 dir) {
    obj->dir = dir;
}

void fieldstg_actor_update_sprite(FieldstgActor *obj) {
    Tim tim;
    s32 id = obj->anim;
    u32 file = obj->sprite_file & 0xFFFF0000;
    s16 *entry;
    s32 *script;
    FieldstgVramPlace *vram;
    s32 k;
    s32 i;

    if (obj->loaded_anim != id) {
        if (obj->base.key1 == 2) {
            /* Evidence (class C, block placement; DECISIONS "LOOP_BLOCK audit"): the original places the ==8, <0x16,
             * <0x1A and <0x25 bodies out of line, before their tests. */
            LOOP_BLOCK(
                if (id < 8) {
                    obj->sprite_file = 0x01870070;
                    break;
                }
                if (id == 8) {
                    obj->sprite_file = 0x03CB001E;
                    break;
                }
                if (id < 0x16) {
                    obj->sprite_file = 0x03CA0064;
                    break;
                }
                if (id < 0x1A) {
                    obj->sprite_file = 0x03C9000B;
                    break;
                }
                if (id < 0x25) {
                    obj->sprite_file = 0x03CC0017;
                    break;
                }
                obj->sprite_file = 0x03CA0064;
            );
            file = obj->sprite_file & 0xFFFF0000;
        }
        entry = cdload_module.get_subfile_by_id(obj->sprite_file);
        while (entry[0] != 0) {
            if (entry[0] == id) {
                break;
            }
            entry += 6;
        }
        if (entry[0] == 0) {
            entry = cdload_module.get_subfile_by_id(obj->sprite_file);
        }
        for (k = 0; k < 5; k++) {
            obj->anim_scripts[k] = entry[k + 1] | file;
        }
        obj->loaded_anim = id;
        obj->anim_done = 0;
    }
    if (obj->dir < 5) {
        i = obj->dir;
    } else {
        i = 8 - obj->dir;
    }
    script = cdload_module.get_subfile_by_id(obj->anim_scripts[i]);
    /* Evidence (class C, block placement; DECISIONS "LOOP_BLOCK audit"): the original places the end-of-script (-1)
     * block out of line, after the dir < 5 branch. */
    LOOP_BLOCK(
        if (obj->anim_time > 0) {
            break;
        }
        if (script[obj->anim_pos] == -1) {
            obj->anim_time = 0x7FFFFF;
            obj->anim_done = 1;
            break;
        }
        if (script[obj->anim_pos] == 0) {
            obj->anim_pos = 0;
        }
        obj->anim_time = script[obj->anim_pos++];
        obj->frame_file = script[obj->anim_pos++] | file;
        obj->frame_x = script[obj->anim_pos++];
        obj->frame_y = script[obj->anim_pos++];
    );
    obj->anim_time -= gfx_module.funcs.get_frame_ticks();
    if (obj->reload != 0) {
        obj->anim_time = script[obj->anim_pos - 4];
        obj->frame_file = script[obj->anim_pos - 3] | file;
        obj->frame_x = script[obj->anim_pos - 2];
        obj->frame_y = script[obj->anim_pos - 1];
    }
    if (obj->reload != 0 || obj->loaded_frame_file != obj->frame_file) {
        vram = obj->vram;
        tim_init(&tim);
        tim.set_image_pos(vram->image_x, vram->image_y);
        tim.set_clut_pos(vram->clut_x, vram->clut_y);
        tim.load(cdload_module.get_subfile_by_id(obj->frame_file));
        obj->loaded_frame_file = obj->frame_file;
        obj->frame_w = tim.width * 4;
        obj->frame_h = tim.height;
        obj->reload = 0;
    }
}

void fieldstg_actor_restore_control(FieldstgActor *obj) {
    obj->control = fieldstg_player_control;
    obj->climb_height = 0;
}

void fieldstg_actor_play_steps(FieldstgActor *obj, s32 moving, s32 encounters) {
    if (obj->base.key2 == 0) {
        if (moving) {
            if ((obj->base.timer & 7) == 0) {
                switch (obj->base.key1) {
                default:
                    sound_module.play(0x8004583C);
                    break;
                case 0x146:
                    if ((obj->base.timer & 0x1F) == 0) {
                        sound_module.play(0x80045FCB);
                    }
                    break;
                case 0x147:
                    break;
                }
                if (encounters) {
                    fieldstg_encounter_step_func();
                }
            }
        } else if ((obj->base.timer & 0x1F) == 0) {
            sound_module.play(0x8004583C);
        }
        obj->base.timer++;
    }
}

void fieldstg_actor_play_climb(FieldstgActor *obj) {
    if (obj->base.key2 == 0) {
        if ((obj->base.timer & 0xF) == 0) {
            sound_module.play(0x800458BD);
        }
        obj->base.timer++;
    }
}

void fieldstg_actor_run_state(FieldstgActor *obj, void **data) {
    GamestatePos walk;
    GamestatePos run;
    GamestatePos slide;
    GamestatePos push;
    GamestatePos carry;
    FieldstgTalk *talk;
    s32 special;

    switch (obj->base.step) {
    case 1:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 1);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        break;
    case 2:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 4);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (!(obj->base.key2 & 0xE)) {
            fieldstg_attr.get_step(&obj->pixel_pos, obj->speed >> 2, obj->dir, &walk);
            obj->pos.x += walk.x;
            obj->pos.y += walk.y;
        }
        fieldstg_actor_play_steps(obj, 0, 0);
        break;
    case 3:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 5);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (!(obj->base.key2 & 0xE)) {
            fieldstg_attr.get_step(&obj->pixel_pos, obj->speed, obj->dir, &run);
            obj->pos.x += run.x;
            obj->pos.y += run.y;
        }
        fieldstg_actor_play_steps(obj, 1, 1);
        if (gamestate_data.funcs.get_map() != 0x22D && obj->base.key2 == 0) {
            fieldstg_actor_collide(obj);
        }
        break;
    case 0x4C:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 1);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->speed != 0) {
            obj->speed -= 8;
            if (obj->speed < 0) {
                obj->speed = 0;
            }
            fieldstg_attr.get_flat_step(&obj->pixel_pos, obj->speed, obj->dir, &slide);
            obj->pos.x += slide.x;
            obj->pos.y += slide.y;
            fieldstg_actor_collide(obj);
        }
        break;
    case 0x4B:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 4);
            obj->speed = 0;
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        obj->speed += 8;
        if (records_60hz != 0) {
            if (obj->speed > 0x200) {
                obj->speed = 0x200;
            }
        } else if (obj->speed > 0x266) {
            obj->speed = 0x266;
        }
        fieldstg_attr.get_flat_step(&obj->pixel_pos, obj->speed, obj->dir, &slide);
        obj->pos.x += slide.x;
        obj->pos.y += slide.y;
        fieldstg_actor_play_steps(obj, 1, 1);
        fieldstg_actor_collide(obj);
        break;
    case 4:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 6);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 1);
        }
        break;
    case 5:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_stage.actor_busy = 1;
            if (obj->height_control == 0) {
                fieldstg_actor_set_anim(obj, 5);
            }
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        fieldstg_attr.get_step(&obj->pixel_pos, obj->speed, obj->dir, &push);
        obj->pos.x += push.x;
        obj->pos.y += push.y;
        if (obj->height_control == 0) {
            fieldstg_actor_play_steps(obj, 1, 0);
        }
        break;
    case 0x4F:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_stage.actor_busy = 1;
            fieldstg_actor_set_anim(obj, 1);
            fieldstg_carry_voice = sound_module.play(0xA064683C);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        fieldstg_attr.get_step(&obj->pixel_pos, obj->speed, obj->dir, &carry);
        obj->pos.x += carry.x;
        obj->pos.y += carry.y;
        break;
    case 0x50:
        if (obj->base.substep == 0) {
            sound_module.key_off(0xA064683C, fieldstg_carry_voice);
        }
        obj->base.substep += gfx_module.funcs.get_frame_ticks();
        if (obj->base.substep >= 30) {
            fieldstg_stage.actor_busy = 0;
            obj->base.set_step(obj, 1);
            fieldstg_actor_restore_control(obj);
        }
        break;
    case 0x40:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x20);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        break;
    case 0x41:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x1A);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        obj->climb_height += 0x100;
        if (obj->climb_height >= obj->climb_top) {
            obj->climb_height = obj->climb_top;
            obj->base.set_step(obj, 0x45);
            obj->control = NULL;
        }
        fieldstg_actor_play_climb(obj);
        break;
    case 0x42:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x1B);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        obj->climb_height -= 0x100;
        if (obj->climb_height <= 0) {
            obj->climb_height = 0;
            obj->base.set_step(obj, 0x46);
            obj->control = NULL;
        }
        fieldstg_actor_play_climb(obj);
        break;
    case 0x43:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x1C);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 0x40);
            obj->control = fieldstg_actor_control_ladder;
        }
        break;
    case 0x44:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 0;
            fieldstg_actor_set_anim(obj, 0x1E);
            obj->pos.x += obj->ladder_side != 0 ? 0x1000 : -0x1000;
            obj->pos.y += 0x1800 + obj->climb_top;
            fieldstg_camera_follow(0, 2);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 0x40);
            obj->control = fieldstg_actor_control_ladder;
            obj->has_shadow = 1;
        }
        break;
    case 0:
    default:
        obj->has_shadow = 1;
        break;
    case 0x46:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x1F);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 1);
            fieldstg_actor_restore_control(obj);
            fieldstg_partners_follow();
            obj->dir = 0;
            fieldstg_stage.actor_busy = 0;
        }
        break;
    case 0x45:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 0;
            fieldstg_actor_set_anim(obj, 0x1D);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 1);
            obj->has_shadow = 1;
            fieldstg_actor_set_anim(obj, 1);
            fieldstg_partners_follow();
            fieldstg_actor_restore_control(obj);
            obj->pos.x += obj->ladder_side != 0 ? -0x1000 : 0x1000;
            obj->pos.y -= 0x1800 + obj->climb_top;
            fieldstg_camera_follow(0, 2);
            fieldstg_stage.actor_busy = 0;
        }
        break;
    case 0x47:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->has_shadow = 0;
            fieldstg_actor_set_anim(obj, 0x16);
            obj->pos.y += obj->climb_top;
            obj->pos.x += obj->ladder_side != 0 ? 0x1000 : -0x1000;
            obj->base.next_substep(obj);
        case 1:
            if (obj->anim_done == 0) {
                break;
            }
            fieldstg_actor_set_anim(obj, 0x17);
            obj->base.next_substep(obj);
        case 2:
            obj->has_shadow = 1;
            obj->climb_height -= 0x300;
            if (obj->anim == 0x17 && obj->climb_top - obj->climb_height > 0x1800) {
                fieldstg_actor_set_anim(obj, 0x18);
            }
            if (obj->climb_height <= 0) {
                obj->climb_height = 0;
                fieldstg_actor_set_anim(obj, 0x19);
                sound_module.play(0x8004593E);
                obj->base.next_substep(obj);
            }
            break;
        case 3:
            if (obj->anim_done != 0) {
                obj->base.set_step(obj, 1);
                fieldstg_actor_set_anim(obj, 1);
                fieldstg_actor_restore_control(obj);
                fieldstg_partners_follow();
                fieldstg_stage.actor_busy = 0;
            }
            break;
        }
        break;
    case 0x48:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_actor_set_anim(obj, 0x11);
            obj->base.next_substep(obj);
        case 1:
            if (obj->anim_done == 0) {
                break;
            }
            fieldstg_actor_set_anim(obj, 0x13);
            sound_module.play(0x80045CC5);
            obj->base.next_substep(obj);
        case 2:
            if (obj->anim_done == 0) {
                break;
            }
            fieldstg_actor_set_anim(obj, 0x14);
            sound_module.play(0x80045D46);
            obj->base.next_substep(obj);
        case 3:
            if (data[2] != NULL) {
                break;
            }
            data[1] = fieldstg_icon_create(0, 1, 6);
            fieldstg_actor_set_anim(obj, 0x12);
            obj->base.next_substep(obj);
        case 4:
            obj->base.timer += gfx_module.funcs.get_frame_ticks();
            if (obj->base.timer < 60) {
                break;
            }
            ((Object *)data[1])->set_state(data[1], OBJECT_STATE_DONE);
            fieldstg_actor_set_anim(obj, 0x15);
            obj->base.next_substep(obj);
        case 5:
            if (obj->anim_done != 0) {
                obj->base.set_step(obj, 1);
                fieldstg_actor_set_anim(obj, 1);
                fieldstg_actor_restore_control(obj);
                fieldstg_stage.event_running = 0;
            }
            break;
        }
        break;
    case 0x49:
        switch (obj->base.substep) {
        case 0:
        default:
            obj->control = NULL;
            fieldstg_actor_set_anim(obj, 8);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 1);
            fieldstg_actor_set_anim(obj, 1);
            fieldstg_actor_restore_control(obj);
            fieldstg_stage.actor_busy = 0;
        }
        break;
    case 0x4A:
        special = 0;
        if (obj->base.key1 == 0x21 || (obj->base.key1 >= 0x4D && obj->base.key1 <= 0x57)
            || (obj->base.key1 >= 0x154 && obj->base.key1 <= 0x158) || obj->base.key1 == 0x15B) {
            special = 1;
        }
        switch (obj->base.substep) {
        case 0:
        default:
            talk = obj->placed->talks;
            for (;;) {
                if (talk->flags_required == NULL || gamestate_flags.check_flags(talk->flags_required) == 1) {
                    break;
                }
                talk++;
            }
            obj->talk_flags = talk->flags_set;
            if (!special && obj->no_turn == 0) {
                obj->dir = (obj->talker->dir + 4) & 7;
            }
            if (obj->sprite_file != 0 && !special) {
                data[3] = fieldstg_dialog_create_talk(obj, talk->message);
            } else {
                data[3] = fieldstg_dialog_create_talk(obj->talker, talk->message);
            }
            if (special) {
                fieldstg_actor_set_anim(obj, 0x41);
                sound_module.play(0x80045DC7);
            }
            obj->base.next_substep(obj);
            break;
        case 1:
            if (data[3] == NULL) {
                if (!special) {
                    obj->base.set_step(obj, 1);
                } else {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
                if (obj->talker->height_control == 0) {
                    fieldstg_actor_restore_control(obj->talker);
                } else {
                    obj->talker->control = fieldstg_player_control_height;
                }
                if (obj->talk_flags != NULL) {
                    gamestate_flags.set_flags(obj->talk_flags);
                }
                fieldstg_stage.actor_busy = 0;
            }
            break;
        }
        break;
    case 0x4D:
        switch (obj->base.substep) {
        case 0:
        default:
            fieldstg_stage.actor_busy = 1;
            obj->control = NULL;
            fieldstg_actor_set_anim(obj, 0x45);
            obj->base.next_substep(obj);
        case 1:
            break;
        }
        if (obj->anim_done != 0) {
            obj->base.set_step(obj, 1);
            fieldstg_actor_set_anim(obj, 1);
            fieldstg_actor_restore_control(obj);
            fieldstg_stage.actor_busy = 0;
        }
        break;
    case 0x4E:
        switch (obj->base.substep) {
        case 0:
        default:
            if (obj->base.timer < 20) {
                obj->base.timer += gfx_module.funcs.get_frame_ticks();
                break;
            }
            fieldstg_actor_set_anim(obj, 0x54);
            sound_module.play(0x800446C9);
            switch (obj->base.key1) {
            case 0x148:
                gamestate_flags.set_flag(8, 1);
                break;
            case 0x15F:
                gamestate_flags.set_flag(9, 1);
                break;
            case 0x160:
                gamestate_flags.set_flag(10, 1);
                break;
            }
            obj->base.next_substep(obj);
        case 1:
            if (obj->anim_done != 0) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    }
}

void fieldstg_partners_hide(void) {
    FieldstgActor *obj;
    s32 i;

    for (i = 0; i < 3; i++) {
        obj = (FieldstgActor *)heap_objects.find(5, -1, fieldstg_hide_partner_types[i]);
        if (obj != NULL) {
            obj->control = fieldstg_actor_follow_hide;
        }
    }
}

void fieldstg_partners_follow(void) {
    FieldstgActor *obj;
    s32 i;

    for (i = 0; i < 3; i++) {
        obj = (FieldstgActor *)heap_objects.find(5, -1, fieldstg_follow_partner_types[i]);
        if (obj != NULL) {
            obj->control = fieldstg_actor_follow;
        }
    }
}

void fieldstg_actor_get_front(FieldstgActor *obj, FieldstgPos *out) {
    FieldstgPos *offset = &fieldstg_front_offsets[obj->dir];

    out->x = obj->pixel_pos.x + offset->x;
    out->y = obj->pixel_pos.y + offset->y;
}

void fieldstg_actor_update(FieldstgActor *obj, void **data) {
    GfxLayer *layer;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->sprite_file != 0 && cdload_module.is_loading(obj->sprite_file >> 16) != 0) {
            break;
        }
        if (obj->base.key2 == 0) {
            data[0] = fieldstg_actor_effect_create(obj);
        }
        obj->base.next_state(obj);
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_RUN:
        if ((obj->base.key2 & 0xE) || fieldstg_stage.title_shown == 0) {
            if (obj->control != NULL) {
                obj->control(obj);
            }
        }
        fieldstg_actor_run_state(obj, data);
        obj->pixel_pos.x = obj->pos.x >> 8;
        obj->pixel_pos.y = (obj->pos.y - obj->climb_height) >> 8;
        if (obj->sprite_file != 0) {
            fieldstg_actor_update_sprite(obj);
            if (obj->pixel_pos.x + obj->pixel_pos.y != 0) {
                layer = gfx_module.funcs.get_layer(0x1002);
                layer->add_callback(layer, (void (*)(void *, GfxLayer *, s32))fieldstg_actor_draw, obj, obj->pixel_pos.y, 0);
            }
        }
        break;
    case OBJECT_STATE_END:
        gamestate_data.player_height = obj->height;
        if (obj->voice != -1) {
            sound_module.key_off(0xA0045F4A, obj->voice);
        }
        if (obj->trail != NULL) {
            heap_funcs.free(obj->trail);
        }
        break;
    }
}

FieldstgActor *fieldstg_actor_create(s32 id, s32 type, s32 vram, FieldstgPlacedActor *placed) {
    FieldstgActor *obj = object_create(fieldstg_actor_update, sizeof(FieldstgActor), 4 * sizeof(void *), 5);
    s32 flag;

    obj->walk_out = fieldstg_actor_walk_out;
    obj->climb_from_bottom = fieldstg_actor_climb_from_bottom;
    obj->climb_from_top = fieldstg_actor_climb_from_top;
    obj->jump_down = fieldstg_actor_jump_down;
    obj->start_meter = fieldstg_actor_start_meter;
    obj->warp = fieldstg_actor_warp;
    obj->start_walk_to = fieldstg_actor_start_walk_to;
    obj->start_carry = fieldstg_actor_start_carry;
    obj->stop_carry = fieldstg_actor_stop_carry;
    obj->launch = fieldstg_actor_launch;
    obj->reset_control = fieldstg_actor_reset_control;
    obj->set_dir = fieldstg_actor_set_dir;
    obj->is_anim_done = fieldstg_actor_is_anim_done;
    obj->is_walking = fieldstg_actor_is_walking;
    obj->set_walk_target = fieldstg_actor_set_walk_target;
    obj->set_anim = fieldstg_actor_set_anim;
    obj->play_anim = fieldstg_actor_play_anim;
    obj->base.key1 = id;
    obj->base.key2 = type;
    obj->get_front = fieldstg_actor_get_front;
    obj->sprite_file = fieldstg_stage.get_actor_sprite_file(id);
    obj->width = fieldstg_stage.get_actor_width(id) >> 1;
    obj->anim = 1;
    obj->dir = 1;
    obj->vram = &fieldstg_stage.vram_places[vram + 2];
    obj->shadow_vram = fieldstg_stage.vram_places;
    cdload_module.queue_file(obj->sprite_file >> 16);
    if (records_60hz != 0) {
        obj->speed = 0x400;
    } else {
        obj->speed = 0x4CC;
    }
    if (id == 0x146) {
        obj->speed /= 2;
    }
    obj->voice = -1;
    obj->depth = 4;
    if (type == 0) {
        if (id == 0x147) {
            obj->control = fieldstg_player_control_height;
            if (gamestate_data.map_is_new != 0) {
                obj->height = 0x3000;
            } else {
                obj->height = gamestate_data.player_height;
            }
            obj->height_control = 1;
        } else {
            obj->control = fieldstg_player_control;
        }
        if (gamestate_data.funcs.get_map_entry() != -1) {
            obj->pos = fieldstg_stage.start_pos;
            obj->dir = fieldstg_stage.start_dir;
        } else {
            obj->pos = fieldstg_stage.return_pos;
            obj->dir = fieldstg_stage.return_dir;
        }
        obj->pixel_pos.x = obj->pos.x >> 8;
        obj->pixel_pos.y = obj->pos.y >> 8;
        if (gamestate_data.map_is_new != 0) {
            obj->depth = 4;
            gamestate_data.player_depth = 4;
        } else {
            obj->depth = gamestate_data.player_depth;
        }
    } else if (type & 0xE) {
        obj->trail = heap_funcs.alloc_zero(sizeof(FieldstgTrail), 2);
        obj->control = fieldstg_actor_follow;
        if (type == 2) {
            obj->trail->read = 0x37;
        } else if (type == 4) {
            obj->trail->read = 0x2E;
        } else {
            obj->trail->read = 0x25;
        }
        if (gamestate_data.map_is_new != 0) {
            obj->depth = 4;
            gamestate_data.player_depth = 4;
        } else {
            obj->depth = gamestate_data.player_depth;
        }
    } else {
        flag = 0;
        if (id == 0x28) {
            flag = 1;
        }
        if (id == 0x29) {
            flag = 1;
        }
        if (id == 0x2A) {
            flag = 1;
        }
        if (id == 0x3E) {
            flag = 1;
        }
        if (id == 0x11A) {
            flag = 1;
        }
        obj->has_shadow = 1;
        obj->no_turn = flag;
        if (id == 0x82) {
            obj->has_shadow = 0;
        }
        if (id == 0x83) {
            obj->has_shadow = 0;
        }
        if (id == 0x11F) {
            obj->has_shadow = 0;
        }
        if (id == 0x13C) {
            obj->has_shadow = 0;
        }
        if (id == 0x13F) {
            obj->has_shadow = 0;
        }
        if (id == 0xD5) {
            obj->has_shadow = 0;
        }
        obj->placed = placed;
    }
    return obj;
}

/* End callbacks of stage 528's answer events (the first/second answer of choice N, fieldstg_choices): they set
 * flag 0x400 + N (fieldstg_flag_events' first flag: that story event is done), the first answer two more. */
void fieldstg_choice_end_first_0(void) {
    gamestate_flags.set_flag(0x707E, 1);
    gamestate_flags.set_flag(0x8B19, 1);
    gamestate_flags.set_flag(0x400, 1);
}

void fieldstg_choice_end_second_0(void) {
    gamestate_flags.set_flag(0x400, 1);
}

void fieldstg_choice_end_first_1(void) {
    gamestate_flags.set_flag(0x707E, 1);
    gamestate_flags.set_flag(0x8B1F, 1);
    gamestate_flags.set_flag(0x401, 1);
}

void fieldstg_choice_end_second_1(void) {
    gamestate_flags.set_flag(0x401, 1);
}

void fieldstg_choice_end_first_2(void) {
    gamestate_flags.set_flag(0x707F, 1);
    gamestate_flags.set_flag(0x8B1A, 1);
    gamestate_flags.set_flag(0x402, 1);
}

void fieldstg_choice_end_second_2(void) {
    gamestate_flags.set_flag(0x402, 1);
}

void fieldstg_choice_end_first_3(void) {
    gamestate_flags.set_flag(0x707F, 1);
    gamestate_flags.set_flag(0x8B20, 1);
    gamestate_flags.set_flag(0x403, 1);
}

void fieldstg_choice_end_second_3(void) {
    gamestate_flags.set_flag(0x403, 1);
}

void fieldstg_choice_end_first_4(void) {
    gamestate_flags.set_flag(0x7080, 1);
    gamestate_flags.set_flag(0x8489, 1);
    gamestate_flags.set_flag(0x404, 1);
}

void fieldstg_choice_end_second_4(void) {
    gamestate_flags.set_flag(0x404, 1);
}

void fieldstg_choice_end_first_5(void) {
    gamestate_flags.set_flag(0x7080, 1);
    gamestate_flags.set_flag(0x8495, 1);
    gamestate_flags.set_flag(0x405, 1);
}

void fieldstg_choice_end_second_5(void) {
    gamestate_flags.set_flag(0x405, 1);
}

void fieldstg_choice_end_first_6(void) {
    gamestate_flags.set_flag(0x7081, 1);
    gamestate_flags.set_flag(0x847C, 1);
    gamestate_flags.set_flag(0x406, 1);
}

void fieldstg_choice_end_second_6(void) {
    gamestate_flags.set_flag(0x406, 1);
}

void fieldstg_choice_end_first_7(void) {
    gamestate_flags.set_flag(0x7081, 1);
    gamestate_flags.set_flag(0x8462, 1);
    gamestate_flags.set_flag(0x407, 1);
}

void fieldstg_choice_end_second_7(void) {
    gamestate_flags.set_flag(0x407, 1);
}

void fieldstg_choice_end_first_8(void) {
    gamestate_flags.set_flag(0x7082, 1);
    gamestate_flags.set_flag(0x8ADE, 1);
    gamestate_flags.set_flag(0x408, 1);
}

void fieldstg_choice_end_second_8(void) {
    gamestate_flags.set_flag(0x408, 1);
}

void fieldstg_choice_end_first_9(void) {
    gamestate_flags.set_flag(0x7082, 1);
    gamestate_flags.set_flag(0x8AE8, 1);
    gamestate_flags.set_flag(0x409, 1);
}

void fieldstg_choice_end_second_9(void) {
    gamestate_flags.set_flag(0x409, 1);
}

void fieldstg_choice_end_first_10(void) {
    gamestate_flags.set_flag(0x7083, 1);
    gamestate_flags.set_flag(0x8AF4, 1);
    gamestate_flags.set_flag(0x40A, 1);
}

void fieldstg_choice_end_second_10(void) {
    gamestate_flags.set_flag(0x40A, 1);
}

void fieldstg_choice_end_first_11(void) {
    gamestate_flags.set_flag(0x7083, 1);
    gamestate_flags.set_flag(0x8AF3, 1);
    gamestate_flags.set_flag(0x40B, 1);
}

void fieldstg_choice_end_second_11(void) {
    gamestate_flags.set_flag(0x40B, 1);
}

void fieldstg_choice_end_first_12(void) {
    gamestate_flags.set_flag(0x7084, 1);
    gamestate_flags.set_flag(0x8B01, 1);
    gamestate_flags.set_flag(0x40C, 1);
}

void fieldstg_choice_end_second_12(void) {
    gamestate_flags.set_flag(0x40C, 1);
}

void fieldstg_choice_end_first_13(void) {
    gamestate_flags.set_flag(0x7085, 1);
    gamestate_flags.set_flag(0x8B0D, 1);
    gamestate_flags.set_flag(0x40D, 1);
}

void fieldstg_choice_end_second_13(void) {
    gamestate_flags.set_flag(0x40D, 1);
}

void fieldstg_choice_end_first_14(void) {
    gamestate_flags.set_flag(0x7086, 1);
    gamestate_flags.set_flag(0x8B02, 1);
    gamestate_flags.set_flag(0x40E, 1);
}

void fieldstg_choice_end_second_14(void) {
    gamestate_flags.set_flag(0x40E, 1);
}

void fieldstg_choice_end_first_15(void) {
    gamestate_flags.set_flag(0x7087, 1);
    gamestate_flags.set_flag(0x8B0F, 1);
    gamestate_flags.set_flag(0x40F, 1);
}

void fieldstg_choice_end_second_15(void) {
    gamestate_flags.set_flag(0x40F, 1);
}

void fieldstg_stage_setup(void) {
    static const CVECTOR color = { 0x80, 0x80, 0x80, 0 };

    fieldstg_stage.background_file = 0x1AC;
    fieldstg_stage.sprite_file = 0x01AD0000;
    fieldstg_stage.sprites = D_FIELDSTG_800996C4;
    fieldstg_stage.map_events = D_FIELDSTG_800997C0;
    fieldstg_stage.mask_file = 0x32C;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x6700, 0x12300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = D_FIELDSTG_80097D0C;
    fieldstg_stage.music = 0x42;
    fieldstg_stage.actors = D_FIELDSTG_800994F4;
    fieldstg_stage.sound = 0x61080002;
    fieldstg_stage.color = color;
    fieldstg_stage.events = D_FIELDSTG_80099844;
    fieldstg_attr.set_file(0, 0x01AD0002);
    fieldstg_attr.set_file(1, 0x01AD0003);
    fieldstg_attr.set_file(7, 0x01AD0001);
    fieldstg_attr.init_layer(0);
    switch (gamestate_data.progress) {
    case 5:
    case 8:
    case 12:
    case 14:
    case 16:
    case 22:
    case 24:
    case 26:
    case 28:
    case 30:
    case 31:
    case 34:
    case 36:
    case 37:
    case 38:
    case 39:
        fieldstg_stage.music = 0x42;
        fieldstg_stage.sound = 0x61080000;
        break;
    }
}

void fieldstg_window_anim_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 fieldstg_window_anim_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

s32 fieldstg_get_actor_sprite_file(s32 i) {
    return fieldstg_actor_sprite_files[i];
}

u8 fieldstg_get_actor_width(s32 i) {
    return fieldstg_actor_widths[i];
}

void fieldstg_find_stage(void) {
    FieldstgStageEntry *entry;
    s32 id;

    if (gamestate_data.progress != 0x2D) {
        entry = fieldstg_stages;
    } else {
        entry = fieldstg_stages_2d;
    }
    id = gamestate_data.funcs.get_map();
    heap_funcs.bzero(&fieldstg_stage, 0x64); /* PC_PORT: the fields before return_pos (offsetof) */
    while (1) {
        if (entry->id == id) {
            fieldstg_stage.code_file = entry->file;
            fieldstg_stage.entry = entry->entry;
            break;
        }
        entry++;
        if (entry->id == 0) {
            break;
        }
    }
    if (entry->id == 0) {
        while (1) {
            PLATFORM_HALT();
        }
    }
}

FieldstgBattleLists *fieldstg_find_battle_lists(FieldstgBattleLists *entries, s32 id) {
    s32 i;

    for (i = 0; i < 30; i++, entries++) {
        if (entries->id == id) {
            return entries;
        }
    }
    return NULL;
}

void fieldstg_timer_reset(void) {
    heap_funcs.bzero(&fieldstg_timer, 8); /* PC_PORT: frames and running, not the functions (offsetof) */
}

FieldstgActor *fieldstg_find_actor(s32 id) {
    return (FieldstgActor *)heap_objects.find(5, id, -1);
}

void fieldstg_wait_frames(s32 frames, s32 *count) {
    if (frames != 0 && fieldstg_timer.running == 0) {
        fieldstg_timer.running = 1;
        fieldstg_timer.frames = frames;
    }
    fieldstg_timer.frames -= gfx_module.funcs.get_frame_ticks();
    if (fieldstg_timer.frames <= 0) {
        fieldstg_timer.frames = 0;
        fieldstg_timer.running = 0;
        (*count)++;
    }
}

void fieldstg_wait_anim_done(s32 id, s32 *count) {
    FieldstgActor *obj = fieldstg_find_actor(id);

    if (obj->is_anim_done(obj) != 0) {
        (*count)++;
    }
}

void fieldstg_wait_walk_done(s32 id, s32 *count) {
    FieldstgActor *obj = fieldstg_find_actor(id);

    if (obj->is_walking(obj) == 0) {
        (*count)++;
    }
}

void fieldstg_to_screen_pos(FieldstgPos *pos) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 offset[2];

    layer->get_scroll(layer, offset);
    pos->x -= offset[0];
    pos->y -= offset[1];
}

/* Clears unk_10C of the player (actor 1, else 2); fieldstg_event_funcs.player_clear_unk_10C, which nothing calls. */
void fieldstg_player_clear_unk_10C(void) {
    FieldstgActor *obj = fieldstg_find_actor(1);

    if (obj == NULL) {
        obj = fieldstg_find_actor(2);
    }
    obj->unk_10C = 0;
}

FieldstgScriptObject *fieldstg_find_script_object(s32 id) {
    FieldstgScriptObject *entry;

    for (entry = fieldstg_script_objects; entry->id != 0; entry++) {
        if (entry->id == id) {
            return entry;
        }
    }
    return NULL;
}

Object *fieldstg_start_script_object(s32 id) {
    FieldstgScriptObject *entry = fieldstg_find_script_object(id);
    Object *ret = NULL;

    if (entry != NULL) {
        ret = OVERLAY_FN(2, entry->start)(id);
    }
    return ret;
}

void fieldstg_send_script_object(void *obj, s32 id, s32 arg2, s32 arg3) {
    FieldstgScriptObject *entry = fieldstg_find_script_object(id);

    if (entry != NULL && entry->message != NULL) {
        OVERLAY_FN(2, entry->message)(obj, arg2, arg3);
    }
}

void fieldstg_encounter_reset(void) {
    s32 r = pad_random.next() % 2304;

    if (r < 0x100) {
        gamestate_data.encounter_timer = r;
    } else {
        gamestate_data.encounter_timer = (r + 0x100) / 2;
    }
}

void fieldstg_encounter_start(void) {
    FieldstgActor *obj = (FieldstgActor *)heap_objects.find(5, -1, 0);
    FieldstgPos pos = obj->pixel_pos;
    s32 area = fieldstg_attr.get(4, &pos) - 1;
    s32 r = pad_random.next() & 7;
    FieldstgListedBattle *event = fieldstg_stage.battle_lists->encounters[area]->events[r];

    records_state.stage = event->stage;
    records_state.music = event->music;
    fieldstg_start_battle(event->battle);
}

void fieldstg_encounter_step(void) {
    FieldstgActor *obj;
    FieldstgPos pos;
    u32 area;
    FieldstgBattleList *list;
    s32 rate;

    if (fieldstg_attr.files[4] != 0 && fieldstg_stage.battle_lists != NULL && fieldstg_stage.battle_starting == 0
        && fieldstg_stage.event_running == 0 && fieldstg_stage.actor_busy == 0 && fieldstg_stage.title_shown == 0) {
        obj = (FieldstgActor *)heap_objects.find(5, -1, 0);
        pos = obj->pixel_pos;
        area = fieldstg_attr.get(4, &pos);
        if (area != 0) {
            area--;
            list = fieldstg_stage.battle_lists->encounters[area];
            rate = fieldstg_encounter_rates[list->rate];
            gamestate_data.encounter_timer -= rate;
            if (gamestate_data.encounter_timer <= 0) {
                if (records_state.encounters != 0) {
                    fieldstg_encounter_start();
                }
                fieldstg_encounter_reset();
            }
        }
    }
}

void fieldstg_start_listed_battle(s32 i) {
    FieldstgListedBattle *event;

    if (fieldstg_stage.battle_lists != NULL) {
        event = fieldstg_stage.battle_lists->scripted->events[i];
        records_state.stage = event->stage;
        records_state.music = event->music;
        fieldstg_start_battle(event->battle);
    }
}

s32 fieldstg_attr_load_layer(s32 i) {
    s32 *data;
    s32 id;

    /* Evidence (class A2, sched2 barrier; DECISIONS "LOOP_BLOCK audit"): the original finishes the register saves
     * before loading the fieldstg_attr base. */
    LOOP_BARRIER();
    id = fieldstg_attr.files[i];
    if (id == 0) {
        return 0;
    }
    data = cdload_module.get_subfile_by_id(id);
    fieldstg_attr.blocks = PTR_ADD(u8 *, data[0], data);
    fieldstg_attr.quarters_64 = PTR_ADD(u8 *, data[1], data);
    fieldstg_attr.quarters_32 = PTR_ADD(s16 *, data[2], data);
    fieldstg_attr.quarters_16 = PTR_ADD(s16 *, data[3], data);
    fieldstg_attr.quarters_8 = PTR_ADD(s16 *, data[4], data);
    fieldstg_attr.cells = PTR_ADD(u8 *, data[5], data);
    fieldstg_attr.width = fieldstg_attr.blocks[0];
    fieldstg_attr.height = fieldstg_attr.blocks[1];
    fieldstg_attr.blocks += 2;
    return 1;
}

void fieldstg_attr_set_file(s32 i, s32 id) {
    fieldstg_attr.files[i] = id;
}

void fieldstg_attr_init_layer(s32 layer) {
    if (gamestate_data.map_is_new != 0) {
        gamestate_data.attr_layer = layer;
    }
}

void fieldstg_attr_set_layer(s32 layer) {
    gamestate_data.attr_layer = layer;
}

u8 fieldstg_attr_get(s32 layer, FieldstgPos *pos) {
    s32 x, y, i;

    if (!fieldstg_attr_load_layer(layer)) {
        return 1;
    }
    y = pos->y;
    x = pos->x;
    i = fieldstg_attr.blocks[(y / 128) * fieldstg_attr.width + x / 128];
    i *= 4;
    if (y & 0x40) {
        i += 2;
    }
    i = fieldstg_attr.quarters_64[(x & 0x40) ? i + 1 : i];
    i *= 4;
    if (y & 0x20) {
        i += 2;
    }
    i = fieldstg_attr.quarters_32[(x & 0x20) ? i + 1 : i];
    i *= 4;
    if (y & 0x10) {
        i += 2;
    }
    i = fieldstg_attr.quarters_16[(x & 0x10) ? i + 1 : i];
    i *= 4;
    if (y & 0x8) {
        i += 2;
    }
    i = fieldstg_attr.quarters_8[(x & 0x8) ? i + 1 : i];
    return fieldstg_attr.cells[i * 64 + (y & 7) * 8 + (x & 7)];
}

s32 fieldstg_attr_is_free(FieldstgPos *pos) {
    s32 frame = gfx_module.funcs.get_frames();
    FieldstgActor *obj;
    s32 i;
    s32 dx;
    s32 dy;
    s32 w;
    s32 h;
    s32 hw;
    s32 hh;
    s32 slope;
    s32 r;

    if (frame != fieldstg_actor_boxes_frame) {
        for (obj = (FieldstgActor *)heap_objects.find(5, -1, 1), i = 0; obj != NULL;
             obj = (FieldstgActor *)heap_objects.find_next()) {
            r = obj->width;
            fieldstg_actor_boxes[i].x0 = obj->pixel_pos.x - r;
            fieldstg_actor_boxes[i].x1 = obj->pixel_pos.x + r;
            r /= 2;
            fieldstg_actor_boxes[i].y0 = obj->pixel_pos.y - r;
            fieldstg_actor_boxes[i].y1 = obj->pixel_pos.y + r;
            i++;
        }
        fieldstg_actor_box_count = i;
        fieldstg_actor_boxes_frame = frame;
    }
    for (i = 0; i < fieldstg_actor_box_count; i++) {
        if (pos->x >= fieldstg_actor_boxes[i].x0 && fieldstg_actor_boxes[i].x1 >= pos->x
            && pos->y >= fieldstg_actor_boxes[i].y0 && fieldstg_actor_boxes[i].y1 >= pos->y) {
            dx = pos->x - fieldstg_actor_boxes[i].x0;
            dy = pos->y - fieldstg_actor_boxes[i].y0;
            w = fieldstg_actor_boxes[i].x1 - fieldstg_actor_boxes[i].x0;
            h = fieldstg_actor_boxes[i].y1 - fieldstg_actor_boxes[i].y0;
            hw = w / 2;
            hh = h / 2;
            slope = w / h;
            if (dx > hw) {
                dx = hw - (dx - hw);
            }
            if (dy > hh) {
                dy = hh - (dy - hh);
            }
            if (dx >= hw - dy * slope) {
                return 0;
            }
        }
    }
    return fieldstg_spots_find(&pos->x, 0) == NULL;
}

void fieldstg_attr_get_step(FieldstgPos *pos, s32 speed, s32 dir, FieldstgPos *out) {
    u32 attr = fieldstg_attr_get(gamestate_data.attr_layer, pos);
    s32 slope = (attr & 0xF) - ((attr & 0xF) != 0);
    s32 d = (attr & 0x10) ? fieldstg_slope_mirror_dirs[dir] : dir;
    s32 sign = (attr & 0x10) ? -1 : 1;

    out->x = fieldstg_slope_steps[slope][d].x * speed * sign / 4096;
    out->y = fieldstg_slope_steps[slope][d].y * speed / 4096;
}

void fieldstg_attr_get_flat_step(s32 arg0, s32 speed, s32 dir, FieldstgPos *out) {
    out->x = fieldstg_slope_steps[0][dir].x * speed / 4096;
    out->y = fieldstg_slope_steps[0][dir].y * speed / 4096;
}

/* .data (address order) */

/* Functions of the other files and asm ones that the tables below point to. */
struct FieldstgLift;
struct FieldstgStage;
void fieldstg_effects_message(Object *obj, s32 cmd);
OBJECT_V0(Object *) fieldstg_effects_start(void); /* PC_PORT: FINDINGS 8 */
void fieldstg_lift_message(struct FieldstgLift *obj, s32 cmd);
struct FieldstgLift *fieldstg_lift_create(s32 id);
/* PC_PORT: FINDINGS 8: the creators return their object on the host (FieldstgChoice). */
OBJECT_V0(Object *) fieldstg_choice_start_0(void);
OBJECT_V0(Object *) fieldstg_choice_start_1(void);
OBJECT_V0(Object *) fieldstg_choice_start_2(void);
OBJECT_V0(Object *) fieldstg_choice_start_3(void);
OBJECT_V0(Object *) fieldstg_choice_start_4(void);
OBJECT_V0(Object *) fieldstg_choice_start_5(void);
OBJECT_V0(Object *) fieldstg_choice_start_6(void);
OBJECT_V0(Object *) fieldstg_choice_start_7(void);
OBJECT_V0(Object *) fieldstg_choice_start_8(void);
OBJECT_V0(Object *) fieldstg_choice_start_9(void);
OBJECT_V0(Object *) fieldstg_choice_start_10(void);
OBJECT_V0(Object *) fieldstg_choice_start_11(void);
OBJECT_V0(Object *) fieldstg_choice_start_12(void);
OBJECT_V0(Object *) fieldstg_choice_start_13(void);
OBJECT_V0(Object *) fieldstg_choice_start_14(void);
OBJECT_V0(Object *) fieldstg_choice_start_15(void);
struct FieldstgStage *fieldstg_stage_entry(s32 manager);
void fieldstg_stage_setup(void);
void fieldstg_encounter_step(void);

u8 fieldstg_icon_anims[11][9] = {
    { 0, 0x3C, 0xFF, 0, 0, 0, 0, 0, 0 }, { 1, 0xA, 2, 0xA, 0xFF, 0, 0, 0, 0 },
    { 3, 0x16, 4, 0x16, 0xFF, 0, 0, 0, 0 }, { 5, 4, 6, 5, 7, 4, 0x35, 0x3C, 0xFF },
    { 8, 0x16, 9, 0x16, 0xFF, 0, 0, 0, 0 }, { 0xA, 0x16, 0xB, 0x16, 0xFF, 0, 0, 0, 0 },
    { 0x10, 0x16, 0x11, 0x16, 0xFF, 0, 0, 0, 0 }, { 0xE, 0x16, 0xF, 0x16, 0xFF, 0, 0, 0, 0 },
    { 0xC, 0x16, 0xD, 0x16, 0xFF, 0, 0, 0, 0 }, { 0x12, 0x16, 0x13, 0x16, 0xFF, 0, 0, 0, 0 },
    { 0x45, 0x16, 0x46, 0x16, 0xFF, 0, 0, 0, 0 },
};

/* unreferenced */
u8 D_FIELDSTG_80097623 = 3;

u8 fieldstg_map_event_facing[8][8] = {
    { 1, 1, 0, 0, 0, 0, 0, 1 }, { 1, 1, 1, 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 0, 0, 0, 0 }, { 0, 0, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 0, 1, 1, 1, 0 }, { 0, 0, 0, 0, 0, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 1, 1 },
};

s16 fieldstg_inn_maps[22] = {
    522, 547, 559, 568, 575, 603, 605, 611,
    623, 621, 633, 658, 669, 677, 684, 708,
    710, 715, 726, 585, 691, 0,
};

FieldstgAnimFrame D_FIELDSTG_80097690[6] = { { 0x23, 8 }, { 0x24, 8 }, { 0x25, 8 }, { 0x26, 8 }, { 0xFF, 0 }, { 0, 0 } };

FieldstgAnimFrame D_FIELDSTG_8009769C[24] = {
    { 0x23, 3 }, { 0x27, 4 }, { 0x28, 4 }, { 0x2D, 4 }, { 0x2E, 4 }, { 0x2F, 3 }, { 0x30, 2 }, { 0x2E, 2 },
    { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x2E, 2 }, { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x2E, 2 },
    { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x31, 2 }, { 0x2B, 2 }, { 0x29, 2 }, { 0x2A, 2 }, { 0xFF, 0x13 },
};

FieldstgAnimFrame D_FIELDSTG_800976CC[10] = {
    { 0x29, 6 }, { 0x2E, 3 }, { 0x30, 3 }, { 0x28, 6 }, { 0x2C, 8 }, { 0x23, 8 }, { 0x24, 8 }, { 0x25, 8 },
    { 0x26, 8 }, { 0xFF, 5 },
};

FieldstgAnimFrame D_FIELDSTG_800976E0[30] = {
    { 0x23, 3 }, { 0x27, 4 }, { 0x28, 4 }, { 0x2D, 4 }, { 0x2E, 4 }, { 0x2F, 3 }, { 0x30, 2 }, { 0x2E, 2 },
    { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x2E, 2 }, { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x2E, 2 },
    { 0x2D, 2 }, { 0x2E, 2 }, { 0x30, 2 }, { 0x2F, 2 }, { 0x2E, 2 }, { 0x2D, 2 }, { 0x28, 3 }, { 0x23, 3 },
    { 0x23, 8 }, { 0x24, 8 }, { 0x25, 8 }, { 0x26, 8 }, { 0xFF, 0x18 }, { 0, 0 },
};

FieldstgAnimFrame *fieldstg_actor_effect_anims[4] = { D_FIELDSTG_80097690, D_FIELDSTG_8009769C, D_FIELDSTG_800976CC, D_FIELDSTG_800976E0 };

s16 fieldstg_ladder_offsets[16][2] = {
    { -5, -4 }, { -5, -4 }, { -5, -4 }, { -5, -9 }, { -6, -13 }, { -8, -17 }, { -10, -20 }, { -12, -23 },
    { -13, -23 }, { -14, -23 }, { -15, -22 }, { -16, -21 }, { -17, -20 }, { -16, -22 }, { -16, -22 }, { 0, 0 },
};

s32 fieldstg_ladder_step = 0;

FieldstgShatterStep fieldstg_shatter_steps[30] = {
    { &fieldstg_shatter_tiles[4][0].x, -1, 192 }, { &fieldstg_shatter_tiles[3][0].x, -1, 128 },
    { &fieldstg_shatter_tiles[2][0].x, -1, 64 }, { &fieldstg_shatter_tiles[1][0].x, -1, 0 },
    { &fieldstg_shatter_tiles[0][0].y, 1, 40 }, { &fieldstg_shatter_tiles[0][1].y, 1, 80 },
    { &fieldstg_shatter_tiles[0][2].y, 1, 120 }, { &fieldstg_shatter_tiles[0][3].y, 1, 160 },
    { &fieldstg_shatter_tiles[0][4].y, 1, 200 }, { &fieldstg_shatter_tiles[0][5].x, 1, 64 },
    { &fieldstg_shatter_tiles[1][5].x, 1, 128 }, { &fieldstg_shatter_tiles[2][5].x, 1, 192 },
    { &fieldstg_shatter_tiles[3][5].x, 1, 256 }, { &fieldstg_shatter_tiles[4][5].y, -1, 160 },
    { &fieldstg_shatter_tiles[4][4].y, -1, 120 }, { &fieldstg_shatter_tiles[4][3].y, -1, 80 },
    { &fieldstg_shatter_tiles[4][2].y, -1, 40 }, { &fieldstg_shatter_tiles[4][1].x, -1, 192 },
    { &fieldstg_shatter_tiles[3][1].x, -1, 128 }, { &fieldstg_shatter_tiles[2][1].x, -1, 64 },
    { &fieldstg_shatter_tiles[1][1].y, 1, 80 }, { &fieldstg_shatter_tiles[1][2].y, 1, 120 },
    { &fieldstg_shatter_tiles[1][3].y, 1, 160 }, { &fieldstg_shatter_tiles[1][4].x, 1, 128 },
    { &fieldstg_shatter_tiles[2][4].x, 1, 192 }, { &fieldstg_shatter_tiles[3][4].y, -1, 120 },
    { &fieldstg_shatter_tiles[3][3].y, -1, 80 }, { &fieldstg_shatter_tiles[3][2].x, -1, 192 },
    { &fieldstg_shatter_tiles[2][2].y, 1, 120 }, { NULL, 0, 0 },
};

s16 fieldstg_indexed_events[124] = {
    1522, 1200, 1201, 8, 9, 1512, 54, 1514,
    1516, 140, 142, 1205, 1210, 1211, 1215, 1220,
    1221, 1457, 1458, 1230, 1231, 1235, 1236, 1240,
    1241, 1245, 1510, 1518, 1520, 1250, 1251, 240,
    205, 290, 67, 890, 560, 1265, 1267, 1269,
    1271, 1273, 1275, 1277, 1279, 300, 70, 60,
    1299, 1301, 1302, 1303, 1304, 1260, 1262, 950,
    1281, 1283, 1285, 1287, 1289, 1291, 320, 320,
    695, 375, 260, 690, 430, 820, 735, 1310,
    736, 750, 745, 895, 1320, 1325, 13, 14,
    1421, 1423, 1425, 1427, 1429, 1430, 1436, 1438,
    1440, 1442, 1444, 1445, 1415, 220, 510, 143,
    144, 1450, 1321, 1326, 1455, 1459, 1460, 1461,
    1462, 1463, 1464, 1431, 1446, 1602, 1604, 1606,
    1612, 1614, 1616, 1618, 1620, 1622, 1624, 1626,
    1628, 1646, 1648, 785,
};

FieldstgAnimFrame fieldstg_spots_open_anim[12] = {
    { 0, 0 }, { 0x39, 6 }, { 0x3A, 4 }, { 0x3B, 4 }, { 0x3C, 8 }, { 0x3D, 8 }, { 0x3E, 8 }, { 0x3F, 0xA },
    { 0x40, 8 }, { 0x38, 8 }, { 0xFF, 0 }, { 5, 0 },
};

FieldstgAnimFrame D_FIELDSTG_800979E8[8] = {
    { 0, 8 }, { 0x5B, 4 }, { 0x5C, 4 }, { 0x5D, 4 }, { 0x5E, 4 }, { 0x5F, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_800979F8[8] = {
    { 0, 8 }, { 0x56, 4 }, { 0x57, 4 }, { 0x58, 4 }, { 0x59, 4 }, { 0x5A, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A08[8] = {
    { 0, 8 }, { 0x51, 4 }, { 0x52, 4 }, { 0x53, 4 }, { 0x54, 4 }, { 0x55, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A18[8] = {
    { 0, 8 }, { 0x4C, 4 }, { 0x4D, 4 }, { 0x4E, 4 }, { 0x4F, 4 }, { 0x50, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A28[8] = {
    { 0, 8 }, { 0x47, 4 }, { 0x48, 4 }, { 0x49, 4 }, { 0x4A, 4 }, { 0x4B, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A38[8] = {
    { 0, 8 }, { 0x14, 4 }, { 0x15, 4 }, { 0x16, 4 }, { 0x17, 4 }, { 0x18, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A48[8] = {
    { 0, 8 }, { 0x19, 4 }, { 0x1A, 4 }, { 0x1B, 4 }, { 0x1C, 4 }, { 0x1D, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame D_FIELDSTG_80097A58[8] = {
    { 0, 8 }, { 0x1E, 4 }, { 0x1F, 4 }, { 0x20, 4 }, { 0x21, 4 }, { 0x22, 4 }, { 0xFF, 0 }, { 0, 0 },
};

FieldstgAnimFrame *fieldstg_spots_effect_anims[8] = {
    D_FIELDSTG_800979E8, D_FIELDSTG_800979F8, D_FIELDSTG_80097A08, D_FIELDSTG_80097A18, D_FIELDSTG_80097A28,
    D_FIELDSTG_80097A38, D_FIELDSTG_80097A48, D_FIELDSTG_80097A58,
};

s32 fieldstg_spots_effect_depths[8] = { 1, 1, 1, -1, -1, -1, 1, 1 };
u8 D_FIELDSTG_80097AA8[12] = { 0, 0, 0, 0, 0, 0, 0xAA, 0xAA, 0x95, 0xAA, 0xA, 0 };
u8 D_FIELDSTG_80097AB4[12] = { 0xAA, 0x15, 0, 0x95, 0xAA, 0x15, 0, 0x15, 0, 0x54, 0xAA, 0x56 };
u8 D_FIELDSTG_80097AC0[12] = { 0xAA, 0x56, 0xAA, 0xAA, 0xAA, 2, 0, 0, 0, 0, 0xA8, 0xAA };
u8 D_FIELDSTG_80097ACC[12] = { 0x80, 0xAA, 0x2A, 0, 0xA8, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0, 0x54 };
u8 D_FIELDSTG_80097AD8[12] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x5A, 0xA9, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
u8 D_FIELDSTG_80097AE4[12] = { 0xAA, 0xA, 0, 0, 0, 0, 0xA8, 0xAA, 0xAA, 0xAA, 0xAA, 0x56 };
u8 D_FIELDSTG_80097AF0[12] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x2A, 0, 0xA8, 0xAA };
u8 D_FIELDSTG_80097AFC[12] = { 0, 0, 0, 0, 0x55, 0xA5, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
u8 D_FIELDSTG_80097B08[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

u8 *fieldstg_meter_patterns[9] = {
    D_FIELDSTG_80097AA8, D_FIELDSTG_80097AB4, D_FIELDSTG_80097AC0, D_FIELDSTG_80097ACC, D_FIELDSTG_80097AD8,
    D_FIELDSTG_80097AE4, D_FIELDSTG_80097AF0, D_FIELDSTG_80097AFC, D_FIELDSTG_80097B08,
};

FieldstgPos fieldstg_camera_shake_offsets[5] = { { 0, 0 }, { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

u8 fieldstg_collision_probes[8][5] = {
    { 0xA, 5, 4, 6, 3 }, { 0xE, 6, 5, 7, 4 }, { 0xB, 7, 6, 0, 5 }, { 0xF, 7, 0, 6, 1 }, { 8, 0, 1, 7, 2 },
    { 0xC, 2, 1, 3, 0 }, { 9, 2, 3, 1, 4 }, { 0xD, 3, 4, 2, 5 },
};

FieldstgPos fieldstg_collision_probe_pos[16] = {
    { -6, -8 }, { 6, -8 }, { 8, -6 }, { 8, 6 }, { 6, 8 }, { -6, 8 }, { -8, 6 }, { -8, -6 }, { 0, -8 }, { 8, 0 },
    { 0, 8 }, { -8, 0 }, { 8, -8 }, { 8, 8 }, { -8, 8 }, { -8, -8 },
};

u8 fieldstg_collision_probe_push[16][2] = {
    { 0, 0x81 }, { 0, 0x81 }, { 1, 0 }, { 1, 0 }, { 0, 1 }, { 0, 1 }, { 0x81, 0 }, { 0x81, 0 }, { 0, 0x81 },
    { 1, 0 }, { 0, 1 }, { 0x81, 0 }, { 1, 0x81 }, { 1, 1 }, { 0x81, 1 }, { 0x81, 0x81 },
};

s32 fieldstg_pad_dirs[16] = {
    0, 4, 6, 5, 0, 0, 7, 0,
    2, 3, 0, 0, 1, 0, 0, 0,
};

s16 fieldstg_talk_redirects[16][2] = {
    { 112, 32 }, { 113, 36 }, { 219, 20 }, { 220, 206 }, { 221, 41 }, { 222, 40 }, { 223, 42 }, { 224, 25 },
    { 225, 51 }, { 256, 52 }, { 268, 267 }, { 270, 157 }, { 271, 158 }, { 284, 75 }, { 288, 12 }, { 0, 0 },
};

s32 fieldstg_meter_partner_types[3] = { 2, 4, 8 };
s32 fieldstg_hide_partner_types[3] = { 2, 4, 8 };
s32 fieldstg_follow_partner_types[3] = { 2, 4, 8 };

FieldstgPos fieldstg_front_offsets[8] = {
    { 0, 16 }, { -11, 11 }, { -16, 0 }, { -11, -11 }, { 0, -16 }, { 11, -11 }, { 16, 0 }, { 11, 11 },
};

FieldstgVramPlace D_FIELDSTG_80097D0C[22] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 482, 318, 648, 62, 352, 510 }, { 448, 256, 490, 318, 680, 62, 368, 510 },
    { 448, 256, 498, 318, 712, 62, 320, 509 }, { 448, 256, 458, 320, 552, 64, 336, 509 },
    { 448, 256, 466, 348, 584, 92, 352, 509 }, { 448, 256, 474, 348, 616, 92, 368, 509 },
    { 448, 256, 448, 354, 512, 98, 320, 508 }, { 384, 256, 416, 475, 384, 219, 336, 508 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 448, 256, 448, 394, 512, 138, 352, 508 },
    { 448, 256, 490, 406, 680, 150, 368, 508 }, { 448, 256, 456, 408, 544, 152, 320, 507 },
    { 448, 256, 476, 409, 624, 153, 336, 507 }, { 448, 256, 464, 412, 576, 156, 352, 507 },
    { 448, 256, 498, 422, 712, 166, 368, 507 }, { 448, 256, 448, 426, 512, 170, 320, 506 },
};

u16 D_FIELDSTG_80097E6C[4] = { 0x1C3D, 0, 0xFFFF, 0 };
u16 D_FIELDSTG_80097E74[6] = { 0x904C, 1, 0x1C3D, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80097E80[4] = { 0x1C3D, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80097E88[6] = { 0x904D, 1, 0x1C3D, 0, 0xFFFF, 0 };
u16 D_FIELDSTG_80097E94[4] = { 0x1C3D, 0, 0xFFFF, 0 };
u16 D_FIELDSTG_80097E9C[6] = { 0x904C, 1, 0x1C3D, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80097EA8[4] = { 0x1C3D, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80097EB0[6] = { 0x904D, 1, 0x1C3D, 0, 0xFFFF, 0 };
FieldstgTalk D_FIELDSTG_80097EBC[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097ED4[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097EEC[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F04[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F1C[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F34[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F4C[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F64[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F7C[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097F94[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097FAC[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097FC4[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097FDC[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80097FF4[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009800C[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098024[2] = { { NULL, NULL, 29 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009803C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098054[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009806C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098084[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009809C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800980B4[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800980CC[2] = { { NULL, NULL, 1037 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800980E4[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800980FC[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098114[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009812C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098144[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009815C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098174[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009818C[2] = { { NULL, NULL, 1156 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800981A4[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800981BC[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800981D4[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800981EC[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098204[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009821C[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098234[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009824C[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098264[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009827C[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098294[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800982AC[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800982C4[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800982DC[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800982F4[2] = { { NULL, NULL, 1155 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009830C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098324[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009833C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098354[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009836C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098384[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009839C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800983B4[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800983CC[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800983E4[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800983FC[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098414[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009842C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098444[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009845C[2] = { { NULL, NULL, 1151 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098474[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009848C[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800984A4[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800984BC[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800984D4[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800984EC[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098504[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009851C[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098534[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009854C[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098564[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009857C[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098594[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800985AC[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800985C4[2] = { { NULL, NULL, 1154 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800985DC[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800985F4[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009860C[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098624[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009863C[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098654[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009866C[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098684[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009869C[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800986B4[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800986CC[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800986E4[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800986FC[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098714[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_8009872C[2] = { { NULL, NULL, 1153 }, { NULL, NULL, 0 } };

FieldstgTalk D_FIELDSTG_80098744[3] = {
    { D_FIELDSTG_80097E6C, D_FIELDSTG_80097E74, 97 }, { D_FIELDSTG_80097E80, D_FIELDSTG_80097E88, 98 },
    { NULL, NULL, 0 },
};

FieldstgTalk D_FIELDSTG_80098768[3] = {
    { D_FIELDSTG_80097E94, D_FIELDSTG_80097E9C, 97 }, { D_FIELDSTG_80097EA8, D_FIELDSTG_80097EB0, 98 },
    { NULL, NULL, 0 },
};

FieldstgTalk D_FIELDSTG_8009878C[2] = { { NULL, NULL, 192 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800987A4[2] = { { NULL, NULL, 193 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800987BC[2] = { { NULL, NULL, 194 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800987D4[2] = { { NULL, NULL, 195 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_800987EC[2] = { { NULL, NULL, 196 }, { NULL, NULL, 0 } };
FieldstgTalk D_FIELDSTG_80098804[2] = { { NULL, NULL, 197 }, { NULL, NULL, 0 } };
u16 D_FIELDSTG_8009881C[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098824[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009882C[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098834[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009883C[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098844[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009884C[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098854[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009885C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098864[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009886C[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098874[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009887C[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098884[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009888C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098894[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009889C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988A4[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988AC[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988B4[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988BC[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988C4[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988CC[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988D4[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988DC[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988E4[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988EC[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988F4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800988FC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098904[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009890C[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098914[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009891C[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098924[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009892C[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098934[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009893C[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098944[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009894C[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098954[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009895C[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098964[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009896C[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098974[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009897C[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098984[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009898C[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098994[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_8009899C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989A4[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989AC[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989B4[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989BC[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989C4[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989CC[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989D4[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989DC[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989E4[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989EC[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989F4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_800989FC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A04[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A0C[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A14[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A1C[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A24[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A2C[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A34[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A3C[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A44[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A4C[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A54[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A5C[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A64[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A6C[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A74[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A7C[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A84[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A8C[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A94[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098A9C[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AA4[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AAC[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AB4[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098ABC[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AC4[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098ACC[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AD4[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098ADC[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AE4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AEC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AF4[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098AFC[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B04[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B0C[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B14[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B1C[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B24[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B2C[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B34[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B3C[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B44[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B4C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B54[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B5C[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098B64[4] = { 0x6016, 1, 0xFFFF, 0 };

u16 D_FIELDSTG_80098B6C[38] = {
    0x7009, 1, 0x6005, 0, 0x6008, 0, 0x600C, 0,
    0x600E, 0, 0x6010, 0, 0x6016, 0, 0x6018, 0,
    0x601A, 0, 0x601C, 0, 0x601E, 0, 0x601F, 0,
    0x6022, 0, 0x6024, 0, 0x6025, 0, 0x6026, 0,
    0x6027, 0, 0x1C3D, 0, 0xFFFF, 0,
};

u16 D_FIELDSTG_80098BB8[4] = { 0x1C3D, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BC0[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BC8[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BD0[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BD8[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BE0[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BE8[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_FIELDSTG_80098BF0[4] = { 0x6027, 1, 0xFFFF, 0 };

FieldstgPlacedActor D_FIELDSTG_80098BF8[115] = {
    { D_FIELDSTG_8009881C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098824, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009882C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098834, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009883C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098844, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009884C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098854, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009885C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098864, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009886C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098874, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009887C, NULL, 30, 4, 336, 329, 1 }, { D_FIELDSTG_80098884, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_8009888C, NULL, 30, 4, 336, 329, 1 },
    { D_FIELDSTG_80098894, D_FIELDSTG_80097EBC, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_8009889C, D_FIELDSTG_80097ED4, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988A4, D_FIELDSTG_80097EEC, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988AC, D_FIELDSTG_80097F04, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988B4, D_FIELDSTG_80097F1C, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988BC, D_FIELDSTG_80097F34, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988C4, D_FIELDSTG_80097F4C, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988CC, D_FIELDSTG_80097F64, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988D4, D_FIELDSTG_80097F7C, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988DC, D_FIELDSTG_80097F94, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988E4, D_FIELDSTG_80097FAC, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988EC, D_FIELDSTG_80097FC4, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988F4, D_FIELDSTG_80097FDC, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_800988FC, D_FIELDSTG_80097FF4, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_80098904, D_FIELDSTG_8009800C, 50, 5, 192, 402, 5 },
    { D_FIELDSTG_8009890C, D_FIELDSTG_80098024, 51, 6, 242, 139, 1 },
    { D_FIELDSTG_80098914, D_FIELDSTG_8009803C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009891C, D_FIELDSTG_80098054, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098924, D_FIELDSTG_8009806C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009892C, D_FIELDSTG_80098084, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098934, D_FIELDSTG_8009809C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009893C, D_FIELDSTG_800980B4, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098944, D_FIELDSTG_800980CC, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009894C, D_FIELDSTG_800980E4, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098954, D_FIELDSTG_800980FC, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009895C, D_FIELDSTG_80098114, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098964, D_FIELDSTG_8009812C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009896C, D_FIELDSTG_80098144, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098974, D_FIELDSTG_8009815C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009897C, D_FIELDSTG_80098174, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_80098984, D_FIELDSTG_8009818C, 52, 7, 303, 376, 5 },
    { D_FIELDSTG_8009898C, D_FIELDSTG_800981A4, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_80098994, D_FIELDSTG_800981BC, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_8009899C, D_FIELDSTG_800981D4, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989A4, D_FIELDSTG_800981EC, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989AC, D_FIELDSTG_80098204, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989B4, D_FIELDSTG_8009821C, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989BC, D_FIELDSTG_80098234, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989C4, D_FIELDSTG_8009824C, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989CC, D_FIELDSTG_80098264, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989D4, D_FIELDSTG_8009827C, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989DC, D_FIELDSTG_80098294, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989E4, D_FIELDSTG_800982AC, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989EC, D_FIELDSTG_800982C4, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989F4, D_FIELDSTG_800982DC, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_800989FC, D_FIELDSTG_800982F4, 53, 8, 224, 368, 5 },
    { D_FIELDSTG_80098A04, D_FIELDSTG_8009830C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A0C, D_FIELDSTG_80098324, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A14, D_FIELDSTG_8009833C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A1C, D_FIELDSTG_80098354, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A24, D_FIELDSTG_8009836C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A2C, D_FIELDSTG_80098384, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A34, D_FIELDSTG_8009839C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A3C, D_FIELDSTG_800983B4, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A44, D_FIELDSTG_800983CC, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A4C, D_FIELDSTG_800983E4, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A54, D_FIELDSTG_800983FC, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A5C, D_FIELDSTG_80098414, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A64, D_FIELDSTG_8009842C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A6C, D_FIELDSTG_80098444, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A74, D_FIELDSTG_8009845C, 54, 9, 256, 384, 5 },
    { D_FIELDSTG_80098A7C, D_FIELDSTG_80098474, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098A84, D_FIELDSTG_8009848C, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098A8C, D_FIELDSTG_800984A4, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098A94, D_FIELDSTG_800984BC, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098A9C, D_FIELDSTG_800984D4, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AA4, D_FIELDSTG_800984EC, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AAC, D_FIELDSTG_80098504, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AB4, D_FIELDSTG_8009851C, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098ABC, D_FIELDSTG_80098534, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AC4, D_FIELDSTG_8009854C, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098ACC, D_FIELDSTG_80098564, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AD4, D_FIELDSTG_8009857C, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098ADC, D_FIELDSTG_80098594, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AE4, D_FIELDSTG_800985AC, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AEC, D_FIELDSTG_800985C4, 55, 10, 272, 360, 5 },
    { D_FIELDSTG_80098AF4, D_FIELDSTG_800985DC, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098AFC, D_FIELDSTG_800985F4, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B04, D_FIELDSTG_8009860C, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B0C, D_FIELDSTG_80098624, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B14, D_FIELDSTG_8009863C, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B1C, D_FIELDSTG_80098654, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B24, D_FIELDSTG_8009866C, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B2C, D_FIELDSTG_80098684, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B34, D_FIELDSTG_8009869C, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B3C, D_FIELDSTG_800986B4, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B44, D_FIELDSTG_800986CC, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B4C, D_FIELDSTG_800986E4, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B54, D_FIELDSTG_800986FC, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B5C, D_FIELDSTG_80098714, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B64, D_FIELDSTG_8009872C, 57, 11, 303, 408, 5 },
    { D_FIELDSTG_80098B6C, D_FIELDSTG_80098744, 63, 12, 176, 392, 1 },
    { D_FIELDSTG_80098BB8, D_FIELDSTG_80098768, 63, 12, 176, 265, 1 },
    { D_FIELDSTG_80098BC0, D_FIELDSTG_8009878C, 157, 13, 256, 384, 5 },
    { D_FIELDSTG_80098BC8, NULL, 158, 14, 336, 329, 1 },
    { D_FIELDSTG_80098BD0, D_FIELDSTG_800987A4, 159, 15, 303, 408, 5 },
    { D_FIELDSTG_80098BD8, D_FIELDSTG_800987BC, 160, 16, 272, 360, 5 },
    { D_FIELDSTG_80098BE0, D_FIELDSTG_800987D4, 161, 17, 224, 368, 5 },
    { D_FIELDSTG_80098BE8, D_FIELDSTG_800987EC, 162, 18, 303, 376, 5 },
    { D_FIELDSTG_80098BF0, D_FIELDSTG_80098804, 174, 19, 192, 402, 5 },
};

FieldstgPlacedActor *D_FIELDSTG_800994F4[116] = {
    &D_FIELDSTG_80098BF8[0], &D_FIELDSTG_80098BF8[1], &D_FIELDSTG_80098BF8[2], &D_FIELDSTG_80098BF8[3],
    &D_FIELDSTG_80098BF8[4], &D_FIELDSTG_80098BF8[5], &D_FIELDSTG_80098BF8[6], &D_FIELDSTG_80098BF8[7],
    &D_FIELDSTG_80098BF8[8], &D_FIELDSTG_80098BF8[9], &D_FIELDSTG_80098BF8[10], &D_FIELDSTG_80098BF8[11],
    &D_FIELDSTG_80098BF8[12], &D_FIELDSTG_80098BF8[13], &D_FIELDSTG_80098BF8[14], &D_FIELDSTG_80098BF8[15],
    &D_FIELDSTG_80098BF8[16], &D_FIELDSTG_80098BF8[17], &D_FIELDSTG_80098BF8[18], &D_FIELDSTG_80098BF8[19],
    &D_FIELDSTG_80098BF8[20], &D_FIELDSTG_80098BF8[21], &D_FIELDSTG_80098BF8[22], &D_FIELDSTG_80098BF8[23],
    &D_FIELDSTG_80098BF8[24], &D_FIELDSTG_80098BF8[25], &D_FIELDSTG_80098BF8[26], &D_FIELDSTG_80098BF8[27],
    &D_FIELDSTG_80098BF8[28], &D_FIELDSTG_80098BF8[29], &D_FIELDSTG_80098BF8[30], &D_FIELDSTG_80098BF8[31],
    &D_FIELDSTG_80098BF8[32], &D_FIELDSTG_80098BF8[33], &D_FIELDSTG_80098BF8[34], &D_FIELDSTG_80098BF8[35],
    &D_FIELDSTG_80098BF8[36], &D_FIELDSTG_80098BF8[37], &D_FIELDSTG_80098BF8[38], &D_FIELDSTG_80098BF8[39],
    &D_FIELDSTG_80098BF8[40], &D_FIELDSTG_80098BF8[41], &D_FIELDSTG_80098BF8[42], &D_FIELDSTG_80098BF8[43],
    &D_FIELDSTG_80098BF8[44], &D_FIELDSTG_80098BF8[45], &D_FIELDSTG_80098BF8[46], &D_FIELDSTG_80098BF8[47],
    &D_FIELDSTG_80098BF8[48], &D_FIELDSTG_80098BF8[49], &D_FIELDSTG_80098BF8[50], &D_FIELDSTG_80098BF8[51],
    &D_FIELDSTG_80098BF8[52], &D_FIELDSTG_80098BF8[53], &D_FIELDSTG_80098BF8[54], &D_FIELDSTG_80098BF8[55],
    &D_FIELDSTG_80098BF8[56], &D_FIELDSTG_80098BF8[57], &D_FIELDSTG_80098BF8[58], &D_FIELDSTG_80098BF8[59],
    &D_FIELDSTG_80098BF8[60], &D_FIELDSTG_80098BF8[61], &D_FIELDSTG_80098BF8[62], &D_FIELDSTG_80098BF8[63],
    &D_FIELDSTG_80098BF8[64], &D_FIELDSTG_80098BF8[65], &D_FIELDSTG_80098BF8[66], &D_FIELDSTG_80098BF8[67],
    &D_FIELDSTG_80098BF8[68], &D_FIELDSTG_80098BF8[69], &D_FIELDSTG_80098BF8[70], &D_FIELDSTG_80098BF8[71],
    &D_FIELDSTG_80098BF8[72], &D_FIELDSTG_80098BF8[73], &D_FIELDSTG_80098BF8[74], &D_FIELDSTG_80098BF8[75],
    &D_FIELDSTG_80098BF8[76], &D_FIELDSTG_80098BF8[77], &D_FIELDSTG_80098BF8[78], &D_FIELDSTG_80098BF8[79],
    &D_FIELDSTG_80098BF8[80], &D_FIELDSTG_80098BF8[81], &D_FIELDSTG_80098BF8[82], &D_FIELDSTG_80098BF8[83],
    &D_FIELDSTG_80098BF8[84], &D_FIELDSTG_80098BF8[85], &D_FIELDSTG_80098BF8[86], &D_FIELDSTG_80098BF8[87],
    &D_FIELDSTG_80098BF8[88], &D_FIELDSTG_80098BF8[89], &D_FIELDSTG_80098BF8[90], &D_FIELDSTG_80098BF8[91],
    &D_FIELDSTG_80098BF8[92], &D_FIELDSTG_80098BF8[93], &D_FIELDSTG_80098BF8[94], &D_FIELDSTG_80098BF8[95],
    &D_FIELDSTG_80098BF8[96], &D_FIELDSTG_80098BF8[97], &D_FIELDSTG_80098BF8[98], &D_FIELDSTG_80098BF8[99],
    &D_FIELDSTG_80098BF8[100], &D_FIELDSTG_80098BF8[101], &D_FIELDSTG_80098BF8[102], &D_FIELDSTG_80098BF8[103],
    &D_FIELDSTG_80098BF8[104], &D_FIELDSTG_80098BF8[105], &D_FIELDSTG_80098BF8[106], &D_FIELDSTG_80098BF8[107],
    &D_FIELDSTG_80098BF8[108], &D_FIELDSTG_80098BF8[109], &D_FIELDSTG_80098BF8[110], &D_FIELDSTG_80098BF8[111],
    &D_FIELDSTG_80098BF8[112], &D_FIELDSTG_80098BF8[113], &D_FIELDSTG_80098BF8[114], NULL,
};

FieldstgSprite D_FIELDSTG_800996C4[14] = {
    { 1, 1, 0x44, 2, 0x52, 2, 0, 1, 4, 0, 312, 282, 0, 0 },
    { 1, 2, 0x40, 2, 0x4A, 1, 0x4A, 0x51, 4, 0, 135, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x43, 4, 0, 357, 161, 0, 0 },
    { 1, 0, 0x40, 6, 1, 0, 0, 0, 0, 0, 192, 417, 0, 0 }, { 1, 0, 0x40, 6, 2, 1, 2, 7, 4, 0, 357, 161, 0, 0 },
    { 1, 3, 0xCD, 6, 0, 0, 0, 0, 0, 0, 160, 338, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x49, 4, 0, 357, 241, 0, 0 },
    { 1, 0, 0x40, 0xA, 8, 1, 8, 0xD, 4, 0, 357, 241, 0, 0 },
    { 1, 0, 0x40, 0xA, 0xF, 1, 0xF, 0x13, 4, 0, 62, 326, 0, 0 },
    { 1, 0, 0x48, 0xA, 0x14, 0, 0, 0, 0, 0, 128, 202, 0, 0 },
    { 1, 0, 0x49, 0xA, 0x15, 0, 0, 0, 0, 0, 200, 183, 0, 0 },
    { 1, 0, 0x40, 4, 0x32, 1, 0x32, 0x37, 4, 0, 357, 161, 200, 0 },
    { 1, 0, 0x40, 8, 0x38, 1, 0x38, 0x3D, 4, 0, 357, 241, 280, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

FieldstgMapEvent D_FIELDSTG_800997C0[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x201, 0x368, 0xF4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x212, 0x2FE, 0xA5, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

FieldstgStageFuncs fieldstg_stage_funcs = { fieldstg_stage_setup, fieldstg_window_anim_start, fieldstg_window_anim_update };

FieldstgEventDef D_FIELDSTG_80099844[67] = {
    { 1320, SLOT_PTR(2, s16 *, 0x800A5DE0), 0x1190040, NULL, NULL }, { 1325, SLOT_PTR(2, s16 *, 0x800A5EC4), 0x1190041, NULL, NULL },
    { 1331, SLOT_PTR(2, s16 *, 0x800A5FA8), 0x1190000, NULL, NULL },
    { 1332, NULL, 0, (s32 (*)(void))fieldstg_choice_start_0, NULL },
    { 1333, SLOT_PTR(2, s16 *, 0x800A6124), 0x1190001, NULL, fieldstg_choice_end_first_0 },
    { 1334, SLOT_PTR(2, s16 *, 0x800A62BC), 0x1190003, NULL, fieldstg_choice_end_second_0 },
    { 1336, SLOT_PTR(2, s16 *, 0x800A63A0), 0x1190004, NULL, NULL },
    { 1337, NULL, 0, (s32 (*)(void))fieldstg_choice_start_1, NULL },
    { 1338, SLOT_PTR(2, s16 *, 0x800A651C), 0x1190006, NULL, fieldstg_choice_end_first_1 },
    { 1339, SLOT_PTR(2, s16 *, 0x800A66B4), 0x1190007, NULL, fieldstg_choice_end_second_1 },
    { 1341, SLOT_PTR(2, s16 *, 0x800A6798), 0x1190008, NULL, NULL },
    { 1342, NULL, 0, (s32 (*)(void))fieldstg_choice_start_2, NULL },
    { 1343, SLOT_PTR(2, s16 *, 0x800A6914), 0x119000A, NULL, fieldstg_choice_end_first_2 },
    { 1344, SLOT_PTR(2, s16 *, 0x800A6AAC), 0x119000B, NULL, fieldstg_choice_end_second_2 },
    { 1346, SLOT_PTR(2, s16 *, 0x800A6B90), 0x119000C, NULL, NULL },
    { 1347, NULL, 0, (s32 (*)(void))fieldstg_choice_start_3, NULL },
    { 1348, SLOT_PTR(2, s16 *, 0x800A6D0C), 0x119000E, NULL, fieldstg_choice_end_first_3 },
    { 1349, SLOT_PTR(2, s16 *, 0x800A6EA4), 0x119000F, NULL, fieldstg_choice_end_second_3 },
    { 1351, SLOT_PTR(2, s16 *, 0x800A6F88), 0x1190010, NULL, NULL },
    { 1352, NULL, 0, (s32 (*)(void))fieldstg_choice_start_4, NULL },
    { 1353, SLOT_PTR(2, s16 *, 0x800A7104), 0x1190012, NULL, fieldstg_choice_end_first_4 },
    { 1354, SLOT_PTR(2, s16 *, 0x800A729C), 0x1190013, NULL, fieldstg_choice_end_second_4 },
    { 1356, SLOT_PTR(2, s16 *, 0x800A7388), 0x1190014, NULL, NULL },
    { 1357, NULL, 0, (s32 (*)(void))fieldstg_choice_start_5, NULL },
    { 1358, SLOT_PTR(2, s16 *, 0x800A7504), 0x1190016, NULL, fieldstg_choice_end_first_5 },
    { 1359, SLOT_PTR(2, s16 *, 0x800A769C), 0x1190017, NULL, fieldstg_choice_end_second_5 },
    { 1361, SLOT_PTR(2, s16 *, 0x800A7780), 0x1190018, NULL, NULL },
    { 1362, NULL, 0, (s32 (*)(void))fieldstg_choice_start_6, NULL },
    { 1363, SLOT_PTR(2, s16 *, 0x800A78FC), 0x119001A, NULL, fieldstg_choice_end_first_6 },
    { 1364, SLOT_PTR(2, s16 *, 0x800A7A9C), 0x119001B, NULL, fieldstg_choice_end_second_6 },
    { 1366, SLOT_PTR(2, s16 *, 0x800A7B80), 0x119001C, NULL, NULL },
    { 1367, NULL, 0, (s32 (*)(void))fieldstg_choice_start_7, NULL },
    { 1368, SLOT_PTR(2, s16 *, 0x800A7CFC), 0x119001E, NULL, fieldstg_choice_end_first_7 },
    { 1369, SLOT_PTR(2, s16 *, 0x800A7E94), 0x119001F, NULL, fieldstg_choice_end_second_7 },
    { 1371, SLOT_PTR(2, s16 *, 0x800A7F78), 0x1190020, NULL, NULL },
    { 1372, NULL, 0, (s32 (*)(void))fieldstg_choice_start_8, NULL },
    { 1373, SLOT_PTR(2, s16 *, 0x800A80F4), 0x1190022, NULL, fieldstg_choice_end_first_8 },
    { 1374, SLOT_PTR(2, s16 *, 0x800A828C), 0x1190023, NULL, fieldstg_choice_end_second_8 },
    { 1376, SLOT_PTR(2, s16 *, 0x800A8370), 0x1190024, NULL, NULL },
    { 1377, NULL, 0, (s32 (*)(void))fieldstg_choice_start_9, NULL },
    { 1378, SLOT_PTR(2, s16 *, 0x800A84EC), 0x1190026, NULL, fieldstg_choice_end_first_9 },
    { 1379, SLOT_PTR(2, s16 *, 0x800A8684), 0x1190027, NULL, fieldstg_choice_end_second_9 },
    { 1381, SLOT_PTR(2, s16 *, 0x800A8768), 0x1190028, NULL, NULL },
    { 1382, NULL, 0, (s32 (*)(void))fieldstg_choice_start_10, NULL },
    { 1383, SLOT_PTR(2, s16 *, 0x800A88E4), 0x119002A, NULL, fieldstg_choice_end_first_10 },
    { 1384, SLOT_PTR(2, s16 *, 0x800A8A7C), 0x119002B, NULL, fieldstg_choice_end_second_10 },
    { 1386, SLOT_PTR(2, s16 *, 0x800A8B60), 0x119002C, NULL, NULL },
    { 1387, NULL, 0, (s32 (*)(void))fieldstg_choice_start_11, NULL },
    { 1388, SLOT_PTR(2, s16 *, 0x800A8CDC), 0x119002E, NULL, fieldstg_choice_end_first_11 },
    { 1389, SLOT_PTR(2, s16 *, 0x800A8E88), 0x119002F, NULL, fieldstg_choice_end_second_11 },
    { 1391, SLOT_PTR(2, s16 *, 0x800A8F6C), 0x1190030, NULL, NULL },
    { 1392, NULL, 0, (s32 (*)(void))fieldstg_choice_start_12, NULL },
    { 1393, SLOT_PTR(2, s16 *, 0x800A90E8), 0x1190032, NULL, fieldstg_choice_end_first_12 },
    { 1394, SLOT_PTR(2, s16 *, 0x800A9270), 0x1190033, NULL, fieldstg_choice_end_second_12 },
    { 1396, SLOT_PTR(2, s16 *, 0x800A9354), 0x1190034, NULL, NULL },
    { 1397, NULL, 0, (s32 (*)(void))fieldstg_choice_start_13, NULL },
    { 1398, SLOT_PTR(2, s16 *, 0x800A94D4), 0x1190036, NULL, fieldstg_choice_end_first_13 },
    { 1399, SLOT_PTR(2, s16 *, 0x800A965C), 0x1190037, NULL, fieldstg_choice_end_second_13 },
    { 1401, SLOT_PTR(2, s16 *, 0x800A9740), 0x1190038, NULL, NULL },
    { 1402, NULL, 0, (s32 (*)(void))fieldstg_choice_start_14, NULL },
    { 1403, SLOT_PTR(2, s16 *, 0x800A98BC), 0x119003A, NULL, fieldstg_choice_end_first_14 },
    { 1404, SLOT_PTR(2, s16 *, 0x800A9A64), 0x119003B, NULL, fieldstg_choice_end_second_14 },
    { 1406, SLOT_PTR(2, s16 *, 0x800A9B48), 0x119003C, NULL, NULL },
    { 1407, NULL, 0, (s32 (*)(void))fieldstg_choice_start_15, NULL },
    { 1408, SLOT_PTR(2, s16 *, 0x800A9CC8), 0x119003E, NULL, fieldstg_choice_end_first_15 },
    { 1409, SLOT_PTR(2, s16 *, 0x800A9E60), 0x119003F, NULL, fieldstg_choice_end_second_15 }, { -1, NULL, 0, NULL, NULL },
};

FieldstgStageState fieldstg_stage = {
    .find_stage = fieldstg_find_stage,
    .get_actor_sprite_file = fieldstg_get_actor_sprite_file,
    .get_actor_width = (s32 (*)(s32))fieldstg_get_actor_width,
    .find_battle_lists = (FieldstgBattleLists *(*)(FieldstgBattleLists *, s32))fieldstg_find_battle_lists,
};

s32 fieldstg_actor_sprite_files[406] = {
    0, 0x18600CA, 0x1870070, 0x1880071, 0x189005F, 0x18A005F, 0x18B0073, 0x18C006E,
    0x18D006B, 0x18E0072, 0x18F009B, 0x19000A8, 0x19100AD, 0x192003D, 0x192003D, 0x192003D,
    0x193002A, 0x3140020, 0x3130020, 0x3790020, 0x2F50010, 0x31C002D, 0x2F60010, 0x2F40021,
    0x2F7001E, 0x3190010, 0x31A0021, 0x317000E, 0x3880007, 0x4260012, 0x2E20020, 0x3270008,
    0x4010014, 0x3FC0015, 0x2FB003C, 0x2FA0037, 0x4010014, 0x31B0020, 0x31B0020, 0x31B0020,
    0x30B0004, 0x30C0004, 0x30D0004, 0x30A0020, 0x30A0020, 0x3160020, 0x3140020, 0x3150020,
    0x3050015, 0x3120020, 0x3100020, 0x3130020, 0x30F0020, 0x3110020, 0x30E0020, 0x3790020,
    0x3770020, 0x3780025, 0x3850020, 0x3860008, 0x2E50020, 0x3760008, 0x41B0005, 0,
    0x2FC0032, 0x2FD003B, 0x2FE005A, 0x2EF001E, 0x31A0021, 0x3580020, 0x3580020, 0x3580020,
    0x3580020, 0x3580020, 0x3580020, 0x41A0014, 0x41A0014, 0x3FC0015, 0x3FC0015, 0x3FC0015,
    0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015,
    0x41A0014, 0x4010014, 0x3870004, 0x4260012, 0x4260012, 0x4260012, 0x4260012, 0x4260012,
    0x4260012, 0x3740012, 0x3740012, 0x4F40004, 0x4190008, 0x5510042, 0x4F50032, 0x64B0032,
    0x4F0002E, 0x3730037, 0x64C006E, 0x64E006E, 0x4F20020, 0x3560020, 0x3560020, 0x31B0020,
    0, 0, 0x64D0008, 0x3550020, 0x3550020, 0x3550020, 0x3E30032, 0x3560020,
    0x2F50010, 0x2F50010, 0x2E60020, 0x38B0008, 0x38C0008, 0x42D0008, 0x38E0008, 0x38D0008,
    0x4180008, 0x42C0008, 0x7D10011, 0x7D20011, 0x3750008, 0x3EE000F, 0x318000F, 0x35C000E,
    0x35A0010, 0x35D000E, 0x41C0011, 0x3590012, 0x35B000E, 0x2E40020, 0x2E70022, 0x2E80020,
    0x2EA0020, 0x2EB0020, 0x2E90022, 0x3710009, 0x3710009, 0x3710009, 0x3EF0008, 0x3EC000A,
    0x3570020, 0x7960002, 0x7950008, 0x7950008, 0x7950008, 0x412002D, 0x412002D, 0x412002D,
    0x412002D, 0x412002D, 0x412002D, 0x30A0020, 0x3580020, 0x3ED0008, 0x4150008, 0x4140009,
    0x4110006, 0x4160009, 0x4130008, 0x38A0009, 0x3890009, 0x4170009, 0x412002D, 0x412002D,
    0x412002D, 0x412002D, 0x2F80026, 0x2F90026, 0x7920032, 0x7930032, 0x794003A, 0x7400008,
    0x3740012, 0x3740012, 0x3740012, 0x3740012, 0x3740012, 0x3740012, 0x3740012, 0x3740012,
    0x3700016, 0x3720012, 0x3530023, 0x3530023, 0x3530023, 0x7A8000C, 0, 0,
    0x2ED001E, 0x2EE001E, 0x2F0001E, 0x2F1001E, 0x2EE001E, 0x2E3001E, 0x4260012, 0x3720012,
    0x3540022, 0x3540022, 0x37A0020, 0x37A0020, 0x37A0020, 0x73E0019, 0x7A70021, 0x7D50013,
    0x7D50013, 0x7D50013, 0x7D50013, 0, 0, 0, 0, 0,
    0, 0, 0x2FB003C, 0x2FB003C, 0x2FA0037, 0x2FA0037, 0x31B0020, 0x2FC0032,
    0x2FC0032, 0x2FE005A, 0x2FE005A, 0x2FD003B, 0x2FD003B, 0x794003A, 0x794003A, 0x7930032,
    0x7930032, 0x7920032, 0x7920032, 0x3EA0008, 0x3EA0008, 0x3EA0008, 0x3EA0008, 0x3EA0008,
    0x3EA0008, 0x3EA0008, 0x3EA0008, 0x412002D, 0x3580020, 0x3580020, 0x2EF001E, 0x2EC001C,
    0, 0x3550020, 0x4F10020, 0x3550020, 0x3550020, 0x3E40008, 0x3EB0008, 0x3E70008,
    0x3E90008, 0x3E80008, 0x4F30004, 0x3880007, 0, 0x3580020, 0, 0,
    0x3E30032, 0x3E30032, 0x3E30032, 0x3E30032, 0x3E30032, 0x3E30032, 0x3E30032, 0x3E30032,
    0x3120020, 0x412002D, 0x7AD0029, 0x73F0026, 0, 0x3730037, 0x3EC000A, 0x7AA0004,
    0, 0, 0, 0, 0, 0x3790020, 0x3160020, 0x4260012,
    0x4260012, 0x3740012, 0x3720012, 0x3550020, 0x18E0072, 0x3530023, 0x3530023, 0x3550020,
    0x3550020, 0x3550020, 0x3580020, 0x3580020, 0x3580020, 0x3580020, 0x3580020, 0x3580020,
    0x3530023, 0x3530023, 0x412002D, 0x3130020, 0x8150008, 0x7D4002D, 0x7D3002D, 0x7A90002,
    0x4260012, 0x4260012, 0x4260012, 0x3740012, 0x3740012, 0x3740012, 0x3E6005F, 0x3E50023,
    0x839000E, 0x73C0037, 0x73D0037, 0x7D60005, 0x7D70008, 0x4F6006E, 0x4F6006E, 0x4F6006E,
    0x4F6006E, 0x4F6006E, 0x4F6006E, 0x4F6006E, 0x3FC0015, 0x3FC0015, 0x3FC0015, 0x3FC0015,
    0x3FC0015, 0x3730037, 0x3730037, 0x3FC0015, 0x3730037, 0x3730037, 0x3730037, 0x839000E,
    0x839000E, 0x839000E, 0x839000E, 0x839000E, 0x839000E, 0x839000E, 0x839000E, 0x839000E,
    0x839000E, 0x4260012, 0x4260012, 0x3740012, 0x3740012, 0x8940019, 0x8940019, 0x8990019,
    0x8990019, 0x8950014, 0x8950014, 0x8970032, 0x8970032, 0x8960019, 0x8960019, 0x8910019,
    0x8910019, 0x8930014, 0x8920014, 0x8920014, 0x8920014, 0x31B0020, 0x31B0020, 0x31B0020,
    0x89C0002, 0x89C0002, 0x89C0002, 0x3740012, 0x3740012, 0x8A00008, 0x31B0020, 0x31B0020,
    0x73E0019, 0x3EB0008, 0x3E90008, 0x3E70008, 0x3E80008, 0x3E40008, 0x4F30004, 0x3140020,
    0x3780025, 0x3110020, 0x30F0020, 0x3100020, 0x8960019, 0x8970032,
};

u8 fieldstg_actor_widths[408] = {
    0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x20, 0x28, 0x20, 0x20, 0x28, 0x20, 0x20, 0x28, 0x38, 0x28,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x28, 0x20, 0x20, 0x20, 0x18, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x20, 0x20, 0x40, 0x20,
    0x20, 0x20, 0x20, 0x18, 0x28, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x28, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x20, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
    0x28, 0x20, 0x20, 0x28, 0x30, 0x20, 0x18, 0x20, 0x18, 0x20, 0x20, 0x20, 0x18, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x50, 0x40, 0x30, 0x48, 0x40,
    0x40, 0x28, 0x10, 0x18, 0x30, 0x38, 0x28, 0x20, 0x20, 0x20, 0x28, 0x30, 0x20, 0x28, 0x20, 0x20,
    0x28, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x28, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x18, 0x18, 0x20, 0x18, 0x18, 0x20, 0x28, 0x20,
    0x30, 0x30, 0x28, 0x28, 0x28, 0x40, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x20, 0x20, 0x20, 0x18, 0x38,
    0x20, 0x20, 0x18, 0x20, 0x20, 0x38, 0x38, 0x40, 0x50, 0x48, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x38, 0x20, 0x20, 0x20, 0x20, 0x30, 0x18,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x14, 0x20, 0x20, 0x14,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x28, 0x20, 0x50, 0x20, 0x20, 0x14, 0x14, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60,
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x38, 0x38, 0x38, 0x38, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x38, 0x38,
    0x40, 0x40, 0x40, 0x38, 0x38, 0x14, 0x20, 0x20, 0x40, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x38,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0, 0,
};

FieldstgStageEntry fieldstg_stages_2d[55] = {
    { 624, 1851, WSTAG_ENTRY(0x800A5E2C) }, { 625, 2217, WSTAG_ENTRY(0x800A5E2C) },
    { 626, 2219, WSTAG_ENTRY(0x800A5E2C) }, { 627, 2220, WSTAG_ENTRY(0x800A5E2C) },
    { 629, 2221, WSTAG_ENTRY(0x800A5E8C) }, { 630, 2222, WSTAG_ENTRY(0x800A5E64) },
    { 631, 2223, WSTAG_ENTRY(0x800A5E28) }, { 632, 2224, WSTAG_ENTRY(0x800A60BC) },
    { 633, 2225, WSTAG_ENTRY(0x800A5E28) }, { 634, 2226, WSTAG_ENTRY(0x800A5E28) },
    { 635, 2227, WSTAG_ENTRY(0x800A5E28) }, { 636, 2228, WSTAG_ENTRY(0x800A5E28) },
    { 637, 2229, WSTAG_ENTRY(0x800A5E28) }, { 638, 2230, WSTAG_ENTRY(0x800A5E28) },
    { 639, 2231, WSTAG_ENTRY(0x800A633C) }, { 640, 2232, WSTAG_ENTRY(0x800A5E88) },
    { 641, 2233, WSTAG_ENTRY(0x800A5E2C) }, { 642, 2234, WSTAG_ENTRY(0x800A5E28) },
    { 650, 2235, WSTAG_ENTRY(0x800A5E2C) }, { 651, 2236, WSTAG_ENTRY(0x800A5E28) },
    { 652, 2237, WSTAG_ENTRY(0x800A5E2C) }, { 653, 2238, WSTAG_ENTRY(0x800A5E28) },
    { 654, 2239, WSTAG_ENTRY(0x800A5E2C) }, { 655, 2240, WSTAG_ENTRY(0x800A5E2C) },
    { 656, 2241, WSTAG_ENTRY(0x800A5E28) }, { 657, 2242, WSTAG_ENTRY(0x800A5E28) },
    { 658, 2243, WSTAG_ENTRY(0x800A5E28) }, { 659, 2244, WSTAG_ENTRY(0x800A5E28) },
    { 660, 2245, WSTAG_ENTRY(0x800A5E28) }, { 661, 2246, WSTAG_ENTRY(0x800A5FC8) },
    { 662, 2247, WSTAG_ENTRY(0x800A5E2C) }, { 663, 2248, WSTAG_ENTRY(0x800A5E2C) },
    { 664, 2249, WSTAG_ENTRY(0x800A5E28) }, { 665, 2250, WSTAG_ENTRY(0x800A5E2C) },
    { 666, 2251, WSTAG_ENTRY(0x800A6038) }, { 667, 2252, WSTAG_ENTRY(0x800A5E2C) },
    { 668, 2253, WSTAG_ENTRY(0x800A5E28) }, { 669, 2254, WSTAG_ENTRY(0x800A5E28) },
    { 670, 2255, WSTAG_ENTRY(0x800A5E28) }, { 671, 2256, WSTAG_ENTRY(0x800A5E28) },
    { 736, 2257, WSTAG_ENTRY(0x800A5F58) }, { 737, 2258, WSTAG_ENTRY(0x800A5F58) },
    { 738, 2259, WSTAG_ENTRY(0x800A5F58) }, { 739, 2260, WSTAG_ENTRY(0x800A5F58) },
    { 740, 2261, WSTAG_ENTRY(0x800A5F58) }, { 741, 2262, WSTAG_ENTRY(0x800A5F58) },
    { 742, 2263, WSTAG_ENTRY(0x800A5F58) }, { 743, 2264, WSTAG_ENTRY(0x800A5F58) },
    { 744, 2265, WSTAG_ENTRY(0x800A5F54) }, { 745, 2266, WSTAG_ENTRY(0x800A5F54) },
    { 746, 2267, WSTAG_ENTRY(0x800A5F54) }, { 747, 2268, WSTAG_ENTRY(0x800A5F54) },
    { 748, 2269, WSTAG_ENTRY(0x800A5F54) }, { 749, 2270, WSTAG_ENTRY(0x800A5F54) },
    { 750, 2271, WSTAG_ENTRY(0x800A5F54) },
};

FieldstgStageEntry fieldstg_stages[240] = {
    { 512, 466, WSTAG_ENTRY(0x800A5E7C) }, { 513, 523, WSTAG_ENTRY(0x800A5E2C) },
    { 514, 467, WSTAG_ENTRY(0x800A6194) }, { 515, 468, WSTAG_ENTRY(0x800A74A4) },
    { 516, 536, WSTAG_ENTRY(0x800A63DC) }, { 517, 560, WSTAG_ENTRY(0x800A5E2C) },
    { 518, 469, WSTAG_ENTRY(0x800A7074) }, { 519, 470, WSTAG_ENTRY(0x800A62B4) },
    { 520, 471, WSTAG_ENTRY(0x800A5E28) }, { 521, 594, WSTAG_ENTRY(0x800A605C) },
    { 522, 472, WSTAG_ENTRY(0x800A5E28) }, { 523, 639, WSTAG_ENTRY(0x800A5E28) },
    { 524, 473, WSTAG_ENTRY(0x800A5E28) }, { 525, 474, WSTAG_ENTRY(0x800A5E28) },
    { 526, 475, WSTAG_ENTRY(0x800A5E28) }, { 527, 476, WSTAG_ENTRY(0x800A5E28) },
    { 528, 477, (void *(*)())fieldstg_stage_entry }, { 529, 478, WSTAG_ENTRY(0x800A6A58) },
    { 530, 479, WSTAG_ENTRY(0x800A5E2C) }, { 531, 480, WSTAG_ENTRY(0x800A64B8) },
    { 532, 694, WSTAG_ENTRY(0x800A5EAC) }, { 533, 685, WSTAG_ENTRY(0x800A5E28) },
    { 534, 840, WSTAG_ENTRY(0x800A5EE0) }, { 535, 848, WSTAG_ENTRY(0x800A5ED8) },
    { 536, 844, WSTAG_ENTRY(0x800A63A8) }, { 537, 836, WSTAG_ENTRY(0x800A6D7C) },
    { 538, 936, WSTAG_ENTRY(0x800A5E28) }, { 539, 677, WSTAG_ENTRY(0x800A5E88) },
    { 540, 1031, WSTAG_ENTRY(0x800A6068) }, { 541, 481, WSTAG_ENTRY(0x800A5E2C) },
    { 542, 482, WSTAG_ENTRY(0x800A5E3C) }, { 543, 931, WSTAG_ENTRY(0x800A5E2C) },
    { 544, 935, WSTAG_ENTRY(0x800A5E2C) }, { 545, 483, WSTAG_ENTRY(0x800A5E28) },
    { 546, 484, WSTAG_ENTRY(0x800A5E28) }, { 547, 495, WSTAG_ENTRY(0x800A5E28) },
    { 548, 541, WSTAG_ENTRY(0x800A5E28) }, { 549, 485, WSTAG_ENTRY(0x800A5E28) },
    { 550, 486, WSTAG_ENTRY(0x800A6018) }, { 551, 700, WSTAG_ENTRY(0x800A5E2C) },
    { 552, 1231, WSTAG_ENTRY(0x800A5E84) }, { 553, 493, WSTAG_ENTRY(0x800A5EB8) },
    { 554, 1035, WSTAG_ENTRY(0x800A5E8C) }, { 555, 573, WSTAG_ENTRY(0x800A6098) },
    { 556, 926, WSTAG_ENTRY(0x800A5E2C) }, { 557, 1259, WSTAG_ENTRY(0x800A6FC0) },
    { 558, 1260, WSTAG_ENTRY(0x800A5E74) }, { 559, 566, WSTAG_ENTRY(0x800A5E28) },
    { 560, 496, WSTAG_ENTRY(0x800A5E28) }, { 561, 487, WSTAG_ENTRY(0x800A5E28) },
    { 562, 894, WSTAG_ENTRY(0x800A5E50) }, { 563, 1298, WSTAG_ENTRY(0x800A5E84) },
    { 564, 922, WSTAG_ENTRY(0x800A5E84) }, { 565, 914, WSTAG_ENTRY(0x800A60D4) },
    { 566, 868, WSTAG_ENTRY(0x800A701C) }, { 567, 488, WSTAG_ENTRY(0x800A5E2C) },
    { 568, 533, WSTAG_ENTRY(0x800A5E28) }, { 569, 497, WSTAG_ENTRY(0x800A5F88) },
    { 570, 871, WSTAG_ENTRY(0x800A6230) }, { 571, 951, WSTAG_ENTRY(0x800A5E2C) },
    { 572, 1024, WSTAG_ENTRY(0x800A5EC8) }, { 573, 968, WSTAG_ENTRY(0x800A5E28) },
    { 574, 1039, WSTAG_ENTRY(0x800A5E28) }, { 575, 540, WSTAG_ENTRY(0x800A5E28) },
    { 576, 547, WSTAG_ENTRY(0x800A5E74) }, { 577, 964, WSTAG_ENTRY(0x800A5E80) },
    { 578, 589, WSTAG_ENTRY(0x800A5E28) }, { 579, 661, WSTAG_ENTRY(0x800A5E80) },
    { 580, 1365, WSTAG_ENTRY(0x800A5E88) }, { 581, 1402, WSTAG_ENTRY(0x800A5E28) },
    { 582, 574, WSTAG_ENTRY(0x800A6984) }, { 583, 1135, WSTAG_ENTRY(0x800A5E84) },
    { 584, 544, WSTAG_ENTRY(0x800A60D0) }, { 585, 689, WSTAG_ENTRY(0x800A5E2C) },
    { 586, 1308, WSTAG_ENTRY(0x800A5E80) }, { 587, 1061, WSTAG_ENTRY(0x800A5E2C) },
    { 588, 1154, WSTAG_ENTRY(0x800A5E74) }, { 589, 1430, WSTAG_ENTRY(0x800A5EA8) },
    { 590, 1292, WSTAG_ENTRY(0x800A5E88) }, { 591, 1293, WSTAG_ENTRY(0x800A5E28) },
    { 592, 1294, WSTAG_ENTRY(0x800A5E28) }, { 593, 1282, WSTAG_ENTRY(0x800A5E28) },
    { 594, 1158, WSTAG_ENTRY(0x800A5E68) }, { 595, 1217, WSTAG_ENTRY(0x800A5E28) },
    { 596, 1374, WSTAG_ENTRY(0x800A5E78) }, { 597, 1165, WSTAG_ENTRY(0x800A6314) },
    { 598, 1166, WSTAG_ENTRY(0x800A6204) }, { 599, 1173, WSTAG_ENTRY(0x800A60D0) },
    { 600, 1181, WSTAG_ENTRY(0x800A61F4) }, { 601, 1185, WSTAG_ENTRY(0x800A61F4) },
    { 602, 1193, WSTAG_ENTRY(0x800A60D0) }, { 603, 1359, WSTAG_ENTRY(0x800A5E28) },
    { 604, 1177, WSTAG_ENTRY(0x800A5E74) }, { 605, 1224, WSTAG_ENTRY(0x800A5E2C) },
    { 606, 1201, WSTAG_ENTRY(0x800A5E74) }, { 607, 1499, WSTAG_ENTRY(0x800A6454) },
    { 608, 1225, WSTAG_ENTRY(0x800A5E58) }, { 609, 1504, WSTAG_ENTRY(0x800A5E28) },
    { 610, 1512, WSTAG_ENTRY(0x800A5E28) }, { 611, 1520, WSTAG_ENTRY(0x800A5E28) },
    { 612, 1528, WSTAG_ENTRY(0x800A5E28) }, { 613, 681, WSTAG_ENTRY(0x800A5E28) },
    { 614, 1540, WSTAG_ENTRY(0x800A5E28) }, { 615, 1544, WSTAG_ENTRY(0x800A5E28) },
    { 616, 1556, WSTAG_ENTRY(0x800A5E28) }, { 617, 1564, WSTAG_ENTRY(0x800A6024) },
    { 618, 1572, WSTAG_ENTRY(0x800A5EE4) }, { 619, 1619, WSTAG_ENTRY(0x800A6138) },
    { 620, 1621, WSTAG_ENTRY(0x800A62B8) }, { 621, 1623, WSTAG_ENTRY(0x800A6424) },
    { 622, 1624, WSTAG_ENTRY(0x800A5E6C) }, { 623, 1626, WSTAG_ENTRY(0x800A5E28) },
    { 624, 522, WSTAG_ENTRY(0x800A5E2C) }, { 625, 524, WSTAG_ENTRY(0x800A5E2C) },
    { 626, 525, WSTAG_ENTRY(0x800A5EC0) }, { 627, 535, WSTAG_ENTRY(0x800A5E2C) },
    { 628, 564, WSTAG_ENTRY(0x800A5E2C) }, { 629, 590, WSTAG_ENTRY(0x800A5E2C) },
    { 630, 592, WSTAG_ENTRY(0x800A62C8) }, { 631, 593, WSTAG_ENTRY(0x800A5E28) },
    { 632, 595, WSTAG_ENTRY(0x800A605C) }, { 633, 624, WSTAG_ENTRY(0x800A5E28) },
    { 634, 667, WSTAG_ENTRY(0x800A5E28) }, { 635, 672, WSTAG_ENTRY(0x800A5E28) },
    { 636, 697, WSTAG_ENTRY(0x800A5E28) }, { 637, 701, WSTAG_ENTRY(0x800A5E28) },
    { 638, 658, WSTAG_ENTRY(0x800A5E28) }, { 639, 702, WSTAG_ENTRY(0x800A634C) },
    { 640, 711, WSTAG_ENTRY(0x800A5E28) }, { 641, 809, WSTAG_ENTRY(0x800A5E2C) },
    { 642, 810, WSTAG_ENTRY(0x800A5E28) }, { 643, 826, WSTAG_ENTRY(0x800A5E28) },
    { 644, 830, WSTAG_ENTRY(0x800A5E28) }, { 645, 874, WSTAG_ENTRY(0x800A5E28) },
    { 646, 875, WSTAG_ENTRY(0x800A5E28) }, { 647, 876, WSTAG_ENTRY(0x800A6160) },
    { 648, 877, WSTAG_ENTRY(0x800A6DD0) }, { 649, 937, WSTAG_ENTRY(0x800A5E28) },
    { 650, 1030, WSTAG_ENTRY(0x800A5E2C) }, { 651, 1077, WSTAG_ENTRY(0x800A6068) },
    { 652, 1100, WSTAG_ENTRY(0x800A5E2C) }, { 653, 1120, WSTAG_ENTRY(0x800A5E28) },
    { 654, 1121, WSTAG_ENTRY(0x800A5E28) }, { 655, 1122, WSTAG_ENTRY(0x800A5E28) },
    { 656, 1123, WSTAG_ENTRY(0x800A5E28) }, { 657, 1140, WSTAG_ENTRY(0x800A5E28) },
    { 658, 1150, WSTAG_ENTRY(0x800A5E28) }, { 659, 1189, WSTAG_ENTRY(0x800A5E28) },
    { 660, 1197, WSTAG_ENTRY(0x800A5E28) }, { 661, 1205, WSTAG_ENTRY(0x800A5E84) },
    { 662, 1209, WSTAG_ENTRY(0x800A5E2C) }, { 663, 1235, WSTAG_ENTRY(0x800A5E28) },
    { 664, 1236, WSTAG_ENTRY(0x800A5E28) }, { 665, 1241, WSTAG_ENTRY(0x800A5E2C) },
    { 666, 1245, WSTAG_ENTRY(0x800A6084) }, { 667, 1249, WSTAG_ENTRY(0x800A5E2C) },
    { 668, 1261, WSTAG_ENTRY(0x800A5F1C) }, { 669, 1273, WSTAG_ENTRY(0x800A5E28) },
    { 670, 1278, WSTAG_ENTRY(0x800A5E28) }, { 671, 1300, WSTAG_ENTRY(0x800A5E28) },
    { 672, 1304, WSTAG_ENTRY(0x800A5E2C) }, { 673, 1312, WSTAG_ENTRY(0x800A5E2C) },
    { 674, 1316, WSTAG_ENTRY(0x800A5E2C) }, { 675, 1320, WSTAG_ENTRY(0x800A5E2C) },
    { 676, 1324, WSTAG_ENTRY(0x800A5E2C) }, { 677, 1328, WSTAG_ENTRY(0x800A5E28) },
    { 678, 1332, WSTAG_ENTRY(0x800A5E28) }, { 679, 1351, WSTAG_ENTRY(0x800A61F4) },
    { 680, 1352, WSTAG_ENTRY(0x800A5E2C) }, { 681, 1353, WSTAG_ENTRY(0x800A5E28) },
    { 682, 1354, WSTAG_ENTRY(0x800A5E28) }, { 683, 1355, WSTAG_ENTRY(0x800A5E28) },
    { 684, 1360, WSTAG_ENTRY(0x800A5E28) }, { 685, 1386, WSTAG_ENTRY(0x800A5E74) },
    { 686, 1390, WSTAG_ENTRY(0x800A7460) }, { 687, 1394, WSTAG_ENTRY(0x800A5E28) },
    { 688, 1398, WSTAG_ENTRY(0x800A5E80) }, { 689, 1406, WSTAG_ENTRY(0x800A5E28) },
    { 690, 1410, WSTAG_ENTRY(0x800A5E28) }, { 691, 1414, WSTAG_ENTRY(0x800A5E28) },
    { 692, 1418, WSTAG_ENTRY(0x800A5E28) }, { 693, 1422, WSTAG_ENTRY(0x800A5E28) },
    { 694, 1426, WSTAG_ENTRY(0x800A5E28) }, { 695, 1431, WSTAG_ENTRY(0x800A5E28) },
    { 696, 1435, WSTAG_ENTRY(0x800A5E28) }, { 697, 1439, WSTAG_ENTRY(0x800A5E28) },
    { 698, 1443, WSTAG_ENTRY(0x800A5E28) }, { 699, 1447, WSTAG_ENTRY(0x800A5E28) },
    { 700, 1451, WSTAG_ENTRY(0x800A5E80) }, { 701, 1455, WSTAG_ENTRY(0x800A5E28) },
    { 702, 1459, WSTAG_ENTRY(0x800A5E84) }, { 703, 1463, WSTAG_ENTRY(0x800A5E28) },
    { 704, 1467, WSTAG_ENTRY(0x800A5E28) }, { 705, 1471, WSTAG_ENTRY(0x800A5F54) },
    { 706, 1475, WSTAG_ENTRY(0x800A5F54) }, { 707, 1479, WSTAG_ENTRY(0x800A5E28) },
    { 708, 1483, WSTAG_ENTRY(0x800A5E28) }, { 709, 1487, WSTAG_ENTRY(0x800A5E28) },
    { 710, 1491, WSTAG_ENTRY(0x800A5E2C) }, { 711, 1495, WSTAG_ENTRY(0x800A5E68) },
    { 712, 1503, WSTAG_ENTRY(0x800A5E28) }, { 713, 1508, WSTAG_ENTRY(0x800A5E28) },
    { 714, 1516, WSTAG_ENTRY(0x800A5E28) }, { 715, 1524, WSTAG_ENTRY(0x800A5E28) },
    { 716, 1532, WSTAG_ENTRY(0x800A5E28) }, { 717, 1536, WSTAG_ENTRY(0x800A5E28) },
    { 718, 1543, WSTAG_ENTRY(0x800A5E28) }, { 719, 1552, WSTAG_ENTRY(0x800A5E28) },
    { 720, 1560, WSTAG_ENTRY(0x800A5E28) }, { 721, 1568, WSTAG_ENTRY(0x800A6024) },
    { 722, 1588, WSTAG_ENTRY(0x800A5EE4) }, { 723, 1620, WSTAG_ENTRY(0x800A6138) },
    { 724, 1622, WSTAG_ENTRY(0x800A62B8) }, { 725, 1625, WSTAG_ENTRY(0x800A5EA0) },
    { 726, 1627, WSTAG_ENTRY(0x800A5E28) }, { 727, 489, WSTAG_ENTRY(0x800A75B0) },
    { 728, 490, WSTAG_ENTRY(0x800A66B0) }, { 729, 491, WSTAG_ENTRY(0x800A631C) },
    { 730, 1631, WSTAG_ENTRY(0x800A6654) }, { 731, 1635, WSTAG_ENTRY(0x800A69CC) },
    { 732, 1639, WSTAG_ENTRY(0x800A6B44) }, { 733, 1643, WSTAG_ENTRY(0x800A6418) },
    { 734, 1647, WSTAG_ENTRY(0x800A6184) }, { 735, 1651, WSTAG_ENTRY(0x800A6160) },
    { 736, 1655, WSTAG_ENTRY(0x800A5F50) }, { 737, 1659, WSTAG_ENTRY(0x800A5F58) },
    { 738, 1665, WSTAG_ENTRY(0x800A5F50) }, { 739, 1669, WSTAG_ENTRY(0x800A5F50) },
    { 740, 1673, WSTAG_ENTRY(0x800A5F50) }, { 741, 1697, WSTAG_ENTRY(0x800A5F50) },
    { 742, 1701, WSTAG_ENTRY(0x800A5F50) }, { 743, 1705, WSTAG_ENTRY(0x800A5F50) },
    { 744, 1709, WSTAG_ENTRY(0x800A5F54) }, { 745, 1713, WSTAG_ENTRY(0x800A5F54) },
    { 746, 1677, WSTAG_ENTRY(0x800A5F54) }, { 747, 1690, WSTAG_ENTRY(0x800A5F54) },
    { 748, 1691, WSTAG_ENTRY(0x800A5F54) }, { 749, 1692, WSTAG_ENTRY(0x800A5F54) },
    { 750, 1693, WSTAG_ENTRY(0x800A5F54) }, { 0, 0, NULL },
};

FieldstgTimer fieldstg_timer = { 0, 0, fieldstg_timer_reset, fieldstg_find_actor };
FieldstgEventFuncs fieldstg_event_funcs = { fieldstg_to_screen_pos, fieldstg_wait_frames, fieldstg_wait_anim_done, fieldstg_wait_walk_done, fieldstg_player_clear_unk_10C };

FieldstgScriptObject fieldstg_script_objects[58] = {
    { 800, SLOT_FUNC(Object *(*)(s32), 0x800A6F8C), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6F40) },
    { 801, SLOT_FUNC(Object *(*)(s32), 0x800A749C), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A7450) },
    { 802, SLOT_FUNC(Object *(*)(s32), 0x800A635C), NULL },
    { 803, (Object *(*)(s32))fieldstg_icon_start, (void (*)(void *, s32, s32))fieldstg_icon_message },
    { 804, (Object *(*)(s32))fieldstg_icon_start, (void (*)(void *, s32, s32))fieldstg_icon_message },
    { 805, (Object *(*)(s32))fieldstg_icon_start, (void (*)(void *, s32, s32))fieldstg_icon_message },
    { 806, (Object *(*)(s32))fieldstg_icon_start, (void (*)(void *, s32, s32))fieldstg_icon_message },
    { 807, SLOT_FUNC(Object *(*)(s32), 0x800A6510), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6540) },
    { 808, SLOT_FUNC(Object *(*)(s32), 0x800A625C), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6224) },
    { 809, SLOT_FUNC(Object *(*)(s32), 0x800A6108), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A60D0) },
    { 810, SLOT_FUNC(Object *(*)(s32), 0x800A61FC), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6148) },
    { 811, SLOT_FUNC(Object *(*)(s32), 0x800A6A98), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A698C) },
    { 812, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6E88) },
    { 813, (Object *(*)(s32))fieldstg_effects_start, (void (*)(void *, s32, s32))fieldstg_effects_message },
    { 814, SLOT_FUNC(Object *(*)(s32), 0x800A63AC), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A627C) },
    { 815, SLOT_FUNC(Object *(*)(s32), 0x800A61E0), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6174) },
    { 816, SLOT_FUNC(Object *(*)(s32), 0x800A6230), NULL }, { 817, SLOT_FUNC(Object *(*)(s32), 0x800A6264), NULL },
    { 818, SLOT_FUNC(Object *(*)(s32), 0x800A629C), NULL }, { 819, SLOT_FUNC(Object *(*)(s32), 0x800A62D4), NULL },
    { 820, SLOT_FUNC(Object *(*)(s32), 0x800A630C), NULL }, { 821, SLOT_FUNC(Object *(*)(s32), 0x800A6344), NULL },
    { 822, SLOT_FUNC(Object *(*)(s32), 0x800A637C), NULL }, { 823, SLOT_FUNC(Object *(*)(s32), 0x800A63B4), NULL },
    { 824, SLOT_FUNC(Object *(*)(s32), 0x800A63EC), NULL }, { 825, SLOT_FUNC(Object *(*)(s32), 0x800A6424), NULL },
    { 826, (Object *(*)(s32))fieldstg_lift_create, (void (*)(void *, s32, s32))fieldstg_lift_message },
    { 827, SLOT_FUNC(Object *(*)(s32), 0x800A628C), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6218) },
    { 828, SLOT_FUNC(Object *(*)(s32), 0x800A6010), NULL },
    { 829, SLOT_FUNC(Object *(*)(s32), 0x800A6028), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A5FF0) },
    { 830, SLOT_FUNC(Object *(*)(s32), 0x800A5FC8), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A5F90) },
    { 831, SLOT_FUNC(Object *(*)(s32), 0x800A65D4), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6598) },
    { 832, SLOT_FUNC(Object *(*)(s32), 0x800A6B58), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6B1C) },
    { 833, SLOT_FUNC(Object *(*)(s32), 0x800A5F60), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A5F90) },
    { 834, SLOT_FUNC(Object *(*)(s32), 0x800A5F60), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A5F90) },
    { 835, SLOT_FUNC(Object *(*)(s32), 0x800A61D8), NULL }, { 836, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6180) },
    { 837, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6174) }, { 838, SLOT_FUNC(Object *(*)(s32), 0x800A73E4), NULL },
    { 839, SLOT_FUNC(Object *(*)(s32), 0x800A6510), NULL }, { 840, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A63C4) },
    { 841, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A5F64) }, { 842, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6178) },
    { 843, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6178) }, { 844, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6178) },
    { 845, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A67BC) }, { 846, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6170) },
    { 847, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6304) },
    { 848, SLOT_FUNC(Object *(*)(s32), 0x800A6894), SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6780) },
    { 849, SLOT_FUNC(Object *(*)(s32), 0x800A6198), NULL }, { 850, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6018) },
    { 851, NULL, SLOT_FUNC(void (*)(void *, s32, s32), 0x800A6018) }, { 852, SLOT_FUNC(Object *(*)(s32), 0x800A72CC), NULL },
    { 853, SLOT_FUNC(Object *(*)(s32), 0x800A5FB4), NULL }, { 854, SLOT_FUNC(Object *(*)(s32), 0x800A609C), NULL },
    { 855, SLOT_FUNC(Object *(*)(s32), 0x800A639C), NULL }, { 0, NULL, NULL },
    { 0, NULL, (void (*)(void *, s32, s32))fieldstg_encounter_reset },
};

void (*fieldstg_encounter_step_func)(void) = fieldstg_encounter_step;
void (*fieldstg_start_listed_battle_func)() = fieldstg_start_listed_battle;
void (*fieldstg_encounter_start_func)(void) = fieldstg_encounter_start;
s32 fieldstg_encounter_rates[6] = { 2000, 3, 4, 6, 9, 18 };

FieldstgAttr fieldstg_attr = {
    .set_file = fieldstg_attr_set_file,
    .get = fieldstg_attr_get,
    .get_step = fieldstg_attr_get_step,
    .get_flat_step = fieldstg_attr_get_flat_step,
    .init_layer = fieldstg_attr_init_layer,
    .set_layer = fieldstg_attr_set_layer,
    .is_free = (u8 (*)(FieldstgPos *))fieldstg_attr_is_free,
};

s32 fieldstg_actor_boxes_frame = -1;

FieldstgPos fieldstg_slope_steps[7][8] = {
    { { 0, 2048 }, { -2896, 1448 }, { -4096, 0 }, { -2896, -1448 }, { 0, -2048 }, { 2896, -1448 }, { 4096, 0 }, { 2896, 1448 } },
    { { 599, 3405 }, { -2048, 3368 }, { -3496, 1358 }, { -2896, -1448 }, { -600, -3405 }, { 2048, -3368 }, { 3495, -1358 }, { 2896, 1448 } },
    { { 155, 2865 }, { -2676, 2606 }, { -3940, 818 }, { -2896, -1448 }, { -156, -2866 }, { 2676, -2606 }, { 3939, -819 }, { 2896, 1448 } },
    { { 68, 2616 }, { -2799, 2254 }, { -4027, 569 }, { -2896, -1448 }, { -69, -2618 }, { 2797, -2254 }, { 4026, -571 }, { 2896, 1448 } },
    { { 273, 3081 }, { -2509, 2911 }, { -3822, 1034 }, { -2896, -1448 }, { -274, -3082 }, { 2509, -2911 }, { 3821, -1035 }, { 2896, 1448 } },
    { { 344, 3178 }, { -2409, 3047 }, { -3751, 1130 }, { -2896, -1448 }, { -345, -3179 }, { 2409, -3047 }, { 3750, -1132 }, { 2896, 1448 } },
    { { 9, 2271 }, { -2882, 1764 }, { -4086, 224 }, { -2896, -1448 }, { -10, -2272 }, { 2882, -1764 }, { 4085, -225 }, { 2896, 1448 } },
};

u8 fieldstg_slope_mirror_dirs[8] = { 0, 7, 6, 5, 4, 3, 2, 1 };
