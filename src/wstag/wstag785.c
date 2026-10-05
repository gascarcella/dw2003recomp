#include "wstag.h"

/* WSTAG785: stage 0x2D8 (fieldstg_stages). */

extern WstagFuncs wstag785_funcs;
void wstag785_update(WstagObject *obj, WstagEventData *data);
/* An object over the sprites of types 1-4 (a gate that opens: types 4, 3 and 2 animate in turn). */
typedef struct Wstag785Gate {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 starts_open; /* starts open */
    /* 0x54 */ FieldstgSprite *sprite_4; /* type 4 */
    /* 0x58 */ FieldstgSprite *sprite_3; /* type 3 */
    /* 0x5C */ FieldstgSprite *sprite_2; /* type 2 */
    /* 0x60 */ FieldstgSprite *sprite_1; /* type 1 */
    /* 0x64 */ WstagAnim anim;
    /* 0x68 */ WstagAnim anim_1;
} Wstag785Gate; /* size 0x6C */

extern WstagAnimKey D_WSTAG785_800A7118[];
extern WstagAnimKey D_WSTAG785_800A7154[];
extern WstagAnimKey D_WSTAG785_800A7188[];
extern WstagAnimKey D_WSTAG785_800A71A4[];
const CVECTOR wstag785_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgVramPlace wstag785_vram_places[];
extern FieldstgPlacedActor *wstag785_actors[];
extern FieldstgSprite wstag785_sprites[];
extern FieldstgEventDef wstag785_events[];
void wstag785_gate_update(Wstag785Gate *obj);
/* A mark drawn over an actor (heap_objects.find(5, ...)), in the states wstag785_mark_message sets. */
typedef struct Wstag785Mark {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode;
    /* 0x54 */ FieldstgActor *actor;
} Wstag785Mark; /* size 0x58 */

void wstag785_mark_update(Wstag785Mark *obj);

s32 wstag785_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[anim->key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        anim->time -= step;
    }
    if (anim->time <= 0) {
        key++;
        anim->key++;
        anim->time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag785_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag785_gate_update(Wstag785Gate *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                obj->sprite_1 = sprite;
                break;
            case 2:
                obj->sprite_2 = sprite;
                break;
            case 3:
                obj->sprite_3 = sprite;
                break;
            case 4:
                obj->sprite_4 = sprite;
                break;
            }
        }
        obj->anim_1.key = 0;
        obj->anim_1.time = D_WSTAG785_800A71A4[0].time;
        if (obj->starts_open == 0) {
            obj->anim.key = 0;
            obj->anim.time = D_WSTAG785_800A7118[0].time;
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            spr = obj->sprite_4;
            frame = wstag785_anim_advance(&obj->anim, D_WSTAG785_800A7118, 1, 0);
            if (frame == 0xFF) {
                obj->base.set_step(obj, 1);
                spr->shown = 0;
                obj->anim.key = 0;
                obj->anim.time = D_WSTAG785_800A7154[0].time;
            } else {
                spr->frame = frame;
                spr->shown = 1;
            }
            break;
        case 1:
            spr = obj->sprite_3;
            spr->frame = wstag785_anim_advance(&obj->anim, D_WSTAG785_800A7154, 0, 0);
            spr->shown = 1;
            break;
        case 2:
            break;
        }
        spr = obj->sprite_1;
        spr->frame = wstag785_anim_advance(&obj->anim_1, D_WSTAG785_800A71A4, 0, 0);
        spr->shown = 1;
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->anim.key = 0;
            obj->anim.time = D_WSTAG785_800A7188[0].time;
            obj->anim_1.key = 0;
            obj->anim_1.time = D_WSTAG785_800A71A4[0].time;
            obj->base.next_step(obj);
        }
        spr = obj->sprite_2;
        spr->frame = wstag785_anim_advance(&obj->anim, D_WSTAG785_800A7188, 0, 0);
        spr->shown = 1;
        spr = obj->sprite_1;
        spr->frame = wstag785_anim_advance(&obj->anim_1, D_WSTAG785_800A71A4, 0, 0);
        spr->shown = 1;
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Wstag785Gate *wstag785_gate_new(void) {
    Wstag785Gate *obj = object_new(wstag785_gate_update, sizeof(Wstag785Gate), 0);

    obj->starts_open = 1;
    return obj;
}

Object *wstag785_gate_create(s32 arg0) {
    return object_create(wstag785_gate_update, sizeof(Wstag785Gate), 0, arg0);
}

