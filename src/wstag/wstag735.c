#include "wstag.h"

/* WSTAG735: stage 0x26A (fieldstg_stages). */

extern WstagAnimKey D_WSTAG735_800A6854[];
extern WstagAnimKey D_WSTAG735_800A686C[];
extern WstagFuncs wstag735_funcs;
extern WstagSpawn D_WSTAG735_800A67DC[];
extern FieldstgBattleLists wstag735_battle_lists;
extern FieldstgVramPlace wstag735_vram_places[];
extern FieldstgPlacedActor *wstag735_actors[];
extern FieldstgSprite wstag735_sprites[];
extern FieldstgMapEvent wstag735_map_events[];
extern FieldstgEventDef wstag735_events[];
void wstag735_update(WstagObject *obj, WstagSpawnData *data);
WstagTwoSpriteObject *wstag735_two_sprite_create(s32 x, s32 y, s32 frame);
void wstag735_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);

void wstag735_update(WstagObject *obj, WstagSpawnData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 10; i++) {
            if (D_WSTAG735_800A67DC[i].condition == 0) {
                data->objs[i] = wstag735_two_sprite_create(D_WSTAG735_800A67DC[i].x, D_WSTAG735_800A67DC[i].y,
                                                       D_WSTAG735_800A67DC[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x400E, 1) && gamestate_flags.get_flag(0x4019, 0)) {
            data->event = fieldstg_event_start(0x317);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag735_start(void *arg0) {
    WstagObject *obj = object_new(wstag735_update, sizeof(WstagObject), sizeof(WstagSpawnData));

    obj->manager = arg0;
    wstag735_funcs.setup();
    return obj;
}

s32 wstag735_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag735_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag735_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag735_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag735_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG735_800A686C[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG735_800A6854[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag735_anim_advance(&obj->palette_anim, D_WSTAG735_800A6854, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag735_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag735_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG735_800A686C[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG735_800A6854[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag735_anim_advance(&obj->palette_anim, D_WSTAG735_800A6854, 0, 0);
        done = 0;
        frame = wstag735_anim_advance(&obj->frame_anim, D_WSTAG735_800A686C, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag735_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, wstag735_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag735_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, wstag735_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag735_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag735_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag735_event_790_end(void) {
    gamestate_flags.set_flag(0x400E, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag735_event_791_end(void) {
    gamestate_flags.set_flag(0x8ADD, 1);
    gamestate_flags.set_flag(0x4019, 1);
}

void wstag735_setup(void) {
    fieldstg_stage.background_file = 0x6BA;
    fieldstg_stage.sprite_file = 0x06BB0000;
    fieldstg_stage.sprites = wstag735_sprites;
    fieldstg_stage.map_events = wstag735_map_events;
    fieldstg_stage.mask_file = 0x6B9;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12500, 0x24E00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag735_vram_places;
    fieldstg_stage.music = 0x19;
    fieldstg_stage.sound = 0x60640000;
    fieldstg_stage.actors = wstag735_actors;
    fieldstg_stage.battle_lists = &wstag735_battle_lists;
    fieldstg_stage.events = wstag735_events;
    fieldstg_attr.set_file(0, 0x06BB0001);
    fieldstg_attr.set_file(7, 0x06BB0002);
    fieldstg_attr.set_file(4, 0x06BB0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag735_setup(void);

s16 D_WSTAG735_800A66AC[78] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 312, 604, 3),
    FIELDSTG_EVENT_PLACE(200, 288, 593),
    FIELDSTG_EVENT_ANIM(200, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 200, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 200, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x800A, /* padding, not read */
};
s16 D_WSTAG735_800A6748[74] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 200),
    FIELDSTG_EVENT_PLACE(2, 312, 604),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(200, 288, 593),
    FIELDSTG_EVENT_ANIM(200, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 200, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(200, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(200, 336, 569, 5),
    FIELDSTG_EVENT_WAIT_WALK(200),
    FIELDSTG_EVENT_ANIM(200, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 288, 593, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 200, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x800A, /* padding, not read */
};
WstagSpawn D_WSTAG735_800A67DC[10] = {
    { 36, 0, 332, 612 }, { 28, 0, 236, 436 }, { 28, 0, 380, 460 }, { 28, 0, 444, 652 }, { 28, 0, 540, 700 },
    { 28, 0, 684, 724 }, { 28, 0, 812, 756 }, { 28, 0, 908, 756 }, { 28, 0, 1020, 700 }, { 20, 0, 940, 68 },
};
WstagAnimKey D_WSTAG735_800A6854[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG735_800A686C[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG735_800A68C4 = { 117, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A68D0 = { 117, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A68DC = { 117, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A68E8 = { 117, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A68F4 = { 167, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A6900 = { 167, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A690C = { 167, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG735_800A6918 = { 167, 25, 0x60080000 };
FieldstgBattleList D_WSTAG735_800A6924 = {
    5,
    { &D_WSTAG735_800A68C4, &D_WSTAG735_800A68D0, &D_WSTAG735_800A68DC, &D_WSTAG735_800A68E8, &D_WSTAG735_800A68F4,
        &D_WSTAG735_800A6900, &D_WSTAG735_800A690C, &D_WSTAG735_800A6918 },
};
FieldstgListedBattle D_WSTAG735_800A6948 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6954 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6960 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A696C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6978 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6984 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6990 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A699C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG735_800A69A8 = {
    0,
    { &D_WSTAG735_800A6948, &D_WSTAG735_800A6954, &D_WSTAG735_800A6960, &D_WSTAG735_800A696C, &D_WSTAG735_800A6978,
        &D_WSTAG735_800A6984, &D_WSTAG735_800A6990, &D_WSTAG735_800A699C },
};
FieldstgListedBattle D_WSTAG735_800A69CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A69D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A69E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A69F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A69FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A20 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG735_800A6A2C = {
    0,
    { &D_WSTAG735_800A69CC, &D_WSTAG735_800A69D8, &D_WSTAG735_800A69E4, &D_WSTAG735_800A69F0, &D_WSTAG735_800A69FC,
        &D_WSTAG735_800A6A08, &D_WSTAG735_800A6A14, &D_WSTAG735_800A6A20 },
};
FieldstgListedBattle D_WSTAG735_800A6A50 = { 19, 25, 0x608C0000 };
FieldstgListedBattle D_WSTAG735_800A6A5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6A98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG735_800A6AA4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG735_800A6AB0 = {
    0,
    { &D_WSTAG735_800A6A50, &D_WSTAG735_800A6A5C, &D_WSTAG735_800A6A68, &D_WSTAG735_800A6A74, &D_WSTAG735_800A6A80,
        &D_WSTAG735_800A6A8C, &D_WSTAG735_800A6A98, &D_WSTAG735_800A6AA4 },
};
FieldstgBattleLists wstag735_battle_lists = {
    87, 0, 0, { &D_WSTAG735_800A6924, &D_WSTAG735_800A69A8, &D_WSTAG735_800A6A2C }, &D_WSTAG735_800A6AB0,
};
FieldstgVramPlace wstag735_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 350, 424, 120, 168, 336, 502 }, { 320, 256, 358, 424, 152, 168, 352, 502 },
    { 320, 256, 366, 424, 184, 168, 368, 502 }, { 320, 256, 374, 424, 216, 168, 320, 501 },
    { 320, 256, 320, 432, 0, 176, 336, 501 },
};
u16 D_WSTAG735_800A6BA0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG735_800A6BA8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6BB0[4] = { 0, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG735_800A6BB8[2] = { { NULL, NULL, 503 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6BD0[2] = { { NULL, NULL, 501 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6BE8[3] = {
    { D_WSTAG735_800A6BA0, D_WSTAG735_800A6BA8, 500 }, { D_WSTAG735_800A6BB0, NULL, 501 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG735_800A6C0C[2] = { { NULL, NULL, 507 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C24[2] = { { NULL, NULL, 506 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C3C[2] = { { NULL, NULL, 505 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C54[2] = { { NULL, NULL, 504 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C6C[2] = { { NULL, NULL, 502 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C84[2] = { { NULL, NULL, 516 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6C9C[2] = { { NULL, NULL, 517 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6CB4[2] = { { NULL, NULL, 515 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6CCC[2] = { { NULL, NULL, 514 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG735_800A6CE4[2] = { { NULL, NULL, 514 }, { NULL, NULL, 0 } };
u16 D_WSTAG735_800A6CFC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D04[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D0C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D14[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D1C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D24[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D2C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D34[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D3C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D44[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D4C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D54[6] = { 0x4019, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG735_800A6D60[6] = { 0x7019, 1, 0x4019, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG735_800A6D6C = { D_WSTAG735_800A6CFC, D_WSTAG735_800A6BB8, 46, 4, 1041, 105, 1 };
FieldstgPlacedActor D_WSTAG735_800A6D80 = { D_WSTAG735_800A6D04, D_WSTAG735_800A6BD0, 46, 4, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG735_800A6D94 = { D_WSTAG735_800A6D0C, D_WSTAG735_800A6BE8, 46, 4, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG735_800A6DA8 = { D_WSTAG735_800A6D14, D_WSTAG735_800A6C0C, 65, 5, 944, 233, 5 };
FieldstgPlacedActor D_WSTAG735_800A6DBC = { D_WSTAG735_800A6D1C, D_WSTAG735_800A6C24, 65, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG735_800A6DD0 = { D_WSTAG735_800A6D24, D_WSTAG735_800A6C3C, 65, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG735_800A6DE4 = { D_WSTAG735_800A6D2C, D_WSTAG735_800A6C54, 65, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG735_800A6DF8 = { D_WSTAG735_800A6D34, D_WSTAG735_800A6C6C, 157, 6, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG735_800A6E0C = { D_WSTAG735_800A6D3C, D_WSTAG735_800A6C84, 158, 7, 336, 569, 1 };
FieldstgPlacedActor D_WSTAG735_800A6E20 = { D_WSTAG735_800A6D44, D_WSTAG735_800A6C9C, 200, 8, 336, 569, 1 };
FieldstgPlacedActor D_WSTAG735_800A6E34 = { D_WSTAG735_800A6D4C, D_WSTAG735_800A6CB4, 200, 8, 336, 569, 1 };
FieldstgPlacedActor D_WSTAG735_800A6E48 = { D_WSTAG735_800A6D54, D_WSTAG735_800A6CCC, 200, 8, 288, 593, 7 };
FieldstgPlacedActor D_WSTAG735_800A6E5C = { D_WSTAG735_800A6D60, D_WSTAG735_800A6CE4, 200, 8, 336, 569, 1 };
FieldstgPlacedActor *wstag735_actors[14] = {
    &D_WSTAG735_800A6D6C, &D_WSTAG735_800A6D80, &D_WSTAG735_800A6D94, &D_WSTAG735_800A6DA8, &D_WSTAG735_800A6DBC,
    &D_WSTAG735_800A6DD0, &D_WSTAG735_800A6DE4, &D_WSTAG735_800A6DF8, &D_WSTAG735_800A6E0C, &D_WSTAG735_800A6E20,
    &D_WSTAG735_800A6E34, &D_WSTAG735_800A6E48, &D_WSTAG735_800A6E5C, NULL,
};
FieldstgSprite wstag735_sprites[49] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 430, 326, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 443, 315, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 456, 288, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 458, 346, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 464, 275, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 494, 375, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 500, 402, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 555, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 574, 190, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 698, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 727, 379, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 778, 446, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 814, 430, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 872, 226, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 886, 236, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 909, 415, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 971, 387, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 996, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 1022, 295, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 1036, 293, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 435, 329, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 446, 320, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 452, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 462, 280, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 464, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 497, 381, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 501, 395, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 548, 194, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 567, 186, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 573, 196, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 584, 253, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 691, 394, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 694, 361, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 699, 371, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 724, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 785, 443, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 817, 425, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 841, 435, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 848, 267, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 852, 465, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 875, 232, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 890, 242, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 915, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 976, 389, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 988, 346, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 991, 377, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 1014, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 1034, 300, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag735_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x268, 0x78, 0x144, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26B, 0x40C, 0x6E, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x400, 0x80, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x3F0, 0xD8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 9, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0xB, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x26A, 0x140, 0x260, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x26A, 0x3C0, 0x50, 7, 0, 0, 0 },
    { 0x8011, 1, 0x400E, 0, 8, 0x316, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0xB, 0, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xC, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag735_funcs = { wstag735_setup };
FieldstgEventDef wstag735_events[3] = {
    { 790, D_WSTAG735_800A66AC, 0x014A000A, NULL, wstag735_event_790_end },
    { 791, D_WSTAG735_800A6748, 0x014A000B, NULL, wstag735_event_791_end }, { -1, NULL, 0, NULL, NULL },
};
