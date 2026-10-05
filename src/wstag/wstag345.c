#include "wstag.h"

/* WSTAG345: stage 0x220 (fieldstg_stages). */

extern WstagFuncs wstag345_funcs;
extern CVECTOR wstag345_color;
extern FieldstgBattleLists wstag345_battle_lists[];
extern FieldstgVramPlace wstag345_vram_places[];
extern FieldstgPlacedActor *wstag345_actors[];
extern FieldstgSprite wstag345_sprites[];
extern FieldstgMapEvent wstag345_map_events[];
extern FieldstgEventDef wstag345_events[];

void wstag345_update(WstagObject *obj) {
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

WstagObject *wstag345_start(void *arg0) {
    WstagObject *obj = object_new(wstag345_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag345_funcs.setup();
    return obj;
}

void wstag345_event_1264_end(void) {
    gamestate_flags.set_flag(0x1C07, 1);
    gamestate_flags.set_flag(0x1A22, 1);
}

void wstag345_setup(void) {
    fieldstg_stage.background_file = 0x3A5;
    fieldstg_stage.sprite_file = 0x03A60000;
    fieldstg_stage.sprites = wstag345_sprites;
    fieldstg_stage.map_events = wstag345_map_events;
    fieldstg_stage.mask_file = 0x3A4;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x6E00, 0xB600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag345_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag345_actors;
    fieldstg_stage.color = wstag345_color;
    fieldstg_stage.battle_lists = wstag345_battle_lists;
    fieldstg_stage.events = wstag345_events;
    fieldstg_attr.set_file(0, 0x03A60001);
    fieldstg_attr.set_file(7, 0x03A60002);
    fieldstg_attr.set_file(4, 0x03A60003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress < 0xB) {
        fieldstg_stage.battle_lists = wstag345_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag345_battle_lists[1];
    }
}

INCLUDE_RODATA("asm/wstag345/nonmatchings/wstag345", wstag345_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag345_setup(void);

s16 D_WSTAG345_800A6054[251] = {
    FIELDSTG_EVENT_WALK(2, 935, 916, 5),
    FIELDSTG_EVENT_PLACE(283, 967, 901),
    FIELDSTG_EVENT_ANIM(283, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WALK(283, 992, 888, 5),
    FIELDSTG_EVENT_WAIT_WALK(283),
    FIELDSTG_EVENT_ANIM(283, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(283, 8, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 886, 2),
    FIELDSTG_EVENT_WAIT_ANIM(283),
    FIELDSTG_EVENT_ANIM(283, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(283, 8, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 886, 2),
    FIELDSTG_EVENT_WAIT_ANIM(283),
    FIELDSTG_EVENT_ANIM(283, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(283, 8, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 886, 2),
    FIELDSTG_EVENT_WAIT_ANIM(283),
    FIELDSTG_EVENT_ANIM(283, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 283, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 283),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 283),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(283, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(283, 967, 901, 1),
    FIELDSTG_EVENT_WAIT_WALK(283),
    FIELDSTG_EVENT_ANIM(283, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 283, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 283, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 283, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 895, 935, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG345_800A624C = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6258 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6264 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6270 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A627C = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6288 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6294 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A62A0 = { 40, 13, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A62AC = {
    3,
    { &D_WSTAG345_800A624C, &D_WSTAG345_800A6258, &D_WSTAG345_800A6264, &D_WSTAG345_800A6270, &D_WSTAG345_800A627C,
        &D_WSTAG345_800A6288, &D_WSTAG345_800A6294, &D_WSTAG345_800A62A0 },
};
FieldstgListedBattle D_WSTAG345_800A62D0 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A62DC = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A62E8 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A62F4 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6300 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A630C = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6318 = { 51, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6324 = { 51, 4, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A6330 = {
    3,
    { &D_WSTAG345_800A62D0, &D_WSTAG345_800A62DC, &D_WSTAG345_800A62E8, &D_WSTAG345_800A62F4, &D_WSTAG345_800A6300,
        &D_WSTAG345_800A630C, &D_WSTAG345_800A6318, &D_WSTAG345_800A6324 },
};
FieldstgListedBattle D_WSTAG345_800A6354 = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6360 = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A636C = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6378 = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6384 = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6390 = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A639C = { 54, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A63A8 = { 54, 2, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A63B4 = {
    3,
    { &D_WSTAG345_800A6354, &D_WSTAG345_800A6360, &D_WSTAG345_800A636C, &D_WSTAG345_800A6378, &D_WSTAG345_800A6384,
        &D_WSTAG345_800A6390, &D_WSTAG345_800A639C, &D_WSTAG345_800A63A8 },
};
FieldstgListedBattle D_WSTAG345_800A63D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A63E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A63FC = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6408 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A6420 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A642C = { 54, 2, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A6438 = {
    0,
    { &D_WSTAG345_800A63D8, &D_WSTAG345_800A63E4, &D_WSTAG345_800A63F0, &D_WSTAG345_800A63FC, &D_WSTAG345_800A6408,
        &D_WSTAG345_800A6414, &D_WSTAG345_800A6420, &D_WSTAG345_800A642C },
};
FieldstgListedBattle D_WSTAG345_800A645C = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6468 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6474 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6480 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A648C = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6498 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A64A4 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A64B0 = { 40, 13, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A64BC = {
    3,
    { &D_WSTAG345_800A645C, &D_WSTAG345_800A6468, &D_WSTAG345_800A6474, &D_WSTAG345_800A6480, &D_WSTAG345_800A648C,
        &D_WSTAG345_800A6498, &D_WSTAG345_800A64A4, &D_WSTAG345_800A64B0 },
};
FieldstgListedBattle D_WSTAG345_800A64E0 = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A64EC = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A64F8 = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6504 = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6510 = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A651C = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6528 = { 70, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6534 = { 70, 4, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A6540 = {
    3,
    { &D_WSTAG345_800A64E0, &D_WSTAG345_800A64EC, &D_WSTAG345_800A64F8, &D_WSTAG345_800A6504, &D_WSTAG345_800A6510,
        &D_WSTAG345_800A651C, &D_WSTAG345_800A6528, &D_WSTAG345_800A6534 },
};
FieldstgListedBattle D_WSTAG345_800A6564 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6570 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A657C = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6588 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6594 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A65A0 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A65AC = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A65B8 = { 59, 2, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A65C4 = {
    3,
    { &D_WSTAG345_800A6564, &D_WSTAG345_800A6570, &D_WSTAG345_800A657C, &D_WSTAG345_800A6588, &D_WSTAG345_800A6594,
        &D_WSTAG345_800A65A0, &D_WSTAG345_800A65AC, &D_WSTAG345_800A65B8 },
};
FieldstgListedBattle D_WSTAG345_800A65E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A65F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A6600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A660C = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6618 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A6624 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG345_800A6630 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG345_800A663C = { 54, 2, 0x60080000 };
FieldstgBattleList D_WSTAG345_800A6648 = {
    0,
    { &D_WSTAG345_800A65E8, &D_WSTAG345_800A65F4, &D_WSTAG345_800A6600, &D_WSTAG345_800A660C, &D_WSTAG345_800A6618,
        &D_WSTAG345_800A6624, &D_WSTAG345_800A6630, &D_WSTAG345_800A663C },
};
FieldstgBattleLists wstag345_battle_lists[2] = {
    { 4, 0, 0, { &D_WSTAG345_800A62AC, &D_WSTAG345_800A6330, &D_WSTAG345_800A63B4 }, &D_WSTAG345_800A6438 },
    { 27, 1, 0, { &D_WSTAG345_800A64BC, &D_WSTAG345_800A6540, &D_WSTAG345_800A65C4 }, &D_WSTAG345_800A6648 },
};
FieldstgVramPlace wstag345_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 311, 216, 55, 336, 511 }, { 320, 256, 374, 343, 216, 87, 352, 511 },
    { 320, 256, 374, 375, 216, 119, 368, 511 }, { 320, 256, 374, 407, 216, 151, 320, 510 },
};
u16 D_WSTAG345_800A6744[4] = { 0x1A22, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A674C[8] = { 0x1A22, 1, 0x1A23, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A675C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A6764[8] = { 0x1A22, 1, 0x1A23, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A6774[8] = { 0x1A22, 1, 0x1A23, 1, 0x1A24, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A6784[6] = { 0x1A24, 1, 0x1C08, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A6790[10] = {
    0x1A22, 1, 0x1A23, 1, 0x1A24, 1, 0x8004, 0,
    0xFFFF, 0,
};
u16 D_WSTAG345_800A67A4[10] = {
    0x1A22, 1, 0x1A23, 1, 0x1A24, 1, 0x8004, 1,
    0xFFFF, 0,
};
u16 D_WSTAG345_800A67B8[4] = { 0x1A22, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A67C0[6] = { 0x1A22, 1, 0x1A23, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A67CC[8] = { 0x1A22, 1, 0x1A23, 1, 0x1A24, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A67DC[6] = { 0x1A24, 1, 0x1C08, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A67E8[10] = {
    0x1A22, 1, 0x1A23, 1, 0x1A24, 1, 0x8004, 0,
    0xFFFF, 0,
};
u16 D_WSTAG345_800A67FC[10] = {
    0x1A22, 1, 0x1A23, 1, 0x1A24, 1, 0x8004, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG345_800A6810[2] = { { NULL, NULL, 28 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG345_800A6828[2] = { { NULL, NULL, 672 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG345_800A6840[7] = {
    { D_WSTAG345_800A6744, NULL, 667 }, { D_WSTAG345_800A674C, D_WSTAG345_800A675C, 668 },
    { D_WSTAG345_800A6764, NULL, 670 }, { D_WSTAG345_800A6774, D_WSTAG345_800A6784, 669 },
    { D_WSTAG345_800A6790, NULL, 861 }, { D_WSTAG345_800A67A4, NULL, 671 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG345_800A6894[2] = { { NULL, NULL, 671 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG345_800A68AC[6] = {
    { D_WSTAG345_800A67B8, NULL, 667 }, { D_WSTAG345_800A67C0, NULL, 668 },
    { D_WSTAG345_800A67CC, D_WSTAG345_800A67DC, 669 }, { D_WSTAG345_800A67E8, NULL, 670 },
    { D_WSTAG345_800A67FC, NULL, 673 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG345_800A68F4[2] = { { NULL, NULL, 673 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG345_800A690C[2] = { { NULL, NULL, 29 }, { NULL, NULL, 0 } };
u16 D_WSTAG345_800A6924[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A692C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A6934[6] = { 0x7022, 1, 0x8004, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A6940[6] = { 0x7022, 1, 0x8004, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A694C[6] = { 0x602B, 1, 0x8004, 0, 0xFFFF, 0 };
u16 D_WSTAG345_800A6958[6] = { 0x602B, 1, 0x8004, 1, 0xFFFF, 0 };
u16 D_WSTAG345_800A6964[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG345_800A696C = { D_WSTAG345_800A6924, D_WSTAG345_800A6810, 157, 4, 885, 951, 5 };
FieldstgPlacedActor D_WSTAG345_800A6980 = { D_WSTAG345_800A692C, D_WSTAG345_800A6828, 281, 5, 865, 941, 5 };
FieldstgPlacedActor D_WSTAG345_800A6994 = { D_WSTAG345_800A6934, D_WSTAG345_800A6840, 283, 6, 967, 901, 1 };
FieldstgPlacedActor D_WSTAG345_800A69A8 = { D_WSTAG345_800A6940, D_WSTAG345_800A6894, 283, 6, 967, 901, 1 };
FieldstgPlacedActor D_WSTAG345_800A69BC = { D_WSTAG345_800A694C, D_WSTAG345_800A68AC, 283, 6, 967, 901, 1 };
FieldstgPlacedActor D_WSTAG345_800A69D0 = { D_WSTAG345_800A6958, D_WSTAG345_800A68F4, 283, 6, 967, 901, 1 };
FieldstgPlacedActor D_WSTAG345_800A69E4 = { D_WSTAG345_800A6964, D_WSTAG345_800A690C, 314, 7, 905, 961, 5 };
FieldstgPlacedActor *wstag345_actors[8] = {
    &D_WSTAG345_800A696C, &D_WSTAG345_800A6980, &D_WSTAG345_800A6994, &D_WSTAG345_800A69A8, &D_WSTAG345_800A69BC,
    &D_WSTAG345_800A69D0, &D_WSTAG345_800A69E4, NULL,
};
FieldstgSprite wstag345_sprites[49] = {
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 330, 524, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 922, 950, 0, 0 },
    { 1, 0, 0x80, 2, 7, 0, 0, 0, 0, 0, 256, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 176, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 212, 597, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 534, 857, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 646, 785, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 820, 685, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 874, 498, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 139, 93, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 148, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 154, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 460, 421, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 651, 773, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 750, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 842, 515, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 185, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 291, 768, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 398, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 480, 897, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 608, 960, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 693, 1010, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 213, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 422, 830, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 462, 898, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 569, 865, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 647, 985, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 680, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 165, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 170, 169, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 258, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 323, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 434, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 595, 944, 0, 0 },
    { 1, 0, 0xFF, 6, 8, 0, 0, 0, 0, 0, 703, 232, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 593, 792, 804, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 623, 849, 864, 0 }, { 1, 0, 0x80, 4, 9, 0, 0, 0, 0, 0, 631, 378, 433, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 333, 235, 235, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 381, 259, 259, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 429, 283, 283, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 481, 532, 532, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 486, 775, 775, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 557, 557, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 533, 751, 751, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 541, 699, 699, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 574, 580, 580, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1014, 879, 879, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag345_map_events[16] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21D, 0x1D2, 0x92, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xC, 0x231, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xC, 0x221, 0x178, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 3, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 2, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xF, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFA2, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x28, 0x20, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x29, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 3, 1 },
    { 0x1A22, 0, 0x8192, 1, 8, 0x4F0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag345_funcs = { wstag345_setup };
FieldstgEventDef wstag345_events[2] = {
    { 1264, D_WSTAG345_800A6054, 0x0127001D, NULL, wstag345_event_1264_end }, { -1, NULL, 0, NULL, NULL },
};
