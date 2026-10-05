#include "wstag.h"

/* WSTAG959: stage 0x29F (fieldstg_stages_2d). */

extern WstagFuncs wstag959_funcs;
extern FieldstgVramPlace wstag959_vram_places[];
extern FieldstgPlacedActor *wstag959_actors[];
extern FieldstgSprite wstag959_sprites[];
extern FieldstgMapEvent wstag959_map_events[];

void wstag959_update(WstagObject *obj) {
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

WstagObject *wstag959_start(void *arg0) {
    WstagObject *obj = object_new(wstag959_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag959_funcs.setup();
    return obj;
}

void wstag959_setup(void) {
    fieldstg_stage.background_file = 0x1C0;
    fieldstg_stage.sprite_file = 0x092D0000;
    fieldstg_stage.sprites = wstag959_sprites;
    fieldstg_stage.map_events = wstag959_map_events;
    fieldstg_stage.mask_file = 0x92C;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15100, 0x18800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag959_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag959_actors;
    fieldstg_attr.set_file(0, 0x092D0001);
    fieldstg_attr.set_file(7, 0x092D0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag959_setup(void);

FieldstgVramPlace wstag959_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 256, 136, 0, 336, 511 },
};
FieldstgTalk D_WSTAG959_800A5FE0[2] = { { NULL, NULL, 127 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG959_800A5FF8 = { NULL, D_WSTAG959_800A5FE0, 62, 4, 256, 455, 1 };
FieldstgPlacedActor *wstag959_actors[2] = { &D_WSTAG959_800A5FF8, NULL };
FieldstgSprite wstag959_sprites[3] = {
    { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 51, 95, 0, 0 }, { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 300, 55, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag959_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x350, 0x230, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag959_funcs = { wstag959_setup };
