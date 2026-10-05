#include "wstag.h"

/* WSTAG800: stage 0x2DB (fieldstg_stages). */

extern s16 D_WSTAG800_800A76F8[];
extern WstagAnimKey D_WSTAG800_800A7730[];
extern WstagAnimKey D_WSTAG800_800A7748[];
extern WstagFuncs wstag800_funcs;
/* WstagDoorObject with s16 flags. */
typedef struct Wstag800Door {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 opening; /* opening */
    /* 0x52 */ s16 starts_open; /* starts open */
    /* 0x54 */ WstagSpriteAnim sprites[2];
} Wstag800Door; /* size 0x64 */

extern WstagAnimKeyB D_WSTAG800_800A7604[];
extern WstagAnimKeyB D_WSTAG800_800A7610[];
extern WstagAnimKeyB D_WSTAG800_800A763C[];
void wstag800_door_update(Wstag800Door *obj);
/* An animation played once on a sprite (types 3 and 4). */
typedef struct Wstag800Slot {
    /* 0x0 */ FieldstgSprite *sprite;
    /* 0x4 */ s32 active;
    /* 0x8 */ WstagAnim anim;
} Wstag800Slot; /* size 0xC */

typedef struct Wstag800Pair {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unk_50;
    /* 0x54 */ Wstag800Slot slots[2];
} Wstag800Pair; /* size 0x6C */

extern WstagAnimKeyB D_WSTAG800_800A7658[];
extern WstagAnimKeyB D_WSTAG800_800A76A4[];
extern WstagAnimKeyB *D_WSTAG800_800A76F0[];
void wstag800_pair_update(Wstag800Pair *obj);
void wstag800_countdown_update();
/* The data of the stage object. */
typedef struct Wstag800Data {
    /* 0x00 */ void *countdown;
    /* 0x04 */ WstagTwoSpriteObject *objs[4];
    /* 0x14 */ void *door;
    /* 0x18 */ s32 unk_18;
    /* 0x1C */ FieldstgEvent *event;
} Wstag800Data; /* size 0x20 */

extern WstagSpawn D_WSTAG800_800A7700[];
extern FieldstgBattleLists wstag800_battle_lists;
extern FieldstgVramPlace wstag800_vram_places[];
extern FieldstgPlacedActor *wstag800_actors[];
extern FieldstgSprite wstag800_sprites[];
extern FieldstgMapEvent wstag800_map_events[];
extern FieldstgEventDef wstag800_events[];
void wstag800_update(WstagObject *obj, Wstag800Data *data);
Object *wstag800_countdown_new(void);
Object *wstag800_door_create(s32 arg0);
WstagTwoSpriteObject *wstag800_two_sprite_create(s32 x, s32 y, s32 frame);
Wstag800Door *wstag800_door_new(void);
void wstag800_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

