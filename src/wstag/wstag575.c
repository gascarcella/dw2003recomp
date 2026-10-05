#include "wstag.h"

/* WSTAG575: stage 0x24C (fieldstg_stages). */

extern WstagFuncs wstag575_funcs;
extern FieldstgBattleLists wstag575_battle_lists;
extern FieldstgVramPlace wstag575_vram_places[];
extern FieldstgPlacedActor *wstag575_actors[];
extern FieldstgSprite wstag575_sprites[];
extern FieldstgMapEvent wstag575_map_events[];
extern FieldstgEventDef wstag575_events[];
void wstag575_update();

void wstag575_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0x10 && gamestate_flags.get_flag(0x40A2, 0)) {
            data->event = fieldstg_event_start(0x1A6);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag575_start(void *arg0) {
    WstagObject *obj = object_new(wstag575_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag575_funcs.setup();
    return obj;
}

void wstag575_event_422_end(void) {
    gamestate_flags.set_flag(0x40A2, 1);
}

void wstag575_setup(void) {
    fieldstg_stage.background_file = 0x480;
    fieldstg_stage.sprite_file = 0x04810000;
    fieldstg_stage.sprites = wstag575_sprites;
    fieldstg_stage.map_events = wstag575_map_events;
    fieldstg_stage.mask_file = 0x47F;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15C00, 0xF800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag575_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag575_actors;
    fieldstg_stage.events = wstag575_events;
    fieldstg_stage.battle_lists = &wstag575_battle_lists;
    fieldstg_attr.set_file(0, 0x04810001);
    fieldstg_attr.set_file(7, 0x04810002);
    fieldstg_attr.set_file(4, 0x04810003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag575_setup(void);

s16 D_WSTAG575_800A6014[202] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 210, 497),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_PLACE(105, 114, 449),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 170, 477, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_PLACE(2, 145, 464),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(105, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 210, 497, 7),
    FIELDSTG_EVENT_WALK(105, 210, 457, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WALK(105, 175, 475, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 105, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 105, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 105, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 105, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(105, 159, 467, 7),
    FIELDSTG_EVENT_WAIT_WALK(105),
    FIELDSTG_EVENT_ANIM(105, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
    0x6004, /* padding, not read */
};
FieldstgListedBattle D_WSTAG575_800A61A8 = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61B4 = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61C0 = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61CC = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61D8 = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61E4 = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61F0 = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A61FC = { 70, 3, 0x60080000 };
FieldstgBattleList D_WSTAG575_800A6208 = {
    3,
    { &D_WSTAG575_800A61A8, &D_WSTAG575_800A61B4, &D_WSTAG575_800A61C0, &D_WSTAG575_800A61CC, &D_WSTAG575_800A61D8,
        &D_WSTAG575_800A61E4, &D_WSTAG575_800A61F0, &D_WSTAG575_800A61FC },
};
FieldstgListedBattle D_WSTAG575_800A622C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6238 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6244 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6250 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A625C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6268 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6280 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG575_800A628C = {
    0,
    { &D_WSTAG575_800A622C, &D_WSTAG575_800A6238, &D_WSTAG575_800A6244, &D_WSTAG575_800A6250, &D_WSTAG575_800A625C,
        &D_WSTAG575_800A6268, &D_WSTAG575_800A6274, &D_WSTAG575_800A6280 },
};
FieldstgListedBattle D_WSTAG575_800A62B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6304 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG575_800A6310 = {
    0,
    { &D_WSTAG575_800A62B0, &D_WSTAG575_800A62BC, &D_WSTAG575_800A62C8, &D_WSTAG575_800A62D4, &D_WSTAG575_800A62E0,
        &D_WSTAG575_800A62EC, &D_WSTAG575_800A62F8, &D_WSTAG575_800A6304 },
};
FieldstgListedBattle D_WSTAG575_800A6334 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6340 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A634C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6358 = { 329, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A6364 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A6370 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG575_800A637C = { 156, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG575_800A6388 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG575_800A6394 = {
    0,
    { &D_WSTAG575_800A6334, &D_WSTAG575_800A6340, &D_WSTAG575_800A634C, &D_WSTAG575_800A6358, &D_WSTAG575_800A6364,
        &D_WSTAG575_800A6370, &D_WSTAG575_800A637C, &D_WSTAG575_800A6388 },
};
FieldstgBattleLists wstag575_battle_lists = {
    33, 0, 0, { &D_WSTAG575_800A6208, &D_WSTAG575_800A628C, &D_WSTAG575_800A6310 }, &D_WSTAG575_800A6394,
};
FieldstgVramPlace wstag575_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 280, 200, 24, 336, 511 },
};
FieldstgTalk D_WSTAG575_800A6444[2] = { { NULL, NULL, 784 }, { NULL, NULL, 0 } };
u16 D_WSTAG575_800A645C[4] = { 0x6010, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG575_800A6464 = { D_WSTAG575_800A645C, D_WSTAG575_800A6444, 105, 4, 0, 0, 0 };
FieldstgPlacedActor *wstag575_actors[2] = { &D_WSTAG575_800A6464, NULL };
FieldstgSprite wstag575_sprites[41] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 166, 265, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 186, 261, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 186, 277, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 195, 287, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 207, 278, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 208, 273, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 217, 281, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 223, 291, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 237, 301, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 244, 290, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 9, 4, 0, 264, 305, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 532, 182, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 551, 182, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 9, 4, 0, 553, 172, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 496, 200, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 520, 201, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 9, 4, 0, 526, 192, 0, 0 }, { 1, 0, 0x40, 2, 6, 1, 6, 0xD, 4, 0, 490, 26, 0, 0 },
    { 1, 0, 0x40, 2, 6, 1, 6, 0xD, 4, 0, 528, 45, 0, 0 }, { 1, 0, 0x40, 2, 6, 1, 6, 0xD, 4, 0, 567, 64, 0, 0 },
    { 1, 0, 0x64, 2, 0xE, 0, 0, 0, 0, 0, 698, 491, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 418, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 421, 128, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 434, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 9, 4, 0, 450, 120, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 384, 141, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 391, 147, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 392, 125, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 9, 4, 0, 414, 137, 0, 0 }, { 1, 0, 0x73, 4, 0, 0, 0, 0, 0, 0, 349, 733, 824, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 444, 766, 777, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 310, 576, 599, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 487, 596, 610, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 592, 118, 160, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 103, 411, 477, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 336, 535, 535, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 384, 559, 559, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 576, 895, 895, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 636, 800, 800, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 673, 767, 767, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag575_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24A, 0x5D8, 0x104, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x88, 0x3F4, 5, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x10, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 8, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag575_funcs = { wstag575_setup };
FieldstgEventDef wstag575_events[2] = {
    { 422, D_WSTAG575_800A6014, 0x013C0005, NULL, wstag575_event_422_end }, { -1, NULL, 0, NULL, NULL },
};
