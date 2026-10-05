#include "wstag.h"

/* WSTAG491: stage 0x2A9 (fieldstg_stages). */

extern WstagFuncs wstag491_funcs;
extern FieldstgBattleLists wstag491_battle_lists;
extern FieldstgVramPlace wstag491_vram_places[];
extern FieldstgPlacedActor *wstag491_actors[];
extern FieldstgSprite wstag491_sprites[];
extern FieldstgMapEvent wstag491_map_events[];

void wstag491_update(WstagObject *obj) {
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

WstagObject *wstag491_start(void *arg0) {
    WstagObject *obj = object_new(wstag491_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag491_funcs.setup();
    return obj;
}

void wstag491_setup(void) {
    fieldstg_stage.background_file = 0x5B9;
    fieldstg_stage.sprite_file = 0x05BA0000;
    fieldstg_stage.sprites = wstag491_sprites;
    fieldstg_stage.map_events = wstag491_map_events;
    fieldstg_stage.mask_file = 0x5B8;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x3A900, 0x37D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag491_vram_places;
    fieldstg_stage.music = 0x35;
    fieldstg_stage.sound = 0x60D40000;
    fieldstg_stage.actors = wstag491_actors;
    fieldstg_stage.battle_lists = &wstag491_battle_lists;
    fieldstg_attr.set_file(0, 0x05BA0001);
    fieldstg_attr.set_file(7, 0x05BA0002);
    fieldstg_attr.set_file(4, 0x05BA0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag491_setup(void);

FieldstgListedBattle D_WSTAG491_800A5F94 = { 109, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FA0 = { 109, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FAC = { 176, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FB8 = { 176, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FC4 = { 95, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FD0 = { 95, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FDC = { 95, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A5FE8 = { 95, 8, 0x60080000 };
FieldstgBattleList D_WSTAG491_800A5FF4 = {
    3,
    { &D_WSTAG491_800A5F94, &D_WSTAG491_800A5FA0, &D_WSTAG491_800A5FAC, &D_WSTAG491_800A5FB8, &D_WSTAG491_800A5FC4,
        &D_WSTAG491_800A5FD0, &D_WSTAG491_800A5FDC, &D_WSTAG491_800A5FE8 },
};
FieldstgListedBattle D_WSTAG491_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG491_800A6078 = {
    0,
    { &D_WSTAG491_800A6018, &D_WSTAG491_800A6024, &D_WSTAG491_800A6030, &D_WSTAG491_800A603C, &D_WSTAG491_800A6048,
        &D_WSTAG491_800A6054, &D_WSTAG491_800A6060, &D_WSTAG491_800A606C },
};
FieldstgListedBattle D_WSTAG491_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG491_800A60FC = {
    0,
    { &D_WSTAG491_800A609C, &D_WSTAG491_800A60A8, &D_WSTAG491_800A60B4, &D_WSTAG491_800A60C0, &D_WSTAG491_800A60CC,
        &D_WSTAG491_800A60D8, &D_WSTAG491_800A60E4, &D_WSTAG491_800A60F0 },
};
FieldstgListedBattle D_WSTAG491_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6144 = { 331, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A6150 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG491_800A6168 = { 177, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG491_800A6174 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG491_800A6180 = {
    0,
    { &D_WSTAG491_800A6120, &D_WSTAG491_800A612C, &D_WSTAG491_800A6138, &D_WSTAG491_800A6144, &D_WSTAG491_800A6150,
        &D_WSTAG491_800A615C, &D_WSTAG491_800A6168, &D_WSTAG491_800A6174 },
};
FieldstgBattleLists wstag491_battle_lists = {
    76, 0, 0, { &D_WSTAG491_800A5FF4, &D_WSTAG491_800A6078, &D_WSTAG491_800A60FC }, &D_WSTAG491_800A6180,
};
FieldstgVramPlace wstag491_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 492, 256, 688, 0, 336, 510 }, { 448, 256, 500, 256, 720, 0, 352, 510 },
    { 448, 256, 482, 280, 648, 24, 368, 510 }, { 448, 256, 490, 296, 680, 40, 336, 509 },
    { 448, 256, 456, 280, 544, 24, 352, 509 },
};
u16 D_WSTAG491_800A6270[4] = { 0x8667, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A6278[6] = { 0x8667, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG491_800A6284[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A628C[8] = { 0x8667, 0, 0, 1, 0x8463, 0, 0xFFFF, 0 };
u16 D_WSTAG491_800A629C[8] = { 0x8667, 0, 0, 1, 0x8463, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A62AC[10] = {
    0x8667, 1, 0x8666, 0, 0x8463, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG491_800A62C0[2] = { { NULL, NULL, 260 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A62D8[2] = { { NULL, NULL, 263 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A62F0[2] = { { NULL, NULL, 262 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A6308[2] = { { NULL, NULL, 264 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A6320[2] = { { NULL, NULL, 259 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A6338[2] = { { NULL, NULL, 261 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG491_800A6350[5] = {
    { D_WSTAG491_800A6270, NULL, 734 }, { D_WSTAG491_800A6278, D_WSTAG491_800A6284, 735 },
    { D_WSTAG491_800A628C, NULL, 736 }, { D_WSTAG491_800A629C, D_WSTAG491_800A62AC, 737 }, { NULL, NULL, 0 },
};
u16 D_WSTAG491_800A638C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A6398[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A63A4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG491_800A63B0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A63B8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG491_800A63C4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG491_800A63CC[10] = {
    0x7042, 1, 0x704A, 1, 0x8666, 1, 0x8667, 0,
    0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG491_800A63E0 = { D_WSTAG491_800A638C, D_WSTAG491_800A62C0, 47, 4, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG491_800A63F4 = { D_WSTAG491_800A6398, D_WSTAG491_800A62D8, 55, 5, 1049, 365, 1 };
FieldstgPlacedActor D_WSTAG491_800A6408 = { D_WSTAG491_800A63A4, D_WSTAG491_800A62F0, 157, 6, 1049, 365, 1 };
FieldstgPlacedActor D_WSTAG491_800A641C = { D_WSTAG491_800A63B0, D_WSTAG491_800A6308, 157, 6, 1049, 365, 1 };
FieldstgPlacedActor D_WSTAG491_800A6430 = { D_WSTAG491_800A63B8, D_WSTAG491_800A6320, 158, 7, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG491_800A6444 = { D_WSTAG491_800A63C4, D_WSTAG491_800A6338, 158, 7, 561, 458, 7 };
FieldstgPlacedActor D_WSTAG491_800A6458 = { D_WSTAG491_800A63CC, D_WSTAG491_800A6350, 166, 8, 449, 1034, 3 };
FieldstgPlacedActor *wstag491_actors[8] = {
    &D_WSTAG491_800A63E0, &D_WSTAG491_800A63F4, &D_WSTAG491_800A6408, &D_WSTAG491_800A641C, &D_WSTAG491_800A6430,
    &D_WSTAG491_800A6444, &D_WSTAG491_800A6458, NULL,
};
FieldstgSprite wstag491_sprites[51] = {
    { 1, 0, 0x70, 2, 0xA, 0, 0, 0, 0, 0, 896, 548, 0, 0 }, { 1, 0, 0x78, 2, 0xB, 0, 0, 0, 0, 0, 768, 265, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 323, 130, 0, 0 }, { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 548, 651, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 1011, 794, 0, 0 },
    { 1, 0, 0x40, 6, 0x62, 2, 0, 0xD, 0xA, 0, 902, 118, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 324, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 480, 1007, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x61, 0xA, 0, 1091, 911, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 259, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 452, 984, 0, 0 },
    { 1, 0, 0x40, 6, 0x28, 1, 0x28, 0x31, 0xA, 0, 1034, 904, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 1, 0x34, 0x43, 0xA, 0, 121, 863, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x4E, 0xA, 0, 231, 1080, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x4E, 0xA, 0, 643, 1068, 0, 0 },
    { 1, 0, 0x64, 6, 0x14, 0, 0, 0, 0, 0, 177, 889, 0, 0 }, { 1, 0, 0x64, 6, 0x15, 0, 0, 0, 0, 0, 31, 789, 0, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 103, 696, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 287, 790, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 319, 634, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 331, 860, 1152, 0 },
    { 1, 0, 0x40, 4, 0x4F, 1, 0x4F, 0x57, 0xA, 0, 425, 935, 1152, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 566, 586, 673, 0 }, { 1, 0, 0x50, 4, 1, 0, 0, 0, 0, 0, 1013, 384, 452, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 959, 427, 502, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 310, 860, 900, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 448, 513, 569, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 592, 744, 782, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 992, 151, 200, 0 },
    { 1, 0, 0x64, 4, 0x12, 0, 0, 0, 0, 0, 445, 977, 1021, 0 },
    { 1, 0, 0x64, 4, 0x13, 0, 0, 0, 0, 0, 896, 756, 810, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 80, 759, 759, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 191, 191, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 223, 223, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 247, 247, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 368, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 447, 647, 647, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 439, 439, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 615, 615, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 512, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 613, 879, 879, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 624, 839, 839, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 447, 447, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 487, 487, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 887, 887, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 704, 543, 543, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 935, 935, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 871, 871, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 823, 823, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag491_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AA, 0x372, 0x272, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AF, 0x90, 0x128, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A8, 0xA0, 0x80, 7, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 4, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 8, 2 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xE, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x48, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x20, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xE, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag491_funcs = { wstag491_setup };
