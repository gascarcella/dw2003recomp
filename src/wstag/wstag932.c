#include "wstag.h"

/* WSTAG932: stage 0x27D (fieldstg_stages_2d). */

extern WstagFuncs wstag932_funcs;
extern FieldstgVramPlace wstag932_vram_places[];
extern FieldstgPlacedActor *wstag932_actors[];
extern FieldstgSprite wstag932_sprites[];
extern FieldstgMapEvent wstag932_map_events[];

void wstag932_update(WstagObject *obj) {
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

WstagObject *wstag932_start(void *arg0) {
    WstagObject *obj = object_new(wstag932_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag932_funcs.setup();
    return obj;
}

void wstag932_setup(void) {
    fieldstg_stage.background_file = 0x1A8;
    fieldstg_stage.sprite_file = 0x08F70000;
    fieldstg_stage.sprites = wstag932_sprites;
    fieldstg_stage.map_events = wstag932_map_events;
    fieldstg_stage.mask_file = 0x8F6;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13D00, 0x16F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag932_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag932_actors;
    fieldstg_attr.set_file(0, 0x08F70001);
    fieldstg_attr.set_file(7, 0x08F70002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag932_setup(void);

FieldstgVramPlace wstag932_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 368, 511 }, { 320, 256, 372, 296, 208, 40, 320, 510 },
    { 320, 256, 372, 328, 208, 72, 336, 510 }, { 320, 256, 372, 368, 208, 112, 352, 510 },
};
u16 D_WSTAG932_800A6010[4] = { 0x7A13, 1, 0xFFFF, 0 };
u16 D_WSTAG932_800A6018[4] = { 0x7A12, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG932_800A6020[2] = { { NULL, D_WSTAG932_800A6010, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG932_800A6038[2] = { { NULL, D_WSTAG932_800A6018, 54 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG932_800A6050[2] = { { NULL, NULL, 55 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG932_800A6068[2] = { { NULL, NULL, 56 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG932_800A6080 = { NULL, D_WSTAG932_800A6020, 22, 4, 451, 178, 7 };
FieldstgPlacedActor D_WSTAG932_800A6094 = { NULL, D_WSTAG932_800A6038, 23, 5, 382, 331, 1 };
FieldstgPlacedActor D_WSTAG932_800A60A8 = { NULL, D_WSTAG932_800A6050, 51, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG932_800A60BC = { NULL, D_WSTAG932_800A6068, 54, 7, 195, 402, 1 };
FieldstgPlacedActor *wstag932_actors[5] = {
    &D_WSTAG932_800A6080, &D_WSTAG932_800A6094, &D_WSTAG932_800A60A8, &D_WSTAG932_800A60BC, NULL,
};
FieldstgSprite wstag932_sprites[20] = {
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x3D, 4, 0, 224, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x53, 0xA, 0, 95, 248, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x53, 0xA, 0, 181, 470, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 1, 0x57, 0x59, 0xA, 0, 123, 226, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 148, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 153, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 114, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 155, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 168, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 105, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 126, 300, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 219, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 59, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 62, 328, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 63, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 95, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 126, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0x4D, 1, 0x4D, 0x50, 8, 0, 73, 439, 0, 0 },
    { 1, 0, 0x40, 6, 0, 1, 0, 5, 4, 0, 299, 241, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag932_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x150, 0x98, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x60, 0x150, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag932_funcs = { wstag932_setup };
