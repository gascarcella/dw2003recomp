#include "wstag.h"

/* WSTAG930: stage 0x27B (fieldstg_stages_2d). */

extern WstagFuncs wstag930_funcs;
extern FieldstgVramPlace wstag930_vram_places[];
extern FieldstgPlacedActor *wstag930_actors[];
extern FieldstgSprite wstag930_sprites[];
extern FieldstgMapEvent wstag930_map_events[];

void wstag930_update(WstagObject *obj) {
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

WstagObject *wstag930_start(void *arg0) {
    WstagObject *obj = object_new(wstag930_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag930_funcs.setup();
    return obj;
}

void wstag930_setup(void) {
    fieldstg_stage.background_file = 0x1A4;
    fieldstg_stage.sprite_file = 0x08F30000;
    fieldstg_stage.sprites = wstag930_sprites;
    fieldstg_stage.map_events = wstag930_map_events;
    fieldstg_stage.mask_file = 0x8F2;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB300, 0xFF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag930_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag930_actors;
    fieldstg_attr.set_file(0, 0x08F30001);
    fieldstg_attr.set_file(7, 0x08F30002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag930_setup(void);

FieldstgVramPlace wstag930_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 352, 511 }, { 320, 256, 330, 256, 40, 0, 368, 511 },
    { 320, 256, 340, 256, 80, 0, 352, 510 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgTalk D_WSTAG930_800A6028[2] = { { NULL, NULL, 129 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG930_800A6040[2] = { { NULL, NULL, 130 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG930_800A6058[2] = { { NULL, NULL, 128 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG930_800A6070 = { NULL, D_WSTAG930_800A6028, 40, 4, 135, 212, 1 };
FieldstgPlacedActor D_WSTAG930_800A6084 = { NULL, D_WSTAG930_800A6040, 41, 5, 173, 155, 1 };
FieldstgPlacedActor D_WSTAG930_800A6098 = { NULL, D_WSTAG930_800A6058, 42, 6, 56, 193, 1 };
FieldstgPlacedActor D_WSTAG930_800A60AC = { NULL, NULL, 221, 7, 184, 173, 7 };
FieldstgPlacedActor D_WSTAG930_800A60C0 = { NULL, NULL, 222, 8, 143, 240, 7 };
FieldstgPlacedActor D_WSTAG930_800A60D4 = { NULL, NULL, 223, 9, 64, 224, 7 };
FieldstgPlacedActor *wstag930_actors[7] = {
    &D_WSTAG930_800A6070, &D_WSTAG930_800A6084, &D_WSTAG930_800A6098, &D_WSTAG930_800A60AC, &D_WSTAG930_800A60C0,
    &D_WSTAG930_800A60D4, NULL,
};
FieldstgSprite wstag930_sprites[7] = {
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 62, 115, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 124, 173, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 218, 227, 0, 0 }, { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 242, 146, 0, 0 },
    { 1, 0, 0x40, 2, 0, 2, 0, 1, 4, 0, 295, 210, 0, 0 }, { 1, 0, 0x40, 2, 1, 2, 0, 1, 4, 0, 227, 130, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag930_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0xD8, 0x54, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF6, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0xF0, 0xBF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0xF6, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x12F, 0xAE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag930_funcs = { wstag930_setup };
