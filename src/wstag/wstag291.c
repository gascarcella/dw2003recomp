#include "wstag.h"

/* WSTAG291: stage 0x284 (fieldstg_stages). */

extern WstagFuncs wstag291_funcs;
extern FieldstgBattleLists wstag291_battle_lists;
extern FieldstgVramPlace wstag291_vram_places[];
extern FieldstgPlacedActor *wstag291_actors[];
extern FieldstgSprite wstag291_sprites[];
extern FieldstgMapEvent wstag291_map_events[];

void wstag291_update(WstagObject *obj) {
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

WstagObject *wstag291_start(void *arg0) {
    WstagObject *obj = object_new(wstag291_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag291_funcs.setup();
    return obj;
}

void wstag291_setup(void) {
    fieldstg_stage.background_file = 0x4FC;
    fieldstg_stage.sprite_file = 0x04FD0000;
    fieldstg_stage.sprites = wstag291_sprites;
    fieldstg_stage.map_events = wstag291_map_events;
    fieldstg_stage.mask_file = 0x4FB;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x17000, 0x19D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag291_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag291_actors;
    fieldstg_stage.battle_lists = &wstag291_battle_lists;
    fieldstg_attr.set_file(0, 0x04FD0001);
    fieldstg_attr.set_file(7, 0x04FD0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag291_setup(void);

FieldstgListedBattle D_WSTAG291_800A5F7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5F88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5F94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5FA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5FAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5FB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG291_800A5FDC = {
    0,
    { &D_WSTAG291_800A5F7C, &D_WSTAG291_800A5F88, &D_WSTAG291_800A5F94, &D_WSTAG291_800A5FA0, &D_WSTAG291_800A5FAC,
        &D_WSTAG291_800A5FB8, &D_WSTAG291_800A5FC4, &D_WSTAG291_800A5FD0 },
};
FieldstgListedBattle D_WSTAG291_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6054 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG291_800A6060 = {
    0,
    { &D_WSTAG291_800A6000, &D_WSTAG291_800A600C, &D_WSTAG291_800A6018, &D_WSTAG291_800A6024, &D_WSTAG291_800A6030,
        &D_WSTAG291_800A603C, &D_WSTAG291_800A6048, &D_WSTAG291_800A6054 },
};
FieldstgListedBattle D_WSTAG291_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A60D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG291_800A60E4 = {
    0,
    { &D_WSTAG291_800A6084, &D_WSTAG291_800A6090, &D_WSTAG291_800A609C, &D_WSTAG291_800A60A8, &D_WSTAG291_800A60B4,
        &D_WSTAG291_800A60C0, &D_WSTAG291_800A60CC, &D_WSTAG291_800A60D8 },
};
FieldstgListedBattle D_WSTAG291_800A6108 = { 196, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG291_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG291_800A615C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG291_800A6168 = {
    0,
    { &D_WSTAG291_800A6108, &D_WSTAG291_800A6114, &D_WSTAG291_800A6120, &D_WSTAG291_800A612C, &D_WSTAG291_800A6138,
        &D_WSTAG291_800A6144, &D_WSTAG291_800A6150, &D_WSTAG291_800A615C },
};
FieldstgBattleLists wstag291_battle_lists = {
    146, 0, 0, { &D_WSTAG291_800A5FDC, &D_WSTAG291_800A6060, &D_WSTAG291_800A60E4 }, &D_WSTAG291_800A6168,
};
FieldstgVramPlace wstag291_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 256, 256, 0, 368, 453 }, { 384, 256, 392, 256, 288, 0, 368, 452 },
    { 384, 256, 400, 256, 320, 0, 368, 451 }, { 384, 256, 400, 304, 320, 48, 368, 448 },
    { 384, 256, 424, 328, 416, 72, 368, 447 }, { 384, 256, 432, 328, 448, 72, 368, 446 },
    { 384, 256, 432, 256, 448, 0, 368, 444 }, { 384, 256, 408, 296, 352, 40, 368, 443 },
    { 384, 256, 416, 296, 384, 40, 368, 442 },
};
u16 D_WSTAG291_800A6298[6] = { 0x7400, 1, 0xC28, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A62A4[6] = { 0x7400, 1, 0xC2A, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A62B0[6] = { 0x7400, 1, 0xC29, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG291_800A62BC[2] = { { NULL, NULL, 167 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A62D4[2] = { { NULL, NULL, 168 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A62EC[2] = { { NULL, NULL, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A6304[2] = { { NULL, NULL, 170 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A631C[2] = { { NULL, NULL, 171 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A6334[2] = { { NULL, NULL, 172 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A634C[2] = { { NULL, D_WSTAG291_800A6298, 448 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A6364[2] = { { NULL, D_WSTAG291_800A62A4, 450 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG291_800A637C[2] = { { NULL, D_WSTAG291_800A62B0, 449 }, { NULL, NULL, 0 } };
u16 D_WSTAG291_800A6394[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A639C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A63A4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A63AC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A63B4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A63BC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG291_800A63C4[6] = { 0x6025, 1, 0xC28, 0, 0xFFFF, 0 };
u16 D_WSTAG291_800A63D0[6] = { 0x6025, 1, 0xC2A, 0, 0xFFFF, 0 };
u16 D_WSTAG291_800A63DC[6] = { 0x6025, 1, 0xC29, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG291_800A63E8 = { D_WSTAG291_800A6394, D_WSTAG291_800A62BC, 37, 4, 273, 361, 1 };
FieldstgPlacedActor D_WSTAG291_800A63FC = { D_WSTAG291_800A639C, D_WSTAG291_800A62D4, 38, 5, 817, 377, 3 };
FieldstgPlacedActor D_WSTAG291_800A6410 = { D_WSTAG291_800A63A4, D_WSTAG291_800A62EC, 39, 6, 479, 473, 1 };
FieldstgPlacedActor D_WSTAG291_800A6424 = { D_WSTAG291_800A63AC, D_WSTAG291_800A6304, 157, 7, 273, 361, 1 };
FieldstgPlacedActor D_WSTAG291_800A6438 = { D_WSTAG291_800A63B4, D_WSTAG291_800A631C, 158, 8, 817, 377, 3 };
FieldstgPlacedActor D_WSTAG291_800A644C = { D_WSTAG291_800A63BC, D_WSTAG291_800A6334, 159, 9, 479, 473, 1 };
FieldstgPlacedActor D_WSTAG291_800A6460 = { D_WSTAG291_800A63C4, D_WSTAG291_800A634C, 299, 10, 419, 250, 1 };
FieldstgPlacedActor D_WSTAG291_800A6474 = { D_WSTAG291_800A63D0, D_WSTAG291_800A6364, 301, 11, 209, 353, 7 };
FieldstgPlacedActor D_WSTAG291_800A6488 = { D_WSTAG291_800A63DC, D_WSTAG291_800A637C, 303, 12, 544, 249, 7 };
FieldstgPlacedActor *wstag291_actors[10] = {
    &D_WSTAG291_800A63E8, &D_WSTAG291_800A63FC, &D_WSTAG291_800A6410, &D_WSTAG291_800A6424, &D_WSTAG291_800A6438,
    &D_WSTAG291_800A644C, &D_WSTAG291_800A6460, &D_WSTAG291_800A6474, &D_WSTAG291_800A6488, NULL,
};
FieldstgSprite wstag291_sprites[35] = {
    { 1, 0, 0x40, 2, 0x40, 2, 0, 5, 8, 0, 171, 28, 0, 0 }, { 1, 0, 0x40, 2, 0x40, 2, 0, 5, 8, 0, 549, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 264, 45, 0, 0 }, { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 361, 94, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 614, 108, 0, 0 }, { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 699, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 0xD, 0x10, 0, 467, 144, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 0xD, 0x10, 0, 467, 144, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 0xD, 0x10, 0, 676, 280, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 0xD, 0x10, 0, 676, 280, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 8, 0x10, 0, 81, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x37, 2, 0, 8, 0x10, 0, 81, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 8, 0x10, 0, 789, 254, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 8, 0x10, 0, 789, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 166, 188, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 262, 236, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 294, 252, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 326, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 358, 284, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 390, 300, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 421, 316, 0, 0 }, { 1, 0, 0x40, 6, 0x3B, 2, 0, 3, 6, 0, 197, 203, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 3, 6, 0, 229, 217, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 6, 0, 451, 328, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 6, 0, 487, 328, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 3, 6, 0, 517, 321, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 2, 0, 3, 6, 0, 549, 305, 0, 0 }, { 1, 0x64, 0x40, 6, 6, 0, 0, 0, 0, 0, 81, 237, 0, 0 },
    { 1, 0x65, 0x40, 6, 5, 0, 0, 0, 0, 0, 161, 70, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 114, 71, 123, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 59, 239, 290, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 572, 463, 490, 0 },
    { 1, 0, 0x68, 4, 3, 0, 0, 0, 0, 0, 656, 281, 368, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 209, 283, 336, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag291_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x283, 0x386, 0x34A, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x289, 0x3A8, 0x13C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x274, 0x368, 0x18C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag291_funcs = { wstag291_setup };
