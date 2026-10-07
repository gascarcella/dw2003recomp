#ifndef WSTAG_H
#define WSTAG_H

/* WSTAG###.PRO, the stage overlays (tier 2, loaded at 0x800A5DE0 while FIELDSTG stays resident): types and
 * declarations shared by their C files (src/wstag/wstag###.c). Each file is one stage's code and data: an
 * entry that FIELDSTG calls (fieldstg_stage.entry), a setup that fills fieldstg_stage and loads the
 * map attribute layers (fieldstg_attr), and the stage's own objects. Their .data (battle lists, VRAM places,
 * talk flags and texts, placed actors, sprites, map events, event scripts and the event table, the stage's
 * function table) is defined at the end of each file (tools/wstag_data.py, `data` in config/wstag_c.txt), in
 * address order; the functions above it declare what they use extern. */

#include "common.h"
#include "object.h"
#include "heap.h"
#include "gamestate.h"
#include "records.h"
#include "gfx.h"
#include "sound.h"
#include "cdload.h"
#include "window_anim_type.h"
#include "fieldstg.h"
#include "message.h"

/* The first sprite of a type in fieldstg_stage.sprites (FIELDSTG defines it returning void: its tail call to
 * fieldstg_sprites_find_next leaves the sprite in v0, which the stages use). */
FieldstgSprite *fieldstg_sprites_find_first(s32 type);

/* Starts the stage's listed battle 5 (an event's FieldstgEventDef.start in several stages). */
Object *fieldstg_start_battle_5(void); /* returns no child object */

/* A layer draw callback as GfxLayer.add_callback takes it; the stages' draw functions take their own object type as
 * the first argument and are passed through this cast. */
typedef void (*WstagDrawCallback)(void *, GfxLayer *, s32);

/* The stage's object (the entry creates it, with the field manager it was called with). */
typedef struct WstagObject {
    /* 0x00 */ Object base;
    /* 0x50 */ void *manager; /* the entry's argument (the field manager) */
} WstagObject; /* size 0x54 */

/* The stage's function table, the last word of its .data: [0] the setup. */
typedef struct WstagFuncs {
    /* 0x0 */ void (*setup)(void);
} WstagFuncs; /* size 0x4 */

/* A map exit (an entry of fieldstg_stage.map_events), one node of a list (WstagExits). */
typedef struct WstagExit {
    /* 0x0 */ u16 stage; /* -> FieldstgMapEvent.param (the stage it leads to) */
    /* 0x2 */ u16 route; /* -> route */
    /* 0x4 */ u16 room;  /* -> room */
    /* 0x6 */ u16 x;     /* -> x */
    /* 0x8 */ u16 y;     /* -> y */
    /* 0xA */ u16 dir;   /* -> dir */
    /* 0xC */ struct WstagExit *next;
} WstagExit; /* size 0x10 */

/* The exits of the stage when it is entered at (route, room) (gamestate_data.route, room: a position along a
 * route of maps, which FieldstgMapEvent.route/room set on arrival); stages keep a NULL-terminated list of them. */
typedef struct WstagExits {
    /* 0x0 */ s16 route;
    /* 0x2 */ s16 room;
    /* 0x4 */ WstagExit *exits;
} WstagExits; /* size 0x8 */

/* A key of a sprite animation: frame (0xFF: back to the first key) and how long it is shown. */
typedef struct WstagAnimKey {
    /* 0x0 */ s16 frame;
    /* 0x2 */ s16 time;
} WstagAnimKey; /* size 0x4 */

/* The same with a byte time (WSTAG795's wstag795_sprite_anim_advance_b and its copies). */
typedef struct WstagAnimKeyB {
    /* 0x0 */ s16 frame;
    /* 0x2 */ u8 time;
    /* 0x3 */ u8 unk_3;
} WstagAnimKeyB; /* size 0x4 */

/* A running sprite animation: its key and the time left. */
typedef struct WstagAnim {
    /* 0x0 */ s16 key;
    /* 0x2 */ s16 time;
} WstagAnim; /* size 0x4 */

/* An object that runs three sprite animations (WSTAG635's wstag635_anim_update and its copies): it sets the
 * frame of the stage's sprites of type 1, 2 and 3. */
typedef struct WstagAnimObject {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagAnim anims[3];
} WstagAnimObject; /* size 0x5C */

