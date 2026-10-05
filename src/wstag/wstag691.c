#include "wstag.h"

/* WSTAG691: stage 0x2C9 (fieldstg_stages). */

extern WstagFuncs wstag691_funcs;
extern FieldstgBattleLists wstag691_battle_lists;
extern FieldstgVramPlace wstag691_vram_places[];
extern FieldstgPlacedActor *wstag691_actors[];
extern FieldstgSprite wstag691_sprites[];
extern FieldstgMapEvent wstag691_map_events[];

void wstag691_update(WstagObject *obj) {
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

WstagObject *wstag691_start(void *arg0) {
    WstagObject *obj = object_new(wstag691_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag691_funcs.setup();
    return obj;
}

void wstag691_setup(void) {
    fieldstg_stage.background_file = 0x62C;
    fieldstg_stage.sprite_file = 0x06300000;
    fieldstg_stage.sprites = wstag691_sprites;
    fieldstg_stage.map_events = wstag691_map_events;
    fieldstg_stage.mask_file = 0x628;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x47400, 0x2B500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag691_vram_places;
    fieldstg_stage.music = 0x3D;
    fieldstg_stage.sound = 0x60F40000;
    fieldstg_stage.actors = wstag691_actors;
    fieldstg_stage.battle_lists = &wstag691_battle_lists;
    fieldstg_attr.set_file(0, 0x06300001);
    fieldstg_attr.set_file(7, 0x06300002);
    fieldstg_attr.set_file(4, 0x06300003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag691_setup(void);

FieldstgListedBattle D_WSTAG691_800A5F94 = { 136, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FA0 = { 136, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FAC = { 136, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FB8 = { 136, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FC4 = { 183, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FD0 = { 183, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FDC = { 183, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A5FE8 = { 183, 4, 0x60080000 };
FieldstgBattleList D_WSTAG691_800A5FF4 = {
    3,
    { &D_WSTAG691_800A5F94, &D_WSTAG691_800A5FA0, &D_WSTAG691_800A5FAC, &D_WSTAG691_800A5FB8, &D_WSTAG691_800A5FC4,
        &D_WSTAG691_800A5FD0, &D_WSTAG691_800A5FDC, &D_WSTAG691_800A5FE8 },
};
FieldstgListedBattle D_WSTAG691_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG691_800A6078 = {
    0,
    { &D_WSTAG691_800A6018, &D_WSTAG691_800A6024, &D_WSTAG691_800A6030, &D_WSTAG691_800A603C, &D_WSTAG691_800A6048,
        &D_WSTAG691_800A6054, &D_WSTAG691_800A6060, &D_WSTAG691_800A606C },
};
FieldstgListedBattle D_WSTAG691_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG691_800A60FC = {
    0,
    { &D_WSTAG691_800A609C, &D_WSTAG691_800A60A8, &D_WSTAG691_800A60B4, &D_WSTAG691_800A60C0, &D_WSTAG691_800A60CC,
        &D_WSTAG691_800A60D8, &D_WSTAG691_800A60E4, &D_WSTAG691_800A60F0 },
};
FieldstgListedBattle D_WSTAG691_800A6120 = { 243, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A612C = { 244, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A6138 = { 291, 4, 0x600C0000 };
FieldstgListedBattle D_WSTAG691_800A6144 = { 333, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG691_800A6168 = { 175, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG691_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG691_800A6180 = {
    0,
    { &D_WSTAG691_800A6120, &D_WSTAG691_800A612C, &D_WSTAG691_800A6138, &D_WSTAG691_800A6144, &D_WSTAG691_800A6150,
        &D_WSTAG691_800A615C, &D_WSTAG691_800A6168, &D_WSTAG691_800A6174 },
};
FieldstgBattleLists wstag691_battle_lists = {
    116, 0, 0, { &D_WSTAG691_800A5FF4, &D_WSTAG691_800A6078, &D_WSTAG691_800A60FC }, &D_WSTAG691_800A6180,
};
FieldstgVramPlace wstag691_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 373, 192, 117, 352, 510 }, { 320, 256, 372, 293, 208, 37, 368, 510 },
    { 320, 256, 362, 325, 168, 69, 352, 509 }, { 320, 256, 352, 333, 128, 77, 368, 509 },
    { 320, 256, 370, 333, 200, 77, 352, 508 }, { 320, 256, 360, 365, 160, 109, 368, 508 },
    { 320, 256, 352, 373, 128, 117, 320, 507 },
};
u16 D_WSTAG691_800A6290[8] = { 0x250, 1, 0x822C, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A62A0[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A62AC[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A62B8[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A62C4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A62CC[6] = { 0, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A62D8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A62E0[8] = { 0x7208, 0, 0, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A62F0[10] = {
    0x720A, 0, 0x7208, 1, 0, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A6304[4] = { 0x763C, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A630C[12] = {
    0xE2F, 0, 0x720A, 1, 0x7208, 1, 0, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A6324[6] = { 0x7401, 1, 0xE2F, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6330[14] = {
    0xE2F, 1, 0x720A, 1, 0x7208, 1, 0, 1,
    0x11, 0, 0x8014, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A634C[16] = {
    0x720B, 0, 0x8014, 1, 0xE2F, 1, 0x720A, 1,
    0x7208, 1, 0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A636C[16] = {
    0x720B, 1, 0x8014, 1, 0xE2F, 1, 0x720A, 1,
    0x7208, 1, 0, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A638C[4] = { 0x783C, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6394[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A639C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A63A4[6] = { 1, 1, 0x720B, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A63B0[8] = { 1, 1, 0x720B, 1, 0xE4F, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A63C0[6] = { 0xE4F, 1, 0x7401, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A63CC[10] = {
    1, 1, 0x720B, 1, 0xE4F, 1, 0x720D, 0,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A63E0[10] = {
    1, 1, 0x720B, 1, 0xE4F, 1, 0x720D, 1,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A63F4[4] = { 0x783C, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A63FC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6404[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A640C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6414[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A641C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6428[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6434[6] = { 0x10, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6440[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A644C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6454[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6460[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6468[8] = { 0x7208, 0, 1, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6478[10] = {
    0x720A, 0, 0x7208, 1, 1, 1, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A648C[4] = { 0x763B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6494[12] = {
    0xE2E, 0, 0x720A, 1, 0x7208, 1, 1, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A64AC[6] = { 0x7400, 1, 0xE2E, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A64B8[14] = {
    0x8014, 0, 0xE2E, 1, 0x720A, 1, 0x7208, 1,
    1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A64D4[16] = {
    0x720B, 0, 0x8014, 1, 0xE2E, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A64F4[16] = {
    0x720B, 1, 0x8014, 1, 0xE2E, 1, 0x720A, 1,
    0x7208, 1, 1, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG691_800A6514[4] = { 0x783B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A651C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6524[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A652C[6] = { 0, 1, 0x720B, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6538[8] = { 0, 1, 0x720B, 1, 0xE4E, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6548[6] = { 0xE4E, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6554[10] = {
    0x720D, 0, 0, 1, 0x720B, 1, 0xE4E, 1,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A6568[10] = {
    0xE4E, 1, 0x720D, 1, 0, 1, 0x720B, 1,
    0xFFFF, 0,
};
u16 D_WSTAG691_800A657C[4] = { 0x783B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6584[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A658C[4] = { 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6594[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A659C[4] = { 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A65A4[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
FieldstgTalk D_WSTAG691_800A65B0[2] = { { NULL, D_WSTAG691_800A6290, 381 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A65C8[2] = { { NULL, NULL, 546 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A65E0[2] = { { NULL, NULL, 544 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A65F8[10] = {
    { D_WSTAG691_800A62A0, D_WSTAG691_800A62AC, 586 }, { D_WSTAG691_800A62B8, D_WSTAG691_800A62C4, 585 },
    { D_WSTAG691_800A62CC, D_WSTAG691_800A62D8, 580 }, { D_WSTAG691_800A62E0, NULL, 581 },
    { D_WSTAG691_800A62F0, D_WSTAG691_800A6304, 582 }, { D_WSTAG691_800A630C, D_WSTAG691_800A6324, 583 },
    { D_WSTAG691_800A6330, NULL, 584 }, { D_WSTAG691_800A634C, NULL, 581 },
    { D_WSTAG691_800A636C, D_WSTAG691_800A638C, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A6670[6] = {
    { D_WSTAG691_800A6394, D_WSTAG691_800A639C, 588 }, { D_WSTAG691_800A63A4, NULL, 590 },
    { D_WSTAG691_800A63B0, D_WSTAG691_800A63C0, 589 }, { D_WSTAG691_800A63CC, NULL, 584 },
    { D_WSTAG691_800A63E0, D_WSTAG691_800A63F4, 591 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A66B8[4] = {
    { D_WSTAG691_800A63FC, NULL, 924 }, { D_WSTAG691_800A6404, D_WSTAG691_800A640C, 585 },
    { D_WSTAG691_800A6414, D_WSTAG691_800A641C, 586 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A66E8[2] = { { NULL, NULL, 587 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A6700[10] = {
    { D_WSTAG691_800A6428, D_WSTAG691_800A6434, 554 }, { D_WSTAG691_800A6440, D_WSTAG691_800A644C, 553 },
    { D_WSTAG691_800A6454, D_WSTAG691_800A6460, 548 }, { D_WSTAG691_800A6468, NULL, 549 },
    { D_WSTAG691_800A6478, D_WSTAG691_800A648C, 550 }, { D_WSTAG691_800A6494, D_WSTAG691_800A64AC, 551 },
    { D_WSTAG691_800A64B8, NULL, 552 }, { D_WSTAG691_800A64D4, NULL, 549 },
    { D_WSTAG691_800A64F4, D_WSTAG691_800A6514, 550 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A6778[6] = {
    { D_WSTAG691_800A651C, D_WSTAG691_800A6524, 556 }, { D_WSTAG691_800A652C, NULL, 558 },
    { D_WSTAG691_800A6538, D_WSTAG691_800A6548, 557 }, { D_WSTAG691_800A6554, NULL, 552 },
    { D_WSTAG691_800A6568, D_WSTAG691_800A657C, 559 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A67C0[4] = {
    { D_WSTAG691_800A6584, NULL, 925 }, { D_WSTAG691_800A658C, D_WSTAG691_800A6594, 553 },
    { D_WSTAG691_800A659C, D_WSTAG691_800A65A4, 554 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG691_800A67F0[2] = { { NULL, NULL, 555 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A6808[2] = { { NULL, NULL, 547 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A6820[2] = { { NULL, NULL, 547 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A6838[2] = { { NULL, NULL, 545 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG691_800A6850[2] = { { NULL, NULL, 545 }, { NULL, NULL, 0 } };
u16 D_WSTAG691_800A6868[4] = { 0x250, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A6870[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A687C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6888[6] = { 0x8192, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A6894[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A689C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68A4[6] = { 0x8192, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68B0[6] = { 0x7019, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68BC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68C4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68CC[6] = { 0x7019, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A68D8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A68E4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG691_800A68EC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG691_800A68F8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG691_800A6900 = { D_WSTAG691_800A6868, D_WSTAG691_800A65B0, 33, 4, 432, 521, 1 };
FieldstgPlacedActor D_WSTAG691_800A6914 = { D_WSTAG691_800A6870, D_WSTAG691_800A65C8, 47, 5, 737, 673, 5 };
FieldstgPlacedActor D_WSTAG691_800A6928 = { D_WSTAG691_800A687C, D_WSTAG691_800A65E0, 50, 6, 945, 249, 1 };
FieldstgPlacedActor D_WSTAG691_800A693C = { D_WSTAG691_800A6888, D_WSTAG691_800A65F8, 69, 7, 617, 589, 1 };
FieldstgPlacedActor D_WSTAG691_800A6950 = { D_WSTAG691_800A6894, D_WSTAG691_800A6670, 69, 7, 617, 589, 1 };
FieldstgPlacedActor D_WSTAG691_800A6964 = { D_WSTAG691_800A689C, D_WSTAG691_800A66B8, 69, 7, 617, 589, 1 };
FieldstgPlacedActor D_WSTAG691_800A6978 = { D_WSTAG691_800A68A4, D_WSTAG691_800A66E8, 69, 7, 617, 589, 1 };
FieldstgPlacedActor D_WSTAG691_800A698C = { D_WSTAG691_800A68B0, D_WSTAG691_800A6700, 70, 8, 851, 490, 1 };
FieldstgPlacedActor D_WSTAG691_800A69A0 = { D_WSTAG691_800A68BC, D_WSTAG691_800A6778, 70, 8, 851, 490, 1 };
FieldstgPlacedActor D_WSTAG691_800A69B4 = { D_WSTAG691_800A68C4, D_WSTAG691_800A67C0, 70, 8, 851, 490, 1 };
FieldstgPlacedActor D_WSTAG691_800A69C8 = { D_WSTAG691_800A68CC, D_WSTAG691_800A67F0, 70, 8, 851, 490, 1 };
FieldstgPlacedActor D_WSTAG691_800A69DC = { D_WSTAG691_800A68D8, D_WSTAG691_800A6808, 157, 9, 737, 673, 5 };
FieldstgPlacedActor D_WSTAG691_800A69F0 = { D_WSTAG691_800A68E4, D_WSTAG691_800A6820, 157, 9, 737, 673, 5 };
FieldstgPlacedActor D_WSTAG691_800A6A04 = { D_WSTAG691_800A68EC, D_WSTAG691_800A6838, 158, 10, 945, 249, 1 };
FieldstgPlacedActor D_WSTAG691_800A6A18 = { D_WSTAG691_800A68F8, D_WSTAG691_800A6850, 158, 10, 945, 249, 1 };
FieldstgPlacedActor *wstag691_actors[16] = {
    &D_WSTAG691_800A6900, &D_WSTAG691_800A6914, &D_WSTAG691_800A6928, &D_WSTAG691_800A693C, &D_WSTAG691_800A6950,
    &D_WSTAG691_800A6964, &D_WSTAG691_800A6978, &D_WSTAG691_800A698C, &D_WSTAG691_800A69A0, &D_WSTAG691_800A69B4,
    &D_WSTAG691_800A69C8, &D_WSTAG691_800A69DC, &D_WSTAG691_800A69F0, &D_WSTAG691_800A6A04, &D_WSTAG691_800A6A18,
    NULL,
};
FieldstgSprite wstag691_sprites[28] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 323, 826, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 690, 795, 0, 0 },
    { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 450, 288, 0, 0 }, { 1, 0, 0x40, 2, 0, 0, 0, 0, 0, 0, 745, 533, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 210, 424, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 277, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 788, 245, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 960, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 850, 660, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 939, 699, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 950, 624, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 997, 651, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 8, 0, 893, 649, 0, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 130, 97, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 434, 412, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 480, 645, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 513, 10, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 611, 294, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 628, 630, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 893, 695, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 1101, 650, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 1232, -8, 896, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 527, 527, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 512, 703, 703, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 375, 375, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1088, 207, 207, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag691_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CD, 0x4F0, 0x2B8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CA, 0xA0, 0x1D0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CA, 0xA0, 0x240, 7, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x17, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 9, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 5, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xD, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x27F, 0x280, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x26E, 0x2E8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x4C0, 0x1A0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x4B0, 0x208, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 0xA, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag691_funcs = { wstag691_setup };
