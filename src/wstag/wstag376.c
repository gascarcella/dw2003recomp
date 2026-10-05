#include "wstag.h"

/* WSTAG376: stage 0x295 (fieldstg_stages). */

extern WstagFuncs wstag376_funcs;
extern CVECTOR wstag376_color;
extern FieldstgBattleLists wstag376_battle_lists;
extern FieldstgVramPlace wstag376_vram_places[];
extern FieldstgPlacedActor *wstag376_actors[];
extern FieldstgSprite wstag376_sprites[];
extern FieldstgMapEvent wstag376_map_events[];
extern FieldstgEventDef wstag376_events[];
void wstag376_update();

void wstag376_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4078, 1) && gamestate_flags.get_flag(0x4079, 0)) {
            data->event = fieldstg_event_start(0x504);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag376_start(void *arg0) {
    WstagObject *obj = object_new(wstag376_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag376_funcs.setup();
    return obj;
}

void wstag376_event_1283_end(void) {
    gamestate_flags.set_flag(0x4078, 1);
    gamestate_flags.set_flag(0x7401, 1);
}

void wstag376_event_1284_end(void) {
    gamestate_flags.set_flag(0x4079, 1);
    gamestate_flags.set_flag(0x8AF2, 1);
}

void wstag376_setup(void) {
    fieldstg_stage.background_file = 0x588;
    fieldstg_stage.sprite_file = 0x05890000;
    fieldstg_stage.sprites = wstag376_sprites;
    fieldstg_stage.map_events = wstag376_map_events;
    fieldstg_stage.mask_file = 0x587;
    fieldstg_stage.talk_file = records_language + 0xE1;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11A00, 0x3AD00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag376_vram_places;
    fieldstg_stage.music = 0xB;
    fieldstg_stage.sound = 0x602C0000;
    fieldstg_stage.actors = wstag376_actors;
    fieldstg_stage.color = wstag376_color;
    fieldstg_stage.battle_lists = &wstag376_battle_lists;
    fieldstg_stage.events = wstag376_events;
    fieldstg_attr.set_file(0, 0x05890001);
    fieldstg_attr.set_file(1, 0x05890002);
    fieldstg_attr.set_file(7, 0x05890003);
    fieldstg_attr.set_file(4, 0x05890004);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag376/nonmatchings/wstag376", wstag376_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag376_setup(void);

s16 D_WSTAG376_800A60C8[79] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 1041, 161, 5),
    FIELDSTG_EVENT_PLACE(262, 1073, 145),
    FIELDSTG_EVENT_ANIM(262, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 262, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 262, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG376_800A6168[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 1041, 161),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(262, 1073, 145),
    FIELDSTG_EVENT_ANIM(262, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 262, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 262, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 262, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG376_800A620C = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6218 = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6224 = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6230 = { 96, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A623C = { 97, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6248 = { 97, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6254 = { 97, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A6260 = { 97, 7, 0x60080000 };
FieldstgBattleList D_WSTAG376_800A626C = {
    3,
    { &D_WSTAG376_800A620C, &D_WSTAG376_800A6218, &D_WSTAG376_800A6224, &D_WSTAG376_800A6230, &D_WSTAG376_800A623C,
        &D_WSTAG376_800A6248, &D_WSTAG376_800A6254, &D_WSTAG376_800A6260 },
};
FieldstgListedBattle D_WSTAG376_800A6290 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A629C = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62A8 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62B4 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62C0 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62CC = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62D8 = { 0, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A62E4 = { 0, 7, 0x60080000 };
FieldstgBattleList D_WSTAG376_800A62F0 = {
    0,
    { &D_WSTAG376_800A6290, &D_WSTAG376_800A629C, &D_WSTAG376_800A62A8, &D_WSTAG376_800A62B4, &D_WSTAG376_800A62C0,
        &D_WSTAG376_800A62CC, &D_WSTAG376_800A62D8, &D_WSTAG376_800A62E4 },
};
FieldstgListedBattle D_WSTAG376_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A6344 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A6350 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A635C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A6368 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG376_800A6374 = {
    0,
    { &D_WSTAG376_800A6314, &D_WSTAG376_800A6320, &D_WSTAG376_800A632C, &D_WSTAG376_800A6338, &D_WSTAG376_800A6344,
        &D_WSTAG376_800A6350, &D_WSTAG376_800A635C, &D_WSTAG376_800A6368 },
};
FieldstgListedBattle D_WSTAG376_800A6398 = { 13, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG376_800A63A4 = { 316, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG376_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A63C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A63D4 = { 97, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG376_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG376_800A63EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG376_800A63F8 = {
    0,
    { &D_WSTAG376_800A6398, &D_WSTAG376_800A63A4, &D_WSTAG376_800A63B0, &D_WSTAG376_800A63BC, &D_WSTAG376_800A63C8,
        &D_WSTAG376_800A63D4, &D_WSTAG376_800A63E0, &D_WSTAG376_800A63EC },
};
FieldstgBattleLists wstag376_battle_lists = {
    68, 0, 0, { &D_WSTAG376_800A626C, &D_WSTAG376_800A62F0, &D_WSTAG376_800A6374 }, &D_WSTAG376_800A63F8,
};
FieldstgVramPlace wstag376_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 334, 256, 56, 0, 320, 511 }, { 320, 256, 320, 256, 0, 0, 336, 511 },
};
u16 D_WSTAG376_800A64B8[4] = { 0x868E, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A64C0[6] = { 0x868E, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A64CC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A64D4[8] = { 0x868E, 0, 0, 1, 0x848A, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A64E4[8] = { 0x868E, 0, 0, 1, 0x848A, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A64F4[10] = {
    0x868E, 1, 0x868D, 0, 0x848A, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG376_800A6508[4] = { 0xA07, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A6510[6] = { 0xA07, 1, 0x9039, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A651C[4] = { 0xA07, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6524[4] = { 0xA07, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A652C[6] = { 0xA07, 1, 0x9039, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6538[4] = { 0xA07, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6540[4] = { 0xA07, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A6548[6] = { 0xA07, 1, 0x9039, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6554[4] = { 0xA07, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A655C[4] = { 0xA07, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A6564[6] = { 0xA07, 1, 0x9039, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6570[4] = { 0xA07, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A6578[4] = { 0xA07, 0, 0xFFFF, 0 };
u16 D_WSTAG376_800A6580[6] = { 0xA07, 1, 0x9039, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A658C[4] = { 0xA07, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG376_800A6594[5] = {
    { D_WSTAG376_800A64B8, NULL, 758 }, { D_WSTAG376_800A64C0, D_WSTAG376_800A64CC, 759 },
    { D_WSTAG376_800A64D4, NULL, 760 }, { D_WSTAG376_800A64E4, D_WSTAG376_800A64F4, 761 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG376_800A65D0[3] = {
    { D_WSTAG376_800A6508, D_WSTAG376_800A6510, 722 }, { D_WSTAG376_800A651C, NULL, 73 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG376_800A65F4[3] = {
    { D_WSTAG376_800A6524, D_WSTAG376_800A652C, 722 }, { D_WSTAG376_800A6538, NULL, 76 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG376_800A6618[3] = {
    { D_WSTAG376_800A6540, D_WSTAG376_800A6548, 722 }, { D_WSTAG376_800A6554, NULL, 74 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG376_800A663C[3] = {
    { D_WSTAG376_800A655C, D_WSTAG376_800A6564, 722 }, { D_WSTAG376_800A6570, NULL, 75 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG376_800A6660[3] = {
    { D_WSTAG376_800A6578, D_WSTAG376_800A6580, 722 }, { D_WSTAG376_800A658C, NULL, 77 }, { NULL, NULL, 0 },
};
u16 D_WSTAG376_800A6684[10] = {
    0x7048, 1, 0x7050, 1, 0x868D, 1, 0x868E, 0,
    0xFFFF, 0,
};
u16 D_WSTAG376_800A6698[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A66A0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A66A8[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A66B0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG376_800A66B8[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG376_800A66C0 = { D_WSTAG376_800A6684, D_WSTAG376_800A6594, 28, 4, 97, 242, 7 };
FieldstgPlacedActor D_WSTAG376_800A66D4 = { D_WSTAG376_800A6698, D_WSTAG376_800A65D0, 262, 5, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG376_800A66E8 = { D_WSTAG376_800A66A0, D_WSTAG376_800A65F4, 262, 5, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG376_800A66FC = { D_WSTAG376_800A66A8, D_WSTAG376_800A6618, 262, 5, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG376_800A6710 = { D_WSTAG376_800A66B0, D_WSTAG376_800A663C, 262, 5, 1073, 145, 1 };
FieldstgPlacedActor D_WSTAG376_800A6724 = { D_WSTAG376_800A66B8, D_WSTAG376_800A6660, 262, 5, 1073, 145, 1 };
FieldstgPlacedActor *wstag376_actors[7] = {
    &D_WSTAG376_800A66C0, &D_WSTAG376_800A66D4, &D_WSTAG376_800A66E8, &D_WSTAG376_800A66FC, &D_WSTAG376_800A6710,
    &D_WSTAG376_800A6724, NULL,
};
FieldstgSprite wstag376_sprites[1] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
FieldstgMapEvent wstag376_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x294, 0x510, 0x88, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 5, 8, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 5, 4, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 6, 1, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 6, 0, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 6, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x7E, 0x1AF, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x6D, 0x117, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag376_funcs = { wstag376_setup };
FieldstgEventDef wstag376_events[4] = {
    { 1283, D_WSTAG376_800A60C8, 0x01270022, NULL, wstag376_event_1283_end },
    { 1284, D_WSTAG376_800A6168, 0x01270023, NULL, wstag376_event_1284_end },
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
