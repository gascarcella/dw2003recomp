#include "wstag.h"

/* WSTAG330: stage 0x21D (fieldstg_stages). */

extern WstagFuncs wstag330_funcs;
const CVECTOR wstag330_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag330_battle_lists;
extern FieldstgVramPlace wstag330_vram_places[];
extern FieldstgPlacedActor *wstag330_actors[];
extern FieldstgSprite wstag330_sprites[];
extern FieldstgMapEvent wstag330_map_events[];

void wstag330_update(WstagObject *obj) {
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

WstagObject *wstag330_start(void *arg0) {
    WstagObject *obj = object_new(wstag330_update, sizeof(WstagObject), sizeof(Object *));

    obj->manager = arg0;
    wstag330_funcs.setup();
    return obj;
}

void wstag330_setup(void) {
    fieldstg_stage.background_file = 0x1B4;
    fieldstg_stage.sprite_file = 0x01B50000;
    fieldstg_stage.sprites = wstag330_sprites;
    fieldstg_stage.map_events = wstag330_map_events;
    fieldstg_stage.mask_file = 0x328;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x38400, 0x3E800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag330_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag330_actors;
    fieldstg_stage.color = wstag330_color;
    fieldstg_stage.battle_lists = &wstag330_battle_lists;
    fieldstg_attr.set_file(0, 0x01B50001);
    fieldstg_attr.set_file(7, 0x01B50002);
    fieldstg_attr.set_file(4, 0x01B50003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag330_setup(void);

FieldstgListedBattle D_WSTAG330_800A5FE4 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A5FF0 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A5FFC = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6008 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6014 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6020 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A602C = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6038 = { 35, 1, 0x60080000 };
FieldstgBattleList D_WSTAG330_800A6044 = {
    3,
    { &D_WSTAG330_800A5FE4, &D_WSTAG330_800A5FF0, &D_WSTAG330_800A5FFC, &D_WSTAG330_800A6008, &D_WSTAG330_800A6014,
        &D_WSTAG330_800A6020, &D_WSTAG330_800A602C, &D_WSTAG330_800A6038 },
};
FieldstgListedBattle D_WSTAG330_800A6068 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6074 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6080 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A608C = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A6098 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A60A4 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A60B0 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A60BC = { 35, 1, 0x60080000 };
FieldstgBattleList D_WSTAG330_800A60C8 = {
    1,
    { &D_WSTAG330_800A6068, &D_WSTAG330_800A6074, &D_WSTAG330_800A6080, &D_WSTAG330_800A608C, &D_WSTAG330_800A6098,
        &D_WSTAG330_800A60A4, &D_WSTAG330_800A60B0, &D_WSTAG330_800A60BC },
};
FieldstgListedBattle D_WSTAG330_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6140 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG330_800A614C = {
    0,
    { &D_WSTAG330_800A60EC, &D_WSTAG330_800A60F8, &D_WSTAG330_800A6104, &D_WSTAG330_800A6110, &D_WSTAG330_800A611C,
        &D_WSTAG330_800A6128, &D_WSTAG330_800A6134, &D_WSTAG330_800A6140 },
};
FieldstgListedBattle D_WSTAG330_800A6170 = { 201, 1, 0x600C0000 };
FieldstgListedBattle D_WSTAG330_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A6194 = { 327, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A61A0 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG330_800A61B8 = { 49, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG330_800A61C4 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG330_800A61D0 = {
    0,
    { &D_WSTAG330_800A6170, &D_WSTAG330_800A617C, &D_WSTAG330_800A6188, &D_WSTAG330_800A6194, &D_WSTAG330_800A61A0,
        &D_WSTAG330_800A61AC, &D_WSTAG330_800A61B8, &D_WSTAG330_800A61C4 },
};
FieldstgBattleLists wstag330_battle_lists = {
    1, 0, 0, { &D_WSTAG330_800A6044, &D_WSTAG330_800A60C8, &D_WSTAG330_800A614C }, &D_WSTAG330_800A61D0,
};
FieldstgVramPlace wstag330_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 452, 216, 196, 352, 511 }, { 320, 256, 376, 304, 224, 48, 368, 511 },
    { 384, 256, 438, 283, 472, 27, 320, 510 }, { 384, 256, 384, 287, 256, 31, 336, 510 },
    { 384, 256, 392, 287, 288, 31, 352, 510 }, { 384, 256, 400, 287, 320, 31, 368, 510 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 376, 256, 224, 0, 336, 509 }, { 384, 256, 416, 287, 384, 31, 352, 509 },
    { 384, 256, 400, 319, 320, 63, 368, 509 }, { 384, 256, 408, 319, 352, 63, 320, 508 },
    { 384, 256, 416, 319, 384, 63, 336, 508 },
};
u16 D_WSTAG330_800A6330[4] = { 0x8028, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6338[4] = { 0x9400, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6340[6] = { 0x8029, 0, 0x8028, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A634C[4] = { 0x9401, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6354[6] = { 0x8028, 1, 0x8029, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6360[4] = { 0x9406, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6368[4] = { 0x940D, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6370[8] = { 0x201, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6380[4] = { 0x1A1F, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6388[4] = { 0x1A1F, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6390[6] = { 0x1A1F, 1, 0x702D, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A639C[8] = { 0x1A1F, 1, 0x702D, 1, 0x7026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A63AC[10] = {
    0x7026, 1, 0x702D, 1, 0x1A1F, 1, 0x7028, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A63C0[4] = { 0x602, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A63C8[10] = {
    0x1A1F, 1, 0x702D, 1, 0x7026, 1, 0x7028, 1,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A63DC[4] = { 0x700E, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A63E4[6] = { 0x7034, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A63F0[4] = { 0x700E, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A63F8[4] = { 0x1A1F, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6400[4] = { 0x1A1F, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6408[6] = { 0x1A1F, 1, 0x702D, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6414[8] = { 0x1A1F, 1, 0x702D, 1, 0x7026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6424[10] = {
    0x1A1F, 1, 0x702D, 1, 0x7026, 1, 0x7028, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6438[4] = { 0x602, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6440[10] = {
    0x1A1F, 1, 0x702D, 1, 0x7026, 1, 0x7028, 1,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6454[4] = { 0x700E, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A645C[6] = { 0x7034, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6468[4] = { 0x700E, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6470[4] = { 0x1A20, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6478[4] = { 0x1A20, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6480[6] = { 0x1A20, 1, 0x7036, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A648C[8] = { 0x1A20, 1, 0x7036, 1, 0x7026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A649C[10] = {
    0x1A20, 1, 0x7036, 1, 0x7026, 1, 0x7028, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A64B0[4] = { 0x603, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A64B8[10] = {
    0x1A20, 1, 0x7036, 1, 0x7026, 1, 0x7028, 1,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A64CC[4] = { 0x700F, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A64D4[6] = { 0x7035, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A64E0[4] = { 0x700F, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A64E8[4] = { 0x1A20, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A64F0[4] = { 0x1A20, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A64F8[6] = { 0x1A20, 1, 0x7036, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6504[8] = { 0x1A20, 1, 0x7036, 1, 0x7026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6514[10] = {
    0x1A20, 1, 0x7036, 1, 0x7026, 1, 0x7028, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6528[4] = { 0x603, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6530[10] = {
    0x1A20, 1, 0x7036, 1, 0x7026, 1, 0x7028, 1,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6544[4] = { 0x700F, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A654C[6] = { 0x7035, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6558[4] = { 0x700F, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6560[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6568[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6570[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A657C[8] = { 0, 1, 0x7200, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A658C[4] = { 0x7605, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6594[10] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0xE01, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A65A8[6] = { 0xE01, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A65B4[12] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 0,
    0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A65CC[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 0, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A65E8[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 1, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6604[4] = { 0x7805, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A660C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6614[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6620[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6628[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6634[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6640[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6648[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6650[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A665C[8] = { 0, 1, 0x7200, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A666C[4] = { 0x7605, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6674[10] = {
    0, 1, 0x7202, 1, 0x7200, 1, 0xE01, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6688[6] = { 0x7400, 1, 0xE01, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6694[12] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 0,
    0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A66AC[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 0, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A66C8[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 1, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A66E4[4] = { 0x7805, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A66EC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A66F4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A66FC[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6708[8] = { 0, 1, 0x7200, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6718[4] = { 0x7605, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6720[10] = {
    0, 1, 0x7202, 1, 0x7200, 1, 0xE01, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A6734[6] = { 0x7400, 1, 0xE01, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6740[12] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 0,
    0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6758[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 0, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6774[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 1, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6790[4] = { 0x7805, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6798[4] = { 0x1C1C, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A67A0[4] = { 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A67A8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A67B0[6] = { 0, 1, 0x7200, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A67BC[8] = { 0, 1, 0x7200, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A67CC[4] = { 0x7605, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A67D4[10] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0xE01, 0,
    0xFFFF, 0,
};
u16 D_WSTAG330_800A67E8[4] = { 0xE01, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A67F0[12] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 0,
    0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6808[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 0, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6824[14] = {
    0, 1, 0x7200, 1, 0x7202, 1, 0x8012, 1,
    0x7204, 1, 0xE01, 1, 0xFFFF, 0,
};
u16 D_WSTAG330_800A6840[4] = { 0x7805, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG330_800A6848[4] = {
    { D_WSTAG330_800A6330, D_WSTAG330_800A6338, 27 }, { D_WSTAG330_800A6340, D_WSTAG330_800A634C, 27 },
    { D_WSTAG330_800A6354, D_WSTAG330_800A6360, 27 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6878[2] = { { NULL, D_WSTAG330_800A6368, 27 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6890[2] = { { NULL, D_WSTAG330_800A6370, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A68A8[6] = {
    { D_WSTAG330_800A6380, D_WSTAG330_800A6388, 631 }, { D_WSTAG330_800A6390, NULL, 642 },
    { D_WSTAG330_800A639C, NULL, 632 }, { D_WSTAG330_800A63AC, D_WSTAG330_800A63C0, 633 },
    { D_WSTAG330_800A63C8, NULL, 652 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A68F0[3] = {
    { D_WSTAG330_800A63DC, D_WSTAG330_800A63E4, 634 }, { D_WSTAG330_800A63F0, NULL, 636 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6914[6] = {
    { D_WSTAG330_800A63F8, D_WSTAG330_800A6400, 631 }, { D_WSTAG330_800A6408, NULL, 642 },
    { D_WSTAG330_800A6414, NULL, 632 }, { D_WSTAG330_800A6424, D_WSTAG330_800A6438, 633 },
    { D_WSTAG330_800A6440, NULL, 652 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A695C[3] = {
    { D_WSTAG330_800A6454, D_WSTAG330_800A645C, 634 }, { D_WSTAG330_800A6468, NULL, 636 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6980[6] = {
    { D_WSTAG330_800A6470, D_WSTAG330_800A6478, 645 }, { D_WSTAG330_800A6480, NULL, 651 },
    { D_WSTAG330_800A648C, NULL, 646 }, { D_WSTAG330_800A649C, D_WSTAG330_800A64B0, 647 },
    { D_WSTAG330_800A64B8, NULL, 653 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A69C8[3] = {
    { D_WSTAG330_800A64CC, D_WSTAG330_800A64D4, 648 }, { D_WSTAG330_800A64E0, NULL, 650 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A69EC[6] = {
    { D_WSTAG330_800A64E8, D_WSTAG330_800A64F0, 645 }, { D_WSTAG330_800A64F8, NULL, 651 },
    { D_WSTAG330_800A6504, NULL, 646 }, { D_WSTAG330_800A6514, D_WSTAG330_800A6528, 647 },
    { D_WSTAG330_800A6530, NULL, 653 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6A34[3] = {
    { D_WSTAG330_800A6544, D_WSTAG330_800A654C, 648 }, { D_WSTAG330_800A6558, NULL, 650 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6A58[8] = {
    { D_WSTAG330_800A6560, D_WSTAG330_800A6568, 46 }, { D_WSTAG330_800A6570, NULL, 51 },
    { D_WSTAG330_800A657C, D_WSTAG330_800A658C, 52 }, { D_WSTAG330_800A6594, D_WSTAG330_800A65A8, 53 },
    { D_WSTAG330_800A65B4, NULL, 54 }, { D_WSTAG330_800A65CC, NULL, 55 },
    { D_WSTAG330_800A65E8, D_WSTAG330_800A6604, 106 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6AB8[4] = {
    { D_WSTAG330_800A660C, NULL, 46 }, { D_WSTAG330_800A6614, D_WSTAG330_800A6620, 56 },
    { D_WSTAG330_800A6628, D_WSTAG330_800A6634, 57 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6AE8[8] = {
    { D_WSTAG330_800A6640, D_WSTAG330_800A6648, 50 }, { D_WSTAG330_800A6650, NULL, 51 },
    { D_WSTAG330_800A665C, D_WSTAG330_800A666C, 52 }, { D_WSTAG330_800A6674, D_WSTAG330_800A6688, 53 },
    { D_WSTAG330_800A6694, NULL, 54 }, { D_WSTAG330_800A66AC, NULL, 55 },
    { D_WSTAG330_800A66C8, D_WSTAG330_800A66E4, 106 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6B48[8] = {
    { D_WSTAG330_800A66EC, D_WSTAG330_800A66F4, 48 }, { D_WSTAG330_800A66FC, NULL, 51 },
    { D_WSTAG330_800A6708, D_WSTAG330_800A6718, 52 }, { D_WSTAG330_800A6720, D_WSTAG330_800A6734, 53 },
    { D_WSTAG330_800A6740, NULL, 54 }, { D_WSTAG330_800A6758, NULL, 55 },
    { D_WSTAG330_800A6774, D_WSTAG330_800A6790, 106 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6BA8[2] = { { NULL, NULL, 624 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6BC0[2] = { { NULL, NULL, 234 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6BD8[2] = { { NULL, NULL, 254 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6BF0[2] = { { NULL, NULL, 26 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C08[2] = { { NULL, NULL, 253 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C20[2] = { { NULL, NULL, 258 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C38[2] = { { NULL, NULL, 258 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C50[2] = { { NULL, NULL, 255 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C68[2] = { { NULL, NULL, 259 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C80[2] = { { NULL, NULL, 258 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6C98[2] = { { NULL, NULL, 257 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6CB0[2] = { { NULL, NULL, 257 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6CC8[3] = {
    { D_WSTAG330_800A6798, NULL, 258 }, { D_WSTAG330_800A67A0, NULL, 16 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6CEC[2] = { { NULL, NULL, 233 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D04[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D1C[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D34[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D4C[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D64[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D7C[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6D94[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6DAC[2] = { { NULL, NULL, 820 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6DC4[8] = {
    { D_WSTAG330_800A67A8, NULL, 49 }, { D_WSTAG330_800A67B0, NULL, 49 },
    { D_WSTAG330_800A67BC, D_WSTAG330_800A67CC, 49 }, { D_WSTAG330_800A67D4, D_WSTAG330_800A67E8, 49 },
    { D_WSTAG330_800A67F0, NULL, 49 }, { D_WSTAG330_800A6808, NULL, 49 },
    { D_WSTAG330_800A6824, D_WSTAG330_800A6840, 49 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG330_800A6E24[2] = { { NULL, NULL, 260 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6E3C[2] = { { NULL, NULL, 635 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG330_800A6E54[2] = { { NULL, NULL, 649 }, { NULL, NULL, 0 } };
u16 D_WSTAG330_800A6E6C[4] = { 0x802A, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6E74[4] = { 0x802A, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6E7C[4] = { 0x201, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6E84[6] = { 0x8027, 0, 0x7024, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6E90[6] = { 0x8027, 1, 0x7024, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6E9C[6] = { 0x602B, 1, 0x8027, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6EA8[6] = { 0x602B, 1, 0x8027, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6EB4[6] = { 0x7024, 1, 0x8026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6EC0[6] = { 0x7024, 1, 0x8026, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6ECC[6] = { 0x8026, 0, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6ED8[6] = { 0x602B, 1, 0x8026, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6EE4[8] = { 0x7003, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6EF4[8] = { 0x7009, 1, 0x11, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F04[8] = { 0x7004, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F14[8] = { 0x6026, 1, 0x11, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F24[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F34[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F3C[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F44[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F4C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F54[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F5C[6] = { 0x7020, 1, 0x6021, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F68[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F70[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F78[6] = { 0x7017, 1, 0x6014, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F84[6] = { 0x7021, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F90[4] = { 0x6021, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6F98[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FA0[4] = { 0x6007, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FA8[4] = { 0x6008, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FB0[4] = { 0x6009, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FB8[4] = { 0x600A, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FC0[4] = { 0x600B, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FC8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FD0[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FD8[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FE0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FE8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FF0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG330_800A6FF8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG330_800A7000 = { D_WSTAG330_800A6E6C, D_WSTAG330_800A6848, 27, 4, 923, 374, 7 };
FieldstgPlacedActor D_WSTAG330_800A7014 = { D_WSTAG330_800A6E74, D_WSTAG330_800A6878, 27, 4, 923, 374, 7 };
FieldstgPlacedActor D_WSTAG330_800A7028 = { D_WSTAG330_800A6E7C, D_WSTAG330_800A6890, 33, 5, 738, 394, 1 };
FieldstgPlacedActor D_WSTAG330_800A703C = { D_WSTAG330_800A6E84, D_WSTAG330_800A68A8, 43, 6, 737, 954, 3 };
FieldstgPlacedActor D_WSTAG330_800A7050 = { D_WSTAG330_800A6E90, D_WSTAG330_800A68F0, 43, 6, 737, 954, 3 };
FieldstgPlacedActor D_WSTAG330_800A7064 = { D_WSTAG330_800A6E9C, D_WSTAG330_800A6914, 43, 6, 737, 954, 3 };
FieldstgPlacedActor D_WSTAG330_800A7078 = { D_WSTAG330_800A6EA8, D_WSTAG330_800A695C, 43, 6, 737, 954, 3 };
FieldstgPlacedActor D_WSTAG330_800A708C = { D_WSTAG330_800A6EB4, D_WSTAG330_800A6980, 44, 7, 913, 233, 1 };
FieldstgPlacedActor D_WSTAG330_800A70A0 = { D_WSTAG330_800A6EC0, D_WSTAG330_800A69C8, 44, 7, 913, 233, 1 };
FieldstgPlacedActor D_WSTAG330_800A70B4 = { D_WSTAG330_800A6ECC, D_WSTAG330_800A69EC, 44, 7, 913, 233, 1 };
FieldstgPlacedActor D_WSTAG330_800A70C8 = { D_WSTAG330_800A6ED8, D_WSTAG330_800A6A34, 44, 7, 913, 233, 1 };
FieldstgPlacedActor D_WSTAG330_800A70DC = { D_WSTAG330_800A6EE4, D_WSTAG330_800A6A58, 48, 8, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A70F0 = { D_WSTAG330_800A6EF4, D_WSTAG330_800A6AB8, 48, 8, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A7104 = { D_WSTAG330_800A6F04, D_WSTAG330_800A6AE8, 48, 8, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A7118 = { D_WSTAG330_800A6F14, D_WSTAG330_800A6B48, 48, 8, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A712C = { D_WSTAG330_800A6F24, D_WSTAG330_800A6BA8, 48, 8, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A7140 = { D_WSTAG330_800A6F34, D_WSTAG330_800A6BC0, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A7154 = { D_WSTAG330_800A6F3C, D_WSTAG330_800A6BD8, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A7168 = { D_WSTAG330_800A6F44, D_WSTAG330_800A6BF0, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A717C = { D_WSTAG330_800A6F4C, D_WSTAG330_800A6C08, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A7190 = { D_WSTAG330_800A6F54, D_WSTAG330_800A6C20, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A71A4 = { D_WSTAG330_800A6F5C, D_WSTAG330_800A6C38, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A71B8 = { D_WSTAG330_800A6F68, D_WSTAG330_800A6C50, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A71CC = { D_WSTAG330_800A6F70, D_WSTAG330_800A6C68, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A71E0 = { D_WSTAG330_800A6F78, D_WSTAG330_800A6C80, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A71F4 = { D_WSTAG330_800A6F84, D_WSTAG330_800A6C98, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A7208 = { D_WSTAG330_800A6F90, D_WSTAG330_800A6CB0, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A721C = { D_WSTAG330_800A6F98, D_WSTAG330_800A6CC8, 58, 9, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A7230 = { NULL, D_WSTAG330_800A6CEC, 63, 10, 961, 681, 1 };
FieldstgPlacedActor D_WSTAG330_800A7244 = { D_WSTAG330_800A6FA0, D_WSTAG330_800A6D04, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A7258 = { D_WSTAG330_800A6FA8, D_WSTAG330_800A6D1C, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A726C = { D_WSTAG330_800A6FB0, D_WSTAG330_800A6D34, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A7280 = { D_WSTAG330_800A6FB8, D_WSTAG330_800A6D4C, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A7294 = { D_WSTAG330_800A6FC0, D_WSTAG330_800A6D64, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A72A8 = { D_WSTAG330_800A6FC8, D_WSTAG330_800A6D7C, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A72BC = { D_WSTAG330_800A6FD0, D_WSTAG330_800A6D94, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A72D0 = { D_WSTAG330_800A6FD8, D_WSTAG330_800A6DAC, 102, 11, 400, 784, 7 };
FieldstgPlacedActor D_WSTAG330_800A72E4 = { D_WSTAG330_800A6FE0, D_WSTAG330_800A6DC4, 157, 12, 1152, 793, 1 };
FieldstgPlacedActor D_WSTAG330_800A72F8 = { D_WSTAG330_800A6FE8, D_WSTAG330_800A6E24, 158, 13, 1065, 997, 1 };
FieldstgPlacedActor D_WSTAG330_800A730C = { D_WSTAG330_800A6FF0, D_WSTAG330_800A6E3C, 159, 14, 737, 954, 3 };
FieldstgPlacedActor D_WSTAG330_800A7320 = { D_WSTAG330_800A6FF8, D_WSTAG330_800A6E54, 160, 15, 913, 233, 1 };
FieldstgPlacedActor *wstag330_actors[42] = {
    &D_WSTAG330_800A7000, &D_WSTAG330_800A7014, &D_WSTAG330_800A7028, &D_WSTAG330_800A703C, &D_WSTAG330_800A7050,
    &D_WSTAG330_800A7064, &D_WSTAG330_800A7078, &D_WSTAG330_800A708C, &D_WSTAG330_800A70A0, &D_WSTAG330_800A70B4,
    &D_WSTAG330_800A70C8, &D_WSTAG330_800A70DC, &D_WSTAG330_800A70F0, &D_WSTAG330_800A7104, &D_WSTAG330_800A7118,
    &D_WSTAG330_800A712C, &D_WSTAG330_800A7140, &D_WSTAG330_800A7154, &D_WSTAG330_800A7168, &D_WSTAG330_800A717C,
    &D_WSTAG330_800A7190, &D_WSTAG330_800A71A4, &D_WSTAG330_800A71B8, &D_WSTAG330_800A71CC, &D_WSTAG330_800A71E0,
    &D_WSTAG330_800A71F4, &D_WSTAG330_800A7208, &D_WSTAG330_800A721C, &D_WSTAG330_800A7230, &D_WSTAG330_800A7244,
    &D_WSTAG330_800A7258, &D_WSTAG330_800A726C, &D_WSTAG330_800A7280, &D_WSTAG330_800A7294, &D_WSTAG330_800A72A8,
    &D_WSTAG330_800A72BC, &D_WSTAG330_800A72D0, &D_WSTAG330_800A72E4, &D_WSTAG330_800A72F8, &D_WSTAG330_800A730C,
    &D_WSTAG330_800A7320, NULL,
};
FieldstgSprite wstag330_sprites[129] = {
    { 1, 0, 0x40, 2, 6, 1, 6, 8, 8, 0, 787, 422, 0, 0 }, { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 202, 246, 0, 0 },
    { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 410, 878, 0, 0 }, { 1, 0, 0x40, 2, 9, 1, 9, 0xE, 8, 0, 1322, 886, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 150, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 500, 1060, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 575, 940, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 681, 1030, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 715, 1004, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 870, 843, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 910, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1271, 1057, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1520, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 97, 735, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 195, 304, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 268, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 584, 151, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 668, 1135, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 738, 892, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 742, 475, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 750, 999, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1053, 1074, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1331, 461, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 182, 792, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 645, 1050, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 867, 469, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 869, 1066, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1399, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1458, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 97, 892, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 138, 930, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 145, 273, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 180, 706, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 184, 407, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 293, 496, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 386, 648, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 495, 624, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 527, 75, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 530, 1028, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 611, 1160, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 643, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 910, 1071, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1039, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1365, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1565, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 212, 806, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 232, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 250, 329, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 367, 607, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 673, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1070, 347, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1172, 1074, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1298, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 208, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 261, 826, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 317, 582, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 468, 997, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 579, 1121, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 786, 1012, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 990, 1080, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1076, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1417, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1513, 371, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 78, 891, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 113, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 133, 615, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 249, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 277, 870, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 459, 1003, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 554, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 569, 1062, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 675, 1037, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 711, 518, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 727, 511, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 751, 903, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 878, 1051, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 939, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1088, 1054, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1279, 482, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1391, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1410, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1486, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1589, 336, 0, 0 },
    { 1, 0, 0x72, 4, 0, 0, 0, 0, 0, 0, 765, 572, 665, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 951, 632, 679, 0 },
    { 1, 0, 0x7D, 4, 2, 0, 0, 0, 0, 0, 816, 122, 245, 0 }, { 1, 0, 0x7D, 4, 3, 0, 0, 0, 0, 0, 800, 114, 238, 0 },
    { 1, 0, 0x7D, 4, 4, 0, 0, 0, 0, 0, 784, 106, 230, 0 }, { 1, 0, 0x7D, 4, 5, 0, 0, 0, 0, 0, 767, 99, 223, 0 },
    { 1, 0, 0x46, 4, 0xF, 0, 0, 0, 0, 0, 1224, 424, 484, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 243, 754, 754, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 266, 606, 606, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 275, 778, 778, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 314, 630, 630, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 338, 538, 538, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 363, 654, 654, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 410, 678, 678, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 483, 930, 930, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 531, 906, 906, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 547, 674, 674, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 579, 882, 882, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 595, 650, 650, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 626, 474, 474, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 627, 858, 858, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 643, 994, 994, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 667, 454, 454, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 691, 490, 490, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 723, 826, 826, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 771, 802, 802, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 787, 938, 938, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 819, 778, 778, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 835, 194, 194, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 835, 914, 914, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 883, 890, 890, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 883, 938, 938, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 931, 914, 914, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 979, 378, 378, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 979, 890, 890, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1027, 402, 402, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1075, 426, 426, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1314, 642, 642, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1315, 690, 690, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1315, 738, 738, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 570, 570, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 618, 618, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 666, 666, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 714, 714, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1363, 762, 762, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag330_map_events[24] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x202, 0x100, 0x1E0, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21E, 0x7C, 0x7E, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21F, 0x30A, 0x98, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x220, 0x4AC, 0x326, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x2CE, 0x124, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x2DF, 0xF0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x324, 0x170, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x333, 0x11A, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 1, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x11, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 3, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 1, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x60, 0xFFF0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x40, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x14, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 1, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag330_funcs = { wstag330_setup };
