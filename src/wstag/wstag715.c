#include "wstag.h"

/* WSTAG715: stage 0x266 (fieldstg_stages). */

extern WstagFuncs wstag715_funcs;
extern FieldstgBattleLists wstag715_battle_lists;
extern FieldstgVramPlace wstag715_vram_places[];
extern FieldstgPlacedActor *wstag715_actors[];
extern FieldstgSprite wstag715_sprites[];
extern FieldstgMapEvent wstag715_map_events[];

void wstag715_update(WstagObject *obj) {
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

WstagObject *wstag715_start(void *arg0) {
    WstagObject *obj = object_new(wstag715_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag715_funcs.setup();
    return obj;
}

void wstag715_setup(void) {
    fieldstg_stage.background_file = 0x735;
    fieldstg_stage.sprite_file = 0x07360000;
    fieldstg_stage.sprites = wstag715_sprites;
    fieldstg_stage.map_events = wstag715_map_events;
    fieldstg_stage.mask_file = 0x734;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x36300, 0x1C700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag715_vram_places;
    fieldstg_stage.music = 0x3E;
    fieldstg_stage.sound = 0x60F80000;
    fieldstg_stage.actors = wstag715_actors;
    fieldstg_stage.battle_lists = &wstag715_battle_lists;
    fieldstg_attr.set_file(0, 0x07360001);
    fieldstg_attr.set_file(7, 0x07360002);
    fieldstg_attr.set_file(4, 0x07360003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag715_setup(void);

FieldstgListedBattle D_WSTAG715_800A5F94 = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FA0 = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FAC = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FB8 = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FC4 = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FD0 = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FDC = { 115, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A5FE8 = { 115, 14, 0x60080000 };
FieldstgBattleList D_WSTAG715_800A5FF4 = {
    3,
    { &D_WSTAG715_800A5F94, &D_WSTAG715_800A5FA0, &D_WSTAG715_800A5FAC, &D_WSTAG715_800A5FB8, &D_WSTAG715_800A5FC4,
        &D_WSTAG715_800A5FD0, &D_WSTAG715_800A5FDC, &D_WSTAG715_800A5FE8 },
};
FieldstgListedBattle D_WSTAG715_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG715_800A6078 = {
    0,
    { &D_WSTAG715_800A6018, &D_WSTAG715_800A6024, &D_WSTAG715_800A6030, &D_WSTAG715_800A603C, &D_WSTAG715_800A6048,
        &D_WSTAG715_800A6054, &D_WSTAG715_800A6060, &D_WSTAG715_800A606C },
};
FieldstgListedBattle D_WSTAG715_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG715_800A60FC = {
    0,
    { &D_WSTAG715_800A609C, &D_WSTAG715_800A60A8, &D_WSTAG715_800A60B4, &D_WSTAG715_800A60C0, &D_WSTAG715_800A60CC,
        &D_WSTAG715_800A60D8, &D_WSTAG715_800A60E4, &D_WSTAG715_800A60F0 },
};
FieldstgListedBattle D_WSTAG715_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6144 = { 333, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A6150 = { 332, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG715_800A6168 = { 114, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG715_800A6174 = { 179, 14, 0x60080000 };
FieldstgBattleList D_WSTAG715_800A6180 = {
    0,
    { &D_WSTAG715_800A6120, &D_WSTAG715_800A612C, &D_WSTAG715_800A6138, &D_WSTAG715_800A6144, &D_WSTAG715_800A6150,
        &D_WSTAG715_800A615C, &D_WSTAG715_800A6168, &D_WSTAG715_800A6174 },
};
FieldstgBattleLists wstag715_battle_lists = {
    85, 0, 0, { &D_WSTAG715_800A5FF4, &D_WSTAG715_800A6078, &D_WSTAG715_800A60FC }, &D_WSTAG715_800A6180,
};
FieldstgVramPlace wstag715_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 421, 160, 165, 352, 511 }, { 320, 256, 368, 421, 192, 165, 368, 511 },
    { 320, 256, 348, 424, 112, 168, 336, 510 }, { 320, 256, 320, 433, 0, 177, 352, 510 },
    { 320, 256, 328, 433, 32, 177, 368, 510 },
};
FieldstgTalk D_WSTAG715_800A6270[2] = { { NULL, NULL, 531 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6288[2] = { { NULL, NULL, 534 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A62A0[2] = { { NULL, NULL, 532 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A62B8[2] = { { NULL, NULL, 535 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A62D0[2] = { { NULL, NULL, 536 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A62E8[2] = { { NULL, NULL, 538 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6300[2] = { { NULL, NULL, 539 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6318[2] = { { NULL, NULL, 542 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6330[2] = { { NULL, NULL, 541 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6348[2] = { { NULL, NULL, 540 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6360[2] = { { NULL, NULL, 533 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG715_800A6378[2] = { { NULL, NULL, 537 }, { NULL, NULL, 0 } };
u16 D_WSTAG715_800A6390[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A6398[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63A0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63A8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63B0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63B8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63C0[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63C8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63D0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63D8[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63E0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG715_800A63E8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG715_800A63F0 = { D_WSTAG715_800A6390, D_WSTAG715_800A6270, 49, 4, 433, 393, 7 };
FieldstgPlacedActor D_WSTAG715_800A6404 = { D_WSTAG715_800A6398, D_WSTAG715_800A6288, 49, 4, 256, 431, 7 };
FieldstgPlacedActor D_WSTAG715_800A6418 = { D_WSTAG715_800A63A0, D_WSTAG715_800A62A0, 49, 4, 433, 393, 7 };
FieldstgPlacedActor D_WSTAG715_800A642C = { D_WSTAG715_800A63A8, D_WSTAG715_800A62B8, 53, 5, 272, 209, 5 };
FieldstgPlacedActor D_WSTAG715_800A6440 = { D_WSTAG715_800A63B0, D_WSTAG715_800A62D0, 53, 5, 272, 209, 5 };
FieldstgPlacedActor D_WSTAG715_800A6454 = { D_WSTAG715_800A63B8, D_WSTAG715_800A62E8, 53, 5, 512, 519, 1 };
FieldstgPlacedActor D_WSTAG715_800A6468 = { D_WSTAG715_800A63C0, D_WSTAG715_800A6300, 64, 6, 529, 392, 1 };
FieldstgPlacedActor D_WSTAG715_800A647C = { D_WSTAG715_800A63C8, D_WSTAG715_800A6318, 64, 6, 272, 209, 7 };
FieldstgPlacedActor D_WSTAG715_800A6490 = { D_WSTAG715_800A63D0, D_WSTAG715_800A6330, 64, 6, 529, 392, 1 };
FieldstgPlacedActor D_WSTAG715_800A64A4 = { D_WSTAG715_800A63D8, D_WSTAG715_800A6348, 64, 6, 529, 392, 1 };
FieldstgPlacedActor D_WSTAG715_800A64B8 = { D_WSTAG715_800A63E0, D_WSTAG715_800A6360, 157, 7, 433, 393, 7 };
FieldstgPlacedActor D_WSTAG715_800A64CC = { D_WSTAG715_800A63E8, D_WSTAG715_800A6378, 158, 8, 272, 209, 5 };
FieldstgPlacedActor *wstag715_actors[13] = {
    &D_WSTAG715_800A63F0, &D_WSTAG715_800A6404, &D_WSTAG715_800A6418, &D_WSTAG715_800A642C, &D_WSTAG715_800A6440,
    &D_WSTAG715_800A6454, &D_WSTAG715_800A6468, &D_WSTAG715_800A647C, &D_WSTAG715_800A6490, &D_WSTAG715_800A64A4,
    &D_WSTAG715_800A64B8, &D_WSTAG715_800A64CC, NULL,
};
FieldstgSprite wstag715_sprites[28] = {
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 49, 138, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 128, 512, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 222, 441, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 600, 352, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 615, 376, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 640, 368, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 5, 6, 0, 21, 133, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 4, 0, 801, 513, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x38, 4, 0, 243, 53, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x38, 4, 0, 270, 56, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 5, 6, 0, 234, 51, 0, 0 }, { 1, 0, 0x40, 4, 0x3B, 0, 0, 0, 0, 0, 406, 413, 478, 0 },
    { 1, 0, 0x40, 4, 0x3C, 0, 0, 0, 0, 0, 360, 401, 458, 0 },
    { 1, 0, 0x40, 4, 0x46, 1, 0x46, 0x51, 0xC, 0, 381, 406, 458, 0 },
    { 1, 0, 0x40, 4, 0x3E, 0, 0, 0, 0, 0, 406, 388, 438, 0 },
    { 1, 0, 0x40, 4, 0x3F, 0, 0, 0, 0, 0, 433, 401, 458, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 128, 120, 175, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 235, 368, 374, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 665, 307, 322, 0 },
    { 1, 0, 0x4F, 4, 9, 0, 0, 0, 0, 0, 65, 377, 445, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 152, 152, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 352, 128, 128, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 199, 199, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 704, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 263, 263, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag715_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x265, 0xB4, 0x98, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x267, 0x298, 0x134, 3, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 6, 1 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 5, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x170, 0x98, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x180, 0xE0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0xFFDC, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFC8, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag715_funcs = { wstag715_setup };
