#include "wstag.h"

/* WSTAG506: stage 0x2AC (fieldstg_stages). */

extern WstagFuncs wstag506_funcs;
extern FieldstgVramPlace wstag506_vram_places[];
extern FieldstgPlacedActor *wstag506_actors[];
extern FieldstgSprite wstag506_sprites[];
extern FieldstgMapEvent wstag506_map_events[];
extern FieldstgEventDef wstag506_events[];

void wstag506_update(WstagObject *obj) {
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

WstagObject *wstag506_start(void *arg0) {
    WstagObject *obj = object_new(wstag506_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag506_funcs.setup();
    return obj;
}

void wstag506_setup(void) {
    fieldstg_stage.background_file = 0x5C1;
    fieldstg_stage.sprite_file = 0x05C20000;
    fieldstg_stage.sprites = wstag506_sprites;
    fieldstg_stage.map_events = wstag506_map_events;
    fieldstg_stage.mask_file = 0x5C0;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x27400, 0x1A400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag506_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag506_actors;
    fieldstg_stage.events = wstag506_events;
    fieldstg_attr.set_file(0, 0x05C20001);
    fieldstg_attr.set_file(7, 0x05C20002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag506_setup(void);

s16 D_WSTAG506_800A5F7C[57] = {
    FIELDSTG_EVENT_WALK(2, 720, 416, 5),
    FIELDSTG_EVENT_PLACE(21, 752, 401),
    FIELDSTG_EVENT_ANIM(21, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC0D, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag506_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 293, 216, 37, 320, 511 }, { 320, 256, 342, 323, 88, 67, 336, 511 },
    { 320, 256, 320, 367, 0, 111, 352, 511 }, { 320, 256, 328, 367, 32, 111, 368, 511 },
    { 320, 256, 336, 371, 64, 115, 320, 510 }, { 320, 256, 344, 371, 96, 115, 336, 510 },
};
u16 D_WSTAG506_800A60B0[4] = { 0x7A2C, 1, 0xFFFF, 0 };
u16 D_WSTAG506_800A60B8[4] = { 0x9010, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG506_800A60C0[2] = { { NULL, D_WSTAG506_800A60B0, 419 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A60D8[2] = { { NULL, D_WSTAG506_800A60B8, 418 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A60F0[2] = { { NULL, NULL, 283 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A6108[2] = { { NULL, NULL, 280 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A6120[2] = { { NULL, NULL, 279 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A6138[2] = { { NULL, NULL, 281 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A6150[2] = { { NULL, NULL, 282 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG506_800A6168[2] = { { NULL, NULL, 284 }, { NULL, NULL, 0 } };
u16 D_WSTAG506_800A6180[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG506_800A618C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG506_800A6198[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG506_800A61A4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG506_800A61AC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG506_800A61B8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG506_800A61C0 = { NULL, D_WSTAG506_800A60C0, 20, 4, 673, 370, 1 };
FieldstgPlacedActor D_WSTAG506_800A61D4 = { NULL, D_WSTAG506_800A60D8, 21, 5, 752, 401, 1 };
FieldstgPlacedActor D_WSTAG506_800A61E8 = { D_WSTAG506_800A6180, D_WSTAG506_800A60F0, 48, 6, 273, 217, 1 };
FieldstgPlacedActor D_WSTAG506_800A61FC = { D_WSTAG506_800A618C, D_WSTAG506_800A6108, 53, 7, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG506_800A6210 = { D_WSTAG506_800A6198, D_WSTAG506_800A6120, 157, 8, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG506_800A6224 = { D_WSTAG506_800A61A4, D_WSTAG506_800A6138, 157, 8, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG506_800A6238 = { D_WSTAG506_800A61AC, D_WSTAG506_800A6150, 158, 9, 273, 217, 1 };
FieldstgPlacedActor D_WSTAG506_800A624C = { D_WSTAG506_800A61B8, D_WSTAG506_800A6168, 158, 9, 273, 217, 1 };
FieldstgPlacedActor *wstag506_actors[9] = {
    &D_WSTAG506_800A61C0, &D_WSTAG506_800A61D4, &D_WSTAG506_800A61E8, &D_WSTAG506_800A61FC, &D_WSTAG506_800A6210,
    &D_WSTAG506_800A6224, &D_WSTAG506_800A6238, &D_WSTAG506_800A624C, NULL,
};
FieldstgSprite wstag506_sprites[11] = {
    { 1, 0, 0x50, 4, 0, 0, 0, 0, 0, 0, 510, 318, 407, 0 }, { 1, 0, 0x80, 4, 1, 0, 0, 0, 0, 0, 671, 351, 374, 0 },
    { 1, 0, 0x80, 4, 2, 0, 0, 0, 0, 0, 676, 315, 367, 0 }, { 1, 0, 0xE0, 4, 3, 0, 0, 0, 0, 0, 608, 315, 374, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 175, 201, 239, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 207, 185, 224, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 237, 176, 208, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 632, 227, 236, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 615, 191, 229, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 575, 174, 212, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag506_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AB, 0xE8, 0x17C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag506_funcs = { wstag506_setup };
FieldstgEventDef wstag506_events[2] = {
    { 1221, D_WSTAG506_800A5F7C, 0x013C0009, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
