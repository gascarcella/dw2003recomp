#include "wstag.h"

/* WSTAG237: stage 0x20B (fieldstg_stages). */

extern WstagFuncs wstag237_funcs;
extern FieldstgVramPlace wstag237_vram_places[];
extern FieldstgPlacedActor *wstag237_actors[];
extern FieldstgSprite wstag237_sprites[];
extern FieldstgMapEvent wstag237_map_events[];
extern FieldstgEventDef wstag237_events[];

void wstag237_update(WstagObject *obj) {
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

WstagObject *wstag237_start(void *arg0) {
    WstagObject *obj = object_new(wstag237_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag237_funcs.setup();
    return obj;
}

void wstag237_event_300_end(void) {
    gamestate_data.progress = 0xD;
}

void wstag237_setup(void) {
    fieldstg_stage.background_file = 0x2B7;
    fieldstg_stage.sprite_file = 0x02B80000;
    fieldstg_stage.sprites = wstag237_sprites;
    fieldstg_stage.map_events = wstag237_map_events;
    fieldstg_stage.mask_file = 0x33F;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x5900, 0x12C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag237_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag237_actors;
    fieldstg_stage.events = wstag237_events;
    fieldstg_attr.set_file(0, 0x02B80001);
    fieldstg_attr.set_file(7, 0x02B80002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag237_setup(void);

s16 D_WSTAG237_800A5F88[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 380, 161, 5),
    FIELDSTG_EVENT_PLACE(11, 417, 145),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 11, 2),
    FIELDSTG_EVENT_ANIM(11, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 11, 2),
    FIELDSTG_EVENT_ANIM(11, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x203, 96, 400, 5),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag237_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 328, 304, 32, 48, 368, 511 },
};
u16 D_WSTAG237_800A609C[4] = { 0x902D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG237_800A60A4[2] = { { NULL, D_WSTAG237_800A609C, 88 }, { NULL, NULL, 0 } };
u16 D_WSTAG237_800A60BC[6] = { 0x600C, 1, 0x800F, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG237_800A60C8 = { D_WSTAG237_800A60BC, D_WSTAG237_800A60A4, 11, 4, 417, 145, 7 };
FieldstgPlacedActor *wstag237_actors[2] = { &D_WSTAG237_800A60C8, NULL };
FieldstgSprite wstag237_sprites[10] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 261, 137, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 53, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 173, 179, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 325, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 435, 83, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 523, 127, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 555, 239, 0, 0 }, { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 286, 127, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 224, 144, 184, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag237_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x201, 0xF0, 0x108, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21B, 0x138, 0x64, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20A, 0xA7, 0x153, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x203, 0xC2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x212, 0x128, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag237_funcs = { wstag237_setup };
FieldstgEventDef wstag237_events[2] = {
    { 300, D_WSTAG237_800A5F88, 0x0112001B, NULL, wstag237_event_300_end }, { -1, NULL, 0, NULL, NULL },
};
