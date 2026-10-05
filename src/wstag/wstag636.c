#include "wstag.h"

/* WSTAG636: stage 0x2C1 (fieldstg_stages). */

extern WstagExits *wstag636_exits[];
extern WstagFuncs wstag636_funcs;
extern FieldstgBattleLists wstag636_battle_lists;
extern FieldstgVramPlace wstag636_vram_places[];
extern FieldstgPlacedActor *wstag636_actors[];
extern FieldstgSprite wstag636_sprites[];
extern FieldstgMapEvent wstag636_map_events[];

s32 wstag636_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag636_set_exits(dst, list + 1, arg2, arg3);
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

void wstag636_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag636_set_exits(fieldstg_stage.map_events, wstag636_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag636_start(void *arg0) {
    WstagObject *obj = object_new(wstag636_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag636_funcs.setup();
    return obj;
}

void wstag636_setup(void) {
    fieldstg_stage.background_file = 0x636;
    fieldstg_stage.sprite_file = 0x06370000;
    fieldstg_stage.sprites = wstag636_sprites;
    fieldstg_stage.map_events = wstag636_map_events;
    fieldstg_stage.mask_file = 0x635;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11F00, 0x2F100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag636_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag636_actors;
    fieldstg_stage.battle_lists = &wstag636_battle_lists;
    fieldstg_attr.set_file(0, 0x06370001);
    fieldstg_attr.set_file(7, 0x06370002);
    fieldstg_attr.set_file(4, 0x06370003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag636_setup(void);

WstagExit D_WSTAG636_800A60C0 = { 0x2C2, 1, 2, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A60D0 = { 0x2C2, 1, 3, 0x358, 0xFC, 1, &D_WSTAG636_800A60C0 };
WstagExit D_WSTAG636_800A60E0 = { 0x2C2, 1, 1, 0x120, 0x100, 7, &D_WSTAG636_800A60D0 };
WstagExit D_WSTAG636_800A60F0 = { 0x2C0, 0, 0, 0x100, 0x278, 5, &D_WSTAG636_800A60E0 };
WstagExits D_WSTAG636_800A6100 = { 1, 1, &D_WSTAG636_800A60F0 };
WstagExit D_WSTAG636_800A6108 = { 0x2C2, 1, 1, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A6118 = { 0x2C2, 1, 4, 0x358, 0xFC, 1, &D_WSTAG636_800A6108 };
WstagExit D_WSTAG636_800A6128 = { 0x2C2, 1, 2, 0x120, 0x100, 7, &D_WSTAG636_800A6118 };
WstagExit D_WSTAG636_800A6138 = { 0x2C0, 0, 0, 0x100, 0x278, 5, &D_WSTAG636_800A6128 };
WstagExits D_WSTAG636_800A6148 = { 1, 2, &D_WSTAG636_800A6138 };
WstagExit D_WSTAG636_800A6150 = { 0x2C2, 1, 3, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A6160 = { 0x2C2, 1, 5, 0x358, 0xFC, 1, &D_WSTAG636_800A6150 };
WstagExit D_WSTAG636_800A6170 = { 0x2C2, 1, 4, 0x120, 0x100, 7, &D_WSTAG636_800A6160 };
WstagExit D_WSTAG636_800A6180 = { 0x2C2, 1, 1, 0x118, 0x2EC, 5, &D_WSTAG636_800A6170 };
WstagExits D_WSTAG636_800A6190 = { 1, 3, &D_WSTAG636_800A6180 };
WstagExit D_WSTAG636_800A6198 = { 0x2C2, 1, 4, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A61A8 = { 0x2C2, 1, 6, 0x358, 0xFC, 1, &D_WSTAG636_800A6198 };
WstagExit D_WSTAG636_800A61B8 = { 0x2C2, 1, 3, 0x120, 0x100, 7, &D_WSTAG636_800A61A8 };
WstagExit D_WSTAG636_800A61C8 = { 0x2C2, 1, 2, 0x118, 0x2EC, 5, &D_WSTAG636_800A61B8 };
WstagExits D_WSTAG636_800A61D8 = { 1, 4, &D_WSTAG636_800A61C8 };
WstagExit D_WSTAG636_800A61E0 = { 0x2C2, 1, 6, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A61F0 = { 0x2C2, 1, 7, 0x358, 0xFC, 1, &D_WSTAG636_800A61E0 };
WstagExit D_WSTAG636_800A6200 = { 0x2C2, 1, 5, 0x120, 0x100, 7, &D_WSTAG636_800A61F0 };
WstagExit D_WSTAG636_800A6210 = { 0x2C2, 1, 3, 0x118, 0x2EC, 5, &D_WSTAG636_800A6200 };
WstagExits D_WSTAG636_800A6220 = { 1, 5, &D_WSTAG636_800A6210 };
WstagExit D_WSTAG636_800A6228 = { 0x2C3, 0, 0, 0x2F8, 0x244, 3, NULL };
WstagExit D_WSTAG636_800A6238 = { 0x2C2, 1, 8, 0x358, 0xFC, 1, &D_WSTAG636_800A6228 };
WstagExit D_WSTAG636_800A6248 = { 0x2C2, 1, 6, 0x120, 0x100, 7, &D_WSTAG636_800A6238 };
WstagExit D_WSTAG636_800A6258 = { 0x2C2, 1, 4, 0x118, 0x2EC, 5, &D_WSTAG636_800A6248 };
WstagExits D_WSTAG636_800A6268 = { 1, 6, &D_WSTAG636_800A6258 };
WstagExit D_WSTAG636_800A6270 = { 0x2C2, 1, 7, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A6280 = { 0x2C2, 1, 1, 0x358, 0xFC, 1, &D_WSTAG636_800A6270 };
WstagExit D_WSTAG636_800A6290 = { 0x2C2, 1, 8, 0x120, 0x100, 7, &D_WSTAG636_800A6280 };
WstagExit D_WSTAG636_800A62A0 = { 0x2C2, 1, 5, 0x118, 0x2EC, 5, &D_WSTAG636_800A6290 };
WstagExits D_WSTAG636_800A62B0 = { 1, 7, &D_WSTAG636_800A62A0 };
WstagExit D_WSTAG636_800A62B8 = { 0x2C2, 1, 8, 0x350, 0x2F8, 3, NULL };
WstagExit D_WSTAG636_800A62C8 = { 0x2C2, 1, 2, 0x358, 0xFC, 1, &D_WSTAG636_800A62B8 };
WstagExit D_WSTAG636_800A62D8 = { 0x2C2, 1, 7, 0x120, 0x100, 7, &D_WSTAG636_800A62C8 };
WstagExit D_WSTAG636_800A62E8 = { 0x2C2, 1, 6, 0x118, 0x2EC, 5, &D_WSTAG636_800A62D8 };
WstagExits D_WSTAG636_800A62F8 = { 1, 8, &D_WSTAG636_800A62E8 };
WstagExits D_WSTAG636_800A6300 = { 0, 0, &D_WSTAG636_800A60F0 };
WstagExits *wstag636_exits[10] = {
    &D_WSTAG636_800A6100, &D_WSTAG636_800A6148, &D_WSTAG636_800A6190, &D_WSTAG636_800A61D8, &D_WSTAG636_800A6220,
    &D_WSTAG636_800A6268, &D_WSTAG636_800A62B0, &D_WSTAG636_800A62F8, &D_WSTAG636_800A6300, NULL,
};
FieldstgListedBattle D_WSTAG636_800A6330 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A633C = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A6348 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A6354 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A6360 = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A636C = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A6378 = { 162, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG636_800A6384 = { 162, 5, 0x60080000 };
FieldstgBattleList D_WSTAG636_800A6390 = {
    3,
    { &D_WSTAG636_800A6330, &D_WSTAG636_800A633C, &D_WSTAG636_800A6348, &D_WSTAG636_800A6354, &D_WSTAG636_800A6360,
        &D_WSTAG636_800A636C, &D_WSTAG636_800A6378, &D_WSTAG636_800A6384 },
};
FieldstgListedBattle D_WSTAG636_800A63B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A63FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6408 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG636_800A6414 = {
    0,
    { &D_WSTAG636_800A63B4, &D_WSTAG636_800A63C0, &D_WSTAG636_800A63CC, &D_WSTAG636_800A63D8, &D_WSTAG636_800A63E4,
        &D_WSTAG636_800A63F0, &D_WSTAG636_800A63FC, &D_WSTAG636_800A6408 },
};
FieldstgListedBattle D_WSTAG636_800A6438 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6444 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6450 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A645C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6468 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6474 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6480 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A648C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG636_800A6498 = {
    0,
    { &D_WSTAG636_800A6438, &D_WSTAG636_800A6444, &D_WSTAG636_800A6450, &D_WSTAG636_800A645C, &D_WSTAG636_800A6468,
        &D_WSTAG636_800A6474, &D_WSTAG636_800A6480, &D_WSTAG636_800A648C },
};
FieldstgListedBattle D_WSTAG636_800A64BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A64C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A64D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A64E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A64EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A64F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG636_800A6510 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG636_800A651C = {
    0,
    { &D_WSTAG636_800A64BC, &D_WSTAG636_800A64C8, &D_WSTAG636_800A64D4, &D_WSTAG636_800A64E0, &D_WSTAG636_800A64EC,
        &D_WSTAG636_800A64F8, &D_WSTAG636_800A6504, &D_WSTAG636_800A6510 },
};
FieldstgBattleLists wstag636_battle_lists = {
    102, 0, 0, { &D_WSTAG636_800A6390, &D_WSTAG636_800A6414, &D_WSTAG636_800A6498 }, &D_WSTAG636_800A651C,
};
FieldstgVramPlace wstag636_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 416, 136, 160, 320, 511 }, { 320, 256, 344, 416, 96, 160, 336, 511 },
    { 320, 256, 354, 448, 136, 192, 352, 511 },
};
u16 D_WSTAG636_800A65EC[4] = { 0x8682, 1, 0xFFFF, 0 };
u16 D_WSTAG636_800A65F4[6] = { 0x8682, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG636_800A6600[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG636_800A6608[8] = { 0x8682, 0, 0, 1, 0x847E, 0, 0xFFFF, 0 };
u16 D_WSTAG636_800A6618[8] = { 0, 1, 0x847E, 1, 0x8682, 0, 0xFFFF, 0 };
u16 D_WSTAG636_800A6628[10] = {
    0x8682, 1, 0x8681, 0, 0x847E, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG636_800A663C[2] = { { NULL, NULL, 969 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG636_800A6654[5] = {
    { D_WSTAG636_800A65EC, NULL, 848 }, { D_WSTAG636_800A65F4, D_WSTAG636_800A6600, 849 },
    { D_WSTAG636_800A6608, NULL, 850 }, { D_WSTAG636_800A6618, D_WSTAG636_800A6628, 851 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG636_800A6690[2] = { { NULL, NULL, 970 }, { NULL, NULL, 0 } };
u16 D_WSTAG636_800A66A8[6] = { 0x7E00, 1, 0x7E1E, 1, 0xFFFF, 0 };
u16 D_WSTAG636_800A66B4[14] = {
    0x7043, 1, 0x704B, 1, 0x8681, 1, 0x8682, 0,
    0x7E00, 1, 0x7E1F, 1, 0xFFFF, 0,
};
u16 D_WSTAG636_800A66D0[6] = { 0x7E00, 1, 0x7E23, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG636_800A66DC = { D_WSTAG636_800A66A8, D_WSTAG636_800A663C, 64, 4, 408, 364, 7 };
FieldstgPlacedActor D_WSTAG636_800A66F0 = { D_WSTAG636_800A66B4, D_WSTAG636_800A6654, 167, 5, 480, 696, 7 };
FieldstgPlacedActor D_WSTAG636_800A6704 = { D_WSTAG636_800A66D0, D_WSTAG636_800A6690, 181, 6, 480, 696, 7 };
FieldstgPlacedActor *wstag636_actors[4] = {
    &D_WSTAG636_800A66DC, &D_WSTAG636_800A66F0, &D_WSTAG636_800A6704, NULL,
};
FieldstgSprite wstag636_sprites[10] = {
    { 1, 0, 0x40, 2, 7, 0, 0, 0, 0, 0, 568, 143, 0, 0 }, { 1, 0, 0x40, 2, 7, 0, 0, 0, 0, 0, 904, 503, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 417, 278, 329, 0 }, { 1, 0, 0x48, 4, 1, 0, 0, 0, 0, 0, 348, 278, 342, 0 },
    { 1, 0, 0x4A, 4, 2, 0, 0, 0, 0, 0, 529, 611, 680, 0 }, { 1, 0, 0x5A, 4, 3, 0, 0, 0, 0, 0, 422, 570, 656, 0 },
    { 1, 0, 0x58, 4, 4, 0, 0, 0, 0, 0, 384, 635, 719, 0 }, { 1, 0, 0x64, 4, 5, 0, 0, 0, 0, 0, 918, 517, 608, 0 },
    { 1, 0, 0x64, 4, 6, 0, 0, 0, 0, 0, 582, 156, 248, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag636_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x340, 0xF0, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x368, 0x2EC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x128, 0x2F4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x110, 0x108, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag636_funcs = { wstag636_setup };
