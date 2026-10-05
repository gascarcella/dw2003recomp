#include "wstag.h"

/* WSTAG396: stage 0x298 (fieldstg_stages). */

extern WstagFuncs wstag396_funcs;
extern FieldstgBattleLists wstag396_battle_lists;
extern FieldstgVramPlace wstag396_vram_places[];
extern FieldstgPlacedActor *wstag396_actors[];
extern FieldstgSprite wstag396_sprites[];
extern FieldstgMapEvent wstag396_map_events[];

void wstag396_update(WstagObject *obj) {
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

WstagObject *wstag396_start(void *arg0) {
    WstagObject *obj = object_new(wstag396_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag396_funcs.setup();
    return obj;
}

void wstag396_setup(void) {
    fieldstg_stage.background_file = 0x590;
    fieldstg_stage.sprite_file = 0x05910000;
    fieldstg_stage.sprites = wstag396_sprites;
    fieldstg_stage.map_events = wstag396_map_events;
    fieldstg_stage.mask_file = 0x58F;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFE00, 0xA600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag396_vram_places;
    fieldstg_stage.music = 0x2E;
    fieldstg_stage.sound = 0x60B80000;
    fieldstg_stage.actors = wstag396_actors;
    fieldstg_stage.battle_lists = &wstag396_battle_lists;
    fieldstg_attr.set_file(0, 0x05910002);
    fieldstg_attr.set_file(7, 0x05910001);
    fieldstg_attr.set_file(4, 0x05910003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag396_setup(void);

FieldstgListedBattle D_WSTAG396_800A5F8C = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5F98 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FA4 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FB0 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FBC = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FC8 = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FD4 = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A5FE0 = { 94, 1, 0x60080000 };
FieldstgBattleList D_WSTAG396_800A5FEC = {
    3,
    { &D_WSTAG396_800A5F8C, &D_WSTAG396_800A5F98, &D_WSTAG396_800A5FA4, &D_WSTAG396_800A5FB0, &D_WSTAG396_800A5FBC,
        &D_WSTAG396_800A5FC8, &D_WSTAG396_800A5FD4, &D_WSTAG396_800A5FE0 },
};
FieldstgListedBattle D_WSTAG396_800A6010 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A601C = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6028 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6034 = { 151, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6040 = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A604C = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6058 = { 94, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6064 = { 94, 1, 0x60080000 };
FieldstgBattleList D_WSTAG396_800A6070 = {
    1,
    { &D_WSTAG396_800A6010, &D_WSTAG396_800A601C, &D_WSTAG396_800A6028, &D_WSTAG396_800A6034, &D_WSTAG396_800A6040,
        &D_WSTAG396_800A604C, &D_WSTAG396_800A6058, &D_WSTAG396_800A6064 },
};
FieldstgListedBattle D_WSTAG396_800A6094 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60A0 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60AC = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60B8 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60C4 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60D0 = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60DC = { 100, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A60E8 = { 100, 4, 0x60080000 };
FieldstgBattleList D_WSTAG396_800A60F4 = {
    3,
    { &D_WSTAG396_800A6094, &D_WSTAG396_800A60A0, &D_WSTAG396_800A60AC, &D_WSTAG396_800A60B8, &D_WSTAG396_800A60C4,
        &D_WSTAG396_800A60D0, &D_WSTAG396_800A60DC, &D_WSTAG396_800A60E8 },
};
FieldstgListedBattle D_WSTAG396_800A6118 = { 227, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6124 = { 275, 1, 0x600C0000 };
FieldstgListedBattle D_WSTAG396_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG396_800A613C = { 329, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG396_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG396_800A6160 = { 99, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG396_800A616C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG396_800A6178 = {
    0,
    { &D_WSTAG396_800A6118, &D_WSTAG396_800A6124, &D_WSTAG396_800A6130, &D_WSTAG396_800A613C, &D_WSTAG396_800A6148,
        &D_WSTAG396_800A6154, &D_WSTAG396_800A6160, &D_WSTAG396_800A616C },
};
FieldstgBattleLists wstag396_battle_lists = {
    65, 0, 0, { &D_WSTAG396_800A5FEC, &D_WSTAG396_800A6070, &D_WSTAG396_800A60F4 }, &D_WSTAG396_800A6178,
};
FieldstgVramPlace wstag396_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 438, 256, 472, 0, 320, 511 }, { 384, 256, 422, 256, 408, 0, 352, 511 },
};
u16 D_WSTAG396_800A6238[8] = { 0x708F, 1, 0x22A, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A6248[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6250[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A6258[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6264[8] = { 0, 1, 0x7206, 1, 0x7208, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6274[4] = { 0x762B, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A627C[10] = {
    0, 1, 0x7206, 1, 0x7208, 1, 0xE1E, 0,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A6290[6] = { 0x7400, 1, 0xE1E, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A629C[10] = {
    0, 1, 0x7206, 1, 0x7208, 1, 0xE1E, 1,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A62B0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A62B8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A62C0[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A62CC[8] = { 0, 1, 0x7209, 1, 0xE3F, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A62DC[6] = { 0x7400, 1, 0xE3F, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A62E8[10] = {
    0, 1, 0x7209, 1, 0xE3F, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A62FC[10] = {
    0, 1, 0x7209, 1, 0xE3F, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A6310[4] = { 0x782B, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A6318[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6320[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6328[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A6330[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A6338[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG396_800A6344[2] = { { NULL, D_WSTAG396_800A6238, 384 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG396_800A635C[6] = {
    { D_WSTAG396_800A6248, D_WSTAG396_800A6250, 580 }, { D_WSTAG396_800A6258, NULL, 581 },
    { D_WSTAG396_800A6264, D_WSTAG396_800A6274, 582 }, { D_WSTAG396_800A627C, D_WSTAG396_800A6290, 583 },
    { D_WSTAG396_800A629C, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG396_800A63A4[6] = {
    { D_WSTAG396_800A62B0, D_WSTAG396_800A62B8, 588 }, { D_WSTAG396_800A62C0, NULL, 590 },
    { D_WSTAG396_800A62CC, D_WSTAG396_800A62DC, 589 }, { D_WSTAG396_800A62E8, NULL, 584 },
    { D_WSTAG396_800A62FC, D_WSTAG396_800A6310, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG396_800A63EC[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG396_800A6404[4] = {
    { D_WSTAG396_800A6318, NULL, 924 }, { D_WSTAG396_800A6320, D_WSTAG396_800A6328, 585 },
    { D_WSTAG396_800A6330, D_WSTAG396_800A6338, 586 }, { NULL, NULL, 0 },
};
u16 D_WSTAG396_800A6434[4] = { 0x22A, 0, 0xFFFF, 0 };
u16 D_WSTAG396_800A643C[10] = {
    0x8192, 1, 0x11, 0, 0x7019, 1, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A6450[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG396_800A6464[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG396_800A6470[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG396_800A6480 = { D_WSTAG396_800A6434, D_WSTAG396_800A6344, 33, 4, 1566, 529, 1 };
FieldstgPlacedActor D_WSTAG396_800A6494 = { D_WSTAG396_800A643C, D_WSTAG396_800A635C, 69, 5, 1408, 936, 1 };
FieldstgPlacedActor D_WSTAG396_800A64A8 = { D_WSTAG396_800A6450, D_WSTAG396_800A63A4, 69, 5, 1408, 936, 1 };
FieldstgPlacedActor D_WSTAG396_800A64BC = { D_WSTAG396_800A6464, D_WSTAG396_800A63EC, 69, 5, 1408, 936, 1 };
FieldstgPlacedActor D_WSTAG396_800A64D0 = { D_WSTAG396_800A6470, D_WSTAG396_800A6404, 69, 5, 1408, 936, 1 };
FieldstgPlacedActor *wstag396_actors[6] = {
    &D_WSTAG396_800A6480, &D_WSTAG396_800A6494, &D_WSTAG396_800A64A8, &D_WSTAG396_800A64BC, &D_WSTAG396_800A64D0,
    NULL,
};
FieldstgSprite wstag396_sprites[42] = {
    { 1, 0, 0x78, 2, 0x12, 0, 0, 0, 0, 0, 1216, 594, 0, 0 },
    { 1, 0, 0x78, 2, 0x14, 0, 0, 0, 0, 0, 1172, 388, 0, 0 }, { 1, 0, 0x78, 2, 0x16, 0, 0, 0, 0, 0, 998, 303, 0, 0 },
    { 1, 0, 0x78, 2, 0x17, 0, 0, 0, 0, 0, 953, 355, 0, 0 }, { 1, 0, 0x40, 2, 8, 0, 0, 0, 0, 0, 84, 106, 0, 0 },
    { 1, 0, 0x40, 2, 8, 0, 0, 0, 0, 0, 619, 641, 0, 0 }, { 1, 0, 0x40, 2, 8, 0, 0, 0, 0, 0, 1198, 301, 0, 0 },
    { 1, 0, 0x40, 2, 8, 0, 0, 0, 0, 0, 1556, 782, 0, 0 }, { 1, 0, 0x78, 4, 0x13, 0, 0, 0, 0, 0, 1264, 489, 562, 0 },
    { 1, 0, 0x78, 4, 0x15, 0, 0, 0, 0, 0, 905, 338, 355, 0 },
    { 1, 0, 0x78, 4, 0x18, 0, 0, 0, 0, 0, 914, 450, 464, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1115, 218, 262, 0 }, { 1, 0, 0x44, 4, 1, 0, 0, 0, 0, 0, 828, 359, 400, 0 },
    { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 476, 268, 321, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 1216, 649, 682, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 1169, 434, 465, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 545, 557, 583, 0 },
    { 1, 0, 0x44, 4, 6, 0, 0, 0, 0, 0, 925, 722, 766, 0 }, { 1, 0, 0x73, 4, 7, 0, 0, 0, 0, 0, 634, 658, 767, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 133, 133, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 384, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 425, 155, 155, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 311, 311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 472, 179, 179, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 521, 203, 203, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 263, 263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 391, 391, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 574, 368, 368, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 607, 214, 214, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 617, 347, 347, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 648, 235, 235, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 695, 259, 259, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 744, 283, 283, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 792, 307, 307, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 840, 331, 331, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 799, 799, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1008, 791, 791, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1376, 751, 751, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1424, 775, 775, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1472, 799, 799, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1520, 967, 967, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag396_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x296, 0x4A2, 0x36A, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0xAA, 0xC3, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x150, 0x268, 5, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 8, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 7, 3 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag396_funcs = { wstag396_setup };
