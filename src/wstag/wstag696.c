#include "wstag.h"

/* WSTAG696: stage 0x2CA (fieldstg_stages). */

extern WstagFuncs wstag696_funcs;
extern FieldstgBattleLists wstag696_battle_lists;
extern FieldstgVramPlace wstag696_vram_places[];
extern FieldstgPlacedActor *wstag696_actors[];
extern FieldstgSprite wstag696_sprites[];
extern FieldstgMapEvent wstag696_map_events[];

void wstag696_update(WstagObject *obj) {
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

WstagObject *wstag696_start(void *arg0) {
    WstagObject *obj = object_new(wstag696_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag696_funcs.setup();
    return obj;
}

void wstag696_setup(void) {
    fieldstg_stage.background_file = 0x602;
    fieldstg_stage.sprite_file = 0x06030000;
    fieldstg_stage.sprites = wstag696_sprites;
    fieldstg_stage.map_events = wstag696_map_events;
    fieldstg_stage.mask_file = 0x601;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xBA00, 0x24900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag696_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag696_actors;
    fieldstg_stage.battle_lists = &wstag696_battle_lists;
    fieldstg_attr.set_file(0, 0x06030001);
    fieldstg_attr.set_file(7, 0x06030002);
    fieldstg_attr.set_file(4, 0x06030003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag696_setup(void);

FieldstgListedBattle D_WSTAG696_800A5F90 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5F9C = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FA8 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FB4 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FC0 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FCC = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FD8 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A5FE4 = { 160, 14, 0x60080000 };
FieldstgBattleList D_WSTAG696_800A5FF0 = {
    3,
    { &D_WSTAG696_800A5F90, &D_WSTAG696_800A5F9C, &D_WSTAG696_800A5FA8, &D_WSTAG696_800A5FB4, &D_WSTAG696_800A5FC0,
        &D_WSTAG696_800A5FCC, &D_WSTAG696_800A5FD8, &D_WSTAG696_800A5FE4 },
};
FieldstgListedBattle D_WSTAG696_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG696_800A6074 = {
    0,
    { &D_WSTAG696_800A6014, &D_WSTAG696_800A6020, &D_WSTAG696_800A602C, &D_WSTAG696_800A6038, &D_WSTAG696_800A6044,
        &D_WSTAG696_800A6050, &D_WSTAG696_800A605C, &D_WSTAG696_800A6068 },
};
FieldstgListedBattle D_WSTAG696_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG696_800A60F8 = {
    0,
    { &D_WSTAG696_800A6098, &D_WSTAG696_800A60A4, &D_WSTAG696_800A60B0, &D_WSTAG696_800A60BC, &D_WSTAG696_800A60C8,
        &D_WSTAG696_800A60D4, &D_WSTAG696_800A60E0, &D_WSTAG696_800A60EC },
};
FieldstgListedBattle D_WSTAG696_800A611C = { 245, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A6128 = { 246, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A6134 = { 293, 14, 0x600C0000 };
FieldstgListedBattle D_WSTAG696_800A6140 = { 333, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG696_800A6164 = { 187, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG696_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG696_800A617C = {
    0,
    { &D_WSTAG696_800A611C, &D_WSTAG696_800A6128, &D_WSTAG696_800A6134, &D_WSTAG696_800A6140, &D_WSTAG696_800A614C,
        &D_WSTAG696_800A6158, &D_WSTAG696_800A6164, &D_WSTAG696_800A6170 },
};
FieldstgBattleLists wstag696_battle_lists = {
    115, 0, 0, { &D_WSTAG696_800A5FF0, &D_WSTAG696_800A6074, &D_WSTAG696_800A60F8 }, &D_WSTAG696_800A617C,
};
FieldstgVramPlace wstag696_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 332, 456, 48, 200, 368, 510 }, { 320, 256, 340, 456, 80, 200, 368, 509 },
    { 384, 256, 384, 256, 256, 0, 368, 508 }, { 384, 256, 392, 256, 288, 0, 352, 507 },
    { 384, 256, 400, 256, 320, 0, 368, 507 }, { 384, 256, 408, 256, 352, 0, 352, 506 },
};
u16 D_WSTAG696_800A627C[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6284[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A628C[6] = { 1, 1, 0x720B, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6298[8] = { 1, 1, 0x720B, 1, 0xE51, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A62A8[6] = { 0xE51, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A62B4[10] = {
    1, 1, 0x720B, 1, 0xE51, 1, 0x720D, 0,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A62C8[10] = {
    1, 1, 0x720B, 1, 0xE51, 1, 0x720D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A62DC[4] = { 0x783E, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A62E4[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A62F0[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A62FC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6308[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6310[6] = { 0, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A631C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6324[8] = { 0x7208, 0, 0, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6334[10] = {
    0x7208, 1, 0, 1, 0x11, 0, 0x720A, 0,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A6348[4] = { 0x763E, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6350[12] = {
    0xE31, 0, 0x720A, 1, 0x7208, 1, 0, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A6368[6] = { 0x7401, 1, 0xE31, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6374[14] = {
    0x8014, 0, 0xE31, 1, 0x720A, 1, 0x7208, 1,
    0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A6390[16] = {
    0xE31, 1, 0x720B, 0, 0x8014, 1, 0x720A, 1,
    0x7208, 1, 0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A63B0[16] = {
    0, 1, 0x11, 0, 0x720A, 1, 0x720B, 1,
    0x8014, 1, 0xE31, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG696_800A63D0[4] = { 0x783E, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A63D8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A63E0[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A63E8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A63F0[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A63F8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6404[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A640C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6414[6] = { 0, 1, 0x720B, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6420[8] = { 0, 1, 0x720B, 1, 0xE52, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6430[6] = { 0xE52, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A643C[10] = {
    0, 1, 0x720B, 1, 0xE52, 1, 0x720D, 0,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A6450[10] = {
    0, 1, 0x720B, 1, 0xE52, 1, 0x720D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A6464[4] = { 0x783D, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A646C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6478[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6484[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6490[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6498[6] = { 1, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A64A4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A64AC[8] = { 0x7208, 0, 1, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A64BC[10] = {
    0x720A, 0, 0x7208, 1, 1, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG696_800A64D0[4] = { 0x763D, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A64D8[12] = {
    0xE30, 0, 0x720A, 1, 0x7208, 1, 1, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A64F0[6] = { 0x7400, 1, 0xE30, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A64FC[14] = {
    0x8014, 0, 0xE30, 1, 0x720A, 1, 0x7208, 1,
    1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A6518[16] = {
    0x720B, 0, 0x8014, 1, 0xE30, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A6538[16] = {
    0x720B, 1, 0x8014, 1, 0xE30, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG696_800A6558[4] = { 0x783D, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6560[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6568[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6570[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6578[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6580[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG696_800A658C[2] = { { NULL, NULL, 612 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A65A4[2] = { { NULL, NULL, 614 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A65BC[6] = {
    { D_WSTAG696_800A627C, D_WSTAG696_800A6284, 588 }, { D_WSTAG696_800A628C, NULL, 590 },
    { D_WSTAG696_800A6298, D_WSTAG696_800A62A8, 589 }, { D_WSTAG696_800A62B4, NULL, 584 },
    { D_WSTAG696_800A62C8, D_WSTAG696_800A62DC, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A6604[10] = {
    { D_WSTAG696_800A62E4, D_WSTAG696_800A62F0, 586 }, { D_WSTAG696_800A62FC, D_WSTAG696_800A6308, 585 },
    { D_WSTAG696_800A6310, D_WSTAG696_800A631C, 580 }, { D_WSTAG696_800A6324, NULL, 581 },
    { D_WSTAG696_800A6334, D_WSTAG696_800A6348, 582 }, { D_WSTAG696_800A6350, D_WSTAG696_800A6368, 583 },
    { D_WSTAG696_800A6374, NULL, 584 }, { D_WSTAG696_800A6390, NULL, 587 },
    { D_WSTAG696_800A63B0, D_WSTAG696_800A63D0, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A667C[4] = {
    { D_WSTAG696_800A63D8, NULL, 923 }, { D_WSTAG696_800A63E0, D_WSTAG696_800A63E8, 585 },
    { D_WSTAG696_800A63F0, D_WSTAG696_800A63F8, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A66AC[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A66C4[6] = {
    { D_WSTAG696_800A6404, D_WSTAG696_800A640C, 556 }, { D_WSTAG696_800A6414, NULL, 558 },
    { D_WSTAG696_800A6420, D_WSTAG696_800A6430, 557 }, { D_WSTAG696_800A643C, NULL, 552 },
    { D_WSTAG696_800A6450, D_WSTAG696_800A6464, 559 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A670C[10] = {
    { D_WSTAG696_800A646C, D_WSTAG696_800A6478, 554 }, { D_WSTAG696_800A6484, D_WSTAG696_800A6490, 553 },
    { D_WSTAG696_800A6498, D_WSTAG696_800A64A4, 548 }, { D_WSTAG696_800A64AC, NULL, 549 },
    { D_WSTAG696_800A64BC, D_WSTAG696_800A64D0, 550 }, { D_WSTAG696_800A64D8, D_WSTAG696_800A64F0, 551 },
    { D_WSTAG696_800A64FC, NULL, 552 }, { D_WSTAG696_800A6518, NULL, 556 },
    { D_WSTAG696_800A6538, D_WSTAG696_800A6558, 550 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A6784[4] = {
    { D_WSTAG696_800A6560, NULL, 925 }, { D_WSTAG696_800A6568, D_WSTAG696_800A6570, 553 },
    { D_WSTAG696_800A6578, D_WSTAG696_800A6580, 554 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG696_800A67B4[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A67CC[2] = { { NULL, NULL, 615 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A67E4[2] = { { NULL, NULL, 615 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A67FC[2] = { { NULL, NULL, 613 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG696_800A6814[2] = { { NULL, NULL, 613 }, { NULL, NULL, 0 } };
u16 D_WSTAG696_800A682C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6838[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6844[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A684C[6] = { 0x8192, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6858[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6860[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A686C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6874[6] = { 0x7019, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6880[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A6888[6] = { 0x7019, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A6894[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A68A0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG696_800A68A8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG696_800A68B4[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG696_800A68BC = { D_WSTAG696_800A682C, D_WSTAG696_800A658C, 49, 4, 656, 409, 1 };
FieldstgPlacedActor D_WSTAG696_800A68D0 = { D_WSTAG696_800A6838, D_WSTAG696_800A65A4, 55, 5, 272, 825, 3 };
FieldstgPlacedActor D_WSTAG696_800A68E4 = { D_WSTAG696_800A6844, D_WSTAG696_800A65BC, 69, 6, 721, 569, 1 };
FieldstgPlacedActor D_WSTAG696_800A68F8 = { D_WSTAG696_800A684C, D_WSTAG696_800A6604, 69, 6, 721, 569, 1 };
FieldstgPlacedActor D_WSTAG696_800A690C = { D_WSTAG696_800A6858, D_WSTAG696_800A667C, 69, 6, 721, 569, 1 };
FieldstgPlacedActor D_WSTAG696_800A6920 = { D_WSTAG696_800A6860, D_WSTAG696_800A66AC, 69, 6, 721, 569, 1 };
FieldstgPlacedActor D_WSTAG696_800A6934 = { D_WSTAG696_800A686C, D_WSTAG696_800A66C4, 70, 7, 353, 320, 7 };
FieldstgPlacedActor D_WSTAG696_800A6948 = { D_WSTAG696_800A6874, D_WSTAG696_800A670C, 70, 7, 353, 320, 7 };
FieldstgPlacedActor D_WSTAG696_800A695C = { D_WSTAG696_800A6880, D_WSTAG696_800A6784, 70, 7, 353, 320, 7 };
FieldstgPlacedActor D_WSTAG696_800A6970 = { D_WSTAG696_800A6888, D_WSTAG696_800A67B4, 70, 7, 353, 320, 7 };
FieldstgPlacedActor D_WSTAG696_800A6984 = { D_WSTAG696_800A6894, D_WSTAG696_800A67CC, 157, 8, 272, 825, 3 };
FieldstgPlacedActor D_WSTAG696_800A6998 = { D_WSTAG696_800A68A0, D_WSTAG696_800A67E4, 157, 8, 272, 825, 3 };
FieldstgPlacedActor D_WSTAG696_800A69AC = { D_WSTAG696_800A68A8, D_WSTAG696_800A67FC, 158, 9, 656, 409, 1 };
FieldstgPlacedActor D_WSTAG696_800A69C0 = { D_WSTAG696_800A68B4, D_WSTAG696_800A6814, 158, 9, 656, 409, 1 };
FieldstgPlacedActor *wstag696_actors[15] = {
    &D_WSTAG696_800A68BC, &D_WSTAG696_800A68D0, &D_WSTAG696_800A68E4, &D_WSTAG696_800A68F8, &D_WSTAG696_800A690C,
    &D_WSTAG696_800A6920, &D_WSTAG696_800A6934, &D_WSTAG696_800A6948, &D_WSTAG696_800A695C, &D_WSTAG696_800A6970,
    &D_WSTAG696_800A6984, &D_WSTAG696_800A6998, &D_WSTAG696_800A69AC, &D_WSTAG696_800A69C0, NULL,
};
FieldstgSprite wstag696_sprites[14] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 6, 0, 472, 329, 0, 0 },
    { 1, 0, 0xC8, 2, 0x34, 1, 0x34, 0x37, 8, 0, 542, 48, 0, 0 }, { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 90, 310, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 584, 768, 0, 0 }, { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 1032, 512, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xD, 6, 0, 579, 336, 0, 0 },
    { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 546, 340, 0, 0 }, { 1, 0, 0x48, 4, 0, 0, 0, 0, 0, 0, 517, 329, 393, 0 },
    { 1, 0, 0x53, 4, 8, 0, 0, 0, 0, 0, 448, 290, 368, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 783, 783, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 263, 263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 199, 199, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag696_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C9, 0x500, 0x290, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C9, 0x500, 0x300, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CC, 0x238, 0x444, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CB, 0x108, 0x12C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1CE, 0xAA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1BF, 0x112, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x3B0, 0x16A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x3BF, 0x1D0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x380, 0x253, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x38F, 0x2EA, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x14, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 7, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag696_funcs = { wstag696_setup };
