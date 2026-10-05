#include "wstag.h"

/* WSTAG656: stage 0x2C5 (fieldstg_stages). */

extern WstagFuncs wstag656_funcs;
extern FieldstgBattleLists wstag656_battle_lists;
extern FieldstgVramPlace wstag656_vram_places[];
extern FieldstgPlacedActor *wstag656_actors[];
extern FieldstgSprite wstag656_sprites[];
extern FieldstgMapEvent wstag656_map_events[];
extern FieldstgEventDef wstag656_events[];

void wstag656_update(WstagObject *obj) {
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

WstagObject *wstag656_start(void *arg0) {
    WstagObject *obj = object_new(wstag656_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag656_funcs.setup();
    return obj;
}

void wstag656_setup(void) {
    fieldstg_stage.background_file = 0x626;
    fieldstg_stage.sprite_file = 0x06270000;
    fieldstg_stage.sprites = wstag656_sprites;
    fieldstg_stage.map_events = wstag656_map_events;
    fieldstg_stage.mask_file = 0x625;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xF200, 0x12F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag656_vram_places;
    fieldstg_stage.music = 0x16;
    fieldstg_stage.sound = 0x60580000;
    fieldstg_stage.actors = wstag656_actors;
    fieldstg_stage.battle_lists = &wstag656_battle_lists;
    fieldstg_stage.events = wstag656_events;
    fieldstg_attr.set_file(0, 0x06270001);
    fieldstg_attr.set_file(7, 0x06270002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag656_setup(void);

s16 D_WSTAG656_800A5F84[134] = {
    FIELDSTG_EVENT_WALK(2, 388, 116, 5),
    FIELDSTG_EVENT_PLACE(37, 420, 103),
    FIELDSTG_EVENT_ANIM(37, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 37, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 37, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 37, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 37, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 37, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG656_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A60E4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG656_800A60F0 = {
    3,
    { &D_WSTAG656_800A6090, &D_WSTAG656_800A609C, &D_WSTAG656_800A60A8, &D_WSTAG656_800A60B4, &D_WSTAG656_800A60C0,
        &D_WSTAG656_800A60CC, &D_WSTAG656_800A60D8, &D_WSTAG656_800A60E4 },
};
FieldstgListedBattle D_WSTAG656_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6168 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG656_800A6174 = {
    0,
    { &D_WSTAG656_800A6114, &D_WSTAG656_800A6120, &D_WSTAG656_800A612C, &D_WSTAG656_800A6138, &D_WSTAG656_800A6144,
        &D_WSTAG656_800A6150, &D_WSTAG656_800A615C, &D_WSTAG656_800A6168 },
};
FieldstgListedBattle D_WSTAG656_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A61EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG656_800A61F8 = {
    0,
    { &D_WSTAG656_800A6198, &D_WSTAG656_800A61A4, &D_WSTAG656_800A61B0, &D_WSTAG656_800A61BC, &D_WSTAG656_800A61C8,
        &D_WSTAG656_800A61D4, &D_WSTAG656_800A61E0, &D_WSTAG656_800A61EC },
};
FieldstgListedBattle D_WSTAG656_800A621C = { 0, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG656_800A6228 = { 0, 18, 0x600C0000 };
FieldstgListedBattle D_WSTAG656_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG656_800A6270 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG656_800A627C = {
    0,
    { &D_WSTAG656_800A621C, &D_WSTAG656_800A6228, &D_WSTAG656_800A6234, &D_WSTAG656_800A6240, &D_WSTAG656_800A624C,
        &D_WSTAG656_800A6258, &D_WSTAG656_800A6264, &D_WSTAG656_800A6270 },
};
FieldstgBattleLists wstag656_battle_lists = {
    162, 0, 0, { &D_WSTAG656_800A60F0, &D_WSTAG656_800A6174, &D_WSTAG656_800A61F8 }, &D_WSTAG656_800A627C,
};
FieldstgVramPlace wstag656_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 256, 208, 0, 336, 511 }, { 320, 256, 364, 256, 176, 0, 352, 511 },
    { 320, 256, 372, 360, 208, 104, 368, 511 }, { 320, 256, 372, 296, 208, 40, 336, 510 },
    { 320, 256, 364, 304, 176, 48, 352, 510 }, { 320, 256, 372, 328, 208, 72, 368, 510 },
    { 320, 256, 364, 336, 176, 80, 336, 509 },
};
u16 D_WSTAG656_800A638C[4] = { 0x1A34, 0, 0xFFFF, 0 };
u16 D_WSTAG656_800A6394[6] = { 0x1A34, 1, 0x9048, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A63A0[4] = { 0x1A34, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG656_800A63A8[2] = { { NULL, NULL, 343 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A63C0[3] = {
    { D_WSTAG656_800A638C, D_WSTAG656_800A6394, 733 }, { D_WSTAG656_800A63A0, NULL, 803 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG656_800A63E4[2] = { { NULL, NULL, 340 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A63FC[2] = { { NULL, NULL, 339 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A6414[2] = { { NULL, NULL, 345 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A642C[2] = { { NULL, NULL, 344 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A6444[2] = { { NULL, NULL, 344 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A645C[2] = { { NULL, NULL, 344 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A6474[2] = { { NULL, NULL, 347 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG656_800A648C[2] = { { NULL, NULL, 342 }, { NULL, NULL, 0 } };
u16 D_WSTAG656_800A64A4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64B0[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64B8[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64C0[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64C8[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64D0[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG656_800A64DC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64E4[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64EC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG656_800A64F4[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG656_800A64FC = { D_WSTAG656_800A64A4, D_WSTAG656_800A63A8, 32, 4, 128, 196, 1 };
FieldstgPlacedActor D_WSTAG656_800A6510 = { D_WSTAG656_800A64B0, D_WSTAG656_800A63C0, 37, 5, 420, 103, 1 };
FieldstgPlacedActor D_WSTAG656_800A6524 = { D_WSTAG656_800A64B8, D_WSTAG656_800A63E4, 45, 6, 305, 283, 1 };
FieldstgPlacedActor D_WSTAG656_800A6538 = { D_WSTAG656_800A64C0, D_WSTAG656_800A63FC, 45, 6, 305, 283, 1 };
FieldstgPlacedActor D_WSTAG656_800A654C = { D_WSTAG656_800A64C8, D_WSTAG656_800A6414, 57, 7, 420, 103, 1 };
FieldstgPlacedActor D_WSTAG656_800A6560 = { D_WSTAG656_800A64D0, D_WSTAG656_800A642C, 157, 8, 128, 196, 1 };
FieldstgPlacedActor D_WSTAG656_800A6574 = { D_WSTAG656_800A64DC, D_WSTAG656_800A6444, 157, 8, 128, 196, 1 };
FieldstgPlacedActor D_WSTAG656_800A6588 = { D_WSTAG656_800A64E4, D_WSTAG656_800A645C, 157, 8, 128, 196, 1 };
FieldstgPlacedActor D_WSTAG656_800A659C = { D_WSTAG656_800A64EC, D_WSTAG656_800A6474, 158, 9, 420, 103, 1 };
FieldstgPlacedActor D_WSTAG656_800A65B0 = { D_WSTAG656_800A64F4, D_WSTAG656_800A648C, 159, 10, 305, 283, 1 };
FieldstgPlacedActor *wstag656_actors[11] = {
    &D_WSTAG656_800A64FC, &D_WSTAG656_800A6510, &D_WSTAG656_800A6524, &D_WSTAG656_800A6538, &D_WSTAG656_800A654C,
    &D_WSTAG656_800A6560, &D_WSTAG656_800A6574, &D_WSTAG656_800A6588, &D_WSTAG656_800A659C, &D_WSTAG656_800A65B0,
    NULL,
};
FieldstgSprite wstag656_sprites[2] = {
    { 1, 0, 0xFF, 6, 0x32, 2, 0, 2, 0xC, 0, 318, 266, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag656_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C4, 0x126, 0x96, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0xEE, 0x138, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0xDC, 0x1A0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag656_funcs = { wstag656_setup };
FieldstgEventDef wstag656_events[2] = {
    { 736, D_WSTAG656_800A5F84, 0x0143001C, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
