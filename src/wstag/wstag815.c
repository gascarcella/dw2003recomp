#include "wstag.h"

/* WSTAG815: stage 0x2DE (fieldstg_stages). */

extern WstagSpawn D_WSTAG815_800A6D14[];
extern WstagAnimKey D_WSTAG815_800A6D50[];
extern WstagAnimKey D_WSTAG815_800A6D68[];
extern WstagFuncs wstag815_funcs;
extern FieldstgBattleLists wstag815_battle_lists;
extern FieldstgVramPlace wstag815_vram_places[];
extern FieldstgPlacedActor *wstag815_actors[];
extern FieldstgSprite wstag815_sprites[];
extern FieldstgMapEvent wstag815_map_events[];
extern FieldstgEventDef wstag815_events[];
void wstag815_backdrop_update(Object *obj);
Object *wstag815_backdrop_new(void);
WstagTwoSpriteObject *wstag815_two_sprite_create(s32 x, s32 y, s32 frame);
void wstag815_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

void wstag815_backdrop_draw(Object *obj) {
    Sprite spr;
    GamestatePos pos;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);

    layer->get_scroll(layer, &pos.x);
    pos.x = (pos.x << 8) / 384;
    pos.y = (pos.y << 8) / 384;
    sprite_init(&spr);
    spr.set_layer_id(0x1002, 7);
    spr.set_vram_pos(0x280, 0);
    spr.set_clut8_pos(0, 0xF0);
    spr.draw(cdload_module.get_subfile_by_id(0x07760002), 0, pos.x, pos.y);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.draw(cdload_module.get_subfile_by_id(0x07760003), 0, pos.x, pos.y + 0x100);
}

void wstag815_backdrop_update(Object *obj) {
    Tim img;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        tim_init(&img);
        img.set_image_pos(0x280, 0);
        img.set_clut_pos(0, 0xF0);
        img.load_all((s32 *)cdload_module.files.get_file(0x775));
        img.set_image_pos(0x280, 0x100);
        img.set_clut_pos(0, 0x1F0);
        img.load_all((s32 *)cdload_module.files.get_file(0x898));
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        wstag815_backdrop_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}


Object *wstag815_backdrop_new(void) {
    return object_new(wstag815_backdrop_update, 0x60, 0);
}

