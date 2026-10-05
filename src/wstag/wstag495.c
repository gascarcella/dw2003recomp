#include "wstag.h"

/* WSTAG495: stage 0x23D (fieldstg_stages). */

extern WstagFuncs wstag495_funcs;
extern FieldstgBattleLists wstag495_battle_lists;
extern FieldstgVramPlace wstag495_vram_places[];
extern FieldstgPlacedActor *wstag495_actors[];
extern FieldstgSprite wstag495_sprites[];
extern FieldstgMapEvent wstag495_map_events[];

void wstag495_update(WstagObject *obj) {
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

WstagObject *wstag495_start(void *arg0) {
    WstagObject *obj = object_new(wstag495_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag495_funcs.setup();
    return obj;
}

void wstag495_setup(void) {
    fieldstg_stage.background_file = 0x3C6;
    fieldstg_stage.sprite_file = 0x03C70000;
    fieldstg_stage.sprites = wstag495_sprites;
    fieldstg_stage.map_events = wstag495_map_events;
    fieldstg_stage.mask_file = 0x3C5;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10300, 0x1EC00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag495_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag495_actors;
    fieldstg_stage.battle_lists = &wstag495_battle_lists;
    fieldstg_attr.set_file(0, 0x03C70001);
    fieldstg_attr.set_file(7, 0x03C70002);
    fieldstg_attr.set_file(4, 0x03C70003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag495_setup(void);

FieldstgListedBattle D_WSTAG495_800A5F94 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FA0 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FAC = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FB8 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FC4 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FD0 = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FDC = { 59, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A5FE8 = { 59, 2, 0x60080000 };
FieldstgBattleList D_WSTAG495_800A5FF4 = {
    3,
    { &D_WSTAG495_800A5F94, &D_WSTAG495_800A5FA0, &D_WSTAG495_800A5FAC, &D_WSTAG495_800A5FB8, &D_WSTAG495_800A5FC4,
        &D_WSTAG495_800A5FD0, &D_WSTAG495_800A5FDC, &D_WSTAG495_800A5FE8 },
};
FieldstgListedBattle D_WSTAG495_800A6018 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A6024 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A6030 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A603C = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A6048 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A6054 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A6060 = { 152, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A606C = { 152, 2, 0x60080000 };
FieldstgBattleList D_WSTAG495_800A6078 = {
    3,
    { &D_WSTAG495_800A6018, &D_WSTAG495_800A6024, &D_WSTAG495_800A6030, &D_WSTAG495_800A603C, &D_WSTAG495_800A6048,
        &D_WSTAG495_800A6054, &D_WSTAG495_800A6060, &D_WSTAG495_800A606C },
};
FieldstgListedBattle D_WSTAG495_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG495_800A60FC = {
    0,
    { &D_WSTAG495_800A609C, &D_WSTAG495_800A60A8, &D_WSTAG495_800A60B4, &D_WSTAG495_800A60C0, &D_WSTAG495_800A60CC,
        &D_WSTAG495_800A60D8, &D_WSTAG495_800A60E4, &D_WSTAG495_800A60F0 },
};
FieldstgListedBattle D_WSTAG495_800A6120 = { 214, 2, 0x600C0000 };
FieldstgListedBattle D_WSTAG495_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A6150 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG495_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG495_800A6174 = { 59, 2, 0x60080000 };
FieldstgBattleList D_WSTAG495_800A6180 = {
    0,
    { &D_WSTAG495_800A6120, &D_WSTAG495_800A612C, &D_WSTAG495_800A6138, &D_WSTAG495_800A6144, &D_WSTAG495_800A6150,
        &D_WSTAG495_800A615C, &D_WSTAG495_800A6168, &D_WSTAG495_800A6174 },
};
FieldstgBattleLists wstag495_battle_lists = {
    21, 0, 0, { &D_WSTAG495_800A5FF4, &D_WSTAG495_800A6078, &D_WSTAG495_800A60FC }, &D_WSTAG495_800A6180,
};
FieldstgVramPlace wstag495_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 364, 304, 176, 48, 336, 511 }, { 320, 256, 372, 304, 208, 48, 352, 511 },
    { 320, 256, 364, 256, 176, 0, 368, 511 }, { 320, 256, 364, 344, 176, 88, 320, 510 },
};
u16 D_WSTAG495_800A6260[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6268[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6270[6] = { 0x7202, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A627C[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A628C[4] = { 0x7618, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6294[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 0,
    0xFFFF, 0,
};
u16 D_WSTAG495_800A62A8[6] = { 0x7400, 1, 0xE0E, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A62B4[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A62CC[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A62E8[14] = {
    0, 1, 0x7204, 1, 0xE0E, 1, 0x7202, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6304[4] = { 0x7818, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A630C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6314[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A631C[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6328[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6338[4] = { 0x7618, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6340[10] = {
    0x7202, 1, 0xE0E, 0, 0, 1, 0x7204, 1,
    0xFFFF, 0,
};
u16 D_WSTAG495_800A6354[6] = { 0x7400, 1, 0xE0E, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6360[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6378[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6394[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG495_800A63B0[4] = { 0x7818, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A63B8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A63C0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A63C8[6] = { 0x7202, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A63D4[8] = { 0, 1, 0x7204, 0, 0x7202, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A63E4[4] = { 0x7618, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A63EC[10] = {
    0x7202, 1, 0xE0E, 0, 0, 1, 0x7204, 1,
    0xFFFF, 0,
};
u16 D_WSTAG495_800A6400[6] = { 0x7400, 1, 0xE0E, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A640C[12] = {
    0x7202, 1, 0xE0E, 1, 0, 1, 0x7204, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6424[14] = {
    0, 1, 0x7204, 1, 0x7202, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6440[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG495_800A645C[4] = { 0x7818, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6464[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A646C[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6478[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6480[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A648C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6498[6] = { 0x8028, 1, 0x8029, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A64A4[4] = { 0x9402, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A64AC[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A64B8[4] = { 0x9406, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A64C0[4] = { 0x940D, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A64C8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A64D0[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A64DC[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A64EC[4] = { 0x7618, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A64F4[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 0,
    0xFFFF, 0,
};
u16 D_WSTAG495_800A6508[4] = { 0xE0E, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6510[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6528[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6544[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0E, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG495_800A6560[4] = { 0x7818, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG495_800A6568[8] = {
    { D_WSTAG495_800A6260, D_WSTAG495_800A6268, 178 }, { D_WSTAG495_800A6270, NULL, 182 },
    { D_WSTAG495_800A627C, D_WSTAG495_800A628C, 183 }, { D_WSTAG495_800A6294, D_WSTAG495_800A62A8, 184 },
    { D_WSTAG495_800A62B4, NULL, 185 }, { D_WSTAG495_800A62CC, NULL, 186 },
    { D_WSTAG495_800A62E8, D_WSTAG495_800A6304, 187 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG495_800A65C8[8] = {
    { D_WSTAG495_800A630C, D_WSTAG495_800A6314, 179 }, { D_WSTAG495_800A631C, NULL, 182 },
    { D_WSTAG495_800A6328, D_WSTAG495_800A6338, 183 }, { D_WSTAG495_800A6340, D_WSTAG495_800A6354, 184 },
    { D_WSTAG495_800A6360, NULL, 185 }, { D_WSTAG495_800A6378, NULL, 186 },
    { D_WSTAG495_800A6394, D_WSTAG495_800A63B0, 187 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG495_800A6628[2] = { { NULL, NULL, 178 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A6640[8] = {
    { D_WSTAG495_800A63B8, D_WSTAG495_800A63C0, 180 }, { D_WSTAG495_800A63C8, NULL, 182 },
    { D_WSTAG495_800A63D4, D_WSTAG495_800A63E4, 183 }, { D_WSTAG495_800A63EC, D_WSTAG495_800A6400, 184 },
    { D_WSTAG495_800A640C, NULL, 185 }, { D_WSTAG495_800A6424, NULL, 186 },
    { D_WSTAG495_800A6440, D_WSTAG495_800A645C, 187 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG495_800A66A0[4] = {
    { D_WSTAG495_800A6464, NULL, 178 }, { D_WSTAG495_800A646C, D_WSTAG495_800A6478, 188 },
    { D_WSTAG495_800A6480, D_WSTAG495_800A648C, 189 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG495_800A66D0[2] = { { NULL, NULL, 818 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A66E8[2] = { { NULL, NULL, 818 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A6700[2] = { { NULL, NULL, 818 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A6718[2] = { { NULL, NULL, 818 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A6730[3] = {
    { D_WSTAG495_800A6498, D_WSTAG495_800A64A4, 35 }, { D_WSTAG495_800A64AC, D_WSTAG495_800A64B8, 35 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG495_800A6754[2] = { { NULL, D_WSTAG495_800A64C0, 35 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG495_800A676C[8] = {
    { D_WSTAG495_800A64C8, NULL, 181 }, { D_WSTAG495_800A64D0, NULL, 181 },
    { D_WSTAG495_800A64DC, D_WSTAG495_800A64EC, 181 }, { D_WSTAG495_800A64F4, D_WSTAG495_800A6508, 181 },
    { D_WSTAG495_800A6510, NULL, 181 }, { D_WSTAG495_800A6528, NULL, 181 },
    { D_WSTAG495_800A6544, D_WSTAG495_800A6560, 181 }, { NULL, NULL, 0 },
};
u16 D_WSTAG495_800A67CC[8] = { 0x7003, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A67DC[8] = { 0x7004, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A67EC[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A67FC[8] = { 0x6026, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A680C[8] = { 0x7009, 1, 0x11, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A681C[4] = { 0x600B, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6824[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A682C[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A6834[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A683C[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG495_800A6844[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG495_800A684C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG495_800A6854 = { D_WSTAG495_800A67CC, D_WSTAG495_800A6568, 53, 4, 480, 480, 7 };
FieldstgPlacedActor D_WSTAG495_800A6868 = { D_WSTAG495_800A67DC, D_WSTAG495_800A65C8, 53, 4, 480, 480, 7 };
FieldstgPlacedActor D_WSTAG495_800A687C = { D_WSTAG495_800A67EC, D_WSTAG495_800A6628, 53, 4, 480, 480, 7 };
FieldstgPlacedActor D_WSTAG495_800A6890 = { D_WSTAG495_800A67FC, D_WSTAG495_800A6640, 53, 4, 480, 480, 7 };
FieldstgPlacedActor D_WSTAG495_800A68A4 = { D_WSTAG495_800A680C, D_WSTAG495_800A66A0, 53, 4, 480, 480, 7 };
FieldstgPlacedActor D_WSTAG495_800A68B8 = { D_WSTAG495_800A681C, D_WSTAG495_800A66D0, 101, 5, 192, 496, 1 };
FieldstgPlacedActor D_WSTAG495_800A68CC = { D_WSTAG495_800A6824, D_WSTAG495_800A66E8, 101, 5, 192, 496, 1 };
FieldstgPlacedActor D_WSTAG495_800A68E0 = { D_WSTAG495_800A682C, D_WSTAG495_800A6700, 101, 5, 192, 496, 1 };
FieldstgPlacedActor D_WSTAG495_800A68F4 = { D_WSTAG495_800A6834, D_WSTAG495_800A6718, 101, 5, 192, 496, 1 };
FieldstgPlacedActor D_WSTAG495_800A6908 = { D_WSTAG495_800A683C, D_WSTAG495_800A6730, 134, 6, 204, 412, 7 };
FieldstgPlacedActor D_WSTAG495_800A691C = { D_WSTAG495_800A6844, D_WSTAG495_800A6754, 134, 6, 204, 412, 7 };
FieldstgPlacedActor D_WSTAG495_800A6930 = { D_WSTAG495_800A684C, D_WSTAG495_800A676C, 157, 7, 480, 480, 7 };
FieldstgPlacedActor *wstag495_actors[13] = {
    &D_WSTAG495_800A6854, &D_WSTAG495_800A6868, &D_WSTAG495_800A687C, &D_WSTAG495_800A6890, &D_WSTAG495_800A68A4,
    &D_WSTAG495_800A68B8, &D_WSTAG495_800A68CC, &D_WSTAG495_800A68E0, &D_WSTAG495_800A68F4, &D_WSTAG495_800A6908,
    &D_WSTAG495_800A691C, &D_WSTAG495_800A6930, NULL,
};
FieldstgSprite wstag495_sprites[44] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 427, 217, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 182, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 267, 201, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 377, 286, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 400, 136, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 456, 92, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 211, 228, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 347, 466, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 445, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 48, 497, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 369, 416, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 496, 331, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 667, 251, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 712, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 142, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 356, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 541, 241, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 564, 420, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 640, 332, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 730, 597, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 839, 559, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 34, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 138, 506, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 19, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 214, 575, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 324, 588, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 445, 584, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 587, 528, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 739, 511, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 18, 509, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 182, 191, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 232, 601, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 262, 206, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 361, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 417, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 467, 189, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 479, 600, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 516, 331, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 558, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 648, 406, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 697, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 716, 634, 0, 0 },
    { 1, 0, 0x64, 4, 6, 0, 0, 0, 0, 0, 209, 351, 391, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag495_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23C, 0x88, 0x1BE, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 9, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFD8, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag495_funcs = { wstag495_setup };
