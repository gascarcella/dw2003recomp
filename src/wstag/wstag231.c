#include "wstag.h"

/* WSTAG231: stage 0x277 (fieldstg_stages). */

extern WstagFuncs wstag231_funcs;
extern FieldstgVramPlace wstag231_vram_places[];
extern FieldstgPlacedActor *wstag231_actors[];
extern FieldstgSprite wstag231_sprites[];
extern FieldstgMapEvent wstag231_map_events[];

void wstag231_update(WstagObject *obj) {
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

WstagObject *wstag231_start(void *arg0) {
    WstagObject *obj = object_new(wstag231_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag231_funcs.setup();
    return obj;
}

void wstag231_setup(void) {
    fieldstg_stage.background_file = 0x4B3;
    fieldstg_stage.sprite_file = 0x04B40000;
    fieldstg_stage.sprites = wstag231_sprites;
    fieldstg_stage.map_events = wstag231_map_events;
    fieldstg_stage.mask_file = 0x4B2;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDB00, 0xDF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag231_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag231_actors;
    fieldstg_attr.set_file(0, 0x04B40001);
    fieldstg_attr.set_file(7, 0x04B40002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag231_setup(void);

FieldstgVramPlace wstag231_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 348, 353, 112, 97, 368, 511 }, { 320, 256, 356, 353, 144, 97, 352, 510 },
    { 320, 256, 372, 328, 208, 72, 368, 510 }, { 320, 256, 368, 408, 192, 152, 336, 509 },
    { 320, 256, 364, 353, 176, 97, 352, 509 }, { 320, 256, 320, 373, 0, 117, 368, 509 },
    { 320, 256, 336, 373, 64, 117, 336, 508 }, { 320, 256, 372, 376, 208, 120, 352, 508 },
    { 320, 256, 344, 393, 96, 137, 368, 508 },
};
FieldstgTalk D_WSTAG231_800A60A0[2] = { { NULL, NULL, 428 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A60B8[2] = { { NULL, NULL, 432 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A60D0[2] = { { NULL, NULL, 429 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A60E8[2] = { { NULL, NULL, 433 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A6100[2] = { { NULL, NULL, 434 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A6118[2] = { { NULL, NULL, 435 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A6130[2] = { { NULL, NULL, 430 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG231_800A6148[2] = { { NULL, NULL, 431 }, { NULL, NULL, 0 } };
u16 D_WSTAG231_800A6160[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A616C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A6174[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A6180[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A6188[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A6194[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A619C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A61A4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A61AC[6] = { 0x1A0A, 0, 0x700A, 1, 0xFFFF, 0 };
u16 D_WSTAG231_800A61B8[6] = { 0x700A, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG231_800A61C4[6] = { 0x700A, 1, 0x1A0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG231_800A61D0 = { D_WSTAG231_800A6160, D_WSTAG231_800A60A0, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG231_800A61E4 = { D_WSTAG231_800A616C, D_WSTAG231_800A60B8, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG231_800A61F8 = { D_WSTAG231_800A6174, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG231_800A620C = { D_WSTAG231_800A6180, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG231_800A6220 = { D_WSTAG231_800A6188, D_WSTAG231_800A60D0, 37, 6, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG231_800A6234 = { D_WSTAG231_800A6194, D_WSTAG231_800A60E8, 45, 7, 288, 176, 1 };
FieldstgPlacedActor D_WSTAG231_800A6248 = { D_WSTAG231_800A619C, D_WSTAG231_800A6100, 48, 8, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG231_800A625C = { D_WSTAG231_800A61A4, D_WSTAG231_800A6118, 54, 9, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG231_800A6270 = { D_WSTAG231_800A61AC, D_WSTAG231_800A6130, 157, 10, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG231_800A6284 = { D_WSTAG231_800A61B8, NULL, 158, 11, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG231_800A6298 = { D_WSTAG231_800A61C4, D_WSTAG231_800A6148, 159, 12, 237, 202, 3 };
FieldstgPlacedActor *wstag231_actors[12] = {
    &D_WSTAG231_800A61D0, &D_WSTAG231_800A61E4, &D_WSTAG231_800A61F8, &D_WSTAG231_800A620C, &D_WSTAG231_800A6220,
    &D_WSTAG231_800A6234, &D_WSTAG231_800A6248, &D_WSTAG231_800A625C, &D_WSTAG231_800A6270, &D_WSTAG231_800A6284,
    &D_WSTAG231_800A6298, NULL,
};
FieldstgSprite wstag231_sprites[24] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 192, 256, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 144, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 168, 92, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 80, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 196, 289, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 226, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 266, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 196, 275, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 226, 260, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x39, 0xA, 0, 200, 290, 0, 0 },
    { 1, 0, 0x40, 6, 2, 0, 0, 0, 0, 0, 256, 336, 0, 0 },
    { 1, 0, 0x40, 4, 0x3A, 1, 0x3A, 0x3C, 0xA, 0, 256, 286, 315, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 304, 312, 329, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 256, 280, 315, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag231_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x200, 0x11C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag231_funcs = { wstag231_setup };
