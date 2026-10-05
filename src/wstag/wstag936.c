#include "wstag.h"

/* WSTAG936: stage 0x281 (fieldstg_stages_2d). */

extern WstagFuncs wstag936_funcs;
extern CVECTOR wstag936_color;
extern FieldstgVramPlace wstag936_vram_places[];
extern FieldstgPlacedActor *wstag936_actors[];
extern FieldstgSprite wstag936_sprites[];
extern FieldstgMapEvent wstag936_map_events[];

void wstag936_update(WstagObject *obj) {
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

WstagObject *wstag936_start(void *arg0) {
    WstagObject *obj = object_new(wstag936_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag936_funcs.setup();
    return obj;
}

void wstag936_setup(void) {
    fieldstg_stage.background_file = 0x1B0;
    fieldstg_stage.sprite_file = 0x08FF0000;
    fieldstg_stage.sprites = wstag936_sprites;
    fieldstg_stage.map_events = wstag936_map_events;
    fieldstg_stage.mask_file = 0x8FE;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFD00, 0x1CD00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag936_vram_places;
    fieldstg_stage.music = 0x2C;
    fieldstg_stage.sound = 0x60B00000;
    fieldstg_stage.actors = wstag936_actors;
    fieldstg_stage.color = wstag936_color;
    fieldstg_attr.set_file(0, 0x08FF0001);
    fieldstg_attr.set_file(7, 0x08FF0002);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag936/nonmatchings/wstag936", wstag936_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag936_setup(void);

FieldstgVramPlace wstag936_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG936_800A5FFC[4] = { 0x1806, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A6004[6] = { 0x1806, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG936_800A6010[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A6018[8] = { 0x1806, 0, 0, 1, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG936_800A6028[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A6030[10] = {
    0x1806, 0, 0, 1, 1, 1, 2, 0,
    0xFFFF, 0,
};
u16 D_WSTAG936_800A6044[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A604C[12] = {
    0x1806, 0, 0, 1, 1, 1, 2, 1,
    3, 0, 0xFFFF, 0,
};
u16 D_WSTAG936_800A6064[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A606C[14] = {
    0x1806, 0, 0, 1, 1, 1, 2, 1,
    4, 0, 3, 1, 0xFFFF, 0,
};
u16 D_WSTAG936_800A6088[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG936_800A6090[14] = {
    0x1806, 0, 0, 1, 1, 1, 2, 1,
    3, 1, 4, 1, 0xFFFF, 0,
};
u16 D_WSTAG936_800A60AC[8] = { 0x1806, 1, 0x8025, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG936_800A60BC[8] = {
    { D_WSTAG936_800A5FFC, NULL, 88 }, { D_WSTAG936_800A6004, D_WSTAG936_800A6010, 85 },
    { D_WSTAG936_800A6018, D_WSTAG936_800A6028, 86 }, { D_WSTAG936_800A6030, D_WSTAG936_800A6044, 86 },
    { D_WSTAG936_800A604C, D_WSTAG936_800A6064, 86 }, { D_WSTAG936_800A606C, D_WSTAG936_800A6088, 86 },
    { D_WSTAG936_800A6090, D_WSTAG936_800A60AC, 87 }, { NULL, NULL, 0 },
};
FieldstgPlacedActor D_WSTAG936_800A611C = { NULL, D_WSTAG936_800A60BC, 63, 4, 175, 408, 1 };
FieldstgPlacedActor *wstag936_actors[2] = { &D_WSTAG936_800A611C, NULL };
FieldstgSprite wstag936_sprites[19] = {
    { 1, 0, 0x9F, 2, 0x37, 1, 0x37, 0x39, 6, 0, 450, 296, 0, 0 },
    { 1, 0, 0xBE, 2, 0x3A, 1, 0x3A, 0x3C, 6, 0, 560, 270, 0, 0 },
    { 1, 0, 0x8F, 2, 0x1D, 1, 0x1D, 0x31, 8, 0, 584, 352, 0, 0 },
    { 1, 0, 0xB6, 2, 6, 1, 6, 0x1C, 8, 0, 471, 277, 0, 0 }, { 1, 0, 0xB6, 2, 6, 1, 6, 0x1C, 8, 0, 535, 294, 0, 0 },
    { 1, 0, 0x70, 2, 4, 0, 0, 0, 0, 0, 374, 310, 0, 0 }, { 1, 0, 0xD0, 2, 5, 0, 0, 0, 0, 0, 438, 232, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 609, 253, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 4, 0, 633, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 640, 242, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 1, 4, 0, 652, 281, 0, 0 },
    { 1, 0, 0x9F, 6, 0x37, 1, 0x37, 0x39, 6, 0, 366, 170, 0, 0 },
    { 1, 0, 0x67, 6, 0x3D, 1, 0x3D, 0x52, 8, 0, 507, 185, 0, 0 },
    { 1, 0, 0xB6, 6, 6, 1, 6, 0x1C, 8, 0, 421, 106, 0, 0 }, { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 91, 362, 0, 0 },
    { 1, 0x65, 0x40, 6, 3, 0, 0, 0, 0, 0, 770, 115, 0, 0 }, { 1, 0, 0x90, 4, 0, 0, 0, 0, 0, 0, 0, 256, 410, 0 },
    { 1, 0, 0xB0, 4, 1, 0, 0, 0, 0, 0, 768, 0, 165, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag936_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x282, 0x13A, 0x144, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27F, 0xE8, 0x1A2, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag936_funcs = { wstag936_setup };
