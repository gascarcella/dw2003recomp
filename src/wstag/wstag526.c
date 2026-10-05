#include "wstag.h"
#include "pad.h"

/* WSTAG526: stage 0x2AE (fieldstg_stages). */

/* WSTAG460's Wstag460Fly with a second sprite (of type `type` + 9) that follows the first. */
typedef struct Wstag526Fly {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 mode; /* 0: hidden, 1: flies, 2: waits `wait` frames, 3: slows down */
    /* 0x52 */ u8 type;  /* the sprite's type */
    /* 0x53 */ u8 speed_id;  /* the speed (D_WSTAG526_800A786C) */
    /* 0x54 */ s32 first_key; /* the animations' first key and the turn timer's start */
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
    /* 0x78 */ WstagSpriteAnim sprites[2];
} Wstag526Fly; /* size 0x88 */

/* The data of wstag526_group_update's object, which message 0x34C... makes stop all of them. */
typedef struct Wstag526Data {
    /* 0x00 */ WstagGlowObject *glow;
    /* 0x04 */ WstagGlow2Object *glow2;
    /* 0x08 */ WstagGlowObject *glow3;
    /* 0x0C */ Wstag526Fly *flies[9];
} Wstag526Data; /* size 0x30 */

typedef struct Wstag526StageData {
    /* 0x0 */ Object *group;
} Wstag526StageData; /* size 0x4 */

extern WstagAnimKey D_WSTAG526_800A76E8[];
extern WstagAnimKey D_WSTAG526_800A76FC[];
extern WstagAnimKey *D_WSTAG526_800A7780[2];
extern WstagAnimKey *D_WSTAG526_800A7788[2];
extern WstagAnimKey D_WSTAG526_800A7790[];
extern WstagAnimKey *D_WSTAG526_800A785C[2];
extern WstagAnimKey *D_WSTAG526_800A7864[2];
extern s16 D_WSTAG526_800A786C[];
extern u16 D_WSTAG526_800A7874[];
extern WstagFuncs wstag526_funcs;
extern FieldstgBattleLists wstag526_battle_lists;
extern FieldstgVramPlace wstag526_vram_places[];
extern FieldstgPlacedActor *wstag526_actors[];
extern FieldstgSprite wstag526_sprites[];
extern FieldstgMapEvent wstag526_map_events[];
extern FieldstgEventDef wstag526_events[];
s32 rcos(s32 a);
void wstag526_glow_update(WstagGlowObject *obj);
void wstag526_glow_message(WstagGlowObject *obj, s32 arg1, s32 arg2);
WstagGlowObject *wstag526_glow_new(void);
void wstag526_glow2_update(WstagGlow2Object *obj);
void wstag526_glow2_message(WstagGlow2Object *obj, s32 arg1, s32 arg2);
WstagGlow2Object *wstag526_glow2_new(void);
void wstag526_glow3_update(WstagGlowObject *obj);
void wstag526_glow3_message(WstagGlowObject *obj, s32 arg1, s32 arg2);
WstagGlowObject *wstag526_glow3_new(void);
void wstag526_fly_update(void *arg0);
void wstag526_fly_message(Wstag526Fly *obj, s32 arg1, s32 arg2);
Wstag526Fly *wstag526_fly_new(s32 type, s32 speed, s32 key);
void wstag526_update(Object *obj, Wstag526StageData *data);

void wstag526_group_update(Object *obj, Wstag526Data *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        data->glow = wstag526_glow_new();
        data->glow2 = wstag526_glow2_new();
        data->glow3 = wstag526_glow3_new();
        data->flies[0] = wstag526_fly_new(1, 0, 0);
        data->flies[1] = wstag526_fly_new(2, 0, 5);
        data->flies[2] = wstag526_fly_new(3, 1, 0);
        data->flies[3] = wstag526_fly_new(4, 1, 5);
        data->flies[4] = wstag526_fly_new(5, 1, 0);
        data->flies[5] = wstag526_fly_new(6, 2, 0);
        data->flies[6] = wstag526_fly_new(7, 2, 2);
        data->flies[7] = wstag526_fly_new(8, 3, 5);
        data->flies[8] = wstag526_fly_new(9, 3, 3);
        obj->next_state(obj);
        break;
    case OBJECT_STATE_DONE:
        wstag526_glow_message(data->glow, 0, 0);
        wstag526_glow2_message(data->glow2, 0, 0);
        wstag526_glow3_message(data->glow3, 0, 0);
        wstag526_fly_message(data->flies[0], 0, 0);
        wstag526_fly_message(data->flies[1], 0, 0);
        wstag526_fly_message(data->flies[2], 0, 0);
        wstag526_fly_message(data->flies[3], 0, 0);
        wstag526_fly_message(data->flies[4], 0, 0);
        wstag526_fly_message(data->flies[5], 0, 0);
        wstag526_fly_message(data->flies[6], 0, 0);
        wstag526_fly_message(data->flies[7], 0, 0);
        wstag526_fly_message(data->flies[8], 0, 0);
        obj->set_state(obj, OBJECT_STATE_RUN);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag526_group_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x34B) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

