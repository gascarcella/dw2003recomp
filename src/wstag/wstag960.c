#include "wstag.h"

/* WSTAG960: stage 0x2E0 (fieldstg_stages_2d). */

extern WstagExits *wstag960_exits[];
extern WstagFuncs wstag960_funcs;
const CVECTOR wstag960_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgVramPlace wstag960_vram_places[];
extern FieldstgPlacedActor *wstag960_actors[];
extern FieldstgSprite wstag960_sprites[];
extern FieldstgMapEvent wstag960_map_events[];
extern FieldstgBattleLists wstag960_battle_lists[];

s32 wstag960_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag960_set_exits(dst, list + 1, arg2, arg3);
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

void wstag960_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag960_set_exits(fieldstg_stage.map_events, wstag960_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag960_start(void *arg0) {
    WstagObject *obj = object_new(wstag960_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag960_funcs.setup();
    return obj;
}

void wstag960_setup(void) {
    fieldstg_stage.background_file = 0x69F;
    fieldstg_stage.sprite_file = 0x092F0000;
    fieldstg_stage.sprites = wstag960_sprites;
    fieldstg_stage.map_events = wstag960_map_events;
    fieldstg_stage.mask_file = 0x92E;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x9D00, 0x17F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag960_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag960_actors;
    fieldstg_stage.color = wstag960_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag960_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x092F0002);
    fieldstg_attr.set_file(7, 0x092F0003);
    fieldstg_attr.set_file(4, 0x092F0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag960_setup(void);

WstagExit D_WSTAG960_800A60F8 = { 0x2E6, 2, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG960_800A6108 = { 0x299, 0, 0, 0x32C, 0x232, 0, &D_WSTAG960_800A60F8 };
WstagExits D_WSTAG960_800A6118 = { 2, 1, &D_WSTAG960_800A6108 };
WstagExit D_WSTAG960_800A6120 = { 0x2E4, 3, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG960_800A6130 = { 0x297, 0, 0, 0x210, 0x370, 0, &D_WSTAG960_800A6120 };
WstagExits D_WSTAG960_800A6140 = { 3, 1, &D_WSTAG960_800A6130 };
WstagExit D_WSTAG960_800A6148 = { 0x2E4, 5, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG960_800A6158 = { 0x28F, 0, 0, 0x70, 0xB8, 0, &D_WSTAG960_800A6148 };
WstagExits D_WSTAG960_800A6168 = { 5, 1, &D_WSTAG960_800A6158 };
WstagExits *wstag960_exits[4] = { &D_WSTAG960_800A6118, &D_WSTAG960_800A6140, &D_WSTAG960_800A6168, NULL };
FieldstgVramPlace wstag960_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 432, 349, 448, 93, 352, 511 },
};
FieldstgPlacedActor D_WSTAG960_800A61F0 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag960_actors[2] = { &D_WSTAG960_800A61F0, NULL };
FieldstgSprite wstag960_sprites[19] = {
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
FieldstgMapEvent wstag960_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E0, 0x240, 0xD8, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E0, 0xB0, 0x178, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag960_funcs = { wstag960_setup };
FieldstgListedBattle D_WSTAG960_800A63B0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63BC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63C8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63D4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63E0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63EC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A63F8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6404 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG960_800A6410 = {
    3,
    { &D_WSTAG960_800A63B0, &D_WSTAG960_800A63BC, &D_WSTAG960_800A63C8, &D_WSTAG960_800A63D4, &D_WSTAG960_800A63E0,
        &D_WSTAG960_800A63EC, &D_WSTAG960_800A63F8, &D_WSTAG960_800A6404 },
};
FieldstgListedBattle D_WSTAG960_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6488 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A6494 = {
    0,
    { &D_WSTAG960_800A6434, &D_WSTAG960_800A6440, &D_WSTAG960_800A644C, &D_WSTAG960_800A6458, &D_WSTAG960_800A6464,
        &D_WSTAG960_800A6470, &D_WSTAG960_800A647C, &D_WSTAG960_800A6488 },
};
FieldstgListedBattle D_WSTAG960_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A650C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A6518 = {
    0,
    { &D_WSTAG960_800A64B8, &D_WSTAG960_800A64C4, &D_WSTAG960_800A64D0, &D_WSTAG960_800A64DC, &D_WSTAG960_800A64E8,
        &D_WSTAG960_800A64F4, &D_WSTAG960_800A6500, &D_WSTAG960_800A650C },
};
FieldstgListedBattle D_WSTAG960_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6560 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A656C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6590 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A659C = {
    0,
    { &D_WSTAG960_800A653C, &D_WSTAG960_800A6548, &D_WSTAG960_800A6554, &D_WSTAG960_800A6560, &D_WSTAG960_800A656C,
        &D_WSTAG960_800A6578, &D_WSTAG960_800A6584, &D_WSTAG960_800A6590 },
};
FieldstgListedBattle D_WSTAG960_800A65C0 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A65CC = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A65D8 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A65E4 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A65F0 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A65FC = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6608 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6614 = { 64, 11, 0x60080000 };
FieldstgBattleList D_WSTAG960_800A6620 = {
    3,
    { &D_WSTAG960_800A65C0, &D_WSTAG960_800A65CC, &D_WSTAG960_800A65D8, &D_WSTAG960_800A65E4, &D_WSTAG960_800A65F0,
        &D_WSTAG960_800A65FC, &D_WSTAG960_800A6608, &D_WSTAG960_800A6614 },
};
FieldstgListedBattle D_WSTAG960_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A665C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6668 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6674 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6680 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A668C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6698 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A66A4 = {
    0,
    { &D_WSTAG960_800A6644, &D_WSTAG960_800A6650, &D_WSTAG960_800A665C, &D_WSTAG960_800A6668, &D_WSTAG960_800A6674,
        &D_WSTAG960_800A6680, &D_WSTAG960_800A668C, &D_WSTAG960_800A6698 },
};
FieldstgListedBattle D_WSTAG960_800A66C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A66D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A66E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A66EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A66F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6704 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6710 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A671C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A6728 = {
    0,
    { &D_WSTAG960_800A66C8, &D_WSTAG960_800A66D4, &D_WSTAG960_800A66E0, &D_WSTAG960_800A66EC, &D_WSTAG960_800A66F8,
        &D_WSTAG960_800A6704, &D_WSTAG960_800A6710, &D_WSTAG960_800A671C },
};
FieldstgListedBattle D_WSTAG960_800A674C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6758 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6764 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6770 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A677C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6788 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A67A0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A67AC = {
    0,
    { &D_WSTAG960_800A674C, &D_WSTAG960_800A6758, &D_WSTAG960_800A6764, &D_WSTAG960_800A6770, &D_WSTAG960_800A677C,
        &D_WSTAG960_800A6788, &D_WSTAG960_800A6794, &D_WSTAG960_800A67A0 },
};
FieldstgListedBattle D_WSTAG960_800A67D0 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A67DC = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A67E8 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A67F4 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6800 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A680C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6818 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG960_800A6824 = { 63, 11, 0x60080000 };
FieldstgBattleList D_WSTAG960_800A6830 = {
    4,
    { &D_WSTAG960_800A67D0, &D_WSTAG960_800A67DC, &D_WSTAG960_800A67E8, &D_WSTAG960_800A67F4, &D_WSTAG960_800A6800,
        &D_WSTAG960_800A680C, &D_WSTAG960_800A6818, &D_WSTAG960_800A6824 },
};
FieldstgListedBattle D_WSTAG960_800A6854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6860 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A686C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6878 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6884 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6890 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A689C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A68A8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A68B4 = {
    0,
    { &D_WSTAG960_800A6854, &D_WSTAG960_800A6860, &D_WSTAG960_800A686C, &D_WSTAG960_800A6878, &D_WSTAG960_800A6884,
        &D_WSTAG960_800A6890, &D_WSTAG960_800A689C, &D_WSTAG960_800A68A8 },
};
FieldstgListedBattle D_WSTAG960_800A68D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A68E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A68F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A68FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6908 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6914 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6920 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A692C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A6938 = {
    0,
    { &D_WSTAG960_800A68D8, &D_WSTAG960_800A68E4, &D_WSTAG960_800A68F0, &D_WSTAG960_800A68FC, &D_WSTAG960_800A6908,
        &D_WSTAG960_800A6914, &D_WSTAG960_800A6920, &D_WSTAG960_800A692C },
};
FieldstgListedBattle D_WSTAG960_800A695C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6968 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6974 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6980 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A698C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A6998 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A69A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG960_800A69B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG960_800A69BC = {
    0,
    { &D_WSTAG960_800A695C, &D_WSTAG960_800A6968, &D_WSTAG960_800A6974, &D_WSTAG960_800A6980, &D_WSTAG960_800A698C,
        &D_WSTAG960_800A6998, &D_WSTAG960_800A69A4, &D_WSTAG960_800A69B0 },
};
FieldstgBattleLists wstag960_battle_lists[3] = {
    { 397, 2, 0, { &D_WSTAG960_800A6410, &D_WSTAG960_800A6494, &D_WSTAG960_800A6518 }, &D_WSTAG960_800A659C },
    { 401, 3, 0, { &D_WSTAG960_800A6620, &D_WSTAG960_800A66A4, &D_WSTAG960_800A6728 }, &D_WSTAG960_800A67AC },
    { 410, 5, 0, { &D_WSTAG960_800A6830, &D_WSTAG960_800A68B4, &D_WSTAG960_800A6938 }, &D_WSTAG960_800A69BC },
};
