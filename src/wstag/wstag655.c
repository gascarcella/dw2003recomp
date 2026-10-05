#include "wstag.h"

/* WSTAG655: stage 0x25C (fieldstg_stages). */

extern WstagFuncs wstag655_funcs;
extern FieldstgBattleLists wstag655_battle_lists;
extern FieldstgVramPlace wstag655_vram_places[];
extern FieldstgPlacedActor *wstag655_actors[];
extern FieldstgSprite wstag655_sprites[];
extern FieldstgMapEvent wstag655_map_events[];
extern FieldstgEventDef wstag655_events[];
void wstag655_update();

void wstag655_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0x10 && gamestate_flags.get_flag(0x403E, 1)) {
            data->event = fieldstg_event_start(0x1AF);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag655_start(void *arg0) {
    WstagObject *obj = object_new(wstag655_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag655_funcs.setup();
    return obj;
}

void wstag655_event_430_end(void) {
    gamestate_flags.set_flag(0x403E, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag655_event_431_end(void) {
    gamestate_data.progress = 0x11;
    gamestate_flags.set_flag(0x800B, 1);
}

void wstag655_setup(void) {
    fieldstg_stage.background_file = 0x496;
    fieldstg_stage.sprite_file = 0x04970000;
    fieldstg_stage.sprites = wstag655_sprites;
    fieldstg_stage.map_events = wstag655_map_events;
    fieldstg_stage.mask_file = 0x498;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x7900, 0xD000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag655_vram_places;
    fieldstg_stage.music = 0x16;
    fieldstg_stage.sound = 0x60580000;
    fieldstg_stage.actors = wstag655_actors;
    fieldstg_stage.events = wstag655_events;
    fieldstg_stage.battle_lists = &wstag655_battle_lists;
    fieldstg_attr.set_file(0, 0x04970001);
    fieldstg_attr.set_file(7, 0x04970002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag655_setup(void);

s16 D_WSTAG655_800A6080[128] = {
    FIELDSTG_EVENT_WALK(2, 387, 119, 5),
    FIELDSTG_EVENT_PLACE(122, 419, 103),
    FIELDSTG_EVENT_ANIM(122, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 122),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 122),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG655_800A6180[169] = {
    FIELDSTG_EVENT_PLACE(2, 387, 119),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(122, 419, 103),
    FIELDSTG_EVENT_ANIM(122, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 122, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 259, 183, 1),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x25B, 294, 150, 1),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG655_800A62D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6328 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG655_800A6334 = {
    3,
    { &D_WSTAG655_800A62D4, &D_WSTAG655_800A62E0, &D_WSTAG655_800A62EC, &D_WSTAG655_800A62F8, &D_WSTAG655_800A6304,
        &D_WSTAG655_800A6310, &D_WSTAG655_800A631C, &D_WSTAG655_800A6328 },
};
FieldstgListedBattle D_WSTAG655_800A6358 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A63AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG655_800A63B8 = {
    0,
    { &D_WSTAG655_800A6358, &D_WSTAG655_800A6364, &D_WSTAG655_800A6370, &D_WSTAG655_800A637C, &D_WSTAG655_800A6388,
        &D_WSTAG655_800A6394, &D_WSTAG655_800A63A0, &D_WSTAG655_800A63AC },
};
FieldstgListedBattle D_WSTAG655_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A63F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6400 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A640C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6418 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6424 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6430 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG655_800A643C = {
    0,
    { &D_WSTAG655_800A63DC, &D_WSTAG655_800A63E8, &D_WSTAG655_800A63F4, &D_WSTAG655_800A6400, &D_WSTAG655_800A640C,
        &D_WSTAG655_800A6418, &D_WSTAG655_800A6424, &D_WSTAG655_800A6430 },
};
FieldstgListedBattle D_WSTAG655_800A6460 = { 6, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG655_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6478 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6484 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A6490 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A649C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG655_800A64B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG655_800A64C0 = {
    0,
    { &D_WSTAG655_800A6460, &D_WSTAG655_800A646C, &D_WSTAG655_800A6478, &D_WSTAG655_800A6484, &D_WSTAG655_800A6490,
        &D_WSTAG655_800A649C, &D_WSTAG655_800A64A8, &D_WSTAG655_800A64B4 },
};
FieldstgBattleLists wstag655_battle_lists = {
    166, 0, 0, { &D_WSTAG655_800A6334, &D_WSTAG655_800A63B8, &D_WSTAG655_800A643C }, &D_WSTAG655_800A64C0,
};
FieldstgVramPlace wstag655_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 364, 256, 176, 0, 336, 511 },
};
u16 D_WSTAG655_800A6570[4] = { 0x800B, 0, 0xFFFF, 0 };
u16 D_WSTAG655_800A6578[4] = { 0x9044, 1, 0xFFFF, 0 };
u16 D_WSTAG655_800A6580[4] = { 0x800B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG655_800A6588[2] = { { NULL, NULL, 25 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG655_800A65A0[2] = { { NULL, NULL, 354 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG655_800A65B8[2] = { { NULL, NULL, 356 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG655_800A65D0[3] = {
    { D_WSTAG655_800A6570, D_WSTAG655_800A6578, 355 }, { D_WSTAG655_800A6580, NULL, 776 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG655_800A65F4[2] = { { NULL, NULL, 354 }, { NULL, NULL, 0 } };
u16 D_WSTAG655_800A660C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG655_800A6614[4] = { 0x6011, 1, 0xFFFF, 0 };
u16 D_WSTAG655_800A661C[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG655_800A6624[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_WSTAG655_800A662C[4] = { 0x6012, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG655_800A6634 = { D_WSTAG655_800A660C, D_WSTAG655_800A6588, 122, 4, 419, 103, 1 };
FieldstgPlacedActor D_WSTAG655_800A6648 = { D_WSTAG655_800A6614, D_WSTAG655_800A65A0, 122, 4, 419, 103, 1 };
FieldstgPlacedActor D_WSTAG655_800A665C = { D_WSTAG655_800A661C, D_WSTAG655_800A65B8, 122, 4, 419, 103, 1 };
FieldstgPlacedActor D_WSTAG655_800A6670 = { D_WSTAG655_800A6624, D_WSTAG655_800A65D0, 122, 4, 419, 103, 1 };
FieldstgPlacedActor D_WSTAG655_800A6684 = { D_WSTAG655_800A662C, D_WSTAG655_800A65F4, 122, 4, 419, 103, 1 };
FieldstgPlacedActor *wstag655_actors[6] = {
    &D_WSTAG655_800A6634, &D_WSTAG655_800A6648, &D_WSTAG655_800A665C, &D_WSTAG655_800A6670, &D_WSTAG655_800A6684,
    NULL,
};
FieldstgSprite wstag655_sprites[2] = {
    { 1, 0, 0xFF, 6, 0x32, 2, 0, 2, 0xC, 0, 318, 266, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag655_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25B, 0x126, 0x96, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xEE, 0x138, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0xDC, 0x1A0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag655_funcs = { wstag655_setup };
FieldstgEventDef wstag655_events[3] = {
    { 430, D_WSTAG655_800A6080, 0x01430011, NULL, wstag655_event_430_end },
    { 431, D_WSTAG655_800A6180, 0x01430012, NULL, wstag655_event_431_end }, { -1, NULL, 0, NULL, NULL },
};
