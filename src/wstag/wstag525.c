#include "wstag.h"

/* WSTAG525: stage 0x241 (fieldstg_stages). */

extern WstagFuncs wstag525_funcs;
extern FieldstgBattleLists wstag525_battle_lists;
extern FieldstgVramPlace wstag525_vram_places[];
extern FieldstgPlacedActor *wstag525_actors[];
extern FieldstgSprite wstag525_sprites[];
extern FieldstgMapEvent wstag525_map_events[];
extern FieldstgEventDef wstag525_events[];
void wstag525_update();

void wstag525_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x402B, 1) && gamestate_flags.get_flag(0x402C, 0)) {
            data->event = fieldstg_event_start(0x4FA);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag525_start(void *arg0) {
    WstagObject *obj = object_new(wstag525_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag525_funcs.setup();
    return obj;
}

void wstag525_event_550_end(void) {
    gamestate_flags.set_flag(0x4003, 1);
}

void wstag525_event_1273_end(void) {
    gamestate_flags.set_flag(0x402B, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag525_event_1274_end(void) {
    gamestate_flags.set_flag(0x402C, 1);
    gamestate_flags.set_flag(0x8013, 1);
}

void wstag525_setup(void) {
    fieldstg_stage.background_file = 0x3C2;
    fieldstg_stage.sprite_file = 0x03C30000;
    fieldstg_stage.sprites = wstag525_sprites;
    fieldstg_stage.map_events = wstag525_map_events;
    fieldstg_stage.mask_file = 0x3C1;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x18600, 0x18600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag525_vram_places;
    fieldstg_stage.music = 0x37;
    fieldstg_stage.sound = 0x60DC0000;
    fieldstg_stage.actors = wstag525_actors;
    fieldstg_stage.battle_lists = &wstag525_battle_lists;
    fieldstg_stage.events = wstag525_events;
    fieldstg_attr.set_file(0, 0x03C30001);
    fieldstg_attr.set_file(7, 0x03C30002);
    fieldstg_attr.set_file(4, 0x03C30003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag525_setup(void);

s16 D_WSTAG525_800A60B4[345] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_PLACE(102, 113, 729),
    FIELDSTG_EVENT_ANIM(102, 1, 5),
    FIELDSTG_EVENT_PLACE(103, 145, 713),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 102),
    FIELDSTG_EVENT_DIALOG(0, 2, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 102, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 102, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 96, 712, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 102),
    FIELDSTG_EVENT_ANIM(0x324, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(102, 1, 3),
    FIELDSTG_EVENT_ANIM(103, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 102),
    FIELDSTG_EVENT_ANIM(0x324, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 103, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 102, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 102),
    FIELDSTG_EVENT_ANIM(0x324, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 102),
    FIELDSTG_EVENT_ANIM(0x324, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 102, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(102, 1, 5),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 103, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(103, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(102, 143, 721, 7),
    FIELDSTG_EVENT_WALK(103, 112, 705, 3),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WALK(102, 112, 705, 3),
    FIELDSTG_EVENT_WALK(103, 144, 689, 5),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_WALK(102, 160, 679, 5),
    FIELDSTG_EVENT_WALK(103, 192, 663, 5),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_DIALOG(0, 15, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 102, 0),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_ANIM(103, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 103, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 18, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(102, 1, 5),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 234, 644, 5),
    FIELDSTG_EVENT_WALK(102, 234, 644, 5),
    FIELDSTG_EVENT_WALK(103, 234, 644, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x255, 616, 428, 3),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG525_800A6368[55] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 160, 184, 3),
    FIELDSTG_EVENT_PLACE(128, 128, 168),
    FIELDSTG_EVENT_ANIM(128, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 128, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG525_800A63D8[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 160, 184),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(128, 128, 168),
    FIELDSTG_EVENT_ANIM(128, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 128, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG525_800A645C = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A6468 = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A6474 = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A6480 = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A648C = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A6498 = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A64A4 = { 65, 27, 0x60080000 };
FieldstgListedBattle D_WSTAG525_800A64B0 = { 65, 27, 0x60080000 };
FieldstgBattleList D_WSTAG525_800A64BC = {
    3,
    { &D_WSTAG525_800A645C, &D_WSTAG525_800A6468, &D_WSTAG525_800A6474, &D_WSTAG525_800A6480, &D_WSTAG525_800A648C,
        &D_WSTAG525_800A6498, &D_WSTAG525_800A64A4, &D_WSTAG525_800A64B0 },
};
FieldstgListedBattle D_WSTAG525_800A64E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A64EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A64F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6510 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A651C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6534 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG525_800A6540 = {
    0,
    { &D_WSTAG525_800A64E0, &D_WSTAG525_800A64EC, &D_WSTAG525_800A64F8, &D_WSTAG525_800A6504, &D_WSTAG525_800A6510,
        &D_WSTAG525_800A651C, &D_WSTAG525_800A6528, &D_WSTAG525_800A6534 },
};
FieldstgListedBattle D_WSTAG525_800A6564 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6570 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A657C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6588 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6594 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A65A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A65AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A65B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG525_800A65C4 = {
    0,
    { &D_WSTAG525_800A6564, &D_WSTAG525_800A6570, &D_WSTAG525_800A657C, &D_WSTAG525_800A6588, &D_WSTAG525_800A6594,
        &D_WSTAG525_800A65A0, &D_WSTAG525_800A65AC, &D_WSTAG525_800A65B8 },
};
FieldstgListedBattle D_WSTAG525_800A65E8 = { 268, 27, 0x60880000 };
FieldstgListedBattle D_WSTAG525_800A65F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6600 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A660C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6618 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6624 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A6630 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG525_800A663C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG525_800A6648 = {
    0,
    { &D_WSTAG525_800A65E8, &D_WSTAG525_800A65F4, &D_WSTAG525_800A6600, &D_WSTAG525_800A660C, &D_WSTAG525_800A6618,
        &D_WSTAG525_800A6624, &D_WSTAG525_800A6630, &D_WSTAG525_800A663C },
};
FieldstgBattleLists wstag525_battle_lists = {
    57, 0, 0, { &D_WSTAG525_800A64BC, &D_WSTAG525_800A6540, &D_WSTAG525_800A65C4 }, &D_WSTAG525_800A6648,
};
FieldstgVramPlace wstag525_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 410, 408, 360, 152, 368, 510 }, { 320, 256, 366, 448, 184, 192, 368, 509 },
    { 320, 256, 372, 448, 208, 192, 368, 508 }, { 320, 256, 340, 448, 80, 192, 368, 507 },
};
u16 D_WSTAG525_800A6728[8] = { 0x208, 1, 0x88C0, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG525_800A6738[4] = { 0x1C16, 0, 0xFFFF, 0 };
u16 D_WSTAG525_800A6740[6] = { 0x9029, 1, 0x1C16, 1, 0xFFFF, 0 };
u16 D_WSTAG525_800A674C[4] = { 0x1C16, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG525_800A6754[2] = { { NULL, D_WSTAG525_800A6728, 593 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG525_800A676C[3] = {
    { D_WSTAG525_800A6738, D_WSTAG525_800A6740, 709 }, { D_WSTAG525_800A674C, NULL, 710 }, { NULL, NULL, 0 },
};
u16 D_WSTAG525_800A6790[4] = { 0x208, 0, 0xFFFF, 0 };
u16 D_WSTAG525_800A6798[6] = { 0x6014, 1, 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG525_800A67A4[6] = { 0x6014, 1, 0x1C1C, 1, 0xFFFF, 0 };
u16 D_WSTAG525_800A67B0[6] = { 0x604, 1, 0x8013, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG525_800A67BC = { D_WSTAG525_800A6790, D_WSTAG525_800A6754, 33, 4, 545, 177, 1 };
FieldstgPlacedActor D_WSTAG525_800A67D0 = { D_WSTAG525_800A6798, NULL, 102, 5, 113, 729, 5 };
FieldstgPlacedActor D_WSTAG525_800A67E4 = { D_WSTAG525_800A67A4, NULL, 103, 6, 145, 713, 1 };
FieldstgPlacedActor D_WSTAG525_800A67F8 = { D_WSTAG525_800A67B0, D_WSTAG525_800A676C, 128, 7, 128, 168, 7 };
FieldstgPlacedActor *wstag525_actors[5] = {
    &D_WSTAG525_800A67BC, &D_WSTAG525_800A67D0, &D_WSTAG525_800A67E4, &D_WSTAG525_800A67F8, NULL,
};
FieldstgSprite wstag525_sprites[69] = {
    { 1, 0, 0x40, 2, 0x59, 3, 0, 0xB, 0xA, 0, 196, 230, 0, 0 },
    { 1, 0, 0x40, 2, 0x58, 3, 0, 0xB, 0xA, 0, 355, 190, 0, 0 },
    { 1, 0, 0x40, 2, 0x5A, 3, 0, 0xB, 0xA, 0, 220, 263, 0, 0 },
    { 1, 0, 0xC8, 6, 0x4A, 3, 0, 0xD, 0xA, 0, 256, 20, 0, 0 },
    { 1, 0, 0xC8, 6, 0x4B, 3, 0, 0xD, 0xA, 0, 580, 86, 0, 0 },
    { 1, 0, 0xC8, 6, 0x4C, 3, 0, 0xD, 0xA, 0, 444, 40, 0, 0 },
    { 1, 0, 0xC8, 6, 0x4D, 3, 0, 0xD, 0xA, 0, 562, 430, 0, 0 },
    { 1, 0, 0x64, 6, 0x4E, 3, 0, 9, 8, 0, 24, 48, 0, 0 }, { 1, 0, 0x80, 6, 0x4F, 3, 0, 9, 8, 0, 490, 64, 0, 0 },
    { 1, 0, 0x80, 6, 0x50, 3, 0, 9, 8, 0, 188, 497, 0, 0 }, { 1, 0, 0x80, 6, 0x51, 3, 0, 9, 8, 0, 589, 489, 0, 0 },
    { 1, 0, 0x40, 6, 0x52, 3, 0, 9, 0xA, 0, 435, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x53, 3, 0, 9, 0xC, 0, 472, 309, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 3, 0, 9, 0xC, 0, 549, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 3, 0, 0xB, 0xA, 0, 228, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 3, 0, 0xB, 0xA, 0, 245, 184, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 3, 0, 0xB, 0xA, 0, 271, 188, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 158, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 292, 1043, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 458, 1054, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 487, 547, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 582, 1050, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 633, 966, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 725, 881, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 191, 910, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 253, 1029, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 313, 1044, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 316, 981, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 415, 457, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 417, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 584, 875, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 119, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 140, 860, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 212, 1046, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 256, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 467, 509, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 615, 1035, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 679, 960, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 728, 917, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 139, 608, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 161, 621, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 423, 1047, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 532, 1048, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 158, 607, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 117, 863, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 150, 1030, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 228, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 235, 944, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 353, 1000, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 82, 858, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 140, 523, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 242, 537, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 274, 1005, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 299, 1048, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 373, 514, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 460, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 493, 1039, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 525, 682, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 529, 712, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 637, 805, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 637, 892, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 675, 850, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 676, 799, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 717, 667, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 728, 657, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 732, 855, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 735, 663, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 339, 392, 409, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag525_map_events[10] = {
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 7, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x22F, 0x3C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x220, 0x372, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x251, 0x2F7, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x260, 0x2A1, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 0xA, 0x1C0, 0x181, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 0xA, 0x1D0, 0xDA, 0, 0, 0, 0 }, { 0x1C1C, 1, 0x6014, 1, 8, 0x226, 0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 7, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag525_funcs = { wstag525_setup };
FieldstgEventDef wstag525_events[4] = {
    { 550, D_WSTAG525_800A60B4, 0x013C001C, NULL, wstag525_event_550_end },
    { 1273, D_WSTAG525_800A6368, 0x013C0012, NULL, wstag525_event_1273_end },
    { 1274, D_WSTAG525_800A63D8, 0x013C0013, NULL, wstag525_event_1274_end }, { -1, NULL, 0, NULL, NULL },
};