/* The same with four animations (sprite types 1-4; WSTAG310's wstag310_anim4_update and its copy). */
typedef struct WstagAnim4Object {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagAnim anims[4];
} WstagAnim4Object; /* size 0x60 */

/* The same with two animations (sprite types 1 and 2; WSTAG232's wstag232_anim2_update and its copies). */
typedef struct WstagAnim2Object {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagAnim anims[2];
} WstagAnim2Object; /* size 0x58 */

/* The same with one animation (sprite type 1; WSTAG405's wstag405_anim1_update and its copies). */
typedef struct WstagAnim1Object {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagAnim anim;
} WstagAnim1Object; /* size 0x54 */

/* A sprite of the stage (fieldstg_stage.sprites) and its animation. */
typedef struct WstagSpriteAnim {
    /* 0x0 */ FieldstgSprite *sprite;
    /* 0x4 */ WstagAnim anim;
} WstagSpriteAnim; /* size 0x8 */

/* An object that plays the animation of the sprite of type 1 once when it is told to (WSTAG455's
 * wstag455_sprite_anim1_update, WSTAG820's wstag820_sprite_anim1_update). */
typedef struct WstagSpriteAnim1Object {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagSpriteAnim sprite;
} WstagSpriteAnim1Object; /* size 0x58 */

/* An object that places three sprites (types 1-3) at a position and plays their animations once (WSTAG226's
 * wstag226_sprite_anim_update and its copies). */
typedef struct WstagSpriteAnimObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 x;      /* x */
    /* 0x54 */ s32 y;      /* y */
    /* 0x58 */ WstagSpriteAnim sprites[3];
} WstagSpriteAnimObject; /* size 0x70 */

/* One of the sprite animations an object runs (WSTAG620's wstag620_anim_slot_update; WSTAG625 has its own with an
 * s16 mode): the animation (an index into the stage's table of WstagAnimEntry, 0: none) and the sprite it sets. */
typedef struct WstagAnimSlot {
    /* 0x0 */ s16 entry; /* the animation (index into the stage's WstagAnimEntry table), 0: none */
    /* 0x2 */ u8 sound_pending; /* 1: plays a sound at frame 0x34 (once) */
    /* 0x3 */ u8 sets_sprite; /* 0: the animation sets the sprite's frame, else its sprite */
    /* 0x4 */ FieldstgSprite *sprite;
    /* 0x8 */ WstagAnim anim;
} WstagAnimSlot; /* size 0xC */

/* An animation of a stage's table for WstagAnimSlot (per slot, by WstagAnimSlot.entry). */
typedef struct WstagAnimEntry {
    /* 0x0 */ WstagAnimKey *keys;
    /* 0x4 */ s16 once; /* played once, then `next` */
    /* 0x6 */ s16 next; /* 0: the slot stops */
} WstagAnimEntry; /* size 0x8 */

/* A gate over six sprites (WSTAG310's wstag310_gate_update, WSTAG311's wstag311_gate_update and
 * wstag311_gate2_update): the first two open (mode 1) and close (mode 3) once, the other four show while it is
 * open (mode 2). */
typedef struct WstagGateObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode; /* 0: closed, 1: opening, 2: open, 3: closing */
    /* 0x54 */ WstagSpriteAnim sprites[6];
} WstagGateObject; /* size 0x84 */

/* An object that shows a sprite (looping its animation) until told to stop (its stop function sets mode 2 and a
 * count of frames), then plays its closing animation once (WSTAG460's wstag460_glow_update, WSTAG526's
 * wstag526_glow_update and wstag526_glow3_update). */
typedef struct WstagGlowObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 mode; /* 0: hidden, 1: on, 2: stopping */
    /* 0x52 */ s16 stop_delay; /* frames left before it closes */
    /* 0x54 */ WstagSpriteAnim sprite;
} WstagGlowObject; /* size 0x5C */

/* The same over two sprites (WSTAG460's wstag460_glow2_update, WSTAG526's wstag526_glow2_update). */
typedef struct WstagGlow2Object {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 mode;       /* as WstagGlowObject */
    /* 0x52 */ s16 stop_delay;
    /* 0x54 */ WstagSpriteAnim sprites[2];
} WstagGlow2Object; /* size 0x64 */

/* An object that plays a sprite's animation (type 1) once with a sound when it is told to (WSTAG305's
 * wstag305_sound_anim_update and its copy). */
typedef struct WstagSoundAnimObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 voice;  /* the sound's voice */
    /* 0x52 */ s16 unk_52;
    /* 0x54 */ WstagSpriteAnim sprite;
} WstagSoundAnimObject; /* size 0x5C */

