#include "wstag.h"

/* WSTAG250: stage 0x20E (fieldstg_stages). */

extern WstagFuncs wstag250_funcs;
extern FieldstgVramPlace wstag250_vram_places[];
extern FieldstgPlacedActor *wstag250_actors[];
extern FieldstgSprite wstag250_sprites[];
extern FieldstgMapEvent wstag250_map_events[];

void wstag250_update(WstagObject *obj) {
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

WstagObject *wstag250_start(void *arg0) {
    WstagObject *obj = object_new(wstag250_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag250_funcs.setup();
    return obj;
}

void wstag250_setup(void) {
    fieldstg_stage.background_file = 0x1A8;
    fieldstg_stage.sprite_file = 0x01A90000;
    fieldstg_stage.sprites = wstag250_sprites;
    fieldstg_stage.map_events = wstag250_map_events;
    fieldstg_stage.mask_file = 0x3CF;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13D00, 0x16F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag250_vram_places;
    fieldstg_stage.music = 7;
    fieldstg_stage.sound = 0x601C0000;
    fieldstg_stage.actors = wstag250_actors;
    fieldstg_attr.set_file(0, 0x01A90001);
    fieldstg_attr.set_file(7, 0x01A90002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag250_setup(void);

FieldstgVramPlace wstag250_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 368, 511 }, { 320, 256, 372, 296, 208, 40, 320, 510 },
    { 320, 256, 372, 328, 208, 72, 336, 510 }, { 320, 256, 372, 368, 208, 112, 352, 510 },
    { 384, 256, 438, 344, 472, 88, 368, 510 }, { 384, 256, 384, 400, 256, 144, 320, 509 },
};
u16 D_WSTAG250_800A6030[4] = { 0x1A13, 0, 0xFFFF, 0 };
u16 D_WSTAG250_800A6038[4] = { 0x1A13, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6040[4] = { 0x1A13, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6048[4] = { 0x7A04, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6050[4] = { 0x1A12, 0, 0xFFFF, 0 };
u16 D_WSTAG250_800A6058[4] = { 0x1A12, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6060[4] = { 0x1A12, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6068[4] = { 0x7A03, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG250_800A6070[3] = {
    { D_WSTAG250_800A6030, D_WSTAG250_800A6038, 74 }, { D_WSTAG250_800A6040, D_WSTAG250_800A6048, 40 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG250_800A6094[3] = {
    { D_WSTAG250_800A6050, D_WSTAG250_800A6058, 75 }, { D_WSTAG250_800A6060, D_WSTAG250_800A6068, 41 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG250_800A60B8[2] = { { NULL, NULL, 43 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A60D0[2] = { { NULL, NULL, 1098 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A60E8[2] = { { NULL, NULL, 1090 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6100[2] = { { NULL, NULL, 1092 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6118[2] = { { NULL, NULL, 1094 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6130[2] = { { NULL, NULL, 1096 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6148[2] = { { NULL, NULL, 1089 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6160[2] = { { NULL, NULL, 1091 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6178[2] = { { NULL, NULL, 1093 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6190[2] = { { NULL, NULL, 1095 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A61A8[2] = { { NULL, NULL, 42 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A61C0[2] = { { NULL, NULL, 1088 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A61D8[2] = { { NULL, NULL, 1080 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A61F0[2] = { { NULL, NULL, 1082 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6208[2] = { { NULL, NULL, 1084 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6220[2] = { { NULL, NULL, 1086 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6238[2] = { { NULL, NULL, 1079 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6250[2] = { { NULL, NULL, 1081 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6268[2] = { { NULL, NULL, 1083 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6280[2] = { { NULL, NULL, 1085 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A6298[2] = { { NULL, NULL, 1087 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG250_800A62B0[2] = { { NULL, NULL, 1097 }, { NULL, NULL, 0 } };
u16 D_WSTAG250_800A62C8[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62D0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62D8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62E0[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62E8[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62F0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A62F8[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6300[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6308[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6310[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6318[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6320[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6328[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6330[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6338[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6340[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6348[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6350[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6358[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6360[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6368[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG250_800A6370[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG250_800A6378 = { NULL, D_WSTAG250_800A6070, 22, 4, 451, 178, 7 };
FieldstgPlacedActor D_WSTAG250_800A638C = { NULL, D_WSTAG250_800A6094, 23, 5, 382, 331, 1 };
FieldstgPlacedActor D_WSTAG250_800A63A0 = { D_WSTAG250_800A62C8, D_WSTAG250_800A60B8, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A63B4 = { D_WSTAG250_800A62D0, D_WSTAG250_800A60D0, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A63C8 = { D_WSTAG250_800A62D8, D_WSTAG250_800A60E8, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A63DC = { D_WSTAG250_800A62E0, D_WSTAG250_800A6100, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A63F0 = { D_WSTAG250_800A62E8, D_WSTAG250_800A6118, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A6404 = { D_WSTAG250_800A62F0, D_WSTAG250_800A6130, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A6418 = { D_WSTAG250_800A62F8, D_WSTAG250_800A6148, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A642C = { D_WSTAG250_800A6300, D_WSTAG250_800A6160, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A6440 = { D_WSTAG250_800A6308, D_WSTAG250_800A6178, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A6454 = { D_WSTAG250_800A6310, D_WSTAG250_800A6190, 53, 6, 533, 171, 5 };
FieldstgPlacedActor D_WSTAG250_800A6468 = { D_WSTAG250_800A6318, D_WSTAG250_800A61A8, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A647C = { D_WSTAG250_800A6320, D_WSTAG250_800A61C0, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A6490 = { D_WSTAG250_800A6328, D_WSTAG250_800A61D8, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A64A4 = { D_WSTAG250_800A6330, D_WSTAG250_800A61F0, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A64B8 = { D_WSTAG250_800A6338, D_WSTAG250_800A6208, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A64CC = { D_WSTAG250_800A6340, D_WSTAG250_800A6220, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A64E0 = { D_WSTAG250_800A6348, D_WSTAG250_800A6238, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A64F4 = { D_WSTAG250_800A6350, D_WSTAG250_800A6250, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A6508 = { D_WSTAG250_800A6358, D_WSTAG250_800A6268, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A651C = { D_WSTAG250_800A6360, D_WSTAG250_800A6280, 54, 7, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A6530 = { D_WSTAG250_800A6368, D_WSTAG250_800A6298, 157, 8, 195, 402, 1 };
FieldstgPlacedActor D_WSTAG250_800A6544 = { D_WSTAG250_800A6370, D_WSTAG250_800A62B0, 158, 9, 533, 171, 5 };
FieldstgPlacedActor *wstag250_actors[25] = {
    &D_WSTAG250_800A6378, &D_WSTAG250_800A638C, &D_WSTAG250_800A63A0, &D_WSTAG250_800A63B4, &D_WSTAG250_800A63C8,
    &D_WSTAG250_800A63DC, &D_WSTAG250_800A63F0, &D_WSTAG250_800A6404, &D_WSTAG250_800A6418, &D_WSTAG250_800A642C,
    &D_WSTAG250_800A6440, &D_WSTAG250_800A6454, &D_WSTAG250_800A6468, &D_WSTAG250_800A647C, &D_WSTAG250_800A6490,
    &D_WSTAG250_800A64A4, &D_WSTAG250_800A64B8, &D_WSTAG250_800A64CC, &D_WSTAG250_800A64E0, &D_WSTAG250_800A64F4,
    &D_WSTAG250_800A6508, &D_WSTAG250_800A651C, &D_WSTAG250_800A6530, &D_WSTAG250_800A6544, NULL,
};
FieldstgSprite wstag250_sprites[20] = {
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x3D, 4, 0, 224, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x53, 0xA, 0, 95, 248, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x53, 0xA, 0, 181, 470, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 1, 0x57, 0x59, 0xA, 0, 123, 226, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 148, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 153, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 114, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 155, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 168, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 105, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 126, 300, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 0xA, 0, 219, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 59, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 62, 328, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 63, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 95, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 126, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0x4D, 1, 0x4D, 0x50, 8, 0, 73, 439, 0, 0 },
    { 1, 0, 0x40, 6, 0, 1, 0, 5, 4, 0, 299, 241, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag250_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20D, 0x150, 0x98, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x20D, 0x60, 0x150, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag250_funcs = { wstag250_setup };
