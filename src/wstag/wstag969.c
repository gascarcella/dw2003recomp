#include "wstag.h"

/* WSTAG969: stage 0x2E9 (fieldstg_stages_2d). */

extern WstagExits *wstag969_exits[];
extern WstagFuncs wstag969_funcs;
extern FieldstgVramPlace wstag969_vram_places[];
extern FieldstgPlacedActor *wstag969_actors[];
extern FieldstgSprite wstag969_sprites[];
extern FieldstgMapEvent wstag969_map_events[];
extern FieldstgBattleLists wstag969_battle_lists[];

s32 wstag969_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag969_set_exits(dst, list + 1, arg2, arg3);
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

void wstag969_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag969_set_exits(fieldstg_stage.map_events, wstag969_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag969_start(void *arg0) {
    WstagObject *obj = object_new(wstag969_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag969_funcs.setup();
    return obj;
}

void wstag969_setup(void) {
    fieldstg_stage.background_file = 0x6B3;
    fieldstg_stage.sprites = wstag969_sprites;
    fieldstg_stage.map_events = wstag969_map_events;
    fieldstg_stage.sprite_file = 0x09410004;
    fieldstg_stage.mask_file = 0x940;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x17900, 0x17300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag969_vram_places;
    fieldstg_stage.music = 0x1E;
    fieldstg_stage.sound = 0x60780000;
    fieldstg_stage.actors = wstag969_actors;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag969_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09410006);
    fieldstg_attr.set_file(7, 0x09410007);
    fieldstg_attr.set_file(4, 0x09410005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag969_setup(void);

WstagExit D_WSTAG969_800A60DC = { 0x2EE, 2, 9, 0xE0, 0x240, 5, NULL };
WstagExit D_WSTAG969_800A60EC = { 0x28C, 0, 0, 0x410, 0x400, 0, &D_WSTAG969_800A60DC };
WstagExits D_WSTAG969_800A60FC = { 2, 1, &D_WSTAG969_800A60EC };
WstagExit D_WSTAG969_800A6104 = { 0x2EE, 3, 5, 0xE0, 0x240, 5, NULL };
WstagExit D_WSTAG969_800A6114 = { 0x28C, 0, 0, 0x110, 0x290, 0, &D_WSTAG969_800A6104 };
WstagExits D_WSTAG969_800A6124 = { 3, 1, &D_WSTAG969_800A6114 };
WstagExit D_WSTAG969_800A612C = { 0x2EE, 4, 3, 0xE0, 0x240, 5, NULL };
WstagExit D_WSTAG969_800A613C = { 0x299, 0, 0, 0x440, 0x2F8, 0, &D_WSTAG969_800A612C };
WstagExits D_WSTAG969_800A614C = { 4, 1, &D_WSTAG969_800A613C };
WstagExit D_WSTAG969_800A6154 = { 0x2EE, 6, 9, 0xE0, 0x240, 5, NULL };
WstagExit D_WSTAG969_800A6164 = { 0x299, 0, 0, 0x2D0, 0x590, 0, &D_WSTAG969_800A6154 };
WstagExits D_WSTAG969_800A6174 = { 6, 1, &D_WSTAG969_800A6164 };
WstagExits *wstag969_exits[5] = {
    &D_WSTAG969_800A60FC, &D_WSTAG969_800A6124, &D_WSTAG969_800A614C, &D_WSTAG969_800A6174, NULL,
};
FieldstgVramPlace wstag969_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 332, 320, 48, 64, 336, 511 }, { 320, 256, 332, 256, 48, 0, 352, 511 },
    { 320, 256, 352, 256, 128, 0, 368, 511 },
};
u16 D_WSTAG969_800A6220[6] = { 0x7E01, 0, 8, 0, 0xFFFF, 0 };
u16 D_WSTAG969_800A622C[6] = { 0x7E02, 0, 9, 0, 0xFFFF, 0 };
u16 D_WSTAG969_800A6238[6] = { 0x7E02, 1, 9, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG969_800A6244 = { NULL, NULL, 326, 4, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG969_800A6258 = { D_WSTAG969_800A6220, NULL, 328, 5, 272, 344, 1 };
FieldstgPlacedActor D_WSTAG969_800A626C = { D_WSTAG969_800A622C, NULL, 351, 6, 432, 312, 1 };
FieldstgPlacedActor D_WSTAG969_800A6280 = { D_WSTAG969_800A6238, NULL, 351, 6, 224, 320, 1 };
FieldstgPlacedActor *wstag969_actors[5] = {
    &D_WSTAG969_800A6244, &D_WSTAG969_800A6258, &D_WSTAG969_800A626C, &D_WSTAG969_800A6280, NULL,
};
FieldstgSprite wstag969_sprites[2] = {
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x14, 0, 152, 140, 253, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag969_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E9, 0xB0, 0xF8, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E9, 0x240, 0xF0, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag969_funcs = { wstag969_setup };
FieldstgListedBattle D_WSTAG969_800A6318 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6324 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6330 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A633C = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6348 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6354 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6360 = { 55, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A636C = { 55, 10, 0x60080000 };
FieldstgBattleList D_WSTAG969_800A6378 = {
    3,
    { &D_WSTAG969_800A6318, &D_WSTAG969_800A6324, &D_WSTAG969_800A6330, &D_WSTAG969_800A633C, &D_WSTAG969_800A6348,
        &D_WSTAG969_800A6354, &D_WSTAG969_800A6360, &D_WSTAG969_800A636C },
};
FieldstgListedBattle D_WSTAG969_800A639C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A63F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A63FC = {
    0,
    { &D_WSTAG969_800A639C, &D_WSTAG969_800A63A8, &D_WSTAG969_800A63B4, &D_WSTAG969_800A63C0, &D_WSTAG969_800A63CC,
        &D_WSTAG969_800A63D8, &D_WSTAG969_800A63E4, &D_WSTAG969_800A63F0 },
};
FieldstgListedBattle D_WSTAG969_800A6420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A642C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6438 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6444 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6450 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A645C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6468 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6474 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6480 = {
    0,
    { &D_WSTAG969_800A6420, &D_WSTAG969_800A642C, &D_WSTAG969_800A6438, &D_WSTAG969_800A6444, &D_WSTAG969_800A6450,
        &D_WSTAG969_800A645C, &D_WSTAG969_800A6468, &D_WSTAG969_800A6474 },
};
FieldstgListedBattle D_WSTAG969_800A64A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A64F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6504 = {
    0,
    { &D_WSTAG969_800A64A4, &D_WSTAG969_800A64B0, &D_WSTAG969_800A64BC, &D_WSTAG969_800A64C8, &D_WSTAG969_800A64D4,
        &D_WSTAG969_800A64E0, &D_WSTAG969_800A64EC, &D_WSTAG969_800A64F8 },
};
FieldstgListedBattle D_WSTAG969_800A6528 = { 111, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6534 = { 111, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6540 = { 112, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A654C = { 112, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6558 = { 119, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6564 = { 119, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6570 = { 168, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A657C = { 168, 10, 0x60080000 };
FieldstgBattleList D_WSTAG969_800A6588 = {
    3,
    { &D_WSTAG969_800A6528, &D_WSTAG969_800A6534, &D_WSTAG969_800A6540, &D_WSTAG969_800A654C, &D_WSTAG969_800A6558,
        &D_WSTAG969_800A6564, &D_WSTAG969_800A6570, &D_WSTAG969_800A657C },
};
FieldstgListedBattle D_WSTAG969_800A65AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A65F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6600 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A660C = {
    0,
    { &D_WSTAG969_800A65AC, &D_WSTAG969_800A65B8, &D_WSTAG969_800A65C4, &D_WSTAG969_800A65D0, &D_WSTAG969_800A65DC,
        &D_WSTAG969_800A65E8, &D_WSTAG969_800A65F4, &D_WSTAG969_800A6600 },
};
FieldstgListedBattle D_WSTAG969_800A6630 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A663C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6648 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6654 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6660 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A666C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6678 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6684 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6690 = {
    0,
    { &D_WSTAG969_800A6630, &D_WSTAG969_800A663C, &D_WSTAG969_800A6648, &D_WSTAG969_800A6654, &D_WSTAG969_800A6660,
        &D_WSTAG969_800A666C, &D_WSTAG969_800A6678, &D_WSTAG969_800A6684 },
};
FieldstgListedBattle D_WSTAG969_800A66B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A66FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6708 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6714 = {
    0,
    { &D_WSTAG969_800A66B4, &D_WSTAG969_800A66C0, &D_WSTAG969_800A66CC, &D_WSTAG969_800A66D8, &D_WSTAG969_800A66E4,
        &D_WSTAG969_800A66F0, &D_WSTAG969_800A66FC, &D_WSTAG969_800A6708 },
};
FieldstgListedBattle D_WSTAG969_800A6738 = { 113, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6744 = { 113, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6750 = { 114, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A675C = { 114, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6768 = { 115, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6774 = { 115, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6780 = { 167, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A678C = { 167, 10, 0x60080000 };
FieldstgBattleList D_WSTAG969_800A6798 = {
    3,
    { &D_WSTAG969_800A6738, &D_WSTAG969_800A6744, &D_WSTAG969_800A6750, &D_WSTAG969_800A675C, &D_WSTAG969_800A6768,
        &D_WSTAG969_800A6774, &D_WSTAG969_800A6780, &D_WSTAG969_800A678C },
};
FieldstgListedBattle D_WSTAG969_800A67BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A67C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A67D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A67E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A67EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A67F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6804 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6810 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A681C = {
    0,
    { &D_WSTAG969_800A67BC, &D_WSTAG969_800A67C8, &D_WSTAG969_800A67D4, &D_WSTAG969_800A67E0, &D_WSTAG969_800A67EC,
        &D_WSTAG969_800A67F8, &D_WSTAG969_800A6804, &D_WSTAG969_800A6810 },
};
FieldstgListedBattle D_WSTAG969_800A6840 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A684C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6858 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6864 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6870 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A687C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6888 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6894 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A68A0 = {
    0,
    { &D_WSTAG969_800A6840, &D_WSTAG969_800A684C, &D_WSTAG969_800A6858, &D_WSTAG969_800A6864, &D_WSTAG969_800A6870,
        &D_WSTAG969_800A687C, &D_WSTAG969_800A6888, &D_WSTAG969_800A6894 },
};
FieldstgListedBattle D_WSTAG969_800A68C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A68D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A68DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A68E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A68F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6900 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A690C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6918 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6924 = {
    0,
    { &D_WSTAG969_800A68C4, &D_WSTAG969_800A68D0, &D_WSTAG969_800A68DC, &D_WSTAG969_800A68E8, &D_WSTAG969_800A68F4,
        &D_WSTAG969_800A6900, &D_WSTAG969_800A690C, &D_WSTAG969_800A6918 },
};
FieldstgListedBattle D_WSTAG969_800A6948 = { 73, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6954 = { 73, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6960 = { 86, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A696C = { 86, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6978 = { 85, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6984 = { 170, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A6990 = { 92, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG969_800A699C = { 174, 10, 0x60080000 };
FieldstgBattleList D_WSTAG969_800A69A8 = {
    3,
    { &D_WSTAG969_800A6948, &D_WSTAG969_800A6954, &D_WSTAG969_800A6960, &D_WSTAG969_800A696C, &D_WSTAG969_800A6978,
        &D_WSTAG969_800A6984, &D_WSTAG969_800A6990, &D_WSTAG969_800A699C },
};
FieldstgListedBattle D_WSTAG969_800A69CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A69D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A69E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A69F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A69FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A20 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6A2C = {
    0,
    { &D_WSTAG969_800A69CC, &D_WSTAG969_800A69D8, &D_WSTAG969_800A69E4, &D_WSTAG969_800A69F0, &D_WSTAG969_800A69FC,
        &D_WSTAG969_800A6A08, &D_WSTAG969_800A6A14, &D_WSTAG969_800A6A20 },
};
FieldstgListedBattle D_WSTAG969_800A6A50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6A98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6AA4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6AB0 = {
    0,
    { &D_WSTAG969_800A6A50, &D_WSTAG969_800A6A5C, &D_WSTAG969_800A6A68, &D_WSTAG969_800A6A74, &D_WSTAG969_800A6A80,
        &D_WSTAG969_800A6A8C, &D_WSTAG969_800A6A98, &D_WSTAG969_800A6AA4 },
};
FieldstgListedBattle D_WSTAG969_800A6AD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6AE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6AEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6AF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6B04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6B10 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6B1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG969_800A6B28 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG969_800A6B34 = {
    0,
    { &D_WSTAG969_800A6AD4, &D_WSTAG969_800A6AE0, &D_WSTAG969_800A6AEC, &D_WSTAG969_800A6AF8, &D_WSTAG969_800A6B04,
        &D_WSTAG969_800A6B10, &D_WSTAG969_800A6B1C, &D_WSTAG969_800A6B28 },
};
FieldstgBattleLists wstag969_battle_lists[4] = {
    { 420, 2, 0, { &D_WSTAG969_800A6378, &D_WSTAG969_800A63FC, &D_WSTAG969_800A6480 }, &D_WSTAG969_800A6504 },
    { 426, 3, 0, { &D_WSTAG969_800A6588, &D_WSTAG969_800A660C, &D_WSTAG969_800A6690 }, &D_WSTAG969_800A6714 },
    { 432, 4, 0, { &D_WSTAG969_800A6798, &D_WSTAG969_800A681C, &D_WSTAG969_800A68A0 }, &D_WSTAG969_800A6924 },
    { 443, 6, 0, { &D_WSTAG969_800A69A8, &D_WSTAG969_800A6A2C, &D_WSTAG969_800A6AB0 }, &D_WSTAG969_800A6B34 },
};
