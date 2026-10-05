#include "wstag.h"

/* WSTAG446: stage 0x2A1 (fieldstg_stages). */

extern WstagFuncs wstag446_funcs;
const CVECTOR wstag446_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag446_battle_lists;
extern FieldstgVramPlace wstag446_vram_places[];
extern FieldstgPlacedActor *wstag446_actors[];
extern FieldstgSprite wstag446_sprites[];
extern FieldstgMapEvent wstag446_map_events[];

void wstag446_update(WstagObject *obj) {
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

WstagObject *wstag446_start(void *arg0) {
    WstagObject *obj = object_new(wstag446_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag446_funcs.setup();
    return obj;
}

void wstag446_setup(void) {
    fieldstg_stage.background_file = 0x5A5;
    fieldstg_stage.sprite_file = 0x05A60000;
    fieldstg_stage.sprites = wstag446_sprites;
    fieldstg_stage.map_events = wstag446_map_events;
    fieldstg_stage.mask_file = 0x5A4;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10300, 0x13800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag446_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag446_actors;
    fieldstg_stage.color = wstag446_color;
    fieldstg_stage.battle_lists = &wstag446_battle_lists;
    fieldstg_attr.set_file(0, 0x05A60001);
    fieldstg_attr.set_file(7, 0x05A60002);
    fieldstg_attr.set_file(4, 0x05A60003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag446_setup(void);

FieldstgListedBattle D_WSTAG446_800A5FB4 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FC0 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FCC = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FD8 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FE4 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FF0 = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A5FFC = { 105, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A6008 = { 105, 8, 0x60080000 };
FieldstgBattleList D_WSTAG446_800A6014 = {
    3,
    { &D_WSTAG446_800A5FB4, &D_WSTAG446_800A5FC0, &D_WSTAG446_800A5FCC, &D_WSTAG446_800A5FD8, &D_WSTAG446_800A5FE4,
        &D_WSTAG446_800A5FF0, &D_WSTAG446_800A5FFC, &D_WSTAG446_800A6008 },
};
FieldstgListedBattle D_WSTAG446_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A608C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG446_800A6098 = {
    0,
    { &D_WSTAG446_800A6038, &D_WSTAG446_800A6044, &D_WSTAG446_800A6050, &D_WSTAG446_800A605C, &D_WSTAG446_800A6068,
        &D_WSTAG446_800A6074, &D_WSTAG446_800A6080, &D_WSTAG446_800A608C },
};
FieldstgListedBattle D_WSTAG446_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6110 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG446_800A611C = {
    0,
    { &D_WSTAG446_800A60BC, &D_WSTAG446_800A60C8, &D_WSTAG446_800A60D4, &D_WSTAG446_800A60E0, &D_WSTAG446_800A60EC,
        &D_WSTAG446_800A60F8, &D_WSTAG446_800A6104, &D_WSTAG446_800A6110 },
};
FieldstgListedBattle D_WSTAG446_800A6140 = { 229, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A614C = { 277, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG446_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6164 = { 331, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A6170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG446_800A6188 = { 177, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG446_800A6194 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG446_800A61A0 = {
    0,
    { &D_WSTAG446_800A6140, &D_WSTAG446_800A614C, &D_WSTAG446_800A6158, &D_WSTAG446_800A6164, &D_WSTAG446_800A6170,
        &D_WSTAG446_800A617C, &D_WSTAG446_800A6188, &D_WSTAG446_800A6194 },
};
FieldstgBattleLists wstag446_battle_lists = {
    70, 0, 0, { &D_WSTAG446_800A6014, &D_WSTAG446_800A6098, &D_WSTAG446_800A611C }, &D_WSTAG446_800A61A0,
};
FieldstgVramPlace wstag446_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 366, 368, 184, 112, 336, 511 }, { 320, 256, 358, 318, 152, 62, 352, 511 },
    { 320, 256, 366, 328, 184, 72, 368, 511 }, { 320, 256, 374, 328, 216, 72, 320, 510 },
    { 320, 256, 358, 358, 152, 102, 336, 510 },
};
u16 D_WSTAG446_800A6290[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6298[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A62A0[6] = { 0, 1, 0x7205, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A62AC[8] = { 0, 1, 0x7205, 1, 0x7208, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A62BC[4] = { 0x762D, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A62C4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE20, 0,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A62D8[6] = { 0x7400, 1, 0xE20, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A62E4[10] = {
    0, 1, 0x7205, 1, 0x7208, 1, 0xE20, 1,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A62F8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6300[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6308[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6314[8] = { 0, 1, 0x7209, 1, 0xE41, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6324[6] = { 0x7400, 1, 0xE41, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6330[10] = {
    0, 1, 0x7209, 1, 0xE41, 1, 0x720B, 0,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A6344[10] = {
    0, 1, 0x7209, 1, 0xE41, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A6358[4] = { 0x782D, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6360[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6368[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6370[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A6378[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6380[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG446_800A638C[2] = { { NULL, NULL, 205 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A63A4[2] = { { NULL, NULL, 202 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A63BC[6] = {
    { D_WSTAG446_800A6290, D_WSTAG446_800A6298, 580 }, { D_WSTAG446_800A62A0, NULL, 581 },
    { D_WSTAG446_800A62AC, D_WSTAG446_800A62BC, 582 }, { D_WSTAG446_800A62C4, D_WSTAG446_800A62D8, 583 },
    { D_WSTAG446_800A62E4, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG446_800A6404[6] = {
    { D_WSTAG446_800A62F8, D_WSTAG446_800A6300, 588 }, { D_WSTAG446_800A6308, NULL, 590 },
    { D_WSTAG446_800A6314, D_WSTAG446_800A6324, 589 }, { D_WSTAG446_800A6330, NULL, 584 },
    { D_WSTAG446_800A6344, D_WSTAG446_800A6358, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG446_800A644C[4] = {
    { D_WSTAG446_800A6360, NULL, 924 }, { D_WSTAG446_800A6368, D_WSTAG446_800A6370, 585 },
    { D_WSTAG446_800A6378, D_WSTAG446_800A6380, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG446_800A647C[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A6494[2] = { { NULL, NULL, 204 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A64AC[2] = { { NULL, NULL, 206 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A64C4[2] = { { NULL, NULL, 201 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG446_800A64DC[2] = { { NULL, NULL, 203 }, { NULL, NULL, 0 } };
u16 D_WSTAG446_800A64F4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6500[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A650C[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A6520[10] = {
    0x8192, 1, 0x7019, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG446_800A6534[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6544[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6550[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG446_800A655C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6564[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG446_800A6570[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG446_800A6578 = { D_WSTAG446_800A64F4, D_WSTAG446_800A638C, 45, 4, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG446_800A658C = { D_WSTAG446_800A6500, D_WSTAG446_800A63A4, 51, 5, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG446_800A65A0 = { D_WSTAG446_800A650C, D_WSTAG446_800A63BC, 69, 6, 232, 316, 7 };
FieldstgPlacedActor D_WSTAG446_800A65B4 = { D_WSTAG446_800A6520, D_WSTAG446_800A6404, 69, 6, 232, 316, 7 };
FieldstgPlacedActor D_WSTAG446_800A65C8 = { D_WSTAG446_800A6534, D_WSTAG446_800A644C, 69, 6, 232, 316, 7 };
FieldstgPlacedActor D_WSTAG446_800A65DC = { D_WSTAG446_800A6544, D_WSTAG446_800A647C, 69, 6, 232, 316, 7 };
FieldstgPlacedActor D_WSTAG446_800A65F0 = { D_WSTAG446_800A6550, D_WSTAG446_800A6494, 157, 7, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG446_800A6604 = { D_WSTAG446_800A655C, D_WSTAG446_800A64AC, 157, 7, 611, 171, 1 };
FieldstgPlacedActor D_WSTAG446_800A6618 = { D_WSTAG446_800A6564, D_WSTAG446_800A64C4, 158, 8, 595, 475, 1 };
FieldstgPlacedActor D_WSTAG446_800A662C = { D_WSTAG446_800A6570, D_WSTAG446_800A64DC, 158, 8, 595, 475, 1 };
FieldstgPlacedActor *wstag446_actors[11] = {
    &D_WSTAG446_800A6578, &D_WSTAG446_800A658C, &D_WSTAG446_800A65A0, &D_WSTAG446_800A65B4, &D_WSTAG446_800A65C8,
    &D_WSTAG446_800A65DC, &D_WSTAG446_800A65F0, &D_WSTAG446_800A6604, &D_WSTAG446_800A6618, &D_WSTAG446_800A662C,
    NULL,
};
FieldstgSprite wstag446_sprites[17] = {
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 89, 30, 0, 0 }, { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 694, 46, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 185, 413, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 428, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 405, 484, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 706, 428, 0, 0 },
    { 1, 0x64, 0x40, 6, 1, 0, 0, 0, 0, 0, 490, 108, 0, 0 }, { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 464, 77, 161, 0 },
    { 1, 0, 0x80, 4, 0xD, 0, 0, 0, 0, 0, 358, 119, 155, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 223, 167, 167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 248, 140, 140, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 295, 115, 115, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 599, 123, 123, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 313, 313, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 690, 375, 375, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag446_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A2, 0x668, 0xB4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A0, 0x190, 0x218, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag446_funcs = { wstag446_setup };
