#include "wstag.h"

/* WSTAG695: stage 0x262 (fieldstg_stages). */

extern WstagFuncs wstag695_funcs;
extern FieldstgBattleLists wstag695_battle_lists;
extern FieldstgVramPlace wstag695_vram_places[];
extern FieldstgPlacedActor *wstag695_actors[];
extern FieldstgSprite wstag695_sprites[];
extern FieldstgMapEvent wstag695_map_events[];

void wstag695_update(WstagObject *obj) {
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

WstagObject *wstag695_start(void *arg0) {
    WstagObject *obj = object_new(wstag695_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag695_funcs.setup();
    return obj;
}

void wstag695_setup(void) {
    fieldstg_stage.background_file = 0x76D;
    fieldstg_stage.sprite_file = 0x076E0000;
    fieldstg_stage.sprites = wstag695_sprites;
    fieldstg_stage.map_events = wstag695_map_events;
    fieldstg_stage.mask_file = 0x76C;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x33200, 0xD500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag695_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag695_actors;
    fieldstg_stage.battle_lists = &wstag695_battle_lists;
    fieldstg_attr.set_file(0, 0x076E0001);
    fieldstg_attr.set_file(7, 0x076E0002);
    fieldstg_attr.set_file(4, 0x076E0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag695_setup(void);

FieldstgListedBattle D_WSTAG695_800A5F90 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5F9C = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FA8 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FB4 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FC0 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FCC = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FD8 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A5FE4 = { 114, 14, 0x60080000 };
FieldstgBattleList D_WSTAG695_800A5FF0 = {
    3,
    { &D_WSTAG695_800A5F90, &D_WSTAG695_800A5F9C, &D_WSTAG695_800A5FA8, &D_WSTAG695_800A5FB4, &D_WSTAG695_800A5FC0,
        &D_WSTAG695_800A5FCC, &D_WSTAG695_800A5FD8, &D_WSTAG695_800A5FE4 },
};
FieldstgListedBattle D_WSTAG695_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG695_800A6074 = {
    0,
    { &D_WSTAG695_800A6014, &D_WSTAG695_800A6020, &D_WSTAG695_800A602C, &D_WSTAG695_800A6038, &D_WSTAG695_800A6044,
        &D_WSTAG695_800A6050, &D_WSTAG695_800A605C, &D_WSTAG695_800A6068 },
};
FieldstgListedBattle D_WSTAG695_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG695_800A60F8 = {
    0,
    { &D_WSTAG695_800A6098, &D_WSTAG695_800A60A4, &D_WSTAG695_800A60B0, &D_WSTAG695_800A60BC, &D_WSTAG695_800A60C8,
        &D_WSTAG695_800A60D4, &D_WSTAG695_800A60E0, &D_WSTAG695_800A60EC },
};
FieldstgListedBattle D_WSTAG695_800A611C = { 218, 14, 0x600C0000 };
FieldstgListedBattle D_WSTAG695_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6140 = { 333, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG695_800A6164 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG695_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG695_800A617C = {
    0,
    { &D_WSTAG695_800A611C, &D_WSTAG695_800A6128, &D_WSTAG695_800A6134, &D_WSTAG695_800A6140, &D_WSTAG695_800A614C,
        &D_WSTAG695_800A6158, &D_WSTAG695_800A6164, &D_WSTAG695_800A6170 },
};
FieldstgBattleLists wstag695_battle_lists = {
    82, 0, 0, { &D_WSTAG695_800A5FF0, &D_WSTAG695_800A6074, &D_WSTAG695_800A60F8 }, &D_WSTAG695_800A617C,
};
FieldstgVramPlace wstag695_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 342, 456, 88, 200, 368, 509 }, { 320, 256, 374, 392, 216, 136, 368, 508 },
    { 320, 256, 374, 464, 216, 208, 352, 507 }, { 320, 256, 374, 432, 216, 176, 368, 507 },
};
u16 D_WSTAG695_800A625C[8] = { 0x7090, 1, 0x222, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A626C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6274[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A627C[6] = { 0x7206, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6288[8] = { 0, 1, 0x8014, 0, 0x7206, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6298[4] = { 0x761C, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A62A0[10] = {
    0x7206, 1, 0, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG695_800A62B4[12] = {
    0, 1, 0x8014, 1, 0x7208, 1, 0x7206, 1,
    0xE12, 0, 0xFFFF, 0,
};
u16 D_WSTAG695_800A62CC[6] = { 0x7400, 1, 0xE12, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A62D8[14] = {
    0x7206, 1, 0xE12, 1, 0x720A, 0, 0, 1,
    0x8014, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG695_800A62F4[14] = {
    0, 1, 0x8014, 1, 0x7208, 1, 0x7206, 1,
    0xE12, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG695_800A6310[4] = { 0x781C, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6318[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6320[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A632C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6334[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6340[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A634C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6354[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A635C[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6368[8] = { 0x8014, 0, 0, 1, 0x7206, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6378[4] = { 0x761C, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6380[10] = {
    0x7206, 1, 0, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG695_800A6394[12] = {
    0x8014, 1, 0x7208, 1, 0, 1, 0x7206, 1,
    0xE12, 0, 0xFFFF, 0,
};
u16 D_WSTAG695_800A63AC[6] = { 0xE12, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A63B8[14] = {
    0x7206, 1, 0xE12, 1, 0x720A, 0, 0, 1,
    0x8014, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG695_800A63D4[14] = {
    0x8014, 1, 0x7208, 1, 0, 1, 0x7206, 1,
    0xE12, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG695_800A63F0[4] = { 0x781C, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A63F8[8] = { 0x223, 1, 0x88A0, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6408[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6410[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A641C[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A642C[4] = { 0x761C, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6434[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG695_800A6448[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE12, 0, 0xFFFF, 0,
};
u16 D_WSTAG695_800A6460[4] = { 0xE12, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6468[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE12, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG695_800A6484[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE12, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG695_800A64A0[4] = { 0x781C, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG695_800A64A8[2] = { { NULL, D_WSTAG695_800A625C, 814 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG695_800A64C0[8] = {
    { D_WSTAG695_800A626C, D_WSTAG695_800A6274, 13 }, { D_WSTAG695_800A627C, NULL, 17 },
    { D_WSTAG695_800A6288, D_WSTAG695_800A6298, 18 }, { D_WSTAG695_800A62A0, NULL, 19 },
    { D_WSTAG695_800A62B4, D_WSTAG695_800A62CC, 20 }, { D_WSTAG695_800A62D8, NULL, 21 },
    { D_WSTAG695_800A62F4, D_WSTAG695_800A6310, 22 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG695_800A6520[4] = {
    { D_WSTAG695_800A6318, NULL, 13 }, { D_WSTAG695_800A6320, D_WSTAG695_800A632C, 23 },
    { D_WSTAG695_800A6334, D_WSTAG695_800A6340, 24 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG695_800A6550[8] = {
    { D_WSTAG695_800A634C, D_WSTAG695_800A6354, 14 }, { D_WSTAG695_800A635C, NULL, 17 },
    { D_WSTAG695_800A6368, D_WSTAG695_800A6378, 18 }, { D_WSTAG695_800A6380, NULL, 19 },
    { D_WSTAG695_800A6394, D_WSTAG695_800A63AC, 20 }, { D_WSTAG695_800A63B8, NULL, 21 },
    { D_WSTAG695_800A63D4, D_WSTAG695_800A63F0, 22 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG695_800A65B0[2] = { { NULL, NULL, 661 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG695_800A65C8[2] = { { NULL, D_WSTAG695_800A63F8, 616 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG695_800A65E0[8] = {
    { D_WSTAG695_800A6408, NULL, 15 }, { D_WSTAG695_800A6410, NULL, 15 },
    { D_WSTAG695_800A641C, D_WSTAG695_800A642C, 15 }, { D_WSTAG695_800A6434, NULL, 15 },
    { D_WSTAG695_800A6448, D_WSTAG695_800A6460, 15 }, { D_WSTAG695_800A6468, NULL, 15 },
    { D_WSTAG695_800A6484, D_WSTAG695_800A64A0, 15 }, { NULL, NULL, 0 },
};
u16 D_WSTAG695_800A6640[4] = { 0x222, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6648[8] = { 0x11, 0, 0x7004, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6658[8] = { 0x11, 1, 0x8192, 1, 0x7009, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6668[8] = { 0x6026, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG695_800A6678[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6688[4] = { 0x223, 0, 0xFFFF, 0 };
u16 D_WSTAG695_800A6690[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG695_800A6698 = { D_WSTAG695_800A6640, D_WSTAG695_800A64A8, 33, 4, 384, 453, 1 };
FieldstgPlacedActor D_WSTAG695_800A66AC = { D_WSTAG695_800A6648, D_WSTAG695_800A64C0, 52, 5, 657, 408, 1 };
FieldstgPlacedActor D_WSTAG695_800A66C0 = { D_WSTAG695_800A6658, D_WSTAG695_800A6520, 52, 5, 657, 408, 1 };
FieldstgPlacedActor D_WSTAG695_800A66D4 = { D_WSTAG695_800A6668, D_WSTAG695_800A6550, 52, 5, 657, 408, 1 };
FieldstgPlacedActor D_WSTAG695_800A66E8 = { D_WSTAG695_800A6678, D_WSTAG695_800A65B0, 52, 5, 657, 408, 1 };
FieldstgPlacedActor D_WSTAG695_800A66FC = { D_WSTAG695_800A6688, D_WSTAG695_800A65C8, 77, 6, 961, 241, 1 };
FieldstgPlacedActor D_WSTAG695_800A6710 = { D_WSTAG695_800A6690, D_WSTAG695_800A65E0, 157, 7, 657, 408, 1 };
FieldstgPlacedActor *wstag695_actors[8] = {
    &D_WSTAG695_800A6698, &D_WSTAG695_800A66AC, &D_WSTAG695_800A66C0, &D_WSTAG695_800A66D4, &D_WSTAG695_800A66E8,
    &D_WSTAG695_800A66FC, &D_WSTAG695_800A6710, NULL,
};
FieldstgSprite wstag695_sprites[18] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 5, 6, 0, 472, 329, 0, 0 },
    { 1, 0, 0xC8, 2, 0x34, 1, 0x34, 0x37, 8, 0, 542, 52, 0, 0 },
    { 1, 0, 0x40, 2, 0x3B, 2, 0, 3, 8, 0, 412, 670, 0, 0 }, { 1, 0, 0x40, 2, 2, 1, 2, 7, 4, 0, 93, 305, 0, 0 },
    { 1, 0, 0x40, 2, 2, 1, 2, 7, 4, 0, 585, 772, 0, 0 }, { 1, 0, 0x40, 2, 2, 1, 2, 7, 4, 0, 1033, 514, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xD, 6, 0, 579, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 3, 8, 0, 557, 494, 0, 0 }, { 1, 0, 0x50, 6, 0x39, 2, 0, 3, 8, 0, 549, 528, 0, 0 },
    { 1, 0, 0x50, 6, 0x3A, 2, 0, 3, 8, 0, 493, 598, 0, 0 }, { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 546, 340, 0, 0 },
    { 1, 0, 0x48, 4, 0, 0, 0, 0, 0, 0, 517, 329, 393, 0 }, { 1, 0, 0x53, 4, 8, 0, 0, 0, 0, 0, 448, 290, 368, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 783, 783, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 263, 263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 199, 199, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag695_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x261, 0x500, 0x290, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x261, 0x500, 0x300, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x264, 0x238, 0x444, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x263, 0x108, 0x12C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1CE, 0xAA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1BF, 0x112, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x3B0, 0x16A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x3BF, 0x1D0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x380, 0x253, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x38F, 0x2EA, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x13, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 7, 2 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag695_funcs = { wstag695_setup };
