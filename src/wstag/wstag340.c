#include "wstag.h"

/* WSTAG340: stage 0x21F (fieldstg_stages). */

extern WstagFuncs wstag340_funcs;
extern CVECTOR wstag340_color;
extern FieldstgBattleLists wstag340_battle_lists[];
extern FieldstgVramPlace wstag340_vram_places[];
extern FieldstgPlacedActor *wstag340_actors[];
extern FieldstgSprite wstag340_sprites[];
extern FieldstgMapEvent wstag340_map_events[];
extern FieldstgEventDef wstag340_events[];

void wstag340_update(WstagObject *obj) {
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

WstagObject *wstag340_start(void *arg0) {
    WstagObject *obj = object_new(wstag340_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag340_funcs.setup();
    return obj;
}

void wstag340_setup(void) {
    fieldstg_stage.background_file = 0x3A1;
    fieldstg_stage.sprite_file = 0x03A20000;
    fieldstg_stage.sprites = wstag340_sprites;
    fieldstg_stage.map_events = wstag340_map_events;
    fieldstg_stage.mask_file = 0x3A0;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x30700, 0x9A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag340_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag340_actors;
    fieldstg_stage.color = wstag340_color;
    fieldstg_stage.battle_lists = wstag340_battle_lists;
    fieldstg_stage.events = wstag340_events;
    fieldstg_attr.set_file(0, 0x03A20001);
    fieldstg_attr.set_file(7, 0x03A20002);
    fieldstg_attr.set_file(4, 0x03A20003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress < 0xB) {
        fieldstg_stage.battle_lists = wstag340_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag340_battle_lists[1];
    }
}

INCLUDE_RODATA("asm/wstag340/nonmatchings/wstag340", wstag340_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag340_setup(void);

s16 D_WSTAG340_800A600C[78] = {
    FIELDSTG_EVENT_WALK(2, 448, 624, 1),
    FIELDSTG_EVENT_ANIM(282, 7, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 282, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 282, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(282, 7, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(282, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG340_800A60A8 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60B4 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60C0 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60CC = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60D8 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60E4 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60F0 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A60FC = { 40, 1, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A6108 = {
    3,
    { &D_WSTAG340_800A60A8, &D_WSTAG340_800A60B4, &D_WSTAG340_800A60C0, &D_WSTAG340_800A60CC, &D_WSTAG340_800A60D8,
        &D_WSTAG340_800A60E4, &D_WSTAG340_800A60F0, &D_WSTAG340_800A60FC },
};
FieldstgListedBattle D_WSTAG340_800A612C = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6138 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6144 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6150 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A615C = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6168 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6174 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6180 = { 37, 2, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A618C = {
    3,
    { &D_WSTAG340_800A612C, &D_WSTAG340_800A6138, &D_WSTAG340_800A6144, &D_WSTAG340_800A6150, &D_WSTAG340_800A615C,
        &D_WSTAG340_800A6168, &D_WSTAG340_800A6174, &D_WSTAG340_800A6180 },
};
FieldstgListedBattle D_WSTAG340_800A61B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A61F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6204 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG340_800A6210 = {
    0,
    { &D_WSTAG340_800A61B0, &D_WSTAG340_800A61BC, &D_WSTAG340_800A61C8, &D_WSTAG340_800A61D4, &D_WSTAG340_800A61E0,
        &D_WSTAG340_800A61EC, &D_WSTAG340_800A61F8, &D_WSTAG340_800A6204 },
};
FieldstgListedBattle D_WSTAG340_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6258 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6264 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A627C = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6288 = { 54, 2, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A6294 = {
    0,
    { &D_WSTAG340_800A6234, &D_WSTAG340_800A6240, &D_WSTAG340_800A624C, &D_WSTAG340_800A6258, &D_WSTAG340_800A6264,
        &D_WSTAG340_800A6270, &D_WSTAG340_800A627C, &D_WSTAG340_800A6288 },
};
FieldstgListedBattle D_WSTAG340_800A62B8 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A62C4 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A62D0 = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A62DC = { 39, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A62E8 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A62F4 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6300 = { 40, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A630C = { 40, 1, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A6318 = {
    3,
    { &D_WSTAG340_800A62B8, &D_WSTAG340_800A62C4, &D_WSTAG340_800A62D0, &D_WSTAG340_800A62DC, &D_WSTAG340_800A62E8,
        &D_WSTAG340_800A62F4, &D_WSTAG340_800A6300, &D_WSTAG340_800A630C },
};
FieldstgListedBattle D_WSTAG340_800A633C = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6348 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6354 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6360 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A636C = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6378 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6384 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6390 = { 59, 2, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A639C = {
    3,
    { &D_WSTAG340_800A633C, &D_WSTAG340_800A6348, &D_WSTAG340_800A6354, &D_WSTAG340_800A6360, &D_WSTAG340_800A636C,
        &D_WSTAG340_800A6378, &D_WSTAG340_800A6384, &D_WSTAG340_800A6390 },
};
FieldstgListedBattle D_WSTAG340_800A63C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A63CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A63D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A63E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A63FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6414 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG340_800A6420 = {
    0,
    { &D_WSTAG340_800A63C0, &D_WSTAG340_800A63CC, &D_WSTAG340_800A63D8, &D_WSTAG340_800A63E4, &D_WSTAG340_800A63F0,
        &D_WSTAG340_800A63FC, &D_WSTAG340_800A6408, &D_WSTAG340_800A6414 },
};
FieldstgListedBattle D_WSTAG340_800A6444 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6450 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A645C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A6468 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6474 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6480 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG340_800A648C = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG340_800A6498 = { 54, 2, 0x60080000 };
FieldstgBattleList D_WSTAG340_800A64A4 = {
    0,
    { &D_WSTAG340_800A6444, &D_WSTAG340_800A6450, &D_WSTAG340_800A645C, &D_WSTAG340_800A6468, &D_WSTAG340_800A6474,
        &D_WSTAG340_800A6480, &D_WSTAG340_800A648C, &D_WSTAG340_800A6498 },
};
FieldstgBattleLists wstag340_battle_lists[2] = {
    { 3, 0, 0, { &D_WSTAG340_800A6108, &D_WSTAG340_800A618C, &D_WSTAG340_800A6210 }, &D_WSTAG340_800A6294 },
    { 26, 1, 0, { &D_WSTAG340_800A6318, &D_WSTAG340_800A639C, &D_WSTAG340_800A6420 }, &D_WSTAG340_800A64A4 },
};
FieldstgVramPlace wstag340_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 385, 200, 129, 336, 511 }, { 320, 256, 370, 417, 200, 161, 352, 511 },
};
u16 D_WSTAG340_800A6580[4] = { 0x1A36, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A6588[4] = { 0x1A36, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A6590[6] = { 0x1A36, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A659C[8] = { 0x1A36, 1, 0x703A, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A65AC[4] = { 0x1A21, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A65B4[10] = {
    0x1A36, 1, 0x703A, 1, 0x8005, 0, 0x8192, 1,
    0xFFFF, 0,
};
u16 D_WSTAG340_800A65C8[6] = { 0x8005, 1, 0x9047, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A65D4[10] = {
    0x1A36, 1, 0x703A, 1, 0x8005, 1, 0x8192, 1,
    0xFFFF, 0,
};
u16 D_WSTAG340_800A65E8[4] = { 0x1A36, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A65F0[4] = { 0x1A36, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A65F8[6] = { 0x8192, 0, 0x1A36, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A6604[8] = { 0x1A36, 1, 0x8192, 1, 0x703A, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A6614[4] = { 0x1A21, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A661C[10] = {
    0x8192, 1, 0x703A, 1, 0x8005, 0, 0x1A36, 1,
    0xFFFF, 0,
};
u16 D_WSTAG340_800A6630[6] = { 0x8005, 1, 0x9047, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A663C[10] = {
    0x1A36, 1, 0x8192, 1, 0x703A, 1, 0x8005, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG340_800A6650[2] = { { NULL, NULL, 659 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG340_800A6668[2] = { { NULL, NULL, 658 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG340_800A6680[6] = {
    { D_WSTAG340_800A6580, D_WSTAG340_800A6588, 654 }, { D_WSTAG340_800A6590, NULL, 655 },
    { D_WSTAG340_800A659C, D_WSTAG340_800A65AC, 656 }, { D_WSTAG340_800A65B4, D_WSTAG340_800A65C8, 657 },
    { D_WSTAG340_800A65D4, NULL, 658 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG340_800A66C8[6] = {
    { D_WSTAG340_800A65E8, D_WSTAG340_800A65F0, 654 }, { D_WSTAG340_800A65F8, NULL, 655 },
    { D_WSTAG340_800A6604, D_WSTAG340_800A6614, 656 }, { D_WSTAG340_800A661C, D_WSTAG340_800A6630, 657 },
    { D_WSTAG340_800A663C, NULL, 658 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG340_800A6710[2] = { { NULL, NULL, 658 }, { NULL, NULL, 0 } };
u16 D_WSTAG340_800A6728[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A6730[6] = { 0x7022, 1, 0x8005, 1, 0xFFFF, 0 };
u16 D_WSTAG340_800A673C[6] = { 0x7022, 1, 0x8005, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A6748[6] = { 0x602B, 1, 0x8005, 0, 0xFFFF, 0 };
u16 D_WSTAG340_800A6754[6] = { 0x602B, 1, 0x8005, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG340_800A6760 = { D_WSTAG340_800A6728, D_WSTAG340_800A6650, 157, 4, 412, 642, 1 };
FieldstgPlacedActor D_WSTAG340_800A6774 = { D_WSTAG340_800A6730, D_WSTAG340_800A6668, 282, 5, 412, 642, 1 };
FieldstgPlacedActor D_WSTAG340_800A6788 = { D_WSTAG340_800A673C, D_WSTAG340_800A6680, 282, 5, 412, 642, 1 };
FieldstgPlacedActor D_WSTAG340_800A679C = { D_WSTAG340_800A6748, D_WSTAG340_800A66C8, 282, 5, 412, 642, 1 };
FieldstgPlacedActor D_WSTAG340_800A67B0 = { D_WSTAG340_800A6754, D_WSTAG340_800A6710, 282, 5, 412, 642, 1 };
FieldstgPlacedActor *wstag340_actors[6] = {
    &D_WSTAG340_800A6760, &D_WSTAG340_800A6774, &D_WSTAG340_800A6788, &D_WSTAG340_800A679C, &D_WSTAG340_800A67B0,
    NULL,
};
FieldstgSprite wstag340_sprites[44] = {
    { 1, 0, 0x40, 2, 5, 1, 5, 0xA, 8, 0, 395, 85, 0, 0 }, { 1, 0, 0x40, 2, 5, 1, 5, 0xA, 8, 0, 698, 285, 0, 0 },
    { 1, 0, 0x50, 2, 0xB, 0, 0, 0, 0, 0, 688, 97, 0, 0 }, { 1, 0, 0x72, 2, 0xC, 0, 0, 0, 0, 0, 784, 47, 0, 0 },
    { 1, 0, 0x80, 2, 0xD, 0, 0, 0, 0, 0, 640, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 72, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 271, 529, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 677, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 224, 557, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 309, 704, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 290, 716, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 698, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 83, 463, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 47, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 146, 531, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 37, 375, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 90, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 218, 638, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 75, 550, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 96, 549, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 182, 545, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 288, 706, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 310, 714, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 321, 691, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 494, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 514, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 671, 111, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 799, 48, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 811, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 817, 43, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 496, 558, 575, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 509, 517, 543, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 514, 471, 489, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 676, 390, 407, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 276, 499, 511, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 255, 255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 390, 243, 243, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 247, 247, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 375, 375, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag340_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21D, 0x102, 0x3AC, 5, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 4, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x3C, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x40, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag340_funcs = { wstag340_setup };
FieldstgEventDef wstag340_events[2] = {
    { 1310, D_WSTAG340_800A600C, 0x01270026, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
