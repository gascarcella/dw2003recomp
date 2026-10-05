#include "wstag.h"

/* WSTAG238: stage 0x27A (fieldstg_stages). */

extern WstagFuncs wstag238_funcs;
extern FieldstgVramPlace wstag238_vram_places[];
extern FieldstgSprite wstag238_sprites[];
extern FieldstgMapEvent wstag238_map_events[];

void wstag238_update(WstagObject *obj) {
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

WstagObject *wstag238_start(void *arg0) {
    WstagObject *obj = object_new(wstag238_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag238_funcs.setup();
    return obj;
}

void wstag238_setup(void) {
    fieldstg_stage.background_file = 0x4D1;
    fieldstg_stage.sprite_file = 0x04D20000;
    fieldstg_stage.sprites = wstag238_sprites;
    fieldstg_stage.map_events = wstag238_map_events;
    fieldstg_stage.mask_file = 0x4D0;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xED00, 0xE900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag238_vram_places;
    fieldstg_stage.music = 0x1F;
    fieldstg_stage.sound = 0x607C0000;
    fieldstg_attr.set_file(0, 0x04D20001);
    fieldstg_attr.set_file(7, 0x04D20002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag238_setup(void);

FieldstgVramPlace wstag238_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag238_sprites[10] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 261, 137, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 53, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 173, 179, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 325, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 435, 83, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 523, 127, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 555, 239, 0, 0 }, { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 286, 127, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 224, 144, 184, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag238_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0xF0, 0x108, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28A, 0x138, 0x64, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0xA7, 0x153, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x203, 0xC2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x212, 0x128, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag238_funcs = { wstag238_setup };
