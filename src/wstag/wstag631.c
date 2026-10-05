#include "wstag.h"

/* WSTAG631: stage 0x2C0 (fieldstg_stages). */

extern WstagFuncs wstag631_funcs;
extern FieldstgBattleLists wstag631_battle_lists;
extern FieldstgVramPlace wstag631_vram_places[];
extern FieldstgPlacedActor *wstag631_actors[];
extern FieldstgSprite wstag631_sprites[];
extern FieldstgMapEvent wstag631_map_events[];

void wstag631_update(WstagObject *obj) {
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

WstagObject *wstag631_start(void *arg0) {
    WstagObject *obj = object_new(wstag631_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag631_funcs.setup();
    return obj;
}

void wstag631_setup(void) {
    fieldstg_stage.background_file = 0x61E;
    fieldstg_stage.sprite_file = 0x061F0000;
    fieldstg_stage.sprites = wstag631_sprites;
    fieldstg_stage.map_events = wstag631_map_events;
    fieldstg_stage.mask_file = 0x61D;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x16000, 0x23100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag631_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag631_actors;
    fieldstg_stage.battle_lists = &wstag631_battle_lists;
    fieldstg_attr.set_file(0, 0x061F0001);
    fieldstg_attr.set_file(7, 0x061F0002);
    fieldstg_attr.set_file(4, 0x061F0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag631_setup(void);

FieldstgListedBattle D_WSTAG631_800A5F94 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FA0 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FAC = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FB8 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FC4 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FD0 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FDC = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A5FE8 = { 72, 5, 0x60080000 };
FieldstgBattleList D_WSTAG631_800A5FF4 = {
    3,
    { &D_WSTAG631_800A5F94, &D_WSTAG631_800A5FA0, &D_WSTAG631_800A5FAC, &D_WSTAG631_800A5FB8, &D_WSTAG631_800A5FC4,
        &D_WSTAG631_800A5FD0, &D_WSTAG631_800A5FDC, &D_WSTAG631_800A5FE8 },
};
FieldstgListedBattle D_WSTAG631_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG631_800A6078 = {
    0,
    { &D_WSTAG631_800A6018, &D_WSTAG631_800A6024, &D_WSTAG631_800A6030, &D_WSTAG631_800A603C, &D_WSTAG631_800A6048,
        &D_WSTAG631_800A6054, &D_WSTAG631_800A6060, &D_WSTAG631_800A606C },
};
FieldstgListedBattle D_WSTAG631_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG631_800A60FC = {
    0,
    { &D_WSTAG631_800A609C, &D_WSTAG631_800A60A8, &D_WSTAG631_800A60B4, &D_WSTAG631_800A60C0, &D_WSTAG631_800A60CC,
        &D_WSTAG631_800A60D8, &D_WSTAG631_800A60E4, &D_WSTAG631_800A60F0 },
};
FieldstgListedBattle D_WSTAG631_800A6120 = { 238, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG631_800A612C = { 286, 5, 0x600C0000 };
FieldstgListedBattle D_WSTAG631_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG631_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG631_800A6180 = {
    0,
    { &D_WSTAG631_800A6120, &D_WSTAG631_800A612C, &D_WSTAG631_800A6138, &D_WSTAG631_800A6144, &D_WSTAG631_800A6150,
        &D_WSTAG631_800A615C, &D_WSTAG631_800A6168, &D_WSTAG631_800A6174 },
};
FieldstgBattleLists wstag631_battle_lists = {
    101, 0, 0, { &D_WSTAG631_800A5FF4, &D_WSTAG631_800A6078, &D_WSTAG631_800A60FC }, &D_WSTAG631_800A6180,
};
FieldstgVramPlace wstag631_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 256, 136, 0, 320, 511 }, { 320, 256, 362, 256, 168, 0, 336, 511 },
    { 320, 256, 370, 256, 200, 0, 352, 511 }, { 320, 256, 354, 296, 136, 40, 368, 511 },
    { 320, 256, 362, 296, 168, 40, 320, 510 }, { 320, 256, 370, 296, 200, 40, 336, 510 },
    { 320, 256, 354, 336, 136, 80, 352, 510 }, { 320, 256, 370, 376, 200, 120, 368, 510 },
    { 320, 256, 362, 336, 168, 80, 320, 509 }, { 320, 256, 370, 336, 200, 80, 336, 509 },
    { 320, 256, 354, 376, 136, 120, 352, 509 }, { 320, 256, 362, 376, 168, 120, 368, 509 },
};
u16 D_WSTAG631_800A62E0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A62E8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A62F0[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A62FC[8] = { 0, 1, 0x7207, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A630C[4] = { 0x7636, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6314[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE29, 0,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A6328[6] = { 0x7400, 1, 0xE29, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6334[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE29, 1,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A6348[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6350[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6358[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6364[8] = { 0, 1, 0x720A, 1, 0xE4A, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6374[6] = { 0x7400, 1, 0xE4A, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6380[10] = {
    0, 1, 0x720A, 1, 0xE4A, 1, 0x720C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A6394[10] = {
    0, 1, 0x720A, 1, 0xE4A, 1, 0x720C, 1,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A63A8[4] = { 0x7836, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A63B0[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A63B8[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A63C0[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A63C8[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A63D0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG631_800A63DC[2] = { { NULL, NULL, 454 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A63F4[6] = {
    { D_WSTAG631_800A62E0, D_WSTAG631_800A62E8, 580 }, { D_WSTAG631_800A62F0, NULL, 581 },
    { D_WSTAG631_800A62FC, D_WSTAG631_800A630C, 582 }, { D_WSTAG631_800A6314, D_WSTAG631_800A6328, 583 },
    { D_WSTAG631_800A6334, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG631_800A643C[6] = {
    { D_WSTAG631_800A6348, D_WSTAG631_800A6350, 588 }, { D_WSTAG631_800A6358, NULL, 590 },
    { D_WSTAG631_800A6364, D_WSTAG631_800A6374, 589 }, { D_WSTAG631_800A6380, NULL, 584 },
    { D_WSTAG631_800A6394, D_WSTAG631_800A63A8, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG631_800A6484[4] = {
    { D_WSTAG631_800A63B0, NULL, 923 }, { D_WSTAG631_800A63B8, D_WSTAG631_800A63C0, 585 },
    { D_WSTAG631_800A63C8, D_WSTAG631_800A63D0, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG631_800A64B4[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A64CC[2] = { { NULL, NULL, 455 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A64E4[2] = { { NULL, NULL, 455 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A64FC[2] = { { NULL, NULL, 811 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A6514[2] = { { NULL, NULL, 812 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG631_800A652C[2] = { { NULL, NULL, 813 }, { NULL, NULL, 0 } };
u16 D_WSTAG631_800A6544[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6550[10] = {
    0x8192, 1, 0x11, 0, 0x8014, 0, 0x7019, 1,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A6564[10] = {
    0x7019, 1, 0x11, 0, 0x8192, 1, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG631_800A6578[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6588[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A6594[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65A4[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65B4[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65C4[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65D4[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65E4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A65F0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG631_800A65F8[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6608[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6618[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG631_800A6628[8] = { 0x7021, 1, 0x6025, 0, 0x6026, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG631_800A6638 = { D_WSTAG631_800A6544, D_WSTAG631_800A63DC, 50, 4, 752, 289, 1 };
FieldstgPlacedActor D_WSTAG631_800A664C = { D_WSTAG631_800A6550, D_WSTAG631_800A63F4, 69, 5, 432, 273, 7 };
FieldstgPlacedActor D_WSTAG631_800A6660 = { D_WSTAG631_800A6564, D_WSTAG631_800A643C, 69, 5, 432, 273, 7 };
FieldstgPlacedActor D_WSTAG631_800A6674 = { D_WSTAG631_800A6578, D_WSTAG631_800A6484, 69, 5, 432, 273, 7 };
FieldstgPlacedActor D_WSTAG631_800A6688 = { D_WSTAG631_800A6588, D_WSTAG631_800A64B4, 69, 5, 432, 273, 7 };
FieldstgPlacedActor D_WSTAG631_800A669C = { D_WSTAG631_800A6594, NULL, 70, 6, 143, 672, 1 };
FieldstgPlacedActor D_WSTAG631_800A66B0 = { D_WSTAG631_800A65A4, NULL, 71, 7, 177, 689, 1 };
FieldstgPlacedActor D_WSTAG631_800A66C4 = { D_WSTAG631_800A65B4, NULL, 72, 8, 207, 657, 1 };
FieldstgPlacedActor D_WSTAG631_800A66D8 = { D_WSTAG631_800A65C4, NULL, 73, 9, 232, 620, 1 };
FieldstgPlacedActor D_WSTAG631_800A66EC = { D_WSTAG631_800A65D4, NULL, 74, 10, 263, 628, 1 };
FieldstgPlacedActor D_WSTAG631_800A6700 = { D_WSTAG631_800A65E4, D_WSTAG631_800A64CC, 157, 11, 752, 289, 1 };
FieldstgPlacedActor D_WSTAG631_800A6714 = { D_WSTAG631_800A65F0, D_WSTAG631_800A64E4, 157, 11, 752, 289, 1 };
FieldstgPlacedActor D_WSTAG631_800A6728 = { D_WSTAG631_800A65F8, NULL, 164, 12, 280, 645, 1 };
FieldstgPlacedActor D_WSTAG631_800A673C = { D_WSTAG631_800A6608, D_WSTAG631_800A64FC, 306, 13, 272, 601, 1 };
FieldstgPlacedActor D_WSTAG631_800A6750 = { D_WSTAG631_800A6618, D_WSTAG631_800A6514, 307, 14, 303, 607, 1 };
FieldstgPlacedActor D_WSTAG631_800A6764 = { D_WSTAG631_800A6628, D_WSTAG631_800A652C, 308, 15, 321, 624, 1 };
FieldstgPlacedActor *wstag631_actors[17] = {
    &D_WSTAG631_800A6638, &D_WSTAG631_800A664C, &D_WSTAG631_800A6660, &D_WSTAG631_800A6674, &D_WSTAG631_800A6688,
    &D_WSTAG631_800A669C, &D_WSTAG631_800A66B0, &D_WSTAG631_800A66C4, &D_WSTAG631_800A66D8, &D_WSTAG631_800A66EC,
    &D_WSTAG631_800A6700, &D_WSTAG631_800A6714, &D_WSTAG631_800A6728, &D_WSTAG631_800A673C, &D_WSTAG631_800A6750,
    &D_WSTAG631_800A6764, NULL,
};
FieldstgSprite wstag631_sprites[3] = {
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 541, 275, 326, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 364, 459, 509, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag631_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B2, 0x90, 0x540, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x340, 0xF0, 1, 0, 1, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag631_funcs = { wstag631_setup };
