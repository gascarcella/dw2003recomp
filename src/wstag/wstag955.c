#include "wstag.h"

/* WSTAG955: stage 0x29B (fieldstg_stages_2d). */

extern WstagFuncs wstag955_funcs;
extern CVECTOR wstag955_color;
extern FieldstgVramPlace wstag955_vram_places[];
extern FieldstgPlacedActor *wstag955_actors[];
extern FieldstgSprite wstag955_sprites[];
extern FieldstgMapEvent wstag955_map_events[];

void wstag955_update(WstagObject *obj) {
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

WstagObject *wstag955_start(void *arg0) {
    WstagObject *obj = object_new(wstag955_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag955_funcs.setup();
    return obj;
}

void wstag955_setup(void) {
    fieldstg_stage.background_file = 0x39C;
    fieldstg_stage.sprite_file = 0x09250000;
    fieldstg_stage.sprites = wstag955_sprites;
    fieldstg_stage.map_events = wstag955_map_events;
    fieldstg_stage.mask_file = 0x924;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x26F00, 0xA800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag955_vram_places;
    fieldstg_stage.music = 0xE;
    fieldstg_stage.sound = 0x60380000;
    fieldstg_stage.actors = wstag955_actors;
    fieldstg_stage.color = wstag955_color;
    fieldstg_attr.set_file(0, 0x09250001);
    fieldstg_attr.set_file(7, 0x09250002);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag955/nonmatchings/wstag955", wstag955_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag955_setup(void);

FieldstgVramPlace wstag955_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 368, 511 }, { 320, 256, 372, 304, 208, 48, 352, 510 },
    { 320, 256, 362, 334, 168, 78, 368, 510 },
};
u16 D_WSTAG955_800A601C[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG955_800A6028[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG955_800A6034[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG955_800A6040[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG955_800A6050[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG955_800A605C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG955_800A6064[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG955_800A6070[4] = { 0x7838, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG955_800A6078[2] = { { NULL, NULL, 147 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG955_800A6090[2] = { { NULL, NULL, 77 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG955_800A60A8[5] = {
    { D_WSTAG955_800A601C, D_WSTAG955_800A6028, 79 }, { D_WSTAG955_800A6034, D_WSTAG955_800A6040, 80 },
    { D_WSTAG955_800A6050, D_WSTAG955_800A605C, 77 }, { D_WSTAG955_800A6064, D_WSTAG955_800A6070, 78 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG955_800A60E4[2] = { { NULL, NULL, 148 }, { NULL, NULL, 0 } };
u16 D_WSTAG955_800A60FC[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG955_800A6104[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG955_800A610C = { NULL, D_WSTAG955_800A6078, 37, 4, 479, 320, 1 };
FieldstgPlacedActor D_WSTAG955_800A6120 = { D_WSTAG955_800A60FC, D_WSTAG955_800A6090, 46, 5, 624, 152, 7 };
FieldstgPlacedActor D_WSTAG955_800A6134 = { D_WSTAG955_800A6104, D_WSTAG955_800A60A8, 46, 5, 624, 152, 7 };
FieldstgPlacedActor D_WSTAG955_800A6148 = { NULL, D_WSTAG955_800A60E4, 365, 6, 528, 209, 1 };
FieldstgPlacedActor *wstag955_actors[5] = {
    &D_WSTAG955_800A610C, &D_WSTAG955_800A6120, &D_WSTAG955_800A6134, &D_WSTAG955_800A6148, NULL,
};
FieldstgSprite wstag955_sprites[6] = {
    { 1, 0, 0x80, 2, 0, 0, 0, 0, 0, 0, 218, 322, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 429, 308, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 6, 0, 674, 320, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 1, 0x34, 0x39, 4, 0, 776, 392, 0, 0 },
    { 1, 0, 0x80, 6, 1, 0, 0, 0, 0, 0, 219, 367, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag955_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0xD0, 0x2FC, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag955_funcs = { wstag955_setup };
