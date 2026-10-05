#include "wstag.h"

/* WSTAG326: stage 0x28B (fieldstg_stages). */

extern WstagFuncs wstag326_funcs;
extern FieldstgBattleLists wstag326_battle_lists;
extern FieldstgVramPlace wstag326_vram_places[];
extern FieldstgPlacedActor *wstag326_actors[];
extern FieldstgSprite wstag326_sprites[];
extern FieldstgMapEvent wstag326_map_events[];
extern FieldstgEventDef wstag326_events[];
void wstag326_update();
void wstag326_sprite_switch_update();

void wstag326_sprite_switch_update(Object *obj) {
    FieldstgSprite *sprite;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->shown = 1;
                break;
            case 2:
                sprite->shown = 1;
                break;
            case 3:
                sprite->shown = 0;
                break;
            case 4:
                sprite->shown = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                sprite->shown = 0;
                break;
            case 2:
                sprite->shown = 0;
                break;
            case 3:
                sprite->shown = 1;
                break;
            case 4:
                sprite->shown = 1;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag326_sprite_switch_create(s32 arg0) {
    return object_create(wstag326_sprite_switch_update, 0x50, 0, arg0);
}

void wstag326_sprite_switch_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x35A) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

void wstag326_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4076, 1) && gamestate_flags.get_flag(0x4077, 0)) {
            data->event = fieldstg_event_start(0x502);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag326_start(void *arg0) {
    WstagObject *obj = object_new(wstag326_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag326_funcs.setup();
    return obj;
}

void wstag326_event_1281_end(void) {
    gamestate_flags.set_flag(0x4076, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag326_event_1282_end(void) {
    gamestate_flags.set_flag(0x4077, 1);
    gamestate_flags.set_flag(0x802A, 1);
}

void wstag326_setup(void) {
    fieldstg_stage.background_file = 0x539;
    fieldstg_stage.sprite_file = 0x053A0000;
    fieldstg_stage.sprites = wstag326_sprites;
    fieldstg_stage.map_events = wstag326_map_events;
    fieldstg_stage.mask_file = 0x538;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF300, 0x17A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag326_vram_places;
    fieldstg_stage.music = 0x2B;
    fieldstg_stage.sound = 0x60AC0000;
    fieldstg_stage.actors = wstag326_actors;
    fieldstg_stage.events = wstag326_events;
    fieldstg_stage.battle_lists = &wstag326_battle_lists;
    fieldstg_attr.set_file(0, 0x053A0001);
    fieldstg_attr.set_file(7, 0x053A0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag326_setup(void);

s16 D_WSTAG326_800A625C[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 397, 270, 5),
    FIELDSTG_EVENT_PLACE(261, 425, 256),
    FIELDSTG_EVENT_ANIM(261, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 261, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 261, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x6004, /* padding, not read */
};
s16 D_WSTAG326_800A62FC[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 397, 270),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(261, 425, 256),
    FIELDSTG_EVENT_ANIM(261, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 261, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 261, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 261, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG326_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A63F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG326_800A6400 = {
    0,
    { &D_WSTAG326_800A63A0, &D_WSTAG326_800A63AC, &D_WSTAG326_800A63B8, &D_WSTAG326_800A63C4, &D_WSTAG326_800A63D0,
        &D_WSTAG326_800A63DC, &D_WSTAG326_800A63E8, &D_WSTAG326_800A63F4 },
};
FieldstgListedBattle D_WSTAG326_800A6424 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6430 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A643C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6448 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6454 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6460 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A646C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6478 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG326_800A6484 = {
    0,
    { &D_WSTAG326_800A6424, &D_WSTAG326_800A6430, &D_WSTAG326_800A643C, &D_WSTAG326_800A6448, &D_WSTAG326_800A6454,
        &D_WSTAG326_800A6460, &D_WSTAG326_800A646C, &D_WSTAG326_800A6478 },
};
FieldstgListedBattle D_WSTAG326_800A64A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A64FC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG326_800A6508 = {
    0,
    { &D_WSTAG326_800A64A8, &D_WSTAG326_800A64B4, &D_WSTAG326_800A64C0, &D_WSTAG326_800A64CC, &D_WSTAG326_800A64D8,
        &D_WSTAG326_800A64E4, &D_WSTAG326_800A64F0, &D_WSTAG326_800A64FC },
};
FieldstgListedBattle D_WSTAG326_800A652C = { 31, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG326_800A6538 = { 321, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG326_800A6544 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6550 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A655C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6568 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6574 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG326_800A6580 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG326_800A658C = {
    0,
    { &D_WSTAG326_800A652C, &D_WSTAG326_800A6538, &D_WSTAG326_800A6544, &D_WSTAG326_800A6550, &D_WSTAG326_800A655C,
        &D_WSTAG326_800A6568, &D_WSTAG326_800A6574, &D_WSTAG326_800A6580 },
};
FieldstgBattleLists wstag326_battle_lists = {
    129, 0, 0, { &D_WSTAG326_800A6400, &D_WSTAG326_800A6484, &D_WSTAG326_800A6508 }, &D_WSTAG326_800A658C,
};
FieldstgVramPlace wstag326_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 256, 0, 0, 320, 507 },
};
u16 D_WSTAG326_800A663C[6] = { 0x6025, 1, 0xA0D, 0, 0xFFFF, 0 };
u16 D_WSTAG326_800A6648[6] = { 0xA0D, 1, 0x9038, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6654[6] = { 0xA0D, 1, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6660[6] = { 0x6026, 1, 0xA0D, 0, 0xFFFF, 0 };
u16 D_WSTAG326_800A666C[6] = { 0xA0D, 1, 0x9038, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6678[6] = { 0xA0D, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6684[4] = { 0xA0D, 0, 0xFFFF, 0 };
u16 D_WSTAG326_800A668C[6] = { 0xA0D, 1, 0x9038, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6698[4] = { 0xA0D, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A66A0[4] = { 0xA0D, 0, 0xFFFF, 0 };
u16 D_WSTAG326_800A66A8[6] = { 0xA0D, 1, 0x9038, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A66B4[4] = { 0xA0D, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG326_800A66BC[5] = {
    { D_WSTAG326_800A663C, D_WSTAG326_800A6648, 427 }, { D_WSTAG326_800A6654, NULL, 177 },
    { D_WSTAG326_800A6660, D_WSTAG326_800A666C, 427 }, { D_WSTAG326_800A6678, NULL, 178 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG326_800A66F8[3] = {
    { D_WSTAG326_800A6684, D_WSTAG326_800A668C, 427 }, { D_WSTAG326_800A6698, NULL, 179 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG326_800A671C[3] = {
    { D_WSTAG326_800A66A0, D_WSTAG326_800A66A8, 427 }, { D_WSTAG326_800A66B4, NULL, 180 }, { NULL, NULL, 0 },
};
u16 D_WSTAG326_800A6740[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6748[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG326_800A6750[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG326_800A6758 = { D_WSTAG326_800A6740, D_WSTAG326_800A66BC, 261, 4, 425, 256, 1 };
FieldstgPlacedActor D_WSTAG326_800A676C = { D_WSTAG326_800A6748, D_WSTAG326_800A66F8, 261, 4, 425, 256, 1 };
FieldstgPlacedActor D_WSTAG326_800A6780 = { D_WSTAG326_800A6750, D_WSTAG326_800A671C, 261, 4, 425, 256, 1 };
FieldstgPlacedActor *wstag326_actors[4] = {
    &D_WSTAG326_800A6758, &D_WSTAG326_800A676C, &D_WSTAG326_800A6780, NULL,
};
FieldstgSprite wstag326_sprites[16] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 374, 301, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 390, 293, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 406, 285, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 422, 277, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 458, 262, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 2, 0, 2, 0x10, 0, 438, 220, 0, 0 },
    { 1, 3, 0x40, 6, 0x33, 2, 0, 1, 0x10, 0, 438, 220, 0, 0 },
    { 1, 4, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 444, 202, 0, 0 }, { 1, 2, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 444, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 294, 261, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 310, 253, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 326, 245, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 342, 237, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 374, 220, 0, 0 }, { 1, 0, 0x40, 6, 0xC, 0, 0, 0, 0, 0, 448, 207, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag326_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28A, 0x3F8, 0x2DC, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag326_funcs = { wstag326_setup };
FieldstgEventDef wstag326_events[3] = {
    { 1281, D_WSTAG326_800A625C, 0x01270020, NULL, wstag326_event_1281_end },
    { 1282, D_WSTAG326_800A62FC, 0x01270021, NULL, wstag326_event_1282_end }, { -1, NULL, 0, NULL, NULL },
};
