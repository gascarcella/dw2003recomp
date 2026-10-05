#include "wstag.h"

/* WSTAG351: stage 0x290 (fieldstg_stages). */

extern WstagFuncs wstag351_funcs;
extern FieldstgBattleLists wstag351_battle_lists;
extern FieldstgVramPlace wstag351_vram_places[];
extern FieldstgPlacedActor *wstag351_actors[];
extern FieldstgSprite wstag351_sprites[];
extern FieldstgMapEvent wstag351_map_events[];
extern FieldstgEventDef wstag351_events[];

void wstag351_update(WstagObject *obj) {
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

WstagObject *wstag351_start(void *arg0) {
    WstagObject *obj = object_new(wstag351_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag351_funcs.setup();
    return obj;
}

void wstag351_setup(void) {
    fieldstg_stage.background_file = 0x6A7;
    fieldstg_stage.sprite_file = 0x06A80000;
    fieldstg_stage.sprites = wstag351_sprites;
    fieldstg_stage.map_events = wstag351_map_events;
    fieldstg_stage.mask_file = 0x6A6;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2F400, 0x9C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag351_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag351_actors;
    fieldstg_stage.events = wstag351_events;
    fieldstg_stage.battle_lists = &wstag351_battle_lists;
    fieldstg_attr.set_file(0, 0x06A80001);
    fieldstg_attr.set_file(7, 0x06A80002);
    fieldstg_attr.set_file(4, 0x06A80003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag351_setup(void);

FieldstgListedBattle D_WSTAG351_800A5FE4 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A5FF0 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A5FFC = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6008 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6014 = { 95, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6020 = { 95, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A602C = { 95, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6038 = { 95, 13, 0x60080000 };
FieldstgBattleList D_WSTAG351_800A6044 = {
    3,
    { &D_WSTAG351_800A5FE4, &D_WSTAG351_800A5FF0, &D_WSTAG351_800A5FFC, &D_WSTAG351_800A6008, &D_WSTAG351_800A6014,
        &D_WSTAG351_800A6020, &D_WSTAG351_800A602C, &D_WSTAG351_800A6038 },
};
FieldstgListedBattle D_WSTAG351_800A6068 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6074 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6080 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A608C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6098 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A60A4 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A60B0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A60BC = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG351_800A60C8 = {
    0,
    { &D_WSTAG351_800A6068, &D_WSTAG351_800A6074, &D_WSTAG351_800A6080, &D_WSTAG351_800A608C, &D_WSTAG351_800A6098,
        &D_WSTAG351_800A60A4, &D_WSTAG351_800A60B0, &D_WSTAG351_800A60BC },
};
FieldstgListedBattle D_WSTAG351_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG351_800A6140 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG351_800A614C = {
    0,
    { &D_WSTAG351_800A60EC, &D_WSTAG351_800A60F8, &D_WSTAG351_800A6104, &D_WSTAG351_800A6110, &D_WSTAG351_800A611C,
        &D_WSTAG351_800A6128, &D_WSTAG351_800A6134, &D_WSTAG351_800A6140 },
};
FieldstgListedBattle D_WSTAG351_800A6170 = { 225, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A617C = { 226, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A6188 = { 273, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG351_800A6194 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A61A0 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A61AC = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A61B8 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG351_800A61C4 = { 180, 8, 0x60080000 };
FieldstgBattleList D_WSTAG351_800A61D0 = {
    0,
    { &D_WSTAG351_800A6170, &D_WSTAG351_800A617C, &D_WSTAG351_800A6188, &D_WSTAG351_800A6194, &D_WSTAG351_800A61A0,
        &D_WSTAG351_800A61AC, &D_WSTAG351_800A61B8, &D_WSTAG351_800A61C4 },
};
FieldstgBattleLists wstag351_battle_lists = {
    62, 0, 0, { &D_WSTAG351_800A6044, &D_WSTAG351_800A60C8, &D_WSTAG351_800A614C }, &D_WSTAG351_800A61D0,
};
FieldstgVramPlace wstag351_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 280, 152, 24, 368, 511 }, { 320, 256, 366, 280, 184, 24, 320, 510 },
};
u16 D_WSTAG351_800A6290[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A629C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A62A8[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A62B4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A62BC[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A62C8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A62D0[8] = { 0x11, 0, 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A62E0[10] = {
    0, 1, 0x7206, 1, 0x7208, 0, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A62F4[4] = { 0x7623, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A62FC[12] = {
    0x11, 0, 0, 1, 0x7206, 1, 0x7208, 1,
    0xE1C, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A6314[6] = { 0xE1C, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6320[14] = {
    0x11, 0, 0, 1, 0x7206, 1, 0x7208, 1,
    0xE1C, 1, 0x8014, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A633C[16] = {
    0x11, 0, 0, 1, 0x7206, 1, 0x7208, 1,
    0xE1C, 1, 0x8014, 1, 0x7209, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A635C[16] = {
    0x11, 0, 0, 1, 0x7206, 1, 0x7208, 1,
    0xE1C, 1, 0x8014, 1, 0x7209, 1, 0xFFFF, 0,
};
u16 D_WSTAG351_800A637C[4] = { 0x7823, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6384[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A638C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6394[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A63A0[8] = { 0, 1, 0x7209, 1, 0xE3D, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A63B0[6] = { 0x7400, 1, 0xE3D, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A63BC[10] = {
    0, 1, 0x7209, 1, 0x720B, 0, 0xE3D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A63D0[10] = {
    0, 1, 0x7209, 1, 0x720B, 1, 0xE3D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A63E4[4] = { 0x7823, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A63EC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A63F4[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A63FC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6404[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A640C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6418[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6424[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6430[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A643C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6444[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6450[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6458[8] = { 0x11, 0, 1, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6468[10] = {
    0x11, 0, 1, 1, 0x7206, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A647C[4] = { 0x7624, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6484[12] = {
    0x11, 0, 1, 1, 0x7206, 1, 0x7208, 1,
    0xE1D, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A649C[6] = { 0xE1D, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A64A8[14] = {
    0x11, 0, 1, 1, 0x7206, 1, 0x7208, 1,
    0xE1D, 1, 0x8014, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A64C4[16] = {
    0x11, 0, 1, 1, 0x7206, 1, 0x7208, 1,
    0xE1D, 1, 0x8014, 1, 0x7209, 0, 0xFFFF, 0,
};
u16 D_WSTAG351_800A64E4[16] = {
    0x11, 0, 1, 1, 0x7206, 1, 0x7208, 1,
    0xE1D, 1, 0x7209, 1, 0x8014, 1, 0xFFFF, 0,
};
u16 D_WSTAG351_800A6504[4] = { 0x7824, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A650C[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6514[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A651C[6] = { 1, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6528[8] = { 1, 1, 0x7209, 1, 0xE3E, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6538[6] = { 0x7401, 1, 0xE3E, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6544[10] = {
    1, 1, 0x7209, 1, 0xE3E, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A6558[10] = {
    1, 1, 0x7209, 1, 0xE3E, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG351_800A656C[4] = { 0x7824, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6574[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A657C[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A6584[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG351_800A658C[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A6594[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG351_800A65A0[10] = {
    { D_WSTAG351_800A6290, D_WSTAG351_800A629C, 586 }, { D_WSTAG351_800A62A8, D_WSTAG351_800A62B4, 585 },
    { D_WSTAG351_800A62BC, D_WSTAG351_800A62C8, 588 }, { D_WSTAG351_800A62D0, NULL, 590 },
    { D_WSTAG351_800A62E0, D_WSTAG351_800A62F4, 582 }, { D_WSTAG351_800A62FC, D_WSTAG351_800A6314, 583 },
    { D_WSTAG351_800A6320, NULL, 552 }, { D_WSTAG351_800A633C, NULL, 549 },
    { D_WSTAG351_800A635C, D_WSTAG351_800A637C, 582 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG351_800A6618[6] = {
    { D_WSTAG351_800A6384, D_WSTAG351_800A638C, 588 }, { D_WSTAG351_800A6394, NULL, 590 },
    { D_WSTAG351_800A63A0, D_WSTAG351_800A63B0, 589 }, { D_WSTAG351_800A63BC, NULL, 584 },
    { D_WSTAG351_800A63D0, D_WSTAG351_800A63E4, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG351_800A6660[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG351_800A6678[4] = {
    { D_WSTAG351_800A63EC, NULL, 923 }, { D_WSTAG351_800A63F4, D_WSTAG351_800A63FC, 585 },
    { D_WSTAG351_800A6404, D_WSTAG351_800A640C, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG351_800A66A8[10] = {
    { D_WSTAG351_800A6418, D_WSTAG351_800A6424, 554 }, { D_WSTAG351_800A6430, D_WSTAG351_800A643C, 553 },
    { D_WSTAG351_800A6444, D_WSTAG351_800A6450, 548 }, { D_WSTAG351_800A6458, NULL, 549 },
    { D_WSTAG351_800A6468, D_WSTAG351_800A647C, 550 }, { D_WSTAG351_800A6484, D_WSTAG351_800A649C, 551 },
    { D_WSTAG351_800A64A8, NULL, 552 }, { D_WSTAG351_800A64C4, NULL, 549 },
    { D_WSTAG351_800A64E4, D_WSTAG351_800A6504, 550 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG351_800A6720[6] = {
    { D_WSTAG351_800A650C, D_WSTAG351_800A6514, 556 }, { D_WSTAG351_800A651C, NULL, 558 },
    { D_WSTAG351_800A6528, D_WSTAG351_800A6538, 557 }, { D_WSTAG351_800A6544, NULL, 552 },
    { D_WSTAG351_800A6558, D_WSTAG351_800A656C, 559 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG351_800A6768[2] = { { NULL, NULL, 555 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG351_800A6780[4] = {
    { D_WSTAG351_800A6574, NULL, 925 }, { D_WSTAG351_800A657C, D_WSTAG351_800A6584, 553 },
    { D_WSTAG351_800A658C, D_WSTAG351_800A6594, 554 }, { NULL, NULL, 0 },
};
u16 D_WSTAG351_800A67B0[6] = { 0x8192, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67BC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67C4[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67D0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67D8[6] = { 0x8192, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67E4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67EC[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG351_800A67F8[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG351_800A6800 = { D_WSTAG351_800A67B0, D_WSTAG351_800A65A0, 69, 4, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG351_800A6814 = { D_WSTAG351_800A67BC, D_WSTAG351_800A6618, 69, 4, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG351_800A6828 = { D_WSTAG351_800A67C4, D_WSTAG351_800A6660, 69, 4, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG351_800A683C = { D_WSTAG351_800A67D0, D_WSTAG351_800A6678, 69, 4, 722, 161, 7 };
FieldstgPlacedActor D_WSTAG351_800A6850 = { D_WSTAG351_800A67D8, D_WSTAG351_800A66A8, 70, 5, 786, 643, 3 };
FieldstgPlacedActor D_WSTAG351_800A6864 = { D_WSTAG351_800A67E4, D_WSTAG351_800A6720, 70, 5, 786, 643, 3 };
FieldstgPlacedActor D_WSTAG351_800A6878 = { D_WSTAG351_800A67EC, D_WSTAG351_800A6768, 70, 5, 786, 643, 3 };
FieldstgPlacedActor D_WSTAG351_800A688C = { D_WSTAG351_800A67F8, D_WSTAG351_800A6780, 70, 5, 786, 643, 3 };
FieldstgPlacedActor *wstag351_actors[9] = {
    &D_WSTAG351_800A6800, &D_WSTAG351_800A6814, &D_WSTAG351_800A6828, &D_WSTAG351_800A683C, &D_WSTAG351_800A6850,
    &D_WSTAG351_800A6864, &D_WSTAG351_800A6878, &D_WSTAG351_800A688C, NULL,
};
FieldstgSprite wstag351_sprites[34] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 311, 249, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 728, 359, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 930, 44, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 145, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 239, 425, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 441, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 488, 363, 363, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 527, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 584, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 632, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 679, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 687, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 760, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 808, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 856, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 880, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 463, 463, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 440, 440, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1037, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1079, 395, 395, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1144, 387, 387, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1192, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1240, 435, 435, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 455, 455, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1304, 499, 499, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1336, 531, 531, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1368, 355, 355, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1380, 553, 553, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1411, 376, 376, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag351_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28D, 0x28C, 0x33C, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x8C, 0xCE, 7, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x38, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag351_funcs = { wstag351_setup };
FieldstgEventDef wstag351_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
