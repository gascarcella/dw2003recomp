#include "wstag.h"

/* WSTAG921: stage 0x271 (fieldstg_stages_2d). */

extern WstagFuncs wstag921_funcs;
extern CVECTOR wstag921_color;
extern FieldstgVramPlace wstag921_vram_places[];
extern FieldstgPlacedActor *wstag921_actors[];
extern FieldstgSprite wstag921_sprites[];
extern FieldstgMapEvent wstag921_map_events[];
extern FieldstgEventDef wstag921_events[];
extern FieldstgBattleLists wstag921_battle_lists;

void wstag921_update(WstagObject *obj) {
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

WstagObject *wstag921_start(void *arg0) {
    WstagObject *obj = object_new(wstag921_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag921_funcs.setup();
    return obj;
}

void wstag921_setup(void) {
    fieldstg_stage.background_file = 0x338;
    fieldstg_stage.sprites = wstag921_sprites;
    fieldstg_stage.map_events = wstag921_map_events;
    fieldstg_stage.sprite_file = 0x08E30003;
    fieldstg_stage.mask_file = 0x8E1;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13500, 0x12500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag921_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag921_actors;
    fieldstg_stage.color = wstag921_color;
    fieldstg_stage.events = wstag921_events;
    fieldstg_stage.battle_lists = &wstag921_battle_lists;
    fieldstg_attr.set_file(0, 0x08E30004);
    fieldstg_attr.set_file(7, 0x08E30005);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag921/nonmatchings/wstag921", wstag921_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag921_setup(void);

FieldstgVramPlace wstag921_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 396, 369, 304, 113, 368, 510 }, { 384, 256, 438, 256, 472, 0, 368, 509 },
    { 384, 256, 438, 296, 472, 40, 368, 508 }, { 384, 256, 432, 336, 448, 80, 320, 507 },
    { 384, 256, 422, 355, 408, 99, 336, 507 }, { 384, 256, 384, 373, 256, 117, 352, 507 },
};
u16 D_WSTAG921_800A606C[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG921_800A6078[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG921_800A6084[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG921_800A6090[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG921_800A60A0[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG921_800A60AC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG921_800A60B4[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG921_800A60C0[4] = { 0x7853, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG921_800A60C8[2] = { { NULL, NULL, 21 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG921_800A60E0[2] = { { NULL, NULL, 13 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG921_800A60F8[5] = {
    { D_WSTAG921_800A606C, D_WSTAG921_800A6078, 15 }, { D_WSTAG921_800A6084, D_WSTAG921_800A6090, 16 },
    { D_WSTAG921_800A60A0, D_WSTAG921_800A60AC, 13 }, { D_WSTAG921_800A60B4, D_WSTAG921_800A60C0, 14 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG921_800A6134[2] = { { NULL, NULL, 20 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG921_800A614C[2] = { { NULL, NULL, 19 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG921_800A6164[2] = { { NULL, NULL, 17 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG921_800A617C[2] = { { NULL, NULL, 18 }, { NULL, NULL, 0 } };
u16 D_WSTAG921_800A6194[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG921_800A619C[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG921_800A61A4 = { NULL, D_WSTAG921_800A60C8, 46, 4, 648, 396, 5 };
FieldstgPlacedActor D_WSTAG921_800A61B8 = { D_WSTAG921_800A6194, D_WSTAG921_800A60E0, 48, 5, 534, 435, 1 };
FieldstgPlacedActor D_WSTAG921_800A61CC = { D_WSTAG921_800A619C, D_WSTAG921_800A60F8, 48, 5, 534, 435, 1 };
FieldstgPlacedActor D_WSTAG921_800A61E0 = { NULL, D_WSTAG921_800A6134, 49, 6, 460, 203, 1 };
FieldstgPlacedActor D_WSTAG921_800A61F4 = { NULL, D_WSTAG921_800A614C, 54, 7, 672, 304, 5 };
FieldstgPlacedActor D_WSTAG921_800A6208 = { NULL, D_WSTAG921_800A6164, 136, 8, 208, 344, 5 };
FieldstgPlacedActor D_WSTAG921_800A621C = { NULL, D_WSTAG921_800A617C, 379, 9, 680, 380, 1 };
FieldstgPlacedActor *wstag921_actors[8] = {
    &D_WSTAG921_800A61A4, &D_WSTAG921_800A61B8, &D_WSTAG921_800A61CC, &D_WSTAG921_800A61E0, &D_WSTAG921_800A61F4,
    &D_WSTAG921_800A6208, &D_WSTAG921_800A621C, NULL,
};
FieldstgSprite wstag921_sprites[117] = {
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 174, 210, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 244, 175, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 840, 188, 0, 0 }, { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 886, 211, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 6, 0, 341, 183, 0, 0 }, { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 98, 114, 0, 0 },
    { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 168, 256, 0, 0 }, { 1, 0, 0x40, 2, 0xE, 0, 0, 0, 0, 0, 846, 150, 0, 0 },
    { 1, 0, 0x58, 2, 0xF, 0, 0, 0, 0, 0, 426, 256, 0, 0 }, { 1, 0, 0x50, 2, 0x10, 0, 0, 0, 0, 0, 323, 384, 0, 0 },
    { 1, 0, 0x40, 2, 0x11, 0, 0, 0, 0, 0, 640, 256, 0, 0 }, { 1, 0, 0x60, 2, 0x12, 0, 0, 0, 0, 0, 548, 202, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 676, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 386, 160, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 498, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 123, 295, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 247, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 571, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 632, 171, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 654, 417, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 684, 140, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 719, 379, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 726, 101, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 772, 330, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 793, 216, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 994, 283, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 705, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 706, 129, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 718, 471, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 741, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 757, 70, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 764, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 799, 58, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 855, 294, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 912, 279, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 119, 307, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 203, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 217, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 283, 332, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 598, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 647, 259, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 689, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 691, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 726, 257, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 742, 457, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 771, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 798, 48, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 810, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 819, 305, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 895, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 84, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 116, 353, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 277, 443, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 306, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 399, 464, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 610, 94, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 673, 147, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 769, 210, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 776, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 802, 323, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 835, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 984, 292, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 199, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 416, 464, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 533, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 589, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 659, 135, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 749, 190, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 790, 408, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 97, 297, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 143, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 169, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 185, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 311, 450, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 465, 474, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 631, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 708, 421, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 748, 169, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 758, 250, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 803, 312, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 820, 208, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 821, 53, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 822, 406, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 961, 283, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 140, 346, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 256, 443, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 360, 456, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 400, 290, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 422, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 579, 126, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 594, 125, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 594, 201, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 596, 187, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 610, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 611, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 620, 136, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 633, 468, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 669, 271, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 699, 272, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 737, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 738, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 756, 153, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 771, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 782, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 784, 320, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 819, 213, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 863, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 908, 242, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1023, 270, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 350, 152, 0, 0 },
    { 1, 0x65, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 463, 106, 0, 0 },
    { 1, 0x66, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 878, 199, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 224, 268, 289, 0 },
    { 1, 0, 0x41, 4, 1, 0, 0, 0, 0, 0, 336, 159, 216, 0 }, { 1, 0, 0x46, 4, 2, 0, 0, 0, 0, 0, 448, 103, 162, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 891, 200, 247, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag921_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x68, 0x134, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x282, 0x218, 0xF4, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x280, 0x216, 0xE2, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27F, 0x98, 0x14A, 5, 0x66, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x18, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x1C, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag921_funcs = { wstag921_setup };
FieldstgEventDef wstag921_events[1] = { { -1, NULL, 0, NULL, NULL } };
FieldstgListedBattle D_WSTAG921_800A6B64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6B70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6B7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6B88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6B94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6BA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6BAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6BB8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG921_800A6BC4 = {
    0,
    { &D_WSTAG921_800A6B64, &D_WSTAG921_800A6B70, &D_WSTAG921_800A6B7C, &D_WSTAG921_800A6B88, &D_WSTAG921_800A6B94,
        &D_WSTAG921_800A6BA0, &D_WSTAG921_800A6BAC, &D_WSTAG921_800A6BB8 },
};
FieldstgListedBattle D_WSTAG921_800A6BE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6BF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C3C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG921_800A6C48 = {
    0,
    { &D_WSTAG921_800A6BE8, &D_WSTAG921_800A6BF4, &D_WSTAG921_800A6C00, &D_WSTAG921_800A6C0C, &D_WSTAG921_800A6C18,
        &D_WSTAG921_800A6C24, &D_WSTAG921_800A6C30, &D_WSTAG921_800A6C3C },
};
FieldstgListedBattle D_WSTAG921_800A6C6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6C9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6CA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6CB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6CC0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG921_800A6CCC = {
    0,
    { &D_WSTAG921_800A6C6C, &D_WSTAG921_800A6C78, &D_WSTAG921_800A6C84, &D_WSTAG921_800A6C90, &D_WSTAG921_800A6C9C,
        &D_WSTAG921_800A6CA8, &D_WSTAG921_800A6CB4, &D_WSTAG921_800A6CC0 },
};
FieldstgListedBattle D_WSTAG921_800A6CF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6CFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6D08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6D14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6D20 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG921_800A6D2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6D38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG921_800A6D44 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG921_800A6D50 = {
    0,
    { &D_WSTAG921_800A6CF0, &D_WSTAG921_800A6CFC, &D_WSTAG921_800A6D08, &D_WSTAG921_800A6D14, &D_WSTAG921_800A6D20,
        &D_WSTAG921_800A6D2C, &D_WSTAG921_800A6D38, &D_WSTAG921_800A6D44 },
};
FieldstgBattleLists wstag921_battle_lists = {
    394, 0, 0, { &D_WSTAG921_800A6BC4, &D_WSTAG921_800A6C48, &D_WSTAG921_800A6CCC }, &D_WSTAG921_800A6D50,
};
