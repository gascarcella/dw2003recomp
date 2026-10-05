#include "wstag.h"

/* WSTAG957: stage 0x29D (fieldstg_stages_2d). */

extern WstagFuncs wstag957_funcs;
extern FieldstgVramPlace wstag957_vram_places[];
extern FieldstgPlacedActor *wstag957_actors[];
extern FieldstgSprite wstag957_sprites[];
extern FieldstgMapEvent wstag957_map_events[];

void wstag957_update(WstagObject *obj) {
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

WstagObject *wstag957_start(void *arg0) {
    WstagObject *obj = object_new(wstag957_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag957_funcs.setup();
    return obj;
}

void wstag957_setup(void) {
    fieldstg_stage.background_file = 0x24C;
    fieldstg_stage.sprite_file = 0x09290000;
    fieldstg_stage.sprites = wstag957_sprites;
    fieldstg_stage.map_events = wstag957_map_events;
    fieldstg_stage.mask_file = 0x928;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1A200, 0x17A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag957_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag957_actors;
    fieldstg_attr.set_file(0, 0x09290001);
    fieldstg_attr.set_file(7, 0x09290002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag957_setup(void);

FieldstgVramPlace wstag957_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 348, 423, 112, 167, 368, 511 }, { 320, 256, 374, 371, 216, 115, 368, 510 },
    { 320, 256, 352, 327, 128, 71, 368, 509 }, { 384, 256, 416, 256, 384, 0, 368, 508 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 384, 256, 400, 282, 320, 26, 368, 507 },
};
u16 D_WSTAG957_800A6030[4] = { 0x7A2A, 1, 0xFFFF, 0 };
u16 D_WSTAG957_800A6038[4] = { 0x7A15, 1, 0xFFFF, 0 };
u16 D_WSTAG957_800A6040[4] = { 0x7A14, 1, 0xFFFF, 0 };
u16 D_WSTAG957_800A6048[4] = { 0x7A47, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG957_800A6050[2] = { { NULL, D_WSTAG957_800A6030, 43 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG957_800A6068[2] = { { NULL, D_WSTAG957_800A6038, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG957_800A6080[2] = { { NULL, D_WSTAG957_800A6040, 54 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG957_800A6098[2] = { { NULL, D_WSTAG957_800A6048, 135 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG957_800A60B0[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG957_800A60C8[2] = { { NULL, NULL, 122 }, { NULL, NULL, 0 } };
u16 D_WSTAG957_800A60E0[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG957_800A60E8[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG957_800A60F0 = { NULL, D_WSTAG957_800A6050, 20, 4, 345, 197, 7 };
FieldstgPlacedActor D_WSTAG957_800A6104 = { NULL, D_WSTAG957_800A6068, 22, 5, 408, 341, 7 };
FieldstgPlacedActor D_WSTAG957_800A6118 = { NULL, D_WSTAG957_800A6080, 23, 6, 330, 380, 7 };
FieldstgPlacedActor D_WSTAG957_800A612C = { D_WSTAG957_800A60E0, D_WSTAG957_800A6098, 206, 7, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG957_800A6140 = { D_WSTAG957_800A60E8, D_WSTAG957_800A60B0, 206, 7, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG957_800A6154 = { NULL, NULL, 219, 8, 345, 213, 7 };
FieldstgPlacedActor D_WSTAG957_800A6168 = { NULL, D_WSTAG957_800A60C8, 377, 9, 384, 418, 3 };
FieldstgPlacedActor *wstag957_actors[8] = {
    &D_WSTAG957_800A60F0, &D_WSTAG957_800A6104, &D_WSTAG957_800A6118, &D_WSTAG957_800A612C, &D_WSTAG957_800A6140,
    &D_WSTAG957_800A6154, &D_WSTAG957_800A6168, NULL,
};
FieldstgSprite wstag957_sprites[22] = {
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 304, 189, 222, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 290, 166, 222, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 328, 195, 215, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 336, 187, 207, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 352, 180, 199, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 367, 164, 199, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 472, 281, 331, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 496, 352, 399, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 271, 382, 399, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 289, 374, 391, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 336, 350, 367, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 353, 343, 358, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 366, 323, 351, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 416, 309, 326, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 435, 292, 319, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 176, 155, 211, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 160, 148, 199, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 144, 140, 191, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 128, 132, 184, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 118, 127, 179, 0 },
    { 1, 0, 0x64, 4, 0x14, 0, 0, 0, 0, 0, 448, 251, 310, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag957_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x1E8, 0x1AC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x100, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x10F, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xB0, 0xD8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xBF, 0x11E, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag957_funcs = { wstag957_setup };
