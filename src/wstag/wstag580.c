#include "wstag.h"

/* WSTAG580: stage 0x24D (fieldstg_stages). */

extern WstagFuncs wstag580_funcs;
extern FieldstgBattleLists wstag580_battle_lists;
extern FieldstgVramPlace wstag580_vram_places[];
extern FieldstgPlacedActor *wstag580_actors[];
extern FieldstgSprite wstag580_sprites[];
extern FieldstgMapEvent wstag580_map_events[];
extern FieldstgEventDef wstag580_events[];
void wstag580_update();

void wstag580_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        /* Evidence (class B, register priority only; DECISIONS "LOOP_BLOCK audit"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_data.progress == 0x11 && gamestate_flags.get_flag(0x403C, 0)) {
            data->event = fieldstg_event_start(0x1B8);
        } else if (gamestate_data.progress == 0x12 && gamestate_flags.get_flag(0x4038, 0)) {
            data->event = fieldstg_event_start(0x208);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag580_start(void *arg0) {
    WstagObject *obj = object_new(wstag580_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag580_funcs.setup();
    return obj;
}

void wstag580_event_440_end(void) {
    gamestate_flags.set_flag(0x403C, 1);
    gamestate_flags.set_flag(0x1C1F, 1);
}

void wstag580_event_450_end(void) {
    gamestate_flags.set_flag(0x4021, 1);
    gamestate_flags.set_flag(0x1C20, 1);
}

void wstag580_event_470_end(void) {
    gamestate_flags.set_flag(0x4034, 1);
    gamestate_flags.set_flag(0x1C22, 1);
}

void wstag580_event_480_end(void) {
    gamestate_flags.set_flag(0x4035, 1);
    gamestate_flags.set_flag(0x1C23, 1);
}

void wstag580_event_490_end(void) {
    gamestate_flags.set_flag(0x4036, 1);
    gamestate_flags.set_flag(0x1C24, 1);
}

void wstag580_event_500_end(void) {
    gamestate_flags.set_flag(0x4037, 1);
    gamestate_flags.set_flag(0x1C25, 1);
}

void wstag580_event_520_end(void) {
    gamestate_flags.set_flag(0x4038, 1);
}

void wstag580_setup(void) {
    fieldstg_stage.background_file = 0x77F;
    fieldstg_stage.sprite_file = 0x07800000;
    fieldstg_stage.sprites = wstag580_sprites;
    fieldstg_stage.map_events = wstag580_map_events;
    fieldstg_stage.mask_file = 0x77E;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF600, 0x3B200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag580_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag580_actors;
    fieldstg_stage.events = wstag580_events;
    fieldstg_stage.battle_lists = &wstag580_battle_lists;
    fieldstg_attr.set_file(0, 0x07800001);
    fieldstg_attr.set_file(7, 0x07800002);
    fieldstg_attr.set_file(4, 0x07800003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag580_setup(void);

s16 D_WSTAG580_800A6210[162] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 112, 1024),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(285, 455, 955),
    FIELDSTG_EVENT_ANIM(285, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 213, 974, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 257, 952),
    FIELDSTG_EVENT_WALK(285, 352, 903, 3),
    FIELDSTG_EVENT_WAIT_WALK(285),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_ANIM(285, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 285),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(285, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 105),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 285, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 285, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(285, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(285, 423, 867, 5),
    FIELDSTG_EVENT_WAIT_WALK(285),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_PLACE(285, 0, 0),
    FIELDSTG_EVENT_ANIM(285, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A6354[109] = {
    FIELDSTG_EVENT_WALK(2, 496, 688, 7),
    FIELDSTG_EVENT_PLACE(105, 825, 876),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 641, 762, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 736, 808),
    FIELDSTG_EVENT_WALK(105, 855, 892, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 868, 886, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 846, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 916, 862, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 852, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A6430[160] = {
    FIELDSTG_EVENT_WALK(2, 640, 328, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 720, 288),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(105, 830, 216),
    FIELDSTG_EVENT_ANIM(105, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 847, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(105, 769, 247, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 786, 255, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 752, 272, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_ANIM(105, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 105),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 105),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 105, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 786, 255, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 769, 247, 3),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 784, 240, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 824, 220, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 853, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A6570[102] = {
    FIELDSTG_EVENT_WALK(2, 1071, 692, 6),
    FIELDSTG_EVENT_PLACE(105, 1241, 708),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 1150, 692),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WALK(105, 1265, 720, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 1271, 716, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 850, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 1311, 696, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 856, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A663C[104] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 1210, 475, 5),
    FIELDSTG_EVENT_PLACE(105, 972, 449),
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 2),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 1120, 455),
    FIELDSTG_EVENT_WALK(105, 1039, 415, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 848, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WALK(105, 1088, 391, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 854, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A670C[110] = {
    FIELDSTG_EVENT_WALK(2, 639, 473, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WALK(2, 785, 545, 7),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_WALK(2, 924, 475, 5),
    FIELDSTG_EVENT_PLACE(105, 1150, 329),
    FIELDSTG_EVENT_ANIM(105, 1, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 1023, 264),
    FIELDSTG_EVENT_WALK(105, 1104, 271, 3),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 1088, 264, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 1120, 247, 5),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG580_800A67E8[139] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 280, 164),
    FIELDSTG_EVENT_PLACE(2, 207, 109),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_PLACE(105, 169, 539),
    FIELDSTG_EVENT_ANIM(105, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 845, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 261, 136, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 851, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 297, 154, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 207, 480),
    FIELDSTG_EVENT_WALK(105, 137, 555, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 129, 551, 3),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 849, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 86, 531, 3),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 855, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG580_800A6900 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A690C = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A6918 = { 79, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A6924 = { 79, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A6930 = { 78, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A693C = { 78, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A6948 = { 77, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG580_800A6954 = { 77, 12, 0x60080000 };
FieldstgBattleList D_WSTAG580_800A6960 = {
    2,
    { &D_WSTAG580_800A6900, &D_WSTAG580_800A690C, &D_WSTAG580_800A6918, &D_WSTAG580_800A6924, &D_WSTAG580_800A6930,
        &D_WSTAG580_800A693C, &D_WSTAG580_800A6948, &D_WSTAG580_800A6954 },
};
FieldstgListedBattle D_WSTAG580_800A6984 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6990 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A699C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A69A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A69B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A69C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A69CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A69D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG580_800A69E4 = {
    0,
    { &D_WSTAG580_800A6984, &D_WSTAG580_800A6990, &D_WSTAG580_800A699C, &D_WSTAG580_800A69A8, &D_WSTAG580_800A69B4,
        &D_WSTAG580_800A69C0, &D_WSTAG580_800A69CC, &D_WSTAG580_800A69D8 },
};
FieldstgListedBattle D_WSTAG580_800A6A08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A5C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG580_800A6A68 = {
    0,
    { &D_WSTAG580_800A6A08, &D_WSTAG580_800A6A14, &D_WSTAG580_800A6A20, &D_WSTAG580_800A6A2C, &D_WSTAG580_800A6A38,
        &D_WSTAG580_800A6A44, &D_WSTAG580_800A6A50, &D_WSTAG580_800A6A5C },
};
FieldstgListedBattle D_WSTAG580_800A6A8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6A98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6AA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6AB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6ABC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6AC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6AD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG580_800A6AE0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG580_800A6AEC = {
    0,
    { &D_WSTAG580_800A6A8C, &D_WSTAG580_800A6A98, &D_WSTAG580_800A6AA4, &D_WSTAG580_800A6AB0, &D_WSTAG580_800A6ABC,
        &D_WSTAG580_800A6AC8, &D_WSTAG580_800A6AD4, &D_WSTAG580_800A6AE0 },
};
FieldstgBattleLists wstag580_battle_lists = {
    41, 0, 0, { &D_WSTAG580_800A6960, &D_WSTAG580_800A69E4, &D_WSTAG580_800A6A68 }, &D_WSTAG580_800A6AEC,
};
FieldstgVramPlace wstag580_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 336, 475, 64, 219, 352, 511 }, { 320, 256, 344, 475, 96, 219, 368, 511 },
};
u16 D_WSTAG580_800A6BAC[6] = { 0x6000, 1, 0x4021, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6BB8[8] = { 0x6000, 1, 0x20D, 1, 0x4034, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6BC8[8] = { 0x6000, 1, 0x20E, 1, 0x4035, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6BD8[8] = { 0x6000, 1, 0x20F, 1, 0x4036, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6BE8[8] = { 0x6000, 1, 0x210, 1, 0x4037, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6BF8[6] = { 0x6012, 1, 0x4038, 0, 0xFFFF, 0 };
u16 D_WSTAG580_800A6C04[4] = { 0x6011, 1, 0xFFFF, 0 };
u16 D_WSTAG580_800A6C0C[6] = { 0x403C, 0, 0x6011, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG580_800A6C18 = { D_WSTAG580_800A6BAC, NULL, 105, 4, 825, 876, 7 };
FieldstgPlacedActor D_WSTAG580_800A6C2C = { D_WSTAG580_800A6BB8, NULL, 105, 4, 830, 216, 1 };
FieldstgPlacedActor D_WSTAG580_800A6C40 = { D_WSTAG580_800A6BC8, NULL, 105, 4, 1241, 708, 7 };
FieldstgPlacedActor D_WSTAG580_800A6C54 = { D_WSTAG580_800A6BD8, NULL, 105, 4, 972, 449, 5 };
FieldstgPlacedActor D_WSTAG580_800A6C68 = { D_WSTAG580_800A6BE8, NULL, 105, 4, 1150, 329, 3 };
FieldstgPlacedActor D_WSTAG580_800A6C7C = { D_WSTAG580_800A6BF8, NULL, 105, 4, 169, 539, 1 };
FieldstgPlacedActor D_WSTAG580_800A6C90 = { D_WSTAG580_800A6C04, NULL, 105, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG580_800A6CA4 = { D_WSTAG580_800A6C0C, NULL, 285, 5, 455, 955, 7 };
FieldstgPlacedActor *wstag580_actors[9] = {
    &D_WSTAG580_800A6C18, &D_WSTAG580_800A6C2C, &D_WSTAG580_800A6C40, &D_WSTAG580_800A6C54, &D_WSTAG580_800A6C68,
    &D_WSTAG580_800A6C7C, &D_WSTAG580_800A6C90, &D_WSTAG580_800A6CA4, NULL,
};
FieldstgSprite wstag580_sprites[53] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 249, 482, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 273, 494, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 720, 847, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 745, 378, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 769, 390, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 891, 346, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 913, 750, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 984, 754, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 1269, 276, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 889, 739, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 960, 766, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 1234, 293, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 1, 6, 0, 946, 161, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 787, 181, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 867, 829, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 1043, 353, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 1268, 657, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 6, 0, 236, 78, 0, 0 },
    { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 6, 0, 1092, 186, 0, 0 }, { 1, 0, 0x47, 2, 0xE, 0, 0, 0, 0, 0, 799, 790, 0, 0 },
    { 1, 0, 0x41, 2, 0xF, 0, 0, 0, 0, 0, 727, 809, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 9, 4, 0, 696, 835, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 241, 879, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 305, 559, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 1025, 775, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 217, 891, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 281, 571, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 1001, 787, 0, 0 },
    { 1, 0x68, 0x40, 6, 7, 0, 0, 0, 0, 0, 93, 496, 0, 0 }, { 1, 0x69, 0x40, 6, 8, 0, 0, 0, 0, 0, 1260, 665, 0, 0 },
    { 1, 0x67, 0x40, 6, 9, 0, 0, 0, 0, 0, 1036, 355, 0, 0 },
    { 1, 0x6B, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 1086, 196, 0, 0 },
    { 1, 0x6A, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 943, 171, 0, 0 },
    { 1, 0x66, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 783, 191, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x11, 0, 0, 0, 0, 0, 859, 837, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 230, 86, 0, 0 }, { 1, 0, 0x80, 6, 0x19, 0, 0, 0, 0, 0, 876, 192, 0, 0 },
    { 1, 0, 0x40, 4, 0x33, 2, 0, 9, 4, 0, 401, 463, 491, 0 },
    { 1, 0, 0x40, 4, 0x34, 2, 0, 9, 4, 0, 377, 475, 497, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 64, 512, 552, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1071, 379, 415, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 1049, 211, 247, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 975, 187, 224, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 815, 207, 239, 0 },
    { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 894, 347, 370, 0 }, { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 896, 855, 888, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 208, 103, 138, 0 }, { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 397, 464, 491, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 381, 476, 497, 0 },
    { 1, 0, 0x80, 4, 0x16, 0, 0, 0, 0, 0, 891, 116, 221, 0 },
    { 1, 0, 0x80, 4, 0x17, 0, 0, 0, 0, 0, 899, 112, 207, 0 },
    { 1, 0, 0x80, 4, 0x18, 0, 0, 0, 0, 0, 907, 108, 203, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag580_map_events[30] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24C, 0x228, 0x94, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24E, 0x108, 0x174, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24F, 0x108, 0x174, 5, 0x69, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x252, 0x2F8, 0x264, 3, 0x6B, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x250, 0x108, 0x174, 5, 0x67, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x252, 0x258, 0x24C, 5, 0x6A, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x251, 0x108, 0x174, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x252, 0x68, 0x20C, 3, 0x64, 0, 0 },
    { 0x7040, 1, 0xFFFF, 0, 1, 0x253, 0x278, 0xD4, 3, 0x68, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x160, 0x80, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x170, 0xE8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xC, 0xDF, 0x142, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xC, 0xD2, 0x208, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x2D0, 0x29A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x2C1, 0x322, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x33E, 0x2D2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x330, 0x35A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x42E, 0x34A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x420, 0x3D2, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x4B0, 0x25A, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x4BE, 0x2AE, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0x6011, 1, 0x4021, 0, 8, 0x1C2, 0, 0, 0, 0, 0, 0 }, { 0x20D, 1, 0x4034, 0, 8, 0x1D6, 0, 0, 0, 0, 0, 0 },
    { 0x20E, 1, 0x4035, 0, 8, 0x1E0, 0, 0, 0, 0, 0, 0 }, { 0x20F, 1, 0x4036, 0, 8, 0x1EA, 0, 0, 0, 0, 0, 0 },
    { 0x210, 1, 0x4037, 0, 8, 0x1F4, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag580_funcs = { wstag580_setup };
FieldstgEventDef wstag580_events[8] = {
    { 440, D_WSTAG580_800A6210, 0x013C0006, NULL, wstag580_event_440_end },
    { 450, D_WSTAG580_800A6354, 0x013C0016, NULL, wstag580_event_450_end },
    { 470, D_WSTAG580_800A6430, 0x013C0017, NULL, wstag580_event_470_end },
    { 480, D_WSTAG580_800A6570, 0x013C0018, NULL, wstag580_event_480_end },
    { 490, D_WSTAG580_800A663C, 0x013C0019, NULL, wstag580_event_490_end },
    { 500, D_WSTAG580_800A670C, 0x013C001A, NULL, wstag580_event_500_end },
    { 520, D_WSTAG580_800A67E8, 0x013C001B, NULL, wstag580_event_520_end }, { -1, NULL, 0, NULL, NULL },
};
