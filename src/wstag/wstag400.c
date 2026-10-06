#include "wstag.h"

/* WSTAG400: stage 0x22A (fieldstg_stages). */

extern WstagFuncs wstag400_funcs;
const CVECTOR wstag400_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgBattleLists wstag400_battle_lists;
extern FieldstgBattleLists wstag400_battle_lists2;
extern FieldstgVramPlace wstag400_vram_places[];
extern FieldstgPlacedActor *wstag400_actors[];
extern FieldstgSprite wstag400_sprites[];
extern FieldstgMapEvent wstag400_map_events[];
extern FieldstgEventDef wstag400_events[];
void wstag400_update();

void wstag400_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_flags.get_flag(0x1A26, 1) && gamestate_flags.get_flag(0x1C4B, 0)) {
            data->event = fieldstg_event_start(0x514);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag400_start(void *arg0) {
    WstagObject *obj = object_new(wstag400_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag400_funcs.setup();
    return obj;
}

void wstag400_event_1300_end(void) {
    gamestate_flags.set_flag(0x1C4B, 1);
}

void wstag400_event_1301_end(void) {
    gamestate_flags.set_flag(0x1C4C, 1);
}

void wstag400_event_1302_end(void) {
    gamestate_flags.set_flag(0x1C4D, 1);
}

void wstag400_event_1303_end(void) {
    gamestate_flags.set_flag(0x1C4E, 1);
}

void wstag400_event_1304_end(void) {
    gamestate_flags.set_flag(0x1A27, 1);
    gamestate_flags.set_flag(0x1A26, 0);
}

void wstag400_event_1507_end(void) {
    gamestate_flags.set_flag(0x1A26, 0);
    gamestate_flags.set_flag(0x1C4B, 0);
    gamestate_flags.set_flag(0x1C4C, 0);
    gamestate_flags.set_flag(0x1C4D, 0);
    gamestate_flags.set_flag(0x1C4E, 0);
}

void wstag400_setup(void) {
    fieldstg_stage.background_file = 0x409;
    fieldstg_stage.sprite_file = 0x040A0000;
    fieldstg_stage.sprites = wstag400_sprites;
    fieldstg_stage.map_events = wstag400_map_events;
    fieldstg_stage.mask_file = 0x408;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x12C00, 0x12C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag400_vram_places;
    fieldstg_stage.music = 0x2D;
    fieldstg_stage.sound = 0x60B40000;
    fieldstg_stage.actors = wstag400_actors;
    fieldstg_stage.color = wstag400_color;
    fieldstg_stage.events = wstag400_events;
    fieldstg_attr.set_file(0, 0x040A0001);
    fieldstg_attr.set_file(7, 0x040A0002);
    fieldstg_attr.set_file(4, 0x040A0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0xE) {
        fieldstg_stage.battle_lists = &wstag400_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag400_battle_lists2;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag400_setup(void);

s16 D_WSTAG400_800A61C4[40] = {
    FIELDSTG_EVENT_PLACE(2, 239, 225),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 368, 288, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A6214[45] = {
    FIELDSTG_EVENT_WALK(2, 1016, 261, 7),
    FIELDSTG_EVENT_WALK(34, 1048, 276, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(34),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(34, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 34, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A6270[50] = {
    FIELDSTG_EVENT_WALK(2, 1124, 1055, 7),
    FIELDSTG_EVENT_WALK(226, 1156, 1071, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(226),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(226, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 226, 2),
    FIELDSTG_EVENT_DIALOG(0, 1, 226, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A62D4[45] = {
    FIELDSTG_EVENT_WALK(2, 184, 844, 1),
    FIELDSTG_EVENT_WALK(227, 153, 860, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(227),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(227, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 227, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A6330[54] = {
    FIELDSTG_EVENT_WALK(2, 328, 428, 7),
    FIELDSTG_EVENT_WALK(35, 360, 445, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(35),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(35, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 35, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(35, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x229, 1487, 1000, 7),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A639C[67] = {
    FIELDSTG_EVENT_WALK(2, 176, 193, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x229, 1620, 1090, 3),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A6424[67] = {
    FIELDSTG_EVENT_WALK(2, 1608, 205, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x22B, 176, 1272, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG400_800A64AC[67] = {
    FIELDSTG_EVENT_WALK(2, 208, 765, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x22C, 900, 352, 3),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG400_800A6534 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6540 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A654C = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6558 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6564 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6570 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A657C = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6588 = { 37, 8, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A6594 = {
    3,
    { &D_WSTAG400_800A6534, &D_WSTAG400_800A6540, &D_WSTAG400_800A654C, &D_WSTAG400_800A6558, &D_WSTAG400_800A6564,
        &D_WSTAG400_800A6570, &D_WSTAG400_800A657C, &D_WSTAG400_800A6588 },
};
FieldstgListedBattle D_WSTAG400_800A65B8 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A65C4 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A65D0 = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A65DC = { 49, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A65E8 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A65F4 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6600 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A660C = { 40, 13, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A6618 = {
    3,
    { &D_WSTAG400_800A65B8, &D_WSTAG400_800A65C4, &D_WSTAG400_800A65D0, &D_WSTAG400_800A65DC, &D_WSTAG400_800A65E8,
        &D_WSTAG400_800A65F4, &D_WSTAG400_800A6600, &D_WSTAG400_800A660C },
};
FieldstgListedBattle D_WSTAG400_800A663C = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6648 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6654 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6660 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A666C = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6678 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6684 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6690 = { 0, 4, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A669C = {
    0,
    { &D_WSTAG400_800A663C, &D_WSTAG400_800A6648, &D_WSTAG400_800A6654, &D_WSTAG400_800A6660, &D_WSTAG400_800A666C,
        &D_WSTAG400_800A6678, &D_WSTAG400_800A6684, &D_WSTAG400_800A6690 },
};
FieldstgListedBattle D_WSTAG400_800A66C0 = { 207, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG400_800A66CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG400_800A66D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG400_800A66E4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A66F0 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A66FC = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6708 = { 147, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6714 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A6720 = {
    0,
    { &D_WSTAG400_800A66C0, &D_WSTAG400_800A66CC, &D_WSTAG400_800A66D8, &D_WSTAG400_800A66E4, &D_WSTAG400_800A66F0,
        &D_WSTAG400_800A66FC, &D_WSTAG400_800A6708, &D_WSTAG400_800A6714 },
};
FieldstgListedBattle D_WSTAG400_800A6744 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6750 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A675C = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6768 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6774 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6780 = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A678C = { 37, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6798 = { 37, 8, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A67A4 = {
    3,
    { &D_WSTAG400_800A6744, &D_WSTAG400_800A6750, &D_WSTAG400_800A675C, &D_WSTAG400_800A6768, &D_WSTAG400_800A6774,
        &D_WSTAG400_800A6780, &D_WSTAG400_800A678C, &D_WSTAG400_800A6798 },
};
FieldstgListedBattle D_WSTAG400_800A67C8 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A67D4 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A67E0 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A67EC = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A67F8 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6804 = { 145, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6810 = { 40, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A681C = { 40, 13, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A6828 = {
    3,
    { &D_WSTAG400_800A67C8, &D_WSTAG400_800A67D4, &D_WSTAG400_800A67E0, &D_WSTAG400_800A67EC, &D_WSTAG400_800A67F8,
        &D_WSTAG400_800A6804, &D_WSTAG400_800A6810, &D_WSTAG400_800A681C },
};
FieldstgListedBattle D_WSTAG400_800A684C = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6858 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6864 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6870 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A687C = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6888 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6894 = { 0, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A68A0 = { 0, 4, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A68AC = {
    0,
    { &D_WSTAG400_800A684C, &D_WSTAG400_800A6858, &D_WSTAG400_800A6864, &D_WSTAG400_800A6870, &D_WSTAG400_800A687C,
        &D_WSTAG400_800A6888, &D_WSTAG400_800A6894, &D_WSTAG400_800A68A0 },
};
FieldstgListedBattle D_WSTAG400_800A68D0 = { 207, 13, 0x600C0000 };
FieldstgListedBattle D_WSTAG400_800A68DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG400_800A68E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG400_800A68F4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6900 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A690C = { 101, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6918 = { 147, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG400_800A6924 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG400_800A6930 = {
    0,
    { &D_WSTAG400_800A68D0, &D_WSTAG400_800A68DC, &D_WSTAG400_800A68E8, &D_WSTAG400_800A68F4, &D_WSTAG400_800A6900,
        &D_WSTAG400_800A690C, &D_WSTAG400_800A6918, &D_WSTAG400_800A6924 },
};
FieldstgBattleLists wstag400_battle_lists = {
    9, 0, 0, { &D_WSTAG400_800A6594, &D_WSTAG400_800A6618, &D_WSTAG400_800A669C }, &D_WSTAG400_800A6720,
};
FieldstgBattleLists wstag400_battle_lists2 = {
    28, 1, 0, { &D_WSTAG400_800A67A4, &D_WSTAG400_800A6828, &D_WSTAG400_800A68AC }, &D_WSTAG400_800A6930,
};
FieldstgVramPlace wstag400_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 320, 480, 64, 352, 511 }, { 384, 256, 436, 256, 464, 0, 368, 511 },
    { 384, 256, 436, 288, 464, 32, 320, 510 }, { 384, 256, 424, 293, 416, 37, 336, 510 },
    { 384, 256, 432, 320, 448, 64, 352, 510 }, { 384, 256, 432, 352, 448, 96, 368, 510 },
    { 384, 256, 432, 384, 448, 128, 320, 509 },
};
u16 D_WSTAG400_800A6A5C[8] = { 0x228, 1, 0x8AFE, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A6C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A74[6] = { 0x9031, 1, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A80[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A88[4] = { 0x9034, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A90[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6A98[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AA0[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AAC[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AB8[4] = { 0x7611, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AC0[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AC8[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AD4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6ADC[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AE8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AF4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6AFC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B04[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B10[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B1C[4] = { 0x7611, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B24[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B2C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B34[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B40[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B4C[4] = { 0x7611, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B54[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B5C[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B68[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B70[8] = { 0x10, 1, 0x9207, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6B80[10] = {
    0x11, 0, 0x9207, 1, 0x7013, 1, 0x10, 0,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6B94[8] = { 0x10, 1, 0x9207, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BA4[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BB0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BB8[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BC0[6] = { 0, 1, 0xE07, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BCC[6] = { 0xE07, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BD8[8] = { 0, 1, 0xE07, 1, 0x720D, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BE8[8] = { 0, 1, 0xE07, 1, 0x720D, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6BF8[4] = { 0x7811, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C00[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C08[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C14[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C20[4] = { 0x7611, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C28[4] = { 1, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C30[6] = { 0x9032, 1, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C3C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C44[4] = { 2, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C4C[6] = { 0x9033, 1, 2, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6C58[4] = { 2, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG400_800A6C60[2] = { { NULL, D_WSTAG400_800A6A5C, 621 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG400_800A6C78[3] = {
    { D_WSTAG400_800A6A6C, D_WSTAG400_800A6A74, 738 }, { D_WSTAG400_800A6A80, NULL, 815 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6C9C[2] = { { NULL, NULL, 815 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG400_800A6CB4[2] = { { NULL, D_WSTAG400_800A6A88, 738 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG400_800A6CCC[4] = {
    { D_WSTAG400_800A6A90, D_WSTAG400_800A6A98, 123 }, { D_WSTAG400_800A6AA0, NULL, 128 },
    { D_WSTAG400_800A6AAC, D_WSTAG400_800A6AB8, 129 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6CFC[2] = { { NULL, NULL, 637 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG400_800A6D14[4] = {
    { D_WSTAG400_800A6AC0, NULL, 123 }, { D_WSTAG400_800A6AC8, D_WSTAG400_800A6AD4, 133 },
    { D_WSTAG400_800A6ADC, D_WSTAG400_800A6AE8, 134 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6D44[4] = {
    { D_WSTAG400_800A6AF4, D_WSTAG400_800A6AFC, 124 }, { D_WSTAG400_800A6B04, NULL, 128 },
    { D_WSTAG400_800A6B10, D_WSTAG400_800A6B1C, 129 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6D74[4] = {
    { D_WSTAG400_800A6B24, D_WSTAG400_800A6B2C, 125 }, { D_WSTAG400_800A6B34, NULL, 128 },
    { D_WSTAG400_800A6B40, D_WSTAG400_800A6B4C, 129 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6DA4[5] = {
    { D_WSTAG400_800A6B54, NULL, 123 }, { D_WSTAG400_800A6B5C, D_WSTAG400_800A6B68, 135 },
    { D_WSTAG400_800A6B70, D_WSTAG400_800A6B80, 136 }, { D_WSTAG400_800A6B94, D_WSTAG400_800A6BA4, 137 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6DE0[5] = {
    { D_WSTAG400_800A6BB0, D_WSTAG400_800A6BB8, 127 }, { D_WSTAG400_800A6BC0, D_WSTAG400_800A6BCC, 130 },
    { D_WSTAG400_800A6BD8, NULL, 131 }, { D_WSTAG400_800A6BE8, D_WSTAG400_800A6BF8, 132 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6E1C[4] = {
    { D_WSTAG400_800A6C00, NULL, 126 }, { D_WSTAG400_800A6C08, NULL, 126 },
    { D_WSTAG400_800A6C14, D_WSTAG400_800A6C20, 126 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6E4C[3] = {
    { D_WSTAG400_800A6C28, D_WSTAG400_800A6C30, 738 }, { D_WSTAG400_800A6C3C, NULL, 815 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6E70[2] = { { NULL, NULL, 815 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG400_800A6E88[3] = {
    { D_WSTAG400_800A6C44, D_WSTAG400_800A6C4C, 738 }, { D_WSTAG400_800A6C58, NULL, 815 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG400_800A6EAC[2] = { { NULL, NULL, 815 }, { NULL, NULL, 0 } };
u16 D_WSTAG400_800A6EC4[4] = { 0x228, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6ECC[6] = { 0x1A26, 1, 0x1C4C, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6ED8[6] = { 0x1A26, 1, 0x1C4C, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6EE4[4] = { 0x1A26, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6EEC[10] = {
    0x11, 0, 0x8192, 1, 0x7003, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F00[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6F10[10] = {
    0x8192, 1, 0x11, 1, 0x7022, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F24[10] = {
    0x11, 0, 0x7004, 1, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F38[10] = {
    0x8192, 1, 0x11, 0, 0x6026, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F4C[10] = {
    0x8192, 1, 0x11, 1, 0x8012, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F60[10] = {
    0x8192, 1, 0x8012, 1, 0x11, 0, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG400_800A6F74[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6F7C[6] = { 0x1A26, 1, 0x1C4D, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6F88[6] = { 0x1A26, 1, 0x1C4D, 1, 0xFFFF, 0 };
u16 D_WSTAG400_800A6F94[6] = { 0x1A26, 1, 0x1C4E, 0, 0xFFFF, 0 };
u16 D_WSTAG400_800A6FA0[6] = { 0x1A26, 1, 0x1C4E, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG400_800A6FAC = { D_WSTAG400_800A6EC4, D_WSTAG400_800A6C60, 33, 4, 1407, 809, 1 };
FieldstgPlacedActor D_WSTAG400_800A6FC0 = { D_WSTAG400_800A6ECC, D_WSTAG400_800A6C78, 34, 5, 977, 238, 1 };
FieldstgPlacedActor D_WSTAG400_800A6FD4 = { D_WSTAG400_800A6ED8, D_WSTAG400_800A6C9C, 34, 5, 1048, 276, 7 };
FieldstgPlacedActor D_WSTAG400_800A6FE8 = { D_WSTAG400_800A6EE4, D_WSTAG400_800A6CB4, 35, 6, 255, 391, 1 };
FieldstgPlacedActor D_WSTAG400_800A6FFC = { D_WSTAG400_800A6EEC, D_WSTAG400_800A6CCC, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7010 = { D_WSTAG400_800A6F00, D_WSTAG400_800A6CFC, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7024 = { D_WSTAG400_800A6F10, D_WSTAG400_800A6D14, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7038 = { D_WSTAG400_800A6F24, D_WSTAG400_800A6D44, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A704C = { D_WSTAG400_800A6F38, D_WSTAG400_800A6D74, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7060 = { D_WSTAG400_800A6F4C, D_WSTAG400_800A6DA4, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7074 = { D_WSTAG400_800A6F60, D_WSTAG400_800A6DE0, 57, 7, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A7088 = { D_WSTAG400_800A6F74, D_WSTAG400_800A6E1C, 157, 8, 1655, 1299, 7 };
FieldstgPlacedActor D_WSTAG400_800A709C = { D_WSTAG400_800A6F7C, D_WSTAG400_800A6E4C, 226, 9, 1064, 1024, 1 };
FieldstgPlacedActor D_WSTAG400_800A70B0 = { D_WSTAG400_800A6F88, D_WSTAG400_800A6E70, 226, 9, 1156, 1071, 7 };
FieldstgPlacedActor D_WSTAG400_800A70C4 = { D_WSTAG400_800A6F94, D_WSTAG400_800A6E88, 227, 10, 246, 813, 1 };
FieldstgPlacedActor D_WSTAG400_800A70D8 = { D_WSTAG400_800A6FA0, D_WSTAG400_800A6EAC, 227, 10, 153, 860, 1 };
FieldstgPlacedActor *wstag400_actors[17] = {
    &D_WSTAG400_800A6FAC, &D_WSTAG400_800A6FC0, &D_WSTAG400_800A6FD4, &D_WSTAG400_800A6FE8, &D_WSTAG400_800A6FFC,
    &D_WSTAG400_800A7010, &D_WSTAG400_800A7024, &D_WSTAG400_800A7038, &D_WSTAG400_800A704C, &D_WSTAG400_800A7060,
    &D_WSTAG400_800A7074, &D_WSTAG400_800A7088, &D_WSTAG400_800A709C, &D_WSTAG400_800A70B0, &D_WSTAG400_800A70C4,
    &D_WSTAG400_800A70D8, NULL,
};
FieldstgSprite wstag400_sprites[215] = {
    { 1, 0, 0x78, 2, 0, 0, 0, 0, 0, 0, 432, 785, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 730, 539, 0, 0 },
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 791, 950, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 792, 87, 0, 0 },
    { 1, 0, 0x40, 2, 9, 0, 0, 0, 0, 0, 303, 734, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 351, 829, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 374, 627, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 500, 726, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 571, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 631, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 639, 644, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 862, 571, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 954, 512, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1127, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 341, 879, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 374, 929, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 485, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 684, 535, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 784, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 928, 521, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1013, 479, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1117, 497, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 303, 945, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 329, 932, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 390, 785, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 438, 796, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 445, 512, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 450, 586, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 519, 438, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 620, 642, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 663, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 825, 587, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 986, 492, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1456, 664, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 513, 546, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 687, 476, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1189, 636, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1215, 553, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1344, 618, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 444, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 473, 730, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 544, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 669, 458, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 760, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1052, 481, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1152, 493, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1272, 585, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1397, 643, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 434, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 446, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 588, 580, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 730, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 788, 597, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 807, 504, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1087, 490, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1204, 634, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1237, 569, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1284, 600, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 343, 853, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 354, 839, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 359, 937, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 368, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 372, 910, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 398, 765, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 415, 714, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 445, 783, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 456, 726, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 627, 650, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 652, 632, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 824, 605, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 834, 506, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 883, 535, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 974, 510, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1030, 1476, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1144, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1155, 526, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1159, 517, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1276, 774, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1287, 778, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1294, 769, 0, 0 },
    { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 177, 719, 0, 0 },
    { 1, 0, 0xE0, 0xA, 0x4A, 1, 0x4A, 0x4C, 8, 0, 184, 992, 0, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 134, 729, 771, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 139, 776, 813, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 166, 303, 303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 184, 248, 248, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 204, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 471, 471, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 240, 311, 311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 240, 503, 503, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 246, 871, 871, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 351, 351, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 447, 447, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 264, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 391, 391, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 272, 791, 791, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 288, 479, 479, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 327, 327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 304, 423, 423, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 345, 212, 212, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 367, 263, 263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 399, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 951, 951, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 458, 252, 252, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 303, 303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 480, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 506, 276, 276, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 544, 1135, 1135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 560, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 823, 823, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 1015, 1015, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 608, 319, 319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 633, 291, 291, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 1039, 1039, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 640, 1183, 1183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 647, 827, 827, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 1071, 1071, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 681, 315, 315, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 1207, 1207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 703, 815, 815, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 1047, 1047, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 729, 339, 339, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 1087, 1087, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 711, 711, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 752, 807, 807, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 768, 1119, 1119, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 783, 843, 843, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 784, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 800, 1255, 1255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 832, 815, 815, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 832, 1135, 1135, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 848, 759, 759, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 1167, 1167, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 1231, 1231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 880, 791, 791, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 911, 970, 970, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 912, 1143, 1143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 927, 1241, 1241, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 799, 799, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 1183, 1183, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 935, 858, 858, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 960, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 983, 983, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 976, 1159, 1159, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 989, 1034, 1034, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 1262, 1262, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 997, 201, 201, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1024, 959, 959, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1040, 999, 999, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1040, 1143, 1143, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1042, 1251, 1251, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1055, 215, 215, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1064, 1083, 1083, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1087, 1222, 1222, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1088, 1119, 1119, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1104, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 319, 319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 911, 911, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1120, 1007, 1007, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1145, 1251, 1251, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1152, 255, 255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1152, 943, 943, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1168, 295, 295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1168, 983, 983, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1184, 335, 335, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1184, 1279, 1279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1200, 919, 919, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1200, 1239, 1239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1216, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1216, 959, 959, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1223, 1188, 1188, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1232, 1303, 1303, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1248, 1263, 1263, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 935, 935, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1264, 1175, 1175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 975, 975, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1280, 1327, 1327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1311, 1182, 1182, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1312, 327, 327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1312, 367, 367, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1321, 1146, 1146, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 951, 951, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 991, 991, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 1118, 1118, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1344, 1311, 1311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1360, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1376, 1023, 1023, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1392, 1327, 1327, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1398, 1051, 1051, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1400, 227, 227, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1440, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1440, 1063, 1063, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1456, 1319, 1319, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1488, 407, 407, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1503, 1295, 1295, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1504, 223, 223, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1525, 443, 443, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1542, 298, 298, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1568, 1279, 1279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1584, 279, 279, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1601, 464, 464, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag400_map_events[25] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x22B, 0xB0, 0x4F8, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x229, 0x654, 0x442, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x22C, 0x384, 0x160, 3, 0x64, 0, 0 },
    { 0x7093, 1, 0x1A26, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 1, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 2, 1 },
    { 0x7094, 1, 0x1A26, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 3, 4 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x34D, 0xFA, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x33F, 0x16C, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x59E, 0x111, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x58F, 0x163, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x2DE, 0x4E3, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x2D0, 0x550, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0x40, 0x30, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0xFFC0, 0x30, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0x8004, 1, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0x60, 0x10, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0x50, 0xFFE8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0xFFC0, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0x8004, 1, 7, 0xFFD0, 0xFFF0, 0, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0x1A26, 1, 0xFFFF, 0, 8, 0x5E3, 0, 0, 0, 0, 0, 0 }, { 0x1A26, 1, 0xFFFF, 0, 8, 0x5E4, 0, 0, 0, 0, 0, 0 },
    { 0x1A26, 1, 0xFFFF, 0, 8, 0x5E5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag400_funcs = { wstag400_setup };
FieldstgEventDef wstag400_events[10] = {
    { 1300, D_WSTAG400_800A61C4, 0x01350022, NULL, wstag400_event_1300_end },
    { 1301, D_WSTAG400_800A6214, 0x01350023, NULL, wstag400_event_1301_end },
    { 1302, D_WSTAG400_800A6270, 0x01350024, NULL, wstag400_event_1302_end },
    { 1303, D_WSTAG400_800A62D4, 0x01350025, NULL, wstag400_event_1303_end },
    { 1304, D_WSTAG400_800A6330, 0x01350026, NULL, wstag400_event_1304_end },
    { 1507, D_WSTAG400_800A639C, 0x0135002E, NULL, wstag400_event_1507_end },
    { 1508, D_WSTAG400_800A6424, 0x0135002F, NULL, wstag400_event_1507_end },
    { 1509, D_WSTAG400_800A64AC, 0x01350030, NULL, wstag400_event_1507_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
