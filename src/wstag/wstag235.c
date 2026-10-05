#include "wstag.h"

/* WSTAG235: stage 0x20A (fieldstg_stages). */

extern WstagFuncs wstag235_funcs;
extern FieldstgVramPlace wstag235_vram_places[];
extern FieldstgPlacedActor *wstag235_actors[];
extern FieldstgSprite wstag235_sprites[];
extern FieldstgMapEvent wstag235_map_events[];
extern FieldstgEventDef wstag235_events[];

void wstag235_update(WstagObject *obj) {
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

WstagObject *wstag235_start(void *arg0) {
    WstagObject *obj = object_new(wstag235_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag235_funcs.setup();
    return obj;
}

void wstag235_setup(void) {
    fieldstg_stage.background_file = 0x1A2;
    fieldstg_stage.sprite_file = 0x01A30000;
    fieldstg_stage.sprites = wstag235_sprites;
    fieldstg_stage.map_events = wstag235_map_events;
    fieldstg_stage.mask_file = 0x3CE;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0xBC00, 0xB600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag235_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag235_actors;
    fieldstg_stage.events = wstag235_events;
    fieldstg_attr.set_file(0, 0x01A30002);
    fieldstg_attr.set_file(7, 0x01A30001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag235_setup(void);

s16 D_WSTAG235_800A5F74[57] = {
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
    FIELDSTG_EVENT_GOTO_MAP(0xC01, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag235_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 360, 136, 104, 352, 511 }, { 320, 256, 320, 361, 0, 105, 368, 511 },
    { 320, 256, 346, 360, 104, 104, 352, 510 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 372, 364, 208, 108, 368, 510 },
};
u16 D_WSTAG235_800A6098[4] = { 0x1A08, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A60A0[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60A8[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60B0[4] = { 0x7A1E, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60B8[4] = { 0x1A09, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A60C0[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60C8[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60D0[4] = { 0x9001, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A60D8[4] = { 0x1A23, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A60E0[8] = { 0x1A23, 1, 0x92B7, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60F0[4] = { 0x1A23, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A60F8[4] = { 0x1C34, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6100[6] = { 0x1C34, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A610C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6114[8] = { 0x1C34, 0, 0, 1, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A6124[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A612C[10] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 0,
    0xFFFF, 0,
};
u16 D_WSTAG235_800A6140[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6148[12] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 1,
    3, 0, 0xFFFF, 0,
};
u16 D_WSTAG235_800A6160[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6168[14] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 1,
    3, 1, 4, 0, 0xFFFF, 0,
};
u16 D_WSTAG235_800A6184[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A618C[14] = {
    1, 1, 2, 1, 3, 1, 4, 1,
    0x1C34, 0, 0, 1, 0xFFFF, 0,
};
u16 D_WSTAG235_800A61A8[8] = { 0x708C, 1, 0x1C34, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A61B8[4] = { 0x1C34, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A61C0[6] = { 0x1C34, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A61CC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A61D4[8] = { 0x1C34, 0, 0, 1, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A61E4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A61EC[10] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 0,
    0xFFFF, 0,
};
u16 D_WSTAG235_800A6200[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6208[12] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 1,
    3, 0, 0xFFFF, 0,
};
u16 D_WSTAG235_800A6220[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A6228[14] = {
    0, 1, 1, 1, 2, 1, 3, 1,
    4, 0, 0x1C34, 0, 0xFFFF, 0,
};
u16 D_WSTAG235_800A6244[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A624C[14] = {
    0x1C34, 0, 0, 1, 1, 1, 2, 1,
    3, 1, 4, 1, 0xFFFF, 0,
};
u16 D_WSTAG235_800A6268[8] = { 0x1C34, 1, 0x708C, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG235_800A6278[3] = {
    { D_WSTAG235_800A6098, D_WSTAG235_800A60A0, 396 }, { D_WSTAG235_800A60A8, D_WSTAG235_800A60B0, 397 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG235_800A629C[3] = {
    { D_WSTAG235_800A60B8, D_WSTAG235_800A60C0, 395 }, { D_WSTAG235_800A60C8, D_WSTAG235_800A60D0, 394 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG235_800A62C0[2] = { { NULL, NULL, 30 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A62D8[2] = { { NULL, NULL, 519 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A62F0[2] = { { NULL, NULL, 512 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6308[2] = { { NULL, NULL, 513 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6320[2] = { { NULL, NULL, 514 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6338[2] = { { NULL, NULL, 515 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6350[2] = { { NULL, NULL, 516 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6368[2] = { { NULL, NULL, 517 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6380[2] = { { NULL, NULL, 510 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A6398[2] = { { NULL, NULL, 511 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG235_800A63B0[3] = {
    { D_WSTAG235_800A60D8, D_WSTAG235_800A60E0, 181 }, { D_WSTAG235_800A60F0, NULL, 182 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG235_800A63D4[8] = {
    { D_WSTAG235_800A60F8, NULL, 182 }, { D_WSTAG235_800A6100, D_WSTAG235_800A610C, 5 },
    { D_WSTAG235_800A6114, D_WSTAG235_800A6124, 7 }, { D_WSTAG235_800A612C, D_WSTAG235_800A6140, 7 },
    { D_WSTAG235_800A6148, D_WSTAG235_800A6160, 7 }, { D_WSTAG235_800A6168, D_WSTAG235_800A6184, 7 },
    { D_WSTAG235_800A618C, D_WSTAG235_800A61A8, 6 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG235_800A6434[8] = {
    { D_WSTAG235_800A61B8, NULL, 182 }, { D_WSTAG235_800A61C0, D_WSTAG235_800A61CC, 5 },
    { D_WSTAG235_800A61D4, D_WSTAG235_800A61E4, 7 }, { D_WSTAG235_800A61EC, D_WSTAG235_800A6200, 7 },
    { D_WSTAG235_800A6208, D_WSTAG235_800A6220, 7 }, { D_WSTAG235_800A6228, D_WSTAG235_800A6244, 7 },
    { D_WSTAG235_800A624C, D_WSTAG235_800A6268, 6 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG235_800A6494[2] = { { NULL, NULL, 518 }, { NULL, NULL, 0 } };
u16 D_WSTAG235_800A64AC[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64B4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64BC[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64C4[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64CC[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64D4[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64DC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64E4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64EC[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64F4[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A64FC[6] = { 0x1A22, 1, 0x1A23, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A6508[4] = { 0x1A22, 0, 0xFFFF, 0 };
u16 D_WSTAG235_800A6510[6] = { 0x1A22, 1, 0x1A23, 1, 0xFFFF, 0 };
u16 D_WSTAG235_800A651C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG235_800A6524 = { NULL, D_WSTAG235_800A6278, 20, 4, 188, 154, 7 };
FieldstgPlacedActor D_WSTAG235_800A6538 = { NULL, D_WSTAG235_800A629C, 21, 5, 175, 192, 7 };
FieldstgPlacedActor D_WSTAG235_800A654C = { D_WSTAG235_800A64AC, D_WSTAG235_800A62C0, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A6560 = { D_WSTAG235_800A64B4, D_WSTAG235_800A62D8, 56, 6, 88, 332, 7 };
FieldstgPlacedActor D_WSTAG235_800A6574 = { D_WSTAG235_800A64BC, D_WSTAG235_800A62F0, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A6588 = { D_WSTAG235_800A64C4, D_WSTAG235_800A6308, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A659C = { D_WSTAG235_800A64CC, D_WSTAG235_800A6320, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A65B0 = { D_WSTAG235_800A64D4, D_WSTAG235_800A6338, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A65C4 = { D_WSTAG235_800A64DC, D_WSTAG235_800A6350, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A65D8 = { D_WSTAG235_800A64E4, D_WSTAG235_800A6368, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A65EC = { D_WSTAG235_800A64EC, D_WSTAG235_800A6380, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A6600 = { D_WSTAG235_800A64F4, D_WSTAG235_800A6398, 56, 6, 88, 332, 3 };
FieldstgPlacedActor D_WSTAG235_800A6614 = { D_WSTAG235_800A64FC, D_WSTAG235_800A63B0, 63, 7, 145, 289, 7 };
FieldstgPlacedActor D_WSTAG235_800A6628 = { D_WSTAG235_800A6508, D_WSTAG235_800A63D4, 63, 7, 145, 289, 7 };
FieldstgPlacedActor D_WSTAG235_800A663C = { D_WSTAG235_800A6510, D_WSTAG235_800A6434, 63, 7, 145, 289, 7 };
FieldstgPlacedActor D_WSTAG235_800A6650 = { D_WSTAG235_800A651C, D_WSTAG235_800A6494, 157, 8, 88, 332, 3 };
FieldstgPlacedActor *wstag235_actors[17] = {
    &D_WSTAG235_800A6524, &D_WSTAG235_800A6538, &D_WSTAG235_800A654C, &D_WSTAG235_800A6560, &D_WSTAG235_800A6574,
    &D_WSTAG235_800A6588, &D_WSTAG235_800A659C, &D_WSTAG235_800A65B0, &D_WSTAG235_800A65C4, &D_WSTAG235_800A65D8,
    &D_WSTAG235_800A65EC, &D_WSTAG235_800A6600, &D_WSTAG235_800A6614, &D_WSTAG235_800A6628, &D_WSTAG235_800A663C,
    &D_WSTAG235_800A6650, NULL,
};
FieldstgSprite wstag235_sprites[14] = {
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 78, 77, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 97, 115, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 135, 231, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 141, 44, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 261, 63, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 326, 95, 0, 0 },
    { 1, 0, 0x40, 2, 7, 2, 0, 1, 4, 0, 165, 76, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 256, 151, 184, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 215, 20, 86, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 144, 185, 200, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 125, 175, 194, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 148, 127, 167, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 240, 127, 162, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag235_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20C, 0x96, 0x114, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x200, 0x2A0, 0x17A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20F, 0x15A, 0xBA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20B, 0x146, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xC8, 0xEC, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xB8, 0x132, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag235_funcs = { wstag235_setup };
FieldstgEventDef wstag235_events[2] = {
    { 1200, D_WSTAG235_800A5F74, 0x0112002A, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
