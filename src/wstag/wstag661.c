#include "wstag.h"

/* WSTAG661: stage 0x2C6 (fieldstg_stages). */

extern WstagFuncs wstag661_funcs;
const CVECTOR wstag661_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgVramPlace wstag661_vram_places[];
extern FieldstgPlacedActor *wstag661_actors[];
extern FieldstgSprite wstag661_sprites[];
extern FieldstgMapEvent wstag661_map_events[];
extern FieldstgEventDef wstag661_events[];

void wstag661_update(WstagObject *obj) {
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

WstagObject *wstag661_start(void *arg0) {
    WstagObject *obj = object_new(wstag661_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag661_funcs.setup();
    return obj;
}

void wstag661_setup(void) {
    fieldstg_stage.background_file = 0x665;
    fieldstg_stage.sprite_file = 0x06660000;
    fieldstg_stage.sprites = wstag661_sprites;
    fieldstg_stage.map_events = wstag661_map_events;
    fieldstg_stage.mask_file = 0x664;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1FA00, 0x2DE00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag661_vram_places;
    fieldstg_stage.music = 0x17;
    fieldstg_stage.sound = 0x605C0000;
    fieldstg_stage.actors = wstag661_actors;
    fieldstg_stage.color = wstag661_color;
    fieldstg_stage.events = wstag661_events;
    fieldstg_attr.set_file(0, 0x06660001);
    fieldstg_attr.set_file(7, 0x06660002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag661_setup(void);

s16 D_WSTAG661_800A5FE4[58] = {
    FIELDSTG_EVENT_WALK(2, 1086, 516, 5),
    FIELDSTG_EVENT_PLACE(21, 1119, 502),
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
    FIELDSTG_EVENT_GOTO_MAP(0xC0F, 0, 0, 0),
    FIELDSTG_EVENT_END,
    0x2, /* padding, not read */
};
FieldstgVramPlace wstag661_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 502, 352, 728, 96, 336, 507 }, { 384, 256, 404, 424, 336, 168, 352, 507 },
    { 448, 256, 500, 256, 720, 0, 368, 507 }, { 384, 256, 424, 472, 416, 216, 336, 506 },
    { 448, 256, 460, 256, 560, 0, 352, 506 }, { 448, 256, 448, 376, 512, 120, 368, 506 },
    { 448, 256, 456, 376, 544, 120, 320, 505 }, { 448, 256, 464, 376, 576, 120, 336, 505 },
    { 448, 256, 472, 384, 608, 128, 352, 505 }, { 448, 256, 492, 384, 688, 128, 368, 505 },
};
u16 D_WSTAG661_800A6158[4] = { 0x7A2E, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6160[4] = { 0x9013, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6168[4] = { 0x7A1B, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6170[4] = { 0x7A1A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6178[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6180[4] = { 0x701F, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6188[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG661_800A6190[2] = { { NULL, D_WSTAG661_800A6158, 419 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A61A8[2] = { { NULL, D_WSTAG661_800A6160, 418 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A61C0[2] = { { NULL, D_WSTAG661_800A6168, 424 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A61D8[2] = { { NULL, D_WSTAG661_800A6170, 420 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A61F0[2] = { { NULL, D_WSTAG661_800A6178, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6208[2] = { { NULL, NULL, 374 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6220[2] = { { NULL, NULL, 376 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6238[2] = { { NULL, NULL, 372 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6250[2] = { { NULL, NULL, 373 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6268[2] = { { NULL, NULL, 373 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6280[2] = { { NULL, NULL, 375 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A6298[2] = { { NULL, NULL, 375 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A62B0[3] = {
    { D_WSTAG661_800A6180, NULL, 471 }, { D_WSTAG661_800A6188, NULL, 472 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG661_800A62D4[2] = { { NULL, NULL, 473 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG661_800A62EC[2] = { { NULL, NULL, 491 }, { NULL, NULL, 0 } };
u16 D_WSTAG661_800A6304[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6310[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6318[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6324[6] = { 0x7021, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG661_800A6330[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6338[6] = { 0x7021, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG661_800A6344[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A634C[4] = { 0x7021, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A6354[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG661_800A635C[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG661_800A6364 = { NULL, D_WSTAG661_800A6190, 20, 4, 908, 596, 7 };
FieldstgPlacedActor D_WSTAG661_800A6378 = { NULL, D_WSTAG661_800A61A8, 21, 5, 1119, 502, 1 };
FieldstgPlacedActor D_WSTAG661_800A638C = { NULL, D_WSTAG661_800A61C0, 22, 6, 834, 470, 7 };
FieldstgPlacedActor D_WSTAG661_800A63A0 = { NULL, D_WSTAG661_800A61D8, 23, 7, 690, 493, 1 };
FieldstgPlacedActor D_WSTAG661_800A63B4 = { NULL, D_WSTAG661_800A61F0, 24, 8, 1329, 411, 1 };
FieldstgPlacedActor D_WSTAG661_800A63C8 = { D_WSTAG661_800A6304, D_WSTAG661_800A6208, 46, 9, 961, 368, 1 };
FieldstgPlacedActor D_WSTAG661_800A63DC = { D_WSTAG661_800A6310, D_WSTAG661_800A6220, 46, 9, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG661_800A63F0 = { D_WSTAG661_800A6318, D_WSTAG661_800A6238, 57, 10, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG661_800A6404 = { D_WSTAG661_800A6324, D_WSTAG661_800A6250, 157, 11, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG661_800A6418 = { D_WSTAG661_800A6330, D_WSTAG661_800A6268, 157, 11, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG661_800A642C = { D_WSTAG661_800A6338, D_WSTAG661_800A6280, 158, 12, 961, 368, 1 };
FieldstgPlacedActor D_WSTAG661_800A6440 = { D_WSTAG661_800A6344, D_WSTAG661_800A6298, 158, 12, 961, 368, 1 };
FieldstgPlacedActor D_WSTAG661_800A6454 = { D_WSTAG661_800A634C, D_WSTAG661_800A62B0, 373, 13, 897, 289, 1 };
FieldstgPlacedActor D_WSTAG661_800A6468 = { D_WSTAG661_800A6354, D_WSTAG661_800A62D4, 373, 13, 897, 289, 1 };
FieldstgPlacedActor D_WSTAG661_800A647C = { D_WSTAG661_800A635C, D_WSTAG661_800A62EC, 373, 13, 961, 368, 1 };
FieldstgPlacedActor *wstag661_actors[16] = {
    &D_WSTAG661_800A6364, &D_WSTAG661_800A6378, &D_WSTAG661_800A638C, &D_WSTAG661_800A63A0, &D_WSTAG661_800A63B4,
    &D_WSTAG661_800A63C8, &D_WSTAG661_800A63DC, &D_WSTAG661_800A63F0, &D_WSTAG661_800A6404, &D_WSTAG661_800A6418,
    &D_WSTAG661_800A642C, &D_WSTAG661_800A6440, &D_WSTAG661_800A6454, &D_WSTAG661_800A6468, &D_WSTAG661_800A647C,
    NULL,
};
FieldstgSprite wstag661_sprites[32] = {
    { 1, 0, 0x40, 2, 7, 0, 7, 0xC, 4, 0, 374, 313, 0, 0 }, { 1, 0, 0x40, 2, 7, 0, 7, 0xC, 4, 0, 919, 586, 0, 0 },
    { 1, 0, 0xC8, 2, 0x5A, 1, 0x5A, 0x61, 0xA, 0, 1314, 270, 0, 0 },
    { 1, 0, 0x48, 2, 0, 0, 0, 0, 0, 0, 444, 419, 0, 0 }, { 1, 0, 0x58, 2, 0xD, 0, 0, 0, 0, 0, 683, 345, 0, 0 },
    { 1, 0, 0x4C, 2, 0xE, 0, 0, 0, 0, 0, 947, 492, 0, 0 }, { 1, 0, 0x60, 2, 0x10, 0, 0, 0, 0, 0, 640, 515, 0, 0 },
    { 1, 0, 0x74, 2, 0x11, 0, 0, 0, 0, 0, 624, 524, 0, 0 }, { 1, 0, 0x44, 2, 0x12, 0, 0, 0, 0, 0, 304, 556, 0, 0 },
    { 1, 0, 0x62, 2, 0xF, 0, 0, 0, 0, 0, 932, 598, 0, 0 }, { 1, 0, 0x41, 2, 3, 0, 0, 0, 0, 0, 960, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 8, 0, 1378, 348, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 1391, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 8, 0, 1250, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 8, 0, 1263, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x38, 4, 0, 1217, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x38, 4, 0, 1417, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 6, 0, 1310, 259, 0, 0 },
    { 1, 0, 0xC8, 6, 0x57, 1, 0x57, 0x59, 6, 0, 1144, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x49, 0xA, 0, 290, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x49, 0xA, 0, 344, 408, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x49, 0xA, 0, 851, 700, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x49, 0xA, 0, 996, 725, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0x51, 1, 0x51, 0x53, 6, 0, 799, 318, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0x54, 1, 0x54, 0x56, 6, 0, 1052, 371, 0, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 944, 515, 569, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 582, 424, 454, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1056, 269, 327, 0 }, { 1, 0, 0x41, 4, 4, 0, 0, 0, 0, 0, 369, 703, 751, 0 },
    { 1, 0, 0x41, 4, 4, 0, 0, 0, 0, 0, 449, 743, 791, 0 }, { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 914, 233, 303, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag661_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C7, 0x100, 0x220, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B2, 0x1F0, 0x68, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x170, 0x268, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x180, 0x2B0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag661_funcs = { wstag661_setup };
FieldstgEventDef wstag661_events[2] = {
    { 1230, D_WSTAG661_800A5FE4, 0x01430006, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
