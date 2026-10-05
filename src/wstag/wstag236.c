#include "wstag.h"

/* WSTAG236: stage 0x279 (fieldstg_stages). */

extern WstagFuncs wstag236_funcs;
extern FieldstgVramPlace wstag236_vram_places[];
extern FieldstgPlacedActor *wstag236_actors[];
extern FieldstgSprite wstag236_sprites[];
extern FieldstgMapEvent wstag236_map_events[];
extern FieldstgEventDef wstag236_events[];

void wstag236_update(WstagObject *obj) {
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

WstagObject *wstag236_start(void *arg0) {
    WstagObject *obj = object_new(wstag236_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag236_funcs.setup();
    return obj;
}

void wstag236_setup(void) {
    fieldstg_stage.background_file = 0x4D7;
    fieldstg_stage.sprite_file = 0x04D80000;
    fieldstg_stage.sprites = wstag236_sprites;
    fieldstg_stage.map_events = wstag236_map_events;
    fieldstg_stage.mask_file = 0x4D6;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x9200, 0x13C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag236_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag236_actors;
    fieldstg_stage.events = wstag236_events;
    fieldstg_attr.set_file(0, 0x04D80001);
    fieldstg_attr.set_file(7, 0x04D80002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag236_setup(void);

s16 D_WSTAG236_800A5F78[57] = {
    FIELDSTG_EVENT_WALK(2, 207, 208, 3),
    FIELDSTG_EVENT_PLACE(21, 175, 192),
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
    FIELDSTG_EVENT_GOTO_MAP(0xC0A, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag236_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 328, 363, 32, 107, 352, 511 }, { 320, 256, 371, 256, 204, 0, 368, 511 },
    { 320, 256, 364, 361, 176, 105, 352, 510 }, { 320, 256, 336, 363, 64, 107, 368, 510 },
    { 320, 256, 344, 363, 96, 107, 320, 509 },
};
u16 D_WSTAG236_800A609C[4] = { 0x7A28, 1, 0xFFFF, 0 };
u16 D_WSTAG236_800A60A4[4] = { 0x9002, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG236_800A60AC[2] = { { NULL, D_WSTAG236_800A609C, 419 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG236_800A60C4[2] = { { NULL, D_WSTAG236_800A60A4, 418 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG236_800A60DC[2] = { { NULL, NULL, 97 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG236_800A60F4[2] = { { NULL, NULL, 95 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG236_800A610C[2] = { { NULL, NULL, 94 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG236_800A6124[2] = { { NULL, NULL, 96 }, { NULL, NULL, 0 } };
u16 D_WSTAG236_800A613C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG236_800A6144[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG236_800A6150[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG236_800A615C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG236_800A6164 = { NULL, D_WSTAG236_800A60AC, 20, 4, 188, 154, 7 };
FieldstgPlacedActor D_WSTAG236_800A6178 = { NULL, D_WSTAG236_800A60C4, 21, 5, 175, 192, 7 };
FieldstgPlacedActor D_WSTAG236_800A618C = { D_WSTAG236_800A613C, D_WSTAG236_800A60DC, 48, 6, 88, 332, 7 };
FieldstgPlacedActor D_WSTAG236_800A61A0 = { D_WSTAG236_800A6144, D_WSTAG236_800A60F4, 57, 7, 88, 332, 7 };
FieldstgPlacedActor D_WSTAG236_800A61B4 = { D_WSTAG236_800A6150, D_WSTAG236_800A610C, 157, 8, 88, 332, 7 };
FieldstgPlacedActor D_WSTAG236_800A61C8 = { D_WSTAG236_800A615C, D_WSTAG236_800A6124, 157, 8, 88, 332, 7 };
FieldstgPlacedActor *wstag236_actors[7] = {
    &D_WSTAG236_800A6164, &D_WSTAG236_800A6178, &D_WSTAG236_800A618C, &D_WSTAG236_800A61A0, &D_WSTAG236_800A61B4,
    &D_WSTAG236_800A61C8, NULL,
};
FieldstgSprite wstag236_sprites[14] = {
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 78, 77, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 97, 115, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 135, 231, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 141, 44, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 261, 63, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 326, 95, 0, 0 },
    { 1, 0, 0x40, 2, 7, 2, 0, 1, 4, 0, 165, 76, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 256, 151, 184, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 240, 127, 162, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 215, 20, 86, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 144, 185, 200, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 125, 175, 194, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 148, 127, 167, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag236_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27B, 0x96, 0x114, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x2A0, 0x17A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27E, 0x15A, 0xBA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x146, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xC8, 0xEC, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xB8, 0x132, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag236_funcs = { wstag236_setup };
FieldstgEventDef wstag236_events[2] = {
    { 1201, D_WSTAG236_800A5F78, 0x0112002B, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
