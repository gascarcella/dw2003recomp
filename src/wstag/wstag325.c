#include "wstag.h"

/* WSTAG325: stage 0x21C (fieldstg_stages). */

extern WstagFuncs wstag325_funcs;
extern FieldstgBattleLists wstag325_battle_lists;
extern FieldstgVramPlace wstag325_vram_places[];
extern FieldstgPlacedActor *wstag325_actors[];
extern FieldstgSprite wstag325_sprites[];
extern FieldstgMapEvent wstag325_map_events[];
extern FieldstgEventDef wstag325_events[];
void wstag325_update(WstagObject *obj, WstagObjEventData *data);
void wstag325_sprite_switch_update(Object *obj);

void wstag325_sprite_switch_update(Object *obj) {
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

Object *wstag325_sprite_switch_create(s32 arg0) {
    return object_create(wstag325_sprite_switch_update, sizeof(Object), 0, arg0);
}

void wstag325_sprite_switch_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x35A) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

void wstag325_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4008, 1) && gamestate_flags.get_flag(0x4009, 0)) {
            data->event = fieldstg_event_start(0x178);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag325_start(void *arg0) {
    WstagObject *obj = object_new(wstag325_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag325_funcs.setup();
    return obj;
}

void wstag325_event_375_end(void) {
    gamestate_flags.set_flag(0x4008, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag325_event_376_end(void) {
    gamestate_flags.set_flag(0x4009, 1);
    gamestate_flags.set_flag(0x8674, 1);
}

void wstag325_event_560_end(void) {
    gamestate_data.progress = 0x16;
}

void wstag325_setup(void) {
    fieldstg_stage.background_file = 0x764;
    fieldstg_stage.sprite_file = 0x07650000;
    fieldstg_stage.sprites = wstag325_sprites;
    fieldstg_stage.map_events = wstag325_map_events;
    fieldstg_stage.mask_file = 0x763;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF500, 0x17A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag325_vram_places;
    fieldstg_stage.music = 0x2B;
    fieldstg_stage.sound = 0x60AC0000;
    fieldstg_stage.actors = wstag325_actors;
    fieldstg_stage.events = wstag325_events;
    fieldstg_stage.battle_lists = &wstag325_battle_lists;
    fieldstg_attr.set_file(0, 0x07650001);
    fieldstg_attr.set_file(7, 0x07650002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag325_setup(void);

s16 D_WSTAG325_800A626C[84] = {
    FIELDSTG_EVENT_WALK(2, 401, 276, 5),
    FIELDSTG_EVENT_PLACE(165, 433, 260),
    FIELDSTG_EVENT_ANIM(165, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 165, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 165),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 165, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG325_800A6314[71] = {
    FIELDSTG_EVENT_PLACE(2, 401, 276),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(165, 433, 260),
    FIELDSTG_EVENT_ANIM(165, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 165, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 165, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG325_800A63A4[78] = {
    FIELDSTG_EVENT_WALK(2, 401, 276, 5),
    FIELDSTG_EVENT_PLACE(165, 433, 260),
    FIELDSTG_EVENT_ANIM(165, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 165),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 165),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 165, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_WALK(2, 340, 306, 1),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_GOTO_MAP(0x21B, 1016, 732, 1),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG325_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6470 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A647C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6488 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6494 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG325_800A64A0 = {
    0,
    { &D_WSTAG325_800A6440, &D_WSTAG325_800A644C, &D_WSTAG325_800A6458, &D_WSTAG325_800A6464, &D_WSTAG325_800A6470,
        &D_WSTAG325_800A647C, &D_WSTAG325_800A6488, &D_WSTAG325_800A6494 },
};
FieldstgListedBattle D_WSTAG325_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A64F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6500 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A650C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6518 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG325_800A6524 = {
    0,
    { &D_WSTAG325_800A64C4, &D_WSTAG325_800A64D0, &D_WSTAG325_800A64DC, &D_WSTAG325_800A64E8, &D_WSTAG325_800A64F4,
        &D_WSTAG325_800A6500, &D_WSTAG325_800A650C, &D_WSTAG325_800A6518 },
};
FieldstgListedBattle D_WSTAG325_800A6548 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6554 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6560 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A656C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6578 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6584 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6590 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A659C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG325_800A65A8 = {
    0,
    { &D_WSTAG325_800A6548, &D_WSTAG325_800A6554, &D_WSTAG325_800A6560, &D_WSTAG325_800A656C, &D_WSTAG325_800A6578,
        &D_WSTAG325_800A6584, &D_WSTAG325_800A6590, &D_WSTAG325_800A659C },
};
FieldstgListedBattle D_WSTAG325_800A65CC = { 9, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG325_800A65D8 = { 313, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG325_800A65E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A65F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A65FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6608 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6614 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG325_800A6620 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG325_800A662C = {
    0,
    { &D_WSTAG325_800A65CC, &D_WSTAG325_800A65D8, &D_WSTAG325_800A65E4, &D_WSTAG325_800A65F0, &D_WSTAG325_800A65FC,
        &D_WSTAG325_800A6608, &D_WSTAG325_800A6614, &D_WSTAG325_800A6620 },
};
FieldstgBattleLists wstag325_battle_lists = {
    127, 0, 0, { &D_WSTAG325_800A64A0, &D_WSTAG325_800A6524, &D_WSTAG325_800A65A8 }, &D_WSTAG325_800A662C,
};
FieldstgVramPlace wstag325_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 256, 160, 0, 320, 494 },
};
u16 D_WSTAG325_800A66DC[6] = { 0x9041, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A66E8[4] = { 0x1C1B, 0, 0xFFFF, 0 };
u16 D_WSTAG325_800A66F0[4] = { 0x1C1B, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A66F8[4] = { 0x1C1B, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6700[4] = { 0x8191, 0, 0xFFFF, 0 };
u16 D_WSTAG325_800A6708[4] = { 0x8191, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6710[8] = { 0x9024, 1, 0x1C1D, 1, 0x4005, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG325_800A6720[2] = { { NULL, NULL, 1175 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A6738[2] = { { NULL, D_WSTAG325_800A66DC, 179 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A6750[2] = { { NULL, NULL, 1046 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A6768[3] = {
    { D_WSTAG325_800A66E8, D_WSTAG325_800A66F0, 1176 }, { D_WSTAG325_800A66F8, NULL, 1177 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG325_800A678C[3] = {
    { D_WSTAG325_800A6700, NULL, 1040 }, { D_WSTAG325_800A6708, D_WSTAG325_800A6710, 180 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG325_800A67B0[2] = { { NULL, NULL, 1041 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A67C8[2] = { { NULL, NULL, 1042 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A67E0[2] = { { NULL, NULL, 1043 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A67F8[2] = { { NULL, NULL, 1044 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG325_800A6810[2] = { { NULL, NULL, 1045 }, { NULL, NULL, 0 } };
u16 D_WSTAG325_800A6828[6] = { 0x7016, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6834[4] = { 0xA03, 0, 0xFFFF, 0 };
u16 D_WSTAG325_800A683C[6] = { 0x602B, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6848[6] = { 0xA03, 1, 0x6014, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6854[6] = { 0x6015, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6860[6] = { 0xA03, 1, 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A686C[6] = { 0x7018, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6878[6] = { 0xA03, 1, 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6884[6] = { 0x6026, 1, 0xA03, 1, 0xFFFF, 0 };
u16 D_WSTAG325_800A6890[6] = { 0xA03, 1, 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG325_800A689C = { D_WSTAG325_800A6828, D_WSTAG325_800A6720, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A68B0 = { D_WSTAG325_800A6834, D_WSTAG325_800A6738, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A68C4 = { D_WSTAG325_800A683C, D_WSTAG325_800A6750, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A68D8 = { D_WSTAG325_800A6848, D_WSTAG325_800A6768, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A68EC = { D_WSTAG325_800A6854, D_WSTAG325_800A678C, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A6900 = { D_WSTAG325_800A6860, D_WSTAG325_800A67B0, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A6914 = { D_WSTAG325_800A686C, D_WSTAG325_800A67C8, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A6928 = { D_WSTAG325_800A6878, D_WSTAG325_800A67E0, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A693C = { D_WSTAG325_800A6884, D_WSTAG325_800A67F8, 165, 4, 433, 260, 5 };
FieldstgPlacedActor D_WSTAG325_800A6950 = { D_WSTAG325_800A6890, D_WSTAG325_800A6810, 165, 4, 433, 260, 5 };
FieldstgPlacedActor *wstag325_actors[11] = {
    &D_WSTAG325_800A689C, &D_WSTAG325_800A68B0, &D_WSTAG325_800A68C4, &D_WSTAG325_800A68D8, &D_WSTAG325_800A68EC,
    &D_WSTAG325_800A6900, &D_WSTAG325_800A6914, &D_WSTAG325_800A6928, &D_WSTAG325_800A693C, &D_WSTAG325_800A6950,
    NULL,
};
FieldstgSprite wstag325_sprites[22] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 374, 301, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 390, 293, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 406, 285, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 422, 277, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 4, 0, 458, 262, 0, 0 }, { 1, 0, 0x49, 6, 0, 1, 0, 3, 4, 0, 84, 193, 0, 0 },
    { 1, 0, 0x5D, 6, 4, 1, 4, 7, 4, 0, 68, 252, 0, 0 }, { 1, 0, 0x5D, 6, 5, 1, 5, 8, 4, 0, 131, 227, 0, 0 },
    { 1, 0, 0x5D, 6, 0xB, 1, 0xB, 0xE, 4, 0, 182, 153, 0, 0 }, { 1, 0, 0x49, 6, 0, 1, 0, 3, 4, 0, 227, 178, 0, 0 },
    { 1, 0, 0x5D, 6, 7, 1, 7, 0xA, 4, 0, 272, 108, 0, 0 },
    { 1, 1, 0x40, 6, 0x32, 2, 0, 2, 0x10, 0, 438, 220, 0, 0 },
    { 1, 3, 0x40, 6, 0x33, 2, 0, 1, 0x10, 0, 438, 220, 0, 0 },
    { 1, 2, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 444, 202, 0, 0 }, { 1, 4, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 444, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 294, 261, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 310, 253, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 326, 245, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 342, 237, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 374, 220, 0, 0 }, { 1, 0, 0x40, 6, 0xF, 0, 0, 0, 0, 0, 448, 207, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag325_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21B, 0x3F8, 0x2DC, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag325_funcs = { wstag325_setup };
FieldstgEventDef wstag325_events[4] = {
    { 375, D_WSTAG325_800A626C, 0x01270004, NULL, wstag325_event_375_end },
    { 376, D_WSTAG325_800A6314, 0x01270005, NULL, wstag325_event_376_end },
    { 560, D_WSTAG325_800A63A4, 0x01270006, NULL, wstag325_event_560_end }, { -1, NULL, 0, NULL, NULL },
};
