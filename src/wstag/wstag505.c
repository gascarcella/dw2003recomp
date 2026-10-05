#include "wstag.h"

/* WSTAG505: stage 0x23F (fieldstg_stages). */

extern WstagFuncs wstag505_funcs;
extern FieldstgVramPlace wstag505_vram_places[];
extern FieldstgPlacedActor *wstag505_actors[];
extern FieldstgSprite wstag505_sprites[];
extern FieldstgMapEvent wstag505_map_events[];
extern FieldstgEventDef wstag505_events[];

void wstag505_update(WstagObject *obj) {
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

WstagObject *wstag505_start(void *arg0) {
    WstagObject *obj = object_new(wstag505_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag505_funcs.setup();
    return obj;
}

void wstag505_setup(void) {
    fieldstg_stage.background_file = 0x23B;
    fieldstg_stage.sprite_file = 0x023C0000;
    fieldstg_stage.sprites = wstag505_sprites;
    fieldstg_stage.map_events = wstag505_map_events;
    fieldstg_stage.mask_file = 0x3DB;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x28500, 0x1A200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag505_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag505_actors;
    fieldstg_stage.events = wstag505_events;
    fieldstg_attr.set_file(0, 0x023C0002);
    fieldstg_attr.set_file(7, 0x023C0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag505_setup(void);

s16 D_WSTAG505_800A5F7C[57] = {
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
    FIELDSTG_EVENT_GOTO_MAP(0xC04, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag505_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 293, 216, 37, 320, 511 }, { 320, 256, 342, 323, 88, 67, 336, 511 },
    { 320, 256, 320, 367, 0, 111, 352, 511 }, { 320, 256, 328, 367, 32, 111, 368, 511 },
};
u16 D_WSTAG505_800A6090[4] = { 0x7A22, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6098[4] = { 0x900F, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG505_800A60A0[2] = { { NULL, D_WSTAG505_800A6090, 5 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A60B8[2] = { { NULL, D_WSTAG505_800A6098, 2 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A60D0[2] = { { NULL, NULL, 143 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A60E8[2] = { { NULL, NULL, 134 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6100[2] = { { NULL, NULL, 135 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6118[2] = { { NULL, NULL, 136 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6130[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6148[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6160[2] = { { NULL, NULL, 139 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6178[2] = { { NULL, NULL, 139 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A6190[2] = { { NULL, NULL, 140 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A61A8[2] = { { NULL, NULL, 141 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A61C0[2] = { { NULL, NULL, 111 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG505_800A61D8[2] = { { NULL, NULL, 142 }, { NULL, NULL, 0 } };
u16 D_WSTAG505_800A61F0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A61F8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6200[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6208[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6210[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6218[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6220[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6228[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6230[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6238[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6240[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG505_800A6248[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG505_800A6250 = { NULL, D_WSTAG505_800A60A0, 20, 4, 673, 370, 1 };
FieldstgPlacedActor D_WSTAG505_800A6264 = { NULL, D_WSTAG505_800A60B8, 21, 5, 752, 401, 1 };
FieldstgPlacedActor D_WSTAG505_800A6278 = { D_WSTAG505_800A61F0, D_WSTAG505_800A60D0, 54, 6, 273, 217, 1 };
FieldstgPlacedActor D_WSTAG505_800A628C = { D_WSTAG505_800A61F8, D_WSTAG505_800A60E8, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A62A0 = { D_WSTAG505_800A6200, D_WSTAG505_800A6100, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A62B4 = { D_WSTAG505_800A6208, D_WSTAG505_800A6118, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A62C8 = { D_WSTAG505_800A6210, D_WSTAG505_800A6130, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A62DC = { D_WSTAG505_800A6218, D_WSTAG505_800A6148, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A62F0 = { D_WSTAG505_800A6220, D_WSTAG505_800A6160, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A6304 = { D_WSTAG505_800A6228, D_WSTAG505_800A6178, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A6318 = { D_WSTAG505_800A6230, D_WSTAG505_800A6190, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A632C = { D_WSTAG505_800A6238, D_WSTAG505_800A61A8, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A6340 = { D_WSTAG505_800A6240, D_WSTAG505_800A61C0, 54, 6, 550, 222, 5 };
FieldstgPlacedActor D_WSTAG505_800A6354 = { D_WSTAG505_800A6248, D_WSTAG505_800A61D8, 157, 7, 550, 222, 5 };
FieldstgPlacedActor *wstag505_actors[15] = {
    &D_WSTAG505_800A6250, &D_WSTAG505_800A6264, &D_WSTAG505_800A6278, &D_WSTAG505_800A628C, &D_WSTAG505_800A62A0,
    &D_WSTAG505_800A62B4, &D_WSTAG505_800A62C8, &D_WSTAG505_800A62DC, &D_WSTAG505_800A62F0, &D_WSTAG505_800A6304,
    &D_WSTAG505_800A6318, &D_WSTAG505_800A632C, &D_WSTAG505_800A6340, &D_WSTAG505_800A6354, NULL,
};
FieldstgSprite wstag505_sprites[11] = {
    { 1, 0, 0x50, 4, 0, 0, 0, 0, 0, 0, 510, 318, 407, 0 }, { 1, 0, 0x80, 4, 1, 0, 0, 0, 0, 0, 671, 351, 374, 0 },
    { 1, 0, 0x80, 4, 2, 0, 0, 0, 0, 0, 676, 315, 367, 0 }, { 1, 0, 0xE0, 4, 3, 0, 0, 0, 0, 0, 608, 315, 374, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 175, 201, 239, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 207, 185, 224, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 237, 176, 208, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 632, 227, 236, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 615, 191, 229, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 575, 174, 212, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag505_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23E, 0xE8, 0x17C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag505_funcs = { wstag505_setup };
FieldstgEventDef wstag505_events[2] = {
    { 1220, D_WSTAG505_800A5F7C, 0x013C0008, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