void wstag785_mark_draw(Wstag785Mark *obj, s32 frame) {
    Sprite spr;
    GamestatePos pos;

    pos.x = obj->actor->pos.x >> 8;
    pos.y = (obj->actor->pos.y >> 8) - 0x18;
    sprite_init(&spr);
    spr.set_layer_id(0x1002, 2);
    spr.set_vram_pos(0x140, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(0x01C70001), frame, pos.x, pos.y);
}

void wstag785_mark_update(Wstag785Mark *obj) {
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            frame = 0x32;
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < 4) {
                break;
            }
            obj->base.next_step(obj);
        case 1:
            frame = 0x33;
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < 4) {
                break;
            }
            obj->base.next_step(obj);
        case 2:
            frame = 0x34;
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < 4) {
                break;
            }
            obj->base.next_step(obj);
        case 3:
            switch (obj->mode) {
            case 0:
            default:
                frame = 0x35;
                break;
            case 1:
                frame = ((obj->base.substep >> 3) & 1) + 0x38;
                obj->base.substep += gfx_module.funcs.get_frame_ticks();
                break;
            case 2:
                frame = 0x3C;
                break;
            case 3:
                frame = 0x3D;
                break;
            }
            break;
        }
        wstag785_mark_draw(obj, frame);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            frame = 0x36;
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep < 4) {
                break;
            }
            obj->base.next_step(obj);
        case 1:
            frame = 0x37;
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep >= 4) {
                obj->base.next_state(obj);
            }
            break;
        }
        wstag785_mark_draw(obj, frame);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag785_mark_create(s32 arg0) {
    return object_create(wstag785_mark_update, sizeof(Wstag785Mark), 0, arg0);
}

void wstag785_mark_message(void *arg0, s32 msg, s32 arg2) {
    Wstag785Mark *obj = arg0;

    switch (msg) {
    case 0x328:
        obj->mode = 2;
        break;
    case 0x329:
        obj->mode = 3;
        break;
    case 0x32A:
        obj->base.set_state(obj, OBJECT_STATE_DONE);
        break;
    }
    if (msg < 0x32A) {
        if (msg >= 0x328) {
            obj->actor = (FieldstgActor *)heap_objects.find(5, arg2, -1);
        }
    }
}

