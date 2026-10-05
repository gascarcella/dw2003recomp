#include "wstag.h"

/* WSTAG958: stage 0x29E (fieldstg_stages_2d). */

extern WstagFuncs wstag958_funcs;
extern FieldstgVramPlace wstag958_vram_places[];
extern FieldstgPlacedActor *wstag958_actors[];
extern FieldstgSprite wstag958_sprites[];
extern FieldstgMapEvent wstag958_map_events[];
extern FieldstgEventDef wstag958_events[];

void wstag958_update(WstagObject *obj) {
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

WstagObject *wstag958_start(void *arg0) {
    WstagObject *obj = object_new(wstag958_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag958_funcs.setup();
    return obj;
}

void wstag958_event_1646_end(void) {
    gamestate_flags.set_flag(0x7C0E, 1);
}

void wstag958_setup(void) {
    fieldstg_stage.background_file = 0x221;
    fieldstg_stage.sprite_file = 0x092B0000;
    fieldstg_stage.sprites = wstag958_sprites;
    fieldstg_stage.map_events = wstag958_map_events;
    fieldstg_stage.mask_file = 0x92A;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x18300, 0x1E600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag958_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag958_actors;
    fieldstg_stage.events = wstag958_events;
    fieldstg_attr.set_file(0, 0x092B0001);
    fieldstg_attr.set_file(7, 0x092B0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag958_setup(void);
extern s16 D_WSTAG958_800A6304[];

FieldstgVramPlace wstag958_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 366, 152, 110, 352, 509 }, { 320, 256, 370, 366, 200, 110, 368, 509 },
    { 384, 256, 422, 256, 408, 0, 336, 508 }, { 384, 256, 430, 256, 440, 0, 352, 508 },
    { 320, 256, 374, 328, 216, 72, 368, 508 },
};
u16 D_WSTAG958_800A6058[4] = { 0x9079, 1, 0xFFFF, 0 };
u16 D_WSTAG958_800A6060[4] = { 0x7C00, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG958_800A6068[2] = { { NULL, D_WSTAG958_800A6058, 44 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG958_800A6080[2] = { { NULL, D_WSTAG958_800A6060, 123 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG958_800A6098[2] = { { NULL, NULL, 125 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG958_800A60B0[2] = { { NULL, NULL, 124 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG958_800A60C8[2] = { { NULL, NULL, 126 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG958_800A60E0 = { NULL, D_WSTAG958_800A6068, 21, 4, 408, 414, 1 };
FieldstgPlacedActor D_WSTAG958_800A60F4 = { NULL, D_WSTAG958_800A6080, 24, 5, 320, 515, 7 };
FieldstgPlacedActor D_WSTAG958_800A6108 = { NULL, D_WSTAG958_800A6098, 48, 6, 171, 403, 7 };
FieldstgPlacedActor D_WSTAG958_800A611C = { NULL, D_WSTAG958_800A60B0, 50, 7, 513, 241, 5 };
FieldstgPlacedActor D_WSTAG958_800A6130 = { NULL, D_WSTAG958_800A60C8, 57, 8, 352, 488, 3 };
FieldstgPlacedActor *wstag958_actors[6] = {
    &D_WSTAG958_800A60E0, &D_WSTAG958_800A60F4, &D_WSTAG958_800A6108, &D_WSTAG958_800A611C, &D_WSTAG958_800A6130,
    NULL,
};
FieldstgSprite wstag958_sprites[17] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 0xA, 0, 283, 466, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 0xA, 0, 428, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 0xA, 0, 307, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 1, 0xA, 0, 287, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 1, 0xA, 0, 297, 464, 0, 0 }, { 1, 0, 0x40, 6, 4, 1, 4, 9, 4, 0, 350, 338, 0, 0 },
    { 1, 0, 0x40, 6, 0xA, 1, 0xA, 0xD, 4, 0, 293, 279, 0, 0 },
    { 1, 0, 0x40, 6, 0xE, 1, 0xE, 0x11, 4, 0, 298, 381, 0, 0 },
    { 1, 0, 0x40, 6, 0x12, 1, 0x12, 0x15, 4, 0, 293, 427, 0, 0 },
    { 1, 0, 0x40, 4, 0x35, 2, 0, 1, 0xA, 0, 333, 483, 503, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 501, 393, 406, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 451, 436, 456, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 436, 464, 482, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 320, 474, 503, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 438, 274, 292, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 464, 277, 302, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag958_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x2F8, 0x104, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29C, 0x1E8, 0xA4, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag958_funcs = { wstag958_setup };
FieldstgEventDef wstag958_events[2] = {
    { 1646, D_WSTAG958_800A6304, 0x0158000F, NULL, wstag958_event_1646_end }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG958_800A6304[55] = {
    FIELDSTG_EVENT_WALK(2, 376, 428, 5),
    FIELDSTG_EVENT_PLACE(21, 408, 414),
    FIELDSTG_EVENT_ANIM(21, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(36),
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
