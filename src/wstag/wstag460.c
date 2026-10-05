#include "wstag.h"
#include "pad.h"

/* WSTAG460: stage 0x236 (fieldstg_stages). */

/* A sprite (of type `type`) that flies around its place: every turn_period frames it turns randomly, or back home when it is
 * more than 0x1E pixels away (wstag460_fly_new; wstag460_fly_message makes it slow down and vanish). */
typedef struct Wstag460Fly {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 mode; /* 0: hidden, 1: flies, 2: waits `wait` frames, 3: slows down */
    /* 0x52 */ u8 type;  /* the sprite's type */
    /* 0x53 */ u8 speed_id;  /* the speed (D_WSTAG460_800A7438) */
    /* 0x54 */ s32 first_key; /* the animation's first key and the turn timer's start */
    /* 0x58 */ s32 wait;
    /* 0x5C */ s32 home_x; /* home x */
    /* 0x60 */ s32 home_y; /* home y */
    /* 0x64 */ s32 pos_x; /* x, 24.8 */
    /* 0x68 */ s32 pos_y; /* y, 24.8 */
    /* 0x6C */ s16 x; /* x */
    /* 0x6E */ s16 y; /* y */
    /* 0x70 */ s16 turn_timer; /* turn timer */
    /* 0x72 */ s16 turn_period; /* turn period */
    /* 0x74 */ s16 speed; /* speed */
    /* 0x76 */ s16 dir; /* direction (rsin/rcos angle) */
    /* 0x78 */ WstagSpriteAnim sprite;
} Wstag460Fly; /* size 0x80 */

/* The data of wstag460_group_update's object, which message 0x34C makes stop all of them. */
typedef struct Wstag460Data {
    /* 0x00 */ WstagGlowObject *glow;
    /* 0x04 */ WstagGlow2Object *glow2;
    /* 0x08 */ Wstag460Fly *flies[7];
} Wstag460Data; /* size 0x24 */

typedef struct Wstag460StageData {
    /* 0x0 */ Object *group;
} Wstag460StageData; /* size 0x4 */

extern WstagAnimKey D_WSTAG460_800A726C[];
extern WstagAnimKey D_WSTAG460_800A72AC[];
extern WstagAnimKey *D_WSTAG460_800A73C8[2];
extern WstagAnimKey *D_WSTAG460_800A73D0[2];
extern u8 D_WSTAG460_800A73D8[2][2];
extern WstagAnimKey D_WSTAG460_800A73DC[];
extern WstagAnimKey D_WSTAG460_800A7408[];
extern s16 D_WSTAG460_800A7438[];
extern u16 D_WSTAG460_800A7440[];
extern WstagFuncs wstag460_funcs;
extern FieldstgVramPlace wstag460_vram_places[];
extern FieldstgPlacedActor *wstag460_actors[];
extern FieldstgSprite wstag460_sprites[];
extern FieldstgMapEvent wstag460_map_events[];
extern FieldstgEventDef wstag460_events[];
s32 rcos(s32 a);
void wstag460_glow_update(WstagGlowObject *obj);
void wstag460_glow_message(WstagGlowObject *obj, s32 arg1, s32 arg2);
WstagGlowObject *wstag460_glow_new(void);
void wstag460_glow2_update(WstagGlow2Object *obj);
void wstag460_glow2_message(WstagGlow2Object *obj, s32 arg1, s32 arg2);
WstagGlow2Object *wstag460_glow2_new(void);
void wstag460_fly_update(void *arg0);
void wstag460_fly_message(Wstag460Fly *obj, s32 arg1, s32 arg2);
Wstag460Fly *wstag460_fly_new(s32 type, s32 speed, s32 key);
void wstag460_update(Object *obj, Wstag460StageData *data);