Object *wstag526_group_create(s32 arg0) {
    return object_create(wstag526_group_update, sizeof(Object), sizeof(Wstag526Data), arg0);
}

s32 wstag526_glow_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag526_glow_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag526_glow_update(WstagGlowObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG526_800A76E8[0].time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == 0x15) {
                obj->sprite.sprite = list;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite = obj->sprite.sprite;
        if (obj->mode != 0) {
            sprite->shown = 1;
            sprite->sprite = 2;
            sprite->frame = wstag526_glow_anim_advance(&obj->sprite, D_WSTAG526_800A76E8, 0, 0);
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
            obj->sprite.anim.time = D_WSTAG526_800A76FC[0].time;
            obj->base.set_step(obj, 1);
        }
        spr = obj->sprite.sprite;
        frame = wstag526_glow_anim_advance(&obj->sprite, D_WSTAG526_800A76FC, 1, 0);
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
            spr->sprite = 2;
            spr->frame = frame;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag526_glow_message(WstagGlowObject *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->stop_delay = 0x96;
    }
}

WstagGlowObject *wstag526_glow_create(s32 arg0) {
    return object_create(wstag526_glow_update, sizeof(WstagGlowObject), 0, arg0);
}

WstagGlowObject *wstag526_glow_new(void) {
    return object_new(wstag526_glow_update, sizeof(WstagGlowObject), 0);
}

