#include "wstag.h"

/* WSTAG566: stage 0x2B4 (fieldstg_stages). */

extern WstagFuncs wstag566_funcs;
extern FieldstgBattleLists wstag566_battle_lists;
extern FieldstgVramPlace wstag566_vram_places[];
extern FieldstgPlacedActor *wstag566_actors[];
extern FieldstgSprite wstag566_sprites[];
extern FieldstgMapEvent wstag566_map_events[];

void wstag566_update(WstagObject *obj) {
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

WstagObject *wstag566_start(void *arg0) {
    WstagObject *obj = object_new(wstag566_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag566_funcs.setup();
    return obj;
}

void wstag566_setup(void) {
    fieldstg_stage.background_file = 0x6C3;
    fieldstg_stage.sprite_file = 0x06C40000;
    fieldstg_stage.sprites = wstag566_sprites;
    fieldstg_stage.map_events = wstag566_map_events;
    fieldstg_stage.mask_file = 0x6C2;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x50500, 0x38700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag566_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag566_actors;
    fieldstg_stage.battle_lists = &wstag566_battle_lists;
    fieldstg_attr.set_file(0, 0x06C40001);
    fieldstg_attr.set_file(7, 0x06C40002);
    fieldstg_attr.set_file(4, 0x06C40003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag566_setup(void);

FieldstgListedBattle D_WSTAG566_800A5FDC = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A5FE8 = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A5FF4 = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6000 = { 130, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A600C = { 130, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6018 = { 131, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6024 = { 131, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6030 = { 131, 3, 0x60080000 };
FieldstgBattleList D_WSTAG566_800A603C = {
    3,
    { &D_WSTAG566_800A5FDC, &D_WSTAG566_800A5FE8, &D_WSTAG566_800A5FF4, &D_WSTAG566_800A6000, &D_WSTAG566_800A600C,
        &D_WSTAG566_800A6018, &D_WSTAG566_800A6024, &D_WSTAG566_800A6030 },
};
FieldstgListedBattle D_WSTAG566_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A606C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6078 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A60B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG566_800A60C0 = {
    0,
    { &D_WSTAG566_800A6060, &D_WSTAG566_800A606C, &D_WSTAG566_800A6078, &D_WSTAG566_800A6084, &D_WSTAG566_800A6090,
        &D_WSTAG566_800A609C, &D_WSTAG566_800A60A8, &D_WSTAG566_800A60B4 },
};
FieldstgListedBattle D_WSTAG566_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A6138 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG566_800A6144 = {
    0,
    { &D_WSTAG566_800A60E4, &D_WSTAG566_800A60F0, &D_WSTAG566_800A60FC, &D_WSTAG566_800A6108, &D_WSTAG566_800A6114,
        &D_WSTAG566_800A6120, &D_WSTAG566_800A612C, &D_WSTAG566_800A6138 },
};
FieldstgListedBattle D_WSTAG566_800A6168 = { 236, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6174 = { 284, 3, 0x600C0000 };
FieldstgListedBattle D_WSTAG566_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A618C = { 333, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG566_800A61B0 = { 175, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG566_800A61BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG566_800A61C8 = {
    0,
    { &D_WSTAG566_800A6168, &D_WSTAG566_800A6174, &D_WSTAG566_800A6180, &D_WSTAG566_800A618C, &D_WSTAG566_800A6198,
        &D_WSTAG566_800A61A4, &D_WSTAG566_800A61B0, &D_WSTAG566_800A61BC },
};
FieldstgBattleLists wstag566_battle_lists = {
    97, 0, 0, { &D_WSTAG566_800A603C, &D_WSTAG566_800A60C0, &D_WSTAG566_800A6144 }, &D_WSTAG566_800A61C8,
};
FieldstgVramPlace wstag566_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 296, 216, 40, 336, 511 }, { 320, 256, 374, 256, 216, 0, 352, 511 },
    { 320, 256, 364, 326, 176, 70, 368, 511 },
};
u16 D_WSTAG566_800A6298[8] = { 0x24A, 1, 0x822C, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A62A8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A62B0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A62B8[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A62C4[8] = { 0x7207, 1, 0x7209, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A62D4[4] = { 0x7634, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A62DC[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE27, 0,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A62F0[6] = { 0x7400, 1, 0xE27, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A62FC[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE27, 1,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A6310[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A6318[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A6320[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A632C[8] = { 0, 1, 0x720A, 1, 0xE47, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A633C[6] = { 0xE47, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A6348[10] = {
    0, 1, 0x720A, 1, 0xE47, 1, 0x720C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A635C[10] = {
    0x720C, 1, 0, 1, 0x720A, 1, 0xE47, 1,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A6370[4] = { 0x7834, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A6378[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A6380[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A6388[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A6390[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A6398[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A63A4[8] = { 0x24B, 1, 0x8246, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG566_800A63B4[2] = { { NULL, D_WSTAG566_800A6298, 381 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG566_800A63CC[6] = {
    { D_WSTAG566_800A62A8, D_WSTAG566_800A62B0, 580 }, { D_WSTAG566_800A62B8, NULL, 581 },
    { D_WSTAG566_800A62C4, D_WSTAG566_800A62D4, 582 }, { D_WSTAG566_800A62DC, D_WSTAG566_800A62F0, 583 },
    { D_WSTAG566_800A62FC, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG566_800A6414[6] = {
    { D_WSTAG566_800A6310, D_WSTAG566_800A6318, 588 }, { D_WSTAG566_800A6320, NULL, 590 },
    { D_WSTAG566_800A632C, D_WSTAG566_800A633C, 589 }, { D_WSTAG566_800A6348, NULL, 584 },
    { D_WSTAG566_800A635C, D_WSTAG566_800A6370, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG566_800A645C[4] = {
    { D_WSTAG566_800A6378, NULL, 924 }, { D_WSTAG566_800A6380, D_WSTAG566_800A6388, 585 },
    { D_WSTAG566_800A6390, D_WSTAG566_800A6398, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG566_800A648C[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG566_800A64A4[2] = { { NULL, D_WSTAG566_800A63A4, 387 }, { NULL, NULL, 0 } };
u16 D_WSTAG566_800A64BC[4] = { 0x24A, 0, 0xFFFF, 0 };
u16 D_WSTAG566_800A64C4[10] = {
    0x7019, 1, 0x8192, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A64D8[10] = {
    0x8192, 1, 0x11, 0, 0x7019, 1, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG566_800A64EC[8] = { 0x7019, 1, 0x8192, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A64FC[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG566_800A6508[4] = { 0x24B, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG566_800A6510 = { D_WSTAG566_800A64BC, D_WSTAG566_800A63B4, 33, 4, 1057, 961, 1 };
FieldstgPlacedActor D_WSTAG566_800A6524 = { D_WSTAG566_800A64C4, D_WSTAG566_800A63CC, 69, 5, 784, 393, 7 };
FieldstgPlacedActor D_WSTAG566_800A6538 = { D_WSTAG566_800A64D8, D_WSTAG566_800A6414, 69, 5, 784, 393, 7 };
FieldstgPlacedActor D_WSTAG566_800A654C = { D_WSTAG566_800A64EC, D_WSTAG566_800A645C, 69, 5, 784, 393, 7 };
FieldstgPlacedActor D_WSTAG566_800A6560 = { D_WSTAG566_800A64FC, D_WSTAG566_800A648C, 69, 5, 784, 393, 7 };
FieldstgPlacedActor D_WSTAG566_800A6574 = { D_WSTAG566_800A6508, D_WSTAG566_800A64A4, 77, 6, 256, 561, 1 };
FieldstgPlacedActor *wstag566_actors[7] = {
    &D_WSTAG566_800A6510, &D_WSTAG566_800A6524, &D_WSTAG566_800A6538, &D_WSTAG566_800A654C, &D_WSTAG566_800A6560,
    &D_WSTAG566_800A6574, NULL,
};
FieldstgSprite wstag566_sprites[18] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 1234, 165, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 1279, 187, 0, 0 }, { 1, 0, 0x80, 2, 4, 0, 0, 0, 0, 0, 896, 149, 0, 0 },
    { 1, 0, 0x80, 2, 5, 0, 0, 0, 0, 0, 1239, 635, 0, 0 }, { 1, 0, 0x40, 6, 3, 0, 0, 0, 0, 0, 125, 286, 0, 0 },
    { 1, 0, 0x40, 6, 3, 0, 0, 0, 0, 0, 1409, 823, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1497, 247, 283, 0 },
    { 1, 0, 0x6C, 4, 1, 0, 0, 0, 0, 0, 920, 640, 699, 0 }, { 1, 0, 0x57, 4, 2, 0, 0, 0, 0, 0, 1040, 220, 285, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 512, 512, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 320, 320, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 336, 336, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 584, 584, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 936, 900, 900, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1136, 168, 168, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 896, 896, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1568, 479, 479, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag566_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B3, 0x294, 0x1F0, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B6, 0xA8, 0x3FC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B5, 0x90, 0xD0, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 9, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 4, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x1E, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x12, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag566_funcs = { wstag566_setup };
