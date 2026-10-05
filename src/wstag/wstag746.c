#include "wstag.h"

/* WSTAG746: stage 0x2D4 (fieldstg_stages). */

extern WstagSpawn D_WSTAG746_800A6BB0[];
extern WstagAnimKey D_WSTAG746_800A6CA0[];
extern WstagAnimKey D_WSTAG746_800A6CB8[];
extern WstagFuncs wstag746_funcs;
extern FieldstgBattleLists wstag746_battle_lists;
extern FieldstgVramPlace wstag746_vram_places[];
extern FieldstgPlacedActor *wstag746_actors[];
extern FieldstgSprite wstag746_sprites[];
extern FieldstgMapEvent wstag746_map_events[];
extern FieldstgEventDef wstag746_events[];
void wstag746_update();
void wstag746_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);
WstagTwoSpriteObject *wstag746_two_sprite_create(s32 x, s32 y, s32 frame);

void wstag746_flash_update(Object *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    if (obj->state == OBJECT_STATE_INIT) {
        if (gamestate_data.screen_white != 0) {
            obj->set_state(obj, OBJECT_STATE_DONE);
        } else {
            obj->set_state(obj, OBJECT_STATE_RUN);
        }
    }
    switch (obj->state) {
    case OBJECT_STATE_INIT:
        break;
    case OBJECT_STATE_RUN:
        if (obj->key1 != 0) {
            if (obj->step != 5) {
                obj->next_step(obj);
            } else {
                obj->key1 = 0;
                obj->set_state(obj, OBJECT_STATE_DONE);
                sound_module.play(0x800410BD);
            }
        }
        gamestate_data.screen_white = 0;
        break;
    case OBJECT_STATE_DONE:
        layer = gfx_module.funcs.get_layer(0x1002);
        ot = layer->get_ot_entry(layer, 6);
        poly = gfx_module.funcs.get_packet();
        setPolyF4(poly);
        setSemiTrans(poly, 1);
        poly->r0 = poly->g0 = poly->b0 = 0xFF;
        setXYWH(poly, 0, 0, 320, 256);
        addPrim(ot, poly);
        tpage = (DR_TPAGE *)(poly + 1);
        setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
        addPrim(ot, tpage);
        gfx_module.funcs.set_packet(tpage + 1);
        gamestate_data.screen_white = 1;
        break;
    }
}

Object *wstag746_flash_new(void) {
    return object_create(wstag746_flash_update, sizeof(WstagObject), 0, 0x19);
}

s32 wstag746_event_8000_start(void) {
    Object *flash = heap_objects.find(0x19, -1, -1);

    if (flash != NULL) {
        if (flash->state != OBJECT_STATE_RUN) {
            sound_module.play(0x800410BD);
        }
        flash->set_state(flash, OBJECT_STATE_RUN);
        flash->key1 = 0;
    }
    return 0;
}

s32 wstag746_event_8001_start(void) {
    Object *flash = heap_objects.find(0x19, -1, -1);

    if (flash != NULL) {
        if (flash->state != OBJECT_STATE_DONE) {
            sound_module.play(0x800410BD);
        }
        flash->set_state(flash, OBJECT_STATE_DONE);
        flash->key1 = 0;
    }
    return 0;
}

s32 wstag746_event_8002_start(void) {
    Object *flash = heap_objects.find(0x19, -1, -1);

    if (flash != NULL) {
        if (flash->state != OBJECT_STATE_RUN) {
            sound_module.play(0x800410BD);
        }
        flash->set_state(flash, OBJECT_STATE_RUN);
        flash->key1 = 1;
    }
    return 0;
}

