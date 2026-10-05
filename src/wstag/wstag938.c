#include "wstag.h"

/* WSTAG938: stage 0x28A (fieldstg_stages_2d). */

extern WstagFuncs wstag938_funcs;
extern CVECTOR wstag938_color;
extern FieldstgVramPlace wstag938_vram_places[];
extern FieldstgPlacedActor *wstag938_actors[];
extern FieldstgSprite wstag938_sprites[];
extern FieldstgMapEvent wstag938_map_events[];
extern FieldstgBattleLists wstag938_battle_lists;

void wstag938_update(WstagObject *obj) {
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

WstagObject *wstag938_start(void *arg0) {
    WstagObject *obj = object_new(wstag938_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag938_funcs.setup();
    return obj;
}

void wstag938_setup(void) {
    fieldstg_stage.background_file = 0x2A3;
    fieldstg_stage.sprite_file = 0x09030000;
    fieldstg_stage.sprites = wstag938_sprites;
    fieldstg_stage.map_events = wstag938_map_events;
    fieldstg_stage.mask_file = 0x902;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x26F00, 0x1EF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag938_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag938_actors;
    fieldstg_stage.color = wstag938_color;
    fieldstg_stage.battle_lists = &wstag938_battle_lists;
    fieldstg_attr.set_file(0, 0x09030002);
    fieldstg_attr.set_file(7, 0x09030003);
    fieldstg_attr.set_file(4, 0x09030001);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag938/nonmatchings/wstag938", wstag938_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag938_setup(void);

FieldstgVramPlace wstag938_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 368, 501 }, { 384, 256, 420, 296, 400, 40, 320, 500 },
    { 384, 256, 428, 296, 432, 40, 352, 500 }, { 384, 256, 422, 344, 408, 88, 368, 500 },
    { 384, 256, 414, 304, 376, 48, 320, 499 },
};
u16 D_WSTAG938_800A6064[8] = { 0x267, 1, 0x8237, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG938_800A6074[8] = { 0x268, 1, 0x8238, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG938_800A6084[2] = { { NULL, D_WSTAG938_800A6064, 2 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG938_800A609C[2] = { { NULL, NULL, 103 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG938_800A60B4[2] = { { NULL, NULL, 104 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG938_800A60CC[2] = { { NULL, NULL, 105 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG938_800A60E4[2] = { { NULL, D_WSTAG938_800A6074, 3 }, { NULL, NULL, 0 } };
u16 D_WSTAG938_800A60FC[4] = { 0x267, 0, 0xFFFF, 0 };
u16 D_WSTAG938_800A6104[4] = { 0x268, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG938_800A610C = { D_WSTAG938_800A60FC, D_WSTAG938_800A6084, 33, 4, 465, 169, 1 };
FieldstgPlacedActor D_WSTAG938_800A6120 = { NULL, D_WSTAG938_800A609C, 37, 5, 336, 640, 1 };
FieldstgPlacedActor D_WSTAG938_800A6134 = { NULL, D_WSTAG938_800A60B4, 38, 6, 839, 533, 1 };
FieldstgPlacedActor D_WSTAG938_800A6148 = { NULL, D_WSTAG938_800A60CC, 56, 7, 600, 509, 1 };
FieldstgPlacedActor D_WSTAG938_800A615C = { D_WSTAG938_800A6104, D_WSTAG938_800A60E4, 77, 8, 272, 217, 1 };
FieldstgPlacedActor *wstag938_actors[6] = {
    &D_WSTAG938_800A610C, &D_WSTAG938_800A6120, &D_WSTAG938_800A6134, &D_WSTAG938_800A6148, &D_WSTAG938_800A615C,
    NULL,
};
FieldstgSprite wstag938_sprites[129] = {
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 395, 311, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 871, 533, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 1, 0x47, 0x4C, 4, 0, 1037, 703, 0, 0 },
    { 1, 0, 0x40, 2, 0x4D, 2, 0, 3, 4, 0, 1012, 671, 0, 0 }, { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 442, 116, 0, 0 },
    { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 473, 132, 0, 0 }, { 1, 0, 0x40, 2, 0x53, 2, 0, 3, 4, 0, 299, 39, 0, 0 },
    { 1, 0, 0x40, 2, 0x5A, 0, 0, 0, 0, 0, 472, 396, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 417, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 449, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 481, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 513, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 544, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x4C, 4, 0, 735, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 328, 658, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 616, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 760, 730, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 904, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 351, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 639, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 783, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 927, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 324, 632, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 612, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 756, 704, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 900, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 305, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 593, 788, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 737, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 881, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 202, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 210, 420, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 218, 424, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 226, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 234, 432, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 242, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 250, 440, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 258, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 266, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 274, 356, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 282, 360, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 290, 364, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 298, 368, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 306, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 314, 376, 0, 0 }, { 1, 0, 0x40, 6, 0x55, 2, 0, 3, 8, 0, 322, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 270, 444, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 278, 440, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 286, 436, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 294, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 302, 428, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 310, 424, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 358, 400, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 366, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 374, 392, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 382, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 390, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 3, 8, 0, 349, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 3, 4, 0, 439, 395, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 540, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x5B, 2, 0, 3, 4, 0, 612, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 646, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 688, 509, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 602, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 725, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 358, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 400, 653, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 519, 754, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 535, 458, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 560, 733, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 576, 437, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 648, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 690, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 792, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 833, 674, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 936, 725, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 978, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x62, 2, 0, 1, 4, 0, 654, 640, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 102, 448, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 159, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 174, 374, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 273, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 390, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 128, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 177, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 209, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 94, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 194, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 98, 495, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 546, 532, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 366, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 251, 381, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 70, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 76, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 129, 505, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 179, 534, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 186, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 187, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 263, 395, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 352, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 393, 415, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 553, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 956, 689, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 966, 668, 0, 0 },
    { 1, 0xA, 0xA0, 6, 0, 0, 0, 0, 0, 0, 792, 381, 0, 0 }, { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 721, 691, 0, 0 },
    { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 865, 763, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 369, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 401, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 465, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 497, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x20, 1, 0x20, 0x22, 6, 0, 546, 665, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x20, 1, 0x20, 0x22, 6, 0, 833, 798, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 545, 558, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 834, 701, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, -12, 814, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 0, 364, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 100, 758, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 212, 702, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x23, 1, 0x23, 0x25, 6, 0, 324, 646, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, -14, 713, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 99, 658, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 210, 602, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 322, 546, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x26, 1, 0x26, 0x29, 6, 0, 11, 408, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag938_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x242, 0x13E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28B, 0x88, 0x1B4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 1, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x130, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0x140, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag938_funcs = { wstag938_setup };
FieldstgListedBattle D_WSTAG938_800A6B30 = { 88, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B3C = { 88, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B48 = { 88, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B54 = { 88, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B60 = { 90, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B6C = { 90, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B78 = { 90, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG938_800A6B84 = { 90, 7, 0x60080000 };
FieldstgBattleList D_WSTAG938_800A6B90 = {
    3,
    { &D_WSTAG938_800A6B30, &D_WSTAG938_800A6B3C, &D_WSTAG938_800A6B48, &D_WSTAG938_800A6B54, &D_WSTAG938_800A6B60,
        &D_WSTAG938_800A6B6C, &D_WSTAG938_800A6B78, &D_WSTAG938_800A6B84 },
};
FieldstgListedBattle D_WSTAG938_800A6BB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6BFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C08 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG938_800A6C14 = {
    0,
    { &D_WSTAG938_800A6BB4, &D_WSTAG938_800A6BC0, &D_WSTAG938_800A6BCC, &D_WSTAG938_800A6BD8, &D_WSTAG938_800A6BE4,
        &D_WSTAG938_800A6BF0, &D_WSTAG938_800A6BFC, &D_WSTAG938_800A6C08 },
};
FieldstgListedBattle D_WSTAG938_800A6C38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6C8C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG938_800A6C98 = {
    0,
    { &D_WSTAG938_800A6C38, &D_WSTAG938_800A6C44, &D_WSTAG938_800A6C50, &D_WSTAG938_800A6C5C, &D_WSTAG938_800A6C68,
        &D_WSTAG938_800A6C74, &D_WSTAG938_800A6C80, &D_WSTAG938_800A6C8C },
};
FieldstgListedBattle D_WSTAG938_800A6CBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6CC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6CD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6CE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6CEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6CF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6D04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG938_800A6D10 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG938_800A6D1C = {
    0,
    { &D_WSTAG938_800A6CBC, &D_WSTAG938_800A6CC8, &D_WSTAG938_800A6CD4, &D_WSTAG938_800A6CE0, &D_WSTAG938_800A6CEC,
        &D_WSTAG938_800A6CF8, &D_WSTAG938_800A6D04, &D_WSTAG938_800A6D10 },
};
FieldstgBattleLists wstag938_battle_lists = {
    379, 0, 0, { &D_WSTAG938_800A6B90, &D_WSTAG938_800A6C14, &D_WSTAG938_800A6C98 }, &D_WSTAG938_800A6D1C,
};
