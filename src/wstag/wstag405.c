#include "wstag.h"

/* WSTAG405: stage 0x22B (fieldstg_stages). */

extern WstagFuncs wstag405_funcs;
void wstag405_update(WstagObject *obj, WstagEventData *data);
extern WstagAnimKey D_WSTAG405_800A63EC[];
extern FieldstgBattleLists wstag405_battle_lists;
extern FieldstgVramPlace wstag405_vram_places[];
extern FieldstgPlacedActor *wstag405_actors[];
extern FieldstgSprite wstag405_sprites[];
extern FieldstgMapEvent wstag405_map_events[];
extern FieldstgEventDef wstag405_events[];
void wstag405_anim1_update(WstagAnim1Object *obj);

s32 wstag405_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
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
        wstag405_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag405_anim1_update(WstagAnim1Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anim.key = 0;
        obj->anim.time = D_WSTAG405_800A63EC[0].time;
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->sprite = wstag405_anim_loop(&obj->anim, D_WSTAG405_800A63EC, 0);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag405_anim1_create(void) {
    return object_new(wstag405_anim1_update, 0x54, 0);
}

void wstag405_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 4 && gamestate_flags.get_flag(0x400F, 1) && gamestate_flags.get_flag(0x4010, 0)) {
            data->event = fieldstg_event_start(0x3D);
        }
        data->object = wstag405_anim1_create();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag405_start(void *arg0) {
    WstagObject *obj = object_new(wstag405_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag405_funcs.setup();
    return obj;
}

void wstag405_event_60_end(void) {
    gamestate_flags.set_flag(0x400F, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag405_event_61_end(void) {
    gamestate_flags.set_flag(0x4010, 1);
    gamestate_flags.set_flag(0x8699, 1);
}

void wstag405_setup(void) {
    fieldstg_stage.background_file = 0x256;
    fieldstg_stage.sprite_file = 0x02570000;
    fieldstg_stage.sprites = wstag405_sprites;
    fieldstg_stage.map_events = wstag405_map_events;
    fieldstg_stage.mask_file = 0x3D9;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x16B00, 0x44400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag405_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag405_actors;
    fieldstg_stage.events = wstag405_events;
    fieldstg_stage.battle_lists = &wstag405_battle_lists;
    fieldstg_attr.set_file(0, 0x02570001);
    fieldstg_attr.set_file(7, 0x02570002);
    fieldstg_attr.set_file(4, 0x02570003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag405_setup(void);

s16 D_WSTAG405_800A62A8[79] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 892, 339, 5),
    FIELDSTG_EVENT_PLACE(90, 924, 323),
    FIELDSTG_EVENT_ANIM(90, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 90, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 90, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG405_800A6348[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 892, 339),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(90, 924, 323),
    FIELDSTG_EVENT_ANIM(90, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 90, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 90, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 90, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG405_800A63EC[35] = {
    { 50, 8 }, { 51, 4 }, { 52, 8 }, { 53, 4 }, { 54, 8 }, { 55, 4 }, { 56, 8 }, { 57, 16 }, { 58, 4 }, { 59, 8 },
    { 60, 4 }, { 61, 8 }, { 62, 8 }, { 63, 12 }, { 64, 20 }, { 65, 4 }, { 66, 8 }, { 67, 4 }, { 68, 8 }, { 69, 8 },
    { 70, 8 }, { 71, 8 }, { 72, 8 }, { 73, 4 }, { 74, 8 }, { 75, 4 }, { 76, 8 }, { 77, 8 }, { 78, 8 }, { 79, 8 },
    { 80, 8 }, { 81, 12 }, { 82, 8 }, { 83, 30 }, { 255, 0 },
};
FieldstgListedBattle D_WSTAG405_800A6478 = { 50, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6484 = { 50, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6490 = { 50, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A649C = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A64A8 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A64B4 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A64C0 = { 52, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A64CC = { 52, 4, 0x60080000 };
FieldstgBattleList D_WSTAG405_800A64D8 = {
    3,
    { &D_WSTAG405_800A6478, &D_WSTAG405_800A6484, &D_WSTAG405_800A6490, &D_WSTAG405_800A649C, &D_WSTAG405_800A64A8,
        &D_WSTAG405_800A64B4, &D_WSTAG405_800A64C0, &D_WSTAG405_800A64CC },
};
FieldstgListedBattle D_WSTAG405_800A64FC = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6508 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6514 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6520 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A652C = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6538 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6544 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG405_800A6550 = { 51, 4, 0x60080000 };
FieldstgBattleList D_WSTAG405_800A655C = {
    5,
    { &D_WSTAG405_800A64FC, &D_WSTAG405_800A6508, &D_WSTAG405_800A6514, &D_WSTAG405_800A6520, &D_WSTAG405_800A652C,
        &D_WSTAG405_800A6538, &D_WSTAG405_800A6544, &D_WSTAG405_800A6550 },
};
FieldstgListedBattle D_WSTAG405_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A6598 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A65A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A65B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A65BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A65C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A65D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG405_800A65E0 = {
    0,
    { &D_WSTAG405_800A6580, &D_WSTAG405_800A658C, &D_WSTAG405_800A6598, &D_WSTAG405_800A65A4, &D_WSTAG405_800A65B0,
        &D_WSTAG405_800A65BC, &D_WSTAG405_800A65C8, &D_WSTAG405_800A65D4 },
};
FieldstgListedBattle D_WSTAG405_800A6604 = { 2, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG405_800A6610 = { 310, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG405_800A661C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A6628 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A6634 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A6640 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A664C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG405_800A6658 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG405_800A6664 = {
    0,
    { &D_WSTAG405_800A6604, &D_WSTAG405_800A6610, &D_WSTAG405_800A661C, &D_WSTAG405_800A6628, &D_WSTAG405_800A6634,
        &D_WSTAG405_800A6640, &D_WSTAG405_800A664C, &D_WSTAG405_800A6658 },
};
FieldstgBattleLists wstag405_battle_lists = {
    12, 0, 0, { &D_WSTAG405_800A64D8, &D_WSTAG405_800A655C, &D_WSTAG405_800A65E0 }, &D_WSTAG405_800A6664,
};
FieldstgVramPlace wstag405_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 436, 288, 464, 32, 368, 511 }, { 384, 256, 416, 288, 384, 32, 336, 510 },
    { 384, 256, 426, 328, 424, 72, 352, 510 },
};
u16 D_WSTAG405_800A6734[4] = { 0x1A2D, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A673C[4] = { 0x1A2D, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6744[6] = { 0x1A2D, 1, 0x702A, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A6750[8] = { 0x7025, 0, 0x1A2D, 1, 0x702A, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6760[10] = {
    0x7025, 1, 0x7027, 0, 0x1A2D, 1, 0x702A, 1,
    0xFFFF, 0,
};
u16 D_WSTAG405_800A6774[4] = { 0x600, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A677C[10] = {
    0x7025, 1, 0x7027, 1, 0x1A2D, 1, 0x702A, 1,
    0xFFFF, 0,
};
u16 D_WSTAG405_800A6790[4] = { 0x700B, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A6798[6] = { 0x7031, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A67A4[4] = { 0x700B, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A67AC[4] = { 0x1A2D, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A67B4[4] = { 0x1A2D, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A67BC[6] = { 0x1A2D, 1, 0x702A, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A67C8[8] = { 0x1A2D, 1, 0x702A, 1, 0x7025, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A67D8[10] = {
    0x1A2D, 1, 0x702A, 1, 0x7025, 1, 0x7027, 0,
    0xFFFF, 0,
};
u16 D_WSTAG405_800A67EC[4] = { 0x600, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A67F4[10] = {
    0x1A2D, 1, 0x702A, 1, 0x7025, 1, 0x7027, 1,
    0xFFFF, 0,
};
u16 D_WSTAG405_800A6808[4] = { 0x700B, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A6810[6] = { 0x7031, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A681C[4] = { 0x700B, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6824[4] = { 0x1C10, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A682C[6] = { 0x1C10, 1, 0x1C11, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A6838[8] = { 0x902F, 1, 0x1C11, 1, 0xA01, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6848[6] = { 0x1C10, 1, 0x1C11, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG405_800A6854[6] = {
    { D_WSTAG405_800A6734, D_WSTAG405_800A673C, 690 }, { D_WSTAG405_800A6744, NULL, 695 },
    { D_WSTAG405_800A6750, NULL, 696 }, { D_WSTAG405_800A6760, D_WSTAG405_800A6774, 691 },
    { D_WSTAG405_800A677C, NULL, 700 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG405_800A689C[3] = {
    { D_WSTAG405_800A6790, D_WSTAG405_800A6798, 692 }, { D_WSTAG405_800A67A4, NULL, 694 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG405_800A68C0[6] = {
    { D_WSTAG405_800A67AC, D_WSTAG405_800A67B4, 690 }, { D_WSTAG405_800A67BC, NULL, 695 },
    { D_WSTAG405_800A67C8, NULL, 696 }, { D_WSTAG405_800A67D8, D_WSTAG405_800A67EC, 691 },
    { D_WSTAG405_800A67F4, NULL, 700 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG405_800A6908[3] = {
    { D_WSTAG405_800A6808, D_WSTAG405_800A6810, 692 }, { D_WSTAG405_800A681C, NULL, 694 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG405_800A692C[4] = {
    { D_WSTAG405_800A6824, NULL, 241 }, { D_WSTAG405_800A682C, D_WSTAG405_800A6838, 733 },
    { D_WSTAG405_800A6848, NULL, 734 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG405_800A695C[2] = { { NULL, NULL, 745 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A6974[2] = { { NULL, NULL, 758 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A698C[2] = { { NULL, NULL, 756 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A69A4[2] = { { NULL, NULL, 751 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A69BC[2] = { { NULL, NULL, 753 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A69D4[2] = { { NULL, NULL, 754 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A69EC[2] = { { NULL, NULL, 759 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A6A04[2] = { { NULL, NULL, 757 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A6A1C[2] = { { NULL, NULL, 755 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A6A34[2] = { { NULL, NULL, 752 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG405_800A6A4C[2] = { { NULL, NULL, 693 }, { NULL, NULL, 0 } };
u16 D_WSTAG405_800A6A64[8] = { 0x6004, 0, 0x8006, 0, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6A74[8] = { 0x7022, 1, 0x6004, 0, 0x8006, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6A84[6] = { 0x602B, 1, 0x8006, 0, 0xFFFF, 0 };
u16 D_WSTAG405_800A6A90[6] = { 0x602B, 1, 0x8006, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6A9C[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AA4[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AAC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AB4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6ABC[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AC4[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6ACC[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AD4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6ADC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AE4[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AEC[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG405_800A6AF4[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG405_800A6AFC = { D_WSTAG405_800A6A64, D_WSTAG405_800A6854, 43, 4, 764, 1138, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B10 = { D_WSTAG405_800A6A74, D_WSTAG405_800A689C, 43, 4, 764, 1138, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B24 = { D_WSTAG405_800A6A84, D_WSTAG405_800A68C0, 43, 4, 764, 1138, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B38 = { D_WSTAG405_800A6A90, D_WSTAG405_800A6908, 43, 4, 764, 1138, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B4C = { D_WSTAG405_800A6A9C, D_WSTAG405_800A692C, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B60 = { D_WSTAG405_800A6AA4, D_WSTAG405_800A695C, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B74 = { D_WSTAG405_800A6AAC, D_WSTAG405_800A6974, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B88 = { D_WSTAG405_800A6AB4, D_WSTAG405_800A698C, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6B9C = { D_WSTAG405_800A6ABC, D_WSTAG405_800A69A4, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6BB0 = { D_WSTAG405_800A6AC4, D_WSTAG405_800A69BC, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6BC4 = { D_WSTAG405_800A6ACC, D_WSTAG405_800A69D4, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6BD8 = { D_WSTAG405_800A6AD4, D_WSTAG405_800A69EC, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6BEC = { D_WSTAG405_800A6ADC, D_WSTAG405_800A6A04, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6C00 = { D_WSTAG405_800A6AE4, D_WSTAG405_800A6A1C, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6C14 = { D_WSTAG405_800A6AEC, D_WSTAG405_800A6A34, 90, 5, 924, 323, 1 };
FieldstgPlacedActor D_WSTAG405_800A6C28 = { D_WSTAG405_800A6AF4, D_WSTAG405_800A6A4C, 157, 6, 764, 1138, 1 };
FieldstgPlacedActor *wstag405_actors[17] = {
    &D_WSTAG405_800A6AFC, &D_WSTAG405_800A6B10, &D_WSTAG405_800A6B24, &D_WSTAG405_800A6B38, &D_WSTAG405_800A6B4C,
    &D_WSTAG405_800A6B60, &D_WSTAG405_800A6B74, &D_WSTAG405_800A6B88, &D_WSTAG405_800A6B9C, &D_WSTAG405_800A6BB0,
    &D_WSTAG405_800A6BC4, &D_WSTAG405_800A6BD8, &D_WSTAG405_800A6BEC, &D_WSTAG405_800A6C00, &D_WSTAG405_800A6C14,
    &D_WSTAG405_800A6C28, NULL,
};
FieldstgSprite wstag405_sprites[7] = {
    { 1, 1, 0xE6, 2, 0x32, 0, 0, 0, 0, 0, 966, 364, 0, 0 }, { 1, 0, 0x40, 2, 0x54, 2, 0, 2, 6, 0, 931, 293, 0, 0 },
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 179, 400, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 602, 1071, 0, 0 },
    { 1, 0, 0xC8, 6, 7, 0, 0, 0, 0, 0, 745, 598, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 298, 442, 505, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag405_map_events[13] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x22A, 0x648, 0xD0, 1, 0, 0, 0 },
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
WstagFuncs wstag405_funcs = { wstag405_setup };
FieldstgEventDef wstag405_events[3] = {
    { 60, D_WSTAG405_800A62A8, 0x01350000, NULL, wstag405_event_60_end },
    { 61, D_WSTAG405_800A6348, 0x01350001, NULL, wstag405_event_61_end }, { -1, NULL, 0, NULL, NULL },
};
