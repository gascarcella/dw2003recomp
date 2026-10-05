#include "wstag.h"
#include "pad.h"

/* WSTAG415: stage 0x22D (fieldstg_stages). */

/* A position moving by a speed (24.8), and an animation (its functions take this, not the object). */
typedef struct Wstag415Motion {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 vx;
    /* 0x0C */ s32 vy;
    /* 0x10 */ s16 fall_time; /* frames falling (Wstag415Diver) */
    /* 0x12 */ s16 mode; /* Wstag415Flyer: bounced; Wstag415Diver: the mode (messages 0x331-0x334) */
    /* 0x14 */ WstagAnim anim;
} Wstag415Motion; /* size 0x18 */

/* A sprite that flies along one of ten paths (D_WSTAG415_800A7350, wstag415_flyer_create_0 and the next nine), drawn
 * through the layer's list; it is deleted below y 0x245. */
typedef struct Wstag415Flyer {
    /* 0x00 */ Object base; /* base.key1: the path */
    /* 0x50 */ s32 frame; /* frame, 0: none */
    /* 0x54 */ s32 x; /* x */
    /* 0x58 */ s32 y; /* y */
    /* 0x5C */ Wstag415Motion motion;
} Wstag415Flyer; /* size 0x74 */

/* A path of Wstag415Flyer. */
typedef struct Wstag415Path {
    /* 0x0 */ WstagAnimKey *keys;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
    /* 0x8 */ s16 floor;   /* where it bounces once */
    /* 0xA */ s16 vx;      /* 24.8; tripled by the bounce */
    /* 0xC */ s16 gravity; /* per frame */
    /* 0xE */ s16 bounce;  /* the bounce divides the speed by this; 0: no bounce */
} Wstag415Path; /* size 0x10 */

/* The sprite of type 1, which falls in and plays animations on messages 0x331-0x334 (wstag415_diver_create_0). */
typedef struct Wstag415Diver {
    /* 0x00 */ Object base; /* base.key1: starts on the ground */
    /* 0x50 */ s32 frame; /* frame */
    /* 0x54 */ s32 x; /* x */
    /* 0x58 */ s32 y; /* y */
    /* 0x5C */ FieldstgSprite *sprite;
    /* 0x60 */ Wstag415Motion motion;
} Wstag415Diver; /* size 0x78 */

/* A scrolling background of two images (wstag415_sea_create); message 0x336 slows it down to a stop. */
typedef struct Wstag415Sea {
    /* 0x00 */ Object base; /* base.key1: starts stopping */
    /* 0x50 */ s32 x; /* x, 24.8 */
    /* 0x54 */ s32 y; /* y, 24.8 */
    /* 0x58 */ s32 vx; /* x speed */
    /* 0x5C */ s32 vy; /* y speed */
} Wstag415Sea; /* size 0x60 */

/* The data of the stage object (wstag415_update). */
typedef struct Wstag415Data {
    /* 0x0 */ Wstag415Sea *sea;
    /* 0x4 */ FieldstgEvent *event;
    /* 0x8 */ Wstag415Diver *diver;
} Wstag415Data; /* size 0xC */

extern Wstag415Path D_WSTAG415_800A7350[];
extern WstagAnimKey D_WSTAG415_800A73F0[];
extern WstagAnimKey D_WSTAG415_800A7418[];
extern WstagAnimKey D_WSTAG415_800A7428[];
extern WstagFuncs wstag415_funcs;
extern FieldstgBattleLists wstag415_battle_lists;
extern FieldstgVramPlace wstag415_vram_places[];
extern FieldstgPlacedActor *wstag415_actors[];
extern FieldstgSprite wstag415_sprites[];
extern FieldstgEventDef wstag415_events[];
void wstag415_update(WstagObject *obj, Wstag415Data *data);

s32 wstag415_motion_anim_loop(Wstag415Motion *m, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[m->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        m->anim.time -= step;
    }
    if (m->anim.time <= 0) {
        key++;
        m->anim.key++;
        m->anim.time += key->time;
        if (key->frame == 0xFF) {
            key = keys;
            m->anim.key = 0;
            m->anim.time += key->time;
        }
        wstag415_motion_anim_loop(m, keys, depth + 1);
    }
    return key->frame;
}

void wstag415_flyer_draw(Wstag415Flyer *obj, GfxLayer *layer, s32 which) {
    Sprite spr;

    if (obj->base.state == OBJECT_STATE_RUN) {
        sprite_init(&spr);
        spr.set_vram_pos(0x140, 0x100);
        spr.set_layer(layer, 4);
        spr.set_palette(0);
        spr.set_clut8_pos(0, 0x1F0);
        spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), obj->frame, obj->x, obj->y);
    }
}

