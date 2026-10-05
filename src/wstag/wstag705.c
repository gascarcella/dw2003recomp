#include "wstag.h"

/* WSTAG705: stage 0x264 (fieldstg_stages). */

extern WstagFuncs wstag705_funcs;
extern FieldstgBattleLists wstag705_battle_lists;
extern FieldstgVramPlace wstag705_vram_places[];
extern FieldstgPlacedActor *wstag705_actors[];
extern FieldstgSprite wstag705_sprites[];
extern FieldstgMapEvent wstag705_map_events[];

void wstag705_update(WstagObject *obj) {
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

WstagObject *wstag705_start(void *arg0) {
    WstagObject *obj = object_new(wstag705_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag705_funcs.setup();
    return obj;
}

void wstag705_setup(void) {
    fieldstg_stage.background_file = 0x648;
    fieldstg_stage.sprite_file = 0x06490000;
    fieldstg_stage.sprites = wstag705_sprites;
    fieldstg_stage.map_events = wstag705_map_events;
    fieldstg_stage.mask_file = 0x647;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x4E400, 0x7B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag705_vram_places;
    fieldstg_stage.music = 0x3E;
    fieldstg_stage.sound = 0x60F80000;
    fieldstg_stage.actors = wstag705_actors;
    fieldstg_stage.battle_lists = &wstag705_battle_lists;
    fieldstg_attr.set_file(0, 0x06490001);
    fieldstg_attr.set_file(7, 0x06490002);
    fieldstg_attr.set_file(4, 0x06490003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag705_setup(void);

FieldstgListedBattle D_WSTAG705_800A5F90 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5F9C = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FA8 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FB4 = { 113, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FC0 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FCC = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FD8 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG705_800A5FE4 = { 114, 14, 0x60080000 };
FieldstgBattleList D_WSTAG705_800A5FF0 = {
    2,
    { &D_WSTAG705_800A5F90, &D_WSTAG705_800A5F9C, &D_WSTAG705_800A5FA8, &D_WSTAG705_800A5FB4, &D_WSTAG705_800A5FC0,
        &D_WSTAG705_800A5FCC, &D_WSTAG705_800A5FD8, &D_WSTAG705_800A5FE4 },
};
FieldstgListedBattle D_WSTAG705_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG705_800A6074 = {
    0,
    { &D_WSTAG705_800A6014, &D_WSTAG705_800A6020, &D_WSTAG705_800A602C, &D_WSTAG705_800A6038, &D_WSTAG705_800A6044,
        &D_WSTAG705_800A6050, &D_WSTAG705_800A605C, &D_WSTAG705_800A6068 },
};
FieldstgListedBattle D_WSTAG705_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG705_800A60F8 = {
    0,
    { &D_WSTAG705_800A6098, &D_WSTAG705_800A60A4, &D_WSTAG705_800A60B0, &D_WSTAG705_800A60BC, &D_WSTAG705_800A60C8,
        &D_WSTAG705_800A60D4, &D_WSTAG705_800A60E0, &D_WSTAG705_800A60EC },
};
FieldstgListedBattle D_WSTAG705_800A611C = { 219, 14, 0x600C0000 };
FieldstgListedBattle D_WSTAG705_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG705_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG705_800A617C = {
    0,
    { &D_WSTAG705_800A611C, &D_WSTAG705_800A6128, &D_WSTAG705_800A6134, &D_WSTAG705_800A6140, &D_WSTAG705_800A614C,
        &D_WSTAG705_800A6158, &D_WSTAG705_800A6164, &D_WSTAG705_800A6170 },
};
FieldstgBattleLists wstag705_battle_lists = {
    83, 0, 0, { &D_WSTAG705_800A5FF0, &D_WSTAG705_800A6074, &D_WSTAG705_800A60F8 }, &D_WSTAG705_800A617C,
};
FieldstgVramPlace wstag705_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 288, 208, 32, 320, 511 }, { 320, 256, 364, 256, 176, 0, 336, 511 },
    { 320, 256, 364, 296, 176, 40, 352, 511 }, { 320, 256, 372, 256, 208, 0, 368, 511 },
};
u16 D_WSTAG705_800A625C[8] = { 0x224, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A626C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6274[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A627C[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6288[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6298[4] = { 0x761D, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A62A0[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG705_800A62B4[12] = {
    0xE13, 0, 0, 1, 0x7206, 1, 0x8014, 1,
    0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A62CC[6] = { 0xE13, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A62D8[14] = {
    0xE13, 1, 0x720A, 0, 0, 1, 0x7206, 1,
    0x8014, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A62F4[14] = {
    0xE13, 1, 0x720A, 1, 0, 1, 0x7206, 1,
    0x8014, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A6310[4] = { 0x781D, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6318[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6320[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A632C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6334[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6340[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A634C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6354[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A635C[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6368[8] = { 0x7206, 1, 0, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6378[4] = { 0x761D, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6380[10] = {
    0, 1, 0x8014, 1, 0x7208, 0, 0x7206, 1,
    0xFFFF, 0,
};
u16 D_WSTAG705_800A6394[12] = {
    0x7206, 1, 0xE13, 0, 0, 1, 0x8014, 1,
    0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A63AC[6] = { 0xE13, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A63B8[14] = {
    0, 1, 0x8014, 1, 0x7208, 1, 0xE13, 1,
    0x720A, 0, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A63D4[14] = {
    0x7206, 1, 0x7208, 1, 0xE13, 1, 0x720A, 1,
    0, 1, 0x8014, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A63F0[4] = { 0x781D, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A63F8[8] = { 0x225, 1, 0x8B16, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6408[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6410[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A641C[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A642C[4] = { 0x761D, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6434[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG705_800A6448[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE13, 0, 0xFFFF, 0,
};
u16 D_WSTAG705_800A6460[4] = { 0xE13, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6468[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE13, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG705_800A6484[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE13, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG705_800A64A0[4] = { 0x781D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG705_800A64A8[2] = { { NULL, D_WSTAG705_800A625C, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG705_800A64C0[8] = {
    { D_WSTAG705_800A626C, D_WSTAG705_800A6274, 368 }, { D_WSTAG705_800A627C, NULL, 372 },
    { D_WSTAG705_800A6288, D_WSTAG705_800A6298, 373 }, { D_WSTAG705_800A62A0, NULL, 374 },
    { D_WSTAG705_800A62B4, D_WSTAG705_800A62CC, 375 }, { D_WSTAG705_800A62D8, NULL, 376 },
    { D_WSTAG705_800A62F4, D_WSTAG705_800A6310, 377 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG705_800A6520[2] = { { NULL, NULL, 662 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG705_800A6538[4] = {
    { D_WSTAG705_800A6318, NULL, 368 }, { D_WSTAG705_800A6320, D_WSTAG705_800A632C, 378 },
    { D_WSTAG705_800A6334, D_WSTAG705_800A6340, 379 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG705_800A6568[8] = {
    { D_WSTAG705_800A634C, D_WSTAG705_800A6354, 369 }, { D_WSTAG705_800A635C, NULL, 372 },
    { D_WSTAG705_800A6368, D_WSTAG705_800A6378, 373 }, { D_WSTAG705_800A6380, NULL, 374 },
    { D_WSTAG705_800A6394, D_WSTAG705_800A63AC, 375 }, { D_WSTAG705_800A63B8, NULL, 376 },
    { D_WSTAG705_800A63D4, D_WSTAG705_800A63F0, 377 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG705_800A65C8[2] = { { NULL, D_WSTAG705_800A63F8, 618 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG705_800A65E0[8] = {
    { D_WSTAG705_800A6408, NULL, 370 }, { D_WSTAG705_800A6410, NULL, 370 },
    { D_WSTAG705_800A641C, D_WSTAG705_800A642C, 370 }, { D_WSTAG705_800A6434, NULL, 370 },
    { D_WSTAG705_800A6448, D_WSTAG705_800A6460, 370 }, { D_WSTAG705_800A6468, NULL, 370 },
    { D_WSTAG705_800A6484, D_WSTAG705_800A64A0, 370 }, { NULL, NULL, 0 },
};
u16 D_WSTAG705_800A6640[4] = { 0x224, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6648[8] = { 0x11, 0, 0x8192, 1, 0x7004, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6658[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6668[8] = { 0x11, 1, 0x7009, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6678[8] = { 0x11, 0, 0x6026, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG705_800A6688[4] = { 0x225, 0, 0xFFFF, 0 };
u16 D_WSTAG705_800A6690[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG705_800A6698 = { D_WSTAG705_800A6640, D_WSTAG705_800A64A8, 33, 4, 992, 273, 1 };
FieldstgPlacedActor D_WSTAG705_800A66AC = { D_WSTAG705_800A6648, D_WSTAG705_800A64C0, 47, 5, 576, 448, 1 };
FieldstgPlacedActor D_WSTAG705_800A66C0 = { D_WSTAG705_800A6658, D_WSTAG705_800A6520, 47, 5, 576, 448, 1 };
FieldstgPlacedActor D_WSTAG705_800A66D4 = { D_WSTAG705_800A6668, D_WSTAG705_800A6538, 47, 5, 576, 448, 1 };
FieldstgPlacedActor D_WSTAG705_800A66E8 = { D_WSTAG705_800A6678, D_WSTAG705_800A6568, 47, 5, 576, 448, 1 };
FieldstgPlacedActor D_WSTAG705_800A66FC = { D_WSTAG705_800A6688, D_WSTAG705_800A65C8, 77, 6, 896, 697, 1 };
FieldstgPlacedActor D_WSTAG705_800A6710 = { D_WSTAG705_800A6690, D_WSTAG705_800A65E0, 157, 7, 576, 448, 1 };
FieldstgPlacedActor *wstag705_actors[8] = {
    &D_WSTAG705_800A6698, &D_WSTAG705_800A66AC, &D_WSTAG705_800A66C0, &D_WSTAG705_800A66D4, &D_WSTAG705_800A66E8,
    &D_WSTAG705_800A66FC, &D_WSTAG705_800A6710, NULL,
};
FieldstgSprite wstag705_sprites[5] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 837, 766, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 891, 248, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 1001, 145, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 1194, 110, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag705_map_events[22] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26E, 0xE0, 0x3A8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x262, 0x3A0, 0xA0, 1, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x15, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xC, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 5, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xA, 2 },
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
WstagFuncs wstag705_funcs = { wstag705_setup };
