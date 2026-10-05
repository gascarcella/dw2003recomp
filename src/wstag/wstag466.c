#include "wstag.h"

/* WSTAG466: stage 0x2A4 (fieldstg_stages). */

extern WstagFuncs wstag466_funcs;
extern CVECTOR wstag466_color;
extern FieldstgBattleLists wstag466_battle_lists;
extern FieldstgVramPlace wstag466_vram_places[];
extern FieldstgPlacedActor *wstag466_actors[];
extern FieldstgSprite wstag466_sprites[];
extern FieldstgMapEvent wstag466_map_events[];

void wstag466_update(WstagObject *obj) {
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

WstagObject *wstag466_start(void *arg0) {
    WstagObject *obj = object_new(wstag466_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag466_funcs.setup();
    return obj;
}

void wstag466_setup(void) {
    fieldstg_stage.background_file = 0x5B1;
    fieldstg_stage.sprite_file = 0x05B20000;
    fieldstg_stage.sprites = wstag466_sprites;
    fieldstg_stage.map_events = wstag466_map_events;
    fieldstg_stage.mask_file = 0x5B0;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x3A700, 0x1D400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag466_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag466_actors;
    fieldstg_stage.color = wstag466_color;
    fieldstg_stage.battle_lists = &wstag466_battle_lists;
    fieldstg_attr.set_file(0, 0x05B20001);
    fieldstg_attr.set_file(7, 0x05B20002);
    fieldstg_attr.set_file(4, 0x05B20003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag466/nonmatchings/wstag466", wstag466_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag466_setup(void);

FieldstgListedBattle D_WSTAG466_800A5FB4 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FC0 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FCC = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FD8 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FE4 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FF0 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A5FFC = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A6008 = { 159, 8, 0x60080000 };
FieldstgBattleList D_WSTAG466_800A6014 = {
    3,
    { &D_WSTAG466_800A5FB4, &D_WSTAG466_800A5FC0, &D_WSTAG466_800A5FCC, &D_WSTAG466_800A5FD8, &D_WSTAG466_800A5FE4,
        &D_WSTAG466_800A5FF0, &D_WSTAG466_800A5FFC, &D_WSTAG466_800A6008 },
};
FieldstgListedBattle D_WSTAG466_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A608C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG466_800A6098 = {
    0,
    { &D_WSTAG466_800A6038, &D_WSTAG466_800A6044, &D_WSTAG466_800A6050, &D_WSTAG466_800A605C, &D_WSTAG466_800A6068,
        &D_WSTAG466_800A6074, &D_WSTAG466_800A6080, &D_WSTAG466_800A608C },
};
FieldstgListedBattle D_WSTAG466_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6110 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG466_800A611C = {
    0,
    { &D_WSTAG466_800A60BC, &D_WSTAG466_800A60C8, &D_WSTAG466_800A60D4, &D_WSTAG466_800A60E0, &D_WSTAG466_800A60EC,
        &D_WSTAG466_800A60F8, &D_WSTAG466_800A6104, &D_WSTAG466_800A6110 },
};
FieldstgListedBattle D_WSTAG466_800A6140 = { 231, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A614C = { 279, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG466_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6164 = { 331, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A6170 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG466_800A6188 = { 177, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG466_800A6194 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG466_800A61A0 = {
    0,
    { &D_WSTAG466_800A6140, &D_WSTAG466_800A614C, &D_WSTAG466_800A6158, &D_WSTAG466_800A6164, &D_WSTAG466_800A6170,
        &D_WSTAG466_800A617C, &D_WSTAG466_800A6188, &D_WSTAG466_800A6194 },
};
FieldstgBattleLists wstag466_battle_lists = {
    72, 0, 0, { &D_WSTAG466_800A6014, &D_WSTAG466_800A6098, &D_WSTAG466_800A611C }, &D_WSTAG466_800A61A0,
};
FieldstgVramPlace wstag466_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 329, 216, 73, 336, 511 }, { 320, 256, 374, 369, 216, 113, 368, 511 },
    { 320, 256, 374, 409, 216, 153, 320, 510 },
};
u16 D_WSTAG466_800A6270[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A6278[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A6280[6] = { 0, 1, 0x7205, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A628C[8] = { 0, 1, 0x7205, 1, 0x7208, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A629C[4] = { 0x762F, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A62A4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE22, 0,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A62B8[6] = { 0x7400, 1, 0xE22, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A62C4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE22, 1,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A62D8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A62E0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A62E8[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A62F4[8] = { 0, 1, 0x7209, 1, 0xE43, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A6304[6] = { 0x7400, 1, 0xE43, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A6310[10] = {
    0, 1, 0x7209, 1, 0xE43, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A6324[10] = {
    0, 1, 0x7209, 1, 0xE43, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A6338[4] = { 0x782F, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A6340[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A6348[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A6350[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A6358[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A6360[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG466_800A636C[2] = { { NULL, NULL, 234 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG466_800A6384[6] = {
    { D_WSTAG466_800A6270, D_WSTAG466_800A6278, 580 }, { D_WSTAG466_800A6280, NULL, 581 },
    { D_WSTAG466_800A628C, D_WSTAG466_800A629C, 582 }, { D_WSTAG466_800A62A4, D_WSTAG466_800A62B8, 583 },
    { D_WSTAG466_800A62C4, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG466_800A63CC[6] = {
    { D_WSTAG466_800A62D8, D_WSTAG466_800A62E0, 588 }, { D_WSTAG466_800A62E8, NULL, 590 },
    { D_WSTAG466_800A62F4, D_WSTAG466_800A6304, 589 }, { D_WSTAG466_800A6310, NULL, 584 },
    { D_WSTAG466_800A6324, D_WSTAG466_800A6338, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG466_800A6414[4] = {
    { D_WSTAG466_800A6340, NULL, 925 }, { D_WSTAG466_800A6348, D_WSTAG466_800A6350, 585 },
    { D_WSTAG466_800A6358, D_WSTAG466_800A6360, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG466_800A6444[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG466_800A645C[2] = { { NULL, NULL, 233 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG466_800A6474[2] = { { NULL, NULL, 235 }, { NULL, NULL, 0 } };
u16 D_WSTAG466_800A648C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A6498[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A64AC[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG466_800A64C0[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A64D0[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG466_800A64DC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG466_800A64E8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG466_800A64F0 = { D_WSTAG466_800A648C, D_WSTAG466_800A636C, 54, 4, 897, 514, 1 };
FieldstgPlacedActor D_WSTAG466_800A6504 = { D_WSTAG466_800A6498, D_WSTAG466_800A6384, 69, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG466_800A6518 = { D_WSTAG466_800A64AC, D_WSTAG466_800A63CC, 69, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG466_800A652C = { D_WSTAG466_800A64C0, D_WSTAG466_800A6414, 69, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG466_800A6540 = { D_WSTAG466_800A64D0, D_WSTAG466_800A6444, 69, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG466_800A6554 = { D_WSTAG466_800A64DC, D_WSTAG466_800A645C, 157, 6, 897, 514, 1 };
FieldstgPlacedActor D_WSTAG466_800A6568 = { D_WSTAG466_800A64E8, D_WSTAG466_800A6474, 157, 6, 897, 514, 1 };
FieldstgPlacedActor *wstag466_actors[8] = {
    &D_WSTAG466_800A64F0, &D_WSTAG466_800A6504, &D_WSTAG466_800A6518, &D_WSTAG466_800A652C, &D_WSTAG466_800A6540,
    &D_WSTAG466_800A6554, &D_WSTAG466_800A6568, NULL,
};
FieldstgSprite wstag466_sprites[37] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 1100, 594, 0, 0 }, { 1, 0, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 1107, 563, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 1187, 603, 0, 0 }, { 1, 0, 0x40, 2, 0x14, 0, 0, 0, 0, 0, 545, 152, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 273, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 799, 391, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1031, 584, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 101, 298, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 401, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 429, 522, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 534, 322, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 619, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 821, 283, 0, 0 },
    { 1, 0, 0x40, 6, 0x15, 0, 0, 0, 0, 0, 185, 274, 0, 0 }, { 1, 0, 0x40, 6, 0x16, 0, 0, 0, 0, 0, 233, 298, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 0, 0, 0, 0, 0, 281, 322, 0, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 1072, 384, 422, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 1104, 368, 412, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 1120, 368, 404, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 1136, 368, 396, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 1152, 352, 389, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 1168, 352, 381, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 400, 144, 191, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 384, 160, 197, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 446, 290, 318, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 766, 258, 287, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 486, 160, 189, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 1099, 491, 511, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 207, 286, 286, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 255, 310, 310, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 303, 334, 334, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 735, 581, 581, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 799, 581, 581, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 470, 470, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 446, 446, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 960, 422, 422, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag466_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A2, 0x340, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A6, 0xA8, 0x132, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A5, 0x1F2, 0x164, 3, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x38, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x48, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0xFFF8, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag466_funcs = { wstag466_setup };
