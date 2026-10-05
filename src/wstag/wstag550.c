#include "wstag.h"

/* WSTAG550: stage 0x247 (fieldstg_stages). */

extern WstagFuncs wstag550_funcs;
extern CVECTOR wstag550_color;
extern FieldstgBattleLists wstag550_battle_lists;
extern FieldstgVramPlace wstag550_vram_places[];
extern FieldstgPlacedActor *wstag550_actors[];
extern FieldstgSprite wstag550_sprites[];
extern FieldstgMapEvent wstag550_map_events[];
extern FieldstgEventDef wstag550_events[];
void wstag550_update();

void wstag550_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4027, 1) && gamestate_flags.get_flag(0x4028, 0)) {
            data->event = fieldstg_event_start(0x4F6);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag550_start(void *arg0) {
    WstagObject *obj = object_new(wstag550_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag550_funcs.setup();
    return obj;
}

void wstag550_event_1269_end(void) {
    gamestate_flags.set_flag(0x4027, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag550_event_1270_end(void) {
    gamestate_flags.set_flag(0x4028, 1);
    gamestate_flags.set_flag(0x8027, 1);
}

void wstag550_setup(void) {
    fieldstg_stage.background_file = 0x46D;
    fieldstg_stage.sprite_file = 0x046E0000;
    fieldstg_stage.sprites = wstag550_sprites;
    fieldstg_stage.map_events = wstag550_map_events;
    fieldstg_stage.mask_file = 0x46C;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11600, 0x10600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag550_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag550_actors;
    fieldstg_stage.color = wstag550_color;
    fieldstg_stage.events = wstag550_events;
    fieldstg_stage.battle_lists = &wstag550_battle_lists;
    fieldstg_attr.set_file(0, 0x046E0001);
    fieldstg_attr.set_file(7, 0x046E0002);
    fieldstg_attr.set_file(4, 0x046E0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag550/nonmatchings/wstag550", wstag550_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag550_setup(void);

s16 D_WSTAG550_800A60E0[58] = {
    FIELDSTG_EVENT_WALK(2, 400, 224, 3),
    FIELDSTG_EVENT_PLACE(21, 369, 209),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC05, 0, 0, 0),
    FIELDSTG_EVENT_END,
    0x1062, /* padding, not read */
};
s16 D_WSTAG550_800A6154[56] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 1115, 837, 7),
    FIELDSTG_EVENT_PLACE(124, 1153, 858),
    FIELDSTG_EVENT_ANIM(124, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 124, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x8E02, /* padding, not read */
};
s16 D_WSTAG550_800A61C4[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 1115, 837),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_PLACE(124, 1153, 858),
    FIELDSTG_EVENT_ANIM(124, 1, 3),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 124, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG550_800A6248 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6254 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6260 = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A626C = { 69, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6278 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6284 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6290 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A629C = { 68, 3, 0x60080000 };
FieldstgBattleList D_WSTAG550_800A62A8 = {
    3,
    { &D_WSTAG550_800A6248, &D_WSTAG550_800A6254, &D_WSTAG550_800A6260, &D_WSTAG550_800A626C, &D_WSTAG550_800A6278,
        &D_WSTAG550_800A6284, &D_WSTAG550_800A6290, &D_WSTAG550_800A629C },
};
FieldstgListedBattle D_WSTAG550_800A62CC = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A62D8 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A62E4 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A62F0 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A62FC = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6308 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6314 = { 0, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6320 = { 0, 3, 0x60080000 };
FieldstgBattleList D_WSTAG550_800A632C = {
    0,
    { &D_WSTAG550_800A62CC, &D_WSTAG550_800A62D8, &D_WSTAG550_800A62E4, &D_WSTAG550_800A62F0, &D_WSTAG550_800A62FC,
        &D_WSTAG550_800A6308, &D_WSTAG550_800A6314, &D_WSTAG550_800A6320 },
};
FieldstgListedBattle D_WSTAG550_800A6350 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A635C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6368 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6374 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6380 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A63A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG550_800A63B0 = {
    0,
    { &D_WSTAG550_800A6350, &D_WSTAG550_800A635C, &D_WSTAG550_800A6368, &D_WSTAG550_800A6374, &D_WSTAG550_800A6380,
        &D_WSTAG550_800A638C, &D_WSTAG550_800A6398, &D_WSTAG550_800A63A4 },
};
FieldstgListedBattle D_WSTAG550_800A63D4 = { 266, 3, 0x60880000 };
FieldstgListedBattle D_WSTAG550_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A63F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6404 = { 330, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A6410 = { 68, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG550_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG550_800A6428 = { 61, 2, 0x60080000 };
FieldstgBattleList D_WSTAG550_800A6434 = {
    0,
    { &D_WSTAG550_800A63D4, &D_WSTAG550_800A63E0, &D_WSTAG550_800A63EC, &D_WSTAG550_800A63F8, &D_WSTAG550_800A6404,
        &D_WSTAG550_800A6410, &D_WSTAG550_800A641C, &D_WSTAG550_800A6428 },
};
FieldstgBattleLists wstag550_battle_lists = {
    31, 0, 0, { &D_WSTAG550_800A62A8, &D_WSTAG550_800A632C, &D_WSTAG550_800A63B0 }, &D_WSTAG550_800A6434,
};
FieldstgVramPlace wstag550_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 256, 192, 0, 320, 510 }, { 384, 256, 432, 256, 448, 0, 336, 510 },
    { 448, 256, 448, 256, 512, 0, 352, 510 },
};
u16 D_WSTAG550_800A6504[4] = { 0x9011, 1, 0xFFFF, 0 };
u16 D_WSTAG550_800A650C[4] = { 0x1C14, 0, 0xFFFF, 0 };
u16 D_WSTAG550_800A6514[6] = { 0x9027, 1, 0x1C14, 1, 0xFFFF, 0 };
u16 D_WSTAG550_800A6520[4] = { 0x1C14, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG550_800A6528[2] = { { NULL, D_WSTAG550_800A6504, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG550_800A6540[2] = { { NULL, NULL, 822 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG550_800A6558[3] = {
    { D_WSTAG550_800A650C, D_WSTAG550_800A6514, 705 }, { D_WSTAG550_800A6520, NULL, 706 }, { NULL, NULL, 0 },
};
u16 D_WSTAG550_800A657C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG550_800A6584[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG550_800A658C[6] = { 0x602, 1, 0x8027, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG550_800A6598 = { D_WSTAG550_800A657C, D_WSTAG550_800A6528, 21, 4, 369, 209, 7 };
FieldstgPlacedActor D_WSTAG550_800A65AC = { D_WSTAG550_800A6584, D_WSTAG550_800A6540, 103, 5, 352, 720, 1 };
FieldstgPlacedActor D_WSTAG550_800A65C0 = { D_WSTAG550_800A658C, D_WSTAG550_800A6558, 124, 6, 1153, 858, 7 };
FieldstgPlacedActor *wstag550_actors[4] = {
    &D_WSTAG550_800A6598, &D_WSTAG550_800A65AC, &D_WSTAG550_800A65C0, NULL,
};
FieldstgSprite wstag550_sprites[125] = {
    { 1, 0, 0x40, 2, 0xD, 1, 0xD, 0x12, 4, 0, 43, 511, 0, 0 },
    { 1, 0, 0x40, 2, 0xD, 1, 0xD, 0x12, 4, 0, 475, 159, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 132, 946, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 229, 982, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 271, 926, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 483, 887, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 501, 1006, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 624, 930, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 697, 934, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 716, 988, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 809, 964, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 828, 1055, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 956, 1005, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1007, 760, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1011, 659, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1022, 1016, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1053, 571, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1101, 200, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1163, 904, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1176, 1033, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 164, 932, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 266, 1023, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 326, 956, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 543, 983, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 701, 999, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 726, 926, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 869, 1029, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 936, 771, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 979, 860, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 986, 606, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 989, 153, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 990, 1001, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1032, 926, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1078, 204, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1102, 935, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1135, 1056, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1146, 277, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1199, 921, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1236, 1001, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1246, 618, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1252, 366, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 233, 916, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 283, 1011, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 347, 954, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 575, 977, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 657, 913, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 841, 947, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 984, 906, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1014, 853, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1150, 945, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 249, 927, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 303, 954, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 414, 994, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 475, 1008, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 535, 894, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 775, 951, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 797, 1040, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 929, 998, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1009, 600, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1045, 910, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1076, 1052, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1097, 1125, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1120, 273, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1243, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 203, 911, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 205, 988, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 360, 968, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 452, 884, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 567, 911, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 672, 934, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 769, 1028, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 863, 948, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 958, 167, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 961, 765, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 992, 654, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1034, 800, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1045, 1040, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1063, 937, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1103, 1051, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1114, 1125, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1212, 995, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1229, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 375, 979, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 435, 1003, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 511, 882, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 566, 983, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 600, 927, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 756, 919, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 999, 1016, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1090, 943, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1092, 1114, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1254, 668, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 215, 919, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 227, 990, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 255, 932, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 270, 933, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 289, 1020, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 385, 986, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 477, 894, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 487, 1014, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 502, 886, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 504, 1012, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 609, 935, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 624, 938, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 684, 941, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 703, 940, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 739, 986, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 766, 927, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 842, 945, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 849, 1036, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 860, 1038, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 979, 161, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 994, 740, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1024, 759, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1033, 596, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1038, 575, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1140, 941, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1196, 1032, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1197, 1021, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1209, 1023, 0, 0 },
    { 1, 0, 0x40, 6, 0, 0, 0, 0, 0, 0, 678, 464, 478, 0 }, { 1, 0, 0x4A, 6, 7, 1, 7, 0xC, 4, 0, 1178, 782, 0, 0 },
    { 1, 0, 0x4B, 4, 1, 1, 1, 6, 4, 0, 160, 219, 243, 0 }, { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 697, 464, 478, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag550_map_events[17] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x248, 0x610, 0x540, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 3, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 6, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0xA, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x479, 0x2B0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x469, 0x318, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 9, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFE0, 0x2C, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x20, 0x30, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xC, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFEC, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x30, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFE0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 9, 1 },
    { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag550_funcs = { wstag550_setup };
FieldstgEventDef wstag550_events[5] = {
    { 1225, D_WSTAG550_800A60E0, 0x013C000A, NULL, NULL },
    { 1269, D_WSTAG550_800A6154, 0x013C000E, NULL, wstag550_event_1269_end },
    { 1270, D_WSTAG550_800A61C4, 0x013C000F, NULL, wstag550_event_1270_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
