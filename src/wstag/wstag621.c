#include "wstag.h"

/* WSTAG621: stage 0x2BF (fieldstg_stages). */

extern WstagFuncs wstag621_funcs;
extern FieldstgVramPlace wstag621_vram_places[];
extern FieldstgPlacedActor *wstag621_actors[];
extern FieldstgSprite wstag621_sprites[];
extern FieldstgMapEvent wstag621_map_events[];

void wstag621_update(WstagObject *obj) {
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

WstagObject *wstag621_start(void *arg0) {
    WstagObject *obj = object_new(wstag621_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag621_funcs.setup();
    return obj;
}

void wstag621_setup(void) {
    fieldstg_stage.background_file = 0x616;
    fieldstg_stage.sprite_file = 0x06170000;
    fieldstg_stage.sprites = wstag621_sprites;
    fieldstg_stage.map_events = wstag621_map_events;
    fieldstg_stage.mask_file = 0x615;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x21C00, 0x18300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag621_vram_places;
    fieldstg_stage.music = 0x15;
    fieldstg_stage.sound = 0x60540000;
    fieldstg_stage.actors = wstag621_actors;
    fieldstg_attr.set_file(0, 0x06170001);
    fieldstg_attr.set_file(7, 0x06170002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag621_setup(void);

FieldstgVramPlace wstag621_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 336, 208, 80, 352, 502 }, { 320, 256, 332, 376, 48, 120, 352, 501 },
    { 320, 256, 340, 376, 80, 120, 368, 501 }, { 320, 256, 364, 400, 176, 144, 320, 500 },
    { 320, 256, 320, 408, 0, 152, 352, 500 }, { 320, 256, 372, 408, 208, 152, 368, 500 },
    { 320, 256, 328, 416, 32, 160, 320, 499 }, { 320, 256, 336, 416, 64, 160, 336, 499 },
    { 320, 256, 344, 416, 96, 160, 352, 499 }, { 320, 256, 348, 376, 112, 120, 368, 499 },
};
u16 D_WSTAG621_800A6070[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6078[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6080[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6088[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6090[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6098[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60A0[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60A8[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60B0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60B8[4] = { 0x8668, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60C0[6] = { 0x8668, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG621_800A60CC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60D4[8] = { 0x8668, 0, 0, 1, 0x8464, 0, 0xFFFF, 0 };
u16 D_WSTAG621_800A60E4[8] = { 0x8668, 0, 0, 1, 0x8464, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A60F4[10] = {
    0x8668, 1, 0x8667, 0, 0x8464, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG621_800A6108[4] = {
    { D_WSTAG621_800A6070, NULL, 287 }, { D_WSTAG621_800A6078, NULL, 288 }, { D_WSTAG621_800A6080, NULL, 295 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG621_800A6138[4] = {
    { D_WSTAG621_800A6088, NULL, 290 }, { D_WSTAG621_800A6090, NULL, 291 }, { D_WSTAG621_800A6098, NULL, 296 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG621_800A6168[4] = {
    { D_WSTAG621_800A60A0, NULL, 293 }, { D_WSTAG621_800A60A8, NULL, 294 }, { D_WSTAG621_800A60B0, NULL, 297 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG621_800A6198[2] = { { NULL, NULL, 286 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG621_800A61B0[2] = { { NULL, NULL, 289 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG621_800A61C8[2] = { { NULL, NULL, 292 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG621_800A61E0[5] = {
    { D_WSTAG621_800A60B8, NULL, 766 }, { D_WSTAG621_800A60C0, D_WSTAG621_800A60CC, 767 },
    { D_WSTAG621_800A60D4, NULL, 768 }, { D_WSTAG621_800A60E4, D_WSTAG621_800A60F4, 769 }, { NULL, NULL, 0 },
};
u16 D_WSTAG621_800A621C[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6224[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A622C[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6234[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A623C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A6244[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG621_800A624C[10] = {
    0x7042, 1, 0x704A, 1, 0x8667, 1, 0x8668, 0,
    0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG621_800A6260 = { D_WSTAG621_800A621C, D_WSTAG621_800A6108, 32, 4, 385, 417, 3 };
FieldstgPlacedActor D_WSTAG621_800A6274 = { D_WSTAG621_800A6224, D_WSTAG621_800A6138, 36, 5, 449, 360, 5 };
FieldstgPlacedActor D_WSTAG621_800A6288 = { D_WSTAG621_800A622C, D_WSTAG621_800A6168, 89, 6, 515, 414, 7 };
FieldstgPlacedActor D_WSTAG621_800A629C = { NULL, NULL, 154, 7, 641, 350, 0 };
FieldstgPlacedActor D_WSTAG621_800A62B0 = { NULL, NULL, 155, 8, 593, 325, 1 };
FieldstgPlacedActor D_WSTAG621_800A62C4 = { NULL, NULL, 156, 9, 545, 301, 2 };
FieldstgPlacedActor D_WSTAG621_800A62D8 = { D_WSTAG621_800A6234, D_WSTAG621_800A6198, 157, 10, 385, 417, 3 };
FieldstgPlacedActor D_WSTAG621_800A62EC = { D_WSTAG621_800A623C, D_WSTAG621_800A61B0, 158, 11, 449, 360, 5 };
FieldstgPlacedActor D_WSTAG621_800A6300 = { D_WSTAG621_800A6244, D_WSTAG621_800A61C8, 159, 12, 515, 414, 7 };
FieldstgPlacedActor D_WSTAG621_800A6314 = { D_WSTAG621_800A624C, D_WSTAG621_800A61E0, 172, 13, 400, 232, 7 };
FieldstgPlacedActor *wstag621_actors[11] = {
    &D_WSTAG621_800A6260, &D_WSTAG621_800A6274, &D_WSTAG621_800A6288, &D_WSTAG621_800A629C, &D_WSTAG621_800A62B0,
    &D_WSTAG621_800A62C4, &D_WSTAG621_800A62D8, &D_WSTAG621_800A62EC, &D_WSTAG621_800A6300, &D_WSTAG621_800A6314,
    NULL,
};
FieldstgSprite wstag621_sprites[9] = {
    { 1, 0, 0x40, 2, 0x40, 2, 0, 3, 6, 0, 521, 212, 0, 0 }, { 1, 0, 0x40, 2, 0x40, 2, 0, 3, 6, 0, 569, 236, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 3, 6, 0, 617, 260, 0, 0 }, { 1, 0, 0x40, 2, 0x43, 2, 0, 5, 6, 0, 392, 163, 0, 0 },
    { 1, 0, 0x40, 2, 0x44, 2, 0, 5, 6, 0, 369, 205, 0, 0 }, { 1, 0, 0x40, 6, 0x3F, 2, 0, 3, 6, 0, 360, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 2, 0, 5, 6, 0, 361, 186, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 377, 371, 399, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag621_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BE, 0x2D4, 0xC4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x1D0, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x1E0, 0x150, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag621_funcs = { wstag621_setup };
