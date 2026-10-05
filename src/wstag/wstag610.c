#include "wstag.h"

/* WSTAG610: stage 0x253 (fieldstg_stages). */

extern WstagFuncs wstag610_funcs;
extern FieldstgBattleLists wstag610_battle_lists;
extern FieldstgVramPlace wstag610_vram_places[];
extern FieldstgSprite wstag610_sprites[];
extern FieldstgMapEvent wstag610_map_events[];

void wstag610_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag610_start(void *arg0) {
    WstagObject *obj = object_new(wstag610_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag610_funcs.setup();
    return obj;
}

void wstag610_setup(void) {
    fieldstg_stage.background_file = 0x4BA;
    fieldstg_stage.sprite_file = 0x04C00000;
    fieldstg_stage.sprites = wstag610_sprites;
    fieldstg_stage.map_events = wstag610_map_events;
    fieldstg_stage.mask_file = 0x4BF;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xD300, 0x26100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag610_vram_places;
    fieldstg_stage.music = 0x3A;
    fieldstg_stage.sound = 0x60E80000;
    fieldstg_stage.battle_lists = &wstag610_battle_lists;
    fieldstg_attr.set_file(0, 0x04C00001);
    fieldstg_attr.set_file(7, 0x04C00002);
    fieldstg_attr.set_file(4, 0x04C00003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag610_setup(void);

FieldstgListedBattle D_WSTAG610_800A5F84 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5F90 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5F9C = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5FA8 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5FB4 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5FC0 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5FCC = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG610_800A5FD8 = { 75, 12, 0x60080000 };
FieldstgBattleList D_WSTAG610_800A5FE4 = {
    2,
    { &D_WSTAG610_800A5F84, &D_WSTAG610_800A5F90, &D_WSTAG610_800A5F9C, &D_WSTAG610_800A5FA8, &D_WSTAG610_800A5FB4,
        &D_WSTAG610_800A5FC0, &D_WSTAG610_800A5FCC, &D_WSTAG610_800A5FD8 },
};
FieldstgListedBattle D_WSTAG610_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A605C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG610_800A6068 = {
    0,
    { &D_WSTAG610_800A6008, &D_WSTAG610_800A6014, &D_WSTAG610_800A6020, &D_WSTAG610_800A602C, &D_WSTAG610_800A6038,
        &D_WSTAG610_800A6044, &D_WSTAG610_800A6050, &D_WSTAG610_800A605C },
};
FieldstgListedBattle D_WSTAG610_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A60E0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG610_800A60EC = {
    0,
    { &D_WSTAG610_800A608C, &D_WSTAG610_800A6098, &D_WSTAG610_800A60A4, &D_WSTAG610_800A60B0, &D_WSTAG610_800A60BC,
        &D_WSTAG610_800A60C8, &D_WSTAG610_800A60D4, &D_WSTAG610_800A60E0 },
};
FieldstgListedBattle D_WSTAG610_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG610_800A6164 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG610_800A6170 = {
    0,
    { &D_WSTAG610_800A6110, &D_WSTAG610_800A611C, &D_WSTAG610_800A6128, &D_WSTAG610_800A6134, &D_WSTAG610_800A6140,
        &D_WSTAG610_800A614C, &D_WSTAG610_800A6158, &D_WSTAG610_800A6164 },
};
FieldstgBattleLists wstag610_battle_lists = {
    47, 0, 0, { &D_WSTAG610_800A5FE4, &D_WSTAG610_800A6068, &D_WSTAG610_800A60EC }, &D_WSTAG610_800A6170,
};
FieldstgVramPlace wstag610_vram_places[6] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
};
FieldstgSprite wstag610_sprites[20] = {
    { 1, 0, 0x80, 2, 2, 0, 0, 0, 0, 0, 463, 0, 0, 0 }, { 1, 0, 0x80, 2, 3, 0, 0, 0, 0, 0, 512, 0, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 4, 0, 180, 506, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 4, 0, 249, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3D, 4, 0, 160, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3D, 4, 0, 229, 368, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 388, 395, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 429, 415, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 441, 565, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 468, 435, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 482, 585, 0, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 460, 138, 584, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 492, 154, 584, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 586, 201, 584, 0 }, { 1, 0, 0x65, 4, 0, 0, 0, 0, 0, 0, 84, 484, 584, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 489, 136, 190, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 509, 136, 198, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 525, 136, 207, 0 }, { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 541, 136, 215, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag610_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x88, 0x22C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x254, 0x3F0, 0x2F0, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag610_funcs = { wstag610_setup };
