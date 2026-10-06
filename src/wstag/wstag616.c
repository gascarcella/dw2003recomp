#include "wstag.h"

/* WSTAG616: stage 0x2BE (fieldstg_stages). */

extern WstagFuncs wstag616_funcs;
const CVECTOR wstag616_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag616_battle_lists;
extern FieldstgVramPlace wstag616_vram_places[];
extern FieldstgPlacedActor *wstag616_actors[];
extern FieldstgSprite wstag616_sprites[];
extern FieldstgMapEvent wstag616_map_events[];
extern FieldstgEventDef wstag616_events[];
void wstag616_update();

void wstag616_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4080, 1) && gamestate_flags.get_flag(0x4081, 0)) {
            data->event = fieldstg_event_start(0x50C);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag616_start(void *arg0) {
    WstagObject *obj = object_new(wstag616_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag616_funcs.setup();
    return obj;
}

void wstag616_event_1291_end(void) {
    gamestate_flags.set_flag(0x4080, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag616_event_1292_end(void) {
    gamestate_flags.set_flag(0x4081, 1);
    gamestate_flags.set_flag(0x8F42, 1);
}

void wstag616_setup(void) {
    fieldstg_stage.background_file = 0x622;
    fieldstg_stage.sprite_file = 0x06230000;
    fieldstg_stage.sprites = wstag616_sprites;
    fieldstg_stage.map_events = wstag616_map_events;
    fieldstg_stage.mask_file = 0x621;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xB700, 0x2B200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag616_vram_places;
    fieldstg_stage.music = 0x3A;
    fieldstg_stage.sound = 0x60E80000;
    fieldstg_stage.actors = wstag616_actors;
    fieldstg_stage.color = wstag616_color;
    fieldstg_stage.battle_lists = &wstag616_battle_lists;
    fieldstg_stage.events = wstag616_events;
    fieldstg_attr.set_file(0, 0x06230001);
    fieldstg_attr.set_file(7, 0x06230002);
    fieldstg_attr.set_file(4, 0x06230003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag616_setup(void);

s16 D_WSTAG616_800A60AC[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 656, 560, 3),
    FIELDSTG_EVENT_PLACE(266, 624, 544),
    FIELDSTG_EVENT_ANIM(266, 1, 7),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 266, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 266, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x325, /* padding, not read */
};
s16 D_WSTAG616_800A614C[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 656, 560),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(266, 624, 544),
    FIELDSTG_EVENT_ANIM(266, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 266, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 266, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 266, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG616_800A61F0 = { 83, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A61FC = { 83, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A6208 = { 83, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A6214 = { 83, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A6220 = { 84, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A622C = { 84, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A6238 = { 84, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG616_800A6244 = { 84, 12, 0x60080000 };
FieldstgBattleList D_WSTAG616_800A6250 = {
    3,
    { &D_WSTAG616_800A61F0, &D_WSTAG616_800A61FC, &D_WSTAG616_800A6208, &D_WSTAG616_800A6214, &D_WSTAG616_800A6220,
        &D_WSTAG616_800A622C, &D_WSTAG616_800A6238, &D_WSTAG616_800A6244 },
};
FieldstgListedBattle D_WSTAG616_800A6274 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6280 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A628C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6298 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A62A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A62B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A62BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A62C8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG616_800A62D4 = {
    0,
    { &D_WSTAG616_800A6274, &D_WSTAG616_800A6280, &D_WSTAG616_800A628C, &D_WSTAG616_800A6298, &D_WSTAG616_800A62A4,
        &D_WSTAG616_800A62B0, &D_WSTAG616_800A62BC, &D_WSTAG616_800A62C8 },
};
FieldstgListedBattle D_WSTAG616_800A62F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6304 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6310 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A631C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6328 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6334 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A6340 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A634C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG616_800A6358 = {
    0,
    { &D_WSTAG616_800A62F8, &D_WSTAG616_800A6304, &D_WSTAG616_800A6310, &D_WSTAG616_800A631C, &D_WSTAG616_800A6328,
        &D_WSTAG616_800A6334, &D_WSTAG616_800A6340, &D_WSTAG616_800A634C },
};
FieldstgListedBattle D_WSTAG616_800A637C = { 24, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG616_800A6388 = { 320, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG616_800A6394 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A63A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A63AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A63B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A63C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG616_800A63D0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG616_800A63DC = {
    0,
    { &D_WSTAG616_800A637C, &D_WSTAG616_800A6388, &D_WSTAG616_800A6394, &D_WSTAG616_800A63A0, &D_WSTAG616_800A63AC,
        &D_WSTAG616_800A63B8, &D_WSTAG616_800A63C4, &D_WSTAG616_800A63D0 },
};
FieldstgBattleLists wstag616_battle_lists = {
    113, 0, 0, { &D_WSTAG616_800A6250, &D_WSTAG616_800A62D4, &D_WSTAG616_800A6358 }, &D_WSTAG616_800A63DC,
};
FieldstgVramPlace wstag616_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 342, 375, 88, 119, 368, 510 }, { 320, 256, 352, 368, 128, 112, 320, 509 },
};
u16 D_WSTAG616_800A649C[4] = { 0x869A, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A64A4[6] = { 0x869A, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A64B0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A64B8[8] = { 0x869A, 0, 0, 1, 0x8496, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A64C8[8] = { 0x869A, 0, 0, 1, 0x8496, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A64D8[10] = {
    0x869A, 1, 0x8699, 0, 0x8496, 0, 0x7013, 1,
    0xFFFF, 0,
};
u16 D_WSTAG616_800A64EC[4] = { 0xA0C, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A64F4[6] = { 0xA0C, 1, 0x903D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6500[4] = { 0xA0C, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6508[4] = { 0xA0C, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A6510[6] = { 0xA0C, 1, 0x903D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A651C[4] = { 0xA0C, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6524[4] = { 0xA0C, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A652C[6] = { 0xA0C, 1, 0x903D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6538[4] = { 0xA0C, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6540[4] = { 0xA0C, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A6548[6] = { 0xA0C, 1, 0x903D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6554[4] = { 0xA0C, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A655C[4] = { 0xA0C, 0, 0xFFFF, 0 };
u16 D_WSTAG616_800A6564[6] = { 0xA0C, 1, 0x903D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6570[4] = { 0xA0C, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG616_800A6578[5] = {
    { D_WSTAG616_800A649C, NULL, 746 }, { D_WSTAG616_800A64A4, D_WSTAG616_800A64B0, 747 },
    { D_WSTAG616_800A64B8, NULL, 748 }, { D_WSTAG616_800A64C8, D_WSTAG616_800A64D8, 749 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG616_800A65B4[3] = {
    { D_WSTAG616_800A64EC, D_WSTAG616_800A64F4, 726 }, { D_WSTAG616_800A6500, NULL, 281 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG616_800A65D8[3] = {
    { D_WSTAG616_800A6508, D_WSTAG616_800A6510, 726 }, { D_WSTAG616_800A651C, NULL, 285 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG616_800A65FC[3] = {
    { D_WSTAG616_800A6524, D_WSTAG616_800A652C, 726 }, { D_WSTAG616_800A6538, NULL, 284 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG616_800A6620[3] = {
    { D_WSTAG616_800A6540, D_WSTAG616_800A6548, 726 }, { D_WSTAG616_800A6554, NULL, 283 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG616_800A6644[3] = {
    { D_WSTAG616_800A655C, D_WSTAG616_800A6564, 726 }, { D_WSTAG616_800A6570, NULL, 282 }, { NULL, NULL, 0 },
};
u16 D_WSTAG616_800A6668[10] = {
    0x7047, 1, 0x704F, 1, 0x8699, 1, 0x869A, 0,
    0xFFFF, 0,
};
u16 D_WSTAG616_800A667C[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6684[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A668C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A6694[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG616_800A669C[4] = { 0x6025, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG616_800A66A4 = { D_WSTAG616_800A6668, D_WSTAG616_800A6578, 169, 4, 464, 320, 1 };
FieldstgPlacedActor D_WSTAG616_800A66B8 = { D_WSTAG616_800A667C, D_WSTAG616_800A65B4, 266, 5, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG616_800A66CC = { D_WSTAG616_800A6684, D_WSTAG616_800A65D8, 266, 5, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG616_800A66E0 = { D_WSTAG616_800A668C, D_WSTAG616_800A65FC, 266, 5, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG616_800A66F4 = { D_WSTAG616_800A6694, D_WSTAG616_800A6620, 266, 5, 624, 544, 7 };
FieldstgPlacedActor D_WSTAG616_800A6708 = { D_WSTAG616_800A669C, D_WSTAG616_800A6644, 266, 5, 624, 544, 7 };
FieldstgPlacedActor *wstag616_actors[7] = {
    &D_WSTAG616_800A66A4, &D_WSTAG616_800A66B8, &D_WSTAG616_800A66CC, &D_WSTAG616_800A66E0, &D_WSTAG616_800A66F4,
    &D_WSTAG616_800A6708, NULL,
};
FieldstgSprite wstag616_sprites[46] = {
    { 1, 0, 0x40, 2, 0x56, 2, 0, 1, 4, 0, 660, 140, 0, 0 }, { 1, 0, 0x40, 6, 2, 0, 0, 0, 0, 0, 512, 816, 0, 0 },
    { 1, 0, 0x44, 6, 3, 0, 0, 0, 0, 0, 298, 816, 0, 0 }, { 1, 0, 0x48, 6, 4, 0, 0, 0, 0, 0, 143, 793, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 1, 4, 0, 395, 684, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 1, 4, 0, 552, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 1, 4, 0, 96, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 196, 576, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 236, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 436, 252, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 532, 204, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 724, 108, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 1, 4, 0, 789, 575, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 476, 568, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 652, 624, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 684, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 836, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 908, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 1, 4, 0, 988, 664, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x41, 0xA, 0, 227, 467, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x41, 0xA, 0, 676, 506, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x41, 0xA, 0, 838, 140, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x4C, 0xA, 0, 410, 672, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x4C, 0xA, 0, 517, 306, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x4C, 0xA, 0, 686, 382, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x4C, 0xA, 0, 756, 723, 0, 0 },
    { 1, 0x64, 0x40, 6, 0, 0, 0, 0, 0, 0, 687, 136, 0, 0 }, { 1, 0, 0x78, 6, 0x1D, 0, 0, 0, 0, 0, 810, 120, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2C, 1, 0x2C, 0x2E, 0x14, 0, 691, 754, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 681, 794, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 705, 816, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 853, 676, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x58, 1, 0x58, 0x5B, 0x14, 0, 949, 526, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x60, 1, 0x60, 0x62, 0x14, 0, 80, 601, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x60, 1, 0x60, 0x62, 0x14, 0, 80, 702, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 0x14, 0, 295, 495, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 0x14, 0, 298, 606, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 92, 757, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 190, 431, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 329, 641, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 483, 287, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 806, 127, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 872, 90, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x5C, 1, 0x5C, 0x5F, 0x14, 0, 922, 390, 0, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 646, 144, 191, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag616_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BD, 0x90, 0x240, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BF, 0x268, 0x1AC, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xB3, 0x238, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0xC3, 0x2A0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag616_funcs = { wstag616_setup };
FieldstgEventDef wstag616_events[3] = {
    { 1291, D_WSTAG616_800A60AC, 0x0143000A, NULL, wstag616_event_1291_end },
    { 1292, D_WSTAG616_800A614C, 0x0143000B, NULL, wstag616_event_1292_end }, { -1, NULL, 0, NULL, NULL },
};
