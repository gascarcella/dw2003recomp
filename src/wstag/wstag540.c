#include "wstag.h"

/* WSTAG540: stage 0x245 (fieldstg_stages). */

extern WstagFuncs wstag540_funcs;
extern FieldstgBattleLists wstag540_battle_lists;
extern FieldstgVramPlace wstag540_vram_places[];
extern FieldstgPlacedActor *wstag540_actors[];
extern FieldstgSprite wstag540_sprites[];
extern FieldstgMapEvent wstag540_map_events[];

void wstag540_update(WstagObject *obj) {
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

WstagObject *wstag540_start(void *arg0) {
    WstagObject *obj = object_new(wstag540_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag540_funcs.setup();
    return obj;
}

void wstag540_setup(void) {
    fieldstg_stage.background_file = 0x6F8;
    fieldstg_stage.sprite_file = 0x06F90000;
    fieldstg_stage.sprites = wstag540_sprites;
    fieldstg_stage.map_events = wstag540_map_events;
    fieldstg_stage.mask_file = 0x6F7;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15200, 0x1C000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag540_vram_places;
    fieldstg_stage.music = 0x13;
    fieldstg_stage.sound = 0x604C0000;
    fieldstg_stage.actors = wstag540_actors;
    fieldstg_stage.battle_lists = &wstag540_battle_lists;
    fieldstg_attr.set_file(0, 0x06F90002);
    fieldstg_attr.set_file(1, 0x06F90001);
    fieldstg_attr.set_file(7, 0x06F90003);
    fieldstg_attr.set_file(4, 0x06F90004);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag540_setup(void);

FieldstgListedBattle D_WSTAG540_800A5FAC = { 165, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FB8 = { 165, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FC4 = { 173, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FD0 = { 173, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FDC = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FE8 = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A5FF4 = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG540_800A6000 = { 92, 10, 0x60080000 };
FieldstgBattleList D_WSTAG540_800A600C = {
    4,
    { &D_WSTAG540_800A5FAC, &D_WSTAG540_800A5FB8, &D_WSTAG540_800A5FC4, &D_WSTAG540_800A5FD0, &D_WSTAG540_800A5FDC,
        &D_WSTAG540_800A5FE8, &D_WSTAG540_800A5FF4, &D_WSTAG540_800A6000 },
};
FieldstgListedBattle D_WSTAG540_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A606C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6078 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6084 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG540_800A6090 = {
    0,
    { &D_WSTAG540_800A6030, &D_WSTAG540_800A603C, &D_WSTAG540_800A6048, &D_WSTAG540_800A6054, &D_WSTAG540_800A6060,
        &D_WSTAG540_800A606C, &D_WSTAG540_800A6078, &D_WSTAG540_800A6084 },
};
FieldstgListedBattle D_WSTAG540_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6108 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG540_800A6114 = {
    0,
    { &D_WSTAG540_800A60B4, &D_WSTAG540_800A60C0, &D_WSTAG540_800A60CC, &D_WSTAG540_800A60D8, &D_WSTAG540_800A60E4,
        &D_WSTAG540_800A60F0, &D_WSTAG540_800A60FC, &D_WSTAG540_800A6108 },
};
FieldstgListedBattle D_WSTAG540_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG540_800A618C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG540_800A6198 = {
    0,
    { &D_WSTAG540_800A6138, &D_WSTAG540_800A6144, &D_WSTAG540_800A6150, &D_WSTAG540_800A615C, &D_WSTAG540_800A6168,
        &D_WSTAG540_800A6174, &D_WSTAG540_800A6180, &D_WSTAG540_800A618C },
};
FieldstgBattleLists wstag540_battle_lists = {
    60, 0, 0, { &D_WSTAG540_800A600C, &D_WSTAG540_800A6090, &D_WSTAG540_800A6114 }, &D_WSTAG540_800A6198,
};
FieldstgVramPlace wstag540_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 472, 208, 216, 320, 506 }, { 384, 256, 416, 360, 384, 104, 336, 506 },
    { 384, 256, 422, 360, 408, 104, 352, 506 },
};
u16 D_WSTAG540_800A6268[8] = { 0x218, 1, 0x8AFC, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG540_800A6278[8] = { 0x219, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG540_800A6288[8] = { 0x708F, 1, 0x21A, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG540_800A6298[2] = { { NULL, D_WSTAG540_800A6268, 609 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG540_800A62B0[2] = { { NULL, D_WSTAG540_800A6278, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG540_800A62C8[2] = { { NULL, D_WSTAG540_800A6288, 813 }, { NULL, NULL, 0 } };
u16 D_WSTAG540_800A62E0[4] = { 0x218, 0, 0xFFFF, 0 };
u16 D_WSTAG540_800A62E8[4] = { 0x219, 0, 0xFFFF, 0 };
u16 D_WSTAG540_800A62F0[4] = { 0x21A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG540_800A62F8 = { D_WSTAG540_800A62E0, D_WSTAG540_800A6298, 33, 4, 369, 1218, 1 };
FieldstgPlacedActor D_WSTAG540_800A630C = { D_WSTAG540_800A62E8, D_WSTAG540_800A62B0, 77, 5, 1072, 601, 1 };
FieldstgPlacedActor D_WSTAG540_800A6320 = { D_WSTAG540_800A62F0, D_WSTAG540_800A62C8, 78, 6, 1313, 1138, 1 };
FieldstgPlacedActor *wstag540_actors[4] = {
    &D_WSTAG540_800A62F8, &D_WSTAG540_800A630C, &D_WSTAG540_800A6320, NULL,
};
FieldstgSprite wstag540_sprites[46] = {
    { 1, 0, 0x64, 2, 0xD, 0, 0, 0, 0, 0, 358, 319, 0, 0 }, { 1, 0, 0x64, 2, 0xE, 0, 0, 0, 0, 0, 508, 332, 0, 0 },
    { 1, 0, 0x64, 2, 0xF, 0, 0, 0, 0, 0, 887, 399, 0, 0 }, { 1, 0, 0x64, 2, 0xF, 0, 0, 0, 0, 0, 1007, 403, 0, 0 },
    { 1, 0, 0x64, 2, 0x10, 0, 0, 0, 0, 0, 800, 411, 0, 0 }, { 1, 0, 0x64, 2, 0x10, 0, 0, 0, 0, 0, 1159, 424, 0, 0 },
    { 1, 0, 0x64, 2, 0x11, 0, 0, 0, 0, 0, 1126, 413, 0, 0 },
    { 1, 0, 0x50, 2, 0x12, 0, 0, 0, 0, 0, 1266, 453, 0, 0 },
    { 1, 0, 0xDC, 2, 0x13, 0, 0, 0, 0, 0, 1360, 483, 0, 0 }, { 1, 0, 0x64, 2, 0, 0, 0, 0, 0, 0, 502, 965, 0, 0 },
    { 1, 0, 0x64, 2, 0, 0, 0, 0, 0, 0, 619, 747, 0, 0 }, { 1, 0, 0x64, 2, 0, 0, 0, 0, 0, 0, 668, 882, 0, 0 },
    { 1, 0, 0x64, 2, 1, 0, 0, 0, 0, 0, 368, 1064, 0, 0 }, { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 514, 921, 0, 0 },
    { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 538, 877, 0, 0 }, { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 561, 834, 0, 0 },
    { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 631, 702, 0, 0 }, { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 680, 838, 0, 0 },
    { 1, 0, 0x64, 2, 2, 0, 0, 0, 0, 0, 704, 794, 0, 0 }, { 1, 0, 0x64, 2, 3, 0, 0, 0, 0, 0, 624, 866, 0, 0 },
    { 1, 0, 0x64, 2, 3, 0, 0, 0, 0, 0, 647, 822, 0, 0 }, { 1, 0, 0x64, 2, 3, 0, 0, 0, 0, 0, 671, 779, 0, 0 },
    { 1, 0, 0x64, 2, 4, 0, 0, 0, 0, 0, 593, 850, 0, 0 }, { 1, 0, 0x64, 2, 4, 0, 0, 0, 0, 0, 617, 806, 0, 0 },
    { 1, 0, 0x64, 2, 4, 0, 0, 0, 0, 0, 640, 762, 0, 0 }, { 1, 0, 0x64, 2, 4, 0, 0, 0, 0, 0, 663, 719, 0, 0 },
    { 1, 0, 0xA0, 2, 5, 0, 0, 0, 0, 0, 687, 683, 0, 0 }, { 1, 0, 0x8C, 2, 6, 0, 0, 0, 0, 0, 847, 715, 0, 0 },
    { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 939, 866, 0, 0 }, { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 1035, 914, 0, 0 },
    { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 1155, 918, 0, 0 }, { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 1297, 879, 0, 0 },
    { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 1394, 927, 0, 0 }, { 1, 0, 0x64, 2, 7, 0, 0, 0, 0, 0, 1513, 931, 0, 0 },
    { 1, 0, 0x64, 2, 8, 0, 0, 0, 0, 0, 932, 807, 0, 0 }, { 1, 0, 0x64, 2, 8, 0, 0, 0, 0, 0, 1004, 899, 0, 0 },
    { 1, 0, 0x64, 2, 8, 0, 0, 0, 0, 0, 1362, 912, 0, 0 }, { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 920, 828, 0, 0 },
    { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 1111, 924, 0, 0 }, { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 1255, 885, 0, 0 },
    { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 1351, 933, 0, 0 }, { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 1470, 937, 0, 0 },
    { 1, 0, 0x64, 2, 0xA, 0, 0, 0, 0, 0, 1199, 891, 0, 0 }, { 1, 0, 0x64, 2, 0xB, 0, 0, 0, 0, 0, 1226, 864, 0, 0 },
    { 1, 0, 0x64, 2, 0xC, 0, 0, 0, 0, 0, 521, 415, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag540_map_events[12] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x244, 0x40C, 0x1E4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x246, 0x98, 0xF4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x246, 0x98, 0x1E4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x246, 0x98, 0x2C4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x246, 0x98, 0x414, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x220, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x210, 0x1F8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x293, 0x98, 0x264, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag540_funcs = { wstag540_setup };
