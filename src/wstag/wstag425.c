#include "wstag.h"

/* WSTAG425: stage 0x22F (fieldstg_stages). */

extern WstagFuncs wstag425_funcs;
extern FieldstgVramPlace wstag425_vram_places[];
extern FieldstgPlacedActor *wstag425_actors[];
extern FieldstgSprite wstag425_sprites[];
extern FieldstgMapEvent wstag425_map_events[];

void wstag425_update(WstagObject *obj) {
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

WstagObject *wstag425_start(void *arg0) {
    WstagObject *obj = object_new(wstag425_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag425_funcs.setup();
    return obj;
}

void wstag425_setup(void) {
    fieldstg_stage.background_file = 0x24C;
    fieldstg_stage.sprite_file = 0x02540000;
    fieldstg_stage.sprites = wstag425_sprites;
    fieldstg_stage.map_events = wstag425_map_events;
    fieldstg_stage.mask_file = 0x330;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1A200, 0x17A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag425_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag425_actors;
    fieldstg_attr.set_file(0, 0x02540001);
    fieldstg_attr.set_file(7, 0x02540002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag425_setup(void);

FieldstgVramPlace wstag425_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 348, 423, 112, 167, 368, 511 }, { 320, 256, 374, 371, 216, 115, 368, 510 },
    { 320, 256, 352, 327, 128, 71, 368, 509 }, { 384, 256, 400, 282, 320, 26, 368, 508 },
    { 384, 256, 408, 282, 352, 26, 368, 507 }, { 384, 256, 384, 283, 256, 27, 368, 506 },
    { 384, 256, 416, 256, 384, 0, 368, 505 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG425_800A6050[4] = { 0x1A08, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A6058[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6060[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6068[4] = { 0x7A20, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6070[4] = { 0x1A0F, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A6078[4] = { 0x1A0F, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6080[4] = { 0x1A0F, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6088[4] = { 0x7A06, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6090[4] = { 0x1A0E, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A6098[4] = { 0x1A0E, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60A0[4] = { 0x1A0E, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60A8[4] = { 0x7A05, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60B0[4] = { 0x818F, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A60B8[6] = { 0x818F, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60C4[4] = { 0x818F, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60CC[6] = { 0x1A1C, 0, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60D8[4] = { 0x1A1C, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60E0[6] = { 0x1A1C, 1, 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60EC[4] = { 0x7A37, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60F4[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A60FC[4] = { 0x7A37, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6104[4] = { 0x703E, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A610C[4] = { 0x7A37, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6114[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A611C[4] = { 0x7A38, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6124[4] = { 0x7020, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A612C[4] = { 0x7A38, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6134[6] = { 0x7021, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A6140[4] = { 0x7A39, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6148[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6150[4] = { 0x7A3A, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6158[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6160[4] = { 0x7A3A, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6168[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6170[4] = { 0x7A3A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG425_800A6178[3] = {
    { D_WSTAG425_800A6050, D_WSTAG425_800A6058, 4 }, { D_WSTAG425_800A6060, D_WSTAG425_800A6068, 5 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG425_800A619C[3] = {
    { D_WSTAG425_800A6070, D_WSTAG425_800A6078, 289 }, { D_WSTAG425_800A6080, D_WSTAG425_800A6088, 243 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG425_800A61C0[3] = {
    { D_WSTAG425_800A6090, D_WSTAG425_800A6098, 291 }, { D_WSTAG425_800A60A0, D_WSTAG425_800A60A8, 242 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG425_800A61E4[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A61FC[2] = { { NULL, NULL, 67 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A6214[2] = { { NULL, NULL, 73 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A622C[2] = { { NULL, NULL, 72 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A6244[2] = { { NULL, NULL, 71 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A625C[2] = { { NULL, NULL, 70 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A6274[2] = { { NULL, NULL, 69 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A628C[2] = { { NULL, NULL, 68 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A62A4[2] = { { NULL, NULL, 66 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A62BC[2] = { { NULL, NULL, 75 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A62D4[3] = {
    { D_WSTAG425_800A60B0, D_WSTAG425_800A60B8, 296 }, { D_WSTAG425_800A60C4, NULL, 364 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG425_800A62F8[2] = { { NULL, NULL, 74 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG425_800A6310[11] = {
    { D_WSTAG425_800A60CC, D_WSTAG425_800A60D8, 292 }, { D_WSTAG425_800A60E0, D_WSTAG425_800A60EC, 1 },
    { D_WSTAG425_800A60F4, D_WSTAG425_800A60FC, 1 }, { D_WSTAG425_800A6104, D_WSTAG425_800A610C, 1 },
    { D_WSTAG425_800A6114, D_WSTAG425_800A611C, 1 }, { D_WSTAG425_800A6124, D_WSTAG425_800A612C, 1 },
    { D_WSTAG425_800A6134, D_WSTAG425_800A6140, 1 }, { D_WSTAG425_800A6148, D_WSTAG425_800A6150, 1 },
    { D_WSTAG425_800A6158, D_WSTAG425_800A6160, 1 }, { D_WSTAG425_800A6168, D_WSTAG425_800A6170, 1 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG425_800A6394[2] = { { NULL, NULL, 372 }, { NULL, NULL, 0 } };
u16 D_WSTAG425_800A63AC[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63B4[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63BC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63C4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63CC[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63D4[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63DC[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63E4[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63EC[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63F4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A63FC[8] = { 0x1C45, 1, 0x6006, 1, 0x1C46, 0, 0xFFFF, 0 };
u16 D_WSTAG425_800A640C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A6414[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG425_800A641C[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG425_800A6424 = { NULL, D_WSTAG425_800A6178, 20, 4, 345, 197, 7 };
FieldstgPlacedActor D_WSTAG425_800A6438 = { NULL, D_WSTAG425_800A619C, 22, 5, 408, 341, 7 };
FieldstgPlacedActor D_WSTAG425_800A644C = { NULL, D_WSTAG425_800A61C0, 23, 6, 330, 380, 7 };
FieldstgPlacedActor D_WSTAG425_800A6460 = { D_WSTAG425_800A63AC, D_WSTAG425_800A61E4, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A6474 = { D_WSTAG425_800A63B4, D_WSTAG425_800A61FC, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A6488 = { D_WSTAG425_800A63BC, D_WSTAG425_800A6214, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A649C = { D_WSTAG425_800A63C4, D_WSTAG425_800A622C, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A64B0 = { D_WSTAG425_800A63CC, D_WSTAG425_800A6244, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A64C4 = { D_WSTAG425_800A63D4, D_WSTAG425_800A625C, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A64D8 = { D_WSTAG425_800A63DC, D_WSTAG425_800A6274, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A64EC = { D_WSTAG425_800A63E4, D_WSTAG425_800A628C, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A6500 = { D_WSTAG425_800A63EC, D_WSTAG425_800A62A4, 46, 7, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A6514 = { D_WSTAG425_800A63F4, D_WSTAG425_800A62BC, 46, 7, 384, 418, 5 };
FieldstgPlacedActor D_WSTAG425_800A6528 = { D_WSTAG425_800A63FC, D_WSTAG425_800A62D4, 64, 8, 207, 152, 3 };
FieldstgPlacedActor D_WSTAG425_800A653C = { D_WSTAG425_800A640C, D_WSTAG425_800A62F8, 157, 9, 384, 418, 3 };
FieldstgPlacedActor D_WSTAG425_800A6550 = { D_WSTAG425_800A6414, D_WSTAG425_800A6310, 206, 10, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG425_800A6564 = { D_WSTAG425_800A641C, D_WSTAG425_800A6394, 206, 10, 128, 201, 7 };
FieldstgPlacedActor D_WSTAG425_800A6578 = { NULL, NULL, 219, 11, 345, 213, 7 };
FieldstgPlacedActor *wstag425_actors[19] = {
    &D_WSTAG425_800A6424, &D_WSTAG425_800A6438, &D_WSTAG425_800A644C, &D_WSTAG425_800A6460, &D_WSTAG425_800A6474,
    &D_WSTAG425_800A6488, &D_WSTAG425_800A649C, &D_WSTAG425_800A64B0, &D_WSTAG425_800A64C4, &D_WSTAG425_800A64D8,
    &D_WSTAG425_800A64EC, &D_WSTAG425_800A6500, &D_WSTAG425_800A6514, &D_WSTAG425_800A6528, &D_WSTAG425_800A653C,
    &D_WSTAG425_800A6550, &D_WSTAG425_800A6564, &D_WSTAG425_800A6578, NULL,
};
FieldstgSprite wstag425_sprites[22] = {
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
FieldstgMapEvent wstag425_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x22E, 0x1E8, 0x1AC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x100, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x10F, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xB0, 0xD8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xBF, 0x11E, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag425_funcs = { wstag425_setup };
