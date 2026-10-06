#include "wstag.h"

/* WSTAG421: stage 0x29C (fieldstg_stages). */

extern WstagFuncs wstag421_funcs;
extern FieldstgBattleLists wstag421_battle_lists;
extern FieldstgVramPlace wstag421_vram_places[];
extern FieldstgPlacedActor *wstag421_actors[];
extern FieldstgSprite wstag421_sprites[];
extern FieldstgMapEvent wstag421_map_events[];
extern FieldstgEventDef wstag421_events[];
void wstag421_update();

void wstag421_update(WstagObject *obj, WstagEventData *data) {
    FieldstgSprite *sprite;
    s32 anim;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        anim = gamestate_flags.get_flag(0x1A0A, 1) != 0;
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                sprite->anim_mode = anim;
            }
        }
        if (gamestate_data.progress == 0x1B) {
            if (gamestate_flags.get_flag(0x40A3, 1) && gamestate_flags.get_flag(0x40A7, 0)) {
                data->event = fieldstg_event_start(0x2E5);
            } else if (gamestate_flags.get_flag(0x40A7, 1)) {
                data->event = fieldstg_event_start(0x2E7);
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag421_start(void *arg0) {
    WstagObject *obj = object_new(wstag421_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag421_funcs.setup();
    return obj;
}

void wstag421_event_740_end(void) {
    gamestate_flags.set_flag(0x40A3, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag421_event_741_end(void) {
    gamestate_flags.set_flag(0x8016, 1);
    gamestate_flags.set_flag(0x40A7, 1);
}

void wstag421_event_743_end(void) {
    gamestate_data.progress = 0x1C;
}

void wstag421_setup(void) {
    fieldstg_stage.background_file = 0x786;
    fieldstg_stage.sprite_file = 0x07870000;
    fieldstg_stage.sprites = wstag421_sprites;
    fieldstg_stage.map_events = wstag421_map_events;
    fieldstg_stage.mask_file = 0x785;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1D100, 0x1F600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag421_vram_places;
    fieldstg_stage.music = 0xC;
    fieldstg_stage.sound = 0x60300000;
    fieldstg_stage.actors = wstag421_actors;
    fieldstg_stage.battle_lists = &wstag421_battle_lists;
    fieldstg_stage.events = wstag421_events;
    fieldstg_attr.set_file(0, 0x07870001);
    fieldstg_attr.set_file(7, 0x07870002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag421_setup(void);

s16 D_WSTAG421_800A616C[82] = {
    FIELDSTG_EVENT_WALK(2, 384, 218, 1),
    FIELDSTG_EVENT_PLACE(210, 352, 234),
    FIELDSTG_EVENT_ANIM(210, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 210, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 210, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 210, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG421_800A6210[93] = {
    FIELDSTG_EVENT_PLACE(2, 384, 218),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(316, 352, 234),
    FIELDSTG_EVENT_ANIM(316, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 360, 230, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x2D7, 1, 1, 0),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG421_800A62CC[35] = {
    FIELDSTG_EVENT_PLACE(2, 384, 218),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 488, 164, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x29E, 200, 420, 5),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG421_800A6314 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6320 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A632C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6338 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6344 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6350 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A635C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6368 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG421_800A6374 = {
    0,
    { &D_WSTAG421_800A6314, &D_WSTAG421_800A6320, &D_WSTAG421_800A632C, &D_WSTAG421_800A6338, &D_WSTAG421_800A6344,
        &D_WSTAG421_800A6350, &D_WSTAG421_800A635C, &D_WSTAG421_800A6368 },
};
FieldstgListedBattle D_WSTAG421_800A6398 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A63EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG421_800A63F8 = {
    0,
    { &D_WSTAG421_800A6398, &D_WSTAG421_800A63A4, &D_WSTAG421_800A63B0, &D_WSTAG421_800A63BC, &D_WSTAG421_800A63C8,
        &D_WSTAG421_800A63D4, &D_WSTAG421_800A63E0, &D_WSTAG421_800A63EC },
};
FieldstgListedBattle D_WSTAG421_800A641C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6428 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6434 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6440 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A644C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6458 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6464 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A6470 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG421_800A647C = {
    0,
    { &D_WSTAG421_800A641C, &D_WSTAG421_800A6428, &D_WSTAG421_800A6434, &D_WSTAG421_800A6440, &D_WSTAG421_800A644C,
        &D_WSTAG421_800A6458, &D_WSTAG421_800A6464, &D_WSTAG421_800A6470 },
};
FieldstgListedBattle D_WSTAG421_800A64A0 = { 14, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG421_800A64AC = { 305, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG421_800A64B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A64C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A64D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A64DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A64E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG421_800A64F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG421_800A6500 = {
    0,
    { &D_WSTAG421_800A64A0, &D_WSTAG421_800A64AC, &D_WSTAG421_800A64B8, &D_WSTAG421_800A64C4, &D_WSTAG421_800A64D0,
        &D_WSTAG421_800A64DC, &D_WSTAG421_800A64E8, &D_WSTAG421_800A64F4 },
};
FieldstgBattleLists wstag421_battle_lists = {
    150, 0, 0, { &D_WSTAG421_800A6374, &D_WSTAG421_800A63F8, &D_WSTAG421_800A647C }, &D_WSTAG421_800A6500,
};
FieldstgVramPlace wstag421_vram_places[26] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 384, 443, 256, 187, 320, 511 }, { 384, 256, 392, 443, 288, 187, 336, 511 },
    { 448, 256, 478, 256, 632, 0, 352, 511 }, { 384, 256, 426, 443, 424, 187, 368, 511 },
    { 384, 256, 434, 443, 456, 187, 320, 510 }, { 384, 256, 400, 463, 320, 207, 336, 510 },
    { 384, 256, 408, 463, 352, 207, 352, 510 }, { 448, 256, 486, 256, 664, 0, 368, 510 },
    { 448, 256, 494, 256, 696, 0, 320, 509 }, { 448, 256, 448, 256, 512, 0, 336, 509 },
    { 448, 256, 454, 256, 536, 0, 352, 509 }, { 448, 256, 462, 256, 568, 0, 368, 509 },
    { 448, 256, 470, 256, 600, 0, 320, 508 }, { 448, 256, 502, 256, 728, 0, 336, 508 },
    { 448, 256, 478, 288, 632, 32, 352, 508 }, { 448, 256, 486, 288, 664, 32, 368, 508 },
    { 448, 256, 494, 288, 696, 32, 320, 507 }, { 448, 256, 502, 288, 728, 32, 336, 507 },
    { 384, 256, 416, 428, 384, 172, 352, 507 }, { 384, 256, 432, 302, 448, 46, 368, 507 },
};
u16 D_WSTAG421_800A66E0[4] = { 0x601B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A66E8[4] = { 0x701F, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG421_800A66F0[2] = { { NULL, NULL, 269 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6708[2] = { { NULL, NULL, 271 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6720[2] = { { NULL, NULL, 267 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6738[2] = { { NULL, NULL, 264 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6750[2] = { { NULL, NULL, 277 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6768[2] = { { NULL, NULL, 273 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6780[2] = { { NULL, NULL, 276 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6798[2] = { { NULL, NULL, 261 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A67B0[2] = { { NULL, NULL, 275 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A67C8[2] = { { NULL, NULL, 278 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A67E0[2] = { { NULL, NULL, 463 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A67F8[3] = {
    { D_WSTAG421_800A66E0, NULL, 464 }, { D_WSTAG421_800A66E8, NULL, 465 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG421_800A681C[2] = { { NULL, NULL, 274 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6834[2] = { { NULL, NULL, 260 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A684C[2] = { { NULL, NULL, 270 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6864[2] = { { NULL, NULL, 272 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A687C[2] = { { NULL, NULL, 266 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A6894[2] = { { NULL, NULL, 263 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A68AC[2] = { { NULL, NULL, 262 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A68C4[2] = { { NULL, NULL, 268 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A68DC[2] = { { NULL, NULL, 265 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG421_800A68F4[2] = { { NULL, NULL, 259 }, { NULL, NULL, 0 } };
u16 D_WSTAG421_800A690C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6914[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A691C[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6928[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6934[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A693C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6944[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A694C[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6958[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6960[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A696C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6974[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A697C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6984[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG421_800A6990[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A6998[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69A0[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69AC[6] = { 0x1A0A, 0, 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69B8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69C0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69C8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG421_800A69D0[4] = { 0x40A3, 0, 0xFFFF, 0 };
u16 D_WSTAG421_800A69D8[4] = { 0x601B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG421_800A69E0 = { D_WSTAG421_800A690C, D_WSTAG421_800A66F0, 37, 4, 449, 481, 1 };
FieldstgPlacedActor D_WSTAG421_800A69F4 = { D_WSTAG421_800A6914, D_WSTAG421_800A6708, 38, 5, 833, 257, 7 };
FieldstgPlacedActor D_WSTAG421_800A6A08 = { D_WSTAG421_800A691C, D_WSTAG421_800A6720, 46, 6, 632, 406, 1 };
FieldstgPlacedActor D_WSTAG421_800A6A1C = { D_WSTAG421_800A6928, D_WSTAG421_800A6738, 49, 7, 914, 344, 1 };
FieldstgPlacedActor D_WSTAG421_800A6A30 = { D_WSTAG421_800A6934, D_WSTAG421_800A6750, 50, 8, 449, 481, 1 };
FieldstgPlacedActor D_WSTAG421_800A6A44 = { D_WSTAG421_800A693C, D_WSTAG421_800A6768, 52, 9, 833, 257, 7 };
FieldstgPlacedActor D_WSTAG421_800A6A58 = { D_WSTAG421_800A6944, D_WSTAG421_800A6780, 56, 10, 882, 585, 3 };
FieldstgPlacedActor D_WSTAG421_800A6A6C = { D_WSTAG421_800A694C, D_WSTAG421_800A6798, 57, 11, 882, 585, 3 };
FieldstgPlacedActor D_WSTAG421_800A6A80 = { D_WSTAG421_800A6958, D_WSTAG421_800A67B0, 58, 12, 914, 344, 1 };
FieldstgPlacedActor D_WSTAG421_800A6A94 = { D_WSTAG421_800A6960, D_WSTAG421_800A67C8, 102, 13, 553, 461, 1 };
FieldstgPlacedActor D_WSTAG421_800A6AA8 = { D_WSTAG421_800A696C, D_WSTAG421_800A67E0, 115, 14, 449, 481, 1 };
FieldstgPlacedActor D_WSTAG421_800A6ABC = { D_WSTAG421_800A6974, D_WSTAG421_800A67F8, 116, 15, 833, 257, 7 };
FieldstgPlacedActor D_WSTAG421_800A6AD0 = { D_WSTAG421_800A697C, D_WSTAG421_800A681C, 143, 16, 352, 234, 1 };
FieldstgPlacedActor D_WSTAG421_800A6AE4 = { D_WSTAG421_800A6984, D_WSTAG421_800A6834, 157, 17, 882, 585, 3 };
FieldstgPlacedActor D_WSTAG421_800A6AF8 = { D_WSTAG421_800A6990, D_WSTAG421_800A684C, 157, 17, 449, 481, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B0C = { D_WSTAG421_800A6998, D_WSTAG421_800A6864, 158, 18, 833, 257, 7 };
FieldstgPlacedActor D_WSTAG421_800A6B20 = { D_WSTAG421_800A69A0, D_WSTAG421_800A687C, 158, 18, 632, 406, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B34 = { D_WSTAG421_800A69AC, D_WSTAG421_800A6894, 159, 19, 914, 344, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B48 = { D_WSTAG421_800A69B8, D_WSTAG421_800A68AC, 159, 19, 882, 585, 3 };
FieldstgPlacedActor D_WSTAG421_800A6B5C = { D_WSTAG421_800A69C0, D_WSTAG421_800A68C4, 160, 20, 632, 406, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B70 = { D_WSTAG421_800A69C8, D_WSTAG421_800A68DC, 161, 21, 914, 344, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B84 = { D_WSTAG421_800A69D0, D_WSTAG421_800A68F4, 210, 22, 352, 234, 1 };
FieldstgPlacedActor D_WSTAG421_800A6B98 = { D_WSTAG421_800A69D8, NULL, 316, 23, 0, 0, 1 };
FieldstgPlacedActor *wstag421_actors[24] = {
    &D_WSTAG421_800A69E0, &D_WSTAG421_800A69F4, &D_WSTAG421_800A6A08, &D_WSTAG421_800A6A1C, &D_WSTAG421_800A6A30,
    &D_WSTAG421_800A6A44, &D_WSTAG421_800A6A58, &D_WSTAG421_800A6A6C, &D_WSTAG421_800A6A80, &D_WSTAG421_800A6A94,
    &D_WSTAG421_800A6AA8, &D_WSTAG421_800A6ABC, &D_WSTAG421_800A6AD0, &D_WSTAG421_800A6AE4, &D_WSTAG421_800A6AF8,
    &D_WSTAG421_800A6B0C, &D_WSTAG421_800A6B20, &D_WSTAG421_800A6B34, &D_WSTAG421_800A6B48, &D_WSTAG421_800A6B5C,
    &D_WSTAG421_800A6B70, &D_WSTAG421_800A6B84, &D_WSTAG421_800A6B98, NULL,
};
FieldstgSprite wstag421_sprites[36] = {
    { 1, 1, 0x4A, 2, 0, 1, 0, 3, 6, 0, 441, 290, 0, 0 }, { 1, 1, 0x4A, 2, 0, 1, 0, 3, 6, 0, 817, 114, 0, 0 },
    { 1, 1, 0x4A, 2, 0, 1, 0, 3, 6, 0, 949, 528, 0, 0 }, { 1, 1, 0x40, 2, 4, 1, 4, 7, 4, 0, 366, 274, 0, 0 },
    { 1, 1, 0x40, 2, 4, 1, 4, 7, 4, 0, 446, 241, 0, 0 }, { 1, 1, 0x40, 2, 4, 1, 4, 7, 4, 0, 1129, 333, 0, 0 },
    { 1, 0, 0x80, 2, 0x24, 0, 0, 0, 0, 0, 469, 191, 0, 0 }, { 1, 0, 0x80, 2, 0x25, 0, 0, 0, 0, 0, 256, 161, 0, 0 },
    { 1, 1, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 411, 424, 0, 0 }, { 1, 1, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 538, 379, 0, 0 },
    { 1, 1, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 568, 341, 0, 0 }, { 1, 1, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 607, 347, 0, 0 },
    { 1, 1, 0x40, 6, 8, 1, 8, 0xB, 4, 0, 976, 215, 0, 0 },
    { 1, 1, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 544, 363, 0, 0 },
    { 1, 1, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 579, 380, 0, 0 },
    { 1, 1, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 884, 243, 0, 0 },
    { 1, 1, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 913, 244, 0, 0 },
    { 1, 1, 0x40, 6, 0xC, 1, 0xC, 0xF, 4, 0, 1020, 218, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x1B, 0, 0, 0, 0, 0, 721, 200, 0, 0 },
    { 1, 0x64, 0x40, 6, 0x1C, 0, 0, 0, 0, 0, 451, 367, 0, 0 },
    { 1, 0, 0x78, 6, 0x26, 0, 0, 0, 0, 0, 721, 200, 0, 0 }, { 1, 0, 0x80, 6, 0x27, 0, 0, 0, 0, 0, 451, 367, 0, 0 },
    { 1, 1, 0x40, 4, 8, 1, 8, 0xB, 4, 0, 448, 405, 445, 0 },
    { 1, 1, 0x40, 4, 0xC, 1, 0xC, 0xF, 4, 0, 465, 438, 464, 0 },
    { 1, 1, 0x40, 4, 0xC, 1, 0xC, 0xF, 4, 0, 737, 256, 281, 0 },
    { 1, 0, 0x64, 4, 0x1D, 0, 0, 0, 0, 0, 784, 496, 560, 0 },
    { 1, 0, 0x64, 4, 0x1E, 0, 0, 0, 0, 0, 768, 488, 552, 0 },
    { 1, 0, 0x64, 4, 0x1F, 0, 0, 0, 0, 0, 752, 480, 544, 0 },
    { 1, 0, 0x64, 4, 0x20, 0, 0, 0, 0, 0, 736, 472, 536, 0 },
    { 1, 0, 0x64, 4, 0x21, 0, 0, 0, 0, 0, 720, 464, 528, 0 },
    { 1, 0, 0x64, 4, 0x22, 0, 0, 0, 0, 0, 704, 456, 520, 0 },
    { 1, 0, 0x64, 4, 0x23, 0, 0, 0, 0, 0, 672, 456, 496, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 898, 552, 576, 0 },
    { 1, 0, 0x40, 4, 0x19, 0, 0, 0, 0, 0, 727, 258, 280, 0 },
    { 1, 0, 0x40, 4, 0x1A, 0, 0, 0, 0, 0, 447, 408, 444, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag421_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x298, 0x448, 0xF8, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29E, 0xC8, 0x1A4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29E, 0x1B8, 0x204, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29D, 0x208, 0x1AC, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x29F, 0x1A7, 0x254, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x400, 0x120, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x410, 0x188, 0, 0, 0, 0 }, { 0x40A3, 0, 0xFFFF, 0, 8, 0x2E4, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag421_funcs = { wstag421_setup };
FieldstgEventDef wstag421_events[4] = {
    { 740, D_WSTAG421_800A616C, 0x01350029, NULL, wstag421_event_740_end },
    { 741, D_WSTAG421_800A6210, 0x0135002A, NULL, wstag421_event_741_end },
    { 743, D_WSTAG421_800A62CC, 0x0135002B, NULL, wstag421_event_743_end }, { -1, NULL, 0, NULL, NULL },
};
