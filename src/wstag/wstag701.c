#include "wstag.h"

/* WSTAG701: stage 0x2CB (fieldstg_stages). */

extern WstagFuncs wstag701_funcs;
extern FieldstgVramPlace wstag701_vram_places[];
extern FieldstgPlacedActor *wstag701_actors[];
extern FieldstgSprite wstag701_sprites[];
extern FieldstgMapEvent wstag701_map_events[];
extern FieldstgEventDef wstag701_events[];

void wstag701_update(WstagObject *obj) {
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

WstagObject *wstag701_start(void *arg0) {
    WstagObject *obj = object_new(wstag701_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag701_funcs.setup();
    return obj;
}

void wstag701_event_1464_end(void) {
    gamestate_flags.set_flag(0x7C1B, 1);
}

void wstag701_setup(void) {
    fieldstg_stage.background_file = 0x669;
    fieldstg_stage.sprite_file = 0x066A0000;
    fieldstg_stage.sprites = wstag701_sprites;
    fieldstg_stage.map_events = wstag701_map_events;
    fieldstg_stage.mask_file = 0x668;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x14400, 0xF700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag701_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag701_actors;
    fieldstg_stage.events = wstag701_events;
    fieldstg_attr.set_file(0, 0x066A0001);
    fieldstg_attr.set_file(7, 0x066A0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag701_setup(void);

s16 D_WSTAG701_800A5FA4[55] = {
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
    FIELDSTG_EVENT_GOTO_MAP(0xC19, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag701_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 299, 0, 43, 336, 511 }, { 320, 256, 352, 291, 128, 35, 352, 511 },
    { 320, 256, 364, 291, 176, 35, 368, 511 }, { 320, 256, 372, 291, 208, 35, 336, 510 },
    { 320, 256, 328, 299, 32, 43, 352, 510 }, { 320, 256, 336, 299, 64, 43, 368, 510 },
};
u16 D_WSTAG701_800A60D4[4] = { 0x7A2F, 1, 0xFFFF, 0 };
u16 D_WSTAG701_800A60DC[4] = { 0x906A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG701_800A60E4[2] = { { NULL, D_WSTAG701_800A60D4, 718 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A60FC[2] = { { NULL, D_WSTAG701_800A60DC, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A6114[2] = { { NULL, NULL, 493 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A612C[2] = { { NULL, NULL, 491 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A6144[2] = { { NULL, NULL, 492 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A615C[2] = { { NULL, NULL, 492 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A6174[2] = { { NULL, NULL, 494 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG701_800A618C[2] = { { NULL, NULL, 494 }, { NULL, NULL, 0 } };
u16 D_WSTAG701_800A61A4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG701_800A61B0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG701_800A61BC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG701_800A61C8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG701_800A61D0[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG701_800A61DC[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG701_800A61E4 = { NULL, D_WSTAG701_800A60E4, 20, 4, 225, 289, 7 };
FieldstgPlacedActor D_WSTAG701_800A61F8 = { NULL, D_WSTAG701_800A60FC, 21, 5, 283, 251, 7 };
FieldstgPlacedActor D_WSTAG701_800A620C = { D_WSTAG701_800A61A4, D_WSTAG701_800A6114, 51, 6, 145, 297, 7 };
FieldstgPlacedActor D_WSTAG701_800A6220 = { D_WSTAG701_800A61B0, D_WSTAG701_800A612C, 56, 7, 288, 209, 3 };
FieldstgPlacedActor D_WSTAG701_800A6234 = { D_WSTAG701_800A61BC, D_WSTAG701_800A6144, 157, 8, 288, 209, 3 };
FieldstgPlacedActor D_WSTAG701_800A6248 = { D_WSTAG701_800A61C8, D_WSTAG701_800A615C, 157, 8, 288, 209, 3 };
FieldstgPlacedActor D_WSTAG701_800A625C = { D_WSTAG701_800A61D0, D_WSTAG701_800A6174, 158, 9, 145, 297, 7 };
FieldstgPlacedActor D_WSTAG701_800A6270 = { D_WSTAG701_800A61DC, D_WSTAG701_800A618C, 158, 9, 145, 297, 7 };
FieldstgPlacedActor *wstag701_actors[9] = {
    &D_WSTAG701_800A61E4, &D_WSTAG701_800A61F8, &D_WSTAG701_800A620C, &D_WSTAG701_800A6220, &D_WSTAG701_800A6234,
    &D_WSTAG701_800A6248, &D_WSTAG701_800A625C, &D_WSTAG701_800A6270, NULL,
};
FieldstgSprite wstag701_sprites[5] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 318, 111, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 383, 142, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 224, 235, 262, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 172, 267, 294, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag701_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CA, 0x248, 0x18C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag701_funcs = { wstag701_setup };
FieldstgEventDef wstag701_events[2] = {
    { 1464, D_WSTAG701_800A5FA4, 0x014A0027, NULL, wstag701_event_1464_end }, { -1, NULL, 0, NULL, NULL },
};
