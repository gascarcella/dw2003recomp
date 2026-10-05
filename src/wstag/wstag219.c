#include "wstag.h"

/* WSTAG219: stage 0x274 (fieldstg_stages). */

extern WstagFuncs wstag219_funcs;
extern CVECTOR wstag219_color;
extern FieldstgBattleLists wstag219_battle_lists;
extern FieldstgVramPlace wstag219_vram_places[];
extern FieldstgSprite wstag219_sprites[];
extern FieldstgMapEvent wstag219_map_events[];

void wstag219_update(WstagObject *obj) {
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

WstagObject *wstag219_start(void *arg0) {
    WstagObject *obj = object_new(wstag219_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag219_funcs.setup();
    return obj;
}

void wstag219_setup(void) {
    fieldstg_stage.background_file = 0x532;
    fieldstg_stage.sprite_file = 0x05330000;
    fieldstg_stage.sprites = wstag219_sprites;
    fieldstg_stage.map_events = wstag219_map_events;
    fieldstg_stage.mask_file = 0x531;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xE400, 0xD700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag219_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.color = wstag219_color;
    fieldstg_stage.battle_lists = &wstag219_battle_lists;
    fieldstg_attr.set_file(0, 0x05330001);
    fieldstg_attr.set_file(7, 0x05330002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag219/nonmatchings/wstag219", wstag219_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag219_setup(void);

FieldstgListedBattle D_WSTAG219_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A5FDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A5FE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A5FF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6024 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG219_800A6030 = {
    0,
    { &D_WSTAG219_800A5FD0, &D_WSTAG219_800A5FDC, &D_WSTAG219_800A5FE8, &D_WSTAG219_800A5FF4, &D_WSTAG219_800A6000,
        &D_WSTAG219_800A600C, &D_WSTAG219_800A6018, &D_WSTAG219_800A6024 },
};
FieldstgListedBattle D_WSTAG219_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A606C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6078 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A60A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG219_800A60B4 = {
    0,
    { &D_WSTAG219_800A6054, &D_WSTAG219_800A6060, &D_WSTAG219_800A606C, &D_WSTAG219_800A6078, &D_WSTAG219_800A6084,
        &D_WSTAG219_800A6090, &D_WSTAG219_800A609C, &D_WSTAG219_800A60A8 },
};
FieldstgListedBattle D_WSTAG219_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A612C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG219_800A6138 = {
    0,
    { &D_WSTAG219_800A60D8, &D_WSTAG219_800A60E4, &D_WSTAG219_800A60F0, &D_WSTAG219_800A60FC, &D_WSTAG219_800A6108,
        &D_WSTAG219_800A6114, &D_WSTAG219_800A6120, &D_WSTAG219_800A612C },
};
FieldstgListedBattle D_WSTAG219_800A615C = { 195, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG219_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG219_800A61B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG219_800A61BC = {
    0,
    { &D_WSTAG219_800A615C, &D_WSTAG219_800A6168, &D_WSTAG219_800A6174, &D_WSTAG219_800A6180, &D_WSTAG219_800A618C,
        &D_WSTAG219_800A6198, &D_WSTAG219_800A61A4, &D_WSTAG219_800A61B0 },
};
FieldstgBattleLists wstag219_battle_lists = {
    144, 0, 0, { &D_WSTAG219_800A6030, &D_WSTAG219_800A60B4, &D_WSTAG219_800A6138 }, &D_WSTAG219_800A61BC,
};
FieldstgVramPlace wstag219_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag219_sprites[26] = {
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 561, 160, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 664, 211, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 6, 0, 903, 374, 0, 0 }, { 1, 0, 0x80, 2, 8, 0, 0, 0, 0, 0, 896, 256, 0, 0 },
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 896, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 53, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 99, 414, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 149, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x49, 2, 0, 3, 6, 0, 639, 463, 0, 0 }, { 1, 0, 0x40, 6, 0x49, 2, 0, 3, 6, 0, 847, 344, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 239, 331, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 285, 308, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 905, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 762, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 987, 453, 0, 0 },
    { 1, 0x64, 0x40, 6, 4, 0, 0, 0, 0, 0, 351, 103, 0, 0 }, { 1, 0x65, 0x40, 6, 5, 0, 0, 0, 0, 0, 78, 169, 0, 0 },
    { 1, 0x66, 0x40, 6, 6, 0, 0, 0, 0, 0, 846, 246, 0, 0 }, { 1, 0x67, 0x40, 6, 7, 0, 0, 0, 0, 0, 879, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 6, 0, 707, 316, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 6, 0, 719, 338, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 64, 196, 224, 0 }, { 1, 0, 0x44, 4, 1, 0, 0, 0, 0, 0, 377, 108, 163, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 813, 256, 304, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 905, 344, 394, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag219_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x285, 0x1E0, 0x118, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x286, 0x78, 0xFC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x286, 0x508, 0x1CC, 3, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x284, 0x1A8, 0x1F4, 5, 0x67, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag219_funcs = { wstag219_setup };
