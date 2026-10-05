#include "wstag.h"

/* WSTAG716: stage 0x2CE (fieldstg_stages). */

extern WstagFuncs wstag716_funcs;
extern FieldstgBattleLists wstag716_battle_lists;
extern FieldstgVramPlace wstag716_vram_places[];
extern FieldstgPlacedActor *wstag716_actors[];
extern FieldstgSprite wstag716_sprites[];
extern FieldstgMapEvent wstag716_map_events[];

void wstag716_update(WstagObject *obj) {
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

WstagObject *wstag716_start(void *arg0) {
    WstagObject *obj = object_new(wstag716_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag716_funcs.setup();
    return obj;
}

void wstag716_setup(void) {
    fieldstg_stage.background_file = 0x67F;
    fieldstg_stage.sprite_file = 0x06800000;
    fieldstg_stage.sprites = wstag716_sprites;
    fieldstg_stage.map_events = wstag716_map_events;
    fieldstg_stage.mask_file = 0x67E;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10700, 0xAF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag716_vram_places;
    fieldstg_stage.music = 0x3E;
    fieldstg_stage.sound = 0x60F80000;
    fieldstg_stage.actors = wstag716_actors;
    fieldstg_stage.battle_lists = &wstag716_battle_lists;
    fieldstg_attr.set_file(0, 0x06800001);
    fieldstg_attr.set_file(7, 0x06800002);
    fieldstg_attr.set_file(4, 0x06800003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag716_setup(void);

FieldstgListedBattle D_WSTAG716_800A5F90 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5F9C = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FA8 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FB4 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FC0 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FCC = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FD8 = { 160, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A5FE4 = { 160, 14, 0x60080000 };
FieldstgBattleList D_WSTAG716_800A5FF0 = {
    3,
    { &D_WSTAG716_800A5F90, &D_WSTAG716_800A5F9C, &D_WSTAG716_800A5FA8, &D_WSTAG716_800A5FB4, &D_WSTAG716_800A5FC0,
        &D_WSTAG716_800A5FCC, &D_WSTAG716_800A5FD8, &D_WSTAG716_800A5FE4 },
};
FieldstgListedBattle D_WSTAG716_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG716_800A6074 = {
    0,
    { &D_WSTAG716_800A6014, &D_WSTAG716_800A6020, &D_WSTAG716_800A602C, &D_WSTAG716_800A6038, &D_WSTAG716_800A6044,
        &D_WSTAG716_800A6050, &D_WSTAG716_800A605C, &D_WSTAG716_800A6068 },
};
FieldstgListedBattle D_WSTAG716_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG716_800A60F8 = {
    0,
    { &D_WSTAG716_800A6098, &D_WSTAG716_800A60A4, &D_WSTAG716_800A60B0, &D_WSTAG716_800A60BC, &D_WSTAG716_800A60C8,
        &D_WSTAG716_800A60D4, &D_WSTAG716_800A60E0, &D_WSTAG716_800A60EC },
};
FieldstgListedBattle D_WSTAG716_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6140 = { 333, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A614C = { 334, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG716_800A6164 = { 187, 14, 0x60080000 };
FieldstgListedBattle D_WSTAG716_800A6170 = { 186, 14, 0x60080000 };
FieldstgBattleList D_WSTAG716_800A617C = {
    0,
    { &D_WSTAG716_800A611C, &D_WSTAG716_800A6128, &D_WSTAG716_800A6134, &D_WSTAG716_800A6140, &D_WSTAG716_800A614C,
        &D_WSTAG716_800A6158, &D_WSTAG716_800A6164, &D_WSTAG716_800A6170 },
};
FieldstgBattleLists wstag716_battle_lists = {
    118, 0, 0, { &D_WSTAG716_800A5FF0, &D_WSTAG716_800A6074, &D_WSTAG716_800A60F8 }, &D_WSTAG716_800A617C,
};
FieldstgVramPlace wstag716_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 375, 0, 119, 352, 511 }, { 320, 256, 356, 364, 144, 108, 368, 511 },
    { 320, 256, 340, 374, 80, 118, 336, 510 }, { 320, 256, 328, 375, 32, 119, 352, 510 },
    { 320, 256, 364, 375, 176, 119, 368, 510 }, { 320, 256, 372, 375, 208, 119, 336, 509 },
    { 320, 256, 353, 404, 132, 148, 352, 509 }, { 320, 256, 320, 407, 0, 151, 368, 509 },
};
FieldstgTalk D_WSTAG716_800A629C[2] = { { NULL, NULL, 661 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A62B4[2] = { { NULL, NULL, 663 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A62CC[2] = { { NULL, NULL, 657 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A62E4[2] = { { NULL, NULL, 659 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A62FC[2] = { { NULL, NULL, 664 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A6314[2] = { { NULL, NULL, 658 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A632C[2] = { { NULL, NULL, 658 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A6344[2] = { { NULL, NULL, 662 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A635C[2] = { { NULL, NULL, 662 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A6374[2] = { { NULL, NULL, 660 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A638C[2] = { { NULL, NULL, 660 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG716_800A63A4[2] = { { NULL, NULL, 665 }, { NULL, NULL, 0 } };
u16 D_WSTAG716_800A63BC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A63C8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A63D0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A63DC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A63E8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A63F0[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG716_800A63FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A6404[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG716_800A6410[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A6418[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG716_800A6424[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG716_800A642C[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG716_800A6434 = { D_WSTAG716_800A63BC, D_WSTAG716_800A629C, 46, 4, 272, 209, 7 };
FieldstgPlacedActor D_WSTAG716_800A6448 = { D_WSTAG716_800A63C8, D_WSTAG716_800A62B4, 46, 4, 529, 392, 1 };
FieldstgPlacedActor D_WSTAG716_800A645C = { D_WSTAG716_800A63D0, D_WSTAG716_800A62CC, 47, 5, 432, 392, 3 };
FieldstgPlacedActor D_WSTAG716_800A6470 = { D_WSTAG716_800A63DC, D_WSTAG716_800A62E4, 51, 6, 257, 432, 5 };
FieldstgPlacedActor D_WSTAG716_800A6484 = { D_WSTAG716_800A63E8, D_WSTAG716_800A62FC, 65, 7, 432, 392, 7 };
FieldstgPlacedActor D_WSTAG716_800A6498 = { D_WSTAG716_800A63F0, D_WSTAG716_800A6314, 157, 8, 432, 392, 3 };
FieldstgPlacedActor D_WSTAG716_800A64AC = { D_WSTAG716_800A63FC, D_WSTAG716_800A632C, 157, 8, 432, 392, 3 };
FieldstgPlacedActor D_WSTAG716_800A64C0 = { D_WSTAG716_800A6404, D_WSTAG716_800A6344, 158, 9, 272, 209, 7 };
FieldstgPlacedActor D_WSTAG716_800A64D4 = { D_WSTAG716_800A6410, D_WSTAG716_800A635C, 158, 9, 272, 209, 7 };
FieldstgPlacedActor D_WSTAG716_800A64E8 = { D_WSTAG716_800A6418, D_WSTAG716_800A6374, 159, 10, 257, 432, 5 };
FieldstgPlacedActor D_WSTAG716_800A64FC = { D_WSTAG716_800A6424, D_WSTAG716_800A638C, 159, 10, 257, 432, 5 };
FieldstgPlacedActor D_WSTAG716_800A6510 = { D_WSTAG716_800A642C, D_WSTAG716_800A63A4, 235, 11, 512, 520, 1 };
FieldstgPlacedActor *wstag716_actors[13] = {
    &D_WSTAG716_800A6434, &D_WSTAG716_800A6448, &D_WSTAG716_800A645C, &D_WSTAG716_800A6470, &D_WSTAG716_800A6484,
    &D_WSTAG716_800A6498, &D_WSTAG716_800A64AC, &D_WSTAG716_800A64C0, &D_WSTAG716_800A64D4, &D_WSTAG716_800A64E8,
    &D_WSTAG716_800A64FC, &D_WSTAG716_800A6510, NULL,
};
FieldstgSprite wstag716_sprites[24] = {
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 49, 138, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 128, 512, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 222, 441, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 600, 352, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 615, 376, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x38, 4, 0, 640, 368, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 5, 6, 0, 21, 133, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 800, 511, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x38, 4, 0, 243, 53, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x38, 4, 0, 270, 56, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 5, 6, 0, 234, 51, 0, 0 }, { 1, 0, 0x40, 4, 0x46, 0, 0, 0, 0, 0, 381, 406, 458, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 665, 307, 322, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 235, 368, 374, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 128, 120, 175, 0 }, { 1, 0, 0x4F, 4, 9, 0, 0, 0, 0, 0, 65, 377, 443, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 152, 152, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 352, 128, 128, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 199, 199, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 704, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 263, 263, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag716_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CD, 0xB4, 0x98, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CF, 0x298, 0x134, 3, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0x10, 1 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 0xF, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x170, 0x98, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x180, 0xE0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x48, 0xFFDC, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFC8, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag716_funcs = { wstag716_setup };
