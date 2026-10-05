#include "wstag.h"

/* WSTAG571: stage 0x2B5 (fieldstg_stages). */

extern WstagFuncs wstag571_funcs;
extern FieldstgBattleLists wstag571_battle_lists;
extern FieldstgVramPlace wstag571_vram_places[];
extern FieldstgPlacedActor *wstag571_actors[];
extern FieldstgSprite wstag571_sprites[];
extern FieldstgMapEvent wstag571_map_events[];

void wstag571_update(WstagObject *obj) {
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

WstagObject *wstag571_start(void *arg0) {
    WstagObject *obj = object_new(wstag571_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag571_funcs.setup();
    return obj;
}

void wstag571_setup(void) {
    fieldstg_stage.background_file = 0x5D9;
    fieldstg_stage.sprite_file = 0x05DA0000;
    fieldstg_stage.sprites = wstag571_sprites;
    fieldstg_stage.map_events = wstag571_map_events;
    fieldstg_stage.mask_file = 0x5D8;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xC900, 0xEB00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag571_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag571_actors;
    fieldstg_stage.battle_lists = &wstag571_battle_lists;
    fieldstg_attr.set_file(0, 0x05DA0001);
    fieldstg_attr.set_file(7, 0x05DA0002);
    fieldstg_attr.set_file(4, 0x05DA0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag571_setup(void);

FieldstgListedBattle D_WSTAG571_800A5FD4 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A5FE0 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A5FEC = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A5FF8 = { 132, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A6004 = { 133, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A6010 = { 133, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A601C = { 169, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A6028 = { 169, 3, 0x60080000 };
FieldstgBattleList D_WSTAG571_800A6034 = {
    3,
    { &D_WSTAG571_800A5FD4, &D_WSTAG571_800A5FE0, &D_WSTAG571_800A5FEC, &D_WSTAG571_800A5FF8, &D_WSTAG571_800A6004,
        &D_WSTAG571_800A6010, &D_WSTAG571_800A601C, &D_WSTAG571_800A6028 },
};
FieldstgListedBattle D_WSTAG571_800A6058 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6064 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A60AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG571_800A60B8 = {
    0,
    { &D_WSTAG571_800A6058, &D_WSTAG571_800A6064, &D_WSTAG571_800A6070, &D_WSTAG571_800A607C, &D_WSTAG571_800A6088,
        &D_WSTAG571_800A6094, &D_WSTAG571_800A60A0, &D_WSTAG571_800A60AC },
};
FieldstgListedBattle D_WSTAG571_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6130 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG571_800A613C = {
    0,
    { &D_WSTAG571_800A60DC, &D_WSTAG571_800A60E8, &D_WSTAG571_800A60F4, &D_WSTAG571_800A6100, &D_WSTAG571_800A610C,
        &D_WSTAG571_800A6118, &D_WSTAG571_800A6124, &D_WSTAG571_800A6130 },
};
FieldstgListedBattle D_WSTAG571_800A6160 = { 237, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A616C = { 285, 3, 0x600C0000 };
FieldstgListedBattle D_WSTAG571_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A6190 = { 334, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG571_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A61A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG571_800A61B4 = { 180, 2, 0x60080000 };
FieldstgBattleList D_WSTAG571_800A61C0 = {
    0,
    { &D_WSTAG571_800A6160, &D_WSTAG571_800A616C, &D_WSTAG571_800A6178, &D_WSTAG571_800A6184, &D_WSTAG571_800A6190,
        &D_WSTAG571_800A619C, &D_WSTAG571_800A61A8, &D_WSTAG571_800A61B4 },
};
FieldstgBattleLists wstag571_battle_lists = {
    96, 0, 0, { &D_WSTAG571_800A6034, &D_WSTAG571_800A60B8, &D_WSTAG571_800A613C }, &D_WSTAG571_800A61C0,
};
FieldstgVramPlace wstag571_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 400, 152, 144, 336, 511 }, { 320, 256, 374, 400, 216, 144, 352, 511 },
    { 320, 256, 366, 400, 184, 144, 368, 511 }, { 320, 256, 346, 376, 104, 120, 320, 510 },
    { 320, 256, 340, 424, 80, 168, 336, 510 }, { 320, 256, 348, 424, 112, 168, 352, 510 },
};
u16 D_WSTAG571_800A62C0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A62C8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A62D0[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A62DC[8] = { 0, 1, 0x7207, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A62EC[4] = { 0x7635, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A62F4[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE28, 0,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A6308[6] = { 0xE28, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A6314[10] = {
    0, 1, 0x7207, 1, 0x7209, 1, 0xE28, 1,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A6328[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A6330[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A6338[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A6344[8] = { 0, 1, 0x720A, 1, 0xE49, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A6354[6] = { 0x7400, 1, 0xE49, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A6360[10] = {
    0, 1, 0x720A, 1, 0xE49, 1, 0x720C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A6374[10] = {
    0, 1, 0x720A, 1, 0xE49, 1, 0x720C, 1,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A6388[4] = { 0x7835, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A6390[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A6398[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A63A0[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A63A8[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A63B0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A63BC[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A63C8[4] = { 0x9409, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A63D0[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A63DC[4] = { 0x940A, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A63E4[4] = { 0x940D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG571_800A63EC[2] = { { NULL, NULL, 448 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A6404[2] = { { NULL, NULL, 446 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A641C[6] = {
    { D_WSTAG571_800A62C0, D_WSTAG571_800A62C8, 580 }, { D_WSTAG571_800A62D0, NULL, 581 },
    { D_WSTAG571_800A62DC, D_WSTAG571_800A62EC, 582 }, { D_WSTAG571_800A62F4, D_WSTAG571_800A6308, 583 },
    { D_WSTAG571_800A6314, NULL, 584 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG571_800A6464[6] = {
    { D_WSTAG571_800A6328, D_WSTAG571_800A6330, 588 }, { D_WSTAG571_800A6338, NULL, 590 },
    { D_WSTAG571_800A6344, D_WSTAG571_800A6354, 589 }, { D_WSTAG571_800A6360, NULL, 584 },
    { D_WSTAG571_800A6374, D_WSTAG571_800A6388, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG571_800A64AC[4] = {
    { D_WSTAG571_800A6390, NULL, 925 }, { D_WSTAG571_800A6398, D_WSTAG571_800A63A0, 585 },
    { D_WSTAG571_800A63A8, D_WSTAG571_800A63B0, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG571_800A64DC[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A64F4[3] = {
    { D_WSTAG571_800A63BC, D_WSTAG571_800A63C8, 729 }, { D_WSTAG571_800A63D0, D_WSTAG571_800A63DC, 729 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG571_800A6518[2] = { { NULL, D_WSTAG571_800A63E4, 729 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A6530[2] = { { NULL, NULL, 447 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A6548[2] = { { NULL, NULL, 447 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A6560[2] = { { NULL, NULL, 449 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG571_800A6578[2] = { { NULL, NULL, 449 }, { NULL, NULL, 0 } };
u16 D_WSTAG571_800A6590[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A659C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A65A8[10] = {
    0x8192, 1, 0x11, 0, 0x7019, 1, 0x8014, 0,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A65BC[10] = {
    0x7019, 1, 0x8192, 1, 0x11, 0, 0x8014, 1,
    0xFFFF, 0,
};
u16 D_WSTAG571_800A65D0[8] = { 0x8192, 1, 0x11, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A65E0[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A65EC[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A65F4[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A65FC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A6608[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG571_800A6610[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG571_800A661C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG571_800A6624 = { D_WSTAG571_800A6590, D_WSTAG571_800A63EC, 50, 4, 748, 281, 7 };
FieldstgPlacedActor D_WSTAG571_800A6638 = { D_WSTAG571_800A659C, D_WSTAG571_800A6404, 58, 5, 537, 413, 1 };
FieldstgPlacedActor D_WSTAG571_800A664C = { D_WSTAG571_800A65A8, D_WSTAG571_800A641C, 69, 6, 333, 716, 1 };
FieldstgPlacedActor D_WSTAG571_800A6660 = { D_WSTAG571_800A65BC, D_WSTAG571_800A6464, 69, 6, 333, 716, 1 };
FieldstgPlacedActor D_WSTAG571_800A6674 = { D_WSTAG571_800A65D0, D_WSTAG571_800A64AC, 69, 6, 333, 716, 1 };
FieldstgPlacedActor D_WSTAG571_800A6688 = { D_WSTAG571_800A65E0, D_WSTAG571_800A64DC, 69, 6, 333, 716, 1 };
FieldstgPlacedActor D_WSTAG571_800A669C = { D_WSTAG571_800A65EC, D_WSTAG571_800A64F4, 139, 7, 730, 440, 7 };
FieldstgPlacedActor D_WSTAG571_800A66B0 = { D_WSTAG571_800A65F4, D_WSTAG571_800A6518, 139, 7, 730, 440, 7 };
FieldstgPlacedActor D_WSTAG571_800A66C4 = { D_WSTAG571_800A65FC, D_WSTAG571_800A6530, 157, 8, 537, 413, 1 };
FieldstgPlacedActor D_WSTAG571_800A66D8 = { D_WSTAG571_800A6608, D_WSTAG571_800A6548, 157, 8, 537, 413, 1 };
FieldstgPlacedActor D_WSTAG571_800A66EC = { D_WSTAG571_800A6610, D_WSTAG571_800A6560, 158, 9, 748, 281, 7 };
FieldstgPlacedActor D_WSTAG571_800A6700 = { D_WSTAG571_800A661C, D_WSTAG571_800A6578, 158, 9, 748, 281, 7 };
FieldstgPlacedActor *wstag571_actors[13] = {
    &D_WSTAG571_800A6624, &D_WSTAG571_800A6638, &D_WSTAG571_800A664C, &D_WSTAG571_800A6660, &D_WSTAG571_800A6674,
    &D_WSTAG571_800A6688, &D_WSTAG571_800A669C, &D_WSTAG571_800A66B0, &D_WSTAG571_800A66C4, &D_WSTAG571_800A66D8,
    &D_WSTAG571_800A66EC, &D_WSTAG571_800A6700, NULL,
};
FieldstgSprite wstag571_sprites[12] = {
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 153, 305, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 875, 481, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 391, 837, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 693, 860, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 826, 679, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 290, 854, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 602, 785, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 817, 793, 0, 0 },
    { 1, 0, 0x40, 6, 2, 0, 0, 0, 0, 0, 541, 351, 0, 0 }, { 1, 0, 0x40, 6, 3, 0, 0, 0, 0, 0, 480, 468, 0, 0 },
    { 1, 0, 0x5D, 4, 0, 0, 0, 0, 0, 0, 621, 413, 490, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag571_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B4, 0x570, 0x3C0, 3, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xB, 2 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xD, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x1C0, 0x272, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x1CF, 0x2D8, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE7, 0x46, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x19, 0x46, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xD, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag571_funcs = { wstag571_setup };
