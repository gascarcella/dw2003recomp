#include "wstag.h"

/* WSTAG365: stage 0x224 (fieldstg_stages). */

extern WstagFuncs wstag365_funcs;
extern FieldstgVramPlace wstag365_vram_places[];
extern FieldstgPlacedActor *wstag365_actors[];
extern FieldstgSprite wstag365_sprites[];
extern FieldstgMapEvent wstag365_map_events[];

void wstag365_update(WstagObject *obj) {
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

WstagObject *wstag365_start(void *arg0) {
    WstagObject *obj = object_new(wstag365_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag365_funcs.setup();
    return obj;
}

void wstag365_setup(void) {
    fieldstg_stage.background_file = 0x239;
    fieldstg_stage.sprite_file = 0x023A0000;
    fieldstg_stage.sprites = wstag365_sprites;
    fieldstg_stage.map_events = wstag365_map_events;
    fieldstg_stage.mask_file = 0x3D7;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB700, 0xD300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag365_vram_places;
    fieldstg_stage.music = 0x31;
    fieldstg_stage.sound = 0x60C40000;
    fieldstg_stage.actors = wstag365_actors;
    fieldstg_attr.set_file(0, 0x023A0002);
    fieldstg_attr.set_file(7, 0x023A0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag365_setup(void);

FieldstgVramPlace wstag365_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 336, 216, 80, 352, 511 }, { 320, 256, 350, 312, 120, 56, 368, 511 },
    { 320, 256, 358, 332, 152, 76, 352, 510 },
};
u16 D_WSTAG365_800A5FF8[4] = { 0x818E, 0, 0xFFFF, 0 };
u16 D_WSTAG365_800A6000[6] = { 0x818E, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A600C[4] = { 0x818E, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG365_800A6014[2] = { { NULL, NULL, 29 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A602C[2] = { { NULL, NULL, 270 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A6044[2] = { { NULL, NULL, 263 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A605C[2] = { { NULL, NULL, 271 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A6074[2] = { { NULL, NULL, 266 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A608C[2] = { { NULL, NULL, 268 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A60A4[2] = { { NULL, NULL, 262 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A60BC[2] = { { NULL, NULL, 264 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A60D4[2] = { { NULL, NULL, 265 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A60EC[2] = { { NULL, NULL, 267 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG365_800A6104[3] = {
    { D_WSTAG365_800A5FF8, D_WSTAG365_800A6000, 743 }, { D_WSTAG365_800A600C, NULL, 808 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG365_800A6128[2] = { { NULL, NULL, 269 }, { NULL, NULL, 0 } };
u16 D_WSTAG365_800A6140[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6148[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6150[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6158[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6160[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6168[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6170[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6178[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6180[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6188[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A6190[6] = { 0x6006, 1, 0x1C47, 1, 0xFFFF, 0 };
u16 D_WSTAG365_800A619C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG365_800A61A4 = { D_WSTAG365_800A6140, D_WSTAG365_800A6014, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A61B8 = { D_WSTAG365_800A6148, D_WSTAG365_800A602C, 45, 4, 241, 201, 3 };
FieldstgPlacedActor D_WSTAG365_800A61CC = { D_WSTAG365_800A6150, D_WSTAG365_800A6044, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A61E0 = { D_WSTAG365_800A6158, D_WSTAG365_800A605C, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A61F4 = { D_WSTAG365_800A6160, D_WSTAG365_800A6074, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A6208 = { D_WSTAG365_800A6168, D_WSTAG365_800A608C, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A621C = { D_WSTAG365_800A6170, D_WSTAG365_800A60A4, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A6230 = { D_WSTAG365_800A6178, D_WSTAG365_800A60BC, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A6244 = { D_WSTAG365_800A6180, D_WSTAG365_800A60D4, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A6258 = { D_WSTAG365_800A6188, D_WSTAG365_800A60EC, 45, 4, 241, 201, 5 };
FieldstgPlacedActor D_WSTAG365_800A626C = { D_WSTAG365_800A6190, D_WSTAG365_800A6104, 64, 5, 192, 633, 1 };
FieldstgPlacedActor D_WSTAG365_800A6280 = { D_WSTAG365_800A619C, D_WSTAG365_800A6128, 157, 6, 241, 201, 5 };
FieldstgPlacedActor *wstag365_actors[13] = {
    &D_WSTAG365_800A61A4, &D_WSTAG365_800A61B8, &D_WSTAG365_800A61CC, &D_WSTAG365_800A61E0, &D_WSTAG365_800A61F4,
    &D_WSTAG365_800A6208, &D_WSTAG365_800A621C, &D_WSTAG365_800A6230, &D_WSTAG365_800A6244, &D_WSTAG365_800A6258,
    &D_WSTAG365_800A626C, &D_WSTAG365_800A6280, NULL,
};
FieldstgSprite wstag365_sprites[14] = {
    { 1, 0, 0x40, 2, 0x34, 2, 0, 7, 4, 0, 97, 134, 0, 0 }, { 1, 0, 0x40, 2, 0x35, 2, 0, 7, 4, 0, 137, 114, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 321, 137, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 577, 313, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 497, 272, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 4, 0, 167, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 7, 4, 0, 187, 293, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 7, 4, 0, 307, 233, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 276, 208, 242, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 51, 156, 208, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 425, 290, 314, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 421, 428, 438, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 423, 392, 429, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag365_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x223, 0x156, 0x1B0, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag365_funcs = { wstag365_setup };