/* A position in a stage's tables. */
typedef struct WstagPos {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} WstagPos; /* size 0x4 */

/* The data of a stage object that starts an event and keeps one more object. */
typedef struct WstagEventData {
    /* 0x0 */ FieldstgEvent *event;
    /* 0x4 */ void *object; /* the stage's other object (per stage) */
} WstagEventData; /* size 0x8 */

/* The same the other way round: one more object, then the event (WSTAG306, WSTAG406). */
typedef struct WstagObjEventData {
    /* 0x0 */ void *object;
    /* 0x4 */ FieldstgEvent *event;
} WstagObjEventData; /* size 0x8 */

/* A sprite frame drawn by a stage object, and its palette (Sprite.set_palette). */
typedef struct WstagFrame {
    /* 0x0 */ s16 frame; /* 0: not drawn */
    /* 0x2 */ s16 palette;
} WstagFrame; /* size 0x4 */

/* An object that draws two sprites at a position, one looping, one played once when it is told to (WSTAG310's
 * wstag310_two_sprite2_update and its copies). */
typedef struct WstagTwoSpriteObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 frame; /* first frame of the sprite bank */
    /* 0x54 */ s32 x;      /* x */
    /* 0x58 */ s32 y;      /* y */
    /* 0x5C */ WstagAnim frame_anim;   /* the second sprite's frame animation (played once, added to frame) */
    /* 0x60 */ WstagAnim palette_anim; /* the first sprite's palette animation (loops) */
    /* 0x64 */ WstagFrame sprites[2];
} WstagTwoSpriteObject; /* size 0x6C */

/* An object that plays two animations once at a position (WSTAG740's wstag740_two_anim_update and its copy; a
 * WstagTwoSpriteObject with its animations as an array). */
typedef struct WstagTwoAnimObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unk_50;
    /* 0x54 */ s32 x;      /* x */
    /* 0x58 */ s32 y;      /* y */
    /* 0x5C */ WstagAnim anims[2];
    /* 0x64 */ WstagFrame sprites[2];
} WstagTwoAnimObject; /* size 0x6C */

/* The data of WSTAG740's wstag740_event_9000_update (and its copy): the objects it places on three actors. */
typedef struct WstagTwoAnimData {
    /* 0x0 */ WstagTwoAnimObject *objs[3];
} WstagTwoAnimData; /* size 0xC */

/* An object over the sprites of types 1 and 2 that opens when it is told to (WSTAG795's wstag795_door_update and
 * its copies in WSTAG800 and WSTAG810; keys WstagAnimKeyB). */
typedef struct WstagDoorObject {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 opening; /* opening */
    /* 0x54 */ s32 starts_open; /* starts open */
    /* 0x58 */ WstagSpriteAnim sprites[2];
} WstagDoorObject; /* size 0x68 */

/* A WstagTwoSpriteObject a stage object places (WSTAG735's D_WSTAG735_800A67DC, 10 of them). */
typedef struct WstagSpawn {
    /* 0x0 */ s16 frame;
    /* 0x2 */ s16 condition; /* 0: placed, 2: placed from progress 0x28 (WSTAG311), else not */
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} WstagSpawn; /* size 0xC */

/* The data of that stage object (WSTAG735, WSTAG736). */
typedef struct WstagSpawnData {
    /* 0x00 */ WstagTwoSpriteObject *objs[10];
    /* 0x28 */ FieldstgEvent *event;
} WstagSpawnData; /* size 0x2C */

/* The same with one more object first and five placed objects (WSTAG815, WSTAG745, WSTAG746). */
typedef struct WstagObjSpawnData {
    /* 0x00 */ void *object;
    /* 0x04 */ WstagTwoSpriteObject *objs[5];
    /* 0x18 */ FieldstgEvent *event;
} WstagObjSpawnData; /* size 0x1C */

/* Twenty placed objects, one more object and an event (WSTAG745, WSTAG746). */
typedef struct WstagObjSpawn20Data {
    /* 0x00 */ WstagTwoSpriteObject *objs[20];
    /* 0x50 */ void *object;
    /* 0x54 */ FieldstgEvent *event;
} WstagObjSpawn20Data; /* size 0x58 */

