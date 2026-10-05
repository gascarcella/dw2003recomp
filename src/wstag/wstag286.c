#include "wstag.h"

/* WSTAG286: stage 0x283 (fieldstg_stages). */

extern WstagFuncs wstag286_funcs;
extern FieldstgBattleLists wstag286_battle_lists;
extern FieldstgVramPlace wstag286_vram_places[];
extern FieldstgPlacedActor *wstag286_actors[];
extern FieldstgSprite wstag286_sprites[];
extern FieldstgMapEvent wstag286_map_events[];

void wstag286_update(WstagObject *obj) {
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

WstagObject *wstag286_start(void *arg0) {
    WstagObject *obj = object_new(wstag286_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag286_funcs.setup();
    return obj;
}

void wstag286_setup(void) {
    fieldstg_stage.background_file = 0x51E;
    fieldstg_stage.sprite_file = 0x051F0000;
    fieldstg_stage.sprites = wstag286_sprites;
    fieldstg_stage.map_events = wstag286_map_events;
    fieldstg_stage.mask_file = 0x51D;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1F300, 0x27F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag286_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag286_actors;
    fieldstg_stage.battle_lists = &wstag286_battle_lists;
    fieldstg_attr.set_file(0, 0x051F0001);
    fieldstg_attr.set_file(7, 0x051F0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag286_setup(void);

FieldstgListedBattle D_WSTAG286_800A5FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A5FDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A5FE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A5FF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6018 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG286_800A6024 = {
    0,
    { &D_WSTAG286_800A5FC4, &D_WSTAG286_800A5FD0, &D_WSTAG286_800A5FDC, &D_WSTAG286_800A5FE8, &D_WSTAG286_800A5FF4,
        &D_WSTAG286_800A6000, &D_WSTAG286_800A600C, &D_WSTAG286_800A6018 },
};
FieldstgListedBattle D_WSTAG286_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A606C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6078 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A609C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG286_800A60A8 = {
    0,
    { &D_WSTAG286_800A6048, &D_WSTAG286_800A6054, &D_WSTAG286_800A6060, &D_WSTAG286_800A606C, &D_WSTAG286_800A6078,
        &D_WSTAG286_800A6084, &D_WSTAG286_800A6090, &D_WSTAG286_800A609C },
};
FieldstgListedBattle D_WSTAG286_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6120 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG286_800A612C = {
    0,
    { &D_WSTAG286_800A60CC, &D_WSTAG286_800A60D8, &D_WSTAG286_800A60E4, &D_WSTAG286_800A60F0, &D_WSTAG286_800A60FC,
        &D_WSTAG286_800A6108, &D_WSTAG286_800A6114, &D_WSTAG286_800A6120 },
};
FieldstgListedBattle D_WSTAG286_800A6150 = { 196, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG286_800A615C = { 197, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG286_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG286_800A61A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG286_800A61B0 = {
    0,
    { &D_WSTAG286_800A6150, &D_WSTAG286_800A615C, &D_WSTAG286_800A6168, &D_WSTAG286_800A6174, &D_WSTAG286_800A6180,
        &D_WSTAG286_800A618C, &D_WSTAG286_800A6198, &D_WSTAG286_800A61A4 },
};
FieldstgBattleLists wstag286_battle_lists = {
    145, 0, 0, { &D_WSTAG286_800A6024, &D_WSTAG286_800A60A8, &D_WSTAG286_800A612C }, &D_WSTAG286_800A61B0,
};
FieldstgVramPlace wstag286_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 352, 511 }, { 320, 256, 368, 256, 192, 0, 368, 511 },
    { 320, 256, 368, 304, 192, 48, 352, 510 }, { 320, 256, 370, 473, 200, 217, 368, 510 },
    { 384, 256, 438, 376, 472, 120, 336, 509 }, { 320, 256, 370, 433, 200, 177, 352, 509 },
    { 384, 256, 434, 256, 456, 0, 368, 509 }, { 384, 256, 434, 296, 456, 40, 336, 508 },
    { 384, 256, 438, 336, 472, 80, 352, 508 },
};
u16 D_WSTAG286_800A62E0[8] = { 0x25F, 1, 0x822D, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A62F0[6] = { 0xC26, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A62FC[6] = { 0xC27, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6308[6] = { 0xC27, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6314[6] = { 0xC25, 1, 0x7400, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG286_800A6320[2] = { { NULL, D_WSTAG286_800A62E0, 366 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A6338[2] = { { NULL, NULL, 163 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A6350[2] = { { NULL, NULL, 165 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A6368[2] = { { NULL, NULL, 164 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A6380[2] = { { NULL, NULL, 166 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A6398[2] = { { NULL, D_WSTAG286_800A62F0, 444 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A63B0[2] = { { NULL, D_WSTAG286_800A62FC, 446 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A63C8[2] = { { NULL, D_WSTAG286_800A6308, 447 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG286_800A63E0[2] = { { NULL, D_WSTAG286_800A6314, 445 }, { NULL, NULL, 0 } };
u16 D_WSTAG286_800A63F8[4] = { 0x25F, 0, 0xFFFF, 0 };
u16 D_WSTAG286_800A6400[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6408[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6410[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6418[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6420[6] = { 0xC26, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A642C[6] = { 0xC27, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6438[6] = { 0xC27, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG286_800A6444[6] = { 0xC25, 0, 0x6025, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG286_800A6450 = { D_WSTAG286_800A63F8, D_WSTAG286_800A6320, 33, 4, 825, 285, 1 };
FieldstgPlacedActor D_WSTAG286_800A6464 = { D_WSTAG286_800A6400, D_WSTAG286_800A6338, 37, 5, 560, 673, 1 };
FieldstgPlacedActor D_WSTAG286_800A6478 = { D_WSTAG286_800A6408, D_WSTAG286_800A6350, 38, 6, 371, 574, 7 };
FieldstgPlacedActor D_WSTAG286_800A648C = { D_WSTAG286_800A6410, D_WSTAG286_800A6368, 157, 7, 560, 673, 1 };
FieldstgPlacedActor D_WSTAG286_800A64A0 = { D_WSTAG286_800A6418, D_WSTAG286_800A6380, 158, 8, 296, 532, 7 };
FieldstgPlacedActor D_WSTAG286_800A64B4 = { D_WSTAG286_800A6420, D_WSTAG286_800A6398, 299, 9, 823, 819, 3 };
FieldstgPlacedActor D_WSTAG286_800A64C8 = { D_WSTAG286_800A642C, D_WSTAG286_800A63B0, 301, 10, 371, 574, 3 };
FieldstgPlacedActor D_WSTAG286_800A64DC = { D_WSTAG286_800A6438, D_WSTAG286_800A63C8, 302, 11, 349, 586, 7 };
FieldstgPlacedActor D_WSTAG286_800A64F0 = { D_WSTAG286_800A6444, D_WSTAG286_800A63E0, 303, 12, 438, 244, 1 };
FieldstgPlacedActor *wstag286_actors[10] = {
    &D_WSTAG286_800A6450, &D_WSTAG286_800A6464, &D_WSTAG286_800A6478, &D_WSTAG286_800A648C, &D_WSTAG286_800A64A0,
    &D_WSTAG286_800A64B4, &D_WSTAG286_800A64C8, &D_WSTAG286_800A64DC, &D_WSTAG286_800A64F0, NULL,
};
FieldstgSprite wstag286_sprites[44] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 5, 8, 0, 437, 121, 0, 0 }, { 1, 0, 0x78, 2, 0x32, 2, 0, 1, 4, 0, 121, 400, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 104, 383, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 212, 501, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 264, 240, 0, 0 }, { 1, 0, 0x78, 6, 0x33, 2, 0, 1, 4, 0, 116, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 104, 334, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 212, 215, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 263, 476, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 76, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 152, 217, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 203, 161, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 284, 121, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 748, 208, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 269, 421, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 348, 461, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 429, 501, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 509, 540, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 517, 161, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 597, 201, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 653, 613, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 733, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 813, 693, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 837, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 893, 733, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 2, 0, 5, 8, 0, 554, 542, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 384, 133, 0, 0 },
    { 1, 0, 0x78, 4, 0x32, 2, 0, 1, 4, 0, 172, 373, 385, 0 },
    { 1, 0, 0x40, 4, 0x34, 2, 0, 1, 4, 0, 156, 358, 385, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 833, 737, 808, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 850, 744, 800, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 753, 706, 752, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 672, 666, 712, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 448, 554, 600, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 368, 514, 560, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 288, 474, 520, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 174, 336, 385, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 608, 250, 296, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 528, 210, 256, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 449, 161, 233, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 466, 168, 224, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 408, 135, 186, 0 },
    { 1, 0, 0x78, 4, 0x33, 2, 0, 1, 4, 0, 167, 255, 385, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag286_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x286, 0x318, 0x1DC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x14A, 0xC4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x284, 0xC8, 0x7C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag286_funcs = { wstag286_setup };
