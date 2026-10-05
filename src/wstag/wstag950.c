#include "wstag.h"

/* WSTAG950: stage 0x296 (fieldstg_stages_2d). */

extern WstagFuncs wstag950_funcs;
extern CVECTOR wstag950_color;
extern FieldstgVramPlace wstag950_vram_places[];
extern FieldstgPlacedActor *wstag950_actors[];
extern FieldstgSprite wstag950_sprites[];
extern FieldstgMapEvent wstag950_map_events[];
extern FieldstgBattleLists wstag950_battle_lists;

void wstag950_update(WstagObject *obj) {
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

WstagObject *wstag950_start(void *arg0) {
    WstagObject *obj = object_new(wstag950_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag950_funcs.setup();
    return obj;
}

void wstag950_setup(void) {
    fieldstg_stage.background_file = 0x2BA;
    fieldstg_stage.sprite_file = 0x091B0000;
    fieldstg_stage.sprites = wstag950_sprites;
    fieldstg_stage.map_events = wstag950_map_events;
    fieldstg_stage.mask_file = 0x91A;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13D00, 0x34000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag950_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag950_actors;
    fieldstg_stage.color = wstag950_color;
    fieldstg_stage.battle_lists = &wstag950_battle_lists;
    fieldstg_attr.set_file(0, 0x091B0002);
    fieldstg_attr.set_file(7, 0x091B0003);
    fieldstg_attr.set_file(4, 0x091B0001);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag950/nonmatchings/wstag950", wstag950_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag950_setup(void);

FieldstgVramPlace wstag950_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 336, 511 }, { 384, 256, 432, 256, 448, 0, 352, 511 },
    { 320, 256, 368, 412, 192, 156, 368, 511 }, { 320, 256, 368, 460, 192, 204, 320, 510 },
};
u16 D_WSTAG950_800A6054[8] = { 0x26B, 1, 0x8233, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG950_800A6064[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A6070[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A607C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG950_800A6088[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A6098[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A60A4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG950_800A60AC[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG950_800A60B8[4] = { 0x7840, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG950_800A60C0[2] = { { NULL, D_WSTAG950_800A6054, 3 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG950_800A60D8[2] = { { NULL, NULL, 61 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG950_800A60F0[5] = {
    { D_WSTAG950_800A6064, D_WSTAG950_800A6070, 63 }, { D_WSTAG950_800A607C, D_WSTAG950_800A6088, 64 },
    { D_WSTAG950_800A6098, D_WSTAG950_800A60A4, 61 }, { D_WSTAG950_800A60AC, D_WSTAG950_800A60B8, 62 },
    { NULL, NULL, 0 },
};
u16 D_WSTAG950_800A612C[4] = { 0x26B, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A6134[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG950_800A613C[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG950_800A6144 = { D_WSTAG950_800A612C, D_WSTAG950_800A60C0, 33, 4, 129, 417, 1 };
FieldstgPlacedActor D_WSTAG950_800A6158 = { D_WSTAG950_800A6134, D_WSTAG950_800A60D8, 55, 5, 776, 125, 7 };
FieldstgPlacedActor D_WSTAG950_800A616C = { D_WSTAG950_800A613C, D_WSTAG950_800A60F0, 55, 5, 776, 125, 7 };
FieldstgPlacedActor D_WSTAG950_800A6180 = { NULL, NULL, 75, 6, 760, 357, 7 };
FieldstgPlacedActor D_WSTAG950_800A6194 = { NULL, NULL, 76, 7, 670, 280, 1 };
FieldstgPlacedActor *wstag950_actors[6] = {
    &D_WSTAG950_800A6144, &D_WSTAG950_800A6158, &D_WSTAG950_800A616C, &D_WSTAG950_800A6180, &D_WSTAG950_800A6194,
    NULL,
};
FieldstgSprite wstag950_sprites[96] = {
    { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 43, 521, 0, 0 }, { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 426, -16, 0, 0 },
    { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 761, 743, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 337, 409, 0, 0 },
    { 1, 0, 0x74, 2, 2, 0, 0, 0, 0, 0, 655, 662, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 732, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 810, 177, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 868, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 989, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1051, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1125, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 431, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 541, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 618, 226, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 781, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 907, 130, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 34, 772, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 470, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 504, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 606, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 611, 441, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 660, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 686, 214, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 700, 583, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 764, 605, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 840, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 372, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 411, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 492, 502, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 988, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 58, 779, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 293, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 427, 470, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 537, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 940, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 337, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 538, 522, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 624, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 764, 635, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 793, 425, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1050, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 45, 781, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 466, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 467, 390, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 523, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 552, 295, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 563, 474, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 564, 271, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 575, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 632, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 644, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 664, 324, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 684, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 690, 456, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 733, 208, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 788, 632, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 792, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 819, 419, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 850, 168, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 952, 332, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 977, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1021, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1072, 376, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1131, 347, 0, 0 },
    { 1, 0, 0x74, 6, 3, 0, 0, 0, 0, 0, 768, 640, 0, 0 }, { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 96, 687, 687, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 103, 923, 923, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 144, 663, 663, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 703, 703, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 639, 639, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 679, 679, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 952, 952, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 655, 655, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 197, 197, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 274, 262, 262, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 623, 623, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 937, 937, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 305, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 336, 599, 599, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 352, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 583, 583, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 591, 591, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 127, 127, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 624, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 831, 831, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 992, 783, 783, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1115, 428, 428, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag950_map_events[16] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x32A, 0x334, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x298, 0x9A, 0x74, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 3, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x1A1, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x1B0, 0x176, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x58, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0x40, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFC0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB8, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFF90, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x18, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFF0, 0x10, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag950_funcs = { wstag950_setup };
FieldstgListedBattle D_WSTAG950_800A6A04 = { 109, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A10 = { 109, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A1C = { 157, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A28 = { 157, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A34 = { 176, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A40 = { 176, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A4C = { 158, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6A58 = { 158, 13, 0x60080000 };
FieldstgBattleList D_WSTAG950_800A6A64 = {
    3,
    { &D_WSTAG950_800A6A04, &D_WSTAG950_800A6A10, &D_WSTAG950_800A6A1C, &D_WSTAG950_800A6A28, &D_WSTAG950_800A6A34,
        &D_WSTAG950_800A6A40, &D_WSTAG950_800A6A4C, &D_WSTAG950_800A6A58 },
};
FieldstgListedBattle D_WSTAG950_800A6A88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6A94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6AA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6AAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6AB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6AC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6AD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6ADC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG950_800A6AE8 = {
    0,
    { &D_WSTAG950_800A6A88, &D_WSTAG950_800A6A94, &D_WSTAG950_800A6AA0, &D_WSTAG950_800A6AAC, &D_WSTAG950_800A6AB8,
        &D_WSTAG950_800A6AC4, &D_WSTAG950_800A6AD0, &D_WSTAG950_800A6ADC },
};
FieldstgListedBattle D_WSTAG950_800A6B0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B60 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG950_800A6B6C = {
    0,
    { &D_WSTAG950_800A6B0C, &D_WSTAG950_800A6B18, &D_WSTAG950_800A6B24, &D_WSTAG950_800A6B30, &D_WSTAG950_800A6B3C,
        &D_WSTAG950_800A6B48, &D_WSTAG950_800A6B54, &D_WSTAG950_800A6B60 },
};
FieldstgListedBattle D_WSTAG950_800A6B90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6B9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6BA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6BB4 = { 331, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6BC0 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6BCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG950_800A6BD8 = { 127, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG950_800A6BE4 = { 64, 8, 0x60080000 };
FieldstgBattleList D_WSTAG950_800A6BF0 = {
    0,
    { &D_WSTAG950_800A6B90, &D_WSTAG950_800A6B9C, &D_WSTAG950_800A6BA8, &D_WSTAG950_800A6BB4, &D_WSTAG950_800A6BC0,
        &D_WSTAG950_800A6BCC, &D_WSTAG950_800A6BD8, &D_WSTAG950_800A6BE4 },
};
FieldstgBattleLists wstag950_battle_lists = {
    388, 0, 0, { &D_WSTAG950_800A6A64, &D_WSTAG950_800A6AE8, &D_WSTAG950_800A6B6C }, &D_WSTAG950_800A6BF0,
};
