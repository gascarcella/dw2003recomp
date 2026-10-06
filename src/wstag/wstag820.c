#include "wstag.h"

/* WSTAG820: stage 0x2DF (fieldstg_stages). */

/* The data of the stage object: an event and the placed objects. */
typedef struct Wstag820Data {
    /* 0x00 */ FieldstgEvent *event;
    /* 0x04 */ WstagTwoSpriteObject *objs[4];
} Wstag820Data; /* size 0x14 */

extern WstagAnimKey D_WSTAG820_800A711C[];
extern WstagSpawn D_WSTAG820_800A71AC[];
extern WstagAnimKey D_WSTAG820_800A71DC[];
extern WstagAnimKey D_WSTAG820_800A71F4[];
extern WstagFuncs wstag820_funcs;
extern FieldstgBattleLists wstag820_battle_lists;
extern FieldstgVramPlace wstag820_vram_places[];
extern FieldstgPlacedActor *wstag820_actors[];
extern FieldstgSprite wstag820_sprites[];
extern FieldstgMapEvent wstag820_map_events[];
extern FieldstgEventDef wstag820_events[];
void wstag820_update(WstagObject *obj, Wstag820Data *data);
void wstag820_two_sprite_update(WstagTwoSpriteObject *obj);
WstagTwoSpriteObject *wstag820_two_sprite_create(s32 x, s32 y, s32 frame);
WstagTwoSpriteObject *wstag820_two_sprite_create_fixed(s32 arg0);
void wstag820_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

