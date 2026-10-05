#include "wstag.h"

/* WSTAG561: stage 0x2B3 (fieldstg_stages). */

extern WstagFuncs wstag561_funcs;
extern FieldstgBattleLists wstag561_battle_lists;
extern FieldstgVramPlace wstag561_vram_places[];
extern FieldstgPlacedActor *wstag561_actors[];
extern FieldstgSprite wstag561_sprites[];
extern FieldstgMapEvent wstag561_map_events[];
extern FieldstgEventDef wstag561_events[];

void wstag561_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag561_start(void *arg0) {
    WstagObject *obj = object_new(wstag561_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag561_funcs.setup();
    return obj;
}

void wstag561_event_1463_end(void) {
    gamestate_flags.set_flag(0x7C1A, 1);
}

void wstag561_setup(void) {
    fieldstg_stage.background_file = 0x5D5;
    fieldstg_stage.sprite_file = 0x05D60000;
    fieldstg_stage.sprites = wstag561_sprites;
    fieldstg_stage.map_events = wstag561_map_events;
    fieldstg_stage.mask_file = 0x5D4;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x26F00, 0x1DF00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag561_vram_places;
    fieldstg_stage.music = 0x41;
    fieldstg_stage.sound = 0x61040000;
    fieldstg_stage.actors = wstag561_actors;
    fieldstg_stage.battle_lists = &wstag561_battle_lists;
    fieldstg_stage.events = wstag561_events;
    fieldstg_attr.set_file(0, 0x05D60001);
    fieldstg_attr.set_file(7, 0x05D60002);
    fieldstg_attr.set_file(4, 0x05D60003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag561_setup(void);

s16 D_WSTAG561_800A5FCC[55] = {
    FIELDSTG_EVENT_WALK(2, 279, 307, 3),
    FIELDSTG_EVENT_PLACE(21, 247, 291),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(36),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC18, 0, 0, 0),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG561_800A603C = { 134, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6048 = { 134, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6054 = { 134, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6060 = { 134, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A606C = { 172, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6078 = { 172, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6084 = { 172, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6090 = { 172, 3, 0x60080000 };
FieldstgBattleList D_WSTAG561_800A609C = {
    3,
    { &D_WSTAG561_800A603C, &D_WSTAG561_800A6048, &D_WSTAG561_800A6054, &D_WSTAG561_800A6060, &D_WSTAG561_800A606C,
        &D_WSTAG561_800A6078, &D_WSTAG561_800A6084, &D_WSTAG561_800A6090 },
};
FieldstgListedBattle D_WSTAG561_800A60C0 = { 134, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A60CC = { 134, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A60D8 = { 134, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A60E4 = { 134, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A60F0 = { 172, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A60FC = { 172, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6108 = { 172, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6114 = { 172, 8, 0x60080000 };
FieldstgBattleList D_WSTAG561_800A6120 = {
    3,
    { &D_WSTAG561_800A60C0, &D_WSTAG561_800A60CC, &D_WSTAG561_800A60D8, &D_WSTAG561_800A60E4, &D_WSTAG561_800A60F0,
        &D_WSTAG561_800A60FC, &D_WSTAG561_800A6108, &D_WSTAG561_800A6114 },
};
FieldstgListedBattle D_WSTAG561_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6198 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG561_800A61A4 = {
    0,
    { &D_WSTAG561_800A6144, &D_WSTAG561_800A6150, &D_WSTAG561_800A615C, &D_WSTAG561_800A6168, &D_WSTAG561_800A6174,
        &D_WSTAG561_800A6180, &D_WSTAG561_800A618C, &D_WSTAG561_800A6198 },
};
FieldstgListedBattle D_WSTAG561_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A61EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A61F8 = { 334, 8, 0x60080000 };
FieldstgListedBattle D_WSTAG561_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A6210 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG561_800A621C = { 180, 8, 0x60080000 };
FieldstgBattleList D_WSTAG561_800A6228 = {
    0,
    { &D_WSTAG561_800A61C8, &D_WSTAG561_800A61D4, &D_WSTAG561_800A61E0, &D_WSTAG561_800A61EC, &D_WSTAG561_800A61F8,
        &D_WSTAG561_800A6204, &D_WSTAG561_800A6210, &D_WSTAG561_800A621C },
};
FieldstgBattleLists wstag561_battle_lists = {
    98, 0, 0, { &D_WSTAG561_800A609C, &D_WSTAG561_800A6120, &D_WSTAG561_800A61A4 }, &D_WSTAG561_800A6228,
};
FieldstgVramPlace wstag561_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 373, 216, 117, 352, 511 }, { 384, 256, 416, 442, 384, 186, 368, 511 },
    { 320, 256, 374, 405, 216, 149, 336, 510 },
};
u16 D_WSTAG561_800A62F8[4] = { 0x7A45, 1, 0xFFFF, 0 };
u16 D_WSTAG561_800A6300[4] = { 0x9069, 1, 0xFFFF, 0 };
u16 D_WSTAG561_800A6308[8] = { 0x254, 1, 0x8236, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG561_800A6318[2] = { { NULL, D_WSTAG561_800A62F8, 727 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG561_800A6330[2] = { { NULL, D_WSTAG561_800A6300, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG561_800A6348[2] = { { NULL, D_WSTAG561_800A6308, 391 }, { NULL, NULL, 0 } };
u16 D_WSTAG561_800A6360[4] = { 0x254, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG561_800A6368 = { NULL, D_WSTAG561_800A6318, 20, 4, 450, 368, 1 };
FieldstgPlacedActor D_WSTAG561_800A637C = { NULL, D_WSTAG561_800A6330, 21, 5, 247, 291, 7 };
FieldstgPlacedActor D_WSTAG561_800A6390 = { D_WSTAG561_800A6360, D_WSTAG561_800A6348, 33, 6, 502, 546, 1 };
FieldstgPlacedActor *wstag561_actors[4] = {
    &D_WSTAG561_800A6368, &D_WSTAG561_800A637C, &D_WSTAG561_800A6390, NULL,
};
FieldstgSprite wstag561_sprites[15] = {
    { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 407, 226, 0, 0 }, { 1, 0, 0x64, 2, 1, 0, 0, 0, 0, 0, 551, 516, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 211, 335, 0, 0 }, { 1, 0, 0x80, 2, 2, 0, 0, 0, 0, 0, 0, 384, 0, 0 },
    { 1, 0, 0x80, 2, 3, 0, 0, 0, 0, 0, 384, 384, 0, 0 }, { 1, 0, 0x70, 2, 4, 0, 0, 0, 0, 0, 20, 512, 0, 0 },
    { 1, 0, 0x78, 2, 5, 0, 0, 0, 0, 0, 264, 523, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x49, 6, 0, 204, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 1, 0x4A, 0x4D, 6, 0, 213, 360, 0, 0 },
    { 1, 0, 0x68, 6, 0x4E, 2, 0, 2, 6, 0, 181, 376, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 153, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 277, 512, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 322, 397, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 584, 137, 169, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag561_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B2, 0x610, 0x240, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B4, 0x80, 0x178, 7, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xF, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC8, 0x3C, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFD0, 0xFFF8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0x40, 0xFFF0, 0, 0, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E2, 0xB0, 0x148, 7, 0, 0xF, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag561_funcs = { wstag561_setup };
FieldstgEventDef wstag561_events[2] = {
    { 1463, D_WSTAG561_800A5FCC, 0x013C0027, NULL, wstag561_event_1463_end }, { -1, NULL, 0, NULL, NULL },
};
