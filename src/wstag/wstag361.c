#include "wstag.h"

/* WSTAG361: stage 0x292 (fieldstg_stages). */

extern WstagFuncs wstag361_funcs;
extern FieldstgVramPlace wstag361_vram_places[];
extern FieldstgPlacedActor *wstag361_actors[];
extern FieldstgSprite wstag361_sprites[];
extern FieldstgMapEvent wstag361_map_events[];
extern FieldstgEventDef wstag361_events[];

void wstag361_update(WstagObject *obj) {
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

WstagObject *wstag361_start(void *arg0) {
    WstagObject *obj = object_new(wstag361_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag361_funcs.setup();
    return obj;
}

void wstag361_setup(void) {
    fieldstg_stage.background_file = 0x594;
    fieldstg_stage.sprite_file = 0x05950000;
    fieldstg_stage.sprites = wstag361_sprites;
    fieldstg_stage.map_events = wstag361_map_events;
    fieldstg_stage.mask_file = 0x593;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x17C00, 0x14F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag361_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag361_actors;
    fieldstg_stage.events = wstag361_events;
    fieldstg_attr.set_file(0, 0x05950001);
    fieldstg_attr.set_file(7, 0x05950002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag361_setup(void);

s16 D_WSTAG361_800A5F7C[57] = {
    FIELDSTG_EVENT_WALK(2, 153, 181, 3),
    FIELDSTG_EVENT_PLACE(21, 121, 165),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC0B, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag361_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 358, 0, 102, 336, 507 }, { 320, 256, 368, 256, 192, 0, 352, 507 },
    { 320, 256, 360, 361, 160, 105, 368, 507 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG361_800A6090[4] = { 0x7A29, 1, 0xFFFF, 0 };
u16 D_WSTAG361_800A6098[4] = { 0x900B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG361_800A60A0[2] = { { NULL, D_WSTAG361_800A6090, 718 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG361_800A60B8[2] = { { NULL, D_WSTAG361_800A6098, 719 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG361_800A60D0 = { NULL, D_WSTAG361_800A60A0, 20, 4, 345, 308, 7 };
FieldstgPlacedActor D_WSTAG361_800A60E4 = { NULL, D_WSTAG361_800A60B8, 21, 5, 121, 165, 7 };
FieldstgPlacedActor D_WSTAG361_800A60F8 = { NULL, NULL, 120, 6, 374, 268, 5 };
FieldstgPlacedActor D_WSTAG361_800A610C = { NULL, NULL, 219, 7, 346, 325, 7 };
FieldstgPlacedActor *wstag361_actors[5] = {
    &D_WSTAG361_800A60D0, &D_WSTAG361_800A60E4, &D_WSTAG361_800A60F8, &D_WSTAG361_800A610C, NULL,
};
FieldstgSprite wstag361_sprites[30] = {
    { 1, 0, 0x40, 6, 0x32, 0, 0, 0, 0, 0, 484, 223, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 57, 39, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 89, 23, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 203, 224, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 272, 360, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 292, 139, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 305, 376, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 337, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 467, 220, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 344, 109, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 321, 107, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 370, 108, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 289, 326, 340, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 294, 310, 332, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 322, 310, 325, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 337, 303, 317, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 351, 282, 309, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 375, 277, 301, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 383, 270, 293, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 400, 271, 285, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 415, 270, 282, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 77, 127, 175, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 109, 113, 158, 0 }, { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 141, 97, 142, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 174, 81, 127, 0 }, { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 257, 176, 209, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 255, 245, 263, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 239, 252, 272, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 420, 237, 278, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag361_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x80, 0x31C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x293, 0x88, 0xBC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xFA, 0x126, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x10B, 0x188, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag361_funcs = { wstag361_setup };
FieldstgEventDef wstag361_events[2] = {
    { 1205, D_WSTAG361_800A5F7C, 0x0127001A, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
