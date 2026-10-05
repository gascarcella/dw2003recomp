#include "wstag.h"

/* WSTAG948: stage 0x294 (fieldstg_stages_2d). */

extern WstagFuncs wstag948_funcs;
extern FieldstgVramPlace wstag948_vram_places[];
extern FieldstgPlacedActor *wstag948_actors[];
extern FieldstgSprite wstag948_sprites[];
extern FieldstgMapEvent wstag948_map_events[];
extern FieldstgBattleLists wstag948_battle_lists;

void wstag948_update(WstagObject *obj) {
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

WstagObject *wstag948_start(void *arg0) {
    WstagObject *obj = object_new(wstag948_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag948_funcs.setup();
    return obj;
}

void wstag948_setup(void) {
    fieldstg_stage.background_file = 0x1BC;
    fieldstg_stage.sprite_file = 0x09170000;
    fieldstg_stage.sprites = wstag948_sprites;
    fieldstg_stage.map_events = wstag948_map_events;
    fieldstg_stage.mask_file = 0x916;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19B00, 0x23500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag948_vram_places;
    fieldstg_stage.music = 0x32;
    fieldstg_stage.sound = 0x60C80000;
    fieldstg_stage.actors = wstag948_actors;
    fieldstg_stage.battle_lists = &wstag948_battle_lists;
    fieldstg_attr.set_file(0, 0x09170002);
    fieldstg_attr.set_file(7, 0x09170003);
    fieldstg_attr.set_file(4, 0x09170001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag948_setup(void);

FieldstgVramPlace wstag948_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 256, 128, 0, 320, 511 },
};
u16 D_WSTAG948_800A6004[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG948_800A6010[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG948_800A601C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG948_800A6028[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG948_800A6038[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG948_800A6044[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG948_800A604C[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG948_800A6058[4] = { 0x7851, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG948_800A6060[2] = { { NULL, NULL, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG948_800A6078[5] = {
    { D_WSTAG948_800A6004, D_WSTAG948_800A6010, 55 }, { D_WSTAG948_800A601C, D_WSTAG948_800A6028, 56 },
    { D_WSTAG948_800A6038, D_WSTAG948_800A6044, 53 }, { D_WSTAG948_800A604C, D_WSTAG948_800A6058, 54 },
    { NULL, NULL, 0 },
};
u16 D_WSTAG948_800A60B4[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG948_800A60BC[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG948_800A60C4 = { D_WSTAG948_800A60B4, D_WSTAG948_800A6060, 51, 4, 320, 736, 3 };
FieldstgPlacedActor D_WSTAG948_800A60D8 = { D_WSTAG948_800A60BC, D_WSTAG948_800A6078, 51, 4, 320, 736, 3 };
FieldstgPlacedActor *wstag948_actors[3] = { &D_WSTAG948_800A60C4, &D_WSTAG948_800A60D8, NULL };
FieldstgSprite wstag948_sprites[9] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 1024, 448, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 1088, 448, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 896, 576, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 960, 576, 0, 0 },
    { 1, 0, 0x40, 2, 4, 0, 0, 0, 0, 0, 1024, 576, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 1088, 576, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 1024, 640, 0, 0 }, { 1, 0, 0x40, 2, 7, 0, 0, 0, 0, 0, 1088, 640, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag948_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x295, 0x5E, 0x40E, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x374, 0x9B, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag948_funcs = { wstag948_setup };
FieldstgListedBattle D_WSTAG948_800A61E8 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A61F4 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6200 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A620C = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6218 = { 47, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6224 = { 47, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6230 = { 47, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A623C = { 47, 3, 0x60080000 };
FieldstgBattleList D_WSTAG948_800A6248 = {
    3,
    { &D_WSTAG948_800A61E8, &D_WSTAG948_800A61F4, &D_WSTAG948_800A6200, &D_WSTAG948_800A620C, &D_WSTAG948_800A6218,
        &D_WSTAG948_800A6224, &D_WSTAG948_800A6230, &D_WSTAG948_800A623C },
};
FieldstgListedBattle D_WSTAG948_800A626C = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6278 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6284 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A6290 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A629C = { 149, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A62A8 = { 149, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A62B4 = { 149, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG948_800A62C0 = { 149, 8, 0x60080000 };
FieldstgBattleList D_WSTAG948_800A62CC = {
    2,
    { &D_WSTAG948_800A626C, &D_WSTAG948_800A6278, &D_WSTAG948_800A6284, &D_WSTAG948_800A6290, &D_WSTAG948_800A629C,
        &D_WSTAG948_800A62A8, &D_WSTAG948_800A62B4, &D_WSTAG948_800A62C0 },
};
FieldstgListedBattle D_WSTAG948_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6344 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG948_800A6350 = {
    0,
    { &D_WSTAG948_800A62F0, &D_WSTAG948_800A62FC, &D_WSTAG948_800A6308, &D_WSTAG948_800A6314, &D_WSTAG948_800A6320,
        &D_WSTAG948_800A632C, &D_WSTAG948_800A6338, &D_WSTAG948_800A6344 },
};
FieldstgListedBattle D_WSTAG948_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG948_800A63C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG948_800A63D4 = {
    0,
    { &D_WSTAG948_800A6374, &D_WSTAG948_800A6380, &D_WSTAG948_800A638C, &D_WSTAG948_800A6398, &D_WSTAG948_800A63A4,
        &D_WSTAG948_800A63B0, &D_WSTAG948_800A63BC, &D_WSTAG948_800A63C8 },
};
FieldstgBattleLists wstag948_battle_lists = {
    386, 0, 0, { &D_WSTAG948_800A6248, &D_WSTAG948_800A62CC, &D_WSTAG948_800A6350 }, &D_WSTAG948_800A63D4,
};
