#include "wstag.h"

/* WSTAG721: stage 0x2CF (fieldstg_stages). */

extern WstagFuncs wstag721_funcs;
extern FieldstgBattleLists wstag721_battle_lists;
extern FieldstgVramPlace wstag721_vram_places[];
extern FieldstgPlacedActor *wstag721_actors[];
extern FieldstgSprite wstag721_sprites[];
extern FieldstgMapEvent wstag721_map_events[];

void wstag721_update(WstagObject *obj) {
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

WstagObject *wstag721_start(void *arg0) {
    WstagObject *obj = object_new(wstag721_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag721_funcs.setup();
    return obj;
}

void wstag721_setup(void) {
    fieldstg_stage.background_file = 0x675;
    fieldstg_stage.sprite_file = 0x06760000;
    fieldstg_stage.sprites = wstag721_sprites;
    fieldstg_stage.map_events = wstag721_map_events;
    fieldstg_stage.mask_file = 0x674;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x8C00, 0x2BB00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag721_vram_places;
    fieldstg_stage.music = 0x18;
    fieldstg_stage.sound = 0x60600000;
    fieldstg_stage.actors = wstag721_actors;
    fieldstg_stage.battle_lists = &wstag721_battle_lists;
    fieldstg_attr.set_file(0, 0x06760001);
    fieldstg_attr.set_file(7, 0x06760002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag721_setup(void);

FieldstgListedBattle D_WSTAG721_800A5F78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5F84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5F90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5F9C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5FA8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5FB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5FC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A5FCC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG721_800A5FD8 = {
    3,
    { &D_WSTAG721_800A5F78, &D_WSTAG721_800A5F84, &D_WSTAG721_800A5F90, &D_WSTAG721_800A5F9C, &D_WSTAG721_800A5FA8,
        &D_WSTAG721_800A5FB4, &D_WSTAG721_800A5FC0, &D_WSTAG721_800A5FCC },
};
FieldstgListedBattle D_WSTAG721_800A5FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6050 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG721_800A605C = {
    0,
    { &D_WSTAG721_800A5FFC, &D_WSTAG721_800A6008, &D_WSTAG721_800A6014, &D_WSTAG721_800A6020, &D_WSTAG721_800A602C,
        &D_WSTAG721_800A6038, &D_WSTAG721_800A6044, &D_WSTAG721_800A6050 },
};
FieldstgListedBattle D_WSTAG721_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A60D4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG721_800A60E0 = {
    0,
    { &D_WSTAG721_800A6080, &D_WSTAG721_800A608C, &D_WSTAG721_800A6098, &D_WSTAG721_800A60A4, &D_WSTAG721_800A60B0,
        &D_WSTAG721_800A60BC, &D_WSTAG721_800A60C8, &D_WSTAG721_800A60D4 },
};
FieldstgListedBattle D_WSTAG721_800A6104 = { 249, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG721_800A6110 = { 250, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG721_800A611C = { 297, 18, 0x600C0000 };
FieldstgListedBattle D_WSTAG721_800A6128 = { 298, 18, 0x600C0000 };
FieldstgListedBattle D_WSTAG721_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG721_800A6158 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG721_800A6164 = {
    0,
    { &D_WSTAG721_800A6104, &D_WSTAG721_800A6110, &D_WSTAG721_800A611C, &D_WSTAG721_800A6128, &D_WSTAG721_800A6134,
        &D_WSTAG721_800A6140, &D_WSTAG721_800A614C, &D_WSTAG721_800A6158 },
};
FieldstgBattleLists wstag721_battle_lists = {
    163, 0, 0, { &D_WSTAG721_800A5FD8, &D_WSTAG721_800A605C, &D_WSTAG721_800A60E0 }, &D_WSTAG721_800A6164,
};
FieldstgVramPlace wstag721_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 372, 200, 116, 352, 511 }, { 320, 256, 374, 313, 216, 57, 368, 511 },
    { 320, 256, 328, 374, 32, 118, 352, 510 }, { 320, 256, 320, 374, 0, 118, 368, 510 },
    { 320, 256, 336, 374, 64, 118, 352, 509 }, { 320, 256, 344, 374, 96, 118, 368, 509 },
};
u16 D_WSTAG721_800A6264[4] = { 0x868F, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A626C[6] = { 2, 0, 0x868F, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6278[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6280[8] = { 2, 1, 0x868F, 0, 0x848B, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6290[8] = { 2, 1, 0x868F, 0, 0x848B, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A62A0[10] = {
    0x868F, 1, 0x868E, 0, 0x848B, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG721_800A62B4[8] = { 0x11, 1, 0x10, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A62C4[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A62D0[8] = { 0x11, 1, 0x10, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A62E0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A62EC[8] = { 0x11, 1, 0x10, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A62FC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6304[8] = { 0x11, 1, 0x10, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6314[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A631C[8] = { 0, 0, 0x11, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A632C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6334[8] = { 0x11, 0, 0, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6344[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A634C[8] = { 0, 1, 0x7208, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A635C[10] = {
    0, 1, 0x7208, 1, 0x720A, 0, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG721_800A6370[4] = { 0x7641, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6378[12] = {
    0, 1, 0x7208, 1, 0xE34, 0, 0x720A, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A6390[6] = { 0xE34, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A639C[14] = {
    0, 1, 0x7208, 1, 0x8014, 0, 0x720A, 1,
    0xE34, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A63B8[16] = {
    0x720C, 0, 0, 1, 0x7208, 1, 0x8014, 1,
    0x720A, 1, 0xE34, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A63D8[16] = {
    0xE34, 1, 0x720C, 1, 0, 1, 0x7208, 1,
    0x8014, 1, 0x720A, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A63F8[4] = { 0x7841, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6400[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6408[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6410[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6418[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6420[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A642C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6434[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A643C[8] = { 0x11, 1, 0x10, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A644C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6458[8] = { 0x11, 1, 0x10, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6468[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6474[8] = { 0x11, 1, 0x10, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6484[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A648C[8] = { 0x11, 1, 0x10, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A649C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A64A4[8] = { 1, 0, 0x11, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A64B4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A64BC[8] = { 0x11, 0, 1, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A64CC[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A64D4[8] = { 1, 1, 0x7208, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A64E4[10] = {
    1, 1, 0x7208, 1, 0x720A, 0, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG721_800A64F8[4] = { 0x7642, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6500[12] = {
    1, 1, 0x7208, 1, 0xE35, 0, 0x720A, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A6518[6] = { 0xE35, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6524[14] = {
    1, 1, 0x7208, 1, 0x8014, 0, 0x720A, 1,
    0xE35, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A6540[16] = {
    1, 1, 0x7208, 1, 0x8014, 1, 0x720A, 1,
    0xE35, 1, 0x720C, 0, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A6560[16] = {
    1, 1, 0x7208, 1, 0x8014, 1, 0x720A, 1,
    0xE35, 1, 0x720C, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG721_800A6580[4] = { 0x7842, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A6588[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6590[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A6598[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A65A0[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65A8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A65B4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65BC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65C4[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A65D0[4] = { 0x940B, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65D8[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65E4[4] = { 0x940C, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A65EC[4] = { 0x940D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG721_800A65F4[5] = {
    { D_WSTAG721_800A6264, NULL, 774 }, { D_WSTAG721_800A626C, D_WSTAG721_800A6278, 775 },
    { D_WSTAG721_800A6280, NULL, 776 }, { D_WSTAG721_800A6290, D_WSTAG721_800A62A0, 777 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A6630[13] = {
    { D_WSTAG721_800A62B4, D_WSTAG721_800A62C4, 673 }, { D_WSTAG721_800A62D0, D_WSTAG721_800A62E0, 37 },
    { D_WSTAG721_800A62EC, D_WSTAG721_800A62FC, 672 }, { D_WSTAG721_800A6304, D_WSTAG721_800A6314, 36 },
    { D_WSTAG721_800A631C, D_WSTAG721_800A632C, 666 }, { D_WSTAG721_800A6334, D_WSTAG721_800A6344, 35 },
    { D_WSTAG721_800A634C, NULL, 668 }, { D_WSTAG721_800A635C, D_WSTAG721_800A6370, 669 },
    { D_WSTAG721_800A6378, D_WSTAG721_800A6390, 671 }, { D_WSTAG721_800A639C, NULL, 670 },
    { D_WSTAG721_800A63B8, NULL, 675 }, { D_WSTAG721_800A63D8, D_WSTAG721_800A63F8, 676 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A66CC[4] = {
    { D_WSTAG721_800A6400, NULL, 674 }, { D_WSTAG721_800A6408, D_WSTAG721_800A6410, 672 },
    { D_WSTAG721_800A6418, D_WSTAG721_800A6420, 673 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A66FC[3] = {
    { D_WSTAG721_800A642C, NULL, 674 }, { D_WSTAG721_800A6434, NULL, 35 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A6720[13] = {
    { D_WSTAG721_800A643C, D_WSTAG721_800A644C, 686 }, { D_WSTAG721_800A6458, D_WSTAG721_800A6468, 46 },
    { D_WSTAG721_800A6474, D_WSTAG721_800A6484, 685 }, { D_WSTAG721_800A648C, D_WSTAG721_800A649C, 45 },
    { D_WSTAG721_800A64A4, D_WSTAG721_800A64B4, 677 }, { D_WSTAG721_800A64BC, D_WSTAG721_800A64CC, 677 },
    { D_WSTAG721_800A64D4, NULL, 679 }, { D_WSTAG721_800A64E4, D_WSTAG721_800A64F8, 680 },
    { D_WSTAG721_800A6500, D_WSTAG721_800A6518, 682 }, { D_WSTAG721_800A6524, NULL, 686 },
    { D_WSTAG721_800A6540, NULL, 683 }, { D_WSTAG721_800A6560, D_WSTAG721_800A6580, 684 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A67BC[4] = {
    { D_WSTAG721_800A6588, NULL, 687 }, { D_WSTAG721_800A6590, D_WSTAG721_800A6598, 685 },
    { D_WSTAG721_800A65A0, D_WSTAG721_800A65A8, 686 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A67EC[3] = {
    { D_WSTAG721_800A65B4, NULL, 687 }, { D_WSTAG721_800A65BC, NULL, 44 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A6810[3] = {
    { D_WSTAG721_800A65C4, D_WSTAG721_800A65D0, 731 }, { D_WSTAG721_800A65D8, D_WSTAG721_800A65E4, 731 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG721_800A6834[2] = { { NULL, D_WSTAG721_800A65EC, 731 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG721_800A684C[2] = { { NULL, NULL, 688 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG721_800A6864[2] = { { NULL, NULL, 689 }, { NULL, NULL, 0 } };
u16 D_WSTAG721_800A687C[10] = {
    0x7049, 1, 0x7051, 1, 0x868E, 1, 0x868F, 0,
    0xFFFF, 0,
};
u16 D_WSTAG721_800A6890[6] = { 0x8192, 1, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A689C[6] = { 0x8192, 1, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68A8[6] = { 0x8192, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68B4[6] = { 0x8192, 1, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68C0[6] = { 0x8192, 1, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68CC[6] = { 0x8192, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68D8[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG721_800A68E0[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68E8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG721_800A68F0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG721_800A68F8 = { D_WSTAG721_800A687C, D_WSTAG721_800A65F4, 31, 4, 538, 145, 1 };
FieldstgPlacedActor D_WSTAG721_800A690C = { D_WSTAG721_800A6890, D_WSTAG721_800A6630, 54, 5, 320, 216, 1 };
FieldstgPlacedActor D_WSTAG721_800A6920 = { D_WSTAG721_800A689C, D_WSTAG721_800A66CC, 54, 5, 320, 216, 1 };
FieldstgPlacedActor D_WSTAG721_800A6934 = { D_WSTAG721_800A68A8, D_WSTAG721_800A66FC, 54, 5, 320, 216, 1 };
FieldstgPlacedActor D_WSTAG721_800A6948 = { D_WSTAG721_800A68B4, D_WSTAG721_800A6720, 57, 6, 203, 152, 1 };
FieldstgPlacedActor D_WSTAG721_800A695C = { D_WSTAG721_800A68C0, D_WSTAG721_800A67BC, 57, 6, 273, 89, 1 };
FieldstgPlacedActor D_WSTAG721_800A6970 = { D_WSTAG721_800A68CC, D_WSTAG721_800A67EC, 57, 6, 203, 152, 1 };
FieldstgPlacedActor D_WSTAG721_800A6984 = { D_WSTAG721_800A68D8, D_WSTAG721_800A6810, 140, 7, 124, 700, 7 };
FieldstgPlacedActor D_WSTAG721_800A6998 = { D_WSTAG721_800A68E0, D_WSTAG721_800A6834, 140, 7, 124, 700, 7 };
FieldstgPlacedActor D_WSTAG721_800A69AC = { D_WSTAG721_800A68E8, D_WSTAG721_800A684C, 157, 8, 320, 216, 1 };
FieldstgPlacedActor D_WSTAG721_800A69C0 = { D_WSTAG721_800A68F0, D_WSTAG721_800A6864, 158, 9, 203, 152, 1 };
FieldstgPlacedActor *wstag721_actors[12] = {
    &D_WSTAG721_800A68F8, &D_WSTAG721_800A690C, &D_WSTAG721_800A6920, &D_WSTAG721_800A6934, &D_WSTAG721_800A6948,
    &D_WSTAG721_800A695C, &D_WSTAG721_800A6970, &D_WSTAG721_800A6984, &D_WSTAG721_800A6998, &D_WSTAG721_800A69AC,
    &D_WSTAG721_800A69C0, NULL,
};
FieldstgSprite wstag721_sprites[53] = {
    { 1, 0, 0x76, 2, 2, 0, 0, 0, 0, 0, 15, 51, 0, 0 }, { 1, 0, 0x76, 2, 2, 0, 0, 0, 0, 0, 367, 644, 0, 0 },
    { 1, 0, 0x48, 2, 3, 0, 0, 0, 0, 0, 184, 716, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 106, 91, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 115, 78, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 120, 99, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 189, 348, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 192, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 209, 362, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 317, 142, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 326, 165, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 341, 120, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 361, 421, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 368, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 408, 678, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 421, 702, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 457, 290, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 468, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 502, 466, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 509, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 519, 66, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 522, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 550, 31, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 633, 234, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 703, 228, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 4, 0, 714, 256, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 103, 97, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 115, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 122, 106, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 188, 402, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 192, 354, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 207, 370, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 314, 148, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 328, 171, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 342, 125, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 360, 427, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 371, 449, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 409, 685, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 419, 707, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 454, 296, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 471, 276, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 505, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 506, 456, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 517, 71, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 523, 102, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 553, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 634, 239, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 702, 233, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 4, 0, 717, 264, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 257, 598, 631, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 385, 439, 488, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 495, 232, 286, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag721_map_events[12] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CE, 0xC8, 0xAC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x1B0, 0xE8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x1A0, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x100, 0x140, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0xF0, 0x198, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x161, 0x200, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x171, 0x258, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0x12, 0xA2, 0xB0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0x12, 0xB2, 0x1D8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 0x10, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag721_funcs = { wstag721_setup };