s32 wstag526_glow2_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag526_glow2_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag526_glow2_update(WstagGlow2Object *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG526_800A7780[0]->time;
        obj->sprites[1].anim.key = 0;
        obj->sprites[1].anim.time = D_WSTAG526_800A7780[1]->time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            switch (list->type) {
            case 0x13:
                obj->sprites[0].sprite = list;
                break;
            case 0x14:
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
                sprite->sprite = wstag526_glow2_anim_advance(&obj->sprites[i], D_WSTAG526_800A7780[i], 0, 0);
                sprite->frame = 0;
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
            obj->sprites[0].anim.time = D_WSTAG526_800A7788[0]->time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG526_800A7788[1]->time;
            obj->base.set_step(obj, 1);
        }
        for (j = 0; j < 2; j++) {
            sprite = obj->sprites[j].sprite;
            frame = wstag526_glow2_anim_advance(&obj->sprites[j], D_WSTAG526_800A7788[j], 1, 0);
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
                sprite->sprite = frame;
                sprite->frame = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag526_glow2_message(WstagGlow2Object *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->stop_delay = 0x5A;
    }
}

WstagGlow2Object *wstag526_glow2_create(s32 arg0) {
    return object_create(wstag526_glow2_update, sizeof(WstagGlow2Object), 0, arg0);
}

WstagGlow2Object *wstag526_glow2_new(void) {
    return object_new(wstag526_glow2_update, sizeof(WstagGlow2Object), 0);
}

s32 wstag526_glow3_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag526_glow3_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag526_glow3_update(WstagGlowObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == 0x16) {
                obj->sprite.sprite = list;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite = obj->sprite.sprite;
        if (obj->mode != 0) {
            sprite->shown = 1;
            sprite->sprite = 0x21;
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
            obj->sprite.anim.time = D_WSTAG526_800A7790[0].time;
            obj->base.set_step(obj, 1);
        }
        spr = obj->sprite.sprite;
        frame = wstag526_glow3_anim_advance(&obj->sprite, D_WSTAG526_800A7790, 1, 0);
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
            spr->sprite = frame;
            spr->frame = 0;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag526_glow3_message(WstagGlowObject *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->stop_delay = 0x28;
    }
}

WstagGlowObject *wstag526_glow3_create(s32 arg0) {
    return object_create(wstag526_glow3_update, sizeof(WstagGlowObject), 0, arg0);
}

WstagGlowObject *wstag526_glow3_new(void) {
    return object_new(wstag526_glow3_update, sizeof(WstagGlowObject), 0);
}

s32 wstag526_fly_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag526_fly_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

s32 wstag526_fly_is_far(Wstag526Fly *obj, s32 dist) {
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

s32 wstag526_get_angle(s32 dx, s32 dy) {
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
        if (ratio >= D_WSTAG526_800A7874[i] && ratio <= D_WSTAG526_800A7874[i + 1]) {
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

void wstag526_fly_move(Wstag526Fly *obj) {
    s32 dx;
    s32 dy;

    obj->turn_timer += gfx_module.funcs.get_frame_ticks();
    if (obj->turn_timer > obj->turn_period) {
        if (wstag526_fly_is_far(obj, 0x1E)) {
            obj->dir = ((wstag526_get_angle(obj->home_x - obj->x, obj->home_y - obj->y) - 0x40) * 16) & 0xFFF;
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

void wstag526_fly_update(void *arg0) {
    Wstag526Fly *obj = arg0;
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprites[0].anim.key = obj->first_key;
        obj->sprites[0].anim.time = D_WSTAG526_800A785C[0]->time;
        obj->sprites[1].anim.key = obj->first_key;
        obj->sprites[1].anim.time = D_WSTAG526_800A785C[1]->time;
        obj->mode = 1;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == obj->type) {
                obj->sprites[0].sprite = list;
                obj->home_x = list->x;
                obj->home_y = list->y;
                obj->pos_x = list->x << 8;
                obj->pos_y = list->y << 8;
            }
            if (list->type == obj->type + 9) {
                obj->sprites[1].sprite = list;
            }
        }
        obj->turn_period = 0x28;
        obj->turn_timer = obj->first_key;
        obj->speed = D_WSTAG526_800A786C[obj->speed_id];
        obj->dir = (pad_random.next() & 0xFF) * 16;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->mode != 0) {
            wstag526_fly_move(obj);
        }
        for (i = 0; i < 2; i++) {
            sprite = obj->sprites[i].sprite;
            if (obj->mode != 0) {
                sprite->shown = 1;
                sprite->frame = wstag526_fly_anim_advance(&obj->sprites[i], D_WSTAG526_800A785C[i], 0, 0);
                sprite->x = obj->x;
                sprite->y = obj->y;
            } else {
                sprite->shown = 0;
            }
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
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG526_800A7864[0]->time;
            obj->sprites[1].anim.key = 0;
            obj->sprites[1].anim.time = D_WSTAG526_800A7864[1]->time;
            obj->base.set_step(obj, 1);
        }
        wstag526_fly_move(obj);
        for (j = 0; j < 2; j++) {
            sprite = obj->sprites[j].sprite;
            frame = wstag526_fly_anim_advance(&obj->sprites[j], D_WSTAG526_800A7864[j], 1, 0);
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
                sprite->frame = frame;
                sprite->x = obj->x;
                sprite->y = obj->y;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag526_fly_message(Wstag526Fly *obj, s32 arg1, s32 arg2) {
    if (obj != NULL) {
        obj->mode = 2;
        obj->wait = 0x3C;
    }
}

Wstag526Fly *wstag526_fly_create(s32 arg0) {
    return object_create(wstag526_fly_update, sizeof(Wstag526Fly), 0, arg0);
}

Wstag526Fly *wstag526_fly_new(s32 type, s32 speed, s32 key) {
    Wstag526Fly *obj = object_new(wstag526_fly_update, sizeof(Wstag526Fly), 0);

    obj->first_key = key;
    obj->speed_id = speed;
    obj->type = type;
    return obj;
}

void wstag526_update(Object *obj, Wstag526StageData *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        if (gamestate_data.progress < 0x1E) {
            data->group = wstag526_group_create(0x33D);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag526_start(void *arg0) {
    WstagObject *obj = object_new(wstag526_update, sizeof(WstagObject), sizeof(Wstag526StageData));

    obj->manager = arg0;
    wstag526_funcs.setup();
    return obj;
}

void wstag526_event_760_end(void) {
    gamestate_data.progress = 0x1E;
    gamestate_flags.set_flag(0x8011, 1);
}

void wstag526_setup(void) {
    fieldstg_stage.background_file = 0x6B7;
    fieldstg_stage.sprite_file = 0x06B80000;
    fieldstg_stage.sprites = wstag526_sprites;
    fieldstg_stage.map_events = wstag526_map_events;
    fieldstg_stage.mask_file = 0x6B6;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13D00, 0x38500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag526_vram_places;
    fieldstg_stage.music = 0x37;
    fieldstg_stage.sound = 0x60DC0000;
    fieldstg_stage.actors = wstag526_actors;
    fieldstg_stage.battle_lists = &wstag526_battle_lists;
    fieldstg_stage.events = wstag526_events;
    fieldstg_attr.set_file(0, 0x06B80001);
    fieldstg_attr.set_file(7, 0x06B80002);
    fieldstg_attr.set_file(4, 0x06B80003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag526_setup(void);

s16 D_WSTAG526_800A7610[108] = {
    FIELDSTG_EVENT_WALK(2, 152, 190, 3),
    FIELDSTG_EVENT_PLACE(131, 128, 178),
    FIELDSTG_EVENT_ANIM(131, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 144, 186, 3),
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
    FIELDSTG_EVENT_PLACE(131, 0, 0),
    FIELDSTG_EVENT_ANIM(131, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x33D, 843, 2),
    FIELDSTG_EVENT_WAIT(210),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 152, 190, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 208, 256, 7),
    FIELDSTG_EVENT_GOTO_MAP(0x2AB, 1054, 488, 5),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG526_800A76E8[5] = { { 0, 12 }, { 1, 12 }, { 2, 12 }, { 1, 12 }, { 255, 0 } };
WstagAnimKey D_WSTAG526_800A76FC[7] = {
    { 0, 12 }, { 1, 12 }, { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 255, 999 },
};
WstagAnimKey D_WSTAG526_800A7718[7] = {
    { 37, 12 }, { 38, 12 }, { 39, 12 }, { 40, 12 }, { 41, 12 }, { 42, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG526_800A7734[6] = { { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG526_800A774C[7] = {
    { 91, 12 }, { 92, 12 }, { 93, 12 }, { 94, 12 }, { 95, 12 }, { 96, 12 }, { 255, 0 },
};
WstagAnimKey D_WSTAG526_800A7768[6] = { { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 4 }, { 12, 4 }, { 255, 999 } };
WstagAnimKey *D_WSTAG526_800A7780[2] = { D_WSTAG526_800A7718, D_WSTAG526_800A774C };
WstagAnimKey *D_WSTAG526_800A7788[2] = { D_WSTAG526_800A7734, D_WSTAG526_800A7768 };
WstagAnimKey D_WSTAG526_800A7790[5] = { { 33, 4 }, { 34, 4 }, { 35, 4 }, { 36, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG526_800A77A4[11] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 }, { 255, 0 },
};
WstagAnimKey D_WSTAG526_800A77D0[12] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 255, 999 },
};
WstagAnimKey D_WSTAG526_800A7800[11] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 }, { 255, 0 },
};
WstagAnimKey D_WSTAG526_800A782C[12] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 8 }, { 7, 8 }, { 8, 8 }, { 9, 8 }, { 10, 8 },
    { 255, 999 },
};
WstagAnimKey *D_WSTAG526_800A785C[2] = { D_WSTAG526_800A77A4, D_WSTAG526_800A7800 };
WstagAnimKey *D_WSTAG526_800A7864[2] = { D_WSTAG526_800A77D0, D_WSTAG526_800A782C };
s16 D_WSTAG526_800A786C[4] = { 96, 80, 72, 64 };
u16 D_WSTAG526_800A7874[34] = {
    0, 0x648, 0xC93, 0x12E2, 0x1936, 0x1F92, 0x259F, 0x2C6B,
    0x32EB, 0x39C7, 0x401F, 0x46D7, 0x4DA7, 0x5491, 0x5B98, 0x62BF,
    0x6A09, 0x7179, 0x7913, 0x80DB, 0x88B5, 0x9105, 0x9970, 0xA21B,
    0xAB0D, 0xB44A, 0xBDDC, 0xC7C8, 0xD217, 0xDCD2, 0xE805, 0xF3BA,
    0xFFFE, 0xFFFF,
};
FieldstgListedBattle D_WSTAG526_800A78B8 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A78C4 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A78D0 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A78DC = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A78E8 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A78F4 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A7900 = { 158, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG526_800A790C = { 158, 27, 0x60080000 };
FieldstgBattleList D_WSTAG526_800A7918 = {
    3,
    { &D_WSTAG526_800A78B8, &D_WSTAG526_800A78C4, &D_WSTAG526_800A78D0, &D_WSTAG526_800A78DC, &D_WSTAG526_800A78E8,
        &D_WSTAG526_800A78F4, &D_WSTAG526_800A7900, &D_WSTAG526_800A790C },
};
FieldstgListedBattle D_WSTAG526_800A793C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7948 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7954 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7960 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A796C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7978 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7984 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7990 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG526_800A799C = {
    0,
    { &D_WSTAG526_800A793C, &D_WSTAG526_800A7948, &D_WSTAG526_800A7954, &D_WSTAG526_800A7960, &D_WSTAG526_800A796C,
        &D_WSTAG526_800A7978, &D_WSTAG526_800A7984, &D_WSTAG526_800A7990 },
};
FieldstgListedBattle D_WSTAG526_800A79C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A79CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A79D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A79E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A79F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A79FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A14 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG526_800A7A20 = {
    0,
    { &D_WSTAG526_800A79C0, &D_WSTAG526_800A79CC, &D_WSTAG526_800A79D8, &D_WSTAG526_800A79E4, &D_WSTAG526_800A79F0,
        &D_WSTAG526_800A79FC, &D_WSTAG526_800A7A08, &D_WSTAG526_800A7A14 },
};
FieldstgListedBattle D_WSTAG526_800A7A44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG526_800A7A98 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG526_800A7AA4 = {
    0,
    { &D_WSTAG526_800A7A44, &D_WSTAG526_800A7A50, &D_WSTAG526_800A7A5C, &D_WSTAG526_800A7A68, &D_WSTAG526_800A7A74,
        &D_WSTAG526_800A7A80, &D_WSTAG526_800A7A8C, &D_WSTAG526_800A7A98 },
};
FieldstgBattleLists wstag526_battle_lists = {
    78, 0, 0, { &D_WSTAG526_800A7918, &D_WSTAG526_800A799C, &D_WSTAG526_800A7A20 }, &D_WSTAG526_800A7AA4,
};
FieldstgVramPlace wstag526_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 430, 280, 440, 24, 368, 501 }, { 384, 256, 384, 312, 256, 56, 320, 500 },
    { 320, 256, 373, 288, 212, 32, 336, 500 },
};
u16 D_WSTAG526_800A7B74[8] = { 0x21F, 1, 0x8494, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG526_800A7B84[4] = { 0x8681, 1, 0xFFFF, 0 };
u16 D_WSTAG526_800A7B8C[6] = { 0x8681, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG526_800A7B98[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG526_800A7BA0[8] = { 0x8681, 0, 0, 1, 0x847D, 0, 0xFFFF, 0 };
u16 D_WSTAG526_800A7BB0[8] = { 0x8681, 0, 0, 1, 0x847D, 1, 0xFFFF, 0 };
u16 D_WSTAG526_800A7BC0[10] = {
    0x8681, 1, 0x8680, 0, 0x847D, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG526_800A7BD4[2] = { { NULL, D_WSTAG526_800A7B74, 383 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG526_800A7BEC[5] = {
    { D_WSTAG526_800A7B84, NULL, 738 }, { D_WSTAG526_800A7B8C, D_WSTAG526_800A7B98, 739 },
    { D_WSTAG526_800A7BA0, NULL, 740 }, { D_WSTAG526_800A7BB0, D_WSTAG526_800A7BC0, 741 }, { NULL, NULL, 0 },
};
u16 D_WSTAG526_800A7C28[4] = { 0x21F, 0, 0xFFFF, 0 };
u16 D_WSTAG526_800A7C30[4] = { 0x601D, 1, 0xFFFF, 0 };
u16 D_WSTAG526_800A7C38[10] = {
    0x7043, 1, 0x704B, 1, 0x8680, 1, 0x8681, 0,
    0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG526_800A7C4C = { D_WSTAG526_800A7C28, D_WSTAG526_800A7BD4, 33, 4, 545, 177, 1 };
FieldstgPlacedActor D_WSTAG526_800A7C60 = { D_WSTAG526_800A7C30, NULL, 131, 5, 128, 178, 1 };
FieldstgPlacedActor D_WSTAG526_800A7C74 = { D_WSTAG526_800A7C38, D_WSTAG526_800A7BEC, 171, 6, 641, 291, 1 };
FieldstgPlacedActor *wstag526_actors[4] = {
    &D_WSTAG526_800A7C4C, &D_WSTAG526_800A7C60, &D_WSTAG526_800A7C74, NULL,
};
FieldstgSprite wstag526_sprites[29] = {
    { 0, 0x15, 0xFF, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0x13, 0xFF, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0x14, 0xFF, 2, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 1, 0xFF, 2, 0xD, 0, 0, 0, 0, 0, 80, 128, 0, 0 },
    { 0, 2, 0xFF, 2, 0xD, 0, 0, 0, 0, 0, 88, 48, 0, 0 }, { 0, 3, 0xFF, 2, 0xD, 0, 0, 0, 0, 0, 176, 72, 0, 0 },
    { 0, 6, 0xFF, 2, 0xE, 0, 0, 0, 0, 0, 32, 104, 0, 0 }, { 0, 4, 0xFF, 2, 0xE, 0, 0, 0, 0, 0, 152, 128, 0, 0 },
    { 0, 5, 0xFF, 2, 0xE, 0, 0, 0, 0, 0, 168, 48, 0, 0 }, { 0, 7, 0xFF, 2, 0xF, 0, 0, 0, 0, 0, 56, 112, 0, 0 },
    { 0, 9, 0xFF, 2, 0xF, 0, 0, 0, 0, 0, 112, 144, 0, 0 }, { 0, 8, 0xFF, 2, 0xF, 0, 0, 0, 0, 0, 208, 88, 0, 0 },
    { 0, 0xA, 0xFF, 2, 0x10, 0, 0, 0, 0, 0, 80, 128, 0, 0 }, { 0, 0xC, 0xFF, 2, 0x10, 0, 0, 0, 0, 0, 88, 48, 0, 0 },
    { 0, 0xB, 0xFF, 2, 0x10, 0, 0, 0, 0, 0, 176, 72, 0, 0 },
    { 0, 0xF, 0xFF, 2, 0x11, 0, 0, 0, 0, 0, 32, 104, 0, 0 },
    { 0, 0xE, 0xFF, 2, 0x11, 0, 0, 0, 0, 0, 152, 128, 0, 0 },
    { 0, 0xD, 0xFF, 2, 0x11, 0, 0, 0, 0, 0, 168, 48, 0, 0 },
    { 0, 0x10, 0xFF, 2, 0x12, 0, 0, 0, 0, 0, 56, 112, 0, 0 },
    { 0, 0x11, 0xFF, 2, 0x12, 0, 0, 0, 0, 0, 112, 144, 0, 0 },
    { 0, 0x12, 0xFF, 2, 0x12, 0, 0, 0, 0, 0, 208, 88, 0, 0 }, { 0, 0x16, 0xFF, 6, 0x21, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 128, 607, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 128, 907, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 675, 970, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 284, 1043, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 711, 813, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 339, 392, 409, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag526_map_events[10] = {
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 0x11, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x22F, 0x3C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x220, 0x372, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x251, 0x2F7, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x260, 0x2A1, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xA, 0x1C0, 0x181, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xA, 0x1D0, 0xDA, 0, 0, 0, 0 }, { 0x601D, 1, 0xFFFF, 0, 8, 0x2F8, 0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 0x11, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag526_funcs = { wstag526_setup };
FieldstgEventDef wstag526_events[2] = {
    { 760, D_WSTAG526_800A7610, 0x013C0023, NULL, wstag526_event_760_end }, { -1, NULL, 0, NULL, NULL },
};