/* A key of an animation made of sequences (WSTAG480's wstag480_seq_anim_advance and its copy). */
typedef struct WstagSeqKey {
    /* 0x0 */ u8 sprite; /* -> the sprite's sprite */
    /* 0x1 */ u8 time;
    /* 0x2 */ u8 frame; /* -> the sprite's frame */
    /* 0x3 */ u8 last;  /* the last key of its sequence */
} WstagSeqKey; /* size 0x4 */

/* A running animation of sequences (a NULL-terminated list of key lists). */
typedef struct WstagSeqAnim {
    /* 0x0 */ s16 seq;
    /* 0x2 */ s16 key;
    /* 0x4 */ s32 time;
} WstagSeqAnim; /* size 0x8 */

/* An object that runs five of them for the sprites of types 1-5 (WSTAG480, WSTAG481). */
typedef struct WstagSeqObject {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagSeqAnim anims[5];
} WstagSeqObject; /* size 0x78 */

/* A lift (WSTAG261's wstag261_lift_update and its copy in WSTAG934): it moves two sprites (types 3 and 2) and the
 * player up or down by 0x7F (down: which way; FIELDSTG's FieldstgLift). */
typedef struct WstagLiftObject {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgSprite *sprite_3; /* the sprite of type 3 */
    /* 0x54 */ FieldstgSprite *sprite_2; /* the sprite of type 2 */
    /* 0x58 */ s16 down; /* the next move goes down */
    /* 0x5A */ s16 timer;  /* timer */
    /* 0x5C */ s16 step;   /* step */
    /* 0x5E */ s16 unk_5E;
    /* 0x60 */ s16 start_y_3; /* sprite_3's y at the start of the move */
    /* 0x62 */ s16 start_y_2; /* sprite_2's */
    /* 0x64 */ s32 start_player_y; /* the player's */
    /* 0x68 */ s16 low_y_3; /* sprite_3's y in the data: the lower position */
    /* 0x6A */ s16 low_y_2; /* sprite_2's */
} WstagLiftObject; /* size 0x6C */

/* A question with two answers, each starting an event (FIELDSTG's fieldstg_choice_update without the text table:
 * WSTAG210, 220, 270, 280). States: 0 create the windows, 1 sub-states: 0/1 fade in and show the texts, 2 move the
 * cursor until the button, 10/11 close and fade out, 3 start the answer's event, 4 wait for it, then end. */
typedef struct WstagChoice {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 cursor; /* the answer */
    /* 0x54 */ WindowAnim fade;
} WstagChoice; /* size 0x64 */

/* Data block of WstagChoice. */
typedef struct WstagChoiceData {
    /* 0x00 */ MessageWindow *question;
    /* 0x04 */ MessageWindow *answers[2];
    /* 0x0C */ MessageCursor *cursor;
    /* 0x10 */ FieldstgEvent *event; /* the answer's event */
} WstagChoiceData; /* size 0x14 */

/* The stage's function table when an object reaches the fade pair through it (WSTAG924, WSTAG935; other stages'
 * .data declares the pair as two pointers after WstagFuncs). */
typedef struct WstagFadeFuncs {
    /* 0x0 */ void (*setup)(void);
    /* 0x4 */ void (*fade_start)(WindowAnim *fade, s32 in);
    /* 0x8 */ s32 (*fade_update)(WindowAnim *fade);
} WstagFadeFuncs; /* size 0xC */

/* A list of answers (count of them, up to 8) with a text for the chosen one (WSTAG210, 924, 935). States: 0 create
 * the windows, 1 sub-states: 0/1 fade the list in, 2 move the cursor, 3/4 show the entry's text, 5 page through it,
 * 6/7 close it, 10/11 close the list (button 13 on the last entry or button 14); 2 fade once. */
typedef struct WstagMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 count;
    /* 0x54 */ s32 cursor;
    /* 0x58 */ s32 arrow;      /* the text's "more" arrow is drawn */
    /* 0x5C */ s32 arrow_frame; /* 0-4 */
    /* 0x60 */ s32 arrow_time;  /* gfx time of the last frame change */
    /* 0x64 */ WindowAnim fade;      /* the list */
    /* 0x74 */ WindowAnim text_fade; /* the text */
} WstagMenu; /* size 0x84 */

/* Data block of WstagMenu. */
typedef struct WstagMenuData {
    /* 0x00 */ MessageWindow *items[8];
    /* 0x20 */ MessageCursor *cursor;
    /* 0x24 */ MessageWindow *text;
} WstagMenuData; /* size 0x28 */

#endif /* WSTAG_H */
