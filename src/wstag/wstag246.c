#include "wstag.h"

/* WSTAG246: stage 0x27C (fieldstg_stages). */

extern FieldstgVramPlace wstag246_vram_places[];
extern FieldstgPlacedActor *wstag246_actors[];
extern FieldstgSprite wstag246_sprites[];
extern FieldstgMapEvent wstag246_map_events[];
extern WstagFuncs wstag246_funcs;

void wstag246_update(WstagObject *obj) {
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

WstagObject *wstag246_start(void *arg0) {
    WstagObject *obj = object_new(wstag246_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag246_funcs.setup();
    return obj;
}

void wstag246_setup(void) {
    fieldstg_stage.background_file = 0x4E6;
    fieldstg_stage.sprite_file = 0x04E70000;
    fieldstg_stage.sprites = wstag246_sprites;
    fieldstg_stage.map_events = wstag246_map_events;
    fieldstg_stage.mask_file = 0x4E5;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15C00, 0x18B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag246_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag246_actors;
    fieldstg_attr.set_file(0, 0x04E70001);
    fieldstg_attr.set_file(7, 0x04E70002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag246_setup(void);

FieldstgVramPlace wstag246_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 432, 366, 448, 110, 336, 511 }, { 384, 256, 432, 406, 448, 150, 352, 511 },
    { 384, 256, 432, 478, 448, 222, 368, 511 }, { 384, 256, 432, 438, 448, 182, 336, 510 },
    { 448, 256, 498, 256, 712, 0, 352, 510 }, { 448, 256, 498, 288, 712, 32, 368, 510 },
    { 448, 256, 482, 319, 648, 63, 336, 509 }, { 384, 256, 416, 452, 384, 196, 352, 509 },
    { 448, 256, 490, 319, 680, 63, 368, 509 },
};
u16 D_WSTAG246_800A6060[4] = { 0x7A11, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6068[4] = { 0x7A0F, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6070[4] = { 0x7A10, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG246_800A6078[2] = { { NULL, D_WSTAG246_800A6060, 424 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6090[2] = { { NULL, D_WSTAG246_800A6068, 420 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A60A8[2] = { { NULL, NULL, 111 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A60C0[2] = { { NULL, NULL, 109 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A60D8[2] = { { NULL, NULL, 113 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A60F0[2] = { { NULL, NULL, 112 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6108[2] = { { NULL, NULL, 114 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6120[2] = { { NULL, NULL, 108 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6138[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6150[2] = { { NULL, D_WSTAG246_800A6070, 422 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6168[2] = { { NULL, NULL, 443 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6180[2] = { { NULL, NULL, 105 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A6198[2] = { { NULL, NULL, 107 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A61B0[2] = { { NULL, NULL, 442 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A61C8[2] = { { NULL, NULL, 104 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG246_800A61E0[2] = { { NULL, NULL, 106 }, { NULL, NULL, 0 } };
u16 D_WSTAG246_800A61F8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6200[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A620C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6218[6] = { 0x1A0A, 0, 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6224[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A622C[6] = { 0x1A0A, 0, 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6238[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6240[6] = { 0x602B, 1, 0x8008, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A624C[8] = { 0x8008, 0, 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A625C[6] = { 0x8008, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6268[8] = { 0x6026, 1, 0x1A0A, 1, 0x8008, 1, 0xFFFF, 0 };
u16 D_WSTAG246_800A6278[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG246_800A6284[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG246_800A628C = { NULL, D_WSTAG246_800A6078, 22, 4, 449, 281, 7 };
FieldstgPlacedActor D_WSTAG246_800A62A0 = { NULL, D_WSTAG246_800A6090, 23, 5, 304, 369, 7 };
FieldstgPlacedActor D_WSTAG246_800A62B4 = { D_WSTAG246_800A61F8, D_WSTAG246_800A60A8, 46, 6, 287, 433, 5 };
FieldstgPlacedActor D_WSTAG246_800A62C8 = { D_WSTAG246_800A6200, D_WSTAG246_800A60C0, 46, 6, 287, 433, 5 };
FieldstgPlacedActor D_WSTAG246_800A62DC = { D_WSTAG246_800A620C, D_WSTAG246_800A60D8, 52, 7, 480, 338, 3 };
FieldstgPlacedActor D_WSTAG246_800A62F0 = { D_WSTAG246_800A6218, D_WSTAG246_800A60F0, 157, 8, 480, 338, 3 };
FieldstgPlacedActor D_WSTAG246_800A6304 = { D_WSTAG246_800A6224, D_WSTAG246_800A6108, 157, 8, 480, 338, 3 };
FieldstgPlacedActor D_WSTAG246_800A6318 = { D_WSTAG246_800A622C, D_WSTAG246_800A6120, 158, 9, 287, 433, 5 };
FieldstgPlacedActor D_WSTAG246_800A632C = { D_WSTAG246_800A6238, D_WSTAG246_800A6138, 158, 9, 287, 433, 5 };
FieldstgPlacedActor D_WSTAG246_800A6340 = { NULL, D_WSTAG246_800A6150, 183, 10, 359, 308, 7 };
FieldstgPlacedActor D_WSTAG246_800A6354 = { D_WSTAG246_800A6240, D_WSTAG246_800A6168, 255, 11, 144, 352, 1 };
FieldstgPlacedActor D_WSTAG246_800A6368 = { D_WSTAG246_800A624C, D_WSTAG246_800A6180, 255, 11, 152, 397, 7 };
FieldstgPlacedActor D_WSTAG246_800A637C = { D_WSTAG246_800A625C, D_WSTAG246_800A6198, 255, 11, 152, 397, 7 };
FieldstgPlacedActor D_WSTAG246_800A6390 = { D_WSTAG246_800A6268, D_WSTAG246_800A61B0, 255, 11, 144, 352, 1 };
FieldstgPlacedActor D_WSTAG246_800A63A4 = { D_WSTAG246_800A6278, D_WSTAG246_800A61C8, 281, 12, 152, 397, 7 };
FieldstgPlacedActor D_WSTAG246_800A63B8 = { D_WSTAG246_800A6284, D_WSTAG246_800A61E0, 281, 12, 152, 397, 7 };
FieldstgPlacedActor *wstag246_actors[17] = {
    &D_WSTAG246_800A628C, &D_WSTAG246_800A62A0, &D_WSTAG246_800A62B4, &D_WSTAG246_800A62C8, &D_WSTAG246_800A62DC,
    &D_WSTAG246_800A62F0, &D_WSTAG246_800A6304, &D_WSTAG246_800A6318, &D_WSTAG246_800A632C, &D_WSTAG246_800A6340,
    &D_WSTAG246_800A6354, &D_WSTAG246_800A6368, &D_WSTAG246_800A637C, &D_WSTAG246_800A6390, &D_WSTAG246_800A63A4,
    &D_WSTAG246_800A63B8, NULL,
};
FieldstgSprite wstag246_sprites[13] = {
    { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 384, 79, 0, 0 }, { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 492, 137, 0, 0 },
    { 1, 0, 0x40, 2, 0x11, 2, 0, 1, 4, 0, 548, 165, 0, 0 }, { 1, 0x64, 0x40, 6, 5, 0, 0, 0, 0, 0, 304, 99, 0, 0 },
    { 1, 0x65, 0x40, 6, 6, 0, 0, 0, 0, 0, 64, 285, 0, 0 }, { 1, 0, 0x40, 6, 7, 0, 0, 0, 0, 0, 485, 145, 0, 0 },
    { 1, 0, 0x40, 6, 7, 0, 0, 0, 0, 0, 541, 173, 0, 0 }, { 1, 0, 0xA0, 4, 0, 0, 0, 0, 0, 0, 144, 257, 380, 0 },
    { 1, 0, 0x78, 4, 1, 0, 0, 0, 0, 0, 256, 257, 337, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 384, 262, 287, 0 },
    { 1, 0, 0x64, 4, 3, 0, 0, 0, 0, 0, 240, 73, 150, 0 }, { 1, 0, 0x64, 4, 4, 0, 0, 0, 0, 0, 0, 255, 335, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag246_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x1E0, 0x1DA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x240, 0x1AA, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27D, 0x218, 0xE4, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27D, 0x128, 0x19C, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x11F, 0xBA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x110, 0xFE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag246_funcs = { wstag246_setup };
