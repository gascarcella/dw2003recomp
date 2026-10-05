#include "wstag.h"

/* WSTAG956: stage 0x29C (fieldstg_stages_2d). */

extern WstagFuncs wstag956_funcs;
extern FieldstgVramPlace wstag956_vram_places[];
extern FieldstgPlacedActor *wstag956_actors[];
extern FieldstgSprite wstag956_sprites[];
extern FieldstgMapEvent wstag956_map_events[];
extern FieldstgBattleLists wstag956_battle_lists;

void wstag956_update(WstagObject *obj) {
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

WstagObject *wstag956_start(void *arg0) {
    WstagObject *obj = object_new(wstag956_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag956_funcs.setup();
    return obj;
}

void wstag956_setup(void) {
    fieldstg_stage.background_file = 0x783;
    fieldstg_stage.sprite_file = 0x09270000;
    fieldstg_stage.sprites = wstag956_sprites;
    fieldstg_stage.map_events = wstag956_map_events;
    fieldstg_stage.mask_file = 0x926;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1D100, 0x1F600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag956_vram_places;
    fieldstg_stage.music = 0xC;
    fieldstg_stage.sound = 0x60300000;
    fieldstg_stage.actors = wstag956_actors;
    fieldstg_stage.battle_lists = &wstag956_battle_lists;
    fieldstg_attr.set_file(0, 0x09270001);
    fieldstg_attr.set_file(7, 0x09270002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag956_setup(void);

FieldstgVramPlace wstag956_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 377, 462, 228, 206, 320, 511 }, { 320, 256, 334, 476, 56, 220, 336, 511 },
    { 448, 256, 464, 256, 576, 0, 352, 511 }, { 384, 256, 432, 430, 448, 174, 368, 511 },
    { 448, 256, 472, 256, 608, 0, 320, 510 }, { 384, 256, 420, 320, 400, 64, 336, 510 },
    { 448, 256, 500, 256, 720, 0, 352, 510 },
};
u16 D_WSTAG956_800A604C[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6058[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6064[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A6070[8] = { 0x11, 0, 0x10, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6080[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A608C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A6094[6] = { 0x11, 0, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A60A0[4] = { 0x7845, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A60A8[4] = { 0x7096, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A60B0[6] = { 0x7096, 1, 0x100D, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A60BC[6] = { 0x100D, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A60C8[6] = { 0x7096, 1, 0x100D, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A60D4[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A60E0[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A60EC[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A60F8[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6108[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6114[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A611C[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A6128[4] = { 0x784B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG956_800A6130[2] = { { NULL, NULL, 94 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG956_800A6148[5] = {
    { D_WSTAG956_800A604C, D_WSTAG956_800A6058, 96 }, { D_WSTAG956_800A6064, D_WSTAG956_800A6070, 97 },
    { D_WSTAG956_800A6080, D_WSTAG956_800A608C, 94 }, { D_WSTAG956_800A6094, D_WSTAG956_800A60A0, 95 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG956_800A6184[2] = { { NULL, NULL, 118 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG956_800A619C[2] = { { NULL, NULL, 117 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG956_800A61B4[2] = { { NULL, NULL, 121 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG956_800A61CC[4] = {
    { D_WSTAG956_800A60A8, NULL, 114 }, { D_WSTAG956_800A60B0, D_WSTAG956_800A60BC, 115 },
    { D_WSTAG956_800A60C8, NULL, 116 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG956_800A61FC[2] = { { NULL, NULL, 90 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG956_800A6214[5] = {
    { D_WSTAG956_800A60D4, D_WSTAG956_800A60E0, 92 }, { D_WSTAG956_800A60EC, D_WSTAG956_800A60F8, 93 },
    { D_WSTAG956_800A6108, D_WSTAG956_800A6114, 90 }, { D_WSTAG956_800A611C, D_WSTAG956_800A6128, 91 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG956_800A6250[2] = { { NULL, NULL, 120 }, { NULL, NULL, 0 } };
u16 D_WSTAG956_800A6268[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6270[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG956_800A6278[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG956_800A6280[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG956_800A6288 = { D_WSTAG956_800A6268, D_WSTAG956_800A6130, 45, 4, 623, 408, 7 };
FieldstgPlacedActor D_WSTAG956_800A629C = { D_WSTAG956_800A6270, D_WSTAG956_800A6148, 45, 4, 623, 408, 7 };
FieldstgPlacedActor D_WSTAG956_800A62B0 = { NULL, D_WSTAG956_800A6184, 46, 5, 592, 424, 5 };
FieldstgPlacedActor D_WSTAG956_800A62C4 = { NULL, D_WSTAG956_800A619C, 49, 6, 449, 481, 1 };
FieldstgPlacedActor D_WSTAG956_800A62D8 = { NULL, D_WSTAG956_800A61B4, 134, 7, 1088, 400, 7 };
FieldstgPlacedActor D_WSTAG956_800A62EC = { NULL, D_WSTAG956_800A61CC, 143, 8, 352, 234, 1 };
FieldstgPlacedActor D_WSTAG956_800A6300 = { D_WSTAG956_800A6278, D_WSTAG956_800A61FC, 179, 9, 914, 344, 7 };
FieldstgPlacedActor D_WSTAG956_800A6314 = { D_WSTAG956_800A6280, D_WSTAG956_800A6214, 179, 9, 914, 344, 7 };
FieldstgPlacedActor D_WSTAG956_800A6328 = { NULL, D_WSTAG956_800A6250, 373, 10, 833, 257, 5 };
FieldstgPlacedActor *wstag956_actors[10] = {
    &D_WSTAG956_800A6288, &D_WSTAG956_800A629C, &D_WSTAG956_800A62B0, &D_WSTAG956_800A62C4, &D_WSTAG956_800A62D8,
    &D_WSTAG956_800A62EC, &D_WSTAG956_800A6300, &D_WSTAG956_800A6314, &D_WSTAG956_800A6328, NULL,
};
FieldstgSprite wstag956_sprites[35] = {
    { 1, 0, 0x4A, 2, 0, 1, 0, 3, 6, 0, 441, 290, 0, 0 }, { 1, 0, 0x4A, 2, 0, 1, 0, 3, 6, 0, 817, 114, 0, 0 },
    { 1, 0, 0x4A, 2, 0, 1, 0, 3, 6, 0, 949, 528, 0, 0 }, { 1, 0, 0x40, 2, 4, 1, 4, 7, 4, 0, 366, 274, 0, 0 },
    { 1, 0, 0x40, 2, 4, 1, 4, 7, 4, 0, 446, 241, 0, 0 }, { 1, 0, 0x40, 2, 4, 1, 4, 7, 4, 0, 1129, 333, 0, 0 },
    { 1, 0, 0x80, 2, 0x24, 0, 0, 0, 0, 0, 469, 191, 0, 0 }, { 1, 0, 0x80, 2, 0x25, 0, 0, 0, 0, 0, 256, 161, 0, 0 },
    { 1, 0, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 411, 424, 0, 0 }, { 1, 0, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 538, 379, 0, 0 },
    { 1, 0, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 568, 341, 0, 0 }, { 1, 0, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 607, 347, 0, 0 },
    { 1, 0, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 976, 215, 0, 0 },
    { 1, 0, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 544, 363, 0, 0 },
    { 1, 0, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 579, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 884, 243, 0, 0 },
    { 1, 0, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 913, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 1020, 218, 0, 0 },
    { 1, 0, 0x40, 6, 0x10, 1, 0x10, 0x12, 4, 0, 906, 526, 0, 0 },
    { 1, 0x64, 0x40, 6, 0x1B, 0, 0, 0, 0, 0, 721, 200, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x1C, 0, 0, 0, 0, 0, 451, 367, 0, 0 },
    { 1, 0, 0x40, 4, 8, 1, 8, 0xB, 4, 0, 448, 405, 445, 0 },
    { 1, 0, 0x40, 4, 0xC, 1, 0xC, 0xF, 4, 0, 465, 438, 464, 0 },
    { 1, 0, 0x40, 4, 0xC, 1, 0xC, 0xF, 4, 0, 737, 256, 281, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 898, 552, 576, 0 },
    { 1, 0, 0x40, 4, 0x19, 0, 0, 0, 0, 0, 727, 260, 280, 0 },
    { 1, 0, 0x40, 4, 0x1A, 0, 0, 0, 0, 0, 447, 417, 444, 0 },
    { 1, 0, 0x64, 4, 0x1D, 0, 0, 0, 0, 0, 784, 496, 560, 0 },
    { 1, 0, 0x64, 4, 0x1E, 0, 0, 0, 0, 0, 768, 488, 552, 0 },
    { 1, 0, 0x64, 4, 0x1F, 0, 0, 0, 0, 0, 752, 480, 544, 0 },
    { 1, 0, 0x64, 4, 0x20, 0, 0, 0, 0, 0, 736, 472, 536, 0 },
    { 1, 0, 0x64, 4, 0x21, 0, 0, 0, 0, 0, 720, 464, 528, 0 },
    { 1, 0, 0x64, 4, 0x22, 0, 0, 0, 0, 0, 704, 456, 520, 0 },
    { 1, 0, 0x64, 4, 0x23, 0, 0, 0, 0, 0, 672, 456, 496, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag956_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x298, 0x448, 0xF8, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29E, 0xC8, 0x1A4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29E, 0x1B8, 0x204, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29D, 0x208, 0x1AC, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29F, 0x1A7, 0x254, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x400, 0x120, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x410, 0x188, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag956_funcs = { wstag956_setup };
FieldstgListedBattle D_WSTAG956_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A66F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG956_800A6700 = {
    3,
    { &D_WSTAG956_800A66A0, &D_WSTAG956_800A66AC, &D_WSTAG956_800A66B8, &D_WSTAG956_800A66C4, &D_WSTAG956_800A66D0,
        &D_WSTAG956_800A66DC, &D_WSTAG956_800A66E8, &D_WSTAG956_800A66F4 },
};
FieldstgListedBattle D_WSTAG956_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A673C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6748 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6754 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6760 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A676C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6778 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG956_800A6784 = {
    0,
    { &D_WSTAG956_800A6724, &D_WSTAG956_800A6730, &D_WSTAG956_800A673C, &D_WSTAG956_800A6748, &D_WSTAG956_800A6754,
        &D_WSTAG956_800A6760, &D_WSTAG956_800A676C, &D_WSTAG956_800A6778 },
};
FieldstgListedBattle D_WSTAG956_800A67A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A67FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG956_800A6808 = {
    0,
    { &D_WSTAG956_800A67A8, &D_WSTAG956_800A67B4, &D_WSTAG956_800A67C0, &D_WSTAG956_800A67CC, &D_WSTAG956_800A67D8,
        &D_WSTAG956_800A67E4, &D_WSTAG956_800A67F0, &D_WSTAG956_800A67FC },
};
FieldstgListedBattle D_WSTAG956_800A682C = { 305, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG956_800A6838 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6844 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6850 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A685C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG956_800A6880 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG956_800A688C = {
    0,
    { &D_WSTAG956_800A682C, &D_WSTAG956_800A6838, &D_WSTAG956_800A6844, &D_WSTAG956_800A6850, &D_WSTAG956_800A685C,
        &D_WSTAG956_800A6868, &D_WSTAG956_800A6874, &D_WSTAG956_800A6880 },
};
FieldstgBattleLists wstag956_battle_lists = {
    393, 0, 0, { &D_WSTAG956_800A6700, &D_WSTAG956_800A6784, &D_WSTAG956_800A6808 }, &D_WSTAG956_800A688C,
};
