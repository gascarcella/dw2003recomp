#include "wstag.h"

/* WSTAG615: stage 0x254 (fieldstg_stages). */

extern WstagFuncs wstag615_funcs;
const CVECTOR wstag615_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag615_battle_lists;
extern FieldstgVramPlace wstag615_vram_places[];
extern FieldstgPlacedActor *wstag615_actors[];
extern FieldstgSprite wstag615_sprites[];
extern FieldstgMapEvent wstag615_map_events[];
extern FieldstgMapEvent wstag615_map_events2[];
extern FieldstgEventDef wstag615_events[];
void wstag615_update();

void wstag615_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0x12 && gamestate_flags.get_flag(0x4039, 1)) {
            data->event = fieldstg_event_start(0x213);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag615_start(void *arg0) {
    WstagObject *obj = object_new(wstag615_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag615_funcs.setup();
    return obj;
}

void wstag615_event_530_end(void) {
    gamestate_flags.set_flag(0x4039, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag615_event_531_end(void) {
    gamestate_data.progress = 0x13;
}

void wstag615_setup(void) {
    fieldstg_stage.background_file = 0x55C;
    fieldstg_stage.sprite_file = 0x055D0000;
    fieldstg_stage.sprites = wstag615_sprites;
    fieldstg_stage.mask_file = 0x55B;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xBF00, 0x2AF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag615_vram_places;
    fieldstg_stage.music = 0x3A;
    fieldstg_stage.sound = 0x60E80000;
    fieldstg_stage.actors = wstag615_actors;
    fieldstg_stage.color = wstag615_color;
    fieldstg_stage.events = wstag615_events;
    fieldstg_stage.battle_lists = &wstag615_battle_lists;
    fieldstg_attr.set_file(0, 0x055D0001);
    fieldstg_attr.set_file(7, 0x055D0002);
    fieldstg_attr.set_file(4, 0x055D0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0x15) {
        fieldstg_stage.map_events = wstag615_map_events;
    } else {
        fieldstg_stage.map_events = wstag615_map_events2;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag615_setup(void);

s16 D_WSTAG615_800A608C[223] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 561, 370, 7),
    FIELDSTG_EVENT_PLACE(101, 640, 568),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_PLACE(151, 668, 554),
    FIELDSTG_EVENT_ANIM(151, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 6),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 657, 417, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 657, 497),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 151, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_DIALOG(0, 5, 101, 0),
    FIELDSTG_EVENT_ANIM(101, 77, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 101),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 101),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 101, 0),
    FIELDSTG_EVENT_ANIM(101, 76, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(101, 77, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_WALK(2, 832, 505, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 704, 569, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(151, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 151, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG615_800A624C[308] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_PLACE(1, 687, 576),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_PLACE(101, 655, 560),
    FIELDSTG_EVENT_ANIM(101, 3, 7),
    FIELDSTG_EVENT_PLACE(105, 696, 434),
    FIELDSTG_EVENT_ANIM(105, 1, 6),
    FIELDSTG_EVENT_PLACE(151, 609, 545),
    FIELDSTG_EVENT_ANIM(151, 16, 1),
    FIELDSTG_EVENT_PLACE(287, 609, 546),
    FIELDSTG_EVENT_ANIM(287, 1, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x32D, 894, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 101, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(101, 75, 7),
    FIELDSTG_EVENT_WAIT_ANIM(101),
    FIELDSTG_EVENT_ANIM(101, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 101, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 105, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x324, 805, 101),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_ANIM(0x324, 806, 101),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WALK(105, 832, 504, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_WALK(105, 800, 520, 1),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_DIALOG(0, 7, 101, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 105, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(105, 71, 1),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_PLACE(104, 800, 519),
    FIELDSTG_EVENT_ANIM(104, 71, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 872, 1),
    FIELDSTG_EVENT_WAIT_ANIM(104),
    FIELDSTG_EVENT_PLACE(105, 0, 0),
    FIELDSTG_EVENT_ANIM(105, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 9, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 101, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 104, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(104, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(104, 832, 504, 5),
    FIELDSTG_EVENT_WAIT_WALK(104),
    FIELDSTG_EVENT_WALK(1, 720, 560, 5),
    FIELDSTG_EVENT_WALK(101, 688, 576, 7),
    FIELDSTG_EVENT_WALK(104, 800, 487, 3),
    FIELDSTG_EVENT_WAIT_WALK(101),
    FIELDSTG_EVENT_WALK(1, 831, 503, 5),
    FIELDSTG_EVENT_WALK(101, 831, 503, 5),
    FIELDSTG_EVENT_WALK(104, 623, 400, 3),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0x255, 448, 393, 1),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG615_800A64B4 = { 81, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64C0 = { 81, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64CC = { 81, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64D8 = { 81, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64E4 = { 82, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64F0 = { 82, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A64FC = { 82, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG615_800A6508 = { 82, 12, 0x60080000 };
FieldstgBattleList D_WSTAG615_800A6514 = {
    3,
    { &D_WSTAG615_800A64B4, &D_WSTAG615_800A64C0, &D_WSTAG615_800A64CC, &D_WSTAG615_800A64D8, &D_WSTAG615_800A64E4,
        &D_WSTAG615_800A64F0, &D_WSTAG615_800A64FC, &D_WSTAG615_800A6508 },
};
FieldstgListedBattle D_WSTAG615_800A6538 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6544 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6550 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A655C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6568 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6574 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6580 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A658C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG615_800A6598 = {
    0,
    { &D_WSTAG615_800A6538, &D_WSTAG615_800A6544, &D_WSTAG615_800A6550, &D_WSTAG615_800A655C, &D_WSTAG615_800A6568,
        &D_WSTAG615_800A6574, &D_WSTAG615_800A6580, &D_WSTAG615_800A658C },
};
FieldstgListedBattle D_WSTAG615_800A65BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A65C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A65D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A65E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A65EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A65F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6604 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6610 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG615_800A661C = {
    0,
    { &D_WSTAG615_800A65BC, &D_WSTAG615_800A65C8, &D_WSTAG615_800A65D4, &D_WSTAG615_800A65E0, &D_WSTAG615_800A65EC,
        &D_WSTAG615_800A65F8, &D_WSTAG615_800A6604, &D_WSTAG615_800A6610 },
};
FieldstgListedBattle D_WSTAG615_800A6640 = { 8, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG615_800A664C = { 312, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG615_800A6658 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6664 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6670 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A667C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6688 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG615_800A6694 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG615_800A66A0 = {
    0,
    { &D_WSTAG615_800A6640, &D_WSTAG615_800A664C, &D_WSTAG615_800A6658, &D_WSTAG615_800A6664, &D_WSTAG615_800A6670,
        &D_WSTAG615_800A667C, &D_WSTAG615_800A6688, &D_WSTAG615_800A6694 },
};
FieldstgBattleLists wstag615_battle_lists = {
    48, 0, 0, { &D_WSTAG615_800A6514, &D_WSTAG615_800A6598, &D_WSTAG615_800A661C }, &D_WSTAG615_800A66A0,
};
FieldstgVramPlace wstag615_vram_places[12] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 424, 360, 416, 104, 352, 510 }, { 384, 256, 438, 304, 472, 48, 368, 510 },
    { 320, 256, 377, 256, 228, 0, 320, 509 }, { 384, 256, 384, 366, 256, 110, 336, 509 },
    { 320, 256, 368, 395, 192, 139, 352, 509 }, { 320, 256, 376, 360, 224, 104, 368, 509 },
};
FieldstgTalk D_WSTAG615_800A67A0[2] = { { NULL, NULL, 342 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A67B8[2] = { { NULL, NULL, 346 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A67D0[2] = { { NULL, NULL, 343 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A67E8[2] = { { NULL, NULL, 344 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A6800[2] = { { NULL, NULL, 345 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A6818[2] = { { NULL, NULL, 347 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG615_800A6830[2] = { { NULL, NULL, 341 }, { NULL, NULL, 0 } };
u16 D_WSTAG615_800A6848[6] = { 0x6012, 1, 0x4039, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A6854[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A685C[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A6864[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A686C[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A6874[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A687C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A6884[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A688C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A6894[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A689C[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG615_800A68A4[4] = { 0x6012, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG615_800A68AC = { D_WSTAG615_800A6848, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG615_800A68C0 = { D_WSTAG615_800A6854, NULL, 101, 5, 640, 570, 5 };
FieldstgPlacedActor D_WSTAG615_800A68D4 = { D_WSTAG615_800A685C, NULL, 104, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG615_800A68E8 = { D_WSTAG615_800A6864, NULL, 105, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG615_800A68FC = { D_WSTAG615_800A686C, D_WSTAG615_800A67A0, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A6910 = { D_WSTAG615_800A6874, D_WSTAG615_800A67B8, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A6924 = { D_WSTAG615_800A687C, D_WSTAG615_800A67D0, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A6938 = { D_WSTAG615_800A6884, D_WSTAG615_800A67E8, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A694C = { D_WSTAG615_800A688C, D_WSTAG615_800A6800, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A6960 = { D_WSTAG615_800A6894, D_WSTAG615_800A6818, 151, 8, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG615_800A6974 = { D_WSTAG615_800A689C, D_WSTAG615_800A6830, 151, 8, 668, 554, 1 };
FieldstgPlacedActor D_WSTAG615_800A6988 = { D_WSTAG615_800A68A4, NULL, 287, 9, 0, 0, 1 };
FieldstgPlacedActor *wstag615_actors[13] = {
    &D_WSTAG615_800A68AC, &D_WSTAG615_800A68C0, &D_WSTAG615_800A68D4, &D_WSTAG615_800A68E8, &D_WSTAG615_800A68FC,
    &D_WSTAG615_800A6910, &D_WSTAG615_800A6924, &D_WSTAG615_800A6938, &D_WSTAG615_800A694C, &D_WSTAG615_800A6960,
    &D_WSTAG615_800A6974, &D_WSTAG615_800A6988, NULL,
};
FieldstgSprite wstag615_sprites[136] = {
    { 1, 0, 0x40, 2, 0x56, 2, 0, 1, 4, 0, 660, 140, 0, 0 }, { 1, 0, 0x48, 6, 0, 0, 0, 0, 0, 0, 143, 793, 0, 0 },
    { 1, 0, 0x44, 6, 1, 0, 0, 0, 0, 0, 298, 816, 0, 0 }, { 1, 0, 0x40, 6, 2, 0, 0, 0, 0, 0, 512, 816, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 1, 4, 0, 395, 684, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 1, 4, 0, 552, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 1, 4, 0, 96, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 196, 576, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 236, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 436, 252, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 532, 204, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 724, 108, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 789, 575, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 476, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 652, 624, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 684, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 836, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 908, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 988, 664, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 200, 474, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 215, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 422, 463, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 477, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 493, 353, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 755, 721, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0x14, 0, 764, 711, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 39, 789, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 120, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 268, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 282, 682, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 341, 409, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 436, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 726, 215, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 829, 116, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0x14, 0, 859, 101, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0x14, 0, 206, 865, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0x14, 0, 222, 867, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0x14, 0, 284, 673, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0x14, 0, 17, 551, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0x14, 0, 497, 703, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0x14, 0, 577, 412, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0x14, 0, 597, 754, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0x14, 0, 664, 468, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0x14, 0, 217, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0x14, 0, 509, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0x14, 0, 682, 371, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0x14, 0, 913, 92, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 424, 668, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 434, 677, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 482, 843, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 490, 851, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 683, 466, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 706, 379, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0x14, 0, 880, 477, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 107, 521, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 198, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 206, 427, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 262, 450, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 360, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 432, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 439, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 498, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 683, 390, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 686, 524, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 692, 394, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 698, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 714, 219, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 734, 836, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 739, 826, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 792, 179, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 803, 181, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 819, 714, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 833, 707, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 865, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 875, 481, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 878, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0x14, 0, 882, 98, 0, 0 },
    { 1, 0x64, 0x40, 6, 3, 0, 0, 0, 0, 0, 687, 136, 0, 0 }, { 1, 0, 0x40, 6, 5, 1, 5, 0xA, 4, 0, 304, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0xB, 1, 0xB, 0x10, 4, 0, 157, 799, 0, 0 },
    { 1, 0, 0x40, 6, 0xB, 1, 0xB, 0x10, 4, 0, 430, 744, 0, 0 },
    { 1, 0, 0x40, 6, 0xB, 1, 0xB, 0x10, 4, 0, 527, 823, 0, 0 },
    { 1, 0, 0x40, 6, 0x11, 1, 0x11, 0x16, 4, 0, 563, 592, 0, 0 },
    { 1, 0, 0x40, 6, 0x11, 1, 0x11, 0x16, 4, 0, 602, 612, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 1, 0x17, 0x1C, 4, 0, 70, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 1, 0x17, 0x1C, 4, 0, 326, 327, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 1, 0x17, 0x1C, 4, 0, 366, 307, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 1, 0x17, 0x1C, 4, 0, 582, 199, 0, 0 },
    { 1, 0, 0x40, 6, 0x17, 1, 0x17, 0x1C, 4, 0, 622, 179, 0, 0 },
    { 1, 0, 0x78, 6, 0x1D, 0, 0, 0, 0, 0, 810, 120, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2C, 1, 0x2C, 0x2E, 0x14, 0, 691, 754, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 681, 794, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 705, 816, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 853, 676, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 949, 526, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 6, 803, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 22, 594, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 229, 490, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 449, 422, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 471, 311, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 587, 259, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 648, 810, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 687, 352, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 724, 471, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 808, 662, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 844, 541, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 893, 386, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0x14, 0, 925, 508, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 46, 550, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 106, 587, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 177, 523, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 258, 699, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 319, 464, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 383, 643, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 420, 742, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 455, 384, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 549, 414, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 620, 365, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 626, 460, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 671, 285, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 769, 744, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 830, 460, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 0x14, 0, 859, 156, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x60, 1, 0x60, 0x62, 0x14, 0, 80, 601, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x60, 1, 0x60, 0x62, 0x14, 0, 80, 702, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 0x14, 0, 295, 495, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 0x14, 0, 298, 606, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 92, 757, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 190, 431, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 329, 641, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 483, 287, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 806, 127, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 872, 90, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 922, 390, 0, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 646, 144, 191, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag615_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x253, 0x90, 0x240, 7, 0, 0, 0 },
    { 0x7041, 1, 0xFFFF, 0, 1, 0x255, 0x268, 0x1AC, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xB3, 0x238, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0xC3, 0x2A0, 0, 0, 0, 0 }, { 0x6012, 1, 0x4039, 0, 8, 0x212, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag615_map_events2[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x253, 0x90, 0x240, 7, 0, 0, 0 },
    { 0x7041, 1, 0xFFFF, 0, 1, 0x256, 0x268, 0x1AC, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xB3, 0x238, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0xC3, 0x2A0, 0, 0, 0, 0 }, { 0x6012, 1, 0x4039, 0, 8, 0x212, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag615_funcs = { wstag615_setup };
FieldstgEventDef wstag615_events[3] = {
    { 530, D_WSTAG615_800A608C, 0x01430015, NULL, wstag615_event_530_end },
    { 531, D_WSTAG615_800A624C, 0x01430005, NULL, wstag615_event_531_end }, { -1, NULL, 0, NULL, NULL },
};
