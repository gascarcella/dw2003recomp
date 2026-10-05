#include "wstag.h"

/* WSTAG965: stage 0x2E5 (fieldstg_stages_2d). */

extern WstagExits *wstag965_exits[];
extern WstagFuncs wstag965_funcs;
extern CVECTOR wstag965_color;
extern FieldstgVramPlace wstag965_vram_places[];
extern FieldstgPlacedActor *wstag965_actors[];
extern FieldstgSprite wstag965_sprites[];
extern FieldstgMapEvent wstag965_map_events[];
extern FieldstgBattleLists wstag965_battle_lists[];

s32 wstag965_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag965_set_exits(dst, list + 1, arg2, arg3);
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

void wstag965_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag965_set_exits(fieldstg_stage.map_events, wstag965_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag965_start(void *arg0) {
    WstagObject *obj = object_new(wstag965_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag965_funcs.setup();
    return obj;
}

void wstag965_setup(void) {
    fieldstg_stage.background_file = 0x6E0;
    fieldstg_stage.sprites = wstag965_sprites;
    fieldstg_stage.map_events = wstag965_map_events;
    fieldstg_stage.sprite_file = 0x09390004;
    fieldstg_stage.mask_file = 0x938;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x24000, 0x15200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag965_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag965_actors;
    fieldstg_stage.color = wstag965_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag965_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09390006);
    fieldstg_attr.set_file(7, 0x09390007);
    fieldstg_attr.set_file(4, 0x09390005);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag965/nonmatchings/wstag965", wstag965_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag965_setup(void);

WstagExit D_WSTAG965_800A6100 = { 0x2E6, 3, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG965_800A6110 = { 0x2E4, 3, 1, 0xC0, 0x180, 5, &D_WSTAG965_800A6100 };
WstagExit D_WSTAG965_800A6120 = { 0x296, 0, 0, 0x240, 0x200, 0, &D_WSTAG965_800A6110 };
WstagExits D_WSTAG965_800A6130 = { 3, 1, &D_WSTAG965_800A6120 };
WstagExits *wstag965_exits[2] = { &D_WSTAG965_800A6130, NULL };
FieldstgVramPlace wstag965_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 463, 192, 207, 352, 511 },
};
FieldstgPlacedActor D_WSTAG965_800A61B0 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag965_actors[2] = { &D_WSTAG965_800A61B0, NULL };
FieldstgSprite wstag965_sprites[16] = {
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
FieldstgMapEvent wstag965_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0x240, 0x120, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0x3B0, 0xD8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E5, 0xE0, 0x120, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag965_funcs = { wstag965_setup };
FieldstgListedBattle D_WSTAG965_800A6350 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A635C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A6368 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A6374 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A6380 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A638C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A6398 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG965_800A63A4 = { 64, 11, 0x60080000 };
FieldstgBattleList D_WSTAG965_800A63B0 = {
    3,
    { &D_WSTAG965_800A6350, &D_WSTAG965_800A635C, &D_WSTAG965_800A6368, &D_WSTAG965_800A6374, &D_WSTAG965_800A6380,
        &D_WSTAG965_800A638C, &D_WSTAG965_800A6398, &D_WSTAG965_800A63A4 },
};
FieldstgListedBattle D_WSTAG965_800A63D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6410 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6428 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG965_800A6434 = {
    0,
    { &D_WSTAG965_800A63D4, &D_WSTAG965_800A63E0, &D_WSTAG965_800A63EC, &D_WSTAG965_800A63F8, &D_WSTAG965_800A6404,
        &D_WSTAG965_800A6410, &D_WSTAG965_800A641C, &D_WSTAG965_800A6428 },
};
FieldstgListedBattle D_WSTAG965_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6494 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A64A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A64AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG965_800A64B8 = {
    0,
    { &D_WSTAG965_800A6458, &D_WSTAG965_800A6464, &D_WSTAG965_800A6470, &D_WSTAG965_800A647C, &D_WSTAG965_800A6488,
        &D_WSTAG965_800A6494, &D_WSTAG965_800A64A0, &D_WSTAG965_800A64AC },
};
FieldstgListedBattle D_WSTAG965_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6518 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6524 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG965_800A6530 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG965_800A653C = {
    0,
    { &D_WSTAG965_800A64DC, &D_WSTAG965_800A64E8, &D_WSTAG965_800A64F4, &D_WSTAG965_800A6500, &D_WSTAG965_800A650C,
        &D_WSTAG965_800A6518, &D_WSTAG965_800A6524, &D_WSTAG965_800A6530 },
};
FieldstgBattleLists wstag965_battle_lists[1] = {
    { 403, 3, 0, { &D_WSTAG965_800A63B0, &D_WSTAG965_800A6434, &D_WSTAG965_800A64B8 }, &D_WSTAG965_800A653C },
};
