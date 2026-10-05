#include "wstag.h"

/* WSTAG251: stage 0x27D (fieldstg_stages). */

extern WstagFuncs wstag251_funcs;
extern FieldstgVramPlace wstag251_vram_places[];
extern FieldstgPlacedActor *wstag251_actors[];
extern FieldstgSprite wstag251_sprites[];
extern FieldstgMapEvent wstag251_map_events[];

void wstag251_update(WstagObject *obj) {
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

WstagObject *wstag251_start(void *arg0) {
    WstagObject *obj = object_new(wstag251_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag251_funcs.setup();
    return obj;
}

void wstag251_setup(void) {
    fieldstg_stage.background_file = 0x4E3;
    fieldstg_stage.sprite_file = 0x04E40000;
    fieldstg_stage.sprites = wstag251_sprites;
    fieldstg_stage.map_events = wstag251_map_events;
    fieldstg_stage.mask_file = 0x4E2;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12100, 0x17D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag251_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag251_actors;
    fieldstg_attr.set_file(0, 0x04E40001);
    fieldstg_attr.set_file(7, 0x04E40002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag251_setup(void);

FieldstgVramPlace wstag251_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 344, 456, 96, 200, 320, 510 }, { 384, 256, 384, 256, 256, 0, 336, 510 },
    { 320, 256, 376, 472, 224, 216, 352, 510 }, { 384, 256, 394, 256, 296, 0, 368, 510 },
    { 384, 256, 422, 256, 408, 0, 320, 509 }, { 384, 256, 430, 256, 440, 0, 336, 509 },
};
u16 D_WSTAG251_800A6030[4] = { 0x7A13, 1, 0xFFFF, 0 };
u16 D_WSTAG251_800A6038[4] = { 0x7A12, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG251_800A6040[2] = { { NULL, D_WSTAG251_800A6030, 425 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG251_800A6058[2] = { { NULL, D_WSTAG251_800A6038, 426 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG251_800A6070[2] = { { NULL, NULL, 115 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG251_800A6088[2] = { { NULL, NULL, 117 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG251_800A60A0[2] = { { NULL, NULL, 118 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG251_800A60B8[2] = { { NULL, NULL, 116 }, { NULL, NULL, 0 } };
u16 D_WSTAG251_800A60D0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG251_800A60DC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG251_800A60E4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG251_800A60EC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG251_800A60F8 = { NULL, D_WSTAG251_800A6040, 22, 4, 451, 178, 7 };
FieldstgPlacedActor D_WSTAG251_800A610C = { NULL, D_WSTAG251_800A6058, 23, 5, 382, 331, 1 };
FieldstgPlacedActor D_WSTAG251_800A6120 = { D_WSTAG251_800A60D0, D_WSTAG251_800A6070, 45, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG251_800A6134 = { D_WSTAG251_800A60DC, D_WSTAG251_800A6088, 54, 7, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG251_800A6148 = { D_WSTAG251_800A60E4, D_WSTAG251_800A60A0, 57, 8, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG251_800A615C = { D_WSTAG251_800A60EC, D_WSTAG251_800A60B8, 58, 9, 195, 402, 1 };
FieldstgPlacedActor *wstag251_actors[7] = {
    &D_WSTAG251_800A60F8, &D_WSTAG251_800A610C, &D_WSTAG251_800A6120, &D_WSTAG251_800A6134, &D_WSTAG251_800A6148,
    &D_WSTAG251_800A615C, NULL,
};
FieldstgSprite wstag251_sprites[7] = {
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x3D, 4, 0, 224, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x4D, 1, 0x4D, 0x50, 8, 0, 73, 439, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x60, 0xA, 0, 90, 278, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x60, 0xA, 0, 133, 435, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x48, 0xA, 0, 67, 338, 0, 0 },
    { 1, 0, 0x40, 6, 0, 0, 0, 0, 0, 0, 314, 248, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag251_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x150, 0x98, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x60, 0x150, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag251_funcs = { wstag251_setup };
