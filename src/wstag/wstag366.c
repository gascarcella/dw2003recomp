#include "wstag.h"

/* WSTAG366: stage 0x293 (fieldstg_stages). */

extern WstagFuncs wstag366_funcs;
extern FieldstgVramPlace wstag366_vram_places[];
extern FieldstgPlacedActor *wstag366_actors[];
extern FieldstgSprite wstag366_sprites[];
extern FieldstgMapEvent wstag366_map_events[];
extern FieldstgEventDef wstag366_events[];

void wstag366_update(WstagObject *obj) {
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

WstagObject *wstag366_start(void *arg0) {
    WstagObject *obj = object_new(wstag366_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag366_funcs.setup();
    return obj;
}

void wstag366_event_720_end(void) {
    gamestate_flags.set_flag(0x401D, 1);
    gamestate_flags.set_flag(0x818C, 1);
}

void wstag366_setup(void) {
    fieldstg_stage.background_file = 0x584;
    fieldstg_stage.sprite_file = 0x05850000;
    fieldstg_stage.sprites = wstag366_sprites;
    fieldstg_stage.map_events = wstag366_map_events;
    fieldstg_stage.mask_file = 0x583;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xCB00, 0xDB00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag366_vram_places;
    fieldstg_stage.music = 0x31;
    fieldstg_stage.sound = 0x60C40000;
    fieldstg_stage.actors = wstag366_actors;
    fieldstg_stage.events = wstag366_events;
    fieldstg_attr.set_file(0, 0x05850001);
    fieldstg_attr.set_file(7, 0x05850002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag366_setup(void);

s16 D_WSTAG366_800A5FC0[302] = {
    FIELDSTG_EVENT_WALK(2, 225, 241, 3),
    FIELDSTG_EVENT_PLACE(45, 241, 201),
    FIELDSTG_EVENT_ANIM(45, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 45),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(45, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 45),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(45, 193, 225, 1),
    FIELDSTG_EVENT_WAIT_WALK(45),
    FIELDSTG_EVENT_ANIM(45, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 18, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(45, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(45, 73, 165, 3),
    FIELDSTG_EVENT_WAIT_WALK(45),
    FIELDSTG_EVENT_PLACE(45, 0, 0),
    FIELDSTG_EVENT_ANIM(45, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 19, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag366_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 336, 192, 80, 352, 511 },
};
u16 D_WSTAG366_800A628C[6] = { 0x601B, 1, 0x401D, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG366_800A6298 = { D_WSTAG366_800A628C, NULL, 45, 4, 241, 201, 5 };
FieldstgPlacedActor *wstag366_actors[2] = { &D_WSTAG366_800A6298, NULL };
FieldstgSprite wstag366_sprites[14] = {
    { 1, 0, 0x40, 2, 0x34, 2, 0, 7, 4, 0, 97, 134, 0, 0 }, { 1, 0, 0x40, 2, 0x35, 2, 0, 7, 4, 0, 137, 114, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 321, 137, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 577, 313, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 497, 272, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 4, 0, 167, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 7, 4, 0, 187, 293, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 7, 4, 0, 307, 233, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 276, 208, 242, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 51, 156, 208, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 425, 290, 314, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 421, 428, 438, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 423, 392, 429, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag366_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x292, 0x156, 0x1B0, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x245, 0x5E8, 0xDC, 3, 0, 0, 0 },
    { 0x601B, 1, 0x401D, 0, 8, 0x2D0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag366_funcs = { wstag366_setup };
FieldstgEventDef wstag366_events[2] = {
    { 720, D_WSTAG366_800A5FC0, 0x0127000D, NULL, wstag366_event_720_end }, { -1, NULL, 0, NULL, NULL },
};
