#include "wstag.h"

/* WSTAG490: stage 0x23C (fieldstg_stages). */

extern WstagFuncs wstag490_funcs;
const CVECTOR wstag490_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag490_battle_lists;
extern FieldstgVramPlace wstag490_vram_places[];
extern FieldstgPlacedActor *wstag490_actors[];
extern FieldstgSprite wstag490_sprites[];
extern FieldstgMapEvent wstag490_map_events[];
extern FieldstgEventDef wstag490_events[];
void wstag490_update();

void wstag490_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_flags.get_flag(0x4025, 1) && gamestate_flags.get_flag(0x4026, 0)) {
            data->event = fieldstg_event_start(0x4F4);
        } else if (gamestate_flags.get_flag(0x402F, 1) && gamestate_flags.get_flag(0x4030, 0)) {
            data->event = fieldstg_event_start(0x4FE);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag490_start(void *arg0) {
    WstagObject *obj = object_new(wstag490_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag490_funcs.setup();
    return obj;
}

void wstag490_event_1267_end(void) {
    gamestate_flags.set_flag(0x4025, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag490_event_1268_end(void) {
    gamestate_flags.set_flag(0x4026, 1);
    gamestate_flags.set_flag(0x8022, 1);
}

void wstag490_event_1277_end(void) {
    gamestate_flags.set_flag(0x402F, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag490_event_1278_end(void) {
    gamestate_flags.set_flag(0x4030, 1);
    gamestate_flags.set_flag(0x818B, 1);
}

void wstag490_setup(void) {
    fieldstg_stage.background_file = 0x3FE;
    fieldstg_stage.sprite_file = 0x03FF0000;
    fieldstg_stage.sprites = wstag490_sprites;
    fieldstg_stage.map_events = wstag490_map_events;
    fieldstg_stage.mask_file = 0x3FD;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x33900, 0x3D200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag490_vram_places;
    fieldstg_stage.music = 0x35;
    fieldstg_stage.sound = 0x60D40000;
    fieldstg_stage.actors = wstag490_actors;
    fieldstg_stage.color = wstag490_color;
    fieldstg_stage.events = wstag490_events;
    fieldstg_stage.battle_lists = &wstag490_battle_lists;
    fieldstg_attr.set_file(0, 0x03FF0001);
    fieldstg_attr.set_file(7, 0x03FF0002);
    fieldstg_attr.set_file(4, 0x03FF0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag490_setup(void);

s16 D_WSTAG490_800A618C[55] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 997, 391, 5),
    FIELDSTG_EVENT_PLACE(126, 1029, 375),
    FIELDSTG_EVENT_ANIM(126, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 126, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG490_800A61FC[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 997, 391),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(126, 1029, 375),
    FIELDSTG_EVENT_ANIM(126, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 126, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG490_800A6280[55] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 312, 724, 3),
    FIELDSTG_EVENT_PLACE(129, 280, 708),
    FIELDSTG_EVENT_ANIM(129, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 129, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG490_800A62F0[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 312, 724),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(129, 280, 708),
    FIELDSTG_EVENT_ANIM(129, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 129, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG490_800A6374 = { 152, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A6380 = { 152, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A638C = { 152, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A6398 = { 152, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A63A4 = { 145, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A63B0 = { 145, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A63BC = { 58, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A63C8 = { 58, 8, 0x60080000 };
FieldstgBattleList D_WSTAG490_800A63D4 = {
    3,
    { &D_WSTAG490_800A6374, &D_WSTAG490_800A6380, &D_WSTAG490_800A638C, &D_WSTAG490_800A6398, &D_WSTAG490_800A63A4,
        &D_WSTAG490_800A63B0, &D_WSTAG490_800A63BC, &D_WSTAG490_800A63C8 },
};
FieldstgListedBattle D_WSTAG490_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6410 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A644C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG490_800A6458 = {
    0,
    { &D_WSTAG490_800A63F8, &D_WSTAG490_800A6404, &D_WSTAG490_800A6410, &D_WSTAG490_800A641C, &D_WSTAG490_800A6428,
        &D_WSTAG490_800A6434, &D_WSTAG490_800A6440, &D_WSTAG490_800A644C },
};
FieldstgListedBattle D_WSTAG490_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6494 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A64A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A64AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A64D0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG490_800A64DC = {
    0,
    { &D_WSTAG490_800A647C, &D_WSTAG490_800A6488, &D_WSTAG490_800A6494, &D_WSTAG490_800A64A0, &D_WSTAG490_800A64AC,
        &D_WSTAG490_800A64B8, &D_WSTAG490_800A64C4, &D_WSTAG490_800A64D0 },
};
FieldstgListedBattle D_WSTAG490_800A6500 = { 265, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG490_800A650C = { 269, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG490_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6524 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A6530 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG490_800A6548 = { 152, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG490_800A6554 = { 59, 8, 0x60080000 };
FieldstgBattleList D_WSTAG490_800A6560 = {
    0,
    { &D_WSTAG490_800A6500, &D_WSTAG490_800A650C, &D_WSTAG490_800A6518, &D_WSTAG490_800A6524, &D_WSTAG490_800A6530,
        &D_WSTAG490_800A653C, &D_WSTAG490_800A6548, &D_WSTAG490_800A6554 },
};
FieldstgBattleLists wstag490_battle_lists = {
    20, 0, 0, { &D_WSTAG490_800A63D4, &D_WSTAG490_800A6458, &D_WSTAG490_800A64DC }, &D_WSTAG490_800A6560,
};
FieldstgVramPlace wstag490_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 502, 344, 728, 88, 352, 510 }, { 448, 256, 496, 344, 704, 88, 368, 510 },
    { 384, 256, 414, 307, 376, 51, 336, 509 }, { 384, 256, 432, 460, 448, 204, 352, 509 },
    { 448, 256, 456, 346, 544, 90, 368, 509 },
};
u16 D_WSTAG490_800A6650[4] = { 0x1C13, 0, 0xFFFF, 0 };
u16 D_WSTAG490_800A6658[6] = { 0x9026, 1, 0x1C13, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6664[4] = { 0x1C13, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A666C[4] = { 0x1C18, 0, 0xFFFF, 0 };
u16 D_WSTAG490_800A6674[6] = { 0x902B, 1, 0x1C18, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6680[4] = { 0x1C18, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG490_800A6688[2] = { { NULL, NULL, 34 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A66A0[2] = { { NULL, NULL, 324 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A66B8[2] = { { NULL, NULL, 327 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A66D0[2] = { { NULL, NULL, 325 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A66E8[2] = { { NULL, NULL, 323 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A6700[2] = { { NULL, NULL, 821 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG490_800A6718[3] = {
    { D_WSTAG490_800A6650, D_WSTAG490_800A6658, 703 }, { D_WSTAG490_800A6664, NULL, 704 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG490_800A673C[3] = {
    { D_WSTAG490_800A666C, D_WSTAG490_800A6674, 713 }, { D_WSTAG490_800A6680, NULL, 714 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG490_800A6760[2] = { { NULL, NULL, 326 }, { NULL, NULL, 0 } };
u16 D_WSTAG490_800A6778[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6780[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6788[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6790[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A6798[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A67A0[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG490_800A67A8[6] = { 0x601, 1, 0x8022, 0, 0xFFFF, 0 };
u16 D_WSTAG490_800A67B4[6] = { 0x605, 1, 0x818B, 0, 0xFFFF, 0 };
u16 D_WSTAG490_800A67C0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG490_800A67C8 = { D_WSTAG490_800A6778, D_WSTAG490_800A6688, 46, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG490_800A67DC = { D_WSTAG490_800A6780, D_WSTAG490_800A66A0, 46, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG490_800A67F0 = { D_WSTAG490_800A6788, D_WSTAG490_800A66B8, 46, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG490_800A6804 = { D_WSTAG490_800A6790, D_WSTAG490_800A66D0, 46, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG490_800A6818 = { D_WSTAG490_800A6798, D_WSTAG490_800A66E8, 46, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG490_800A682C = { D_WSTAG490_800A67A0, D_WSTAG490_800A6700, 102, 5, 776, 996, 1 };
FieldstgPlacedActor D_WSTAG490_800A6840 = { D_WSTAG490_800A67A8, D_WSTAG490_800A6718, 126, 6, 1029, 375, 1 };
FieldstgPlacedActor D_WSTAG490_800A6854 = { D_WSTAG490_800A67B4, D_WSTAG490_800A673C, 129, 7, 280, 708, 7 };
FieldstgPlacedActor D_WSTAG490_800A6868 = { D_WSTAG490_800A67C0, D_WSTAG490_800A6760, 157, 8, 561, 458, 7 };
FieldstgPlacedActor *wstag490_actors[10] = {
    &D_WSTAG490_800A67C8, &D_WSTAG490_800A67DC, &D_WSTAG490_800A67F0, &D_WSTAG490_800A6804, &D_WSTAG490_800A6818,
    &D_WSTAG490_800A682C, &D_WSTAG490_800A6840, &D_WSTAG490_800A6854, &D_WSTAG490_800A6868, NULL,
};
FieldstgSprite wstag490_sprites[79] = {
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 330, 130, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 555, 650, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 1017, 793, 0, 0 },
    { 1, 0, 0x70, 2, 0xA, 0, 0, 0, 0, 0, 896, 548, 0, 0 }, { 1, 0, 0x78, 2, 0xB, 0, 0, 0, 0, 0, 768, 265, 0, 0 },
    { 1, 0, 0x40, 6, 0x62, 2, 0, 0xD, 0xA, 0, 902, 118, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 324, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 480, 1007, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 1091, 911, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 259, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 452, 984, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 1034, 904, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 70, 825, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 200, 1009, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 200, 1054, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 562, 1103, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 683, 1086, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 260, 993, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 155, 855, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 281, 1130, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 30, 836, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 220, 983, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 301, 1071, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 438, 1073, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 530, 1107, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 651, 1054, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 743, 1116, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 842, 1095, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 29, 789, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 148, 1013, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 162, 889, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 471, 1092, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 701, 1043, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 802, 1118, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 109, 831, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 166, 952, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 241, 980, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 336, 1112, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 377, 1083, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 502, 1067, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 588, 1055, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 599, 1017, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 659, 1036, 0, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 103, 696, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 287, 790, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 319, 634, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 331, 860, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 425, 935, 1152, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 566, 586, 673, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 455, 303, 335, 0 },
    { 1, 0, 0x50, 4, 1, 0, 0, 0, 0, 0, 1013, 384, 452, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 959, 427, 502, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 310, 860, 900, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 455, 303, 335, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 448, 513, 569, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 592, 744, 782, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 992, 151, 200, 0 },
    { 1, 0, 0x64, 4, 0x12, 0, 0, 0, 0, 0, 445, 977, 1021, 0 },
    { 1, 0, 0x64, 4, 0x13, 0, 0, 0, 0, 0, 896, 756, 810, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 80, 759, 759, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 191, 191, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 223, 223, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 247, 247, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 368, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 447, 647, 647, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 439, 439, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 615, 615, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 512, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 613, 879, 879, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 624, 839, 839, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 447, 447, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 487, 487, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 887, 887, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 704, 543, 543, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 935, 935, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 871, 871, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 823, 823, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag490_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23D, 0x372, 0x272, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x242, 0x90, 0x128, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23B, 0xA0, 0x80, 7, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 4, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 1, 2 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 4, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x48, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x20, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 4, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag490_funcs = { wstag490_setup };
FieldstgEventDef wstag490_events[5] = {
    { 1267, D_WSTAG490_800A618C, 0x01350015, NULL, wstag490_event_1267_end },
    { 1268, D_WSTAG490_800A61FC, 0x01350016, NULL, wstag490_event_1268_end },
    { 1277, D_WSTAG490_800A6280, 0x01350017, NULL, wstag490_event_1277_end },
    { 1278, D_WSTAG490_800A62F0, 0x01350018, NULL, wstag490_event_1278_end }, { -1, NULL, 0, NULL, NULL },
};
