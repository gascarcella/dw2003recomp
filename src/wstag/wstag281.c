#include "wstag.h"

/* WSTAG281: stage 0x282 (fieldstg_stages). */

extern WstagFuncs wstag281_funcs;
extern FieldstgVramPlace wstag281_vram_places[];
extern FieldstgPlacedActor *wstag281_actors[];
extern FieldstgSprite wstag281_sprites[];
extern FieldstgMapEvent wstag281_map_events[];

void wstag281_update(WstagObject *obj) {
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

WstagObject *wstag281_start(void *arg0) {
    WstagObject *obj = object_new(wstag281_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag281_funcs.setup();
    return obj;
}

void wstag281_setup(void) {
    fieldstg_stage.background_file = 0x516;
    fieldstg_stage.sprite_file = 0x05170000;
    fieldstg_stage.sprites = wstag281_sprites;
    fieldstg_stage.map_events = wstag281_map_events;
    fieldstg_stage.mask_file = 0x515;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12000, 0x13400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag281_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag281_actors;
    fieldstg_attr.set_file(0, 0x05170001);
    fieldstg_attr.set_file(7, 0x05170002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag281_setup(void);

FieldstgVramPlace wstag281_vram_places[21] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 426, 128, 170, 368, 511 }, { 320, 256, 358, 386, 152, 130, 352, 510 },
    { 320, 256, 366, 386, 184, 130, 368, 510 }, { 320, 256, 374, 386, 216, 130, 320, 509 },
    { 320, 256, 336, 393, 64, 137, 352, 509 }, { 320, 256, 360, 426, 160, 170, 368, 509 },
    { 320, 256, 368, 426, 192, 170, 320, 508 }, { 320, 256, 328, 433, 32, 177, 336, 508 },
    { 320, 256, 336, 433, 64, 177, 352, 508 }, { 320, 256, 344, 458, 96, 202, 368, 508 },
    { 320, 256, 352, 458, 128, 202, 320, 507 }, { 320, 256, 344, 418, 96, 162, 352, 507 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 368, 458, 192, 202, 368, 507 },
    { 320, 256, 320, 425, 0, 169, 320, 506 },
};
FieldstgTalk D_WSTAG281_800A60C0[2] = { { NULL, NULL, 143 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A60D8[2] = { { NULL, NULL, 145 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A60F0[2] = { { NULL, NULL, 147 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6108[2] = { { NULL, NULL, 159 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6120[2] = { { NULL, NULL, 160 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6138[2] = { { NULL, NULL, 151 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6150[2] = { { NULL, NULL, 161 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6168[2] = { { NULL, NULL, 154 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6180[2] = { { NULL, NULL, 142 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6198[2] = { { NULL, NULL, 144 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A61B0[2] = { { NULL, NULL, 153 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A61C8[2] = { { NULL, NULL, 155 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A61E0[2] = { { NULL, NULL, 146 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A61F8[2] = { { NULL, NULL, 148 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6210[2] = { { NULL, NULL, 150 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6228[2] = { { NULL, NULL, 152 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6240[2] = { { NULL, NULL, 502 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6258[2] = { { NULL, NULL, 156 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6270[2] = { { NULL, NULL, 158 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A6288[2] = { { NULL, NULL, 157 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG281_800A62A0[2] = { { NULL, NULL, 149 }, { NULL, NULL, 0 } };
u16 D_WSTAG281_800A62B8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62C4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62CC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62D8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62E0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62E8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62F4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A62FC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A6308[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG281_800A6314[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A631C[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG281_800A6328[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A6330[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG281_800A633C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A6344[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG281_800A6350[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A6358[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG281_800A6364[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A636C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG281_800A6378[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG281_800A6380 = { D_WSTAG281_800A62B8, D_WSTAG281_800A60C0, 46, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG281_800A6394 = { D_WSTAG281_800A62C4, D_WSTAG281_800A60D8, 46, 4, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG281_800A63A8 = { D_WSTAG281_800A62CC, D_WSTAG281_800A60F0, 47, 5, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG281_800A63BC = { D_WSTAG281_800A62D8, D_WSTAG281_800A6108, 48, 6, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG281_800A63D0 = { D_WSTAG281_800A62E0, D_WSTAG281_800A6120, 53, 7, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG281_800A63E4 = { D_WSTAG281_800A62E8, D_WSTAG281_800A6138, 56, 8, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG281_800A63F8 = { D_WSTAG281_800A62F4, D_WSTAG281_800A6150, 57, 9, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG281_800A640C = { D_WSTAG281_800A62FC, D_WSTAG281_800A6168, 58, 10, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG281_800A6420 = { D_WSTAG281_800A6308, D_WSTAG281_800A6180, 157, 11, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG281_800A6434 = { D_WSTAG281_800A6314, D_WSTAG281_800A6198, 157, 11, 471, 245, 5 };
FieldstgPlacedActor D_WSTAG281_800A6448 = { D_WSTAG281_800A631C, D_WSTAG281_800A61B0, 158, 12, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG281_800A645C = { D_WSTAG281_800A6328, D_WSTAG281_800A61C8, 158, 12, 352, 296, 5 };
FieldstgPlacedActor D_WSTAG281_800A6470 = { D_WSTAG281_800A6330, D_WSTAG281_800A61E0, 159, 13, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG281_800A6484 = { D_WSTAG281_800A633C, D_WSTAG281_800A61F8, 159, 13, 216, 277, 7 };
FieldstgPlacedActor D_WSTAG281_800A6498 = { D_WSTAG281_800A6344, D_WSTAG281_800A6210, 160, 14, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG281_800A64AC = { D_WSTAG281_800A6350, D_WSTAG281_800A6228, 160, 14, 264, 301, 3 };
FieldstgPlacedActor D_WSTAG281_800A64C0 = { NULL, D_WSTAG281_800A6240, 267, 15, 353, 160, 7 };
FieldstgPlacedActor D_WSTAG281_800A64D4 = { NULL, NULL, 268, 16, 360, 172, 7 };
FieldstgPlacedActor D_WSTAG281_800A64E8 = { D_WSTAG281_800A6358, D_WSTAG281_800A6258, 281, 17, 305, 193, 7 };
FieldstgPlacedActor D_WSTAG281_800A64FC = { D_WSTAG281_800A6364, D_WSTAG281_800A6270, 281, 17, 305, 193, 7 };
FieldstgPlacedActor D_WSTAG281_800A6510 = { D_WSTAG281_800A636C, D_WSTAG281_800A6288, 293, 18, 368, 224, 3 };
FieldstgPlacedActor D_WSTAG281_800A6524 = { D_WSTAG281_800A6378, D_WSTAG281_800A62A0, 293, 18, 305, 193, 7 };
FieldstgPlacedActor *wstag281_actors[23] = {
    &D_WSTAG281_800A6380, &D_WSTAG281_800A6394, &D_WSTAG281_800A63A8, &D_WSTAG281_800A63BC, &D_WSTAG281_800A63D0,
    &D_WSTAG281_800A63E4, &D_WSTAG281_800A63F8, &D_WSTAG281_800A640C, &D_WSTAG281_800A6420, &D_WSTAG281_800A6434,
    &D_WSTAG281_800A6448, &D_WSTAG281_800A645C, &D_WSTAG281_800A6470, &D_WSTAG281_800A6484, &D_WSTAG281_800A6498,
    &D_WSTAG281_800A64AC, &D_WSTAG281_800A64C0, &D_WSTAG281_800A64D4, &D_WSTAG281_800A64E8, &D_WSTAG281_800A64FC,
    &D_WSTAG281_800A6510, &D_WSTAG281_800A6524, NULL,
};
FieldstgSprite wstag281_sprites[26] = {
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 427, 90, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 84, 130, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 116, 114, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 148, 98, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 272, 44, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 368, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 417, 91, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 453, 113, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 473, 112, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 493, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 497, 124, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 4, 0, 292, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 400, 59, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 68, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 392, 48, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 416, 60, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 208, 263, 288, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 384, 191, 217, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 320, 169, 184, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 304, 161, 174, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 336, 161, 174, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 288, 153, 166, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 352, 153, 166, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 272, 144, 158, 0 },
    { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 368, 145, 158, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag281_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0x180, 0xD8, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x281, 0x92, 0x1A2, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x110, 0xB8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x100, 0xF0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag281_funcs = { wstag281_setup };
