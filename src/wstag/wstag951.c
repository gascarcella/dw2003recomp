#include "wstag.h"

/* WSTAG951: stage 0x297 (fieldstg_stages_2d). */

extern WstagFuncs wstag951_funcs;
const CVECTOR wstag951_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgVramPlace wstag951_vram_places[];
extern FieldstgPlacedActor *wstag951_actors[];
extern FieldstgSprite wstag951_sprites[];
extern FieldstgMapEvent wstag951_map_events[];
extern FieldstgEventDef wstag951_events[];

void wstag951_update(WstagObject *obj) {
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

WstagObject *wstag951_start(void *arg0) {
    WstagObject *obj = object_new(wstag951_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag951_funcs.setup();
    return obj;
}

void wstag951_setup(void) {
    fieldstg_stage.background_file = 0x513;
    fieldstg_stage.sprite_file = 0x091D0000;
    fieldstg_stage.sprites = wstag951_sprites;
    fieldstg_stage.map_events = wstag951_map_events;
    fieldstg_stage.mask_file = 0x91C;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x3C500, 0x3D200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag951_vram_places;
    fieldstg_stage.music = 0xA;
    fieldstg_stage.sound = 0x60280000;
    fieldstg_stage.actors = wstag951_actors;
    fieldstg_stage.color = wstag951_color;
    fieldstg_stage.events = wstag951_events;
    fieldstg_attr.set_file(0, 0x091D0001);
    fieldstg_attr.set_file(7, 0x091D0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag951_setup(void);
extern s16 D_WSTAG951_800A75BC[];
extern s16 D_WSTAG951_800A7658[];
extern s16 D_WSTAG951_800A76F4[];
extern s16 D_WSTAG951_800A776C[];
extern s16 D_WSTAG951_800A77E4[];

FieldstgVramPlace wstag951_vram_places[21] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 428, 336, 432, 80, 368, 504 }, { 320, 256, 370, 435, 200, 179, 320, 503 },
    { 320, 256, 362, 453, 168, 197, 336, 503 }, { 384, 256, 428, 256, 432, 0, 368, 503 },
    { 384, 256, 436, 336, 464, 80, 320, 502 }, { 320, 256, 376, 256, 224, 0, 336, 502 },
    { 384, 256, 428, 368, 432, 112, 368, 502 }, { 384, 256, 436, 368, 464, 112, 320, 501 },
    { 384, 256, 384, 376, 256, 120, 336, 501 }, { 384, 256, 392, 376, 288, 120, 368, 501 },
    { 384, 256, 436, 256, 464, 0, 320, 500 }, { 384, 256, 428, 296, 432, 40, 336, 500 },
    { 384, 256, 436, 296, 464, 40, 368, 500 }, { 384, 256, 400, 376, 320, 120, 320, 499 },
    { 384, 256, 408, 376, 352, 120, 336, 499 },
};
u16 D_WSTAG951_800A60EC[4] = { 0x1011, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A60F4[8] = { 0x1011, 0, 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6104[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A610C[8] = { 0x1011, 0, 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A611C[4] = { 0x7848, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6124[8] = { 0x1011, 0, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6134[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6140[8] = { 0x1011, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6150[14] = {
    0x11, 0, 0x10, 0, 0, 0, 0x1011, 1,
    0x8024, 1, 0x7013, 1, 0xFFFF, 0,
};
u16 D_WSTAG951_800A616C[4] = { 0x1012, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6174[8] = { 0x1012, 0, 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6184[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A618C[8] = { 0x1012, 0, 0x11, 0, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A619C[4] = { 0x784F, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A61A4[8] = { 0x1012, 0, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A61B4[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A61C0[8] = { 0x1012, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A61D0[12] = {
    0x11, 0, 0x10, 0, 1, 0, 0x1012, 1,
    0x9078, 1, 0xFFFF, 0,
};
u16 D_WSTAG951_800A61E8[4] = { 0x1013, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A61F0[8] = { 0x1013, 0, 0x11, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6200[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6208[8] = { 0x1013, 0, 0x11, 0, 2, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6218[4] = { 0x784C, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6220[8] = { 0x1013, 0, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6230[6] = { 0x11, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A623C[8] = { 0x1013, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A624C[12] = {
    0x11, 0, 0x10, 0, 2, 0, 0x1013, 1,
    0x9077, 1, 0xFFFF, 0,
};
u16 D_WSTAG951_800A6264[4] = { 0x1014, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A626C[8] = { 0x1014, 0, 0x11, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A627C[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6284[8] = { 0x1014, 0, 0x11, 0, 3, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6294[4] = { 0x7850, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A629C[8] = { 0x1014, 0, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A62AC[6] = { 0x11, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A62B8[8] = { 0x1014, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A62C8[12] = {
    0x11, 0, 0x10, 0, 3, 0, 0x1014, 1,
    0x9076, 1, 0xFFFF, 0,
};
u16 D_WSTAG951_800A62E0[4] = { 0x1015, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A62E8[8] = { 0x1015, 0, 0x11, 0, 4, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A62F8[4] = { 4, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6300[8] = { 0x1015, 0, 0x11, 0, 4, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6310[4] = { 0x784A, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6318[8] = { 0x1015, 0, 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6328[6] = { 0x11, 0, 4, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6334[8] = { 0x1015, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6344[12] = {
    0x11, 0, 0x10, 0, 4, 0, 0x1015, 1,
    0x9075, 1, 0xFFFF, 0,
};
u16 D_WSTAG951_800A635C[4] = { 0x1016, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6364[8] = { 0x1016, 0, 0x11, 0, 5, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6374[4] = { 5, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A637C[8] = { 0x1016, 0, 0x11, 0, 5, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A638C[4] = { 0x784E, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6394[8] = { 0x11, 1, 0x10, 0, 0x1016, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A63A4[6] = { 0x11, 0, 5, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A63B0[8] = { 0x1016, 0, 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A63C0[12] = {
    0x11, 0, 0x10, 0, 5, 0, 0x1016, 1,
    0x9074, 1, 0xFFFF, 0,
};
FieldstgTalk D_WSTAG951_800A63D8[2] = { { NULL, NULL, 90 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A63F0[2] = { { NULL, NULL, 110 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A6408[2] = { { NULL, NULL, 105 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A6420[2] = { { NULL, NULL, 100 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A6438[2] = { { NULL, NULL, 95 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A6450[6] = {
    { D_WSTAG951_800A60EC, NULL, 85 }, { D_WSTAG951_800A60F4, D_WSTAG951_800A6104, 81 },
    { D_WSTAG951_800A610C, D_WSTAG951_800A611C, 82 }, { D_WSTAG951_800A6124, D_WSTAG951_800A6134, 83 },
    { D_WSTAG951_800A6140, D_WSTAG951_800A6150, 84 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A6498[2] = { { NULL, NULL, 138 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A64B0[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A64C8[6] = {
    { D_WSTAG951_800A616C, NULL, 90 }, { D_WSTAG951_800A6174, D_WSTAG951_800A6184, 86 },
    { D_WSTAG951_800A618C, D_WSTAG951_800A619C, 87 }, { D_WSTAG951_800A61A4, D_WSTAG951_800A61B4, 88 },
    { D_WSTAG951_800A61C0, D_WSTAG951_800A61D0, 89 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A6510[6] = {
    { D_WSTAG951_800A61E8, NULL, 95 }, { D_WSTAG951_800A61F0, D_WSTAG951_800A6200, 91 },
    { D_WSTAG951_800A6208, D_WSTAG951_800A6218, 92 }, { D_WSTAG951_800A6220, D_WSTAG951_800A6230, 93 },
    { D_WSTAG951_800A623C, D_WSTAG951_800A624C, 94 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A6558[6] = {
    { D_WSTAG951_800A6264, NULL, 100 }, { D_WSTAG951_800A626C, D_WSTAG951_800A627C, 96 },
    { D_WSTAG951_800A6284, D_WSTAG951_800A6294, 97 }, { D_WSTAG951_800A629C, D_WSTAG951_800A62AC, 98 },
    { D_WSTAG951_800A62B8, D_WSTAG951_800A62C8, 99 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A65A0[6] = {
    { D_WSTAG951_800A62E0, NULL, 105 }, { D_WSTAG951_800A62E8, D_WSTAG951_800A62F8, 101 },
    { D_WSTAG951_800A6300, D_WSTAG951_800A6310, 102 }, { D_WSTAG951_800A6318, D_WSTAG951_800A6328, 103 },
    { D_WSTAG951_800A6334, D_WSTAG951_800A6344, 104 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A65E8[6] = {
    { D_WSTAG951_800A635C, NULL, 110 }, { D_WSTAG951_800A6364, D_WSTAG951_800A6374, 106 },
    { D_WSTAG951_800A637C, D_WSTAG951_800A638C, 107 }, { D_WSTAG951_800A6394, D_WSTAG951_800A63A4, 108 },
    { D_WSTAG951_800A63B0, D_WSTAG951_800A63C0, 109 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG951_800A6630[2] = { { NULL, NULL, 136 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG951_800A6648[2] = { { NULL, NULL, 139 }, { NULL, NULL, 0 } };
u16 D_WSTAG951_800A6660[4] = { 0x1012, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6668[4] = { 0x1016, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6670[4] = { 0x1015, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6678[4] = { 0x1014, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6680[4] = { 0x1013, 1, 0xFFFF, 0 };
u16 D_WSTAG951_800A6688[4] = { 0x1012, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6690[4] = { 0x1013, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A6698[4] = { 0x1014, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A66A0[4] = { 0x1015, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A66A8[4] = { 0x1016, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A66B0[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG951_800A66B8[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG951_800A66C0 = { D_WSTAG951_800A6660, D_WSTAG951_800A63D8, 46, 4, 260, 247, 1 };
FieldstgPlacedActor D_WSTAG951_800A66D4 = { D_WSTAG951_800A6668, D_WSTAG951_800A63F0, 50, 5, 504, 496, 1 };
FieldstgPlacedActor D_WSTAG951_800A66E8 = { D_WSTAG951_800A6670, D_WSTAG951_800A6408, 52, 6, 321, 485, 7 };
FieldstgPlacedActor D_WSTAG951_800A66FC = { D_WSTAG951_800A6678, D_WSTAG951_800A6420, 53, 7, 204, 462, 7 };
FieldstgPlacedActor D_WSTAG951_800A6710 = { D_WSTAG951_800A6680, D_WSTAG951_800A6438, 57, 8, 341, 349, 1 };
FieldstgPlacedActor D_WSTAG951_800A6724 = { NULL, D_WSTAG951_800A6450, 102, 9, 159, 177, 7 };
FieldstgPlacedActor D_WSTAG951_800A6738 = { NULL, D_WSTAG951_800A6498, 367, 10, 929, 417, 1 };
FieldstgPlacedActor D_WSTAG951_800A674C = { NULL, D_WSTAG951_800A64B0, 375, 11, 769, 209, 3 };
FieldstgPlacedActor D_WSTAG951_800A6760 = { D_WSTAG951_800A6688, D_WSTAG951_800A64C8, 399, 12, 216, 269, 7 };
FieldstgPlacedActor D_WSTAG951_800A6774 = { D_WSTAG951_800A6690, D_WSTAG951_800A6510, 400, 13, 297, 373, 7 };
FieldstgPlacedActor D_WSTAG951_800A6788 = { D_WSTAG951_800A6698, D_WSTAG951_800A6558, 401, 14, 272, 497, 7 };
FieldstgPlacedActor D_WSTAG951_800A679C = { D_WSTAG951_800A66A0, D_WSTAG951_800A65A0, 402, 15, 375, 493, 7 };
FieldstgPlacedActor D_WSTAG951_800A67B0 = { D_WSTAG951_800A66A8, D_WSTAG951_800A65E8, 403, 16, 480, 521, 7 };
FieldstgPlacedActor D_WSTAG951_800A67C4 = { D_WSTAG951_800A66B0, D_WSTAG951_800A6630, 404, 17, 501, 549, 7 };
FieldstgPlacedActor D_WSTAG951_800A67D8 = { D_WSTAG951_800A66B8, D_WSTAG951_800A6648, 405, 18, 522, 538, 7 };
FieldstgPlacedActor *wstag951_actors[16] = {
    &D_WSTAG951_800A66C0, &D_WSTAG951_800A66D4, &D_WSTAG951_800A66E8, &D_WSTAG951_800A66FC, &D_WSTAG951_800A6710,
    &D_WSTAG951_800A6724, &D_WSTAG951_800A6738, &D_WSTAG951_800A674C, &D_WSTAG951_800A6760, &D_WSTAG951_800A6774,
    &D_WSTAG951_800A6788, &D_WSTAG951_800A679C, &D_WSTAG951_800A67B0, &D_WSTAG951_800A67C4, &D_WSTAG951_800A67D8,
    NULL,
};
FieldstgSprite wstag951_sprites[170] = {
    { 1, 0, 0x40, 2, 0x47, 2, 0, 5, 8, 0, 85, 81, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 83, 117, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 83, 135, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 91, 113, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 91, 131, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 99, 109, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 99, 127, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 107, 105, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 107, 123, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 115, 101, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 115, 119, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 123, 97, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 123, 115, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 131, 93, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 131, 111, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 139, 89, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 139, 107, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 147, 85, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 147, 103, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 155, 81, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 155, 99, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 163, 77, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 5, 8, 0, 163, 95, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 87, 115, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 87, 133, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 95, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 95, 129, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 103, 107, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 103, 125, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 111, 103, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 111, 121, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 119, 99, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 119, 117, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 127, 95, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 127, 113, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 135, 91, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 135, 109, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 143, 87, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 143, 105, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 151, 83, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 151, 101, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 159, 79, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 159, 97, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 167, 75, 0, 0 },
    { 1, 0, 0x40, 2, 0x49, 2, 0, 5, 8, 0, 167, 93, 0, 0 }, { 1, 0, 0x40, 2, 0x4A, 2, 0, 7, 4, 0, 103, 201, 0, 0 },
    { 1, 0, 0x40, 2, 0x4B, 2, 0, 7, 4, 0, 105, 205, 0, 0 }, { 1, 0, 0x80, 2, 1, 0, 0, 0, 0, 0, 768, 128, 0, 0 },
    { 1, 0, 0x45, 2, 2, 0, 0, 0, 0, 0, 880, 156, 0, 0 }, { 1, 0, 0x78, 2, 3, 0, 0, 0, 0, 0, 896, 256, 0, 0 },
    { 1, 0, 0x55, 2, 4, 0, 0, 0, 0, 0, 984, 299, 0, 0 }, { 1, 0, 0x33, 2, 5, 0, 0, 0, 0, 0, 278, 813, 0, 0 },
    { 1, 0, 0x1E, 2, 6, 0, 0, 0, 0, 0, 323, 829, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 7, 4, 0, 220, 180, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 7, 4, 0, 232, 138, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 7, 4, 0, 297, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 7, 4, 0, 300, 284, 0, 0 }, { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 168, 143, 0, 0 },
    { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 220, 184, 0, 0 }, { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 234, 143, 0, 0 },
    { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 235, 299, 0, 0 }, { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 280, 241, 0, 0 },
    { 1, 0, 0x40, 6, 0x4B, 2, 0, 7, 4, 0, 300, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 203, 909, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 236, 1022, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 237, 973, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 238, 733, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 273, 861, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 309, 635, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 320, 858, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 396, 960, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 575, 884, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 630, 1071, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 769, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1138, 909, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 281, 1085, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 320, 983, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 359, 980, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 503, 793, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 657, 718, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 95, 790, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 117, 1016, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 145, 858, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 178, 1068, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 185, 1000, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 212, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 288, 998, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 302, 1076, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 391, 819, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 541, 902, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 773, 705, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 800, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 850, 469, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 967, 574, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 101, 951, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 275, 890, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 461, 890, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 509, 339, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 534, 794, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 600, 441, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 683, 709, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 715, 508, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 874, 539, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 917, 557, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 131, 1058, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 273, 953, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 19, 963, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 106, 1043, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 122, 952, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 249, 923, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 367, 919, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 545, 804, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 582, 430, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 742, 512, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 931, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1086, 533, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 56, 960, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 117, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 158, 846, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 171, 949, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 216, 834, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 221, 1050, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 227, 1041, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 235, 750, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 280, 713, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 286, 1071, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 287, 1094, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 305, 665, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 332, 909, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 333, 845, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 363, 326, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 378, 309, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 380, 320, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 390, 431, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 391, 981, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 398, 440, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 412, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 430, 770, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 470, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 474, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 499, 344, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 515, 599, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 517, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 526, 345, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 534, 910, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 546, 377, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 591, 852, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 616, 1076, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 622, 1085, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 633, 1077, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 694, 498, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 706, 727, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 775, 559, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 785, 551, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 793, 661, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 797, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 798, 492, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 838, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 916, 780, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 922, 772, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 929, 778, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1026, 1107, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1032, 1114, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1039, 1107, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1047, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1066, 898, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1149, 890, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1161, 892, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 477, 577, 626, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag951_map_events[12] = {
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xC0, 0xC2, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xCF, 0x106, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x110, 0x12B, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x11F, 0x16E, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x100, 0x192, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xF1, 0x1D6, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x330, 0xEA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x33F, 0x12E, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x2DE, 0x141, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x2CE, 0x186, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 3, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag951_funcs = { wstag951_setup };
FieldstgEventDef wstag951_events[6] = {
    { 1620, D_WSTAG951_800A75BC, 0x0158000A, NULL, NULL }, { 1622, D_WSTAG951_800A7658, 0x0158000B, NULL, NULL },
    { 1624, D_WSTAG951_800A76F4, 0x0158000C, NULL, NULL }, { 1626, D_WSTAG951_800A776C, 0x0158000D, NULL, NULL },
    { 1628, D_WSTAG951_800A77E4, 0x0158000E, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
s16 D_WSTAG951_800A75BC[78] = {
    FIELDSTG_EVENT_WALK(2, 503, 532, 3),
    FIELDSTG_EVENT_PLACE(403, 480, 521),
    FIELDSTG_EVENT_ANIM(403, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(403, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(403, 469, 514, 3),
    FIELDSTG_EVENT_WAIT_WALK(403),
    FIELDSTG_EVENT_ANIM(403, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(403, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(403, 504, 496, 5),
    FIELDSTG_EVENT_WAIT_WALK(403),
    FIELDSTG_EVENT_ANIM(403, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(403, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 403, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG951_800A7658[78] = {
    FIELDSTG_EVENT_WALK(2, 401, 481, 1),
    FIELDSTG_EVENT_PLACE(402, 375, 493),
    FIELDSTG_EVENT_ANIM(402, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(402, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(402, 357, 501, 1),
    FIELDSTG_EVENT_WAIT_WALK(402),
    FIELDSTG_EVENT_ANIM(402, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(402, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(402, 321, 485, 3),
    FIELDSTG_EVENT_WAIT_WALK(402),
    FIELDSTG_EVENT_ANIM(402, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(402, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 402, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG951_800A76F4[59] = {
    FIELDSTG_EVENT_WALK(2, 296, 508, 3),
    FIELDSTG_EVENT_PLACE(401, 272, 497),
    FIELDSTG_EVENT_ANIM(401, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(401, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(401, 204, 462, 3),
    FIELDSTG_EVENT_WAIT_WALK(401),
    FIELDSTG_EVENT_ANIM(401, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(401, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 401, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG951_800A776C[59] = {
    FIELDSTG_EVENT_WALK(2, 320, 384, 3),
    FIELDSTG_EVENT_PLACE(400, 297, 373),
    FIELDSTG_EVENT_ANIM(400, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(400, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(400, 341, 349, 5),
    FIELDSTG_EVENT_WAIT_WALK(400),
    FIELDSTG_EVENT_ANIM(400, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(400, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 400, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG951_800A77E4[59] = {
    FIELDSTG_EVENT_WALK(2, 240, 280, 3),
    FIELDSTG_EVENT_PLACE(399, 216, 269),
    FIELDSTG_EVENT_ANIM(399, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(399, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(399, 260, 247, 5),
    FIELDSTG_EVENT_WAIT_WALK(399),
    FIELDSTG_EVENT_ANIM(399, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(399, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 399, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
