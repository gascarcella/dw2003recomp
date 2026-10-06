#include "wstag.h"

/* WSTAG676: stage 0x2C7 (fieldstg_stages). */

extern WstagFuncs wstag676_funcs;
extern FieldstgBattleLists wstag676_battle_lists;
extern FieldstgVramPlace wstag676_vram_places[];
extern FieldstgPlacedActor *wstag676_actors[];
extern FieldstgSprite wstag676_sprites[];
extern FieldstgMapEvent wstag676_map_events[];
extern FieldstgEventDef wstag676_events[];
void wstag676_update();

void wstag676_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_data.progress == 0x22 && gamestate_flags.get_flag(0x405A, 1)) {
            data->event = fieldstg_event_start(0x385);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag676_start(void *arg0) {
    WstagObject *obj = object_new(wstag676_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag676_funcs.setup();
    return obj;
}

void wstag676_event_900_end(void) {
    gamestate_flags.set_flag(0x405A, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag676_event_901_end(void) {
    gamestate_data.progress = 0x23;
    gamestate_flags.set_flag(0x8018, 1);
}

void wstag676_setup(void) {
    fieldstg_stage.background_file = 0x661;
    fieldstg_stage.sprite_file = 0x06620000;
    fieldstg_stage.sprites = wstag676_sprites;
    fieldstg_stage.map_events = wstag676_map_events;
    fieldstg_stage.mask_file = 0x660;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x14E00, 0x1E900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag676_vram_places;
    fieldstg_stage.music = 0x17;
    fieldstg_stage.sound = 0x605C0000;
    fieldstg_stage.actors = wstag676_actors;
    fieldstg_stage.battle_lists = &wstag676_battle_lists;
    fieldstg_stage.events = wstag676_events;
    fieldstg_attr.set_file(0, 0x06620001);
    fieldstg_attr.set_file(7, 0x06620002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag676_setup(void);

s16 D_WSTAG676_800A6094[78] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 352, 169, 3),
    FIELDSTG_EVENT_PLACE(210, 320, 153),
    FIELDSTG_EVENT_ANIM(210, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 210, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 210, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x3, /* padding, not read */
};
s16 D_WSTAG676_800A6130[70] = {
    FIELDSTG_EVENT_PLACE(2, 352, 169),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(316, 320, 153),
    FIELDSTG_EVENT_ANIM(316, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 328, 157, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 416, 201, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x2C6, 1132, 170, 1),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG676_800A61BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A61EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A61F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6210 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG676_800A621C = {
    0,
    { &D_WSTAG676_800A61BC, &D_WSTAG676_800A61C8, &D_WSTAG676_800A61D4, &D_WSTAG676_800A61E0, &D_WSTAG676_800A61EC,
        &D_WSTAG676_800A61F8, &D_WSTAG676_800A6204, &D_WSTAG676_800A6210 },
};
FieldstgListedBattle D_WSTAG676_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6288 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6294 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG676_800A62A0 = {
    0,
    { &D_WSTAG676_800A6240, &D_WSTAG676_800A624C, &D_WSTAG676_800A6258, &D_WSTAG676_800A6264, &D_WSTAG676_800A6270,
        &D_WSTAG676_800A627C, &D_WSTAG676_800A6288, &D_WSTAG676_800A6294 },
};
FieldstgListedBattle D_WSTAG676_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A62D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A62DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A62E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A62F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A630C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6318 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG676_800A6324 = {
    0,
    { &D_WSTAG676_800A62C4, &D_WSTAG676_800A62D0, &D_WSTAG676_800A62DC, &D_WSTAG676_800A62E8, &D_WSTAG676_800A62F4,
        &D_WSTAG676_800A6300, &D_WSTAG676_800A630C, &D_WSTAG676_800A6318 },
};
FieldstgListedBattle D_WSTAG676_800A6348 = { 25, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG676_800A6354 = { 307, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG676_800A6360 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A636C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A6390 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG676_800A639C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG676_800A63A8 = {
    0,
    { &D_WSTAG676_800A6348, &D_WSTAG676_800A6354, &D_WSTAG676_800A6360, &D_WSTAG676_800A636C, &D_WSTAG676_800A6378,
        &D_WSTAG676_800A6384, &D_WSTAG676_800A6390, &D_WSTAG676_800A639C },
};
FieldstgBattleLists wstag676_battle_lists = {
    152, 0, 0, { &D_WSTAG676_800A621C, &D_WSTAG676_800A62A0, &D_WSTAG676_800A6324 }, &D_WSTAG676_800A63A8,
};
FieldstgVramPlace wstag676_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 436, 296, 464, 40, 368, 510 }, { 384, 256, 412, 296, 368, 40, 320, 509 },
    { 384, 256, 420, 296, 400, 40, 336, 509 }, { 384, 256, 384, 336, 256, 80, 352, 509 },
    { 384, 256, 392, 336, 288, 80, 368, 509 }, { 384, 256, 408, 344, 352, 88, 320, 508 },
    { 384, 256, 404, 296, 336, 40, 336, 508 }, { 384, 256, 428, 296, 432, 40, 352, 508 },
    { 384, 256, 400, 336, 320, 80, 368, 508 }, { 320, 256, 328, 390, 32, 134, 320, 507 },
    { 320, 256, 338, 390, 72, 134, 336, 507 }, { 320, 256, 364, 488, 176, 232, 352, 507 },
};
u16 D_WSTAG676_800A6508[4] = { 0x405A, 0, 0xFFFF, 0 };
u16 D_WSTAG676_800A6510[4] = { 0x405A, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6518[4] = { 0x405A, 0, 0xFFFF, 0 };
u16 D_WSTAG676_800A6520[4] = { 0x405A, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6528[4] = { 0x405A, 0, 0xFFFF, 0 };
u16 D_WSTAG676_800A6530[4] = { 0x405A, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6538[4] = { 0x405A, 0, 0xFFFF, 0 };
u16 D_WSTAG676_800A6540[4] = { 0x405A, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6548[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG676_800A6550[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6558[4] = { 0x7A49, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG676_800A6560[2] = { { NULL, NULL, 343 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG676_800A6578[2] = { { NULL, NULL, 344 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG676_800A6590[2] = { { NULL, NULL, 345 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG676_800A65A8[3] = {
    { D_WSTAG676_800A6508, NULL, 494 }, { D_WSTAG676_800A6510, NULL, 495 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG676_800A65CC[3] = {
    { D_WSTAG676_800A6518, NULL, 496 }, { D_WSTAG676_800A6520, NULL, 497 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG676_800A65F0[3] = {
    { D_WSTAG676_800A6528, NULL, 498 }, { D_WSTAG676_800A6530, NULL, 499 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG676_800A6614[3] = {
    { D_WSTAG676_800A6538, NULL, 500 }, { D_WSTAG676_800A6540, NULL, 501 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG676_800A6638[2] = { { NULL, NULL, 346 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG676_800A6650[2] = { { NULL, NULL, 342 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG676_800A6668[3] = {
    { D_WSTAG676_800A6548, NULL, 525 }, { D_WSTAG676_800A6550, D_WSTAG676_800A6558, 423 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG676_800A668C[2] = { { NULL, NULL, 341 }, { NULL, NULL, 0 } };
u16 D_WSTAG676_800A66A4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66AC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66B4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66BC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66C4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66CC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66D4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66DC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66E4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66EC[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A66F4[6] = { 0x405A, 0, 0x6022, 1, 0xFFFF, 0 };
u16 D_WSTAG676_800A6700[4] = { 0x6022, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG676_800A6708 = { D_WSTAG676_800A66A4, D_WSTAG676_800A6560, 37, 4, 345, 470, 3 };
FieldstgPlacedActor D_WSTAG676_800A671C = { D_WSTAG676_800A66AC, D_WSTAG676_800A6578, 38, 5, 296, 445, 7 };
FieldstgPlacedActor D_WSTAG676_800A6730 = { D_WSTAG676_800A66B4, D_WSTAG676_800A6590, 39, 6, 297, 470, 5 };
FieldstgPlacedActor D_WSTAG676_800A6744 = { D_WSTAG676_800A66BC, D_WSTAG676_800A65A8, 70, 7, 345, 470, 3 };
FieldstgPlacedActor D_WSTAG676_800A6758 = { D_WSTAG676_800A66C4, D_WSTAG676_800A65CC, 71, 8, 296, 445, 7 };
FieldstgPlacedActor D_WSTAG676_800A676C = { D_WSTAG676_800A66CC, D_WSTAG676_800A65F0, 72, 9, 297, 470, 5 };
FieldstgPlacedActor D_WSTAG676_800A6780 = { D_WSTAG676_800A66D4, D_WSTAG676_800A6614, 73, 10, 344, 446, 1 };
FieldstgPlacedActor D_WSTAG676_800A6794 = { D_WSTAG676_800A66DC, D_WSTAG676_800A6638, 111, 11, 344, 446, 1 };
FieldstgPlacedActor D_WSTAG676_800A67A8 = { D_WSTAG676_800A66E4, D_WSTAG676_800A6650, 145, 12, 320, 153, 7 };
FieldstgPlacedActor D_WSTAG676_800A67BC = { D_WSTAG676_800A66EC, D_WSTAG676_800A6668, 206, 13, 509, 563, 3 };
FieldstgPlacedActor D_WSTAG676_800A67D0 = { D_WSTAG676_800A66F4, D_WSTAG676_800A668C, 210, 14, 320, 153, 7 };
FieldstgPlacedActor D_WSTAG676_800A67E4 = { D_WSTAG676_800A6700, NULL, 316, 15, 0, 0, 1 };
FieldstgPlacedActor *wstag676_actors[13] = {
    &D_WSTAG676_800A6708, &D_WSTAG676_800A671C, &D_WSTAG676_800A6730, &D_WSTAG676_800A6744, &D_WSTAG676_800A6758,
    &D_WSTAG676_800A676C, &D_WSTAG676_800A6780, &D_WSTAG676_800A6794, &D_WSTAG676_800A67A8, &D_WSTAG676_800A67BC,
    &D_WSTAG676_800A67D0, &D_WSTAG676_800A67E4, NULL,
};
FieldstgSprite wstag676_sprites[23] = {
    { 1, 0, 0x50, 2, 0x2F, 0, 0, 0, 0, 0, 718, 505, 0, 0 }, { 1, 0, 0x50, 2, 0x2F, 0, 0, 0, 0, 0, 815, 456, 0, 0 },
    { 1, 0, 0x40, 2, 0x30, 2, 0, 1, 0xA, 0, 625, 376, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x35, 0xA, 0, 719, 451, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 1, 0x32, 0x35, 0xA, 0, 815, 343, 0, 0 },
    { 1, 0, 0x40, 2, 0x3E, 1, 0x3E, 0x47, 6, 0, 637, 147, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 1, 0x48, 0x51, 6, 0, 769, 352, 0, 0 },
    { 1, 0, 0x50, 6, 0x2E, 0, 0, 0, 0, 0, 182, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x30, 2, 0, 1, 0xA, 0, 157, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x30, 2, 0, 1, 0xA, 0, 213, 344, 0, 0 },
    { 1, 0, 0x40, 6, 0x30, 2, 0, 1, 0xA, 0, 221, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x30, 2, 0, 1, 0xA, 0, 549, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x30, 2, 0, 1, 0xA, 0, 742, 201, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x35, 0xA, 0, 182, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 1, 0x36, 0x39, 8, 0, 91, 349, 0, 0 },
    { 1, 0, 0x40, 6, 0x52, 1, 0x52, 0x5B, 6, 0, 487, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x5C, 1, 0x5C, 0x5F, 0xA, 0, 661, 151, 0, 0 },
    { 1, 0, 0x40, 6, 0x5C, 1, 0x5C, 0x5F, 0xA, 0, 780, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 1, 0x60, 0x63, 0xA, 0, 504, 449, 0, 0 },
    { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 249, 99, 171, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 287, 425, 455, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 511, 525, 552, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag676_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C6, 0x46C, 0xAA, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C8, 0xA8, 0xA4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x2C0, 0x100, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x2B0, 0x148, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x210, 0x118, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x200, 0x160, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x1C0, 0x180, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x1B0, 0x1C8, 0, 0, 0, 0 }, { 0x6022, 1, 0x405A, 0, 8, 0x384, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag676_funcs = { wstag676_setup };
FieldstgEventDef wstag676_events[3] = {
    { 900, D_WSTAG676_800A6094, 0x01430019, NULL, wstag676_event_900_end },
    { 901, D_WSTAG676_800A6130, 0x0143001A, NULL, wstag676_event_901_end }, { -1, NULL, 0, NULL, NULL },
};
