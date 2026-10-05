#include "wstag.h"

/* WSTAG555: stage 0x248 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG555_800A63E4[];
extern WstagAnimKey D_WSTAG555_800A6418[];
extern WstagAnimKey D_WSTAG555_800A644C[];
extern WstagFuncs wstag555_funcs;
extern FieldstgBattleLists wstag555_battle_lists;
extern FieldstgVramPlace wstag555_vram_places[];
extern FieldstgPlacedActor *wstag555_actors[];
extern FieldstgSprite wstag555_sprites[];
extern FieldstgMapEvent wstag555_map_events[];
extern FieldstgEventDef wstag555_events[];

s32 wstag555_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag555_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag555_anim_update(WstagAnimObject *obj) {
    FieldstgSprite *sprite;
    s32 frames[3];

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG555_800A63E4[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG555_800A6418[0].time;
        obj->anims[2].key = 0;
        obj->anims[2].time = D_WSTAG555_800A644C[0].time;
        break;
    case OBJECT_STATE_RUN:
        sprite = fieldstg_stage.sprites;
        frames[0] = wstag555_anim_loop(&obj->anims[0], D_WSTAG555_800A63E4, 0);
        frames[1] = wstag555_anim_loop(&obj->anims[1], D_WSTAG555_800A6418, 0);
        frames[2] = wstag555_anim_loop(&obj->anims[2], D_WSTAG555_800A644C, 0);
        for (; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->sprite = frames[0];
                break;
            case 2:
                sprite->sprite = frames[1];
                break;
            case 3:
                sprite->sprite = frames[2];
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagAnimObject *wstag555_anim_new(void) {
    return object_new(wstag555_anim_update, sizeof(WstagAnimObject), 0);
}

void wstag555_update(WstagObject *obj, WstagAnimObject **data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        *data = wstag555_anim_new();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag555_start(void *arg0) {
    WstagObject *obj = object_new(wstag555_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag555_funcs.setup();
    return obj;
}

void wstag555_event_380_end(void) {
    gamestate_flags.set_flag(0x400C, 1);
}

void wstag555_setup(void) {
    fieldstg_stage.background_file = 0x243;
    fieldstg_stage.sprite_file = 0x02440000;
    fieldstg_stage.sprites = wstag555_sprites;
    fieldstg_stage.map_events = wstag555_map_events;
    fieldstg_stage.mask_file = 0x321;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x4D700, 0x4B700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag555_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag555_actors;
    fieldstg_stage.events = wstag555_events;
    fieldstg_stage.battle_lists = &wstag555_battle_lists;
    fieldstg_attr.set_file(0, 0x02440002);
    fieldstg_attr.set_file(7, 0x02440001);
    fieldstg_attr.set_file(4, 0x02440003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag555_setup(void);

s16 D_WSTAG555_800A6274[184] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 246, 355, 3),
    FIELDSTG_EVENT_PLACE(105, 320, 200),
    FIELDSTG_EVENT_ANIM(105, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(105, 219, 250, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 105),
    FIELDSTG_EVENT_ANIM(0x323, 805, 105),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 105),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(105, 219, 337, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 105, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 105, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 105, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(105, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(105, 199, 348, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WALK(105, 270, 384, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WALK(105, 423, 459, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 2),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG555_800A63E4[13] = {
    { 50, 8 }, { 51, 8 }, { 52, 8 }, { 53, 8 }, { 54, 8 }, { 55, 8 }, { 56, 8 }, { 57, 8 }, { 58, 8 }, { 59, 8 },
    { 60, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG555_800A6418[13] = {
    { 61, 8 }, { 62, 8 }, { 63, 8 }, { 64, 8 }, { 65, 8 }, { 66, 8 }, { 67, 8 }, { 68, 8 }, { 69, 8 }, { 70, 8 },
    { 71, 8 }, { 82, 160 }, { 255, 0 },
};
WstagAnimKey D_WSTAG555_800A644C[12] = {
    { 72, 8 }, { 73, 8 }, { 74, 8 }, { 75, 8 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 }, { 80, 8 }, { 81, 8 },
    { 82, 160 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG555_800A647C = { 148, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A6488 = { 148, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A6494 = { 148, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A64A0 = { 148, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A64AC = { 154, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A64B8 = { 154, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A64C4 = { 154, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG555_800A64D0 = { 154, 5, 0x60080000 };
FieldstgBattleList D_WSTAG555_800A64DC = {
    3,
    { &D_WSTAG555_800A647C, &D_WSTAG555_800A6488, &D_WSTAG555_800A6494, &D_WSTAG555_800A64A0, &D_WSTAG555_800A64AC,
        &D_WSTAG555_800A64B8, &D_WSTAG555_800A64C4, &D_WSTAG555_800A64D0 },
};
FieldstgListedBattle D_WSTAG555_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6554 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG555_800A6560 = {
    0,
    { &D_WSTAG555_800A6500, &D_WSTAG555_800A650C, &D_WSTAG555_800A6518, &D_WSTAG555_800A6524, &D_WSTAG555_800A6530,
        &D_WSTAG555_800A653C, &D_WSTAG555_800A6548, &D_WSTAG555_800A6554 },
};
FieldstgListedBattle D_WSTAG555_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A659C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A65A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A65B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A65C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A65CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A65D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG555_800A65E4 = {
    0,
    { &D_WSTAG555_800A6584, &D_WSTAG555_800A6590, &D_WSTAG555_800A659C, &D_WSTAG555_800A65A8, &D_WSTAG555_800A65B4,
        &D_WSTAG555_800A65C0, &D_WSTAG555_800A65CC, &D_WSTAG555_800A65D8 },
};
FieldstgListedBattle D_WSTAG555_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A6650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG555_800A665C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG555_800A6668 = {
    0,
    { &D_WSTAG555_800A6608, &D_WSTAG555_800A6614, &D_WSTAG555_800A6620, &D_WSTAG555_800A662C, &D_WSTAG555_800A6638,
        &D_WSTAG555_800A6644, &D_WSTAG555_800A6650, &D_WSTAG555_800A665C },
};
FieldstgBattleLists wstag555_battle_lists = {
    32, 0, 0, { &D_WSTAG555_800A64DC, &D_WSTAG555_800A6560, &D_WSTAG555_800A65E4 }, &D_WSTAG555_800A6668,
};
FieldstgVramPlace wstag555_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 432, 384, 448, 128, 368, 511 },
};
u16 D_WSTAG555_800A6718[4] = { 0x600F, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG555_800A6720 = { D_WSTAG555_800A6718, NULL, 105, 4, 0, 0, 7 };
FieldstgPlacedActor *wstag555_actors[2] = { &D_WSTAG555_800A6720, NULL };
FieldstgSprite wstag555_sprites[25] = {
    { 1, 0, 0x80, 2, 6, 0, 0, 0, 0, 0, 874, 1273, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 50, 721, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 87, 297, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 150, 1359, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 185, 1233, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 211, 917, 0, 0 },
    { 1, 3, 0xC8, 6, 0x48, 0, 0, 0, 0, 0, 232, 991, 0, 0 }, { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 299, 296, 0, 0 },
    { 1, 3, 0xC8, 6, 0x48, 0, 0, 0, 0, 0, 337, 292, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 401, 938, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 530, 1299, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 710, 1019, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 832, 395, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 964, 526, 0, 0 }, { 1, 1, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 967, 699, 0, 0 },
    { 1, 3, 0xC8, 6, 0x48, 0, 0, 0, 0, 0, 1223, 896, 0, 0 },
    { 1, 2, 0x40, 6, 0x3D, 0, 0, 0, 0, 0, 1411, 668, 0, 0 }, { 1, 0, 0x80, 6, 7, 0, 0, 0, 0, 0, 856, 1221, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 506, 475, 544, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 317, 423, 488, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 225, 615, 666, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 476, 647, 713, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 592, 815, 880, 0 }, { 1, 0, 0x64, 4, 5, 0, 0, 0, 0, 0, 289, 127, 230, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag555_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25D, 0xC8, 0x3BC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x249, 0x7A, 0xEA, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x247, 0xB0, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x257, 0x320, 0x88, 1, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0x1B, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 1, 2 },
    { 0x600F, 1, 0x400C, 0, 8, 0x17C, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag555_funcs = { wstag555_setup };
FieldstgEventDef wstag555_events[2] = {
    { 380, D_WSTAG555_800A6274, 0x013C0004, NULL, wstag555_event_380_end }, { -1, NULL, 0, NULL, NULL },
};
