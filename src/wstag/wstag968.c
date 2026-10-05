#include "wstag.h"

/* WSTAG968: stage 0x2E8 (fieldstg_stages_2d). */

extern WstagExits *wstag968_exits[];
extern WstagFuncs wstag968_funcs;
extern FieldstgVramPlace wstag968_vram_places[];
extern FieldstgPlacedActor *wstag968_actors[];
extern FieldstgSprite wstag968_sprites[];
extern FieldstgMapEvent wstag968_map_events[];
extern FieldstgBattleLists wstag968_battle_lists[];

s32 wstag968_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag968_set_exits(dst, list + 1, arg2, arg3);
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

void wstag968_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag968_set_exits(fieldstg_stage.map_events, wstag968_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag968_start(void *arg0) {
    WstagObject *obj = object_new(wstag968_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag968_funcs.setup();
    return obj;
}

void wstag968_setup(void) {
    fieldstg_stage.background_file = 0x6AF;
    fieldstg_stage.sprites = wstag968_sprites;
    fieldstg_stage.map_events = wstag968_map_events;
    fieldstg_stage.sprite_file = 0x093F0004;
    fieldstg_stage.mask_file = 0x93E;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF900, 0x14000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag968_vram_places;
    fieldstg_stage.music = 0x1E;
    fieldstg_stage.sound = 0x60780000;
    fieldstg_stage.actors = wstag968_actors;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag968_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x093F0006);
    fieldstg_attr.set_file(7, 0x093F0007);
    fieldstg_attr.set_file(4, 0x093F0005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag968_setup(void);

WstagExit D_WSTAG968_800A60D8 = { 0x2EE, 1, 1, 0x3A0, 0x1A0, 1, NULL };
WstagExit D_WSTAG968_800A60E8 = { 0x298, 0, 0, 0x1F0, 0x360, 0, &D_WSTAG968_800A60D8 };
WstagExits D_WSTAG968_800A60F8 = { 1, 1, &D_WSTAG968_800A60E8 };
WstagExit D_WSTAG968_800A6100 = { 0x2EC, 1, 8, 0x3B0, 0x78, 1, NULL };
WstagExit D_WSTAG968_800A6110 = { 0x298, 0, 0, 0x560, 0x238, 0, &D_WSTAG968_800A6100 };
WstagExits D_WSTAG968_800A6120 = { 1, 2, &D_WSTAG968_800A6110 };
WstagExit D_WSTAG968_800A6128 = { 0x2EC, 3, 2, 0x3B0, 0x78, 1, NULL };
WstagExit D_WSTAG968_800A6138 = { 0x28F, 0, 0, 0x398, 0x270, 0, &D_WSTAG968_800A6128 };
WstagExits D_WSTAG968_800A6148 = { 3, 1, &D_WSTAG968_800A6138 };
WstagExit D_WSTAG968_800A6150 = { 0x2EC, 4, 1, 0x3B0, 0x78, 1, NULL };
WstagExit D_WSTAG968_800A6160 = { 0x28C, 0, 0, 0x290, 0x200, 0, &D_WSTAG968_800A6150 };
WstagExits D_WSTAG968_800A6170 = { 4, 1, &D_WSTAG968_800A6160 };
WstagExit D_WSTAG968_800A6178 = { 0x2ED, 5, 1, 0x3A0, 0x80, 1, NULL };
WstagExit D_WSTAG968_800A6188 = { 0x28F, 0, 0, 0x450, 0x226, 0, &D_WSTAG968_800A6178 };
WstagExits D_WSTAG968_800A6198 = { 5, 1, &D_WSTAG968_800A6188 };
WstagExits *wstag968_exits[6] = {
    &D_WSTAG968_800A60F8, &D_WSTAG968_800A6120, &D_WSTAG968_800A6148, &D_WSTAG968_800A6170, &D_WSTAG968_800A6198,
    NULL,
};
FieldstgVramPlace wstag968_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 332, 320, 48, 64, 336, 511 }, { 320, 256, 332, 256, 48, 0, 352, 511 },
    { 320, 256, 352, 256, 128, 0, 368, 511 },
};
u16 D_WSTAG968_800A6248[6] = { 0x7E00, 0, 8, 0, 0xFFFF, 0 };
u16 D_WSTAG968_800A6254[6] = { 0x7E04, 1, 9, 0, 0xFFFF, 0 };
u16 D_WSTAG968_800A6260[6] = { 0x7E02, 1, 9, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG968_800A626C = { NULL, NULL, 326, 4, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG968_800A6280 = { D_WSTAG968_800A6248, NULL, 328, 5, 368, 344, 1 };
FieldstgPlacedActor D_WSTAG968_800A6294 = { D_WSTAG968_800A6254, NULL, 351, 6, 528, 280, 1 };
FieldstgPlacedActor D_WSTAG968_800A62A8 = { D_WSTAG968_800A6260, NULL, 351, 6, 272, 312, 1 };
FieldstgPlacedActor *wstag968_actors[5] = {
    &D_WSTAG968_800A626C, &D_WSTAG968_800A6280, &D_WSTAG968_800A6294, &D_WSTAG968_800A62A8, NULL,
};
FieldstgSprite wstag968_sprites[2] = {
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x14, 0, 552, 100, 213, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag968_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E8, 0x240, 0xD0, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E8, 0xB0, 0x168, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag968_funcs = { wstag968_setup };
FieldstgListedBattle D_WSTAG968_800A6340 = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A634C = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6358 = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6364 = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6370 = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A637C = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6388 = { 38, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6394 = { 38, 10, 0x60080000 };
FieldstgBattleList D_WSTAG968_800A63A0 = {
    3,
    { &D_WSTAG968_800A6340, &D_WSTAG968_800A634C, &D_WSTAG968_800A6358, &D_WSTAG968_800A6364, &D_WSTAG968_800A6370,
        &D_WSTAG968_800A637C, &D_WSTAG968_800A6388, &D_WSTAG968_800A6394 },
};
FieldstgListedBattle D_WSTAG968_800A63C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A63D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6400 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A640C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6418 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6424 = {
    0,
    { &D_WSTAG968_800A63C4, &D_WSTAG968_800A63D0, &D_WSTAG968_800A63DC, &D_WSTAG968_800A63E8, &D_WSTAG968_800A63F4,
        &D_WSTAG968_800A6400, &D_WSTAG968_800A640C, &D_WSTAG968_800A6418 },
};
FieldstgListedBattle D_WSTAG968_800A6448 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6454 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A649C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A64A8 = {
    0,
    { &D_WSTAG968_800A6448, &D_WSTAG968_800A6454, &D_WSTAG968_800A6460, &D_WSTAG968_800A646C, &D_WSTAG968_800A6478,
        &D_WSTAG968_800A6484, &D_WSTAG968_800A6490, &D_WSTAG968_800A649C },
};
FieldstgListedBattle D_WSTAG968_800A64CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A64D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A64E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A64F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A64FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6508 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6514 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6520 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A652C = {
    0,
    { &D_WSTAG968_800A64CC, &D_WSTAG968_800A64D8, &D_WSTAG968_800A64E4, &D_WSTAG968_800A64F0, &D_WSTAG968_800A64FC,
        &D_WSTAG968_800A6508, &D_WSTAG968_800A6514, &D_WSTAG968_800A6520 },
};
FieldstgListedBattle D_WSTAG968_800A6550 = { 111, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A655C = { 111, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6568 = { 112, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6574 = { 112, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6580 = { 119, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A658C = { 119, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6598 = { 168, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A65A4 = { 168, 10, 0x60080000 };
FieldstgBattleList D_WSTAG968_800A65B0 = {
    3,
    { &D_WSTAG968_800A6550, &D_WSTAG968_800A655C, &D_WSTAG968_800A6568, &D_WSTAG968_800A6574, &D_WSTAG968_800A6580,
        &D_WSTAG968_800A658C, &D_WSTAG968_800A6598, &D_WSTAG968_800A65A4 },
};
FieldstgListedBattle D_WSTAG968_800A65D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A65E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A65EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A65F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6604 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6610 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A661C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6628 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6634 = {
    0,
    { &D_WSTAG968_800A65D4, &D_WSTAG968_800A65E0, &D_WSTAG968_800A65EC, &D_WSTAG968_800A65F8, &D_WSTAG968_800A6604,
        &D_WSTAG968_800A6610, &D_WSTAG968_800A661C, &D_WSTAG968_800A6628 },
};
FieldstgListedBattle D_WSTAG968_800A6658 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6664 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6670 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A667C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6688 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6694 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A66AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A66B8 = {
    0,
    { &D_WSTAG968_800A6658, &D_WSTAG968_800A6664, &D_WSTAG968_800A6670, &D_WSTAG968_800A667C, &D_WSTAG968_800A6688,
        &D_WSTAG968_800A6694, &D_WSTAG968_800A66A0, &D_WSTAG968_800A66AC },
};
FieldstgListedBattle D_WSTAG968_800A66DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A66F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6700 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A670C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6718 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6730 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A673C = {
    0,
    { &D_WSTAG968_800A66DC, &D_WSTAG968_800A66E8, &D_WSTAG968_800A66F4, &D_WSTAG968_800A6700, &D_WSTAG968_800A670C,
        &D_WSTAG968_800A6718, &D_WSTAG968_800A6724, &D_WSTAG968_800A6730 },
};
FieldstgListedBattle D_WSTAG968_800A6760 = { 113, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A676C = { 113, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6778 = { 114, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6784 = { 114, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6790 = { 115, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A679C = { 115, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A67A8 = { 167, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A67B4 = { 167, 10, 0x60080000 };
FieldstgBattleList D_WSTAG968_800A67C0 = {
    3,
    { &D_WSTAG968_800A6760, &D_WSTAG968_800A676C, &D_WSTAG968_800A6778, &D_WSTAG968_800A6784, &D_WSTAG968_800A6790,
        &D_WSTAG968_800A679C, &D_WSTAG968_800A67A8, &D_WSTAG968_800A67B4 },
};
FieldstgListedBattle D_WSTAG968_800A67E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A67FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6808 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6814 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6820 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A682C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6838 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6844 = {
    0,
    { &D_WSTAG968_800A67E4, &D_WSTAG968_800A67F0, &D_WSTAG968_800A67FC, &D_WSTAG968_800A6808, &D_WSTAG968_800A6814,
        &D_WSTAG968_800A6820, &D_WSTAG968_800A682C, &D_WSTAG968_800A6838 },
};
FieldstgListedBattle D_WSTAG968_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6880 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A688C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6898 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A68B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A68BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A68C8 = {
    0,
    { &D_WSTAG968_800A6868, &D_WSTAG968_800A6874, &D_WSTAG968_800A6880, &D_WSTAG968_800A688C, &D_WSTAG968_800A6898,
        &D_WSTAG968_800A68A4, &D_WSTAG968_800A68B0, &D_WSTAG968_800A68BC },
};
FieldstgListedBattle D_WSTAG968_800A68EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A68F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6904 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6910 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A691C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6928 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6934 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6940 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A694C = {
    0,
    { &D_WSTAG968_800A68EC, &D_WSTAG968_800A68F8, &D_WSTAG968_800A6904, &D_WSTAG968_800A6910, &D_WSTAG968_800A691C,
        &D_WSTAG968_800A6928, &D_WSTAG968_800A6934, &D_WSTAG968_800A6940 },
};
FieldstgListedBattle D_WSTAG968_800A6970 = { 74, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A697C = { 77, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6988 = { 78, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A6994 = { 79, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A69A0 = { 80, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A69AC = { 75, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A69B8 = { 76, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG968_800A69C4 = { 89, 10, 0x60080000 };
FieldstgBattleList D_WSTAG968_800A69D0 = {
    3,
    { &D_WSTAG968_800A6970, &D_WSTAG968_800A697C, &D_WSTAG968_800A6988, &D_WSTAG968_800A6994, &D_WSTAG968_800A69A0,
        &D_WSTAG968_800A69AC, &D_WSTAG968_800A69B8, &D_WSTAG968_800A69C4 },
};
FieldstgListedBattle D_WSTAG968_800A69F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A48 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6A54 = {
    0,
    { &D_WSTAG968_800A69F4, &D_WSTAG968_800A6A00, &D_WSTAG968_800A6A0C, &D_WSTAG968_800A6A18, &D_WSTAG968_800A6A24,
        &D_WSTAG968_800A6A30, &D_WSTAG968_800A6A3C, &D_WSTAG968_800A6A48 },
};
FieldstgListedBattle D_WSTAG968_800A6A78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6A9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6AA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6AB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6AC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6ACC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6AD8 = {
    0,
    { &D_WSTAG968_800A6A78, &D_WSTAG968_800A6A84, &D_WSTAG968_800A6A90, &D_WSTAG968_800A6A9C, &D_WSTAG968_800A6AA8,
        &D_WSTAG968_800A6AB4, &D_WSTAG968_800A6AC0, &D_WSTAG968_800A6ACC },
};
FieldstgListedBattle D_WSTAG968_800A6AFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG968_800A6B50 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG968_800A6B5C = {
    0,
    { &D_WSTAG968_800A6AFC, &D_WSTAG968_800A6B08, &D_WSTAG968_800A6B14, &D_WSTAG968_800A6B20, &D_WSTAG968_800A6B2C,
        &D_WSTAG968_800A6B38, &D_WSTAG968_800A6B44, &D_WSTAG968_800A6B50 },
};
FieldstgBattleLists wstag968_battle_lists[4] = {
    { 414, 1, 0, { &D_WSTAG968_800A63A0, &D_WSTAG968_800A6424, &D_WSTAG968_800A64A8 }, &D_WSTAG968_800A652C },
    { 425, 3, 0, { &D_WSTAG968_800A65B0, &D_WSTAG968_800A6634, &D_WSTAG968_800A66B8 }, &D_WSTAG968_800A673C },
    { 431, 4, 0, { &D_WSTAG968_800A67C0, &D_WSTAG968_800A6844, &D_WSTAG968_800A68C8 }, &D_WSTAG968_800A694C },
    { 437, 5, 0, { &D_WSTAG968_800A69D0, &D_WSTAG968_800A6A54, &D_WSTAG968_800A6AD8 }, &D_WSTAG968_800A6B5C },
};
