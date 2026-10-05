#include "wstag.h"

/* WSTAG966: stage 0x2E6 (fieldstg_stages_2d). */

extern WstagExits *wstag966_exits[];
extern WstagFuncs wstag966_funcs;
extern CVECTOR wstag966_color;
extern FieldstgVramPlace wstag966_vram_places[];
extern FieldstgPlacedActor *wstag966_actors[];
extern FieldstgSprite wstag966_sprites[];
extern FieldstgMapEvent wstag966_map_events[];
extern FieldstgBattleLists wstag966_battle_lists[];

s32 wstag966_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag966_set_exits(dst, list + 1, arg2, arg3);
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

void wstag966_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag966_set_exits(fieldstg_stage.map_events, wstag966_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag966_start(void *arg0) {
    WstagObject *obj = object_new(wstag966_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag966_funcs.setup();
    return obj;
}

void wstag966_setup(void) {
    fieldstg_stage.background_file = 0x6F0;
    fieldstg_stage.sprites = wstag966_sprites;
    fieldstg_stage.map_events = wstag966_map_events;
    fieldstg_stage.sprite_file = 0x093B0004;
    fieldstg_stage.mask_file = 0x93A;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13300, 0x13B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag966_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag966_actors;
    fieldstg_stage.color = wstag966_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag966_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x093B0006);
    fieldstg_attr.set_file(7, 0x093B0007);
    fieldstg_attr.set_file(4, 0x093B0005);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag966/nonmatchings/wstag966", wstag966_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag966_setup(void);

WstagExit D_WSTAG966_800A6100 = { 0x2E4, 2, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG966_800A6110 = { 0x2E0, 2, 1, 0xB0, 0x178, 5, &D_WSTAG966_800A6100 };
WstagExits D_WSTAG966_800A6120 = { 2, 1, &D_WSTAG966_800A6110 };
WstagExit D_WSTAG966_800A6128 = { 0x2E4, 2, 2, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG966_800A6138 = { 0x2E4, 2, 1, 0xC0, 0x180, 5, &D_WSTAG966_800A6128 };
WstagExits D_WSTAG966_800A6148 = { 2, 2, &D_WSTAG966_800A6138 };
WstagExit D_WSTAG966_800A6150 = { 0x2E4, 2, 4, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG966_800A6160 = { 0x2E4, 2, 3, 0xC0, 0x180, 5, &D_WSTAG966_800A6150 };
WstagExits D_WSTAG966_800A6170 = { 2, 3, &D_WSTAG966_800A6160 };
WstagExit D_WSTAG966_800A6178 = { 0x2E7, 3, 1, 0x250, 0xE8, 1, NULL };
WstagExit D_WSTAG966_800A6188 = { 0x2E5, 3, 1, 0xE0, 0x120, 5, &D_WSTAG966_800A6178 };
WstagExits D_WSTAG966_800A6198 = { 3, 1, &D_WSTAG966_800A6188 };
WstagExit D_WSTAG966_800A61A0 = { 0x2E4, 4, 1, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG966_800A61B0 = { 0x2E3, 4, 1, 0xA0, 0x180, 5, &D_WSTAG966_800A61A0 };
WstagExits D_WSTAG966_800A61C0 = { 4, 1, &D_WSTAG966_800A61B0 };
WstagExit D_WSTAG966_800A61C8 = { 0x2E4, 5, 2, 0x340, 0xA0, 1, NULL };
WstagExit D_WSTAG966_800A61D8 = { 0x2E4, 5, 1, 0xC0, 0x180, 5, &D_WSTAG966_800A61C8 };
WstagExits D_WSTAG966_800A61E8 = { 5, 1, &D_WSTAG966_800A61D8 };
WstagExits *wstag966_exits[7] = {
    &D_WSTAG966_800A6120, &D_WSTAG966_800A6148, &D_WSTAG966_800A6170, &D_WSTAG966_800A6198, &D_WSTAG966_800A61C0,
    &D_WSTAG966_800A61E8, NULL,
};
FieldstgVramPlace wstag966_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 444, 192, 188, 336, 511 },
};
FieldstgPlacedActor D_WSTAG966_800A627C = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag966_actors[2] = { &D_WSTAG966_800A627C, NULL };
FieldstgSprite wstag966_sprites[35] = {
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
FieldstgMapEvent wstag966_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E6, 0x450, 0x248, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E6, 0xA0, 0x150, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag966_funcs = { wstag966_setup };
FieldstgListedBattle D_WSTAG966_800A655C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6568 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6574 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6580 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A658C = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6598 = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A65A4 = { 106, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A65B0 = { 106, 11, 0x60080000 };
FieldstgBattleList D_WSTAG966_800A65BC = {
    3,
    { &D_WSTAG966_800A655C, &D_WSTAG966_800A6568, &D_WSTAG966_800A6574, &D_WSTAG966_800A6580, &D_WSTAG966_800A658C,
        &D_WSTAG966_800A6598, &D_WSTAG966_800A65A4, &D_WSTAG966_800A65B0 },
};
FieldstgListedBattle D_WSTAG966_800A65E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A65EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A65F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6604 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6610 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A661C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6628 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6634 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6640 = {
    0,
    { &D_WSTAG966_800A65E0, &D_WSTAG966_800A65EC, &D_WSTAG966_800A65F8, &D_WSTAG966_800A6604, &D_WSTAG966_800A6610,
        &D_WSTAG966_800A661C, &D_WSTAG966_800A6628, &D_WSTAG966_800A6634 },
};
FieldstgListedBattle D_WSTAG966_800A6664 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6670 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A667C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6688 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6694 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A66AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A66B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A66C4 = {
    0,
    { &D_WSTAG966_800A6664, &D_WSTAG966_800A6670, &D_WSTAG966_800A667C, &D_WSTAG966_800A6688, &D_WSTAG966_800A6694,
        &D_WSTAG966_800A66A0, &D_WSTAG966_800A66AC, &D_WSTAG966_800A66B8 },
};
FieldstgListedBattle D_WSTAG966_800A66E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A66F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6700 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A670C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6718 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A673C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6748 = {
    0,
    { &D_WSTAG966_800A66E8, &D_WSTAG966_800A66F4, &D_WSTAG966_800A6700, &D_WSTAG966_800A670C, &D_WSTAG966_800A6718,
        &D_WSTAG966_800A6724, &D_WSTAG966_800A6730, &D_WSTAG966_800A673C },
};
FieldstgListedBattle D_WSTAG966_800A676C = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6778 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6784 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6790 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A679C = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A67A8 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A67B4 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A67C0 = { 105, 11, 0x60080000 };
FieldstgBattleList D_WSTAG966_800A67CC = {
    5,
    { &D_WSTAG966_800A676C, &D_WSTAG966_800A6778, &D_WSTAG966_800A6784, &D_WSTAG966_800A6790, &D_WSTAG966_800A679C,
        &D_WSTAG966_800A67A8, &D_WSTAG966_800A67B4, &D_WSTAG966_800A67C0 },
};
FieldstgListedBattle D_WSTAG966_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A67FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6808 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6814 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6820 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A682C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6838 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6844 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6850 = {
    0,
    { &D_WSTAG966_800A67F0, &D_WSTAG966_800A67FC, &D_WSTAG966_800A6808, &D_WSTAG966_800A6814, &D_WSTAG966_800A6820,
        &D_WSTAG966_800A682C, &D_WSTAG966_800A6838, &D_WSTAG966_800A6844 },
};
FieldstgListedBattle D_WSTAG966_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6880 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A688C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6898 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A68B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A68BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A68C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A68D4 = {
    0,
    { &D_WSTAG966_800A6874, &D_WSTAG966_800A6880, &D_WSTAG966_800A688C, &D_WSTAG966_800A6898, &D_WSTAG966_800A68A4,
        &D_WSTAG966_800A68B0, &D_WSTAG966_800A68BC, &D_WSTAG966_800A68C8 },
};
FieldstgListedBattle D_WSTAG966_800A68F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6904 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6910 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A691C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6928 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6934 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6940 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A694C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6958 = {
    0,
    { &D_WSTAG966_800A68F8, &D_WSTAG966_800A6904, &D_WSTAG966_800A6910, &D_WSTAG966_800A691C, &D_WSTAG966_800A6928,
        &D_WSTAG966_800A6934, &D_WSTAG966_800A6940, &D_WSTAG966_800A694C },
};
FieldstgListedBattle D_WSTAG966_800A697C = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6988 = { 61, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6994 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A69A0 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A69AC = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A69B8 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A69C4 = { 159, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A69D0 = { 159, 11, 0x60080000 };
FieldstgBattleList D_WSTAG966_800A69DC = {
    5,
    { &D_WSTAG966_800A697C, &D_WSTAG966_800A6988, &D_WSTAG966_800A6994, &D_WSTAG966_800A69A0, &D_WSTAG966_800A69AC,
        &D_WSTAG966_800A69B8, &D_WSTAG966_800A69C4, &D_WSTAG966_800A69D0 },
};
FieldstgListedBattle D_WSTAG966_800A6A00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A54 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6A60 = {
    0,
    { &D_WSTAG966_800A6A00, &D_WSTAG966_800A6A0C, &D_WSTAG966_800A6A18, &D_WSTAG966_800A6A24, &D_WSTAG966_800A6A30,
        &D_WSTAG966_800A6A3C, &D_WSTAG966_800A6A48, &D_WSTAG966_800A6A54 },
};
FieldstgListedBattle D_WSTAG966_800A6A84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6A9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6AA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6AB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6AC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6ACC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6AD8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6AE4 = {
    0,
    { &D_WSTAG966_800A6A84, &D_WSTAG966_800A6A90, &D_WSTAG966_800A6A9C, &D_WSTAG966_800A6AA8, &D_WSTAG966_800A6AB4,
        &D_WSTAG966_800A6AC0, &D_WSTAG966_800A6ACC, &D_WSTAG966_800A6AD8 },
};
FieldstgListedBattle D_WSTAG966_800A6B08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6B5C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6B68 = {
    0,
    { &D_WSTAG966_800A6B08, &D_WSTAG966_800A6B14, &D_WSTAG966_800A6B20, &D_WSTAG966_800A6B2C, &D_WSTAG966_800A6B38,
        &D_WSTAG966_800A6B44, &D_WSTAG966_800A6B50, &D_WSTAG966_800A6B5C },
};
FieldstgListedBattle D_WSTAG966_800A6B8C = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6B98 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BA4 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BB0 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BBC = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BC8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BD4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG966_800A6BE0 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG966_800A6BEC = {
    4,
    { &D_WSTAG966_800A6B8C, &D_WSTAG966_800A6B98, &D_WSTAG966_800A6BA4, &D_WSTAG966_800A6BB0, &D_WSTAG966_800A6BBC,
        &D_WSTAG966_800A6BC8, &D_WSTAG966_800A6BD4, &D_WSTAG966_800A6BE0 },
};
FieldstgListedBattle D_WSTAG966_800A6C10 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6C64 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6C70 = {
    0,
    { &D_WSTAG966_800A6C10, &D_WSTAG966_800A6C1C, &D_WSTAG966_800A6C28, &D_WSTAG966_800A6C34, &D_WSTAG966_800A6C40,
        &D_WSTAG966_800A6C4C, &D_WSTAG966_800A6C58, &D_WSTAG966_800A6C64 },
};
FieldstgListedBattle D_WSTAG966_800A6C94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6CE8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6CF4 = {
    0,
    { &D_WSTAG966_800A6C94, &D_WSTAG966_800A6CA0, &D_WSTAG966_800A6CAC, &D_WSTAG966_800A6CB8, &D_WSTAG966_800A6CC4,
        &D_WSTAG966_800A6CD0, &D_WSTAG966_800A6CDC, &D_WSTAG966_800A6CE8 },
};
FieldstgListedBattle D_WSTAG966_800A6D18 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D24 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG966_800A6D6C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG966_800A6D78 = {
    0,
    { &D_WSTAG966_800A6D18, &D_WSTAG966_800A6D24, &D_WSTAG966_800A6D30, &D_WSTAG966_800A6D3C, &D_WSTAG966_800A6D48,
        &D_WSTAG966_800A6D54, &D_WSTAG966_800A6D60, &D_WSTAG966_800A6D6C },
};
FieldstgBattleLists wstag966_battle_lists[4] = {
    { 400, 2, 0, { &D_WSTAG966_800A65BC, &D_WSTAG966_800A6640, &D_WSTAG966_800A66C4 }, &D_WSTAG966_800A6748 },
    { 404, 3, 0, { &D_WSTAG966_800A67CC, &D_WSTAG966_800A6850, &D_WSTAG966_800A68D4 }, &D_WSTAG966_800A6958 },
    { 408, 4, 0, { &D_WSTAG966_800A69DC, &D_WSTAG966_800A6A60, &D_WSTAG966_800A6AE4 }, &D_WSTAG966_800A6B68 },
    { 412, 5, 0, { &D_WSTAG966_800A6BEC, &D_WSTAG966_800A6C70, &D_WSTAG966_800A6CF4 }, &D_WSTAG966_800A6D78 },
};
