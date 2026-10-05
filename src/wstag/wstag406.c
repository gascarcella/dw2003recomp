#include "wstag.h"

/* WSTAG406: stage 0x29A (fieldstg_stages). */

extern WstagAnimKey D_WSTAG406_800A63D8[];
extern WstagFuncs wstag406_funcs;
extern FieldstgBattleLists wstag406_battle_lists;
extern FieldstgVramPlace wstag406_vram_places[];
extern FieldstgPlacedActor *wstag406_actors[];
extern FieldstgSprite wstag406_sprites[];
extern FieldstgMapEvent wstag406_map_events[];
extern FieldstgEventDef wstag406_events[];
void wstag406_anim1_update();
void wstag406_update(WstagObject *obj, WstagObjEventData *data);

s32 wstag406_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag406_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag406_anim1_update(WstagAnim1Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anim.key = 0;
        obj->anim.time = D_WSTAG406_800A63D8[0].time;
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag406_anim_loop(&obj->anim, D_WSTAG406_800A63D8, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag406_anim1_create(void) {
    return object_new(wstag406_anim1_update, sizeof(WstagAnim1Object), 0);
}

void wstag406_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag406_anim1_create();
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x407A, 1) && gamestate_flags.get_flag(0x407B, 0)) {
            data->event = fieldstg_event_start(0x506);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag406_start(void *arg0) {
    WstagObject *obj = object_new(wstag406_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag406_funcs.setup();
    return obj;
}

void wstag406_event_1285_end(void) {
    gamestate_flags.set_flag(0x407A, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag406_event_1286_end(void) {
    gamestate_flags.set_flag(0x407B, 1);
    gamestate_flags.set_flag(0x8AF5, 1);
}

void wstag406_setup(void) {
    fieldstg_stage.background_file = 0x743;
    fieldstg_stage.sprite_file = 0x07440000;
    fieldstg_stage.sprites = wstag406_sprites;
    fieldstg_stage.map_events = wstag406_map_events;
    fieldstg_stage.mask_file = 0x742;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x17A00, 0x44500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag406_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag406_actors;
    fieldstg_stage.battle_lists = &wstag406_battle_lists;
    fieldstg_stage.events = wstag406_events;
    fieldstg_attr.set_file(0, 0x07440001);
    fieldstg_attr.set_file(7, 0x07440002);
    fieldstg_attr.set_file(4, 0x07440003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag406_setup(void);

s16 D_WSTAG406_800A6294[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 891, 339, 5),
    FIELDSTG_EVENT_PLACE(264, 923, 317),
    FIELDSTG_EVENT_ANIM(264, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 264, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 264, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x2, /* padding, not read */
};
s16 D_WSTAG406_800A6334[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 891, 339),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(264, 923, 317),
    FIELDSTG_EVENT_ANIM(264, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 264, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 264, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 264, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG406_800A63D8[35] = {
    { 50, 8 }, { 51, 4 }, { 52, 8 }, { 53, 4 }, { 54, 8 }, { 55, 4 }, { 56, 8 }, { 57, 16 }, { 58, 4 }, { 59, 8 },
    { 60, 4 }, { 61, 8 }, { 62, 8 }, { 63, 12 }, { 64, 20 }, { 65, 4 }, { 66, 8 }, { 67, 4 }, { 68, 8 }, { 69, 8 },
    { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 4 }, { 74, 8 }, { 75, 4 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 },
    { 80, 8 }, { 81, 12 }, { 82, 8 }, { 83, 30 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG406_800A6464 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6470 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A647C = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6488 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6494 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A64A0 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A64AC = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A64B8 = { 101, 4, 0x60080000 };
FieldstgBattleList D_WSTAG406_800A64C4 = {
    3,
    { &D_WSTAG406_800A6464, &D_WSTAG406_800A6470, &D_WSTAG406_800A647C, &D_WSTAG406_800A6488, &D_WSTAG406_800A6494,
        &D_WSTAG406_800A64A0, &D_WSTAG406_800A64AC, &D_WSTAG406_800A64B8 },
};
FieldstgListedBattle D_WSTAG406_800A64E8 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A64F4 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6500 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A650C = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6518 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6524 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A6530 = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG406_800A653C = { 101, 4, 0x60080000 };
FieldstgBattleList D_WSTAG406_800A6548 = {
    5,
    { &D_WSTAG406_800A64E8, &D_WSTAG406_800A64F4, &D_WSTAG406_800A6500, &D_WSTAG406_800A650C, &D_WSTAG406_800A6518,
        &D_WSTAG406_800A6524, &D_WSTAG406_800A6530, &D_WSTAG406_800A653C },
};
FieldstgListedBattle D_WSTAG406_800A656C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A659C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A65A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A65B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A65C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG406_800A65CC = {
    0,
    { &D_WSTAG406_800A656C, &D_WSTAG406_800A6578, &D_WSTAG406_800A6584, &D_WSTAG406_800A6590, &D_WSTAG406_800A659C,
        &D_WSTAG406_800A65A8, &D_WSTAG406_800A65B4, &D_WSTAG406_800A65C0 },
};
FieldstgListedBattle D_WSTAG406_800A65F0 = { 15, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG406_800A65FC = { 317, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG406_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG406_800A6644 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG406_800A6650 = {
    0,
    { &D_WSTAG406_800A65F0, &D_WSTAG406_800A65FC, &D_WSTAG406_800A6608, &D_WSTAG406_800A6614, &D_WSTAG406_800A6620,
        &D_WSTAG406_800A662C, &D_WSTAG406_800A6638, &D_WSTAG406_800A6644 },
};
FieldstgBattleLists wstag406_battle_lists = {
    69, 0, 0, { &D_WSTAG406_800A64C4, &D_WSTAG406_800A6548, &D_WSTAG406_800A65CC }, &D_WSTAG406_800A6650,
};
FieldstgVramPlace wstag406_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 362, 256, 168, 0, 368, 511 },
};
u16 D_WSTAG406_800A6700[4] = { 0xA09, 0, 0xFFFF, 0 };
u16 D_WSTAG406_800A6708[6] = { 0xA09, 1, 0x903A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6714[4] = { 0xA09, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A671C[4] = { 0xA09, 0, 0xFFFF, 0 };
u16 D_WSTAG406_800A6724[6] = { 0xA09, 1, 0x903A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6730[4] = { 0xA09, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6738[4] = { 0xA09, 0, 0xFFFF, 0 };
u16 D_WSTAG406_800A6740[6] = { 0xA09, 1, 0x903A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A674C[4] = { 0xA09, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6754[4] = { 0xA09, 0, 0xFFFF, 0 };
u16 D_WSTAG406_800A675C[6] = { 0xA09, 1, 0x903A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6768[4] = { 0xA09, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6770[4] = { 0xA09, 0, 0xFFFF, 0 };
u16 D_WSTAG406_800A6778[6] = { 0xA09, 1, 0x903A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6784[4] = { 0xA09, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG406_800A678C[3] = {
    { D_WSTAG406_800A6700, D_WSTAG406_800A6708, 723 }, { D_WSTAG406_800A6714, NULL, 153 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG406_800A67B0[3] = {
    { D_WSTAG406_800A671C, D_WSTAG406_800A6724, 723 }, { D_WSTAG406_800A6730, NULL, 154 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG406_800A67D4[3] = {
    { D_WSTAG406_800A6738, D_WSTAG406_800A6740, 723 }, { D_WSTAG406_800A674C, NULL, 155 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG406_800A67F8[3] = {
    { D_WSTAG406_800A6754, D_WSTAG406_800A675C, 723 }, { D_WSTAG406_800A6768, NULL, 156 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG406_800A681C[3] = {
    { D_WSTAG406_800A6770, D_WSTAG406_800A6778, 723 }, { D_WSTAG406_800A6784, NULL, 157 }, { NULL, NULL, 0 },
};
u16 D_WSTAG406_800A6840[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6848[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6850[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6858[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG406_800A6860[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG406_800A6868 = { D_WSTAG406_800A6840, D_WSTAG406_800A678C, 264, 4, 923, 317, 1 };
FieldstgPlacedActor D_WSTAG406_800A687C = { D_WSTAG406_800A6848, D_WSTAG406_800A67B0, 264, 4, 923, 317, 1 };
FieldstgPlacedActor D_WSTAG406_800A6890 = { D_WSTAG406_800A6850, D_WSTAG406_800A67D4, 264, 4, 923, 317, 1 };
FieldstgPlacedActor D_WSTAG406_800A68A4 = { D_WSTAG406_800A6858, D_WSTAG406_800A67F8, 264, 4, 923, 317, 1 };
FieldstgPlacedActor D_WSTAG406_800A68B8 = { D_WSTAG406_800A6860, D_WSTAG406_800A681C, 264, 4, 923, 317, 1 };
FieldstgPlacedActor *wstag406_actors[6] = {
    &D_WSTAG406_800A6868, &D_WSTAG406_800A687C, &D_WSTAG406_800A6890, &D_WSTAG406_800A68A4, &D_WSTAG406_800A68B8,
    NULL,
};
FieldstgSprite wstag406_sprites[7] = {
    { 1, 1, 0xE6, 2, 0x32, 0, 0, 0, 0, 0, 966, 364, 0, 0 }, { 1, 0, 0x40, 2, 0x54, 2, 0, 2, 6, 0, 931, 293, 0, 0 },
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 176, 403, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 600, 1073, 0, 0 },
    { 1, 0, 0x78, 6, 7, 0, 0, 0, 0, 0, 745, 598, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 298, 442, 505, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag406_map_events[13] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0x648, 0xD0, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0xF0, 0x4D8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x100, 0x450, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x102, 0x420, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0xF2, 0x388, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1D2, 0x4C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1C2, 0x460, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xF, 0x1AF, 0x428, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xF, 0x1BF, 0x330, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0x16, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x330, 0x208, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x340, 0x170, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag406_funcs = { wstag406_setup };
FieldstgEventDef wstag406_events[3] = {
    { 1285, D_WSTAG406_800A6294, 0x0135001B, NULL, wstag406_event_1285_end },
    { 1286, D_WSTAG406_800A6334, 0x0135001C, NULL, wstag406_event_1286_end }, { -1, NULL, 0, NULL, NULL },
};
