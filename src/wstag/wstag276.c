#include "wstag.h"

/* WSTAG276: stage 0x281 (fieldstg_stages). */

extern WstagFuncs wstag276_funcs;
const CVECTOR wstag276_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgVramPlace wstag276_vram_places[];
extern FieldstgPlacedActor *wstag276_actors[];
extern FieldstgSprite wstag276_sprites[];
extern FieldstgMapEvent wstag276_map_events[];

void wstag276_update(WstagObject *obj) {
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

WstagObject *wstag276_start(void *arg0) {
    WstagObject *obj = object_new(wstag276_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag276_funcs.setup();
    return obj;
}

void wstag276_setup(void) {
    fieldstg_stage.background_file = 0x52E;
    fieldstg_stage.sprite_file = 0x052F0000;
    fieldstg_stage.sprites = wstag276_sprites;
    fieldstg_stage.map_events = wstag276_map_events;
    fieldstg_stage.mask_file = 0x52D;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10100, 0x1C800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag276_vram_places;
    fieldstg_stage.music = 0x2C;
    fieldstg_stage.sound = 0x60B00000;
    fieldstg_stage.actors = wstag276_actors;
    fieldstg_stage.color = wstag276_color;
    fieldstg_attr.set_file(0, 0x052F0001);
    fieldstg_attr.set_file(7, 0x052F0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag276_setup(void);

FieldstgVramPlace wstag276_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 408, 296, 352, 40, 352, 509 }, { 384, 256, 416, 296, 384, 40, 368, 509 },
};
FieldstgTalk D_WSTAG276_800A6010[2] = { { NULL, NULL, 140 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG276_800A6028[2] = { { NULL, NULL, 139 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG276_800A6040[2] = { { NULL, NULL, 141 }, { NULL, NULL, 0 } };
u16 D_WSTAG276_800A6058[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG276_800A6064[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG276_800A6070[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG276_800A6078 = { D_WSTAG276_800A6058, D_WSTAG276_800A6010, 51, 4, 256, 443, 7 };
FieldstgPlacedActor D_WSTAG276_800A608C = { D_WSTAG276_800A6064, D_WSTAG276_800A6028, 157, 5, 256, 443, 7 };
FieldstgPlacedActor D_WSTAG276_800A60A0 = { D_WSTAG276_800A6070, D_WSTAG276_800A6040, 157, 5, 256, 443, 7 };
FieldstgPlacedActor *wstag276_actors[4] = {
    &D_WSTAG276_800A6078, &D_WSTAG276_800A608C, &D_WSTAG276_800A60A0, NULL,
};
FieldstgSprite wstag276_sprites[19] = {
    { 1, 0, 0x9F, 2, 0x37, 1, 0x37, 0x39, 6, 0, 450, 296, 0, 0 },
    { 1, 0, 0xBE, 2, 0x3A, 1, 0x3A, 0x3C, 6, 0, 560, 270, 0, 0 },
    { 1, 0, 0x8F, 2, 0x1D, 1, 0x1D, 0x31, 8, 0, 584, 352, 0, 0 },
    { 1, 0, 0xB6, 2, 6, 1, 6, 0x1C, 8, 0, 471, 277, 0, 0 }, { 1, 0, 0xB6, 2, 6, 1, 6, 0x1C, 8, 0, 535, 294, 0, 0 },
    { 1, 0, 0x40, 2, 0x53, 0, 0, 0, 0, 0, 90, 366, 0, 0 }, { 1, 0, 0x70, 2, 4, 0, 0, 0, 0, 0, 374, 310, 0, 0 },
    { 1, 0, 0xD0, 2, 5, 0, 0, 0, 0, 0, 438, 232, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 609, 253, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 4, 0, 633, 246, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 640, 242, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 1, 4, 0, 652, 281, 0, 0 },
    { 1, 0, 0x9F, 6, 0x37, 1, 0x37, 0x39, 6, 0, 366, 170, 0, 0 },
    { 1, 0, 0x67, 6, 0x3D, 1, 0x3D, 0x52, 8, 0, 507, 185, 0, 0 },
    { 1, 0, 0xB6, 6, 6, 1, 6, 0x1C, 8, 0, 421, 106, 0, 0 }, { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 91, 362, 0, 0 },
    { 1, 0x65, 0x40, 6, 3, 0, 0, 0, 0, 0, 770, 115, 0, 0 }, { 1, 0, 0x40, 6, 0, 0, 0, 0, 0, 0, 403, 378, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag276_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x282, 0x13A, 0x144, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27F, 0xE8, 0x1A2, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag276_funcs = { wstag276_setup };
