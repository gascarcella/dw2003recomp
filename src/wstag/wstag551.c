#include "wstag.h"

/* WSTAG551: stage 0x2B1 (fieldstg_stages). */

extern WstagFuncs wstag551_funcs;
extern FieldstgBattleLists wstag551_battle_lists;
extern FieldstgVramPlace wstag551_vram_places[];
extern FieldstgPlacedActor *wstag551_actors[];
extern FieldstgSprite wstag551_sprites[];
extern FieldstgMapEvent wstag551_map_events[];
extern FieldstgEventDef wstag551_events[];

void wstag551_update(WstagObject *obj) {
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

WstagObject *wstag551_start(void *arg0) {
    WstagObject *obj = object_new(wstag551_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag551_funcs.setup();
    return obj;
}

void wstag551_setup(void) {
    fieldstg_stage.background_file = 0x5D1;
    fieldstg_stage.sprite_file = 0x05D20000;
    fieldstg_stage.sprites = wstag551_sprites;
    fieldstg_stage.map_events = wstag551_map_events;
    fieldstg_stage.mask_file = 0x5D0;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xCA00, 0x9800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag551_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag551_actors;
    fieldstg_stage.events = wstag551_events;
    fieldstg_stage.battle_lists = &wstag551_battle_lists;
    fieldstg_attr.set_file(0, 0x05D20001);
    fieldstg_attr.set_file(7, 0x05D20002);
    fieldstg_attr.set_file(4, 0x05D20003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag551_setup(void);

s16 D_WSTAG551_800A5FE0[57] = {
    FIELDSTG_EVENT_WALK(2, 400, 224, 3),
    FIELDSTG_EVENT_PLACE(21, 369, 209),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC0E, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG551_800A6054 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6060 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A606C = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6078 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6084 = { 133, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6090 = { 133, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A609C = { 169, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A60A8 = { 169, 3, 0x60080000 };
FieldstgBattleList D_WSTAG551_800A60B4 = {
    3,
    { &D_WSTAG551_800A6054, &D_WSTAG551_800A6060, &D_WSTAG551_800A606C, &D_WSTAG551_800A6078, &D_WSTAG551_800A6084,
        &D_WSTAG551_800A6090, &D_WSTAG551_800A609C, &D_WSTAG551_800A60A8 },
};
FieldstgListedBattle D_WSTAG551_800A60D8 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A60E4 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A60F0 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A60FC = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6108 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6114 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6120 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A612C = { 0, 3, 0x60080000 };
FieldstgBattleList D_WSTAG551_800A6138 = {
    0,
    { &D_WSTAG551_800A60D8, &D_WSTAG551_800A60E4, &D_WSTAG551_800A60F0, &D_WSTAG551_800A60FC, &D_WSTAG551_800A6108,
        &D_WSTAG551_800A6114, &D_WSTAG551_800A6120, &D_WSTAG551_800A612C },
};
FieldstgListedBattle D_WSTAG551_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A61B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG551_800A61BC = {
    0,
    { &D_WSTAG551_800A615C, &D_WSTAG551_800A6168, &D_WSTAG551_800A6174, &D_WSTAG551_800A6180, &D_WSTAG551_800A618C,
        &D_WSTAG551_800A6198, &D_WSTAG551_800A61A4, &D_WSTAG551_800A61B0 },
};
FieldstgListedBattle D_WSTAG551_800A61E0 = { 234, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A61EC = { 282, 3, 0x600C0000 };
FieldstgListedBattle D_WSTAG551_800A61F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6210 = { 334, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A621C = { 169, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG551_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG551_800A6234 = { 180, 2, 0x60080000 };
FieldstgBattleList D_WSTAG551_800A6240 = {
    0,
    { &D_WSTAG551_800A61E0, &D_WSTAG551_800A61EC, &D_WSTAG551_800A61F8, &D_WSTAG551_800A6204, &D_WSTAG551_800A6210,
        &D_WSTAG551_800A621C, &D_WSTAG551_800A6228, &D_WSTAG551_800A6234 },
};
FieldstgBattleLists wstag551_battle_lists = {
    105, 0, 0, { &D_WSTAG551_800A60B4, &D_WSTAG551_800A6138, &D_WSTAG551_800A61BC }, &D_WSTAG551_800A6240,
};
FieldstgVramPlace wstag551_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 325, 128, 69, 336, 511 }, { 320, 256, 366, 397, 184, 141, 352, 511 },
};
u16 D_WSTAG551_800A6300[4] = { 0x9012, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A6308[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A6310[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A6318[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A6324[8] = { 0x7207, 1, 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A6334[4] = { 0x7632, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A633C[10] = {
    0, 1, 0x7209, 1, 0xE25, 0, 0x7207, 1,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A6350[6] = { 0x7400, 1, 0xE25, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A635C[10] = {
    0x7207, 1, 0, 1, 0x7209, 1, 0xE25, 1,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A6370[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A6378[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A6380[6] = { 0x720A, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A638C[8] = { 0, 1, 0xE46, 0, 0x720A, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A639C[6] = { 0x7400, 1, 0xE46, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A63A8[10] = {
    0x720A, 1, 0, 1, 0xE46, 1, 0x720C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A63BC[10] = {
    0, 1, 0xE46, 1, 0x720C, 1, 0x720A, 1,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A63D0[4] = { 0x7832, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A63D8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A63E0[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A63E8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG551_800A63F0[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A63F8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG551_800A6404[2] = { { NULL, D_WSTAG551_800A6300, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG551_800A641C[6] = {
    { D_WSTAG551_800A6308, D_WSTAG551_800A6310, 580 }, { D_WSTAG551_800A6318, NULL, 581 },
    { D_WSTAG551_800A6324, D_WSTAG551_800A6334, 582 }, { D_WSTAG551_800A633C, D_WSTAG551_800A6350, 583 },
    { D_WSTAG551_800A635C, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG551_800A6464[6] = {
    { D_WSTAG551_800A6370, D_WSTAG551_800A6378, 588 }, { D_WSTAG551_800A6380, NULL, 590 },
    { D_WSTAG551_800A638C, D_WSTAG551_800A639C, 589 }, { D_WSTAG551_800A63A8, NULL, 584 },
    { D_WSTAG551_800A63BC, D_WSTAG551_800A63D0, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG551_800A64AC[4] = {
    { D_WSTAG551_800A63D8, NULL, 925 }, { D_WSTAG551_800A63E0, D_WSTAG551_800A63E8, 585 },
    { D_WSTAG551_800A63F0, D_WSTAG551_800A63F8, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG551_800A64DC[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
u16 D_WSTAG551_800A64F4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A64FC[10] = {
    0x7019, 1, 0x11, 0, 0x8192, 1, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A6510[10] = {
    0x7019, 1, 0x11, 0, 0x8192, 1, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG551_800A6524[8] = { 0x8192, 1, 0x7019, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG551_800A6534[6] = { 0x7019, 1, 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG551_800A6540 = { D_WSTAG551_800A64F4, D_WSTAG551_800A6404, 21, 4, 369, 209, 7 };
FieldstgPlacedActor D_WSTAG551_800A6554 = { D_WSTAG551_800A64FC, D_WSTAG551_800A641C, 69, 5, 752, 690, 5 };
FieldstgPlacedActor D_WSTAG551_800A6568 = { D_WSTAG551_800A6510, D_WSTAG551_800A6464, 69, 5, 752, 690, 5 };
FieldstgPlacedActor D_WSTAG551_800A657C = { D_WSTAG551_800A6524, D_WSTAG551_800A64AC, 69, 5, 752, 690, 5 };
FieldstgPlacedActor D_WSTAG551_800A6590 = { D_WSTAG551_800A6534, D_WSTAG551_800A64DC, 69, 5, 752, 690, 5 };
FieldstgPlacedActor *wstag551_actors[6] = {
    &D_WSTAG551_800A6540, &D_WSTAG551_800A6554, &D_WSTAG551_800A6568, &D_WSTAG551_800A657C, &D_WSTAG551_800A6590,
    NULL,
};
FieldstgSprite wstag551_sprites[16] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 40, 512, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 472, 160, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 362, 998, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 634, 946, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 964, 182, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1102, 947, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1169, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 202, 926, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 495, 1034, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 986, 1028, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1213, 654, 0, 0 },
    { 1, 0, 0x40, 6, 0, 0, 0, 0, 0, 0, 678, 464, 0, 0 }, { 1, 0, 0x4A, 6, 2, 0, 0, 0, 0, 0, 1178, 782, 0, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 697, 464, 478, 0 }, { 1, 0, 0x4B, 4, 1, 0, 0, 0, 0, 0, 160, 219, 243, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag551_map_events[17] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B2, 0x610, 0x540, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 3, 5 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 6, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xB, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x479, 0x2B0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x469, 0x318, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0x13, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x2C, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x30, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xC, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFEC, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFE0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0x13, 1 },
    { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag551_funcs = { wstag551_setup };
FieldstgEventDef wstag551_events[3] = {
    { 1226, D_WSTAG551_800A5FE0, 0x013C000B, NULL, NULL }, { 9000, NULL, 0, fieldstg_start_battle_5, NULL },
    { -1, NULL, 0, NULL, NULL },
};
