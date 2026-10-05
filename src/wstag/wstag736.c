#include "wstag.h"

/* WSTAG736: stage 0x2D2 (fieldstg_stages). */

extern WstagSpawn D_WSTAG736_800A6800[];
extern WstagAnimKey D_WSTAG736_800A6878[];
extern WstagAnimKey D_WSTAG736_800A6890[];
extern WstagFuncs wstag736_funcs;
extern FieldstgBattleLists wstag736_battle_lists;
extern FieldstgVramPlace wstag736_vram_places[];
extern FieldstgPlacedActor *wstag736_actors[];
extern FieldstgSprite wstag736_sprites[];
extern FieldstgMapEvent wstag736_map_events[];
extern FieldstgEventDef wstag736_events[];
void wstag736_update();
void wstag736_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *layer, s32 which);
WstagTwoSpriteObject *wstag736_two_sprite_create(s32 x, s32 y, s32 frame);

void wstag736_update(WstagObject *obj, WstagSpawnData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 10; i++) {
            if (D_WSTAG736_800A6800[i].condition == 0) {
                data->objs[i] = wstag736_two_sprite_create(D_WSTAG736_800A6800[i].x, D_WSTAG736_800A6800[i].y,
                                                       D_WSTAG736_800A6800[i].frame);
            }
        }
        if (gamestate_flags.get_flag(0x4082, 1) && gamestate_flags.get_flag(0x4083, 0)) {
            data->event = fieldstg_event_start(0x50E);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag736_start(void *arg0) {
    WstagObject *obj = object_new(wstag736_update, sizeof(WstagObject), sizeof(WstagSpawnData));

    obj->manager = arg0;
    wstag736_funcs.setup();
    return obj;
}

s32 wstag736_anim_advance(WstagAnim *anim, WstagAnimKey *keys, s32 once, s32 depth) {
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
        wstag736_anim_advance(anim, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag736_two_sprite_draw(WstagTwoSpriteObject *obj, GfxLayer *arg1, s32 which) {
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

s32 wstag736_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
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

void wstag736_two_sprite_update(WstagTwoSpriteObject *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame_anim.key = 0;
        obj->base.key1 = obj->x;
        obj->base.key2 = obj->y;
        obj->frame_anim.time = D_WSTAG736_800A6890[0].time;
        obj->palette_anim.key = 0;
        obj->palette_anim.time = D_WSTAG736_800A6878[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag736_anim_advance(&obj->palette_anim, D_WSTAG736_800A6878, 0, 0);
        if (obj->sprites[0].frame != 0 && wstag736_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag736_two_sprite_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->frame_anim.key = 0;
            obj->frame_anim.time = D_WSTAG736_800A6890[0].time;
            obj->palette_anim.key = 0;
            obj->palette_anim.time = D_WSTAG736_800A6878[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->sprites[0].frame = obj->frame;
        obj->sprites[0].palette = wstag736_anim_advance(&obj->palette_anim, D_WSTAG736_800A6878, 0, 0);
        done = 0;
        frame = wstag736_anim_advance(&obj->frame_anim, D_WSTAG736_800A6890, 1, 0);
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
        if (obj->sprites[0].frame != 0 && wstag736_is_on_screen(obj->x, obj->y, 0x20, 0x20)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag736_two_sprite_draw, obj, obj->y, 0);
        }
        if (obj->sprites[1].frame != 0 && wstag736_is_on_screen(obj->x, obj->y - 0x20, 0x20, 0x40)) {
            layer->add_callback(layer, (WstagDrawCallback)wstag736_two_sprite_draw, obj, obj->y + 0x12, 1);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

WstagTwoSpriteObject *wstag736_two_sprite_create(s32 x, s32 y, s32 frame) {
    WstagTwoSpriteObject *obj = object_create(wstag736_two_sprite_update, sizeof(WstagTwoSpriteObject), 0, 0x17);

    obj->x = x;
    obj->y = y;
    obj->frame = frame;
    return obj;
}

void wstag736_event_1293_end(void) {
    gamestate_flags.set_flag(0x4082, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag736_event_1294_end(void) {
    gamestate_flags.set_flag(0x4083, 1);
    gamestate_flags.set_flag(0x8230, 1);
}

void wstag736_setup(void) {
    fieldstg_stage.background_file = 0x6C9;
    fieldstg_stage.sprite_file = 0x06CA0000;
    fieldstg_stage.sprites = wstag736_sprites;
    fieldstg_stage.map_events = wstag736_map_events;
    fieldstg_stage.mask_file = 0x6C8;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12700, 0x24B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag736_vram_places;
    fieldstg_stage.music = 0x19;
    fieldstg_stage.sound = 0x60640000;
    fieldstg_stage.actors = wstag736_actors;
    fieldstg_stage.battle_lists = &wstag736_battle_lists;
    fieldstg_stage.events = wstag736_events;
    fieldstg_attr.set_file(0, 0x06CA0001);
    fieldstg_attr.set_file(7, 0x06CA0002);
    fieldstg_attr.set_file(4, 0x06CA0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag736_setup(void);

s16 D_WSTAG736_800A66AC[77] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 312, 604, 3),
    FIELDSTG_EVENT_PLACE(203, 288, 593),
    FIELDSTG_EVENT_ANIM(203, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 203, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 203, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG736_800A6748[91] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 203),
    FIELDSTG_EVENT_PLACE(2, 312, 604),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(203, 288, 593),
    FIELDSTG_EVENT_ANIM(203, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 203, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(203, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(203, 336, 569, 5),
    FIELDSTG_EVENT_WAIT_WALK(203),
    FIELDSTG_EVENT_ANIM(203, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 288, 593, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 203, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
WstagSpawn D_WSTAG736_800A6800[10] = {
    { 36, 0, 332, 612 }, { 28, 0, 236, 436 }, { 28, 0, 380, 460 }, { 28, 0, 444, 652 }, { 28, 0, 540, 700 },
    { 28, 0, 684, 724 }, { 28, 0, 812, 756 }, { 28, 0, 908, 756 }, { 28, 0, 1020, 700 }, { 20, 0, 940, 68 },
};
WstagAnimKey D_WSTAG736_800A6878[6] = { { 0, 6 }, { 1, 6 }, { 2, 68 }, { 1, 4 }, { 0, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG736_800A6890[22] = {
    { 300, 18 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
    { 5, 4 }, { 6, 4 }, { 7, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 4, 4 }, { 3, 4 }, { 2, 4 }, { 1, 4 },
    { 255, 999 },
};
FieldstgListedBattle D_WSTAG736_800A68E8 = { 187, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A68F4 = { 187, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A6900 = { 187, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A690C = { 187, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A6918 = { 129, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A6924 = { 129, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A6930 = { 129, 25, 0x60080000 };
FieldstgListedBattle D_WSTAG736_800A693C = { 129, 25, 0x60080000 };
FieldstgBattleList D_WSTAG736_800A6948 = {
    5,
    { &D_WSTAG736_800A68E8, &D_WSTAG736_800A68F4, &D_WSTAG736_800A6900, &D_WSTAG736_800A690C, &D_WSTAG736_800A6918,
        &D_WSTAG736_800A6924, &D_WSTAG736_800A6930, &D_WSTAG736_800A693C },
};
FieldstgListedBattle D_WSTAG736_800A696C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6978 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6984 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6990 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A699C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A69A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A69B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A69C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG736_800A69CC = {
    0,
    { &D_WSTAG736_800A696C, &D_WSTAG736_800A6978, &D_WSTAG736_800A6984, &D_WSTAG736_800A6990, &D_WSTAG736_800A699C,
        &D_WSTAG736_800A69A8, &D_WSTAG736_800A69B4, &D_WSTAG736_800A69C0 },
};
FieldstgListedBattle D_WSTAG736_800A69F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A69FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A44 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG736_800A6A50 = {
    0,
    { &D_WSTAG736_800A69F0, &D_WSTAG736_800A69FC, &D_WSTAG736_800A6A08, &D_WSTAG736_800A6A14, &D_WSTAG736_800A6A20,
        &D_WSTAG736_800A6A2C, &D_WSTAG736_800A6A38, &D_WSTAG736_800A6A44 },
};
FieldstgListedBattle D_WSTAG736_800A6A74 = { 27, 25, 0x608C0000 };
FieldstgListedBattle D_WSTAG736_800A6A80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6A98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6AA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6AB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6ABC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG736_800A6AC8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG736_800A6AD4 = {
    0,
    { &D_WSTAG736_800A6A74, &D_WSTAG736_800A6A80, &D_WSTAG736_800A6A8C, &D_WSTAG736_800A6A98, &D_WSTAG736_800A6AA4,
        &D_WSTAG736_800A6AB0, &D_WSTAG736_800A6ABC, &D_WSTAG736_800A6AC8 },
};
FieldstgBattleLists wstag736_battle_lists = {
    120, 0, 0, { &D_WSTAG736_800A6948, &D_WSTAG736_800A69CC, &D_WSTAG736_800A6A50 }, &D_WSTAG736_800A6AD4,
};
FieldstgVramPlace wstag736_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 326, 432, 24, 176, 336, 502 }, { 320, 256, 350, 424, 120, 168, 352, 502 },
    { 320, 256, 358, 424, 152, 168, 368, 502 }, { 320, 256, 366, 424, 184, 168, 320, 501 },
    { 320, 256, 320, 432, 0, 176, 352, 501 },
};
u16 D_WSTAG736_800A6BC4[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BCC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BD4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BDC[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BE4[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BEC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BF4[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6BFC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C04[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C0C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C14[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C1C[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C24[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6C2C[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG736_800A6C34[4] = {
    { D_WSTAG736_800A6BC4, NULL, 519 }, { D_WSTAG736_800A6BCC, NULL, 520 }, { D_WSTAG736_800A6BD4, NULL, 521 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG736_800A6C64[2] = { { NULL, NULL, 523 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6C7C[4] = {
    { D_WSTAG736_800A6BDC, NULL, 524 }, { D_WSTAG736_800A6BE4, NULL, 525 }, { D_WSTAG736_800A6BEC, NULL, 526 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG736_800A6CAC[2] = { { NULL, NULL, 528 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6CC4[2] = { { NULL, NULL, 527 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6CDC[2] = { { NULL, NULL, 517 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6CF4[2] = { { NULL, NULL, 517 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6D0C[2] = { { NULL, NULL, 522 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG736_800A6D24[5] = {
    { D_WSTAG736_800A6BF4, NULL, 514 }, { D_WSTAG736_800A6BFC, NULL, 515 }, { D_WSTAG736_800A6C04, NULL, 516 },
    { D_WSTAG736_800A6C0C, NULL, 518 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG736_800A6D60[5] = {
    { D_WSTAG736_800A6C14, NULL, 514 }, { D_WSTAG736_800A6C1C, NULL, 515 }, { D_WSTAG736_800A6C24, NULL, 516 },
    { D_WSTAG736_800A6C2C, NULL, 518 }, { NULL, NULL, 0 },
};
u16 D_WSTAG736_800A6D9C[6] = { 0x7008, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DA8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DB0[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DB8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DC0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DC8[6] = { 0x4083, 0, 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DD4[6] = { 0x701A, 1, 0x4083, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DE0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DE8[8] = { 0x701A, 0, 0x4083, 1, 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG736_800A6DF8[8] = { 0x7008, 1, 0x4083, 0, 0x701A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG736_800A6E08 = { D_WSTAG736_800A6D9C, D_WSTAG736_800A6C34, 45, 4, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG736_800A6E1C = { D_WSTAG736_800A6DA8, D_WSTAG736_800A6C64, 45, 4, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG736_800A6E30 = { D_WSTAG736_800A6DB0, D_WSTAG736_800A6C7C, 66, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG736_800A6E44 = { D_WSTAG736_800A6DB8, D_WSTAG736_800A6CAC, 66, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG736_800A6E58 = { D_WSTAG736_800A6DC0, D_WSTAG736_800A6CC4, 66, 5, 1041, 105, 3 };
FieldstgPlacedActor D_WSTAG736_800A6E6C = { D_WSTAG736_800A6DC8, D_WSTAG736_800A6CDC, 157, 6, 288, 593, 7 };
FieldstgPlacedActor D_WSTAG736_800A6E80 = { D_WSTAG736_800A6DD4, D_WSTAG736_800A6CF4, 157, 6, 336, 569, 1 };
FieldstgPlacedActor D_WSTAG736_800A6E94 = { D_WSTAG736_800A6DE0, D_WSTAG736_800A6D0C, 158, 7, 944, 233, 1 };
FieldstgPlacedActor D_WSTAG736_800A6EA8 = { D_WSTAG736_800A6DE8, D_WSTAG736_800A6D24, 203, 8, 336, 569, 1 };
FieldstgPlacedActor D_WSTAG736_800A6EBC = { D_WSTAG736_800A6DF8, D_WSTAG736_800A6D60, 203, 8, 288, 593, 7 };
FieldstgPlacedActor *wstag736_actors[11] = {
    &D_WSTAG736_800A6E08, &D_WSTAG736_800A6E1C, &D_WSTAG736_800A6E30, &D_WSTAG736_800A6E44, &D_WSTAG736_800A6E58,
    &D_WSTAG736_800A6E6C, &D_WSTAG736_800A6E80, &D_WSTAG736_800A6E94, &D_WSTAG736_800A6EA8, &D_WSTAG736_800A6EBC,
    NULL,
};
FieldstgSprite wstag736_sprites[49] = {
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
FieldstgMapEvent wstag736_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D0, 0x78, 0x144, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D3, 0x40C, 0x6E, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x400, 0x80, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x3F0, 0xD8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 9, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0xB, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D2, 0x140, 0x260, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xD, 0x2D2, 0x3C0, 0x50, 7, 0, 0, 0 },
    { 0x4082, 0, 0x701A, 0, 8, 0x50D, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0xB, 0, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xC, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag736_funcs = { wstag736_setup };
FieldstgEventDef wstag736_events[3] = {
    { 1293, D_WSTAG736_800A66AC, 0x014A0009, NULL, wstag736_event_1293_end },
    { 1294, D_WSTAG736_800A6748, 0x014A0014, NULL, wstag736_event_1294_end }, { -1, NULL, 0, NULL, NULL },
};
