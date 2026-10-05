#include "wstag.h"

/* WSTAG470: stage 0x238 (fieldstg_stages). */

extern WstagFuncs wstag470_funcs;
extern FieldstgVramPlace wstag470_vram_places[];
extern FieldstgPlacedActor *wstag470_actors[];
extern FieldstgSprite wstag470_sprites[];
extern FieldstgMapEvent wstag470_map_events[];
extern FieldstgEventDef wstag470_events[];

void wstag470_update(WstagObject *obj) {
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

WstagObject *wstag470_start(void *arg0) {
    WstagObject *obj = object_new(wstag470_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag470_funcs.setup();
    return obj;
}

void wstag470_setup(void) {
    fieldstg_stage.background_file = 0x233;
    fieldstg_stage.sprite_file = 0x02350000;
    fieldstg_stage.sprites = wstag470_sprites;
    fieldstg_stage.map_events = wstag470_map_events;
    fieldstg_stage.mask_file = 0x2D5;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1DB00, 0x13C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag470_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag470_actors;
    fieldstg_stage.events = wstag470_events;
    fieldstg_attr.set_file(0, 0x02350001);
    fieldstg_attr.set_file(7, 0x02350002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag470_setup(void);

s16 D_WSTAG470_800A5F7C[57] = {
    FIELDSTG_EVENT_WALK(2, 160, 328, 3),
    FIELDSTG_EVENT_PLACE(21, 128, 312),
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
    FIELDSTG_EVENT_GOTO_MAP(0xC03, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag470_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 405, 216, 149, 368, 511 }, { 384, 256, 434, 371, 456, 115, 368, 510 },
    { 384, 256, 436, 327, 464, 71, 368, 509 }, { 384, 256, 432, 419, 448, 163, 368, 508 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG470_800A60A0[4] = { 0x1A08, 0, 0xFFFF, 0 };
u16 D_WSTAG470_800A60A8[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60B0[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60B8[4] = { 0x7A21, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60C0[4] = { 0x1A09, 0, 0xFFFF, 0 };
u16 D_WSTAG470_800A60C8[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60D0[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60D8[4] = { 0x900E, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60E0[4] = { 0x1C1C, 0, 0xFFFF, 0 };
u16 D_WSTAG470_800A60E8[4] = { 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A60F0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG470_800A60F8[6] = { 0x1C43, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6104[4] = { 0, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG470_800A610C[3] = {
    { D_WSTAG470_800A60A0, D_WSTAG470_800A60A8, 362 }, { D_WSTAG470_800A60B0, D_WSTAG470_800A60B8, 363 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG470_800A6130[3] = {
    { D_WSTAG470_800A60C0, D_WSTAG470_800A60C8, 361 }, { D_WSTAG470_800A60D0, D_WSTAG470_800A60D8, 360 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG470_800A6154[2] = { { NULL, NULL, 33 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A616C[2] = { { NULL, NULL, 273 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A6184[2] = { { NULL, NULL, 274 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A619C[2] = { { NULL, NULL, 275 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A61B4[2] = { { NULL, NULL, 276 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A61CC[2] = { { NULL, NULL, 277 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A61E4[2] = { { NULL, NULL, 278 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A61FC[2] = { { NULL, NULL, 280 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A6214[2] = { { NULL, NULL, 281 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A622C[2] = { { NULL, NULL, 282 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A6244[2] = { { NULL, NULL, 279 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A625C[2] = { { NULL, NULL, 279 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A6274[2] = { { NULL, NULL, 33 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG470_800A628C[3] = {
    { D_WSTAG470_800A60E0, NULL, 277 }, { D_WSTAG470_800A60E8, NULL, 85 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG470_800A62B0[3] = {
    { D_WSTAG470_800A60F0, D_WSTAG470_800A60F8, 806 }, { D_WSTAG470_800A6104, NULL, 809 }, { NULL, NULL, 0 },
};
u16 D_WSTAG470_800A62D4[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A62DC[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A62E4[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A62EC[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A62F4[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A62FC[6] = { 0x7017, 1, 0x6014, 0, 0xFFFF, 0 };
u16 D_WSTAG470_800A6308[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6310[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6318[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6320[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6328[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6330[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6338[4] = { 0x6007, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6340[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG470_800A6348[4] = { 0x6008, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG470_800A6350 = { NULL, D_WSTAG470_800A610C, 20, 4, 357, 278, 7 };
FieldstgPlacedActor D_WSTAG470_800A6364 = { NULL, D_WSTAG470_800A6130, 21, 5, 128, 312, 7 };
FieldstgPlacedActor D_WSTAG470_800A6378 = { NULL, NULL, 120, 6, 368, 232, 5 };
FieldstgPlacedActor D_WSTAG470_800A638C = { D_WSTAG470_800A62D4, D_WSTAG470_800A6154, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A63A0 = { D_WSTAG470_800A62DC, D_WSTAG470_800A616C, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A63B4 = { D_WSTAG470_800A62E4, D_WSTAG470_800A6184, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A63C8 = { D_WSTAG470_800A62EC, D_WSTAG470_800A619C, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A63DC = { D_WSTAG470_800A62F4, D_WSTAG470_800A61B4, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A63F0 = { D_WSTAG470_800A62FC, D_WSTAG470_800A61CC, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6404 = { D_WSTAG470_800A6308, D_WSTAG470_800A61E4, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6418 = { D_WSTAG470_800A6310, D_WSTAG470_800A61FC, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A642C = { D_WSTAG470_800A6318, D_WSTAG470_800A6214, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6440 = { D_WSTAG470_800A6320, D_WSTAG470_800A622C, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6454 = { D_WSTAG470_800A6328, D_WSTAG470_800A6244, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6468 = { D_WSTAG470_800A6330, D_WSTAG470_800A625C, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A647C = { D_WSTAG470_800A6338, D_WSTAG470_800A6274, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A6490 = { D_WSTAG470_800A6340, D_WSTAG470_800A628C, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A64A4 = { D_WSTAG470_800A6348, D_WSTAG470_800A62B0, 121, 7, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG470_800A64B8 = { NULL, NULL, 219, 8, 368, 296, 7 };
FieldstgPlacedActor *wstag470_actors[20] = {
    &D_WSTAG470_800A6350, &D_WSTAG470_800A6364, &D_WSTAG470_800A6378, &D_WSTAG470_800A638C, &D_WSTAG470_800A63A0,
    &D_WSTAG470_800A63B4, &D_WSTAG470_800A63C8, &D_WSTAG470_800A63DC, &D_WSTAG470_800A63F0, &D_WSTAG470_800A6404,
    &D_WSTAG470_800A6418, &D_WSTAG470_800A642C, &D_WSTAG470_800A6440, &D_WSTAG470_800A6454, &D_WSTAG470_800A6468,
    &D_WSTAG470_800A647C, &D_WSTAG470_800A6490, &D_WSTAG470_800A64A4, &D_WSTAG470_800A64B8, NULL,
};
FieldstgSprite wstag470_sprites[22] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 249, 225, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 311, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 472, 215, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 193, 297, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 257, 265, 0, 0 }, { 1, 0, 0xFF, 2, 0xD, 0, 0, 0, 0, 0, 133, 187, 0, 0 },
    { 1, 0, 0xFF, 2, 0xD, 0, 0, 0, 0, 0, 277, 116, 0, 0 }, { 1, 0, 0xFF, 2, 0xE, 0, 0, 0, 0, 0, 421, 44, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 265, 384, 412, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 367, 333, 361, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 288, 299, 318, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 288, 305, 327, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 306, 299, 320, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 321, 290, 311, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 338, 282, 303, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 356, 275, 295, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 370, 266, 287, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 386, 258, 279, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 402, 250, 271, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 418, 242, 263, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 434, 235, 255, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag470_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x237, 0x1BE, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x243, 0xC0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x252, 0x116, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0xB1, 0x149, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0xC0, 0x17E, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag470_funcs = { wstag470_setup };
FieldstgEventDef wstag470_events[2] = {
    { 1215, D_WSTAG470_800A5F7C, 0x01350012, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
