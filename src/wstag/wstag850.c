#include "wstag.h"

/* WSTAG850: stage 0x2E5 (fieldstg_stages). */

extern WstagExits *wstag850_exits[];
extern WstagFuncs wstag850_funcs;
extern CVECTOR wstag850_color;
extern FieldstgBattleLists wstag850_battle_lists[];
extern FieldstgVramPlace wstag850_vram_places[];
extern FieldstgPlacedActor *wstag850_actors[];
extern FieldstgSprite wstag850_sprites[];
extern FieldstgMapEvent wstag850_map_events[];

s32 wstag850_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag850_set_exits(dst, list + 1, arg2, arg3);
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

void wstag850_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        wstag850_set_exits(fieldstg_stage.map_events, wstag850_exits, gamestate_data.route,
                               gamestate_data.room);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag850_start(void *arg0) {
    WstagObject *obj = object_new(wstag850_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag850_funcs.setup();
    return obj;
}

void wstag850_setup(void) {
    fieldstg_stage.background_file = 0x6E0;
    fieldstg_stage.sprite_file = 0x06E10000;
    fieldstg_stage.sprites = wstag850_sprites;
    fieldstg_stage.map_events = wstag850_map_events;
    fieldstg_stage.mask_file = 0x6DF;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x24000, 0x15200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag850_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag850_actors;
    fieldstg_stage.color = wstag850_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag850_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x06E10001);
    fieldstg_attr.set_file(7, 0x06E10002);
    fieldstg_attr.set_file(4, 0x06E10003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag850/nonmatchings/wstag850", wstag850_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag850_setup(void);

WstagExit D_WSTAG850_800A60F4 = { 0x2E4, 1, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG850_800A6104 = { 0x2E6, 1, 1, 0xA0, 0x150, 5, &D_WSTAG850_800A60F4 };
WstagExit D_WSTAG850_800A6114 = { 0x21D, 0, 0, 0x328, 0x424, 0, &D_WSTAG850_800A6104 };
WstagExits D_WSTAG850_800A6124 = { 1, 1, &D_WSTAG850_800A6114 };
WstagExit D_WSTAG850_800A612C = { 0x2E7, 1, 1, 0x250, 0xE8, 1, NULL };
WstagExit D_WSTAG850_800A613C = { 0x2E4, 1, 1, 0xC0, 0x180, 5, &D_WSTAG850_800A612C };
WstagExit D_WSTAG850_800A614C = { 0x234, 0, 0, 0x320, 0x1C8, 0, &D_WSTAG850_800A613C };
WstagExits D_WSTAG850_800A615C = { 1, 2, &D_WSTAG850_800A614C };
WstagExit D_WSTAG850_800A6164 = { 0x2E2, 4, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG850_800A6174 = { 0x2E4, 4, 1, 0xC0, 0x180, 5, &D_WSTAG850_800A6164 };
WstagExit D_WSTAG850_800A6184 = { 0x23B, 0, 0, 0x360, 0x12C, 0, &D_WSTAG850_800A6174 };
WstagExits D_WSTAG850_800A6194 = { 4, 1, &D_WSTAG850_800A6184 };
WstagExit D_WSTAG850_800A619C = { 0x2E4, 0xB, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG850_800A61AC = { 0x2E6, 0xB, 1, 0xA0, 0x150, 5, &D_WSTAG850_800A619C };
WstagExit D_WSTAG850_800A61BC = { 0x28C, 0, 0, 0x328, 0x424, 0, &D_WSTAG850_800A61AC };
WstagExits D_WSTAG850_800A61CC = { 11, 1, &D_WSTAG850_800A61BC };
WstagExit D_WSTAG850_800A61D4 = { 0x2E7, 0xB, 1, 0x250, 0xE8, 1, NULL };
WstagExit D_WSTAG850_800A61E4 = { 0x2E4, 0xB, 1, 0xC0, 0x180, 5, &D_WSTAG850_800A61D4 };
WstagExit D_WSTAG850_800A61F4 = { 0x2A2, 0, 0, 0x320, 0x1C8, 0, &D_WSTAG850_800A61E4 };
WstagExits D_WSTAG850_800A6204 = { 11, 2, &D_WSTAG850_800A61F4 };
WstagExit D_WSTAG850_800A620C = { 0x2E2, 0xE, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG850_800A621C = { 0x2E4, 0xE, 1, 0xC0, 0x180, 5, &D_WSTAG850_800A620C };
WstagExit D_WSTAG850_800A622C = { 0x2A8, 0, 0, 0x360, 0x12C, 0, &D_WSTAG850_800A621C };
WstagExits D_WSTAG850_800A623C = { 14, 1, &D_WSTAG850_800A622C };
WstagExits D_WSTAG850_800A6244 = { 0, 0, &D_WSTAG850_800A6114 };
WstagExits *wstag850_exits[8] = {
    &D_WSTAG850_800A6124, &D_WSTAG850_800A615C, &D_WSTAG850_800A6194, &D_WSTAG850_800A61CC, &D_WSTAG850_800A6204,
    &D_WSTAG850_800A623C, &D_WSTAG850_800A6244, NULL,
};
FieldstgListedBattle D_WSTAG850_800A626C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6278 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6284 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6290 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A629C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A62A8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A62B4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A62C0 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG850_800A62CC = {
    2,
    { &D_WSTAG850_800A626C, &D_WSTAG850_800A6278, &D_WSTAG850_800A6284, &D_WSTAG850_800A6290, &D_WSTAG850_800A629C,
        &D_WSTAG850_800A62A8, &D_WSTAG850_800A62B4, &D_WSTAG850_800A62C0 },
};
FieldstgListedBattle D_WSTAG850_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6344 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6350 = {
    0,
    { &D_WSTAG850_800A62F0, &D_WSTAG850_800A62FC, &D_WSTAG850_800A6308, &D_WSTAG850_800A6314, &D_WSTAG850_800A6320,
        &D_WSTAG850_800A632C, &D_WSTAG850_800A6338, &D_WSTAG850_800A6344 },
};
FieldstgListedBattle D_WSTAG850_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A63C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A63D4 = {
    0,
    { &D_WSTAG850_800A6374, &D_WSTAG850_800A6380, &D_WSTAG850_800A638C, &D_WSTAG850_800A6398, &D_WSTAG850_800A63A4,
        &D_WSTAG850_800A63B0, &D_WSTAG850_800A63BC, &D_WSTAG850_800A63C8 },
};
FieldstgListedBattle D_WSTAG850_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6410 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A644C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6458 = {
    0,
    { &D_WSTAG850_800A63F8, &D_WSTAG850_800A6404, &D_WSTAG850_800A6410, &D_WSTAG850_800A641C, &D_WSTAG850_800A6428,
        &D_WSTAG850_800A6434, &D_WSTAG850_800A6440, &D_WSTAG850_800A644C },
};
FieldstgListedBattle D_WSTAG850_800A647C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6488 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6494 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A64A0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A64AC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A64B8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A64C4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A64D0 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG850_800A64DC = {
    2,
    { &D_WSTAG850_800A647C, &D_WSTAG850_800A6488, &D_WSTAG850_800A6494, &D_WSTAG850_800A64A0, &D_WSTAG850_800A64AC,
        &D_WSTAG850_800A64B8, &D_WSTAG850_800A64C4, &D_WSTAG850_800A64D0 },
};
FieldstgListedBattle D_WSTAG850_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6554 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6560 = {
    0,
    { &D_WSTAG850_800A6500, &D_WSTAG850_800A650C, &D_WSTAG850_800A6518, &D_WSTAG850_800A6524, &D_WSTAG850_800A6530,
        &D_WSTAG850_800A653C, &D_WSTAG850_800A6548, &D_WSTAG850_800A6554 },
};
FieldstgListedBattle D_WSTAG850_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A659C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A65A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A65B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A65C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A65CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A65D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A65E4 = {
    0,
    { &D_WSTAG850_800A6584, &D_WSTAG850_800A6590, &D_WSTAG850_800A659C, &D_WSTAG850_800A65A8, &D_WSTAG850_800A65B4,
        &D_WSTAG850_800A65C0, &D_WSTAG850_800A65CC, &D_WSTAG850_800A65D8 },
};
FieldstgListedBattle D_WSTAG850_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A665C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6668 = {
    0,
    { &D_WSTAG850_800A6608, &D_WSTAG850_800A6614, &D_WSTAG850_800A6620, &D_WSTAG850_800A662C, &D_WSTAG850_800A6638,
        &D_WSTAG850_800A6644, &D_WSTAG850_800A6650, &D_WSTAG850_800A665C },
};
FieldstgListedBattle D_WSTAG850_800A668C = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A6698 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66A4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66B0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66BC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66C8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66D4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A66E0 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG850_800A66EC = {
    2,
    { &D_WSTAG850_800A668C, &D_WSTAG850_800A6698, &D_WSTAG850_800A66A4, &D_WSTAG850_800A66B0, &D_WSTAG850_800A66BC,
        &D_WSTAG850_800A66C8, &D_WSTAG850_800A66D4, &D_WSTAG850_800A66E0 },
};
FieldstgListedBattle D_WSTAG850_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A674C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6758 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6764 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6770 = {
    0,
    { &D_WSTAG850_800A6710, &D_WSTAG850_800A671C, &D_WSTAG850_800A6728, &D_WSTAG850_800A6734, &D_WSTAG850_800A6740,
        &D_WSTAG850_800A674C, &D_WSTAG850_800A6758, &D_WSTAG850_800A6764 },
};
FieldstgListedBattle D_WSTAG850_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A67E8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A67F4 = {
    0,
    { &D_WSTAG850_800A6794, &D_WSTAG850_800A67A0, &D_WSTAG850_800A67AC, &D_WSTAG850_800A67B8, &D_WSTAG850_800A67C4,
        &D_WSTAG850_800A67D0, &D_WSTAG850_800A67DC, &D_WSTAG850_800A67E8 },
};
FieldstgListedBattle D_WSTAG850_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6860 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A686C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6878 = {
    0,
    { &D_WSTAG850_800A6818, &D_WSTAG850_800A6824, &D_WSTAG850_800A6830, &D_WSTAG850_800A683C, &D_WSTAG850_800A6848,
        &D_WSTAG850_800A6854, &D_WSTAG850_800A6860, &D_WSTAG850_800A686C },
};
FieldstgListedBattle D_WSTAG850_800A689C = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68A8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68B4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68C0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68CC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68D8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68E4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG850_800A68F0 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG850_800A68FC = {
    2,
    { &D_WSTAG850_800A689C, &D_WSTAG850_800A68A8, &D_WSTAG850_800A68B4, &D_WSTAG850_800A68C0, &D_WSTAG850_800A68CC,
        &D_WSTAG850_800A68D8, &D_WSTAG850_800A68E4, &D_WSTAG850_800A68F0 },
};
FieldstgListedBattle D_WSTAG850_800A6920 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A692C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6938 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6944 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6950 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A695C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6968 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6974 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6980 = {
    0,
    { &D_WSTAG850_800A6920, &D_WSTAG850_800A692C, &D_WSTAG850_800A6938, &D_WSTAG850_800A6944, &D_WSTAG850_800A6950,
        &D_WSTAG850_800A695C, &D_WSTAG850_800A6968, &D_WSTAG850_800A6974 },
};
FieldstgListedBattle D_WSTAG850_800A69A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A69F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6A04 = {
    0,
    { &D_WSTAG850_800A69A4, &D_WSTAG850_800A69B0, &D_WSTAG850_800A69BC, &D_WSTAG850_800A69C8, &D_WSTAG850_800A69D4,
        &D_WSTAG850_800A69E0, &D_WSTAG850_800A69EC, &D_WSTAG850_800A69F8 },
};
FieldstgListedBattle D_WSTAG850_800A6A28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG850_800A6A7C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG850_800A6A88 = {
    0,
    { &D_WSTAG850_800A6A28, &D_WSTAG850_800A6A34, &D_WSTAG850_800A6A40, &D_WSTAG850_800A6A4C, &D_WSTAG850_800A6A58,
        &D_WSTAG850_800A6A64, &D_WSTAG850_800A6A70, &D_WSTAG850_800A6A7C },
};
FieldstgBattleLists wstag850_battle_lists[4] = {
    { 175, 1, 0, { &D_WSTAG850_800A62CC, &D_WSTAG850_800A6350, &D_WSTAG850_800A63D4 }, &D_WSTAG850_800A6458 },
    { 185, 4, 0, { &D_WSTAG850_800A64DC, &D_WSTAG850_800A6560, &D_WSTAG850_800A65E4 }, &D_WSTAG850_800A6668 },
    { 203, 11, 0, { &D_WSTAG850_800A66EC, &D_WSTAG850_800A6770, &D_WSTAG850_800A67F4 }, &D_WSTAG850_800A6878 },
    { 213, 14, 0, { &D_WSTAG850_800A68FC, &D_WSTAG850_800A6980, &D_WSTAG850_800A6A04 }, &D_WSTAG850_800A6A88 },
};
FieldstgVramPlace wstag850_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 463, 192, 207, 352, 511 },
};
FieldstgPlacedActor D_WSTAG850_800A6B8C = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag850_actors[2] = { &D_WSTAG850_800A6B8C, NULL };
FieldstgSprite wstag850_sprites[16] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 231, 159, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 386, 203, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 943, 97, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 587, 241, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 783, 147, 0, 0 },
    { 1, 0, 0xFF, 2, 0x33, 2, 0, 7, 0x18, 0, 540, -76, 0, 0 }, { 1, 0, 0x55, 2, 0, 0, 0, 0, 0, 0, 384, 299, 0, 0 },
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 448, 344, 0, 0 }, { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 640, 339, 0, 0 },
    { 1, 0, 0x52, 2, 3, 0, 0, 0, 0, 0, 704, 302, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 669, 89, 0, 0 },
    { 1, 0, 0xB6, 6, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 538, 16, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 448, 180, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 848, 69, 0, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 540, 52, 296, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag850_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0x240, 0x120, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0x3B0, 0xD8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0xE0, 0x120, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag850_funcs = { wstag850_setup };
