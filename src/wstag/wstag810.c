#include "wstag.h"

/* WSTAG810: stage 0x2DD (fieldstg_stages). */

extern WstagAnimKey D_WSTAG810_800A74DC[];
extern WstagAnimKey D_WSTAG810_800A74F4[];
extern WstagFuncs wstag810_funcs;
/* WstagDoorObject over the sprites of types `type` and `type` + 1. */
typedef struct Wstag810Door {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 opening; /* opening */
    /* 0x51 */ u8 starts_open; /* 1: starts open */
    /* 0x52 */ s16 type;
    /* 0x54 */ WstagSpriteAnim sprites[2];
    /* 0x64 */ s32 unk_64;
} Wstag810Door; /* size 0x68 */

extern WstagAnimKeyB D_WSTAG810_800A718C[];
extern WstagAnimKeyB D_WSTAG810_800A719C[];
extern WstagAnimKeyB D_WSTAG810_800A71D8[];
void wstag810_door_update(Wstag810Door *obj);
typedef struct Wstag810Data {
    /* 0x000 */ WstagTwoSpriteObject *objs[0x3E];
    /* 0x0F8 */ Wstag810Door *doors[3];
} Wstag810Data; /* size 0x104 */

extern WstagSpawn D_WSTAG810_800A71F4[];
extern FieldstgMapEvent wstag810_map_events[];
extern FieldstgMapEvent wstag810_map_events2[];
extern FieldstgBattleLists wstag810_battle_lists;
extern FieldstgVramPlace wstag810_vram_places[];
extern FieldstgPlacedActor *wstag810_actors[];
extern FieldstgSprite wstag810_sprites[];
extern FieldstgEventDef wstag810_events[];
void wstag810_update(WstagObject *obj, Wstag810Data *data);
WstagTwoSpriteObject *wstag810_two_sprite_create(s32 x, s32 y, s32 frame);
void wstag810_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