void wstag460_group_update(Object *obj, Wstag460Data *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        data->glow = wstag460_glow_new();
        data->glow2 = wstag460_glow2_new();
        data->flies[0] = wstag460_fly_new(1, 0, 0);
        data->flies[1] = wstag460_fly_new(2, 0, 0);
        data->flies[2] = wstag460_fly_new(3, 0, 0);
        data->flies[3] = wstag460_fly_new(4, 1, 0);
        data->flies[4] = wstag460_fly_new(5, 1, 0);
        data->flies[5] = wstag460_fly_new(6, 1, 0);
        data->flies[6] = wstag460_fly_new(7, 1, 0);
        obj->next_state(obj);
        break;
    case OBJECT_STATE_DONE:
        wstag460_glow_message(data->glow, 0, 0);
        wstag460_glow2_message(data->glow2, 0, 0);
        wstag460_fly_message(data->flies[0], 0, 0);
        wstag460_fly_message(data->flies[1], 0, 0);
        wstag460_fly_message(data->flies[2], 0, 0);
        wstag460_fly_message(data->flies[3], 0, 0);
        wstag460_fly_message(data->flies[4], 0, 0);
        wstag460_fly_message(data->flies[5], 0, 0);
        wstag460_fly_message(data->flies[6], 0, 0);
        obj->set_state(obj, OBJECT_STATE_RUN);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag460_group_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x34C) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

Object *wstag460_group_create(s32 arg0) {
    return object_create(wstag460_group_update, sizeof(Object), sizeof(Wstag460Data), arg0);
}

