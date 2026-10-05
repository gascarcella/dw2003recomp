#include "wstag.h"

/* WSTAG411: stage 0x29B (fieldstg_stages). */

extern WstagFuncs wstag411_funcs;
extern CVECTOR wstag411_color;
extern FieldstgVramPlace wstag411_vram_places[];
extern FieldstgPlacedActor *wstag411_actors[];
extern FieldstgSprite wstag411_sprites[];
extern FieldstgMapEvent wstag411_map_events[];

void wstag411_update(WstagObject *obj) {
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

WstagObject *wstag411_start(void *arg0) {
    WstagObject *obj = object_new(wstag411_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag411_funcs.setup();
    return obj;
}

void wstag411_setup(void) {
    fieldstg_stage.background_file = 0x5AD;
    fieldstg_stage.sprite_file = 0x05AE0000;
    fieldstg_stage.sprites = wstag411_sprites;
    fieldstg_stage.map_events = wstag411_map_events;
    fieldstg_stage.mask_file = 0x5AC;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x24400, 0xB800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag411_vram_places;
    fieldstg_stage.music = 0xE;
    fieldstg_stage.sound = 0x60380000;
    fieldstg_stage.actors = wstag411_actors;
    fieldstg_stage.color = wstag411_color;
    fieldstg_attr.set_file(0, 0x05AE0001);
    fieldstg_attr.set_file(7, 0x05AE0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag411/nonmatchings/wstag411", wstag411_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag411_setup(void);

FieldstgVramPlace wstag411_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 368, 511 }, { 320, 256, 372, 296, 208, 40, 352, 510 },
    { 320, 256, 348, 335, 112, 79, 368, 510 }, { 320, 256, 356, 335, 144, 79, 352, 509 },
};
FieldstgTalk D_WSTAG411_800A6074[2] = { { NULL, NULL, 82 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG411_800A608C[2] = { { NULL, NULL, 79 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG411_800A60A4[2] = { { NULL, NULL, 78 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG411_800A60BC[2] = { { NULL, NULL, 80 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG411_800A60D4[2] = { { NULL, NULL, 81 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG411_800A60EC[2] = { { NULL, NULL, 83 }, { NULL, NULL, 0 } };
u16 D_WSTAG411_800A6104[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG411_800A6110[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG411_800A611C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG411_800A6128[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG411_800A6130[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG411_800A613C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG411_800A6144 = { D_WSTAG411_800A6104, D_WSTAG411_800A6074, 52, 4, 528, 209, 7 };
FieldstgPlacedActor D_WSTAG411_800A6158 = { D_WSTAG411_800A6110, D_WSTAG411_800A608C, 55, 5, 592, 168, 1 };
FieldstgPlacedActor D_WSTAG411_800A616C = { D_WSTAG411_800A611C, D_WSTAG411_800A60A4, 157, 6, 592, 168, 1 };
FieldstgPlacedActor D_WSTAG411_800A6180 = { D_WSTAG411_800A6128, D_WSTAG411_800A60BC, 157, 6, 592, 168, 1 };
FieldstgPlacedActor D_WSTAG411_800A6194 = { D_WSTAG411_800A6130, D_WSTAG411_800A60D4, 158, 7, 528, 209, 7 };
FieldstgPlacedActor D_WSTAG411_800A61A8 = { D_WSTAG411_800A613C, D_WSTAG411_800A60EC, 158, 7, 528, 209, 7 };
FieldstgPlacedActor *wstag411_actors[7] = {
    &D_WSTAG411_800A6144, &D_WSTAG411_800A6158, &D_WSTAG411_800A616C, &D_WSTAG411_800A6180, &D_WSTAG411_800A6194,
    &D_WSTAG411_800A61A8, NULL,
};
FieldstgSprite wstag411_sprites[4] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 429, 308, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 1, 0x34, 0x39, 4, 0, 776, 392, 0, 0 },
    { 1, 0, 0x68, 6, 0, 0, 0, 0, 0, 0, 218, 321, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag411_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x299, 0xD0, 0x2FC, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag411_funcs = { wstag411_setup };
