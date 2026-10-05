#include "wstag.h"

/* WSTAG933: stage 0x27E (fieldstg_stages_2d). */

extern WstagFuncs wstag933_funcs;
extern FieldstgVramPlace wstag933_vram_places[];
extern FieldstgPlacedActor *wstag933_actors[];
extern FieldstgSprite wstag933_sprites[];
extern FieldstgMapEvent wstag933_map_events[];

void wstag933_update(WstagObject *obj) {
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

WstagObject *wstag933_start(void *arg0) {
    WstagObject *obj = object_new(wstag933_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag933_funcs.setup();
    return obj;
}

void wstag933_setup(void) {
    fieldstg_stage.background_file = 0x1AA;
    fieldstg_stage.sprite_file = 0x08F90000;
    fieldstg_stage.sprites = wstag933_sprites;
    fieldstg_stage.map_events = wstag933_map_events;
    fieldstg_stage.mask_file = 0x8F8;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10B00, 0x12C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag933_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag933_actors;
    fieldstg_attr.set_file(0, 0x08F90001);
    fieldstg_attr.set_file(7, 0x08F90002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag933_setup(void);

FieldstgVramPlace wstag933_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 378, 40, 122, 336, 511 }, { 320, 256, 346, 354, 104, 98, 352, 511 },
    { 320, 256, 320, 392, 0, 136, 368, 511 }, { 320, 256, 356, 354, 144, 98, 336, 510 },
    { 320, 256, 368, 328, 192, 72, 352, 510 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 346, 394, 104, 138, 368, 510 },
};
FieldstgTalk D_WSTAG933_800A6040[2] = { { NULL, NULL, 57 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG933_800A6058[2] = { { NULL, NULL, 58 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG933_800A6070[2] = { { NULL, NULL, 59 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG933_800A6088[2] = { { NULL, NULL, 60 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG933_800A60A0[2] = { { NULL, NULL, 61 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG933_800A60B8 = { NULL, D_WSTAG933_800A6040, 25, 4, 275, 255, 1 };
FieldstgPlacedActor D_WSTAG933_800A60CC = { NULL, D_WSTAG933_800A6058, 26, 5, 256, 321, 1 };
FieldstgPlacedActor D_WSTAG933_800A60E0 = { NULL, D_WSTAG933_800A6070, 57, 6, 133, 187, 1 };
FieldstgPlacedActor D_WSTAG933_800A60F4 = { NULL, NULL, 68, 7, 307, 240, 5 };
FieldstgPlacedActor D_WSTAG933_800A6108 = { NULL, D_WSTAG933_800A6088, 132, 8, 217, 257, 5 };
FieldstgPlacedActor D_WSTAG933_800A611C = { NULL, NULL, 224, 9, 267, 266, 7 };
FieldstgPlacedActor D_WSTAG933_800A6130 = { NULL, D_WSTAG933_800A60A0, 367, 10, 101, 203, 5 };
FieldstgPlacedActor *wstag933_actors[8] = {
    &D_WSTAG933_800A60B8, &D_WSTAG933_800A60CC, &D_WSTAG933_800A60E0, &D_WSTAG933_800A60F4, &D_WSTAG933_800A6108,
    &D_WSTAG933_800A611C, &D_WSTAG933_800A6130, NULL,
};
FieldstgSprite wstag933_sprites[25] = {
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 115, 131, 0, 0 }, { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 163, 107, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 227, 75, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 280, 73, 0, 0 },
    { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 328, 97, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 376, 121, 0, 0 },
    { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 117, 147, 0, 0 }, { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 165, 124, 0, 0 },
    { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 229, 92, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 259, 90, 0, 0 },
    { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 307, 115, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 355, 139, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xF, 0, 0, 0, 0, 0, 350, 125, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 192, 271, 301, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 184, 263, 288, 0 }, { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 152, 216, 233, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 178, 254, 277, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 280, 256, 280, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 264, 248, 271, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 248, 240, 264, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 232, 232, 256, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 217, 222, 248, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 312, 231, 263, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 332, 222, 253, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag933_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x112, 0x220, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0x68, 0x15A, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag933_funcs = { wstag933_setup };
