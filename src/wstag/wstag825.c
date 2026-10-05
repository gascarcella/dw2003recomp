#include "wstag.h"

/* WSTAG825: stage 0x2E0 (fieldstg_stages). */

extern WstagExits *wstag825_exits[];
extern WstagFuncs wstag825_funcs;
const CVECTOR wstag825_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgBattleLists wstag825_battle_lists[];
extern FieldstgVramPlace wstag825_vram_places[];
extern FieldstgPlacedActor *wstag825_actors[];
extern FieldstgSprite wstag825_sprites[];
extern FieldstgMapEvent wstag825_map_events[];

s32 wstag825_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag825_set_exits(dst, list + 1, arg2, arg3);
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

void wstag825_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        wstag825_set_exits(fieldstg_stage.map_events, wstag825_exits, gamestate_data.route,
                               gamestate_data.room);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag825_start(void *arg0) {
    WstagObject *obj = object_new(wstag825_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag825_funcs.setup();
    return obj;
}

void wstag825_setup(void) {
    fieldstg_stage.background_file = 0x69F;
    fieldstg_stage.sprite_file = 0x06A00000;
    fieldstg_stage.sprites = wstag825_sprites;
    fieldstg_stage.map_events = wstag825_map_events;
    fieldstg_stage.mask_file = 0x69E;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x9D00, 0x17F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag825_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag825_actors;
    fieldstg_stage.color = wstag825_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag825_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x06A00001);
    fieldstg_attr.set_file(7, 0x06A00002);
    fieldstg_attr.set_file(4, 0x06A00003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag825_setup(void);

WstagExit D_WSTAG825_800A60F0 = { 0x2E6, 1, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG825_800A6100 = { 0x22A, 0, 0, 0x32C, 0x232, 0, &D_WSTAG825_800A60F0 };
WstagExits D_WSTAG825_800A6110 = { 1, 1, &D_WSTAG825_800A6100 };
WstagExit D_WSTAG825_800A6118 = { 0x2E1, 2, 1, 0x390, 0x108, 1, NULL };
WstagExit D_WSTAG825_800A6128 = { 0x21B, 0, 0, 0x80, 0x1E0, 0, &D_WSTAG825_800A6118 };
WstagExits D_WSTAG825_800A6138 = { 2, 1, &D_WSTAG825_800A6128 };
WstagExit D_WSTAG825_800A6140 = { 0x2E2, 7, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG825_800A6150 = { 0x241, 0, 0, 0xD0, 0x400, 0, &D_WSTAG825_800A6140 };
WstagExits D_WSTAG825_800A6160 = { 7, 1, &D_WSTAG825_800A6150 };
WstagExit D_WSTAG825_800A6168 = { 0x2E2, 8, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG825_800A6178 = { 0x228, 0, 0, 0x210, 0x370, 0, &D_WSTAG825_800A6168 };
WstagExits D_WSTAG825_800A6188 = { 8, 1, &D_WSTAG825_800A6178 };
WstagExit D_WSTAG825_800A6190 = { 0x2E6, 0xB, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG825_800A61A0 = { 0x299, 0, 0, 0x32C, 0x232, 0, &D_WSTAG825_800A6190 };
WstagExits D_WSTAG825_800A61B0 = { 11, 1, &D_WSTAG825_800A61A0 };
WstagExit D_WSTAG825_800A61B8 = { 0x2E1, 0xC, 1, 0x390, 0x108, 1, NULL };
WstagExit D_WSTAG825_800A61C8 = { 0x28A, 0, 0, 0x80, 0x1E0, 0, &D_WSTAG825_800A61B8 };
WstagExits D_WSTAG825_800A61D8 = { 12, 1, &D_WSTAG825_800A61C8 };
WstagExit D_WSTAG825_800A61E0 = { 0x2E2, 0x11, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG825_800A61F0 = { 0x2AE, 0, 0, 0xD0, 0x400, 0, &D_WSTAG825_800A61E0 };
WstagExits D_WSTAG825_800A6200 = { 17, 1, &D_WSTAG825_800A61F0 };
WstagExit D_WSTAG825_800A6208 = { 0x2E2, 0x12, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG825_800A6218 = { 0x297, 0, 0, 0x210, 0x370, 0, &D_WSTAG825_800A6208 };
WstagExits D_WSTAG825_800A6228 = { 18, 1, &D_WSTAG825_800A6218 };
WstagExits D_WSTAG825_800A6230 = { 0, 0, &D_WSTAG825_800A6100 };
WstagExits *wstag825_exits[10] = {
    &D_WSTAG825_800A6110, &D_WSTAG825_800A6138, &D_WSTAG825_800A6160, &D_WSTAG825_800A6188, &D_WSTAG825_800A61B0,
    &D_WSTAG825_800A61D8, &D_WSTAG825_800A6200, &D_WSTAG825_800A6228, &D_WSTAG825_800A6230, NULL,
};
FieldstgListedBattle D_WSTAG825_800A6260 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A626C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6278 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6284 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6290 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A629C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A62A8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A62B4 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A62C0 = {
    2,
    { &D_WSTAG825_800A6260, &D_WSTAG825_800A626C, &D_WSTAG825_800A6278, &D_WSTAG825_800A6284, &D_WSTAG825_800A6290,
        &D_WSTAG825_800A629C, &D_WSTAG825_800A62A8, &D_WSTAG825_800A62B4 },
};
FieldstgListedBattle D_WSTAG825_800A62E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6338 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6344 = {
    0,
    { &D_WSTAG825_800A62E4, &D_WSTAG825_800A62F0, &D_WSTAG825_800A62FC, &D_WSTAG825_800A6308, &D_WSTAG825_800A6314,
        &D_WSTAG825_800A6320, &D_WSTAG825_800A632C, &D_WSTAG825_800A6338 },
};
FieldstgListedBattle D_WSTAG825_800A6368 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A63BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A63C8 = {
    0,
    { &D_WSTAG825_800A6368, &D_WSTAG825_800A6374, &D_WSTAG825_800A6380, &D_WSTAG825_800A638C, &D_WSTAG825_800A6398,
        &D_WSTAG825_800A63A4, &D_WSTAG825_800A63B0, &D_WSTAG825_800A63BC },
};
FieldstgListedBattle D_WSTAG825_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6410 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6440 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A644C = {
    0,
    { &D_WSTAG825_800A63EC, &D_WSTAG825_800A63F8, &D_WSTAG825_800A6404, &D_WSTAG825_800A6410, &D_WSTAG825_800A641C,
        &D_WSTAG825_800A6428, &D_WSTAG825_800A6434, &D_WSTAG825_800A6440 },
};
FieldstgListedBattle D_WSTAG825_800A6470 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A647C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6488 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6494 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A64A0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A64AC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A64B8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A64C4 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A64D0 = {
    4,
    { &D_WSTAG825_800A6470, &D_WSTAG825_800A647C, &D_WSTAG825_800A6488, &D_WSTAG825_800A6494, &D_WSTAG825_800A64A0,
        &D_WSTAG825_800A64AC, &D_WSTAG825_800A64B8, &D_WSTAG825_800A64C4 },
};
FieldstgListedBattle D_WSTAG825_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6548 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6554 = {
    0,
    { &D_WSTAG825_800A64F4, &D_WSTAG825_800A6500, &D_WSTAG825_800A650C, &D_WSTAG825_800A6518, &D_WSTAG825_800A6524,
        &D_WSTAG825_800A6530, &D_WSTAG825_800A653C, &D_WSTAG825_800A6548 },
};
FieldstgListedBattle D_WSTAG825_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A659C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A65A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A65B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A65C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A65CC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A65D8 = {
    0,
    { &D_WSTAG825_800A6578, &D_WSTAG825_800A6584, &D_WSTAG825_800A6590, &D_WSTAG825_800A659C, &D_WSTAG825_800A65A8,
        &D_WSTAG825_800A65B4, &D_WSTAG825_800A65C0, &D_WSTAG825_800A65CC },
};
FieldstgListedBattle D_WSTAG825_800A65FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6650 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A665C = {
    0,
    { &D_WSTAG825_800A65FC, &D_WSTAG825_800A6608, &D_WSTAG825_800A6614, &D_WSTAG825_800A6620, &D_WSTAG825_800A662C,
        &D_WSTAG825_800A6638, &D_WSTAG825_800A6644, &D_WSTAG825_800A6650 },
};
FieldstgListedBattle D_WSTAG825_800A6680 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A668C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6698 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A66A4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A66B0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A66BC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A66C8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A66D4 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A66E0 = {
    4,
    { &D_WSTAG825_800A6680, &D_WSTAG825_800A668C, &D_WSTAG825_800A6698, &D_WSTAG825_800A66A4, &D_WSTAG825_800A66B0,
        &D_WSTAG825_800A66BC, &D_WSTAG825_800A66C8, &D_WSTAG825_800A66D4 },
};
FieldstgListedBattle D_WSTAG825_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6740 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A674C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6758 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6764 = {
    0,
    { &D_WSTAG825_800A6704, &D_WSTAG825_800A6710, &D_WSTAG825_800A671C, &D_WSTAG825_800A6728, &D_WSTAG825_800A6734,
        &D_WSTAG825_800A6740, &D_WSTAG825_800A674C, &D_WSTAG825_800A6758 },
};
FieldstgListedBattle D_WSTAG825_800A6788 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A67DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A67E8 = {
    0,
    { &D_WSTAG825_800A6788, &D_WSTAG825_800A6794, &D_WSTAG825_800A67A0, &D_WSTAG825_800A67AC, &D_WSTAG825_800A67B8,
        &D_WSTAG825_800A67C4, &D_WSTAG825_800A67D0, &D_WSTAG825_800A67DC },
};
FieldstgListedBattle D_WSTAG825_800A680C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6860 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A686C = {
    0,
    { &D_WSTAG825_800A680C, &D_WSTAG825_800A6818, &D_WSTAG825_800A6824, &D_WSTAG825_800A6830, &D_WSTAG825_800A683C,
        &D_WSTAG825_800A6848, &D_WSTAG825_800A6854, &D_WSTAG825_800A6860 },
};
FieldstgListedBattle D_WSTAG825_800A6890 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A689C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68A8 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68B4 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68C0 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68CC = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68D8 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A68E4 = { 64, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A68F0 = {
    4,
    { &D_WSTAG825_800A6890, &D_WSTAG825_800A689C, &D_WSTAG825_800A68A8, &D_WSTAG825_800A68B4, &D_WSTAG825_800A68C0,
        &D_WSTAG825_800A68CC, &D_WSTAG825_800A68D8, &D_WSTAG825_800A68E4 },
};
FieldstgListedBattle D_WSTAG825_800A6914 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6920 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A692C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6938 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6944 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6950 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A695C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6968 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6974 = {
    0,
    { &D_WSTAG825_800A6914, &D_WSTAG825_800A6920, &D_WSTAG825_800A692C, &D_WSTAG825_800A6938, &D_WSTAG825_800A6944,
        &D_WSTAG825_800A6950, &D_WSTAG825_800A695C, &D_WSTAG825_800A6968 },
};
FieldstgListedBattle D_WSTAG825_800A6998 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A69EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A69F8 = {
    0,
    { &D_WSTAG825_800A6998, &D_WSTAG825_800A69A4, &D_WSTAG825_800A69B0, &D_WSTAG825_800A69BC, &D_WSTAG825_800A69C8,
        &D_WSTAG825_800A69D4, &D_WSTAG825_800A69E0, &D_WSTAG825_800A69EC },
};
FieldstgListedBattle D_WSTAG825_800A6A1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6A70 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6A7C = {
    0,
    { &D_WSTAG825_800A6A1C, &D_WSTAG825_800A6A28, &D_WSTAG825_800A6A34, &D_WSTAG825_800A6A40, &D_WSTAG825_800A6A4C,
        &D_WSTAG825_800A6A58, &D_WSTAG825_800A6A64, &D_WSTAG825_800A6A70 },
};
FieldstgListedBattle D_WSTAG825_800A6AA0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AAC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AB8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AC4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AD0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6ADC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AE8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6AF4 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A6B00 = {
    2,
    { &D_WSTAG825_800A6AA0, &D_WSTAG825_800A6AAC, &D_WSTAG825_800A6AB8, &D_WSTAG825_800A6AC4, &D_WSTAG825_800A6AD0,
        &D_WSTAG825_800A6ADC, &D_WSTAG825_800A6AE8, &D_WSTAG825_800A6AF4 },
};
FieldstgListedBattle D_WSTAG825_800A6B24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6B78 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6B84 = {
    0,
    { &D_WSTAG825_800A6B24, &D_WSTAG825_800A6B30, &D_WSTAG825_800A6B3C, &D_WSTAG825_800A6B48, &D_WSTAG825_800A6B54,
        &D_WSTAG825_800A6B60, &D_WSTAG825_800A6B6C, &D_WSTAG825_800A6B78 },
};
FieldstgListedBattle D_WSTAG825_800A6BA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6BFC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6C08 = {
    0,
    { &D_WSTAG825_800A6BA8, &D_WSTAG825_800A6BB4, &D_WSTAG825_800A6BC0, &D_WSTAG825_800A6BCC, &D_WSTAG825_800A6BD8,
        &D_WSTAG825_800A6BE4, &D_WSTAG825_800A6BF0, &D_WSTAG825_800A6BFC },
};
FieldstgListedBattle D_WSTAG825_800A6C2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6C80 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6C8C = {
    0,
    { &D_WSTAG825_800A6C2C, &D_WSTAG825_800A6C38, &D_WSTAG825_800A6C44, &D_WSTAG825_800A6C50, &D_WSTAG825_800A6C5C,
        &D_WSTAG825_800A6C68, &D_WSTAG825_800A6C74, &D_WSTAG825_800A6C80 },
};
FieldstgListedBattle D_WSTAG825_800A6CB0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CBC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CC8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CD4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CE0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CEC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6CF8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6D04 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A6D10 = {
    4,
    { &D_WSTAG825_800A6CB0, &D_WSTAG825_800A6CBC, &D_WSTAG825_800A6CC8, &D_WSTAG825_800A6CD4, &D_WSTAG825_800A6CE0,
        &D_WSTAG825_800A6CEC, &D_WSTAG825_800A6CF8, &D_WSTAG825_800A6D04 },
};
FieldstgListedBattle D_WSTAG825_800A6D34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6D88 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6D94 = {
    0,
    { &D_WSTAG825_800A6D34, &D_WSTAG825_800A6D40, &D_WSTAG825_800A6D4C, &D_WSTAG825_800A6D58, &D_WSTAG825_800A6D64,
        &D_WSTAG825_800A6D70, &D_WSTAG825_800A6D7C, &D_WSTAG825_800A6D88 },
};
FieldstgListedBattle D_WSTAG825_800A6DB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6DC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6DD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6DDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6DE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6DF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E0C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6E18 = {
    0,
    { &D_WSTAG825_800A6DB8, &D_WSTAG825_800A6DC4, &D_WSTAG825_800A6DD0, &D_WSTAG825_800A6DDC, &D_WSTAG825_800A6DE8,
        &D_WSTAG825_800A6DF4, &D_WSTAG825_800A6E00, &D_WSTAG825_800A6E0C },
};
FieldstgListedBattle D_WSTAG825_800A6E3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6E90 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6E9C = {
    0,
    { &D_WSTAG825_800A6E3C, &D_WSTAG825_800A6E48, &D_WSTAG825_800A6E54, &D_WSTAG825_800A6E60, &D_WSTAG825_800A6E6C,
        &D_WSTAG825_800A6E78, &D_WSTAG825_800A6E84, &D_WSTAG825_800A6E90 },
};
FieldstgListedBattle D_WSTAG825_800A6EC0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6ECC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6ED8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6EE4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6EF0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6EFC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6F08 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A6F14 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A6F20 = {
    2,
    { &D_WSTAG825_800A6EC0, &D_WSTAG825_800A6ECC, &D_WSTAG825_800A6ED8, &D_WSTAG825_800A6EE4, &D_WSTAG825_800A6EF0,
        &D_WSTAG825_800A6EFC, &D_WSTAG825_800A6F08, &D_WSTAG825_800A6F14 },
};
FieldstgListedBattle D_WSTAG825_800A6F44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6F98 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A6FA4 = {
    0,
    { &D_WSTAG825_800A6F44, &D_WSTAG825_800A6F50, &D_WSTAG825_800A6F5C, &D_WSTAG825_800A6F68, &D_WSTAG825_800A6F74,
        &D_WSTAG825_800A6F80, &D_WSTAG825_800A6F8C, &D_WSTAG825_800A6F98 },
};
FieldstgListedBattle D_WSTAG825_800A6FC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6FD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6FE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6FEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A6FF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7004 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7010 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A701C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A7028 = {
    0,
    { &D_WSTAG825_800A6FC8, &D_WSTAG825_800A6FD4, &D_WSTAG825_800A6FE0, &D_WSTAG825_800A6FEC, &D_WSTAG825_800A6FF8,
        &D_WSTAG825_800A7004, &D_WSTAG825_800A7010, &D_WSTAG825_800A701C },
};
FieldstgListedBattle D_WSTAG825_800A704C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7058 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7064 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A707C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A70A0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A70AC = {
    0,
    { &D_WSTAG825_800A704C, &D_WSTAG825_800A7058, &D_WSTAG825_800A7064, &D_WSTAG825_800A7070, &D_WSTAG825_800A707C,
        &D_WSTAG825_800A7088, &D_WSTAG825_800A7094, &D_WSTAG825_800A70A0 },
};
FieldstgListedBattle D_WSTAG825_800A70D0 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A70DC = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A70E8 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A70F4 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A7100 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A710C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A7118 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG825_800A7124 = { 64, 11, 0x60080000 };
FieldstgBattleList D_WSTAG825_800A7130 = {
    4,
    { &D_WSTAG825_800A70D0, &D_WSTAG825_800A70DC, &D_WSTAG825_800A70E8, &D_WSTAG825_800A70F4, &D_WSTAG825_800A7100,
        &D_WSTAG825_800A710C, &D_WSTAG825_800A7118, &D_WSTAG825_800A7124 },
};
FieldstgListedBattle D_WSTAG825_800A7154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A716C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A719C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A71A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A71B4 = {
    0,
    { &D_WSTAG825_800A7154, &D_WSTAG825_800A7160, &D_WSTAG825_800A716C, &D_WSTAG825_800A7178, &D_WSTAG825_800A7184,
        &D_WSTAG825_800A7190, &D_WSTAG825_800A719C, &D_WSTAG825_800A71A8 },
};
FieldstgListedBattle D_WSTAG825_800A71D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A71E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A71F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A71FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7208 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7214 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7220 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A722C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A7238 = {
    0,
    { &D_WSTAG825_800A71D8, &D_WSTAG825_800A71E4, &D_WSTAG825_800A71F0, &D_WSTAG825_800A71FC, &D_WSTAG825_800A7208,
        &D_WSTAG825_800A7214, &D_WSTAG825_800A7220, &D_WSTAG825_800A722C },
};
FieldstgListedBattle D_WSTAG825_800A725C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7268 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A728C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A7298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A72A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG825_800A72B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG825_800A72BC = {
    0,
    { &D_WSTAG825_800A725C, &D_WSTAG825_800A7268, &D_WSTAG825_800A7274, &D_WSTAG825_800A7280, &D_WSTAG825_800A728C,
        &D_WSTAG825_800A7298, &D_WSTAG825_800A72A4, &D_WSTAG825_800A72B0 },
};
FieldstgBattleLists wstag825_battle_lists[8] = {
    { 173, 1, 0, { &D_WSTAG825_800A62C0, &D_WSTAG825_800A6344, &D_WSTAG825_800A63C8 }, &D_WSTAG825_800A644C },
    { 178, 2, 0, { &D_WSTAG825_800A64D0, &D_WSTAG825_800A6554, &D_WSTAG825_800A65D8 }, &D_WSTAG825_800A665C },
    { 194, 7, 0, { &D_WSTAG825_800A66E0, &D_WSTAG825_800A6764, &D_WSTAG825_800A67E8 }, &D_WSTAG825_800A686C },
    { 196, 8, 0, { &D_WSTAG825_800A68F0, &D_WSTAG825_800A6974, &D_WSTAG825_800A69F8 }, &D_WSTAG825_800A6A7C },
    { 201, 11, 0, { &D_WSTAG825_800A6B00, &D_WSTAG825_800A6B84, &D_WSTAG825_800A6C08 }, &D_WSTAG825_800A6C8C },
    { 206, 12, 0, { &D_WSTAG825_800A6D10, &D_WSTAG825_800A6D94, &D_WSTAG825_800A6E18 }, &D_WSTAG825_800A6E9C },
    { 222, 17, 0, { &D_WSTAG825_800A6F20, &D_WSTAG825_800A6FA4, &D_WSTAG825_800A7028 }, &D_WSTAG825_800A70AC },
    { 224, 18, 0, { &D_WSTAG825_800A7130, &D_WSTAG825_800A71B4, &D_WSTAG825_800A7238 }, &D_WSTAG825_800A72BC },
};
FieldstgVramPlace wstag825_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 432, 349, 448, 93, 352, 511 },
};
FieldstgPlacedActor D_WSTAG825_800A7430 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag825_actors[2] = { &D_WSTAG825_800A7430, NULL };
FieldstgSprite wstag825_sprites[19] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 346, 191, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 653, 112, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 179, 218, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 563, 120, 0, 0 },
    { 1, 0, 0xA0, 2, 0xB, 0, 0, 0, 0, 0, 448, 198, 0, 0 }, { 1, 0, 0x95, 2, 0xC, 0, 0, 0, 0, 0, 480, 210, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 266, 189, 0, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 540, -12, 244, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 240, 273, 351, 0 }, { 1, 0, 0x68, 4, 1, 0, 0, 0, 0, 0, 208, 259, 345, 0 },
    { 1, 0, 0x58, 4, 2, 0, 0, 0, 0, 0, 196, 254, 332, 0 }, { 1, 0, 0x58, 4, 3, 0, 0, 0, 0, 0, 416, 180, 305, 0 },
    { 1, 0, 0x93, 4, 4, 0, 0, 0, 0, 0, 384, 157, 299, 0 }, { 1, 0, 0x79, 4, 5, 0, 0, 0, 0, 0, 352, 151, 270, 0 },
    { 1, 0, 0x94, 4, 6, 0, 0, 0, 0, 0, 320, 131, 277, 0 }, { 1, 0, 0x5C, 4, 8, 0, 0, 0, 0, 0, 528, 180, 268, 0 },
    { 1, 0, 0x63, 4, 9, 0, 0, 0, 0, 0, 496, 164, 260, 0 }, { 1, 0, 0x5D, 4, 0xA, 0, 0, 0, 0, 0, 485, 159, 250, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag825_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E0, 0x240, 0xD8, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E0, 0xB0, 0x178, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag825_funcs = { wstag825_setup };