void wstag785_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (gamestate_data.progress) {
        case 0:
            data->event = fieldstg_event_start(1);
            break;
        case 1:
            data->object = wstag785_gate_new();
            data->event = fieldstg_event_start(3);
            break;
        }
        obj->base.next_state(obj);
        data->object = NULL;
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag785_start(void *arg0) {
    WstagObject *obj = object_new(wstag785_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag785_funcs.setup();
    return obj;
}

void wstag785_event_1_end(void) {
    gamestate_data.progress = 0x1;
}

void wstag785_setup(void) {
    fieldstg_stage.background_file = 0x1C6;
    fieldstg_stage.sprites = wstag785_sprites;
    fieldstg_stage.sprite_file = 0x01C70001;
    fieldstg_stage.mask_file = 0x3E2;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15000, 0x18300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag785_vram_places;
    fieldstg_stage.music = 0x1B;
    fieldstg_stage.sound = 0x606C0000;
    fieldstg_stage.actors = wstag785_actors;
    fieldstg_stage.color = wstag785_color;
    fieldstg_stage.events = wstag785_events;
    fieldstg_attr.set_file(0, 0x01C70000);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag785_setup(void);

s16 D_WSTAG785_800A6810[1015] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 480, 336),
    FIELDSTG_EVENT_PLACE(1, 640, 256),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_PLACE(11, 672, 240),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_PLACE(12, 704, 224),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_PLACE(13, 330, 289),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_PLACE(14, 389, 294),
    FIELDSTG_EVENT_ANIM(14, 3, 7),
    FIELDSTG_EVENT_PLACE(15, 310, 252),
    FIELDSTG_EVENT_ANIM(15, 3, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 480, 336, 1),
    FIELDSTG_EVENT_WALK(11, 512, 320, 1),
    FIELDSTG_EVENT_WALK(12, 544, 304, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_WALK(1, 464, 344, 1),
    FIELDSTG_EVENT_WALK(11, 480, 320, 1),
    FIELDSTG_EVENT_WALK(12, 528, 312, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_ANIM(11, 58, 3),
    FIELDSTG_EVENT_ANIM(12, 58, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 12),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(12, 512, 304, 3),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 12),
    FIELDSTG_EVENT_DIALOG(0, 2, 12, 0),
    FIELDSTG_EVENT_ANIM(11, 58, 3),
    FIELDSTG_EVENT_ANIM(12, 7, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 59, 3),
    FIELDSTG_EVENT_ANIM(12, 59, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 12),
    FIELDSTG_EVENT_ANIM(0x324, 805, 11),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_ANIM(0x324, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(11, 9, 1),
    FIELDSTG_EVENT_ANIM(12, 9, 1),
    FIELDSTG_EVENT_WAIT_ANIM(12),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 368, 344, 3),
    FIELDSTG_EVENT_WALK(11, 368, 344, 3),
    FIELDSTG_EVENT_WALK(12, 368, 344, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 292, 302, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_WALK(11, 324, 318, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_WALK(12, 356, 334, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(13, 9, 1),
    FIELDSTG_EVENT_WAIT_ANIM(13),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 13),
    FIELDSTG_EVENT_DIALOG(0, 4, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 9, 5),
    FIELDSTG_EVENT_ANIM(11, 9, 5),
    FIELDSTG_EVENT_ANIM(12, 9, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 13),
    FIELDSTG_EVENT_DIALOG(0, 5, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 11),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x324, 805, 11),
    FIELDSTG_EVENT_ANIM(0x325, 805, 12),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_ANIM(0x324, 806, 1),
    FIELDSTG_EVENT_ANIM(0x325, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 13),
    FIELDSTG_EVENT_DIALOG(0, 6, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 9, 5),
    FIELDSTG_EVENT_ANIM(11, 9, 5),
    FIELDSTG_EVENT_ANIM(12, 9, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_DIALOG(0, 7, 13, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_ANIM(1, 41, 5),
    FIELDSTG_EVENT_ANIM(11, 9, 5),
    FIELDSTG_EVENT_ANIM(12, 9, 5),
    FIELDSTG_EVENT_ANIM(0x323, 807, 1),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 11),
    FIELDSTG_EVENT_ANIM(11, 57, 3),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WALK(11, 244, 358, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 260, 318, 1),
    FIELDSTG_EVENT_WALK(12, 324, 350, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 11, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_DIALOG(0, 9, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 59, 1),
    FIELDSTG_EVENT_ANIM(0x327, 808, 12),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 57, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(0x327, 810, 1),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 11),
    FIELDSTG_EVENT_DIALOG(0, 10, 11, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_DIALOG(0, 11, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_ANIM(0x327, 809, 12),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(0x327, 810, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 11),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(11, 57, 1),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 11, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WALK(11, 324, 318, 5),
    FIELDSTG_EVENT_WALK(12, 356, 334, 5),
    FIELDSTG_EVENT_ANIM(0x323, 807, 1),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 11, 0),
    FIELDSTG_EVENT_ANIM(11, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(1, 292, 302, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 41, 5),
    FIELDSTG_EVENT_ANIM(11, 9, 5),
    FIELDSTG_EVENT_ANIM(12, 9, 5),
    FIELDSTG_EVENT_ANIM(0x323, 807, 1),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 11, 0),
    FIELDSTG_EVENT_ANIM(11, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 46, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 9, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 9, 5),
    FIELDSTG_EVENT_ANIM(11, 9, 5),
    FIELDSTG_EVENT_ANIM(12, 9, 5),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(13, 2, 1),
    FIELDSTG_EVENT_ANIM(0x343, 821, 1),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_GOTO_MAP(0x500, 100, 100, 0),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG785_800A7000[140] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 11),
    FIELDSTG_EVENT_PLACE(1, 292, 302),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(11, 324, 318),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_PLACE(12, 356, 334),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_PLACE(13, 330, 289),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_PLACE(14, 389, 294),
    FIELDSTG_EVENT_ANIM(14, 1, 7),
    FIELDSTG_EVENT_PLACE(15, 310, 252),
    FIELDSTG_EVENT_ANIM(15, 1, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 13, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 179, 245, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 3),
    FIELDSTG_EVENT_ANIM(12, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 4, 12, 1),
    FIELDSTG_EVENT_DIALOG(0, 3, 11, 2),
    FIELDSTG_EVENT_WALK(1, 61, 303, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 2),
    FIELDSTG_EVENT_ANIM(12, 7, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_GOTO_MAP(0x2D9, 100, 100, 0),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG785_800A7118[15] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 },
    { 11, 4 }, { 12, 4 }, { 13, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG785_800A7154[13] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 },
    { 11, 4 }, { 255, 0 },
};
WstagAnimKey D_WSTAG785_800A7188[7] = { { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 255, 0 } };
WstagAnimKey D_WSTAG785_800A71A4[7] = { { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 255, 0 } };
FieldstgVramPlace wstag785_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 412, 287, 368, 31, 320, 497 }, { 384, 256, 420, 297, 400, 41, 352, 497 },
    { 384, 256, 428, 297, 432, 41, 368, 497 }, { 320, 256, 374, 312, 216, 56, 320, 496 },
    { 384, 256, 438, 272, 472, 16, 352, 496 }, { 384, 256, 404, 287, 336, 31, 368, 496 },
};
u16 D_WSTAG785_800A7280[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A7288[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A7290[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A7298[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72A0[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72A8[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72B0[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72B8[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72C0[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72C8[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72D0[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG785_800A72D8[4] = { 0x6001, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG785_800A72E0 = { D_WSTAG785_800A7280, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A72F4 = { D_WSTAG785_800A7288, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7308 = { D_WSTAG785_800A7290, NULL, 11, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A731C = { D_WSTAG785_800A7298, NULL, 11, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7330 = { D_WSTAG785_800A72A0, NULL, 12, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7344 = { D_WSTAG785_800A72A8, NULL, 12, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7358 = { D_WSTAG785_800A72B0, NULL, 13, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A736C = { D_WSTAG785_800A72B8, NULL, 13, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7380 = { D_WSTAG785_800A72C0, NULL, 14, 8, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A7394 = { D_WSTAG785_800A72C8, NULL, 14, 8, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A73A8 = { D_WSTAG785_800A72D0, NULL, 15, 9, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG785_800A73BC = { D_WSTAG785_800A72D8, NULL, 15, 9, 0, 0, 1 };
FieldstgPlacedActor *wstag785_actors[13] = {
    &D_WSTAG785_800A72E0, &D_WSTAG785_800A72F4, &D_WSTAG785_800A7308, &D_WSTAG785_800A731C, &D_WSTAG785_800A7330,
    &D_WSTAG785_800A7344, &D_WSTAG785_800A7358, &D_WSTAG785_800A736C, &D_WSTAG785_800A7380, &D_WSTAG785_800A7394,
    &D_WSTAG785_800A73A8, &D_WSTAG785_800A73BC, NULL,
};
FieldstgSprite wstag785_sprites[26] = {
    { 0, 1, 0x64, 2, 0x4A, 2, 0, 5, 8, 0, 299, 232, 0, 0 },
    { 1, 0, 0x40, 2, 0x51, 2, 0, 0xB, 0xA, 0, 156, 254, 0, 0 },
    { 1, 0, 0x40, 2, 0x51, 2, 0, 0xB, 0xA, 0, 236, 358, 0, 0 },
    { 1, 0, 0x40, 2, 0x51, 2, 0, 0xB, 0xA, 0, 398, 358, 0, 0 },
    { 1, 0, 0x64, 2, 0x4B, 2, 0, 5, 8, 0, 369, 246, 0, 0 },
    { 0, 4, 0x64, 2, 0x46, 2, 0, 0xD, 4, 0, 299, 232, 0, 0 },
    { 0, 3, 0x64, 2, 0x47, 2, 0, 0xB, 8, 0, 299, 232, 0, 0 },
    { 0, 2, 0x64, 2, 0x48, 2, 0, 5, 8, 0, 299, 232, 0, 0 }, { 1, 0, 0x64, 2, 0x49, 2, 0, 5, 8, 0, 369, 246, 0, 0 },
    { 1, 0, 0x40, 2, 0x50, 0, 0, 0, 0, 0, 156, 254, 0, 0 }, { 1, 0, 0x40, 2, 0x50, 0, 0, 0, 0, 0, 236, 358, 0, 0 },
    { 1, 0, 0x40, 2, 0x50, 0, 0, 0, 0, 0, 398, 358, 0, 0 }, { 1, 0, 0x64, 2, 0x52, 2, 0, 3, 8, 0, 404, 239, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 402, 400, 408, 0 }, { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 241, 400, 407, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 160, 295, 303, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 289, 244, 265, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 289, 262, 292, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 305, 272, 300, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 321, 279, 308, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 337, 286, 316, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 353, 298, 324, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 369, 289, 316, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 385, 281, 308, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 399, 273, 300, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag785_funcs = { wstag785_setup };
FieldstgEventDef wstag785_events[3] = {
    { 1, D_WSTAG785_800A6810, 0x014A0001, NULL, wstag785_event_1_end },
    { 3, D_WSTAG785_800A7000, 0x014A0002, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
