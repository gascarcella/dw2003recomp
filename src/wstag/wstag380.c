#include "wstag.h"

/* WSTAG380: stage 0x227 (fieldstg_stages). */

extern WstagFuncs wstag380_funcs;
const CVECTOR wstag380_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgBattleLists wstag380_battle_lists;
extern FieldstgBattleLists wstag380_battle_lists2;
extern FieldstgBattleLists wstag380_battle_lists3;
extern FieldstgVramPlace wstag380_vram_places[];
extern FieldstgPlacedActor *wstag380_actors[];
extern FieldstgSprite wstag380_sprites[];
extern FieldstgMapEvent wstag380_map_events[];
extern FieldstgEventDef wstag380_events[];

void wstag380_update(WstagObject *obj) {
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

WstagObject *wstag380_start(void *arg0) {
    WstagObject *obj = object_new(wstag380_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag380_funcs.setup();
    return obj;
}

void wstag380_event_205_end(void) {
    gamestate_data.progress = 0x9;
}

void wstag380_setup(void) {
    fieldstg_stage.background_file = 0x2BA;
    fieldstg_stage.sprite_file = 0x02BB0000;
    fieldstg_stage.sprites = wstag380_sprites;
    fieldstg_stage.map_events = wstag380_map_events;
    fieldstg_stage.mask_file = 0x32F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13D00, 0x34000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag380_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag380_actors;
    fieldstg_stage.color = wstag380_color;
    fieldstg_stage.events = wstag380_events;
    fieldstg_attr.set_file(0, 0x02BB0001);
    fieldstg_attr.set_file(7, 0x02BB0002);
    fieldstg_attr.set_file(4, 0x02BB0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress < 0xB) {
        fieldstg_stage.battle_lists = &wstag380_battle_lists;
    } else if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = &wstag380_battle_lists2;
    } else {
        fieldstg_stage.battle_lists = &wstag380_battle_lists3;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag380_setup(void);

s16 D_WSTAG380_800A6028[168] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 161, 433, 3),
    FIELDSTG_EVENT_PLACE(103, 129, 417),
    FIELDSTG_EVENT_ANIM(103, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(103, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 160, 464, 0),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 224, 496, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x222, 810, 820, 3),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG380_800A6178 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6184 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6190 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A619C = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A61A8 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A61B4 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A61C0 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A61CC = { 36, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A61D8 = {
    3,
    { &D_WSTAG380_800A6178, &D_WSTAG380_800A6184, &D_WSTAG380_800A6190, &D_WSTAG380_800A619C, &D_WSTAG380_800A61A8,
        &D_WSTAG380_800A61B4, &D_WSTAG380_800A61C0, &D_WSTAG380_800A61CC },
};
FieldstgListedBattle D_WSTAG380_800A61FC = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6208 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6214 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6220 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A622C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6238 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6244 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6250 = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A625C = {
    0,
    { &D_WSTAG380_800A61FC, &D_WSTAG380_800A6208, &D_WSTAG380_800A6214, &D_WSTAG380_800A6220, &D_WSTAG380_800A622C,
        &D_WSTAG380_800A6238, &D_WSTAG380_800A6244, &D_WSTAG380_800A6250 },
};
FieldstgListedBattle D_WSTAG380_800A6280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A628C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A62A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A62B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A62BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A62C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A62D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG380_800A62E0 = {
    0,
    { &D_WSTAG380_800A6280, &D_WSTAG380_800A628C, &D_WSTAG380_800A6298, &D_WSTAG380_800A62A4, &D_WSTAG380_800A62B0,
        &D_WSTAG380_800A62BC, &D_WSTAG380_800A62C8, &D_WSTAG380_800A62D4 },
};
FieldstgListedBattle D_WSTAG380_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6328 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6334 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6340 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A634C = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6358 = { 64, 8, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A6364 = {
    0,
    { &D_WSTAG380_800A6304, &D_WSTAG380_800A6310, &D_WSTAG380_800A631C, &D_WSTAG380_800A6328, &D_WSTAG380_800A6334,
        &D_WSTAG380_800A6340, &D_WSTAG380_800A634C, &D_WSTAG380_800A6358 },
};
FieldstgListedBattle D_WSTAG380_800A6388 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6394 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63A0 = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63AC = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63B8 = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63C4 = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63D0 = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A63DC = { 66, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A63E8 = {
    3,
    { &D_WSTAG380_800A6388, &D_WSTAG380_800A6394, &D_WSTAG380_800A63A0, &D_WSTAG380_800A63AC, &D_WSTAG380_800A63B8,
        &D_WSTAG380_800A63C4, &D_WSTAG380_800A63D0, &D_WSTAG380_800A63DC },
};
FieldstgListedBattle D_WSTAG380_800A640C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6418 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6424 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6430 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A643C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6448 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6454 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6460 = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A646C = {
    0,
    { &D_WSTAG380_800A640C, &D_WSTAG380_800A6418, &D_WSTAG380_800A6424, &D_WSTAG380_800A6430, &D_WSTAG380_800A643C,
        &D_WSTAG380_800A6448, &D_WSTAG380_800A6454, &D_WSTAG380_800A6460 },
};
FieldstgListedBattle D_WSTAG380_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A64E4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG380_800A64F0 = {
    0,
    { &D_WSTAG380_800A6490, &D_WSTAG380_800A649C, &D_WSTAG380_800A64A8, &D_WSTAG380_800A64B4, &D_WSTAG380_800A64C0,
        &D_WSTAG380_800A64CC, &D_WSTAG380_800A64D8, &D_WSTAG380_800A64E4 },
};
FieldstgListedBattle D_WSTAG380_800A6514 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6520 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A652C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6538 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6544 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6550 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A655C = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6568 = { 64, 8, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A6574 = {
    0,
    { &D_WSTAG380_800A6514, &D_WSTAG380_800A6520, &D_WSTAG380_800A652C, &D_WSTAG380_800A6538, &D_WSTAG380_800A6544,
        &D_WSTAG380_800A6550, &D_WSTAG380_800A655C, &D_WSTAG380_800A6568 },
};
FieldstgListedBattle D_WSTAG380_800A6598 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65A4 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65B0 = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65BC = { 66, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65C8 = { 60, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65D4 = { 60, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65E0 = { 60, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A65EC = { 60, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A65F8 = {
    3,
    { &D_WSTAG380_800A6598, &D_WSTAG380_800A65A4, &D_WSTAG380_800A65B0, &D_WSTAG380_800A65BC, &D_WSTAG380_800A65C8,
        &D_WSTAG380_800A65D4, &D_WSTAG380_800A65E0, &D_WSTAG380_800A65EC },
};
FieldstgListedBattle D_WSTAG380_800A661C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6628 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6634 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6640 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A664C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6658 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6664 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6670 = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A667C = {
    0,
    { &D_WSTAG380_800A661C, &D_WSTAG380_800A6628, &D_WSTAG380_800A6634, &D_WSTAG380_800A6640, &D_WSTAG380_800A664C,
        &D_WSTAG380_800A6658, &D_WSTAG380_800A6664, &D_WSTAG380_800A6670 },
};
FieldstgListedBattle D_WSTAG380_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A66F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG380_800A6700 = {
    0,
    { &D_WSTAG380_800A66A0, &D_WSTAG380_800A66AC, &D_WSTAG380_800A66B8, &D_WSTAG380_800A66C4, &D_WSTAG380_800A66D0,
        &D_WSTAG380_800A66DC, &D_WSTAG380_800A66E8, &D_WSTAG380_800A66F4 },
};
FieldstgListedBattle D_WSTAG380_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A673C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG380_800A6748 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6754 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6760 = { 36, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A676C = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG380_800A6778 = { 64, 8, 0x60080000 };
FieldstgBattleList D_WSTAG380_800A6784 = {
    0,
    { &D_WSTAG380_800A6724, &D_WSTAG380_800A6730, &D_WSTAG380_800A673C, &D_WSTAG380_800A6748, &D_WSTAG380_800A6754,
        &D_WSTAG380_800A6760, &D_WSTAG380_800A676C, &D_WSTAG380_800A6778 },
};
FieldstgBattleLists wstag380_battle_lists = {
    7, 0, 0, { &D_WSTAG380_800A61D8, &D_WSTAG380_800A625C, &D_WSTAG380_800A62E0 }, &D_WSTAG380_800A6364,
};
FieldstgBattleLists wstag380_battle_lists2 = {
    24, 1, 0, { &D_WSTAG380_800A63E8, &D_WSTAG380_800A646C, &D_WSTAG380_800A64F0 }, &D_WSTAG380_800A6574,
};
FieldstgBattleLists wstag380_battle_lists3 = {
    53, 2, 0, { &D_WSTAG380_800A65F8, &D_WSTAG380_800A667C, &D_WSTAG380_800A6700 }, &D_WSTAG380_800A6784,
};
FieldstgVramPlace wstag380_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 410, 288, 360, 32, 352, 511 }, { 320, 256, 368, 460, 192, 204, 320, 510 },
    { 384, 256, 432, 256, 448, 0, 336, 510 }, { 384, 256, 384, 288, 256, 32, 352, 510 },
    { 384, 256, 394, 288, 296, 32, 368, 510 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG380_800A68BC[8] = { 0x203, 1, 0x8AD7, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG380_800A68CC[4] = { 0x1A2C, 0, 0xFFFF, 0 };
u16 D_WSTAG380_800A68D4[8] = { 0x1A2C, 1, 0x8007, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG380_800A68E4[4] = { 0x1A2C, 1, 0xFFFF, 0 };
u16 D_WSTAG380_800A68EC[4] = { 0x9020, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG380_800A68F4[2] = { { NULL, D_WSTAG380_800A68BC, 367 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG380_800A690C[3] = {
    { D_WSTAG380_800A68CC, D_WSTAG380_800A68D4, 697 }, { D_WSTAG380_800A68E4, NULL, 698 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG380_800A6930[2] = { { NULL, D_WSTAG380_800A68EC, 584 }, { NULL, NULL, 0 } };
u16 D_WSTAG380_800A6948[4] = { 0x203, 0, 0xFFFF, 0 };
u16 D_WSTAG380_800A6950[6] = { 0x1A21, 1, 0x1A2C, 0, 0xFFFF, 0 };
u16 D_WSTAG380_800A695C[6] = { 0x6008, 1, 0x1A1A, 1, 0xFFFF, 0 };
u16 D_WSTAG380_800A6968[6] = { 0x1A21, 1, 0x1A2C, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG380_800A6974 = { D_WSTAG380_800A6948, D_WSTAG380_800A68F4, 33, 4, 776, 125, 1 };
FieldstgPlacedActor D_WSTAG380_800A6988 = { D_WSTAG380_800A6950, D_WSTAG380_800A690C, 75, 5, 530, 430, 1 };
FieldstgPlacedActor D_WSTAG380_800A699C = { NULL, NULL, 76, 6, 760, 357, 7 };
FieldstgPlacedActor D_WSTAG380_800A69B0 = { NULL, NULL, 88, 7, 670, 280, 1 };
FieldstgPlacedActor D_WSTAG380_800A69C4 = { D_WSTAG380_800A695C, D_WSTAG380_800A6930, 103, 8, 129, 417, 3 };
FieldstgPlacedActor D_WSTAG380_800A69D8 = { D_WSTAG380_800A6968, NULL, 284, 9, 520, 437, 1 };
FieldstgPlacedActor *wstag380_actors[7] = {
    &D_WSTAG380_800A6974, &D_WSTAG380_800A6988, &D_WSTAG380_800A699C, &D_WSTAG380_800A69B0, &D_WSTAG380_800A69C4,
    &D_WSTAG380_800A69D8, NULL,
};
FieldstgSprite wstag380_sprites[96] = {
    { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 43, 521, 0, 0 }, { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 426, -16, 0, 0 },
    { 1, 0, 0x40, 2, 4, 1, 4, 9, 8, 0, 761, 743, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 337, 409, 0, 0 },
    { 1, 0, 0x74, 2, 2, 0, 0, 0, 0, 0, 655, 662, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 732, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 810, 177, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 868, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 989, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1051, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1125, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 431, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 541, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 618, 226, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 781, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 907, 130, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 34, 772, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 470, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 504, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 606, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 611, 441, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 660, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 686, 214, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 700, 583, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 764, 605, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 840, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 372, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 411, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 492, 502, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 988, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 58, 779, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 293, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 427, 470, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 537, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 940, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 337, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 538, 522, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 624, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 764, 635, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 793, 425, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1050, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 45, 781, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 466, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 467, 390, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 523, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 552, 295, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 563, 474, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 564, 271, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 575, 246, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 632, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 644, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 664, 324, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 684, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 690, 456, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 733, 208, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 788, 632, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 792, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 819, 419, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 850, 168, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 952, 332, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 977, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1021, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1072, 376, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1131, 347, 0, 0 },
    { 1, 0, 0x74, 6, 3, 0, 0, 0, 0, 0, 768, 640, 0, 0 }, { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 96, 687, 687, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 103, 923, 923, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 144, 663, 663, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 703, 703, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 639, 639, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 679, 679, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 952, 952, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 655, 655, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 197, 197, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 274, 262, 262, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 623, 623, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 937, 937, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 305, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 336, 599, 599, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 352, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 583, 583, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 591, 591, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 127, 127, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 624, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 831, 831, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 992, 783, 783, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1115, 428, 428, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag380_map_events[17] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x222, 0x32A, 0x334, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x229, 0x9A, 0x74, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 8, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x1A1, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x1B0, 0x176, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x58, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0x40, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFC0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB8, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFF90, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x18, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFF0, 0x10, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag380_funcs = { wstag380_setup };
FieldstgEventDef wstag380_events[3] = {
    { 205, D_WSTAG380_800A6028, 0x01270001, NULL, wstag380_event_205_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
