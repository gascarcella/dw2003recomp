#include "wstag.h"

/* WSTAG947: stage 0x293 (fieldstg_stages_2d). */

extern WstagFuncs wstag947_funcs;
extern FieldstgVramPlace wstag947_vram_places[];
extern FieldstgPlacedActor *wstag947_actors[];
extern FieldstgSprite wstag947_sprites[];
extern FieldstgMapEvent wstag947_map_events[];

void wstag947_update(WstagObject *obj) {
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

WstagObject *wstag947_start(void *arg0) {
    WstagObject *obj = object_new(wstag947_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag947_funcs.setup();
    return obj;
}

void wstag947_setup(void) {
    fieldstg_stage.background_file = 0x239;
    fieldstg_stage.sprite_file = 0x09140000;
    fieldstg_stage.sprites = wstag947_sprites;
    fieldstg_stage.map_events = wstag947_map_events;
    fieldstg_stage.mask_file = 0x915;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB700, 0xD300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag947_vram_places;
    fieldstg_stage.music = 0x31;
    fieldstg_stage.sound = 0x60C40000;
    fieldstg_stage.actors = wstag947_actors;
    fieldstg_attr.set_file(0, 0x09140001);
    fieldstg_attr.set_file(7, 0x09140002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag947_setup(void);

FieldstgVramPlace wstag947_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 350, 312, 120, 56, 352, 511 }, { 320, 256, 358, 332, 152, 76, 368, 511 },
};
FieldstgTalk D_WSTAG947_800A5FE8[2] = { { NULL, NULL, 112 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG947_800A6000[2] = { { NULL, NULL, 113 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG947_800A6018 = { NULL, D_WSTAG947_800A5FE8, 52, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG947_800A602C = { NULL, D_WSTAG947_800A6000, 369, 5, 192, 633, 1 };
FieldstgPlacedActor *wstag947_actors[3] = { &D_WSTAG947_800A6018, &D_WSTAG947_800A602C, NULL };
FieldstgSprite wstag947_sprites[14] = {
    { 1, 0, 0x40, 2, 0x34, 2, 0, 7, 4, 0, 97, 134, 0, 0 }, { 1, 0, 0x40, 2, 0x35, 2, 0, 7, 4, 0, 137, 114, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 321, 137, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 577, 313, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 497, 272, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 4, 0, 167, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 7, 4, 0, 187, 293, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 7, 4, 0, 307, 233, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 276, 208, 242, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 51, 156, 208, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 425, 290, 314, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 421, 428, 438, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 423, 392, 429, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag947_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x292, 0x156, 0x1B0, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag947_funcs = { wstag947_setup };