void wstag746_update(WstagObject *obj, WstagObjSpawn20Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag746_flash_new();
        for (i = 0; i < 20; i++) {
            if (D_WSTAG746_800A6BB0[i].condition == 0) {
                data->objs[i] = wstag746_two_sprite_create(D_WSTAG746_800A6BB0[i].x, D_WSTAG746_800A6BB0[i].y,
                                                       D_WSTAG746_800A6BB0[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x4086, 1) && gamestate_flags.get_flag(0x4087, 0)) {
            data->event = fieldstg_event_start(0x512);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag746_start(void *arg0) {
    WstagObject *obj = object_new(wstag746_update, sizeof(WstagObject), sizeof(WstagObjSpawn20Data));

    obj->manager = arg0;
    wstag746_funcs.setup();
    return obj;
}

s32 wstag746_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag746_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag746_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag746_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag746_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG746_800A6CB8[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG746_800A6CA0[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag746_anim_advance(&obj->palette_anim, D_WSTAG746_800A6CA0, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag746_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag746_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG746_800A6CB8[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG746_800A6CA0[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag746_anim_advance(&obj->palette_anim, D_WSTAG746_800A6CA0, 0, 0);
        done = 0;
        frame = wstag746_anim_advance(&obj->frame_anim, D_WSTAG746_800A6CB8, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag746_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag746_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag746_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, wstag746_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag746_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag746_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag746_event_1297_end(void) {
    gamestate_flags.set_flag(0x4086, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag746_event_1298_end(void) {
    gamestate_flags.set_flag(0x4087, 1);
    gamestate_flags.set_flag(0x8F41, 1);
}

void wstag746_setup(void) {
    fieldstg_stage.background_file = 0x6CE;
    fieldstg_stage.sprite_file = 0x06CF0000;
    fieldstg_stage.sprites = wstag746_sprites;
    fieldstg_stage.map_events = wstag746_map_events;
    fieldstg_stage.mask_file = 0x6CD;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x40000, 0x2C800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag746_vram_places;
    fieldstg_stage.music = 0x19;
    fieldstg_stage.sound = 0x60640000;
    fieldstg_stage.actors = wstag746_actors;
    fieldstg_stage.battle_lists = &wstag746_battle_lists;
    fieldstg_stage.events = wstag746_events;
    fieldstg_attr.set_file(0, 0x06CF0001);
    fieldstg_attr.set_file(7, 0x06CF0002);
    fieldstg_attr.set_file(4, 0x06CF0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.map_is_new != 0) {
        gamestate_data.screen_white = 0;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag746_setup(void);

s16 D_WSTAG746_800A6A98[75] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 259, 218, 3),
    FIELDSTG_EVENT_PLACE(205, 240, 209),
    FIELDSTG_EVENT_ANIM(205, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 205, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 205, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG746_800A6B30[64] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 205),
    FIELDSTG_EVENT_PLACE(2, 259, 218),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(205, 240, 209),
    FIELDSTG_EVENT_ANIM(205, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 205, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 205, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x2, /* padding, not read */
};
WstagSpawn D_WSTAG746_800A6BB0[20] = {
    { 68, 0, 283, 340 }, { 68, 0, 316, 452 }, { 68, 0, 348, 500 }, { 68, 0, 348, 564 }, { 68, 0, 444, 260 },
    { 68, 0, 444, 388 }, { 68, 0, 476, 436 }, { 68, 0, 476, 692 }, { 68, 0, 508, 548 }, { 68, 0, 508, 612 },
    { 68, 0, 572, 324 }, { 68, 0, 604, 436 }, { 68, 0, 636, 548 }, { 68, 0, 668, 660 }, { 68, 0, 700, 708 },
    { 68, 0, 732, 564 }, { 68, 0, 764, 420 }, { 68, 0, 796, 468 }, { 68, 0, 796, 532 }, { 20, 0, 940, 676 },
};
WstagAnimKey D_WSTAG746_800A6CA0[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG746_800A6CB8[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG746_800A6D10 = { 181, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D1C = { 181, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D28 = { 181, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D34 = { 181, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D40 = { 142, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D4C = { 142, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D58 = { 142, 26, 0x60080000 };
FieldstgListedBattle D_WSTAG746_800A6D64 = { 142, 26, 0x60080000 };
FieldstgBattleList D_WSTAG746_800A6D70 = {
    2,
    { &D_WSTAG746_800A6D10, &D_WSTAG746_800A6D1C, &D_WSTAG746_800A6D28, &D_WSTAG746_800A6D34, &D_WSTAG746_800A6D40,
        &D_WSTAG746_800A6D4C, &D_WSTAG746_800A6D58, &D_WSTAG746_800A6D64 },
};
FieldstgListedBattle D_WSTAG746_800A6D94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6DE8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG746_800A6DF4 = {
    0,
    { &D_WSTAG746_800A6D94, &D_WSTAG746_800A6DA0, &D_WSTAG746_800A6DAC, &D_WSTAG746_800A6DB8, &D_WSTAG746_800A6DC4,
        &D_WSTAG746_800A6DD0, &D_WSTAG746_800A6DDC, &D_WSTAG746_800A6DE8 },
};
FieldstgListedBattle D_WSTAG746_800A6E18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6E6C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG746_800A6E78 = {
    0,
    { &D_WSTAG746_800A6E18, &D_WSTAG746_800A6E24, &D_WSTAG746_800A6E30, &D_WSTAG746_800A6E3C, &D_WSTAG746_800A6E48,
        &D_WSTAG746_800A6E54, &D_WSTAG746_800A6E60, &D_WSTAG746_800A6E6C },
};
FieldstgListedBattle D_WSTAG746_800A6E9C = { 29, 26, 0x608C0000 };
FieldstgListedBattle D_WSTAG746_800A6EA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6EB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6EC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6ECC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6ED8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6EE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG746_800A6EF0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG746_800A6EFC = {
    0,
    { &D_WSTAG746_800A6E9C, &D_WSTAG746_800A6EA8, &D_WSTAG746_800A6EB4, &D_WSTAG746_800A6EC0, &D_WSTAG746_800A6ECC,
        &D_WSTAG746_800A6ED8, &D_WSTAG746_800A6EE4, &D_WSTAG746_800A6EF0 },
};
FieldstgBattleLists wstag746_battle_lists = {
    122, 0, 0, { &D_WSTAG746_800A6D70, &D_WSTAG746_800A6DF4, &D_WSTAG746_800A6E78 }, &D_WSTAG746_800A6EFC,
};
FieldstgVramPlace wstag746_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 368, 507 }, { 320, 256, 368, 384, 192, 128, 336, 506 },
    { 320, 256, 320, 352, 0, 96, 352, 506 },
};
u16 D_WSTAG746_800A6FCC[8] = { 0x25D, 1, 0x8B0E, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A6FDC[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A6FE4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A6FEC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A6FF4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A6FFC[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A7004[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A700C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A7014[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG746_800A701C[2] = { { NULL, D_WSTAG746_800A6FCC, 397 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG746_800A7034[2] = { { NULL, NULL, 537 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG746_800A704C[2] = { { NULL, NULL, 537 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG746_800A7064[5] = {
    { D_WSTAG746_800A6FDC, NULL, 534 }, { D_WSTAG746_800A6FE4, NULL, 535 }, { D_WSTAG746_800A6FEC, NULL, 536 },
    { D_WSTAG746_800A6FF4, NULL, 538 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG746_800A70A0[5] = {
    { D_WSTAG746_800A6FFC, NULL, 534 }, { D_WSTAG746_800A7004, NULL, 535 }, { D_WSTAG746_800A700C, NULL, 536 },
    { D_WSTAG746_800A7014, NULL, 538 }, { NULL, NULL, 0 },
};
u16 D_WSTAG746_800A70DC[4] = { 0x25D, 0, 0xFFFF, 0 };
u16 D_WSTAG746_800A70E4[6] = { 0x4087, 0, 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A70F0[6] = { 0x701A, 1, 0x4087, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A70FC[8] = { 0x701A, 0, 0x4087, 0, 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG746_800A710C[8] = { 0x701A, 0, 0x4087, 1, 0x7008, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG746_800A711C = { D_WSTAG746_800A70DC, D_WSTAG746_800A701C, 33, 4, 785, 233, 1 };
FieldstgPlacedActor D_WSTAG746_800A7130 = { D_WSTAG746_800A70E4, D_WSTAG746_800A7034, 157, 5, 240, 209, 7 };
FieldstgPlacedActor D_WSTAG746_800A7144 = { D_WSTAG746_800A70F0, D_WSTAG746_800A704C, 157, 5, 240, 209, 7 };
FieldstgPlacedActor D_WSTAG746_800A7158 = { D_WSTAG746_800A70FC, D_WSTAG746_800A7064, 205, 6, 240, 209, 7 };
FieldstgPlacedActor D_WSTAG746_800A716C = { D_WSTAG746_800A710C, D_WSTAG746_800A70A0, 205, 6, 240, 209, 7 };
FieldstgPlacedActor *wstag746_actors[6] = {
    &D_WSTAG746_800A711C, &D_WSTAG746_800A7130, &D_WSTAG746_800A7144, &D_WSTAG746_800A7158, &D_WSTAG746_800A716C,
    NULL,
};
FieldstgSprite wstag746_sprites[15] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 208, 361, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 240, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 368, 409, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 400, 521, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 432, 633, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 496, 345, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 528, 457, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 560, 569, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 592, 681, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 656, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 688, 505, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 720, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 752, 729, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 0xF, 4, 0, 848, 553, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag746_map_events[25] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D3, 0xD0, 0x33C, 5, 0, 0, 0 },
    { 0x4086, 0, 0x701A, 0, 8, 0x511, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 8, 0x1F40, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 8, 0x1F41, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 8, 0x1F42, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D4, 0x3C0, 0x2B0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag746_funcs = { wstag746_setup };
FieldstgEventDef wstag746_events[6] = {
    { 1297, D_WSTAG746_800A6A98, 0x014A0017, NULL, wstag746_event_1297_end },
    { 1298, D_WSTAG746_800A6B30, 0x014A0018, NULL, wstag746_event_1298_end },
    { 8000, NULL, 0, wstag746_event_8000_start, NULL }, { 8001, NULL, 0, wstag746_event_8001_start, NULL },
    { 8002, NULL, 0, wstag746_event_8002_start, NULL }, { -1, NULL, 0, NULL, NULL },
};
