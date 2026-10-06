#include "wstag.h"

/* WSTAG967: stage 0x2E7 (fieldstg_stages_2d). */

extern WstagExits *wstag967_exits[];
extern WstagFuncs wstag967_funcs;
const CVECTOR wstag967_color = { 0x54, 0x67, 0x96, 1 };
extern FieldstgVramPlace wstag967_vram_places[];
extern FieldstgPlacedActor *wstag967_actors[];
extern FieldstgSprite wstag967_sprites[];
extern FieldstgMapEvent wstag967_map_events[];
extern FieldstgBattleLists wstag967_battle_lists[];

s32 wstag967_set_exits(FieldstgMapEvent *dst, WstagExits **list, s32 arg2, s32 arg3) {
    WstagExits *exits = *list;
    WstagExit *exit;

    if (exits == NULL) {
        return;
    }
    if (exits->route != arg2 || exits->room != arg3) {
        return wstag967_set_exits(dst, list + 1, arg2, arg3);
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

void wstag967_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag967_set_exits(fieldstg_stage.map_events, wstag967_exits, gamestate_data.route,
                               gamestate_data.room);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag967_start(void *arg0) {
    WstagObject *obj = object_new(wstag967_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag967_funcs.setup();
    return obj;
}

void wstag967_setup(void) {
    fieldstg_stage.background_file = 0x6F4;
    fieldstg_stage.sprites = wstag967_sprites;
    fieldstg_stage.map_events = wstag967_map_events;
    fieldstg_stage.sprite_file = 0x093D0004;
    fieldstg_stage.mask_file = 0x93C;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0xEC00, 0x14D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag967_vram_places;
    fieldstg_stage.music = 0x1D;
    fieldstg_stage.sound = 0x60740000;
    fieldstg_stage.actors = wstag967_actors;
    fieldstg_stage.color = wstag967_color;
    fieldstg_stage.battle_lists = fieldstg_stage.find_battle_lists(wstag967_battle_lists, gamestate_data.route);
    fieldstg_attr.set_file(0, 0x093D0006);
    fieldstg_attr.set_file(7, 0x093D0007);
    fieldstg_attr.set_file(4, 0x093D0005);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag967_setup(void);

WstagExit D_WSTAG967_800A60FC = { 0x2E6, 3, 1, 0xA0, 0x150, 5, NULL };
WstagExits D_WSTAG967_800A610C = { 3, 1, &D_WSTAG967_800A60FC };
WstagExit D_WSTAG967_800A6114 = { 0x2E4, 4, 1, 0xC0, 0x180, 5, NULL };
WstagExits D_WSTAG967_800A6124 = { 4, 1, &D_WSTAG967_800A6114 };
WstagExit D_WSTAG967_800A612C = { 0x2E4, 5, 3, 0xC0, 0x180, 5, NULL };
WstagExits D_WSTAG967_800A613C = { 5, 1, &D_WSTAG967_800A612C };
WstagExits *wstag967_exits[4] = { &D_WSTAG967_800A610C, &D_WSTAG967_800A6124, &D_WSTAG967_800A613C, NULL };
FieldstgVramPlace wstag967_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 366, 351, 184, 95, 336, 511 }, { 320, 256, 352, 352, 128, 96, 352, 511 },
};
u16 D_WSTAG967_800A61D4[6] = { 0x26E, 1, 0x8496, 1, 0xFFFF, 0 };
u16 D_WSTAG967_800A61E0[6] = { 0x272, 1, 0x8471, 1, 0xFFFF, 0 };
u16 D_WSTAG967_800A61EC[6] = { 0x271, 1, 0x8463, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG967_800A61F8[2] = { { NULL, D_WSTAG967_800A61D4, 6 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG967_800A6210[2] = { { NULL, D_WSTAG967_800A61E0, 10 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG967_800A6228[2] = { { NULL, D_WSTAG967_800A61EC, 9 }, { NULL, NULL, 0 } };
u16 D_WSTAG967_800A6240[8] = { 0x7E02, 1, 0x7E1E, 1, 0x26E, 0, 0xFFFF, 0 };
u16 D_WSTAG967_800A6250[8] = { 0x7E03, 1, 0x7E1E, 1, 0x272, 0, 0xFFFF, 0 };
u16 D_WSTAG967_800A6260[8] = { 0x7E04, 1, 0x7E1E, 1, 0x271, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG967_800A6270 = { D_WSTAG967_800A6240, D_WSTAG967_800A61F8, 33, 4, 192, 280, 1 };
FieldstgPlacedActor D_WSTAG967_800A6284 = { D_WSTAG967_800A6250, D_WSTAG967_800A6210, 33, 4, 192, 280, 1 };
FieldstgPlacedActor D_WSTAG967_800A6298 = { D_WSTAG967_800A6260, D_WSTAG967_800A6228, 33, 4, 192, 280, 1 };
FieldstgPlacedActor D_WSTAG967_800A62AC = { NULL, NULL, 327, 5, 0, 0, 0 };
FieldstgPlacedActor *wstag967_actors[5] = {
    &D_WSTAG967_800A6270, &D_WSTAG967_800A6284, &D_WSTAG967_800A6298, &D_WSTAG967_800A62AC, NULL,
};
FieldstgSprite wstag967_sprites[13] = {
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 360, 185, 0, 0 },
    { 1, 0, 0x8F, 2, 0x37, 1, 0x37, 0x4B, 0xA, 0, 589, 120, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 268, 192, 0, 0 },
    { 1, 0, 0xB6, 2, 0x4C, 1, 0x4C, 0x62, 0xA, 0, 512, 122, 0, 0 },
    { 1, 0, 0x8F, 6, 0x37, 1, 0x37, 0x4B, 0xA, 0, 110, 120, 0, 0 },
    { 1, 0, 0x68, 6, 0x34, 1, 0x34, 0x36, 0xA, 0, 412, 114, 0, 0 },
    { 1, 0, 0x4A, 4, 0, 0, 0, 0, 0, 0, 416, 215, 288, 0 }, { 1, 0, 0x61, 4, 1, 0, 0, 0, 0, 0, 384, 198, 288, 0 },
    { 1, 0, 0x5F, 4, 2, 0, 0, 0, 0, 0, 365, 191, 281, 0 }, { 1, 0, 0x4F, 4, 3, 0, 0, 0, 0, 0, 592, 173, 251, 0 },
    { 1, 0, 0x63, 4, 4, 0, 0, 0, 0, 0, 560, 157, 247, 0 }, { 1, 0, 0x60, 4, 5, 0, 0, 0, 0, 0, 541, 150, 240, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag967_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2E7, 0x250, 0xE8, 5, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag967_funcs = { wstag967_setup };
FieldstgListedBattle D_WSTAG967_800A63F4 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6400 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A640C = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6418 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6424 = { 105, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6430 = { 275, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A643C = { 275, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6448 = { 275, 11, 0x60080000 };
FieldstgBattleList D_WSTAG967_800A6454 = {
    5,
    { &D_WSTAG967_800A63F4, &D_WSTAG967_800A6400, &D_WSTAG967_800A640C, &D_WSTAG967_800A6418, &D_WSTAG967_800A6424,
        &D_WSTAG967_800A6430, &D_WSTAG967_800A643C, &D_WSTAG967_800A6448 },
};
FieldstgListedBattle D_WSTAG967_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A64B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A64C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A64CC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A64D8 = {
    0,
    { &D_WSTAG967_800A6478, &D_WSTAG967_800A6484, &D_WSTAG967_800A6490, &D_WSTAG967_800A649C, &D_WSTAG967_800A64A8,
        &D_WSTAG967_800A64B4, &D_WSTAG967_800A64C0, &D_WSTAG967_800A64CC },
};
FieldstgListedBattle D_WSTAG967_800A64FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6508 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6514 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6520 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A652C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6538 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6544 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6550 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A655C = {
    0,
    { &D_WSTAG967_800A64FC, &D_WSTAG967_800A6508, &D_WSTAG967_800A6514, &D_WSTAG967_800A6520, &D_WSTAG967_800A652C,
        &D_WSTAG967_800A6538, &D_WSTAG967_800A6544, &D_WSTAG967_800A6550 },
};
FieldstgListedBattle D_WSTAG967_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6598 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A65A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A65B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A65BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A65C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A65D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A65E0 = {
    0,
    { &D_WSTAG967_800A6580, &D_WSTAG967_800A658C, &D_WSTAG967_800A6598, &D_WSTAG967_800A65A4, &D_WSTAG967_800A65B0,
        &D_WSTAG967_800A65BC, &D_WSTAG967_800A65C8, &D_WSTAG967_800A65D4 },
};
FieldstgListedBattle D_WSTAG967_800A6604 = { 179, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6610 = { 179, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A661C = { 179, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6628 = { 179, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6634 = { 179, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6640 = { 274, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A664C = { 274, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6658 = { 274, 11, 0x60080000 };
FieldstgBattleList D_WSTAG967_800A6664 = {
    5,
    { &D_WSTAG967_800A6604, &D_WSTAG967_800A6610, &D_WSTAG967_800A661C, &D_WSTAG967_800A6628, &D_WSTAG967_800A6634,
        &D_WSTAG967_800A6640, &D_WSTAG967_800A664C, &D_WSTAG967_800A6658 },
};
FieldstgListedBattle D_WSTAG967_800A6688 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6694 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A66DC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A66E8 = {
    0,
    { &D_WSTAG967_800A6688, &D_WSTAG967_800A6694, &D_WSTAG967_800A66A0, &D_WSTAG967_800A66AC, &D_WSTAG967_800A66B8,
        &D_WSTAG967_800A66C4, &D_WSTAG967_800A66D0, &D_WSTAG967_800A66DC },
};
FieldstgListedBattle D_WSTAG967_800A670C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6718 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A673C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6748 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6754 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6760 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A676C = {
    0,
    { &D_WSTAG967_800A670C, &D_WSTAG967_800A6718, &D_WSTAG967_800A6724, &D_WSTAG967_800A6730, &D_WSTAG967_800A673C,
        &D_WSTAG967_800A6748, &D_WSTAG967_800A6754, &D_WSTAG967_800A6760 },
};
FieldstgListedBattle D_WSTAG967_800A6790 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A679C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A67E4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A67F0 = {
    0,
    { &D_WSTAG967_800A6790, &D_WSTAG967_800A679C, &D_WSTAG967_800A67A8, &D_WSTAG967_800A67B4, &D_WSTAG967_800A67C0,
        &D_WSTAG967_800A67CC, &D_WSTAG967_800A67D8, &D_WSTAG967_800A67E4 },
};
FieldstgListedBattle D_WSTAG967_800A6814 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6820 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A682C = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6838 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6844 = { 185, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6850 = { 277, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A685C = { 277, 11, 0x60080000 };
FieldstgListedBattle D_WSTAG967_800A6868 = { 277, 11, 0x60080000 };
FieldstgBattleList D_WSTAG967_800A6874 = {
    4,
    { &D_WSTAG967_800A6814, &D_WSTAG967_800A6820, &D_WSTAG967_800A682C, &D_WSTAG967_800A6838, &D_WSTAG967_800A6844,
        &D_WSTAG967_800A6850, &D_WSTAG967_800A685C, &D_WSTAG967_800A6868 },
};
FieldstgListedBattle D_WSTAG967_800A6898 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A68EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A68F8 = {
    0,
    { &D_WSTAG967_800A6898, &D_WSTAG967_800A68A4, &D_WSTAG967_800A68B0, &D_WSTAG967_800A68BC, &D_WSTAG967_800A68C8,
        &D_WSTAG967_800A68D4, &D_WSTAG967_800A68E0, &D_WSTAG967_800A68EC },
};
FieldstgListedBattle D_WSTAG967_800A691C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6928 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6934 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6940 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A694C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6958 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6964 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A6970 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A697C = {
    0,
    { &D_WSTAG967_800A691C, &D_WSTAG967_800A6928, &D_WSTAG967_800A6934, &D_WSTAG967_800A6940, &D_WSTAG967_800A694C,
        &D_WSTAG967_800A6958, &D_WSTAG967_800A6964, &D_WSTAG967_800A6970 },
};
FieldstgListedBattle D_WSTAG967_800A69A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG967_800A69F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG967_800A6A00 = {
    0,
    { &D_WSTAG967_800A69A0, &D_WSTAG967_800A69AC, &D_WSTAG967_800A69B8, &D_WSTAG967_800A69C4, &D_WSTAG967_800A69D0,
        &D_WSTAG967_800A69DC, &D_WSTAG967_800A69E8, &D_WSTAG967_800A69F4 },
};
FieldstgBattleLists wstag967_battle_lists[3] = {
    { 405, 3, 0, { &D_WSTAG967_800A6454, &D_WSTAG967_800A64D8, &D_WSTAG967_800A655C }, &D_WSTAG967_800A65E0 },
    { 409, 4, 0, { &D_WSTAG967_800A6664, &D_WSTAG967_800A66E8, &D_WSTAG967_800A676C }, &D_WSTAG967_800A67F0 },
    { 413, 5, 0, { &D_WSTAG967_800A6874, &D_WSTAG967_800A68F8, &D_WSTAG967_800A697C }, &D_WSTAG967_800A6A00 },
};
