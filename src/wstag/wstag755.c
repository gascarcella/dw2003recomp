#include "wstag.h"

/* WSTAG755: stage 0x26E (fieldstg_stages). */

extern WstagFuncs wstag755_funcs;
const CVECTOR wstag755_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag755_battle_lists;
extern FieldstgVramPlace wstag755_vram_places[];
extern FieldstgPlacedActor *wstag755_actors[];
extern FieldstgSprite wstag755_sprites[];
extern FieldstgMapEvent wstag755_map_events[];
extern FieldstgEventDef wstag755_events[];
void wstag755_update();

void wstag755_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_data.progress == 0x1E && gamestate_flags.get_flag(0x4001, 1)) {
            data->event = fieldstg_event_start(0x30D);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag755_start(void *arg0) {
    WstagObject *obj = object_new(wstag755_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag755_funcs.setup();
    return obj;
}

void wstag755_event_780_end(void) {
    gamestate_flags.set_flag(0x4001, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag755_event_781_end(void) {
    gamestate_data.progress = 0x1F;
    gamestate_flags.set_flag(0x800C, 1);
}

void wstag755_setup(void) {
    fieldstg_stage.background_file = 0x6D1;
    fieldstg_stage.sprite_file = 0x06D20000;
    fieldstg_stage.sprites = wstag755_sprites;
    fieldstg_stage.map_events = wstag755_map_events;
    fieldstg_stage.mask_file = 0x6D0;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x19200, 0x34F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag755_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag755_actors;
    fieldstg_stage.color = wstag755_color;
    fieldstg_stage.battle_lists = &wstag755_battle_lists;
    fieldstg_stage.events = wstag755_events;
    fieldstg_attr.set_file(0, 0x06D20001);
    fieldstg_attr.set_file(7, 0x06D20002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag755_setup(void);

s16 D_WSTAG755_800A606C[74] = {
    FIELDSTG_EVENT_WALK(2, 1034, 173, 5),
    FIELDSTG_EVENT_PLACE(142, 1066, 157),
    FIELDSTG_EVENT_ANIM(142, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 142, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 142, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG755_800A6100[115] = {
    FIELDSTG_EVENT_PLACE(2, 1034, 173),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(142, 1066, 157),
    FIELDSTG_EVENT_ANIM(142, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 142, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 142, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 142, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 986, 197, 1),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x26F, 120, 540, 5),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG755_800A61E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A61F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6200 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A620C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6218 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6224 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6230 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A623C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG755_800A6248 = {
    3,
    { &D_WSTAG755_800A61E8, &D_WSTAG755_800A61F4, &D_WSTAG755_800A6200, &D_WSTAG755_800A620C, &D_WSTAG755_800A6218,
        &D_WSTAG755_800A6224, &D_WSTAG755_800A6230, &D_WSTAG755_800A623C },
};
FieldstgListedBattle D_WSTAG755_800A626C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6278 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6284 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6290 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A629C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A62A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A62B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A62C0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG755_800A62CC = {
    0,
    { &D_WSTAG755_800A626C, &D_WSTAG755_800A6278, &D_WSTAG755_800A6284, &D_WSTAG755_800A6290, &D_WSTAG755_800A629C,
        &D_WSTAG755_800A62A8, &D_WSTAG755_800A62B4, &D_WSTAG755_800A62C0 },
};
FieldstgListedBattle D_WSTAG755_800A62F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A62FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6308 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6344 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG755_800A6350 = {
    0,
    { &D_WSTAG755_800A62F0, &D_WSTAG755_800A62FC, &D_WSTAG755_800A6308, &D_WSTAG755_800A6314, &D_WSTAG755_800A6320,
        &D_WSTAG755_800A632C, &D_WSTAG755_800A6338, &D_WSTAG755_800A6344 },
};
FieldstgListedBattle D_WSTAG755_800A6374 = { 18, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG755_800A6380 = { 304, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG755_800A638C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG755_800A63C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG755_800A63D4 = {
    0,
    { &D_WSTAG755_800A6374, &D_WSTAG755_800A6380, &D_WSTAG755_800A638C, &D_WSTAG755_800A6398, &D_WSTAG755_800A63A4,
        &D_WSTAG755_800A63B0, &D_WSTAG755_800A63BC, &D_WSTAG755_800A63C8 },
};
FieldstgBattleLists wstag755_battle_lists = {
    167, 0, 0, { &D_WSTAG755_800A6248, &D_WSTAG755_800A62CC, &D_WSTAG755_800A6350 }, &D_WSTAG755_800A63D4,
};
FieldstgVramPlace wstag755_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 436, 256, 464, 0, 368, 511 }, { 384, 256, 436, 288, 464, 32, 320, 510 },
    { 320, 256, 372, 288, 208, 32, 336, 510 }, { 384, 256, 432, 403, 448, 147, 352, 510 },
};
u16 D_WSTAG755_800A64B4[8] = { 0x6020, 0, 0x6021, 0, 5, 0, 0xFFFF, 0 };
u16 D_WSTAG755_800A64C4[4] = { 5, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A64CC[8] = { 0x6020, 0, 0x6021, 0, 5, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A64DC[4] = { 0x6020, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A64E4[4] = { 0x6021, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG755_800A64EC[2] = { { NULL, NULL, 361 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A6504[2] = { { NULL, NULL, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A651C[2] = { { NULL, NULL, 359 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A6534[2] = { { NULL, NULL, 358 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A654C[2] = { { NULL, NULL, 357 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A6564[2] = { { NULL, NULL, 355 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A657C[2] = { { NULL, NULL, 354 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A6594[2] = { { NULL, NULL, 356 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A65AC[2] = { { NULL, NULL, 353 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A65C4[2] = { { NULL, NULL, 351 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A65DC[2] = { { NULL, NULL, 350 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG755_800A65F4[5] = {
    { D_WSTAG755_800A64B4, D_WSTAG755_800A64C4, 350 }, { D_WSTAG755_800A64CC, NULL, 8 },
    { D_WSTAG755_800A64DC, NULL, 350 }, { D_WSTAG755_800A64E4, NULL, 350 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG755_800A6630[2] = { { NULL, NULL, 352 }, { NULL, NULL, 0 } };
u16 D_WSTAG755_800A6648[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6650[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6658[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6660[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6668[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6670[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6678[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6680[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6688[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6690[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A6698[6] = { 0x7019, 1, 0x7020, 0, 0xFFFF, 0 };
u16 D_WSTAG755_800A66A4[4] = { 0x7020, 1, 0xFFFF, 0 };
u16 D_WSTAG755_800A66AC[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG755_800A66B4 = { D_WSTAG755_800A6648, D_WSTAG755_800A64EC, 64, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG755_800A66C8 = { D_WSTAG755_800A6650, D_WSTAG755_800A6504, 64, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG755_800A66DC = { D_WSTAG755_800A6658, D_WSTAG755_800A651C, 64, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG755_800A66F0 = { D_WSTAG755_800A6660, D_WSTAG755_800A6534, 64, 4, 641, 449, 1 };
FieldstgPlacedActor D_WSTAG755_800A6704 = { D_WSTAG755_800A6668, D_WSTAG755_800A654C, 65, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG755_800A6718 = { D_WSTAG755_800A6670, D_WSTAG755_800A6564, 65, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG755_800A672C = { D_WSTAG755_800A6678, D_WSTAG755_800A657C, 65, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG755_800A6740 = { D_WSTAG755_800A6680, D_WSTAG755_800A6594, 65, 5, 929, 593, 1 };
FieldstgPlacedActor D_WSTAG755_800A6754 = { D_WSTAG755_800A6688, D_WSTAG755_800A65AC, 142, 6, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG755_800A6768 = { D_WSTAG755_800A6690, D_WSTAG755_800A65C4, 142, 6, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG755_800A677C = { D_WSTAG755_800A6698, D_WSTAG755_800A65DC, 142, 6, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG755_800A6790 = { D_WSTAG755_800A66A4, D_WSTAG755_800A65F4, 142, 6, 1066, 157, 1 };
FieldstgPlacedActor D_WSTAG755_800A67A4 = { D_WSTAG755_800A66AC, D_WSTAG755_800A6630, 157, 7, 1066, 157, 1 };
FieldstgPlacedActor *wstag755_actors[14] = {
    &D_WSTAG755_800A66B4, &D_WSTAG755_800A66C8, &D_WSTAG755_800A66DC, &D_WSTAG755_800A66F0, &D_WSTAG755_800A6704,
    &D_WSTAG755_800A6718, &D_WSTAG755_800A672C, &D_WSTAG755_800A6740, &D_WSTAG755_800A6754, &D_WSTAG755_800A6768,
    &D_WSTAG755_800A677C, &D_WSTAG755_800A6790, &D_WSTAG755_800A67A4, NULL,
};
FieldstgSprite wstag755_sprites[22] = {
    { 1, 0, 0x44, 2, 0xC, 0, 0, 0, 0, 0, 1087, 451, 0, 0 }, { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 1022, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 4, 0, 672, 291, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 1, 0x3A, 0x41, 4, 0, 700, 302, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x45, 4, 0, 696, 569, 0, 0 },
    { 1, 0x64, 0x40, 6, 5, 0, 0, 0, 0, 0, 1035, 452, 0, 0 }, { 1, 0x65, 0x40, 6, 6, 0, 0, 0, 0, 0, 843, 356, 0, 0 },
    { 1, 0x66, 0x48, 6, 7, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 0, 0, 0x48, 6, 8, 0, 0, 0, 0, 0, 777, 548, 0, 0 },
    { 0, 0, 0x48, 6, 9, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 0, 0, 0x48, 6, 0xA, 0, 0, 0, 0, 0, 777, 548, 0, 0 },
    { 0, 0, 0x48, 6, 0xB, 0, 0, 0, 0, 0, 777, 548, 0, 0 }, { 1, 0, 0x4F, 4, 0, 0, 0, 0, 0, 0, 821, 545, 623, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1055, 451, 505, 0 }, { 1, 0, 0x41, 4, 2, 0, 0, 0, 0, 0, 864, 348, 412, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 783, 246, 264, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 785, 410, 448, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 715, 589, 612, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 912, 533, 559, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 896, 539, 580, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 908, 539, 574, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag755_map_events[10] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x264, 0x508, 0x6C, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26F, 0x78, 0x21C, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26F, 0x398, 0x3AC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26F, 0x1F0, 0x358, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x328, 0x108, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x318, 0x180, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 7, 0x3D8, 0xC0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 7, 0x3C8, 0x138, 0, 0, 0, 0 }, { 0x601E, 1, 0x4001, 0, 8, 0x30C, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag755_funcs = { wstag755_setup };
FieldstgEventDef wstag755_events[3] = {
    { 780, D_WSTAG755_800A606C, 0x014A0010, NULL, wstag755_event_780_end },
    { 781, D_WSTAG755_800A6100, 0x014A0011, NULL, wstag755_event_781_end }, { -1, NULL, 0, NULL, NULL },
};
