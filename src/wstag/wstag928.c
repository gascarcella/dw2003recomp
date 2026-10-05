#include "wstag.h"

/* WSTAG928: stage 0x279 (fieldstg_stages_2d). */

extern WstagFuncs wstag928_funcs;
extern FieldstgVramPlace wstag928_vram_places[];
extern FieldstgPlacedActor *wstag928_actors[];
extern FieldstgSprite wstag928_sprites[];
extern FieldstgMapEvent wstag928_map_events[];
extern FieldstgEventDef wstag928_events[];

void wstag928_update(WstagObject *obj) {
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

WstagObject *wstag928_start(void *arg0) {
    WstagObject *obj = object_new(wstag928_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag928_funcs.setup();
    return obj;
}

void wstag928_event_1612_end(void) {
    gamestate_flags.set_flag(0x7C0C, 1);
}

void wstag928_setup(void) {
    fieldstg_stage.background_file = 0x1A2;
    fieldstg_stage.sprite_file = 0x08EF0000;
    fieldstg_stage.sprites = wstag928_sprites;
    fieldstg_stage.map_events = wstag928_map_events;
    fieldstg_stage.mask_file = 0x8EE;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xBC00, 0xB600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag928_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag928_actors;
    fieldstg_stage.events = wstag928_events;
    fieldstg_attr.set_file(0, 0x08EF0001);
    fieldstg_attr.set_file(7, 0x08EF0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag928_setup(void);
extern s16 D_WSTAG928_800A63EC[];

FieldstgVramPlace wstag928_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 394, 40, 138, 352, 511 }, { 320, 256, 338, 391, 72, 135, 368, 511 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 358, 395, 152, 139, 352, 510 },
};
u16 D_WSTAG928_800A6040[4] = { 0x7A28, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A6048[4] = { 0x9070, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A6050[4] = { 0x1805, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A6058[6] = { 0x1805, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG928_800A6064[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A606C[8] = { 0x1805, 0, 0, 1, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG928_800A607C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A6084[10] = {
    0x1805, 0, 0, 1, 1, 1, 2, 0,
    0xFFFF, 0,
};
u16 D_WSTAG928_800A6098[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A60A0[12] = {
    0x1805, 0, 0, 1, 1, 1, 2, 1,
    3, 0, 0xFFFF, 0,
};
u16 D_WSTAG928_800A60B8[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A60C0[14] = {
    0x1805, 0, 0, 1, 1, 1, 2, 1,
    4, 0, 3, 1, 0xFFFF, 0,
};
u16 D_WSTAG928_800A60DC[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG928_800A60E4[14] = {
    3, 1, 0x1805, 0, 0, 1, 1, 1,
    2, 1, 4, 1, 0xFFFF, 0,
};
u16 D_WSTAG928_800A6100[8] = { 0x7013, 1, 0x1805, 1, 0x7092, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG928_800A6110[2] = { { NULL, D_WSTAG928_800A6040, 43 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG928_800A6128[2] = { { NULL, D_WSTAG928_800A6048, 44 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG928_800A6140[8] = {
    { D_WSTAG928_800A6050, NULL, 49 }, { D_WSTAG928_800A6058, D_WSTAG928_800A6064, 46 },
    { D_WSTAG928_800A606C, D_WSTAG928_800A607C, 48 }, { D_WSTAG928_800A6084, D_WSTAG928_800A6098, 48 },
    { D_WSTAG928_800A60A0, D_WSTAG928_800A60B8, 48 }, { D_WSTAG928_800A60C0, D_WSTAG928_800A60DC, 48 },
    { D_WSTAG928_800A60E4, D_WSTAG928_800A6100, 47 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG928_800A61A0[2] = { { NULL, NULL, 45 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG928_800A61B8 = { NULL, D_WSTAG928_800A6110, 20, 4, 188, 154, 7 };
FieldstgPlacedActor D_WSTAG928_800A61CC = { NULL, D_WSTAG928_800A6128, 21, 5, 175, 192, 7 };
FieldstgPlacedActor D_WSTAG928_800A61E0 = { NULL, D_WSTAG928_800A6140, 63, 6, 145, 289, 7 };
FieldstgPlacedActor D_WSTAG928_800A61F4 = { NULL, D_WSTAG928_800A61A0, 375, 7, 88, 332, 7 };
FieldstgPlacedActor *wstag928_actors[5] = {
    &D_WSTAG928_800A61B8, &D_WSTAG928_800A61CC, &D_WSTAG928_800A61E0, &D_WSTAG928_800A61F4, NULL,
};
FieldstgSprite wstag928_sprites[14] = {
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 78, 77, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 97, 115, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 135, 231, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 141, 44, 0, 0 },
    { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 261, 63, 0, 0 }, { 1, 0, 0x40, 2, 6, 2, 0, 1, 4, 0, 326, 95, 0, 0 },
    { 1, 0, 0x40, 2, 7, 2, 0, 1, 4, 0, 165, 76, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 256, 151, 184, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 215, 20, 86, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 144, 185, 200, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 125, 175, 194, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 148, 127, 167, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 240, 127, 162, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag928_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27B, 0x96, 0x114, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x2A0, 0x17A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27E, 0x15A, 0xBA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x146, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xC8, 0xEC, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xB8, 0x132, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag928_funcs = { wstag928_setup };
FieldstgEventDef wstag928_events[2] = {
    { 1612, D_WSTAG928_800A63EC, 0x01580006, NULL, wstag928_event_1612_end }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG928_800A63EC[55] = {
    FIELDSTG_EVENT_WALK(2, 207, 208, 3),
    FIELDSTG_EVENT_PLACE(21, 175, 192),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(36),
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
