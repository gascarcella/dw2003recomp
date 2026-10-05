#include "wstag.h"

/* WSTAG341: stage 0x28E (fieldstg_stages). */

extern WstagFuncs wstag341_funcs;
extern FieldstgBattleLists wstag341_battle_lists;
extern FieldstgVramPlace wstag341_vram_places[];
extern FieldstgPlacedActor *wstag341_actors[];
extern FieldstgSprite wstag341_sprites[];
extern FieldstgMapEvent wstag341_map_events[];

void wstag341_update(WstagObject *obj) {
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

WstagObject *wstag341_start(void *arg0) {
    WstagObject *obj = object_new(wstag341_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag341_funcs.setup();
    return obj;
}

void wstag341_setup(void) {
    fieldstg_stage.background_file = 0x574;
    fieldstg_stage.sprite_file = 0x05750000;
    fieldstg_stage.sprites = wstag341_sprites;
    fieldstg_stage.map_events = wstag341_map_events;
    fieldstg_stage.mask_file = 0x573;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1F500, 0x10300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag341_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag341_actors;
    fieldstg_stage.battle_lists = &wstag341_battle_lists;
    fieldstg_attr.set_file(0, 0x05750001);
    fieldstg_attr.set_file(7, 0x05750002);
    fieldstg_attr.set_file(4, 0x05750003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag341_setup(void);

FieldstgListedBattle D_WSTAG341_800A5FDC = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A5FE8 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A5FF4 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6000 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A600C = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6018 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6024 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6030 = { 126, 1, 0x60080000 };
FieldstgBattleList D_WSTAG341_800A603C = {
    3,
    { &D_WSTAG341_800A5FDC, &D_WSTAG341_800A5FE8, &D_WSTAG341_800A5FF4, &D_WSTAG341_800A6000, &D_WSTAG341_800A600C,
        &D_WSTAG341_800A6018, &D_WSTAG341_800A6024, &D_WSTAG341_800A6030 },
};
FieldstgListedBattle D_WSTAG341_800A6060 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A606C = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6078 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6084 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6090 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A609C = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A60A8 = { 179, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A60B4 = { 179, 2, 0x60080000 };
FieldstgBattleList D_WSTAG341_800A60C0 = {
    3,
    { &D_WSTAG341_800A6060, &D_WSTAG341_800A606C, &D_WSTAG341_800A6078, &D_WSTAG341_800A6084, &D_WSTAG341_800A6090,
        &D_WSTAG341_800A609C, &D_WSTAG341_800A60A8, &D_WSTAG341_800A60B4 },
};
FieldstgListedBattle D_WSTAG341_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6138 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG341_800A6144 = {
    0,
    { &D_WSTAG341_800A60E4, &D_WSTAG341_800A60F0, &D_WSTAG341_800A60FC, &D_WSTAG341_800A6108, &D_WSTAG341_800A6114,
        &D_WSTAG341_800A6120, &D_WSTAG341_800A612C, &D_WSTAG341_800A6138 },
};
FieldstgListedBattle D_WSTAG341_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A618C = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A6198 = { 330, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG341_800A61B0 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG341_800A61BC = { 180, 2, 0x60080000 };
FieldstgBattleList D_WSTAG341_800A61C8 = {
    0,
    { &D_WSTAG341_800A6168, &D_WSTAG341_800A6174, &D_WSTAG341_800A6180, &D_WSTAG341_800A618C, &D_WSTAG341_800A6198,
        &D_WSTAG341_800A61A4, &D_WSTAG341_800A61B0, &D_WSTAG341_800A61BC },
};
FieldstgBattleLists wstag341_battle_lists = {
    94, 0, 0, { &D_WSTAG341_800A603C, &D_WSTAG341_800A60C0, &D_WSTAG341_800A6144 }, &D_WSTAG341_800A61C8,
};
FieldstgVramPlace wstag341_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 348, 415, 112, 159, 336, 511 }, { 320, 256, 356, 423, 144, 167, 352, 511 },
    { 320, 256, 364, 423, 176, 167, 368, 511 }, { 320, 256, 372, 423, 208, 167, 320, 510 },
    { 320, 256, 374, 337, 216, 81, 336, 510 }, { 384, 256, 394, 256, 296, 0, 352, 510 },
    { 384, 256, 402, 256, 328, 0, 368, 510 }, { 384, 256, 410, 256, 360, 0, 320, 509 },
    { 384, 256, 418, 256, 392, 0, 336, 509 }, { 320, 256, 348, 375, 112, 119, 352, 509 },
};
u16 D_WSTAG341_800A6308[4] = { 0x869A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6310[6] = { 0, 0, 0x869A, 0, 0xFFFF, 0 };
u16 D_WSTAG341_800A631C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6324[8] = { 0, 1, 0x869A, 0, 0x8496, 0, 0xFFFF, 0 };
u16 D_WSTAG341_800A6334[8] = { 0, 1, 0x869A, 0, 0x8496, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6344[10] = {
    0x869A, 1, 0x8699, 0, 0x8496, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG341_800A6358[2] = { { NULL, NULL, 56 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6370[2] = { { NULL, NULL, 58 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6388[2] = { { NULL, NULL, 60 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A63A0[2] = { { NULL, NULL, 59 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A63B8[2] = { { NULL, NULL, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A63D0[2] = { { NULL, NULL, 50 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A63E8[2] = { { NULL, NULL, 55 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6400[2] = { { NULL, NULL, 57 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6418[2] = { { NULL, NULL, 52 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6430[2] = { { NULL, NULL, 54 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6448[2] = { { NULL, NULL, 49 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6460[2] = { { NULL, NULL, 51 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG341_800A6478[5] = {
    { D_WSTAG341_800A6308, NULL, 750 }, { D_WSTAG341_800A6310, D_WSTAG341_800A631C, 751 },
    { D_WSTAG341_800A6324, NULL, 752 }, { D_WSTAG341_800A6334, D_WSTAG341_800A6344, 753 }, { NULL, NULL, 0 },
};
u16 D_WSTAG341_800A64B4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64C0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64C8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64D0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64D8[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64E4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A64F0[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG341_800A64FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6504[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6510[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A6518[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG341_800A6524[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG341_800A652C[10] = {
    0x7045, 1, 0x704D, 1, 0x8699, 1, 0x869A, 0,
    0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG341_800A6540 = { D_WSTAG341_800A64B4, D_WSTAG341_800A6358, 50, 4, 299, 282, 3 };
FieldstgPlacedActor D_WSTAG341_800A6554 = { D_WSTAG341_800A64C0, D_WSTAG341_800A6370, 52, 5, 299, 282, 3 };
FieldstgPlacedActor D_WSTAG341_800A6568 = { D_WSTAG341_800A64C8, D_WSTAG341_800A6388, 54, 6, 288, 424, 1 };
FieldstgPlacedActor D_WSTAG341_800A657C = { D_WSTAG341_800A64D0, D_WSTAG341_800A63A0, 56, 7, 481, 520, 1 };
FieldstgPlacedActor D_WSTAG341_800A6590 = { D_WSTAG341_800A64D8, D_WSTAG341_800A63B8, 57, 8, 288, 424, 7 };
FieldstgPlacedActor D_WSTAG341_800A65A4 = { D_WSTAG341_800A64E4, D_WSTAG341_800A63D0, 58, 9, 481, 520, 3 };
FieldstgPlacedActor D_WSTAG341_800A65B8 = { D_WSTAG341_800A64F0, D_WSTAG341_800A63E8, 157, 10, 299, 282, 3 };
FieldstgPlacedActor D_WSTAG341_800A65CC = { D_WSTAG341_800A64FC, D_WSTAG341_800A6400, 157, 10, 299, 282, 3 };
FieldstgPlacedActor D_WSTAG341_800A65E0 = { D_WSTAG341_800A6504, D_WSTAG341_800A6418, 158, 11, 288, 424, 7 };
FieldstgPlacedActor D_WSTAG341_800A65F4 = { D_WSTAG341_800A6510, D_WSTAG341_800A6430, 158, 11, 288, 424, 7 };
FieldstgPlacedActor D_WSTAG341_800A6608 = { D_WSTAG341_800A6518, D_WSTAG341_800A6448, 159, 12, 481, 520, 3 };
FieldstgPlacedActor D_WSTAG341_800A661C = { D_WSTAG341_800A6524, D_WSTAG341_800A6460, 159, 12, 481, 520, 3 };
FieldstgPlacedActor D_WSTAG341_800A6630 = { D_WSTAG341_800A652C, D_WSTAG341_800A6478, 169, 13, 640, 593, 3 };
FieldstgPlacedActor *wstag341_actors[14] = {
    &D_WSTAG341_800A6540, &D_WSTAG341_800A6554, &D_WSTAG341_800A6568, &D_WSTAG341_800A657C, &D_WSTAG341_800A6590,
    &D_WSTAG341_800A65A4, &D_WSTAG341_800A65B8, &D_WSTAG341_800A65CC, &D_WSTAG341_800A65E0, &D_WSTAG341_800A65F4,
    &D_WSTAG341_800A6608, &D_WSTAG341_800A661C, &D_WSTAG341_800A6630, NULL,
};
FieldstgSprite wstag341_sprites[25] = {
    { 1, 0, 0x50, 2, 5, 0, 0, 0, 0, 0, 392, 90, 0, 0 }, { 1, 0, 0x50, 2, 5, 0, 0, 0, 0, 0, 696, 287, 0, 0 },
    { 1, 0, 0x50, 2, 0xB, 0, 0, 0, 0, 0, 688, 97, 0, 0 }, { 1, 0, 0x72, 2, 0xC, 0, 0, 0, 0, 0, 784, 47, 0, 0 },
    { 1, 0, 0x80, 2, 0xD, 0, 0, 0, 0, 0, 640, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 98, 416, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 257, 640, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 642, 721, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 307, 524, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 457, 693, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 794, 50, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 496, 558, 575, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 509, 517, 543, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 514, 471, 489, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 276, 499, 511, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 676, 390, 407, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 255, 255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 390, 243, 243, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 247, 247, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 375, 375, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag341_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x102, 0x3AC, 5, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0xE, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x3C, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x40, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag341_funcs = { wstag341_setup };
