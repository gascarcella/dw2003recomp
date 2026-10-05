#include "wstag.h"

/* WSTAG576: stage 0x2B6 (fieldstg_stages). */

extern WstagFuncs wstag576_funcs;
extern FieldstgBattleLists wstag576_battle_lists;
extern FieldstgVramPlace wstag576_vram_places[];
extern FieldstgPlacedActor *wstag576_actors[];
extern FieldstgSprite wstag576_sprites[];
extern FieldstgMapEvent wstag576_map_events[];

void wstag576_update(WstagObject *obj) {
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

WstagObject *wstag576_start(void *arg0) {
    WstagObject *obj = object_new(wstag576_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag576_funcs.setup();
    return obj;
}

void wstag576_setup(void) {
    fieldstg_stage.background_file = 0x5DD;
    fieldstg_stage.sprite_file = 0x05DE0000;
    fieldstg_stage.sprites = wstag576_sprites;
    fieldstg_stage.map_events = wstag576_map_events;
    fieldstg_stage.mask_file = 0x5DC;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x20000, 0xA600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag576_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag576_actors;
    fieldstg_stage.battle_lists = &wstag576_battle_lists;
    fieldstg_attr.set_file(0, 0x05DE0001);
    fieldstg_attr.set_file(7, 0x05DE0002);
    fieldstg_attr.set_file(4, 0x05DE0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag576_setup(void);

FieldstgListedBattle D_WSTAG576_800A5F8C = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5F98 = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FA4 = { 98, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FB0 = { 130, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FBC = { 130, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FC8 = { 131, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FD4 = { 131, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A5FE0 = { 131, 3, 0x60080000 };
FieldstgBattleList D_WSTAG576_800A5FEC = {
    3,
    { &D_WSTAG576_800A5F8C, &D_WSTAG576_800A5F98, &D_WSTAG576_800A5FA4, &D_WSTAG576_800A5FB0, &D_WSTAG576_800A5FBC,
        &D_WSTAG576_800A5FC8, &D_WSTAG576_800A5FD4, &D_WSTAG576_800A5FE0 },
};
FieldstgListedBattle D_WSTAG576_800A6010 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A601C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6028 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6034 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6040 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A604C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6058 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6064 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG576_800A6070 = {
    0,
    { &D_WSTAG576_800A6010, &D_WSTAG576_800A601C, &D_WSTAG576_800A6028, &D_WSTAG576_800A6034, &D_WSTAG576_800A6040,
        &D_WSTAG576_800A604C, &D_WSTAG576_800A6058, &D_WSTAG576_800A6064 },
};
FieldstgListedBattle D_WSTAG576_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A60E8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG576_800A60F4 = {
    0,
    { &D_WSTAG576_800A6094, &D_WSTAG576_800A60A0, &D_WSTAG576_800A60AC, &D_WSTAG576_800A60B8, &D_WSTAG576_800A60C4,
        &D_WSTAG576_800A60D0, &D_WSTAG576_800A60DC, &D_WSTAG576_800A60E8 },
};
FieldstgListedBattle D_WSTAG576_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A613C = { 333, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG576_800A6160 = { 175, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG576_800A616C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG576_800A6178 = {
    0,
    { &D_WSTAG576_800A6118, &D_WSTAG576_800A6124, &D_WSTAG576_800A6130, &D_WSTAG576_800A613C, &D_WSTAG576_800A6148,
        &D_WSTAG576_800A6154, &D_WSTAG576_800A6160, &D_WSTAG576_800A616C },
};
FieldstgBattleLists wstag576_battle_lists = {
    100, 0, 0, { &D_WSTAG576_800A5FEC, &D_WSTAG576_800A6070, &D_WSTAG576_800A60F4 }, &D_WSTAG576_800A6178,
};
FieldstgVramPlace wstag576_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 280, 200, 24, 336, 511 }, { 320, 256, 370, 320, 200, 64, 352, 511 },
    { 320, 256, 374, 422, 216, 166, 368, 511 }, { 320, 256, 348, 438, 112, 182, 336, 510 },
};
FieldstgTalk D_WSTAG576_800A6258[2] = { { NULL, NULL, 450 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG576_800A6270[2] = { { NULL, NULL, 452 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG576_800A6288[2] = { { NULL, NULL, 451 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG576_800A62A0[2] = { { NULL, NULL, 451 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG576_800A62B8[2] = { { NULL, NULL, 453 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG576_800A62D0[2] = { { NULL, NULL, 453 }, { NULL, NULL, 0 } };
u16 D_WSTAG576_800A62E8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG576_800A62F4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG576_800A6300[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG576_800A630C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG576_800A6314[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG576_800A6320[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG576_800A6328 = { D_WSTAG576_800A62E8, D_WSTAG576_800A6258, 47, 4, 528, 801, 1 };
FieldstgPlacedActor D_WSTAG576_800A633C = { D_WSTAG576_800A62F4, D_WSTAG576_800A6270, 52, 5, 608, 967, 5 };
FieldstgPlacedActor D_WSTAG576_800A6350 = { D_WSTAG576_800A6300, D_WSTAG576_800A6288, 157, 6, 528, 801, 1 };
FieldstgPlacedActor D_WSTAG576_800A6364 = { D_WSTAG576_800A630C, D_WSTAG576_800A62A0, 157, 6, 528, 801, 1 };
FieldstgPlacedActor D_WSTAG576_800A6378 = { D_WSTAG576_800A6314, D_WSTAG576_800A62B8, 158, 7, 608, 967, 5 };
FieldstgPlacedActor D_WSTAG576_800A638C = { D_WSTAG576_800A6320, D_WSTAG576_800A62D0, 158, 7, 608, 967, 5 };
FieldstgPlacedActor *wstag576_actors[7] = {
    &D_WSTAG576_800A6328, &D_WSTAG576_800A633C, &D_WSTAG576_800A6350, &D_WSTAG576_800A6364, &D_WSTAG576_800A6378,
    &D_WSTAG576_800A638C, NULL,
};
FieldstgSprite wstag576_sprites[41] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 166, 265, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 186, 261, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 186, 277, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 195, 287, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 207, 278, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 208, 273, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 217, 281, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 223, 291, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 237, 301, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 244, 290, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 264, 305, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 532, 182, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 551, 182, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 553, 172, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 496, 200, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 520, 201, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 526, 192, 0, 0 }, { 1, 0, 0x64, 2, 0xE, 0, 0, 0, 0, 0, 698, 491, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 490, 26, 0, 0 }, { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 528, 45, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 567, 64, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 418, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 421, 128, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 434, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 450, 120, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 384, 141, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 391, 147, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 392, 125, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 414, 137, 0, 0 }, { 1, 0, 0x73, 4, 0, 0, 0, 0, 0, 0, 349, 733, 824, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 444, 766, 777, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 310, 576, 599, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 487, 596, 610, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 592, 118, 160, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 103, 411, 477, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 336, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 384, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 895, 895, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 636, 800, 800, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 673, 767, 767, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag576_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B4, 0x5D8, 0x104, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x88, 0x3F4, 5, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x19, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag576_funcs = { wstag576_setup };
