#include "wstag.h"

/* WSTAG964: stage 0x2E4 (fieldstg_stages_2d). */

extern WstagExits *wstag964_exits[];
extern WstagFuncs wstag964_funcs;
const CVECTOR wstag964_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgVramPlace wstag964_vram_places[];
extern FieldstgPlacedActor *wstag964_actors[];
extern FieldstgSprite wstag964_sprites[];
extern FieldstgMapEvent wstag964_map_events[];
extern FieldstgBattleLists wstag964_battle_lists[];

s32 wstag964_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag964_set_exits(dst, list + 1, arg2, arg3);
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

void wstag964_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag964_set_exits(fieldstg_stage.map_events, wstag964_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag964_start(void *arg0) {
    WstagObject *obj = object_new(wstag964_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag964_funcs.setup();
    return obj;
}

void wstag964_setup(void) {
    fieldstg_stage.background_file = 0x704;
    fieldstg_stage.sprites = wstag964_sprites;
    fieldstg_stage.map_events = wstag964_map_events;
    fieldstg_stage.sprite_file = 0x09370004;
    fieldstg_stage.mask_file = 0x936;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xE500, 0x16900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag964_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag964_actors;
    fieldstg_stage.color = wstag964_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag964_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09370006);
    fieldstg_attr.set_file(7, 0x09370007);
    fieldstg_attr.set_file(4, 0x09370005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag964_setup(void);

WstagExit D_WSTAG964_800A60FC = { 0x2E6, 2, 2, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG964_800A610C = { 0x2E6, 2, 1, 0xA0, 0x150, 5, &D_WSTAG964_800A60FC };
WstagExits D_WSTAG964_800A611C = { 2, 1, &D_WSTAG964_800A610C };
WstagExit D_WSTAG964_800A6124 = { 0x2E4, 2, 3, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG964_800A6134 = { 0x2E6, 2, 2, 0xA0, 0x150, 5, &D_WSTAG964_800A6124 };
WstagExits D_WSTAG964_800A6144 = { 2, 2, &D_WSTAG964_800A6134 };
WstagExit D_WSTAG964_800A614C = { 0x2E6, 2, 3, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG964_800A615C = { 0x2E4, 2, 2, 0xC0, 0x180, 5, &D_WSTAG964_800A614C };
WstagExits D_WSTAG964_800A616C = { 2, 3, &D_WSTAG964_800A615C };
WstagExit D_WSTAG964_800A6174 = { 0x2E2, 2, 1, 0x330, 0xF8, 1, NULL };
WstagExit D_WSTAG964_800A6184 = { 0x2E6, 2, 3, 0xA0, 0x150, 5, &D_WSTAG964_800A6174 };
WstagExits D_WSTAG964_800A6194 = { 2, 4, &D_WSTAG964_800A6184 };
WstagExit D_WSTAG964_800A619C = { 0x2E5, 3, 1, 0x3B0, 0xD8, 1, NULL };
WstagExit D_WSTAG964_800A61AC = { 0x2E0, 3, 1, 0xB0, 0x178, 5, &D_WSTAG964_800A619C };
WstagExits D_WSTAG964_800A61BC = { 3, 1, &D_WSTAG964_800A61AC };
WstagExit D_WSTAG964_800A61C4 = { 0x2E7, 4, 1, 0x250, 0xE8, 1, NULL };
WstagExit D_WSTAG964_800A61D4 = { 0x2E6, 4, 1, 0xA0, 0x150, 5, &D_WSTAG964_800A61C4 };
WstagExits D_WSTAG964_800A61E4 = { 4, 1, &D_WSTAG964_800A61D4 };
WstagExit D_WSTAG964_800A61EC = { 0x2E6, 5, 1, 0x450, 0x248, 1, NULL };
WstagExit D_WSTAG964_800A61FC = { 0x2E0, 5, 1, 0xB0, 0x178, 5, &D_WSTAG964_800A61EC };
WstagExits D_WSTAG964_800A620C = { 5, 1, &D_WSTAG964_800A61FC };
WstagExit D_WSTAG964_800A6214 = { 0x2E4, 5, 3, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG964_800A6224 = { 0x2E6, 5, 1, 0xA0, 0x150, 5, &D_WSTAG964_800A6214 };
WstagExits D_WSTAG964_800A6234 = { 5, 2, &D_WSTAG964_800A6224 };
WstagExit D_WSTAG964_800A623C = { 0x2E7, 5, 1, 0x250, 0xE8, 1, NULL };
WstagExit D_WSTAG964_800A624C = { 0x2E4, 5, 2, 0xC0, 0x180, 5, &D_WSTAG964_800A623C };
WstagExits D_WSTAG964_800A625C = { 5, 3, &D_WSTAG964_800A624C };
WstagExits *wstag964_exits[10] = {
    &D_WSTAG964_800A611C, &D_WSTAG964_800A6144, &D_WSTAG964_800A616C, &D_WSTAG964_800A6194, &D_WSTAG964_800A61BC,
    &D_WSTAG964_800A61E4, &D_WSTAG964_800A620C, &D_WSTAG964_800A6234, &D_WSTAG964_800A625C, NULL,
};
FieldstgVramPlace wstag964_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 330, 416, 40, 160, 336, 511 }, { 384, 256, 432, 376, 448, 120, 352, 511 },
    { 320, 256, 368, 464, 192, 208, 368, 511 }, { 384, 256, 416, 256, 384, 0, 320, 510 },
};
u16 D_WSTAG964_800A632C[6] = { 0x273, 1, 0x8234, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A6338[6] = { 0x26F, 1, 0x847D, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A6344[6] = { 0x270, 1, 0x848A, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A6350[4] = { 0x8676, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A6358[6] = { 0x8676, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A6364[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A636C[8] = { 0x8676, 0, 0, 1, 0x8472, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A637C[8] = { 0x8676, 0, 0, 1, 0x8472, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A638C[8] = { 0x8676, 1, 0x8675, 0, 0x8472, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A639C[4] = { 0x869B, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A63A4[6] = { 0x869B, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A63B0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A63B8[8] = { 0x869B, 0, 0, 1, 0x8497, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A63C8[8] = { 0x869B, 0, 0, 1, 0x8497, 1, 0xFFFF, 0 };
u16 D_WSTAG964_800A63D8[8] = { 0x869B, 1, 0x869A, 0, 0x8497, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG964_800A63E8[2] = { { NULL, D_WSTAG964_800A632C, 11 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG964_800A6400[2] = { { NULL, D_WSTAG964_800A6338, 7 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG964_800A6418[2] = { { NULL, D_WSTAG964_800A6344, 8 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG964_800A6430[5] = {
    { D_WSTAG964_800A6350, NULL, 177 }, { D_WSTAG964_800A6358, D_WSTAG964_800A6364, 178 },
    { D_WSTAG964_800A636C, NULL, 179 }, { D_WSTAG964_800A637C, D_WSTAG964_800A638C, 180 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG964_800A646C[5] = {
    { D_WSTAG964_800A639C, NULL, 181 }, { D_WSTAG964_800A63A4, D_WSTAG964_800A63B0, 182 },
    { D_WSTAG964_800A63B8, NULL, 183 }, { D_WSTAG964_800A63C8, D_WSTAG964_800A63D8, 184 }, { NULL, NULL, 0 },
};
u16 D_WSTAG964_800A64A8[8] = { 0x7E01, 1, 0x7E1E, 1, 0x273, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A64B8[8] = { 0x7E01, 1, 0x7E21, 1, 0x26F, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A64C8[8] = { 0x7E04, 1, 0x7E1E, 1, 0x270, 0, 0xFFFF, 0 };
u16 D_WSTAG964_800A64D8[14] = {
    0x7E01, 1, 0x7E1F, 1, 0x7055, 1, 0x7095, 1,
    0x8675, 1, 0x8676, 0, 0xFFFF, 0,
};
u16 D_WSTAG964_800A64F4[14] = {
    0x7E03, 1, 0x7E1E, 1, 0x7055, 1, 0x7095, 1,
    0x869A, 1, 0x869B, 0, 0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG964_800A6510 = { D_WSTAG964_800A64A8, D_WSTAG964_800A63E8, 33, 4, 544, 240, 1 };
FieldstgPlacedActor D_WSTAG964_800A6524 = { D_WSTAG964_800A64B8, D_WSTAG964_800A6400, 33, 4, 544, 240, 1 };
FieldstgPlacedActor D_WSTAG964_800A6538 = { D_WSTAG964_800A64C8, D_WSTAG964_800A6418, 33, 4, 544, 240, 1 };
FieldstgPlacedActor D_WSTAG964_800A654C = { D_WSTAG964_800A64D8, D_WSTAG964_800A6430, 168, 5, 544, 240, 1 };
FieldstgPlacedActor D_WSTAG964_800A6560 = { D_WSTAG964_800A64F4, D_WSTAG964_800A646C, 170, 6, 544, 240, 1 };
FieldstgPlacedActor D_WSTAG964_800A6574 = { NULL, NULL, 327, 7, 0, 0, 0 };
FieldstgPlacedActor *wstag964_actors[7] = {
    &D_WSTAG964_800A6510, &D_WSTAG964_800A6524, &D_WSTAG964_800A6538, &D_WSTAG964_800A654C, &D_WSTAG964_800A6560,
    &D_WSTAG964_800A6574, NULL,
};
FieldstgSprite wstag964_sprites[16] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 200, 265, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 560, 185, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 857, 33, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 395, 180, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 500, 39, 0, 0 },
    { 1, 0, 0xB6, 6, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 659, 0, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 176, 128, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 758, 41, 0, 0 },
    { 1, 0, 0x8A, 4, 3, 0, 0, 0, 0, 0, 416, 191, 327, 0 }, { 1, 0, 0x52, 4, 4, 0, 0, 0, 0, 0, 384, 174, 311, 0 },
    { 1, 0, 0x83, 4, 5, 0, 0, 0, 0, 0, 352, 169, 295, 0 }, { 1, 0, 0x8E, 4, 6, 0, 0, 0, 0, 0, 320, 154, 279, 0 },
    { 1, 0, 0x54, 4, 8, 0, 0, 0, 0, 0, 672, 177, 268, 0 }, { 1, 0, 0x66, 4, 9, 0, 0, 0, 0, 0, 640, 162, 251, 0 },
    { 1, 0, 0x60, 4, 0xA, 0, 0, 0, 0, 0, 624, 156, 242, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag964_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E4, 0x340, 0xA0, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E4, 0xC0, 0x180, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag964_funcs = { wstag964_setup };
FieldstgListedBattle D_WSTAG964_800A6710 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A671C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6728 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6734 = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6740 = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A674C = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6758 = { 276, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6764 = { 276, 11, 0x60080000 };
FieldstgBattleList D_WSTAG964_800A6770 = {
    3,
    { &D_WSTAG964_800A6710, &D_WSTAG964_800A671C, &D_WSTAG964_800A6728, &D_WSTAG964_800A6734, &D_WSTAG964_800A6740,
        &D_WSTAG964_800A674C, &D_WSTAG964_800A6758, &D_WSTAG964_800A6764 },
};
FieldstgListedBattle D_WSTAG964_800A6794 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A67E8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A67F4 = {
    0,
    { &D_WSTAG964_800A6794, &D_WSTAG964_800A67A0, &D_WSTAG964_800A67AC, &D_WSTAG964_800A67B8, &D_WSTAG964_800A67C4,
        &D_WSTAG964_800A67D0, &D_WSTAG964_800A67DC, &D_WSTAG964_800A67E8 },
};
FieldstgListedBattle D_WSTAG964_800A6818 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6824 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6830 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A683C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6848 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6854 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6860 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A686C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6878 = {
    0,
    { &D_WSTAG964_800A6818, &D_WSTAG964_800A6824, &D_WSTAG964_800A6830, &D_WSTAG964_800A683C, &D_WSTAG964_800A6848,
        &D_WSTAG964_800A6854, &D_WSTAG964_800A6860, &D_WSTAG964_800A686C },
};
FieldstgListedBattle D_WSTAG964_800A689C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A68F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A68FC = {
    0,
    { &D_WSTAG964_800A689C, &D_WSTAG964_800A68A8, &D_WSTAG964_800A68B4, &D_WSTAG964_800A68C0, &D_WSTAG964_800A68CC,
        &D_WSTAG964_800A68D8, &D_WSTAG964_800A68E4, &D_WSTAG964_800A68F0 },
};
FieldstgListedBattle D_WSTAG964_800A6920 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A692C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6938 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6944 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6950 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A695C = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6968 = { 64, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6974 = { 64, 11, 0x60080000 };
FieldstgBattleList D_WSTAG964_800A6980 = {
    3,
    { &D_WSTAG964_800A6920, &D_WSTAG964_800A692C, &D_WSTAG964_800A6938, &D_WSTAG964_800A6944, &D_WSTAG964_800A6950,
        &D_WSTAG964_800A695C, &D_WSTAG964_800A6968, &D_WSTAG964_800A6974 },
};
FieldstgListedBattle D_WSTAG964_800A69A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A69F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6A04 = {
    0,
    { &D_WSTAG964_800A69A4, &D_WSTAG964_800A69B0, &D_WSTAG964_800A69BC, &D_WSTAG964_800A69C8, &D_WSTAG964_800A69D4,
        &D_WSTAG964_800A69E0, &D_WSTAG964_800A69EC, &D_WSTAG964_800A69F8 },
};
FieldstgListedBattle D_WSTAG964_800A6A28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6A7C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6A88 = {
    0,
    { &D_WSTAG964_800A6A28, &D_WSTAG964_800A6A34, &D_WSTAG964_800A6A40, &D_WSTAG964_800A6A4C, &D_WSTAG964_800A6A58,
        &D_WSTAG964_800A6A64, &D_WSTAG964_800A6A70, &D_WSTAG964_800A6A7C },
};
FieldstgListedBattle D_WSTAG964_800A6AAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6AB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6AC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6AD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6ADC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6AE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6AF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6B00 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6B0C = {
    0,
    { &D_WSTAG964_800A6AAC, &D_WSTAG964_800A6AB8, &D_WSTAG964_800A6AC4, &D_WSTAG964_800A6AD0, &D_WSTAG964_800A6ADC,
        &D_WSTAG964_800A6AE8, &D_WSTAG964_800A6AF4, &D_WSTAG964_800A6B00 },
};
FieldstgListedBattle D_WSTAG964_800A6B30 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B3C = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B48 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B54 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B60 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B6C = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B78 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6B84 = { 159, 11, 0x60080000 };
FieldstgBattleList D_WSTAG964_800A6B90 = {
    5,
    { &D_WSTAG964_800A6B30, &D_WSTAG964_800A6B3C, &D_WSTAG964_800A6B48, &D_WSTAG964_800A6B54, &D_WSTAG964_800A6B60,
        &D_WSTAG964_800A6B6C, &D_WSTAG964_800A6B78, &D_WSTAG964_800A6B84 },
};
FieldstgListedBattle D_WSTAG964_800A6BB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BCC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6BFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C08 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6C14 = {
    0,
    { &D_WSTAG964_800A6BB4, &D_WSTAG964_800A6BC0, &D_WSTAG964_800A6BCC, &D_WSTAG964_800A6BD8, &D_WSTAG964_800A6BE4,
        &D_WSTAG964_800A6BF0, &D_WSTAG964_800A6BFC, &D_WSTAG964_800A6C08 },
};
FieldstgListedBattle D_WSTAG964_800A6C38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6C8C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6C98 = {
    0,
    { &D_WSTAG964_800A6C38, &D_WSTAG964_800A6C44, &D_WSTAG964_800A6C50, &D_WSTAG964_800A6C5C, &D_WSTAG964_800A6C68,
        &D_WSTAG964_800A6C74, &D_WSTAG964_800A6C80, &D_WSTAG964_800A6C8C },
};
FieldstgListedBattle D_WSTAG964_800A6CBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6CC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6CD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6CE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6CEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6CF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6D04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6D10 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6D1C = {
    0,
    { &D_WSTAG964_800A6CBC, &D_WSTAG964_800A6CC8, &D_WSTAG964_800A6CD4, &D_WSTAG964_800A6CE0, &D_WSTAG964_800A6CEC,
        &D_WSTAG964_800A6CF8, &D_WSTAG964_800A6D04, &D_WSTAG964_800A6D10 },
};
FieldstgListedBattle D_WSTAG964_800A6D40 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D4C = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D58 = { 63, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D64 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D70 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D7C = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D88 = { 104, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG964_800A6D94 = { 104, 11, 0x60080000 };
FieldstgBattleList D_WSTAG964_800A6DA0 = {
    4,
    { &D_WSTAG964_800A6D40, &D_WSTAG964_800A6D4C, &D_WSTAG964_800A6D58, &D_WSTAG964_800A6D64, &D_WSTAG964_800A6D70,
        &D_WSTAG964_800A6D7C, &D_WSTAG964_800A6D88, &D_WSTAG964_800A6D94 },
};
FieldstgListedBattle D_WSTAG964_800A6DC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6DD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6DDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6DE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6DF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E18 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6E24 = {
    0,
    { &D_WSTAG964_800A6DC4, &D_WSTAG964_800A6DD0, &D_WSTAG964_800A6DDC, &D_WSTAG964_800A6DE8, &D_WSTAG964_800A6DF4,
        &D_WSTAG964_800A6E00, &D_WSTAG964_800A6E0C, &D_WSTAG964_800A6E18 },
};
FieldstgListedBattle D_WSTAG964_800A6E48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6E9C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6EA8 = {
    0,
    { &D_WSTAG964_800A6E48, &D_WSTAG964_800A6E54, &D_WSTAG964_800A6E60, &D_WSTAG964_800A6E6C, &D_WSTAG964_800A6E78,
        &D_WSTAG964_800A6E84, &D_WSTAG964_800A6E90, &D_WSTAG964_800A6E9C },
};
FieldstgListedBattle D_WSTAG964_800A6ECC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6ED8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6EE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6EF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6EFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6F08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6F14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG964_800A6F20 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG964_800A6F2C = {
    0,
    { &D_WSTAG964_800A6ECC, &D_WSTAG964_800A6ED8, &D_WSTAG964_800A6EE4, &D_WSTAG964_800A6EF0, &D_WSTAG964_800A6EFC,
        &D_WSTAG964_800A6F08, &D_WSTAG964_800A6F14, &D_WSTAG964_800A6F20 },
};
FieldstgBattleLists wstag964_battle_lists[4] = {
    { 399, 2, 0, { &D_WSTAG964_800A6770, &D_WSTAG964_800A67F4, &D_WSTAG964_800A6878 }, &D_WSTAG964_800A68FC },
    { 402, 3, 0, { &D_WSTAG964_800A6980, &D_WSTAG964_800A6A04, &D_WSTAG964_800A6A88 }, &D_WSTAG964_800A6B0C },
    { 407, 4, 0, { &D_WSTAG964_800A6B90, &D_WSTAG964_800A6C14, &D_WSTAG964_800A6C98 }, &D_WSTAG964_800A6D1C },
    { 411, 5, 0, { &D_WSTAG964_800A6DA0, &D_WSTAG964_800A6E24, &D_WSTAG964_800A6EA8 }, &D_WSTAG964_800A6F2C },
};