s32 wstag460_glow_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        sa->anim.time -= step;
    }
    if (sa->anim.time <= 0) {
        key++;
        sa->anim.key++;
        sa->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag460_glow_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag460_glow_update(WstagGlowObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG460_800A726C[0].time;
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG460_800A726C[0].time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == 0xA) {
                obj->sprite.sprite = list;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite = obj->sprite.sprite;
        if (obj->mode != 0) {
            sprite->shown = 1;
            sprite->sprite = 0x3C;
            sprite->frame = wstag460_glow_anim_advance(&obj->sprite, D_WSTAG460_800A726C, 0, 0);
        } else {
            sprite->shown = 0;
        }
        if (obj->mode == 2) {
            if (--obj->stop_delay <= 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->sprite.anim.key = 0;
            obj->sprite.anim.time = D_WSTAG460_800A72AC[0].time;
            obj->base.set_step(obj, 1);
        }
        spr = obj->sprite.sprite;
        frame = wstag460_glow_anim_advance(&obj->sprite, D_WSTAG460_800A72AC, 1, 0);
        switch (frame) {
        case 0x12C:
            spr->shown = 0;
            break;
        case 0xFF:
            spr->shown = 0;
            obj->mode = 0;
            obj->base.set_state(obj, OBJECT_STATE_RUN);
            break;
        default:
            spr->shown = 1;
            spr->sprite = 0x3D;
            spr->frame = frame;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag460_glow_message(WstagGlowObject *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->stop_delay = 0x14;
    }
}

WstagGlowObject *wstag460_glow_create(s32 arg0) {
    return object_create(wstag460_glow_update, sizeof(WstagGlowObject), 0, arg0);
}

WstagGlowObject *wstag460_glow_new(void) {
    return object_new(wstag460_glow_update, sizeof(WstagGlowObject), 0);
}

s32 wstag460_glow2_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        sa->anim.time -= step;
    }
    if (sa->anim.time <= 0) {
        key++;
        sa->anim.key++;
        sa->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag460_glow2_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag460_glow2_update(WstagGlow2Object *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG460_800A73C8[0]->time;
        obj->sprites[1].anim.key = 0;
        obj->sprites[1].anim.time = D_WSTAG460_800A73C8[1]->time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            switch (list->type) {
            case 8:
                obj->sprites[0].sprite = list;
                break;
            case 9:
                obj->sprites[1].sprite = list;
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            sprite = obj->sprites[i].sprite;
            if (obj->mode != 0) {
                sprite->shown = 1;
                sprite->frame = wstag460_glow2_anim_advance(&obj->sprites[i], D_WSTAG460_800A73C8[i], 0, 0);
                sprite->sprite = D_WSTAG460_800A73D8[0][i];
            } else {
                sprite->shown = 0;
            }
        }
        if (obj->mode == 2) {
            if (--obj->stop_delay <= 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG460_800A73D0[0]->time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG460_800A73D0[1]->time;
            obj->base.set_step(obj, 1);
        }
        for (i = 0; i < 2; i++) {
            sprite = obj->sprites[i].sprite;
            frame = wstag460_glow2_anim_advance(&obj->sprites[i], D_WSTAG460_800A73D0[i], 1, 0);
            switch (frame) {
            case 0x12C:
                sprite->shown = 0;
                break;
            case 0xFF:
                sprite->shown = 0;
                obj->mode = 0;
                obj->base.set_state(obj, OBJECT_STATE_RUN);
                break;
            default:
                sprite->shown = 1;
                sprite->sprite = D_WSTAG460_800A73D8[1][i];
                sprite->frame = frame;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag460_glow2_message(WstagGlow2Object *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->stop_delay = 0x96;
    }
}

WstagGlow2Object *wstag460_glow2_create(s32 arg0) {
    return object_create(wstag460_glow2_update, sizeof(WstagGlow2Object), 0, arg0);
}

WstagGlow2Object *wstag460_glow2_new(void) {
    return object_new(wstag460_glow2_update, sizeof(WstagGlow2Object), 0);
}

s32 wstag460_fly_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        sa->anim.time -= step;
    }
    if (sa->anim.time <= 0) {
        key++;
        sa->anim.key++;
        sa->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag460_fly_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

s32 wstag460_fly_is_far(Wstag460Fly *obj, s32 dist) {
    s32 dx = obj->x - obj->home_x;
    s32 dy = obj->y - obj->home_y;

    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }
    return dist < dx + dy;
}

s32 wstag460_get_angle(s32 dx, s32 dy) {
    s32 ofs = 0;
    s32 ratio = 0;
    s32 base;
    s32 i;

    if (dx <= 0 && dy >= 0) {
        base = 0;
    } else if (dx >= 0 && dy >= 0) {
        base = 0x40;
    } else if (dx >= 0 && dy <= 0) {
        base = 0x80;
    } else if (dx <= 0 && dy <= 0) {
        base = 0xC0;
    } else {
        base = 0;
    }
    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }
    if (dx == dy) {
        return base | 0x20;
    }
    if (dy < dx) {
        ratio = dy * 0xFFFF / dx;
    } else if (dx < dy) {
        ratio = dx * 0xFFFF / dy;
    }
    for (i = 0; i <= 32; i++) {
        if (ratio >= D_WSTAG460_800A7440[i] && ratio <= D_WSTAG460_800A7440[i + 1]) {
            switch (base) {
            case 0x00:
            case 0x80:
                if (dy < dx) {
                    ofs = i;
                } else if (dx < dy) {
                    ofs = 0x40 - i;
                }
                return ofs + base;
            case 0x40:
            case 0xC0:
                if (dy < dx) {
                    ofs = 0x40 - i;
                } else if (dx < dy) {
                    ofs = i;
                }
                return ofs + base;
            }
        }
    }
    return 0xFF;
}

void wstag460_fly_move(Wstag460Fly *obj) {
    s32 dx;
    s32 dy;

    obj->turn_timer += gfx_module.funcs.get_frame_ticks();
    if (obj->turn_timer > obj->turn_period) {
        if (wstag460_fly_is_far(obj, 0x1E)) {
            obj->dir = ((wstag460_get_angle(obj->home_x - obj->x, obj->home_y - obj->y) - 0x40) * 16) & 0xFFF;
        } else {
            obj->dir = (pad_random.next() & 0xFF) * 16;
        }
        obj->turn_timer -= obj->turn_period;
        obj->turn_timer += (pad_random.next() & 0xF) - 7;
    }
    dx = rsin(obj->dir) * obj->speed / 4096;
    dy = rcos(obj->dir) * obj->speed / 4096;
    obj->pos_x += dx;
    obj->pos_y += dy;
    obj->x = obj->pos_x >> 8;
    obj->y = obj->pos_y >> 8;
}

void wstag460_fly_update(void *arg0) {
    Wstag460Fly *obj = arg0;
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprite.anim.key = obj->first_key;
        obj->sprite.anim.time = D_WSTAG460_800A73DC[0].time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == obj->type) {
                obj->sprite.sprite = list;
                obj->home_x = list->x;
                obj->home_y = list->y;
                obj->pos_x = list->x << 8;
                obj->pos_y = list->y << 8;
            }
        }
        obj->turn_period = 0x28;
        obj->turn_timer = obj->first_key;
        obj->speed = D_WSTAG460_800A7438[obj->speed_id];
        obj->dir = (pad_random.next() & 0xFF) * 16;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->mode != 0) {
            wstag460_fly_move(obj);
        }
        sprite = obj->sprite.sprite;
        if (obj->mode != 0) {
            sprite->shown = 1;
            sprite->frame = wstag460_fly_anim_advance(&obj->sprite, D_WSTAG460_800A73DC, 0, 0);
            sprite->x = obj->x;
            sprite->y = obj->y;
        } else {
            sprite->shown = 0;
        }
        if (obj->mode == 3) {
            obj->speed -= 4;
            if (obj->speed < 0x11) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
        }
        if (obj->mode == 2) {
            if (--obj->wait <= 0) {
                obj->mode = 3;
            }
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->sprite.anim.key = 0;
            obj->sprite.anim.time = D_WSTAG460_800A7408[0].time;
            obj->base.set_step(obj, 1);
        }
        wstag460_fly_move(obj);
        spr = obj->sprite.sprite;
        frame = wstag460_fly_anim_advance(&obj->sprite, D_WSTAG460_800A7408, 1, 0);
        switch (frame) {
        case 0x12C:
            spr->shown = 0;
            break;
        case 0xFF:
            spr->shown = 0;
            obj->mode = 0;
            obj->base.set_state(obj, OBJECT_STATE_RUN);
            break;
        default:
            spr->shown = 1;
            spr->frame = frame;
            spr->x = obj->x;
            spr->y = obj->y;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag460_fly_message(Wstag460Fly *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->wait = 0x3C;
    }
}

Wstag460Fly *wstag460_fly_create(s32 arg0) {
    return object_create(wstag460_fly_update, sizeof(Wstag460Fly), 0, arg0);
}

Wstag460Fly *wstag460_fly_new(s32 type, s32 speed, s32 key) {
    Wstag460Fly *obj = object_new(wstag460_fly_update, sizeof(Wstag460Fly), 0);

    obj->first_key = key;
    obj->speed_id = speed;
    obj->type = type;
    return obj;
}

void wstag460_update(Object *obj, Wstag460StageData *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        if (gamestate_data.progress < 0xF) {
            data->group = wstag460_group_create(0x33E);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag460_start(void *arg0) {
    WstagObject *obj = object_new(wstag460_update, sizeof(WstagObject), sizeof(Wstag460StageData));

    obj->manager = arg0;
    wstag460_funcs.setup();
    return obj;
}

void wstag460_event_370_end(void) {
    gamestate_data.progress = 0xF;
    gamestate_flags.set_flag(0x8010, 1);
}

void wstag460_setup(void) {
    fieldstg_stage.background_file = 0x362;
    fieldstg_stage.sprite_file = 0x03630000;
    fieldstg_stage.sprites = wstag460_sprites;
    fieldstg_stage.map_events = wstag460_map_events;
    fieldstg_stage.mask_file = 0x45F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xC200, 0x1A500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag460_vram_places;
    fieldstg_stage.music = 0x10;
    fieldstg_stage.sound = 0x60400000;
    fieldstg_stage.actors = wstag460_actors;
    fieldstg_stage.events = wstag460_events;
    fieldstg_attr.set_file(0, 0x03630001);
    fieldstg_attr.set_file(7, 0x03630002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag460_setup(void);

s16 D_WSTAG460_800A71A4[99] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 112, 392, 3),
    FIELDSTG_EVENT_PLACE(130, 96, 367),
    FIELDSTG_EVENT_ANIM(130, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(130, 0, 0),
    FIELDSTG_EVENT_ANIM(130, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x33E, 844, 2),
    FIELDSTG_EVENT_WAIT(210),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 176, 424, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x235, 1136, 224, 7),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG460_800A726C[16] = {
    { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 11, 8 }, { 12, 8 }, { 13, 8 }, { 14, 8 }, { 255, 0 },
};
WstagAnimKey D_WSTAG460_800A72AC[13] = {
    { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 11, 8 }, { 255, 999 },
};
WstagAnimKey D_WSTAG460_800A72E0[16] = {
    { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 11, 8 }, { 12, 8 }, { 13, 8 }, { 14, 8 }, { 255, 0 },
};
WstagAnimKey D_WSTAG460_800A7320[13] = {
    { 0, 6 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 6 }, { 6, 6 }, { 7, 6 }, { 8, 6 }, { 9, 6 }, { 10, 6 },
    { 11, 6 }, { 255, 999 },
};
WstagAnimKey D_WSTAG460_800A7354[16] = {
    { 0, 8 }, { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 }, { 5, 8 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 11, 8 }, { 12, 8 }, { 13, 8 }, { 14, 8 }, { 255, 0 },
};
WstagAnimKey D_WSTAG460_800A7394[13] = {
    { 0, 6 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 6 }, { 6, 6 }, { 7, 6 }, { 8, 6 }, { 9, 6 }, { 10, 6 },
    { 11, 6 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG460_800A73C8[2] = { D_WSTAG460_800A72E0, D_WSTAG460_800A7354 };
WstagAnimKey *D_WSTAG460_800A73D0[2] = { D_WSTAG460_800A7320, D_WSTAG460_800A7394 };
u8 D_WSTAG460_800A73D8[2][2] = { { 0x3E, 0x3F }, { 0x40, 0x41 } };
WstagAnimKey D_WSTAG460_800A73DC[11] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 }, { 255, 0 },
};
WstagAnimKey D_WSTAG460_800A7408[12] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 255, 999 },
};
s16 D_WSTAG460_800A7438[4] = { 96, 80, 72, 64 };
u16 D_WSTAG460_800A7440[34] = {
    0, 0x648, 0xC93, 0x12E2, 0x1936, 0x1F92, 0x259F, 0x2C6B,
    0x32EB, 0x39C7, 0x401F, 0x46D7, 0x4DA7, 0x5491, 0x5B98, 0x62BF,
    0x6A09, 0x7179, 0x7913, 0x80DB, 0x88B5, 0x9105, 0x9970, 0xA21B,
    0xAB0D, 0xB44A, 0xBDDC, 0xC7C8, 0xD217, 0xDCD2, 0xE805, 0xF3BA,
    0xFFFE, 0xFFFF,
};
FieldstgVramPlace wstag460_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 378, 472, 232, 216, 368, 497 },
};
FieldstgTalk D_WSTAG460_800A7504[2] = { { NULL, NULL, 826 }, { NULL, NULL, 0 } };
u16 D_WSTAG460_800A751C[4] = { 0x600E, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG460_800A7524 = { NULL, D_WSTAG460_800A7504, 63, 4, 250, 109, 1 };
FieldstgPlacedActor D_WSTAG460_800A7538 = { D_WSTAG460_800A751C, NULL, 130, 5, 96, 367, 7 };
FieldstgPlacedActor *wstag460_actors[3] = { &D_WSTAG460_800A7524, &D_WSTAG460_800A7538, NULL };
FieldstgSprite wstag460_sprites[16] = {
    { 0, 8, 0xFF, 2, 0x3E, 0, 0, 0, 0, 0, 46, 288, 0, 0 }, { 0, 3, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 52, 288, 0, 0 },
    { 0, 2, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 154, 262, 0, 0 }, { 0, 1, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 174, 359, 0, 0 },
    { 0, 7, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 103, 297, 0, 0 }, { 0, 5, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 117, 236, 0, 0 },
    { 0, 6, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 217, 300, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 0xC, 0, 48, 288, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 0xC, 0, 160, 280, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 0xC, 0, 104, 248, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 0xC, 0, 233, 263, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 0xC, 0, 233, 335, 0, 0 },
    { 0, 0xA, 0xFF, 6, 0x3C, 0, 0, 0, 0, 0, 50, 239, 0, 0 }, { 0, 9, 0xFF, 6, 0x3F, 0, 0, 0, 0, 0, 46, 288, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 240, 72, 135, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag460_map_events[4] = {
    { 0x600E, 1, 0xFFFF, 0, 8, 0x172, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 0xE, 0xEF, 0xA8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xE, 0xDF, 0x190, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag460_funcs = { wstag460_setup };
FieldstgEventDef wstag460_events[2] = {
    { 370, D_WSTAG460_800A71A4, 0x01350028, NULL, wstag460_event_370_end }, { -1, NULL, 0, NULL, NULL },
};
