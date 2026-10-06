#include "wstag.h"

/* WSTAG962: stage 0x2E2 (fieldstg_stages_2d). */

extern WstagExits *wstag962_exits[];
extern WstagFuncs wstag962_funcs;
const CVECTOR wstag962_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgVramPlace wstag962_vram_places[];
extern FieldstgPlacedActor *wstag962_actors[];
extern FieldstgSprite wstag962_sprites[];
extern FieldstgMapEvent wstag962_map_events[];
extern FieldstgBattleLists wstag962_battle_lists[];

s32 wstag962_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag962_set_exits(dst, list + 1, arg2, arg3);
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

void wstag962_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag962_set_exits(fieldstg_stage.map_events, wstag962_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag962_start(void *arg0) {
    WstagObject *obj = object_new(wstag962_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag962_funcs.setup();
    return obj;
}

void wstag962_setup(void) {
    fieldstg_stage.background_file = 0x6E4;
    fieldstg_stage.sprites = wstag962_sprites;
    fieldstg_stage.map_events = wstag962_map_events;
    fieldstg_stage.sprite_file = 0x09330004;
    fieldstg_stage.mask_file = 0x932;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xC700, 0x16D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag962_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag962_actors;
    fieldstg_stage.color = wstag962_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag962_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09330006);
    fieldstg_attr.set_file(7, 0x09330007);
    fieldstg_attr.set_file(4, 0x09330005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag962_setup(void);

WstagExit D_WSTAG962_800A60FC = { 0x2E4, 2, 4, 0xC0, 0x180, 5, NULL };
WstagExit D_WSTAG962_800A610C = { 0x28C, 0, 0, 0x328, 0x424, 0, &D_WSTAG962_800A60FC };
WstagExits D_WSTAG962_800A611C = { 2, 1, &D_WSTAG962_800A610C };
WstagExits *wstag962_exits[2] = { &D_WSTAG962_800A611C, NULL };
FieldstgVramPlace wstag962_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 338, 408, 72, 152, 352, 511 },
};
FieldstgPlacedActor D_WSTAG962_800A619C = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag962_actors[2] = { &D_WSTAG962_800A619C, NULL };
FieldstgSprite wstag962_sprites[17] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 254, 275, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 621, 244, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 852, 128, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 384, 171, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 689, 166, 0, 0 },
    { 1, 0, 0xFF, 2, 0x33, 2, 0, 7, 0x18, 0, 140, -36, 0, 0 }, { 1, 0, 0x98, 2, 5, 0, 0, 0, 0, 0, 432, 254, 0, 0 },
    { 1, 0, 0x8A, 2, 6, 0, 0, 0, 0, 0, 408, 268, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 475, 113, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 169, 206, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 750, 142, 0, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 140, 92, 336, 0 },
    { 1, 0, 0x92, 4, 1, 0, 0, 0, 0, 0, 560, 179, 319, 0 }, { 1, 0, 0x79, 4, 2, 0, 0, 0, 0, 0, 528, 198, 335, 0 },
    { 1, 0, 0x88, 4, 3, 0, 0, 0, 0, 0, 496, 216, 351, 0 }, { 1, 0, 0x86, 4, 4, 0, 0, 0, 0, 0, 464, 232, 368, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag962_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E2, 0xB0, 0x148, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E2, 0x330, 0xF8, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag962_funcs = { wstag962_setup };
FieldstgListedBattle D_WSTAG962_800A6338 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A6344 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A6350 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A635C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A6368 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A6374 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A6380 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG962_800A638C = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG962_800A6398 = {
    3,
    { &D_WSTAG962_800A6338, &D_WSTAG962_800A6344, &D_WSTAG962_800A6350, &D_WSTAG962_800A635C, &D_WSTAG962_800A6368,
        &D_WSTAG962_800A6374, &D_WSTAG962_800A6380, &D_WSTAG962_800A638C },
};
FieldstgListedBattle D_WSTAG962_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A63C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A63D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6404 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6410 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG962_800A641C = {
    0,
    { &D_WSTAG962_800A63BC, &D_WSTAG962_800A63C8, &D_WSTAG962_800A63D4, &D_WSTAG962_800A63E0, &D_WSTAG962_800A63EC,
        &D_WSTAG962_800A63F8, &D_WSTAG962_800A6404, &D_WSTAG962_800A6410 },
};
FieldstgListedBattle D_WSTAG962_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6494 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG962_800A64A0 = {
    0,
    { &D_WSTAG962_800A6440, &D_WSTAG962_800A644C, &D_WSTAG962_800A6458, &D_WSTAG962_800A6464, &D_WSTAG962_800A6470,
        &D_WSTAG962_800A647C, &D_WSTAG962_800A6488, &D_WSTAG962_800A6494 },
};
FieldstgListedBattle D_WSTAG962_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG962_800A6518 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG962_800A6524 = {
    0,
    { &D_WSTAG962_800A64C4, &D_WSTAG962_800A64D0, &D_WSTAG962_800A64DC, &D_WSTAG962_800A64E8, &D_WSTAG962_800A64F4,
        &D_WSTAG962_800A6500, &D_WSTAG962_800A650C, &D_WSTAG962_800A6518 },
};
FieldstgBattleLists wstag962_battle_lists[1] = {
    { 398, 2, 0, { &D_WSTAG962_800A6398, &D_WSTAG962_800A641C, &D_WSTAG962_800A64A0 }, &D_WSTAG962_800A6524 },
};
