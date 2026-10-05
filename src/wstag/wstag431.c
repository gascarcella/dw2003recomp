#include "wstag.h"

/* WSTAG431: stage 0x29E (fieldstg_stages). */

extern WstagFuncs wstag431_funcs;
extern FieldstgVramPlace wstag431_vram_places[];
extern FieldstgPlacedActor *wstag431_actors[];
extern FieldstgSprite wstag431_sprites[];
extern FieldstgMapEvent wstag431_map_events[];
extern FieldstgEventDef wstag431_events[];

void wstag431_update(WstagObject *obj) {
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

WstagObject *wstag431_start(void *arg0) {
    WstagObject *obj = object_new(wstag431_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag431_funcs.setup();
    return obj;
}

void wstag431_setup(void) {
    fieldstg_stage.background_file = 0x59D;
    fieldstg_stage.sprite_file = 0x059E0000;
    fieldstg_stage.sprites = wstag431_sprites;
    fieldstg_stage.map_events = wstag431_map_events;
    fieldstg_stage.mask_file = 0x59C;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19100, 0x1E700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag431_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag431_actors;
    fieldstg_stage.events = wstag431_events;
    fieldstg_attr.set_file(0, 0x059E0001);
    fieldstg_attr.set_file(7, 0x059E0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag431_setup(void);

s16 D_WSTAG431_800A5F7C[57] = {
    FIELDSTG_EVENT_WALK(2, 376, 428, 5),
    FIELDSTG_EVENT_PLACE(21, 408, 414),
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
    FIELDSTG_EVENT_GOTO_MAP(0xC0C, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag431_vram_places[25] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 336, 256, 64, 0, 352, 509 }, { 320, 256, 348, 256, 112, 0, 368, 509 },
    { 320, 256, 328, 294, 32, 38, 336, 508 }, { 320, 256, 348, 296, 112, 40, 352, 508 },
    { 320, 256, 374, 256, 216, 0, 368, 508 }, { 320, 256, 320, 294, 0, 38, 320, 507 },
    { 320, 256, 320, 342, 0, 86, 336, 507 }, { 320, 256, 336, 304, 64, 48, 352, 507 },
    { 320, 256, 370, 304, 200, 48, 368, 507 }, { 320, 256, 356, 322, 144, 66, 320, 506 },
    { 320, 256, 364, 322, 176, 66, 336, 506 }, { 320, 256, 328, 334, 32, 78, 352, 506 },
    { 320, 256, 344, 336, 96, 80, 368, 506 }, { 320, 256, 336, 344, 64, 88, 320, 505 },
    { 320, 256, 370, 344, 200, 88, 336, 505 }, { 320, 256, 352, 362, 128, 106, 352, 505 },
    { 320, 256, 360, 370, 160, 114, 368, 505 }, { 320, 256, 320, 374, 0, 118, 320, 504 },
    { 320, 256, 328, 374, 32, 118, 336, 504 },
};
u16 D_WSTAG431_800A6180[4] = { 0x900D, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6188[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6190[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6198[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A61A0[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A61A8[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A61B0[4] = { 0x1A01, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG431_800A61B8[2] = { { NULL, D_WSTAG431_800A6180, 418 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A61D0[2] = { { NULL, D_WSTAG431_800A6188, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A61E8[2] = { { NULL, D_WSTAG431_800A6190, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6200[2] = { { NULL, D_WSTAG431_800A6198, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6218[2] = { { NULL, D_WSTAG431_800A61A0, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6230[2] = { { NULL, D_WSTAG431_800A61A8, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6248[2] = { { NULL, NULL, 247 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6260[2] = { { NULL, NULL, 253 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6278[2] = { { NULL, NULL, 243 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6290[2] = { { NULL, NULL, 245 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A62A8[2] = { { NULL, NULL, 240 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A62C0[2] = { { NULL, NULL, 242 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A62D8[2] = { { NULL, NULL, 256 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A62F0[2] = { { NULL, NULL, 251 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6308[2] = { { NULL, NULL, 255 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6320[2] = { { NULL, D_WSTAG431_800A61B0, 250 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6338[2] = { { NULL, NULL, 466 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6350[2] = { { NULL, NULL, 467 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6368[2] = { { NULL, NULL, 239 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6380[2] = { { NULL, NULL, 244 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6398[2] = { { NULL, NULL, 246 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A63B0[2] = { { NULL, NULL, 249 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A63C8[2] = { { NULL, NULL, 241 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A63E0[2] = { { NULL, NULL, 252 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A63F8[2] = { { NULL, NULL, 248 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG431_800A6410[2] = { { NULL, NULL, 254 }, { NULL, NULL, 0 } };
u16 D_WSTAG431_800A6428[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6430[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG431_800A643C[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6448[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6450[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6458[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6464[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A646C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6474[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A647C[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6488[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6490[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6498[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64A0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64A8[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG431_800A64B4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64BC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64C4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG431_800A64D0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64D8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG431_800A64E4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64EC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64F4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A64FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG431_800A6504[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG431_800A650C = { NULL, D_WSTAG431_800A61B8, 21, 4, 408, 414, 1 };
FieldstgPlacedActor D_WSTAG431_800A6520 = { D_WSTAG431_800A6428, D_WSTAG431_800A61D0, 24, 5, 320, 515, 7 };
FieldstgPlacedActor D_WSTAG431_800A6534 = { D_WSTAG431_800A6430, D_WSTAG431_800A61E8, 24, 5, 352, 488, 7 };
FieldstgPlacedActor D_WSTAG431_800A6548 = { D_WSTAG431_800A643C, D_WSTAG431_800A6200, 24, 5, 320, 515, 7 };
FieldstgPlacedActor D_WSTAG431_800A655C = { D_WSTAG431_800A6448, D_WSTAG431_800A6218, 24, 5, 320, 515, 7 };
FieldstgPlacedActor D_WSTAG431_800A6570 = { D_WSTAG431_800A6450, D_WSTAG431_800A6230, 24, 5, 320, 515, 7 };
FieldstgPlacedActor D_WSTAG431_800A6584 = { D_WSTAG431_800A6458, D_WSTAG431_800A6248, 32, 6, 481, 225, 5 };
FieldstgPlacedActor D_WSTAG431_800A6598 = { D_WSTAG431_800A6464, D_WSTAG431_800A6260, 36, 7, 500, 294, 1 };
FieldstgPlacedActor D_WSTAG431_800A65AC = { D_WSTAG431_800A646C, D_WSTAG431_800A6278, 37, 8, 171, 403, 5 };
FieldstgPlacedActor D_WSTAG431_800A65C0 = { D_WSTAG431_800A6474, D_WSTAG431_800A6290, 38, 9, 257, 272, 7 };
FieldstgPlacedActor D_WSTAG431_800A65D4 = { D_WSTAG431_800A647C, D_WSTAG431_800A62A8, 46, 10, 257, 513, 7 };
FieldstgPlacedActor D_WSTAG431_800A65E8 = { D_WSTAG431_800A6488, D_WSTAG431_800A62C0, 46, 10, 171, 403, 5 };
FieldstgPlacedActor D_WSTAG431_800A65FC = { D_WSTAG431_800A6490, D_WSTAG431_800A62D8, 49, 11, 500, 294, 1 };
FieldstgPlacedActor D_WSTAG431_800A6610 = { D_WSTAG431_800A6498, D_WSTAG431_800A62F0, 51, 12, 401, 233, 7 };
FieldstgPlacedActor D_WSTAG431_800A6624 = { D_WSTAG431_800A64A0, D_WSTAG431_800A6308, 55, 13, 481, 225, 5 };
FieldstgPlacedActor D_WSTAG431_800A6638 = { D_WSTAG431_800A64A8, D_WSTAG431_800A6320, 102, 14, 320, 515, 5 };
FieldstgPlacedActor D_WSTAG431_800A664C = { D_WSTAG431_800A64B4, D_WSTAG431_800A6338, 115, 15, 171, 403, 5 };
FieldstgPlacedActor D_WSTAG431_800A6660 = { D_WSTAG431_800A64BC, D_WSTAG431_800A6350, 116, 16, 500, 294, 1 };
FieldstgPlacedActor D_WSTAG431_800A6674 = { D_WSTAG431_800A64C4, D_WSTAG431_800A6368, 157, 17, 513, 241, 1 };
FieldstgPlacedActor D_WSTAG431_800A6688 = { D_WSTAG431_800A64D0, D_WSTAG431_800A6380, 157, 17, 171, 403, 5 };
FieldstgPlacedActor D_WSTAG431_800A669C = { D_WSTAG431_800A64D8, D_WSTAG431_800A6398, 158, 18, 481, 225, 5 };
FieldstgPlacedActor D_WSTAG431_800A66B0 = { D_WSTAG431_800A64E4, D_WSTAG431_800A63B0, 158, 18, 257, 272, 7 };
FieldstgPlacedActor D_WSTAG431_800A66C4 = { D_WSTAG431_800A64EC, D_WSTAG431_800A63C8, 159, 19, 257, 513, 7 };
FieldstgPlacedActor D_WSTAG431_800A66D8 = { D_WSTAG431_800A64F4, D_WSTAG431_800A63E0, 160, 20, 481, 225, 5 };
FieldstgPlacedActor D_WSTAG431_800A66EC = { D_WSTAG431_800A64FC, D_WSTAG431_800A63F8, 161, 21, 401, 233, 7 };
FieldstgPlacedActor D_WSTAG431_800A6700 = { D_WSTAG431_800A6504, D_WSTAG431_800A6410, 162, 22, 500, 294, 1 };
FieldstgPlacedActor *wstag431_actors[27] = {
    &D_WSTAG431_800A650C, &D_WSTAG431_800A6520, &D_WSTAG431_800A6534, &D_WSTAG431_800A6548, &D_WSTAG431_800A655C,
    &D_WSTAG431_800A6570, &D_WSTAG431_800A6584, &D_WSTAG431_800A6598, &D_WSTAG431_800A65AC, &D_WSTAG431_800A65C0,
    &D_WSTAG431_800A65D4, &D_WSTAG431_800A65E8, &D_WSTAG431_800A65FC, &D_WSTAG431_800A6610, &D_WSTAG431_800A6624,
    &D_WSTAG431_800A6638, &D_WSTAG431_800A664C, &D_WSTAG431_800A6660, &D_WSTAG431_800A6674, &D_WSTAG431_800A6688,
    &D_WSTAG431_800A669C, &D_WSTAG431_800A66B0, &D_WSTAG431_800A66C4, &D_WSTAG431_800A66D8, &D_WSTAG431_800A66EC,
    &D_WSTAG431_800A6700, NULL,
};
FieldstgSprite wstag431_sprites[13] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 0xA, 0, 283, 466, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 0xA, 0, 428, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 0xA, 0, 307, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 1, 0xA, 0, 287, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 1, 0xA, 0, 297, 464, 0, 0 },
    { 1, 0, 0x40, 4, 0x35, 2, 0, 1, 0xA, 0, 333, 483, 503, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 501, 393, 406, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 451, 436, 456, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 436, 464, 482, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 320, 474, 503, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 438, 274, 292, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 464, 277, 302, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag431_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x2F8, 0x104, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x1E8, 0xA4, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag431_funcs = { wstag431_setup };
FieldstgEventDef wstag431_events[2] = {
    { 1211, D_WSTAG431_800A5F7C, 0x01350011, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
