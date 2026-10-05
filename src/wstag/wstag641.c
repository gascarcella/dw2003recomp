#include "wstag.h"

/* WSTAG641: stage 0x2C2 (fieldstg_stages). */

extern WstagExits *wstag641_exits[];
extern WstagFuncs wstag641_funcs;
extern FieldstgBattleLists wstag641_battle_lists;
extern FieldstgVramPlace wstag641_vram_places[];
extern FieldstgSprite wstag641_sprites[];
extern FieldstgMapEvent wstag641_map_events[];

s32 wstag641_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag641_set_exits(dst, list + 1, arg2, arg3);
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

void wstag641_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag641_set_exits(fieldstg_stage.map_events, wstag641_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag641_start(void *arg0) {
    WstagObject *obj = object_new(wstag641_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag641_funcs.setup();
    return obj;
}

void wstag641_setup(void) {
    fieldstg_stage.background_file = 0x632;
    fieldstg_stage.sprite_file = 0x06330000;
    fieldstg_stage.sprites = wstag641_sprites;
    fieldstg_stage.map_events = wstag641_map_events;
    fieldstg_stage.mask_file = 0x631;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF500, 0x2FA00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag641_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.battle_lists = &wstag641_battle_lists;
    fieldstg_attr.set_file(0, 0x06330001);
    fieldstg_attr.set_file(7, 0x06330002);
    fieldstg_attr.set_file(4, 0x06330003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag641_setup(void);

WstagExit D_WSTAG641_800A60B0 = { 0x2C1, 1, 1, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A60C0 = { 0x2C1, 1, 3, 0x340, 0xF0, 1, &D_WSTAG641_800A60B0 };
WstagExit D_WSTAG641_800A60D0 = { 0x2C1, 1, 2, 0x110, 0x108, 7, &D_WSTAG641_800A60C0 };
WstagExit D_WSTAG641_800A60E0 = { 0x2C0, 0, 0, 0x100, 0x278, 5, &D_WSTAG641_800A60D0 };
WstagExits D_WSTAG641_800A60F0 = { 1, 1, &D_WSTAG641_800A60E0 };
WstagExit D_WSTAG641_800A60F8 = { 0x2C1, 1, 2, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A6108 = { 0x2C1, 1, 4, 0x340, 0xF0, 1, &D_WSTAG641_800A60F8 };
WstagExit D_WSTAG641_800A6118 = { 0x2C1, 1, 1, 0x110, 0x108, 7, &D_WSTAG641_800A6108 };
WstagExit D_WSTAG641_800A6128 = { 0x2C0, 0, 0, 0x100, 0x278, 5, &D_WSTAG641_800A6118 };
WstagExits D_WSTAG641_800A6138 = { 1, 2, &D_WSTAG641_800A6128 };
WstagExit D_WSTAG641_800A6140 = { 0x2C1, 1, 4, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A6150 = { 0x2C1, 1, 5, 0x340, 0xF0, 1, &D_WSTAG641_800A6140 };
WstagExit D_WSTAG641_800A6160 = { 0x2C1, 1, 3, 0x110, 0x108, 7, &D_WSTAG641_800A6150 };
WstagExit D_WSTAG641_800A6170 = { 0x2C1, 1, 1, 0x128, 0x2F4, 5, &D_WSTAG641_800A6160 };
WstagExits D_WSTAG641_800A6180 = { 1, 3, &D_WSTAG641_800A6170 };
WstagExit D_WSTAG641_800A6188 = { 0x2C1, 1, 3, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A6198 = { 0x2C1, 1, 6, 0x340, 0xF0, 1, &D_WSTAG641_800A6188 };
WstagExit D_WSTAG641_800A61A8 = { 0x2C1, 1, 4, 0x110, 0x108, 7, &D_WSTAG641_800A6198 };
WstagExit D_WSTAG641_800A61B8 = { 0x2C1, 1, 2, 0x128, 0x2F4, 5, &D_WSTAG641_800A61A8 };
WstagExits D_WSTAG641_800A61C8 = { 1, 4, &D_WSTAG641_800A61B8 };
WstagExit D_WSTAG641_800A61D0 = { 0x2C1, 1, 5, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A61E0 = { 0x2C1, 1, 7, 0x340, 0xF0, 1, &D_WSTAG641_800A61D0 };
WstagExit D_WSTAG641_800A61F0 = { 0x2C1, 1, 6, 0x110, 0x108, 7, &D_WSTAG641_800A61E0 };
WstagExit D_WSTAG641_800A6200 = { 0x2C1, 1, 3, 0x128, 0x2F4, 5, &D_WSTAG641_800A61F0 };
WstagExits D_WSTAG641_800A6210 = { 1, 5, &D_WSTAG641_800A6200 };
WstagExit D_WSTAG641_800A6218 = { 0x2C1, 1, 6, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A6228 = { 0x2C1, 1, 8, 0x340, 0xF0, 1, &D_WSTAG641_800A6218 };
WstagExit D_WSTAG641_800A6238 = { 0x2C1, 1, 5, 0x110, 0x108, 7, &D_WSTAG641_800A6228 };
WstagExit D_WSTAG641_800A6248 = { 0x2C1, 1, 4, 0x128, 0x2F4, 5, &D_WSTAG641_800A6238 };
WstagExits D_WSTAG641_800A6258 = { 1, 6, &D_WSTAG641_800A6248 };
WstagExit D_WSTAG641_800A6260 = { 0x2C1, 1, 8, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A6270 = { 0x2C1, 1, 1, 0x340, 0xF0, 1, &D_WSTAG641_800A6260 };
WstagExit D_WSTAG641_800A6280 = { 0x2C1, 1, 7, 0x110, 0x108, 7, &D_WSTAG641_800A6270 };
WstagExit D_WSTAG641_800A6290 = { 0x2C1, 1, 5, 0x128, 0x2F4, 5, &D_WSTAG641_800A6280 };
WstagExits D_WSTAG641_800A62A0 = { 1, 7, &D_WSTAG641_800A6290 };
WstagExit D_WSTAG641_800A62A8 = { 0x2C1, 1, 7, 0x368, 0x2EC, 3, NULL };
WstagExit D_WSTAG641_800A62B8 = { 0x2C1, 1, 2, 0x340, 0xF0, 1, &D_WSTAG641_800A62A8 };
WstagExit D_WSTAG641_800A62C8 = { 0x2C1, 1, 8, 0x110, 0x108, 7, &D_WSTAG641_800A62B8 };
WstagExit D_WSTAG641_800A62D8 = { 0x2C1, 1, 6, 0x128, 0x2F4, 5, &D_WSTAG641_800A62C8 };
WstagExits D_WSTAG641_800A62E8 = { 1, 8, &D_WSTAG641_800A62D8 };
WstagExits D_WSTAG641_800A62F0 = { 0, 0, &D_WSTAG641_800A60E0 };
WstagExits *wstag641_exits[10] = {
    &D_WSTAG641_800A60F0, &D_WSTAG641_800A6138, &D_WSTAG641_800A6180, &D_WSTAG641_800A61C8, &D_WSTAG641_800A6210,
    &D_WSTAG641_800A6258, &D_WSTAG641_800A62A0, &D_WSTAG641_800A62E8, &D_WSTAG641_800A62F0, NULL,
};
FieldstgListedBattle D_WSTAG641_800A6320 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A632C = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A6338 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A6344 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A6350 = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A635C = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A6368 = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG641_800A6374 = { 162, 5, 0x60080000 };
FieldstgBattleList D_WSTAG641_800A6380 = {
    3,
    { &D_WSTAG641_800A6320, &D_WSTAG641_800A632C, &D_WSTAG641_800A6338, &D_WSTAG641_800A6344, &D_WSTAG641_800A6350,
        &D_WSTAG641_800A635C, &D_WSTAG641_800A6368, &D_WSTAG641_800A6374 },
};
FieldstgListedBattle D_WSTAG641_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A63F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG641_800A6404 = {
    0,
    { &D_WSTAG641_800A63A4, &D_WSTAG641_800A63B0, &D_WSTAG641_800A63BC, &D_WSTAG641_800A63C8, &D_WSTAG641_800A63D4,
        &D_WSTAG641_800A63E0, &D_WSTAG641_800A63EC, &D_WSTAG641_800A63F8 },
};
FieldstgListedBattle D_WSTAG641_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A647C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG641_800A6488 = {
    0,
    { &D_WSTAG641_800A6428, &D_WSTAG641_800A6434, &D_WSTAG641_800A6440, &D_WSTAG641_800A644C, &D_WSTAG641_800A6458,
        &D_WSTAG641_800A6464, &D_WSTAG641_800A6470, &D_WSTAG641_800A647C },
};
FieldstgListedBattle D_WSTAG641_800A64AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG641_800A6500 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG641_800A650C = {
    0,
    { &D_WSTAG641_800A64AC, &D_WSTAG641_800A64B8, &D_WSTAG641_800A64C4, &D_WSTAG641_800A64D0, &D_WSTAG641_800A64DC,
        &D_WSTAG641_800A64E8, &D_WSTAG641_800A64F4, &D_WSTAG641_800A6500 },
};
FieldstgBattleLists wstag641_battle_lists = {
    103, 0, 0, { &D_WSTAG641_800A6380, &D_WSTAG641_800A6404, &D_WSTAG641_800A6488 }, &D_WSTAG641_800A650C,
};
FieldstgVramPlace wstag641_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag641_sprites[17] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 324, 536, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 372, 224, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 516, 664, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 532, 288, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 580, 456, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 708, 616, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 724, 272, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 788, 464, 0, 0 },
    { 1, 0, 0x60, 4, 1, 0, 0, 0, 0, 0, 388, 238, 328, 0 }, { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 548, 302, 392, 0 },
    { 1, 0, 0x60, 4, 3, 0, 0, 0, 0, 0, 740, 286, 377, 0 }, { 1, 0, 0x60, 4, 4, 0, 0, 0, 0, 0, 596, 470, 561, 0 },
    { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 804, 478, 568, 0 }, { 1, 0, 0x60, 4, 6, 0, 0, 0, 0, 0, 340, 550, 640, 0 },
    { 1, 0, 0x60, 4, 7, 0, 0, 0, 0, 0, 532, 678, 768, 0 }, { 1, 0, 0x60, 4, 8, 0, 0, 0, 0, 0, 724, 630, 717, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag641_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C2, 0x358, 0xFC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C2, 0x350, 0x2F8, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C2, 0x118, 0x2EC, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C2, 0x120, 0x100, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag641_funcs = { wstag641_setup };
