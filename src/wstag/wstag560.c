#include "wstag.h"

/* WSTAG560: stage 0x249 (fieldstg_stages). */

extern WstagFuncs wstag560_funcs;
extern CVECTOR wstag560_color;
extern FieldstgBattleLists wstag560_battle_lists;
extern FieldstgVramPlace wstag560_vram_places[];
extern FieldstgPlacedActor *wstag560_actors[];
extern FieldstgSprite wstag560_sprites[];
extern FieldstgMapEvent wstag560_map_events[];
extern FieldstgEventDef wstag560_events[];

void wstag560_update(WstagObject *obj) {
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

WstagObject *wstag560_start(void *arg0) {
    WstagObject *obj = object_new(wstag560_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag560_funcs.setup();
    return obj;
}

void wstag560_event_1459_end(void) {
    gamestate_flags.set_flag(0x7C16, 1);
}

void wstag560_setup(void) {
    fieldstg_stage.background_file = 0x2AF;
    fieldstg_stage.sprite_file = 0x02B00000;
    fieldstg_stage.sprites = wstag560_sprites;
    fieldstg_stage.map_events = wstag560_map_events;
    fieldstg_stage.mask_file = 0x324;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x25900, 0x1AB00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag560_vram_places;
    fieldstg_stage.music = 0x41;
    fieldstg_stage.sound = 0x61040000;
    fieldstg_stage.actors = wstag560_actors;
    fieldstg_stage.color = wstag560_color;
    fieldstg_stage.battle_lists = &wstag560_battle_lists;
    fieldstg_stage.events = wstag560_events;
    fieldstg_attr.set_file(0, 0x02B00001);
    fieldstg_attr.set_file(7, 0x02B00002);
    fieldstg_attr.set_file(4, 0x02B00003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag560/nonmatchings/wstag560", wstag560_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag560_setup(void);

s16 D_WSTAG560_800A5FEC[56] = {
    FIELDSTG_EVENT_WALK(2, 279, 307, 3),
    FIELDSTG_EVENT_PLACE(21, 247, 291),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(36),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC14, 0, 0, 0),
    FIELDSTG_EVENT_END,
    0x6004, /* padding, not read */
};
FieldstgListedBattle D_WSTAG560_800A605C = { 148, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6068 = { 148, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6074 = { 148, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6080 = { 148, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A608C = { 154, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6098 = { 154, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A60A4 = { 154, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A60B0 = { 154, 3, 0x60080000 };
FieldstgBattleList D_WSTAG560_800A60BC = {
    3,
    { &D_WSTAG560_800A605C, &D_WSTAG560_800A6068, &D_WSTAG560_800A6074, &D_WSTAG560_800A6080, &D_WSTAG560_800A608C,
        &D_WSTAG560_800A6098, &D_WSTAG560_800A60A4, &D_WSTAG560_800A60B0 },
};
FieldstgListedBattle D_WSTAG560_800A60E0 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A60EC = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A60F8 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6104 = { 148, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6110 = { 154, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A611C = { 154, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6128 = { 154, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6134 = { 154, 8, 0x60080000 };
FieldstgBattleList D_WSTAG560_800A6140 = {
    3,
    { &D_WSTAG560_800A60E0, &D_WSTAG560_800A60EC, &D_WSTAG560_800A60F8, &D_WSTAG560_800A6104, &D_WSTAG560_800A6110,
        &D_WSTAG560_800A611C, &D_WSTAG560_800A6128, &D_WSTAG560_800A6134 },
};
FieldstgListedBattle D_WSTAG560_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A61A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A61B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG560_800A61C4 = {
    0,
    { &D_WSTAG560_800A6164, &D_WSTAG560_800A6170, &D_WSTAG560_800A617C, &D_WSTAG560_800A6188, &D_WSTAG560_800A6194,
        &D_WSTAG560_800A61A0, &D_WSTAG560_800A61AC, &D_WSTAG560_800A61B8 },
};
FieldstgListedBattle D_WSTAG560_800A61E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A61F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6200 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A620C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6218 = { 330, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG560_800A6224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A6230 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG560_800A623C = { 61, 8, 0x60080000 };
FieldstgBattleList D_WSTAG560_800A6248 = {
    0,
    { &D_WSTAG560_800A61E8, &D_WSTAG560_800A61F4, &D_WSTAG560_800A6200, &D_WSTAG560_800A620C, &D_WSTAG560_800A6218,
        &D_WSTAG560_800A6224, &D_WSTAG560_800A6230, &D_WSTAG560_800A623C },
};
FieldstgBattleLists wstag560_battle_lists = {
    35, 0, 0, { &D_WSTAG560_800A60BC, &D_WSTAG560_800A6140, &D_WSTAG560_800A61C4 }, &D_WSTAG560_800A6248,
};
FieldstgVramPlace wstag560_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 413, 216, 157, 352, 511 }, { 448, 256, 490, 374, 680, 118, 368, 511 },
    { 320, 256, 374, 445, 216, 189, 336, 510 }, { 320, 256, 374, 373, 216, 117, 352, 510 },
};
u16 D_WSTAG560_800A6328[4] = { 0x7A30, 1, 0xFFFF, 0 };
u16 D_WSTAG560_800A6330[4] = { 0x9065, 1, 0xFFFF, 0 };
u16 D_WSTAG560_800A6338[8] = { 0x229, 1, 0x8236, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG560_800A6348[2] = { { NULL, D_WSTAG560_800A6328, 777 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG560_800A6360[2] = { { NULL, D_WSTAG560_800A6330, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG560_800A6378[2] = { { NULL, D_WSTAG560_800A6338, 622 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG560_800A6390[2] = { { NULL, NULL, 819 }, { NULL, NULL, 0 } };
u16 D_WSTAG560_800A63A8[4] = { 0x229, 0, 0xFFFF, 0 };
u16 D_WSTAG560_800A63B0[6] = { 0x7016, 1, 0x6012, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG560_800A63BC = { NULL, D_WSTAG560_800A6348, 20, 4, 450, 368, 1 };
FieldstgPlacedActor D_WSTAG560_800A63D0 = { NULL, D_WSTAG560_800A6360, 21, 5, 247, 291, 7 };
FieldstgPlacedActor D_WSTAG560_800A63E4 = { D_WSTAG560_800A63A8, D_WSTAG560_800A6378, 33, 6, 502, 546, 1 };
FieldstgPlacedActor D_WSTAG560_800A63F8 = { D_WSTAG560_800A63B0, D_WSTAG560_800A6390, 101, 7, 608, 240, 5 };
FieldstgPlacedActor *wstag560_actors[5] = {
    &D_WSTAG560_800A63BC, &D_WSTAG560_800A63D0, &D_WSTAG560_800A63E4, &D_WSTAG560_800A63F8, NULL,
};
FieldstgSprite wstag560_sprites[28] = {
    { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 410, 231, 0, 0 }, { 1, 0, 0x40, 2, 1, 1, 1, 6, 8, 0, 554, 520, 0, 0 },
    { 1, 0, 0x80, 2, 7, 0, 0, 0, 0, 0, 0, 384, 0, 0 }, { 1, 0, 0x80, 2, 8, 0, 0, 0, 0, 0, 384, 384, 0, 0 },
    { 1, 0, 0x70, 2, 9, 0, 0, 0, 0, 0, 20, 512, 0, 0 }, { 1, 0, 0x78, 2, 0xA, 0, 0, 0, 0, 0, 264, 523, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 94, 473, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 254, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 367, 431, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 386, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 160, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 298, 481, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 268, 346, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 347, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 155, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 197, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 208, 580, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 124, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 201, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 226, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 291, 355, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 309, 520, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 419, 451, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 6, 0, 204, 396, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4D, 6, 0, 213, 360, 0, 0 },
    { 1, 0, 0x68, 0xA, 0x4E, 2, 0, 2, 6, 0, 181, 376, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 584, 137, 169, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag560_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x248, 0x610, 0x240, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24A, 0x80, 0x178, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 5, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC8, 0x3C, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 5, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag560_funcs = { wstag560_setup };
FieldstgEventDef wstag560_events[2] = {
    { 1459, D_WSTAG560_800A5FEC, 0x013C0026, NULL, wstag560_event_1459_end }, { -1, NULL, 0, NULL, NULL },
};
