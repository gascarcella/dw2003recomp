#include "wstag.h"

/* WSTAG963: stage 0x2E3 (fieldstg_stages_2d). */

extern WstagExits *wstag963_exits[];
extern WstagFuncs wstag963_funcs;
extern CVECTOR wstag963_color;
extern FieldstgVramPlace wstag963_vram_places[];
extern FieldstgPlacedActor *wstag963_actors[];
extern FieldstgSprite wstag963_sprites[];
extern FieldstgMapEvent wstag963_map_events[];
extern FieldstgBattleLists wstag963_battle_lists[];

s32 wstag963_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag963_set_exits(dst, list + 1, arg2, arg3);
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

void wstag963_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag963_set_exits(fieldstg_stage.map_events, wstag963_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag963_start(void *arg0) {
    WstagObject *obj = object_new(wstag963_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag963_funcs.setup();
    return obj;
}

void wstag963_setup(void) {
    fieldstg_stage.background_file = 0x74A;
    fieldstg_stage.sprites = wstag963_sprites;
    fieldstg_stage.map_events = wstag963_map_events;
    fieldstg_stage.sprite_file = 0x09350004;
    fieldstg_stage.mask_file = 0x934;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15300, 0x13900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag963_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag963_actors;
    fieldstg_stage.color = wstag963_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag963_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09350006);
    fieldstg_attr.set_file(7, 0x09350007);
    fieldstg_attr.set_file(4, 0x09350005);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag963/nonmatchings/wstag963", wstag963_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag963_setup(void);

WstagExit D_WSTAG963_800A6100 = { 0x2E1, 1, 1, 0x390, 0x108, 1, NULL };
WstagExit D_WSTAG963_800A6110 = { 0x28A, 0, 0, 0x80, 0x1E0, 0, &D_WSTAG963_800A6100 };
WstagExits D_WSTAG963_800A6120 = { 1, 1, &D_WSTAG963_800A6110 };
WstagExit D_WSTAG963_800A6128 = { 0x2E6, 4, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG963_800A6138 = { 0x28E, 0, 0, 0xE0, 0x208, 0, &D_WSTAG963_800A6128 };
WstagExits D_WSTAG963_800A6148 = { 4, 1, &D_WSTAG963_800A6138 };
WstagExits *wstag963_exits[3] = { &D_WSTAG963_800A6120, &D_WSTAG963_800A6148, NULL };
FieldstgVramPlace wstag963_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 417, 0, 161, 352, 511 },
};
FieldstgPlacedActor D_WSTAG963_800A61CC = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag963_actors[2] = { &D_WSTAG963_800A61CC, NULL };
FieldstgSprite wstag963_sprites[23] = {
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
FieldstgMapEvent wstag963_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E3, 0x380, 0x70, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E3, 0xA0, 0x180, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag963_funcs = { wstag963_setup };
FieldstgListedBattle D_WSTAG963_800A63D4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A63E0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A63EC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A63F8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6404 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6410 = { 273, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A641C = { 273, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6428 = { 273, 11, 0x60080000 };
FieldstgBattleList D_WSTAG963_800A6434 = {
    5,
    { &D_WSTAG963_800A63D4, &D_WSTAG963_800A63E0, &D_WSTAG963_800A63EC, &D_WSTAG963_800A63F8, &D_WSTAG963_800A6404,
        &D_WSTAG963_800A6410, &D_WSTAG963_800A641C, &D_WSTAG963_800A6428 },
};
FieldstgListedBattle D_WSTAG963_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6494 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A64A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A64AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A64B8 = {
    0,
    { &D_WSTAG963_800A6458, &D_WSTAG963_800A6464, &D_WSTAG963_800A6470, &D_WSTAG963_800A647C, &D_WSTAG963_800A6488,
        &D_WSTAG963_800A6494, &D_WSTAG963_800A64A0, &D_WSTAG963_800A64AC },
};
FieldstgListedBattle D_WSTAG963_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6530 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A653C = {
    0,
    { &D_WSTAG963_800A64DC, &D_WSTAG963_800A64E8, &D_WSTAG963_800A64F4, &D_WSTAG963_800A6500, &D_WSTAG963_800A650C,
        &D_WSTAG963_800A6518, &D_WSTAG963_800A6524, &D_WSTAG963_800A6530 },
};
FieldstgListedBattle D_WSTAG963_800A6560 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A656C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A659C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A65A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A65B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A65C0 = {
    0,
    { &D_WSTAG963_800A6560, &D_WSTAG963_800A656C, &D_WSTAG963_800A6578, &D_WSTAG963_800A6584, &D_WSTAG963_800A6590,
        &D_WSTAG963_800A659C, &D_WSTAG963_800A65A8, &D_WSTAG963_800A65B4 },
};
FieldstgListedBattle D_WSTAG963_800A65E4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A65F0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A65FC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6608 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6614 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6620 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A662C = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG963_800A6638 = { 159, 11, 0x60080000 };
FieldstgBattleList D_WSTAG963_800A6644 = {
    5,
    { &D_WSTAG963_800A65E4, &D_WSTAG963_800A65F0, &D_WSTAG963_800A65FC, &D_WSTAG963_800A6608, &D_WSTAG963_800A6614,
        &D_WSTAG963_800A6620, &D_WSTAG963_800A662C, &D_WSTAG963_800A6638 },
};
FieldstgListedBattle D_WSTAG963_800A6668 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6674 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6680 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A668C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A66A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A66B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A66BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A66C8 = {
    0,
    { &D_WSTAG963_800A6668, &D_WSTAG963_800A6674, &D_WSTAG963_800A6680, &D_WSTAG963_800A668C, &D_WSTAG963_800A6698,
        &D_WSTAG963_800A66A4, &D_WSTAG963_800A66B0, &D_WSTAG963_800A66BC },
};
FieldstgListedBattle D_WSTAG963_800A66EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A66F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A671C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6728 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6734 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6740 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A674C = {
    0,
    { &D_WSTAG963_800A66EC, &D_WSTAG963_800A66F8, &D_WSTAG963_800A6704, &D_WSTAG963_800A6710, &D_WSTAG963_800A671C,
        &D_WSTAG963_800A6728, &D_WSTAG963_800A6734, &D_WSTAG963_800A6740 },
};
FieldstgListedBattle D_WSTAG963_800A6770 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A677C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6788 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG963_800A67C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG963_800A67D0 = {
    0,
    { &D_WSTAG963_800A6770, &D_WSTAG963_800A677C, &D_WSTAG963_800A6788, &D_WSTAG963_800A6794, &D_WSTAG963_800A67A0,
        &D_WSTAG963_800A67AC, &D_WSTAG963_800A67B8, &D_WSTAG963_800A67C4 },
};
FieldstgBattleLists wstag963_battle_lists[2] = {
    { 396, 1, 0, { &D_WSTAG963_800A6434, &D_WSTAG963_800A64B8, &D_WSTAG963_800A653C }, &D_WSTAG963_800A65C0 },
    { 406, 4, 0, { &D_WSTAG963_800A6644, &D_WSTAG963_800A66C8, &D_WSTAG963_800A674C }, &D_WSTAG963_800A67D0 },
};