void wstag815_update(WstagObject *obj, WstagObjSpawnData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag815_backdrop_new();
        for (i = 0; i < 5; i++) {
            if (D_WSTAG815_800A6D14[i].condition == 0) {
                data->objs[i] = wstag815_two_sprite_create(D_WSTAG815_800A6D14[i].x, D_WSTAG815_800A6D14[i].y,
                                                       D_WSTAG815_800A6D14[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x4072, 1) && gamestate_flags.get_flag(0x40A9, 0)) {
            data->event = fieldstg_event_start(0x411);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag815_start(void *arg0) {
    WstagObject *obj = object_create(wstag815_update, sizeof(WstagObject), sizeof(WstagObjSpawnData), 0x17);

    obj->manager = arg0;
    wstag815_funcs.setup();
    return obj;
}


s32 wstag815_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag815_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag815_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag815_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag815_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG815_800A6D68[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG815_800A6D50[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag815_anim_advance(&obj->palette_anim, D_WSTAG815_800A6D50, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag815_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag815_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG815_800A6D68[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG815_800A6D50[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag815_anim_advance(&obj->palette_anim, D_WSTAG815_800A6D50, 0, 0);
        done = 0;
        frame = wstag815_anim_advance(&obj->frame_anim, D_WSTAG815_800A6D68, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag815_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag815_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag815_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag815_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag815_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag815_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag815_event_1020_end(void) {
    gamestate_flags.set_flag(0x4070, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag815_event_1030_end(void) {
    gamestate_flags.set_flag(0x4071, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag815_event_1040_end(void) {
    gamestate_flags.set_flag(0x4072, 1);
}

void wstag815_event_1041_end(void) {
    gamestate_flags.set_flag(0x40A9, 1);
}

void wstag815_event_1050_end(void) {
    gamestate_flags.set_flag(0x4073, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag815_setup(void) {
    fieldstg_stage.sprite_file = 0x07760000;
    fieldstg_stage.sprites = wstag815_sprites;
    fieldstg_stage.map_events = wstag815_map_events;
    fieldstg_stage.mask_file = 0x774;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x3D400, 0x33B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag815_vram_places;
    fieldstg_stage.music = 0x1C;
    fieldstg_stage.sound = 0x60700000;
    fieldstg_stage.actors = wstag815_actors;
    fieldstg_stage.battle_lists = &wstag815_battle_lists;
    fieldstg_stage.events = wstag815_events;
    fieldstg_attr.set_file(0, 0x07760001);
    fieldstg_attr.set_file(7, 0x07760004);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag815_setup(void);

s16 D_WSTAG815_800A69D4[61] = {
    FIELDSTG_EVENT_WALK(2, 352, 715, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 208),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 208),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 208, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 336, 708, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG815_800A6A50[61] = {
    FIELDSTG_EVENT_WALK(2, 976, 259, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 209),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 209, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 960, 251, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG815_800A6ACC[149] = {
    FIELDSTG_EVENT_WALK(2, 637, 505, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
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
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_GOTO_MAP(0xE09, 637, 505, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG815_800A6BF8[33] = {
    FIELDSTG_EVENT_PLACE(2, 637, 505),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG815_800A6C3C[108] = {
    FIELDSTG_EVENT_WALK(2, 381, 281, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
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
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
WstagSpawn D_WSTAG815_800A6D14[5] = {
    { 60, 0, 792, 166 }, { 60, 0, 1016, 278 }, { 60, 0, 168, 622 }, { 60, 0, 392, 734 }, { 44, 0, 1016, 710 },
};
WstagAnimKey D_WSTAG815_800A6D50[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG815_800A6D68[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG815_800A6DC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6DCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6DD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6DE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6DF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6DFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E14 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG815_800A6E20 = {
    0,
    { &D_WSTAG815_800A6DC0, &D_WSTAG815_800A6DCC, &D_WSTAG815_800A6DD8, &D_WSTAG815_800A6DE4, &D_WSTAG815_800A6DF0,
        &D_WSTAG815_800A6DFC, &D_WSTAG815_800A6E08, &D_WSTAG815_800A6E14 },
};
FieldstgListedBattle D_WSTAG815_800A6E44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6E98 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG815_800A6EA4 = {
    0,
    { &D_WSTAG815_800A6E44, &D_WSTAG815_800A6E50, &D_WSTAG815_800A6E5C, &D_WSTAG815_800A6E68, &D_WSTAG815_800A6E74,
        &D_WSTAG815_800A6E80, &D_WSTAG815_800A6E8C, &D_WSTAG815_800A6E98 },
};
FieldstgListedBattle D_WSTAG815_800A6EC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6ED4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6EE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6EEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6EF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F10 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F1C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG815_800A6F28 = {
    0,
    { &D_WSTAG815_800A6EC8, &D_WSTAG815_800A6ED4, &D_WSTAG815_800A6EE0, &D_WSTAG815_800A6EEC, &D_WSTAG815_800A6EF8,
        &D_WSTAG815_800A6F04, &D_WSTAG815_800A6F10, &D_WSTAG815_800A6F1C },
};
FieldstgListedBattle D_WSTAG815_800A6F4C = { 194, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG815_800A6F58 = { 144, 17, 0x60080000 };
FieldstgListedBattle D_WSTAG815_800A6F64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6F94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG815_800A6FA0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG815_800A6FAC = {
    0,
    { &D_WSTAG815_800A6F4C, &D_WSTAG815_800A6F58, &D_WSTAG815_800A6F64, &D_WSTAG815_800A6F70, &D_WSTAG815_800A6F7C,
        &D_WSTAG815_800A6F88, &D_WSTAG815_800A6F94, &D_WSTAG815_800A6FA0 },
};
FieldstgBattleLists wstag815_battle_lists = {
    125, 0, 0, { &D_WSTAG815_800A6E20, &D_WSTAG815_800A6EA4, &D_WSTAG815_800A6F28 }, &D_WSTAG815_800A6FAC,
};
FieldstgVramPlace wstag815_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 461, 386, 564, 130, 352, 508 }, { 448, 256, 461, 346, 564, 90, 368, 508 },
};
u16 D_WSTAG815_800A706C[6] = { 0x4070, 0, 0x6028, 1, 0xFFFF, 0 };
u16 D_WSTAG815_800A7078[6] = { 0x6028, 1, 0x4071, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG815_800A7084 = { D_WSTAG815_800A706C, NULL, 208, 4, 302, 691, 7 };
FieldstgPlacedActor D_WSTAG815_800A7098 = { D_WSTAG815_800A7078, NULL, 209, 5, 927, 235, 7 };
FieldstgPlacedActor *wstag815_actors[3] = { &D_WSTAG815_800A7084, &D_WSTAG815_800A7098, NULL };
FieldstgSprite wstag815_sprites[10] = {
    { 1, 0, 0xFF, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 1, 0, 0xF9, 6, 3, 0, 0, 0, 0, 0, 853, 643, 643, 0 },
    { 1, 0, 0xFF, 6, 4, 0, 0, 0, 0, 0, 124, 99, 0, 0 }, { 1, 0, 0xD6, 6, 1, 0, 0, 0, 0, 0, 341, 195, 0, 0 },
    { 1, 0, 0xD6, 6, 1, 0, 0, 0, 0, 0, 469, 307, 0, 0 }, { 1, 0, 0xD6, 6, 1, 0, 0, 0, 0, 0, 597, 419, 0, 0 },
    { 1, 0, 0xD6, 6, 1, 0, 0, 0, 0, 0, 725, 531, 0, 0 }, { 1, 0, 0xFF, 6, 2, 0, 0, 0, 0, 0, 156, 617, 617, 0 },
    { 1, 0, 0xFF, 6, 2, 0, 0, 0, 0, 0, 780, 161, 161, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag815_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DF, 0x310, 0x228, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DD, 0x504, 0x296, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DD, 0x344, 0xF6, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DD, 0x29C, 0x1AA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xE, 0x2DD, 0x3E4, 0x146, 5, 0, 0, 0 },
    { 0x4070, 0, 0xFFFF, 0, 8, 0x3FC, 0, 0, 0, 0, 0, 0 }, { 0x4071, 0, 0xFFFF, 0, 8, 0x406, 0, 0, 0, 0, 0, 0 },
    { 0x4072, 0, 0xFFFF, 0, 8, 0x410, 0, 0, 0, 0, 0, 0 }, { 0x4073, 0, 0xFFFF, 0, 8, 0x41A, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag815_funcs = { wstag815_setup };
FieldstgEventDef wstag815_events[6] = {
    { 1020, D_WSTAG815_800A69D4, 0x01510009, NULL, wstag815_event_1020_end },
    { 1030, D_WSTAG815_800A6A50, 0x0151000A, NULL, wstag815_event_1030_end },
    { 1040, D_WSTAG815_800A6ACC, 0x0151000C, NULL, wstag815_event_1040_end },
    { 1041, D_WSTAG815_800A6BF8, 0x0151000D, NULL, wstag815_event_1041_end },
    { 1050, D_WSTAG815_800A6C3C, 0x0151000E, NULL, wstag815_event_1050_end }, { -1, NULL, 0, NULL, NULL },
};
