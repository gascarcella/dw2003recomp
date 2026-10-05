#include "wstag.h"

/* WSTAG486: stage 0x2A8 (fieldstg_stages). */

extern WstagFuncs wstag486_funcs;
const CVECTOR wstag486_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag486_battle_lists;
extern FieldstgVramPlace wstag486_vram_places[];
extern FieldstgPlacedActor *wstag486_actors[];
extern FieldstgSprite wstag486_sprites[];
extern FieldstgMapEvent wstag486_map_events[];
extern FieldstgEventDef wstag486_events[];

void wstag486_update(WstagObject *obj) {
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

WstagObject *wstag486_start(void *arg0) {
    WstagObject *obj = object_new(wstag486_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag486_funcs.setup();
    return obj;
}

void wstag486_event_745_end(void) {
    gamestate_flags.set_flag(0x40A8, 1);
}

void wstag486_setup(void) {
    fieldstg_stage.background_file = 0x5BD;
    fieldstg_stage.sprite_file = 0x05BE0000;
    fieldstg_stage.sprites = wstag486_sprites;
    fieldstg_stage.map_events = wstag486_map_events;
    fieldstg_stage.mask_file = 0x5BC;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19500, 0xD000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag486_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag486_actors;
    fieldstg_stage.color = wstag486_color;
    fieldstg_stage.battle_lists = &wstag486_battle_lists;
    fieldstg_stage.events = wstag486_events;
    fieldstg_attr.set_file(0, 0x05BE0001);
    fieldstg_attr.set_file(7, 0x05BE0002);
    fieldstg_attr.set_file(4, 0x05BE0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag486_setup(void);

s16 D_WSTAG486_800A5FE8[119] = {
    FIELDSTG_EVENT_WALK(2, 1281, 217, 7),
    FIELDSTG_EVENT_PLACE(306, 1313, 233),
    FIELDSTG_EVENT_ANIM(306, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 306),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 306),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 306, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 306, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(306, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(306, 1376, 265, 7),
    FIELDSTG_EVENT_WAIT_WALK(306),
    FIELDSTG_EVENT_ANIM(306, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 306, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1352, 252, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 1416, 220, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG486_800A60D8 = { 109, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A60E4 = { 109, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A60F0 = { 109, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A60FC = { 176, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A6108 = { 176, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A6114 = { 176, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A6120 = { 157, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A612C = { 157, 8, 0x60080000 };
FieldstgBattleList D_WSTAG486_800A6138 = {
    3,
    { &D_WSTAG486_800A60D8, &D_WSTAG486_800A60E4, &D_WSTAG486_800A60F0, &D_WSTAG486_800A60FC, &D_WSTAG486_800A6108,
        &D_WSTAG486_800A6114, &D_WSTAG486_800A6120, &D_WSTAG486_800A612C },
};
FieldstgListedBattle D_WSTAG486_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A61B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG486_800A61BC = {
    0,
    { &D_WSTAG486_800A615C, &D_WSTAG486_800A6168, &D_WSTAG486_800A6174, &D_WSTAG486_800A6180, &D_WSTAG486_800A618C,
        &D_WSTAG486_800A6198, &D_WSTAG486_800A61A4, &D_WSTAG486_800A61B0 },
};
FieldstgListedBattle D_WSTAG486_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A61EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A61F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6210 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A621C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6234 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG486_800A6240 = {
    0,
    { &D_WSTAG486_800A61E0, &D_WSTAG486_800A61EC, &D_WSTAG486_800A61F8, &D_WSTAG486_800A6204, &D_WSTAG486_800A6210,
        &D_WSTAG486_800A621C, &D_WSTAG486_800A6228, &D_WSTAG486_800A6234 },
};
FieldstgListedBattle D_WSTAG486_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A6288 = { 331, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A6294 = { 332, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A62A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG486_800A62AC = { 177, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG486_800A62B8 = { 106, 8, 0x60080000 };
FieldstgBattleList D_WSTAG486_800A62C4 = {
    0,
    { &D_WSTAG486_800A6264, &D_WSTAG486_800A6270, &D_WSTAG486_800A627C, &D_WSTAG486_800A6288, &D_WSTAG486_800A6294,
        &D_WSTAG486_800A62A0, &D_WSTAG486_800A62AC, &D_WSTAG486_800A62B8 },
};
FieldstgBattleLists wstag486_battle_lists = {
    75, 0, 0, { &D_WSTAG486_800A6138, &D_WSTAG486_800A61BC, &D_WSTAG486_800A6240 }, &D_WSTAG486_800A62C4,
};
FieldstgVramPlace wstag486_vram_places[16] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 368, 224, 112, 368, 511 }, { 320, 256, 356, 432, 144, 176, 352, 510 },
    { 320, 256, 364, 432, 176, 176, 368, 510 }, { 320, 256, 372, 432, 208, 176, 352, 509 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 320, 256, 342, 448, 88, 192, 368, 509 }, { 320, 256, 320, 459, 0, 203, 352, 508 },
    { 320, 256, 374, 472, 216, 216, 368, 508 }, { 384, 256, 404, 256, 336, 0, 336, 507 },
    { 320, 256, 328, 459, 32, 203, 352, 507 },
};
u16 D_WSTAG486_800A6404[4] = { 0x1A35, 0, 0xFFFF, 0 };
u16 D_WSTAG486_800A640C[6] = { 0x904A, 1, 0x1A35, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A6418[4] = { 0x1A35, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG486_800A6420[2] = { { NULL, NULL, 258 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6438[2] = { { NULL, NULL, 255 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6450[2] = { { NULL, NULL, 257 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6468[2] = { { NULL, NULL, 252 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6480[2] = { { NULL, NULL, 250 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6498[2] = { { NULL, NULL, 807 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A64B0[2] = { { NULL, NULL, 806 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A64C8[2] = { { NULL, NULL, 804 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A64E0[2] = { { NULL, NULL, 810 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A64F8[2] = { { NULL, NULL, 251 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6510[2] = { { NULL, NULL, 253 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6528[2] = { { NULL, NULL, 254 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6540[2] = { { NULL, NULL, 256 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6558[2] = { { NULL, NULL, 804 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG486_800A6570[3] = {
    { D_WSTAG486_800A6404, D_WSTAG486_800A640C, 805 }, { D_WSTAG486_800A6418, NULL, 806 }, { NULL, NULL, 0 },
};
u16 D_WSTAG486_800A6594[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A659C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65A8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65B0[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65BC[6] = { 0x601C, 0, 0x701F, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65C8[6] = { 0x601C, 1, 0x1A35, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65D4[4] = { 0x601B, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65DC[4] = { 0x701F, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65E4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG486_800A65F0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A65F8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG486_800A6604[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A660C[4] = { 0x601B, 1, 0xFFFF, 0 };
u16 D_WSTAG486_800A6614[6] = { 0x1A35, 0, 0x601C, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG486_800A6620 = { D_WSTAG486_800A6594, D_WSTAG486_800A6420, 45, 4, 944, 185, 1 };
FieldstgPlacedActor D_WSTAG486_800A6634 = { D_WSTAG486_800A659C, D_WSTAG486_800A6438, 49, 5, 1394, 778, 7 };
FieldstgPlacedActor D_WSTAG486_800A6648 = { D_WSTAG486_800A65A8, D_WSTAG486_800A6450, 51, 6, 1394, 778, 7 };
FieldstgPlacedActor D_WSTAG486_800A665C = { D_WSTAG486_800A65B0, D_WSTAG486_800A6468, 52, 7, 944, 185, 1 };
FieldstgPlacedActor D_WSTAG486_800A6670 = { NULL, D_WSTAG486_800A6480, 63, 8, 529, 249, 1 };
FieldstgPlacedActor D_WSTAG486_800A6684 = { D_WSTAG486_800A65BC, D_WSTAG486_800A6498, 69, 9, 1376, 265, 7 };
FieldstgPlacedActor D_WSTAG486_800A6698 = { D_WSTAG486_800A65C8, D_WSTAG486_800A64B0, 69, 9, 1376, 265, 7 };
FieldstgPlacedActor D_WSTAG486_800A66AC = { D_WSTAG486_800A65D4, D_WSTAG486_800A64C8, 70, 10, 1344, 208, 3 };
FieldstgPlacedActor D_WSTAG486_800A66C0 = { D_WSTAG486_800A65DC, D_WSTAG486_800A64E0, 70, 10, 1344, 208, 5 };
FieldstgPlacedActor D_WSTAG486_800A66D4 = { D_WSTAG486_800A65E4, D_WSTAG486_800A64F8, 157, 11, 944, 185, 1 };
FieldstgPlacedActor D_WSTAG486_800A66E8 = { D_WSTAG486_800A65F0, D_WSTAG486_800A6510, 157, 11, 944, 185, 1 };
FieldstgPlacedActor D_WSTAG486_800A66FC = { D_WSTAG486_800A65F8, D_WSTAG486_800A6528, 158, 12, 1394, 778, 7 };
FieldstgPlacedActor D_WSTAG486_800A6710 = { D_WSTAG486_800A6604, D_WSTAG486_800A6540, 158, 12, 1394, 778, 7 };
FieldstgPlacedActor D_WSTAG486_800A6724 = { D_WSTAG486_800A660C, D_WSTAG486_800A6558, 306, 13, 1313, 233, 3 };
FieldstgPlacedActor D_WSTAG486_800A6738 = { D_WSTAG486_800A6614, D_WSTAG486_800A6570, 306, 13, 1313, 233, 3 };
FieldstgPlacedActor *wstag486_actors[16] = {
    &D_WSTAG486_800A6620, &D_WSTAG486_800A6634, &D_WSTAG486_800A6648, &D_WSTAG486_800A665C, &D_WSTAG486_800A6670,
    &D_WSTAG486_800A6684, &D_WSTAG486_800A6698, &D_WSTAG486_800A66AC, &D_WSTAG486_800A66C0, &D_WSTAG486_800A66D4,
    &D_WSTAG486_800A66E8, &D_WSTAG486_800A66FC, &D_WSTAG486_800A6710, &D_WSTAG486_800A6724, &D_WSTAG486_800A6738,
    NULL,
};
FieldstgSprite wstag486_sprites[56] = {
    { 1, 0, 0x40, 2, 0x4B, 2, 0, 3, 6, 0, 1729, 477, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1400, 117, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1448, 93, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1456, 145, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1496, 69, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1504, 121, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1544, 45, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1552, 97, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1592, 21, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1600, 73, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1640, -3, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1648, 49, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1688, -27, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1696, 25, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1744, 1, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1792, -23, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 308, 158, 0, 0 },
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 924, 626, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 1732, 319, 0, 0 },
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 1620, 640, 0, 0 }, { 1, 0, 0x4E, 2, 0xA, 0, 0, 0, 0, 0, 1664, 384, 0, 0 },
    { 1, 0, 0x50, 2, 2, 0, 0, 0, 0, 0, 1536, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1083, 205, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1107, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1648, 565, 0, 0 },
    { 1, 0, 0x40, 6, 0x4C, 2, 0, 3, 6, 0, 1800, 729, 0, 0 }, { 1, 0, 0x40, 6, 0x4D, 2, 0, 3, 6, 0, 1255, 43, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 460, 369, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 914, 808, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 935, 499, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 962, 255, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1379, 661, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 1508, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 661, 400, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 683, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 941, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1136, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1322, 331, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 516, 208, 247, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 326, 181, 289, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 448, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 544, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 191, 191, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 703, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 599, 599, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1056, 143, 143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 143, 143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1456, 711, 711, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1488, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1503, 687, 687, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1536, 719, 719, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag486_map_events[13] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A9, 0x48E, 0x3F8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AB, 0xA0, 0x390, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A7, 0x3E8, 0x9C, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 0xE, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x38, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x20, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x30, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 0xE, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag486_funcs = { wstag486_setup };
FieldstgEventDef wstag486_events[2] = {
    { 745, D_WSTAG486_800A5FE8, 0x0135002C, NULL, wstag486_event_745_end }, { -1, NULL, 0, NULL, NULL },
};
