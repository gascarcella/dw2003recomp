#include "wstag.h"

/* WSTAG750: stage 0x26D (fieldstg_stages). */

/* The data of the stage object: the placed WstagTwoSpriteObject, one more object and an event. */
typedef struct Wstag750Data {
    /* 0x0 */ WstagTwoSpriteObject *obj;
    /* 0x4 */ void *sprite_anim;
    /* 0x8 */ FieldstgEvent *event;
} Wstag750Data; /* size 0xC */

extern WstagAnimKey *D_WSTAG750_800A7244[3];
extern s8 D_WSTAG750_800A7250[];
extern s8 D_WSTAG750_800A7254[];
extern WstagPos D_WSTAG750_800A7258;
extern WstagSpawn D_WSTAG750_800A725C;
extern WstagAnimKey D_WSTAG750_800A7268[];
extern WstagAnimKey D_WSTAG750_800A7280[];
extern WstagFuncs wstag750_funcs;
extern FieldstgVramPlace wstag750_vram_places[];
extern FieldstgPlacedActor *wstag750_actors[];
extern FieldstgSprite wstag750_sprites[];
extern FieldstgMapEvent wstag750_map_events[];
extern FieldstgEventDef wstag750_events[];
void wstag750_sprite_anim_update(WstagSpriteAnimObject *obj);
void wstag750_update(WstagObject *obj, Wstag750Data *data);
WstagTwoSpriteObject *wstag750_two_sprite_create(s32 x, s32 y, s32 frame);
void wstag750_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

s32 wstag750_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        wstag750_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag750_sprite_anim_reset(WstagSpriteAnimObject *obj) {
    s32 i;

    for (i = 0; i < 3; i++) {
        obj->sprites[i].anim.key = 0;
        obj->sprites[i].anim.time = D_WSTAG750_800A7244[i]->time;
    }
}

