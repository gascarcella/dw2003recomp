#include "wstag.h"

/* WSTAG285: stage 0x214 (fieldstg_stages). */

extern WstagFuncs wstag285_funcs;
extern FieldstgBattleLists wstag285_battle_lists;
extern FieldstgVramPlace wstag285_vram_places[];
extern FieldstgPlacedActor *wstag285_actors[];
extern FieldstgSprite wstag285_sprites[];
extern FieldstgMapEvent wstag285_map_events[];
extern FieldstgEventDef wstag285_events[];
void wstag285_update();

void wstag285_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        /* Evidence (class B, register priority only; DECISIONS "LOOP_BLOCK audit"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_flags.get_flag(0x4005, 0)) {
            data->event = fieldstg_event_start(0x28);
        } else if (gamestate_data.progress == 0xB) {
            data->event = fieldstg_event_start(0x110);
        } else if (gamestate_data.progress == 0xD && gamestate_flags.get_flag(0x1C0C, 0)) {
            data->event = fieldstg_event_start(0x154);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag285_start(void *arg0) {
    WstagObject *obj = object_new(wstag285_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag285_funcs.setup();
    return obj;
}

void wstag285_event_272_end(void) {
    gamestate_flags.set_flag(0x4005, 0);
}

void wstag285_event_340_end(void) {
    gamestate_flags.set_flag(0x1C0C, 1);
}

void wstag285_setup(void) {
    fieldstg_stage.background_file = 0x2B4;
    fieldstg_stage.sprite_file = 0x02B50000;
    fieldstg_stage.sprites = wstag285_sprites;
    fieldstg_stage.map_events = wstag285_map_events;
    fieldstg_stage.mask_file = 0x32D;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x23900, 0x29F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag285_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag285_actors;
    fieldstg_stage.events = wstag285_events;
    fieldstg_stage.battle_lists = &wstag285_battle_lists;
    fieldstg_attr.set_file(0, 0x02B50002);
    fieldstg_attr.set_file(7, 0x02B50001);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x14 && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag285_setup(void);

s16 D_WSTAG285_800A60B0[98] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 483, 710),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(37, 560, 673),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WALK(2, 520, 692, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 37),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 37),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(37, 544, 679, 1),
    FIELDSTG_EVENT_WAIT_WALK(37),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 37, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 474, 716, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_MOVE(1, 474, 716),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x203, 330, 196, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG285_800A6174[144] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 483, 710),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(37, 560, 673),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 500, 702, 5),
    FIELDSTG_EVENT_PLACE(11, 483, 710),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 520, 692, 5),
    FIELDSTG_EVENT_WALK(11, 500, 702, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 37),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 37),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(37, 544, 679, 1),
    FIELDSTG_EVENT_WAIT_WALK(37),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 37, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 507, 699, 1),
    FIELDSTG_EVENT_WALK(11, 483, 710, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 474, 716, 1),
    FIELDSTG_EVENT_PLACE(11, 0, 0),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_MOVE(1, 474, 716),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x204, 298, 299, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG285_800A6294[107] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 106),
    FIELDSTG_EVENT_PLACE(37, 560, 673),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_PLACE(106, 483, 710),
    FIELDSTG_EVENT_ANIM(106, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(106, 520, 692, 5),
    FIELDSTG_EVENT_WAIT_WALK(106),
    FIELDSTG_EVENT_ANIM(106, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 37),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 37),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(37, 544, 679, 1),
    FIELDSTG_EVENT_WAIT_WALK(37),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 37, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 106, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 37),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 37),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 37, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(37, 560, 673, 5),
    FIELDSTG_EVENT_WAIT_WALK(37),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG285_800A636C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6390 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A639C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A63A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A63B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A63C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG285_800A63CC = {
    0,
    { &D_WSTAG285_800A636C, &D_WSTAG285_800A6378, &D_WSTAG285_800A6384, &D_WSTAG285_800A6390, &D_WSTAG285_800A639C,
        &D_WSTAG285_800A63A8, &D_WSTAG285_800A63B4, &D_WSTAG285_800A63C0 },
};
FieldstgListedBattle D_WSTAG285_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A63FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A642C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6438 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6444 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG285_800A6450 = {
    0,
    { &D_WSTAG285_800A63F0, &D_WSTAG285_800A63FC, &D_WSTAG285_800A6408, &D_WSTAG285_800A6414, &D_WSTAG285_800A6420,
        &D_WSTAG285_800A642C, &D_WSTAG285_800A6438, &D_WSTAG285_800A6444 },
};
FieldstgListedBattle D_WSTAG285_800A6474 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6480 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A648C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6498 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A64A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A64B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A64BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A64C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG285_800A64D4 = {
    0,
    { &D_WSTAG285_800A6474, &D_WSTAG285_800A6480, &D_WSTAG285_800A648C, &D_WSTAG285_800A6498, &D_WSTAG285_800A64A4,
        &D_WSTAG285_800A64B0, &D_WSTAG285_800A64BC, &D_WSTAG285_800A64C8 },
};
FieldstgListedBattle D_WSTAG285_800A64F8 = { 189, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG285_800A6504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6510 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A651C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6534 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A6540 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG285_800A654C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG285_800A6558 = {
    0,
    { &D_WSTAG285_800A64F8, &D_WSTAG285_800A6504, &D_WSTAG285_800A6510, &D_WSTAG285_800A651C, &D_WSTAG285_800A6528,
        &D_WSTAG285_800A6534, &D_WSTAG285_800A6540, &D_WSTAG285_800A654C },
};
FieldstgBattleLists wstag285_battle_lists = {
    134, 0, 0, { &D_WSTAG285_800A63CC, &D_WSTAG285_800A6450, &D_WSTAG285_800A64D4 }, &D_WSTAG285_800A6558,
};
FieldstgVramPlace wstag285_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 434, 432, 456, 176, 352, 511 }, { 320, 256, 376, 256, 224, 0, 368, 511 },
    { 320, 256, 368, 256, 192, 0, 352, 510 }, { 320, 256, 368, 304, 192, 48, 368, 510 },
    { 320, 256, 370, 433, 200, 177, 336, 509 }, { 384, 256, 384, 443, 256, 187, 352, 509 },
    { 384, 256, 438, 352, 472, 96, 368, 509 }, { 384, 256, 438, 392, 472, 136, 336, 508 },
    { 384, 256, 418, 418, 392, 162, 352, 508 }, { 384, 256, 426, 418, 424, 162, 368, 508 },
    { 384, 256, 434, 256, 456, 0, 336, 507 }, { 384, 256, 434, 304, 456, 48, 352, 507 },
};
u16 D_WSTAG285_800A66B8[8] = { 0x215, 1, 0x84CF, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A66C8[6] = { 0xC05, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A66D4[6] = { 0x7400, 1, 0xC06, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A66E0[6] = { 0x7400, 1, 0xC07, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A66EC[6] = { 0x7400, 1, 0xC07, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG285_800A66F8[2] = { { NULL, D_WSTAG285_800A66B8, 1106 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6710[2] = { { NULL, NULL, 166 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6728[2] = { { NULL, D_WSTAG285_800A66C8, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6740[2] = { { NULL, D_WSTAG285_800A66D4, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6758[2] = { { NULL, D_WSTAG285_800A66E0, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6770[2] = { { NULL, D_WSTAG285_800A66EC, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A6788[2] = { { NULL, NULL, 167 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG285_800A67A0[2] = { { NULL, NULL, 168 }, { NULL, NULL, 0 } };
u16 D_WSTAG285_800A67B8[4] = { 0x600B, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67C0[4] = { 0x215, 0, 0xFFFF, 0 };
u16 D_WSTAG285_800A67C8[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67D0[4] = { 0x7023, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67D8[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67E0[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67E8[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A67F0[6] = { 0x6016, 1, 0xC05, 0, 0xFFFF, 0 };
u16 D_WSTAG285_800A67FC[6] = { 0xC06, 0, 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A6808[6] = { 0x6016, 1, 0xC07, 0, 0xFFFF, 0 };
u16 D_WSTAG285_800A6814[6] = { 0x6016, 1, 0xC07, 0, 0xFFFF, 0 };
u16 D_WSTAG285_800A6820[4] = { 0x7023, 1, 0xFFFF, 0 };
u16 D_WSTAG285_800A6828[4] = { 0x7023, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG285_800A6830 = { D_WSTAG285_800A67B8, NULL, 11, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG285_800A6844 = { D_WSTAG285_800A67C0, D_WSTAG285_800A66F8, 33, 5, 825, 285, 1 };
FieldstgPlacedActor D_WSTAG285_800A6858 = { D_WSTAG285_800A67C8, NULL, 37, 6, 560, 673, 1 };
FieldstgPlacedActor D_WSTAG285_800A686C = { D_WSTAG285_800A67D0, D_WSTAG285_800A6710, 37, 6, 560, 673, 1 };
FieldstgPlacedActor D_WSTAG285_800A6880 = { D_WSTAG285_800A67D8, NULL, 38, 7, 823, 819, 3 };
FieldstgPlacedActor D_WSTAG285_800A6894 = { D_WSTAG285_800A67E0, NULL, 39, 8, 438, 244, 1 };
FieldstgPlacedActor D_WSTAG285_800A68A8 = { D_WSTAG285_800A67E8, NULL, 106, 9, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG285_800A68BC = { D_WSTAG285_800A67F0, D_WSTAG285_800A6728, 306, 10, 823, 819, 3 };
FieldstgPlacedActor D_WSTAG285_800A68D0 = { D_WSTAG285_800A67FC, D_WSTAG285_800A6740, 307, 11, 438, 244, 1 };
FieldstgPlacedActor D_WSTAG285_800A68E4 = { D_WSTAG285_800A6808, D_WSTAG285_800A6758, 308, 12, 370, 574, 3 };
FieldstgPlacedActor D_WSTAG285_800A68F8 = { D_WSTAG285_800A6814, D_WSTAG285_800A6770, 309, 13, 349, 586, 7 };
FieldstgPlacedActor D_WSTAG285_800A690C = { D_WSTAG285_800A6820, D_WSTAG285_800A6788, 381, 14, 823, 819, 3 };
FieldstgPlacedActor D_WSTAG285_800A6920 = { D_WSTAG285_800A6828, D_WSTAG285_800A67A0, 382, 15, 438, 244, 1 };
FieldstgPlacedActor *wstag285_actors[14] = {
    &D_WSTAG285_800A6830, &D_WSTAG285_800A6844, &D_WSTAG285_800A6858, &D_WSTAG285_800A686C, &D_WSTAG285_800A6880,
    &D_WSTAG285_800A6894, &D_WSTAG285_800A68A8, &D_WSTAG285_800A68BC, &D_WSTAG285_800A68D0, &D_WSTAG285_800A68E4,
    &D_WSTAG285_800A68F8, &D_WSTAG285_800A690C, &D_WSTAG285_800A6920, NULL,
};
FieldstgSprite wstag285_sprites[44] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 5, 8, 0, 437, 121, 0, 0 }, { 1, 0, 0x78, 2, 0x32, 2, 0, 1, 4, 0, 121, 400, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 104, 383, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 212, 501, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 1, 4, 0, 264, 240, 0, 0 }, { 1, 0, 0x78, 6, 0x33, 2, 0, 1, 4, 0, 116, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 104, 334, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 212, 215, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 4, 0, 263, 476, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 76, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 152, 217, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 203, 161, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 284, 121, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 8, 0, 748, 208, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 269, 421, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 348, 461, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 429, 501, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 509, 540, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 517, 161, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 597, 201, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 653, 613, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 733, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 813, 693, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 837, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 8, 0, 893, 733, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 2, 0, 5, 8, 0, 554, 542, 0, 0 },
    { 1, 0x64, 0x40, 6, 0xD, 0, 0, 0, 0, 0, 384, 133, 0, 0 },
    { 1, 0, 0x78, 4, 0x32, 2, 0, 1, 4, 0, 172, 373, 385, 0 },
    { 1, 0, 0x40, 4, 0x34, 2, 0, 1, 4, 0, 156, 358, 385, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 833, 737, 808, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 850, 744, 800, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 753, 706, 752, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 672, 666, 712, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 448, 554, 600, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 368, 514, 560, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 288, 474, 520, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 174, 336, 385, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 608, 250, 296, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 528, 210, 256, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 449, 161, 233, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 466, 168, 224, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 408, 135, 186, 0 },
    { 1, 0, 0x78, 4, 0x33, 2, 0, 1, 4, 0, 167, 255, 385, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag285_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x217, 0x318, 0x1DC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x203, 0x14A, 0xC4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x215, 0xC8, 0x7C, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag285_funcs = { wstag285_setup };
FieldstgEventDef wstag285_events[4] = {
    { 40, D_WSTAG285_800A60B0, 0x0112000A, NULL, NULL },
    { 272, D_WSTAG285_800A6174, 0x01120019, NULL, wstag285_event_272_end },
    { 340, D_WSTAG285_800A6294, 0x0112001F, NULL, wstag285_event_340_end }, { -1, NULL, 0, NULL, NULL },
};
