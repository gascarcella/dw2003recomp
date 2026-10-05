#include "wstag.h"

/* WSTAG706: stage 0x2CC (fieldstg_stages). */

extern WstagFuncs wstag706_funcs;
extern FieldstgBattleLists wstag706_battle_lists;
extern FieldstgVramPlace wstag706_vram_places[];
extern FieldstgPlacedActor *wstag706_actors[];
extern FieldstgSprite wstag706_sprites[];
extern FieldstgMapEvent wstag706_map_events[];

void wstag706_update(WstagObject *obj) {
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

WstagObject *wstag706_start(void *arg0) {
    WstagObject *obj = object_new(wstag706_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag706_funcs.setup();
    return obj;
}

void wstag706_setup(void) {
    fieldstg_stage.background_file = 0x683;
    fieldstg_stage.sprite_file = 0x06840000;
    fieldstg_stage.sprites = wstag706_sprites;
    fieldstg_stage.map_events = wstag706_map_events;
    fieldstg_stage.mask_file = 0x682;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x4BE00, 0x8800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag706_vram_places;
    fieldstg_stage.music = 0x3E;
    fieldstg_stage.sound = 0x60F80000;
    fieldstg_stage.actors = wstag706_actors;
    fieldstg_stage.battle_lists = &wstag706_battle_lists;
    fieldstg_attr.set_file(0, 0x06840001);
    fieldstg_attr.set_file(7, 0x06840002);
    fieldstg_attr.set_file(4, 0x06840003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag706_setup(void);

FieldstgListedBattle D_WSTAG706_800A5F90 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5F9C = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FA8 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FB4 = { 184, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FC0 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FCC = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FD8 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A5FE4 = { 160, 14, 0x60080000 };
FieldstgBattleList D_WSTAG706_800A5FF0 = {
    2,
    { &D_WSTAG706_800A5F90, &D_WSTAG706_800A5F9C, &D_WSTAG706_800A5FA8, &D_WSTAG706_800A5FB4, &D_WSTAG706_800A5FC0,
        &D_WSTAG706_800A5FCC, &D_WSTAG706_800A5FD8, &D_WSTAG706_800A5FE4 },
};
FieldstgListedBattle D_WSTAG706_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG706_800A6074 = {
    0,
    { &D_WSTAG706_800A6014, &D_WSTAG706_800A6020, &D_WSTAG706_800A602C, &D_WSTAG706_800A6038, &D_WSTAG706_800A6044,
        &D_WSTAG706_800A6050, &D_WSTAG706_800A605C, &D_WSTAG706_800A6068 },
};
FieldstgListedBattle D_WSTAG706_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG706_800A60F8 = {
    0,
    { &D_WSTAG706_800A6098, &D_WSTAG706_800A60A4, &D_WSTAG706_800A60B0, &D_WSTAG706_800A60BC, &D_WSTAG706_800A60C8,
        &D_WSTAG706_800A60D4, &D_WSTAG706_800A60E0, &D_WSTAG706_800A60EC },
};
FieldstgListedBattle D_WSTAG706_800A611C = { 247, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A6128 = { 248, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG706_800A6134 = { 295, 14, 0x600C0000 };
FieldstgListedBattle D_WSTAG706_800A6140 = { 296, 14, 0x600C0000 };
FieldstgListedBattle D_WSTAG706_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG706_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG706_800A617C = {
    0,
    { &D_WSTAG706_800A611C, &D_WSTAG706_800A6128, &D_WSTAG706_800A6134, &D_WSTAG706_800A6140, &D_WSTAG706_800A614C,
        &D_WSTAG706_800A6158, &D_WSTAG706_800A6164, &D_WSTAG706_800A6170 },
};
FieldstgBattleLists wstag706_battle_lists = {
    114, 0, 0, { &D_WSTAG706_800A5FF0, &D_WSTAG706_800A6074, &D_WSTAG706_800A60F8 }, &D_WSTAG706_800A617C,
};
FieldstgVramPlace wstag706_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 362, 256, 168, 0, 320, 511 }, { 320, 256, 346, 256, 104, 0, 336, 511 },
    { 320, 256, 354, 256, 136, 0, 352, 511 },
};
u16 D_WSTAG706_800A624C[8] = { 0x7090, 1, 0x252, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A625C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6264[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A626C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6274[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A627C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6288[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6294[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A62A0[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A62AC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A62B4[6] = { 0, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A62C0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A62C8[8] = { 0x7208, 0, 0, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A62D8[10] = {
    0x720A, 0, 0x7208, 1, 0, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A62EC[4] = { 0x763F, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A62F4[12] = {
    0xE32, 0, 0x720A, 1, 0x7208, 1, 0, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A630C[6] = { 0x7400, 1, 0xE32, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6318[14] = {
    0x8014, 0, 0xE32, 1, 0x720A, 1, 0x7208, 1,
    0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A6334[16] = {
    0x720B, 0, 0x8014, 1, 0xE32, 1, 0x720A, 1,
    0x7208, 1, 0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A6354[16] = {
    0x720B, 1, 0x8014, 1, 0xE32, 1, 0x720A, 1,
    0x7208, 1, 0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A6374[4] = { 0x783F, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A637C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6384[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A638C[6] = { 0x720B, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6398[8] = { 0x720B, 1, 0, 1, 0xE53, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A63A8[6] = { 0xE53, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A63B4[10] = {
    0x720B, 1, 0, 1, 0xE53, 1, 0x720D, 0,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A63C8[10] = {
    0, 1, 0x720B, 1, 0xE53, 1, 0x720D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A63DC[4] = { 0x783F, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A63E4[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A63F0[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A63FC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6408[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6410[6] = { 1, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A641C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6424[8] = { 0x7208, 0, 1, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6434[10] = {
    0x720A, 0, 0x7208, 1, 1, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A6448[4] = { 0x7640, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6450[12] = {
    0xE33, 0, 0x720A, 1, 0x7208, 1, 1, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A6468[6] = { 0x7401, 1, 0xE33, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6474[14] = {
    0x8014, 0, 0xE33, 1, 0x720A, 1, 0x7208, 1,
    1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A6490[16] = {
    0x720B, 0, 0x8014, 1, 0xE33, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A64B0[16] = {
    0x720B, 1, 0x8014, 1, 0xE33, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG706_800A64D0[4] = { 0x7840, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A64D8[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A64E0[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A64E8[6] = { 1, 1, 0x720B, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A64F4[8] = { 1, 1, 0x720B, 1, 0xE54, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6504[6] = { 0x7401, 1, 0xE54, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6510[10] = {
    1, 1, 0x720B, 1, 0xE54, 1, 0x720D, 0,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A6524[10] = {
    1, 1, 0x720B, 1, 0xE54, 1, 0x720D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG706_800A6538[4] = { 0x7840, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6540[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6548[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6550[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A6558[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A6560[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG706_800A656C[2] = { { NULL, D_WSTAG706_800A624C, 837 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG706_800A6584[4] = {
    { D_WSTAG706_800A625C, NULL, 924 }, { D_WSTAG706_800A6264, D_WSTAG706_800A626C, 585 },
    { D_WSTAG706_800A6274, D_WSTAG706_800A627C, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A65B4[10] = {
    { D_WSTAG706_800A6288, D_WSTAG706_800A6294, 554 }, { D_WSTAG706_800A62A0, D_WSTAG706_800A62AC, 553 },
    { D_WSTAG706_800A62B4, D_WSTAG706_800A62C0, 548 }, { D_WSTAG706_800A62C8, NULL, 549 },
    { D_WSTAG706_800A62D8, D_WSTAG706_800A62EC, 550 }, { D_WSTAG706_800A62F4, D_WSTAG706_800A630C, 551 },
    { D_WSTAG706_800A6318, NULL, 552 }, { D_WSTAG706_800A6334, NULL, 549 },
    { D_WSTAG706_800A6354, D_WSTAG706_800A6374, 550 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A662C[6] = {
    { D_WSTAG706_800A637C, D_WSTAG706_800A6384, 588 }, { D_WSTAG706_800A638C, NULL, 590 },
    { D_WSTAG706_800A6398, D_WSTAG706_800A63A8, 589 }, { D_WSTAG706_800A63B4, NULL, 584 },
    { D_WSTAG706_800A63C8, D_WSTAG706_800A63DC, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A6674[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG706_800A668C[10] = {
    { D_WSTAG706_800A63E4, D_WSTAG706_800A63F0, 586 }, { D_WSTAG706_800A63FC, D_WSTAG706_800A6408, 585 },
    { D_WSTAG706_800A6410, D_WSTAG706_800A641C, 580 }, { D_WSTAG706_800A6424, NULL, 581 },
    { D_WSTAG706_800A6434, D_WSTAG706_800A6448, 582 }, { D_WSTAG706_800A6450, D_WSTAG706_800A6468, 583 },
    { D_WSTAG706_800A6474, NULL, 584 }, { D_WSTAG706_800A6490, NULL, 587 },
    { D_WSTAG706_800A64B0, D_WSTAG706_800A64D0, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A6704[6] = {
    { D_WSTAG706_800A64D8, D_WSTAG706_800A64E0, 556 }, { D_WSTAG706_800A64E8, NULL, 558 },
    { D_WSTAG706_800A64F4, D_WSTAG706_800A6504, 557 }, { D_WSTAG706_800A6510, NULL, 552 },
    { D_WSTAG706_800A6524, D_WSTAG706_800A6538, 559 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A674C[4] = {
    { D_WSTAG706_800A6540, NULL, 925 }, { D_WSTAG706_800A6548, D_WSTAG706_800A6550, 553 },
    { D_WSTAG706_800A6558, D_WSTAG706_800A6560, 554 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG706_800A677C[2] = { { NULL, NULL, 555 }, { NULL, NULL, 0 } };
u16 D_WSTAG706_800A6794[4] = { 0x252, 0, 0xFFFF, 0 };
u16 D_WSTAG706_800A679C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67A4[6] = { 0x8192, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67B0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67B8[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67C4[6] = { 0x7019, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67D0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67D8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG706_800A67E0[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG706_800A67EC = { D_WSTAG706_800A6794, D_WSTAG706_800A656C, 33, 4, 1281, 401, 1 };
FieldstgPlacedActor D_WSTAG706_800A6800 = { D_WSTAG706_800A679C, D_WSTAG706_800A6584, 69, 5, 561, 841, 1 };
FieldstgPlacedActor D_WSTAG706_800A6814 = { D_WSTAG706_800A67A4, D_WSTAG706_800A65B4, 69, 5, 561, 841, 1 };
FieldstgPlacedActor D_WSTAG706_800A6828 = { D_WSTAG706_800A67B0, D_WSTAG706_800A662C, 69, 5, 561, 841, 1 };
FieldstgPlacedActor D_WSTAG706_800A683C = { D_WSTAG706_800A67B8, D_WSTAG706_800A6674, 69, 5, 561, 841, 1 };
FieldstgPlacedActor D_WSTAG706_800A6850 = { D_WSTAG706_800A67C4, D_WSTAG706_800A668C, 70, 6, 992, 273, 7 };
FieldstgPlacedActor D_WSTAG706_800A6864 = { D_WSTAG706_800A67D0, D_WSTAG706_800A6704, 70, 6, 992, 273, 7 };
FieldstgPlacedActor D_WSTAG706_800A6878 = { D_WSTAG706_800A67D8, D_WSTAG706_800A674C, 70, 6, 992, 273, 7 };
FieldstgPlacedActor D_WSTAG706_800A688C = { D_WSTAG706_800A67E0, D_WSTAG706_800A677C, 70, 6, 992, 273, 7 };
FieldstgPlacedActor *wstag706_actors[10] = {
    &D_WSTAG706_800A67EC, &D_WSTAG706_800A6800, &D_WSTAG706_800A6814, &D_WSTAG706_800A6828, &D_WSTAG706_800A683C,
    &D_WSTAG706_800A6850, &D_WSTAG706_800A6864, &D_WSTAG706_800A6878, &D_WSTAG706_800A688C, NULL,
};
FieldstgSprite wstag706_sprites[5] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 834, 768, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 889, 248, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 998, 143, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 1192, 110, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag706_map_events[22] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D5, 0xE0, 0x3A8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CA, 0x3A0, 0xA0, 1, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x18, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xD, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 5, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xB, 2 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x420, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x410, 0x1D8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x460, 0x2A0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x450, 0x338, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x430, 0x288, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x420, 0x2F0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x36F, 0x1D8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x37F, 0x240, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x32F, 0x288, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x33F, 0x320, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1F0, 0x338, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1E0, 0x3A0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0xD, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0xA, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag706_funcs = { wstag706_setup };
