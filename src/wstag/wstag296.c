#include "wstag.h"

/* WSTAG296: stage 0x285 (fieldstg_stages). */

extern WstagFuncs wstag296_funcs;
extern FieldstgVramPlace wstag296_vram_places[];
extern FieldstgPlacedActor *wstag296_actors[];
extern FieldstgSprite wstag296_sprites[];
extern FieldstgMapEvent wstag296_map_events[];

void wstag296_update(WstagObject *obj) {
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

WstagObject *wstag296_start(void *arg0) {
    WstagObject *obj = object_new(wstag296_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag296_funcs.setup();
    return obj;
}

void wstag296_setup(void) {
    fieldstg_stage.background_file = 0x542;
    fieldstg_stage.sprite_file = 0x05430000;
    fieldstg_stage.sprites = wstag296_sprites;
    fieldstg_stage.map_events = wstag296_map_events;
    fieldstg_stage.mask_file = 0x541;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15B00, 0xD500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag296_vram_places;
    fieldstg_stage.music = 0x1F;
    fieldstg_stage.sound = 0x607C0000;
    fieldstg_stage.actors = wstag296_actors;
    fieldstg_attr.set_file(0, 0x05430001);
    fieldstg_attr.set_file(7, 0x05430002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag296_setup(void);

FieldstgVramPlace wstag296_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 368, 511 }, { 320, 256, 328, 256, 32, 0, 368, 510 },
    { 320, 256, 336, 256, 64, 0, 368, 509 }, { 320, 256, 344, 256, 96, 0, 368, 508 },
    { 320, 256, 352, 256, 128, 0, 320, 507 }, { 320, 256, 360, 256, 160, 0, 336, 507 },
};
u16 D_WSTAG296_800A602C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG296_800A6034[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG296_800A603C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG296_800A6044[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG296_800A604C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG296_800A6054[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG296_800A605C = { D_WSTAG296_800A602C, NULL, 69, 4, 210, 74, 5 };
FieldstgPlacedActor D_WSTAG296_800A6070 = { D_WSTAG296_800A6034, NULL, 70, 5, 178, 74, 7 };
FieldstgPlacedActor D_WSTAG296_800A6084 = { D_WSTAG296_800A603C, NULL, 157, 6, 146, 97, 7 };
FieldstgPlacedActor D_WSTAG296_800A6098 = { D_WSTAG296_800A6044, NULL, 158, 7, 210, 74, 5 };
FieldstgPlacedActor D_WSTAG296_800A60AC = { D_WSTAG296_800A604C, NULL, 159, 8, 178, 74, 7 };
FieldstgPlacedActor D_WSTAG296_800A60C0 = { D_WSTAG296_800A6054, NULL, 258, 9, 146, 97, 7 };
FieldstgPlacedActor *wstag296_actors[7] = {
    &D_WSTAG296_800A605C, &D_WSTAG296_800A6070, &D_WSTAG296_800A6084, &D_WSTAG296_800A6098, &D_WSTAG296_800A60AC,
    &D_WSTAG296_800A60C0, NULL,
};
FieldstgSprite wstag296_sprites[13] = {
    { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 320, 53, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 384, 85, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 424, 104, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 455, 145, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 480, 180, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 528, 205, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 3, 4, 0, 104, 68, 0, 0 }, { 1, 0, 0x40, 2, 0x35, 2, 0, 3, 4, 0, 140, 69, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 104, 131, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 198, 84, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 4, 0, 382, 161, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 403, 168, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag296_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x274, 0x70, 0xE4, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag296_funcs = { wstag296_setup };