s32 wstag820_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        if (key->frame == 0xFF) {
            return 0xFF;
        }
        wstag820_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag820_sprite_anim1_update(WstagSpriteAnim1Object *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprite.sprite = sprite;
            }
        }
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG820_800A711C[0].time;
        break;
    case OBJECT_STATE_RUN:
        spr = obj->sprite.sprite;
        spr->shown = 1;
        frame = wstag820_sprite_anim_play_once(&obj->sprite, D_WSTAG820_800A711C, 0);
        if (frame != 0xFF) {
            spr->sprite = frame;
        } else {
            spr->shown = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagSpriteAnim1Object *wstag820_sprite_anim1_create(s32 arg0) {
    WstagSpriteAnim1Object *obj = object_create(wstag820_sprite_anim1_update, sizeof(WstagSpriteAnim1Object), 0, arg0);

    sound_module.play(0x4001D);
    return obj;
}

void wstag820_update(WstagObject *obj, Wstag820Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 4; i++) {
            if (D_WSTAG820_800A71AC[i].condition == 0) {
                data->objs[i] = wstag820_two_sprite_create(D_WSTAG820_800A71AC[i].x, D_WSTAG820_800A71AC[i].y,
                                                       D_WSTAG820_800A71AC[i].frame);
            } else if (D_WSTAG820_800A71AC[i].condition == 2) {
                data->objs[i] = wstag820_two_sprite_create_fixed(0x34D);
            }
        }
        /* Evidence (class B, register priority only; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_data.progress == 0x29 && gamestate_flags.get_flag(0x4074, 0)) {
            data->event = fieldstg_event_start(0x424);
        } else if (gamestate_data.progress == 0x29 && gamestate_flags.get_flag(0x4074, 1) && gamestate_flags.get_flag(0x4075, 0)) {
            data->event = fieldstg_event_start(0x42E);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}



WstagObject *wstag820_start(void *arg0) {
    WstagObject *obj = object_new(wstag820_update, sizeof(WstagObject), sizeof(Wstag820Data));

    obj->manager = arg0;
    wstag820_funcs.setup();
    return obj;
}

s32 wstag820_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag820_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag820_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag820_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag820_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG820_800A71F4[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG820_800A71DC[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag820_anim_advance(&obj->palette_anim, D_WSTAG820_800A71DC, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag820_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag820_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG820_800A71F4[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG820_800A71DC[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag820_anim_advance(&obj->palette_anim, D_WSTAG820_800A71DC, 0, 0);
        done = 0;
        frame = wstag820_anim_advance(&obj->frame_anim, D_WSTAG820_800A71F4, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag820_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag820_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag820_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag820_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag820_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag820_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

WstagTwoSpriteObject *wstag820_two_sprite_create_fixed(s32 arg0) {
    WstagTwoSpriteObject *obj = object_create(wstag820_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, arg0);

    obj->x = 0x1CC;
    obj->y = 0x1B4;
    obj->frame = 0x3C;
    return obj;
}

void wstag820_two_sprite_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x335) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

void wstag820_event_1060_end(void) {
    gamestate_flags.set_flag(0x4074, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag820_event_1070_end(void) {
    gamestate_flags.set_flag(0x4075, 1);
}

void wstag820_event_1080_end(void) {
    gamestate_data.progress = 0x2B;
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag820_setup(void) {
    fieldstg_stage.background_file = 0x714;
    fieldstg_stage.sprite_file = 0x07150000;
    fieldstg_stage.sprites = wstag820_sprites;
    fieldstg_stage.map_events = wstag820_map_events;
    fieldstg_stage.mask_file = 0x713;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x24700, 0x18C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag820_vram_places;
    fieldstg_stage.music = 0x44;
    fieldstg_stage.actors = wstag820_actors;
    fieldstg_stage.battle_lists = &wstag820_battle_lists;
    fieldstg_stage.sound = 0x61100001;
    fieldstg_stage.events = wstag820_events;
    fieldstg_attr.set_file(0, 0x07150001);
    fieldstg_attr.set_file(7, 0x07150002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag820_setup(void);

s16 D_WSTAG820_800A69AC[217] = {
    FIELDSTG_EVENT_PLACE(1, 784, 552),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_PLACE(392, 578, 401),
    FIELDSTG_EVENT_ANIM(392, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 712, 516, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 680, 477, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 648, 438, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 632, 430, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(392, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 3, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 5, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 7, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 11, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG820_800A6B60[121] = {
    FIELDSTG_EVENT_PLACE(2, 632, 430),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(392, 578, 401),
    FIELDSTG_EVENT_ANIM(392, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(392, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 578, 426),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_WALK(392, 479, 448, 1),
    FIELDSTG_EVENT_WAIT_WALK(392),
    FIELDSTG_EVENT_ANIM(392, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 890, 2),
    FIELDSTG_EVENT_ANIM(0x34D, 821, 2),
    FIELDSTG_EVENT_WAIT(72),
    FIELDSTG_EVENT_PLACE(392, 0, 0),
    FIELDSTG_EVENT_ANIM(392, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG820_800A6C54[611] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 384, 256),
    FIELDSTG_EVENT_WALK(2, 384, 256, 3),
    FIELDSTG_EVENT_PLACE(213, 336, 232),
    FIELDSTG_EVENT_ANIM(213, 1, 3),
    FIELDSTG_EVENT_PLACE(215, 249, 177),
    FIELDSTG_EVENT_ANIM(215, 1, 7),
    FIELDSTG_EVENT_PLACE(216, 281, 118),
    FIELDSTG_EVENT_ANIM(216, 1, 7),
    FIELDSTG_EVENT_PLACE(217, 120, 197),
    FIELDSTG_EVENT_ANIM(217, 1, 7),
    FIELDSTG_EVENT_PLACE(218, 121, 117),
    FIELDSTG_EVENT_ANIM(218, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 248, 188),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 23, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 873, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(215, 80, 7),
    FIELDSTG_EVENT_ANIM(216, 80, 7),
    FIELDSTG_EVENT_ANIM(217, 80, 7),
    FIELDSTG_EVENT_ANIM(218, 80, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 878, 2),
    FIELDSTG_EVENT_WAIT_ANIM(215),
    FIELDSTG_EVENT_PLACE(215, 0, 0),
    FIELDSTG_EVENT_ANIM(215, 1, 0),
    FIELDSTG_EVENT_PLACE(216, 0, 0),
    FIELDSTG_EVENT_ANIM(216, 1, 0),
    FIELDSTG_EVENT_PLACE(217, 0, 0),
    FIELDSTG_EVENT_ANIM(217, 1, 0),
    FIELDSTG_EVENT_PLACE(218, 0, 0),
    FIELDSTG_EVENT_ANIM(218, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 288, 206),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(214, 288, 206),
    FIELDSTG_EVENT_ANIM(214, 79, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 888, 2),
    FIELDSTG_EVENT_WAIT_ANIM(214),
    FIELDSTG_EVENT_ANIM(214, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 214, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(213, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 213),
    FIELDSTG_EVENT_DIALOG(0, 2, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 288, 206),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 214, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(213, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 214, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 214, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 213, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_MOVE(0, 336, 232),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 10, 213, 4),
    FIELDSTG_EVENT_ANIM(213, 82, 1),
    FIELDSTG_EVENT_ANIM(214, 81, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 879, 2),
    FIELDSTG_EVENT_WAIT_ANIM(213),
    FIELDSTG_EVENT_PLACE(213, 0, 0),
    FIELDSTG_EVENT_ANIM(213, 1, 0),
    FIELDSTG_EVENT_ANIM(214, 1, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 11, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(214, 87, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 889, 2),
    FIELDSTG_EVENT_WAIT_ANIM(214),
    FIELDSTG_EVENT_PLACE(214, 0, 0),
    FIELDSTG_EVENT_ANIM(214, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 224, 134),
    FIELDSTG_EVENT_WALK(2, 288, 208, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(214, 194, 116),
    FIELDSTG_EVENT_ANIM(214, 79, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 888, 2),
    FIELDSTG_EVENT_WAIT_ANIM(214),
    FIELDSTG_EVENT_ANIM(214, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 813),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 18, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 19, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 20, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 21, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 22, 214, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 813),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_ANIM(0x355, 821, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 880, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32D, 881, 2),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG820_800A711C[36] = {
    { 61, 6 }, { 62, 6 }, { 63, 6 }, { 64, 6 }, { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 }, { 67, 4 },
    { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 },
    { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 }, { 67, 4 }, { 65, 4 }, { 66, 4 },
    { 67, 4 }, { 64, 6 }, { 63, 6 }, { 62, 6 }, { 61, 6 }, { 255, 999 },
};
WstagSpawn D_WSTAG820_800A71AC[4] = {
    { 60, 2, 460, 436 }, { 60, 0, 460, 436 }, { 60, 0, 652, 341 }, { 44, 0, 364, 242 },
};
WstagAnimKey D_WSTAG820_800A71DC[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG820_800A71F4[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG820_800A724C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A727C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7288 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7294 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A72A0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG820_800A72AC = {
    0,
    { &D_WSTAG820_800A724C, &D_WSTAG820_800A7258, &D_WSTAG820_800A7264, &D_WSTAG820_800A7270, &D_WSTAG820_800A727C,
        &D_WSTAG820_800A7288, &D_WSTAG820_800A7294, &D_WSTAG820_800A72A0 },
};
FieldstgListedBattle D_WSTAG820_800A72D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A72DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A72E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A72F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A730C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7324 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG820_800A7330 = {
    0,
    { &D_WSTAG820_800A72D0, &D_WSTAG820_800A72DC, &D_WSTAG820_800A72E8, &D_WSTAG820_800A72F4, &D_WSTAG820_800A7300,
        &D_WSTAG820_800A730C, &D_WSTAG820_800A7318, &D_WSTAG820_800A7324 },
};
FieldstgListedBattle D_WSTAG820_800A7354 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7360 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A736C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7390 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A739C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A73A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG820_800A73B4 = {
    0,
    { &D_WSTAG820_800A7354, &D_WSTAG820_800A7360, &D_WSTAG820_800A736C, &D_WSTAG820_800A7378, &D_WSTAG820_800A7384,
        &D_WSTAG820_800A7390, &D_WSTAG820_800A739C, &D_WSTAG820_800A73A8 },
};
FieldstgListedBattle D_WSTAG820_800A73D8 = { 33, 17, 0x608C0000 };
FieldstgListedBattle D_WSTAG820_800A73E4 = { 324, 21, 0x60900000 };
FieldstgListedBattle D_WSTAG820_800A73F0 = { 325, 22, 0x60980000 };
FieldstgListedBattle D_WSTAG820_800A73FC = { 326, 22, 0x60980000 };
FieldstgListedBattle D_WSTAG820_800A7408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A7420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG820_800A742C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG820_800A7438 = {
    0,
    { &D_WSTAG820_800A73D8, &D_WSTAG820_800A73E4, &D_WSTAG820_800A73F0, &D_WSTAG820_800A73FC, &D_WSTAG820_800A7408,
        &D_WSTAG820_800A7414, &D_WSTAG820_800A7420, &D_WSTAG820_800A742C },
};
FieldstgBattleLists wstag820_battle_lists = {
    141, 0, 0, { &D_WSTAG820_800A72AC, &D_WSTAG820_800A7330, &D_WSTAG820_800A73B4 }, &D_WSTAG820_800A7438,
};
FieldstgVramPlace wstag820_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 376, 40, 120, 352, 508 }, { 320, 256, 338, 256, 72, 0, 368, 508 },
    { 320, 256, 320, 256, 0, 0, 320, 507 }, { 320, 256, 330, 344, 40, 88, 336, 507 },
    { 320, 256, 346, 448, 104, 192, 352, 507 }, { 320, 256, 368, 408, 192, 152, 368, 507 },
    { 320, 256, 362, 448, 168, 192, 320, 506 }, { 320, 256, 354, 256, 136, 0, 336, 505 },
};
u16 D_WSTAG820_800A7558[6] = { 0x6029, 1, 0x4074, 0, 0xFFFF, 0 };
u16 D_WSTAG820_800A7564[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A756C[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A7574[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A757C[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A7584[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A758C[4] = { 0x6029, 1, 0xFFFF, 0 };
u16 D_WSTAG820_800A7594[6] = { 0x4075, 0, 0x6029, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG820_800A75A0 = { D_WSTAG820_800A7558, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG820_800A75B4 = { D_WSTAG820_800A7564, NULL, 213, 5, 336, 232, 3 };
FieldstgPlacedActor D_WSTAG820_800A75C8 = { D_WSTAG820_800A756C, NULL, 214, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG820_800A75DC = { D_WSTAG820_800A7574, NULL, 215, 7, 249, 177, 7 };
FieldstgPlacedActor D_WSTAG820_800A75F0 = { D_WSTAG820_800A757C, NULL, 216, 8, 281, 118, 7 };
FieldstgPlacedActor D_WSTAG820_800A7604 = { D_WSTAG820_800A7584, NULL, 217, 9, 120, 197, 7 };
FieldstgPlacedActor D_WSTAG820_800A7618 = { D_WSTAG820_800A758C, NULL, 218, 10, 121, 117, 7 };
FieldstgPlacedActor D_WSTAG820_800A762C = { D_WSTAG820_800A7594, NULL, 392, 11, 578, 401, 7 };
FieldstgPlacedActor *wstag820_actors[9] = {
    &D_WSTAG820_800A75A0, &D_WSTAG820_800A75B4, &D_WSTAG820_800A75C8, &D_WSTAG820_800A75DC, &D_WSTAG820_800A75F0,
    &D_WSTAG820_800A7604, &D_WSTAG820_800A7618, &D_WSTAG820_800A762C, NULL,
};
FieldstgSprite wstag820_sprites[2] = {
    { 0, 1, 0x40, 2, 0x3D, 0, 0, 0, 0, 0, 268, 164, 208, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag820_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DE, 0xA0, 0x7C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2DF, 0x180, 0xFE, 3, 0, 0, 0 },
    { 0x40A5, 0, 0xFFFF, 0, 8, 0x438, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag820_funcs = { wstag820_setup };
FieldstgEventDef wstag820_events[4] = {
    { 1060, D_WSTAG820_800A69AC, 0x0151000F, NULL, wstag820_event_1060_end },
    { 1070, D_WSTAG820_800A6B60, 0x01510010, NULL, wstag820_event_1070_end },
    { 1080, D_WSTAG820_800A6C54, 0x01510011, NULL, wstag820_event_1080_end }, { -1, NULL, 0, NULL, NULL },
};
