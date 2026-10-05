#include "wstag.h"

/* WSTAG927: stage 0x278 (fieldstg_stages_2d). */

extern WstagAnimKey D_WSTAG927_800A6244[];
extern WstagAnimKey D_WSTAG927_800A6294[];
extern WstagFuncs wstag927_funcs;
extern FieldstgVramPlace wstag927_vram_places[];
extern FieldstgPlacedActor *wstag927_actors[];
extern FieldstgSprite wstag927_sprites[];
extern FieldstgEventDef wstag927_events[];
extern FieldstgBattleLists wstag927_battle_lists;
void wstag927_anim2_update();
void wstag927_update();

s32 wstag927_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag927_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag927_anim2_update(WstagAnim2Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->anims[0].key = 0;
        obj->anims[0].time = D_WSTAG927_800A6244[0].time;
        obj->anims[1].key = 0;
        obj->anims[1].time = D_WSTAG927_800A6294[0].time;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag927_anim_loop(&obj->anims[0], D_WSTAG927_800A6244, 0);
            }
            if (sprite->type == 2) {
                sprite->sprite = wstag927_anim_loop(&obj->anims[1], D_WSTAG927_800A6294, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag927_anim2_create(void) {
    return object_new(wstag927_anim2_update, 0x58, 0);
}

void wstag927_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag927_anim2_create();
        if (gamestate_flags.get_flag(0x100C, 0)) {
            data->event = fieldstg_event_start(0x648);
        }
        if (gamestate_flags.get_flag(0x100C, 1)) {
            data->event = fieldstg_event_start(0x64A);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag927_start(void *arg0) {
    WstagObject *obj = object_new(wstag927_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag927_funcs.setup();
    return obj;
}

void wstag927_event_1608_end(void) {
    gamestate_flags.set_flag(0x100C, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag927_setup(void) {
    fieldstg_stage.background_file = 0x346;
    fieldstg_stage.sprite_file = 0x08ED0000;
    fieldstg_stage.sprites = wstag927_sprites;
    fieldstg_stage.mask_file = 0x8EC;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11D00, 0x12800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag927_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag927_actors;
    fieldstg_stage.battle_lists = &wstag927_battle_lists;
    fieldstg_stage.events = wstag927_events;
    fieldstg_attr.set_file(0, 0x08ED0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag927_setup(void);
extern s16 D_WSTAG927_800A6508[];
extern s16 D_WSTAG927_800A65FC[];

WstagAnimKey D_WSTAG927_800A6244[20] = {
    { 50, 18 }, { 44, 6 }, { 50, 60 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 51, 78 }, { 44, 6 }, { 45, 6 },
    { 46, 6 }, { 44, 6 }, { 52, 60 }, { 44, 6 }, { 52, 18 }, { 44, 6 }, { 45, 6 }, { 46, 6 }, { 44, 6 }, { 255, 0 },
};
WstagAnimKey D_WSTAG927_800A6294[20] = {
    { 56, 60 }, { 47, 6 }, { 56, 18 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 57, 78 }, { 47, 6 }, { 48, 6 },
    { 49, 6 }, { 47, 6 }, { 58, 18 }, { 47, 6 }, { 58, 60 }, { 47, 6 }, { 48, 6 }, { 49, 6 }, { 47, 6 }, { 255, 0 },
};
FieldstgVramPlace wstag927_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 440, 224, 184, 368, 500 },
};
FieldstgPlacedActor D_WSTAG927_800A6354 = { NULL, NULL, 104, 4, 0, 0, 1 };
FieldstgPlacedActor *wstag927_actors[2] = { &D_WSTAG927_800A6354, NULL };
FieldstgSprite wstag927_sprites[19] = {
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 350, 180, 0, 0 }, { 1, 0, 0x40, 2, 0x45, 2, 0, 5, 8, 0, 314, 180, 0, 0 },
    { 1, 2, 0x58, 2, 0x38, 0, 0, 0, 0, 0, 229, 79, 0, 0 }, { 1, 1, 0x58, 2, 0x32, 0, 0, 0, 0, 0, 395, 79, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 1, 0x3E, 0x40, 8, 0, 354, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x42, 1, 0x42, 0x44, 8, 0, 318, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x46, 2, 0, 3, 4, 0, 206, 186, 0, 0 }, { 1, 0, 0x40, 2, 0x46, 2, 0, 3, 4, 0, 370, 138, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 4, 0, 238, 170, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 4, 0, 402, 154, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 4, 0, 270, 154, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 4, 0, 434, 170, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 4, 0, 302, 138, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 4, 0, 466, 186, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 6, 0, 387, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 2, 0, 5, 6, 0, 395, 79, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 5, 6, 0, 229, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 5, 6, 0, 229, 79, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag927_funcs = { wstag927_setup };
FieldstgEventDef wstag927_events[3] = {
    { 1608, D_WSTAG927_800A6508, 0x01580004, NULL, wstag927_event_1608_end },
    { 1610, D_WSTAG927_800A65FC, 0x01580005, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG927_800A6508[121] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_PLACE(2, 192, 344),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(104, 384, 248),
    FIELDSTG_EVENT_ANIM(104, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(2, 320, 280, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG927_800A65FC[106] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_PLACE(2, 320, 280),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(104, 384, 248),
    FIELDSTG_EVENT_ANIM(104, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 104, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 192, 344, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x277, 303, 360, 5),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG927_800A66D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A66DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A66F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6700 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A670C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6718 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6724 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG927_800A6730 = {
    0,
    { &D_WSTAG927_800A66D0, &D_WSTAG927_800A66DC, &D_WSTAG927_800A66E8, &D_WSTAG927_800A66F4, &D_WSTAG927_800A6700,
        &D_WSTAG927_800A670C, &D_WSTAG927_800A6718, &D_WSTAG927_800A6724 },
};
FieldstgListedBattle D_WSTAG927_800A6754 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6760 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A676C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6778 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6784 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6790 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A679C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A67A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG927_800A67B4 = {
    0,
    { &D_WSTAG927_800A6754, &D_WSTAG927_800A6760, &D_WSTAG927_800A676C, &D_WSTAG927_800A6778, &D_WSTAG927_800A6784,
        &D_WSTAG927_800A6790, &D_WSTAG927_800A679C, &D_WSTAG927_800A67A8 },
};
FieldstgListedBattle D_WSTAG927_800A67D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A67E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A67FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6808 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6814 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6820 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A682C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG927_800A6838 = {
    0,
    { &D_WSTAG927_800A67D8, &D_WSTAG927_800A67E4, &D_WSTAG927_800A67F0, &D_WSTAG927_800A67FC, &D_WSTAG927_800A6808,
        &D_WSTAG927_800A6814, &D_WSTAG927_800A6820, &D_WSTAG927_800A682C },
};
FieldstgListedBattle D_WSTAG927_800A685C = { 262, 47, 0x60940000 };
FieldstgListedBattle D_WSTAG927_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6880 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A688C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A6898 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG927_800A68B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG927_800A68BC = {
    0,
    { &D_WSTAG927_800A685C, &D_WSTAG927_800A6868, &D_WSTAG927_800A6874, &D_WSTAG927_800A6880, &D_WSTAG927_800A688C,
        &D_WSTAG927_800A6898, &D_WSTAG927_800A68A4, &D_WSTAG927_800A68B0 },
};
FieldstgBattleLists wstag927_battle_lists = {
    392, 0, 0, { &D_WSTAG927_800A6730, &D_WSTAG927_800A67B4, &D_WSTAG927_800A6838 }, &D_WSTAG927_800A68BC,
};
