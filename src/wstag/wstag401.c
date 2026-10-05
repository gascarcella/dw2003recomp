#include "wstag.h"

/* WSTAG401: stage 0x299 (fieldstg_stages). */

extern WstagFuncs wstag401_funcs;
extern CVECTOR wstag401_color;
extern FieldstgBattleLists wstag401_battle_lists;
extern FieldstgVramPlace wstag401_vram_places[];
extern FieldstgPlacedActor *wstag401_actors[];
extern FieldstgSprite wstag401_sprites[];
extern FieldstgMapEvent wstag401_map_events[];
extern FieldstgEventDef wstag401_events[];

void wstag401_update(WstagObject *obj) {
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

WstagObject *wstag401_start(void *arg0) {
    WstagObject *obj = object_new(wstag401_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag401_funcs.setup();
    return obj;
}

void wstag401_setup(void) {
    fieldstg_stage.background_file = 0x599;
    fieldstg_stage.sprite_file = 0x059A0000;
    fieldstg_stage.sprites = wstag401_sprites;
    fieldstg_stage.map_events = wstag401_map_events;
    fieldstg_stage.mask_file = 0x598;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDE00, 0xDE00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag401_vram_places;
    fieldstg_stage.music = 0x2D;
    fieldstg_stage.sound = 0x60B40000;
    fieldstg_stage.actors = wstag401_actors;
    fieldstg_stage.color = wstag401_color;
    fieldstg_stage.events = wstag401_events;
    fieldstg_stage.battle_lists = &wstag401_battle_lists;
    fieldstg_attr.set_file(0, 0x059A0001);
    fieldstg_attr.set_file(7, 0x059A0002);
    fieldstg_attr.set_file(4, 0x059A0003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag401/nonmatchings/wstag401", wstag401_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag401_setup(void);

FieldstgListedBattle D_WSTAG401_800A5FB4 = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FC0 = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FCC = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FD8 = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FE4 = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FF0 = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A5FFC = { 106, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6008 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG401_800A6014 = {
    3,
    { &D_WSTAG401_800A5FB4, &D_WSTAG401_800A5FC0, &D_WSTAG401_800A5FCC, &D_WSTAG401_800A5FD8, &D_WSTAG401_800A5FE4,
        &D_WSTAG401_800A5FF0, &D_WSTAG401_800A5FFC, &D_WSTAG401_800A6008 },
};
FieldstgListedBattle D_WSTAG401_800A6038 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6044 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6050 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A605C = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6068 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6074 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6080 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A608C = { 99, 13, 0x60080000 };
FieldstgBattleList D_WSTAG401_800A6098 = {
    3,
    { &D_WSTAG401_800A6038, &D_WSTAG401_800A6044, &D_WSTAG401_800A6050, &D_WSTAG401_800A605C, &D_WSTAG401_800A6068,
        &D_WSTAG401_800A6074, &D_WSTAG401_800A6080, &D_WSTAG401_800A608C },
};
FieldstgListedBattle D_WSTAG401_800A60BC = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A60C8 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A60D4 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A60E0 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A60EC = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A60F8 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6104 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6110 = { 0, 4, 0x60080000 };
FieldstgBattleList D_WSTAG401_800A611C = {
    0,
    { &D_WSTAG401_800A60BC, &D_WSTAG401_800A60C8, &D_WSTAG401_800A60D4, &D_WSTAG401_800A60E0, &D_WSTAG401_800A60EC,
        &D_WSTAG401_800A60F8, &D_WSTAG401_800A6104, &D_WSTAG401_800A6110 },
};
FieldstgListedBattle D_WSTAG401_800A6140 = { 228, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A614C = { 276, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG401_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG401_800A6164 = { 331, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6170 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A617C = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6188 = { 177, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG401_800A6194 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG401_800A61A0 = {
    0,
    { &D_WSTAG401_800A6140, &D_WSTAG401_800A614C, &D_WSTAG401_800A6158, &D_WSTAG401_800A6164, &D_WSTAG401_800A6170,
        &D_WSTAG401_800A617C, &D_WSTAG401_800A6188, &D_WSTAG401_800A6194 },
};
FieldstgBattleLists wstag401_battle_lists = {
    66, 0, 0, { &D_WSTAG401_800A6014, &D_WSTAG401_800A6098, &D_WSTAG401_800A611C }, &D_WSTAG401_800A61A0,
};
FieldstgVramPlace wstag401_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 416, 160, 160, 352, 511 }, { 384, 256, 436, 299, 464, 43, 368, 511 },
};
u16 D_WSTAG401_800A6260[8] = { 0x22B, 1, 0x8029, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6270[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6278[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6280[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A628C[8] = { 0, 1, 0x7206, 1, 0x7208, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A629C[4] = { 0x762C, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A62A4[10] = {
    0, 1, 0x7206, 1, 0x7208, 1, 0xE1F, 0,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A62B8[6] = { 0x7400, 1, 0xE1F, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A62C4[10] = {
    0, 1, 0x7206, 1, 0x7208, 1, 0xE1F, 1,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A62D8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A62E0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A62E8[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A62F4[8] = { 0, 1, 0x7209, 1, 0xE40, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6304[6] = { 0x7400, 1, 0xE40, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6310[10] = {
    0, 1, 0x7209, 1, 0xE40, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A6324[10] = {
    0, 1, 0x7209, 1, 0xE40, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A6338[4] = { 0x782C, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6340[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6348[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6350[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6358[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6360[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG401_800A636C[2] = { { NULL, D_WSTAG401_800A6260, 385 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG401_800A6384[6] = {
    { D_WSTAG401_800A6270, D_WSTAG401_800A6278, 580 }, { D_WSTAG401_800A6280, NULL, 581 },
    { D_WSTAG401_800A628C, D_WSTAG401_800A629C, 582 }, { D_WSTAG401_800A62A4, D_WSTAG401_800A62B8, 583 },
    { D_WSTAG401_800A62C4, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG401_800A63CC[6] = {
    { D_WSTAG401_800A62D8, D_WSTAG401_800A62E0, 588 }, { D_WSTAG401_800A62E8, NULL, 590 },
    { D_WSTAG401_800A62F4, D_WSTAG401_800A6304, 589 }, { D_WSTAG401_800A6310, NULL, 584 },
    { D_WSTAG401_800A6324, D_WSTAG401_800A6338, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG401_800A6414[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG401_800A642C[4] = {
    { D_WSTAG401_800A6340, NULL, 923 }, { D_WSTAG401_800A6348, D_WSTAG401_800A6350, 585 },
    { D_WSTAG401_800A6358, D_WSTAG401_800A6360, 586 }, { NULL, NULL, 0 },
};
u16 D_WSTAG401_800A645C[4] = { 0x22B, 0, 0xFFFF, 0 };
u16 D_WSTAG401_800A6464[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A6478[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG401_800A648C[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG401_800A6498[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG401_800A64A8 = { D_WSTAG401_800A645C, D_WSTAG401_800A636C, 33, 4, 1407, 809, 1 };
FieldstgPlacedActor D_WSTAG401_800A64BC = { D_WSTAG401_800A6464, D_WSTAG401_800A6384, 69, 5, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG401_800A64D0 = { D_WSTAG401_800A6478, D_WSTAG401_800A63CC, 69, 5, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG401_800A64E4 = { D_WSTAG401_800A648C, D_WSTAG401_800A6414, 69, 5, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG401_800A64F8 = { D_WSTAG401_800A6498, D_WSTAG401_800A642C, 69, 5, 1655, 1299, 7 };
FieldstgPlacedActor *wstag401_actors[6] = {
    &D_WSTAG401_800A64A8, &D_WSTAG401_800A64BC, &D_WSTAG401_800A64D0, &D_WSTAG401_800A64E4, &D_WSTAG401_800A64F8,
    NULL,
};
FieldstgSprite wstag401_sprites[149] = {
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 303, 734, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 728, 541, 0, 0 },
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 789, 952, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 790, 88, 0, 0 },
    { 1, 0, 0x78, 2, 1, 0, 0, 0, 0, 0, 432, 785, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 598, 649, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1022, 494, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1425, 748, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 351, 920, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 381, 639, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 512, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 760, 507, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1274, 769, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1290, 609, 0, 0 },
    { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 177, 719, 0, 0 },
    { 1, 0, 0xE0, 0xA, 0x4A, 1, 0x4A, 0x4C, 8, 0, 184, 992, 0, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 139, 776, 813, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 134, 729, 771, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 166, 303, 303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 248, 248, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 204, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 240, 311, 311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 240, 503, 503, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 246, 871, 871, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 351, 351, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 447, 447, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 264, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 391, 391, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 791, 791, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 479, 479, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 327, 327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 423, 423, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 345, 212, 212, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 367, 263, 263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 399, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 951, 951, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 458, 252, 252, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 303, 303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 506, 276, 276, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 544, 1135, 1135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 560, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 823, 823, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 1015, 1015, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 608, 319, 319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 633, 291, 291, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 1039, 1039, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 1183, 1183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 647, 827, 827, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 1071, 1071, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 681, 315, 315, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 1207, 1207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 703, 815, 815, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 1047, 1047, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 729, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 1087, 1087, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 711, 711, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 768, 1119, 1119, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 783, 843, 843, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 800, 1255, 1255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 832, 815, 815, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 832, 1135, 1135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 848, 759, 759, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 1167, 1167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 1231, 1231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 880, 791, 791, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 911, 970, 970, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 1143, 1143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 927, 1241, 1241, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 799, 799, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 1183, 1183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 935, 858, 858, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 960, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 983, 983, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 989, 1034, 1034, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 1262, 1262, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 997, 201, 201, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1024, 959, 959, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1040, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1040, 1143, 1143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1042, 1251, 1251, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1055, 215, 215, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1064, 1083, 1083, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1087, 1222, 1222, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1088, 1119, 1119, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1104, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 319, 319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 1007, 1007, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1145, 1251, 1251, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1152, 255, 255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1152, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1168, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1168, 983, 983, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1184, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1184, 1279, 1279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1200, 919, 919, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1200, 1239, 1239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1216, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1216, 959, 959, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1223, 1188, 1188, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1232, 1303, 1303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1248, 1263, 1263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 935, 935, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 1175, 1175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 1327, 1327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1311, 1182, 1182, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1312, 327, 327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1312, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1321, 1146, 1146, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 951, 951, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 991, 991, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 1118, 1118, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 1311, 1311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1360, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1376, 1023, 1023, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1392, 1327, 1327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1398, 1051, 1051, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1400, 227, 227, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1440, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1440, 1063, 1063, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1456, 1319, 1319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1488, 407, 407, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1503, 1295, 1295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1504, 223, 223, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1525, 443, 443, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1542, 298, 298, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1568, 1279, 1279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1584, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1601, 464, 464, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag401_map_events[22] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29A, 0xB0, 0x4F8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x298, 0x654, 0x442, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29B, 0x384, 0x160, 3, 0x64, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 0xB, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 9, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 3, 3 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x34D, 0xFA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x33F, 0x16C, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x59E, 0x111, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x58F, 0x163, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x2DE, 0x4E3, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x2D0, 0x550, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x60, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFF0, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag401_funcs = { wstag401_setup };
FieldstgEventDef wstag401_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