void wstag415_flyer_update(Wstag415Flyer *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 accel;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->motion.x = D_WSTAG415_800A7350[obj->base.key1].x << 8;
        obj->motion.y = D_WSTAG415_800A7350[obj->base.key1].y << 8;
        obj->motion.mode = 0;
        obj->motion.vy = 0;
        obj->motion.vx = D_WSTAG415_800A7350[obj->base.key1].vx;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->frame = wstag415_motion_anim_loop(&obj->motion, D_WSTAG415_800A7350[obj->base.key1].keys, 0);
        accel = D_WSTAG415_800A7350[obj->base.key1].gravity * gfx_module.funcs.get_frame_ticks();
        obj->motion.x += obj->motion.vx;
        obj->motion.vy += accel;
        obj->x = obj->motion.x >> 8;
        obj->motion.y += obj->motion.vy;
        obj->y = obj->motion.y >> 8;
        if (D_WSTAG415_800A7350[obj->base.key1].bounce != 0 && obj->motion.mode == 0) {
            if (obj->y >= D_WSTAG415_800A7350[obj->base.key1].floor) {
                obj->motion.y = D_WSTAG415_800A7350[obj->base.key1].floor << 8;
                obj->y = D_WSTAG415_800A7350[obj->base.key1].floor;
                obj->motion.vy = -(obj->motion.vy / D_WSTAG415_800A7350[obj->base.key1].bounce);
                obj->motion.vx = D_WSTAG415_800A7350[obj->base.key1].vx * 3;
                obj->motion.mode = 1;
                sound_module.play(0x40012);
            }
        }
        if (obj->frame != 0) {
            layer->add_callback(layer, wstag415_flyer_draw, obj, obj->y, 0);
        }
        if (obj->y >= 0x245) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Wstag415Flyer *wstag415_flyer_create_0(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 0;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_1(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 1;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_2(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 2;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_3(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 3;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_4(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 4;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_5(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 5;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_6(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 6;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_7(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 7;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_8(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 8;
    return obj;
}

Wstag415Flyer *wstag415_flyer_create_9(s32 arg0) {
    Wstag415Flyer *obj = object_create(wstag415_flyer_update, sizeof(Wstag415Flyer), 0, arg0);

    obj->base.key1 = 9;
    return obj;
}

s32 wstag415_motion_anim_hold(Wstag415Motion *m, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[m->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        m->anim.time -= step;
    }
    if (m->anim.time <= 0) {
        key++;
        m->anim.key++;
        m->anim.time += key->time;
        if (key->frame == 0xFF) {
            key--;
            m->anim.key--;
            m->anim.time += key->time;
        }
        wstag415_motion_anim_hold(m, keys, depth + 1);
    }
    return key->frame;
}

void wstag415_diver_update(Wstag415Diver *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->base.key1 == 0) {
            obj->motion.x = 0x9600;
            obj->motion.y = 0xE600;
            obj->motion.mode = 0;
            obj->motion.vy = 0;
            obj->motion.vx = 0;
            obj->motion.fall_time = 0;
        } else {
            obj->motion.x = 0x9600;
            obj->motion.y = 0x1AE00;
            obj->motion.fall_time = 0;
            obj->motion.mode = 0;
            obj->x = obj->motion.x >> 8;
            obj->y = obj->motion.y >> 8;
        }
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprite = sprite;
                sprite->x = obj->x;
                sprite->x = obj->y;
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->motion.mode) {
        case 0:
            obj->frame = 0x32;
            break;
        case 1:
            obj->frame = 0x32;
            obj->motion.vy += gfx_module.funcs.get_frame_ticks() * 0x30;
            obj->motion.x += obj->motion.vx;
            obj->motion.y += obj->motion.vy;
            obj->x = obj->motion.x >> 8;
            obj->y = obj->motion.y >> 8;
            if (obj->y >= 0x1AE) {
                obj->motion.y = 0x1AE00;
                obj->y = 0x1AE;
                if (obj->motion.vy > 0x200) {
                    sound_module.play(0x8004474A);
                }
                obj->motion.vy = -(obj->motion.vy / 6);
            }
            obj->motion.fall_time++;
            break;
        case 2:
            obj->frame = wstag415_motion_anim_hold(&obj->motion, D_WSTAG415_800A73F0, 0);
            if (obj->frame == 0x35 && obj->base.step == 0) {
                sound_module.play(0x80044648);
                obj->base.step++;
            }
            if (obj->frame == 0x3B && obj->base.step == 1) {
                sound_module.play(0x340001);
                obj->base.step++;
            }
            break;
        case 3:
            obj->frame = wstag415_motion_anim_hold(&obj->motion, D_WSTAG415_800A7418, 0);
            break;
        case 4:
            obj->frame = wstag415_motion_anim_hold(&obj->motion, D_WSTAG415_800A7428, 0);
            if (obj->frame == 2 && obj->base.step == 0) {
                sound_module.play(0x340002);
                obj->base.step++;
            }
            if (obj->frame == 0x20 && obj->base.step == 1) {
                sound_module.play(0x40012);
                obj->base.step++;
            }
            if (obj->frame == 0x2A && obj->base.step == 2) {
                sound_module.play(0x40012);
                obj->base.substep = 0;
                obj->base.step++;
            }
            if (obj->base.step == 3 || obj->base.step == 4) {
                obj->base.substep++;
                if (!(obj->base.substep & 0x1F)) {
                    sound_module.play(0x40012);
                    obj->base.substep = 0;
                    obj->base.step++;
                }
            }
            break;
        }
        spr = obj->sprite;
        spr->sprite = obj->frame;
        spr->x = obj->x;
        spr->y = obj->y;
        spr->shown = 1;
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag415_diver_message(Wstag415Diver *obj, s32 msg) {
    if (obj != NULL) {
        switch (msg) {
        case 0x331:
            obj->motion.x = 0x9600;
            obj->motion.y = 0xE600;
            obj->motion.mode = 1;
            obj->motion.vy = 0;
            obj->motion.vx = 0;
            obj->motion.fall_time = 0;
            break;
        case 0x332:
            sound_module.play(0x800442C1);
            obj->motion.anim.key = 0;
            obj->motion.anim.time = D_WSTAG415_800A73F0[0].time;
            obj->motion.mode = 2;
            obj->base.step = 0;
            break;
        case 0x333:
            sound_module.play(0x80044648);
            obj->motion.anim.key = 0;
            obj->motion.anim.time = D_WSTAG415_800A7418[0].time;
            obj->motion.mode = 3;
            break;
        case 0x334:
            obj->motion.mode = 4;
            obj->motion.anim.key = 0;
            obj->motion.anim.time = D_WSTAG415_800A7428[0].time;
            obj->base.step = 0;
            break;
        }
    }
}

Wstag415Diver *wstag415_diver_create_0(s32 arg0) {
    Wstag415Diver *obj = object_create(wstag415_diver_update, sizeof(Wstag415Diver), 0, arg0);

    obj->base.key1 = 0;
    return obj;
}

Wstag415Diver *wstag415_diver_create_1(s32 arg0) {
    Wstag415Diver *obj = object_create(wstag415_diver_update, sizeof(Wstag415Diver), 0, arg0);

    obj->base.key1 = 1;
    return obj;
}

void wstag415_sea_draw(Wstag415Sea *obj) {
    Sprite spr;
    GamestatePos pos;

    sprite_init(&spr);
    spr.set_layer_id(0x1002, 7);
    spr.set_vram_pos(0x280, 0);
    spr.set_clut8_pos(0, 0xF0);
    pos.x = obj->x >> 8;
    pos.y = (obj->y >> 8) + 0x100;
    spr.draw(cdload_module.get_subfile_by_id(0x075E0002), 0, pos.x, pos.y);
    spr.draw(cdload_module.get_subfile_by_id(0x075E0002), 0, pos.x - 0x200, pos.y + 0x100);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_clut8_pos(0, 0xF8);
    pos.x = (obj->x >> 8) + 0x100;
    spr.draw(cdload_module.get_subfile_by_id(0x075E0004), 0, pos.x, pos.y);
    spr.draw(cdload_module.get_subfile_by_id(0x075E0004), 0, pos.x - 0x200, pos.y + 0x100);
}

void wstag415_sea_update(Wstag415Sea *obj) {
    Tim img;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        tim_init(&img);
        img.set_image_pos(0x280, 0);
        img.set_clut_pos(0, 0xF0);
        img.load_all((s32 *)cdload_module.files.get_file(0x769));
        img.set_image_pos(0x280, 0x100);
        img.set_clut_pos(0, 0xF8);
        img.load_all((s32 *)cdload_module.files.get_file(0x76A));
        obj->base.next_state(obj);
        if (obj->base.key1 != 0) {
            obj->base.next_step(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->vx = 0x800;
            obj->vy = 0x400;
            break;
        case 1:
            if (obj->vx == 0x800) {
                sound_module.play(0x340003);
            }
            if (obj->vx != 0) {
                obj->vx -= 0x10;
                obj->vy = obj->vx / 2;
                if (obj->vx < 0) {
                    obj->vx = 0;
                    obj->vy = 0;
                }
            }
            break;
        }
        obj->x += obj->vx;
        obj->y -= obj->vy;
        if (obj->x >= 0x20000) {
            obj->x -= 0x20000;
            obj->y += 0x10000;
        }
        wstag415_sea_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag415_sea_message(Wstag415Sea *obj, s32 msg) {
    if (obj != NULL && msg == 0x336) {
        obj->base.set_step(obj, 1);
    }
}

Wstag415Sea *wstag415_sea_create(s32 arg0) {
    Wstag415Sea *obj = object_create(wstag415_sea_update, sizeof(Wstag415Sea), 0, 0x32C);

    obj->base.key1 = arg0;
    return obj;
}

void wstag415_update(WstagObject *obj, Wstag415Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 6) {
            if (gamestate_flags.get_flag(0x4018, 0)) {
                data->sea = wstag415_sea_create(0);
                data->event = fieldstg_event_start(0x96);
            } else {
                data->sea = wstag415_sea_create(1);
                data->diver = wstag415_diver_create_1(0x32B);
                data->event = fieldstg_event_start(0x97);
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag415_start(void *arg0) {
    WstagObject *obj = object_new(wstag415_update, sizeof(WstagObject), sizeof(Wstag415Data));

    obj->manager = arg0;
    wstag415_funcs.setup();
    return obj;
}

void wstag415_event_150_end(void) {
    gamestate_flags.set_flag(0x4018, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag415_setup(void) {
    fieldstg_stage.sprite_file = 0x075E0000;
    fieldstg_stage.sprites = wstag415_sprites;
    fieldstg_stage.mask_file = 0x75F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11500, 0x15500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag415_vram_places;
    fieldstg_stage.music = 0xD;
    fieldstg_stage.sound = 0x60340000;
    fieldstg_stage.actors = wstag415_actors;
    fieldstg_stage.events = wstag415_events;
    fieldstg_stage.battle_lists = &wstag415_battle_lists;
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag415_setup(void);

s16 D_WSTAG415_800A7124[190] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_PLACE(1, 276, 343),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32B, 816, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_ANIM(0x32C, 822, 812),
    FIELDSTG_EVENT_WAIT_ANIM(0x32C),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 4),
    FIELDSTG_EVENT_ANIM(0x32D, 893, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 882, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 51, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 216, 373),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x330, 821, 816),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x331, 821, 817),
    FIELDSTG_EVENT_ANIM(0x337, 821, 823),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x332, 821, 818),
    FIELDSTG_EVENT_ANIM(0x334, 821, 820),
    FIELDSTG_EVENT_ANIM(0x336, 821, 822),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x333, 821, 819),
    FIELDSTG_EVENT_ANIM(0x339, 821, 825),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x335, 821, 821),
    FIELDSTG_EVENT_ANIM(0x338, 821, 819),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32B, 817, 811),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 1),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 51, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32B, 818, 811),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x32B, 819, 811),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG415_800A72A0[67] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 216, 373),
    FIELDSTG_EVENT_PLACE(1, 276, 343),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32C, 822, 1),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(84),
    FIELDSTG_EVENT_ANIM(0x32B, 820, 1),
    FIELDSTG_EVENT_WAIT(156),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 4),
    FIELDSTG_EVENT_ANIM(0x32D, 893, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x232, 495, 185, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG415_800A7328[5] = { { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 8 }, { 255, 0 } };
WstagAnimKey D_WSTAG415_800A733C[5] = { { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 255, 0 } };
Wstag415Path D_WSTAG415_800A7350[10] = {
    { D_WSTAG415_800A7328, -32, 230, 0, 384, 88, 0 }, { D_WSTAG415_800A733C, 0, 230, 450, 512, 72, 4 },
    { D_WSTAG415_800A7328, 32, 230, 410, 384, 88, 8 }, { D_WSTAG415_800A733C, 64, 230, 0, 512, 72, 0 },
    { D_WSTAG415_800A733C, 80, 230, 0, 512, 72, 0 }, { D_WSTAG415_800A7328, 16, 230, 0, 96, 88, 0 },
    { D_WSTAG415_800A733C, 48, 230, 460, 128, 72, 4 }, { D_WSTAG415_800A7328, 80, 230, 420, 96, 88, 8 },
    { D_WSTAG415_800A733C, 112, 230, 0, 128, 72, 0 }, { D_WSTAG415_800A733C, 140, 230, 0, 128, 72, 0 },
};
WstagAnimKey D_WSTAG415_800A73F0[10] = {
    { 50, 5 }, { 52, 5 }, { 53, 5 }, { 54, 40 }, { 55, 8 }, { 56, 8 }, { 57, 10 }, { 58, 12 }, { 59, 180 },
    { 255, 0 },
};
WstagAnimKey D_WSTAG415_800A7418[4] = { { 60, 4 }, { 61, 4 }, { 62, 999 }, { 255, 0 } };
WstagAnimKey D_WSTAG415_800A7428[48] = {
    { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 4 },
    { 12, 4 }, { 13, 4 }, { 14, 4 }, { 15, 4 }, { 16, 4 }, { 17, 4 }, { 18, 4 }, { 19, 4 }, { 20, 4 }, { 21, 4 },
    { 22, 4 }, { 23, 4 }, { 24, 4 }, { 25, 4 }, { 26, 4 }, { 27, 4 }, { 28, 4 }, { 29, 4 }, { 30, 4 }, { 31, 4 },
    { 32, 4 }, { 33, 4 }, { 34, 4 }, { 35, 4 }, { 36, 4 }, { 37, 4 }, { 38, 4 }, { 39, 4 }, { 40, 4 }, { 41, 4 },
    { 42, 4 }, { 43, 4 }, { 44, 4 }, { 45, 4 }, { 46, 4 }, { 47, 4 }, { 255, 999 },
};
FieldstgListedBattle D_WSTAG415_800A74E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A74F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A750C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A753C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG415_800A7548 = {
    0,
    { &D_WSTAG415_800A74E8, &D_WSTAG415_800A74F4, &D_WSTAG415_800A7500, &D_WSTAG415_800A750C, &D_WSTAG415_800A7518,
        &D_WSTAG415_800A7524, &D_WSTAG415_800A7530, &D_WSTAG415_800A753C },
};
FieldstgListedBattle D_WSTAG415_800A756C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A759C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A75A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A75B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A75C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG415_800A75CC = {
    0,
    { &D_WSTAG415_800A756C, &D_WSTAG415_800A7578, &D_WSTAG415_800A7584, &D_WSTAG415_800A7590, &D_WSTAG415_800A759C,
        &D_WSTAG415_800A75A8, &D_WSTAG415_800A75B4, &D_WSTAG415_800A75C0 },
};
FieldstgListedBattle D_WSTAG415_800A75F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A75FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A762C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7644 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG415_800A7650 = {
    0,
    { &D_WSTAG415_800A75F0, &D_WSTAG415_800A75FC, &D_WSTAG415_800A7608, &D_WSTAG415_800A7614, &D_WSTAG415_800A7620,
        &D_WSTAG415_800A762C, &D_WSTAG415_800A7638, &D_WSTAG415_800A7644 },
};
FieldstgListedBattle D_WSTAG415_800A7674 = { 323, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG415_800A7680 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A768C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A7698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A76A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A76B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A76BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG415_800A76C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG415_800A76D4 = {
    0,
    { &D_WSTAG415_800A7674, &D_WSTAG415_800A7680, &D_WSTAG415_800A768C, &D_WSTAG415_800A7698, &D_WSTAG415_800A76A4,
        &D_WSTAG415_800A76B0, &D_WSTAG415_800A76BC, &D_WSTAG415_800A76C8 },
};
FieldstgBattleLists wstag415_battle_lists = {
    128, 0, 0, { &D_WSTAG415_800A7548, &D_WSTAG415_800A75CC, &D_WSTAG415_800A7650 }, &D_WSTAG415_800A76D4,
};
FieldstgVramPlace wstag415_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 460, 336, 560, 80, 352, 511 },
};
FieldstgPlacedActor D_WSTAG415_800A7784 = { NULL, NULL, 1, 4, 0, 0, 7 };
FieldstgPlacedActor *wstag415_actors[2] = { &D_WSTAG415_800A7784, NULL };
FieldstgSprite wstag415_sprites[3] = {
    { 1, 0, 0x6E, 6, 0, 0, 0, 0, 0, 0, 225, 318, 0, 0 }, { 1, 1, 0x94, 4, 0x32, 0, 0, 0, 0, 0, 0, -50, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag415_funcs = { wstag415_setup };
FieldstgEventDef wstag415_events[3] = {
    { 150, D_WSTAG415_800A7124, 0x01350004, NULL, wstag415_event_150_end },
    { 151, D_WSTAG415_800A72A0, 0x01350005, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
