#include "wstag.h"

/* WSTAG476: stage 0x2A6 (fieldstg_stages). */

extern WstagFuncs wstag476_funcs;
extern FieldstgVramPlace wstag476_vram_places[];
extern FieldstgPlacedActor *wstag476_actors[];
extern FieldstgSprite wstag476_sprites[];
extern FieldstgMapEvent wstag476_map_events[];

void wstag476_update(WstagObject *obj) {
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

WstagObject *wstag476_start(void *arg0) {
    WstagObject *obj = object_new(wstag476_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag476_funcs.setup();
    return obj;
}

void wstag476_setup(void) {
    fieldstg_stage.background_file = 0x70C;
    fieldstg_stage.sprite_file = 0x070D0000;
    fieldstg_stage.sprites = wstag476_sprites;
    fieldstg_stage.map_events = wstag476_map_events;
    fieldstg_stage.mask_file = 0x70B;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB100, 0x12B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag476_vram_places;
    fieldstg_stage.music = 0xF;
    fieldstg_stage.sound = 0x603C0000;
    fieldstg_stage.actors = wstag476_actors;
    fieldstg_attr.set_file(0, 0x070D0001);
    fieldstg_attr.set_file(7, 0x070D0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag476_setup(void);

FieldstgVramPlace wstag476_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 336, 511 },
};
FieldstgTalk D_WSTAG476_800A5FDC[2] = { { NULL, NULL, 4 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG476_800A5FF4 = { NULL, D_WSTAG476_800A5FDC, 132, 4, 144, 257, 7 };
FieldstgPlacedActor *wstag476_actors[2] = { &D_WSTAG476_800A5FF4, NULL };
FieldstgSprite wstag476_sprites[5] = {
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 0xA, 0, 216, 262, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3D, 0xA, 0, 72, 170, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x43, 0xA, 0, 348, 225, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x43, 0xA, 0, 355, 229, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag476_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A4, 0x412, 0x1A4, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag476_funcs = { wstag476_setup };
