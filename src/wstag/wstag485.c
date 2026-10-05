#include "wstag.h"

/* WSTAG485: stage 0x23B (fieldstg_stages). */

extern WstagFuncs wstag485_funcs;
extern CVECTOR wstag485_color;
extern FieldstgBattleLists wstag485_battle_lists;
extern FieldstgVramPlace wstag485_vram_places[];
extern FieldstgPlacedActor *wstag485_actors[];
extern FieldstgSprite wstag485_sprites[];
extern FieldstgMapEvent wstag485_map_events[];
extern FieldstgEventDef wstag485_events[];

void wstag485_update(WstagObject *obj) {
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

WstagObject *wstag485_start(void *arg0) {
    WstagObject *obj = object_new(wstag485_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag485_funcs.setup();
    return obj;
}

void wstag485_event_1530_end(void) {
    gamestate_flags.set_flag(0x1C51, 1);
}

void wstag485_setup(void) {
    fieldstg_stage.background_file = 0x3B5;
    fieldstg_stage.sprite_file = 0x03B60000;
    fieldstg_stage.sprites = wstag485_sprites;
    fieldstg_stage.map_events = wstag485_map_events;
    fieldstg_stage.mask_file = 0x3B4;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x59100, 0x2E900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag485_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag485_actors;
    fieldstg_stage.color = wstag485_color;
    fieldstg_stage.battle_lists = &wstag485_battle_lists;
    fieldstg_stage.events = wstag485_events;
    fieldstg_attr.set_file(0, 0x03B60001);
    fieldstg_attr.set_file(7, 0x03B60002);
    fieldstg_attr.set_file(4, 0x03B60003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag485/nonmatchings/wstag485", wstag485_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag485_setup(void);

s16 D_WSTAG485_800A5FEC[92] = {
    FIELDSTG_EVENT_WALK(2, 1344, 249, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG485_800A60A4 = { 49, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60B0 = { 49, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60BC = { 145, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60C8 = { 145, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60D4 = { 145, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60E0 = { 58, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60EC = { 58, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A60F8 = { 58, 8, 0x60080000 };
FieldstgBattleList D_WSTAG485_800A6104 = {
    3,
    { &D_WSTAG485_800A60A4, &D_WSTAG485_800A60B0, &D_WSTAG485_800A60BC, &D_WSTAG485_800A60C8, &D_WSTAG485_800A60D4,
        &D_WSTAG485_800A60E0, &D_WSTAG485_800A60EC, &D_WSTAG485_800A60F8 },
};
FieldstgListedBattle D_WSTAG485_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A617C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG485_800A6188 = {
    0,
    { &D_WSTAG485_800A6128, &D_WSTAG485_800A6134, &D_WSTAG485_800A6140, &D_WSTAG485_800A614C, &D_WSTAG485_800A6158,
        &D_WSTAG485_800A6164, &D_WSTAG485_800A6170, &D_WSTAG485_800A617C },
};
FieldstgListedBattle D_WSTAG485_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A61F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6200 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG485_800A620C = {
    0,
    { &D_WSTAG485_800A61AC, &D_WSTAG485_800A61B8, &D_WSTAG485_800A61C4, &D_WSTAG485_800A61D0, &D_WSTAG485_800A61DC,
        &D_WSTAG485_800A61E8, &D_WSTAG485_800A61F4, &D_WSTAG485_800A6200 },
};
FieldstgListedBattle D_WSTAG485_800A6230 = { 11, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG485_800A623C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6248 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6254 = { 329, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A6260 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A626C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG485_800A6278 = { 48, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG485_800A6284 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG485_800A6290 = {
    0,
    { &D_WSTAG485_800A6230, &D_WSTAG485_800A623C, &D_WSTAG485_800A6248, &D_WSTAG485_800A6254, &D_WSTAG485_800A6260,
        &D_WSTAG485_800A626C, &D_WSTAG485_800A6278, &D_WSTAG485_800A6284 },
};
FieldstgBattleLists wstag485_battle_lists = {
    19, 0, 0, { &D_WSTAG485_800A6104, &D_WSTAG485_800A6188, &D_WSTAG485_800A620C }, &D_WSTAG485_800A6290,
};
FieldstgVramPlace wstag485_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 340, 453, 80, 197, 352, 510 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 372, 460, 208, 204, 352, 509 },
};
u16 D_WSTAG485_800A6360[4] = { 0x1C1C, 0, 0xFFFF, 0 };
u16 D_WSTAG485_800A6368[4] = { 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A6370[4] = { 0x1A02, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG485_800A6378[3] = {
    { D_WSTAG485_800A6360, NULL, 316 }, { D_WSTAG485_800A6368, NULL, 62 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG485_800A639C[2] = { { NULL, NULL, 320 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A63B4[2] = { { NULL, NULL, 316 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A63CC[2] = { { NULL, NULL, 318 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A63E4[2] = { { NULL, D_WSTAG485_800A6370, 314 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A63FC[2] = { { NULL, NULL, 319 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A6414[2] = { { NULL, NULL, 315 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A642C[2] = { { NULL, NULL, 317 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A6444[2] = { { NULL, NULL, 318 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A645C[2] = { { NULL, NULL, 763 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG485_800A6474[2] = { { NULL, NULL, 321 }, { NULL, NULL, 0 } };
u16 D_WSTAG485_800A648C[4] = { 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A6494[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A649C[6] = { 0x7017, 1, 0x6014, 0, 0xFFFF, 0 };
u16 D_WSTAG485_800A64A8[4] = { 0x6019, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64B0[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64B8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64C0[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64C8[4] = { 0x6018, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64D0[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG485_800A64D8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG485_800A64E0 = { D_WSTAG485_800A648C, D_WSTAG485_800A6378, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A64F4 = { D_WSTAG485_800A6494, D_WSTAG485_800A639C, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6508 = { D_WSTAG485_800A649C, D_WSTAG485_800A63B4, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A651C = { D_WSTAG485_800A64A8, D_WSTAG485_800A63CC, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6530 = { D_WSTAG485_800A64B0, D_WSTAG485_800A63E4, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6544 = { D_WSTAG485_800A64B8, D_WSTAG485_800A63FC, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6558 = { D_WSTAG485_800A64C0, D_WSTAG485_800A6414, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A656C = { D_WSTAG485_800A64C8, D_WSTAG485_800A642C, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6580 = { D_WSTAG485_800A64D0, D_WSTAG485_800A6444, 50, 4, 1201, 505, 1 };
FieldstgPlacedActor D_WSTAG485_800A6594 = { NULL, D_WSTAG485_800A645C, 63, 5, 529, 249, 1 };
FieldstgPlacedActor D_WSTAG485_800A65A8 = { D_WSTAG485_800A64D8, D_WSTAG485_800A6474, 157, 6, 1201, 505, 1 };
FieldstgPlacedActor *wstag485_actors[12] = {
    &D_WSTAG485_800A64E0, &D_WSTAG485_800A64F4, &D_WSTAG485_800A6508, &D_WSTAG485_800A651C, &D_WSTAG485_800A6530,
    &D_WSTAG485_800A6544, &D_WSTAG485_800A6558, &D_WSTAG485_800A656C, &D_WSTAG485_800A6580, &D_WSTAG485_800A6594,
    &D_WSTAG485_800A65A8, NULL,
};
FieldstgSprite wstag485_sprites[199] = {
    { 1, 0, 0x40, 2, 0x4B, 2, 0, 3, 6, 0, 1729, 477, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1400, 117, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1448, 93, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1456, 145, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1496, 69, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1504, 121, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1544, 45, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1552, 97, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1592, 21, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1600, 73, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1640, -3, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1648, 49, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1688, -27, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1696, 25, 0, 0 }, { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1744, 1, 0, 0 },
    { 1, 0, 0x40, 2, 0x4E, 2, 0, 5, 8, 0, 1792, -23, 0, 0 }, { 1, 0, 0x50, 2, 2, 0, 0, 0, 0, 0, 1536, 384, 0, 0 },
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 314, 157, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 930, 627, 0, 0 },
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 1739, 314, 0, 0 }, { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1083, 205, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1107, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 1648, 565, 0, 0 },
    { 1, 0, 0x40, 6, 0x4C, 2, 0, 3, 6, 0, 1800, 729, 0, 0 }, { 1, 0, 0x40, 6, 0x4D, 2, 0, 3, 6, 0, 1255, 43, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 486, 740, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 489, 491, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 538, 376, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 674, 309, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 737, 437, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 820, 549, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 883, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 939, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1014, 770, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1128, 261, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1149, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1187, 590, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1222, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1235, 140, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1326, 660, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1433, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1475, 270, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1542, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 429, 365, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 507, 493, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 683, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 996, 464, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1103, 755, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1157, 342, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1234, 767, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1276, 127, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1278, 535, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1321, 670, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1322, 307, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1415, 286, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1439, 373, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1452, 861, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 656, 788, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 756, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 872, 521, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 911, 306, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 937, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1035, 770, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1158, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1511, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1550, 241, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1675, 49, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 452, 673, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 667, 643, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 744, 645, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 751, 777, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1119, 599, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 771, 429, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 814, 812, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 856, 779, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1020, 255, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1370, 572, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 409, 646, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 538, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 749, 734, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 769, 821, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 793, 670, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1079, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1180, 361, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1203, 218, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1438, 598, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1481, 292, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1571, 262, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 368, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 371, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 372, 735, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 405, 391, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 491, 498, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 497, 352, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 632, 645, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 640, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 651, 690, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 655, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 978, 784, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1038, 534, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1045, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1085, 296, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1182, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1184, 840, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1214, 858, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1231, 592, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1270, 856, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1286, 890, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1287, 587, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1314, 570, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1339, 668, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1339, 895, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1351, 489, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1361, 492, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1362, 481, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1448, 887, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1459, 371, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1494, 886, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1500, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1514, 50, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1522, 210, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1569, 188, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1590, 65, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1619, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1695, 288, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 333, 710, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 394, 594, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 395, 352, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 516, 759, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 569, 322, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 584, 508, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 626, 772, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 694, 261, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 704, 409, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 715, 614, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 813, 436, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 817, 219, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 830, 514, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 835, 308, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 873, 761, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 933, 410, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 960, 562, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 989, 213, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1070, 472, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1087, 243, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1123, 218, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1211, 54, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1222, 329, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1230, 538, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1258, 812, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1259, 235, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1277, 434, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1287, 94, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1360, 304, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1364, 172, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1364, 852, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1406, 433, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1411, 681, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1437, 264, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1469, 319, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1493, 855, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1503, 605, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1525, 22, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0xA, 0, 1685, 316, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 449, 531, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 515, 360, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 640, 706, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 748, 757, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 819, 668, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 853, 811, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1075, 610, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1113, 768, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1170, 287, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1172, 393, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1230, 788, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1273, 179, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1277, 566, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1307, 312, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1312, 717, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 0xA, 0, 1506, 100, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 516, 208, 247, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 326, 181, 289, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 400, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 448, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 544, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 191, 191, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 703, 183, 183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 159, 159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 151, 151, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 143, 143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 599, 599, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1056, 143, 143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 143, 143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1456, 711, 711, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1488, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1503, 687, 687, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1536, 719, 719, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag485_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23C, 0x48E, 0x3F8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23E, 0xA0, 0x390, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23A, 0x3E8, 0x9C, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 4, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x38, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x38, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFD0, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x20, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0x30, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 4, 1 },
    { 0x6019, 1, 0x1C51, 0, 8, 0x5FA, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag485_funcs = { wstag485_setup };
FieldstgEventDef wstag485_events[2] = {
    { 1530, D_WSTAG485_800A5FEC, 0x01350032, NULL, wstag485_event_1530_end }, { -1, NULL, 0, NULL, NULL },
};
