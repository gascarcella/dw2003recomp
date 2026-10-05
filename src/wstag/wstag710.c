#include "wstag.h"

/* WSTAG710: stage 0x265 (fieldstg_stages). */

extern WstagFuncs wstag710_funcs;
extern FieldstgBattleLists wstag710_battle_lists;
extern FieldstgVramPlace wstag710_vram_places[];
extern FieldstgPlacedActor *wstag710_actors[];
extern FieldstgSprite wstag710_sprites[];
extern FieldstgMapEvent wstag710_map_events[];

void wstag710_update(WstagObject *obj) {
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

WstagObject *wstag710_start(void *arg0) {
    WstagObject *obj = object_new(wstag710_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag710_funcs.setup();
    return obj;
}

void wstag710_setup(void) {
    fieldstg_stage.background_file = 0x2A7;
    fieldstg_stage.sprite_file = 0x02A80000;
    fieldstg_stage.sprites = wstag710_sprites;
    fieldstg_stage.map_events = wstag710_map_events;
    fieldstg_stage.mask_file = 0x31D;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x43C00, 0x25900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag710_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag710_actors;
    fieldstg_stage.battle_lists = &wstag710_battle_lists;
    fieldstg_attr.set_file(0, 0x02A80001);
    fieldstg_attr.set_file(7, 0x02A80002);
    fieldstg_attr.set_file(4, 0x02A80003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag710_setup(void);

FieldstgListedBattle D_WSTAG710_800A5F94 = { 161, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FA0 = { 161, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FAC = { 161, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FB8 = { 110, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FC4 = { 110, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FD0 = { 153, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FDC = { 153, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A5FE8 = { 153, 4, 0x60080000 };
FieldstgBattleList D_WSTAG710_800A5FF4 = {
    3,
    { &D_WSTAG710_800A5F94, &D_WSTAG710_800A5FA0, &D_WSTAG710_800A5FAC, &D_WSTAG710_800A5FB8, &D_WSTAG710_800A5FC4,
        &D_WSTAG710_800A5FD0, &D_WSTAG710_800A5FDC, &D_WSTAG710_800A5FE8 },
};
FieldstgListedBattle D_WSTAG710_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG710_800A6078 = {
    0,
    { &D_WSTAG710_800A6018, &D_WSTAG710_800A6024, &D_WSTAG710_800A6030, &D_WSTAG710_800A603C, &D_WSTAG710_800A6048,
        &D_WSTAG710_800A6054, &D_WSTAG710_800A6060, &D_WSTAG710_800A606C },
};
FieldstgListedBattle D_WSTAG710_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG710_800A60FC = {
    0,
    { &D_WSTAG710_800A609C, &D_WSTAG710_800A60A8, &D_WSTAG710_800A60B4, &D_WSTAG710_800A60C0, &D_WSTAG710_800A60CC,
        &D_WSTAG710_800A60D8, &D_WSTAG710_800A60E4, &D_WSTAG710_800A60F0 },
};
FieldstgListedBattle D_WSTAG710_800A6120 = { 220, 4, 0x600C0000 };
FieldstgListedBattle D_WSTAG710_800A612C = { 221, 4, 0x600C0000 };
FieldstgListedBattle D_WSTAG710_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6144 = { 333, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG710_800A6168 = { 127, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG710_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG710_800A6180 = {
    0,
    { &D_WSTAG710_800A6120, &D_WSTAG710_800A612C, &D_WSTAG710_800A6138, &D_WSTAG710_800A6144, &D_WSTAG710_800A6150,
        &D_WSTAG710_800A615C, &D_WSTAG710_800A6168, &D_WSTAG710_800A6174 },
};
FieldstgBattleLists wstag710_battle_lists = {
    84, 0, 0, { &D_WSTAG710_800A5FF4, &D_WSTAG710_800A6078, &D_WSTAG710_800A60FC }, &D_WSTAG710_800A6180,
};
FieldstgVramPlace wstag710_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 436, 320, 464, 64, 352, 511 }, { 320, 256, 372, 449, 208, 193, 368, 511 },
    { 384, 256, 436, 256, 464, 0, 352, 510 }, { 384, 256, 436, 288, 464, 32, 368, 510 },
};
u16 D_WSTAG710_800A6260[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A626C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6278[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6284[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A628C[8] = { 0, 0, 0x7004, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A629C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A62A4[8] = { 0, 0, 0x6026, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A62B4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A62BC[8] = { 0, 1, 0x7206, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A62CC[10] = {
    0, 1, 0x7206, 1, 0x7208, 0, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A62E0[4] = { 0x761E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A62E8[12] = {
    0, 1, 0x7206, 1, 0xE14, 0, 0x7208, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6300[6] = { 0xE14, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A630C[14] = {
    0, 1, 0x7206, 1, 0x8012, 0, 0x7208, 1,
    0xE14, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6328[16] = {
    0xE14, 1, 0x720A, 0, 0, 1, 0x7206, 1,
    0x8012, 1, 0x7208, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6348[16] = {
    0, 1, 0x7206, 1, 0x8012, 1, 0x7208, 1,
    0xE14, 1, 0x720A, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6368[4] = { 0x781E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6370[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6378[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6384[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A638C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6398[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A63A4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A63AC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A63B4[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A63C0[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A63D0[4] = { 0x761E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A63D8[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A63EC[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6404[6] = { 0x7400, 1, 0xE14, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6410[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A642C[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6448[4] = { 0x781E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6450[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A645C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6468[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6474[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A647C[8] = { 1, 0, 0x11, 0, 0x7004, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A648C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6494[8] = { 0x11, 0, 1, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A64A4[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A64AC[8] = { 1, 1, 0x7206, 0, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A64BC[10] = {
    1, 1, 0x7206, 1, 0x7208, 0, 0x11, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A64D0[4] = { 0x761F, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A64D8[12] = {
    1, 1, 0x7206, 1, 0xE15, 0, 0x7208, 1,
    0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A64F0[6] = { 0xE15, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A64FC[14] = {
    1, 1, 0x7206, 1, 0x8012, 0, 0x7208, 1,
    0xE15, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6518[16] = {
    1, 1, 0x7206, 1, 0x8012, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 0, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6538[16] = {
    1, 1, 0x7206, 1, 0x8012, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 1, 0x11, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6558[4] = { 0x781F, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6560[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6568[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6574[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A657C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6588[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6594[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A659C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A65A4[6] = { 1, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A65B0[8] = { 1, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A65C0[4] = { 0x761F, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A65C8[10] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A65DC[12] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A65F4[6] = { 0x7400, 1, 0xE15, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6600[14] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A661C[14] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6638[4] = { 0x781F, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6640[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6648[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6654[8] = { 0, 1, 0x8014, 0, 0x7206, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6664[4] = { 0x761E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A666C[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A6680[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6698[4] = { 0xE14, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A66A0[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A66BC[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE14, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG710_800A66D8[4] = { 0x781E, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A66E0[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A66E8[6] = { 1, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A66F4[8] = { 1, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG710_800A6704[4] = { 0x761F, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A670C[10] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG710_800A6720[12] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6738[4] = { 0xE15, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6740[14] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG710_800A675C[14] = {
    1, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE15, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG710_800A6778[4] = { 0x781F, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG710_800A6780[11] = {
    { D_WSTAG710_800A6260, D_WSTAG710_800A626C, 391 }, { D_WSTAG710_800A6278, D_WSTAG710_800A6284, 390 },
    { D_WSTAG710_800A628C, D_WSTAG710_800A629C, 380 }, { D_WSTAG710_800A62A4, D_WSTAG710_800A62B4, 381 },
    { D_WSTAG710_800A62BC, NULL, 384 }, { D_WSTAG710_800A62CC, D_WSTAG710_800A62E0, 385 },
    { D_WSTAG710_800A62E8, D_WSTAG710_800A6300, 387 }, { D_WSTAG710_800A630C, NULL, 386 },
    { D_WSTAG710_800A6328, NULL, 388 }, { D_WSTAG710_800A6348, D_WSTAG710_800A6368, 389 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A6804[2] = { { NULL, NULL, 663 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG710_800A681C[4] = {
    { D_WSTAG710_800A6370, NULL, 380 }, { D_WSTAG710_800A6378, D_WSTAG710_800A6384, 390 },
    { D_WSTAG710_800A638C, D_WSTAG710_800A6398, 391 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A684C[8] = {
    { D_WSTAG710_800A63A4, D_WSTAG710_800A63AC, 381 }, { D_WSTAG710_800A63B4, NULL, 384 },
    { D_WSTAG710_800A63C0, D_WSTAG710_800A63D0, 385 }, { D_WSTAG710_800A63D8, NULL, 386 },
    { D_WSTAG710_800A63EC, D_WSTAG710_800A6404, 387 }, { D_WSTAG710_800A6410, NULL, 388 },
    { D_WSTAG710_800A642C, D_WSTAG710_800A6448, 389 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A68AC[11] = {
    { D_WSTAG710_800A6450, D_WSTAG710_800A645C, 403 }, { D_WSTAG710_800A6468, D_WSTAG710_800A6474, 402 },
    { D_WSTAG710_800A647C, D_WSTAG710_800A648C, 392 }, { D_WSTAG710_800A6494, D_WSTAG710_800A64A4, 393 },
    { D_WSTAG710_800A64AC, NULL, 396 }, { D_WSTAG710_800A64BC, D_WSTAG710_800A64D0, 397 },
    { D_WSTAG710_800A64D8, D_WSTAG710_800A64F0, 399 }, { D_WSTAG710_800A64FC, NULL, 398 },
    { D_WSTAG710_800A6518, NULL, 400 }, { D_WSTAG710_800A6538, D_WSTAG710_800A6558, 401 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A6930[2] = { { NULL, NULL, 664 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG710_800A6948[4] = {
    { D_WSTAG710_800A6560, NULL, 392 }, { D_WSTAG710_800A6568, D_WSTAG710_800A6574, 402 },
    { D_WSTAG710_800A657C, D_WSTAG710_800A6588, 403 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A6978[8] = {
    { D_WSTAG710_800A6594, D_WSTAG710_800A659C, 393 }, { D_WSTAG710_800A65A4, NULL, 396 },
    { D_WSTAG710_800A65B0, D_WSTAG710_800A65C0, 397 }, { D_WSTAG710_800A65C8, NULL, 398 },
    { D_WSTAG710_800A65DC, D_WSTAG710_800A65F4, 399 }, { D_WSTAG710_800A6600, NULL, 400 },
    { D_WSTAG710_800A661C, D_WSTAG710_800A6638, 401 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A69D8[8] = {
    { D_WSTAG710_800A6640, NULL, 382 }, { D_WSTAG710_800A6648, NULL, 382 },
    { D_WSTAG710_800A6654, D_WSTAG710_800A6664, 382 }, { D_WSTAG710_800A666C, NULL, 382 },
    { D_WSTAG710_800A6680, D_WSTAG710_800A6698, 382 }, { D_WSTAG710_800A66A0, NULL, 382 },
    { D_WSTAG710_800A66BC, D_WSTAG710_800A66D8, 382 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG710_800A6A38[8] = {
    { D_WSTAG710_800A66E0, NULL, 394 }, { D_WSTAG710_800A66E8, NULL, 394 },
    { D_WSTAG710_800A66F4, D_WSTAG710_800A6704, 394 }, { D_WSTAG710_800A670C, NULL, 394 },
    { D_WSTAG710_800A6720, D_WSTAG710_800A6738, 394 }, { D_WSTAG710_800A6740, NULL, 394 },
    { D_WSTAG710_800A675C, D_WSTAG710_800A6778, 394 }, { NULL, NULL, 0 },
};
u16 D_WSTAG710_800A6A98[6] = { 0x8192, 1, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AA4[6] = { 0x8192, 0, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AB0[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6ABC[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AC8[6] = { 0x7022, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AD4[6] = { 0x8192, 0, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AE0[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AEC[6] = { 0x602B, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6AF8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG710_800A6B00[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG710_800A6B08 = { D_WSTAG710_800A6A98, D_WSTAG710_800A6780, 45, 4, 464, 193, 1 };
FieldstgPlacedActor D_WSTAG710_800A6B1C = { D_WSTAG710_800A6AA4, D_WSTAG710_800A6804, 45, 4, 464, 193, 1 };
FieldstgPlacedActor D_WSTAG710_800A6B30 = { D_WSTAG710_800A6AB0, D_WSTAG710_800A681C, 45, 4, 464, 193, 1 };
FieldstgPlacedActor D_WSTAG710_800A6B44 = { D_WSTAG710_800A6ABC, D_WSTAG710_800A684C, 45, 4, 464, 193, 1 };
FieldstgPlacedActor D_WSTAG710_800A6B58 = { D_WSTAG710_800A6AC8, D_WSTAG710_800A68AC, 57, 5, 1088, 234, 7 };
FieldstgPlacedActor D_WSTAG710_800A6B6C = { D_WSTAG710_800A6AD4, D_WSTAG710_800A6930, 57, 5, 1088, 234, 7 };
FieldstgPlacedActor D_WSTAG710_800A6B80 = { D_WSTAG710_800A6AE0, D_WSTAG710_800A6948, 57, 5, 1088, 234, 7 };
FieldstgPlacedActor D_WSTAG710_800A6B94 = { D_WSTAG710_800A6AEC, D_WSTAG710_800A6978, 57, 5, 1088, 234, 7 };
FieldstgPlacedActor D_WSTAG710_800A6BA8 = { D_WSTAG710_800A6AF8, D_WSTAG710_800A69D8, 157, 6, 464, 193, 1 };
FieldstgPlacedActor D_WSTAG710_800A6BBC = { D_WSTAG710_800A6B00, D_WSTAG710_800A6A38, 158, 7, 1088, 234, 7 };
FieldstgPlacedActor *wstag710_actors[11] = {
    &D_WSTAG710_800A6B08, &D_WSTAG710_800A6B1C, &D_WSTAG710_800A6B30, &D_WSTAG710_800A6B44, &D_WSTAG710_800A6B58,
    &D_WSTAG710_800A6B6C, &D_WSTAG710_800A6B80, &D_WSTAG710_800A6B94, &D_WSTAG710_800A6BA8, &D_WSTAG710_800A6BBC,
    NULL,
};
FieldstgSprite wstag710_sprites[39] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 603, 223, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 952, 485, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 567, 357, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 604, 375, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 857, 450, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 529, 660, 0, 0 },
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 201, 398, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 215, 598, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 140, 499, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 476, 616, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 569, 552, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 827, 661, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 408, 122, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 479, 678, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 772, 301, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 869, 528, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 919, 263, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 929, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 1166, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 1218, 758, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 986, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 1025, 319, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 1195, 732, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 1, 6, 0, 522, 632, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 2, 0, 1, 6, 0, 599, 379, 0, 0 }, { 1, 0, 0xC4, 4, 0, 0, 0, 0, 0, 0, 413, 523, 715, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 699, 190, 233, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 138, 383, 410, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 161, 328, 364, 0 }, { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 542, 196, 217, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 311, 311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 719, 719, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 455, 455, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 479, 479, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 351, 351, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 167, 167, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag710_map_events[18] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x266, 0x390, 0x1E8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x261, 0x80, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x268, 0x390, 0x118, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x20F, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x21F, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x34F, 0x106, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x35F, 0x16F, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x440, 0x161, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x430, 0x1C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x3EF, 0x2C9, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x3E1, 0x32F, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x150, 0x258, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x15F, 0x2BE, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 5, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xA, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 6, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x16, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag710_funcs = { wstag710_setup };
