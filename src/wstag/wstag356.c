#include "wstag.h"

/* WSTAG356: stage 0x291 (fieldstg_stages). */

extern WstagFuncs wstag356_funcs;
extern FieldstgBattleLists wstag356_battle_lists;
extern FieldstgVramPlace wstag356_vram_places[];
extern FieldstgPlacedActor *wstag356_actors[];
extern FieldstgSprite wstag356_sprites[];
extern FieldstgMapEvent wstag356_map_events[];

void wstag356_update(WstagObject *obj) {
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

WstagObject *wstag356_start(void *arg0) {
    WstagObject *obj = object_new(wstag356_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag356_funcs.setup();
    return obj;
}

void wstag356_setup(void) {
    fieldstg_stage.background_file = 0x6AB;
    fieldstg_stage.sprite_file = 0x06AC0000;
    fieldstg_stage.sprites = wstag356_sprites;
    fieldstg_stage.map_events = wstag356_map_events;
    fieldstg_stage.mask_file = 0x6AA;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF100, 0xD800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag356_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag356_actors;
    fieldstg_stage.battle_lists = &wstag356_battle_lists;
    fieldstg_attr.set_file(0, 0x06AC0001);
    fieldstg_attr.set_file(7, 0x06AC0002);
    fieldstg_attr.set_file(4, 0x06AC0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag356_setup(void);

FieldstgListedBattle D_WSTAG356_800A5FD4 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A5FE0 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A5FEC = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A5FF8 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A6004 = { 94, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A6010 = { 94, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A601C = { 94, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A6028 = { 94, 13, 0x60080000 };
FieldstgBattleList D_WSTAG356_800A6034 = {
    3,
    { &D_WSTAG356_800A5FD4, &D_WSTAG356_800A5FE0, &D_WSTAG356_800A5FEC, &D_WSTAG356_800A5FF8, &D_WSTAG356_800A6004,
        &D_WSTAG356_800A6010, &D_WSTAG356_800A601C, &D_WSTAG356_800A6028 },
};
FieldstgListedBattle D_WSTAG356_800A6058 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6064 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A60AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG356_800A60B8 = {
    0,
    { &D_WSTAG356_800A6058, &D_WSTAG356_800A6064, &D_WSTAG356_800A6070, &D_WSTAG356_800A607C, &D_WSTAG356_800A6088,
        &D_WSTAG356_800A6094, &D_WSTAG356_800A60A0, &D_WSTAG356_800A60AC },
};
FieldstgListedBattle D_WSTAG356_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6130 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG356_800A613C = {
    0,
    { &D_WSTAG356_800A60DC, &D_WSTAG356_800A60E8, &D_WSTAG356_800A60F4, &D_WSTAG356_800A6100, &D_WSTAG356_800A610C,
        &D_WSTAG356_800A6118, &D_WSTAG356_800A6124, &D_WSTAG356_800A6130 },
};
FieldstgListedBattle D_WSTAG356_800A6160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A6184 = { 329, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A6190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG356_800A61A8 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG356_800A61B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG356_800A61C0 = {
    0,
    { &D_WSTAG356_800A6160, &D_WSTAG356_800A616C, &D_WSTAG356_800A6178, &D_WSTAG356_800A6184, &D_WSTAG356_800A6190,
        &D_WSTAG356_800A619C, &D_WSTAG356_800A61A8, &D_WSTAG356_800A61B4 },
};
FieldstgBattleLists wstag356_battle_lists = {
    63, 0, 0, { &D_WSTAG356_800A6034, &D_WSTAG356_800A60B8, &D_WSTAG356_800A613C }, &D_WSTAG356_800A61C0,
};
FieldstgVramPlace wstag356_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 344, 192, 88, 336, 511 }, { 320, 256, 368, 384, 192, 128, 352, 511 },
    { 320, 256, 368, 424, 192, 168, 368, 511 }, { 320, 256, 368, 456, 192, 200, 336, 510 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 320, 472, 0, 216, 352, 510 }, { 320, 256, 328, 472, 32, 216, 368, 510 },
    { 320, 256, 336, 472, 64, 216, 336, 509 }, { 384, 256, 406, 256, 344, 0, 352, 509 },
};
FieldstgTalk D_WSTAG356_800A62F0[2] = { { NULL, NULL, 114 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6308[2] = { { NULL, NULL, 117 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6320[2] = { { NULL, NULL, 120 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6338[2] = { { NULL, NULL, 123 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6350[2] = { { NULL, NULL, 112 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6368[2] = { { NULL, NULL, 113 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6380[2] = { { NULL, NULL, 115 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6398[2] = { { NULL, NULL, 116 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A63B0[2] = { { NULL, NULL, 118 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A63C8[2] = { { NULL, NULL, 119 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A63E0[2] = { { NULL, NULL, 121 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A63F8[2] = { { NULL, NULL, 124 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG356_800A6410[2] = { { NULL, NULL, 122 }, { NULL, NULL, 0 } };
u16 D_WSTAG356_800A6428[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A6434[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A6440[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A644C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A6458[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG356_800A6464[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A646C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG356_800A6478[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A6480[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG356_800A648C[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A6494[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG356_800A649C[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG356_800A64A8 = { D_WSTAG356_800A6428, D_WSTAG356_800A62F0, 47, 4, 319, 313, 1 };
FieldstgPlacedActor D_WSTAG356_800A64BC = { D_WSTAG356_800A6434, D_WSTAG356_800A6308, 48, 5, 361, 749, 7 };
FieldstgPlacedActor D_WSTAG356_800A64D0 = { D_WSTAG356_800A6440, D_WSTAG356_800A6320, 57, 6, 768, 664, 1 };
FieldstgPlacedActor D_WSTAG356_800A64E4 = { D_WSTAG356_800A644C, D_WSTAG356_800A6338, 58, 7, 760, 260, 3 };
FieldstgPlacedActor D_WSTAG356_800A64F8 = { NULL, D_WSTAG356_800A6350, 63, 8, 319, 600, 1 };
FieldstgPlacedActor D_WSTAG356_800A650C = { D_WSTAG356_800A6458, D_WSTAG356_800A6368, 157, 9, 319, 313, 1 };
FieldstgPlacedActor D_WSTAG356_800A6520 = { D_WSTAG356_800A6464, D_WSTAG356_800A6380, 157, 9, 319, 313, 1 };
FieldstgPlacedActor D_WSTAG356_800A6534 = { D_WSTAG356_800A646C, D_WSTAG356_800A6398, 158, 10, 361, 749, 7 };
FieldstgPlacedActor D_WSTAG356_800A6548 = { D_WSTAG356_800A6478, D_WSTAG356_800A63B0, 158, 10, 361, 749, 7 };
FieldstgPlacedActor D_WSTAG356_800A655C = { D_WSTAG356_800A6480, D_WSTAG356_800A63C8, 159, 11, 768, 664, 1 };
FieldstgPlacedActor D_WSTAG356_800A6570 = { D_WSTAG356_800A648C, D_WSTAG356_800A63E0, 159, 11, 768, 664, 1 };
FieldstgPlacedActor D_WSTAG356_800A6584 = { D_WSTAG356_800A6494, D_WSTAG356_800A63F8, 160, 12, 760, 260, 3 };
FieldstgPlacedActor D_WSTAG356_800A6598 = { D_WSTAG356_800A649C, D_WSTAG356_800A6410, 160, 12, 760, 260, 3 };
FieldstgPlacedActor *wstag356_actors[14] = {
    &D_WSTAG356_800A64A8, &D_WSTAG356_800A64BC, &D_WSTAG356_800A64D0, &D_WSTAG356_800A64E4, &D_WSTAG356_800A64F8,
    &D_WSTAG356_800A650C, &D_WSTAG356_800A6520, &D_WSTAG356_800A6534, &D_WSTAG356_800A6548, &D_WSTAG356_800A655C,
    &D_WSTAG356_800A6570, &D_WSTAG356_800A6584, &D_WSTAG356_800A6598, NULL,
};
FieldstgSprite wstag356_sprites[48] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 68, 778, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 159, 733, 0, 0 },
    { 1, 0, 0x80, 2, 4, 0, 0, 0, 0, 0, 13, 758, 0, 0 }, { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 89, 730, 0, 0 },
    { 1, 0, 0x80, 4, 5, 0, 0, 0, 0, 0, 63, 783, 795, 0 }, { 1, 0, 0x72, 4, 0, 0, 0, 0, 0, 0, 0, 657, 795, 0 },
    { 1, 0, 0x76, 4, 1, 0, 0, 0, 0, 0, 40, 677, 795, 0 }, { 1, 0, 0x60, 4, 3, 0, 0, 0, 0, 0, 290, 502, 595, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 103, 347, 347, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 119, 387, 387, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 156, 539, 539, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 168, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 169, 579, 579, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 298, 298, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 185, 379, 379, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 206, 603, 603, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 232, 356, 356, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 238, 215, 215, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 249, 635, 635, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 280, 667, 667, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 296, 723, 723, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 312, 219, 219, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 360, 198, 198, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 392, 323, 323, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 631, 631, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 423, 219, 219, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 424, 291, 291, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 440, 603, 603, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 472, 347, 347, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 487, 307, 307, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 552, 451, 451, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 595, 479, 479, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 597, 595, 595, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 597, 691, 691, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 600, 203, 203, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 608, 823, 823, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 623, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 625, 723, 723, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 642, 763, 763, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 648, 803, 803, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 680, 570, 570, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 728, 603, 603, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 760, 195, 195, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 833, 659, 659, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 840, 267, 267, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 845, 571, 571, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 871, 315, 315, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag356_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x290, 0x598, 0x260, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x296, 0x154, 0x6A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x294, 0x80, 0x31C, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x292, 0x1B2, 0x156, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag356_funcs = { wstag356_setup };
