#include "wstag.h"

/* WSTAG350: stage 0x221 (fieldstg_stages). */

extern WstagFuncs wstag350_funcs;
extern FieldstgBattleLists wstag350_battle_lists;
extern FieldstgBattleLists wstag350_battle_lists2;
extern FieldstgBattleLists wstag350_battle_lists3;
extern FieldstgVramPlace wstag350_vram_places[];
extern FieldstgPlacedActor *wstag350_actors[];
extern FieldstgSprite wstag350_sprites[];
extern FieldstgMapEvent wstag350_map_events[];
extern FieldstgEventDef wstag350_events[];

void wstag350_update(WstagObject *obj) {
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

WstagObject *wstag350_start(void *arg0) {
    WstagObject *obj = object_new(wstag350_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag350_funcs.setup();
    return obj;
}

void wstag350_setup(void) {
    fieldstg_stage.background_file = 0x1B8;
    fieldstg_stage.sprite_file = 0x01B90000;
    fieldstg_stage.sprites = wstag350_sprites;
    fieldstg_stage.map_events = wstag350_map_events;
    fieldstg_stage.mask_file = 0x3D4;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2D600, 0x8000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag350_vram_places;
    fieldstg_stage.music = 0x2D;
    fieldstg_stage.sound = 0x60B40000;
    fieldstg_stage.actors = wstag350_actors;
    fieldstg_stage.events = wstag350_events;
    fieldstg_attr.set_file(0, 0x01B90001);
    fieldstg_attr.set_file(7, 0x01B90002);
    fieldstg_attr.set_file(4, 0x01B90003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress < 0xB) {
        fieldstg_stage.battle_lists = &wstag350_battle_lists;
    } else if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = &wstag350_battle_lists2;
    } else {
        fieldstg_stage.battle_lists = &wstag350_battle_lists3;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag350_setup(void);

FieldstgListedBattle D_WSTAG350_800A5FF4 = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6000 = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A600C = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6018 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6024 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6030 = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A603C = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6048 = { 41, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A6054 = {
    3,
    { &D_WSTAG350_800A5FF4, &D_WSTAG350_800A6000, &D_WSTAG350_800A600C, &D_WSTAG350_800A6018, &D_WSTAG350_800A6024,
        &D_WSTAG350_800A6030, &D_WSTAG350_800A603C, &D_WSTAG350_800A6048 },
};
FieldstgListedBattle D_WSTAG350_800A6078 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6084 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6090 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A609C = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A60A8 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A60B4 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A60C0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A60CC = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A60D8 = {
    0,
    { &D_WSTAG350_800A6078, &D_WSTAG350_800A6084, &D_WSTAG350_800A6090, &D_WSTAG350_800A609C, &D_WSTAG350_800A60A8,
        &D_WSTAG350_800A60B4, &D_WSTAG350_800A60C0, &D_WSTAG350_800A60CC },
};
FieldstgListedBattle D_WSTAG350_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6150 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG350_800A615C = {
    0,
    { &D_WSTAG350_800A60FC, &D_WSTAG350_800A6108, &D_WSTAG350_800A6114, &D_WSTAG350_800A6120, &D_WSTAG350_800A612C,
        &D_WSTAG350_800A6138, &D_WSTAG350_800A6144, &D_WSTAG350_800A6150 },
};
FieldstgListedBattle D_WSTAG350_800A6180 = { 203, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG350_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A61A4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A61B0 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A61BC = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A61C8 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A61D4 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A61E0 = {
    0,
    { &D_WSTAG350_800A6180, &D_WSTAG350_800A618C, &D_WSTAG350_800A6198, &D_WSTAG350_800A61A4, &D_WSTAG350_800A61B0,
        &D_WSTAG350_800A61BC, &D_WSTAG350_800A61C8, &D_WSTAG350_800A61D4 },
};
FieldstgListedBattle D_WSTAG350_800A6204 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6210 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A621C = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6228 = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6234 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6240 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A624C = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6258 = { 48, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A6264 = {
    3,
    { &D_WSTAG350_800A6204, &D_WSTAG350_800A6210, &D_WSTAG350_800A621C, &D_WSTAG350_800A6228, &D_WSTAG350_800A6234,
        &D_WSTAG350_800A6240, &D_WSTAG350_800A624C, &D_WSTAG350_800A6258 },
};
FieldstgListedBattle D_WSTAG350_800A6288 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6294 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62A0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62AC = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62B8 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62C4 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62D0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A62DC = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A62E8 = {
    0,
    { &D_WSTAG350_800A6288, &D_WSTAG350_800A6294, &D_WSTAG350_800A62A0, &D_WSTAG350_800A62AC, &D_WSTAG350_800A62B8,
        &D_WSTAG350_800A62C4, &D_WSTAG350_800A62D0, &D_WSTAG350_800A62DC },
};
FieldstgListedBattle D_WSTAG350_800A630C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6324 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6330 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A633C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6348 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6354 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6360 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG350_800A636C = {
    0,
    { &D_WSTAG350_800A630C, &D_WSTAG350_800A6318, &D_WSTAG350_800A6324, &D_WSTAG350_800A6330, &D_WSTAG350_800A633C,
        &D_WSTAG350_800A6348, &D_WSTAG350_800A6354, &D_WSTAG350_800A6360 },
};
FieldstgListedBattle D_WSTAG350_800A6390 = { 203, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG350_800A639C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A63A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A63B4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A63C0 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A63CC = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A63D8 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A63E4 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A63F0 = {
    0,
    { &D_WSTAG350_800A6390, &D_WSTAG350_800A639C, &D_WSTAG350_800A63A8, &D_WSTAG350_800A63B4, &D_WSTAG350_800A63C0,
        &D_WSTAG350_800A63CC, &D_WSTAG350_800A63D8, &D_WSTAG350_800A63E4 },
};
FieldstgListedBattle D_WSTAG350_800A6414 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6420 = { 39, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A642C = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6438 = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6444 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6450 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A645C = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A6468 = { 146, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A6474 = {
    3,
    { &D_WSTAG350_800A6414, &D_WSTAG350_800A6420, &D_WSTAG350_800A642C, &D_WSTAG350_800A6438, &D_WSTAG350_800A6444,
        &D_WSTAG350_800A6450, &D_WSTAG350_800A645C, &D_WSTAG350_800A6468 },
};
FieldstgListedBattle D_WSTAG350_800A6498 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64A4 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64B0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64BC = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64C8 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64D4 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64E0 = { 0, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A64EC = { 0, 13, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A64F8 = {
    0,
    { &D_WSTAG350_800A6498, &D_WSTAG350_800A64A4, &D_WSTAG350_800A64B0, &D_WSTAG350_800A64BC, &D_WSTAG350_800A64C8,
        &D_WSTAG350_800A64D4, &D_WSTAG350_800A64E0, &D_WSTAG350_800A64EC },
};
FieldstgListedBattle D_WSTAG350_800A651C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6534 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6540 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A654C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6558 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6564 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A6570 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG350_800A657C = {
    0,
    { &D_WSTAG350_800A651C, &D_WSTAG350_800A6528, &D_WSTAG350_800A6534, &D_WSTAG350_800A6540, &D_WSTAG350_800A654C,
        &D_WSTAG350_800A6558, &D_WSTAG350_800A6564, &D_WSTAG350_800A6570 },
};
FieldstgListedBattle D_WSTAG350_800A65A0 = { 203, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG350_800A65AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A65B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG350_800A65C4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A65D0 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A65DC = { 41, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A65E8 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG350_800A65F4 = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG350_800A6600 = {
    0,
    { &D_WSTAG350_800A65A0, &D_WSTAG350_800A65AC, &D_WSTAG350_800A65B8, &D_WSTAG350_800A65C4, &D_WSTAG350_800A65D0,
        &D_WSTAG350_800A65DC, &D_WSTAG350_800A65E8, &D_WSTAG350_800A65F4 },
};
FieldstgBattleLists wstag350_battle_lists = {
    5, 0, 0, { &D_WSTAG350_800A6054, &D_WSTAG350_800A60D8, &D_WSTAG350_800A615C }, &D_WSTAG350_800A61E0,
};
FieldstgBattleLists wstag350_battle_lists2 = {
    22, 1, 0, { &D_WSTAG350_800A6264, &D_WSTAG350_800A62E8, &D_WSTAG350_800A636C }, &D_WSTAG350_800A63F0,
};
FieldstgBattleLists wstag350_battle_lists3 = {
    51, 2, 0, { &D_WSTAG350_800A6474, &D_WSTAG350_800A64F8, &D_WSTAG350_800A657C }, &D_WSTAG350_800A6600,
};
FieldstgVramPlace wstag350_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 336, 511 }, { 320, 256, 364, 256, 176, 0, 352, 511 },
    { 320, 256, 372, 352, 208, 96, 368, 511 },
};
u16 D_WSTAG350_800A6708[8] = { 0x202, 1, 0x8B13, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6718[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6720[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6728[6] = { 0, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6734[8] = { 0, 1, 0x7201, 1, 0x7203, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6744[4] = { 0x7607, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A674C[10] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0xE03, 0,
    0xFFFF, 0,
};
u16 D_WSTAG350_800A6760[6] = { 0xE03, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A676C[12] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG350_800A6784[14] = {
    0x7205, 0, 0xE03, 1, 0, 1, 0x7201, 1,
    0x7203, 1, 0x8012, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A67A0[14] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0x8012, 1,
    0x7205, 1, 0xE03, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A67BC[4] = { 0x7807, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A67C4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A67CC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A67D8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A67E0[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A67EC[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A67F8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6800[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6808[6] = { 0, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6814[8] = { 0, 1, 0x7201, 1, 0x7203, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6824[4] = { 0x7607, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A682C[10] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0xE03, 0,
    0xFFFF, 0,
};
u16 D_WSTAG350_800A6840[6] = { 0xE03, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A684C[12] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0x8012, 0,
    0xE03, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A6864[14] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0x8012, 1,
    0x7205, 0, 0xE03, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A6880[14] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x8012, 1,
    0x7203, 1, 0x7205, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A689C[4] = { 0x7807, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A68A4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A68AC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A68B4[6] = { 0, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A68C0[8] = { 0, 1, 0x7201, 1, 0x7203, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A68D0[4] = { 0x7607, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A68D8[10] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0xE03, 0,
    0xFFFF, 0,
};
u16 D_WSTAG350_800A68EC[6] = { 0x7400, 1, 0xE03, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A68F8[12] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG350_800A6910[14] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 1, 0x7205, 0, 0xFFFF, 0,
};
u16 D_WSTAG350_800A692C[14] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 1, 0x7205, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A6948[4] = { 0x7807, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6950[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6958[6] = { 0, 1, 0x7201, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6964[8] = { 0, 1, 0x7201, 1, 0x7203, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6974[4] = { 0x7607, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A697C[10] = {
    0, 1, 0x7201, 1, 0x7203, 1, 0xE03, 0,
    0xFFFF, 0,
};
u16 D_WSTAG350_800A6990[4] = { 0xE03, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6998[12] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG350_800A69B0[14] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 1, 0x7205, 0, 0xFFFF, 0,
};
u16 D_WSTAG350_800A69CC[14] = {
    0xE03, 1, 0, 1, 0x7201, 1, 0x7203, 1,
    0x8012, 1, 0x7205, 1, 0xFFFF, 0,
};
u16 D_WSTAG350_800A69E8[4] = { 0x7807, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG350_800A69F0[2] = { { NULL, D_WSTAG350_800A6708, 366 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG350_800A6A08[8] = {
    { D_WSTAG350_800A6718, D_WSTAG350_800A6720, 58 }, { D_WSTAG350_800A6728, NULL, 63 },
    { D_WSTAG350_800A6734, D_WSTAG350_800A6744, 64 }, { D_WSTAG350_800A674C, D_WSTAG350_800A6760, 65 },
    { D_WSTAG350_800A676C, NULL, 66 }, { D_WSTAG350_800A6784, NULL, 67 },
    { D_WSTAG350_800A67A0, D_WSTAG350_800A67BC, 107 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG350_800A6A68[4] = {
    { D_WSTAG350_800A67C4, NULL, 58 }, { D_WSTAG350_800A67CC, D_WSTAG350_800A67D8, 68 },
    { D_WSTAG350_800A67E0, D_WSTAG350_800A67EC, 69 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG350_800A6A98[8] = {
    { D_WSTAG350_800A67F8, D_WSTAG350_800A6800, 59 }, { D_WSTAG350_800A6808, NULL, 63 },
    { D_WSTAG350_800A6814, D_WSTAG350_800A6824, 64 }, { D_WSTAG350_800A682C, D_WSTAG350_800A6840, 65 },
    { D_WSTAG350_800A684C, NULL, 66 }, { D_WSTAG350_800A6864, NULL, 67 },
    { D_WSTAG350_800A6880, D_WSTAG350_800A689C, 107 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG350_800A6AF8[8] = {
    { D_WSTAG350_800A68A4, D_WSTAG350_800A68AC, 60 }, { D_WSTAG350_800A68B4, NULL, 63 },
    { D_WSTAG350_800A68C0, D_WSTAG350_800A68D0, 64 }, { D_WSTAG350_800A68D8, D_WSTAG350_800A68EC, 65 },
    { D_WSTAG350_800A68F8, NULL, 66 }, { D_WSTAG350_800A6910, NULL, 67 },
    { D_WSTAG350_800A692C, D_WSTAG350_800A6948, 107 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG350_800A6B58[2] = { { NULL, NULL, 626 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG350_800A6B70[8] = {
    { D_WSTAG350_800A6950, NULL, 61 }, { D_WSTAG350_800A6958, NULL, 61 },
    { D_WSTAG350_800A6964, D_WSTAG350_800A6974, 61 }, { D_WSTAG350_800A697C, D_WSTAG350_800A6990, 61 },
    { D_WSTAG350_800A6998, NULL, 61 }, { D_WSTAG350_800A69B0, NULL, 61 },
    { D_WSTAG350_800A69CC, D_WSTAG350_800A69E8, 61 }, { NULL, NULL, 0 },
};
u16 D_WSTAG350_800A6BD0[4] = { 0x202, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6BD8[8] = { 0x7003, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6BE8[8] = { 0x7009, 1, 0x11, 1, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG350_800A6BF8[8] = { 0x7004, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6C08[8] = { 0x6026, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6C18[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG350_800A6C28[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG350_800A6C30 = { D_WSTAG350_800A6BD0, D_WSTAG350_800A69F0, 33, 4, 1353, 405, 1 };
FieldstgPlacedActor D_WSTAG350_800A6C44 = { D_WSTAG350_800A6BD8, D_WSTAG350_800A6A08, 46, 5, 241, 297, 7 };
FieldstgPlacedActor D_WSTAG350_800A6C58 = { D_WSTAG350_800A6BE8, D_WSTAG350_800A6A68, 46, 5, 241, 297, 7 };
FieldstgPlacedActor D_WSTAG350_800A6C6C = { D_WSTAG350_800A6BF8, D_WSTAG350_800A6A98, 46, 5, 241, 297, 7 };
FieldstgPlacedActor D_WSTAG350_800A6C80 = { D_WSTAG350_800A6C08, D_WSTAG350_800A6AF8, 46, 5, 241, 297, 7 };
FieldstgPlacedActor D_WSTAG350_800A6C94 = { D_WSTAG350_800A6C18, D_WSTAG350_800A6B58, 46, 5, 241, 297, 7 };
FieldstgPlacedActor D_WSTAG350_800A6CA8 = { D_WSTAG350_800A6C28, D_WSTAG350_800A6B70, 157, 6, 241, 297, 7 };
FieldstgPlacedActor *wstag350_actors[8] = {
    &D_WSTAG350_800A6C30, &D_WSTAG350_800A6C44, &D_WSTAG350_800A6C58, &D_WSTAG350_800A6C6C, &D_WSTAG350_800A6C80,
    &D_WSTAG350_800A6C94, &D_WSTAG350_800A6CA8, NULL,
};
FieldstgSprite wstag350_sprites[45] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 313, 257, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 730, 365, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 932, 55, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 161, 378, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 83, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 50, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 182, 431, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 196, 377, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 208, 446, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 188, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 230, 394, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 273, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 129, 383, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x44, 0x46, 0xA, 0, 196, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 248, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 272, 461, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 441, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 488, 363, 363, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 527, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 230, 230, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 584, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 632, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 679, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 687, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 760, 83, 83, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 808, 107, 107, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 856, 131, 131, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 880, 551, 551, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 896, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 463, 463, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 440, 440, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1037, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1079, 395, 395, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1144, 387, 387, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1192, 410, 410, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1240, 435, 435, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 455, 455, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1304, 499, 499, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1336, 531, 531, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1368, 355, 355, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1380, 553, 553, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1411, 376, 376, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag350_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21E, 0x28C, 0x33C, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x222, 0x8C, 0xCE, 7, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0x38, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag350_funcs = { wstag350_setup };
FieldstgEventDef wstag350_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
