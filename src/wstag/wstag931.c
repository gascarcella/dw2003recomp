#include "wstag.h"

/* WSTAG931: stage 0x27C (fieldstg_stages_2d). */

extern WstagFuncs wstag931_funcs;
extern FieldstgVramPlace wstag931_vram_places[];
extern FieldstgPlacedActor *wstag931_actors[];
extern FieldstgSprite wstag931_sprites[];
extern FieldstgMapEvent wstag931_map_events[];
extern FieldstgEventDef wstag931_events[];

void wstag931_update(WstagObject *obj) {
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

WstagObject *wstag931_start(void *arg0) {
    WstagObject *obj = object_new(wstag931_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag931_funcs.setup();
    return obj;
}

void wstag931_setup(void) {
    fieldstg_stage.background_file = 0x1A6;
    fieldstg_stage.sprite_file = 0x08F50000;
    fieldstg_stage.sprites = wstag931_sprites;
    fieldstg_stage.map_events = wstag931_map_events;
    fieldstg_stage.mask_file = 0x8F4;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15A00, 0x18E00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag931_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag931_actors;
    fieldstg_stage.events = wstag931_events;
    fieldstg_attr.set_file(0, 0x08F50001);
    fieldstg_attr.set_file(7, 0x08F50002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag931_setup(void);
extern s16 D_WSTAG931_800A637C[];

FieldstgVramPlace wstag931_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 500, 320, 720, 64, 336, 511 }, { 320, 256, 354, 476, 136, 220, 352, 511 },
    { 448, 256, 462, 420, 568, 164, 368, 511 }, { 320, 256, 372, 476, 208, 220, 336, 510 },
    { 448, 256, 470, 425, 600, 169, 352, 510 },
};
u16 D_WSTAG931_800A602C[4] = { 0x7A11, 1, 0xFFFF, 0 };
u16 D_WSTAG931_800A6034[4] = { 0x7A0F, 1, 0xFFFF, 0 };
u16 D_WSTAG931_800A603C[4] = { 0x7A10, 1, 0xFFFF, 0 };
u16 D_WSTAG931_800A6044[4] = { 0x8025, 0, 0xFFFF, 0 };
u16 D_WSTAG931_800A604C[6] = { 0x8025, 1, 0x1807, 0, 0xFFFF, 0 };
u16 D_WSTAG931_800A6058[6] = { 0x9071, 1, 0x1807, 1, 0xFFFF, 0 };
u16 D_WSTAG931_800A6064[6] = { 0x8025, 1, 0x1807, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG931_800A6070[2] = { { NULL, D_WSTAG931_800A602C, 51 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG931_800A6088[2] = { { NULL, D_WSTAG931_800A6034, 50 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG931_800A60A0[2] = { { NULL, NULL, 134 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG931_800A60B8[2] = { { NULL, D_WSTAG931_800A603C, 52 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG931_800A60D0[4] = {
    { D_WSTAG931_800A6044, NULL, 131 }, { D_WSTAG931_800A604C, D_WSTAG931_800A6058, 132 },
    { D_WSTAG931_800A6064, NULL, 133 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG931_800A6100[2] = { { NULL, NULL, 133 }, { NULL, NULL, 0 } };
u16 D_WSTAG931_800A6118[4] = { 0x1807, 0, 0xFFFF, 0 };
u16 D_WSTAG931_800A6120[4] = { 0x1807, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG931_800A6128 = { NULL, D_WSTAG931_800A6070, 22, 4, 449, 281, 7 };
FieldstgPlacedActor D_WSTAG931_800A613C = { NULL, D_WSTAG931_800A6088, 23, 5, 304, 369, 7 };
FieldstgPlacedActor D_WSTAG931_800A6150 = { NULL, D_WSTAG931_800A60A0, 52, 6, 287, 432, 3 };
FieldstgPlacedActor D_WSTAG931_800A6164 = { NULL, D_WSTAG931_800A60B8, 183, 7, 359, 308, 7 };
FieldstgPlacedActor D_WSTAG931_800A6178 = { D_WSTAG931_800A6118, D_WSTAG931_800A60D0, 255, 8, 152, 397, 7 };
FieldstgPlacedActor D_WSTAG931_800A618C = { D_WSTAG931_800A6120, D_WSTAG931_800A6100, 255, 8, 144, 352, 1 };
FieldstgPlacedActor *wstag931_actors[7] = {
    &D_WSTAG931_800A6128, &D_WSTAG931_800A613C, &D_WSTAG931_800A6150, &D_WSTAG931_800A6164, &D_WSTAG931_800A6178,
    &D_WSTAG931_800A618C, NULL,
};
FieldstgSprite wstag931_sprites[13] = {
    { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 384, 79, 0, 0 }, { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 492, 137, 0, 0 },
    { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 548, 165, 0, 0 }, { 1, 0, 0x40, 6, 3, 1, 3, 8, 4, 0, 485, 145, 0, 0 },
    { 1, 0, 0x40, 6, 9, 1, 9, 0xE, 4, 0, 541, 172, 0, 0 }, { 1, 0x64, 0x40, 6, 0x12, 0, 0, 0, 0, 0, 304, 99, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x13, 0, 0, 0, 0, 0, 64, 285, 0, 0 }, { 1, 0, 0xA0, 4, 0, 0, 0, 0, 0, 0, 144, 257, 380, 0 },
    { 1, 0, 0xA0, 4, 1, 0, 0, 0, 0, 0, 256, 257, 337, 0 }, { 1, 0, 0xA0, 4, 2, 0, 0, 0, 0, 0, 384, 261, 287, 0 },
    { 1, 0, 0x60, 4, 0xF, 0, 0, 0, 0, 0, 240, 73, 150, 0 }, { 1, 0, 0x60, 4, 0x10, 0, 0, 0, 0, 0, 0, 255, 335, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag931_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x1E0, 0x1DA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x240, 0x1AA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27D, 0x218, 0xE4, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27D, 0x128, 0x19C, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x11F, 0xBA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x110, 0xFE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag931_funcs = { wstag931_setup };
FieldstgEventDef wstag931_events[2] = {
    { 1614, D_WSTAG931_800A637C, 0x01580007, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG931_800A637C[69] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 176, 408, 3),
    FIELDSTG_EVENT_PLACE(255, 152, 397),
    FIELDSTG_EVENT_ANIM(255, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 255),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 255),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 255, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(255, 104, 372, 3),
    FIELDSTG_EVENT_WAIT_WALK(255),
    FIELDSTG_EVENT_WALK(255, 144, 352, 5),
    FIELDSTG_EVENT_WAIT_WALK(255),
    FIELDSTG_EVENT_ANIM(255, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
