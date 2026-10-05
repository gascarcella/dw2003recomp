#include "wstag.h"

/* WSTAG961: stage 0x2E1 (fieldstg_stages_2d). */

extern WstagExits *wstag961_exits[];
extern WstagFuncs wstag961_funcs;
extern CVECTOR wstag961_color;
extern FieldstgVramPlace wstag961_vram_places[];
extern FieldstgPlacedActor *wstag961_actors[];
extern FieldstgSprite wstag961_sprites[];
extern FieldstgMapEvent wstag961_map_events[];
extern FieldstgBattleLists wstag961_battle_lists[];

s32 wstag961_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag961_set_exits(dst, list + 1, arg2, arg3);
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

void wstag961_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag961_set_exits(fieldstg_stage.map_events, wstag961_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag961_start(void *arg0) {
    WstagObject *obj = object_new(wstag961_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag961_funcs.setup();
    return obj;
}

void wstag961_setup(void) {
    fieldstg_stage.background_file = 0x700;
    fieldstg_stage.sprites = wstag961_sprites;
    fieldstg_stage.map_events = wstag961_map_events;
    fieldstg_stage.sprite_file = 0x09310004;
    fieldstg_stage.mask_file = 0x930;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDB00, 0x12300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag961_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag961_actors;
    fieldstg_stage.color = wstag961_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag961_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09310006);
    fieldstg_attr.set_file(7, 0x09310007);
    fieldstg_attr.set_file(4, 0x09310005);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag961/nonmatchings/wstag961", wstag961_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag961_setup(void);

WstagExit D_WSTAG961_800A60FC = { 0x2E3, 1, 1, 0xA0, 0x180, 5, NULL };
WstagExit D_WSTAG961_800A610C = { 0x272, 0, 0, 0x420, 0x1F0, 0, &D_WSTAG961_800A60FC };
WstagExit D_WSTAG961_800A611C = { 0x272, 0, 0, 0x150, 0x148, 0, &D_WSTAG961_800A610C };
WstagExits D_WSTAG961_800A612C = { 1, 1, &D_WSTAG961_800A611C };
WstagExits *wstag961_exits[2] = { &D_WSTAG961_800A612C, NULL };
FieldstgVramPlace wstag961_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 416, 256, 384, 0, 352, 511 },
};
FieldstgPlacedActor D_WSTAG961_800A61AC = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag961_actors[2] = { &D_WSTAG961_800A61AC, NULL };
FieldstgSprite wstag961_sprites[22] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 550, 238, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 239, 158, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 661, 204, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 896, 119, 0, 0 },
    { 1, 0, 0xFF, 2, 0x33, 2, 0, 7, 0x18, 0, 188, -92, 0, 0 },
    { 1, 0, 0xFF, 2, 0x33, 2, 0, 7, 0x18, 0, 428, -20, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 128, 276, 0, 0 },
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 192, 297, 0, 0 }, { 1, 0, 0x52, 2, 2, 0, 0, 0, 0, 0, 256, 292, 0, 0 },
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 320, 320, 0, 0 }, { 1, 0, 0x48, 2, 4, 0, 0, 0, 0, 0, 768, 312, 0, 0 },
    { 1, 0, 0x54, 2, 5, 0, 0, 0, 0, 0, 832, 270, 0, 0 }, { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 512, 335, 0, 0 },
    { 1, 0, 0x44, 2, 7, 0, 0, 0, 0, 0, 576, 316, 0, 0 }, { 1, 0, 0x40, 2, 8, 0, 0, 0, 0, 0, 640, 337, 0, 0 },
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 704, 338, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 460, 182, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 348, 194, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 761, 156, 0, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 188, 36, 280, 0 },
    { 1, 0, 0xFF, 4, 0x32, 2, 0, 7, 0x18, 0, 428, 108, 352, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag961_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0xE0, 0x110, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0x1D0, 0x154, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0x390, 0x108, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag961_funcs = { wstag961_setup };
FieldstgListedBattle D_WSTAG961_800A63B8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A63C4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A63D0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A63DC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A63E8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A63F4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A6400 = { 103, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG961_800A640C = { 103, 11, 0x60080000 };
FieldstgBattleList D_WSTAG961_800A6418 = {
    5,
    { &D_WSTAG961_800A63B8, &D_WSTAG961_800A63C4, &D_WSTAG961_800A63D0, &D_WSTAG961_800A63DC, &D_WSTAG961_800A63E8,
        &D_WSTAG961_800A63F4, &D_WSTAG961_800A6400, &D_WSTAG961_800A640C },
};
FieldstgListedBattle D_WSTAG961_800A643C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6448 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6454 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6490 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG961_800A649C = {
    0,
    { &D_WSTAG961_800A643C, &D_WSTAG961_800A6448, &D_WSTAG961_800A6454, &D_WSTAG961_800A6460, &D_WSTAG961_800A646C,
        &D_WSTAG961_800A6478, &D_WSTAG961_800A6484, &D_WSTAG961_800A6490 },
};
FieldstgListedBattle D_WSTAG961_800A64C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A64CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A64D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A64E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A64F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A64FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6508 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6514 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG961_800A6520 = {
    0,
    { &D_WSTAG961_800A64C0, &D_WSTAG961_800A64CC, &D_WSTAG961_800A64D8, &D_WSTAG961_800A64E4, &D_WSTAG961_800A64F0,
        &D_WSTAG961_800A64FC, &D_WSTAG961_800A6508, &D_WSTAG961_800A6514 },
};
FieldstgListedBattle D_WSTAG961_800A6544 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6550 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A655C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6568 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6574 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG961_800A6598 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG961_800A65A4 = {
    0,
    { &D_WSTAG961_800A6544, &D_WSTAG961_800A6550, &D_WSTAG961_800A655C, &D_WSTAG961_800A6568, &D_WSTAG961_800A6574,
        &D_WSTAG961_800A6580, &D_WSTAG961_800A658C, &D_WSTAG961_800A6598 },
};
FieldstgBattleLists wstag961_battle_lists[1] = {
    { 395, 1, 0, { &D_WSTAG961_800A6418, &D_WSTAG961_800A649C, &D_WSTAG961_800A6520 }, &D_WSTAG961_800A65A4 },
};
