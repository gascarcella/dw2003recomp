#include "wstag.h"

/* WSTAG952: stage 0x298 (fieldstg_stages_2d). */

extern WstagFuncs wstag952_funcs;
extern FieldstgVramPlace wstag952_vram_places[];
extern FieldstgPlacedActor *wstag952_actors[];
extern FieldstgSprite wstag952_sprites[];
extern FieldstgMapEvent wstag952_map_events[];
extern FieldstgBattleLists wstag952_battle_lists;

void wstag952_update(WstagObject *obj) {
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

WstagObject *wstag952_start(void *arg0) {
    WstagObject *obj = object_new(wstag952_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag952_funcs.setup();
    return obj;
}

void wstag952_setup(void) {
    fieldstg_stage.background_file = 0x21A;
    fieldstg_stage.sprite_file = 0x091F0000;
    fieldstg_stage.sprites = wstag952_sprites;
    fieldstg_stage.map_events = wstag952_map_events;
    fieldstg_stage.mask_file = 0x91E;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x30200, 0x23500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag952_vram_places;
    fieldstg_stage.music = 0x2E;
    fieldstg_stage.sound = 0x60B80000;
    fieldstg_stage.actors = wstag952_actors;
    fieldstg_stage.battle_lists = &wstag952_battle_lists;
    fieldstg_attr.set_file(0, 0x091F0002);
    fieldstg_attr.set_file(7, 0x091F0003);
    fieldstg_attr.set_file(4, 0x091F0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag952_setup(void);

FieldstgVramPlace wstag952_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 332, 480, 76, 320, 511 }, { 384, 256, 394, 346, 296, 90, 336, 511 },
    { 384, 256, 432, 332, 448, 76, 352, 511 },
};
u16 D_WSTAG952_800A6024[8] = { 0x26C, 1, 0x823A, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG952_800A6034[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A6040[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A604C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG952_800A6058[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A6068[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A6074[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG952_800A607C[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG952_800A6088[4] = { 0x783A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG952_800A6090[2] = { { NULL, D_WSTAG952_800A6024, 4 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG952_800A60A8[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG952_800A60C0[5] = {
    { D_WSTAG952_800A6034, D_WSTAG952_800A6040, 67 }, { D_WSTAG952_800A604C, D_WSTAG952_800A6058, 68 },
    { D_WSTAG952_800A6068, D_WSTAG952_800A6074, 65 }, { D_WSTAG952_800A607C, D_WSTAG952_800A6088, 66 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG952_800A60FC[2] = { { NULL, NULL, 140 }, { NULL, NULL, 0 } };
u16 D_WSTAG952_800A6114[4] = { 0x26C, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A611C[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG952_800A6124[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG952_800A612C = { D_WSTAG952_800A6114, D_WSTAG952_800A6090, 33, 4, 1566, 529, 1 };
FieldstgPlacedActor D_WSTAG952_800A6140 = { D_WSTAG952_800A611C, D_WSTAG952_800A60A8, 45, 5, 512, 554, 1 };
FieldstgPlacedActor D_WSTAG952_800A6154 = { D_WSTAG952_800A6124, D_WSTAG952_800A60C0, 45, 5, 512, 554, 1 };
FieldstgPlacedActor D_WSTAG952_800A6168 = { NULL, D_WSTAG952_800A60FC, 379, 6, 1408, 936, 7 };
FieldstgPlacedActor *wstag952_actors[5] = {
    &D_WSTAG952_800A612C, &D_WSTAG952_800A6140, &D_WSTAG952_800A6154, &D_WSTAG952_800A6168, NULL,
};
FieldstgSprite wstag952_sprites[41] = {
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 1559, 780, 0, 0 },
    { 1, 0, 0x78, 2, 0x12, 0, 0, 0, 0, 0, 1216, 594, 0, 0 },
    { 1, 0, 0x78, 2, 0x14, 0, 0, 0, 0, 0, 1172, 388, 0, 0 }, { 1, 0, 0x78, 2, 0x16, 0, 0, 0, 0, 0, 998, 303, 0, 0 },
    { 1, 0, 0x78, 2, 0x17, 0, 0, 0, 0, 0, 953, 355, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 86, 107, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 621, 642, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 1, 0xC, 0x11, 8, 0, 1201, 300, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1115, 218, 262, 0 }, { 1, 0, 0x48, 4, 1, 0, 0, 0, 0, 0, 828, 359, 400, 0 },
    { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 476, 268, 321, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 1216, 649, 682, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 1169, 434, 465, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 545, 557, 583, 0 },
    { 1, 0, 0x44, 4, 6, 0, 0, 0, 0, 0, 925, 722, 766, 0 }, { 1, 0, 0x73, 4, 7, 0, 0, 0, 0, 0, 634, 661, 767, 0 },
    { 1, 0, 0x78, 4, 0x13, 0, 0, 0, 0, 0, 1264, 489, 562, 0 },
    { 1, 0, 0x78, 4, 0x15, 0, 0, 0, 0, 0, 905, 338, 355, 0 },
    { 1, 0, 0x78, 4, 0x18, 0, 0, 0, 0, 0, 914, 450, 464, 0 },
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
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1472, 799, 799, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag952_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x296, 0x4A2, 0x36A, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0xAA, 0xC3, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x150, 0x268, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 0, 0, 1, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 0, 0, 1, 2 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag952_funcs = { wstag952_setup };
FieldstgListedBattle D_WSTAG952_800A6520 = { 153, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A652C = { 153, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6538 = { 161, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6544 = { 161, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6550 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A655C = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6568 = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6574 = { 126, 1, 0x60080000 };
FieldstgBattleList D_WSTAG952_800A6580 = {
    3,
    { &D_WSTAG952_800A6520, &D_WSTAG952_800A652C, &D_WSTAG952_800A6538, &D_WSTAG952_800A6544, &D_WSTAG952_800A6550,
        &D_WSTAG952_800A655C, &D_WSTAG952_800A6568, &D_WSTAG952_800A6574 },
};
FieldstgListedBattle D_WSTAG952_800A65A4 = { 153, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65B0 = { 153, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65BC = { 161, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65C8 = { 161, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65D4 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65E0 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65EC = { 126, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A65F8 = { 126, 1, 0x60080000 };
FieldstgBattleList D_WSTAG952_800A6604 = {
    1,
    { &D_WSTAG952_800A65A4, &D_WSTAG952_800A65B0, &D_WSTAG952_800A65BC, &D_WSTAG952_800A65C8, &D_WSTAG952_800A65D4,
        &D_WSTAG952_800A65E0, &D_WSTAG952_800A65EC, &D_WSTAG952_800A65F8 },
};
FieldstgListedBattle D_WSTAG952_800A6628 = { 68, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6634 = { 68, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6640 = { 68, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A664C = { 132, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6658 = { 132, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6664 = { 132, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6670 = { 133, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A667C = { 133, 4, 0x60080000 };
FieldstgBattleList D_WSTAG952_800A6688 = {
    5,
    { &D_WSTAG952_800A6628, &D_WSTAG952_800A6634, &D_WSTAG952_800A6640, &D_WSTAG952_800A664C, &D_WSTAG952_800A6658,
        &D_WSTAG952_800A6664, &D_WSTAG952_800A6670, &D_WSTAG952_800A667C },
};
FieldstgListedBattle D_WSTAG952_800A66AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG952_800A66B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG952_800A66C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG952_800A66D0 = { 333, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A66DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG952_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG952_800A66F4 = { 175, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG952_800A6700 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG952_800A670C = {
    0,
    { &D_WSTAG952_800A66AC, &D_WSTAG952_800A66B8, &D_WSTAG952_800A66C4, &D_WSTAG952_800A66D0, &D_WSTAG952_800A66DC,
        &D_WSTAG952_800A66E8, &D_WSTAG952_800A66F4, &D_WSTAG952_800A6700 },
};
FieldstgBattleLists wstag952_battle_lists = {
    389, 0, 0, { &D_WSTAG952_800A6580, &D_WSTAG952_800A6604, &D_WSTAG952_800A6688 }, &D_WSTAG952_800A670C,
};
