#include "wstag.h"

/* WSTAG203: stage 0x271 (fieldstg_stages). */

extern WstagFuncs wstag203_funcs;
extern CVECTOR wstag203_color;
extern FieldstgBattleLists wstag203_battle_lists;
extern FieldstgVramPlace wstag203_vram_places[];
extern FieldstgPlacedActor *wstag203_actors[];
extern FieldstgSprite wstag203_sprites[];
extern FieldstgMapEvent wstag203_map_events[];

void wstag203_update(WstagObject *obj) {
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

WstagObject *wstag203_start(void *arg0) {
    WstagObject *obj = object_new(wstag203_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag203_funcs.setup();
    return obj;
}

void wstag203_setup(void) {
    fieldstg_stage.background_file = 0x52A;
    fieldstg_stage.sprite_file = 0x052B0000;
    fieldstg_stage.sprites = wstag203_sprites;
    fieldstg_stage.map_events = wstag203_map_events;
    fieldstg_stage.mask_file = 0x529;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12C00, 0x12100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag203_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag203_actors;
    fieldstg_stage.color = wstag203_color;
    fieldstg_stage.battle_lists = &wstag203_battle_lists;
    fieldstg_attr.set_file(0, 0x052B0001);
    fieldstg_attr.set_file(7, 0x052B0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag203/nonmatchings/wstag203", wstag203_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag203_setup(void);

FieldstgListedBattle D_WSTAG203_800A5FE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A5FF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A5FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6038 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG203_800A6044 = {
    3,
    { &D_WSTAG203_800A5FE4, &D_WSTAG203_800A5FF0, &D_WSTAG203_800A5FFC, &D_WSTAG203_800A6008, &D_WSTAG203_800A6014,
        &D_WSTAG203_800A6020, &D_WSTAG203_800A602C, &D_WSTAG203_800A6038 },
};
FieldstgListedBattle D_WSTAG203_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A60BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG203_800A60C8 = {
    0,
    { &D_WSTAG203_800A6068, &D_WSTAG203_800A6074, &D_WSTAG203_800A6080, &D_WSTAG203_800A608C, &D_WSTAG203_800A6098,
        &D_WSTAG203_800A60A4, &D_WSTAG203_800A60B0, &D_WSTAG203_800A60BC },
};
FieldstgListedBattle D_WSTAG203_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6140 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG203_800A614C = {
    0,
    { &D_WSTAG203_800A60EC, &D_WSTAG203_800A60F8, &D_WSTAG203_800A6104, &D_WSTAG203_800A6110, &D_WSTAG203_800A611C,
        &D_WSTAG203_800A6128, &D_WSTAG203_800A6134, &D_WSTAG203_800A6140 },
};
FieldstgListedBattle D_WSTAG203_800A6170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A6194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A61A0 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG203_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A61B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG203_800A61C4 = { 178, 8, 0x60080000 };
FieldstgBattleList D_WSTAG203_800A61D0 = {
    0,
    { &D_WSTAG203_800A6170, &D_WSTAG203_800A617C, &D_WSTAG203_800A6188, &D_WSTAG203_800A6194, &D_WSTAG203_800A61A0,
        &D_WSTAG203_800A61AC, &D_WSTAG203_800A61B8, &D_WSTAG203_800A61C4 },
};
FieldstgBattleLists wstag203_battle_lists = {
    376, 0, 0, { &D_WSTAG203_800A6044, &D_WSTAG203_800A60C8, &D_WSTAG203_800A614C }, &D_WSTAG203_800A61D0,
};
FieldstgVramPlace wstag203_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 321, 256, 65, 368, 510 }, { 384, 256, 410, 357, 360, 101, 368, 509 },
    { 384, 256, 432, 256, 448, 0, 368, 508 }, { 384, 256, 392, 337, 288, 81, 320, 507 },
    { 384, 256, 430, 338, 440, 82, 336, 507 }, { 384, 256, 438, 338, 472, 82, 352, 507 },
    { 384, 256, 418, 355, 392, 99, 368, 507 }, { 384, 256, 436, 378, 464, 122, 320, 506 },
    { 384, 256, 394, 383, 296, 127, 336, 506 }, { 384, 256, 402, 383, 328, 127, 352, 506 },
    { 384, 256, 410, 389, 360, 133, 368, 506 }, { 384, 256, 418, 395, 392, 139, 320, 505 },
};
FieldstgTalk D_WSTAG203_800A6330[2] = { { NULL, NULL, 227 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6348[2] = { { NULL, NULL, 228 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6360[2] = { { NULL, NULL, 226 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6378[2] = { { NULL, NULL, 220 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6390[2] = { { NULL, NULL, 218 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A63A8[2] = { { NULL, NULL, 216 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A63C0[2] = { { NULL, NULL, 225 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A63D8[2] = { { NULL, NULL, 223 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A63F0[2] = { { NULL, NULL, 224 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6408[2] = { { NULL, NULL, 222 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6420[2] = { { NULL, NULL, 221 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6438[2] = { { NULL, NULL, 219 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6450[2] = { { NULL, NULL, 217 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6468[2] = { { NULL, NULL, 215 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6480[2] = { { NULL, NULL, 229 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG203_800A6498[2] = { { NULL, NULL, 230 }, { NULL, NULL, 0 } };
u16 D_WSTAG203_800A64B0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64B8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64C0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64C8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64D4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64DC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64E8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64F0[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A64FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A6504[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG203_800A6510[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A6518[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG203_800A6524[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A652C[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG203_800A6538[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG203_800A6540[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG203_800A6548 = { D_WSTAG203_800A64B0, D_WSTAG203_800A6330, 32, 4, 648, 396, 5 };
FieldstgPlacedActor D_WSTAG203_800A655C = { D_WSTAG203_800A64B8, D_WSTAG203_800A6348, 35, 5, 330, 398, 1 };
FieldstgPlacedActor D_WSTAG203_800A6570 = { D_WSTAG203_800A64C0, D_WSTAG203_800A6360, 37, 6, 680, 380, 1 };
FieldstgPlacedActor D_WSTAG203_800A6584 = { D_WSTAG203_800A64C8, D_WSTAG203_800A6378, 47, 7, 208, 344, 5 };
FieldstgPlacedActor D_WSTAG203_800A6598 = { D_WSTAG203_800A64D4, D_WSTAG203_800A6390, 51, 8, 460, 203, 5 };
FieldstgPlacedActor D_WSTAG203_800A65AC = { D_WSTAG203_800A64DC, D_WSTAG203_800A63A8, 51, 8, 460, 203, 5 };
FieldstgPlacedActor D_WSTAG203_800A65C0 = { D_WSTAG203_800A64E8, D_WSTAG203_800A63C0, 52, 9, 494, 186, 1 };
FieldstgPlacedActor D_WSTAG203_800A65D4 = { D_WSTAG203_800A64F0, D_WSTAG203_800A63D8, 53, 10, 513, 425, 7 };
FieldstgPlacedActor D_WSTAG203_800A65E8 = { D_WSTAG203_800A64FC, D_WSTAG203_800A63F0, 157, 11, 513, 425, 7 };
FieldstgPlacedActor D_WSTAG203_800A65FC = { D_WSTAG203_800A6504, D_WSTAG203_800A6408, 157, 11, 513, 425, 7 };
FieldstgPlacedActor D_WSTAG203_800A6610 = { D_WSTAG203_800A6510, D_WSTAG203_800A6420, 158, 12, 208, 344, 5 };
FieldstgPlacedActor D_WSTAG203_800A6624 = { D_WSTAG203_800A6518, D_WSTAG203_800A6438, 158, 12, 208, 344, 5 };
FieldstgPlacedActor D_WSTAG203_800A6638 = { D_WSTAG203_800A6524, D_WSTAG203_800A6450, 159, 13, 460, 203, 5 };
FieldstgPlacedActor D_WSTAG203_800A664C = { D_WSTAG203_800A652C, D_WSTAG203_800A6468, 159, 13, 460, 203, 5 };
FieldstgPlacedActor D_WSTAG203_800A6660 = { D_WSTAG203_800A6538, D_WSTAG203_800A6480, 228, 14, 513, 425, 7 };
FieldstgPlacedActor D_WSTAG203_800A6674 = { D_WSTAG203_800A6540, D_WSTAG203_800A6498, 229, 15, 672, 304, 5 };
FieldstgPlacedActor *wstag203_actors[17] = {
    &D_WSTAG203_800A6548, &D_WSTAG203_800A655C, &D_WSTAG203_800A6570, &D_WSTAG203_800A6584, &D_WSTAG203_800A6598,
    &D_WSTAG203_800A65AC, &D_WSTAG203_800A65C0, &D_WSTAG203_800A65D4, &D_WSTAG203_800A65E8, &D_WSTAG203_800A65FC,
    &D_WSTAG203_800A6610, &D_WSTAG203_800A6624, &D_WSTAG203_800A6638, &D_WSTAG203_800A664C, &D_WSTAG203_800A6660,
    &D_WSTAG203_800A6674, NULL,
};
FieldstgSprite wstag203_sprites[30] = {
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 174, 210, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 244, 175, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 840, 188, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 886, 211, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 6, 0, 341, 183, 0, 0 }, { 1, 0, 0x40, 2, 4, 0, 0, 0, 0, 0, 95, 116, 0, 0 },
    { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 168, 256, 0, 0 }, { 1, 0, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 846, 150, 0, 0 },
    { 1, 0, 0x58, 2, 0xF, 0, 0, 0, 0, 0, 426, 256, 0, 0 }, { 1, 0, 0x50, 2, 0x10, 0, 0, 0, 0, 0, 323, 384, 0, 0 },
    { 1, 0, 0x40, 2, 0x11, 0, 0, 0, 0, 0, 640, 256, 0, 0 }, { 1, 0, 0x60, 2, 0x12, 0, 0, 0, 0, 0, 548, 202, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 676, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 386, 160, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 498, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 155, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 668, 159, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 847, 321, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 185, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 408, 477, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 686, 248, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 745, 469, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 350, 152, 0, 0 },
    { 1, 0x65, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 463, 106, 0, 0 },
    { 1, 0x66, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 878, 199, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 224, 268, 289, 0 },
    { 1, 0, 0x41, 4, 1, 0, 0, 0, 0, 0, 336, 159, 216, 0 }, { 1, 0, 0x46, 4, 2, 0, 0, 0, 0, 0, 448, 103, 162, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 891, 200, 247, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag203_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x68, 0x134, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x282, 0x218, 0xF4, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x280, 0x216, 0xE2, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27F, 0x98, 0x14A, 5, 0x66, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x18, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x1C, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag203_funcs = { wstag203_setup };
