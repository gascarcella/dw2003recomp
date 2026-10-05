#include "wstag.h"

/* WSTAG941: stage 0x28D (fieldstg_stages_2d). */

extern WstagFuncs wstag941_funcs;
extern FieldstgVramPlace wstag941_vram_places[];
extern FieldstgPlacedActor *wstag941_actors[];
extern FieldstgSprite wstag941_sprites[];
extern FieldstgMapEvent wstag941_map_events[];
extern FieldstgBattleLists wstag941_battle_lists;

void wstag941_update(WstagObject *obj) {
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

WstagObject *wstag941_start(void *arg0) {
    WstagObject *obj = object_new(wstag941_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag941_funcs.setup();
    return obj;
}

void wstag941_setup(void) {
    fieldstg_stage.background_file = 0x1B6;
    fieldstg_stage.sprite_file = 0x09090000;
    fieldstg_stage.sprites = wstag941_sprites;
    fieldstg_stage.map_events = wstag941_map_events;
    fieldstg_stage.mask_file = 0x908;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13800, 0x27100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag941_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag941_actors;
    fieldstg_stage.battle_lists = &wstag941_battle_lists;
    fieldstg_attr.set_file(0, 0x09090002);
    fieldstg_attr.set_file(7, 0x09090003);
    fieldstg_attr.set_file(4, 0x09090001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag941_setup(void);

FieldstgVramPlace wstag941_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 364, 256, 176, 0, 336, 511 }, { 320, 256, 372, 256, 208, 0, 352, 511 },
    { 320, 256, 372, 288, 208, 32, 368, 511 },
};
u16 D_WSTAG941_800A6024[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG941_800A6030[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG941_800A603C[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG941_800A6048[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG941_800A6058[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG941_800A6064[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG941_800A606C[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG941_800A6078[4] = { 0x782C, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG941_800A6080[2] = { { NULL, NULL, 116 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG941_800A6098[5] = {
    { D_WSTAG941_800A6024, D_WSTAG941_800A6030, 31 }, { D_WSTAG941_800A603C, D_WSTAG941_800A6048, 32 },
    { D_WSTAG941_800A6058, D_WSTAG941_800A6064, 29 }, { D_WSTAG941_800A606C, D_WSTAG941_800A6078, 30 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG941_800A60D4[2] = { { NULL, NULL, 29 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG941_800A60EC[2] = { { NULL, NULL, 115 }, { NULL, NULL, 0 } };
u16 D_WSTAG941_800A6104[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG941_800A610C[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG941_800A6114 = { NULL, D_WSTAG941_800A6080, 27, 4, 361, 684, 5 };
FieldstgPlacedActor D_WSTAG941_800A6128 = { D_WSTAG941_800A6104, D_WSTAG941_800A6098, 58, 5, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG941_800A613C = { D_WSTAG941_800A610C, D_WSTAG941_800A60D4, 58, 5, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG941_800A6150 = { NULL, D_WSTAG941_800A60EC, 365, 6, 391, 309, 1 };
FieldstgPlacedActor *wstag941_actors[5] = {
    &D_WSTAG941_800A6114, &D_WSTAG941_800A6128, &D_WSTAG941_800A613C, &D_WSTAG941_800A6150, NULL,
};
FieldstgSprite wstag941_sprites[44] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 199, 607, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 353, -16, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 513, 285, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 113, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 153, 554, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 167, 539, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 192, 494, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 225, 493, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 251, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 313, 468, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 86, 400, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 169, 500, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 193, 527, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 216, 519, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 235, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 248, 454, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 253, 491, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 264, 469, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 58, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 130, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 214, 356, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 233, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 204, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 273, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 219, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 289, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 307, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 317, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 323, 421, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 226, 226, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 232, 250, 250, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 243, 544, 544, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 267, 602, 602, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 271, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 277, 521, 521, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 399, 167, 167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 422, 187, 187, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 431, 568, 568, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 489, 501, 501, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 506, 203, 203, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 542, 228, 228, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 641, 286, 286, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag941_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x5F4, 0x39E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x290, 0x138, 0x7A, 7, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC8, 0xFFF0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag941_funcs = { wstag941_setup };
FieldstgListedBattle D_WSTAG941_800A650C = { 53, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6518 = { 53, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6524 = { 53, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6530 = { 53, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A653C = { 57, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6548 = { 57, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6554 = { 57, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A6560 = { 57, 1, 0x60080000 };
FieldstgBattleList D_WSTAG941_800A656C = {
    3,
    { &D_WSTAG941_800A650C, &D_WSTAG941_800A6518, &D_WSTAG941_800A6524, &D_WSTAG941_800A6530, &D_WSTAG941_800A653C,
        &D_WSTAG941_800A6548, &D_WSTAG941_800A6554, &D_WSTAG941_800A6560 },
};
FieldstgListedBattle D_WSTAG941_800A6590 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A659C = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65A8 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65B4 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65C0 = { 58, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65CC = { 58, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65D8 = { 58, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A65E4 = { 58, 13, 0x60080000 };
FieldstgBattleList D_WSTAG941_800A65F0 = {
    3,
    { &D_WSTAG941_800A6590, &D_WSTAG941_800A659C, &D_WSTAG941_800A65A8, &D_WSTAG941_800A65B4, &D_WSTAG941_800A65C0,
        &D_WSTAG941_800A65CC, &D_WSTAG941_800A65D8, &D_WSTAG941_800A65E4 },
};
FieldstgListedBattle D_WSTAG941_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A6620 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A662C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A6638 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A6644 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A6650 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A665C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A6668 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG941_800A6674 = {
    0,
    { &D_WSTAG941_800A6614, &D_WSTAG941_800A6620, &D_WSTAG941_800A662C, &D_WSTAG941_800A6638, &D_WSTAG941_800A6644,
        &D_WSTAG941_800A6650, &D_WSTAG941_800A665C, &D_WSTAG941_800A6668 },
};
FieldstgListedBattle D_WSTAG941_800A6698 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A66A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A66B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A66BC = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A66C8 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A66D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG941_800A66E0 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG941_800A66EC = { 61, 8, 0x60080000 };
FieldstgBattleList D_WSTAG941_800A66F8 = {
    0,
    { &D_WSTAG941_800A6698, &D_WSTAG941_800A66A4, &D_WSTAG941_800A66B0, &D_WSTAG941_800A66BC, &D_WSTAG941_800A66C8,
        &D_WSTAG941_800A66D4, &D_WSTAG941_800A66E0, &D_WSTAG941_800A66EC },
};
FieldstgBattleLists wstag941_battle_lists = {
    383, 0, 0, { &D_WSTAG941_800A656C, &D_WSTAG941_800A65F0, &D_WSTAG941_800A6674 }, &D_WSTAG941_800A66F8,
};
