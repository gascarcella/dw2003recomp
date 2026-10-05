#include "wstag.h"

/* WSTAG381: stage 0x296 (fieldstg_stages). */

extern WstagFuncs wstag381_funcs;
extern CVECTOR wstag381_color;
extern FieldstgBattleLists wstag381_battle_lists;
extern FieldstgVramPlace wstag381_vram_places[];
extern FieldstgPlacedActor *wstag381_actors[];
extern FieldstgSprite wstag381_sprites[];
extern FieldstgMapEvent wstag381_map_events[];

void wstag381_update(WstagObject *obj) {
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

WstagObject *wstag381_start(void *arg0) {
    WstagObject *obj = object_new(wstag381_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag381_funcs.setup();
    return obj;
}

void wstag381_setup(void) {
    fieldstg_stage.background_file = 0x580;
    fieldstg_stage.sprite_file = 0x05810000;
    fieldstg_stage.sprites = wstag381_sprites;
    fieldstg_stage.map_events = wstag381_map_events;
    fieldstg_stage.mask_file = 0x57F;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1A200, 0xA000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag381_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag381_actors;
    fieldstg_stage.color = wstag381_color;
    fieldstg_stage.battle_lists = &wstag381_battle_lists;
    fieldstg_attr.set_file(0, 0x05810001);
    fieldstg_attr.set_file(7, 0x05810002);
    fieldstg_attr.set_file(4, 0x05810003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag381/nonmatchings/wstag381", wstag381_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag381_setup(void);

FieldstgListedBattle D_WSTAG381_800A5FF8 = { 106, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A6004 = { 106, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A6010 = { 106, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A601C = { 106, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A6028 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A6034 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A6040 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A604C = { 151, 13, 0x60080000 };
FieldstgBattleList D_WSTAG381_800A6058 = {
    3,
    { &D_WSTAG381_800A5FF8, &D_WSTAG381_800A6004, &D_WSTAG381_800A6010, &D_WSTAG381_800A601C, &D_WSTAG381_800A6028,
        &D_WSTAG381_800A6034, &D_WSTAG381_800A6040, &D_WSTAG381_800A604C },
};
FieldstgListedBattle D_WSTAG381_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A60AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A60B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A60C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A60D0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG381_800A60DC = {
    0,
    { &D_WSTAG381_800A607C, &D_WSTAG381_800A6088, &D_WSTAG381_800A6094, &D_WSTAG381_800A60A0, &D_WSTAG381_800A60AC,
        &D_WSTAG381_800A60B8, &D_WSTAG381_800A60C4, &D_WSTAG381_800A60D0 },
};
FieldstgListedBattle D_WSTAG381_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A613C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6154 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG381_800A6160 = {
    0,
    { &D_WSTAG381_800A6100, &D_WSTAG381_800A610C, &D_WSTAG381_800A6118, &D_WSTAG381_800A6124, &D_WSTAG381_800A6130,
        &D_WSTAG381_800A613C, &D_WSTAG381_800A6148, &D_WSTAG381_800A6154 },
};
FieldstgListedBattle D_WSTAG381_800A6184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A6190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A61A8 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A61B4 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A61C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG381_800A61CC = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG381_800A61D8 = { 64, 8, 0x60080000 };
FieldstgBattleList D_WSTAG381_800A61E4 = {
    0,
    { &D_WSTAG381_800A6184, &D_WSTAG381_800A6190, &D_WSTAG381_800A619C, &D_WSTAG381_800A61A8, &D_WSTAG381_800A61B4,
        &D_WSTAG381_800A61C0, &D_WSTAG381_800A61CC, &D_WSTAG381_800A61D8 },
};
FieldstgBattleLists wstag381_battle_lists = {
    64, 0, 0, { &D_WSTAG381_800A6058, &D_WSTAG381_800A60DC, &D_WSTAG381_800A6160 }, &D_WSTAG381_800A61E4,
};
FieldstgVramPlace wstag381_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 256, 256, 0, 336, 511 }, { 320, 256, 370, 412, 200, 156, 352, 511 },
    { 320, 256, 320, 450, 0, 194, 368, 511 },
};
u16 D_WSTAG381_800A62B4[8] = { 0x21C, 1, 0x8B16, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG381_800A62C4[2] = { { NULL, D_WSTAG381_800A62B4, 380 }, { NULL, NULL, 0 } };
u16 D_WSTAG381_800A62DC[4] = { 0x21C, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG381_800A62E4 = { D_WSTAG381_800A62DC, D_WSTAG381_800A62C4, 33, 4, 129, 417, 1 };
FieldstgPlacedActor D_WSTAG381_800A62F8 = { NULL, NULL, 75, 5, 760, 357, 7 };
FieldstgPlacedActor D_WSTAG381_800A630C = { NULL, NULL, 76, 6, 670, 280, 1 };
FieldstgPlacedActor *wstag381_actors[4] = {
    &D_WSTAG381_800A62E4, &D_WSTAG381_800A62F8, &D_WSTAG381_800A630C, NULL,
};
FieldstgSprite wstag381_sprites[46] = {
    { 1, 0, 0x40, 2, 4, 0, 4, 9, 8, 0, 39, 524, 0, 0 }, { 1, 0, 0x40, 2, 4, 0, 4, 9, 8, 0, 423, -13, 0, 0 },
    { 1, 0, 0x40, 2, 4, 0, 4, 9, 8, 0, 759, 748, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 337, 409, 0, 0 },
    { 1, 0, 0x74, 2, 2, 0, 0, 0, 0, 0, 655, 662, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 420, 521, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 646, 239, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 727, 501, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 42, 777, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 351, 449, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 549, 447, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 631, 642, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 852, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1012, 373, 0, 0 },
    { 1, 0, 0x74, 6, 3, 0, 0, 0, 0, 0, 768, 640, 0, 0 }, { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 96, 687, 687, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 103, 923, 923, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 144, 663, 663, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 703, 703, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 160, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 639, 639, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 192, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 679, 679, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 952, 952, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 655, 655, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 197, 197, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 274, 262, 262, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 623, 623, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 937, 937, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 305, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 336, 599, 599, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 352, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 583, 583, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 591, 591, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 127, 127, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 624, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 110, 110, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 831, 831, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 992, 783, 783, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1115, 428, 428, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag381_map_events[16] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x291, 0x32A, 0x334, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x298, 0x9A, 0x74, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0x12, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x1A1, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x1B0, 0x176, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0x58, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0x40, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFC0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB8, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFF90, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x18, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFF0, 0x10, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag381_funcs = { wstag381_setup };
