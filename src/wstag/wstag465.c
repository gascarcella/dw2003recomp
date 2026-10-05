#include "wstag.h"

/* WSTAG465: stage 0x237 (fieldstg_stages). */

extern WstagFuncs wstag465_funcs;
extern CVECTOR wstag465_color;
extern FieldstgBattleLists wstag465_battle_lists;
extern FieldstgVramPlace wstag465_vram_places[];
extern FieldstgPlacedActor *wstag465_actors[];
extern FieldstgSprite wstag465_sprites[];
extern FieldstgMapEvent wstag465_map_events[];

void wstag465_update(WstagObject *obj) {
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

WstagObject *wstag465_start(void *arg0) {
    WstagObject *obj = object_new(wstag465_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag465_funcs.setup();
    return obj;
}

void wstag465_setup(void) {
    fieldstg_stage.background_file = 0x1C2;
    fieldstg_stage.sprites = wstag465_sprites;
    fieldstg_stage.map_events = wstag465_map_events;
    fieldstg_stage.sprite_file = 0x01C30001;
    fieldstg_stage.mask_file = 0x3E1;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x37F00, 0x1F600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag465_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag465_actors;
    fieldstg_stage.color = wstag465_color;
    fieldstg_stage.battle_lists = &wstag465_battle_lists;
    fieldstg_attr.set_file(0, 0x01C30000);
    fieldstg_attr.set_file(7, 0x01C30002);
    fieldstg_attr.set_file(4, 0x01C30003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag465/nonmatchings/wstag465", wstag465_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag465_setup(void);

FieldstgListedBattle D_WSTAG465_800A5FB4 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FC0 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FCC = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FD8 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FE4 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FF0 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A5FFC = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A6008 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG465_800A6014 = {
    3,
    { &D_WSTAG465_800A5FB4, &D_WSTAG465_800A5FC0, &D_WSTAG465_800A5FCC, &D_WSTAG465_800A5FD8, &D_WSTAG465_800A5FE4,
        &D_WSTAG465_800A5FF0, &D_WSTAG465_800A5FFC, &D_WSTAG465_800A6008 },
};
FieldstgListedBattle D_WSTAG465_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A608C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG465_800A6098 = {
    0,
    { &D_WSTAG465_800A6038, &D_WSTAG465_800A6044, &D_WSTAG465_800A6050, &D_WSTAG465_800A605C, &D_WSTAG465_800A6068,
        &D_WSTAG465_800A6074, &D_WSTAG465_800A6080, &D_WSTAG465_800A608C },
};
FieldstgListedBattle D_WSTAG465_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6110 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG465_800A611C = {
    0,
    { &D_WSTAG465_800A60BC, &D_WSTAG465_800A60C8, &D_WSTAG465_800A60D4, &D_WSTAG465_800A60E0, &D_WSTAG465_800A60EC,
        &D_WSTAG465_800A60F8, &D_WSTAG465_800A6104, &D_WSTAG465_800A6110 },
};
FieldstgListedBattle D_WSTAG465_800A6140 = { 213, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG465_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6164 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A6170 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG465_800A6188 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG465_800A6194 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG465_800A61A0 = {
    0,
    { &D_WSTAG465_800A6140, &D_WSTAG465_800A614C, &D_WSTAG465_800A6158, &D_WSTAG465_800A6164, &D_WSTAG465_800A6170,
        &D_WSTAG465_800A617C, &D_WSTAG465_800A6188, &D_WSTAG465_800A6194 },
};
FieldstgBattleLists wstag465_battle_lists = {
    15, 0, 0, { &D_WSTAG465_800A6014, &D_WSTAG465_800A6098, &D_WSTAG465_800A611C }, &D_WSTAG465_800A61A0,
};
FieldstgVramPlace wstag465_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 328, 468, 32, 212, 368, 511 }, { 384, 256, 408, 256, 352, 0, 320, 510 },
    { 384, 256, 416, 256, 384, 0, 336, 510 }, { 384, 256, 424, 256, 416, 0, 352, 510 },
};
u16 D_WSTAG465_800A6280[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6288[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6290[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A629C[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62A8[4] = { 0x7617, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62B0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A62B8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62C0[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A62CC[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62D8[4] = { 0x7617, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62E0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A62E8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A62F0[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A62FC[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6308[4] = { 0x7617, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6310[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6318[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6320[6] = { 0, 1, 0xE0D, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A632C[6] = { 0x7400, 1, 0xE0D, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6338[8] = { 0, 1, 0xE0D, 1, 0x720D, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6348[8] = { 0, 1, 0xE0D, 1, 0x720D, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6358[4] = { 0x7817, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6360[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6368[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6374[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A637C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6388[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6394[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A639C[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A63A8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A63B0[8] = { 0x10, 1, 0x9213, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A63C0[10] = {
    0x9213, 1, 0x11, 0, 0x7013, 1, 0x10, 0,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A63D4[8] = { 0x10, 1, 0x9213, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A63E4[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A63F0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A63F8[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG465_800A6404[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6410[4] = { 0x7617, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG465_800A6418[2] = { { NULL, NULL, 32 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6430[2] = { { NULL, NULL, 307 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6448[2] = { { NULL, NULL, 307 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6460[2] = { { NULL, NULL, 32 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6478[2] = { { NULL, NULL, 32 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6490[2] = { { NULL, NULL, 301 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A64A8[2] = { { NULL, NULL, 302 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A64C0[2] = { { NULL, NULL, 303 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A64D8[2] = { { NULL, NULL, 304 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A64F0[2] = { { NULL, NULL, 311 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6508[2] = { { NULL, NULL, 309 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6520[2] = { { NULL, NULL, 305 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6538[2] = { { NULL, NULL, 306 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6550[2] = { { NULL, NULL, 308 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A6568[4] = {
    { D_WSTAG465_800A6280, D_WSTAG465_800A6288, 190 }, { D_WSTAG465_800A6290, NULL, 194 },
    { D_WSTAG465_800A629C, D_WSTAG465_800A62A8, 195 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A6598[4] = {
    { D_WSTAG465_800A62B0, D_WSTAG465_800A62B8, 191 }, { D_WSTAG465_800A62C0, NULL, 194 },
    { D_WSTAG465_800A62CC, D_WSTAG465_800A62D8, 195 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A65C8[4] = {
    { D_WSTAG465_800A62E0, D_WSTAG465_800A62E8, 192 }, { D_WSTAG465_800A62F0, NULL, 194 },
    { D_WSTAG465_800A62FC, D_WSTAG465_800A6308, 195 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A65F8[5] = {
    { D_WSTAG465_800A6310, D_WSTAG465_800A6318, 196 }, { D_WSTAG465_800A6320, D_WSTAG465_800A632C, 197 },
    { D_WSTAG465_800A6338, NULL, 198 }, { D_WSTAG465_800A6348, D_WSTAG465_800A6358, 199 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A6634[4] = {
    { D_WSTAG465_800A6360, NULL, 190 }, { D_WSTAG465_800A6368, D_WSTAG465_800A6374, 202 },
    { D_WSTAG465_800A637C, D_WSTAG465_800A6388, 201 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A6664[5] = {
    { D_WSTAG465_800A6394, NULL, 190 }, { D_WSTAG465_800A639C, D_WSTAG465_800A63A8, 200 },
    { D_WSTAG465_800A63B0, D_WSTAG465_800A63C0, 203 }, { D_WSTAG465_800A63D4, D_WSTAG465_800A63E4, 201 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A66A0[2] = { { NULL, NULL, 643 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG465_800A66B8[4] = {
    { D_WSTAG465_800A63F0, NULL, 193 }, { D_WSTAG465_800A63F8, NULL, 193 },
    { D_WSTAG465_800A6404, D_WSTAG465_800A6410, 193 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG465_800A66E8[2] = { { NULL, NULL, 310 }, { NULL, NULL, 0 } };
u16 D_WSTAG465_800A6700[4] = { 0x6007, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6708[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6710[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6718[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6720[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6728[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6730[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6738[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6740[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6748[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6750[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6758[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6760[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6768[4] = { 0x601B, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6770[10] = {
    0x7003, 1, 0x11, 0, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A6784[10] = {
    0x7004, 1, 0x11, 0, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A6798[10] = {
    0x6026, 1, 0x11, 0, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A67AC[10] = {
    0x8012, 1, 0x11, 0, 0x8192, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A67C0[10] = {
    0x700A, 1, 0x11, 1, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A67D4[10] = {
    0x8012, 1, 0x8192, 1, 0x11, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG465_800A67E8[8] = { 0x8192, 0, 0x701A, 0, 0x7009, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A67F8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG465_800A6800[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG465_800A6808 = { D_WSTAG465_800A6700, D_WSTAG465_800A6418, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A681C = { D_WSTAG465_800A6708, D_WSTAG465_800A6430, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6830 = { D_WSTAG465_800A6710, D_WSTAG465_800A6448, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6844 = { D_WSTAG465_800A6718, D_WSTAG465_800A6460, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6858 = { D_WSTAG465_800A6720, D_WSTAG465_800A6478, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A686C = { D_WSTAG465_800A6728, D_WSTAG465_800A6490, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6880 = { D_WSTAG465_800A6730, D_WSTAG465_800A64A8, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6894 = { D_WSTAG465_800A6738, D_WSTAG465_800A64C0, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A68A8 = { D_WSTAG465_800A6740, D_WSTAG465_800A64D8, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A68BC = { D_WSTAG465_800A6748, D_WSTAG465_800A64F0, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A68D0 = { D_WSTAG465_800A6750, D_WSTAG465_800A6508, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A68E4 = { D_WSTAG465_800A6758, D_WSTAG465_800A6520, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A68F8 = { D_WSTAG465_800A6760, D_WSTAG465_800A6538, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A690C = { D_WSTAG465_800A6768, D_WSTAG465_800A6550, 51, 4, 897, 514, 5 };
FieldstgPlacedActor D_WSTAG465_800A6920 = { D_WSTAG465_800A6770, D_WSTAG465_800A6568, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A6934 = { D_WSTAG465_800A6784, D_WSTAG465_800A6598, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A6948 = { D_WSTAG465_800A6798, D_WSTAG465_800A65C8, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A695C = { D_WSTAG465_800A67AC, D_WSTAG465_800A65F8, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A6970 = { D_WSTAG465_800A67C0, D_WSTAG465_800A6634, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A6984 = { D_WSTAG465_800A67D4, D_WSTAG465_800A6664, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A6998 = { D_WSTAG465_800A67E8, D_WSTAG465_800A66A0, 58, 5, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A69AC = { D_WSTAG465_800A67F8, D_WSTAG465_800A66B8, 157, 6, 690, 251, 7 };
FieldstgPlacedActor D_WSTAG465_800A69C0 = { D_WSTAG465_800A6800, D_WSTAG465_800A66E8, 158, 7, 897, 514, 5 };
FieldstgPlacedActor *wstag465_actors[24] = {
    &D_WSTAG465_800A6808, &D_WSTAG465_800A681C, &D_WSTAG465_800A6830, &D_WSTAG465_800A6844, &D_WSTAG465_800A6858,
    &D_WSTAG465_800A686C, &D_WSTAG465_800A6880, &D_WSTAG465_800A6894, &D_WSTAG465_800A68A8, &D_WSTAG465_800A68BC,
    &D_WSTAG465_800A68D0, &D_WSTAG465_800A68E4, &D_WSTAG465_800A68F8, &D_WSTAG465_800A690C, &D_WSTAG465_800A6920,
    &D_WSTAG465_800A6934, &D_WSTAG465_800A6948, &D_WSTAG465_800A695C, &D_WSTAG465_800A6970, &D_WSTAG465_800A6984,
    &D_WSTAG465_800A6998, &D_WSTAG465_800A69AC, &D_WSTAG465_800A69C0, NULL,
};
FieldstgSprite wstag465_sprites[124] = {
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 1098, 580, 0, 0 }, { 1, 0, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 1107, 563, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 1187, 603, 0, 0 }, { 1, 0, 0x40, 2, 0x14, 0, 0, 0, 0, 0, 545, 152, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 255, 371, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 476, 316, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 482, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 656, 359, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 815, 365, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 960, 347, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 978, 599, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 319, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 474, 400, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 606, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 707, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 784, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 999, 583, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1177, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 373, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 712, 325, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 797, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3A, 0x3D, 0xA, 0, 290, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 415, 553, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 528, 419, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 702, 495, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1084, 593, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 211, 355, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 312, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 436, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 504, 517, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 195, 355, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 290, 442, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 431, 551, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 544, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 668, 475, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 747, 327, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 125, 305, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 128, 296, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 245, 93, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 252, 98, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 262, 91, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 299, 379, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 366, 425, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 392, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 416, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 507, 441, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 521, 46, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 526, 292, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 532, 49, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 562, 496, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 584, 131, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 599, 479, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 633, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 642, 298, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 643, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 650, 514, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 651, 522, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 672, 157, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 718, 410, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 758, 167, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 812, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 820, 409, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 821, 417, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 835, 409, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 908, 370, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 971, 337, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1080, 569, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1268, 556, 0, 0 },
    { 1, 0, 0x40, 6, 0x15, 0, 0, 0, 0, 0, 185, 274, 0, 0 }, { 1, 0, 0x40, 6, 0x16, 0, 0, 0, 0, 0, 233, 298, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 0, 0, 0, 0, 0, 281, 322, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 164, 311, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 192, 229, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 393, 331, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 511, 381, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 514, 482, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 541, 92, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 639, 447, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 660, 178, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 731, 383, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 767, 326, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 791, 457, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 843, 243, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 861, 362, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1153, 592, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 139, 211, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 349, 379, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 467, 497, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 504, 336, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 567, 337, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 569, 438, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 577, 275, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 613, 264, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 650, 386, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 730, 179, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 735, 458, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 762, 229, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 776, 165, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 912, 408, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 919, 329, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1140, 529, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1234, 356, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1336, 600, 0, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 1072, 384, 422, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 1104, 368, 412, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 1120, 368, 404, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 1136, 368, 396, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 1152, 352, 389, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 1168, 352, 381, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 400, 144, 191, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 384, 160, 197, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 446, 290, 318, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 766, 258, 287, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 486, 160, 189, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 1099, 491, 511, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 207, 286, 286, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 255, 310, 310, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 303, 334, 334, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 735, 581, 581, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 799, 581, 581, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 470, 470, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 446, 446, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 960, 422, 422, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag465_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x234, 0x340, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x239, 0xA8, 0x132, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x238, 0x1F2, 0x164, 3, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x38, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x48, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x38, 0xFFF8, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag465_funcs = { wstag465_setup };
