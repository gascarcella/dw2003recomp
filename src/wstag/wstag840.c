#include "wstag.h"

/* WSTAG840: stage 0x2E3 (fieldstg_stages). */

extern WstagExits *wstag840_exits[];
extern WstagFuncs wstag840_funcs;
extern CVECTOR wstag840_color;
extern FieldstgBattleLists wstag840_battle_lists[];
extern FieldstgVramPlace wstag840_vram_places[];
extern FieldstgPlacedActor *wstag840_actors[];
extern FieldstgSprite wstag840_sprites[];
extern FieldstgMapEvent wstag840_map_events[];

s32 wstag840_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag840_set_exits(dst, list + 1, arg2, arg3);
    }
    exit = exits->exits;
    dst->param = exit->stage;
    dst->x = exit->x;
    dst->y = exit->y;
    dst->dir = exit->dir;
    dst->route = exit->route;
    dst->room = exit->room;
    while (exit->next != NULL) {
        exit = exit->next;
        dst++;
        dst->param = exit->stage;
        dst->x = exit->x;
        dst->y = exit->y;
        dst->dir = exit->dir;
        dst->route = exit->route;
        dst->room = exit->room;
    }
}

void wstag840_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        wstag840_set_exits(fieldstg_stage.map_events, wstag840_exits, gamestate_data.route,
                               gamestate_data.room);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag840_start(void *arg0) {
    WstagObject *obj = object_new(wstag840_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag840_funcs.setup();
    return obj;
}

void wstag840_setup(void) {
    fieldstg_stage.background_file = 0x74A;
    fieldstg_stage.sprite_file = 0x074B0000;
    fieldstg_stage.sprites = wstag840_sprites;
    fieldstg_stage.map_events = wstag840_map_events;
    fieldstg_stage.mask_file = 0x749;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15300, 0x13900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag840_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag840_actors;
    fieldstg_stage.color = wstag840_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag840_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x074B0001);
    fieldstg_attr.set_file(7, 0x074B0002);
    fieldstg_attr.set_file(4, 0x074B0003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag840/nonmatchings/wstag840", wstag840_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag840_setup(void);

WstagExit D_WSTAG840_800A60F4 = { 0x2E6, 3, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG840_800A6104 = { 0x220, 0, 0, 0x70, 0xB8, 0, &D_WSTAG840_800A60F4 };
WstagExits D_WSTAG840_800A6114 = { 3, 1, &D_WSTAG840_800A6104 };
WstagExit D_WSTAG840_800A611C = { 0x2E4, 4, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A612C = { 0x21F, 0, 0, 0xE0, 0x208, 0, &D_WSTAG840_800A611C };
WstagExits D_WSTAG840_800A613C = { 4, 1, &D_WSTAG840_800A612C };
WstagExit D_WSTAG840_800A6144 = { 0x2E4, 5, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A6154 = { 0x266, 0, 0, 0x2A0, 0x228, 0, &D_WSTAG840_800A6144 };
WstagExits D_WSTAG840_800A6164 = { 5, 1, &D_WSTAG840_800A6154 };
WstagExit D_WSTAG840_800A616C = { 0x2E4, 6, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A617C = { 0x266, 0, 0, 0x1C0, 0x108, 0, &D_WSTAG840_800A616C };
WstagExits D_WSTAG840_800A618C = { 6, 1, &D_WSTAG840_800A617C };
WstagExit D_WSTAG840_800A6194 = { 0x2E6, 9, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG840_800A61A4 = { 0x23D, 0, 0, 0x240, 0xD8, 0, &D_WSTAG840_800A6194 };
WstagExits D_WSTAG840_800A61B4 = { 9, 1, &D_WSTAG840_800A61A4 };
WstagExit D_WSTAG840_800A61BC = { 0x2E6, 0xD, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG840_800A61CC = { 0x28F, 0, 0, 0x70, 0xB8, 0, &D_WSTAG840_800A61BC };
WstagExits D_WSTAG840_800A61DC = { 13, 1, &D_WSTAG840_800A61CC };
WstagExit D_WSTAG840_800A61E4 = { 0x2E4, 0xE, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A61F4 = { 0x28E, 0, 0, 0xE0, 0x208, 0, &D_WSTAG840_800A61E4 };
WstagExits D_WSTAG840_800A6204 = { 14, 1, &D_WSTAG840_800A61F4 };
WstagExit D_WSTAG840_800A620C = { 0x2E4, 0xF, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A621C = { 0x2CE, 0, 0, 0x2A0, 0x228, 0, &D_WSTAG840_800A620C };
WstagExits D_WSTAG840_800A622C = { 15, 1, &D_WSTAG840_800A621C };
WstagExit D_WSTAG840_800A6234 = { 0x2E4, 0x10, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG840_800A6244 = { 0x2CE, 0, 0, 0x1C0, 0x108, 0, &D_WSTAG840_800A6234 };
WstagExits D_WSTAG840_800A6254 = { 16, 1, &D_WSTAG840_800A6244 };
WstagExit D_WSTAG840_800A625C = { 0x2E6, 0x13, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG840_800A626C = { 0x2AA, 0, 0, 0x240, 0xD8, 0, &D_WSTAG840_800A625C };
WstagExits D_WSTAG840_800A627C = { 19, 1, &D_WSTAG840_800A626C };
WstagExits D_WSTAG840_800A6284 = { 0, 0, &D_WSTAG840_800A6104 };
WstagExits *wstag840_exits[12] = {
    &D_WSTAG840_800A6114, &D_WSTAG840_800A613C, &D_WSTAG840_800A6164, &D_WSTAG840_800A618C, &D_WSTAG840_800A61B4,
    &D_WSTAG840_800A61DC, &D_WSTAG840_800A6204, &D_WSTAG840_800A622C, &D_WSTAG840_800A6254, &D_WSTAG840_800A627C,
    &D_WSTAG840_800A6284, NULL,
};
FieldstgListedBattle D_WSTAG840_800A62BC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A62C8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A62D4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A62E0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A62EC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A62F8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6304 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6310 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A631C = {
    3,
    { &D_WSTAG840_800A62BC, &D_WSTAG840_800A62C8, &D_WSTAG840_800A62D4, &D_WSTAG840_800A62E0, &D_WSTAG840_800A62EC,
        &D_WSTAG840_800A62F8, &D_WSTAG840_800A6304, &D_WSTAG840_800A6310 },
};
FieldstgListedBattle D_WSTAG840_800A6340 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A634C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6394 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A63A0 = {
    0,
    { &D_WSTAG840_800A6340, &D_WSTAG840_800A634C, &D_WSTAG840_800A6358, &D_WSTAG840_800A6364, &D_WSTAG840_800A6370,
        &D_WSTAG840_800A637C, &D_WSTAG840_800A6388, &D_WSTAG840_800A6394 },
};
FieldstgListedBattle D_WSTAG840_800A63C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A63D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6400 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A640C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6418 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6424 = {
    0,
    { &D_WSTAG840_800A63C4, &D_WSTAG840_800A63D0, &D_WSTAG840_800A63DC, &D_WSTAG840_800A63E8, &D_WSTAG840_800A63F4,
        &D_WSTAG840_800A6400, &D_WSTAG840_800A640C, &D_WSTAG840_800A6418 },
};
FieldstgListedBattle D_WSTAG840_800A6448 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6454 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A649C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A64A8 = {
    0,
    { &D_WSTAG840_800A6448, &D_WSTAG840_800A6454, &D_WSTAG840_800A6460, &D_WSTAG840_800A646C, &D_WSTAG840_800A6478,
        &D_WSTAG840_800A6484, &D_WSTAG840_800A6490, &D_WSTAG840_800A649C },
};
FieldstgListedBattle D_WSTAG840_800A64CC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A64D8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A64E4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A64F0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A64FC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6508 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6514 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6520 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A652C = {
    2,
    { &D_WSTAG840_800A64CC, &D_WSTAG840_800A64D8, &D_WSTAG840_800A64E4, &D_WSTAG840_800A64F0, &D_WSTAG840_800A64FC,
        &D_WSTAG840_800A6508, &D_WSTAG840_800A6514, &D_WSTAG840_800A6520 },
};
FieldstgListedBattle D_WSTAG840_800A6550 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A655C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6568 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6574 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6598 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A65A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A65B0 = {
    0,
    { &D_WSTAG840_800A6550, &D_WSTAG840_800A655C, &D_WSTAG840_800A6568, &D_WSTAG840_800A6574, &D_WSTAG840_800A6580,
        &D_WSTAG840_800A658C, &D_WSTAG840_800A6598, &D_WSTAG840_800A65A4 },
};
FieldstgListedBattle D_WSTAG840_800A65D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A65E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A65EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A65F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6604 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6610 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A661C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6628 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6634 = {
    0,
    { &D_WSTAG840_800A65D4, &D_WSTAG840_800A65E0, &D_WSTAG840_800A65EC, &D_WSTAG840_800A65F8, &D_WSTAG840_800A6604,
        &D_WSTAG840_800A6610, &D_WSTAG840_800A661C, &D_WSTAG840_800A6628 },
};
FieldstgListedBattle D_WSTAG840_800A6658 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6664 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6670 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A667C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6688 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6694 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A66AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A66B8 = {
    0,
    { &D_WSTAG840_800A6658, &D_WSTAG840_800A6664, &D_WSTAG840_800A6670, &D_WSTAG840_800A667C, &D_WSTAG840_800A6688,
        &D_WSTAG840_800A6694, &D_WSTAG840_800A66A0, &D_WSTAG840_800A66AC },
};
FieldstgListedBattle D_WSTAG840_800A66DC = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A66E8 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A66F4 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6700 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A670C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6718 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6724 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6730 = { 63, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A673C = {
    3,
    { &D_WSTAG840_800A66DC, &D_WSTAG840_800A66E8, &D_WSTAG840_800A66F4, &D_WSTAG840_800A6700, &D_WSTAG840_800A670C,
        &D_WSTAG840_800A6718, &D_WSTAG840_800A6724, &D_WSTAG840_800A6730 },
};
FieldstgListedBattle D_WSTAG840_800A6760 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A676C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6778 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6784 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6790 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A679C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A67A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A67B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A67C0 = {
    0,
    { &D_WSTAG840_800A6760, &D_WSTAG840_800A676C, &D_WSTAG840_800A6778, &D_WSTAG840_800A6784, &D_WSTAG840_800A6790,
        &D_WSTAG840_800A679C, &D_WSTAG840_800A67A8, &D_WSTAG840_800A67B4 },
};
FieldstgListedBattle D_WSTAG840_800A67E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A67FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6808 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6814 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6820 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A682C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6838 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6844 = {
    0,
    { &D_WSTAG840_800A67E4, &D_WSTAG840_800A67F0, &D_WSTAG840_800A67FC, &D_WSTAG840_800A6808, &D_WSTAG840_800A6814,
        &D_WSTAG840_800A6820, &D_WSTAG840_800A682C, &D_WSTAG840_800A6838 },
};
FieldstgListedBattle D_WSTAG840_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6880 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A688C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6898 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A68B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A68BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A68C8 = {
    0,
    { &D_WSTAG840_800A6868, &D_WSTAG840_800A6874, &D_WSTAG840_800A6880, &D_WSTAG840_800A688C, &D_WSTAG840_800A6898,
        &D_WSTAG840_800A68A4, &D_WSTAG840_800A68B0, &D_WSTAG840_800A68BC },
};
FieldstgListedBattle D_WSTAG840_800A68EC = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A68F8 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6904 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6910 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A691C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6928 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6934 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6940 = { 63, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A694C = {
    5,
    { &D_WSTAG840_800A68EC, &D_WSTAG840_800A68F8, &D_WSTAG840_800A6904, &D_WSTAG840_800A6910, &D_WSTAG840_800A691C,
        &D_WSTAG840_800A6928, &D_WSTAG840_800A6934, &D_WSTAG840_800A6940 },
};
FieldstgListedBattle D_WSTAG840_800A6970 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A697C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6988 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6994 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A69A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A69AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A69B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A69C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A69D0 = {
    0,
    { &D_WSTAG840_800A6970, &D_WSTAG840_800A697C, &D_WSTAG840_800A6988, &D_WSTAG840_800A6994, &D_WSTAG840_800A69A0,
        &D_WSTAG840_800A69AC, &D_WSTAG840_800A69B8, &D_WSTAG840_800A69C4 },
};
FieldstgListedBattle D_WSTAG840_800A69F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A48 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6A54 = {
    0,
    { &D_WSTAG840_800A69F4, &D_WSTAG840_800A6A00, &D_WSTAG840_800A6A0C, &D_WSTAG840_800A6A18, &D_WSTAG840_800A6A24,
        &D_WSTAG840_800A6A30, &D_WSTAG840_800A6A3C, &D_WSTAG840_800A6A48 },
};
FieldstgListedBattle D_WSTAG840_800A6A78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6A9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6AA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6AB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6AC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6ACC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6AD8 = {
    0,
    { &D_WSTAG840_800A6A78, &D_WSTAG840_800A6A84, &D_WSTAG840_800A6A90, &D_WSTAG840_800A6A9C, &D_WSTAG840_800A6AA8,
        &D_WSTAG840_800A6AB4, &D_WSTAG840_800A6AC0, &D_WSTAG840_800A6ACC },
};
FieldstgListedBattle D_WSTAG840_800A6AFC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B08 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B14 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B20 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B2C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B38 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B44 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6B50 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A6B5C = {
    3,
    { &D_WSTAG840_800A6AFC, &D_WSTAG840_800A6B08, &D_WSTAG840_800A6B14, &D_WSTAG840_800A6B20, &D_WSTAG840_800A6B2C,
        &D_WSTAG840_800A6B38, &D_WSTAG840_800A6B44, &D_WSTAG840_800A6B50 },
};
FieldstgListedBattle D_WSTAG840_800A6B80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6B8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6B98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6BA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6BB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6BBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6BC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6BD4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6BE0 = {
    0,
    { &D_WSTAG840_800A6B80, &D_WSTAG840_800A6B8C, &D_WSTAG840_800A6B98, &D_WSTAG840_800A6BA4, &D_WSTAG840_800A6BB0,
        &D_WSTAG840_800A6BBC, &D_WSTAG840_800A6BC8, &D_WSTAG840_800A6BD4 },
};
FieldstgListedBattle D_WSTAG840_800A6C04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C10 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C58 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6C64 = {
    0,
    { &D_WSTAG840_800A6C04, &D_WSTAG840_800A6C10, &D_WSTAG840_800A6C1C, &D_WSTAG840_800A6C28, &D_WSTAG840_800A6C34,
        &D_WSTAG840_800A6C40, &D_WSTAG840_800A6C4C, &D_WSTAG840_800A6C58 },
};
FieldstgListedBattle D_WSTAG840_800A6C88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6C94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6CDC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6CE8 = {
    0,
    { &D_WSTAG840_800A6C88, &D_WSTAG840_800A6C94, &D_WSTAG840_800A6CA0, &D_WSTAG840_800A6CAC, &D_WSTAG840_800A6CB8,
        &D_WSTAG840_800A6CC4, &D_WSTAG840_800A6CD0, &D_WSTAG840_800A6CDC },
};
FieldstgListedBattle D_WSTAG840_800A6D0C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D18 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D24 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D30 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D3C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D48 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D54 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6D60 = { 104, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A6D6C = {
    3,
    { &D_WSTAG840_800A6D0C, &D_WSTAG840_800A6D18, &D_WSTAG840_800A6D24, &D_WSTAG840_800A6D30, &D_WSTAG840_800A6D3C,
        &D_WSTAG840_800A6D48, &D_WSTAG840_800A6D54, &D_WSTAG840_800A6D60 },
};
FieldstgListedBattle D_WSTAG840_800A6D90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6D9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6DE4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6DF0 = {
    0,
    { &D_WSTAG840_800A6D90, &D_WSTAG840_800A6D9C, &D_WSTAG840_800A6DA8, &D_WSTAG840_800A6DB4, &D_WSTAG840_800A6DC0,
        &D_WSTAG840_800A6DCC, &D_WSTAG840_800A6DD8, &D_WSTAG840_800A6DE4 },
};
FieldstgListedBattle D_WSTAG840_800A6E14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6E68 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6E74 = {
    0,
    { &D_WSTAG840_800A6E14, &D_WSTAG840_800A6E20, &D_WSTAG840_800A6E2C, &D_WSTAG840_800A6E38, &D_WSTAG840_800A6E44,
        &D_WSTAG840_800A6E50, &D_WSTAG840_800A6E5C, &D_WSTAG840_800A6E68 },
};
FieldstgListedBattle D_WSTAG840_800A6E98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6ED4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6EEC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A6EF8 = {
    0,
    { &D_WSTAG840_800A6E98, &D_WSTAG840_800A6EA4, &D_WSTAG840_800A6EB0, &D_WSTAG840_800A6EBC, &D_WSTAG840_800A6EC8,
        &D_WSTAG840_800A6ED4, &D_WSTAG840_800A6EE0, &D_WSTAG840_800A6EEC },
};
FieldstgListedBattle D_WSTAG840_800A6F1C = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F28 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F34 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F40 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F4C = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F58 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F64 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A6F70 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A6F7C = {
    2,
    { &D_WSTAG840_800A6F1C, &D_WSTAG840_800A6F28, &D_WSTAG840_800A6F34, &D_WSTAG840_800A6F40, &D_WSTAG840_800A6F4C,
        &D_WSTAG840_800A6F58, &D_WSTAG840_800A6F64, &D_WSTAG840_800A6F70 },
};
FieldstgListedBattle D_WSTAG840_800A6FA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A6FF4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7000 = {
    0,
    { &D_WSTAG840_800A6FA0, &D_WSTAG840_800A6FAC, &D_WSTAG840_800A6FB8, &D_WSTAG840_800A6FC4, &D_WSTAG840_800A6FD0,
        &D_WSTAG840_800A6FDC, &D_WSTAG840_800A6FE8, &D_WSTAG840_800A6FF4 },
};
FieldstgListedBattle D_WSTAG840_800A7024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A703C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A706C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7078 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7084 = {
    0,
    { &D_WSTAG840_800A7024, &D_WSTAG840_800A7030, &D_WSTAG840_800A703C, &D_WSTAG840_800A7048, &D_WSTAG840_800A7054,
        &D_WSTAG840_800A7060, &D_WSTAG840_800A706C, &D_WSTAG840_800A7078 },
};
FieldstgListedBattle D_WSTAG840_800A70A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A70FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7108 = {
    0,
    { &D_WSTAG840_800A70A8, &D_WSTAG840_800A70B4, &D_WSTAG840_800A70C0, &D_WSTAG840_800A70CC, &D_WSTAG840_800A70D8,
        &D_WSTAG840_800A70E4, &D_WSTAG840_800A70F0, &D_WSTAG840_800A70FC },
};
FieldstgListedBattle D_WSTAG840_800A712C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7138 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7144 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7150 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A715C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7168 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7174 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7180 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A718C = {
    3,
    { &D_WSTAG840_800A712C, &D_WSTAG840_800A7138, &D_WSTAG840_800A7144, &D_WSTAG840_800A7150, &D_WSTAG840_800A715C,
        &D_WSTAG840_800A7168, &D_WSTAG840_800A7174, &D_WSTAG840_800A7180 },
};
FieldstgListedBattle D_WSTAG840_800A71B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A71F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7204 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7210 = {
    0,
    { &D_WSTAG840_800A71B0, &D_WSTAG840_800A71BC, &D_WSTAG840_800A71C8, &D_WSTAG840_800A71D4, &D_WSTAG840_800A71E0,
        &D_WSTAG840_800A71EC, &D_WSTAG840_800A71F8, &D_WSTAG840_800A7204 },
};
FieldstgListedBattle D_WSTAG840_800A7234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A724C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A727C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7288 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7294 = {
    0,
    { &D_WSTAG840_800A7234, &D_WSTAG840_800A7240, &D_WSTAG840_800A724C, &D_WSTAG840_800A7258, &D_WSTAG840_800A7264,
        &D_WSTAG840_800A7270, &D_WSTAG840_800A727C, &D_WSTAG840_800A7288 },
};
FieldstgListedBattle D_WSTAG840_800A72B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A72C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A72D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A72DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A72E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A72F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A730C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7318 = {
    0,
    { &D_WSTAG840_800A72B8, &D_WSTAG840_800A72C4, &D_WSTAG840_800A72D0, &D_WSTAG840_800A72DC, &D_WSTAG840_800A72E8,
        &D_WSTAG840_800A72F4, &D_WSTAG840_800A7300, &D_WSTAG840_800A730C },
};
FieldstgListedBattle D_WSTAG840_800A733C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7348 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7354 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7360 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A736C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7378 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7384 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7390 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A739C = {
    5,
    { &D_WSTAG840_800A733C, &D_WSTAG840_800A7348, &D_WSTAG840_800A7354, &D_WSTAG840_800A7360, &D_WSTAG840_800A736C,
        &D_WSTAG840_800A7378, &D_WSTAG840_800A7384, &D_WSTAG840_800A7390 },
};
FieldstgListedBattle D_WSTAG840_800A73C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A73CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A73D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A73E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A73F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A73FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7414 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7420 = {
    0,
    { &D_WSTAG840_800A73C0, &D_WSTAG840_800A73CC, &D_WSTAG840_800A73D8, &D_WSTAG840_800A73E4, &D_WSTAG840_800A73F0,
        &D_WSTAG840_800A73FC, &D_WSTAG840_800A7408, &D_WSTAG840_800A7414 },
};
FieldstgListedBattle D_WSTAG840_800A7444 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7450 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A745C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7468 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7474 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7480 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A748C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7498 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A74A4 = {
    0,
    { &D_WSTAG840_800A7444, &D_WSTAG840_800A7450, &D_WSTAG840_800A745C, &D_WSTAG840_800A7468, &D_WSTAG840_800A7474,
        &D_WSTAG840_800A7480, &D_WSTAG840_800A748C, &D_WSTAG840_800A7498 },
};
FieldstgListedBattle D_WSTAG840_800A74C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A74D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A74E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A74EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A74F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7510 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A751C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7528 = {
    0,
    { &D_WSTAG840_800A74C8, &D_WSTAG840_800A74D4, &D_WSTAG840_800A74E0, &D_WSTAG840_800A74EC, &D_WSTAG840_800A74F8,
        &D_WSTAG840_800A7504, &D_WSTAG840_800A7510, &D_WSTAG840_800A751C },
};
FieldstgListedBattle D_WSTAG840_800A754C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7558 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7564 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7570 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A757C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7588 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A7594 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG840_800A75A0 = { 104, 11, 0x60080000 };
FieldstgBattleList D_WSTAG840_800A75AC = {
    3,
    { &D_WSTAG840_800A754C, &D_WSTAG840_800A7558, &D_WSTAG840_800A7564, &D_WSTAG840_800A7570, &D_WSTAG840_800A757C,
        &D_WSTAG840_800A7588, &D_WSTAG840_800A7594, &D_WSTAG840_800A75A0 },
};
FieldstgListedBattle D_WSTAG840_800A75D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A75DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A75E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A75F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A760C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7618 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7624 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7630 = {
    0,
    { &D_WSTAG840_800A75D0, &D_WSTAG840_800A75DC, &D_WSTAG840_800A75E8, &D_WSTAG840_800A75F4, &D_WSTAG840_800A7600,
        &D_WSTAG840_800A760C, &D_WSTAG840_800A7618, &D_WSTAG840_800A7624 },
};
FieldstgListedBattle D_WSTAG840_800A7654 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7660 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A766C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7678 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7684 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7690 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A769C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A76A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A76B4 = {
    0,
    { &D_WSTAG840_800A7654, &D_WSTAG840_800A7660, &D_WSTAG840_800A766C, &D_WSTAG840_800A7678, &D_WSTAG840_800A7684,
        &D_WSTAG840_800A7690, &D_WSTAG840_800A769C, &D_WSTAG840_800A76A8 },
};
FieldstgListedBattle D_WSTAG840_800A76D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A76E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A76F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A76FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7708 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7714 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A7720 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG840_800A772C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG840_800A7738 = {
    0,
    { &D_WSTAG840_800A76D8, &D_WSTAG840_800A76E4, &D_WSTAG840_800A76F0, &D_WSTAG840_800A76FC, &D_WSTAG840_800A7708,
        &D_WSTAG840_800A7714, &D_WSTAG840_800A7720, &D_WSTAG840_800A772C },
};
FieldstgBattleLists wstag840_battle_lists[10] = {
    { 180, 3, 0, { &D_WSTAG840_800A631C, &D_WSTAG840_800A63A0, &D_WSTAG840_800A6424 }, &D_WSTAG840_800A64A8 },
    { 183, 4, 0, { &D_WSTAG840_800A652C, &D_WSTAG840_800A65B0, &D_WSTAG840_800A6634 }, &D_WSTAG840_800A66B8 },
    { 187, 5, 0, { &D_WSTAG840_800A673C, &D_WSTAG840_800A67C0, &D_WSTAG840_800A6844 }, &D_WSTAG840_800A68C8 },
    { 191, 6, 0, { &D_WSTAG840_800A694C, &D_WSTAG840_800A69D0, &D_WSTAG840_800A6A54 }, &D_WSTAG840_800A6AD8 },
    { 198, 9, 0, { &D_WSTAG840_800A6B5C, &D_WSTAG840_800A6BE0, &D_WSTAG840_800A6C64 }, &D_WSTAG840_800A6CE8 },
    { 208, 13, 0, { &D_WSTAG840_800A6D6C, &D_WSTAG840_800A6DF0, &D_WSTAG840_800A6E74 }, &D_WSTAG840_800A6EF8 },
    { 211, 14, 0, { &D_WSTAG840_800A6F7C, &D_WSTAG840_800A7000, &D_WSTAG840_800A7084 }, &D_WSTAG840_800A7108 },
    { 215, 15, 0, { &D_WSTAG840_800A718C, &D_WSTAG840_800A7210, &D_WSTAG840_800A7294 }, &D_WSTAG840_800A7318 },
    { 219, 16, 0, { &D_WSTAG840_800A739C, &D_WSTAG840_800A7420, &D_WSTAG840_800A74A4 }, &D_WSTAG840_800A7528 },
    { 226, 19, 0, { &D_WSTAG840_800A75AC, &D_WSTAG840_800A7630, &D_WSTAG840_800A76B4 }, &D_WSTAG840_800A7738 },
};
FieldstgVramPlace wstag840_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 417, 0, 161, 352, 511 },
};
FieldstgPlacedActor D_WSTAG840_800A78E4 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag840_actors[2] = { &D_WSTAG840_800A78E4, NULL };
FieldstgSprite wstag840_sprites[23] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 948, 29, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 311, 207, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 636, 93, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 871, 27, 0, 0 },
    { 1, 0, 0x8B, 2, 0, 0, 0, 0, 0, 0, 720, 158, 0, 0 }, { 1, 0, 0xA1, 2, 1, 0, 0, 0, 0, 0, 688, 136, 0, 0 },
    { 1, 0, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 262, 340, 0, 0 }, { 1, 0, 0x40, 2, 0xB, 0, 0, 0, 0, 0, 320, 330, 0, 0 },
    { 1, 0, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 384, 299, 0, 0 }, { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 448, 266, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 373, 118, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 490, 66, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 148, 233, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 753, 1, 0, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 860, -124, 120, 0 },
    { 1, 0, 0x8B, 4, 2, 0, 0, 0, 0, 0, 656, 118, 250, 0 }, { 1, 0, 0x8A, 4, 3, 0, 0, 0, 0, 0, 624, 104, 234, 0 },
    { 1, 0, 0x79, 4, 4, 0, 0, 0, 0, 0, 592, 89, 218, 0 }, { 1, 0, 0x98, 4, 5, 0, 0, 0, 0, 0, 560, 70, 202, 0 },
    { 1, 0, 0x55, 4, 7, 0, 0, 0, 0, 0, 400, 249, 338, 0 }, { 1, 0, 0x67, 4, 8, 0, 0, 0, 0, 0, 368, 233, 322, 0 },
    { 1, 0, 0x5E, 4, 9, 0, 0, 0, 0, 0, 352, 228, 314, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag840_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E3, 0x380, 0x70, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E3, 0xA0, 0x180, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag840_funcs = { wstag840_setup };
