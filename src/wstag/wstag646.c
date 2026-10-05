#include "wstag.h"

/* WSTAG646: stage 0x2C3 (fieldstg_stages). */

extern WstagFuncs wstag646_funcs;
extern FieldstgBattleLists wstag646_battle_lists;
extern FieldstgVramPlace wstag646_vram_places[];
extern FieldstgPlacedActor *wstag646_actors[];
extern FieldstgSprite wstag646_sprites[];
extern FieldstgMapEvent wstag646_map_events[];
extern FieldstgEventDef wstag646_events[];

void wstag646_update(WstagObject *obj) {
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

WstagObject *wstag646_start(void *arg0) {
    WstagObject *obj = object_new(wstag646_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag646_funcs.setup();
    return obj;
}

void wstag646_event_735_end(void) {
    gamestate_flags.set_flag(0x4049, 1);
}

void wstag646_setup(void) {
    fieldstg_stage.background_file = 0x62A;
    fieldstg_stage.sprite_file = 0x062B0000;
    fieldstg_stage.sprites = wstag646_sprites;
    fieldstg_stage.map_events = wstag646_map_events;
    fieldstg_stage.mask_file = 0x629;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2C900, 0x23B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag646_vram_places;
    fieldstg_stage.music = 0x38;
    fieldstg_stage.sound = 0x60E00000;
    fieldstg_stage.actors = wstag646_actors;
    fieldstg_stage.battle_lists = &wstag646_battle_lists;
    fieldstg_stage.events = wstag646_events;
    fieldstg_attr.set_file(0, 0x062B0001);
    fieldstg_attr.set_file(7, 0x062B0002);
    fieldstg_attr.set_file(4, 0x062B0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag646_setup(void);

s16 D_WSTAG646_800A5FCC[162] = {
    FIELDSTG_EVENT_WALK(2, 584, 284, 5),
    FIELDSTG_EVENT_PLACE(45, 624, 264),
    FIELDSTG_EVENT_ANIM(45, 1, 1),
    FIELDSTG_EVENT_PLACE(49, 648, 277),
    FIELDSTG_EVENT_ANIM(49, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 600, 276, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 45),
    FIELDSTG_EVENT_ANIM(0x324, 805, 49),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 45),
    FIELDSTG_EVENT_ANIM(0x324, 806, 49),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 49, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 45),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 45, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 45, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 45, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 45, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(45, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(45, 592, 248, 3),
    FIELDSTG_EVENT_WAIT_WALK(45),
    FIELDSTG_EVENT_PLACE(45, 0, 0),
    FIELDSTG_EVENT_ANIM(45, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG646_800A6110 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A611C = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A6128 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A6134 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A6140 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A614C = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A6158 = { 72, 5, 0x60080000 };
FieldstgListedBattle D_WSTAG646_800A6164 = { 72, 5, 0x60080000 };
FieldstgBattleList D_WSTAG646_800A6170 = {
    3,
    { &D_WSTAG646_800A6110, &D_WSTAG646_800A611C, &D_WSTAG646_800A6128, &D_WSTAG646_800A6134, &D_WSTAG646_800A6140,
        &D_WSTAG646_800A614C, &D_WSTAG646_800A6158, &D_WSTAG646_800A6164 },
};
FieldstgListedBattle D_WSTAG646_800A6194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A61E8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG646_800A61F4 = {
    0,
    { &D_WSTAG646_800A6194, &D_WSTAG646_800A61A0, &D_WSTAG646_800A61AC, &D_WSTAG646_800A61B8, &D_WSTAG646_800A61C4,
        &D_WSTAG646_800A61D0, &D_WSTAG646_800A61DC, &D_WSTAG646_800A61E8 },
};
FieldstgListedBattle D_WSTAG646_800A6218 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A6224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A6230 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A623C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A6248 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A6254 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A6260 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A626C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG646_800A6278 = {
    0,
    { &D_WSTAG646_800A6218, &D_WSTAG646_800A6224, &D_WSTAG646_800A6230, &D_WSTAG646_800A623C, &D_WSTAG646_800A6248,
        &D_WSTAG646_800A6254, &D_WSTAG646_800A6260, &D_WSTAG646_800A626C },
};
FieldstgListedBattle D_WSTAG646_800A629C = { 239, 5, 0x600C0000 };
FieldstgListedBattle D_WSTAG646_800A62A8 = { 287, 5, 0x600C0000 };
FieldstgListedBattle D_WSTAG646_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A62C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A62CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A62D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A62E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG646_800A62F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG646_800A62FC = {
    0,
    { &D_WSTAG646_800A629C, &D_WSTAG646_800A62A8, &D_WSTAG646_800A62B4, &D_WSTAG646_800A62C0, &D_WSTAG646_800A62CC,
        &D_WSTAG646_800A62D8, &D_WSTAG646_800A62E4, &D_WSTAG646_800A62F0 },
};
FieldstgBattleLists wstag646_battle_lists = {
    104, 0, 0, { &D_WSTAG646_800A6170, &D_WSTAG646_800A61F4, &D_WSTAG646_800A6278 }, &D_WSTAG646_800A62FC,
};
FieldstgVramPlace wstag646_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 317, 0, 61, 336, 511 }, { 320, 256, 336, 349, 64, 93, 352, 511 },
    { 320, 256, 328, 317, 32, 61, 368, 511 }, { 320, 256, 336, 317, 64, 61, 336, 510 },
};
u16 D_WSTAG646_800A63DC[4] = { 0x868E, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A63E4[6] = { 1, 0, 0x868E, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A63F0[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A63F8[8] = { 1, 1, 0x868E, 0, 0x848A, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A6408[8] = { 1, 1, 0x868E, 0, 0x848A, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6418[10] = {
    0x868E, 1, 0x868D, 0, 0x848A, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG646_800A642C[6] = { 0x1A0B, 1, 0x9046, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6438[6] = { 0x11, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6444[6] = { 0x11, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6450[6] = { 0x10, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A645C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A6464[6] = { 0x10, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6470[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A6478[6] = { 0x10, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6484[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A6490[6] = { 0x10, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A649C[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A64A8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64B0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64B8[6] = { 0, 0, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64C4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64CC[6] = { 0, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64D8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A64E0[6] = { 0, 1, 0x7207, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A64EC[8] = { 0, 1, 0x7207, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A64FC[4] = { 0x7637, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6504[10] = {
    0, 1, 0x7207, 1, 0x8014, 1, 0x7209, 0,
    0xFFFF, 0,
};
u16 D_WSTAG646_800A6518[12] = {
    0, 1, 0x7207, 1, 0x8014, 1, 0x7209, 1,
    0xE2A, 0, 0xFFFF, 0,
};
u16 D_WSTAG646_800A6530[6] = { 0x7400, 1, 0xE2A, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A653C[14] = {
    0, 1, 0x7207, 1, 0x8014, 1, 0x7209, 1,
    0xE2A, 1, 0x720B, 0, 0xFFFF, 0,
};
u16 D_WSTAG646_800A6558[14] = {
    0, 1, 0x7207, 1, 0x8014, 1, 0x7209, 1,
    0xE2A, 1, 0x720B, 1, 0xFFFF, 0,
};
u16 D_WSTAG646_800A6574[4] = { 0x7837, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG646_800A657C[5] = {
    { D_WSTAG646_800A63DC, NULL, 762 }, { D_WSTAG646_800A63E4, D_WSTAG646_800A63F0, 763 },
    { D_WSTAG646_800A63F8, NULL, 764 }, { D_WSTAG646_800A6408, D_WSTAG646_800A6418, 765 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG646_800A65B8[2] = { { NULL, D_WSTAG646_800A642C, 473 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG646_800A65D0[7] = {
    { D_WSTAG646_800A6438, NULL, 488 }, { D_WSTAG646_800A6444, NULL, 19 },
    { D_WSTAG646_800A6450, D_WSTAG646_800A645C, 486 }, { D_WSTAG646_800A6464, D_WSTAG646_800A6470, 20 },
    { D_WSTAG646_800A6478, D_WSTAG646_800A6484, 487 }, { D_WSTAG646_800A6490, D_WSTAG646_800A649C, 21 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG646_800A6624[3] = {
    { D_WSTAG646_800A64A8, NULL, 488 }, { D_WSTAG646_800A64B0, NULL, 19 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG646_800A6648[9] = {
    { D_WSTAG646_800A64B8, D_WSTAG646_800A64C4, 478 }, { D_WSTAG646_800A64CC, D_WSTAG646_800A64D8, 19 },
    { D_WSTAG646_800A64E0, NULL, 22 }, { D_WSTAG646_800A64EC, D_WSTAG646_800A64FC, 481 },
    { D_WSTAG646_800A6504, NULL, 482 }, { D_WSTAG646_800A6518, D_WSTAG646_800A6530, 483 },
    { D_WSTAG646_800A653C, NULL, 484 }, { D_WSTAG646_800A6558, D_WSTAG646_800A6574, 485 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG646_800A66B4[2] = { { NULL, NULL, 477 }, { NULL, NULL, 0 } };
u16 D_WSTAG646_800A66CC[10] = {
    0x7049, 1, 0x7051, 1, 0x868D, 1, 0x868E, 0,
    0xFFFF, 0,
};
u16 D_WSTAG646_800A66E0[6] = { 0x701E, 1, 0x4049, 0, 0xFFFF, 0 };
u16 D_WSTAG646_800A66EC[8] = { 0x8192, 1, 0x11, 1, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A66FC[6] = { 0x8192, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6708[8] = { 0x11, 0, 0x8192, 1, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG646_800A6718[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG646_800A6720 = { D_WSTAG646_800A66CC, D_WSTAG646_800A657C, 28, 4, 720, 393, 1 };
FieldstgPlacedActor D_WSTAG646_800A6734 = { D_WSTAG646_800A66E0, D_WSTAG646_800A65B8, 45, 5, 624, 264, 1 };
FieldstgPlacedActor D_WSTAG646_800A6748 = { D_WSTAG646_800A66EC, D_WSTAG646_800A65D0, 49, 6, 648, 277, 1 };
FieldstgPlacedActor D_WSTAG646_800A675C = { D_WSTAG646_800A66FC, D_WSTAG646_800A6624, 49, 6, 648, 277, 1 };
FieldstgPlacedActor D_WSTAG646_800A6770 = { D_WSTAG646_800A6708, D_WSTAG646_800A6648, 49, 6, 648, 277, 1 };
FieldstgPlacedActor D_WSTAG646_800A6784 = { D_WSTAG646_800A6718, D_WSTAG646_800A66B4, 157, 7, 648, 277, 1 };
FieldstgPlacedActor *wstag646_actors[7] = {
    &D_WSTAG646_800A6720, &D_WSTAG646_800A6734, &D_WSTAG646_800A6748, &D_WSTAG646_800A675C, &D_WSTAG646_800A6770,
    &D_WSTAG646_800A6784, NULL,
};
FieldstgSprite wstag646_sprites[11] = {
    { 1, 0, 0x40, 6, 4, 0, 0, 0, 0, 0, 562, 254, 0, 0 }, { 1, 0, 0x40, 4, 0x53, 2, 0, 0xF, 8, 0, 604, 194, 256, 0 },
    { 1, 0, 0x40, 4, 0x54, 2, 0, 0xF, 8, 0, 604, 194, 252, 0 },
    { 1, 0, 0x40, 4, 0x55, 2, 0, 0xF, 8, 0, 604, 194, 248, 0 },
    { 1, 0, 0x40, 4, 0x56, 2, 0, 0xF, 8, 0, 604, 194, 244, 0 },
    { 1, 0, 0x40, 4, 0x57, 2, 0, 0xF, 8, 0, 604, 194, 240, 0 },
    { 1, 0, 0x40, 4, 0x58, 2, 0, 0xF, 8, 0, 604, 194, 236, 0 },
    { 1, 0, 0x40, 4, 0x59, 2, 0, 0xF, 8, 0, 604, 194, 232, 0 },
    { 1, 0, 0x40, 4, 0x5A, 2, 0, 0xF, 8, 0, 604, 194, 228, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 566, 208, 264, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag646_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C4, 0x27A, 0x256, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C1, 0x110, 0x108, 7, 0, 1, 6 },
    { 0x4049, 0, 0xFFFF, 0, 8, 0x2DF, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag646_funcs = { wstag646_setup };
FieldstgEventDef wstag646_events[2] = {
    { 735, D_WSTAG646_800A5FCC, 0x0143001D, NULL, wstag646_event_735_end }, { -1, NULL, 0, NULL, NULL },
};
