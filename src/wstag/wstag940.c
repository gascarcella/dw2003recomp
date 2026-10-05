#include "wstag.h"

/* WSTAG940: stage 0x28C (fieldstg_stages_2d). */

extern WstagFuncs wstag940_funcs;
const CVECTOR wstag940_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgVramPlace wstag940_vram_places[];
extern FieldstgPlacedActor *wstag940_actors[];
extern FieldstgSprite wstag940_sprites[];
extern FieldstgMapEvent wstag940_map_events[];
extern FieldstgBattleLists wstag940_battle_lists;

void wstag940_update(WstagObject *obj) {
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

WstagObject *wstag940_start(void *arg0) {
    WstagObject *obj = object_new(wstag940_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag940_funcs.setup();
    return obj;
}

void wstag940_setup(void) {
    fieldstg_stage.background_file = 0x1B4;
    fieldstg_stage.sprite_file = 0x09070000;
    fieldstg_stage.sprites = wstag940_sprites;
    fieldstg_stage.map_events = wstag940_map_events;
    fieldstg_stage.mask_file = 0x906;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x38400, 0x3E800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag940_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag940_actors;
    fieldstg_stage.color = wstag940_color;
    fieldstg_stage.battle_lists = &wstag940_battle_lists;
    fieldstg_attr.set_file(0, 0x09070002);
    fieldstg_attr.set_file(7, 0x09070003);
    fieldstg_attr.set_file(4, 0x09070001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag940_setup(void);

FieldstgVramPlace wstag940_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 352, 511 }, { 320, 256, 374, 452, 216, 196, 368, 511 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 384, 256, 438, 256, 472, 0, 320, 510 }, { 384, 256, 384, 287, 256, 31, 336, 510 },
    { 384, 256, 392, 287, 288, 31, 352, 510 },
};
u16 D_WSTAG940_800A6074[8] = { 0x269, 1, 0x8232, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A6084[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A6090[6] = { 1, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A609C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A60A8[8] = { 0x11, 0, 0x10, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A60B8[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A60C4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A60CC[6] = { 0x11, 0, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A60D8[4] = { 0x7822, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A60E0[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A60EC[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A60F8[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A6104[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A6114[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A6120[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A6128[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A6134[4] = { 0x784D, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A613C[4] = { 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A6144[4] = { 0x9407, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A614C[4] = { 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A6154[4] = { 0x9408, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A615C[4] = { 0x940D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG940_800A6164[2] = { { NULL, D_WSTAG940_800A6074, 1 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG940_800A617C[2] = { { NULL, NULL, 25 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG940_800A6194[5] = {
    { D_WSTAG940_800A6084, D_WSTAG940_800A6090, 27 }, { D_WSTAG940_800A609C, D_WSTAG940_800A60A8, 28 },
    { D_WSTAG940_800A60B8, D_WSTAG940_800A60C4, 25 }, { D_WSTAG940_800A60CC, D_WSTAG940_800A60D8, 26 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG940_800A61D0[2] = { { NULL, NULL, 112 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG940_800A61E8[2] = { { NULL, NULL, 21 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG940_800A6200[5] = {
    { D_WSTAG940_800A60E0, D_WSTAG940_800A60EC, 23 }, { D_WSTAG940_800A60F8, D_WSTAG940_800A6104, 24 },
    { D_WSTAG940_800A6114, D_WSTAG940_800A6120, 21 }, { D_WSTAG940_800A6128, D_WSTAG940_800A6134, 22 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG940_800A623C[2] = { { NULL, NULL, 113 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG940_800A6254[3] = {
    { D_WSTAG940_800A613C, D_WSTAG940_800A6144, 111 }, { D_WSTAG940_800A614C, D_WSTAG940_800A6154, 111 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG940_800A6278[2] = { { NULL, D_WSTAG940_800A615C, 111 }, { NULL, NULL, 0 } };
u16 D_WSTAG940_800A6290[4] = { 0x269, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A6298[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A62A0[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A62A8[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A62B0[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG940_800A62B8[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG940_800A62C0[4] = { 0x802A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG940_800A62C8 = { D_WSTAG940_800A6290, D_WSTAG940_800A6164, 33, 4, 738, 394, 1 };
FieldstgPlacedActor D_WSTAG940_800A62DC = { D_WSTAG940_800A6298, D_WSTAG940_800A617C, 51, 5, 1072, 665, 3 };
FieldstgPlacedActor D_WSTAG940_800A62F0 = { D_WSTAG940_800A62A0, D_WSTAG940_800A6194, 51, 5, 1072, 665, 3 };
FieldstgPlacedActor D_WSTAG940_800A6304 = { NULL, D_WSTAG940_800A61D0, 63, 6, 961, 681, 1 };
FieldstgPlacedActor D_WSTAG940_800A6318 = { D_WSTAG940_800A62A8, D_WSTAG940_800A61E8, 101, 7, 864, 577, 7 };
FieldstgPlacedActor D_WSTAG940_800A632C = { D_WSTAG940_800A62B0, D_WSTAG940_800A6200, 101, 7, 864, 577, 7 };
FieldstgPlacedActor D_WSTAG940_800A6340 = { NULL, D_WSTAG940_800A623C, 135, 8, 560, 728, 7 };
FieldstgPlacedActor D_WSTAG940_800A6354 = { D_WSTAG940_800A62B8, D_WSTAG940_800A6254, 137, 9, 923, 374, 7 };
FieldstgPlacedActor D_WSTAG940_800A6368 = { D_WSTAG940_800A62C0, D_WSTAG940_800A6278, 137, 9, 923, 374, 7 };
FieldstgPlacedActor *wstag940_actors[10] = {
    &D_WSTAG940_800A62C8, &D_WSTAG940_800A62DC, &D_WSTAG940_800A62F0, &D_WSTAG940_800A6304, &D_WSTAG940_800A6318,
    &D_WSTAG940_800A632C, &D_WSTAG940_800A6340, &D_WSTAG940_800A6354, &D_WSTAG940_800A6368, NULL,
};
FieldstgSprite wstag940_sprites[129] = {
    { 1, 0, 0x40, 2, 6, 1, 6, 8, 8, 0, 787, 422, 0, 0 }, { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 202, 246, 0, 0 },
    { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 410, 878, 0, 0 }, { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 1322, 886, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 150, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 500, 1060, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 575, 940, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 681, 1030, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 715, 1004, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 870, 843, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 910, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1271, 1057, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1520, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 97, 735, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 195, 304, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 268, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 584, 151, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 668, 1135, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 738, 892, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 742, 475, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 750, 999, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1053, 1074, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1331, 461, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 182, 792, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 645, 1050, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 867, 469, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 869, 1066, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1399, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1458, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 97, 892, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 138, 930, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 145, 273, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 180, 706, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 184, 407, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 293, 496, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 386, 648, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 495, 624, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 527, 75, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 530, 1028, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 611, 1160, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 643, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 910, 1071, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1039, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1365, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1565, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 212, 806, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 232, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 250, 329, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 367, 607, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 673, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1070, 347, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1172, 1074, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1298, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 208, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 261, 826, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 317, 582, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 468, 997, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 579, 1121, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 786, 1012, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 990, 1080, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1076, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1417, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1513, 371, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 78, 891, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 113, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 133, 615, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 249, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 277, 870, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 459, 1003, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 554, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 569, 1062, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 675, 1037, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 711, 518, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 727, 511, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 751, 903, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 878, 1051, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 939, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1088, 1054, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1279, 482, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1391, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1410, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1486, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1589, 336, 0, 0 },
    { 1, 0, 0x72, 4, 0, 0, 0, 0, 0, 0, 765, 572, 665, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 951, 632, 679, 0 },
    { 1, 0, 0x7D, 4, 2, 0, 0, 0, 0, 0, 816, 122, 245, 0 }, { 1, 0, 0x7D, 4, 3, 0, 0, 0, 0, 0, 800, 114, 238, 0 },
    { 1, 0, 0x7D, 4, 4, 0, 0, 0, 0, 0, 784, 106, 230, 0 }, { 1, 0, 0x7D, 4, 5, 0, 0, 0, 0, 0, 767, 99, 223, 0 },
    { 1, 0, 0x46, 4, 0xF, 0, 0, 0, 0, 0, 1224, 424, 484, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 243, 754, 754, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 266, 606, 606, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 275, 778, 778, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 314, 630, 630, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 338, 538, 538, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 363, 654, 654, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 410, 678, 678, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 483, 930, 930, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 531, 906, 906, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 547, 674, 674, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 579, 882, 882, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 595, 650, 650, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 626, 474, 474, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 627, 858, 858, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 643, 994, 994, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 667, 454, 454, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 691, 490, 490, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 723, 826, 826, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 771, 802, 802, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 787, 938, 938, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 819, 778, 778, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 835, 194, 194, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 835, 914, 914, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 883, 890, 890, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 883, 938, 938, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 931, 914, 914, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 979, 378, 378, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 979, 890, 890, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1027, 402, 402, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1075, 426, 426, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1314, 642, 642, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1315, 690, 690, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1315, 738, 738, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 570, 570, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 618, 618, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 666, 666, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 714, 714, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 762, 762, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag940_map_events[24] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x272, 0x100, 0x1E0, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28D, 0x7C, 0x7E, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28E, 0x30A, 0x98, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28F, 0x4AC, 0x326, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x2CE, 0x124, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x2DF, 0xF0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x324, 0x170, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x333, 0x11A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 2, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 0, 0, 4, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 0, 0, 3, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 0, 0, 2, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x60, 0xFFF0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x40, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x14, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 2, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag940_funcs = { wstag940_setup };
FieldstgListedBattle D_WSTAG940_800A6EFC = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F08 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F14 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F20 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F2C = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F38 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F44 = { 41, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F50 = { 41, 1, 0x60080000 };
FieldstgBattleList D_WSTAG940_800A6F5C = {
    3,
    { &D_WSTAG940_800A6EFC, &D_WSTAG940_800A6F08, &D_WSTAG940_800A6F14, &D_WSTAG940_800A6F20, &D_WSTAG940_800A6F2C,
        &D_WSTAG940_800A6F38, &D_WSTAG940_800A6F44, &D_WSTAG940_800A6F50 },
};
FieldstgListedBattle D_WSTAG940_800A6F80 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F8C = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6F98 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6FA4 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6FB0 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6FBC = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6FC8 = { 41, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A6FD4 = { 41, 1, 0x60080000 };
FieldstgBattleList D_WSTAG940_800A6FE0 = {
    1,
    { &D_WSTAG940_800A6F80, &D_WSTAG940_800A6F8C, &D_WSTAG940_800A6F98, &D_WSTAG940_800A6FA4, &D_WSTAG940_800A6FB0,
        &D_WSTAG940_800A6FBC, &D_WSTAG940_800A6FC8, &D_WSTAG940_800A6FD4 },
};
FieldstgListedBattle D_WSTAG940_800A7004 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7010 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A701C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7028 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7034 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7040 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A704C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7058 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG940_800A7064 = {
    0,
    { &D_WSTAG940_800A7004, &D_WSTAG940_800A7010, &D_WSTAG940_800A701C, &D_WSTAG940_800A7028, &D_WSTAG940_800A7034,
        &D_WSTAG940_800A7040, &D_WSTAG940_800A704C, &D_WSTAG940_800A7058 },
};
FieldstgListedBattle D_WSTAG940_800A7088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A7094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A70A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A70AC = { 329, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A70B8 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A70C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG940_800A70D0 = { 147, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG940_800A70DC = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG940_800A70E8 = {
    0,
    { &D_WSTAG940_800A7088, &D_WSTAG940_800A7094, &D_WSTAG940_800A70A0, &D_WSTAG940_800A70AC, &D_WSTAG940_800A70B8,
        &D_WSTAG940_800A70C4, &D_WSTAG940_800A70D0, &D_WSTAG940_800A70DC },
};
FieldstgBattleLists wstag940_battle_lists = {
    380, 0, 0, { &D_WSTAG940_800A6F5C, &D_WSTAG940_800A6FE0, &D_WSTAG940_800A7064 }, &D_WSTAG940_800A70E8,
};
