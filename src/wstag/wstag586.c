#include "wstag.h"

/* WSTAG586: stage 0x2B8 (fieldstg_stages). */

extern WstagFuncs wstag586_funcs;
extern FieldstgBattleLists wstag586_battle_lists;
extern FieldstgVramPlace wstag586_vram_places[];
extern FieldstgPlacedActor *wstag586_actors[];
extern FieldstgSprite wstag586_sprites[];
extern FieldstgMapEvent wstag586_map_events[];

void wstag586_update(WstagObject *obj) {
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

WstagObject *wstag586_start(void *arg0) {
    WstagObject *obj = object_new(wstag586_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag586_funcs.setup();
    return obj;
}

void wstag586_setup(void) {
    fieldstg_stage.background_file = 0x5EA;
    fieldstg_stage.sprite_file = 0x05EB0000;
    fieldstg_stage.sprites = wstag586_sprites;
    fieldstg_stage.map_events = wstag586_map_events;
    fieldstg_stage.mask_file = 0x5E9;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12C00, 0x15400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag586_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag586_actors;
    fieldstg_stage.battle_lists = &wstag586_battle_lists;
    fieldstg_attr.set_file(0, 0x05EB0001);
    fieldstg_attr.set_file(7, 0x05EB0002);
    fieldstg_attr.set_file(4, 0x05EB0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag586_setup(void);

FieldstgListedBattle D_WSTAG586_800A5F94 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FA0 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FAC = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FB8 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FC4 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FD0 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FDC = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG586_800A5FE8 = { 116, 12, 0x60080000 };
FieldstgBattleList D_WSTAG586_800A5FF4 = {
    4,
    { &D_WSTAG586_800A5F94, &D_WSTAG586_800A5FA0, &D_WSTAG586_800A5FAC, &D_WSTAG586_800A5FB8, &D_WSTAG586_800A5FC4,
        &D_WSTAG586_800A5FD0, &D_WSTAG586_800A5FDC, &D_WSTAG586_800A5FE8 },
};
FieldstgListedBattle D_WSTAG586_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG586_800A6078 = {
    0,
    { &D_WSTAG586_800A6018, &D_WSTAG586_800A6024, &D_WSTAG586_800A6030, &D_WSTAG586_800A603C, &D_WSTAG586_800A6048,
        &D_WSTAG586_800A6054, &D_WSTAG586_800A6060, &D_WSTAG586_800A606C },
};
FieldstgListedBattle D_WSTAG586_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG586_800A60FC = {
    0,
    { &D_WSTAG586_800A609C, &D_WSTAG586_800A60A8, &D_WSTAG586_800A60B4, &D_WSTAG586_800A60C0, &D_WSTAG586_800A60CC,
        &D_WSTAG586_800A60D8, &D_WSTAG586_800A60E4, &D_WSTAG586_800A60F0 },
};
FieldstgListedBattle D_WSTAG586_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG586_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG586_800A6180 = {
    0,
    { &D_WSTAG586_800A6120, &D_WSTAG586_800A612C, &D_WSTAG586_800A6138, &D_WSTAG586_800A6144, &D_WSTAG586_800A6150,
        &D_WSTAG586_800A615C, &D_WSTAG586_800A6168, &D_WSTAG586_800A6174 },
};
FieldstgBattleLists wstag586_battle_lists = {
    107, 0, 0, { &D_WSTAG586_800A5FF4, &D_WSTAG586_800A6078, &D_WSTAG586_800A60FC }, &D_WSTAG586_800A6180,
};
FieldstgVramPlace wstag586_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 336, 511 }, { 320, 256, 344, 256, 96, 0, 352, 511 },
    { 320, 256, 352, 256, 128, 0, 368, 511 }, { 320, 256, 360, 256, 160, 0, 336, 510 },
    { 320, 256, 368, 256, 192, 0, 352, 510 }, { 320, 256, 320, 288, 0, 32, 368, 510 },
    { 320, 256, 328, 288, 32, 32, 336, 509 }, { 320, 256, 336, 288, 64, 32, 352, 509 },
};
u16 D_WSTAG586_800A62A0[8] = { 0x24C, 1, 0x8232, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A62B0[10] = {
    0x8259, 1, 0x825A, 1, 0x84A7, 1, 0x8007, 1,
    0xFFFF, 0,
};
u16 D_WSTAG586_800A62C4[12] = {
    0x8674, 1, 0x8680, 1, 0x868D, 1, 0x8666, 1,
    0x8699, 1, 0xFFFF, 0,
};
u16 D_WSTAG586_800A62DC[12] = {
    0x8463, 1, 0x8496, 1, 0x847D, 1, 0x848A, 1,
    0x8471, 1, 0xFFFF, 0,
};
u16 D_WSTAG586_800A62F4[12] = {
    0x8497, 1, 0x847E, 1, 0x848B, 1, 0x8464, 1,
    0x8472, 1, 0xFFFF, 0,
};
u16 D_WSTAG586_800A630C[12] = {
    0x8498, 1, 0x847F, 1, 0x848C, 1, 0x8465, 1,
    0x8473, 1, 0xFFFF, 0,
};
u16 D_WSTAG586_800A6324[4] = { 0x7092, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG586_800A632C[2] = { { NULL, D_WSTAG586_800A62A0, 388 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A6344[2] = { { NULL, D_WSTAG586_800A62B0, 927 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A635C[2] = { { NULL, D_WSTAG586_800A62C4, 928 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A6374[2] = { { NULL, D_WSTAG586_800A62DC, 929 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A638C[2] = { { NULL, D_WSTAG586_800A62F4, 930 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A63A4[2] = { { NULL, D_WSTAG586_800A630C, 931 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A63BC[2] = { { NULL, NULL, 932 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG586_800A63D4[2] = { { NULL, D_WSTAG586_800A6324, 926 }, { NULL, NULL, 0 } };
u16 D_WSTAG586_800A63EC[6] = { 0x24C, 0, 0x6000, 0, 0xFFFF, 0 };
u16 D_WSTAG586_800A63F8[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6400[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6408[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6410[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6418[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6420[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG586_800A6428[4] = { 0x6000, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG586_800A6430 = { D_WSTAG586_800A63EC, D_WSTAG586_800A632C, 33, 4, 379, 318, 1 };
FieldstgPlacedActor D_WSTAG586_800A6444 = { D_WSTAG586_800A63F8, D_WSTAG586_800A6344, 69, 5, 209, 329, 7 };
FieldstgPlacedActor D_WSTAG586_800A6458 = { D_WSTAG586_800A6400, D_WSTAG586_800A635C, 70, 6, 240, 313, 7 };
FieldstgPlacedActor D_WSTAG586_800A646C = { D_WSTAG586_800A6408, D_WSTAG586_800A6374, 71, 7, 272, 297, 7 };
FieldstgPlacedActor D_WSTAG586_800A6480 = { D_WSTAG586_800A6410, D_WSTAG586_800A638C, 72, 8, 354, 401, 3 };
FieldstgPlacedActor D_WSTAG586_800A6494 = { D_WSTAG586_800A6418, D_WSTAG586_800A63A4, 73, 9, 385, 385, 3 };
FieldstgPlacedActor D_WSTAG586_800A64A8 = { D_WSTAG586_800A6420, D_WSTAG586_800A63BC, 74, 10, 415, 369, 3 };
FieldstgPlacedActor D_WSTAG586_800A64BC = { D_WSTAG586_800A6428, D_WSTAG586_800A63D4, 152, 11, 379, 317, 1 };
FieldstgPlacedActor *wstag586_actors[9] = {
    &D_WSTAG586_800A6430, &D_WSTAG586_800A6444, &D_WSTAG586_800A6458, &D_WSTAG586_800A646C, &D_WSTAG586_800A6480,
    &D_WSTAG586_800A6494, &D_WSTAG586_800A64A8, &D_WSTAG586_800A64BC, NULL,
};
FieldstgSprite wstag586_sprites[3] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 277, 252, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 6, 0, 405, 317, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag586_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x358, 0x37C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag586_funcs = { wstag586_setup };
