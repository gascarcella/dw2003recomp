#include "wstag.h"

/* WSTAG700: stage 0x263 (fieldstg_stages). */

extern WstagFuncs wstag700_funcs;
extern FieldstgVramPlace wstag700_vram_places[];
extern FieldstgPlacedActor *wstag700_actors[];
extern FieldstgSprite wstag700_sprites[];
extern FieldstgMapEvent wstag700_map_events[];
extern FieldstgEventDef wstag700_events[];

void wstag700_update(WstagObject *obj) {
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

WstagObject *wstag700_start(void *arg0) {
    WstagObject *obj = object_new(wstag700_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag700_funcs.setup();
    return obj;
}

void wstag700_event_1460_end(void) {
    gamestate_flags.set_flag(0x7C17, 1);
}

void wstag700_setup(void) {
    fieldstg_stage.background_file = 0x644;
    fieldstg_stage.sprite_file = 0x06450000;
    fieldstg_stage.sprites = wstag700_sprites;
    fieldstg_stage.map_events = wstag700_map_events;
    fieldstg_stage.mask_file = 0x643;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10000, 0x12500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag700_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag700_actors;
    fieldstg_stage.events = wstag700_events;
    fieldstg_attr.set_file(0, 0x06450001);
    fieldstg_attr.set_file(7, 0x06450002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag700_setup(void);

s16 D_WSTAG700_800A5FA4[55] = {
    FIELDSTG_EVENT_WALK(2, 315, 267, 3),
    FIELDSTG_EVENT_PLACE(21, 283, 251),
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
    FIELDSTG_EVENT_GOTO_MAP(0xC15, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag700_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 364, 256, 176, 0, 336, 511 }, { 320, 256, 352, 256, 128, 0, 352, 511 },
};
u16 D_WSTAG700_800A6094[4] = { 0x7A25, 1, 0xFFFF, 0 };
u16 D_WSTAG700_800A609C[4] = { 0x9066, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG700_800A60A4[2] = { { NULL, D_WSTAG700_800A6094, 363 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG700_800A60BC[2] = { { NULL, D_WSTAG700_800A609C, 360 }, { NULL, NULL, 0 } };
FieldstgPlacedActor D_WSTAG700_800A60D4 = { NULL, D_WSTAG700_800A60A4, 20, 4, 225, 289, 7 };
FieldstgPlacedActor D_WSTAG700_800A60E8 = { NULL, D_WSTAG700_800A60BC, 21, 5, 283, 251, 7 };
FieldstgPlacedActor *wstag700_actors[3] = { &D_WSTAG700_800A60D4, &D_WSTAG700_800A60E8, NULL };
FieldstgSprite wstag700_sprites[5] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 318, 111, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 383, 142, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 172, 267, 294, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 224, 235, 262, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag700_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x262, 0x248, 0x18C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag700_funcs = { wstag700_setup };
FieldstgEventDef wstag700_events[2] = {
    { 1460, D_WSTAG700_800A5FA4, 0x014A0026, NULL, wstag700_event_1460_end }, { -1, NULL, 0, NULL, NULL },
};
