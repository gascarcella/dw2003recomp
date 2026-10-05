#include "wstag.h"

/* WSTAG346: stage 0x28F (fieldstg_stages). */

extern WstagFuncs wstag346_funcs;
extern FieldstgBattleLists wstag346_battle_lists;
extern FieldstgVramPlace wstag346_vram_places[];
extern FieldstgPlacedActor *wstag346_actors[];
extern FieldstgSprite wstag346_sprites[];
extern FieldstgMapEvent wstag346_map_events[];

void wstag346_update(WstagObject *obj) {
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

WstagObject *wstag346_start(void *arg0) {
    WstagObject *obj = object_new(wstag346_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag346_funcs.setup();
    return obj;
}

void wstag346_setup(void) {
    fieldstg_stage.background_file = 0x578;
    fieldstg_stage.sprite_file = 0x05790000;
    fieldstg_stage.sprites = wstag346_sprites;
    fieldstg_stage.map_events = wstag346_map_events;
    fieldstg_stage.mask_file = 0x577;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xCA00, 0x8200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag346_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag346_actors;
    fieldstg_stage.battle_lists = &wstag346_battle_lists;
    fieldstg_attr.set_file(0, 0x05790001);
    fieldstg_attr.set_file(7, 0x05790002);
    fieldstg_attr.set_file(4, 0x05790003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag346_setup(void);

FieldstgListedBattle D_WSTAG346_800A5FD4 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A5FE0 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A5FEC = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A5FF8 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6004 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6010 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A601C = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6028 = { 93, 13, 0x60080000 };
FieldstgBattleList D_WSTAG346_800A6034 = {
    3,
    { &D_WSTAG346_800A5FD4, &D_WSTAG346_800A5FE0, &D_WSTAG346_800A5FEC, &D_WSTAG346_800A5FF8, &D_WSTAG346_800A6004,
        &D_WSTAG346_800A6010, &D_WSTAG346_800A601C, &D_WSTAG346_800A6028 },
};
FieldstgListedBattle D_WSTAG346_800A6058 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6064 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6070 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A607C = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6088 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6094 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A60A0 = { 98, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A60AC = { 98, 4, 0x60080000 };
FieldstgBattleList D_WSTAG346_800A60B8 = {
    3,
    { &D_WSTAG346_800A6058, &D_WSTAG346_800A6064, &D_WSTAG346_800A6070, &D_WSTAG346_800A607C, &D_WSTAG346_800A6088,
        &D_WSTAG346_800A6094, &D_WSTAG346_800A60A0, &D_WSTAG346_800A60AC },
};
FieldstgListedBattle D_WSTAG346_800A60DC = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A60E8 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A60F4 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6100 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A610C = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6118 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6124 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6130 = { 179, 2, 0x60080000 };
FieldstgBattleList D_WSTAG346_800A613C = {
    3,
    { &D_WSTAG346_800A60DC, &D_WSTAG346_800A60E8, &D_WSTAG346_800A60F4, &D_WSTAG346_800A6100, &D_WSTAG346_800A610C,
        &D_WSTAG346_800A6118, &D_WSTAG346_800A6124, &D_WSTAG346_800A6130 },
};
FieldstgListedBattle D_WSTAG346_800A6160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG346_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG346_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG346_800A6184 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A6190 = { 330, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG346_800A61A8 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG346_800A61B4 = { 180, 2, 0x60080000 };
FieldstgBattleList D_WSTAG346_800A61C0 = {
    0,
    { &D_WSTAG346_800A6160, &D_WSTAG346_800A616C, &D_WSTAG346_800A6178, &D_WSTAG346_800A6184, &D_WSTAG346_800A6190,
        &D_WSTAG346_800A619C, &D_WSTAG346_800A61A8, &D_WSTAG346_800A61B4 },
};
FieldstgBattleLists wstag346_battle_lists = {
    95, 0, 0, { &D_WSTAG346_800A6034, &D_WSTAG346_800A60B8, &D_WSTAG346_800A613C }, &D_WSTAG346_800A61C0,
};
FieldstgVramPlace wstag346_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 371, 40, 115, 336, 511 }, { 320, 256, 370, 359, 200, 103, 352, 511 },
    { 320, 256, 346, 362, 104, 106, 368, 511 }, { 320, 256, 338, 371, 72, 115, 320, 510 },
    { 320, 256, 354, 386, 136, 130, 336, 510 }, { 320, 256, 362, 386, 168, 130, 352, 510 },
    { 320, 256, 320, 395, 0, 139, 368, 510 }, { 320, 256, 370, 399, 200, 143, 320, 509 },
};
FieldstgTalk D_WSTAG346_800A62E0[2] = { { NULL, NULL, 68 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A62F8[2] = { { NULL, NULL, 72 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6310[2] = { { NULL, NULL, 62 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6328[2] = { { NULL, NULL, 70 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6340[2] = { { NULL, NULL, 71 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6358[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6370[2] = { { NULL, NULL, 64 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A6388[2] = { { NULL, NULL, 66 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A63A0[2] = { { NULL, NULL, 67 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A63B8[2] = { { NULL, NULL, 69 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A63D0[2] = { { NULL, NULL, 61 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG346_800A63E8[2] = { { NULL, NULL, 63 }, { NULL, NULL, 0 } };
u16 D_WSTAG346_800A6400[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A640C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6414[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6420[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6428[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6430[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A643C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG346_800A6448[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6450[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A645C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6464[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG346_800A6470[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG346_800A6478 = { D_WSTAG346_800A6400, D_WSTAG346_800A62E0, 46, 4, 657, 648, 3 };
FieldstgPlacedActor D_WSTAG346_800A648C = { D_WSTAG346_800A640C, D_WSTAG346_800A62F8, 46, 4, 657, 648, 3 };
FieldstgPlacedActor D_WSTAG346_800A64A0 = { D_WSTAG346_800A6414, D_WSTAG346_800A6310, 51, 5, 316, 322, 1 };
FieldstgPlacedActor D_WSTAG346_800A64B4 = { D_WSTAG346_800A6420, D_WSTAG346_800A6328, 53, 6, 316, 322, 1 };
FieldstgPlacedActor D_WSTAG346_800A64C8 = { D_WSTAG346_800A6428, D_WSTAG346_800A6340, 57, 7, 677, 303, 1 };
FieldstgPlacedActor D_WSTAG346_800A64DC = { D_WSTAG346_800A6430, D_WSTAG346_800A6358, 58, 8, 641, 161, 3 };
FieldstgPlacedActor D_WSTAG346_800A64F0 = { D_WSTAG346_800A643C, D_WSTAG346_800A6370, 157, 9, 641, 161, 3 };
FieldstgPlacedActor D_WSTAG346_800A6504 = { D_WSTAG346_800A6448, D_WSTAG346_800A6388, 157, 9, 641, 161, 3 };
FieldstgPlacedActor D_WSTAG346_800A6518 = { D_WSTAG346_800A6450, D_WSTAG346_800A63A0, 158, 10, 657, 648, 3 };
FieldstgPlacedActor D_WSTAG346_800A652C = { D_WSTAG346_800A645C, D_WSTAG346_800A63B8, 158, 10, 657, 648, 3 };
FieldstgPlacedActor D_WSTAG346_800A6540 = { D_WSTAG346_800A6464, D_WSTAG346_800A63D0, 159, 11, 316, 322, 1 };
FieldstgPlacedActor D_WSTAG346_800A6554 = { D_WSTAG346_800A6470, D_WSTAG346_800A63E8, 159, 11, 316, 322, 1 };
FieldstgPlacedActor *wstag346_actors[13] = {
    &D_WSTAG346_800A6478, &D_WSTAG346_800A648C, &D_WSTAG346_800A64A0, &D_WSTAG346_800A64B4, &D_WSTAG346_800A64C8,
    &D_WSTAG346_800A64DC, &D_WSTAG346_800A64F0, &D_WSTAG346_800A6504, &D_WSTAG346_800A6518, &D_WSTAG346_800A652C,
    &D_WSTAG346_800A6540, &D_WSTAG346_800A6554, NULL,
};
FieldstgSprite wstag346_sprites[26] = {
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 328, 528, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 920, 952, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 149, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 343, 123, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 413, 862, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 91, 124, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 228, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 592, 982, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 674, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 865, 514, 0, 0 },
    { 1, 0, 0xFF, 6, 8, 0, 0, 0, 0, 0, 703, 232, 0, 0 }, { 1, 0, 0x80, 4, 9, 0, 0, 0, 0, 0, 631, 378, 433, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 593, 792, 804, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 623, 849, 846, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 333, 235, 235, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 381, 259, 259, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 429, 283, 283, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 481, 532, 532, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 486, 775, 775, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 557, 557, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 533, 751, 751, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 541, 699, 699, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 574, 580, 580, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1014, 879, 879, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1054, 899, 899, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag346_map_events[15] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x1D2, 0x92, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xC, 0x231, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xC, 0x221, 0x178, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0xD, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 9, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x1D, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFA2, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x28, 0x20, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x29, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0xD, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag346_funcs = { wstag346_setup };
