#include "wstag.h"

/* WSTAG581: stage 0x2B7 (fieldstg_stages). */

extern WstagFuncs wstag581_funcs;
extern FieldstgBattleLists wstag581_battle_lists;
extern FieldstgVramPlace wstag581_vram_places[];
extern FieldstgSprite wstag581_sprites[];
extern FieldstgMapEvent wstag581_map_events[];

void wstag581_update(WstagObject *obj) {
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

WstagObject *wstag581_start(void *arg0) {
    WstagObject *obj = object_new(wstag581_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag581_funcs.setup();
    return obj;
}

void wstag581_setup(void) {
    fieldstg_stage.background_file = 0x779;
    fieldstg_stage.sprite_file = 0x077A0000;
    fieldstg_stage.sprites = wstag581_sprites;
    fieldstg_stage.map_events = wstag581_map_events;
    fieldstg_stage.mask_file = 0x778;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10400, 0x3AD00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag581_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.battle_lists = &wstag581_battle_lists;
    fieldstg_attr.set_file(0, 0x077A0001);
    fieldstg_attr.set_file(7, 0x077A0002);
    fieldstg_attr.set_file(4, 0x077A0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag581_setup(void);

FieldstgListedBattle D_WSTAG581_800A5F88 = { 87, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5F94 = { 87, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FA0 = { 87, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FAC = { 139, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FB8 = { 139, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FC4 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FD0 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG581_800A5FDC = { 164, 12, 0x60080000 };
FieldstgBattleList D_WSTAG581_800A5FE8 = {
    2,
    { &D_WSTAG581_800A5F88, &D_WSTAG581_800A5F94, &D_WSTAG581_800A5FA0, &D_WSTAG581_800A5FAC, &D_WSTAG581_800A5FB8,
        &D_WSTAG581_800A5FC4, &D_WSTAG581_800A5FD0, &D_WSTAG581_800A5FDC },
};
FieldstgListedBattle D_WSTAG581_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6060 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG581_800A606C = {
    0,
    { &D_WSTAG581_800A600C, &D_WSTAG581_800A6018, &D_WSTAG581_800A6024, &D_WSTAG581_800A6030, &D_WSTAG581_800A603C,
        &D_WSTAG581_800A6048, &D_WSTAG581_800A6054, &D_WSTAG581_800A6060 },
};
FieldstgListedBattle D_WSTAG581_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A60E4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG581_800A60F0 = {
    0,
    { &D_WSTAG581_800A6090, &D_WSTAG581_800A609C, &D_WSTAG581_800A60A8, &D_WSTAG581_800A60B4, &D_WSTAG581_800A60C0,
        &D_WSTAG581_800A60CC, &D_WSTAG581_800A60D8, &D_WSTAG581_800A60E4 },
};
FieldstgListedBattle D_WSTAG581_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG581_800A6168 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG581_800A6174 = {
    0,
    { &D_WSTAG581_800A6114, &D_WSTAG581_800A6120, &D_WSTAG581_800A612C, &D_WSTAG581_800A6138, &D_WSTAG581_800A6144,
        &D_WSTAG581_800A6150, &D_WSTAG581_800A615C, &D_WSTAG581_800A6168 },
};
FieldstgBattleLists wstag581_battle_lists = {
    106, 0, 0, { &D_WSTAG581_800A5FE8, &D_WSTAG581_800A606C, &D_WSTAG581_800A60F0 }, &D_WSTAG581_800A6174,
};
FieldstgVramPlace wstag581_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag581_sprites[53] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 249, 482, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 273, 494, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 720, 847, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 745, 378, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 769, 390, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 891, 346, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 913, 750, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 984, 754, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 1269, 276, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 889, 739, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 960, 766, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 1234, 293, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 1, 6, 0, 946, 161, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 787, 181, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 867, 829, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 1043, 353, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 1268, 657, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 6, 0, 236, 78, 0, 0 },
    { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 6, 0, 1092, 186, 0, 0 }, { 1, 0, 0x47, 2, 0xE, 0, 0, 0, 0, 0, 799, 790, 0, 0 },
    { 1, 0, 0x41, 2, 0xF, 0, 0, 0, 0, 0, 727, 809, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 9, 4, 0, 696, 835, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 241, 879, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 305, 559, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 1025, 775, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 217, 891, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 281, 571, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 1001, 787, 0, 0 },
    { 1, 0x6B, 0x40, 6, 7, 0, 0, 0, 0, 0, 93, 496, 0, 0 }, { 1, 0x6A, 0x40, 6, 8, 0, 0, 0, 0, 0, 1260, 665, 0, 0 },
    { 1, 0x69, 0x40, 6, 9, 0, 0, 0, 0, 0, 1036, 355, 0, 0 },
    { 1, 0x68, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 1086, 196, 0, 0 },
    { 1, 0x67, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 943, 171, 0, 0 },
    { 1, 0x66, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 783, 191, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x11, 0, 0, 0, 0, 0, 859, 837, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 230, 86, 0, 0 }, { 1, 0, 0x80, 6, 0x19, 0, 0, 0, 0, 0, 876, 192, 0, 0 },
    { 1, 0, 0x40, 4, 0x33, 2, 0, 9, 4, 0, 401, 463, 491, 0 },
    { 1, 0, 0x40, 4, 0x34, 2, 0, 9, 4, 0, 377, 475, 497, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 397, 464, 491, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 381, 476, 497, 0 },
    { 1, 0, 0x80, 4, 0x16, 0, 0, 0, 0, 0, 891, 116, 221, 0 },
    { 1, 0, 0x80, 4, 0x17, 0, 0, 0, 0, 0, 899, 112, 207, 0 },
    { 1, 0, 0x80, 4, 0x18, 0, 0, 0, 0, 0, 907, 108, 203, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 64, 512, 552, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1071, 379, 415, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 1049, 211, 247, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 975, 187, 224, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 815, 207, 239, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 894, 347, 370, 0 }, { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 896, 855, 888, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 208, 103, 138, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag581_map_events[25] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B6, 0x228, 0x94, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B8, 0x108, 0x174, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B9, 0x108, 0x174, 5, 0x6A, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BC, 0x2F8, 0x264, 3, 0x68, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BA, 0x108, 0x174, 5, 0x69, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BC, 0x258, 0x24C, 5, 0x67, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BB, 0x108, 0x174, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BC, 0x68, 0x20C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BD, 0x278, 0xD4, 3, 0x6B, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x160, 0x80, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x170, 0xE8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xC, 0xDF, 0x142, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xC, 0xD2, 0x208, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x2D0, 0x29A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x2C1, 0x322, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x33E, 0x2D2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x330, 0x35A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x42E, 0x34A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x420, 0x3D2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x4B0, 0x25A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x4BE, 0x2AE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag581_funcs = { wstag581_setup };
