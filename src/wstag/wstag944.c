#include "wstag.h"

/* WSTAG944: stage 0x290 (fieldstg_stages_2d). */

extern WstagFuncs wstag944_funcs;
extern FieldstgVramPlace wstag944_vram_places[];
extern FieldstgPlacedActor *wstag944_actors[];
extern FieldstgSprite wstag944_sprites[];
extern FieldstgMapEvent wstag944_map_events[];
extern FieldstgBattleLists wstag944_battle_lists;

void wstag944_update(WstagObject *obj) {
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

WstagObject *wstag944_start(void *arg0) {
    WstagObject *obj = object_new(wstag944_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag944_funcs.setup();
    return obj;
}

void wstag944_setup(void) {
    fieldstg_stage.background_file = 0x1B8;
    fieldstg_stage.sprite_file = 0x090F0000;
    fieldstg_stage.sprites = wstag944_sprites;
    fieldstg_stage.map_events = wstag944_map_events;
    fieldstg_stage.mask_file = 0x90E;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2D600, 0x8000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag944_vram_places;
    fieldstg_stage.music = 0x2D;
    fieldstg_stage.sound = 0x60B40000;
    fieldstg_stage.actors = wstag944_actors;
    fieldstg_stage.battle_lists = &wstag944_battle_lists;
    fieldstg_attr.set_file(0, 0x090F0002);
    fieldstg_attr.set_file(7, 0x090F0003);
    fieldstg_attr.set_file(4, 0x090F0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag944_setup(void);

FieldstgVramPlace wstag944_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 296, 216, 40, 336, 511 }, { 320, 256, 374, 256, 216, 0, 352, 511 },
    { 320, 256, 364, 256, 176, 0, 368, 511 },
};
u16 D_WSTAG944_800A6020[8] = { 0x26A, 1, 0x8239, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG944_800A6030[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A603C[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A6048[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG944_800A6054[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A6064[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A6070[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG944_800A6078[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG944_800A6084[4] = { 0x7833, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG944_800A608C[2] = { { NULL, D_WSTAG944_800A6020, 2 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG944_800A60A4[2] = { { NULL, NULL, 41 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG944_800A60BC[5] = {
    { D_WSTAG944_800A6030, D_WSTAG944_800A603C, 43 }, { D_WSTAG944_800A6048, D_WSTAG944_800A6054, 44 },
    { D_WSTAG944_800A6064, D_WSTAG944_800A6070, 41 }, { D_WSTAG944_800A6078, D_WSTAG944_800A6084, 42 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG944_800A60F8[2] = { { NULL, NULL, 125 }, { NULL, NULL, 0 } };
u16 D_WSTAG944_800A6110[4] = { 0x26A, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A6118[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG944_800A6120[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG944_800A6128 = { D_WSTAG944_800A6110, D_WSTAG944_800A608C, 33, 4, 1353, 405, 1 };
FieldstgPlacedActor D_WSTAG944_800A613C = { D_WSTAG944_800A6118, D_WSTAG944_800A60A4, 47, 5, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG944_800A6150 = { D_WSTAG944_800A6120, D_WSTAG944_800A60BC, 47, 5, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG944_800A6164 = { NULL, D_WSTAG944_800A60F8, 138, 6, 241, 297, 1 };
FieldstgPlacedActor *wstag944_actors[5] = {
    &D_WSTAG944_800A6128, &D_WSTAG944_800A613C, &D_WSTAG944_800A6150, &D_WSTAG944_800A6164, NULL,
};
FieldstgSprite wstag944_sprites[45] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 313, 257, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 730, 365, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 932, 55, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 161, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 83, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 50, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 182, 431, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 196, 377, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 208, 446, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 188, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 230, 394, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 273, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 129, 383, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x44, 0x46, 0xA, 0, 196, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 248, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 272, 461, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 441, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 488, 363, 363, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 527, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 584, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 632, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 679, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 687, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 760, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 808, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 856, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 880, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 463, 463, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 440, 440, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1037, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1079, 395, 395, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1144, 387, 387, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1192, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1240, 435, 435, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 455, 455, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1304, 499, 499, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1336, 531, 531, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1368, 355, 355, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1380, 553, 553, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1411, 376, 376, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag944_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28D, 0x28C, 0x33C, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x8C, 0xCE, 7, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x38, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag944_funcs = { wstag944_setup };
FieldstgListedBattle D_WSTAG944_800A6534 = { 69, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6540 = { 69, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A654C = { 154, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6558 = { 154, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6564 = { 70, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6570 = { 70, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A657C = { 67, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6588 = { 67, 13, 0x60080000 };
FieldstgBattleList D_WSTAG944_800A6594 = {
    3,
    { &D_WSTAG944_800A6534, &D_WSTAG944_800A6540, &D_WSTAG944_800A654C, &D_WSTAG944_800A6558, &D_WSTAG944_800A6564,
        &D_WSTAG944_800A6570, &D_WSTAG944_800A657C, &D_WSTAG944_800A6588 },
};
FieldstgListedBattle D_WSTAG944_800A65B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A65C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A65D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A65DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A65E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A65F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A660C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG944_800A6618 = {
    0,
    { &D_WSTAG944_800A65B8, &D_WSTAG944_800A65C4, &D_WSTAG944_800A65D0, &D_WSTAG944_800A65DC, &D_WSTAG944_800A65E8,
        &D_WSTAG944_800A65F4, &D_WSTAG944_800A6600, &D_WSTAG944_800A660C },
};
FieldstgListedBattle D_WSTAG944_800A663C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6648 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6654 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6660 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A666C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6678 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6684 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6690 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG944_800A669C = {
    0,
    { &D_WSTAG944_800A663C, &D_WSTAG944_800A6648, &D_WSTAG944_800A6654, &D_WSTAG944_800A6660, &D_WSTAG944_800A666C,
        &D_WSTAG944_800A6678, &D_WSTAG944_800A6684, &D_WSTAG944_800A6690 },
};
FieldstgListedBattle D_WSTAG944_800A66C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A66CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A66D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A66E4 = { 331, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A66F0 = { 334, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A66FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG944_800A6708 = { 152, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG944_800A6714 = { 186, 8, 0x60080000 };
FieldstgBattleList D_WSTAG944_800A6720 = {
    0,
    { &D_WSTAG944_800A66C0, &D_WSTAG944_800A66CC, &D_WSTAG944_800A66D8, &D_WSTAG944_800A66E4, &D_WSTAG944_800A66F0,
        &D_WSTAG944_800A66FC, &D_WSTAG944_800A6708, &D_WSTAG944_800A6714 },
};
FieldstgBattleLists wstag944_battle_lists = {
    384, 0, 0, { &D_WSTAG944_800A6594, &D_WSTAG944_800A6618, &D_WSTAG944_800A669C }, &D_WSTAG944_800A6720,
};
