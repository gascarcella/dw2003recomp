#include "wstag.h"

/* WSTAG726: stage 0x2D0 (fieldstg_stages). */

extern WstagFuncs wstag726_funcs;
extern FieldstgBattleLists wstag726_battle_lists;
extern FieldstgVramPlace wstag726_vram_places[];
extern FieldstgPlacedActor *wstag726_actors[];
extern FieldstgSprite wstag726_sprites[];
extern FieldstgMapEvent wstag726_map_events[];

void wstag726_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag726_start(void *arg0) {
    WstagObject *obj = object_new(wstag726_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag726_funcs.setup();
    return obj;
}

void wstag726_setup(void) {
    fieldstg_stage.background_file = 0x687;
    fieldstg_stage.sprite_file = 0x06880000;
    fieldstg_stage.sprites = wstag726_sprites;
    fieldstg_stage.map_events = wstag726_map_events;
    fieldstg_stage.mask_file = 0x686;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x37700, 0x10700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag726_vram_places;
    fieldstg_stage.music = 0x3F;
    fieldstg_stage.sound = 0x60FC0000;
    fieldstg_stage.actors = wstag726_actors;
    fieldstg_stage.battle_lists = &wstag726_battle_lists;
    fieldstg_attr.set_file(0, 0x06880001);
    fieldstg_attr.set_file(7, 0x06880002);
    fieldstg_attr.set_file(4, 0x06880003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag726_setup(void);

FieldstgListedBattle D_WSTAG726_800A5F94 = { 163, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FA0 = { 163, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FAC = { 163, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FB8 = { 163, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FC4 = { 137, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FD0 = { 137, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FDC = { 137, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG726_800A5FE8 = { 137, 7, 0x60080000 };
FieldstgBattleList D_WSTAG726_800A5FF4 = {
    3,
    { &D_WSTAG726_800A5F94, &D_WSTAG726_800A5FA0, &D_WSTAG726_800A5FAC, &D_WSTAG726_800A5FB8, &D_WSTAG726_800A5FC4,
        &D_WSTAG726_800A5FD0, &D_WSTAG726_800A5FDC, &D_WSTAG726_800A5FE8 },
};
FieldstgListedBattle D_WSTAG726_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG726_800A6078 = {
    0,
    { &D_WSTAG726_800A6018, &D_WSTAG726_800A6024, &D_WSTAG726_800A6030, &D_WSTAG726_800A603C, &D_WSTAG726_800A6048,
        &D_WSTAG726_800A6054, &D_WSTAG726_800A6060, &D_WSTAG726_800A606C },
};
FieldstgListedBattle D_WSTAG726_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG726_800A60FC = {
    0,
    { &D_WSTAG726_800A609C, &D_WSTAG726_800A60A8, &D_WSTAG726_800A60B4, &D_WSTAG726_800A60C0, &D_WSTAG726_800A60CC,
        &D_WSTAG726_800A60D8, &D_WSTAG726_800A60E4, &D_WSTAG726_800A60F0 },
};
FieldstgListedBattle D_WSTAG726_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG726_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG726_800A6180 = {
    0,
    { &D_WSTAG726_800A6120, &D_WSTAG726_800A612C, &D_WSTAG726_800A6138, &D_WSTAG726_800A6144, &D_WSTAG726_800A6150,
        &D_WSTAG726_800A615C, &D_WSTAG726_800A6168, &D_WSTAG726_800A6174 },
};
FieldstgBattleLists wstag726_battle_lists = {
    119, 0, 0, { &D_WSTAG726_800A5FF4, &D_WSTAG726_800A6078, &D_WSTAG726_800A60FC }, &D_WSTAG726_800A6180,
};
FieldstgVramPlace wstag726_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 288, 192, 32, 352, 511 }, { 320, 256, 354, 322, 136, 66, 368, 511 },
    { 320, 256, 362, 328, 168, 72, 352, 510 }, { 320, 256, 370, 328, 200, 72, 368, 510 },
    { 320, 256, 354, 362, 136, 106, 352, 509 }, { 320, 256, 362, 368, 168, 112, 368, 509 },
    { 320, 256, 370, 368, 200, 112, 352, 508 }, { 320, 256, 362, 400, 168, 144, 368, 508 },
    { 320, 256, 370, 400, 200, 144, 336, 507 }, { 320, 256, 352, 402, 128, 146, 352, 507 },
};
u16 D_WSTAG726_800A62C0[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A62C8[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A62D0[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG726_800A62D8[4] = {
    { D_WSTAG726_800A62C0, NULL, 499 }, { D_WSTAG726_800A62C8, NULL, 500 }, { D_WSTAG726_800A62D0, NULL, 501 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG726_800A6308[2] = { { NULL, NULL, 505 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6320[2] = { { NULL, NULL, 507 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6338[2] = { { NULL, NULL, 503 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6350[2] = { { NULL, NULL, 510 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6368[2] = { { NULL, NULL, 509 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6380[2] = { { NULL, NULL, 504 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6398[2] = { { NULL, NULL, 504 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A63B0[2] = { { NULL, NULL, 508 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A63C8[2] = { { NULL, NULL, 508 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A63E0[2] = { { NULL, NULL, 506 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A63F8[2] = { { NULL, NULL, 506 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG726_800A6410[2] = { { NULL, NULL, 502 }, { NULL, NULL, 0 } };
u16 D_WSTAG726_800A6428[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A6430[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A643C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A6448[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A6454[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A645C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A6464[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG726_800A6470[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A6478[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG726_800A6484[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A648C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG726_800A6498[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG726_800A64A0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG726_800A64A8 = { D_WSTAG726_800A6428, D_WSTAG726_800A62D8, 48, 4, 692, 265, 5 };
FieldstgPlacedActor D_WSTAG726_800A64BC = { D_WSTAG726_800A6430, D_WSTAG726_800A6308, 49, 5, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG726_800A64D0 = { D_WSTAG726_800A643C, D_WSTAG726_800A6320, 50, 6, 737, 324, 7 };
FieldstgPlacedActor D_WSTAG726_800A64E4 = { D_WSTAG726_800A6448, D_WSTAG726_800A6338, 52, 7, 423, 97, 7 };
FieldstgPlacedActor D_WSTAG726_800A64F8 = { D_WSTAG726_800A6454, D_WSTAG726_800A6350, 55, 8, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG726_800A650C = { D_WSTAG726_800A645C, D_WSTAG726_800A6368, 57, 9, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG726_800A6520 = { D_WSTAG726_800A6464, D_WSTAG726_800A6380, 157, 10, 423, 97, 7 };
FieldstgPlacedActor D_WSTAG726_800A6534 = { D_WSTAG726_800A6470, D_WSTAG726_800A6398, 157, 10, 423, 97, 7 };
FieldstgPlacedActor D_WSTAG726_800A6548 = { D_WSTAG726_800A6478, D_WSTAG726_800A63B0, 158, 11, 737, 324, 7 };
FieldstgPlacedActor D_WSTAG726_800A655C = { D_WSTAG726_800A6484, D_WSTAG726_800A63C8, 158, 11, 737, 324, 7 };
FieldstgPlacedActor D_WSTAG726_800A6570 = { D_WSTAG726_800A648C, D_WSTAG726_800A63E0, 159, 12, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG726_800A6584 = { D_WSTAG726_800A6498, D_WSTAG726_800A63F8, 159, 12, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG726_800A6598 = { D_WSTAG726_800A64A0, D_WSTAG726_800A6410, 160, 13, 692, 265, 5 };
FieldstgPlacedActor *wstag726_actors[14] = {
    &D_WSTAG726_800A64A8, &D_WSTAG726_800A64BC, &D_WSTAG726_800A64D0, &D_WSTAG726_800A64E4, &D_WSTAG726_800A64F8,
    &D_WSTAG726_800A650C, &D_WSTAG726_800A6520, &D_WSTAG726_800A6534, &D_WSTAG726_800A6548, &D_WSTAG726_800A655C,
    &D_WSTAG726_800A6570, &D_WSTAG726_800A6584, &D_WSTAG726_800A6598, NULL,
};
FieldstgSprite wstag726_sprites[29] = {
    { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 807, 124, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 647, 322, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 714, 356, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 755, 312, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 773, 350, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 830, 322, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 831, 297, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 289, 211, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 317, 214, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 4, 0, 762, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 287, 282, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 482, 16, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 566, 355, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 630, 94, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 243, 466, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 435, 24, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 467, 302, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 909, 158, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 330, 362, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 339, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 658, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 3, 0, 9, 8, 0, 105, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 3, 0, 9, 8, 0, 143, 271, 0, 0 }, { 1, 0, 0x40, 4, 0x38, 3, 0, 9, 8, 0, 77, 305, 328, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 635, 206, 246, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 464, 183, 220, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 61, 264, 328, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 800, 104, 175, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag726_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D2, 0x508, 0x54, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D1, 0xA8, 0xF4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CD, 0x220, 0x2C0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x200, 0x60, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x210, 0xC8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x220, 0x100, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x210, 0x198, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag726_funcs = { wstag726_setup };
