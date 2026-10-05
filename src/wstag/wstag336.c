#include "wstag.h"

/* WSTAG336: stage 0x28D (fieldstg_stages). */

extern WstagFuncs wstag336_funcs;
extern FieldstgBattleLists wstag336_battle_lists;
extern FieldstgVramPlace wstag336_vram_places[];
extern FieldstgPlacedActor *wstag336_actors[];
extern FieldstgSprite wstag336_sprites[];
extern FieldstgMapEvent wstag336_map_events[];

void wstag336_update(WstagObject *obj) {
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

WstagObject *wstag336_start(void *arg0) {
    WstagObject *obj = object_new(wstag336_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag336_funcs.setup();
    return obj;
}

void wstag336_setup(void) {
    fieldstg_stage.background_file = 0x570;
    fieldstg_stage.sprite_file = 0x05710000;
    fieldstg_stage.sprites = wstag336_sprites;
    fieldstg_stage.map_events = wstag336_map_events;
    fieldstg_stage.mask_file = 0x56F;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xE400, 0xA600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag336_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag336_actors;
    fieldstg_stage.battle_lists = &wstag336_battle_lists;
    fieldstg_attr.set_file(0, 0x05710001);
    fieldstg_attr.set_file(7, 0x05710002);
    fieldstg_attr.set_file(4, 0x05710003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag336_setup(void);

FieldstgListedBattle D_WSTAG336_800A5FD4 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A5FE0 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A5FEC = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A5FF8 = { 93, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6004 = { 127, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6010 = { 127, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A601C = { 127, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6028 = { 127, 1, 0x60080000 };
FieldstgBattleList D_WSTAG336_800A6034 = {
    3,
    { &D_WSTAG336_800A5FD4, &D_WSTAG336_800A5FE0, &D_WSTAG336_800A5FEC, &D_WSTAG336_800A5FF8, &D_WSTAG336_800A6004,
        &D_WSTAG336_800A6010, &D_WSTAG336_800A601C, &D_WSTAG336_800A6028 },
};
FieldstgListedBattle D_WSTAG336_800A6058 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6064 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6070 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A607C = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6088 = { 127, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6094 = { 127, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A60A0 = { 127, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A60AC = { 127, 13, 0x60080000 };
FieldstgBattleList D_WSTAG336_800A60B8 = {
    3,
    { &D_WSTAG336_800A6058, &D_WSTAG336_800A6064, &D_WSTAG336_800A6070, &D_WSTAG336_800A607C, &D_WSTAG336_800A6088,
        &D_WSTAG336_800A6094, &D_WSTAG336_800A60A0, &D_WSTAG336_800A60AC },
};
FieldstgListedBattle D_WSTAG336_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6130 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG336_800A613C = {
    0,
    { &D_WSTAG336_800A60DC, &D_WSTAG336_800A60E8, &D_WSTAG336_800A60F4, &D_WSTAG336_800A6100, &D_WSTAG336_800A610C,
        &D_WSTAG336_800A6118, &D_WSTAG336_800A6124, &D_WSTAG336_800A6130 },
};
FieldstgListedBattle D_WSTAG336_800A6160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A6184 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A6190 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG336_800A61A8 = { 93, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG336_800A61B4 = { 180, 8, 0x60080000 };
FieldstgBattleList D_WSTAG336_800A61C0 = {
    0,
    { &D_WSTAG336_800A6160, &D_WSTAG336_800A616C, &D_WSTAG336_800A6178, &D_WSTAG336_800A6184, &D_WSTAG336_800A6190,
        &D_WSTAG336_800A619C, &D_WSTAG336_800A61A8, &D_WSTAG336_800A61B4 },
};
FieldstgBattleLists wstag336_battle_lists = {
    93, 0, 0, { &D_WSTAG336_800A6034, &D_WSTAG336_800A60B8, &D_WSTAG336_800A613C }, &D_WSTAG336_800A61C0,
};
FieldstgVramPlace wstag336_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 280, 152, 24, 336, 511 }, { 320, 256, 366, 280, 184, 24, 352, 511 },
    { 320, 256, 374, 280, 216, 24, 368, 511 }, { 320, 256, 340, 304, 80, 48, 320, 510 },
};
FieldstgTalk D_WSTAG336_800A62A0[2] = { { NULL, NULL, 47 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG336_800A62B8[2] = { { NULL, NULL, 44 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG336_800A62D0[2] = { { NULL, NULL, 46 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG336_800A62E8[2] = { { NULL, NULL, 48 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG336_800A6300[2] = { { NULL, NULL, 43 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG336_800A6318[2] = { { NULL, NULL, 45 }, { NULL, NULL, 0 } };
u16 D_WSTAG336_800A6330[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG336_800A633C[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG336_800A6348[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG336_800A6354[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG336_800A635C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG336_800A6368[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG336_800A6370 = { D_WSTAG336_800A6330, D_WSTAG336_800A62A0, 48, 4, 391, 309, 7 };
FieldstgPlacedActor D_WSTAG336_800A6384 = { D_WSTAG336_800A633C, D_WSTAG336_800A62B8, 51, 5, 361, 684, 1 };
FieldstgPlacedActor D_WSTAG336_800A6398 = { D_WSTAG336_800A6348, D_WSTAG336_800A62D0, 157, 6, 391, 309, 7 };
FieldstgPlacedActor D_WSTAG336_800A63AC = { D_WSTAG336_800A6354, D_WSTAG336_800A62E8, 157, 6, 391, 309, 7 };
FieldstgPlacedActor D_WSTAG336_800A63C0 = { D_WSTAG336_800A635C, D_WSTAG336_800A6300, 158, 7, 361, 684, 1 };
FieldstgPlacedActor D_WSTAG336_800A63D4 = { D_WSTAG336_800A6368, D_WSTAG336_800A6318, 158, 7, 361, 684, 1 };
FieldstgPlacedActor *wstag336_actors[7] = {
    &D_WSTAG336_800A6370, &D_WSTAG336_800A6384, &D_WSTAG336_800A6398, &D_WSTAG336_800A63AC, &D_WSTAG336_800A63C0,
    &D_WSTAG336_800A63D4, NULL,
};
FieldstgSprite wstag336_sprites[23] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 197, 606, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 351, -16, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 511, 284, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 27, 509, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 255, 408, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 28, 586, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 160, 515, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 183, 378, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 226, 226, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 232, 250, 250, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 243, 544, 544, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 267, 602, 602, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 271, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 277, 521, 521, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 399, 167, 167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 422, 187, 187, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 431, 568, 568, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 489, 501, 501, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 506, 203, 203, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 542, 228, 228, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 641, 286, 286, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag336_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x5F4, 0x39E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x290, 0x138, 0x7A, 7, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC8, 0xFFF0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag336_funcs = { wstag336_setup };
