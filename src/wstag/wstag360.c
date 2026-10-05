#include "wstag.h"

/* WSTAG360: stage 0x223 (fieldstg_stages). */

extern WstagFuncs wstag360_funcs;
extern FieldstgVramPlace wstag360_vram_places[];
extern FieldstgPlacedActor *wstag360_actors[];
extern FieldstgSprite wstag360_sprites[];
extern FieldstgMapEvent wstag360_map_events[];
extern FieldstgEventDef wstag360_events[];

void wstag360_update(WstagObject *obj) {
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

WstagObject *wstag360_start(void *arg0) {
    WstagObject *obj = object_new(wstag360_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag360_funcs.setup();
    return obj;
}

void wstag360_event_1457_end(void) {
    gamestate_flags.set_flag(0x7C14, 1);
}

void wstag360_setup(void) {
    fieldstg_stage.background_file = 0x21E;
    fieldstg_stage.sprite_file = 0x021F0000;
    fieldstg_stage.sprites = wstag360_sprites;
    fieldstg_stage.map_events = wstag360_map_events;
    fieldstg_stage.mask_file = 0x3D6;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19300, 0x14800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag360_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag360_actors;
    fieldstg_stage.events = wstag360_events;
    fieldstg_attr.set_file(0, 0x021F0001);
    fieldstg_attr.set_file(7, 0x021F0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag360_setup(void);

s16 D_WSTAG360_800A5FA8[57] = {
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
    FIELDSTG_EVENT_GOTO_MAP(0xC12, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag360_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 358, 0, 102, 336, 507 }, { 320, 256, 368, 256, 192, 0, 352, 507 },
    { 320, 256, 360, 361, 160, 105, 368, 507 }, { 320, 256, 346, 364, 104, 108, 320, 506 },
    { 320, 256, 328, 365, 32, 109, 336, 506 }, { 320, 256, 336, 365, 64, 109, 352, 506 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG360_800A60EC[4] = { 0x1A08, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A60F4[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A60FC[4] = { 0x1A08, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A6104[4] = { 0x7A1F, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A610C[4] = { 0x1A09, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A6114[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A611C[4] = { 0x1A09, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A6124[4] = { 0x9011, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A612C[4] = { 0x1C1C, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A6134[4] = { 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A613C[4] = { 0x1C45, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A6144[4] = { 0x1C45, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A614C[4] = { 0x1C45, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG360_800A6154[3] = {
    { D_WSTAG360_800A60EC, D_WSTAG360_800A60F4, 362 }, { D_WSTAG360_800A60FC, D_WSTAG360_800A6104, 363 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG360_800A6178[3] = {
    { D_WSTAG360_800A610C, D_WSTAG360_800A6114, 361 }, { D_WSTAG360_800A611C, D_WSTAG360_800A6124, 360 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG360_800A619C[2] = { { NULL, NULL, 28 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A61B4[3] = {
    { D_WSTAG360_800A612C, NULL, 575 }, { D_WSTAG360_800A6134, NULL, 47 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG360_800A61D8[2] = { { NULL, NULL, 578 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A61F0[2] = { { NULL, NULL, 577 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6208[2] = { { NULL, NULL, 576 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6220[2] = { { NULL, NULL, 575 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6238[2] = { { NULL, NULL, 574 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6250[2] = { { NULL, NULL, 573 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6268[2] = { { NULL, NULL, 572 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6280[2] = { { NULL, NULL, 571 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG360_800A6298[3] = {
    { D_WSTAG360_800A613C, D_WSTAG360_800A6144, 742 }, { D_WSTAG360_800A614C, NULL, 807 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG360_800A62BC[2] = { { NULL, NULL, 579 }, { NULL, NULL, 0 } };
u16 D_WSTAG360_800A62D4[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A62DC[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A62E4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A62EC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A62F4[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A62FC[6] = { 0x7017, 1, 0x6014, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A6308[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A6310[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A6318[6] = { 0x7015, 1, 0x6006, 0, 0xFFFF, 0 };
u16 D_WSTAG360_800A6324[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A632C[8] = { 0x818F, 0, 0x1C44, 1, 0x6006, 1, 0xFFFF, 0 };
u16 D_WSTAG360_800A633C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG360_800A6344 = { NULL, D_WSTAG360_800A6154, 20, 4, 345, 308, 7 };
FieldstgPlacedActor D_WSTAG360_800A6358 = { NULL, D_WSTAG360_800A6178, 21, 5, 121, 165, 7 };
FieldstgPlacedActor D_WSTAG360_800A636C = { D_WSTAG360_800A62D4, D_WSTAG360_800A619C, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6380 = { D_WSTAG360_800A62DC, D_WSTAG360_800A61B4, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6394 = { D_WSTAG360_800A62E4, D_WSTAG360_800A61D8, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A63A8 = { D_WSTAG360_800A62EC, D_WSTAG360_800A61F0, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A63BC = { D_WSTAG360_800A62F4, D_WSTAG360_800A6208, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A63D0 = { D_WSTAG360_800A62FC, D_WSTAG360_800A6220, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A63E4 = { D_WSTAG360_800A6308, D_WSTAG360_800A6238, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A63F8 = { D_WSTAG360_800A6310, D_WSTAG360_800A6250, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A640C = { D_WSTAG360_800A6318, D_WSTAG360_800A6268, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6420 = { D_WSTAG360_800A6324, D_WSTAG360_800A6280, 46, 6, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6434 = { D_WSTAG360_800A632C, D_WSTAG360_800A6298, 64, 7, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6448 = { NULL, NULL, 120, 8, 374, 268, 5 };
FieldstgPlacedActor D_WSTAG360_800A645C = { D_WSTAG360_800A633C, D_WSTAG360_800A62BC, 157, 9, 177, 153, 3 };
FieldstgPlacedActor D_WSTAG360_800A6470 = { NULL, NULL, 219, 10, 346, 325, 7 };
FieldstgPlacedActor *wstag360_actors[17] = {
    &D_WSTAG360_800A6344, &D_WSTAG360_800A6358, &D_WSTAG360_800A636C, &D_WSTAG360_800A6380, &D_WSTAG360_800A6394,
    &D_WSTAG360_800A63A8, &D_WSTAG360_800A63BC, &D_WSTAG360_800A63D0, &D_WSTAG360_800A63E4, &D_WSTAG360_800A63F8,
    &D_WSTAG360_800A640C, &D_WSTAG360_800A6420, &D_WSTAG360_800A6434, &D_WSTAG360_800A6448, &D_WSTAG360_800A645C,
    &D_WSTAG360_800A6470, NULL,
};
FieldstgSprite wstag360_sprites[30] = {
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
FieldstgMapEvent wstag360_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x222, 0x80, 0x31C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x224, 0x8C, 0xC2, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xFA, 0x126, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x10B, 0x188, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag360_funcs = { wstag360_setup };
FieldstgEventDef wstag360_events[2] = {
    { 1457, D_WSTAG360_800A5FA8, 0x01270033, NULL, wstag360_event_1457_end }, { -1, NULL, 0, NULL, NULL },
};
