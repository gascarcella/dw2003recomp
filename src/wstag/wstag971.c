#include "wstag.h"

/* WSTAG971: stage 0x2EB (fieldstg_stages_2d). */

extern WstagExits *wstag971_exits[];
extern WstagFuncs wstag971_funcs;
extern FieldstgVramPlace wstag971_vram_places[];
extern FieldstgPlacedActor *wstag971_actors[];
extern FieldstgSprite wstag971_sprites[];
extern FieldstgMapEvent wstag971_map_events[];
extern FieldstgBattleLists wstag971_battle_lists[];

s32 wstag971_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag971_set_exits(dst, list + 1, arg2, arg3);
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

void wstag971_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag971_set_exits(fieldstg_stage.map_events, wstag971_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag971_start(void *arg0) {
    WstagObject *obj = object_new(wstag971_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag971_funcs.setup();
    return obj;
}

void wstag971_setup(void) {
    fieldstg_stage.background_file = 0x68F;
    fieldstg_stage.sprites = wstag971_sprites;
    fieldstg_stage.map_events = wstag971_map_events;
    fieldstg_stage.sprite_file = 0x09450004;
    fieldstg_stage.mask_file = 0x944;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xEB00, 0x1F500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag971_vram_places;
    fieldstg_stage.music = 0x1E;
    fieldstg_stage.sound = 0x60780000;
    fieldstg_stage.actors = wstag971_actors;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag971_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x09450006);
    fieldstg_attr.set_file(7, 0x09450007);
    fieldstg_attr.set_file(4, 0x09450005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag971_setup(void);

WstagExit D_WSTAG971_800A60D8 = { 0x2EE, 1, 1, 0xE0, 0x240, 5, NULL };
WstagExits D_WSTAG971_800A60E8 = { 1, 1, &D_WSTAG971_800A60D8 };
WstagExit D_WSTAG971_800A60F0 = { 0x2ED, 1, 1, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A6100 = { 1, 2, &D_WSTAG971_800A60F0 };
WstagExit D_WSTAG971_800A6108 = { 0x2ED, 1, 2, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A6118 = { 1, 3, &D_WSTAG971_800A6108 };
WstagExit D_WSTAG971_800A6120 = { 0x2EE, 1, 2, 0xE0, 0x240, 5, NULL };
WstagExits D_WSTAG971_800A6130 = { 1, 4, &D_WSTAG971_800A6120 };
WstagExit D_WSTAG971_800A6138 = { 0x2ED, 1, 5, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A6148 = { 1, 5, &D_WSTAG971_800A6138 };
WstagExit D_WSTAG971_800A6150 = { 0x2EE, 1, 4, 0xE0, 0x240, 5, NULL };
WstagExits D_WSTAG971_800A6160 = { 1, 6, &D_WSTAG971_800A6150 };
WstagExit D_WSTAG971_800A6168 = { 0x2ED, 5, 4, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A6178 = { 5, 1, &D_WSTAG971_800A6168 };
WstagExit D_WSTAG971_800A6180 = { 0x2ED, 5, 5, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A6190 = { 5, 2, &D_WSTAG971_800A6180 };
WstagExit D_WSTAG971_800A6198 = { 0x2ED, 5, 6, 0x350, 0x1F8, 5, NULL };
WstagExits D_WSTAG971_800A61A8 = { 5, 3, &D_WSTAG971_800A6198 };
WstagExit D_WSTAG971_800A61B0 = { 0x2ED, 6, 4, 0xE0, 0xC0, 5, NULL };
WstagExits D_WSTAG971_800A61C0 = { 6, 1, &D_WSTAG971_800A61B0 };
WstagExits *wstag971_exits[11] = {
    &D_WSTAG971_800A60E8, &D_WSTAG971_800A6100, &D_WSTAG971_800A6118, &D_WSTAG971_800A6130, &D_WSTAG971_800A6148,
    &D_WSTAG971_800A6160, &D_WSTAG971_800A6178, &D_WSTAG971_800A6190, &D_WSTAG971_800A61A8, &D_WSTAG971_800A61C0,
    NULL,
};
FieldstgVramPlace wstag971_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 350, 320, 120, 64, 320, 511 }, { 320, 256, 320, 256, 0, 0, 336, 511 },
    { 320, 256, 338, 320, 72, 64, 352, 511 }, { 320, 256, 320, 320, 0, 64, 368, 511 },
    { 320, 256, 362, 320, 168, 64, 320, 510 }, { 320, 256, 340, 256, 80, 0, 336, 510 },
    { 320, 256, 360, 256, 160, 0, 352, 510 },
};
u16 D_WSTAG971_800A62C4[4] = { 0x8168, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A62CC[4] = { 0x7038, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A62D4[8] = { 0x8168, 0, 0, 0, 0xA1A, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A62E4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A62EC[8] = { 0x8168, 0, 0, 1, 0xA1A, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A62FC[6] = { 0x7403, 1, 0xA1A, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6308[8] = { 0x8168, 0, 0xA1A, 1, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6318[6] = { 0x8168, 1, 0x7038, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6324[8] = { 0x8168, 0, 0, 1, 0xA1A, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6334[6] = { 0x8168, 1, 0x7038, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6340[4] = { 0x8006, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6348[4] = { 0x7031, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6350[8] = { 0x8006, 0, 0, 0, 0xA14, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6360[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6368[8] = { 0x8006, 0, 0, 1, 0xA14, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6378[6] = { 0x7400, 1, 0xA14, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6384[8] = { 0, 0, 0xA14, 1, 0x8006, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6394[6] = { 0x7031, 1, 0x8006, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63A0[8] = { 0x8006, 0, 0, 1, 0xA14, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63B0[6] = { 0x8006, 1, 0x7031, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63BC[4] = { 0x8026, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63C4[4] = { 0x7035, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63CC[8] = { 0x8026, 0, 0, 0, 0xA17, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A63DC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A63E4[8] = { 0x8026, 0, 0, 1, 0xA17, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A63F4[6] = { 0x7402, 1, 0xA17, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6400[8] = { 0, 0, 0xA17, 1, 0x8026, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6410[6] = { 0x7035, 1, 0x8026, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A641C[8] = { 0x8026, 0, 0, 1, 0xA17, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A642C[6] = { 0x8026, 1, 0x7035, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6438[4] = { 0x8022, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6440[4] = { 0x7033, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6448[8] = { 0x8022, 0, 0, 0, 0xA15, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6458[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6460[8] = { 0x8022, 0, 0, 1, 0xA15, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6470[6] = { 0x7401, 1, 0xA15, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A647C[8] = { 0, 0, 0xA15, 1, 0x8022, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A648C[6] = { 0x7033, 1, 0x8022, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A6498[8] = { 0x8022, 0, 0, 1, 0xA15, 1, 0xFFFF, 0 };
u16 D_WSTAG971_800A64A8[6] = { 0x8022, 1, 0x7033, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG971_800A64B4[6] = {
    { D_WSTAG971_800A62C4, D_WSTAG971_800A62CC, 233 }, { D_WSTAG971_800A62D4, D_WSTAG971_800A62E4, 234 },
    { D_WSTAG971_800A62EC, D_WSTAG971_800A62FC, 235 }, { D_WSTAG971_800A6308, D_WSTAG971_800A6318, 236 },
    { D_WSTAG971_800A6324, D_WSTAG971_800A6334, 236 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG971_800A64FC[6] = {
    { D_WSTAG971_800A6340, D_WSTAG971_800A6348, 209 }, { D_WSTAG971_800A6350, D_WSTAG971_800A6360, 210 },
    { D_WSTAG971_800A6368, D_WSTAG971_800A6378, 211 }, { D_WSTAG971_800A6384, D_WSTAG971_800A6394, 212 },
    { D_WSTAG971_800A63A0, D_WSTAG971_800A63B0, 212 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG971_800A6544[6] = {
    { D_WSTAG971_800A63BC, D_WSTAG971_800A63C4, 221 }, { D_WSTAG971_800A63CC, D_WSTAG971_800A63DC, 222 },
    { D_WSTAG971_800A63E4, D_WSTAG971_800A63F4, 223 }, { D_WSTAG971_800A6400, D_WSTAG971_800A6410, 224 },
    { D_WSTAG971_800A641C, D_WSTAG971_800A642C, 224 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG971_800A658C[6] = {
    { D_WSTAG971_800A6438, D_WSTAG971_800A6440, 213 }, { D_WSTAG971_800A6448, D_WSTAG971_800A6458, 214 },
    { D_WSTAG971_800A6460, D_WSTAG971_800A6470, 215 }, { D_WSTAG971_800A647C, D_WSTAG971_800A648C, 216 },
    { D_WSTAG971_800A6498, D_WSTAG971_800A64A8, 216 }, { NULL, NULL, 0 },
};
u16 D_WSTAG971_800A65D4[10] = {
    0x7E05, 1, 0x7E1E, 1, 0x7095, 1, 0x7011, 0,
    0xFFFF, 0,
};
u16 D_WSTAG971_800A65E8[10] = {
    0x7095, 1, 0x700B, 0, 0x7E04, 1, 0x7E1F, 1,
    0xFFFF, 0,
};
u16 D_WSTAG971_800A65FC[10] = {
    0x7E04, 1, 0x7E20, 1, 0x7095, 1, 0x700F, 0,
    0xFFFF, 0,
};
u16 D_WSTAG971_800A6610[10] = {
    0x7E04, 1, 0x7E1E, 1, 0x7095, 1, 0x700C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG971_800A6624[6] = { 0x7E00, 1, 8, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6630[6] = { 0x7E04, 1, 8, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A663C[6] = { 0x7E1E, 1, 9, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6648[6] = { 0x7E1F, 1, 9, 0, 0xFFFF, 0 };
u16 D_WSTAG971_800A6654[6] = { 9, 0, 0x7E20, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG971_800A6660 = { D_WSTAG971_800A65D4, D_WSTAG971_800A64B4, 100, 4, 240, 600, 1 };
FieldstgPlacedActor D_WSTAG971_800A6674 = { D_WSTAG971_800A65E8, D_WSTAG971_800A64FC, 123, 5, 240, 600, 1 };
FieldstgPlacedActor D_WSTAG971_800A6688 = { D_WSTAG971_800A65FC, D_WSTAG971_800A6544, 125, 6, 240, 600, 1 };
FieldstgPlacedActor D_WSTAG971_800A669C = { D_WSTAG971_800A6610, D_WSTAG971_800A658C, 126, 7, 240, 600, 1 };
FieldstgPlacedActor D_WSTAG971_800A66B0 = { NULL, NULL, 326, 8, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG971_800A66C4 = { D_WSTAG971_800A6624, NULL, 328, 9, 480, 208, 1 };
FieldstgPlacedActor D_WSTAG971_800A66D8 = { D_WSTAG971_800A6630, NULL, 328, 9, 400, 344, 1 };
FieldstgPlacedActor D_WSTAG971_800A66EC = { D_WSTAG971_800A663C, NULL, 351, 10, 288, 400, 1 };
FieldstgPlacedActor D_WSTAG971_800A6700 = { D_WSTAG971_800A6648, NULL, 351, 10, 240, 488, 1 };
FieldstgPlacedActor D_WSTAG971_800A6714 = { D_WSTAG971_800A6654, NULL, 351, 10, 352, 272, 1 };
FieldstgPlacedActor *wstag971_actors[11] = {
    &D_WSTAG971_800A6660, &D_WSTAG971_800A6674, &D_WSTAG971_800A6688, &D_WSTAG971_800A669C, &D_WSTAG971_800A66B0,
    &D_WSTAG971_800A66C4, &D_WSTAG971_800A66D8, &D_WSTAG971_800A66EC, &D_WSTAG971_800A6700, &D_WSTAG971_800A6714,
    NULL,
};
FieldstgSprite wstag971_sprites[1] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
FieldstgMapEvent wstag971_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2EB, 0x240, 0xA0, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag971_funcs = { wstag971_setup };
FieldstgListedBattle D_WSTAG971_800A679C = { 140, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67A8 = { 140, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67B4 = { 140, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67C0 = { 140, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67CC = { 141, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67D8 = { 141, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67E4 = { 141, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A67F0 = { 141, 10, 0x60080000 };
FieldstgBattleList D_WSTAG971_800A67FC = {
    3,
    { &D_WSTAG971_800A679C, &D_WSTAG971_800A67A8, &D_WSTAG971_800A67B4, &D_WSTAG971_800A67C0, &D_WSTAG971_800A67CC,
        &D_WSTAG971_800A67D8, &D_WSTAG971_800A67E4, &D_WSTAG971_800A67F0 },
};
FieldstgListedBattle D_WSTAG971_800A6820 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A682C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6838 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6844 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6850 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A685C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6874 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6880 = {
    0,
    { &D_WSTAG971_800A6820, &D_WSTAG971_800A682C, &D_WSTAG971_800A6838, &D_WSTAG971_800A6844, &D_WSTAG971_800A6850,
        &D_WSTAG971_800A685C, &D_WSTAG971_800A6868, &D_WSTAG971_800A6874 },
};
FieldstgListedBattle D_WSTAG971_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A68F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6904 = {
    0,
    { &D_WSTAG971_800A68A4, &D_WSTAG971_800A68B0, &D_WSTAG971_800A68BC, &D_WSTAG971_800A68C8, &D_WSTAG971_800A68D4,
        &D_WSTAG971_800A68E0, &D_WSTAG971_800A68EC, &D_WSTAG971_800A68F8 },
};
FieldstgListedBattle D_WSTAG971_800A6928 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6934 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6940 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A694C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6958 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6964 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6970 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A697C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6988 = {
    0,
    { &D_WSTAG971_800A6928, &D_WSTAG971_800A6934, &D_WSTAG971_800A6940, &D_WSTAG971_800A694C, &D_WSTAG971_800A6958,
        &D_WSTAG971_800A6964, &D_WSTAG971_800A6970, &D_WSTAG971_800A697C },
};
FieldstgListedBattle D_WSTAG971_800A69AC = { 116, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69B8 = { 116, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69C4 = { 164, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69D0 = { 164, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69DC = { 139, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69E8 = { 139, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A69F4 = { 163, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6A00 = { 163, 10, 0x60080000 };
FieldstgBattleList D_WSTAG971_800A6A0C = {
    3,
    { &D_WSTAG971_800A69AC, &D_WSTAG971_800A69B8, &D_WSTAG971_800A69C4, &D_WSTAG971_800A69D0, &D_WSTAG971_800A69DC,
        &D_WSTAG971_800A69E8, &D_WSTAG971_800A69F4, &D_WSTAG971_800A6A00 },
};
FieldstgListedBattle D_WSTAG971_800A6A30 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A3C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A48 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6A84 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6A90 = {
    0,
    { &D_WSTAG971_800A6A30, &D_WSTAG971_800A6A3C, &D_WSTAG971_800A6A48, &D_WSTAG971_800A6A54, &D_WSTAG971_800A6A60,
        &D_WSTAG971_800A6A6C, &D_WSTAG971_800A6A78, &D_WSTAG971_800A6A84 },
};
FieldstgListedBattle D_WSTAG971_800A6AB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6AC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6ACC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6AD8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6AE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6AF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6AFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6B08 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6B14 = {
    0,
    { &D_WSTAG971_800A6AB4, &D_WSTAG971_800A6AC0, &D_WSTAG971_800A6ACC, &D_WSTAG971_800A6AD8, &D_WSTAG971_800A6AE4,
        &D_WSTAG971_800A6AF0, &D_WSTAG971_800A6AFC, &D_WSTAG971_800A6B08 },
};
FieldstgListedBattle D_WSTAG971_800A6B38 = { 301, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6B44 = { 302, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6B50 = { 304, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6B5C = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6B68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6B74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6B80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6B8C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6B98 = {
    0,
    { &D_WSTAG971_800A6B38, &D_WSTAG971_800A6B44, &D_WSTAG971_800A6B50, &D_WSTAG971_800A6B5C, &D_WSTAG971_800A6B68,
        &D_WSTAG971_800A6B74, &D_WSTAG971_800A6B80, &D_WSTAG971_800A6B8C },
};
FieldstgListedBattle D_WSTAG971_800A6BBC = { 162, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6BC8 = { 162, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6BD4 = { 162, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6BE0 = { 162, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6BEC = { 135, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6BF8 = { 135, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6C04 = { 135, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG971_800A6C10 = { 135, 10, 0x60080000 };
FieldstgBattleList D_WSTAG971_800A6C1C = {
    3,
    { &D_WSTAG971_800A6BBC, &D_WSTAG971_800A6BC8, &D_WSTAG971_800A6BD4, &D_WSTAG971_800A6BE0, &D_WSTAG971_800A6BEC,
        &D_WSTAG971_800A6BF8, &D_WSTAG971_800A6C04, &D_WSTAG971_800A6C10 },
};
FieldstgListedBattle D_WSTAG971_800A6C40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6C94 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6CA0 = {
    0,
    { &D_WSTAG971_800A6C40, &D_WSTAG971_800A6C4C, &D_WSTAG971_800A6C58, &D_WSTAG971_800A6C64, &D_WSTAG971_800A6C70,
        &D_WSTAG971_800A6C7C, &D_WSTAG971_800A6C88, &D_WSTAG971_800A6C94 },
};
FieldstgListedBattle D_WSTAG971_800A6CC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6CD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6CDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6CE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6CF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6D00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6D0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG971_800A6D18 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG971_800A6D24 = {
    0,
    { &D_WSTAG971_800A6CC4, &D_WSTAG971_800A6CD0, &D_WSTAG971_800A6CDC, &D_WSTAG971_800A6CE8, &D_WSTAG971_800A6CF4,
        &D_WSTAG971_800A6D00, &D_WSTAG971_800A6D0C, &D_WSTAG971_800A6D18 },
};
FieldstgListedBattle D_WSTAG971_800A6D48 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D54 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D60 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D6C = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D78 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D84 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D90 = { 311, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG971_800A6D9C = { 311, 10, 0x60880000 };
FieldstgBattleList D_WSTAG971_800A6DA8 = {
    0,
    { &D_WSTAG971_800A6D48, &D_WSTAG971_800A6D54, &D_WSTAG971_800A6D60, &D_WSTAG971_800A6D6C, &D_WSTAG971_800A6D78,
        &D_WSTAG971_800A6D84, &D_WSTAG971_800A6D90, &D_WSTAG971_800A6D9C },
};
FieldstgBattleLists wstag971_battle_lists[3] = {
    { 416, 1, 0, { &D_WSTAG971_800A67FC, &D_WSTAG971_800A6880, &D_WSTAG971_800A6904 }, &D_WSTAG971_800A6988 },
    { 439, 5, 0, { &D_WSTAG971_800A6A0C, &D_WSTAG971_800A6A90, &D_WSTAG971_800A6B14 }, &D_WSTAG971_800A6B98 },
    { 445, 6, 0, { &D_WSTAG971_800A6C1C, &D_WSTAG971_800A6CA0, &D_WSTAG971_800A6D24 }, &D_WSTAG971_800A6DA8 },
};
