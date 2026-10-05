#include "wstag.h"

/* WSTAG426: stage 0x29D (fieldstg_stages). */

extern WstagFuncs wstag426_funcs;
extern FieldstgVramPlace wstag426_vram_places[];
extern FieldstgPlacedActor *wstag426_actors[];
extern FieldstgSprite wstag426_sprites[];
extern FieldstgMapEvent wstag426_map_events[];

void wstag426_update(WstagObject *obj) {
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

WstagObject *wstag426_start(void *arg0) {
    WstagObject *obj = object_new(wstag426_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag426_funcs.setup();
    return obj;
}

void wstag426_setup(void) {
    fieldstg_stage.background_file = 0x5A1;
    fieldstg_stage.sprite_file = 0x05A20000;
    fieldstg_stage.sprites = wstag426_sprites;
    fieldstg_stage.map_events = wstag426_map_events;
    fieldstg_stage.mask_file = 0x5A0;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1BA00, 0x17F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag426_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag426_actors;
    fieldstg_attr.set_file(0, 0x05A20001);
    fieldstg_attr.set_file(7, 0x05A20002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag426_setup(void);

FieldstgVramPlace wstag426_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 394, 282, 296, 26, 320, 511 }, { 384, 256, 438, 279, 472, 23, 336, 511 },
    { 384, 256, 384, 282, 256, 26, 352, 511 }, { 384, 256, 416, 256, 384, 0, 368, 511 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG426_800A6020[4] = { 0x7A2A, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6028[4] = { 0x7A15, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6030[4] = { 0x7A14, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6038[4] = { 0x7020, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6040[4] = { 0x7A46, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6048[6] = { 0x7021, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG426_800A6054[4] = { 0x7A46, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A605C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6064[4] = { 0x7A47, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A606C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6074[4] = { 0x7A47, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A607C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A6084[4] = { 0x7A47, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG426_800A608C[2] = { { NULL, D_WSTAG426_800A6020, 419 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG426_800A60A4[2] = { { NULL, D_WSTAG426_800A6028, 424 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG426_800A60BC[2] = { { NULL, D_WSTAG426_800A6030, 420 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG426_800A60D4[6] = {
    { D_WSTAG426_800A6038, D_WSTAG426_800A6040, 423 }, { D_WSTAG426_800A6048, D_WSTAG426_800A6054, 423 },
    { D_WSTAG426_800A605C, D_WSTAG426_800A6064, 423 }, { D_WSTAG426_800A606C, D_WSTAG426_800A6074, 423 },
    { D_WSTAG426_800A607C, D_WSTAG426_800A6084, 423 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG426_800A611C[2] = { { NULL, NULL, 525 }, { NULL, NULL, 0 } };
u16 D_WSTAG426_800A6134[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG426_800A613C[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG426_800A6144 = { NULL, D_WSTAG426_800A608C, 20, 4, 345, 197, 7 };
FieldstgPlacedActor D_WSTAG426_800A6158 = { NULL, D_WSTAG426_800A60A4, 22, 5, 408, 341, 7 };
FieldstgPlacedActor D_WSTAG426_800A616C = { NULL, D_WSTAG426_800A60BC, 23, 6, 330, 380, 7 };
FieldstgPlacedActor D_WSTAG426_800A6180 = { D_WSTAG426_800A6134, D_WSTAG426_800A60D4, 206, 7, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG426_800A6194 = { D_WSTAG426_800A613C, D_WSTAG426_800A611C, 206, 7, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG426_800A61A8 = { NULL, NULL, 219, 8, 345, 213, 7 };
FieldstgPlacedActor *wstag426_actors[7] = {
    &D_WSTAG426_800A6144, &D_WSTAG426_800A6158, &D_WSTAG426_800A616C, &D_WSTAG426_800A6180, &D_WSTAG426_800A6194,
    &D_WSTAG426_800A61A8, NULL,
};
FieldstgSprite wstag426_sprites[22] = {
    { 1, 0, 0x64, 4, 0x14, 0, 0, 0, 0, 0, 448, 251, 310, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 304, 189, 222, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 290, 166, 222, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 328, 195, 215, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 336, 187, 207, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 352, 180, 199, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 367, 164, 199, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 472, 281, 331, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 496, 352, 399, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 271, 382, 399, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 289, 374, 391, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 336, 350, 367, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 353, 343, 358, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 366, 323, 351, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 416, 309, 326, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 435, 292, 319, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 176, 155, 211, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 160, 148, 199, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 144, 140, 191, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 128, 132, 184, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 118, 127, 179, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag426_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x1E8, 0x1AC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x100, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x10F, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xB0, 0xD8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xBF, 0x11E, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag426_funcs = { wstag426_setup };
