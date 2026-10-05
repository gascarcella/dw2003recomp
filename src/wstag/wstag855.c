#include "wstag.h"

/* WSTAG855: stage 0x2E6 (fieldstg_stages). */

extern WstagExits *wstag855_exits[];
extern WstagFuncs wstag855_funcs;
const CVECTOR wstag855_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgBattleLists wstag855_battle_lists[];
extern FieldstgVramPlace wstag855_vram_places[];
extern FieldstgPlacedActor *wstag855_actors[];
extern FieldstgSprite wstag855_sprites[];
extern FieldstgMapEvent wstag855_map_events[];

s32 wstag855_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag855_set_exits(dst, list + 1, arg2, arg3);
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

void wstag855_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        wstag855_set_exits(fieldstg_stage.map_events, wstag855_exits, gamestate_data.route,
                               gamestate_data.room);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag855_start(void *arg0) {
    WstagObject *obj = object_new(wstag855_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag855_funcs.setup();
    return obj;
}

void wstag855_setup(void) {
    fieldstg_stage.background_file = 0x6F0;
    fieldstg_stage.sprite_file = 0x06F10000;
    fieldstg_stage.sprites = wstag855_sprites;
    fieldstg_stage.map_events = wstag855_map_events;
    fieldstg_stage.mask_file = 0x6EF;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13300, 0x13B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag855_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag855_actors;
    fieldstg_stage.color = wstag855_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag855_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x06F10001);
    fieldstg_attr.set_file(7, 0x06F10002);
    fieldstg_attr.set_file(4, 0x06F10003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag855_setup(void);

WstagExit D_WSTAG855_800A60F4 = { 0x2E5, 1, 1, 0x3B0, 0xD8, 1, NULL };
WstagExit D_WSTAG855_800A6104 = { 0x2E0, 1, 1, 0xB0, 0x178, 5, &D_WSTAG855_800A60F4 };
WstagExits D_WSTAG855_800A6114 = { 1, 1, &D_WSTAG855_800A6104 };
WstagExit D_WSTAG855_800A611C = { 0x2E2, 3, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A612C = { 0x2E3, 3, 1, 0xA0, 0x180, 5, &D_WSTAG855_800A611C };
WstagExits D_WSTAG855_800A613C = { 3, 1, &D_WSTAG855_800A612C };
WstagExit D_WSTAG855_800A6144 = { 0x2E2, 5, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A6154 = { 0x2E4, 5, 1, 0xC0, 0x180, 5, &D_WSTAG855_800A6144 };
WstagExits D_WSTAG855_800A6164 = { 5, 1, &D_WSTAG855_800A6154 };
WstagExit D_WSTAG855_800A616C = { 0x2E2, 9, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A617C = { 0x2E3, 9, 1, 0xA0, 0x180, 5, &D_WSTAG855_800A616C };
WstagExits D_WSTAG855_800A618C = { 9, 1, &D_WSTAG855_800A617C };
WstagExit D_WSTAG855_800A6194 = { 0x2E5, 0xB, 1, 0x3B0, 0xD8, 1, NULL };
WstagExit D_WSTAG855_800A61A4 = { 0x2E0, 0xB, 1, 0xB0, 0x178, 5, &D_WSTAG855_800A6194 };
WstagExits D_WSTAG855_800A61B4 = { 11, 1, &D_WSTAG855_800A61A4 };
WstagExit D_WSTAG855_800A61BC = { 0x2E2, 0xD, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A61CC = { 0x2E3, 0xD, 1, 0xA0, 0x180, 5, &D_WSTAG855_800A61BC };
WstagExits D_WSTAG855_800A61DC = { 13, 1, &D_WSTAG855_800A61CC };
WstagExit D_WSTAG855_800A61E4 = { 0x2E2, 0xF, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A61F4 = { 0x2E4, 0xF, 1, 0xC0, 0x180, 5, &D_WSTAG855_800A61E4 };
WstagExits D_WSTAG855_800A6204 = { 15, 1, &D_WSTAG855_800A61F4 };
WstagExit D_WSTAG855_800A620C = { 0x2E2, 0x13, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG855_800A621C = { 0x2E3, 0x13, 1, 0xA0, 0x180, 5, &D_WSTAG855_800A620C };
WstagExits D_WSTAG855_800A622C = { 19, 1, &D_WSTAG855_800A621C };
WstagExits D_WSTAG855_800A6234 = { 0, 0, &D_WSTAG855_800A612C };
WstagExits *wstag855_exits[10] = {
    &D_WSTAG855_800A6114, &D_WSTAG855_800A613C, &D_WSTAG855_800A6164, &D_WSTAG855_800A618C, &D_WSTAG855_800A61B4,
    &D_WSTAG855_800A61DC, &D_WSTAG855_800A6204, &D_WSTAG855_800A622C, &D_WSTAG855_800A6234, NULL,
};
FieldstgListedBattle D_WSTAG855_800A6264 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6270 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A627C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6288 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6294 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A62A0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A62AC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A62B8 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A62C4 = {
    2,
    { &D_WSTAG855_800A6264, &D_WSTAG855_800A6270, &D_WSTAG855_800A627C, &D_WSTAG855_800A6288, &D_WSTAG855_800A6294,
        &D_WSTAG855_800A62A0, &D_WSTAG855_800A62AC, &D_WSTAG855_800A62B8 },
};
FieldstgListedBattle D_WSTAG855_800A62E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A62F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A630C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6324 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6330 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A633C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6348 = {
    0,
    { &D_WSTAG855_800A62E8, &D_WSTAG855_800A62F4, &D_WSTAG855_800A6300, &D_WSTAG855_800A630C, &D_WSTAG855_800A6318,
        &D_WSTAG855_800A6324, &D_WSTAG855_800A6330, &D_WSTAG855_800A633C },
};
FieldstgListedBattle D_WSTAG855_800A636C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6390 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A639C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A63A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A63B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A63C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A63CC = {
    0,
    { &D_WSTAG855_800A636C, &D_WSTAG855_800A6378, &D_WSTAG855_800A6384, &D_WSTAG855_800A6390, &D_WSTAG855_800A639C,
        &D_WSTAG855_800A63A8, &D_WSTAG855_800A63B4, &D_WSTAG855_800A63C0 },
};
FieldstgListedBattle D_WSTAG855_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A63FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A642C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6438 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6444 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6450 = {
    0,
    { &D_WSTAG855_800A63F0, &D_WSTAG855_800A63FC, &D_WSTAG855_800A6408, &D_WSTAG855_800A6414, &D_WSTAG855_800A6420,
        &D_WSTAG855_800A642C, &D_WSTAG855_800A6438, &D_WSTAG855_800A6444 },
};
FieldstgListedBattle D_WSTAG855_800A6474 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6480 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A648C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6498 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A64A4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A64B0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A64BC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A64C8 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A64D4 = {
    3,
    { &D_WSTAG855_800A6474, &D_WSTAG855_800A6480, &D_WSTAG855_800A648C, &D_WSTAG855_800A6498, &D_WSTAG855_800A64A4,
        &D_WSTAG855_800A64B0, &D_WSTAG855_800A64BC, &D_WSTAG855_800A64C8 },
};
FieldstgListedBattle D_WSTAG855_800A64F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6510 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A651C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6534 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6540 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A654C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6558 = {
    0,
    { &D_WSTAG855_800A64F8, &D_WSTAG855_800A6504, &D_WSTAG855_800A6510, &D_WSTAG855_800A651C, &D_WSTAG855_800A6528,
        &D_WSTAG855_800A6534, &D_WSTAG855_800A6540, &D_WSTAG855_800A654C },
};
FieldstgListedBattle D_WSTAG855_800A657C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6588 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6594 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A65A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A65AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A65B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A65C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A65D0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A65DC = {
    0,
    { &D_WSTAG855_800A657C, &D_WSTAG855_800A6588, &D_WSTAG855_800A6594, &D_WSTAG855_800A65A0, &D_WSTAG855_800A65AC,
        &D_WSTAG855_800A65B8, &D_WSTAG855_800A65C4, &D_WSTAG855_800A65D0 },
};
FieldstgListedBattle D_WSTAG855_800A6600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A660C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6618 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6624 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6630 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A663C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6648 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6654 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6660 = {
    0,
    { &D_WSTAG855_800A6600, &D_WSTAG855_800A660C, &D_WSTAG855_800A6618, &D_WSTAG855_800A6624, &D_WSTAG855_800A6630,
        &D_WSTAG855_800A663C, &D_WSTAG855_800A6648, &D_WSTAG855_800A6654 },
};
FieldstgListedBattle D_WSTAG855_800A6684 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6690 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A669C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A66A8 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A66B4 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A66C0 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A66CC = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A66D8 = { 63, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A66E4 = {
    3,
    { &D_WSTAG855_800A6684, &D_WSTAG855_800A6690, &D_WSTAG855_800A669C, &D_WSTAG855_800A66A8, &D_WSTAG855_800A66B4,
        &D_WSTAG855_800A66C0, &D_WSTAG855_800A66CC, &D_WSTAG855_800A66D8 },
};
FieldstgListedBattle D_WSTAG855_800A6708 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6714 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6720 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A672C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6738 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6744 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6750 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A675C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6768 = {
    0,
    { &D_WSTAG855_800A6708, &D_WSTAG855_800A6714, &D_WSTAG855_800A6720, &D_WSTAG855_800A672C, &D_WSTAG855_800A6738,
        &D_WSTAG855_800A6744, &D_WSTAG855_800A6750, &D_WSTAG855_800A675C },
};
FieldstgListedBattle D_WSTAG855_800A678C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6798 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A67E0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A67EC = {
    0,
    { &D_WSTAG855_800A678C, &D_WSTAG855_800A6798, &D_WSTAG855_800A67A4, &D_WSTAG855_800A67B0, &D_WSTAG855_800A67BC,
        &D_WSTAG855_800A67C8, &D_WSTAG855_800A67D4, &D_WSTAG855_800A67E0 },
};
FieldstgListedBattle D_WSTAG855_800A6810 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A681C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6828 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6834 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6840 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A684C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6858 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6864 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6870 = {
    0,
    { &D_WSTAG855_800A6810, &D_WSTAG855_800A681C, &D_WSTAG855_800A6828, &D_WSTAG855_800A6834, &D_WSTAG855_800A6840,
        &D_WSTAG855_800A684C, &D_WSTAG855_800A6858, &D_WSTAG855_800A6864 },
};
FieldstgListedBattle D_WSTAG855_800A6894 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68A0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68AC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68B8 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68C4 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68D0 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68DC = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A68E8 = { 61, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A68F4 = {
    3,
    { &D_WSTAG855_800A6894, &D_WSTAG855_800A68A0, &D_WSTAG855_800A68AC, &D_WSTAG855_800A68B8, &D_WSTAG855_800A68C4,
        &D_WSTAG855_800A68D0, &D_WSTAG855_800A68DC, &D_WSTAG855_800A68E8 },
};
FieldstgListedBattle D_WSTAG855_800A6918 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6924 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6930 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A693C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6948 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6954 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6960 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A696C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6978 = {
    0,
    { &D_WSTAG855_800A6918, &D_WSTAG855_800A6924, &D_WSTAG855_800A6930, &D_WSTAG855_800A693C, &D_WSTAG855_800A6948,
        &D_WSTAG855_800A6954, &D_WSTAG855_800A6960, &D_WSTAG855_800A696C },
};
FieldstgListedBattle D_WSTAG855_800A699C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A69F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A69FC = {
    0,
    { &D_WSTAG855_800A699C, &D_WSTAG855_800A69A8, &D_WSTAG855_800A69B4, &D_WSTAG855_800A69C0, &D_WSTAG855_800A69CC,
        &D_WSTAG855_800A69D8, &D_WSTAG855_800A69E4, &D_WSTAG855_800A69F0 },
};
FieldstgListedBattle D_WSTAG855_800A6A20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6A74 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6A80 = {
    0,
    { &D_WSTAG855_800A6A20, &D_WSTAG855_800A6A2C, &D_WSTAG855_800A6A38, &D_WSTAG855_800A6A44, &D_WSTAG855_800A6A50,
        &D_WSTAG855_800A6A5C, &D_WSTAG855_800A6A68, &D_WSTAG855_800A6A74 },
};
FieldstgListedBattle D_WSTAG855_800A6AA4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AB0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6ABC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AC8 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AD4 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AE0 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AEC = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6AF8 = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A6B04 = {
    2,
    { &D_WSTAG855_800A6AA4, &D_WSTAG855_800A6AB0, &D_WSTAG855_800A6ABC, &D_WSTAG855_800A6AC8, &D_WSTAG855_800A6AD4,
        &D_WSTAG855_800A6AE0, &D_WSTAG855_800A6AEC, &D_WSTAG855_800A6AF8 },
};
FieldstgListedBattle D_WSTAG855_800A6B28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6B7C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6B88 = {
    0,
    { &D_WSTAG855_800A6B28, &D_WSTAG855_800A6B34, &D_WSTAG855_800A6B40, &D_WSTAG855_800A6B4C, &D_WSTAG855_800A6B58,
        &D_WSTAG855_800A6B64, &D_WSTAG855_800A6B70, &D_WSTAG855_800A6B7C },
};
FieldstgListedBattle D_WSTAG855_800A6BAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6BF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C00 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6C0C = {
    0,
    { &D_WSTAG855_800A6BAC, &D_WSTAG855_800A6BB8, &D_WSTAG855_800A6BC4, &D_WSTAG855_800A6BD0, &D_WSTAG855_800A6BDC,
        &D_WSTAG855_800A6BE8, &D_WSTAG855_800A6BF4, &D_WSTAG855_800A6C00 },
};
FieldstgListedBattle D_WSTAG855_800A6C30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6C84 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6C90 = {
    0,
    { &D_WSTAG855_800A6C30, &D_WSTAG855_800A6C3C, &D_WSTAG855_800A6C48, &D_WSTAG855_800A6C54, &D_WSTAG855_800A6C60,
        &D_WSTAG855_800A6C6C, &D_WSTAG855_800A6C78, &D_WSTAG855_800A6C84 },
};
FieldstgListedBattle D_WSTAG855_800A6CB4 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CC0 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CCC = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CD8 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CE4 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CF0 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6CFC = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6D08 = { 104, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A6D14 = {
    3,
    { &D_WSTAG855_800A6CB4, &D_WSTAG855_800A6CC0, &D_WSTAG855_800A6CCC, &D_WSTAG855_800A6CD8, &D_WSTAG855_800A6CE4,
        &D_WSTAG855_800A6CF0, &D_WSTAG855_800A6CFC, &D_WSTAG855_800A6D08 },
};
FieldstgListedBattle D_WSTAG855_800A6D38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6D8C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6D98 = {
    0,
    { &D_WSTAG855_800A6D38, &D_WSTAG855_800A6D44, &D_WSTAG855_800A6D50, &D_WSTAG855_800A6D5C, &D_WSTAG855_800A6D68,
        &D_WSTAG855_800A6D74, &D_WSTAG855_800A6D80, &D_WSTAG855_800A6D8C },
};
FieldstgListedBattle D_WSTAG855_800A6DBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6DC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6DD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6DE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6DEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6DF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E10 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6E1C = {
    0,
    { &D_WSTAG855_800A6DBC, &D_WSTAG855_800A6DC8, &D_WSTAG855_800A6DD4, &D_WSTAG855_800A6DE0, &D_WSTAG855_800A6DEC,
        &D_WSTAG855_800A6DF8, &D_WSTAG855_800A6E04, &D_WSTAG855_800A6E10 },
};
FieldstgListedBattle D_WSTAG855_800A6E40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6E94 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6EA0 = {
    0,
    { &D_WSTAG855_800A6E40, &D_WSTAG855_800A6E4C, &D_WSTAG855_800A6E58, &D_WSTAG855_800A6E64, &D_WSTAG855_800A6E70,
        &D_WSTAG855_800A6E7C, &D_WSTAG855_800A6E88, &D_WSTAG855_800A6E94 },
};
FieldstgListedBattle D_WSTAG855_800A6EC4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6ED0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6EDC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6EE8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6EF4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6F00 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6F0C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A6F18 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A6F24 = {
    3,
    { &D_WSTAG855_800A6EC4, &D_WSTAG855_800A6ED0, &D_WSTAG855_800A6EDC, &D_WSTAG855_800A6EE8, &D_WSTAG855_800A6EF4,
        &D_WSTAG855_800A6F00, &D_WSTAG855_800A6F0C, &D_WSTAG855_800A6F18 },
};
FieldstgListedBattle D_WSTAG855_800A6F48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6F9C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A6FA8 = {
    0,
    { &D_WSTAG855_800A6F48, &D_WSTAG855_800A6F54, &D_WSTAG855_800A6F60, &D_WSTAG855_800A6F6C, &D_WSTAG855_800A6F78,
        &D_WSTAG855_800A6F84, &D_WSTAG855_800A6F90, &D_WSTAG855_800A6F9C },
};
FieldstgListedBattle D_WSTAG855_800A6FCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6FD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6FE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6FF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A6FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7020 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A702C = {
    0,
    { &D_WSTAG855_800A6FCC, &D_WSTAG855_800A6FD8, &D_WSTAG855_800A6FE4, &D_WSTAG855_800A6FF0, &D_WSTAG855_800A6FFC,
        &D_WSTAG855_800A7008, &D_WSTAG855_800A7014, &D_WSTAG855_800A7020 },
};
FieldstgListedBattle D_WSTAG855_800A7050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A705C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A708C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A70A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A70B0 = {
    0,
    { &D_WSTAG855_800A7050, &D_WSTAG855_800A705C, &D_WSTAG855_800A7068, &D_WSTAG855_800A7074, &D_WSTAG855_800A7080,
        &D_WSTAG855_800A708C, &D_WSTAG855_800A7098, &D_WSTAG855_800A70A4 },
};
FieldstgListedBattle D_WSTAG855_800A70D4 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A70E0 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A70EC = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A70F8 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A7104 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A7110 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A711C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG855_800A7128 = { 104, 11, 0x60080000 };
FieldstgBattleList D_WSTAG855_800A7134 = {
    3,
    { &D_WSTAG855_800A70D4, &D_WSTAG855_800A70E0, &D_WSTAG855_800A70EC, &D_WSTAG855_800A70F8, &D_WSTAG855_800A7104,
        &D_WSTAG855_800A7110, &D_WSTAG855_800A711C, &D_WSTAG855_800A7128 },
};
FieldstgListedBattle D_WSTAG855_800A7158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A717C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A71A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A71AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A71B8 = {
    0,
    { &D_WSTAG855_800A7158, &D_WSTAG855_800A7164, &D_WSTAG855_800A7170, &D_WSTAG855_800A717C, &D_WSTAG855_800A7188,
        &D_WSTAG855_800A7194, &D_WSTAG855_800A71A0, &D_WSTAG855_800A71AC },
};
FieldstgListedBattle D_WSTAG855_800A71DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A71E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A71F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7200 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A720C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7218 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7230 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A723C = {
    0,
    { &D_WSTAG855_800A71DC, &D_WSTAG855_800A71E8, &D_WSTAG855_800A71F4, &D_WSTAG855_800A7200, &D_WSTAG855_800A720C,
        &D_WSTAG855_800A7218, &D_WSTAG855_800A7224, &D_WSTAG855_800A7230 },
};
FieldstgListedBattle D_WSTAG855_800A7260 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A726C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7278 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7284 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A7290 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A729C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A72A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG855_800A72B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG855_800A72C0 = {
    0,
    { &D_WSTAG855_800A7260, &D_WSTAG855_800A726C, &D_WSTAG855_800A7278, &D_WSTAG855_800A7284, &D_WSTAG855_800A7290,
        &D_WSTAG855_800A729C, &D_WSTAG855_800A72A8, &D_WSTAG855_800A72B4 },
};
FieldstgBattleLists wstag855_battle_lists[8] = {
    { 174, 1, 0, { &D_WSTAG855_800A62C4, &D_WSTAG855_800A6348, &D_WSTAG855_800A63CC }, &D_WSTAG855_800A6450 },
    { 181, 3, 0, { &D_WSTAG855_800A64D4, &D_WSTAG855_800A6558, &D_WSTAG855_800A65DC }, &D_WSTAG855_800A6660 },
    { 189, 5, 0, { &D_WSTAG855_800A66E4, &D_WSTAG855_800A6768, &D_WSTAG855_800A67EC }, &D_WSTAG855_800A6870 },
    { 199, 9, 0, { &D_WSTAG855_800A68F4, &D_WSTAG855_800A6978, &D_WSTAG855_800A69FC }, &D_WSTAG855_800A6A80 },
    { 202, 11, 0, { &D_WSTAG855_800A6B04, &D_WSTAG855_800A6B88, &D_WSTAG855_800A6C0C }, &D_WSTAG855_800A6C90 },
    { 209, 13, 0, { &D_WSTAG855_800A6D14, &D_WSTAG855_800A6D98, &D_WSTAG855_800A6E1C }, &D_WSTAG855_800A6EA0 },
    { 217, 15, 0, { &D_WSTAG855_800A6F24, &D_WSTAG855_800A6FA8, &D_WSTAG855_800A702C }, &D_WSTAG855_800A70B0 },
    { 227, 19, 0, { &D_WSTAG855_800A7134, &D_WSTAG855_800A71B8, &D_WSTAG855_800A723C }, &D_WSTAG855_800A72C0 },
};
FieldstgVramPlace wstag855_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 444, 192, 188, 336, 511 },
};
FieldstgPlacedActor D_WSTAG855_800A7434 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag855_actors[2] = { &D_WSTAG855_800A7434, NULL };
FieldstgSprite wstag855_sprites[35] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 155, 230, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 440, 261, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 797, 447, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 1127, 463, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 312, 203, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 657, 342, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 916, 467, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 768, 289, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 215, 158, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 591, 261, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 983, 460, 0, 0 },
    { 1, 0, 0x50, 4, 0, 0, 0, 0, 0, 0, 192, 257, 340, 0 }, { 1, 0, 0x65, 4, 1, 0, 0, 0, 0, 0, 160, 241, 334, 0 },
    { 1, 0, 0x5E, 4, 2, 0, 0, 0, 0, 0, 144, 237, 324, 0 }, { 1, 0, 0x91, 4, 3, 0, 0, 0, 0, 0, 368, 184, 306, 0 },
    { 1, 0, 0x77, 4, 4, 0, 0, 0, 0, 0, 336, 204, 322, 0 }, { 1, 0, 0x89, 4, 5, 0, 0, 0, 0, 0, 304, 218, 347, 0 },
    { 1, 0, 0x82, 4, 6, 0, 0, 0, 0, 0, 272, 238, 354, 0 }, { 1, 0, 0x5E, 4, 7, 0, 0, 0, 0, 0, 464, 278, 368, 0 },
    { 1, 0, 0x66, 4, 8, 0, 0, 0, 0, 0, 432, 283, 375, 0 }, { 1, 0, 0x52, 4, 9, 0, 0, 0, 0, 0, 414, 298, 380, 0 },
    { 1, 0, 0x51, 4, 0xA, 0, 0, 0, 0, 0, 544, 316, 390, 0 },
    { 1, 0, 0x60, 4, 0xB, 0, 0, 0, 0, 0, 512, 320, 405, 0 },
    { 1, 0, 0x52, 4, 0xC, 0, 0, 0, 0, 0, 494, 336, 415, 0 },
    { 1, 0, 0x93, 4, 0xD, 0, 0, 0, 0, 0, 720, 311, 435, 0 },
    { 1, 0, 0x79, 4, 0xE, 0, 0, 0, 0, 0, 688, 331, 451, 0 },
    { 1, 0, 0x8B, 4, 0xF, 0, 0, 0, 0, 0, 656, 345, 476, 0 },
    { 1, 0, 0x89, 4, 0x10, 0, 0, 0, 0, 0, 624, 360, 483, 0 },
    { 1, 0, 0x5D, 4, 0x11, 0, 0, 0, 0, 0, 752, 421, 506, 0 },
    { 1, 0, 0x67, 4, 0x12, 0, 0, 0, 0, 0, 720, 424, 515, 0 },
    { 1, 0, 0x50, 4, 0x13, 0, 0, 0, 0, 0, 702, 441, 527, 0 },
    { 1, 0, 0x52, 4, 0x14, 0, 0, 0, 0, 0, 1104, 521, 606, 0 },
    { 1, 0, 0x65, 4, 0x15, 0, 0, 0, 0, 0, 1072, 505, 598, 0 },
    { 1, 0, 0x5D, 4, 0x16, 0, 0, 0, 0, 0, 1057, 501, 587, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag855_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E6, 0x450, 0x248, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E6, 0xA0, 0x150, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag855_funcs = { wstag855_setup };
