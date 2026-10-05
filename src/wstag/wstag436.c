#include "wstag.h"

/* WSTAG436: stage 0x29F (fieldstg_stages). */

extern WstagFuncs wstag436_funcs;
extern FieldstgVramPlace wstag436_vram_places[];
extern FieldstgPlacedActor *wstag436_actors[];
extern FieldstgSprite wstag436_sprites[];
extern FieldstgMapEvent wstag436_map_events[];

void wstag436_update(WstagObject *obj) {
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

WstagObject *wstag436_start(void *arg0) {
    WstagObject *obj = object_new(wstag436_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag436_funcs.setup();
    return obj;
}

void wstag436_setup(void) {
    fieldstg_stage.background_file = 0x5A9;
    fieldstg_stage.sprite_file = 0x05AA0000;
    fieldstg_stage.sprites = wstag436_sprites;
    fieldstg_stage.map_events = wstag436_map_events;
    fieldstg_stage.mask_file = 0x5A8;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1B100, 0x1DA00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag436_vram_places;
    fieldstg_stage.music = 0x30;
    fieldstg_stage.sound = 0x60C00000;
    fieldstg_stage.actors = wstag436_actors;
    fieldstg_attr.set_file(0, 0x05AA0001);
    fieldstg_attr.set_file(7, 0x05AA0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag436_setup(void);

FieldstgVramPlace wstag436_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 256, 136, 0, 336, 511 },
};
FieldstgTalk D_WSTAG436_800A5FE0[2] = { { NULL, NULL, 257 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG436_800A5FF8[2] = { { NULL, NULL, 258 }, { NULL, NULL, 0 } };
u16 D_WSTAG436_800A6010[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG436_800A601C[6] = { 0x701A, 1, 0x1A0A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG436_800A6028 = { D_WSTAG436_800A6010, D_WSTAG436_800A5FE0, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG436_800A603C = { D_WSTAG436_800A601C, D_WSTAG436_800A5FF8, 62, 4, 256, 455, 1 };
FieldstgPlacedActor *wstag436_actors[3] = { &D_WSTAG436_800A6028, &D_WSTAG436_800A603C, NULL };
FieldstgSprite wstag436_sprites[3] = {
    { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 51, 95, 0, 0 }, { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 300, 55, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag436_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x350, 0x230, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag436_funcs = { wstag436_setup };
