#include "wstag.h"

/* WSTAG923: stage 0x273 (fieldstg_stages_2d). */

const CVECTOR wstag923_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgVramPlace wstag923_vram_places[];
extern FieldstgPlacedActor *wstag923_actors[];
extern FieldstgSprite wstag923_sprites[];
extern FieldstgMapEvent wstag923_map_events[];
extern FieldstgEventDef wstag923_events[];

extern FieldstgStageFuncs wstag923_funcs; /* the setup and a fade pair, as FIELDSTG's own */

void wstag923_update(WstagObject *obj) {
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

WstagObject *wstag923_start(void *arg0) {
    WstagObject *obj = object_new(wstag923_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag923_funcs.setup();
    return obj;
}

void wstag923_setup(void) {
    fieldstg_stage.background_file = 0x19A;
    fieldstg_stage.sprites = wstag923_sprites;
    fieldstg_stage.map_events = wstag923_map_events;
    fieldstg_stage.sprite_file = 0x08E20003;
    fieldstg_stage.mask_file = 0x8E0;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11C00, 0x13400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag923_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag923_actors;
    fieldstg_stage.color = wstag923_color;
    fieldstg_stage.events = wstag923_events;
    fieldstg_attr.set_file(0, 0x08E20004);
    fieldstg_attr.set_file(7, 0x08E20005);
    fieldstg_attr.init_layer(0);
}

void wstag923_fade_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 wstag923_fade_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

/* The stage's .data (tools/wstag_data.py). */
void wstag923_setup(void);

FieldstgVramPlace wstag923_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 496, 288, 704, 32, 368, 504 }, { 448, 256, 496, 328, 704, 72, 320, 503 },
    { 384, 256, 436, 256, 464, 0, 336, 503 }, { 320, 256, 376, 313, 224, 57, 352, 503 },
    { 448, 256, 448, 365, 512, 109, 368, 503 }, { 448, 256, 456, 365, 544, 109, 320, 502 },
    { 448, 256, 496, 368, 704, 112, 336, 502 }, { 448, 256, 464, 394, 576, 138, 352, 502 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 448, 256, 472, 394, 608, 138, 368, 502 },
    { 448, 256, 480, 394, 640, 138, 320, 501 },
};
FieldstgTalk D_WSTAG923_800A61C0[2] = { { NULL, NULL, 24 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A61D8[2] = { { NULL, NULL, 25 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A61F0[2] = { { NULL, NULL, 31 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6208[2] = { { NULL, NULL, 29 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6220[2] = { { NULL, NULL, 26 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6238[2] = { { NULL, NULL, 30 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6250[2] = { { NULL, NULL, 27 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6268[2] = { { NULL, NULL, 28 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6280[2] = { { NULL, NULL, 32 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG923_800A6298[2] = { { NULL, NULL, 33 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG923_800A62B0 = { NULL, D_WSTAG923_800A61C0, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG923_800A62C4 = { NULL, D_WSTAG923_800A61D8, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG923_800A62D8 = { NULL, D_WSTAG923_800A61F0, 37, 6, 344, 188, 1 };
FieldstgPlacedActor D_WSTAG923_800A62EC = { NULL, D_WSTAG923_800A6208, 45, 7, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG923_800A6300 = { NULL, D_WSTAG923_800A6220, 48, 8, 328, 260, 5 };
FieldstgPlacedActor D_WSTAG923_800A6314 = { NULL, D_WSTAG923_800A6238, 53, 9, 391, 359, 5 };
FieldstgPlacedActor D_WSTAG923_800A6328 = { NULL, D_WSTAG923_800A6250, 57, 10, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG923_800A633C = { NULL, D_WSTAG923_800A6268, 58, 11, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG923_800A6350 = { NULL, NULL, 112, 12, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG923_800A6364 = { NULL, NULL, 113, 13, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG923_800A6378 = { NULL, D_WSTAG923_800A6280, 371, 14, 308, 305, 3 };
FieldstgPlacedActor D_WSTAG923_800A638C = { NULL, D_WSTAG923_800A6298, 377, 15, 285, 295, 7 };
FieldstgPlacedActor *wstag923_actors[13] = {
    &D_WSTAG923_800A62B0, &D_WSTAG923_800A62C4, &D_WSTAG923_800A62D8, &D_WSTAG923_800A62EC, &D_WSTAG923_800A6300,
    &D_WSTAG923_800A6314, &D_WSTAG923_800A6328, &D_WSTAG923_800A633C, &D_WSTAG923_800A6350, &D_WSTAG923_800A6364,
    &D_WSTAG923_800A6378, &D_WSTAG923_800A638C, NULL,
};
FieldstgSprite wstag923_sprites[105] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 4, 0, 243, 69, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 4, 0, 802, 278, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 4, 0, 457, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 339, 212, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 372, 196, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 403, 244, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 435, 228, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 386, 117, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 546, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 674, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 573, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 613, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 151, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 411, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 649, 200, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 10, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 42, 121, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 74, 137, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 290, 69, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 322, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 354, 101, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 482, 165, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 514, 181, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 706, 213, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 738, 229, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 770, 245, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 7, 0x12, 0, 119, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 135, 232, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 155, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 175, 212, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 195, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 215, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 235, 182, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 255, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 275, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 295, 152, 0, 0 },
    { 1, 0x66, 0x40, 6, 4, 0, 0, 0, 0, 0, 335, 141, 0, 0 }, { 1, 0x65, 0x40, 6, 5, 0, 0, 0, 0, 0, 511, 229, 0, 0 },
    { 1, 0x64, 0x40, 6, 6, 0, 0, 0, 0, 0, 736, 326, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 114, 179, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 264, 130, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 429, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x54, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 6, 0x4D, 1, 0x4D, 0x50, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 28, 519, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 81, 492, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 183, 442, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 237, 424, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 656, 492, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 752, 431, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 55, 506, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 100, 493, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 151, 461, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 202, 445, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 296, 424, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 657, 504, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 259, 428, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 366, 433, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 582, 411, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 661, 442, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 674, 537, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 694, 538, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 778, 417, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 449, 517, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 501, 544, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 530, 355, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 546, 354, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 636, 592, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 659, 600, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 87, 303, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 130, 322, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 682, 614, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 61, 292, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 113, 328, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 474, 531, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 529, 554, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 437, 427, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 455, 421, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 608, 405, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 626, 396, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 654, 525, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 691, 424, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 144, 110, 160, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 359, 148, 193, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 535, 236, 281, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 759, 332, 376, 0 },
    { 1, 0, 0x50, 4, 7, 0, 0, 0, 0, 0, 441, 345, 450, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 464, 444, 496, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 595, 442, 478, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 384, 259, 279, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 401, 250, 271, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 368, 251, 270, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 417, 243, 264, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 352, 242, 262, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 433, 233, 255, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 336, 229, 255, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 449, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 320, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 337, 219, 240, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 353, 208, 231, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 369, 203, 223, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 385, 192, 215, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 401, 187, 207, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 499, 317, 334, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag923_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x3E8, 0xEC, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x275, 0x70, 0xF0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x276, 0x58, 0x1CC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x277, 0x69, 0x134, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgStageFuncs wstag923_funcs = { wstag923_setup, wstag923_fade_start, wstag923_fade_update };
FieldstgEventDef wstag923_events[1] = { { -1, NULL, 0, NULL, NULL } };