void wstag750_sprite_anim_update(WstagSpriteAnimObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 n;
    s32 i;
    s32 j;
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag750_sprite_anim_reset(obj);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
                list = fieldstg_stage.sprites;
                for (n = 0; list->present != 0; list++) {
                    if (list->type >= 1 && list->type <= 3) {
                        obj->sprites[n].sprite = list;
                        list->x = obj->x;
                        list->y = obj->y + D_WSTAG750_800A7254[n];
                        list->priority = obj->y + D_WSTAG750_800A7250[n];
                        n++;
                    }
                }
                sound_module.play(0x01000000);
                obj->base.next_substep(obj);
            case 1:
                done = 0;
                for (i = 0; i < 3; i++) {
                    sprite = obj->sprites[i].sprite;
                    frame = wstag750_sprite_anim_play_once(&obj->sprites[i], D_WSTAG750_800A7244[i], 0);
                    switch (frame) {
                    case 0xFF:
                        done++;
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    case 0x12C:
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    default:
                        sprite->shown = 1;
                        sprite->sprite = frame;
                        break;
                    }
                }
                if (done < 3) {
                    break;
                }
                obj->base.next_substep(obj);
            case 2:
                for (j = 0; j < 3; j++) {
                    obj->sprites[j].sprite->shown = 0;
                }
                wstag750_sprite_anim_reset(obj);
                obj->base.set_step(obj, 0);
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag750_sprite_anim_message(WstagSpriteAnimObject *obj, s32 msg) {
    if (obj != NULL && msg == 0x335) {
        obj->x = D_WSTAG750_800A7258.x;
        obj->y = D_WSTAG750_800A7258.y;
        obj->base.set_step(obj, 1);
    }
}

WstagSpriteAnimObject *wstag750_sprite_anim_create(s32 arg0) {
    return object_create(wstag750_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0, arg0);
}

void wstag750_update(WstagObject *obj, Wstag750Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (D_WSTAG750_800A725C.condition == 0) {
            data->obj = wstag750_two_sprite_create(D_WSTAG750_800A725C.x, D_WSTAG750_800A725C.y, D_WSTAG750_800A725C.frame);
        }
        if (gamestate_data.progress == 0x20 && gamestate_flags.get_flag(0x4048, 0) && gamestate_flags.get_flag(0x4046, 0)
            && gamestate_flags.get_flag(0x4066, 0)) {
            data->event = fieldstg_event_start(0x35C);
        } else if (gamestate_data.progress == 0x20 && gamestate_flags.get_flag(0x4046, 1) && gamestate_flags.get_flag(0x4048, 0)
                   && gamestate_flags.get_flag(0x4047, 0) && gamestate_flags.get_flag(0x4066, 0)) {
            data->event = fieldstg_event_start(0x373);
        } else if (gamestate_data.progress == 0x20 && gamestate_flags.get_flag(0x4048, 1) && gamestate_flags.get_flag(0x4046, 0)
                   && gamestate_flags.get_flag(0x4062, 0) && gamestate_flags.get_flag(0x4066, 0)) {
            data->event = fieldstg_event_start(0x374);
        } else if (gamestate_data.progress == 0x20 && gamestate_flags.get_flag(0x4066, 1)) {
            data->event = fieldstg_event_start(0x376);
        }
        data->sprite_anim = wstag750_sprite_anim_create(0x34E);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag750_start(void *arg0) {
    WstagObject *obj = object_new(wstag750_update, sizeof(WstagObject), sizeof(Wstag750Data));

    obj->manager = arg0;
    wstag750_funcs.setup();
    return obj;
}

s32 wstag750_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag750_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag750_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag750_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag750_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG750_800A7280[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG750_800A7268[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag750_anim_advance(&obj->palette_anim, D_WSTAG750_800A7268, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag750_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag750_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG750_800A7280[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG750_800A7268[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag750_anim_advance(&obj->palette_anim, D_WSTAG750_800A7268, 0, 0);
        done = 0;
        frame = wstag750_anim_advance(&obj->frame_anim, D_WSTAG750_800A7280, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag750_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag750_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag750_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, wstag750_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag750_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag750_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag750_event_820_end(void) {
    gamestate_data.progress = 0x20;
}

void wstag750_event_860_end(void) {
    gamestate_flags.set_flag(0x4048, 1);
}

void wstag750_event_883_end(void) {
    gamestate_flags.set_flag(0x4047, 1);
}

void wstag750_event_884_end(void) {
    gamestate_flags.set_flag(0x4062, 1);
}

void wstag750_event_886_end(void) {
    gamestate_data.progress = 0x21;
}

void wstag750_event_1245_end(void) {
    gamestate_flags.set_flag(0x7C0A, 1);
}

void wstag750_setup(void) {
    fieldstg_stage.background_file = 0x6C0;
    fieldstg_stage.sprite_file = 0x06C10000;
    fieldstg_stage.sprites = wstag750_sprites;
    fieldstg_stage.map_events = wstag750_map_events;
    fieldstg_stage.mask_file = 0x6BF;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12A00, 0x1D200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag750_vram_places;
    fieldstg_stage.music = 0x40;
    fieldstg_stage.actors = wstag750_actors;
    fieldstg_stage.sound = 0x61000001;
    fieldstg_stage.events = wstag750_events;
    fieldstg_attr.set_file(0, 0x06C10001);
    fieldstg_attr.set_file(7, 0x06C10002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag750_setup(void);

s16 D_WSTAG750_800A6C04[248] = {
    FIELDSTG_EVENT_WALK(2, 160, 296, 3),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(13, 9, 7),
    FIELDSTG_EVENT_WAIT_ANIM(13),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(13, 111, 272, 3),
    FIELDSTG_EVENT_WAIT_WALK(13),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(13, 2, 3),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(13, 129, 281, 7),
    FIELDSTG_EVENT_WAIT_WALK(13),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 352, 200, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_MOVE(1, 352, 200),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x34E, 821, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x2DA, 1119, 893, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG750_800A6DF4[153] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 240, 224),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_PLACE(13, 129, 281),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_MOVE(0, 208, 240),
    FIELDSTG_EVENT_ANIM(0x323, 805, 13),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 13),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(13, 111, 272, 3),
    FIELDSTG_EVENT_WAIT_WALK(13),
    FIELDSTG_EVENT_ANIM(13, 2, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 240, 224),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x34E, 821, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_PLACE(2, 350, 202),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 208, 240),
    FIELDSTG_EVENT_DIALOG(0, 2, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(13, 129, 281, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_GOTO_MAP(0x2DC, 1, 1, 0),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG750_800A6F28[151] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 240, 224),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_PLACE(13, 129, 281),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_MOVE(0, 208, 240),
    FIELDSTG_EVENT_ANIM(0x323, 805, 13),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 13),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(13, 111, 272, 3),
    FIELDSTG_EVENT_WAIT_WALK(13),
    FIELDSTG_EVENT_ANIM(13, 2, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 240, 224),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x34E, 821, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_PLACE(2, 350, 202),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 208, 240),
    FIELDSTG_EVENT_DIALOG(0, 1, 13, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(13, 129, 281, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x2D7, 0, 0, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG750_800A7058[54] = {
    FIELDSTG_EVENT_PLACE(2, 350, 202),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(13, 129, 281),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x2D7, 100, 100, 0),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG750_800A70C4[51] = {
    FIELDSTG_EVENT_PLACE(2, 350, 202),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_PLACE(13, 129, 281),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 160, 296, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x26A, 960, 80, 7),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG750_800A712C[57] = {
    FIELDSTG_EVENT_WALK(2, 256, 456, 3),
    FIELDSTG_EVENT_PLACE(21, 224, 441),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC08, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG750_800A71A0[5] = { { 70, 4 }, { 71, 4 }, { 70, 4 }, { 71, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG750_800A71B4[20] = {
    { 300, 56 }, { 85, 6 }, { 86, 6 }, { 87, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 },
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG750_800A7204[16] = {
    { 300, 16 }, { 72, 4 }, { 73, 4 }, { 74, 4 }, { 73, 4 }, { 75, 6 }, { 76, 6 }, { 77, 14 }, { 78, 114 },
    { 79, 6 }, { 80, 6 }, { 81, 6 }, { 82, 6 }, { 83, 6 }, { 84, 8 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG750_800A7244[3] = { D_WSTAG750_800A71A0, D_WSTAG750_800A7204, D_WSTAG750_800A71B4 };
s8 D_WSTAG750_800A7250[4] = { 30, 30, 30, 0 };
s8 D_WSTAG750_800A7254[4] = { 0, -47, -47, 0 };
WstagPos D_WSTAG750_800A7258 = { 320, 186 };
WstagSpawn D_WSTAG750_800A725C = { 28, 0, 108, 324 };
WstagAnimKey D_WSTAG750_800A7268[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG750_800A7280[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgVramPlace wstag750_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 432, 216, 176, 352, 504 }, { 320, 256, 356, 456, 144, 200, 368, 504 },
    { 320, 256, 330, 432, 40, 176, 336, 503 }, { 384, 256, 404, 320, 336, 64, 352, 503 },
    { 384, 256, 412, 328, 368, 72, 368, 503 },
};
u16 D_WSTAG750_800A7388[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG750_800A7390[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7398[6] = { 0x800C, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A73A4[8] = { 0x800C, 1, 0x1C02, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A73B4[6] = { 0x9045, 1, 0x1C02, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A73C0[8] = { 0x800C, 1, 0x1C02, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A73D0[4] = { 0x7A27, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A73D8[4] = { 0x9019, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG750_800A73E0[5] = {
    { D_WSTAG750_800A7388, D_WSTAG750_800A7390, 779 }, { D_WSTAG750_800A7398, NULL, 780 },
    { D_WSTAG750_800A73A4, D_WSTAG750_800A73B4, 781 }, { D_WSTAG750_800A73C0, NULL, 782 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG750_800A741C[2] = { { NULL, D_WSTAG750_800A73D0, 363 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A7434[2] = { { NULL, D_WSTAG750_800A73D8, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A744C[2] = { { NULL, NULL, 530 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A7464[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A747C[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A7494[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A74AC[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A74C4[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A74DC[2] = { { NULL, NULL, 528 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A74F4[2] = { { NULL, NULL, 529 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A750C[2] = { { NULL, NULL, 526 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A7524[2] = { { NULL, NULL, 526 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG750_800A753C[2] = { { NULL, NULL, 786 }, { NULL, NULL, 0 } };
u16 D_WSTAG750_800A7554[6] = { 0x700A, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG750_800A7560[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7568[4] = { 0x6021, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7570[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7578[4] = { 0x6023, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7580[4] = { 0x6024, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7588[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7590[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A7598[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A75A0[4] = { 0x601F, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A75A8[4] = { 0x601E, 1, 0xFFFF, 0 };
u16 D_WSTAG750_800A75B0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG750_800A75B8 = { D_WSTAG750_800A7554, D_WSTAG750_800A73E0, 13, 4, 129, 281, 7 };
FieldstgPlacedActor D_WSTAG750_800A75CC = { NULL, D_WSTAG750_800A741C, 20, 5, 162, 473, 7 };
FieldstgPlacedActor D_WSTAG750_800A75E0 = { NULL, D_WSTAG750_800A7434, 21, 6, 224, 441, 7 };
FieldstgPlacedActor D_WSTAG750_800A75F4 = { D_WSTAG750_800A7560, D_WSTAG750_800A744C, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7608 = { D_WSTAG750_800A7568, D_WSTAG750_800A7464, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A761C = { D_WSTAG750_800A7570, D_WSTAG750_800A747C, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7630 = { D_WSTAG750_800A7578, D_WSTAG750_800A7494, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7644 = { D_WSTAG750_800A7580, D_WSTAG750_800A74AC, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7658 = { D_WSTAG750_800A7588, D_WSTAG750_800A74C4, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A766C = { D_WSTAG750_800A7590, D_WSTAG750_800A74DC, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7680 = { D_WSTAG750_800A7598, D_WSTAG750_800A74F4, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A7694 = { D_WSTAG750_800A75A0, D_WSTAG750_800A750C, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A76A8 = { D_WSTAG750_800A75A8, D_WSTAG750_800A7524, 35, 7, 306, 456, 7 };
FieldstgPlacedActor D_WSTAG750_800A76BC = { D_WSTAG750_800A75B0, D_WSTAG750_800A753C, 157, 8, 129, 281, 7 };
FieldstgPlacedActor *wstag750_actors[15] = {
    &D_WSTAG750_800A75B8, &D_WSTAG750_800A75CC, &D_WSTAG750_800A75E0, &D_WSTAG750_800A75F4, &D_WSTAG750_800A7608,
    &D_WSTAG750_800A761C, &D_WSTAG750_800A7630, &D_WSTAG750_800A7644, &D_WSTAG750_800A7658, &D_WSTAG750_800A766C,
    &D_WSTAG750_800A7680, &D_WSTAG750_800A7694, &D_WSTAG750_800A76A8, &D_WSTAG750_800A76BC, NULL,
};
FieldstgSprite wstag750_sprites[14] = {
    { 0, 1, 0x50, 6, 0x46, 0, 0, 0, 0, 0, 320, 186, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 1, 0x60, 0x63, 6, 0, 324, 169, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 0xB, 8, 0, 78, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 0xB, 8, 0, 93, 214, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 0xB, 8, 0, 109, 217, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 0xB, 8, 0, 78, 221, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 0xB, 8, 0, 93, 225, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 0xB, 8, 0, 109, 205, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 82, 234, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 139, 230, 0, 0 },
    { 0, 3, 0x50, 4, 0x55, 0, 0, 0, 0, 0, 320, 139, 206, 0 },
    { 0, 2, 0x50, 4, 0x48, 0, 0, 0, 0, 0, 320, 139, 213, 0 },
    { 1, 0, 0x40, 4, 0x5C, 1, 0x5C, 0x5F, 6, 0, 324, 185, 209, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag750_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26C, 0xB8, 0xB4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x26A, 0x3C0, 0x50, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag750_funcs = { wstag750_setup };
FieldstgEventDef wstag750_events[7] = {
    { 820, D_WSTAG750_800A6C04, 0x014A0019, NULL, wstag750_event_820_end },
    { 860, D_WSTAG750_800A6DF4, 0x014A001C, NULL, wstag750_event_860_end },
    { 883, D_WSTAG750_800A6F28, 0x014A001D, NULL, wstag750_event_883_end },
    { 884, D_WSTAG750_800A7058, 0x014A001E, NULL, wstag750_event_884_end },
    { 886, D_WSTAG750_800A70C4, 0x014A001F, NULL, wstag750_event_886_end },
    { 1245, D_WSTAG750_800A712C, 0x014A0008, NULL, wstag750_event_1245_end }, { -1, NULL, 0, NULL, NULL },
};
