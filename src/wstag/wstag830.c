#include "wstag.h"

/* WSTAG830: stage 0x2E1 (fieldstg_stages). */

extern WstagExits *wstag830_exits[];
extern WstagFuncs wstag830_funcs;
extern CVECTOR wstag830_color;
extern FieldstgBattleLists wstag830_battle_lists[];
extern FieldstgVramPlace wstag830_vram_places[];
extern FieldstgPlacedActor *wstag830_actors[];
extern FieldstgSprite wstag830_sprites[];
extern FieldstgMapEvent wstag830_map_events[];

s32 wstag830_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag830_set_exits(dst, list + 1, arg2, arg3);
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

void wstag830_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag830_set_exits(fieldstg_stage.map_events, wstag830_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag830_start(void *arg0) {
    WstagObject *obj = object_new(wstag830_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag830_funcs.setup();
    return obj;
}

void wstag830_setup(void) {
    fieldstg_stage.background_file = 0x700;
    fieldstg_stage.sprite_file = 0x07010000;
    fieldstg_stage.sprites = wstag830_sprites;
    fieldstg_stage.map_events = wstag830_map_events;
    fieldstg_stage.mask_file = 0x6FF;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDB00, 0x12300 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag830_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag830_actors;
    fieldstg_stage.color = wstag830_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag830_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x07010001);
    fieldstg_attr.set_file(7, 0x07010002);
    fieldstg_attr.set_file(4, 0x07010003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag830/nonmatchings/wstag830", wstag830_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag830_setup(void);

WstagExit D_WSTAG830_800A60F8 = { 0x2E0, 2, 1, 0xB0, 0x178, 5, NULL };
WstagExit D_WSTAG830_800A6108 = { 0x202, 0, 0, 0x420, 0x1F0, 0, &D_WSTAG830_800A60F8 };
WstagExit D_WSTAG830_800A6118 = { 0x202, 0, 0, 0x150, 0x148, 0, &D_WSTAG830_800A6108 };
WstagExits D_WSTAG830_800A6128 = { 2, 1, &D_WSTAG830_800A6118 };
WstagExit D_WSTAG830_800A6130 = { 0x2E0, 0xC, 1, 0xB0, 0x178, 5, NULL };
WstagExit D_WSTAG830_800A6140 = { 0x272, 0, 0, 0x420, 0x1F0, 0, &D_WSTAG830_800A6130 };
WstagExit D_WSTAG830_800A6150 = { 0x272, 0, 0, 0x150, 0x148, 0, &D_WSTAG830_800A6140 };
WstagExits D_WSTAG830_800A6160 = { 12, 1, &D_WSTAG830_800A6150 };
WstagExits D_WSTAG830_800A6168 = { 0, 0, &D_WSTAG830_800A6118 };
WstagExits *wstag830_exits[4] = { &D_WSTAG830_800A6128, &D_WSTAG830_800A6160, &D_WSTAG830_800A6168, NULL };
FieldstgListedBattle D_WSTAG830_800A6180 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A618C = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A6198 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A61A4 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A61B0 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A61BC = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A61C8 = { 62, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A61D4 = { 62, 11, 0x60080000 };
FieldstgBattleList D_WSTAG830_800A61E0 = {
    4,
    { &D_WSTAG830_800A6180, &D_WSTAG830_800A618C, &D_WSTAG830_800A6198, &D_WSTAG830_800A61A4, &D_WSTAG830_800A61B0,
        &D_WSTAG830_800A61BC, &D_WSTAG830_800A61C8, &D_WSTAG830_800A61D4 },
};
FieldstgListedBattle D_WSTAG830_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6210 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A621C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6258 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A6264 = {
    0,
    { &D_WSTAG830_800A6204, &D_WSTAG830_800A6210, &D_WSTAG830_800A621C, &D_WSTAG830_800A6228, &D_WSTAG830_800A6234,
        &D_WSTAG830_800A6240, &D_WSTAG830_800A624C, &D_WSTAG830_800A6258 },
};
FieldstgListedBattle D_WSTAG830_800A6288 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6294 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A62DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A62E8 = {
    0,
    { &D_WSTAG830_800A6288, &D_WSTAG830_800A6294, &D_WSTAG830_800A62A0, &D_WSTAG830_800A62AC, &D_WSTAG830_800A62B8,
        &D_WSTAG830_800A62C4, &D_WSTAG830_800A62D0, &D_WSTAG830_800A62DC },
};
FieldstgListedBattle D_WSTAG830_800A630C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6324 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6330 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A633C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6348 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6354 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6360 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A636C = {
    0,
    { &D_WSTAG830_800A630C, &D_WSTAG830_800A6318, &D_WSTAG830_800A6324, &D_WSTAG830_800A6330, &D_WSTAG830_800A633C,
        &D_WSTAG830_800A6348, &D_WSTAG830_800A6354, &D_WSTAG830_800A6360 },
};
FieldstgListedBattle D_WSTAG830_800A6390 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A639C = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63A8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63B4 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63C0 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63CC = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63D8 = { 178, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG830_800A63E4 = { 178, 11, 0x60080000 };
FieldstgBattleList D_WSTAG830_800A63F0 = {
    4,
    { &D_WSTAG830_800A6390, &D_WSTAG830_800A639C, &D_WSTAG830_800A63A8, &D_WSTAG830_800A63B4, &D_WSTAG830_800A63C0,
        &D_WSTAG830_800A63CC, &D_WSTAG830_800A63D8, &D_WSTAG830_800A63E4 },
};
FieldstgListedBattle D_WSTAG830_800A6414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A642C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6438 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6444 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6450 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A645C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6468 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A6474 = {
    0,
    { &D_WSTAG830_800A6414, &D_WSTAG830_800A6420, &D_WSTAG830_800A642C, &D_WSTAG830_800A6438, &D_WSTAG830_800A6444,
        &D_WSTAG830_800A6450, &D_WSTAG830_800A645C, &D_WSTAG830_800A6468 },
};
FieldstgListedBattle D_WSTAG830_800A6498 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A64EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A64F8 = {
    0,
    { &D_WSTAG830_800A6498, &D_WSTAG830_800A64A4, &D_WSTAG830_800A64B0, &D_WSTAG830_800A64BC, &D_WSTAG830_800A64C8,
        &D_WSTAG830_800A64D4, &D_WSTAG830_800A64E0, &D_WSTAG830_800A64EC },
};
FieldstgListedBattle D_WSTAG830_800A651C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6534 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6540 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A654C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6558 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6564 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG830_800A6570 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG830_800A657C = {
    0,
    { &D_WSTAG830_800A651C, &D_WSTAG830_800A6528, &D_WSTAG830_800A6534, &D_WSTAG830_800A6540, &D_WSTAG830_800A654C,
        &D_WSTAG830_800A6558, &D_WSTAG830_800A6564, &D_WSTAG830_800A6570 },
};
FieldstgBattleLists wstag830_battle_lists[2] = {
    { 179, 2, 0, { &D_WSTAG830_800A61E0, &D_WSTAG830_800A6264, &D_WSTAG830_800A62E8 }, &D_WSTAG830_800A636C },
    { 207, 12, 0, { &D_WSTAG830_800A63F0, &D_WSTAG830_800A6474, &D_WSTAG830_800A64F8 }, &D_WSTAG830_800A657C },
};
FieldstgVramPlace wstag830_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 416, 256, 384, 0, 352, 511 },
};
FieldstgPlacedActor D_WSTAG830_800A6648 = { NULL, NULL, 327, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag830_actors[2] = { &D_WSTAG830_800A6648, NULL };
FieldstgSprite wstag830_sprites[22] = {
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
FieldstgMapEvent wstag830_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0xE0, 0x110, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0x1D0, 0x154, 4, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E1, 0x390, 0x108, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag830_funcs = { wstag830_setup };
