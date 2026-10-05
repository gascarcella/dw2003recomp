#include "wstag.h"

/* WSTAG556: stage 0x2B2 (fieldstg_stages). */

extern WstagFuncs wstag556_funcs;
extern FieldstgBattleLists wstag556_battle_lists;
extern FieldstgVramPlace wstag556_vram_places[];
extern FieldstgPlacedActor *wstag556_actors[];
extern FieldstgSprite wstag556_sprites[];
extern FieldstgMapEvent wstag556_map_events[];

void wstag556_update(WstagObject *obj) {
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

WstagObject *wstag556_start(void *arg0) {
    WstagObject *obj = object_new(wstag556_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag556_funcs.setup();
    return obj;
}

void wstag556_setup(void) {
    fieldstg_stage.background_file = 0x5E2;
    fieldstg_stage.sprite_file = 0x05E30000;
    fieldstg_stage.sprites = wstag556_sprites;
    fieldstg_stage.map_events = wstag556_map_events;
    fieldstg_stage.mask_file = 0x5E1;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10000, 0x50300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag556_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag556_actors;
    fieldstg_stage.battle_lists = &wstag556_battle_lists;
    fieldstg_attr.set_file(0, 0x05E30001);
    fieldstg_attr.set_file(7, 0x05E30002);
    fieldstg_attr.set_file(4, 0x05E30003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag556_setup(void);

FieldstgListedBattle D_WSTAG556_800A5F90 = { 134, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5F9C = { 134, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FA8 = { 134, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FB4 = { 134, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FC0 = { 172, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FCC = { 172, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FD8 = { 172, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A5FE4 = { 172, 5, 0x60080000 };
FieldstgBattleList D_WSTAG556_800A5FF0 = {
    3,
    { &D_WSTAG556_800A5F90, &D_WSTAG556_800A5F9C, &D_WSTAG556_800A5FA8, &D_WSTAG556_800A5FB4, &D_WSTAG556_800A5FC0,
        &D_WSTAG556_800A5FCC, &D_WSTAG556_800A5FD8, &D_WSTAG556_800A5FE4 },
};
FieldstgListedBattle D_WSTAG556_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG556_800A6074 = {
    0,
    { &D_WSTAG556_800A6014, &D_WSTAG556_800A6020, &D_WSTAG556_800A602C, &D_WSTAG556_800A6038, &D_WSTAG556_800A6044,
        &D_WSTAG556_800A6050, &D_WSTAG556_800A605C, &D_WSTAG556_800A6068 },
};
FieldstgListedBattle D_WSTAG556_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG556_800A60F8 = {
    0,
    { &D_WSTAG556_800A6098, &D_WSTAG556_800A60A4, &D_WSTAG556_800A60B0, &D_WSTAG556_800A60BC, &D_WSTAG556_800A60C8,
        &D_WSTAG556_800A60D4, &D_WSTAG556_800A60E0, &D_WSTAG556_800A60EC },
};
FieldstgListedBattle D_WSTAG556_800A611C = { 235, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG556_800A6128 = { 283, 5, 0x600C0000 };
FieldstgListedBattle D_WSTAG556_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG556_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG556_800A617C = {
    0,
    { &D_WSTAG556_800A611C, &D_WSTAG556_800A6128, &D_WSTAG556_800A6134, &D_WSTAG556_800A6140, &D_WSTAG556_800A614C,
        &D_WSTAG556_800A6158, &D_WSTAG556_800A6164, &D_WSTAG556_800A6170 },
};
FieldstgBattleLists wstag556_battle_lists = {
    99, 0, 0, { &D_WSTAG556_800A5FF0, &D_WSTAG556_800A6074, &D_WSTAG556_800A60F8 }, &D_WSTAG556_800A617C,
};
FieldstgVramPlace wstag556_vram_places[17] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 344, 362, 96, 106, 320, 511 }, { 320, 256, 352, 362, 128, 106, 336, 511 },
    { 384, 256, 400, 256, 320, 0, 352, 511 }, { 384, 256, 408, 256, 352, 0, 368, 511 },
    { 384, 256, 416, 256, 384, 0, 320, 510 }, { 384, 256, 400, 296, 320, 40, 336, 510 },
    { 384, 256, 408, 296, 352, 40, 352, 510 }, { 384, 256, 416, 296, 384, 40, 368, 510 },
    { 384, 256, 424, 296, 416, 40, 320, 509 }, { 384, 256, 424, 256, 416, 0, 336, 509 },
    { 384, 256, 432, 256, 448, 0, 352, 509 },
};
u16 D_WSTAG556_800A62CC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A62D4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A62DC[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A62E8[8] = { 0, 1, 0x7207, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A62F8[4] = { 0x7633, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6300[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE26, 0,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A6314[6] = { 0x7400, 1, 0xE26, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6320[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE26, 1,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A6334[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A633C[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6344[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A634C[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6354[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6360[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6368[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6370[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A637C[8] = { 0, 1, 0x720A, 1, 0xE48, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A638C[6] = { 0xE48, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6398[10] = {
    0, 1, 0x720A, 1, 0xE48, 1, 0x720C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A63AC[10] = {
    0x720C, 1, 0, 1, 0x720A, 1, 0xE48, 1,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A63C0[4] = { 0x7833, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG556_800A63C8[2] = { { NULL, NULL, 408 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A63E0[2] = { { NULL, NULL, 406 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A63F8[2] = { { NULL, NULL, 412 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6410[2] = { { NULL, NULL, 410 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6428[6] = {
    { D_WSTAG556_800A62CC, D_WSTAG556_800A62D4, 580 }, { D_WSTAG556_800A62DC, NULL, 581 },
    { D_WSTAG556_800A62E8, D_WSTAG556_800A62F8, 582 }, { D_WSTAG556_800A6300, D_WSTAG556_800A6314, 583 },
    { D_WSTAG556_800A6320, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG556_800A6470[4] = {
    { D_WSTAG556_800A6334, NULL, 923 }, { D_WSTAG556_800A633C, D_WSTAG556_800A6344, 585 },
    { D_WSTAG556_800A634C, D_WSTAG556_800A6354, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG556_800A64A0[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A64B8[6] = {
    { D_WSTAG556_800A6360, D_WSTAG556_800A6368, 588 }, { D_WSTAG556_800A6370, NULL, 590 },
    { D_WSTAG556_800A637C, D_WSTAG556_800A638C, 589 }, { D_WSTAG556_800A6398, NULL, 584 },
    { D_WSTAG556_800A63AC, D_WSTAG556_800A63C0, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG556_800A6500[2] = { { NULL, NULL, 413 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6518[2] = { { NULL, NULL, 413 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6530[2] = { { NULL, NULL, 411 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6548[2] = { { NULL, NULL, 411 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6560[2] = { { NULL, NULL, 409 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6578[2] = { { NULL, NULL, 409 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A6590[2] = { { NULL, NULL, 407 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A65A8[2] = { { NULL, NULL, 407 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A65C0[2] = { { NULL, NULL, 808 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG556_800A65D8[2] = { { NULL, NULL, 809 }, { NULL, NULL, 0 } };
u16 D_WSTAG556_800A65F0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A65FC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6608[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6614[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6620[10] = {
    0x7019, 1, 0x8192, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A6634[8] = { 0x7019, 1, 0x11, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6644[6] = { 0x7019, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6650[10] = {
    0x8014, 1, 0x8192, 1, 0x11, 0, 0x7019, 1,
    0xFFFF, 0,
};
u16 D_WSTAG556_800A6664[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6670[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6678[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A6684[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A668C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A6698[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A66A0[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG556_800A66AC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A66B4[4] = { 0x7020, 1, 0xFFFF, 0 };
u16 D_WSTAG556_800A66BC[4] = { 0x7020, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG556_800A66C4 = { D_WSTAG556_800A65F0, D_WSTAG556_800A63C8, 47, 4, 720, 777, 5 };
FieldstgPlacedActor D_WSTAG556_800A66D8 = { D_WSTAG556_800A65FC, D_WSTAG556_800A63E0, 49, 5, 992, 1209, 1 };
FieldstgPlacedActor D_WSTAG556_800A66EC = { D_WSTAG556_800A6608, D_WSTAG556_800A63F8, 54, 6, 1072, 433, 1 };
FieldstgPlacedActor D_WSTAG556_800A6700 = { D_WSTAG556_800A6614, D_WSTAG556_800A6410, 55, 7, 320, 769, 1 };
FieldstgPlacedActor D_WSTAG556_800A6714 = { D_WSTAG556_800A6620, D_WSTAG556_800A6428, 69, 8, 401, 577, 7 };
FieldstgPlacedActor D_WSTAG556_800A6728 = { D_WSTAG556_800A6634, D_WSTAG556_800A6470, 69, 8, 401, 577, 7 };
FieldstgPlacedActor D_WSTAG556_800A673C = { D_WSTAG556_800A6644, D_WSTAG556_800A64A0, 69, 8, 401, 577, 7 };
FieldstgPlacedActor D_WSTAG556_800A6750 = { D_WSTAG556_800A6650, D_WSTAG556_800A64B8, 69, 8, 401, 577, 7 };
FieldstgPlacedActor D_WSTAG556_800A6764 = { D_WSTAG556_800A6664, D_WSTAG556_800A6500, 157, 9, 1072, 433, 1 };
FieldstgPlacedActor D_WSTAG556_800A6778 = { D_WSTAG556_800A6670, D_WSTAG556_800A6518, 157, 9, 1072, 433, 1 };
FieldstgPlacedActor D_WSTAG556_800A678C = { D_WSTAG556_800A6678, D_WSTAG556_800A6530, 158, 10, 320, 769, 1 };
FieldstgPlacedActor D_WSTAG556_800A67A0 = { D_WSTAG556_800A6684, D_WSTAG556_800A6548, 158, 10, 320, 769, 1 };
FieldstgPlacedActor D_WSTAG556_800A67B4 = { D_WSTAG556_800A668C, D_WSTAG556_800A6560, 159, 11, 720, 777, 5 };
FieldstgPlacedActor D_WSTAG556_800A67C8 = { D_WSTAG556_800A6698, D_WSTAG556_800A6578, 159, 11, 720, 777, 5 };
FieldstgPlacedActor D_WSTAG556_800A67DC = { D_WSTAG556_800A66A0, D_WSTAG556_800A6590, 160, 12, 992, 1209, 1 };
FieldstgPlacedActor D_WSTAG556_800A67F0 = { D_WSTAG556_800A66AC, D_WSTAG556_800A65A8, 160, 12, 992, 1209, 1 };
FieldstgPlacedActor D_WSTAG556_800A6804 = { D_WSTAG556_800A66B4, D_WSTAG556_800A65C0, 306, 13, 411, 158, 1 };
FieldstgPlacedActor D_WSTAG556_800A6818 = { D_WSTAG556_800A66BC, D_WSTAG556_800A65D8, 307, 14, 389, 147, 1 };
FieldstgPlacedActor *wstag556_actors[19] = {
    &D_WSTAG556_800A66C4, &D_WSTAG556_800A66D8, &D_WSTAG556_800A66EC, &D_WSTAG556_800A6700, &D_WSTAG556_800A6714,
    &D_WSTAG556_800A6728, &D_WSTAG556_800A673C, &D_WSTAG556_800A6750, &D_WSTAG556_800A6764, &D_WSTAG556_800A6778,
    &D_WSTAG556_800A678C, &D_WSTAG556_800A67A0, &D_WSTAG556_800A67B4, &D_WSTAG556_800A67C8, &D_WSTAG556_800A67DC,
    &D_WSTAG556_800A67F0, &D_WSTAG556_800A6804, &D_WSTAG556_800A6818, NULL,
};
FieldstgSprite wstag556_sprites[9] = {
    { 1, 0, 0x80, 2, 6, 0, 0, 0, 0, 0, 874, 1273, 0, 0 }, { 1, 0, 0x80, 6, 7, 0, 0, 0, 0, 0, 856, 1221, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 506, 475, 544, 0 }, { 1, 0, 0x50, 4, 1, 0, 0, 0, 0, 0, 317, 423, 488, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 225, 615, 666, 0 }, { 1, 0, 0x48, 4, 3, 0, 0, 0, 0, 0, 476, 647, 713, 0 },
    { 1, 0, 0x49, 4, 4, 0, 0, 0, 0, 0, 592, 815, 880, 0 }, { 1, 0, 0x78, 4, 5, 0, 0, 0, 0, 0, 289, 127, 230, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag556_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C6, 0xC8, 0x3BC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B3, 0x7A, 0xEA, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B1, 0xB0, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C0, 0x320, 0x88, 1, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x1E, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 8, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag556_funcs = { wstag556_setup };
