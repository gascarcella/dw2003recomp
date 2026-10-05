#include "wstag.h"

/* WSTAG945: stage 0x291 (fieldstg_stages_2d). */

extern WstagFuncs wstag945_funcs;
extern FieldstgVramPlace wstag945_vram_places[];
extern FieldstgPlacedActor *wstag945_actors[];
extern FieldstgSprite wstag945_sprites[];
extern FieldstgMapEvent wstag945_map_events[];
extern FieldstgBattleLists wstag945_battle_lists;

void wstag945_update(WstagObject *obj) {
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

WstagObject *wstag945_start(void *arg0) {
    WstagObject *obj = object_new(wstag945_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag945_funcs.setup();
    return obj;
}

void wstag945_setup(void) {
    fieldstg_stage.background_file = 0x1BA;
    fieldstg_stage.sprite_file = 0x09110000;
    fieldstg_stage.sprites = wstag945_sprites;
    fieldstg_stage.map_events = wstag945_map_events;
    fieldstg_stage.mask_file = 0x910;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13800, 0x27100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag945_vram_places;
    fieldstg_stage.music = 0x2D;
    fieldstg_stage.sound = 0x60B40000;
    fieldstg_stage.actors = wstag945_actors;
    fieldstg_stage.battle_lists = &wstag945_battle_lists;
    fieldstg_attr.set_file(0, 0x09110002);
    fieldstg_attr.set_file(7, 0x09110003);
    fieldstg_attr.set_file(4, 0x09110001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag945_setup(void);

FieldstgVramPlace wstag945_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 374, 192, 118, 336, 511 }, { 320, 256, 368, 414, 192, 158, 352, 511 },
    { 320, 256, 368, 454, 192, 198, 368, 511 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 364, 256, 176, 0, 336, 510 },
};
u16 D_WSTAG945_800A6044[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A6050[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A605C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A6068[8] = { 0x11, 0, 0x10, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A6078[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A6084[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A608C[6] = { 0x11, 0, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A6098[4] = { 0x783C, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A60A0[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A60AC[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A60B8[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A60C4[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A60D4[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A60E0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A60E8[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A60F4[4] = { 0x7835, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG945_800A60FC[2] = { { NULL, NULL, 49 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG945_800A6114[5] = {
    { D_WSTAG945_800A6044, D_WSTAG945_800A6050, 51 }, { D_WSTAG945_800A605C, D_WSTAG945_800A6068, 52 },
    { D_WSTAG945_800A6078, D_WSTAG945_800A6084, 49 }, { D_WSTAG945_800A608C, D_WSTAG945_800A6098, 50 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG945_800A6150[2] = { { NULL, NULL, 126 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG945_800A6168[2] = { { NULL, NULL, 45 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG945_800A6180[5] = {
    { D_WSTAG945_800A60A0, D_WSTAG945_800A60AC, 47 }, { D_WSTAG945_800A60B8, D_WSTAG945_800A60C4, 48 },
    { D_WSTAG945_800A60D4, D_WSTAG945_800A60E0, 45 }, { D_WSTAG945_800A60E8, D_WSTAG945_800A60F4, 46 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG945_800A61BC[2] = { { NULL, NULL, 127 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG945_800A61D4[2] = { { NULL, NULL, 128 }, { NULL, NULL, 0 } };
u16 D_WSTAG945_800A61EC[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A61F4[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG945_800A61FC[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG945_800A6204[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG945_800A620C = { D_WSTAG945_800A61EC, D_WSTAG945_800A60FC, 48, 4, 361, 749, 7 };
FieldstgPlacedActor D_WSTAG945_800A6220 = { D_WSTAG945_800A61F4, D_WSTAG945_800A6114, 48, 4, 361, 749, 7 };
FieldstgPlacedActor D_WSTAG945_800A6234 = { NULL, D_WSTAG945_800A6150, 51, 5, 768, 664, 3 };
FieldstgPlacedActor D_WSTAG945_800A6248 = { D_WSTAG945_800A61FC, D_WSTAG945_800A6168, 53, 6, 319, 313, 1 };
FieldstgPlacedActor D_WSTAG945_800A625C = { D_WSTAG945_800A6204, D_WSTAG945_800A6180, 53, 6, 319, 313, 1 };
FieldstgPlacedActor D_WSTAG945_800A6270 = { NULL, D_WSTAG945_800A61BC, 63, 7, 319, 600, 1 };
FieldstgPlacedActor D_WSTAG945_800A6284 = { NULL, D_WSTAG945_800A61D4, 139, 8, 731, 350, 7 };
FieldstgPlacedActor *wstag945_actors[8] = {
    &D_WSTAG945_800A620C, &D_WSTAG945_800A6220, &D_WSTAG945_800A6234, &D_WSTAG945_800A6248, &D_WSTAG945_800A625C,
    &D_WSTAG945_800A6270, &D_WSTAG945_800A6284, NULL,
};
FieldstgSprite wstag945_sprites[48] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 68, 778, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 159, 733, 0, 0 },
    { 1, 0, 0x80, 2, 4, 0, 0, 0, 0, 0, 13, 758, 0, 0 }, { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 89, 730, 0, 0 },
    { 1, 0, 0x72, 4, 0, 0, 0, 0, 0, 0, 0, 657, 795, 0 }, { 1, 0, 0x76, 4, 1, 0, 0, 0, 0, 0, 40, 677, 795, 0 },
    { 1, 0, 0x60, 4, 3, 0, 0, 0, 0, 0, 290, 502, 595, 0 }, { 1, 0, 0x80, 4, 5, 0, 0, 0, 0, 0, 63, 783, 795, 0 },
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
FieldstgMapEvent wstag945_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x290, 0x598, 0x260, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x296, 0x154, 0x6A, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x294, 0x80, 0x31C, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x292, 0x1B2, 0x156, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag945_funcs = { wstag945_setup };
FieldstgListedBattle D_WSTAG945_800A66AC = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66B8 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66C4 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66D0 = { 151, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66DC = { 94, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66E8 = { 94, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A66F4 = { 101, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A6700 = { 101, 13, 0x60080000 };
FieldstgBattleList D_WSTAG945_800A670C = {
    3,
    { &D_WSTAG945_800A66AC, &D_WSTAG945_800A66B8, &D_WSTAG945_800A66C4, &D_WSTAG945_800A66D0, &D_WSTAG945_800A66DC,
        &D_WSTAG945_800A66E8, &D_WSTAG945_800A66F4, &D_WSTAG945_800A6700 },
};
FieldstgListedBattle D_WSTAG945_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A673C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6748 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6754 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6760 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A676C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6778 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6784 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG945_800A6790 = {
    0,
    { &D_WSTAG945_800A6730, &D_WSTAG945_800A673C, &D_WSTAG945_800A6748, &D_WSTAG945_800A6754, &D_WSTAG945_800A6760,
        &D_WSTAG945_800A676C, &D_WSTAG945_800A6778, &D_WSTAG945_800A6784 },
};
FieldstgListedBattle D_WSTAG945_800A67B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A67FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6808 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG945_800A6814 = {
    0,
    { &D_WSTAG945_800A67B4, &D_WSTAG945_800A67C0, &D_WSTAG945_800A67CC, &D_WSTAG945_800A67D8, &D_WSTAG945_800A67E4,
        &D_WSTAG945_800A67F0, &D_WSTAG945_800A67FC, &D_WSTAG945_800A6808 },
};
FieldstgListedBattle D_WSTAG945_800A6838 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6844 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6850 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A685C = { 331, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A6868 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6874 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG945_800A6880 = { 99, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG945_800A688C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG945_800A6898 = {
    0,
    { &D_WSTAG945_800A6838, &D_WSTAG945_800A6844, &D_WSTAG945_800A6850, &D_WSTAG945_800A685C, &D_WSTAG945_800A6868,
        &D_WSTAG945_800A6874, &D_WSTAG945_800A6880, &D_WSTAG945_800A688C },
};
FieldstgBattleLists wstag945_battle_lists = {
    385, 0, 0, { &D_WSTAG945_800A670C, &D_WSTAG945_800A6790, &D_WSTAG945_800A6814 }, &D_WSTAG945_800A6898,
};
