#include "wstag.h"

/* WSTAG370: stage 0x225 (fieldstg_stages). */

extern WstagFuncs wstag370_funcs;
extern FieldstgBattleLists wstag370_battle_lists;
extern FieldstgBattleLists wstag370_battle_lists2;
extern FieldstgVramPlace wstag370_vram_places[];
extern FieldstgPlacedActor *wstag370_actors[];
extern FieldstgSprite wstag370_sprites[];
extern FieldstgMapEvent wstag370_map_events[];

void wstag370_update(WstagObject *obj) {
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

WstagObject *wstag370_start(void *arg0) {
    WstagObject *obj = object_new(wstag370_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag370_funcs.setup();
    return obj;
}

void wstag370_setup(void) {
    fieldstg_stage.background_file = 0x1BC;
    fieldstg_stage.sprite_file = 0x01BD0000;
    fieldstg_stage.sprites = wstag370_sprites;
    fieldstg_stage.map_events = wstag370_map_events;
    fieldstg_stage.mask_file = 0x3D8;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19B00, 0x23500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag370_vram_places;
    fieldstg_stage.music = 0x32;
    fieldstg_stage.sound = 0x60C80000;
    fieldstg_stage.actors = wstag370_actors;
    fieldstg_attr.set_file(0, 0x01BD0001);
    fieldstg_attr.set_file(7, 0x01BD0002);
    fieldstg_attr.set_file(4, 0x01BD0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0xE) {
        fieldstg_stage.battle_lists = &wstag370_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag370_battle_lists2;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag370_setup(void);

FieldstgListedBattle D_WSTAG370_800A5FBC = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A5FC8 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A5FD4 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A5FE0 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A5FEC = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A5FF8 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6004 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6010 = { 44, 3, 0x60080000 };
FieldstgBattleList D_WSTAG370_800A601C = {
    3,
    { &D_WSTAG370_800A5FBC, &D_WSTAG370_800A5FC8, &D_WSTAG370_800A5FD4, &D_WSTAG370_800A5FE0, &D_WSTAG370_800A5FEC,
        &D_WSTAG370_800A5FF8, &D_WSTAG370_800A6004, &D_WSTAG370_800A6010 },
};
FieldstgListedBattle D_WSTAG370_800A6040 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A604C = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6058 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6064 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6070 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A607C = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6088 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6094 = { 45, 8, 0x60080000 };
FieldstgBattleList D_WSTAG370_800A60A0 = {
    1,
    { &D_WSTAG370_800A6040, &D_WSTAG370_800A604C, &D_WSTAG370_800A6058, &D_WSTAG370_800A6064, &D_WSTAG370_800A6070,
        &D_WSTAG370_800A607C, &D_WSTAG370_800A6088, &D_WSTAG370_800A6094 },
};
FieldstgListedBattle D_WSTAG370_800A60C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A60D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6118 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG370_800A6124 = {
    0,
    { &D_WSTAG370_800A60C4, &D_WSTAG370_800A60D0, &D_WSTAG370_800A60DC, &D_WSTAG370_800A60E8, &D_WSTAG370_800A60F4,
        &D_WSTAG370_800A6100, &D_WSTAG370_800A610C, &D_WSTAG370_800A6118 },
};
FieldstgListedBattle D_WSTAG370_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A619C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG370_800A61A8 = {
    0,
    { &D_WSTAG370_800A6148, &D_WSTAG370_800A6154, &D_WSTAG370_800A6160, &D_WSTAG370_800A616C, &D_WSTAG370_800A6178,
        &D_WSTAG370_800A6184, &D_WSTAG370_800A6190, &D_WSTAG370_800A619C },
};
FieldstgListedBattle D_WSTAG370_800A61CC = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A61D8 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A61E4 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A61F0 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A61FC = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6208 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6214 = { 44, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6220 = { 44, 3, 0x60080000 };
FieldstgBattleList D_WSTAG370_800A622C = {
    3,
    { &D_WSTAG370_800A61CC, &D_WSTAG370_800A61D8, &D_WSTAG370_800A61E4, &D_WSTAG370_800A61F0, &D_WSTAG370_800A61FC,
        &D_WSTAG370_800A6208, &D_WSTAG370_800A6214, &D_WSTAG370_800A6220 },
};
FieldstgListedBattle D_WSTAG370_800A6250 = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A625C = { 45, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6268 = { 57, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6274 = { 57, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6280 = { 57, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A628C = { 57, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A6298 = { 57, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG370_800A62A4 = { 57, 8, 0x60080000 };
FieldstgBattleList D_WSTAG370_800A62B0 = {
    1,
    { &D_WSTAG370_800A6250, &D_WSTAG370_800A625C, &D_WSTAG370_800A6268, &D_WSTAG370_800A6274, &D_WSTAG370_800A6280,
        &D_WSTAG370_800A628C, &D_WSTAG370_800A6298, &D_WSTAG370_800A62A4 },
};
FieldstgListedBattle D_WSTAG370_800A62D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6328 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG370_800A6334 = {
    0,
    { &D_WSTAG370_800A62D4, &D_WSTAG370_800A62E0, &D_WSTAG370_800A62EC, &D_WSTAG370_800A62F8, &D_WSTAG370_800A6304,
        &D_WSTAG370_800A6310, &D_WSTAG370_800A631C, &D_WSTAG370_800A6328 },
};
FieldstgListedBattle D_WSTAG370_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG370_800A63AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG370_800A63B8 = {
    0,
    { &D_WSTAG370_800A6358, &D_WSTAG370_800A6364, &D_WSTAG370_800A6370, &D_WSTAG370_800A637C, &D_WSTAG370_800A6388,
        &D_WSTAG370_800A6394, &D_WSTAG370_800A63A0, &D_WSTAG370_800A63AC },
};
FieldstgBattleLists wstag370_battle_lists = {
    10, 0, 0, { &D_WSTAG370_800A601C, &D_WSTAG370_800A60A0, &D_WSTAG370_800A6124 }, &D_WSTAG370_800A61A8,
};
FieldstgBattleLists wstag370_battle_lists2 = {
    29, 1, 0, { &D_WSTAG370_800A622C, &D_WSTAG370_800A62B0, &D_WSTAG370_800A6334 }, &D_WSTAG370_800A63B8,
};
FieldstgVramPlace wstag370_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 256, 128, 0, 320, 511 },
};
FieldstgTalk D_WSTAG370_800A6484[2] = { { NULL, NULL, 816 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG370_800A649C[2] = { { NULL, NULL, 816 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG370_800A64B4[2] = { { NULL, NULL, 816 }, { NULL, NULL, 0 } };
u16 D_WSTAG370_800A64CC[4] = { 0x6005, 1, 0xFFFF, 0 };
u16 D_WSTAG370_800A64D4[4] = { 0x6006, 1, 0xFFFF, 0 };
u16 D_WSTAG370_800A64DC[4] = { 0x6007, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG370_800A64E4 = { D_WSTAG370_800A64CC, D_WSTAG370_800A6484, 178, 4, 320, 736, 3 };
FieldstgPlacedActor D_WSTAG370_800A64F8 = { D_WSTAG370_800A64D4, D_WSTAG370_800A649C, 178, 4, 320, 736, 3 };
FieldstgPlacedActor D_WSTAG370_800A650C = { D_WSTAG370_800A64DC, D_WSTAG370_800A64B4, 178, 4, 320, 736, 3 };
FieldstgPlacedActor *wstag370_actors[4] = {
    &D_WSTAG370_800A64E4, &D_WSTAG370_800A64F8, &D_WSTAG370_800A650C, NULL,
};
FieldstgSprite wstag370_sprites[9] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 1024, 448, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 1088, 448, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 896, 576, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 960, 576, 0, 0 },
    { 1, 0, 0x40, 2, 4, 0, 0, 0, 0, 0, 1024, 576, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 1088, 576, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 1024, 640, 0, 0 }, { 1, 0, 0x40, 2, 7, 0, 0, 0, 0, 0, 1088, 640, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag370_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x226, 0x5E, 0x40E, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x222, 0x374, 0x9B, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag370_funcs = { wstag370_setup };
