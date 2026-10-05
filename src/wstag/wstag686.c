#include "wstag.h"

/* WSTAG686: stage 0x2C8 (fieldstg_stages). */

extern WstagFuncs wstag686_funcs;
extern FieldstgVramPlace wstag686_vram_places[];
extern FieldstgPlacedActor *wstag686_actors[];
extern FieldstgSprite wstag686_sprites[];
extern FieldstgMapEvent wstag686_map_events[];

void wstag686_update(WstagObject *obj) {
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

WstagObject *wstag686_start(void *arg0) {
    WstagObject *obj = object_new(wstag686_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag686_funcs.setup();
    return obj;
}

void wstag686_setup(void) {
    fieldstg_stage.background_file = 0x5FE;
    fieldstg_stage.sprite_file = 0x05FF0000;
    fieldstg_stage.sprites = wstag686_sprites;
    fieldstg_stage.map_events = wstag686_map_events;
    fieldstg_stage.mask_file = 0x5FD;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x3D200, 0x13A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag686_vram_places;
    fieldstg_stage.music = 0x3C;
    fieldstg_stage.sound = 0x60F00000;
    fieldstg_stage.actors = wstag686_actors;
    fieldstg_attr.set_file(0, 0x05FF0001);
    fieldstg_attr.set_file(7, 0x05FF0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag686_setup(void);

FieldstgVramPlace wstag686_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 366, 344, 184, 88, 336, 511 }, { 320, 256, 374, 344, 216, 88, 352, 511 },
    { 320, 256, 356, 384, 144, 128, 368, 511 }, { 320, 256, 364, 392, 176, 136, 336, 510 },
    { 320, 256, 372, 392, 208, 136, 352, 510 }, { 320, 256, 334, 397, 56, 141, 368, 510 },
    { 320, 256, 342, 405, 88, 149, 320, 509 }, { 320, 256, 320, 422, 0, 166, 336, 509 },
    { 320, 256, 364, 424, 176, 168, 352, 509 }, { 320, 256, 372, 424, 208, 168, 368, 509 },
};
u16 D_WSTAG686_800A6070[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6078[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6080[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6088[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6090[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6098[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG686_800A60A0[2] = { { NULL, NULL, 357 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A60B8[2] = { { NULL, NULL, 358 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A60D0[2] = { { NULL, NULL, 359 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A60E8[4] = {
    { D_WSTAG686_800A6070, NULL, 347 }, { D_WSTAG686_800A6078, NULL, 348 }, { D_WSTAG686_800A6080, NULL, 349 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG686_800A6118[2] = { { NULL, NULL, 351 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A6130[4] = {
    { D_WSTAG686_800A6088, NULL, 352 }, { D_WSTAG686_800A6090, NULL, 353 }, { D_WSTAG686_800A6098, NULL, 354 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG686_800A6160[2] = { { NULL, NULL, 356 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A6178[2] = { { NULL, NULL, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A6190[2] = { { NULL, NULL, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A61A8[2] = { { NULL, NULL, 361 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A61C0[2] = { { NULL, NULL, 361 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A61D8[2] = { { NULL, NULL, 362 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A61F0[2] = { { NULL, NULL, 362 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A6208[2] = { { NULL, NULL, 350 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG686_800A6220[2] = { { NULL, NULL, 355 }, { NULL, NULL, 0 } };
u16 D_WSTAG686_800A6238[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6244[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6250[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A625C[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6264[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A626C[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6274[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A627C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG686_800A6288[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A6290[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG686_800A629C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A62A4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG686_800A62B0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A62B8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG686_800A62C0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG686_800A62C8 = { D_WSTAG686_800A6238, D_WSTAG686_800A60A0, 37, 4, 825, 269, 7 };
FieldstgPlacedActor D_WSTAG686_800A62DC = { D_WSTAG686_800A6244, D_WSTAG686_800A60B8, 38, 5, 560, 304, 7 };
FieldstgPlacedActor D_WSTAG686_800A62F0 = { D_WSTAG686_800A6250, D_WSTAG686_800A60D0, 39, 6, 376, 397, 1 };
FieldstgPlacedActor D_WSTAG686_800A6304 = { D_WSTAG686_800A625C, D_WSTAG686_800A60E8, 57, 7, 560, 448, 7 };
FieldstgPlacedActor D_WSTAG686_800A6318 = { D_WSTAG686_800A6264, D_WSTAG686_800A6118, 57, 7, 560, 448, 7 };
FieldstgPlacedActor D_WSTAG686_800A632C = { D_WSTAG686_800A626C, D_WSTAG686_800A6130, 58, 8, 1017, 309, 1 };
FieldstgPlacedActor D_WSTAG686_800A6340 = { D_WSTAG686_800A6274, D_WSTAG686_800A6160, 58, 8, 584, 436, 7 };
FieldstgPlacedActor D_WSTAG686_800A6354 = { D_WSTAG686_800A627C, D_WSTAG686_800A6178, 157, 9, 825, 269, 7 };
FieldstgPlacedActor D_WSTAG686_800A6368 = { D_WSTAG686_800A6288, D_WSTAG686_800A6190, 157, 9, 825, 269, 7 };
FieldstgPlacedActor D_WSTAG686_800A637C = { D_WSTAG686_800A6290, D_WSTAG686_800A61A8, 158, 10, 560, 304, 7 };
FieldstgPlacedActor D_WSTAG686_800A6390 = { D_WSTAG686_800A629C, D_WSTAG686_800A61C0, 158, 10, 560, 304, 7 };
FieldstgPlacedActor D_WSTAG686_800A63A4 = { D_WSTAG686_800A62A4, D_WSTAG686_800A61D8, 159, 11, 376, 397, 1 };
FieldstgPlacedActor D_WSTAG686_800A63B8 = { D_WSTAG686_800A62B0, D_WSTAG686_800A61F0, 159, 11, 376, 397, 1 };
FieldstgPlacedActor D_WSTAG686_800A63CC = { D_WSTAG686_800A62B8, D_WSTAG686_800A6208, 160, 12, 560, 448, 7 };
FieldstgPlacedActor D_WSTAG686_800A63E0 = { D_WSTAG686_800A62C0, D_WSTAG686_800A6220, 161, 13, 1017, 309, 1 };
FieldstgPlacedActor *wstag686_actors[16] = {
    &D_WSTAG686_800A62C8, &D_WSTAG686_800A62DC, &D_WSTAG686_800A62F0, &D_WSTAG686_800A6304, &D_WSTAG686_800A6318,
    &D_WSTAG686_800A632C, &D_WSTAG686_800A6340, &D_WSTAG686_800A6354, &D_WSTAG686_800A6368, &D_WSTAG686_800A637C,
    &D_WSTAG686_800A6390, &D_WSTAG686_800A63A4, &D_WSTAG686_800A63B8, &D_WSTAG686_800A63CC, &D_WSTAG686_800A63E0,
    NULL,
};
FieldstgSprite wstag686_sprites[9] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 177, 76, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 244, 43, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 350, 54, 0, 0 }, { 1, 0, 0x80, 6, 4, 0, 0, 0, 0, 0, 390, 289, 0, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 1025, 223, 307, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 492, 452, 508, 0 },
    { 1, 0, 0x42, 4, 2, 0, 0, 0, 0, 0, 625, 443, 501, 0 }, { 1, 0, 0x5B, 4, 3, 0, 0, 0, 0, 0, 639, 378, 454, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag686_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C7, 0x290, 0x1F0, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 9, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x1C4, 0xA0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x1D5, 0x12C, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x1BF, 0x150, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x1AF, 0x198, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag686_funcs = { wstag686_setup };
