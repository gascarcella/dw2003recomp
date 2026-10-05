#include "wstag.h"

/* WSTAG371: stage 0x294 (fieldstg_stages). */

extern FieldstgSprite wstag371_sprites[];
extern FieldstgMapEvent wstag371_map_events[];
extern FieldstgVramPlace wstag371_vram_places[];
extern FieldstgPlacedActor *wstag371_actors[];
extern FieldstgBattleLists wstag371_battle_lists;
extern WstagFuncs wstag371_funcs;

void wstag371_update(WstagObject *obj) {
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

WstagObject *wstag371_start(void *arg0) {
    WstagObject *obj = object_new(wstag371_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag371_funcs.setup();
    return obj;
}

void wstag371_setup(void) {
    fieldstg_stage.background_file = 0x6BD;
    fieldstg_stage.sprite_file = 0x06BE0000;
    fieldstg_stage.sprites = wstag371_sprites;
    fieldstg_stage.map_events = wstag371_map_events;
    fieldstg_stage.mask_file = 0x6BC;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10600, 0x2BC00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag371_vram_places;
    fieldstg_stage.music = 0x32;
    fieldstg_stage.sound = 0x60C80000;
    fieldstg_stage.actors = wstag371_actors;
    fieldstg_stage.battle_lists = &wstag371_battle_lists;
    fieldstg_attr.set_file(0, 0x06BE0001);
    fieldstg_attr.set_file(7, 0x06BE0002);
    fieldstg_attr.set_file(4, 0x06BE0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag371_setup(void);

FieldstgListedBattle D_WSTAG371_800A5F94 = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FA0 = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FAC = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FB8 = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FC4 = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FD0 = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FDC = { 96, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A5FE8 = { 96, 3, 0x60080000 };
FieldstgBattleList D_WSTAG371_800A5FF4 = {
    3,
    { &D_WSTAG371_800A5F94, &D_WSTAG371_800A5FA0, &D_WSTAG371_800A5FAC, &D_WSTAG371_800A5FB8, &D_WSTAG371_800A5FC4,
        &D_WSTAG371_800A5FD0, &D_WSTAG371_800A5FDC, &D_WSTAG371_800A5FE8 },
};
FieldstgListedBattle D_WSTAG371_800A6018 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A6024 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A6030 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A603C = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A6048 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A6054 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A6060 = { 99, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG371_800A606C = { 99, 8, 0x60080000 };
FieldstgBattleList D_WSTAG371_800A6078 = {
    1,
    { &D_WSTAG371_800A6018, &D_WSTAG371_800A6024, &D_WSTAG371_800A6030, &D_WSTAG371_800A603C, &D_WSTAG371_800A6048,
        &D_WSTAG371_800A6054, &D_WSTAG371_800A6060, &D_WSTAG371_800A606C },
};
FieldstgListedBattle D_WSTAG371_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG371_800A60FC = {
    0,
    { &D_WSTAG371_800A609C, &D_WSTAG371_800A60A8, &D_WSTAG371_800A60B4, &D_WSTAG371_800A60C0, &D_WSTAG371_800A60CC,
        &D_WSTAG371_800A60D8, &D_WSTAG371_800A60E4, &D_WSTAG371_800A60F0 },
};
FieldstgListedBattle D_WSTAG371_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG371_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG371_800A6180 = {
    0,
    { &D_WSTAG371_800A6120, &D_WSTAG371_800A612C, &D_WSTAG371_800A6138, &D_WSTAG371_800A6144, &D_WSTAG371_800A6150,
        &D_WSTAG371_800A615C, &D_WSTAG371_800A6168, &D_WSTAG371_800A6174 },
};
FieldstgBattleLists wstag371_battle_lists = {
    67, 0, 0, { &D_WSTAG371_800A5FF4, &D_WSTAG371_800A6078, &D_WSTAG371_800A60FC }, &D_WSTAG371_800A6180,
};
FieldstgVramPlace wstag371_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 256, 128, 0, 320, 511 },
};
u16 D_WSTAG371_800A6230[4] = { 0x869A, 1, 0xFFFF, 0 };
u16 D_WSTAG371_800A6238[6] = { 0, 0, 0x869A, 0, 0xFFFF, 0 };
u16 D_WSTAG371_800A6244[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG371_800A624C[8] = { 0, 1, 0x869A, 0, 0x8496, 0, 0xFFFF, 0 };
u16 D_WSTAG371_800A625C[8] = { 0, 1, 0x869A, 0, 0x8496, 1, 0xFFFF, 0 };
u16 D_WSTAG371_800A626C[10] = {
    0x869A, 1, 0x8699, 0, 0x8496, 0, 0x7013, 1,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG371_800A6280[5] = {
    { D_WSTAG371_800A6230, NULL, 754 }, { D_WSTAG371_800A6238, D_WSTAG371_800A6244, 755 },
    { D_WSTAG371_800A624C, NULL, 756 }, { D_WSTAG371_800A625C, D_WSTAG371_800A626C, 757 }, { NULL, NULL, 0 },
};
u16 D_WSTAG371_800A62BC[10] = {
    0x7046, 1, 0x704E, 1, 0x8699, 1, 0x869A, 0,
    0xFFFF, 0,
};
FieldstgPlacedActor D_WSTAG371_800A62D0 = { D_WSTAG371_800A62BC, D_WSTAG371_800A6280, 169, 4, 734, 417, 7 };
FieldstgPlacedActor *wstag371_actors[2] = { &D_WSTAG371_800A62D0, NULL };
FieldstgSprite wstag371_sprites[9] = {
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 1024, 448, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 1088, 448, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 896, 576, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 960, 576, 0, 0 },
    { 1, 0, 0x40, 2, 4, 0, 0, 0, 0, 0, 1024, 576, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 1088, 576, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 1024, 640, 0, 0 }, { 1, 0, 0x40, 2, 7, 0, 0, 0, 0, 0, 1088, 640, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag371_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x295, 0x5E, 0x40E, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x374, 0x9B, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag371_funcs = { wstag371_setup };
