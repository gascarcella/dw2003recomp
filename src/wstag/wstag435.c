#include "wstag.h"

/* WSTAG435: stage 0x231 (fieldstg_stages). */

extern WstagFuncs wstag435_funcs;
extern FieldstgVramPlace wstag435_vram_places[];
extern FieldstgPlacedActor *wstag435_actors[];
extern FieldstgSprite wstag435_sprites[];
extern FieldstgMapEvent wstag435_map_events[];
extern FieldstgEventDef wstag435_events[];

void wstag435_update(WstagObject *obj) {
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

WstagObject *wstag435_start(void *arg0) {
    WstagObject *obj = object_new(wstag435_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag435_funcs.setup();
    return obj;
}

void wstag435_setup(void) {
    fieldstg_stage.background_file = 0x1C0;
    fieldstg_stage.sprites = wstag435_sprites;
    fieldstg_stage.map_events = wstag435_map_events;
    fieldstg_stage.sprite_file = 0x01C10001;
    fieldstg_stage.mask_file = 0x3E0;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15100, 0x18800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag435_vram_places;
    fieldstg_stage.music = 0x30;
    fieldstg_stage.sound = 0x60C00000;
    fieldstg_stage.actors = wstag435_actors;
    fieldstg_stage.events = wstag435_events;
    fieldstg_attr.set_file(0, 0x01C10000);
    fieldstg_attr.set_file(7, 0x01C10002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag435_setup(void);

s16 D_WSTAG435_800A5F7C[184] = {
    FIELDSTG_EVENT_WALK(2, 305, 432, 1),
    FIELDSTG_EVENT_PLACE(62, 256, 455),
    FIELDSTG_EVENT_ANIM(62, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 3, 62, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 5, 62, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 207, 385, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 200, 388, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 152, 444, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 145, 448, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 81, 416, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 62),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(62, 2, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(389, 256, 456),
    FIELDSTG_EVENT_ANIM(389, 86, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(2, 256, 329, 5),
    FIELDSTG_EVENT_ANIM(389, 86, 1),
    FIELDSTG_EVENT_WAIT_ANIM(389),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x22E, 848, 560, 7),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag435_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 256, 136, 0, 336, 511 }, { 320, 256, 370, 256, 200, 0, 352, 511 },
    { 320, 256, 378, 256, 232, 0, 368, 511 },
};
u16 D_WSTAG435_800A617C[4] = { 0x1A01, 0, 0xFFFF, 0 };
u16 D_WSTAG435_800A6184[4] = { 0x1A01, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A618C[6] = { 0x9037, 1, 0x1A0A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG435_800A6198[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A61B0[3] = {
    { D_WSTAG435_800A617C, NULL, 110 }, { D_WSTAG435_800A6184, D_WSTAG435_800A618C, 271 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG435_800A61D4[2] = { { NULL, NULL, 273 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A61EC[2] = { { NULL, NULL, 272 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A6204[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A621C[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A6234[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A624C[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A6264[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A627C[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A6294[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A62AC[2] = { { NULL, NULL, 367 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A62C4[2] = { { NULL, NULL, 367 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG435_800A62DC[2] = { { NULL, NULL, 367 }, { NULL, NULL, 0 } };
u16 D_WSTAG435_800A62F4[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A62FC[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG435_800A6308[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6310[6] = { 0x701A, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG435_800A631C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6324[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A632C[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6334[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A633C[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6344[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A634C[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6354[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A635C[4] = { 0x703C, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A6364[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG435_800A636C[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG435_800A6378 = { D_WSTAG435_800A62F4, D_WSTAG435_800A6198, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A638C = { D_WSTAG435_800A62FC, D_WSTAG435_800A61B0, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A63A0 = { D_WSTAG435_800A6308, D_WSTAG435_800A61D4, 62, 4, 255, 456, 1 };
FieldstgPlacedActor D_WSTAG435_800A63B4 = { D_WSTAG435_800A6310, D_WSTAG435_800A61EC, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A63C8 = { D_WSTAG435_800A631C, D_WSTAG435_800A6204, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A63DC = { D_WSTAG435_800A6324, D_WSTAG435_800A621C, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A63F0 = { D_WSTAG435_800A632C, D_WSTAG435_800A6234, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A6404 = { D_WSTAG435_800A6334, D_WSTAG435_800A624C, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A6418 = { D_WSTAG435_800A633C, D_WSTAG435_800A6264, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A642C = { D_WSTAG435_800A6344, D_WSTAG435_800A627C, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A6440 = { D_WSTAG435_800A634C, D_WSTAG435_800A6294, 62, 4, 256, 455, 1 };
FieldstgPlacedActor D_WSTAG435_800A6454 = { D_WSTAG435_800A6354, D_WSTAG435_800A62AC, 103, 5, 288, 408, 1 };
FieldstgPlacedActor D_WSTAG435_800A6468 = { D_WSTAG435_800A635C, D_WSTAG435_800A62C4, 103, 5, 288, 408, 1 };
FieldstgPlacedActor D_WSTAG435_800A647C = { D_WSTAG435_800A6364, D_WSTAG435_800A62DC, 103, 5, 288, 408, 1 };
FieldstgPlacedActor D_WSTAG435_800A6490 = { D_WSTAG435_800A636C, NULL, 389, 6, 0, 0, 1 };
FieldstgPlacedActor *wstag435_actors[16] = {
    &D_WSTAG435_800A6378, &D_WSTAG435_800A638C, &D_WSTAG435_800A63A0, &D_WSTAG435_800A63B4, &D_WSTAG435_800A63C8,
    &D_WSTAG435_800A63DC, &D_WSTAG435_800A63F0, &D_WSTAG435_800A6404, &D_WSTAG435_800A6418, &D_WSTAG435_800A642C,
    &D_WSTAG435_800A6440, &D_WSTAG435_800A6454, &D_WSTAG435_800A6468, &D_WSTAG435_800A647C, &D_WSTAG435_800A6490,
    NULL,
};
FieldstgSprite wstag435_sprites[3] = {
    { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 51, 95, 0, 0 }, { 1, 0, 0xD0, 2, 0x32, 0, 0, 0, 0, 0, 300, 55, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag435_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x22E, 0x350, 0x230, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag435_funcs = { wstag435_setup };
FieldstgEventDef wstag435_events[2] = {
    { 950, D_WSTAG435_800A5F7C, 0x0135002D, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
