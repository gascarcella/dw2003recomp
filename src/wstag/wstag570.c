#include "wstag.h"

/* WSTAG570: stage 0x24B (fieldstg_stages). */

extern WstagFuncs wstag570_funcs;
extern CVECTOR wstag570_color;
extern FieldstgBattleLists wstag570_battle_lists;
extern FieldstgVramPlace wstag570_vram_places[];
extern FieldstgPlacedActor *wstag570_actors[];
extern FieldstgSprite wstag570_sprites[];
extern FieldstgMapEvent wstag570_map_events[];

void wstag570_update(WstagObject *obj) {
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

WstagObject *wstag570_start(void *arg0) {
    WstagObject *obj = object_new(wstag570_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag570_funcs.setup();
    return obj;
}

void wstag570_setup(void) {
    fieldstg_stage.background_file = 0x422;
    fieldstg_stage.sprite_file = 0x04240000;
    fieldstg_stage.sprites = wstag570_sprites;
    fieldstg_stage.map_events = wstag570_map_events;
    fieldstg_stage.mask_file = 0x423;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x29A00, 0xF400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag570_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag570_actors;
    fieldstg_stage.color = wstag570_color;
    fieldstg_stage.battle_lists = &wstag570_battle_lists;
    fieldstg_attr.set_file(0, 0x04240001);
    fieldstg_attr.set_file(7, 0x04240002);
    fieldstg_attr.set_file(4, 0x04240003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag570/nonmatchings/wstag570", wstag570_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag570_setup(void);

FieldstgListedBattle D_WSTAG570_800A5FE0 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A5FEC = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A5FF8 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A6004 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A6010 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A601C = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A6028 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A6034 = { 68, 3, 0x60080000 };
FieldstgBattleList D_WSTAG570_800A6040 = {
    3,
    { &D_WSTAG570_800A5FE0, &D_WSTAG570_800A5FEC, &D_WSTAG570_800A5FF8, &D_WSTAG570_800A6004, &D_WSTAG570_800A6010,
        &D_WSTAG570_800A601C, &D_WSTAG570_800A6028, &D_WSTAG570_800A6034 },
};
FieldstgListedBattle D_WSTAG570_800A6064 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A60AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A60B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG570_800A60C4 = {
    0,
    { &D_WSTAG570_800A6064, &D_WSTAG570_800A6070, &D_WSTAG570_800A607C, &D_WSTAG570_800A6088, &D_WSTAG570_800A6094,
        &D_WSTAG570_800A60A0, &D_WSTAG570_800A60AC, &D_WSTAG570_800A60B8 },
};
FieldstgListedBattle D_WSTAG570_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A613C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG570_800A6148 = {
    0,
    { &D_WSTAG570_800A60E8, &D_WSTAG570_800A60F4, &D_WSTAG570_800A6100, &D_WSTAG570_800A610C, &D_WSTAG570_800A6118,
        &D_WSTAG570_800A6124, &D_WSTAG570_800A6130, &D_WSTAG570_800A613C },
};
FieldstgListedBattle D_WSTAG570_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A6190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A619C = { 330, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG570_800A61A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A61B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG570_800A61C0 = { 61, 2, 0x60080000 };
FieldstgBattleList D_WSTAG570_800A61CC = {
    0,
    { &D_WSTAG570_800A616C, &D_WSTAG570_800A6178, &D_WSTAG570_800A6184, &D_WSTAG570_800A6190, &D_WSTAG570_800A619C,
        &D_WSTAG570_800A61A8, &D_WSTAG570_800A61B4, &D_WSTAG570_800A61C0 },
};
FieldstgBattleLists wstag570_battle_lists = {
    40, 0, 0, { &D_WSTAG570_800A6040, &D_WSTAG570_800A60C4, &D_WSTAG570_800A6148 }, &D_WSTAG570_800A61CC,
};
FieldstgVramPlace wstag570_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 336, 511 }, { 384, 256, 428, 433, 432, 177, 352, 511 },
};
u16 D_WSTAG570_800A628C[8] = { 0x708D, 1, 0x20C, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG570_800A629C[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG570_800A62A8[4] = { 0x9403, 1, 0xFFFF, 0 };
u16 D_WSTAG570_800A62B0[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG570_800A62BC[4] = { 0x9406, 1, 0xFFFF, 0 };
u16 D_WSTAG570_800A62C4[4] = { 0x940D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG570_800A62CC[2] = { { NULL, D_WSTAG570_800A628C, 812 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG570_800A62E4[3] = {
    { D_WSTAG570_800A629C, D_WSTAG570_800A62A8, 36 }, { D_WSTAG570_800A62B0, D_WSTAG570_800A62BC, 36 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG570_800A6308[2] = { { NULL, D_WSTAG570_800A62C4, 36 }, { NULL, NULL, 0 } };
u16 D_WSTAG570_800A6320[4] = { 0x20C, 0, 0xFFFF, 0 };
u16 D_WSTAG570_800A6328[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG570_800A6330[4] = { 0x802A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG570_800A6338 = { D_WSTAG570_800A6320, D_WSTAG570_800A62CC, 33, 4, 335, 718, 1 };
FieldstgPlacedActor D_WSTAG570_800A634C = { D_WSTAG570_800A6328, D_WSTAG570_800A62E4, 135, 5, 730, 440, 7 };
FieldstgPlacedActor D_WSTAG570_800A6360 = { D_WSTAG570_800A6330, D_WSTAG570_800A6308, 135, 5, 730, 440, 7 };
FieldstgPlacedActor *wstag570_actors[4] = {
    &D_WSTAG570_800A6338, &D_WSTAG570_800A634C, &D_WSTAG570_800A6360, NULL,
};
FieldstgSprite wstag570_sprites[69] = {
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 155, 293, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 859, 479, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 394, 624, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 314, 811, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 343, 836, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 571, 789, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 713, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 754, 749, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 760, 813, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 862, 757, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 281, 838, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 424, 826, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 737, 824, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 741, 714, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 278, 818, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 418, 838, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 506, 825, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 588, 791, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 708, 743, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 785, 696, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 845, 683, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 214, 803, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 256, 795, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 365, 833, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 515, 794, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 661, 814, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 818, 774, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 847, 673, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 862, 812, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 232, 815, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 319, 837, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 441, 821, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 547, 792, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 600, 775, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 679, 824, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 769, 690, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 258, 843, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 401, 829, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 467, 829, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 626, 784, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 679, 743, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 706, 843, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 771, 759, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 795, 774, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 820, 672, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 277, 846, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 377, 840, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 388, 831, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 388, 881, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 434, 838, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 511, 810, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 553, 800, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 607, 789, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 621, 874, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 627, 879, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 654, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 669, 891, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 730, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 754, 830, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 758, 821, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 777, 767, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 839, 677, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 853, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 864, 765, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 869, 681, 0, 0 },
    { 1, 0, 0x40, 6, 7, 1, 7, 0xC, 4, 0, 541, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0xD, 1, 0xD, 0x12, 4, 0, 480, 468, 0, 0 },
    { 1, 0, 0x5D, 4, 0, 0, 0, 0, 0, 0, 621, 413, 490, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag570_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24A, 0x570, 0x3C0, 3, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xA, 2 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 3, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1C0, 0x272, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1CF, 0x2D8, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE7, 0x46, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x19, 0x46, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 3, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag570_funcs = { wstag570_setup };
