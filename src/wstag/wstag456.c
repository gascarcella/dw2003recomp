#include "wstag.h"

/* WSTAG456: stage 0x2A3 (fieldstg_stages). */

extern WstagFuncs wstag456_funcs;
extern CVECTOR wstag456_color;
extern FieldstgBattleLists wstag456_battle_lists;
extern FieldstgVramPlace wstag456_vram_places[];
extern FieldstgPlacedActor *wstag456_actors[];
extern FieldstgSprite wstag456_sprites[];
extern FieldstgMapEvent wstag456_map_events[];

void wstag456_update(WstagObject *obj) {
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

WstagObject *wstag456_start(void *arg0) {
    WstagObject *obj = object_new(wstag456_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag456_funcs.setup();
    return obj;
}

void wstag456_setup(void) {
    fieldstg_stage.background_file = 0x6E8;
    fieldstg_stage.sprite_file = 0x06E90000;
    fieldstg_stage.sprites = wstag456_sprites;
    fieldstg_stage.map_events = wstag456_map_events;
    fieldstg_stage.mask_file = 0x6E7;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15E00, 0xD800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag456_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag456_actors;
    fieldstg_stage.color = wstag456_color;
    fieldstg_stage.battle_lists = &wstag456_battle_lists;
    fieldstg_attr.set_file(0, 0x06E90001);
    fieldstg_attr.set_file(7, 0x06E90002);
    fieldstg_attr.set_file(4, 0x06E90003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag456/nonmatchings/wstag456", wstag456_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag456_setup(void);

FieldstgListedBattle D_WSTAG456_800A5FB0 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FBC = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FC8 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FD4 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FE0 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FEC = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A5FF8 = { 159, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A6004 = { 159, 8, 0x60080000 };
FieldstgBattleList D_WSTAG456_800A6010 = {
    3,
    { &D_WSTAG456_800A5FB0, &D_WSTAG456_800A5FBC, &D_WSTAG456_800A5FC8, &D_WSTAG456_800A5FD4, &D_WSTAG456_800A5FE0,
        &D_WSTAG456_800A5FEC, &D_WSTAG456_800A5FF8, &D_WSTAG456_800A6004 },
};
FieldstgListedBattle D_WSTAG456_800A6034 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6040 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A604C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6058 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6064 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6088 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG456_800A6094 = {
    0,
    { &D_WSTAG456_800A6034, &D_WSTAG456_800A6040, &D_WSTAG456_800A604C, &D_WSTAG456_800A6058, &D_WSTAG456_800A6064,
        &D_WSTAG456_800A6070, &D_WSTAG456_800A607C, &D_WSTAG456_800A6088 },
};
FieldstgListedBattle D_WSTAG456_800A60B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A60C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A60D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A610C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG456_800A6118 = {
    0,
    { &D_WSTAG456_800A60B8, &D_WSTAG456_800A60C4, &D_WSTAG456_800A60D0, &D_WSTAG456_800A60DC, &D_WSTAG456_800A60E8,
        &D_WSTAG456_800A60F4, &D_WSTAG456_800A6100, &D_WSTAG456_800A610C },
};
FieldstgListedBattle D_WSTAG456_800A613C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6160 = { 331, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A616C = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG456_800A6184 = { 177, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG456_800A6190 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG456_800A619C = {
    0,
    { &D_WSTAG456_800A613C, &D_WSTAG456_800A6148, &D_WSTAG456_800A6154, &D_WSTAG456_800A6160, &D_WSTAG456_800A616C,
        &D_WSTAG456_800A6178, &D_WSTAG456_800A6184, &D_WSTAG456_800A6190 },
};
FieldstgBattleLists wstag456_battle_lists = {
    73, 0, 0, { &D_WSTAG456_800A6010, &D_WSTAG456_800A6094, &D_WSTAG456_800A6118 }, &D_WSTAG456_800A619C,
};
FieldstgVramPlace wstag456_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 356, 397, 144, 141, 336, 511 }, { 320, 256, 364, 397, 176, 141, 352, 511 },
    { 320, 256, 356, 437, 144, 181, 368, 511 }, { 320, 256, 364, 437, 176, 181, 320, 510 },
    { 320, 256, 372, 445, 208, 189, 336, 510 }, { 320, 256, 320, 448, 0, 192, 352, 510 },
};
FieldstgTalk D_WSTAG456_800A629C[2] = { { NULL, NULL, 225 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A62B4[2] = { { NULL, NULL, 228 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A62CC[2] = { { NULL, NULL, 231 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A62E4[2] = { { NULL, NULL, 230 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A62FC[2] = { { NULL, NULL, 232 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A6314[2] = { { NULL, NULL, 224 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A632C[2] = { { NULL, NULL, 226 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A6344[2] = { { NULL, NULL, 227 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG456_800A635C[2] = { { NULL, NULL, 229 }, { NULL, NULL, 0 } };
u16 D_WSTAG456_800A6374[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A6380[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A638C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A6398[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG456_800A63A4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A63AC[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A63B8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG456_800A63C0[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG456_800A63CC[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG456_800A63D4 = { D_WSTAG456_800A6374, D_WSTAG456_800A629C, 47, 4, 993, 520, 1 };
FieldstgPlacedActor D_WSTAG456_800A63E8 = { D_WSTAG456_800A6380, D_WSTAG456_800A62B4, 51, 5, 881, 311, 7 };
FieldstgPlacedActor D_WSTAG456_800A63FC = { D_WSTAG456_800A638C, D_WSTAG456_800A62CC, 58, 6, 337, 255, 7 };
FieldstgPlacedActor D_WSTAG456_800A6410 = { D_WSTAG456_800A6398, D_WSTAG456_800A62E4, 157, 7, 337, 255, 7 };
FieldstgPlacedActor D_WSTAG456_800A6424 = { D_WSTAG456_800A63A4, D_WSTAG456_800A62FC, 157, 7, 337, 255, 7 };
FieldstgPlacedActor D_WSTAG456_800A6438 = { D_WSTAG456_800A63AC, D_WSTAG456_800A6314, 158, 8, 993, 520, 1 };
FieldstgPlacedActor D_WSTAG456_800A644C = { D_WSTAG456_800A63B8, D_WSTAG456_800A632C, 158, 8, 993, 520, 1 };
FieldstgPlacedActor D_WSTAG456_800A6460 = { D_WSTAG456_800A63C0, D_WSTAG456_800A6344, 159, 9, 881, 311, 7 };
FieldstgPlacedActor D_WSTAG456_800A6474 = { D_WSTAG456_800A63CC, D_WSTAG456_800A635C, 159, 9, 881, 311, 7 };
FieldstgPlacedActor *wstag456_actors[10] = {
    &D_WSTAG456_800A63D4, &D_WSTAG456_800A63E8, &D_WSTAG456_800A63FC, &D_WSTAG456_800A6410, &D_WSTAG456_800A6424,
    &D_WSTAG456_800A6438, &D_WSTAG456_800A644C, &D_WSTAG456_800A6460, &D_WSTAG456_800A6474, NULL,
};
FieldstgSprite wstag456_sprites[23] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 250, 247, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 267, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 506, 695, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 534, 116, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1103, 550, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1245, 347, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 147, 165, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 515, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 593, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 705, 666, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 769, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 913, 605, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1003, 377, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1089, 108, 0, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 979, 425, 436, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 674, 460, 490, 0 },
    { 1, 0, 0xDA, 4, 0, 0, 0, 0, 0, 0, 923, 72, 210, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 392, 185, 185, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 481, 549, 549, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 545, 613, 613, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 817, 189, 189, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 865, 165, 165, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag456_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A2, 0x570, 0x3B8, 3, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0xFFEC, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag456_funcs = { wstag456_setup };
