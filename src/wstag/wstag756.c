#include "wstag.h"

/* WSTAG756: stage 0x2D5 (fieldstg_stages). */

extern WstagFuncs wstag756_funcs;
const CVECTOR wstag756_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag756_battle_lists;
extern FieldstgVramPlace wstag756_vram_places[];
extern FieldstgPlacedActor *wstag756_actors[];
extern FieldstgSprite wstag756_sprites[];
extern FieldstgMapEvent wstag756_map_events[];
extern FieldstgEventDef wstag756_events[];
void wstag756_update();

void wstag756_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_data.progress == 0xD && gamestate_flags.get_flag(0x1C0D, 1)) {
            data->event = fieldstg_event_start(0x160);
        } else if (gamestate_data.progress == 0x23 && gamestate_flags.get_flag(0x405B, 1)) {
            data->event = fieldstg_event_start(0x38F);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag756_start(void *arg0) {
    WstagObject *obj = object_new(wstag756_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag756_funcs.setup();
    return obj;
}

void wstag756_event_910_end(void) {
    gamestate_flags.set_flag(0x405B, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag756_event_911_end(void) {
    gamestate_data.progress = 0x24;
    gamestate_flags.set_flag(0x8019, 1);
}

void wstag756_setup(void) {
    fieldstg_stage.background_file = 0x6D4;
    fieldstg_stage.sprite_file = 0x06D50000;
    fieldstg_stage.sprites = wstag756_sprites;
    fieldstg_stage.map_events = wstag756_map_events;
    fieldstg_stage.mask_file = 0x6D3;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1B000, 0x34100 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag756_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag756_actors;
    fieldstg_stage.color = wstag756_color;
    fieldstg_stage.battle_lists = &wstag756_battle_lists;
    fieldstg_stage.events = wstag756_events;
    fieldstg_attr.set_file(0, 0x06D50001);
    fieldstg_attr.set_file(7, 0x06D50002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag756_setup(void);

s16 D_WSTAG756_800A60A0[78] = {
    FIELDSTG_EVENT_WALK(2, 1034, 173, 5),
    FIELDSTG_EVENT_PLACE(210, 1066, 157),
    FIELDSTG_EVENT_ANIM(210, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG756_800A613C[78] = {
    FIELDSTG_EVENT_PLACE(2, 1034, 173),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(316, 1066, 157),
    FIELDSTG_EVENT_ANIM(316, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 1058, 161, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 986, 197, 1),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x2D6, 120, 540, 5),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG756_800A61D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A61E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A61F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A61FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6208 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6214 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6220 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A622C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG756_800A6238 = {
    0,
    { &D_WSTAG756_800A61D8, &D_WSTAG756_800A61E4, &D_WSTAG756_800A61F0, &D_WSTAG756_800A61FC, &D_WSTAG756_800A6208,
        &D_WSTAG756_800A6214, &D_WSTAG756_800A6220, &D_WSTAG756_800A622C },
};
FieldstgListedBattle D_WSTAG756_800A625C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6268 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A628C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A62A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A62B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG756_800A62BC = {
    0,
    { &D_WSTAG756_800A625C, &D_WSTAG756_800A6268, &D_WSTAG756_800A6274, &D_WSTAG756_800A6280, &D_WSTAG756_800A628C,
        &D_WSTAG756_800A6298, &D_WSTAG756_800A62A4, &D_WSTAG756_800A62B0 },
};
FieldstgListedBattle D_WSTAG756_800A62E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A62EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6328 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6334 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG756_800A6340 = {
    0,
    { &D_WSTAG756_800A62E0, &D_WSTAG756_800A62EC, &D_WSTAG756_800A62F8, &D_WSTAG756_800A6304, &D_WSTAG756_800A6310,
        &D_WSTAG756_800A631C, &D_WSTAG756_800A6328, &D_WSTAG756_800A6334 },
};
FieldstgListedBattle D_WSTAG756_800A6364 = { 26, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG756_800A6370 = { 308, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG756_800A637C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6388 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A63AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG756_800A63B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG756_800A63C4 = {
    0,
    { &D_WSTAG756_800A6364, &D_WSTAG756_800A6370, &D_WSTAG756_800A637C, &D_WSTAG756_800A6388, &D_WSTAG756_800A6394,
        &D_WSTAG756_800A63A0, &D_WSTAG756_800A63AC, &D_WSTAG756_800A63B8 },
};
FieldstgBattleLists wstag756_battle_lists = {
    153, 0, 0, { &D_WSTAG756_800A6238, &D_WSTAG756_800A62BC, &D_WSTAG756_800A6340 }, &D_WSTAG756_800A63C4,
};
FieldstgVramPlace wstag756_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 424, 451, 416, 195, 320, 510 }, { 384, 256, 384, 463, 256, 207, 336, 510 },
    { 384, 256, 436, 256, 464, 0, 368, 510 }, { 384, 256, 400, 431, 320, 175, 336, 509 },
    { 320, 256, 372, 256, 208, 0, 352, 509 }, { 384, 256, 408, 431, 352, 175, 368, 509 },
    { 384, 256, 432, 443, 448, 187, 320, 508 }, { 384, 256, 416, 451, 384, 195, 336, 508 },
    { 320, 256, 364, 419, 176, 163, 352, 508 },
};
u16 D_WSTAG756_800A64F4[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A64FC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A6504[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A650C[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A6514[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A651C[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG756_800A6524[4] = {
    { D_WSTAG756_800A64F4, NULL, 474 }, { D_WSTAG756_800A64FC, NULL, 475 }, { D_WSTAG756_800A6504, NULL, 476 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG756_800A6554[2] = { { NULL, NULL, 477 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A656C[2] = { { NULL, NULL, 478 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A6584[4] = {
    { D_WSTAG756_800A650C, NULL, 479 }, { D_WSTAG756_800A6514, NULL, 480 }, { D_WSTAG756_800A651C, NULL, 481 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG756_800A65B4[2] = { { NULL, NULL, 482 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A65CC[2] = { { NULL, NULL, 483 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A65E4[2] = { { NULL, NULL, 489 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A65FC[2] = { { NULL, NULL, 488 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A6614[2] = { { NULL, NULL, 487 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A662C[2] = { { NULL, NULL, 484 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A6644[2] = { { NULL, NULL, 484 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A665C[2] = { { NULL, NULL, 485 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A6674[2] = { { NULL, NULL, 485 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A668C[2] = { { NULL, NULL, 486 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG756_800A66A4[2] = { { NULL, NULL, 486 }, { NULL, NULL, 0 } };
u16 D_WSTAG756_800A66BC[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66C4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66CC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66D4[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66DC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66E4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66EC[4] = { 0x6023, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66F4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A66FC[8] = { 0x405B, 0, 0x6024, 0, 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A670C[6] = { 0x701E, 1, 0x7021, 0, 0xFFFF, 0 };
u16 D_WSTAG756_800A6718[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A6720[6] = { 0x701E, 1, 0x7021, 0, 0xFFFF, 0 };
u16 D_WSTAG756_800A672C[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A6734[6] = { 0x701E, 1, 0x7021, 0, 0xFFFF, 0 };
u16 D_WSTAG756_800A6740[4] = { 0x6022, 1, 0xFFFF, 0 };
u16 D_WSTAG756_800A6748[4] = { 0x6023, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG756_800A6750 = { D_WSTAG756_800A66BC, D_WSTAG756_800A6524, 34, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG756_800A6764 = { D_WSTAG756_800A66C4, D_WSTAG756_800A6554, 34, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG756_800A6778 = { D_WSTAG756_800A66CC, D_WSTAG756_800A656C, 34, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG756_800A678C = { D_WSTAG756_800A66D4, D_WSTAG756_800A6584, 35, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG756_800A67A0 = { D_WSTAG756_800A66DC, D_WSTAG756_800A65B4, 35, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG756_800A67B4 = { D_WSTAG756_800A66E4, D_WSTAG756_800A65CC, 35, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG756_800A67C8 = { D_WSTAG756_800A66EC, D_WSTAG756_800A65E4, 116, 6, 559, 776, 1 };
FieldstgPlacedActor D_WSTAG756_800A67DC = { D_WSTAG756_800A66F4, D_WSTAG756_800A65FC, 146, 7, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG756_800A67F0 = { D_WSTAG756_800A66FC, D_WSTAG756_800A6614, 210, 8, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG756_800A6804 = { D_WSTAG756_800A670C, D_WSTAG756_800A662C, 299, 9, 520, 757, 1 };
FieldstgPlacedActor D_WSTAG756_800A6818 = { D_WSTAG756_800A6718, D_WSTAG756_800A6644, 299, 9, 520, 757, 1 };
FieldstgPlacedActor D_WSTAG756_800A682C = { D_WSTAG756_800A6720, D_WSTAG756_800A665C, 303, 10, 544, 769, 1 };
FieldstgPlacedActor D_WSTAG756_800A6840 = { D_WSTAG756_800A672C, D_WSTAG756_800A6674, 303, 10, 544, 769, 1 };
FieldstgPlacedActor D_WSTAG756_800A6854 = { D_WSTAG756_800A6734, D_WSTAG756_800A668C, 304, 11, 568, 781, 1 };
FieldstgPlacedActor D_WSTAG756_800A6868 = { D_WSTAG756_800A6740, D_WSTAG756_800A66A4, 304, 11, 568, 781, 1 };
FieldstgPlacedActor D_WSTAG756_800A687C = { D_WSTAG756_800A6748, NULL, 316, 12, 0, 0, 1 };
FieldstgPlacedActor *wstag756_actors[17] = {
    &D_WSTAG756_800A6750, &D_WSTAG756_800A6764, &D_WSTAG756_800A6778, &D_WSTAG756_800A678C, &D_WSTAG756_800A67A0,
    &D_WSTAG756_800A67B4, &D_WSTAG756_800A67C8, &D_WSTAG756_800A67DC, &D_WSTAG756_800A67F0, &D_WSTAG756_800A6804,
    &D_WSTAG756_800A6818, &D_WSTAG756_800A682C, &D_WSTAG756_800A6840, &D_WSTAG756_800A6854, &D_WSTAG756_800A6868,
    &D_WSTAG756_800A687C, NULL,
};
FieldstgSprite wstag756_sprites[22] = {
    { 1, 0, 0x40, 2, 0xD, 0, 0, 0, 0, 0, 1087, 451, 0, 0 }, { 1, 0, 0x40, 2, 0x11, 0, 0, 0, 0, 0, 1022, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 4, 0, 672, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x41, 4, 0, 700, 302, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x45, 4, 0, 696, 569, 0, 0 },
    { 1, 0x64, 0x40, 6, 6, 0, 0, 0, 0, 0, 1035, 452, 0, 0 }, { 1, 0x65, 0x40, 6, 7, 0, 0, 0, 0, 0, 843, 356, 0, 0 },
    { 1, 0x66, 0x40, 6, 8, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 0, 0, 0x40, 6, 9, 0, 0, 0, 0, 0, 777, 548, 0, 0 },
    { 0, 0, 0x40, 6, 0xA, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 0, 0, 0x40, 6, 0xB, 0, 0, 0, 0, 0, 777, 548, 0, 0 },
    { 0, 0, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 821, 545, 623, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1055, 451, 505, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 864, 348, 412, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 783, 246, 264, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 785, 410, 448, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 715, 589, 612, 0 }, { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 912, 533, 559, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 896, 539, 580, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 908, 539, 574, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag756_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CC, 0x508, 0x6C, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D6, 0x78, 0x21C, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D6, 0x398, 0x3AC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D6, 0x1F0, 0x358, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x328, 0x108, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x318, 0x180, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x3D8, 0xC0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x3C8, 0x138, 0, 0, 0, 0 }, { 0x6023, 1, 0x405B, 0, 8, 0x38E, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag756_funcs = { wstag756_setup };
FieldstgEventDef wstag756_events[3] = {
    { 910, D_WSTAG756_800A60A0, 0x014A0012, NULL, wstag756_event_910_end },
    { 911, D_WSTAG756_800A613C, 0x014A0013, NULL, wstag756_event_911_end }, { -1, NULL, 0, NULL, NULL },
};
