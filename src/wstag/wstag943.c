#include "wstag.h"

/* WSTAG943: stage 0x28F (fieldstg_stages_2d). */

extern WstagFuncs wstag943_funcs;
const CVECTOR wstag943_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgVramPlace wstag943_vram_places[];
extern FieldstgPlacedActor *wstag943_actors[];
extern FieldstgSprite wstag943_sprites[];
extern FieldstgMapEvent wstag943_map_events[];
extern FieldstgBattleLists wstag943_battle_lists;

void wstag943_update(WstagObject *obj) {
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

WstagObject *wstag943_start(void *arg0) {
    WstagObject *obj = object_new(wstag943_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag943_funcs.setup();
    return obj;
}

void wstag943_setup(void) {
    fieldstg_stage.background_file = 0x3A5;
    fieldstg_stage.sprite_file = 0x090D0000;
    fieldstg_stage.sprites = wstag943_sprites;
    fieldstg_stage.map_events = wstag943_map_events;
    fieldstg_stage.mask_file = 0x90C;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x6E00, 0xB600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag943_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag943_actors;
    fieldstg_stage.color = wstag943_color;
    fieldstg_stage.battle_lists = &wstag943_battle_lists;
    fieldstg_attr.set_file(0, 0x090D0002);
    fieldstg_attr.set_file(7, 0x090D0003);
    fieldstg_attr.set_file(4, 0x090D0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag943_setup(void);

FieldstgVramPlace wstag943_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 311, 216, 55, 336, 511 }, { 320, 256, 374, 375, 216, 119, 352, 511 },
};
u16 D_WSTAG943_800A602C[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A6038[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A6044[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A6050[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A6060[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A606C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A6074[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A6080[4] = { 0x782B, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A6088[4] = { 5, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A6090[4] = { 5, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A6098[6] = { 5, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A60A4[8] = { 5, 1, 0x8004, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A60B4[6] = { 0x8004, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG943_800A60C0[8] = { 5, 1, 0x8004, 1, 0x8192, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG943_800A60D0[2] = { { NULL, NULL, 37 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG943_800A60E8[5] = {
    { D_WSTAG943_800A602C, D_WSTAG943_800A6038, 39 }, { D_WSTAG943_800A6044, D_WSTAG943_800A6050, 40 },
    { D_WSTAG943_800A6060, D_WSTAG943_800A606C, 37 }, { D_WSTAG943_800A6074, D_WSTAG943_800A6080, 38 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG943_800A6124[5] = {
    { D_WSTAG943_800A6088, D_WSTAG943_800A6090, 122 }, { D_WSTAG943_800A6098, NULL, 124 },
    { D_WSTAG943_800A60A4, D_WSTAG943_800A60B4, 123 }, { D_WSTAG943_800A60C0, NULL, 124 }, { NULL, NULL, 0 },
};
u16 D_WSTAG943_800A6160[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG943_800A6168[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG943_800A6170 = { D_WSTAG943_800A6160, D_WSTAG943_800A60D0, 49, 4, 656, 648, 7 };
FieldstgPlacedActor D_WSTAG943_800A6184 = { D_WSTAG943_800A6168, D_WSTAG943_800A60E8, 49, 4, 656, 648, 7 };
FieldstgPlacedActor D_WSTAG943_800A6198 = { NULL, D_WSTAG943_800A6124, 283, 5, 967, 901, 1 };
FieldstgPlacedActor *wstag943_actors[4] = {
    &D_WSTAG943_800A6170, &D_WSTAG943_800A6184, &D_WSTAG943_800A6198, NULL,
};
FieldstgSprite wstag943_sprites[49] = {
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 330, 524, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 922, 950, 0, 0 },
    { 1, 0, 0x80, 2, 7, 0, 0, 0, 0, 0, 256, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 176, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 212, 597, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 534, 857, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 646, 785, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 820, 685, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 874, 498, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 139, 93, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 148, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 154, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 460, 421, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 651, 773, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 750, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 842, 515, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 185, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 291, 768, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 398, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 480, 897, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 608, 960, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 693, 1010, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 213, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 422, 830, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 462, 898, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 569, 865, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 647, 985, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 680, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 165, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 170, 169, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 258, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 323, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 434, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 595, 944, 0, 0 },
    { 1, 0, 0xFF, 6, 8, 0, 0, 0, 0, 0, 703, 232, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 593, 792, 804, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 623, 849, 864, 0 }, { 1, 0, 0x80, 4, 9, 0, 0, 0, 0, 0, 631, 378, 433, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 333, 235, 235, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 381, 259, 259, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 429, 283, 283, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 481, 532, 532, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 486, 775, 775, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 557, 557, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 533, 751, 751, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 541, 699, 699, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 574, 580, 580, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1014, 879, 879, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag943_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x1D2, 0x92, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xC, 0x231, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xC, 0x221, 0x178, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 5, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 0, 0, 5, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 0, 0, 3, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFA2, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x28, 0x20, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x29, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag943_funcs = { wstag943_setup };
FieldstgListedBattle D_WSTAG943_800A6684 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6690 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A669C = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A66A8 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A66B4 = { 45, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A66C0 = { 45, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A66CC = { 45, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A66D8 = { 45, 13, 0x60080000 };
FieldstgBattleList D_WSTAG943_800A66E4 = {
    3,
    { &D_WSTAG943_800A6684, &D_WSTAG943_800A6690, &D_WSTAG943_800A669C, &D_WSTAG943_800A66A8, &D_WSTAG943_800A66B4,
        &D_WSTAG943_800A66C0, &D_WSTAG943_800A66CC, &D_WSTAG943_800A66D8 },
};
FieldstgListedBattle D_WSTAG943_800A6708 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6714 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6720 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A672C = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6738 = { 52, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6744 = { 52, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6750 = { 52, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A675C = { 52, 4, 0x60080000 };
FieldstgBattleList D_WSTAG943_800A6768 = {
    3,
    { &D_WSTAG943_800A6708, &D_WSTAG943_800A6714, &D_WSTAG943_800A6720, &D_WSTAG943_800A672C, &D_WSTAG943_800A6738,
        &D_WSTAG943_800A6744, &D_WSTAG943_800A6750, &D_WSTAG943_800A675C },
};
FieldstgListedBattle D_WSTAG943_800A678C = { 65, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6798 = { 65, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67A4 = { 65, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67B0 = { 65, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67BC = { 60, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67C8 = { 60, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67D4 = { 60, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A67E0 = { 60, 2, 0x60080000 };
FieldstgBattleList D_WSTAG943_800A67EC = {
    3,
    { &D_WSTAG943_800A678C, &D_WSTAG943_800A6798, &D_WSTAG943_800A67A4, &D_WSTAG943_800A67B0, &D_WSTAG943_800A67BC,
        &D_WSTAG943_800A67C8, &D_WSTAG943_800A67D4, &D_WSTAG943_800A67E0 },
};
FieldstgListedBattle D_WSTAG943_800A6810 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG943_800A681C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG943_800A6828 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG943_800A6834 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6840 = { 332, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A684C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG943_800A6858 = { 156, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG943_800A6864 = { 179, 2, 0x60080000 };
FieldstgBattleList D_WSTAG943_800A6870 = {
    0,
    { &D_WSTAG943_800A6810, &D_WSTAG943_800A681C, &D_WSTAG943_800A6828, &D_WSTAG943_800A6834, &D_WSTAG943_800A6840,
        &D_WSTAG943_800A684C, &D_WSTAG943_800A6858, &D_WSTAG943_800A6864 },
};
FieldstgBattleLists wstag943_battle_lists = {
    382, 0, 0, { &D_WSTAG943_800A66E4, &D_WSTAG943_800A6768, &D_WSTAG943_800A67EC }, &D_WSTAG943_800A6870,
};
