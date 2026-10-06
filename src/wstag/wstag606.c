#include "wstag.h"

/* WSTAG606: stage 0x2BC (fieldstg_stages). */

extern WstagFuncs wstag606_funcs;
extern FieldstgBattleLists wstag606_battle_lists;
extern FieldstgVramPlace wstag606_vram_places[];
extern FieldstgPlacedActor *wstag606_actors[];
extern FieldstgSprite wstag606_sprites[];
extern FieldstgMapEvent wstag606_map_events[];
extern FieldstgEventDef wstag606_events[];
void wstag606_update();

void wstag606_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x407E, 1) && gamestate_flags.get_flag(0x407F, 0)) {
            data->event = fieldstg_event_start(0x50A);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag606_start(void *arg0) {
    WstagObject *obj = object_new(wstag606_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag606_funcs.setup();
    return obj;
}

void wstag606_event_1289_end(void) {
    gamestate_flags.set_flag(0x407E, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag606_event_1290_end(void) {
    gamestate_flags.set_flag(0x407F, 1);
    gamestate_flags.set_flag(0x8B0E, 1);
}

void wstag606_setup(void) {
    fieldstg_stage.background_file = 0x612;
    fieldstg_stage.sprite_file = 0x06130000;
    fieldstg_stage.sprites = wstag606_sprites;
    fieldstg_stage.map_events = wstag606_map_events;
    fieldstg_stage.mask_file = 0x611;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2C000, 0x25C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag606_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag606_actors;
    fieldstg_stage.battle_lists = &wstag606_battle_lists;
    fieldstg_stage.events = wstag606_events;
    fieldstg_attr.set_file(0, 0x06130001);
    fieldstg_attr.set_file(7, 0x06130002);
    fieldstg_attr.set_file(4, 0x06130003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag606_setup(void);

s16 D_WSTAG606_800A6090[80] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 629, 206, 5),
    FIELDSTG_EVENT_PLACE(265, 657, 185),
    FIELDSTG_EVENT_ANIM(265, 1, 1),
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
    FIELDSTG_EVENT_DIALOG(0, 2, 265, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 265, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x7, /* padding, not read */
};
s16 D_WSTAG606_800A6130[82] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 629, 206),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(265, 656, 191),
    FIELDSTG_EVENT_ANIM(265, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 265, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 265, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 3),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 265, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG606_800A61D4 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A61E0 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A61EC = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A61F8 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A6204 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A6210 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A621C = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG606_800A6228 = { 164, 12, 0x60080000 };
FieldstgBattleList D_WSTAG606_800A6234 = {
    3,
    { &D_WSTAG606_800A61D4, &D_WSTAG606_800A61E0, &D_WSTAG606_800A61EC, &D_WSTAG606_800A61F8, &D_WSTAG606_800A6204,
        &D_WSTAG606_800A6210, &D_WSTAG606_800A621C, &D_WSTAG606_800A6228 },
};
FieldstgListedBattle D_WSTAG606_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6288 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6294 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A62A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A62AC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG606_800A62B8 = {
    0,
    { &D_WSTAG606_800A6258, &D_WSTAG606_800A6264, &D_WSTAG606_800A6270, &D_WSTAG606_800A627C, &D_WSTAG606_800A6288,
        &D_WSTAG606_800A6294, &D_WSTAG606_800A62A0, &D_WSTAG606_800A62AC },
};
FieldstgListedBattle D_WSTAG606_800A62DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A62E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A62F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A630C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6324 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6330 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG606_800A633C = {
    0,
    { &D_WSTAG606_800A62DC, &D_WSTAG606_800A62E8, &D_WSTAG606_800A62F4, &D_WSTAG606_800A6300, &D_WSTAG606_800A630C,
        &D_WSTAG606_800A6318, &D_WSTAG606_800A6324, &D_WSTAG606_800A6330 },
};
FieldstgListedBattle D_WSTAG606_800A6360 = { 23, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG606_800A636C = { 319, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG606_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A6390 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A639C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A63A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG606_800A63B4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG606_800A63C0 = {
    0,
    { &D_WSTAG606_800A6360, &D_WSTAG606_800A636C, &D_WSTAG606_800A6378, &D_WSTAG606_800A6384, &D_WSTAG606_800A6390,
        &D_WSTAG606_800A639C, &D_WSTAG606_800A63A8, &D_WSTAG606_800A63B4 },
};
FieldstgBattleLists wstag606_battle_lists = {
    111, 0, 0, { &D_WSTAG606_800A6234, &D_WSTAG606_800A62B8, &D_WSTAG606_800A633C }, &D_WSTAG606_800A63C0,
};
FieldstgVramPlace wstag606_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 348, 256, 112, 0, 336, 505 },
};
u16 D_WSTAG606_800A6470[4] = { 0xA0B, 0, 0xFFFF, 0 };
u16 D_WSTAG606_800A6478[6] = { 0xA0B, 1, 0x903C, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A6484[4] = { 0xA0B, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A648C[4] = { 0xA0B, 0, 0xFFFF, 0 };
u16 D_WSTAG606_800A6494[6] = { 0xA0B, 1, 0x903C, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64A0[4] = { 0xA0B, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64A8[4] = { 0xA0B, 0, 0xFFFF, 0 };
u16 D_WSTAG606_800A64B0[6] = { 0xA0B, 1, 0x903C, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64BC[4] = { 0xA0B, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64C4[4] = { 0xA0B, 0, 0xFFFF, 0 };
u16 D_WSTAG606_800A64CC[6] = { 0xA0B, 1, 0x903C, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64D8[4] = { 0xA0B, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64E0[4] = { 0xA0B, 0, 0xFFFF, 0 };
u16 D_WSTAG606_800A64E8[6] = { 0xA0B, 1, 0x903C, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A64F4[4] = { 0xA0B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG606_800A64FC[3] = {
    { D_WSTAG606_800A6470, D_WSTAG606_800A6478, 725 }, { D_WSTAG606_800A6484, NULL, 272 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG606_800A6520[3] = {
    { D_WSTAG606_800A648C, D_WSTAG606_800A6494, 725 }, { D_WSTAG606_800A64A0, NULL, 275 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG606_800A6544[3] = {
    { D_WSTAG606_800A64A8, D_WSTAG606_800A64B0, 725 }, { D_WSTAG606_800A64BC, NULL, 274 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG606_800A6568[3] = {
    { D_WSTAG606_800A64C4, D_WSTAG606_800A64CC, 725 }, { D_WSTAG606_800A64D8, NULL, 273 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG606_800A658C[3] = {
    { D_WSTAG606_800A64E0, D_WSTAG606_800A64E8, 725 }, { D_WSTAG606_800A64F4, NULL, 276 }, { NULL, NULL, 0 },
};
u16 D_WSTAG606_800A65B0[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A65B8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A65C0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A65C8[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG606_800A65D0[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG606_800A65D8 = { D_WSTAG606_800A65B0, D_WSTAG606_800A64FC, 265, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG606_800A65EC = { D_WSTAG606_800A65B8, D_WSTAG606_800A6520, 265, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG606_800A6600 = { D_WSTAG606_800A65C0, D_WSTAG606_800A6544, 265, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG606_800A6614 = { D_WSTAG606_800A65C8, D_WSTAG606_800A6568, 265, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG606_800A6628 = { D_WSTAG606_800A65D0, D_WSTAG606_800A658C, 265, 4, 657, 185, 1 };
FieldstgPlacedActor *wstag606_actors[6] = {
    &D_WSTAG606_800A65D8, &D_WSTAG606_800A65EC, &D_WSTAG606_800A6600, &D_WSTAG606_800A6614, &D_WSTAG606_800A6628,
    NULL,
};
FieldstgSprite wstag606_sprites[20] = {
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 5, 8, 0, 639, 174, 0, 0 }, { 1, 0, 0x40, 2, 0x3C, 2, 0, 5, 8, 0, 671, 160, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 782, 501, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 6, 0, 800, 478, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 6, 0, 576, 480, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 5, 6, 0, 609, 493, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 5, 6, 0, 646, 507, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 2, 0, 0xB, 8, 0, 587, 71, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 0xB, 8, 0, 573, 142, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 0xB, 8, 0, 705, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 2, 0, 0xB, 8, 0, 692, 116, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 5, 8, 0, 709, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 0xB, 8, 0, 743, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 5, 6, 0, 710, 359, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 5, 6, 0, 758, 383, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 5, 6, 0, 854, 431, 0, 0 }, { 1, 0, 0x40, 6, 0x3F, 2, 0, 5, 6, 0, 32, 447, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 2, 0, 5, 6, 0, 416, 255, 0, 0 }, { 1, 0, 0x40, 6, 0x3F, 2, 0, 5, 6, 0, 463, 231, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag606_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x110, 0x8C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x3A8, 0xE4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x468, 0xFC, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag606_funcs = { wstag606_setup };
FieldstgEventDef wstag606_events[3] = {
    { 1289, D_WSTAG606_800A6090, 0x01430008, NULL, wstag606_event_1289_end },
    { 1290, D_WSTAG606_800A6130, 0x01430009, NULL, wstag606_event_1290_end }, { -1, NULL, 0, NULL, NULL },
};
