#include "wstag.h"

/* WSTAG471: stage 0x2A5 (fieldstg_stages). */

extern WstagFuncs wstag471_funcs;
extern FieldstgVramPlace wstag471_vram_places[];
extern FieldstgPlacedActor *wstag471_actors[];
extern FieldstgSprite wstag471_sprites[];
extern FieldstgMapEvent wstag471_map_events[];
extern FieldstgEventDef wstag471_events[];

void wstag471_update(WstagObject *obj) {
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

WstagObject *wstag471_start(void *arg0) {
    WstagObject *obj = object_new(wstag471_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag471_funcs.setup();
    return obj;
}

void wstag471_event_1461_end(void) {
    gamestate_flags.set_flag(0x7C18, 1);
}

void wstag471_setup(void) {
    fieldstg_stage.background_file = 0x57C;
    fieldstg_stage.sprite_file = 0x057D0000;
    fieldstg_stage.sprites = wstag471_sprites;
    fieldstg_stage.map_events = wstag471_map_events;
    fieldstg_stage.mask_file = 0x57B;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1D400, 0x15400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag471_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag471_actors;
    fieldstg_stage.events = wstag471_events;
    fieldstg_attr.set_file(0, 0x057D0001);
    fieldstg_attr.set_file(7, 0x057D0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag471_setup(void);

s16 D_WSTAG471_800A5FA8[55] = {
    FIELDSTG_EVENT_WALK(2, 160, 328, 3),
    FIELDSTG_EVENT_PLACE(21, 128, 312),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC16, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag471_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 344, 192, 88, 352, 511 }, { 320, 256, 370, 256, 200, 0, 368, 511 },
    { 320, 256, 368, 376, 192, 120, 352, 510 }, { 320, 256, 370, 304, 200, 48, 368, 510 },
    { 320, 256, 336, 406, 64, 150, 352, 509 }, { 320, 256, 344, 406, 96, 150, 368, 509 },
    { 320, 256, 368, 408, 192, 152, 352, 508 }, { 320, 256, 352, 412, 128, 156, 368, 508 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG471_800A6108[4] = { 0x7A2B, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6110[4] = { 0x9067, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG471_800A6118[2] = { { NULL, D_WSTAG471_800A6108, 718 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6130[2] = { { NULL, D_WSTAG471_800A6110, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6148[2] = { { NULL, NULL, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6160[2] = { { NULL, NULL, 167 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6178[2] = { { NULL, NULL, 170 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6190[2] = { { NULL, NULL, 172 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A61A8[2] = { { NULL, NULL, 173 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A61C0[2] = { { NULL, NULL, 174 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A61D8[2] = { { NULL, NULL, 171 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A61F0[2] = { { NULL, NULL, 166 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6208[2] = { { NULL, NULL, 176 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6220[2] = { { NULL, NULL, 168 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG471_800A6238[2] = { { NULL, NULL, 175 }, { NULL, NULL, 0 } };
u16 D_WSTAG471_800A6250[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A625C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6268[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6270[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6278[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6280[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6288[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A6290[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG471_800A629C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG471_800A62A4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG471_800A62B0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG471_800A62B8 = { NULL, D_WSTAG471_800A6118, 20, 4, 357, 278, 7 };
FieldstgPlacedActor D_WSTAG471_800A62CC = { NULL, D_WSTAG471_800A6130, 21, 5, 128, 312, 7 };
FieldstgPlacedActor D_WSTAG471_800A62E0 = { D_WSTAG471_800A6250, D_WSTAG471_800A6148, 46, 6, 496, 321, 1 };
FieldstgPlacedActor D_WSTAG471_800A62F4 = { D_WSTAG471_800A625C, D_WSTAG471_800A6160, 53, 7, 557, 151, 1 };
FieldstgPlacedActor D_WSTAG471_800A6308 = { NULL, NULL, 120, 8, 368, 232, 5 };
FieldstgPlacedActor D_WSTAG471_800A631C = { D_WSTAG471_800A6268, D_WSTAG471_800A6178, 121, 9, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG471_800A6330 = { D_WSTAG471_800A6270, D_WSTAG471_800A6190, 121, 9, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG471_800A6344 = { D_WSTAG471_800A6278, D_WSTAG471_800A61A8, 121, 9, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG471_800A6358 = { D_WSTAG471_800A6280, D_WSTAG471_800A61C0, 121, 9, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG471_800A636C = { D_WSTAG471_800A6288, D_WSTAG471_800A61D8, 121, 9, 289, 338, 7 };
FieldstgPlacedActor D_WSTAG471_800A6380 = { D_WSTAG471_800A6290, D_WSTAG471_800A61F0, 157, 10, 557, 151, 1 };
FieldstgPlacedActor D_WSTAG471_800A6394 = { D_WSTAG471_800A629C, D_WSTAG471_800A6208, 157, 10, 557, 151, 1 };
FieldstgPlacedActor D_WSTAG471_800A63A8 = { D_WSTAG471_800A62A4, D_WSTAG471_800A6220, 158, 11, 496, 321, 1 };
FieldstgPlacedActor D_WSTAG471_800A63BC = { D_WSTAG471_800A62B0, D_WSTAG471_800A6238, 158, 11, 496, 321, 1 };
FieldstgPlacedActor D_WSTAG471_800A63D0 = { NULL, NULL, 219, 12, 368, 296, 7 };
FieldstgPlacedActor *wstag471_actors[16] = {
    &D_WSTAG471_800A62B8, &D_WSTAG471_800A62CC, &D_WSTAG471_800A62E0, &D_WSTAG471_800A62F4, &D_WSTAG471_800A6308,
    &D_WSTAG471_800A631C, &D_WSTAG471_800A6330, &D_WSTAG471_800A6344, &D_WSTAG471_800A6358, &D_WSTAG471_800A636C,
    &D_WSTAG471_800A6380, &D_WSTAG471_800A6394, &D_WSTAG471_800A63A8, &D_WSTAG471_800A63BC, &D_WSTAG471_800A63D0,
    NULL,
};
FieldstgSprite wstag471_sprites[19] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 249, 225, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 311, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 472, 215, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 193, 297, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 257, 265, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 265, 384, 412, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 367, 333, 361, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 288, 299, 318, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 288, 305, 327, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 306, 299, 320, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 321, 290, 311, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 338, 282, 303, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 356, 275, 295, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 370, 266, 287, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 386, 258, 279, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 402, 250, 271, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 418, 242, 263, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 434, 235, 255, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag471_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A4, 0x1BE, 0xBA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x243, 0xC0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x252, 0x116, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0xB1, 0x149, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0xC0, 0x17E, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag471_funcs = { wstag471_setup };
FieldstgEventDef wstag471_events[2] = {
    { 1461, D_WSTAG471_800A5FA8, 0x01350031, NULL, wstag471_event_1461_end }, { -1, NULL, 0, NULL, NULL },
};
