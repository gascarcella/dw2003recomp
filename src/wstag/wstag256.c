#include "wstag.h"

/* WSTAG256: stage 0x27E (fieldstg_stages). */

extern WstagFuncs wstag256_funcs;
extern FieldstgVramPlace wstag256_vram_places[];
extern FieldstgPlacedActor *wstag256_actors[];
extern FieldstgSprite wstag256_sprites[];
extern FieldstgMapEvent wstag256_map_events[];

void wstag256_update(WstagObject *obj) {
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

WstagObject *wstag256_start(void *arg0) {
    WstagObject *obj = object_new(wstag256_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag256_funcs.setup();
    return obj;
}

void wstag256_setup(void) {
    fieldstg_stage.background_file = 0x290;
    fieldstg_stage.sprites = wstag256_sprites;
    fieldstg_stage.map_events = wstag256_map_events;
    fieldstg_stage.sprite_file = 0x02910001;
    fieldstg_stage.mask_file = 0x3D1;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFB00, 0x13500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag256_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag256_actors;
    fieldstg_attr.set_file(0, 0x02910000);
    fieldstg_attr.set_file(7, 0x02910002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag256_setup(void);

FieldstgVramPlace wstag256_vram_places[22] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 362, 354, 168, 98, 336, 511 }, { 320, 256, 320, 346, 0, 90, 352, 511 },
    { 320, 256, 340, 370, 80, 114, 368, 511 }, { 320, 256, 348, 384, 112, 128, 336, 510 },
    { 320, 256, 320, 386, 0, 130, 352, 510 }, { 320, 256, 356, 394, 144, 138, 368, 510 },
    { 320, 256, 328, 403, 32, 147, 336, 509 }, { 320, 256, 370, 352, 200, 96, 352, 509 },
    { 320, 256, 336, 410, 64, 154, 368, 509 }, { 320, 256, 344, 424, 96, 168, 320, 508 },
    { 320, 256, 320, 426, 0, 170, 336, 508 }, { 320, 256, 352, 426, 128, 170, 352, 508 },
    { 320, 256, 360, 426, 160, 170, 368, 508 }, { 320, 256, 368, 426, 192, 170, 320, 507 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgTalk D_WSTAG256_800A60CC[2] = { { NULL, NULL, 120 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A60E4[2] = { { NULL, NULL, 122 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A60FC[2] = { { NULL, NULL, 124 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6114[2] = { { NULL, NULL, 126 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A612C[2] = { { NULL, NULL, 132 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6144[2] = { { NULL, NULL, 128 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A615C[2] = { { NULL, NULL, 130 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6174[2] = { { NULL, NULL, 135 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A618C[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A61A4[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A61BC[2] = { { NULL, NULL, 119 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A61D4[2] = { { NULL, NULL, 121 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A61EC[2] = { { NULL, NULL, 123 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6204[2] = { { NULL, NULL, 125 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A621C[2] = { { NULL, NULL, 134 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6234[2] = { { NULL, NULL, 136 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A624C[2] = { { NULL, NULL, 131 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6264[2] = { { NULL, NULL, 133 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A627C[2] = { { NULL, NULL, 127 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG256_800A6294[2] = { { NULL, NULL, 129 }, { NULL, NULL, 0 } };
u16 D_WSTAG256_800A62AC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62B8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62C0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62CC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62D4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62E0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62EC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A62F4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6300[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6308[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6310[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A631C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6324[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A6330[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6338[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A6344[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A634C[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A6358[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6360[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A636C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6374[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A6380[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A6388[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG256_800A6394[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A639C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A63A8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A63B0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG256_800A63B8[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG256_800A63C4 = { D_WSTAG256_800A62AC, D_WSTAG256_800A60CC, 25, 4, 275, 255, 1 };
FieldstgPlacedActor D_WSTAG256_800A63D8 = { D_WSTAG256_800A62B8, D_WSTAG256_800A60E4, 25, 4, 275, 255, 1 };
FieldstgPlacedActor D_WSTAG256_800A63EC = { D_WSTAG256_800A62C0, D_WSTAG256_800A60FC, 26, 5, 256, 321, 1 };
FieldstgPlacedActor D_WSTAG256_800A6400 = { D_WSTAG256_800A62CC, D_WSTAG256_800A6114, 26, 5, 256, 321, 1 };
FieldstgPlacedActor D_WSTAG256_800A6414 = { D_WSTAG256_800A62D4, D_WSTAG256_800A612C, 53, 6, 101, 203, 5 };
FieldstgPlacedActor D_WSTAG256_800A6428 = { D_WSTAG256_800A62E0, D_WSTAG256_800A6144, 54, 7, 133, 187, 1 };
FieldstgPlacedActor D_WSTAG256_800A643C = { D_WSTAG256_800A62EC, D_WSTAG256_800A615C, 54, 7, 217, 257, 5 };
FieldstgPlacedActor D_WSTAG256_800A6450 = { D_WSTAG256_800A62F4, D_WSTAG256_800A6174, 56, 8, 217, 257, 5 };
FieldstgPlacedActor D_WSTAG256_800A6464 = { D_WSTAG256_800A6300, D_WSTAG256_800A618C, 57, 9, 101, 203, 5 };
FieldstgPlacedActor D_WSTAG256_800A6478 = { D_WSTAG256_800A6308, D_WSTAG256_800A61A4, 58, 10, 133, 187, 1 };
FieldstgPlacedActor D_WSTAG256_800A648C = { D_WSTAG256_800A6310, NULL, 68, 11, 307, 240, 5 };
FieldstgPlacedActor D_WSTAG256_800A64A0 = { D_WSTAG256_800A631C, NULL, 68, 11, 307, 240, 5 };
FieldstgPlacedActor D_WSTAG256_800A64B4 = { D_WSTAG256_800A6324, D_WSTAG256_800A61BC, 157, 12, 275, 255, 1 };
FieldstgPlacedActor D_WSTAG256_800A64C8 = { D_WSTAG256_800A6330, D_WSTAG256_800A61D4, 157, 12, 275, 255, 1 };
FieldstgPlacedActor D_WSTAG256_800A64DC = { D_WSTAG256_800A6338, D_WSTAG256_800A61EC, 158, 13, 256, 321, 1 };
FieldstgPlacedActor D_WSTAG256_800A64F0 = { D_WSTAG256_800A6344, D_WSTAG256_800A6204, 158, 13, 256, 321, 1 };
FieldstgPlacedActor D_WSTAG256_800A6504 = { D_WSTAG256_800A634C, NULL, 159, 14, 307, 240, 5 };
FieldstgPlacedActor D_WSTAG256_800A6518 = { D_WSTAG256_800A6358, NULL, 159, 14, 307, 240, 5 };
FieldstgPlacedActor D_WSTAG256_800A652C = { D_WSTAG256_800A6360, D_WSTAG256_800A621C, 160, 15, 217, 257, 5 };
FieldstgPlacedActor D_WSTAG256_800A6540 = { D_WSTAG256_800A636C, D_WSTAG256_800A6234, 160, 15, 217, 257, 5 };
FieldstgPlacedActor D_WSTAG256_800A6554 = { D_WSTAG256_800A6374, D_WSTAG256_800A624C, 161, 16, 101, 203, 5 };
FieldstgPlacedActor D_WSTAG256_800A6568 = { D_WSTAG256_800A6380, D_WSTAG256_800A6264, 161, 16, 101, 203, 5 };
FieldstgPlacedActor D_WSTAG256_800A657C = { D_WSTAG256_800A6388, D_WSTAG256_800A627C, 162, 17, 133, 187, 1 };
FieldstgPlacedActor D_WSTAG256_800A6590 = { D_WSTAG256_800A6394, D_WSTAG256_800A6294, 162, 17, 133, 187, 1 };
FieldstgPlacedActor D_WSTAG256_800A65A4 = { D_WSTAG256_800A639C, NULL, 224, 18, 267, 266, 1 };
FieldstgPlacedActor D_WSTAG256_800A65B8 = { D_WSTAG256_800A63A8, NULL, 224, 18, 267, 266, 1 };
FieldstgPlacedActor D_WSTAG256_800A65CC = { D_WSTAG256_800A63B0, NULL, 270, 19, 267, 266, 1 };
FieldstgPlacedActor D_WSTAG256_800A65E0 = { D_WSTAG256_800A63B8, NULL, 270, 19, 267, 266, 1 };
FieldstgPlacedActor *wstag256_actors[29] = {
    &D_WSTAG256_800A63C4, &D_WSTAG256_800A63D8, &D_WSTAG256_800A63EC, &D_WSTAG256_800A6400, &D_WSTAG256_800A6414,
    &D_WSTAG256_800A6428, &D_WSTAG256_800A643C, &D_WSTAG256_800A6450, &D_WSTAG256_800A6464, &D_WSTAG256_800A6478,
    &D_WSTAG256_800A648C, &D_WSTAG256_800A64A0, &D_WSTAG256_800A64B4, &D_WSTAG256_800A64C8, &D_WSTAG256_800A64DC,
    &D_WSTAG256_800A64F0, &D_WSTAG256_800A6504, &D_WSTAG256_800A6518, &D_WSTAG256_800A652C, &D_WSTAG256_800A6540,
    &D_WSTAG256_800A6554, &D_WSTAG256_800A6568, &D_WSTAG256_800A657C, &D_WSTAG256_800A6590, &D_WSTAG256_800A65A4,
    &D_WSTAG256_800A65B8, &D_WSTAG256_800A65CC, &D_WSTAG256_800A65E0, NULL,
};
FieldstgSprite wstag256_sprites[25] = {
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 115, 131, 0, 0 }, { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 163, 107, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 227, 75, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 280, 73, 0, 0 },
    { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 328, 97, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 376, 121, 0, 0 },
    { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 117, 147, 0, 0 }, { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 165, 124, 0, 0 },
    { 1, 0, 0x40, 2, 0xA, 2, 0, 2, 4, 0, 229, 92, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 259, 90, 0, 0 },
    { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 307, 115, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 2, 0, 2, 4, 0, 355, 139, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xF, 0, 0, 0, 0, 0, 350, 125, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 192, 271, 301, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 184, 263, 288, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 178, 254, 277, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 152, 216, 233, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 280, 256, 280, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 264, 248, 271, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 248, 240, 264, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 232, 232, 256, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 217, 222, 248, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 312, 231, 263, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 332, 222, 253, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag256_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x112, 0x220, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0x68, 0x15A, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag256_funcs = { wstag256_setup };
