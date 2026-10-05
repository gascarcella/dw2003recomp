#include "wstag.h"

/* WSTAG451: stage 0x2A2 (fieldstg_stages). */

extern WstagFuncs wstag451_funcs;
extern CVECTOR wstag451_color;
extern FieldstgBattleLists wstag451_battle_lists;
extern FieldstgVramPlace wstag451_vram_places[];
extern FieldstgPlacedActor *wstag451_actors[];
extern FieldstgSprite wstag451_sprites[];
extern FieldstgMapEvent wstag451_map_events[];

void wstag451_update(WstagObject *obj) {
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

WstagObject *wstag451_start(void *arg0) {
    WstagObject *obj = object_new(wstag451_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag451_funcs.setup();
    return obj;
}

void wstag451_setup(void) {
    fieldstg_stage.background_file = 0x5F6;
    fieldstg_stage.sprite_file = 0x05F70000;
    fieldstg_stage.sprites = wstag451_sprites;
    fieldstg_stage.map_events = wstag451_map_events;
    fieldstg_stage.mask_file = 0x5F5;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1DE00, 0x2F200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag451_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag451_actors;
    fieldstg_stage.color = wstag451_color;
    fieldstg_stage.battle_lists = &wstag451_battle_lists;
    fieldstg_attr.set_file(0, 0x05F70001);
    fieldstg_attr.set_file(7, 0x05F70002);
    fieldstg_attr.set_file(4, 0x05F70003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag451/nonmatchings/wstag451", wstag451_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag451_setup(void);

FieldstgListedBattle D_WSTAG451_800A5FB4 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FC0 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FCC = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FD8 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FE4 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FF0 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A5FFC = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A6008 = { 159, 8, 0x60080000 };
FieldstgBattleList D_WSTAG451_800A6014 = {
    3,
    { &D_WSTAG451_800A5FB4, &D_WSTAG451_800A5FC0, &D_WSTAG451_800A5FCC, &D_WSTAG451_800A5FD8, &D_WSTAG451_800A5FE4,
        &D_WSTAG451_800A5FF0, &D_WSTAG451_800A5FFC, &D_WSTAG451_800A6008 },
};
FieldstgListedBattle D_WSTAG451_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A608C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG451_800A6098 = {
    0,
    { &D_WSTAG451_800A6038, &D_WSTAG451_800A6044, &D_WSTAG451_800A6050, &D_WSTAG451_800A605C, &D_WSTAG451_800A6068,
        &D_WSTAG451_800A6074, &D_WSTAG451_800A6080, &D_WSTAG451_800A608C },
};
FieldstgListedBattle D_WSTAG451_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6110 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG451_800A611C = {
    0,
    { &D_WSTAG451_800A60BC, &D_WSTAG451_800A60C8, &D_WSTAG451_800A60D4, &D_WSTAG451_800A60E0, &D_WSTAG451_800A60EC,
        &D_WSTAG451_800A60F8, &D_WSTAG451_800A6104, &D_WSTAG451_800A6110 },
};
FieldstgListedBattle D_WSTAG451_800A6140 = { 230, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A614C = { 278, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG451_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6170 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG451_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG451_800A6194 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG451_800A61A0 = {
    0,
    { &D_WSTAG451_800A6140, &D_WSTAG451_800A614C, &D_WSTAG451_800A6158, &D_WSTAG451_800A6164, &D_WSTAG451_800A6170,
        &D_WSTAG451_800A617C, &D_WSTAG451_800A6188, &D_WSTAG451_800A6194 },
};
FieldstgBattleLists wstag451_battle_lists = {
    71, 0, 0, { &D_WSTAG451_800A6014, &D_WSTAG451_800A6098, &D_WSTAG451_800A611C }, &D_WSTAG451_800A61A0,
};
FieldstgVramPlace wstag451_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 256, 216, 0, 336, 511 }, { 320, 256, 348, 325, 112, 69, 368, 511 },
    { 320, 256, 336, 344, 64, 88, 320, 510 },
};
u16 D_WSTAG451_800A6270[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A6278[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A6280[6] = { 0, 1, 0x7205, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A628C[8] = { 0, 1, 0x7205, 1, 0x7208, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A629C[4] = { 0x762E, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A62A4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE21, 0,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A62B8[6] = { 0x7400, 1, 0xE21, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A62C4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE21, 1,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A62D8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A62E0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A62E8[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A62F4[8] = { 0, 1, 0x7209, 1, 0xE42, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A6304[6] = { 0x7400, 1, 0xE42, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A6310[10] = {
    0, 1, 0x7209, 1, 0xE42, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A6324[10] = {
    0, 1, 0x7209, 1, 0xE42, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A6338[4] = { 0x782E, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A6340[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A6348[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A6350[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A6358[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A6360[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG451_800A636C[2] = { { NULL, NULL, 208 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG451_800A6384[6] = {
    { D_WSTAG451_800A6270, D_WSTAG451_800A6278, 580 }, { D_WSTAG451_800A6280, NULL, 581 },
    { D_WSTAG451_800A628C, D_WSTAG451_800A629C, 582 }, { D_WSTAG451_800A62A4, D_WSTAG451_800A62B8, 583 },
    { D_WSTAG451_800A62C4, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG451_800A63CC[6] = {
    { D_WSTAG451_800A62D8, D_WSTAG451_800A62E0, 588 }, { D_WSTAG451_800A62E8, NULL, 590 },
    { D_WSTAG451_800A62F4, D_WSTAG451_800A6304, 589 }, { D_WSTAG451_800A6310, NULL, 584 },
    { D_WSTAG451_800A6324, D_WSTAG451_800A6338, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG451_800A6414[4] = {
    { D_WSTAG451_800A6340, NULL, 925 }, { D_WSTAG451_800A6348, D_WSTAG451_800A6350, 585 },
    { D_WSTAG451_800A6358, D_WSTAG451_800A6360, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG451_800A6444[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG451_800A645C[2] = { { NULL, NULL, 207 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG451_800A6474[2] = { { NULL, NULL, 209 }, { NULL, NULL, 0 } };
u16 D_WSTAG451_800A648C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A6498[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A64AC[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG451_800A64C0[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A64D0[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG451_800A64DC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG451_800A64E8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG451_800A64F0 = { D_WSTAG451_800A648C, D_WSTAG451_800A636C, 49, 4, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG451_800A6504 = { D_WSTAG451_800A6498, D_WSTAG451_800A6384, 69, 5, 305, 281, 7 };
FieldstgPlacedActor D_WSTAG451_800A6518 = { D_WSTAG451_800A64AC, D_WSTAG451_800A63CC, 69, 5, 305, 281, 7 };
FieldstgPlacedActor D_WSTAG451_800A652C = { D_WSTAG451_800A64C0, D_WSTAG451_800A6414, 69, 5, 305, 281, 7 };
FieldstgPlacedActor D_WSTAG451_800A6540 = { D_WSTAG451_800A64D0, D_WSTAG451_800A6444, 69, 5, 305, 281, 7 };
FieldstgPlacedActor D_WSTAG451_800A6554 = { D_WSTAG451_800A64DC, D_WSTAG451_800A645C, 157, 6, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG451_800A6568 = { D_WSTAG451_800A64E8, D_WSTAG451_800A6474, 157, 6, 1345, 225, 7 };
FieldstgPlacedActor *wstag451_actors[8] = {
    &D_WSTAG451_800A64F0, &D_WSTAG451_800A6504, &D_WSTAG451_800A6518, &D_WSTAG451_800A652C, &D_WSTAG451_800A6540,
    &D_WSTAG451_800A6554, &D_WSTAG451_800A6568, NULL,
};
FieldstgSprite wstag451_sprites[24] = {
    { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 29, 246, 0, 0 }, { 1, 0, 0x64, 2, 0xA, 0, 0, 0, 0, 0, 30, 321, 0, 0 },
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 658, 621, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 1094, 879, 0, 0 },
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 1224, 196, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 1660, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 243, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 582, 592, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 845, 348, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 930, 107, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1253, 561, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1303, 821, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1632, 260, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 526, 419, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 694, 821, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 953, 881, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1020, 767, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1301, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1413, 161, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1625, 713, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1425, 433, 443, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1041, 394, 430, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 918, 546, 581, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag451_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A1, 0x80, 0x220, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A4, 0x49C, 0x246, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A3, 0xF8, 0xA4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A7, 0x560, 0x440, 1, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 0xB, 2 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFD8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFB8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x28, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag451_funcs = { wstag451_setup };
