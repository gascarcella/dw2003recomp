#include "wstag.h"

/* WSTAG450: stage 0x234 (fieldstg_stages). */

extern WstagFuncs wstag450_funcs;
const CVECTOR wstag450_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag450_battle_lists[];
extern FieldstgVramPlace wstag450_vram_places[];
extern FieldstgPlacedActor *wstag450_actors[];
extern FieldstgSprite wstag450_sprites[];
extern FieldstgMapEvent wstag450_map_events[];
extern FieldstgEventDef wstag450_events[];
void wstag450_update();

void wstag450_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4031, 1) && gamestate_flags.get_flag(0x4032, 0)) {
            data->event = fieldstg_event_start(0x500);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag450_start(void *arg0) {
    WstagObject *obj = object_new(wstag450_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag450_funcs.setup();
    return obj;
}

void wstag450_event_1279_end(void) {
    gamestate_flags.set_flag(0x4031, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag450_event_1280_end(void) {
    gamestate_flags.set_flag(0x4032, 1);
    gamestate_flags.set_flag(0x8023, 1);
}

void wstag450_setup(void) {
    fieldstg_stage.background_file = 0x398;
    fieldstg_stage.sprite_file = 0x03990000;
    fieldstg_stage.sprites = wstag450_sprites;
    fieldstg_stage.map_events = wstag450_map_events;
    fieldstg_stage.mask_file = 0x397;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1E800, 0x2FB00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag450_vram_places;
    fieldstg_stage.music = 0x34;
    fieldstg_stage.sound = 0x60D00000;
    fieldstg_stage.actors = wstag450_actors;
    fieldstg_stage.color = wstag450_color;
    fieldstg_stage.battle_lists = wstag450_battle_lists;
    fieldstg_stage.events = wstag450_events;
    fieldstg_attr.set_file(0, 0x03990001);
    fieldstg_attr.set_file(7, 0x03990002);
    fieldstg_attr.set_file(4, 0x03990003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0x18) {
        fieldstg_stage.battle_lists = wstag450_battle_lists;
    } else {
        fieldstg_stage.battle_lists = &wstag450_battle_lists[1];
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag450_setup(void);

s16 D_WSTAG450_800A60E4[79] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 330, 293, 3),
    FIELDSTG_EVENT_PLACE(127, 305, 281),
    FIELDSTG_EVENT_ANIM(127, 1, 7),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 127, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 127, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG450_800A6184[74] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 330, 293),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(127, 305, 281),
    FIELDSTG_EVENT_ANIM(127, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 127, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 127, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG450_800A6218 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6224 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6230 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A623C = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6248 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6254 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6260 = { 54, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A626C = { 54, 8, 0x60080000 };
FieldstgBattleList D_WSTAG450_800A6278 = {
    3,
    { &D_WSTAG450_800A6218, &D_WSTAG450_800A6224, &D_WSTAG450_800A6230, &D_WSTAG450_800A623C, &D_WSTAG450_800A6248,
        &D_WSTAG450_800A6254, &D_WSTAG450_800A6260, &D_WSTAG450_800A626C },
};
FieldstgListedBattle D_WSTAG450_800A629C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A62F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG450_800A62FC = {
    0,
    { &D_WSTAG450_800A629C, &D_WSTAG450_800A62A8, &D_WSTAG450_800A62B4, &D_WSTAG450_800A62C0, &D_WSTAG450_800A62CC,
        &D_WSTAG450_800A62D8, &D_WSTAG450_800A62E4, &D_WSTAG450_800A62F0 },
};
FieldstgListedBattle D_WSTAG450_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6344 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6350 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A635C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6368 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6374 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG450_800A6380 = {
    0,
    { &D_WSTAG450_800A6320, &D_WSTAG450_800A632C, &D_WSTAG450_800A6338, &D_WSTAG450_800A6344, &D_WSTAG450_800A6350,
        &D_WSTAG450_800A635C, &D_WSTAG450_800A6368, &D_WSTAG450_800A6374 },
};
FieldstgListedBattle D_WSTAG450_800A63A4 = { 212, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG450_800A63B0 = { 271, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG450_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A63C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A63D4 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A63EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A63F8 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG450_800A6404 = {
    0,
    { &D_WSTAG450_800A63A4, &D_WSTAG450_800A63B0, &D_WSTAG450_800A63BC, &D_WSTAG450_800A63C8, &D_WSTAG450_800A63D4,
        &D_WSTAG450_800A63E0, &D_WSTAG450_800A63EC, &D_WSTAG450_800A63F8 },
};
FieldstgListedBattle D_WSTAG450_800A6428 = { 53, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6434 = { 147, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6440 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A644C = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6458 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6464 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A6470 = { 60, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A647C = { 60, 8, 0x60080000 };
FieldstgBattleList D_WSTAG450_800A6488 = {
    3,
    { &D_WSTAG450_800A6428, &D_WSTAG450_800A6434, &D_WSTAG450_800A6440, &D_WSTAG450_800A644C, &D_WSTAG450_800A6458,
        &D_WSTAG450_800A6464, &D_WSTAG450_800A6470, &D_WSTAG450_800A647C },
};
FieldstgListedBattle D_WSTAG450_800A64AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6500 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG450_800A650C = {
    0,
    { &D_WSTAG450_800A64AC, &D_WSTAG450_800A64B8, &D_WSTAG450_800A64C4, &D_WSTAG450_800A64D0, &D_WSTAG450_800A64DC,
        &D_WSTAG450_800A64E8, &D_WSTAG450_800A64F4, &D_WSTAG450_800A6500 },
};
FieldstgListedBattle D_WSTAG450_800A6530 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A653C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6560 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A656C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6584 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG450_800A6590 = {
    0,
    { &D_WSTAG450_800A6530, &D_WSTAG450_800A653C, &D_WSTAG450_800A6548, &D_WSTAG450_800A6554, &D_WSTAG450_800A6560,
        &D_WSTAG450_800A656C, &D_WSTAG450_800A6578, &D_WSTAG450_800A6584 },
};
FieldstgListedBattle D_WSTAG450_800A65B4 = { 212, 8, 0x600C0000 };
FieldstgListedBattle D_WSTAG450_800A65C0 = { 271, 8, 0x60880000 };
FieldstgListedBattle D_WSTAG450_800A65CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A65D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A65E4 = { 328, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG450_800A65F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A65FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG450_800A6608 = { 66, 8, 0x60080000 };
FieldstgBattleList D_WSTAG450_800A6614 = {
    0,
    { &D_WSTAG450_800A65B4, &D_WSTAG450_800A65C0, &D_WSTAG450_800A65CC, &D_WSTAG450_800A65D8, &D_WSTAG450_800A65E4,
        &D_WSTAG450_800A65F0, &D_WSTAG450_800A65FC, &D_WSTAG450_800A6608 },
};
FieldstgBattleLists wstag450_battle_lists[2] = {
    { 14, 0, 0, { &D_WSTAG450_800A6278, &D_WSTAG450_800A62FC, &D_WSTAG450_800A6380 }, &D_WSTAG450_800A6404 },
    { 55, 1, 0, { &D_WSTAG450_800A6488, &D_WSTAG450_800A650C, &D_WSTAG450_800A6590 }, &D_WSTAG450_800A6614 },
};
FieldstgVramPlace wstag450_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 344, 407, 96, 151, 352, 511 }, { 320, 256, 374, 359, 216, 103, 320, 510 },
    { 320, 256, 346, 256, 104, 0, 336, 510 }, { 320, 256, 330, 388, 40, 132, 352, 510 },
};
u16 D_WSTAG450_800A6710[8] = { 0x204, 1, 0x8B07, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6720[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6728[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6730[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A673C[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A674C[4] = { 0x7616, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6754[10] = {
    0x7202, 1, 0x7204, 1, 0xE0C, 0, 0, 1,
    0xFFFF, 0,
};
u16 D_WSTAG450_800A6768[6] = { 0x7400, 1, 0xE0C, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6774[12] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A678C[14] = {
    0, 1, 0x7204, 1, 0x7202, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A67A8[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A67C4[4] = { 0x7816, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A67CC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A67D4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A67DC[6] = { 0x7202, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A67E8[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A67F8[4] = { 0x7616, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6800[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG450_800A6814[6] = { 0x7400, 1, 0xE0C, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6820[12] = {
    0, 1, 0x8012, 0, 0x7202, 1, 0x7204, 1,
    0xE0C, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6838[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6854[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6870[4] = { 0x7816, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6878[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6880[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A688C[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6894[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A68A0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A68AC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A68B4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A68BC[6] = { 0, 1, 0x7202, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A68C8[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A68D8[4] = { 0x7616, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A68E0[10] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 0,
    0xFFFF, 0,
};
u16 D_WSTAG450_800A68F4[6] = { 0x7400, 1, 0xE0C, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6900[12] = {
    0, 1, 0x7202, 1, 0xE0C, 1, 0x7204, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6918[14] = {
    0x7202, 1, 0xE0C, 1, 0x7206, 0, 0, 1,
    0x7204, 1, 0x8012, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6934[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6950[4] = { 0x7816, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6958[4] = { 0x1C19, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6960[6] = { 0x1C19, 1, 0x902C, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A696C[4] = { 0x1C19, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6974[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A697C[6] = { 0x7202, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6988[8] = { 0, 1, 0x7202, 1, 0x7204, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6998[4] = { 0x7616, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A69A0[10] = {
    0, 1, 0x7202, 1, 0xE0C, 0, 0x7204, 1,
    0xFFFF, 0,
};
u16 D_WSTAG450_800A69B4[4] = { 0xE0C, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A69BC[12] = {
    0, 1, 0x7204, 1, 0x7202, 1, 0xE0C, 1,
    0x8012, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A69D4[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE03, 1,
    0x8012, 1, 0x7206, 0, 0xFFFF, 0,
};
u16 D_WSTAG450_800A69F0[14] = {
    0, 1, 0x7202, 1, 0x7204, 1, 0xE0C, 1,
    0x8012, 1, 0x7206, 1, 0xFFFF, 0,
};
u16 D_WSTAG450_800A6A0C[4] = { 0x7816, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG450_800A6A14[2] = { { NULL, D_WSTAG450_800A6710, 589 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG450_800A6A2C[8] = {
    { D_WSTAG450_800A6720, D_WSTAG450_800A6728, 165 }, { D_WSTAG450_800A6730, NULL, 169 },
    { D_WSTAG450_800A673C, D_WSTAG450_800A674C, 170 }, { D_WSTAG450_800A6754, D_WSTAG450_800A6768, 171 },
    { D_WSTAG450_800A6774, NULL, 172 }, { D_WSTAG450_800A678C, NULL, 173 },
    { D_WSTAG450_800A67A8, D_WSTAG450_800A67C4, 174 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG450_800A6A8C[8] = {
    { D_WSTAG450_800A67CC, D_WSTAG450_800A67D4, 166 }, { D_WSTAG450_800A67DC, NULL, 169 },
    { D_WSTAG450_800A67E8, D_WSTAG450_800A67F8, 170 }, { D_WSTAG450_800A6800, D_WSTAG450_800A6814, 171 },
    { D_WSTAG450_800A6820, NULL, 172 }, { D_WSTAG450_800A6838, NULL, 173 },
    { D_WSTAG450_800A6854, D_WSTAG450_800A6870, 174 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG450_800A6AEC[4] = {
    { D_WSTAG450_800A6878, NULL, 164 }, { D_WSTAG450_800A6880, D_WSTAG450_800A688C, 175 },
    { D_WSTAG450_800A6894, D_WSTAG450_800A68A0, 176 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG450_800A6B1C[8] = {
    { D_WSTAG450_800A68AC, D_WSTAG450_800A68B4, 164 }, { D_WSTAG450_800A68BC, NULL, 169 },
    { D_WSTAG450_800A68C8, D_WSTAG450_800A68D8, 170 }, { D_WSTAG450_800A68E0, D_WSTAG450_800A68F4, 171 },
    { D_WSTAG450_800A6900, NULL, 172 }, { D_WSTAG450_800A6918, NULL, 173 },
    { D_WSTAG450_800A6934, D_WSTAG450_800A6950, 174 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG450_800A6B7C[2] = { { NULL, NULL, 640 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG450_800A6B94[3] = {
    { D_WSTAG450_800A6958, D_WSTAG450_800A6960, 715 }, { D_WSTAG450_800A696C, NULL, 716 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG450_800A6BB8[8] = {
    { D_WSTAG450_800A6974, NULL, 167 }, { D_WSTAG450_800A697C, NULL, 167 },
    { D_WSTAG450_800A6988, D_WSTAG450_800A6998, 167 }, { D_WSTAG450_800A69A0, D_WSTAG450_800A69B4, 167 },
    { D_WSTAG450_800A69BC, NULL, 167 }, { D_WSTAG450_800A69D4, NULL, 167 },
    { D_WSTAG450_800A69F0, D_WSTAG450_800A6A0C, 167 }, { NULL, NULL, 0 },
};
u16 D_WSTAG450_800A6C18[4] = { 0x204, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C20[8] = { 0x7004, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C30[8] = { 0x6026, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C40[8] = { 0x7009, 1, 0x8192, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C50[8] = { 0x7003, 1, 0x8192, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C60[8] = { 0x8192, 0, 0x701A, 0, 0x7009, 1, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C70[6] = { 0x607, 1, 0x8023, 0, 0xFFFF, 0 };
u16 D_WSTAG450_800A6C7C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG450_800A6C84 = { D_WSTAG450_800A6C18, D_WSTAG450_800A6A14, 33, 4, 1569, 642, 1 };
FieldstgPlacedActor D_WSTAG450_800A6C98 = { D_WSTAG450_800A6C20, D_WSTAG450_800A6A2C, 50, 5, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG450_800A6CAC = { D_WSTAG450_800A6C30, D_WSTAG450_800A6A8C, 50, 5, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG450_800A6CC0 = { D_WSTAG450_800A6C40, D_WSTAG450_800A6AEC, 50, 5, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG450_800A6CD4 = { D_WSTAG450_800A6C50, D_WSTAG450_800A6B1C, 50, 5, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG450_800A6CE8 = { D_WSTAG450_800A6C60, D_WSTAG450_800A6B7C, 50, 5, 1345, 225, 7 };
FieldstgPlacedActor D_WSTAG450_800A6CFC = { D_WSTAG450_800A6C70, D_WSTAG450_800A6B94, 127, 6, 305, 281, 7 };
FieldstgPlacedActor D_WSTAG450_800A6D10 = { D_WSTAG450_800A6C7C, D_WSTAG450_800A6BB8, 157, 7, 1345, 225, 7 };
FieldstgPlacedActor *wstag450_actors[9] = {
    &D_WSTAG450_800A6C84, &D_WSTAG450_800A6C98, &D_WSTAG450_800A6CAC, &D_WSTAG450_800A6CC0, &D_WSTAG450_800A6CD4,
    &D_WSTAG450_800A6CE8, &D_WSTAG450_800A6CFC, &D_WSTAG450_800A6D10, NULL,
};
FieldstgSprite wstag450_sprites[178] = {
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 665, 621, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 1102, 881, 0, 0 },
    { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 1232, 197, 0, 0 }, { 1, 0, 0x40, 2, 3, 1, 3, 8, 8, 0, 1668, 433, 0, 0 },
    { 1, 0, 0x64, 2, 9, 0, 0, 0, 0, 0, 29, 246, 0, 0 }, { 1, 0, 0x64, 2, 0xA, 0, 0, 0, 0, 0, 30, 321, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 391, 857, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 615, 820, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 632, 617, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 664, 966, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 708, 828, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 804, 888, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1037, 765, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1113, 492, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1321, 622, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1392, 440, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1427, 532, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1577, 549, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 1693, 707, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 52, 229, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 158, 422, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 206, 840, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 264, 567, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 408, 861, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 958, 570, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1212, 539, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1312, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1447, 533, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1467, 678, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 727, 829, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 796, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 923, 864, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1546, 718, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1592, 300, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1678, 629, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 450, 189, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 614, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 739, 309, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 849, 430, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 939, 336, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1006, 804, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1099, 261, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1106, 204, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1281, 849, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1351, 965, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1373, 906, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1515, 148, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1638, 263, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 487, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 615, 738, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 650, 168, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 726, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 753, 138, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 824, 182, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 919, 759, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1024, 129, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1070, 887, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 448, 657, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 481, 570, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 496, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 499, 893, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 623, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 654, 771, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 729, 295, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 789, 504, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 806, 100, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 821, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 821, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 900, 723, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 946, 346, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 995, 394, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1042, 140, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1084, 901, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1270, 536, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1295, 946, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1326, 147, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 1435, 1007, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 124, 163, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 268, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 367, 150, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 381, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 491, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 500, 597, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 522, 343, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 584, 290, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 644, 607, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 689, 964, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 742, 662, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 789, 927, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 814, 673, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 822, 887, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 850, 658, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 940, 627, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 942, 864, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1028, 955, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1055, 523, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1055, 764, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1059, 515, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1062, 755, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1128, 198, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1178, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1184, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1281, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1309, 391, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1533, 140, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1568, 715, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1628, 413, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1629, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1641, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 1699, 627, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 61, 371, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 76, 156, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 111, 502, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 177, 675, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 215, 912, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 227, 135, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 270, 479, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 349, 725, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 380, 125, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 472, 371, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 539, 550, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 645, 71, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 660, 485, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 697, 754, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 727, 802, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 741, 442, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 815, 657, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 862, 133, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 870, 300, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 925, 595, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 939, 777, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 954, 913, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1040, 166, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1068, 486, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1138, 820, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1157, 911, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1164, 207, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1171, 545, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1343, 282, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1346, 642, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1381, 970, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1384, 858, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1421, 456, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1428, 127, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1446, 956, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1488, 682, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1609, 121, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1623, 498, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1637, 334, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x4A, 0xA, 0, 1740, 602, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 294, 561, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 298, 104, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 305, 774, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 535, 30, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 634, 887, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 727, 249, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 773, 71, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 849, 883, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 924, 465, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 961, 383, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 977, 93, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1133, 96, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1219, 951, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1270, 589, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1312, 412, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1380, 577, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1455, 655, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1484, 908, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1513, 169, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1615, 540, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1647, 699, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1691, 390, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1737, 761, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 0xA, 0, 1738, 188, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1425, 433, 443, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1041, 394, 430, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 918, 546, 581, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag450_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x233, 0x80, 0x220, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x237, 0x49C, 0x246, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x235, 0xF8, 0xA4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23A, 0x560, 0x440, 1, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E5, 0x240, 0x120, 1, 0, 1, 2 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0x28, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x50, 0xFFD8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0xFFB8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFB0, 0x28, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x50, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag450_funcs = { wstag450_setup };
FieldstgEventDef wstag450_events[3] = {
    { 1279, D_WSTAG450_800A60E4, 0x01350019, NULL, wstag450_event_1279_end },
    { 1280, D_WSTAG450_800A6184, 0x0135001A, NULL, wstag450_event_1280_end }, { -1, NULL, 0, NULL, NULL },
};