s32 wstag800_sprite_anim_advance_b(WstagSpriteAnim *sa, WstagAnimKeyB *keys, s32 once, s32 depth) {
    WstagAnimKeyB *key = &keys[sa->anim.key];
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
        wstag800_sprite_anim_advance_b(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag800_door_update(Wstag800Door *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprites[0].anim.key = 0;
                obj->sprites[0].anim.time = D_WSTAG800_800A7604[0].time;
                obj->sprites[0].sprite = sprite;
            }
            if (sprite->type == 2) {
                obj->sprites[1].anim.key = 0;
                obj->sprites[1].anim.time = D_WSTAG800_800A763C[0].time;
                obj->sprites[1].sprite = sprite;
            }
        }
        obj->opening = 0;
        if (obj->starts_open == 0) {
            obj->base.next_state(obj);
        } else {
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG800_800A7610[0].time;
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            spr = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                spr->shown = 1;
                spr->sprite = 0x46;
                spr->frame = wstag800_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG800_800A7604, 0, 0);
                break;
            case 1:
                spr->shown = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
        for (i = 0; i < 2; i++) {
            spr = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                spr->shown = 1;
                if (obj->opening != 0) {
                    frame = wstag800_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG800_800A7610, 1, 0);
                    if (frame == 0xFF) {
                        spr->sprite = 0x50;
                        obj->opening = 0;
                    } else {
                        spr->sprite = frame;
                    }
                } else {
                    spr->sprite = 0x50;
                }
                spr->frame = 0;
                break;
            case 1:
                spr->shown = 1;
                spr->sprite = 0x51;
                spr->frame = wstag800_sprite_anim_advance_b(&obj->sprites[1], D_WSTAG800_800A763C, 0, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag800_door_message(Wstag800Door *obj, s32 msg) {
    if (msg == 0x35B) {
        obj->opening = 1;
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG800_800A7610[0].time;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}


Object *wstag800_door_create(s32 arg0) {
    return object_create(wstag800_door_update, sizeof(Wstag800Door), 0, arg0);
}

Wstag800Door *wstag800_door_new(void) {
    Wstag800Door *obj = object_new(wstag800_door_update, sizeof(Wstag800Door), 0);

    obj->starts_open = 1;
    return obj;
}

s32 wstag800_slot_anim_advance_b(Wstag800Slot *slot, WstagAnimKeyB *keys, s32 once, s32 depth) {
    WstagAnimKeyB *key = &keys[slot->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        slot->anim.time -= step;
    }
    if (slot->anim.time <= 0) {
        key++;
        slot->anim.key++;
        slot->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            slot->anim.key = 0;
            slot->anim.time += key->time;
        }
        wstag800_slot_anim_advance_b(slot, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag800_pair_update(Wstag800Pair *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 3) {
                obj->slots[0].active = 0;
                obj->slots[0].sprite = sprite;
            }
            if (sprite->type == 4) {
                obj->slots[1].active = 0;
                obj->slots[1].sprite = sprite;
            }
        }
        obj->slots[0].active = 1;
        obj->slots[0].anim.key = 0;
        obj->slots[0].anim.time = D_WSTAG800_800A7658[0].time;
        obj->slots[1].active = 1;
        obj->slots[1].anim.key = 0;
        obj->slots[1].anim.time = D_WSTAG800_800A76A4[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            spr = obj->slots[i].sprite;
            if (obj->slots[i].active) {
                frame = wstag800_slot_anim_advance_b(&obj->slots[i], D_WSTAG800_800A76F0[i], 1, 0);
                if (frame == 0xFF) {
                    obj->slots[i].active = 0;
                    spr->shown = 0;
                } else {
                    spr->shown = 1;
                    if (i == 0) {
                        spr->sprite = frame;
                        spr->frame = 0;
                    } else {
                        spr->sprite = 0x33;
                        spr->frame = frame;
                    }
                }
            } else {
                spr->shown = 0;
            }
        }
        if (obj->slots[0].active == 0 && obj->slots[1].active == 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag800_pair_create(s32 arg0) {
    return object_create(wstag800_pair_update, sizeof(Wstag800Pair), 0, arg0);
}

void wstag800_countdown_draw(WstagObject *obj) {
    Sprite spr;
    GamestatePos pos;
    GfxLayer *layer;
    s32 i;

    layer = gfx_module.funcs.get_layer(0x1002);
    i = 0;
    layer->get_scroll(layer, &pos.x);
    sprite_init(&spr);
    spr.set_layer(layer, 0);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.draw(cdload_module.get_subfile_by_id(0x6FE0000), 1, pos.x + 0xE0, pos.y + 0x16);
    pos.y += 0x19;
    for (; i < 3; i++) {
        spr.draw(cdload_module.get_subfile_by_id(0x6FE0000), gamestate_data.countdown[i] + 2, pos.x + D_WSTAG800_800A76F8[i],
                   pos.y);
    }
}

void wstag800_countdown_update(WstagObject *obj, WstagEventData *data) {
    u8 *timer;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class A1, sched1 barrier; DECISIONS "LOOP_BLOCK audit"): the original's temporaries in the digit
         * carries come from sched1's order (without sched1 both forms are the same). */
        LOOP_BLOCK(if (gamestate_flags.get_flag(0x4043, 0) == 0 && fieldstg_stage.event_running == 0
            && fieldstg_stage.battle_starting == 0 && fieldstg_stage.title_shown == 0 && fieldstg_stage.menu_open == 0
            && fieldstg_stage.actor_busy == 0) {
            wstag800_countdown_draw(obj);
            timer = gamestate_data.countdown;
            if (timer[0] != 0 || timer[1] != 0 || timer[2] != 0) {
                timer[3] -= gfx_module.funcs.get_frame_ticks();
                if (timer[3] > 60) {
                    timer[2]--;
                    timer[3] += 60;
                    sound_module.play(0x800452C6);
                }
                if (timer[2] >= 10) {
                    timer[2] = 9;
                    timer[1]--;
                }
                if (timer[1] >= 10) {
                    timer[1] = 9;
                    timer[0]--;
                }
            } else {
                data->event = fieldstg_event_start(0x5E2);
                obj->base.next_state(obj);
            }
        });
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag800_countdown_new(void) {
    return object_new(wstag800_countdown_update, sizeof(WstagObject), sizeof(FieldstgEvent *)); /* data: the event only */
}

void wstag800_update(WstagObject *obj, Wstag800Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->countdown = wstag800_countdown_new();
        for (i = 0; i < 4; i++) {
            if (D_WSTAG800_800A7700[i].condition == 0) {
                data->objs[i] = wstag800_two_sprite_create(D_WSTAG800_800A7700[i].x, D_WSTAG800_800A7700[i].y,
                                                       D_WSTAG800_800A7700[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x4044, 1) && gamestate_flags.get_flag(0x4045, 0)) {
            data->event = fieldstg_event_start(0x367);
        }
        if (gamestate_flags.get_flag(0x4061, 0)) {
            data->door = wstag800_door_create(0x345);
        } else {
            data->door = wstag800_door_new();
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag800_start(void *arg0) {
    WstagObject *obj = object_new(wstag800_update, sizeof(WstagObject), sizeof(Wstag800Data));

    obj->manager = arg0;
    wstag800_funcs.setup();
    return obj;
}

s32 wstag800_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag800_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag800_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
    Sprite spr;
    GfxLayer *layer = arg1;
    WstagFrame *frame = &obj->sprites[which];
    s32 y = obj->y;
    s32 x = obj->x;
    s32 depth;

    if (which == 1) {
        y -= 0x20;
        depth = 4;
    } else {
        depth = 6;
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_layer(layer, depth);
    spr.set_palette(frame->palette);
    spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), frame->frame, x, y);
}

s32 wstag800_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
    GfxRect view;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);

    layer->get_view_rect(layer, &view);
    if (x + w < view.x) {
        return 0;
    }
    if (view.x + view.w < x) {
        return 0;
    }
    if (y + h < view.y) {
        return 0;
    }
    return view.y + view.h >= y;
}

void wstag800_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG800_800A7748[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG800_800A7730[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag800_anim_advance(&obj->palette_anim, D_WSTAG800_800A7730, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag800_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag800_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG800_800A7748[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG800_800A7730[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag800_anim_advance(&obj->palette_anim, D_WSTAG800_800A7730, 0, 0);
        done = 0;
        frame = wstag800_anim_advance(&obj->frame_anim, D_WSTAG800_800A7748, 1, 0);
        switch (frame) {
        case 0xFF:
            obj->sprites[1].frame = 0;
            done = 1;
            break;
        case 0x12C:
            obj->sprites[1].frame = 0;
            break;
        default:
            obj->sprites[1].frame = obj->frame + frame;
            break;
        }
        if (done) {
            obj->base.set_state(obj, OBJECT_STATE_RUN);
        }
        if (obj->sprites[0].frame != 0 && wstag800_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag800_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag800_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag800_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag800_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag800_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag800_event_850_end(void) {
    gamestate_flags.set_flag(0x1C0A, 1);
    gamestate_flags.set_flag(0x4061, 1);
}

void wstag800_event_870_end(void) {
    gamestate_flags.set_flag(0x4044, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag800_event_871_end(void) {
    gamestate_flags.set_flag(0x4045, 1);
    gamestate_flags.set_flag(0x84CB, 1);
}

void wstag800_setup(void) {
    fieldstg_stage.background_file = 0x725;
    fieldstg_stage.sprite_file = 0x06FE0000;
    fieldstg_stage.sprites = wstag800_sprites;
    fieldstg_stage.map_events = wstag800_map_events;
    fieldstg_stage.mask_file = 0x6FA;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1E500, 0x38700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag800_vram_places;
    fieldstg_stage.music = 0xD;
    fieldstg_stage.sound = 0x60340000;
    fieldstg_stage.actors = wstag800_actors;
    fieldstg_stage.battle_lists = &wstag800_battle_lists;
    fieldstg_stage.events = wstag800_events;
    fieldstg_attr.set_file(0, 0x06FE0001);
    fieldstg_attr.set_file(7, 0x06FE0002);
    fieldstg_attr.set_file(4, 0x06FE0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag800_setup(void);

s16 D_WSTAG800_800A71E0[115] = {
    FIELDSTG_EVENT_WALK(2, 956, 823, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x345, 859, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 895, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 992, 840, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG800_800A72C8[67] = {
    FIELDSTG_EVENT_WALK(2, 1424, 432, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 208),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 208),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(208, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 208, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(208, 1448, 420, 1),
    FIELDSTG_EVENT_WAIT_WALK(208),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG800_800A7350[304] = {
    FIELDSTG_EVENT_PLACE(2, 1424, 432),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(316, 1448, 420),
    FIELDSTG_EVENT_ANIM(316, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1432, 428, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1527, 380, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1543, 388, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x347, 859, 2),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 4),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 893, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG800_800A75B0[42] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x26D, 1, 1, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKeyB D_WSTAG800_800A7604[3] = { { 0, 8, 0 }, { 1, 8, 0 }, { 255, 0, 0 } };
WstagAnimKeyB D_WSTAG800_800A7610[11] = {
    { 71, 4, 0 }, { 72, 4, 0 }, { 73, 4, 0 }, { 74, 4, 0 }, { 75, 4, 0 }, { 76, 4, 0 }, { 77, 4, 0 }, { 78, 4, 0 },
    { 79, 4, 0 }, { 80, 4, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG800_800A763C[7] = {
    { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG800_800A7658[19] = {
    { 52, 6, 0 }, { 53, 6, 0 }, { 54, 6, 0 }, { 55, 6, 0 }, { 56, 6, 0 }, { 57, 6, 0 }, { 58, 6, 0 }, { 59, 6, 0 },
    { 60, 6, 0 }, { 61, 6, 0 }, { 62, 6, 0 }, { 63, 6, 0 }, { 64, 6, 0 }, { 65, 6, 0 }, { 66, 6, 0 }, { 67, 6, 0 },
    { 68, 6, 0 }, { 69, 6, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG800_800A76A4[19] = {
    { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 0, 6, 0 }, { 1, 6, 0 },
    { 2, 6, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 3, 6, 0 },
    { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB *D_WSTAG800_800A76F0[2] = { D_WSTAG800_800A7658, D_WSTAG800_800A76A4 };
s16 D_WSTAG800_800A76F8[4] = { 226, 253, 280, 0 };
WstagSpawn D_WSTAG800_800A7700[4] = {
    { 28, 0, 1212, 627 }, { 28, 0, 924, 883 }, { 20, 0, 700, 211 }, { 20, 0, 1084, 803 },
};
WstagAnimKey D_WSTAG800_800A7730[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG800_800A7748[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG800_800A77A0 = { 123, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77AC = { 123, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77B8 = { 123, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77C4 = { 122, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77D0 = { 122, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77DC = { 122, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77E8 = { 102, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A77F4 = { 102, 16, 0x60080000 };
FieldstgBattleList D_WSTAG800_800A7800 = {
    3,
    { &D_WSTAG800_800A77A0, &D_WSTAG800_800A77AC, &D_WSTAG800_800A77B8, &D_WSTAG800_800A77C4, &D_WSTAG800_800A77D0,
        &D_WSTAG800_800A77DC, &D_WSTAG800_800A77E8, &D_WSTAG800_800A77F4 },
};
FieldstgListedBattle D_WSTAG800_800A7824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A783C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7860 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A786C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7878 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG800_800A7884 = {
    0,
    { &D_WSTAG800_800A7824, &D_WSTAG800_800A7830, &D_WSTAG800_800A783C, &D_WSTAG800_800A7848, &D_WSTAG800_800A7854,
        &D_WSTAG800_800A7860, &D_WSTAG800_800A786C, &D_WSTAG800_800A7878 },
};
FieldstgListedBattle D_WSTAG800_800A78A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A78FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG800_800A7908 = {
    0,
    { &D_WSTAG800_800A78A8, &D_WSTAG800_800A78B4, &D_WSTAG800_800A78C0, &D_WSTAG800_800A78CC, &D_WSTAG800_800A78D8,
        &D_WSTAG800_800A78E4, &D_WSTAG800_800A78F0, &D_WSTAG800_800A78FC },
};
FieldstgListedBattle D_WSTAG800_800A792C = { 193, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG800_800A7938 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7944 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7950 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A795C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7968 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7974 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG800_800A7980 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG800_800A798C = {
    0,
    { &D_WSTAG800_800A792C, &D_WSTAG800_800A7938, &D_WSTAG800_800A7944, &D_WSTAG800_800A7950, &D_WSTAG800_800A795C,
        &D_WSTAG800_800A7968, &D_WSTAG800_800A7974, &D_WSTAG800_800A7980 },
};
FieldstgBattleLists wstag800_battle_lists = {
    91, 0, 0, { &D_WSTAG800_800A7800, &D_WSTAG800_800A7884, &D_WSTAG800_800A7908 }, &D_WSTAG800_800A798C,
};
FieldstgVramPlace wstag800_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 412, 344, 368, 88, 368, 473 }, { 384, 256, 418, 344, 392, 88, 368, 472 },
    { 384, 256, 424, 344, 416, 88, 368, 471 }, { 384, 256, 396, 368, 304, 112, 368, 470 },
    { 384, 256, 402, 368, 328, 112, 368, 469 }, { 320, 256, 366, 424, 184, 168, 368, 468 },
    { 320, 256, 378, 432, 232, 176, 368, 467 },
};
u16 D_WSTAG800_800A7A9C[6] = { 0x245, 1, 0x8AE0, 1, 0xFFFF, 0 };
u16 D_WSTAG800_800A7AA8[6] = { 0x246, 1, 0x8AEA, 1, 0xFFFF, 0 };
u16 D_WSTAG800_800A7AB4[6] = { 0x247, 1, 0x822C, 1, 0xFFFF, 0 };
u16 D_WSTAG800_800A7AC0[4] = { 0x248, 1, 0xFFFF, 0 };
u16 D_WSTAG800_800A7AC8[6] = { 0x249, 1, 0x8B11, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG800_800A7AD4[2] = { { NULL, D_WSTAG800_800A7A9C, 539 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG800_800A7AEC[2] = { { NULL, D_WSTAG800_800A7AA8, 540 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG800_800A7B04[2] = { { NULL, D_WSTAG800_800A7AB4, 381 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG800_800A7B1C[2] = { { NULL, D_WSTAG800_800A7AC0, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG800_800A7B34[2] = { { NULL, D_WSTAG800_800A7AC8, 543 }, { NULL, NULL, 0 } };
u16 D_WSTAG800_800A7B4C[4] = { 0x245, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B54[4] = { 0x246, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B5C[4] = { 0x247, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B64[4] = { 0x248, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B6C[4] = { 0x249, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B74[6] = { 0x6020, 1, 0x4044, 0, 0xFFFF, 0 };
u16 D_WSTAG800_800A7B80[4] = { 0x6020, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG800_800A7B88 = { D_WSTAG800_800A7B4C, D_WSTAG800_800A7AD4, 33, 4, 1024, 168, 1 };
FieldstgPlacedActor D_WSTAG800_800A7B9C = { D_WSTAG800_800A7B54, D_WSTAG800_800A7AEC, 77, 5, 640, 938, 1 };
FieldstgPlacedActor D_WSTAG800_800A7BB0 = { D_WSTAG800_800A7B5C, D_WSTAG800_800A7B04, 78, 6, 1217, 328, 1 };
FieldstgPlacedActor D_WSTAG800_800A7BC4 = { D_WSTAG800_800A7B64, D_WSTAG800_800A7B1C, 79, 7, 1249, 281, 1 };
FieldstgPlacedActor D_WSTAG800_800A7BD8 = { D_WSTAG800_800A7B6C, D_WSTAG800_800A7B34, 80, 8, 352, 697, 1 };
FieldstgPlacedActor D_WSTAG800_800A7BEC = { D_WSTAG800_800A7B74, NULL, 208, 9, 1473, 409, 5 };
FieldstgPlacedActor D_WSTAG800_800A7C00 = { D_WSTAG800_800A7B80, NULL, 316, 10, 0, 0, 1 };
FieldstgPlacedActor *wstag800_actors[8] = {
    &D_WSTAG800_800A7B88, &D_WSTAG800_800A7B9C, &D_WSTAG800_800A7BB0, &D_WSTAG800_800A7BC4, &D_WSTAG800_800A7BD8,
    &D_WSTAG800_800A7BEC, &D_WSTAG800_800A7C00, NULL,
};
FieldstgSprite wstag800_sprites[21] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 728, 212, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 760, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 952, 884, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 984, 868, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 1112, 804, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 1144, 788, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 1240, 629, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 4, 0, 1272, 612, 0, 0 }, { 1, 2, 0x64, 6, 0x51, 0, 0, 0, 0, 0, 898, 764, 0, 0 },
    { 1, 1, 0x64, 6, 0x46, 0, 0, 0, 0, 0, 898, 764, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 656, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 688, 160, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 880, 848, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 912, 832, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 1040, 768, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 1072, 752, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 1168, 592, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 4, 0, 1200, 576, 0, 0 },
    { 0, 4, 0x64, 6, 0x33, 0, 0, 0, 0, 0, 1550, 338, 0, 0 },
    { 0, 3, 0x64, 6, 0x34, 0, 0, 0, 0, 0, 1550, 338, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag800_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DA, 0x4F2, 0x150, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DA, 0x522, 0x1F6, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DA, 0x552, 0x28E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DA, 0x554, 0x400, 3, 0, 0, 0 },
    { 0x6020, 1, 0x4044, 0, 8, 0x366, 0, 0, 0, 0, 0, 0 }, { 0x1C05, 1, 0x4061, 0, 8, 0x352, 0, 0, 0, 0, 0, 0 },
    { 0x1C05, 1, 0xFFFF, 0, 0xD, 0x2DB, 0x450, 0x330, 1, 0, 0, 0 },
    { 0x1C05, 1, 0xFFFF, 0, 0xD, 0x2DB, 0x2D0, 0xE0, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag800_funcs = { wstag800_setup };
FieldstgEventDef wstag800_events[5] = {
    { 850, D_WSTAG800_800A71E0, 0x01510000, NULL, wstag800_event_850_end },
    { 870, D_WSTAG800_800A72C8, 0x01510001, NULL, wstag800_event_870_end },
    { 871, D_WSTAG800_800A7350, 0x01510002, NULL, wstag800_event_871_end },
    { 1506, D_WSTAG800_800A75B0, 0x01510012, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
