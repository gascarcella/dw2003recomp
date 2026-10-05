#include "wstag.h"

/* WSTAG720: stage 0x267 (fieldstg_stages). */

extern WstagFuncs wstag720_funcs;
extern FieldstgBattleLists wstag720_battle_lists;
extern FieldstgVramPlace wstag720_vram_places[];
extern FieldstgPlacedActor *wstag720_actors[];
extern FieldstgSprite wstag720_sprites[];
extern FieldstgMapEvent wstag720_map_events[];

void wstag720_update(WstagObject *obj) {
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

WstagObject *wstag720_start(void *arg0) {
    WstagObject *obj = object_new(wstag720_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag720_funcs.setup();
    return obj;
}

void wstag720_setup(void) {
    fieldstg_stage.background_file = 0x66D;
    fieldstg_stage.sprite_file = 0x066E0000;
    fieldstg_stage.sprites = wstag720_sprites;
    fieldstg_stage.map_events = wstag720_map_events;
    fieldstg_stage.mask_file = 0x66C;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDA00, 0x2E500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag720_vram_places;
    fieldstg_stage.music = 0x18;
    fieldstg_stage.sound = 0x60600000;
    fieldstg_stage.actors = wstag720_actors;
    fieldstg_stage.battle_lists = &wstag720_battle_lists;
    fieldstg_attr.set_file(0, 0x066E0001);
    fieldstg_attr.set_file(7, 0x066E0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag720_setup(void);

FieldstgListedBattle D_WSTAG720_800A5F78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5F84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5F90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5F9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5FA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5FB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5FC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A5FCC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG720_800A5FD8 = {
    0,
    { &D_WSTAG720_800A5F78, &D_WSTAG720_800A5F84, &D_WSTAG720_800A5F90, &D_WSTAG720_800A5F9C, &D_WSTAG720_800A5FA8,
        &D_WSTAG720_800A5FB4, &D_WSTAG720_800A5FC0, &D_WSTAG720_800A5FCC },
};
FieldstgListedBattle D_WSTAG720_800A5FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6050 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG720_800A605C = {
    0,
    { &D_WSTAG720_800A5FFC, &D_WSTAG720_800A6008, &D_WSTAG720_800A6014, &D_WSTAG720_800A6020, &D_WSTAG720_800A602C,
        &D_WSTAG720_800A6038, &D_WSTAG720_800A6044, &D_WSTAG720_800A6050 },
};
FieldstgListedBattle D_WSTAG720_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A60D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG720_800A60E0 = {
    0,
    { &D_WSTAG720_800A6080, &D_WSTAG720_800A608C, &D_WSTAG720_800A6098, &D_WSTAG720_800A60A4, &D_WSTAG720_800A60B0,
        &D_WSTAG720_800A60BC, &D_WSTAG720_800A60C8, &D_WSTAG720_800A60D4 },
};
FieldstgListedBattle D_WSTAG720_800A6104 = { 222, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG720_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG720_800A6158 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG720_800A6164 = {
    0,
    { &D_WSTAG720_800A6104, &D_WSTAG720_800A6110, &D_WSTAG720_800A611C, &D_WSTAG720_800A6128, &D_WSTAG720_800A6134,
        &D_WSTAG720_800A6140, &D_WSTAG720_800A614C, &D_WSTAG720_800A6158 },
};
FieldstgBattleLists wstag720_battle_lists = {
    159, 0, 0, { &D_WSTAG720_800A5FD8, &D_WSTAG720_800A605C, &D_WSTAG720_800A60E0 }, &D_WSTAG720_800A6164,
};
FieldstgVramPlace wstag720_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 426, 256, 424, 0, 352, 511 }, { 320, 256, 370, 393, 200, 137, 368, 511 },
    { 320, 256, 374, 313, 216, 57, 352, 510 }, { 320, 256, 374, 353, 216, 97, 368, 510 },
    { 320, 256, 370, 425, 200, 169, 352, 509 }, { 320, 256, 370, 457, 200, 201, 368, 509 },
    { 320, 256, 354, 466, 136, 210, 352, 508 }, { 320, 256, 362, 466, 168, 210, 368, 508 },
    { 384, 256, 418, 256, 392, 0, 352, 507 },
};
u16 D_WSTAG720_800A6294[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A629C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A62A4[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A62B0[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A62C0[4] = { 0x7620, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A62C8[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG720_800A62DC[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A62F4[6] = { 0x7400, 1, 0xE16, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6300[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A631C[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG720_800A6338[4] = { 0x7820, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6340[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6348[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6354[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A635C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6368[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6374[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A637C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6384[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6390[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A63A0[4] = { 0x7620, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A63A8[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG720_800A63BC[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A63D4[6] = { 0x7400, 1, 0xE16, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A63E0[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A63FC[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG720_800A6418[4] = { 0x7820, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6420[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A642C[4] = { 0x9405, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6434[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6440[4] = { 0x9406, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6448[4] = { 0x940D, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6450[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6458[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6464[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6474[4] = { 0x7620, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A647C[10] = {
    0x7206, 1, 0, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG720_800A6490[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A64A8[4] = { 0xE16, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A64B0[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG720_800A64CC[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE16, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG720_800A64E8[4] = { 0x7820, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG720_800A64F0[2] = { { NULL, NULL, 547 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6508[2] = { { NULL, NULL, 550 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6520[2] = { { NULL, NULL, 548 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6538[2] = { { NULL, NULL, 543 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6550[2] = { { NULL, NULL, 546 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6568[2] = { { NULL, NULL, 544 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6580[8] = {
    { D_WSTAG720_800A6294, D_WSTAG720_800A629C, 404 }, { D_WSTAG720_800A62A4, NULL, 408 },
    { D_WSTAG720_800A62B0, D_WSTAG720_800A62C0, 409 }, { D_WSTAG720_800A62C8, NULL, 410 },
    { D_WSTAG720_800A62DC, D_WSTAG720_800A62F4, 411 }, { D_WSTAG720_800A6300, NULL, 412 },
    { D_WSTAG720_800A631C, D_WSTAG720_800A6338, 413 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG720_800A65E0[2] = { { NULL, NULL, 665 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A65F8[4] = {
    { D_WSTAG720_800A6340, NULL, 404 }, { D_WSTAG720_800A6348, D_WSTAG720_800A6354, 414 },
    { D_WSTAG720_800A635C, D_WSTAG720_800A6368, 415 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG720_800A6628[8] = {
    { D_WSTAG720_800A6374, D_WSTAG720_800A637C, 405 }, { D_WSTAG720_800A6384, NULL, 408 },
    { D_WSTAG720_800A6390, D_WSTAG720_800A63A0, 409 }, { D_WSTAG720_800A63A8, NULL, 410 },
    { D_WSTAG720_800A63BC, D_WSTAG720_800A63D4, 411 }, { D_WSTAG720_800A63E0, NULL, 412 },
    { D_WSTAG720_800A63FC, D_WSTAG720_800A6418, 413 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG720_800A6688[3] = {
    { D_WSTAG720_800A6420, D_WSTAG720_800A642C, 778 }, { D_WSTAG720_800A6434, D_WSTAG720_800A6440, 778 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG720_800A66AC[2] = { { NULL, D_WSTAG720_800A6448, 778 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A66C4[8] = {
    { D_WSTAG720_800A6450, NULL, 406 }, { D_WSTAG720_800A6458, NULL, 406 },
    { D_WSTAG720_800A6464, D_WSTAG720_800A6474, 406 }, { D_WSTAG720_800A647C, NULL, 406 },
    { D_WSTAG720_800A6490, D_WSTAG720_800A64A8, 406 }, { D_WSTAG720_800A64B0, NULL, 406 },
    { D_WSTAG720_800A64CC, D_WSTAG720_800A64E8, 406 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG720_800A6724[2] = { { NULL, NULL, 549 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A673C[2] = { { NULL, NULL, 545 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6754[2] = { { NULL, NULL, 558 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A676C[2] = { { NULL, NULL, 557 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A6784[2] = { { NULL, NULL, 556 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A679C[2] = { { NULL, NULL, 555 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A67B4[2] = { { NULL, NULL, 551 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A67CC[2] = { { NULL, NULL, 554 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A67E4[2] = { { NULL, NULL, 552 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG720_800A67FC[2] = { { NULL, NULL, 553 }, { NULL, NULL, 0 } };
u16 D_WSTAG720_800A6814[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A681C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6824[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A682C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6834[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A683C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6844[8] = { 0x11, 0, 0x7004, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6854[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A6864[8] = { 0x11, 1, 0x7009, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6874[8] = { 0x8192, 1, 0x11, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6884[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG720_800A688C[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A6894[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A689C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68A4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68AC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68B4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68BC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68C4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68CC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68D4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68DC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG720_800A68E4[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG720_800A68EC = { D_WSTAG720_800A6814, D_WSTAG720_800A64F0, 45, 4, 537, 145, 3 };
FieldstgPlacedActor D_WSTAG720_800A6900 = { D_WSTAG720_800A681C, D_WSTAG720_800A6508, 45, 4, 120, 504, 7 };
FieldstgPlacedActor D_WSTAG720_800A6914 = { D_WSTAG720_800A6824, D_WSTAG720_800A6520, 45, 4, 537, 145, 3 };
FieldstgPlacedActor D_WSTAG720_800A6928 = { D_WSTAG720_800A682C, D_WSTAG720_800A6538, 46, 5, 119, 137, 7 };
FieldstgPlacedActor D_WSTAG720_800A693C = { D_WSTAG720_800A6834, D_WSTAG720_800A6550, 46, 5, 433, 567, 1 };
FieldstgPlacedActor D_WSTAG720_800A6950 = { D_WSTAG720_800A683C, D_WSTAG720_800A6568, 46, 5, 119, 137, 7 };
FieldstgPlacedActor D_WSTAG720_800A6964 = { D_WSTAG720_800A6844, D_WSTAG720_800A6580, 53, 6, 321, 217, 1 };
FieldstgPlacedActor D_WSTAG720_800A6978 = { D_WSTAG720_800A6854, D_WSTAG720_800A65E0, 53, 6, 321, 217, 1 };
FieldstgPlacedActor D_WSTAG720_800A698C = { D_WSTAG720_800A6864, D_WSTAG720_800A65F8, 53, 6, 321, 217, 1 };
FieldstgPlacedActor D_WSTAG720_800A69A0 = { D_WSTAG720_800A6874, D_WSTAG720_800A6628, 53, 6, 321, 217, 1 };
FieldstgPlacedActor D_WSTAG720_800A69B4 = { D_WSTAG720_800A6884, D_WSTAG720_800A6688, 136, 7, 124, 700, 7 };
FieldstgPlacedActor D_WSTAG720_800A69C8 = { D_WSTAG720_800A688C, D_WSTAG720_800A66AC, 136, 7, 124, 700, 7 };
FieldstgPlacedActor D_WSTAG720_800A69DC = { D_WSTAG720_800A6894, D_WSTAG720_800A66C4, 157, 8, 321, 217, 1 };
FieldstgPlacedActor D_WSTAG720_800A69F0 = { D_WSTAG720_800A689C, D_WSTAG720_800A6724, 158, 9, 537, 145, 3 };
FieldstgPlacedActor D_WSTAG720_800A6A04 = { D_WSTAG720_800A68A4, D_WSTAG720_800A673C, 159, 10, 119, 137, 7 };
FieldstgPlacedActor D_WSTAG720_800A6A18 = { D_WSTAG720_800A68AC, D_WSTAG720_800A6754, 180, 11, 118, 135, 5 };
FieldstgPlacedActor D_WSTAG720_800A6A2C = { D_WSTAG720_800A68B4, D_WSTAG720_800A676C, 180, 11, 369, 345, 1 };
FieldstgPlacedActor D_WSTAG720_800A6A40 = { D_WSTAG720_800A68BC, D_WSTAG720_800A6784, 180, 11, 369, 345, 1 };
FieldstgPlacedActor D_WSTAG720_800A6A54 = { D_WSTAG720_800A68C4, D_WSTAG720_800A679C, 180, 11, 369, 345, 1 };
FieldstgPlacedActor D_WSTAG720_800A6A68 = { D_WSTAG720_800A68CC, D_WSTAG720_800A67B4, 182, 12, 121, 505, 5 };
FieldstgPlacedActor D_WSTAG720_800A6A7C = { D_WSTAG720_800A68D4, D_WSTAG720_800A67CC, 182, 12, 272, 90, 1 };
FieldstgPlacedActor D_WSTAG720_800A6A90 = { D_WSTAG720_800A68DC, D_WSTAG720_800A67E4, 182, 12, 121, 505, 5 };
FieldstgPlacedActor D_WSTAG720_800A6AA4 = { D_WSTAG720_800A68E4, D_WSTAG720_800A67FC, 182, 12, 121, 505, 5 };
FieldstgPlacedActor *wstag720_actors[24] = {
    &D_WSTAG720_800A68EC, &D_WSTAG720_800A6900, &D_WSTAG720_800A6914, &D_WSTAG720_800A6928, &D_WSTAG720_800A693C,
    &D_WSTAG720_800A6950, &D_WSTAG720_800A6964, &D_WSTAG720_800A6978, &D_WSTAG720_800A698C, &D_WSTAG720_800A69A0,
    &D_WSTAG720_800A69B4, &D_WSTAG720_800A69C8, &D_WSTAG720_800A69DC, &D_WSTAG720_800A69F0, &D_WSTAG720_800A6A04,
    &D_WSTAG720_800A6A18, &D_WSTAG720_800A6A2C, &D_WSTAG720_800A6A40, &D_WSTAG720_800A6A54, &D_WSTAG720_800A6A68,
    &D_WSTAG720_800A6A7C, &D_WSTAG720_800A6A90, &D_WSTAG720_800A6AA4, NULL,
};
FieldstgSprite wstag720_sprites[53] = {
    { 1, 0, 0x76, 2, 2, 1, 2, 7, 4, 0, 15, 51, 0, 0 }, { 1, 0, 0x76, 2, 2, 1, 2, 7, 4, 0, 367, 644, 0, 0 },
    { 1, 0, 0x48, 2, 8, 0, 0, 0, 0, 0, 184, 716, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 106, 91, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 115, 78, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 120, 99, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 189, 348, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 192, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 209, 362, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 317, 142, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 326, 165, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 341, 120, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 361, 421, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 368, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 408, 678, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 421, 702, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 457, 290, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 468, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 502, 466, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 509, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 519, 66, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 522, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 550, 31, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 633, 234, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 703, 228, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 714, 256, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 103, 97, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 115, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 122, 106, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 188, 402, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 192, 354, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 207, 370, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 314, 148, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 328, 171, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 342, 125, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 360, 427, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 371, 449, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 409, 685, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 419, 707, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 454, 296, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 471, 276, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 505, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 506, 456, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 517, 71, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 523, 102, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 553, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 634, 239, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 702, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 717, 264, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 257, 598, 631, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 385, 439, 488, 0 }, { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 495, 232, 286, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag720_map_events[12] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x266, 0xC8, 0xAC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x1B0, 0xE8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x1A0, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x100, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0xF0, 0x198, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x161, 0x200, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x171, 0x258, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0x12, 0xA2, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0x12, 0xB2, 0x1D8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0x10, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag720_funcs = { wstag720_setup };
