#include "wstag.h"

/* WSTAG860: stage 0x2E7 (fieldstg_stages). */

extern WstagExits *wstag860_exits[];
extern WstagFuncs wstag860_funcs;
const CVECTOR wstag860_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgBattleLists wstag860_battle_lists[];
extern FieldstgVramPlace wstag860_vram_places[];
extern FieldstgPlacedActor *wstag860_actors[];
extern FieldstgSprite wstag860_sprites[];
extern FieldstgMapEvent wstag860_map_events[];

s32 wstag860_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag860_set_exits(dst, list + 1, arg2, arg3);
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

void wstag860_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        wstag860_set_exits(fieldstg_stage.map_events, wstag860_exits, gamestate_data.route,
                               gamestate_data.room);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag860_start(void *arg0) {
    WstagObject *obj = object_new(wstag860_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag860_funcs.setup();
    return obj;
}

void wstag860_setup(void) {
    fieldstg_stage.background_file = 0x6F4;
    fieldstg_stage.sprite_file = 0x06F50000;
    fieldstg_stage.sprites = wstag860_sprites;
    fieldstg_stage.map_events = wstag860_map_events;
    fieldstg_stage.mask_file = 0x6F3;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xEC00, 0x14D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag860_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag860_actors;
    fieldstg_stage.color = wstag860_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag860_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x06F50001);
    fieldstg_attr.set_file(7, 0x06F50002);
    fieldstg_attr.set_file(4, 0x06F50003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag860_setup(void);

WstagExit D_WSTAG860_800A60F0 = { 0x2E5, 1, 2, 0xE0, 0x120, 5, NULL };
WstagExits D_WSTAG860_800A6100 = { 1, 1, &D_WSTAG860_800A60F0 };
WstagExit D_WSTAG860_800A6108 = { 0x2E4, 6, 1, 0xC0, 0x180, 5, NULL };
WstagExits D_WSTAG860_800A6118 = { 6, 1, &D_WSTAG860_800A6108 };
WstagExit D_WSTAG860_800A6120 = { 0x2E5, 0xB, 2, 0xE0, 0x120, 5, NULL };
WstagExits D_WSTAG860_800A6130 = { 11, 1, &D_WSTAG860_800A6120 };
WstagExit D_WSTAG860_800A6138 = { 0x2E4, 0x10, 1, 0xC0, 0x180, 5, NULL };
WstagExits D_WSTAG860_800A6148 = { 16, 1, &D_WSTAG860_800A6138 };
WstagExits D_WSTAG860_800A6150 = { 0, 0, &D_WSTAG860_800A60F0 };
WstagExits *wstag860_exits[6] = {
    &D_WSTAG860_800A6100, &D_WSTAG860_800A6118, &D_WSTAG860_800A6130, &D_WSTAG860_800A6148, &D_WSTAG860_800A6150,
    NULL,
};
FieldstgListedBattle D_WSTAG860_800A6170 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A617C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A6188 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A6194 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A61A0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A61AC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A61B8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A61C4 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG860_800A61D0 = {
    2,
    { &D_WSTAG860_800A6170, &D_WSTAG860_800A617C, &D_WSTAG860_800A6188, &D_WSTAG860_800A6194, &D_WSTAG860_800A61A0,
        &D_WSTAG860_800A61AC, &D_WSTAG860_800A61B8, &D_WSTAG860_800A61C4 },
};
FieldstgListedBattle D_WSTAG860_800A61F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6200 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A620C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6218 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6230 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A623C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6248 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A6254 = {
    0,
    { &D_WSTAG860_800A61F4, &D_WSTAG860_800A6200, &D_WSTAG860_800A620C, &D_WSTAG860_800A6218, &D_WSTAG860_800A6224,
        &D_WSTAG860_800A6230, &D_WSTAG860_800A623C, &D_WSTAG860_800A6248 },
};
FieldstgListedBattle D_WSTAG860_800A6278 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6284 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6290 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A629C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A62A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A62C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A62CC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A62D8 = {
    0,
    { &D_WSTAG860_800A6278, &D_WSTAG860_800A6284, &D_WSTAG860_800A6290, &D_WSTAG860_800A629C, &D_WSTAG860_800A62A8,
        &D_WSTAG860_800A62B4, &D_WSTAG860_800A62C0, &D_WSTAG860_800A62CC },
};
FieldstgListedBattle D_WSTAG860_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6344 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6350 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A635C = {
    0,
    { &D_WSTAG860_800A62FC, &D_WSTAG860_800A6308, &D_WSTAG860_800A6314, &D_WSTAG860_800A6320, &D_WSTAG860_800A632C,
        &D_WSTAG860_800A6338, &D_WSTAG860_800A6344, &D_WSTAG860_800A6350 },
};
FieldstgListedBattle D_WSTAG860_800A6380 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A638C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A6398 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A63A4 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A63B0 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A63BC = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A63C8 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A63D4 = { 63, 11, 0x60080000 };
FieldstgBattleList D_WSTAG860_800A63E0 = {
    5,
    { &D_WSTAG860_800A6380, &D_WSTAG860_800A638C, &D_WSTAG860_800A6398, &D_WSTAG860_800A63A4, &D_WSTAG860_800A63B0,
        &D_WSTAG860_800A63BC, &D_WSTAG860_800A63C8, &D_WSTAG860_800A63D4 },
};
FieldstgListedBattle D_WSTAG860_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6410 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6458 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A6464 = {
    0,
    { &D_WSTAG860_800A6404, &D_WSTAG860_800A6410, &D_WSTAG860_800A641C, &D_WSTAG860_800A6428, &D_WSTAG860_800A6434,
        &D_WSTAG860_800A6440, &D_WSTAG860_800A644C, &D_WSTAG860_800A6458 },
};
FieldstgListedBattle D_WSTAG860_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6494 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A64DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A64E8 = {
    0,
    { &D_WSTAG860_800A6488, &D_WSTAG860_800A6494, &D_WSTAG860_800A64A0, &D_WSTAG860_800A64AC, &D_WSTAG860_800A64B8,
        &D_WSTAG860_800A64C4, &D_WSTAG860_800A64D0, &D_WSTAG860_800A64DC },
};
FieldstgListedBattle D_WSTAG860_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6560 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A656C = {
    0,
    { &D_WSTAG860_800A650C, &D_WSTAG860_800A6518, &D_WSTAG860_800A6524, &D_WSTAG860_800A6530, &D_WSTAG860_800A653C,
        &D_WSTAG860_800A6548, &D_WSTAG860_800A6554, &D_WSTAG860_800A6560 },
};
FieldstgListedBattle D_WSTAG860_800A6590 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A659C = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65A8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65B4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65C0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65CC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65D8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A65E4 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG860_800A65F0 = {
    2,
    { &D_WSTAG860_800A6590, &D_WSTAG860_800A659C, &D_WSTAG860_800A65A8, &D_WSTAG860_800A65B4, &D_WSTAG860_800A65C0,
        &D_WSTAG860_800A65CC, &D_WSTAG860_800A65D8, &D_WSTAG860_800A65E4 },
};
FieldstgListedBattle D_WSTAG860_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A665C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6668 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A6674 = {
    0,
    { &D_WSTAG860_800A6614, &D_WSTAG860_800A6620, &D_WSTAG860_800A662C, &D_WSTAG860_800A6638, &D_WSTAG860_800A6644,
        &D_WSTAG860_800A6650, &D_WSTAG860_800A665C, &D_WSTAG860_800A6668 },
};
FieldstgListedBattle D_WSTAG860_800A6698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A66EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A66F8 = {
    0,
    { &D_WSTAG860_800A6698, &D_WSTAG860_800A66A4, &D_WSTAG860_800A66B0, &D_WSTAG860_800A66BC, &D_WSTAG860_800A66C8,
        &D_WSTAG860_800A66D4, &D_WSTAG860_800A66E0, &D_WSTAG860_800A66EC },
};
FieldstgListedBattle D_WSTAG860_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A674C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6758 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6764 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6770 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A677C = {
    0,
    { &D_WSTAG860_800A671C, &D_WSTAG860_800A6728, &D_WSTAG860_800A6734, &D_WSTAG860_800A6740, &D_WSTAG860_800A674C,
        &D_WSTAG860_800A6758, &D_WSTAG860_800A6764, &D_WSTAG860_800A6770 },
};
FieldstgListedBattle D_WSTAG860_800A67A0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67AC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67B8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67C4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67D0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67DC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67E8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG860_800A67F4 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG860_800A6800 = {
    5,
    { &D_WSTAG860_800A67A0, &D_WSTAG860_800A67AC, &D_WSTAG860_800A67B8, &D_WSTAG860_800A67C4, &D_WSTAG860_800A67D0,
        &D_WSTAG860_800A67DC, &D_WSTAG860_800A67E8, &D_WSTAG860_800A67F4 },
};
FieldstgListedBattle D_WSTAG860_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6860 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A686C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6878 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A6884 = {
    0,
    { &D_WSTAG860_800A6824, &D_WSTAG860_800A6830, &D_WSTAG860_800A683C, &D_WSTAG860_800A6848, &D_WSTAG860_800A6854,
        &D_WSTAG860_800A6860, &D_WSTAG860_800A686C, &D_WSTAG860_800A6878 },
};
FieldstgListedBattle D_WSTAG860_800A68A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A68FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A6908 = {
    0,
    { &D_WSTAG860_800A68A8, &D_WSTAG860_800A68B4, &D_WSTAG860_800A68C0, &D_WSTAG860_800A68CC, &D_WSTAG860_800A68D8,
        &D_WSTAG860_800A68E4, &D_WSTAG860_800A68F0, &D_WSTAG860_800A68FC },
};
FieldstgListedBattle D_WSTAG860_800A692C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6938 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6944 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6950 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A695C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6968 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6974 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG860_800A6980 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG860_800A698C = {
    0,
    { &D_WSTAG860_800A692C, &D_WSTAG860_800A6938, &D_WSTAG860_800A6944, &D_WSTAG860_800A6950, &D_WSTAG860_800A695C,
        &D_WSTAG860_800A6968, &D_WSTAG860_800A6974, &D_WSTAG860_800A6980 },
};
FieldstgBattleLists wstag860_battle_lists[4] = {
    { 177, 1, 0, { &D_WSTAG860_800A61D0, &D_WSTAG860_800A6254, &D_WSTAG860_800A62D8 }, &D_WSTAG860_800A635C },
    { 193, 6, 0, { &D_WSTAG860_800A63E0, &D_WSTAG860_800A6464, &D_WSTAG860_800A64E8 }, &D_WSTAG860_800A656C },
    { 205, 11, 0, { &D_WSTAG860_800A65F0, &D_WSTAG860_800A6674, &D_WSTAG860_800A66F8 }, &D_WSTAG860_800A677C },
    { 221, 16, 0, { &D_WSTAG860_800A6800, &D_WSTAG860_800A6884, &D_WSTAG860_800A6908 }, &D_WSTAG860_800A698C },
};
FieldstgVramPlace wstag860_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 366, 351, 184, 95, 336, 511 }, { 320, 256, 366, 383, 184, 127, 352, 511 },
    { 320, 256, 352, 352, 128, 96, 368, 511 },
};
u16 D_WSTAG860_800A6AB0[6] = { 0x22C, 1, 0x8236, 1, 0xFFFF, 0 };
u16 D_WSTAG860_800A6ABC[6] = { 0x255, 1, 0x8240, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG860_800A6AC8[2] = { { NULL, D_WSTAG860_800A6AB0, 846 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG860_800A6AE0[2] = { { NULL, D_WSTAG860_800A6ABC, 847 }, { NULL, NULL, 0 } };
u16 D_WSTAG860_800A6AF8[8] = { 0x7E05, 1, 0x7E1E, 1, 0x22C, 0, 0xFFFF, 0 };
u16 D_WSTAG860_800A6B08[8] = { 0x7E0F, 1, 0x7E1E, 1, 0x255, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG860_800A6B18 = { D_WSTAG860_800A6AF8, D_WSTAG860_800A6AC8, 33, 4, 192, 280, 1 };
FieldstgPlacedActor D_WSTAG860_800A6B2C = { D_WSTAG860_800A6B08, D_WSTAG860_800A6AE0, 77, 5, 192, 280, 1 };
FieldstgPlacedActor D_WSTAG860_800A6B40 = { NULL, NULL, 327, 6, 0, 0, 0 };
FieldstgPlacedActor *wstag860_actors[4] = {
    &D_WSTAG860_800A6B18, &D_WSTAG860_800A6B2C, &D_WSTAG860_800A6B40, NULL,
};
FieldstgSprite wstag860_sprites[13] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 360, 185, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 589, 120, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 268, 192, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 512, 122, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 110, 120, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 412, 114, 0, 0 },
    { 1, 0, 0x4A, 4, 0, 0, 0, 0, 0, 0, 416, 215, 288, 0 }, { 1, 0, 0x61, 4, 1, 0, 0, 0, 0, 0, 384, 198, 288, 0 },
    { 1, 0, 0x5F, 4, 2, 0, 0, 0, 0, 0, 365, 191, 281, 0 }, { 1, 0, 0x4F, 4, 3, 0, 0, 0, 0, 0, 592, 173, 251, 0 },
    { 1, 0, 0x63, 4, 4, 0, 0, 0, 0, 0, 560, 157, 247, 0 }, { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 541, 150, 240, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag860_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E7, 0x250, 0xE8, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag860_funcs = { wstag860_setup };
