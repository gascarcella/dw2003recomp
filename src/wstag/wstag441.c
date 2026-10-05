#include "wstag.h"

/* WSTAG441: stage 0x2A0 (fieldstg_stages). */

extern WstagFuncs wstag441_funcs;
extern CVECTOR wstag441_color;
extern FieldstgVramPlace wstag441_vram_places[];
extern FieldstgPlacedActor *wstag441_actors[];
extern FieldstgSprite wstag441_sprites[];
extern FieldstgMapEvent wstag441_map_events[];

void wstag441_update(WstagObject *obj) {
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

WstagObject *wstag441_start(void *arg0) {
    WstagObject *obj = object_new(wstag441_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag441_funcs.setup();
    return obj;
}

void wstag441_setup(void) {
    fieldstg_stage.background_file = 0x6FC;
    fieldstg_stage.sprite_file = 0x06FD0000;
    fieldstg_stage.sprites = wstag441_sprites;
    fieldstg_stage.map_events = wstag441_map_events;
    fieldstg_stage.mask_file = 0x6FB;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x9100, 0x16C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag441_vram_places;
    fieldstg_stage.music = 0xE;
    fieldstg_stage.sound = 0x60380000;
    fieldstg_stage.actors = wstag441_actors;
    fieldstg_stage.color = wstag441_color;
    fieldstg_attr.set_file(0, 0x06FD0001);
    fieldstg_attr.set_file(7, 0x06FD0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag441/nonmatchings/wstag441", wstag441_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag441_setup(void);

FieldstgVramPlace wstag441_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 304, 160, 48, 368, 510 }, { 320, 256, 320, 330, 0, 74, 352, 509 },
    { 320, 256, 336, 330, 64, 74, 368, 509 }, { 320, 256, 344, 336, 96, 80, 352, 508 },
    { 320, 256, 352, 336, 128, 80, 368, 508 }, { 320, 256, 368, 336, 192, 80, 352, 507 },
};
FieldstgTalk D_WSTAG441_800A6094[2] = { { NULL, NULL, 162 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A60AC[2] = { { NULL, NULL, 159 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A60C4[2] = { { NULL, NULL, 164 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A60DC[2] = { { NULL, NULL, 165 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A60F4[2] = { { NULL, NULL, 158 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A610C[2] = { { NULL, NULL, 160 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A6124[2] = { { NULL, NULL, 161 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG441_800A613C[2] = { { NULL, NULL, 163 }, { NULL, NULL, 0 } };
u16 D_WSTAG441_800A6154[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG441_800A6160[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG441_800A616C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG441_800A6174[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG441_800A617C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG441_800A6188[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG441_800A6190[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG441_800A619C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG441_800A61A4 = { D_WSTAG441_800A6154, D_WSTAG441_800A6094, 49, 4, 145, 329, 5 };
FieldstgPlacedActor D_WSTAG441_800A61B8 = { D_WSTAG441_800A6160, D_WSTAG441_800A60AC, 50, 5, 177, 313, 7 };
FieldstgPlacedActor D_WSTAG441_800A61CC = { D_WSTAG441_800A616C, D_WSTAG441_800A60C4, 57, 6, 177, 313, 7 };
FieldstgPlacedActor D_WSTAG441_800A61E0 = { D_WSTAG441_800A6174, D_WSTAG441_800A60DC, 58, 7, 145, 329, 5 };
FieldstgPlacedActor D_WSTAG441_800A61F4 = { D_WSTAG441_800A617C, D_WSTAG441_800A60F4, 157, 8, 177, 313, 7 };
FieldstgPlacedActor D_WSTAG441_800A6208 = { D_WSTAG441_800A6188, D_WSTAG441_800A610C, 157, 8, 177, 313, 7 };
FieldstgPlacedActor D_WSTAG441_800A621C = { D_WSTAG441_800A6190, D_WSTAG441_800A6124, 158, 9, 145, 329, 5 };
FieldstgPlacedActor D_WSTAG441_800A6230 = { D_WSTAG441_800A619C, D_WSTAG441_800A613C, 158, 9, 145, 329, 5 };
FieldstgPlacedActor *wstag441_actors[9] = {
    &D_WSTAG441_800A61A4, &D_WSTAG441_800A61B8, &D_WSTAG441_800A61CC, &D_WSTAG441_800A61E0, &D_WSTAG441_800A61F4,
    &D_WSTAG441_800A6208, &D_WSTAG441_800A621C, &D_WSTAG441_800A6230, NULL,
};
FieldstgSprite wstag441_sprites[5] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 9, 0, 435, 247, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 1, 0x33, 0x38, 9, 0, 443, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x3F, 6, 0, 377, 384, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 482, 177, 200, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag441_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A1, 0x208, 0x9C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag441_funcs = { wstag441_setup };
