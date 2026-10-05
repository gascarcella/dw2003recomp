#include "wstag.h"

/* WSTAG301: stage 0x286 (fieldstg_stages). */

extern WstagFuncs wstag301_funcs;
extern FieldstgBattleLists wstag301_battle_lists;
extern FieldstgVramPlace wstag301_vram_places[];
extern FieldstgPlacedActor *wstag301_actors[];
extern FieldstgSprite wstag301_sprites[];
extern FieldstgMapEvent wstag301_map_events[];

void wstag301_update(WstagObject *obj) {
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

WstagObject *wstag301_start(void *arg0) {
    WstagObject *obj = object_new(wstag301_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag301_funcs.setup();
    return obj;
}

void wstag301_setup(void) {
    fieldstg_stage.background_file = 0x433;
    fieldstg_stage.sprites = wstag301_sprites;
    fieldstg_stage.map_events = wstag301_map_events;
    fieldstg_stage.sprite_file = 0x04340001;
    fieldstg_stage.mask_file = 0x432;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x49D00, 0x20300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag301_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag301_actors;
    fieldstg_stage.battle_lists = &wstag301_battle_lists;
    fieldstg_attr.set_file(0, 0x04340000);
    fieldstg_attr.set_file(7, 0x04340002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag301_setup(void);

FieldstgListedBattle D_WSTAG301_800A5FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A5FDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A5FE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A5FF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6018 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG301_800A6024 = {
    0,
    { &D_WSTAG301_800A5FC4, &D_WSTAG301_800A5FD0, &D_WSTAG301_800A5FDC, &D_WSTAG301_800A5FE8, &D_WSTAG301_800A5FF4,
        &D_WSTAG301_800A6000, &D_WSTAG301_800A600C, &D_WSTAG301_800A6018 },
};
FieldstgListedBattle D_WSTAG301_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A606C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6078 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A609C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG301_800A60A8 = {
    0,
    { &D_WSTAG301_800A6048, &D_WSTAG301_800A6054, &D_WSTAG301_800A6060, &D_WSTAG301_800A606C, &D_WSTAG301_800A6078,
        &D_WSTAG301_800A6084, &D_WSTAG301_800A6090, &D_WSTAG301_800A609C },
};
FieldstgListedBattle D_WSTAG301_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6120 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG301_800A612C = {
    0,
    { &D_WSTAG301_800A60CC, &D_WSTAG301_800A60D8, &D_WSTAG301_800A60E4, &D_WSTAG301_800A60F0, &D_WSTAG301_800A60FC,
        &D_WSTAG301_800A6108, &D_WSTAG301_800A6114, &D_WSTAG301_800A6120 },
};
FieldstgListedBattle D_WSTAG301_800A6150 = { 196, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG301_800A615C = { 197, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG301_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG301_800A61A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG301_800A61B0 = {
    0,
    { &D_WSTAG301_800A6150, &D_WSTAG301_800A615C, &D_WSTAG301_800A6168, &D_WSTAG301_800A6174, &D_WSTAG301_800A6180,
        &D_WSTAG301_800A618C, &D_WSTAG301_800A6198, &D_WSTAG301_800A61A4 },
};
FieldstgBattleLists wstag301_battle_lists = {
    147, 0, 0, { &D_WSTAG301_800A6024, &D_WSTAG301_800A60A8, &D_WSTAG301_800A612C }, &D_WSTAG301_800A61B0,
};
FieldstgVramPlace wstag301_vram_places[17] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 368, 505 }, { 320, 256, 370, 433, 200, 177, 368, 504 },
    { 384, 256, 432, 256, 448, 0, 368, 503 }, { 384, 256, 384, 383, 256, 127, 368, 502 },
    { 384, 256, 392, 383, 288, 127, 336, 501 }, { 384, 256, 432, 304, 448, 48, 352, 501 },
    { 384, 256, 432, 344, 448, 88, 368, 501 }, { 384, 256, 416, 359, 384, 103, 336, 500 },
    { 384, 256, 424, 359, 416, 103, 352, 500 }, { 384, 256, 400, 372, 320, 116, 368, 500 },
    { 384, 256, 408, 372, 352, 116, 336, 499 },
};
u16 D_WSTAG301_800A6300[4] = { 0x260, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6308[6] = { 0x7400, 1, 0xC2B, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6314[6] = { 0x7401, 1, 0xC30, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6320[6] = { 0x7401, 1, 0xC31, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A632C[6] = { 0x7400, 1, 0xC2C, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6338[6] = { 0x7400, 1, 0xC2D, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6344[6] = { 0x7400, 1, 0xC2E, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG301_800A6350[2] = { { NULL, D_WSTAG301_800A6300, 367 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6368[2] = { { NULL, NULL, 173 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6380[2] = { { NULL, NULL, 174 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6398[2] = { { NULL, NULL, 175 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A63B0[2] = { { NULL, NULL, 176 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A63C8[2] = { { NULL, D_WSTAG301_800A6308, 451 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A63E0[2] = { { NULL, D_WSTAG301_800A6314, 455 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A63F8[2] = { { NULL, D_WSTAG301_800A6320, 456 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6410[2] = { { NULL, D_WSTAG301_800A632C, 452 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6428[2] = { { NULL, D_WSTAG301_800A6338, 453 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG301_800A6440[2] = { { NULL, D_WSTAG301_800A6344, 454 }, { NULL, NULL, 0 } };
u16 D_WSTAG301_800A6458[4] = { 0x260, 0, 0xFFFF, 0 };
u16 D_WSTAG301_800A6460[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6468[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6470[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6478[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6480[6] = { 0x6025, 1, 0xC2B, 0, 0xFFFF, 0 };
u16 D_WSTAG301_800A648C[6] = { 0xC30, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A6498[6] = { 0xC31, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG301_800A64A4[6] = { 0x6025, 1, 0xC2C, 0, 0xFFFF, 0 };
u16 D_WSTAG301_800A64B0[6] = { 0x6025, 1, 0xC2D, 0, 0xFFFF, 0 };
u16 D_WSTAG301_800A64BC[6] = { 0x6025, 1, 0xC2E, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG301_800A64C8 = { D_WSTAG301_800A6458, D_WSTAG301_800A6350, 33, 4, 700, 283, 1 };
FieldstgPlacedActor D_WSTAG301_800A64DC = { D_WSTAG301_800A6460, D_WSTAG301_800A6368, 37, 5, 961, 217, 1 };
FieldstgPlacedActor D_WSTAG301_800A64F0 = { D_WSTAG301_800A6468, D_WSTAG301_800A6380, 38, 6, 896, 185, 1 };
FieldstgPlacedActor D_WSTAG301_800A6504 = { D_WSTAG301_800A6470, D_WSTAG301_800A6398, 157, 7, 961, 217, 1 };
FieldstgPlacedActor D_WSTAG301_800A6518 = { D_WSTAG301_800A6478, D_WSTAG301_800A63B0, 158, 8, 896, 185, 1 };
FieldstgPlacedActor D_WSTAG301_800A652C = { D_WSTAG301_800A6480, D_WSTAG301_800A63C8, 299, 9, 1120, 281, 1 };
FieldstgPlacedActor D_WSTAG301_800A6540 = { D_WSTAG301_800A648C, D_WSTAG301_800A63E0, 301, 10, 992, 282, 1 };
FieldstgPlacedActor D_WSTAG301_800A6554 = { D_WSTAG301_800A6498, D_WSTAG301_800A63F8, 302, 11, 769, 169, 1 };
FieldstgPlacedActor D_WSTAG301_800A6568 = { D_WSTAG301_800A64A4, D_WSTAG301_800A6410, 303, 12, 936, 477, 1 };
FieldstgPlacedActor D_WSTAG301_800A657C = { D_WSTAG301_800A64B0, D_WSTAG301_800A6428, 304, 13, 1225, 429, 3 };
FieldstgPlacedActor D_WSTAG301_800A6590 = { D_WSTAG301_800A64BC, D_WSTAG301_800A6440, 305, 14, 321, 201, 7 };
FieldstgPlacedActor *wstag301_actors[12] = {
    &D_WSTAG301_800A64C8, &D_WSTAG301_800A64DC, &D_WSTAG301_800A64F0, &D_WSTAG301_800A6504, &D_WSTAG301_800A6518,
    &D_WSTAG301_800A652C, &D_WSTAG301_800A6540, &D_WSTAG301_800A6554, &D_WSTAG301_800A6568, &D_WSTAG301_800A657C,
    &D_WSTAG301_800A6590, NULL,
};
FieldstgSprite wstag301_sprites[24] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 9, 8, 0, 918, 96, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 9, 0xA, 0, 918, 96, 0, 0 },
    { 1, 0, 0xFF, 2, 0xB, 0, 0, 0, 0, 0, 992, 126, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 8, 0, 166, 118, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 8, 0, 541, 90, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 8, 0, 1115, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 332, 103, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 1009, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 1211, 143, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 1294, 168, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 8, 0, 860, 89, 0, 0 },
    { 1, 0, 0x80, 6, 0x35, 3, 0, 0xF, 0xE, 0, 660, 434, 0, 0 },
    { 1, 0x64, 0x40, 6, 0, 0, 0, 0, 0, 0, 929, 148, 0, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 768, 107, 151, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 992, 218, 264, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 1088, 218, 264, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 1152, 186, 230, 0 }, { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 816, 333, 432, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 945, 441, 462, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 960, 190, 231, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 480, 321, 342, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 497, 256, 272, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 953, 144, 200, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag301_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x274, 0x158, 0xA4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x287, 0x56, 0x18A, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0xC8, 0x9C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x283, 0x177, 0xBD, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x274, 0x372, 0x132, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag301_funcs = { wstag301_setup };
