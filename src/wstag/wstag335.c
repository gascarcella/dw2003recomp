#include "wstag.h"

/* WSTAG335: stage 0x21E (fieldstg_stages). Its stage object creates nothing. */

/* The data of the stage object (wstag335_update): never written. */
typedef struct Wstag335Data {
    /* 0x00 */ Object *unk_0[14];
} Wstag335Data; /* size 0x38 */

extern WstagFuncs wstag335_funcs;
const CVECTOR wstag335_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgBattleLists wstag335_battle_lists;
extern FieldstgBattleLists wstag335_battle_lists2;
extern FieldstgBattleLists wstag335_battle_lists3;
extern FieldstgVramPlace wstag335_vram_places[];
extern FieldstgPlacedActor *wstag335_actors[];
extern FieldstgSprite wstag335_sprites[];
extern FieldstgMapEvent wstag335_map_events[];

void wstag335_update(WstagObject *obj, Wstag335Data *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 14; i++) {
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag335_start(void *arg0) {
    WstagObject *obj = object_new(wstag335_update, sizeof(WstagObject), sizeof(Wstag335Data));

    obj->manager = arg0;
    wstag335_funcs.setup();
    return obj;
}

void wstag335_setup(void) {
    fieldstg_stage.background_file = 0x1B6;
    fieldstg_stage.sprite_file = 0x01B70000;
    fieldstg_stage.sprites = wstag335_sprites;
    fieldstg_stage.map_events = wstag335_map_events;
    fieldstg_stage.mask_file = 0x2D2;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19000, 0x12C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag335_vram_places;
    fieldstg_stage.music = 9;
    fieldstg_stage.sound = 0x60240000;
    fieldstg_stage.actors = wstag335_actors;
    fieldstg_stage.color = wstag335_color;
    fieldstg_attr.set_file(0, 0x01B70001);
    fieldstg_attr.set_file(7, 0x01B70002);
    fieldstg_attr.set_file(4, 0x01B70003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress < 0xB) {
        fieldstg_stage.battle_lists = &wstag335_battle_lists;
    } else if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = &wstag335_battle_lists2;
    } else {
        fieldstg_stage.battle_lists = &wstag335_battle_lists3;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag335_setup(void);

FieldstgListedBattle D_WSTAG335_800A601C = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6028 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6034 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6040 = { 34, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A604C = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6058 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6064 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6070 = { 35, 1, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A607C = {
    3,
    { &D_WSTAG335_800A601C, &D_WSTAG335_800A6028, &D_WSTAG335_800A6034, &D_WSTAG335_800A6040, &D_WSTAG335_800A604C,
        &D_WSTAG335_800A6058, &D_WSTAG335_800A6064, &D_WSTAG335_800A6070 },
};
FieldstgListedBattle D_WSTAG335_800A60A0 = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60AC = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60B8 = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60C4 = { 34, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60D0 = { 35, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60DC = { 35, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60E8 = { 35, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A60F4 = { 35, 13, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6100 = {
    3,
    { &D_WSTAG335_800A60A0, &D_WSTAG335_800A60AC, &D_WSTAG335_800A60B8, &D_WSTAG335_800A60C4, &D_WSTAG335_800A60D0,
        &D_WSTAG335_800A60DC, &D_WSTAG335_800A60E8, &D_WSTAG335_800A60F4 },
};
FieldstgListedBattle D_WSTAG335_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A613C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6160 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6178 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG335_800A6184 = {
    0,
    { &D_WSTAG335_800A6124, &D_WSTAG335_800A6130, &D_WSTAG335_800A613C, &D_WSTAG335_800A6148, &D_WSTAG335_800A6154,
        &D_WSTAG335_800A6160, &D_WSTAG335_800A616C, &D_WSTAG335_800A6178 },
};
FieldstgListedBattle D_WSTAG335_800A61A8 = { 202, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG335_800A61B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A61C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A61CC = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A61D8 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A61E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A61F0 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A61FC = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6208 = {
    0,
    { &D_WSTAG335_800A61A8, &D_WSTAG335_800A61B4, &D_WSTAG335_800A61C0, &D_WSTAG335_800A61CC, &D_WSTAG335_800A61D8,
        &D_WSTAG335_800A61E4, &D_WSTAG335_800A61F0, &D_WSTAG335_800A61FC },
};
FieldstgListedBattle D_WSTAG335_800A622C = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6238 = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6244 = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6250 = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A625C = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6268 = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6274 = { 35, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6280 = { 35, 1, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A628C = {
    3,
    { &D_WSTAG335_800A622C, &D_WSTAG335_800A6238, &D_WSTAG335_800A6244, &D_WSTAG335_800A6250, &D_WSTAG335_800A625C,
        &D_WSTAG335_800A6268, &D_WSTAG335_800A6274, &D_WSTAG335_800A6280 },
};
FieldstgListedBattle D_WSTAG335_800A62B0 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62BC = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62C8 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62D4 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62E0 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62EC = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A62F8 = { 35, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6304 = { 35, 13, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6310 = {
    3,
    { &D_WSTAG335_800A62B0, &D_WSTAG335_800A62BC, &D_WSTAG335_800A62C8, &D_WSTAG335_800A62D4, &D_WSTAG335_800A62E0,
        &D_WSTAG335_800A62EC, &D_WSTAG335_800A62F8, &D_WSTAG335_800A6304 },
};
FieldstgListedBattle D_WSTAG335_800A6334 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6340 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A634C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6388 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG335_800A6394 = {
    0,
    { &D_WSTAG335_800A6334, &D_WSTAG335_800A6340, &D_WSTAG335_800A634C, &D_WSTAG335_800A6358, &D_WSTAG335_800A6364,
        &D_WSTAG335_800A6370, &D_WSTAG335_800A637C, &D_WSTAG335_800A6388 },
};
FieldstgListedBattle D_WSTAG335_800A63B8 = { 202, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG335_800A63C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A63D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A63DC = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A63E8 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6400 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A640C = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6418 = {
    0,
    { &D_WSTAG335_800A63B8, &D_WSTAG335_800A63C4, &D_WSTAG335_800A63D0, &D_WSTAG335_800A63DC, &D_WSTAG335_800A63E8,
        &D_WSTAG335_800A63F4, &D_WSTAG335_800A6400, &D_WSTAG335_800A640C },
};
FieldstgListedBattle D_WSTAG335_800A643C = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6448 = { 48, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6454 = { 146, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6460 = { 146, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A646C = { 146, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6478 = { 146, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6484 = { 146, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6490 = { 146, 1, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A649C = {
    3,
    { &D_WSTAG335_800A643C, &D_WSTAG335_800A6448, &D_WSTAG335_800A6454, &D_WSTAG335_800A6460, &D_WSTAG335_800A646C,
        &D_WSTAG335_800A6478, &D_WSTAG335_800A6484, &D_WSTAG335_800A6490 },
};
FieldstgListedBattle D_WSTAG335_800A64C0 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A64CC = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A64D8 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A64E4 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A64F0 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A64FC = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6508 = { 146, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6514 = { 146, 13, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6520 = {
    3,
    { &D_WSTAG335_800A64C0, &D_WSTAG335_800A64CC, &D_WSTAG335_800A64D8, &D_WSTAG335_800A64E4, &D_WSTAG335_800A64F0,
        &D_WSTAG335_800A64FC, &D_WSTAG335_800A6508, &D_WSTAG335_800A6514 },
};
FieldstgListedBattle D_WSTAG335_800A6544 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6550 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A655C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6568 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6574 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A658C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6598 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG335_800A65A4 = {
    0,
    { &D_WSTAG335_800A6544, &D_WSTAG335_800A6550, &D_WSTAG335_800A655C, &D_WSTAG335_800A6568, &D_WSTAG335_800A6574,
        &D_WSTAG335_800A6580, &D_WSTAG335_800A658C, &D_WSTAG335_800A6598 },
};
FieldstgListedBattle D_WSTAG335_800A65C8 = { 202, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG335_800A65D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A65E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A65EC = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A65F8 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A6604 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG335_800A6610 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG335_800A661C = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG335_800A6628 = {
    0,
    { &D_WSTAG335_800A65C8, &D_WSTAG335_800A65D4, &D_WSTAG335_800A65E0, &D_WSTAG335_800A65EC, &D_WSTAG335_800A65F8,
        &D_WSTAG335_800A6604, &D_WSTAG335_800A6610, &D_WSTAG335_800A661C },
};
FieldstgBattleLists wstag335_battle_lists = {
    2, 0, 0, { &D_WSTAG335_800A607C, &D_WSTAG335_800A6100, &D_WSTAG335_800A6184 }, &D_WSTAG335_800A6208,
};
FieldstgBattleLists wstag335_battle_lists2 = {
    25, 1, 0, { &D_WSTAG335_800A628C, &D_WSTAG335_800A6310, &D_WSTAG335_800A6394 }, &D_WSTAG335_800A6418,
};
FieldstgBattleLists wstag335_battle_lists3 = {
    50, 2, 0, { &D_WSTAG335_800A649C, &D_WSTAG335_800A6520, &D_WSTAG335_800A65A4 }, &D_WSTAG335_800A6628,
};
FieldstgVramPlace wstag335_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 364, 256, 176, 0, 336, 511 }, { 320, 256, 372, 256, 208, 0, 352, 511 },
};
u16 D_WSTAG335_800A6720[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6728[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6734[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A673C[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6748[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6754[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A675C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6764[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6770[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A677C[4] = { 0x7606, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6784[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A678C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6794[6] = { 0x7209, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67A0[6] = { 0x7209, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67AC[4] = { 0x7606, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67B4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A67BC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67C4[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A67D0[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67DC[4] = { 0x7606, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67E4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A67EC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A67F4[6] = { 0, 1, 0xE02, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6800[6] = { 0xE02, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A680C[8] = { 0, 1, 0xE02, 1, 0x720D, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A681C[8] = { 0, 1, 0xE02, 1, 0x720D, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A682C[4] = { 0x7806, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6834[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A683C[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6848[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6850[8] = { 0x10, 1, 0x920D, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6860[10] = {
    0x920D, 1, 0x11, 0, 0x7013, 1, 0x10, 0,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6874[8] = { 0x10, 1, 0x920D, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A6884[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6890[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6898[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A68A4[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG335_800A68B0[4] = { 0x7606, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG335_800A68B8[4] = {
    { D_WSTAG335_800A6720, NULL, 108 }, { D_WSTAG335_800A6728, D_WSTAG335_800A6734, 41 },
    { D_WSTAG335_800A673C, D_WSTAG335_800A6748, 42 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A68E8[4] = {
    { D_WSTAG335_800A6754, D_WSTAG335_800A675C, 108 }, { D_WSTAG335_800A6764, NULL, 112 },
    { D_WSTAG335_800A6770, D_WSTAG335_800A677C, 113 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A6918[4] = {
    { D_WSTAG335_800A6784, D_WSTAG335_800A678C, 109 }, { D_WSTAG335_800A6794, NULL, 112 },
    { D_WSTAG335_800A67A0, D_WSTAG335_800A67AC, 113 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A6948[4] = {
    { D_WSTAG335_800A67B4, D_WSTAG335_800A67BC, 110 }, { D_WSTAG335_800A67C4, NULL, 112 },
    { D_WSTAG335_800A67D0, D_WSTAG335_800A67DC, 113 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A6978[5] = {
    { D_WSTAG335_800A67E4, D_WSTAG335_800A67EC, 116 }, { D_WSTAG335_800A67F4, D_WSTAG335_800A6800, 117 },
    { D_WSTAG335_800A680C, NULL, 118 }, { D_WSTAG335_800A681C, D_WSTAG335_800A682C, 119 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A69B4[5] = {
    { D_WSTAG335_800A6834, NULL, 108 }, { D_WSTAG335_800A683C, D_WSTAG335_800A6848, 43 },
    { D_WSTAG335_800A6850, D_WSTAG335_800A6860, 44 }, { D_WSTAG335_800A6874, D_WSTAG335_800A6884, 45 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG335_800A69F0[2] = { { NULL, NULL, 625 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG335_800A6A08[4] = {
    { D_WSTAG335_800A6890, NULL, 111 }, { D_WSTAG335_800A6898, NULL, 111 },
    { D_WSTAG335_800A68A4, D_WSTAG335_800A68B0, 111 }, { NULL, NULL, 0 },
};
u16 D_WSTAG335_800A6A38[10] = {
    0x11, 1, 0x8192, 1, 0x700A, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6A4C[10] = {
    0x7003, 1, 0x8192, 1, 0x11, 0, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6A60[10] = {
    0x7004, 1, 0x8192, 1, 0x11, 0, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6A74[10] = {
    0x11, 0, 0x8192, 1, 0x6026, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6A88[10] = {
    0x8192, 1, 0x11, 0, 0x8012, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6A9C[10] = {
    0x8192, 1, 0x11, 1, 0x8012, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG335_800A6AB0[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG335_800A6AC0[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG335_800A6AC8 = { D_WSTAG335_800A6A38, D_WSTAG335_800A68B8, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6ADC = { D_WSTAG335_800A6A4C, D_WSTAG335_800A68E8, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6AF0 = { D_WSTAG335_800A6A60, D_WSTAG335_800A6918, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6B04 = { D_WSTAG335_800A6A74, D_WSTAG335_800A6948, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6B18 = { D_WSTAG335_800A6A88, D_WSTAG335_800A6978, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6B2C = { D_WSTAG335_800A6A9C, D_WSTAG335_800A69B4, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6B40 = { D_WSTAG335_800A6AB0, D_WSTAG335_800A69F0, 47, 4, 593, 289, 1 };
FieldstgPlacedActor D_WSTAG335_800A6B54 = { D_WSTAG335_800A6AC0, D_WSTAG335_800A6A08, 157, 5, 593, 289, 1 };
FieldstgPlacedActor *wstag335_actors[9] = {
    &D_WSTAG335_800A6AC8, &D_WSTAG335_800A6ADC, &D_WSTAG335_800A6AF0, &D_WSTAG335_800A6B04, &D_WSTAG335_800A6B18,
    &D_WSTAG335_800A6B2C, &D_WSTAG335_800A6B40, &D_WSTAG335_800A6B54, NULL,
};
FieldstgSprite wstag335_sprites[44] = {
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 199, 607, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 353, -16, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 8, 0, 513, 285, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 113, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 153, 554, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 167, 539, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 192, 494, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 225, 493, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 251, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 313, 468, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 86, 400, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 169, 500, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 193, 527, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 216, 519, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 235, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 248, 454, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 253, 491, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 264, 469, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 58, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 130, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 214, 356, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 233, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 204, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 273, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 219, 483, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 289, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 307, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 317, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 323, 421, 0, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 226, 226, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 232, 250, 250, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 243, 544, 544, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 267, 602, 602, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 271, 135, 135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 277, 521, 521, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 399, 167, 167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 422, 187, 187, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 431, 568, 568, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 489, 501, 501, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 506, 203, 203, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 542, 228, 228, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 641, 286, 286, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag335_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21D, 0x5F4, 0x39E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x221, 0x138, 0x7A, 7, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC8, 0xFFF0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag335_funcs = { wstag335_setup };
