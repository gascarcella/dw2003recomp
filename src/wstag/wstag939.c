#include "wstag.h"

/* WSTAG939: stage 0x28B (fieldstg_stages_2d). */

extern WstagFuncs wstag939_funcs;
extern FieldstgVramPlace wstag939_vram_places[];
extern FieldstgPlacedActor *wstag939_actors[];
extern FieldstgSprite wstag939_sprites[];
extern FieldstgMapEvent wstag939_map_events[];

void wstag939_update(WstagObject *obj) {
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

WstagObject *wstag939_start(void *arg0) {
    WstagObject *obj = object_new(wstag939_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag939_funcs.setup();
    return obj;
}

void wstag939_setup(void) {
    fieldstg_stage.background_file = 0x764;
    fieldstg_stage.sprite_file = 0x09050000;
    fieldstg_stage.sprites = wstag939_sprites;
    fieldstg_stage.map_events = wstag939_map_events;
    fieldstg_stage.mask_file = 0x904;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF500, 0x17A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag939_vram_places;
    fieldstg_stage.music = 0x2B;
    fieldstg_stage.sound = 0x60AC0000;
    fieldstg_stage.actors = wstag939_actors;
    fieldstg_attr.set_file(0, 0x09050001);
    fieldstg_attr.set_file(7, 0x09050002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag939_setup(void);

FieldstgVramPlace wstag939_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 256, 160, 0, 320, 494 }, { 320, 256, 368, 256, 192, 0, 320, 493 },
    { 320, 256, 376, 256, 224, 0, 320, 492 },
};
u16 D_WSTAG939_800A5FFC[4] = { 0x7054, 0, 0xFFFF, 0 };
u16 D_WSTAG939_800A6004[6] = { 0x7054, 1, 0x100C, 0, 0xFFFF, 0 };
u16 D_WSTAG939_800A6010[6] = { 0x7054, 1, 0x100C, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG939_800A601C[2] = { { NULL, NULL, 109 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG939_800A6034[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG939_800A604C[4] = {
    { D_WSTAG939_800A5FFC, NULL, 106 }, { D_WSTAG939_800A6004, NULL, 107 }, { D_WSTAG939_800A6010, NULL, 108 },
    { NULL, NULL, 0 },
};
FieldstgPlacedActor D_WSTAG939_800A607C = { NULL, D_WSTAG939_800A601C, 32, 4, 368, 264, 5 };
FieldstgPlacedActor D_WSTAG939_800A6090 = { NULL, D_WSTAG939_800A6034, 36, 5, 440, 253, 5 };
FieldstgPlacedActor D_WSTAG939_800A60A4 = { NULL, D_WSTAG939_800A604C, 104, 6, 407, 269, 5 };
FieldstgPlacedActor *wstag939_actors[4] = {
    &D_WSTAG939_800A607C, &D_WSTAG939_800A6090, &D_WSTAG939_800A60A4, NULL,
};
FieldstgSprite wstag939_sprites[22] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 374, 301, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 390, 293, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 406, 285, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 422, 277, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 458, 262, 0, 0 }, { 1, 0, 0x49, 6, 0, 1, 0, 3, 4, 0, 84, 193, 0, 0 },
    { 1, 0, 0x5D, 6, 4, 1, 4, 7, 4, 0, 68, 252, 0, 0 }, { 1, 0, 0x5D, 6, 5, 1, 5, 8, 4, 0, 131, 227, 0, 0 },
    { 1, 0, 0x5D, 6, 0xB, 1, 0xB, 0xE, 4, 0, 182, 153, 0, 0 }, { 1, 0, 0x49, 6, 0, 1, 0, 3, 4, 0, 227, 178, 0, 0 },
    { 1, 0, 0x5D, 6, 7, 1, 7, 0xA, 4, 0, 272, 108, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 2, 0, 2, 0x10, 0, 438, 220, 0, 0 },
    { 1, 3, 0x40, 6, 0x33, 2, 0, 1, 0x10, 0, 438, 220, 0, 0 },
    { 1, 2, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 444, 202, 0, 0 }, { 1, 4, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 444, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 294, 261, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 310, 253, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 326, 245, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 342, 237, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 374, 220, 0, 0 }, { 1, 0, 0x40, 6, 0xF, 0, 0, 0, 0, 0, 448, 207, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag939_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28A, 0x3F8, 0x2DC, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag939_funcs = { wstag939_setup };