s32 wstag810_sprite_anim_advance_b(WstagSpriteAnim *sa, WstagAnimKeyB *keys, s32 once, s32 depth) {
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
        wstag810_sprite_anim_advance_b(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag810_door_update(Wstag810Door *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == obj->type) {
                obj->sprites[0].anim.key = 0;
                obj->sprites[0].anim.time = D_WSTAG810_800A718C[0].time;
                obj->sprites[0].sprite = sprite;
            }
            if (sprite->type == obj->type + 1) {
                obj->sprites[1].anim.key = 0;
                obj->sprites[1].anim.time = D_WSTAG810_800A71D8[0].time;
                obj->sprites[1].sprite = sprite;
            }
        }
        obj->opening = 0;
        if (obj->starts_open == 1) {
            obj->opening = 1;
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG810_800A719C[0].time;
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        } else {
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            spr = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                spr->shown = 1;
                spr->sprite = 0x55;
                spr->frame = wstag810_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG810_800A718C, 0, 0);
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
                    frame = wstag810_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG810_800A719C, 1, 0);
                    if (frame == 0xFF) {
                        spr->sprite = 0x54;
                        obj->opening = 0;
                    } else {
                        spr->sprite = frame;
                    }
                } else {
                    spr->sprite = 0x54;
                }
                spr->frame = 0;
                break;
            case 1:
                spr->shown = 1;
                spr->sprite = 0x46;
                spr->frame = wstag810_sprite_anim_advance_b(&obj->sprites[1], D_WSTAG810_800A71D8, 0, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag810_door_message(Wstag810Door *obj, s32 msg) {
    if (msg == 0x35B) {
        obj->opening = 1;
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG810_800A719C[0].time;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}


Wstag810Door *wstag810_door_create_1(s32 arg0) {
    Wstag810Door *obj = object_create(wstag810_door_update, sizeof(Wstag810Door), 0, arg0);

    obj->type = 1;
    obj->starts_open = 0;
    return obj;
}

Wstag810Door *wstag810_door_create_3(s32 arg0) {
    Wstag810Door *obj = object_create(wstag810_door_update, sizeof(Wstag810Door), 0, arg0);

    obj->type = 3;
    obj->starts_open = 0;
    return obj;
}

Wstag810Door *wstag810_door_create_5(s32 arg0) {
    Wstag810Door *obj = object_create(wstag810_door_update, sizeof(Wstag810Door), 0, arg0);

    obj->type = 5;
    obj->starts_open = 0;
    return obj;
}

Wstag810Door *wstag810_door_new(s32 type) {
    Wstag810Door *obj = object_new(wstag810_door_update, sizeof(Wstag810Door), 0);

    obj->type = type;
    obj->starts_open = 1;
    return obj;
}

void wstag810_update(WstagObject *obj, Wstag810Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 0x3E; i++) {
            if (D_WSTAG810_800A71F4[i].condition == 0) {
                data->objs[i] = wstag810_two_sprite_create(D_WSTAG810_800A71F4[i].x, D_WSTAG810_800A71F4[i].y,
                                                       D_WSTAG810_800A71F4[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x406E, 0)) {
            data->doors[0] = wstag810_door_create_1(0x34C);
        } else {
            data->doors[0] = wstag810_door_new(1);
        }
        if (gamestate_flags.get_flag(0x406F, 0)) {
            data->doors[1] = wstag810_door_create_3(0x34B);
        } else {
            data->doors[1] = wstag810_door_new(3);
        }
        if (gamestate_flags.get_flag(0x406D, 0)) {
            data->doors[2] = wstag810_door_create_5(0x34A);
        } else {
            data->doors[2] = wstag810_door_new(5);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag810_start(void *arg0) {
    WstagObject *obj = object_new(wstag810_update, sizeof(WstagObject), sizeof(Wstag810Data));

    obj->manager = arg0;
    wstag810_funcs.setup();
    return obj;
}

s32 wstag810_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag810_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag810_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag810_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag810_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG810_800A74F4[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG810_800A74DC[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag810_anim_advance(&obj->palette_anim, D_WSTAG810_800A74DC, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag810_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag810_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG810_800A74F4[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG810_800A74DC[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag810_anim_advance(&obj->palette_anim, D_WSTAG810_800A74DC, 0, 0);
        done = 0;
        frame = wstag810_anim_advance(&obj->frame_anim, D_WSTAG810_800A74F4, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag810_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag810_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag810_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag810_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag810_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag810_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

Object *wstag810_event_9000_start(void) {
    if (gamestate_data.unk_26E4 != 0) {
        fieldstg_attr.set_file(7, 0x06ED0003);
        fieldstg_stage.map_events = wstag810_map_events;
        gamestate_data.unk_26E4 = 0;
    }
    return NULL;
}

Object *wstag810_event_9001_start(void) {
    if (gamestate_data.unk_26E4 == 0) {
        fieldstg_attr.set_file(7, 0x06ED0004);
        fieldstg_stage.map_events = wstag810_map_events2;
        gamestate_data.unk_26E4 = 0x20;
    }
    return NULL;
}

void wstag810_event_990_end(void) {
    gamestate_flags.set_flag(0x406D, 1);
    gamestate_flags.set_flag(0x1C3A, 1);
}

void wstag810_event_1000_end(void) {
    gamestate_flags.set_flag(0x406E, 1);
    gamestate_flags.set_flag(0x1C3B, 1);
}

void wstag810_event_1010_end(void) {
    gamestate_flags.set_flag(0x406F, 1);
    gamestate_flags.set_flag(0x1C3C, 1);
}

void wstag810_event_1035_end(void) {
    gamestate_data.progress = 0x29;
}

void wstag810_setup(void) {
    fieldstg_stage.background_file = 0x6EC;
    fieldstg_stage.sprite_file = 0x06ED0000;
    fieldstg_stage.sprites = wstag810_sprites;
    fieldstg_stage.mask_file = 0x6EB;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x31000, 0x1F800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag810_vram_places;
    fieldstg_stage.music = 0x1C;
    fieldstg_stage.sound = 0x60700000;
    fieldstg_stage.actors = wstag810_actors;
    fieldstg_stage.battle_lists = &wstag810_battle_lists;
    fieldstg_stage.events = wstag810_events;
    fieldstg_attr.set_file(0, 0x06ED0001);
    fieldstg_attr.set_file(4, 0x06ED0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.map_is_new != 0 || gamestate_data.unk_26E4 == 0) {
        gamestate_data.unk_26E4 = 1;
        wstag810_event_9000_start();
    } else {
        gamestate_data.unk_26E4 = 0;
        wstag810_event_9001_start();
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag810_setup(void);

s16 D_WSTAG810_800A6D24[155] = {
    FIELDSTG_EVENT_WALK(2, 1328, 320, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_ANIM(0x34A, 859, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(384, 0, 0),
    FIELDSTG_EVENT_ANIM(384, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 884, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1296, 337, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG810_800A6E5C[155] = {
    FIELDSTG_EVENT_WALK(2, 912, 144, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_ANIM(0x34C, 859, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(385, 0, 0),
    FIELDSTG_EVENT_ANIM(385, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 884, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 880, 160, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG810_800A6F94[155] = {
    FIELDSTG_EVENT_WALK(2, 1296, 144, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_ANIM(0x34B, 859, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(386, 0, 0),
    FIELDSTG_EVENT_ANIM(386, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 884, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1295, 145, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG810_800A70CC[96] = {
    FIELDSTG_EVENT_WALK(2, 160, 729, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
WstagAnimKeyB D_WSTAG810_800A718C[4] = { { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 255, 0, 0 } };
WstagAnimKeyB D_WSTAG810_800A719C[15] = {
    { 71, 4, 0 }, { 72, 4, 0 }, { 73, 4, 0 }, { 74, 4, 0 }, { 75, 4, 0 }, { 76, 4, 0 }, { 77, 4, 0 }, { 78, 4, 0 },
    { 79, 4, 0 }, { 80, 4, 0 }, { 81, 4, 0 }, { 82, 4, 0 }, { 83, 4, 0 }, { 84, 4, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG810_800A71D8[7] = {
    { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 0 },
};
WstagSpawn D_WSTAG810_800A71F4[62] = {
    { 60, 0, 92, 181 }, { 60, 0, 92, 245 }, { 60, 0, 92, 405 }, { 52, 0, 92, 469 }, { 60, 0, 156, 213 },
    { 52, 0, 220, 245 }, { 60, 0, 220, 309 }, { 44, 0, 236, 748 }, { 44, 0, 252, 917 }, { 60, 0, 284, 149 },
    { 60, 0, 284, 469 }, { 44, 0, 284, 804 }, { 60, 0, 300, 716 }, { 60, 0, 316, 949 }, { 60, 0, 348, 181 },
    { 60, 0, 348, 836 }, { 60, 0, 380, 133 }, { 52, 0, 412, 469 }, { 60, 0, 444, 421 }, { 60, 0, 476, 277 },
    { 60, 0, 508, 549 }, { 52, 0, 540, 469 }, { 60, 0, 572, 133 }, { 60, 0, 572, 581 }, { 60, 0, 636, 613 },
    { 60, 0, 668, 405 }, { 60, 0, 700, 133 }, { 60, 0, 700, 645 }, { 60, 0, 732, 373 }, { 60, 0, 748, 1053 },
    { 60, 0, 764, 677 }, { 60, 0, 796, 245 }, { 60, 0, 828, 549 }, { 52, 0, 860, 437 }, { 60, 0, 876, 701 },
    { 52, 0, 876, 765 }, { 60, 0, 876, 829 }, { 60, 0, 940, 861 }, { 60, 0, 956, 325 }, { 60, 0, 956, 421 },
    { 60, 0, 972, 909 }, { 52, 0, 1004, 829 }, { 60, 0, 1004, 957 }, { 60, 0, 1020, 677 }, { 52, 0, 1068, 925 },
    { 52, 0, 1084, 133 }, { 52, 0, 1084, 581 }, { 60, 0, 1100, 1037 }, { 60, 0, 1116, 501 }, { 60, 0, 1164, 845 },
    { 60, 0, 1180, 437 }, { 52, 0, 1244, 245 }, { 52, 0, 1244, 501 }, { 60, 0, 1244, 661 }, { 60, 0, 1260, 861 },
    { 60, 0, 1260, 1053 }, { 52, 0, 1308, 437 }, { 60, 0, 1308, 565 }, { 60, 0, 1308, 693 }, { 60, 0, 1308, 757 },
    { 52, 0, 108, 732 }, { 44, 0, 204, 1037 },
};
WstagAnimKey D_WSTAG810_800A74DC[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG810_800A74F4[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG810_800A754C = { 185, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A7558 = { 185, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A7564 = { 185, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A7570 = { 135, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A757C = { 135, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A7588 = { 135, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A7594 = { 140, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG810_800A75A0 = { 140, 17, 0x60080000 };
FieldstgBattleList D_WSTAG810_800A75AC = {
    4,
    { &D_WSTAG810_800A754C, &D_WSTAG810_800A7558, &D_WSTAG810_800A7564, &D_WSTAG810_800A7570, &D_WSTAG810_800A757C,
        &D_WSTAG810_800A7588, &D_WSTAG810_800A7594, &D_WSTAG810_800A75A0 },
};
FieldstgListedBattle D_WSTAG810_800A75D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A75DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A75E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A75F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A760C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7618 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7624 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG810_800A7630 = {
    0,
    { &D_WSTAG810_800A75D0, &D_WSTAG810_800A75DC, &D_WSTAG810_800A75E8, &D_WSTAG810_800A75F4, &D_WSTAG810_800A7600,
        &D_WSTAG810_800A760C, &D_WSTAG810_800A7618, &D_WSTAG810_800A7624 },
};
FieldstgListedBattle D_WSTAG810_800A7654 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7660 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A766C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7678 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7684 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7690 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A769C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A76A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG810_800A76B4 = {
    0,
    { &D_WSTAG810_800A7654, &D_WSTAG810_800A7660, &D_WSTAG810_800A766C, &D_WSTAG810_800A7678, &D_WSTAG810_800A7684,
        &D_WSTAG810_800A7690, &D_WSTAG810_800A769C, &D_WSTAG810_800A76A8 },
};
FieldstgListedBattle D_WSTAG810_800A76D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A76E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A76F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A76FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7708 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7714 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A7720 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG810_800A772C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG810_800A7738 = {
    0,
    { &D_WSTAG810_800A76D8, &D_WSTAG810_800A76E4, &D_WSTAG810_800A76F0, &D_WSTAG810_800A76FC, &D_WSTAG810_800A7708,
        &D_WSTAG810_800A7714, &D_WSTAG810_800A7720, &D_WSTAG810_800A772C },
};
FieldstgBattleLists wstag810_battle_lists = {
    124, 0, 0, { &D_WSTAG810_800A75AC, &D_WSTAG810_800A7630, &D_WSTAG810_800A76B4 }, &D_WSTAG810_800A7738,
};
FieldstgVramPlace wstag810_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 338, 433, 72, 177, 368, 504 }, { 320, 256, 354, 433, 136, 177, 320, 503 },
    { 384, 256, 384, 256, 256, 0, 336, 503 },
};
FieldstgTalk D_WSTAG810_800A7808[2] = { { NULL, NULL, 840 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG810_800A7820[2] = { { NULL, NULL, 840 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG810_800A7838[2] = { { NULL, NULL, 840 }, { NULL, NULL, 0 } };
u16 D_WSTAG810_800A7850[4] = { 0x1C3A, 0, 0xFFFF, 0 };
u16 D_WSTAG810_800A7858[4] = { 0x1C3B, 0, 0xFFFF, 0 };
u16 D_WSTAG810_800A7860[4] = { 0x1C3C, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG810_800A7868 = { D_WSTAG810_800A7850, D_WSTAG810_800A7808, 384, 4, 369, 912, 7 };
FieldstgPlacedActor D_WSTAG810_800A787C = { D_WSTAG810_800A7858, D_WSTAG810_800A7820, 385, 5, 400, 800, 7 };
FieldstgPlacedActor D_WSTAG810_800A7890 = { D_WSTAG810_800A7860, D_WSTAG810_800A7838, 386, 6, 208, 704, 7 };
FieldstgPlacedActor *wstag810_actors[4] = {
    &D_WSTAG810_800A7868, &D_WSTAG810_800A787C, &D_WSTAG810_800A7890, NULL,
};
FieldstgSprite wstag810_sprites[28] = {
    { 1, 2, 0x64, 6, 0x46, 0, 0, 0, 0, 0, 910, 80, 0, 0 }, { 1, 4, 0x64, 6, 0x46, 0, 0, 0, 0, 0, 1295, 80, 0, 0 },
    { 1, 6, 0x64, 6, 0x46, 0, 0, 0, 0, 0, 1327, 256, 0, 0 }, { 1, 1, 0x64, 6, 0x47, 0, 0, 0, 0, 0, 910, 80, 0, 0 },
    { 1, 3, 0x64, 6, 0x47, 0, 0, 0, 0, 0, 1295, 80, 0, 0 }, { 1, 5, 0x64, 6, 0x47, 0, 0, 0, 0, 0, 1327, 256, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 26, 975, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 47, 138, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 123, 576, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 125, 877, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 195, 656, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 231, 32, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 307, 580, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 431, 874, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 486, 102, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 532, 732, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 549, 1020, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 595, 820, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 615, 349, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 668, 33, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 713, 235, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 738, 837, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 890, 262, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 1037, 46, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 1054, 329, 0, 0 }, { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 1108, 403, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0, 0, 0, 0, 0, 0, 1354, 1060, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag810_map_events[32] = {
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x314, 0x41E, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x13C, 0x3B6, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x84, 0x196, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x44C, 0x40E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x4C4, 0x1B6, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x84, 0x10A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x3CC, 0x3A2, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x15C, 0x346, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x33C, 0x23A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x3EC, 0x3D2, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x224, 0x226, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x3AC, 0x372, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x2E4, 0x286, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x264, 0x246, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x11C, 0x1D6, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x324, 0x2A6, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x51C, 0x24A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x2A4, 0x266, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DE, 0x189, 0x2DF, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DE, 0xCE, 0x283, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x394, 0x33E, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x12C, 0x2E2, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DE, 0x3F8, 0x117, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DE, 0x33E, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x110, 0x3A0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x130, 0x330, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DE, 0x40C, 0x2D2, 1, 0, 0, 0 },
    { 0x406D, 0, 0xFFFF, 0, 8, 0x3DE, 0, 0, 0, 0, 0, 0 }, { 0x406E, 0, 0xFFFF, 0, 8, 0x3E8, 0, 0, 0, 0, 0, 0 },
    { 0x406F, 0, 0xFFFF, 0, 8, 0x3F2, 0, 0, 0, 0, 0, 0 }, { 0x6028, 1, 0xFFFF, 0, 8, 0x40B, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 8, 0x2329, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag810_map_events2[23] = {
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x11C, 0xAA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x394, 0x2D2, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x48C, 0x34E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x45C, 0x20A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x3FC, 0x2A6, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x184, 0xCA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x204, 0x12A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x104, 0x14A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x51C, 0x2CA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x1BC, 0x1A6, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x51C, 0x30A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x84, 0xB6, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x23C, 0x9A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0xC4, 0xD6, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x3E4, 0x1BA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x2BC, 0x9A, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x4EC, 0x372, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x1A4, 0x9A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x4EC, 0x41E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x304, 0x18A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DD, 0x100, 0x2F8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag810_funcs = { wstag810_setup };
FieldstgEventDef wstag810_events[7] = {
    { 990, D_WSTAG810_800A6D24, 0x01510006, NULL, wstag810_event_990_end },
    { 1000, D_WSTAG810_800A6E5C, 0x01510007, NULL, wstag810_event_1000_end },
    { 1010, D_WSTAG810_800A6F94, 0x01510008, NULL, wstag810_event_1010_end },
    { 1035, D_WSTAG810_800A70CC, 0x0151000B, NULL, wstag810_event_1035_end },
    { 9000, NULL, 0, wstag810_event_9000_start, NULL }, { 9001, NULL, 0, wstag810_event_9001_start, NULL },
    { -1, NULL, 0, NULL, NULL },
};
