#include "wstag.h"

/* WSTAG496: stage 0x2AA (fieldstg_stages). */

extern WstagFuncs wstag496_funcs;
extern FieldstgBattleLists wstag496_battle_lists;
extern FieldstgVramPlace wstag496_vram_places[];
extern FieldstgPlacedActor *wstag496_actors[];
extern FieldstgSprite wstag496_sprites[];
extern FieldstgMapEvent wstag496_map_events[];

void wstag496_update(WstagObject *obj) {
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

WstagObject *wstag496_start(void *arg0) {
    WstagObject *obj = object_new(wstag496_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag496_funcs.setup();
    return obj;
}

void wstag496_setup(void) {
    fieldstg_stage.background_file = 0x5B5;
    fieldstg_stage.sprite_file = 0x05B60000;
    fieldstg_stage.sprites = wstag496_sprites;
    fieldstg_stage.map_events = wstag496_map_events;
    fieldstg_stage.mask_file = 0x5B4;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xEC00, 0x1AC00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag496_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag496_actors;
    fieldstg_stage.battle_lists = &wstag496_battle_lists;
    fieldstg_attr.set_file(0, 0x05B60001);
    fieldstg_attr.set_file(7, 0x05B60002);
    fieldstg_attr.set_file(4, 0x05B60003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag496_setup(void);

FieldstgListedBattle D_WSTAG496_800A5F90 = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5F9C = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FA8 = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FB4 = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FC0 = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FCC = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FD8 = { 157, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A5FE4 = { 157, 2, 0x60080000 };
FieldstgBattleList D_WSTAG496_800A5FF0 = {
    3,
    { &D_WSTAG496_800A5F90, &D_WSTAG496_800A5F9C, &D_WSTAG496_800A5FA8, &D_WSTAG496_800A5FB4, &D_WSTAG496_800A5FC0,
        &D_WSTAG496_800A5FCC, &D_WSTAG496_800A5FD8, &D_WSTAG496_800A5FE4 },
};
FieldstgListedBattle D_WSTAG496_800A6014 = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6020 = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A602C = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6038 = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6044 = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6050 = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A605C = { 95, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6068 = { 95, 2, 0x60080000 };
FieldstgBattleList D_WSTAG496_800A6074 = {
    3,
    { &D_WSTAG496_800A6014, &D_WSTAG496_800A6020, &D_WSTAG496_800A602C, &D_WSTAG496_800A6038, &D_WSTAG496_800A6044,
        &D_WSTAG496_800A6050, &D_WSTAG496_800A605C, &D_WSTAG496_800A6068 },
};
FieldstgListedBattle D_WSTAG496_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG496_800A60F8 = {
    0,
    { &D_WSTAG496_800A6098, &D_WSTAG496_800A60A4, &D_WSTAG496_800A60B0, &D_WSTAG496_800A60BC, &D_WSTAG496_800A60C8,
        &D_WSTAG496_800A60D4, &D_WSTAG496_800A60E0, &D_WSTAG496_800A60EC },
};
FieldstgListedBattle D_WSTAG496_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A614C = { 332, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG496_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG496_800A6170 = { 106, 2, 0x60080000 };
FieldstgBattleList D_WSTAG496_800A617C = {
    0,
    { &D_WSTAG496_800A611C, &D_WSTAG496_800A6128, &D_WSTAG496_800A6134, &D_WSTAG496_800A6140, &D_WSTAG496_800A614C,
        &D_WSTAG496_800A6158, &D_WSTAG496_800A6164, &D_WSTAG496_800A6170 },
};
FieldstgBattleLists wstag496_battle_lists = {
    77, 0, 0, { &D_WSTAG496_800A5FF0, &D_WSTAG496_800A6074, &D_WSTAG496_800A60F8 }, &D_WSTAG496_800A617C,
};
FieldstgVramPlace wstag496_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 344, 304, 96, 48, 336, 511 }, { 320, 256, 372, 304, 208, 48, 352, 511 },
    { 320, 256, 346, 256, 104, 0, 368, 511 }, { 320, 256, 320, 317, 0, 61, 320, 510 },
    { 320, 256, 328, 317, 32, 61, 336, 510 },
};
u16 D_WSTAG496_800A626C[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG496_800A6278[4] = { 0x9404, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A6280[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A628C[4] = { 0x9406, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A6294[4] = { 0x940D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG496_800A629C[2] = { { NULL, NULL, 267 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A62B4[2] = { { NULL, NULL, 270 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A62CC[3] = {
    { D_WSTAG496_800A626C, D_WSTAG496_800A6278, 730 }, { D_WSTAG496_800A6280, D_WSTAG496_800A628C, 730 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG496_800A62F0[2] = { { NULL, D_WSTAG496_800A6294, 730 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A6308[2] = { { NULL, NULL, 266 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A6320[2] = { { NULL, NULL, 268 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A6338[2] = { { NULL, NULL, 269 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG496_800A6350[2] = { { NULL, NULL, 271 }, { NULL, NULL, 0 } };
u16 D_WSTAG496_800A6368[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A6374[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A6380[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG496_800A6388[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A6390[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG496_800A639C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG496_800A63A4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG496_800A63B0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG496_800A63B8 = { D_WSTAG496_800A6368, D_WSTAG496_800A629C, 54, 4, 480, 480, 1 };
FieldstgPlacedActor D_WSTAG496_800A63CC = { D_WSTAG496_800A6374, D_WSTAG496_800A62B4, 57, 5, 287, 536, 7 };
FieldstgPlacedActor D_WSTAG496_800A63E0 = { D_WSTAG496_800A6380, D_WSTAG496_800A62CC, 138, 6, 204, 412, 7 };
FieldstgPlacedActor D_WSTAG496_800A63F4 = { D_WSTAG496_800A6388, D_WSTAG496_800A62F0, 138, 6, 204, 412, 7 };
FieldstgPlacedActor D_WSTAG496_800A6408 = { D_WSTAG496_800A6390, D_WSTAG496_800A6308, 157, 7, 480, 480, 1 };
FieldstgPlacedActor D_WSTAG496_800A641C = { D_WSTAG496_800A639C, D_WSTAG496_800A6320, 157, 7, 480, 480, 1 };
FieldstgPlacedActor D_WSTAG496_800A6430 = { D_WSTAG496_800A63A4, D_WSTAG496_800A6338, 158, 8, 287, 536, 7 };
FieldstgPlacedActor D_WSTAG496_800A6444 = { D_WSTAG496_800A63B0, D_WSTAG496_800A6350, 158, 8, 287, 536, 7 };
FieldstgPlacedActor *wstag496_actors[9] = {
    &D_WSTAG496_800A63B8, &D_WSTAG496_800A63CC, &D_WSTAG496_800A63E0, &D_WSTAG496_800A63F4, &D_WSTAG496_800A6408,
    &D_WSTAG496_800A641C, &D_WSTAG496_800A6430, &D_WSTAG496_800A6444, NULL,
};
FieldstgSprite wstag496_sprites[9] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 424, 218, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 281, 604, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 520, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 621, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 347, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 382, 315, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 615, 560, 0, 0 },
    { 1, 0, 0x64, 4, 6, 0, 0, 0, 0, 0, 209, 351, 391, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag496_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A9, 0x88, 0x1BE, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0x13, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFD8, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag496_funcs = { wstag496_setup };
