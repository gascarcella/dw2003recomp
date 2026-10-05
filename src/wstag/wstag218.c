#include "wstag.h"

/* WSTAG218: stage 0x205 (fieldstg_stages). */

extern WstagFuncs wstag218_funcs;
extern CVECTOR wstag218_color;
extern FieldstgBattleLists wstag218_battle_lists;
extern FieldstgVramPlace wstag218_vram_places[];
extern FieldstgPlacedActor *wstag218_actors[];
extern FieldstgSprite wstag218_sprites[];
extern FieldstgMapEvent wstag218_map_events[];

void wstag218_update(WstagObject *obj) {
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

WstagObject *wstag218_start(void *arg0) {
    WstagObject *obj = object_new(wstag218_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag218_funcs.setup();
    return obj;
}

void wstag218_setup(void) {
    fieldstg_stage.background_file = 0x33C;
    fieldstg_stage.sprite_file = 0x033D0000;
    fieldstg_stage.sprites = wstag218_sprites;
    fieldstg_stage.map_events = wstag218_map_events;
    fieldstg_stage.mask_file = 0x33B;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11400, 0xC100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag218_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag218_actors;
    fieldstg_stage.color = wstag218_color;
    fieldstg_stage.battle_lists = &wstag218_battle_lists;
    fieldstg_attr.set_file(0, 0x033D0001);
    fieldstg_attr.set_file(7, 0x033D0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x14 && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag218/nonmatchings/wstag218", wstag218_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag218_setup(void);

FieldstgListedBattle D_WSTAG218_800A5FE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A5FF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A5FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6038 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG218_800A6044 = {
    0,
    { &D_WSTAG218_800A5FE4, &D_WSTAG218_800A5FF0, &D_WSTAG218_800A5FFC, &D_WSTAG218_800A6008, &D_WSTAG218_800A6014,
        &D_WSTAG218_800A6020, &D_WSTAG218_800A602C, &D_WSTAG218_800A6038 },
};
FieldstgListedBattle D_WSTAG218_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A60BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG218_800A60C8 = {
    0,
    { &D_WSTAG218_800A6068, &D_WSTAG218_800A6074, &D_WSTAG218_800A6080, &D_WSTAG218_800A608C, &D_WSTAG218_800A6098,
        &D_WSTAG218_800A60A4, &D_WSTAG218_800A60B0, &D_WSTAG218_800A60BC },
};
FieldstgListedBattle D_WSTAG218_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6140 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG218_800A614C = {
    0,
    { &D_WSTAG218_800A60EC, &D_WSTAG218_800A60F8, &D_WSTAG218_800A6104, &D_WSTAG218_800A6110, &D_WSTAG218_800A611C,
        &D_WSTAG218_800A6128, &D_WSTAG218_800A6134, &D_WSTAG218_800A6140 },
};
FieldstgListedBattle D_WSTAG218_800A6170 = { 188, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG218_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A6194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A61A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A61B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG218_800A61C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG218_800A61D0 = {
    0,
    { &D_WSTAG218_800A6170, &D_WSTAG218_800A617C, &D_WSTAG218_800A6188, &D_WSTAG218_800A6194, &D_WSTAG218_800A61A0,
        &D_WSTAG218_800A61AC, &D_WSTAG218_800A61B8, &D_WSTAG218_800A61C4 },
};
FieldstgBattleLists wstag218_battle_lists = {
    133, 0, 0, { &D_WSTAG218_800A6044, &D_WSTAG218_800A60C8, &D_WSTAG218_800A614C }, &D_WSTAG218_800A61D0,
};
FieldstgVramPlace wstag218_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 344, 224, 88, 368, 510 }, { 320, 256, 376, 432, 224, 176, 368, 509 },
};
u16 D_WSTAG218_800A6290[8] = { 0x216, 1, 0x708E, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG218_800A62A0[8] = { 0x217, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG218_800A62B0[2] = { { NULL, D_WSTAG218_800A6290, 1162 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG218_800A62C8[2] = { { NULL, D_WSTAG218_800A62A0, 694 }, { NULL, NULL, 0 } };
u16 D_WSTAG218_800A62E0[4] = { 0x216, 0, 0xFFFF, 0 };
u16 D_WSTAG218_800A62E8[4] = { 0x217, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG218_800A62F0 = { D_WSTAG218_800A62E0, D_WSTAG218_800A62B0, 33, 4, 809, 389, 1 };
FieldstgPlacedActor D_WSTAG218_800A6304 = { D_WSTAG218_800A62E8, D_WSTAG218_800A62C8, 77, 5, 919, 300, 1 };
FieldstgPlacedActor *wstag218_actors[3] = { &D_WSTAG218_800A62F0, &D_WSTAG218_800A6304, NULL };
FieldstgSprite wstag218_sprites[37] = {
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 561, 160, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 664, 211, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 6, 0, 903, 374, 0, 0 }, { 1, 0, 0x80, 2, 8, 0, 0, 0, 0, 0, 896, 256, 0, 0 },
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 896, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 53, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 99, 414, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 149, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x49, 2, 0, 3, 6, 0, 639, 463, 0, 0 }, { 1, 0, 0x40, 6, 0x49, 2, 0, 3, 6, 0, 847, 344, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 239, 331, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 285, 308, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 821, 438, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 959, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 989, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 735, 352, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 928, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 929, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 813, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 876, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 944, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 985, 463, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 799, 406, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 777, 394, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 667, 411, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 677, 414, 0, 0 },
    { 1, 0x65, 0x40, 6, 4, 0, 0, 0, 0, 0, 351, 103, 0, 0 }, { 1, 0x64, 0x40, 6, 5, 0, 0, 0, 0, 0, 78, 169, 0, 0 },
    { 1, 0x66, 0x40, 6, 6, 0, 0, 0, 0, 0, 846, 246, 0, 0 }, { 1, 0x67, 0x40, 6, 7, 0, 0, 0, 0, 0, 879, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 6, 0, 707, 316, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 6, 0, 719, 338, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 64, 196, 224, 0 }, { 1, 0, 0x44, 4, 1, 0, 0, 0, 0, 0, 377, 108, 163, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 813, 256, 304, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 905, 344, 394, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag218_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x216, 0x1E0, 0x118, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x217, 0x78, 0xFC, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x217, 0x508, 0x1CC, 3, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x215, 0x1A8, 0x1F4, 5, 0x67, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag218_funcs = { wstag218_setup };
