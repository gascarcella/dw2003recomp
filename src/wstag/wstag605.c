#include "wstag.h"

/* WSTAG605: stage 0x252 (fieldstg_stages). */

extern WstagFuncs wstag605_funcs;
extern FieldstgBattleLists wstag605_battle_lists;
extern FieldstgVramPlace wstag605_vram_places[];
extern FieldstgPlacedActor *wstag605_actors[];
extern FieldstgSprite wstag605_sprites[];
extern FieldstgMapEvent wstag605_map_events[];
extern FieldstgEventDef wstag605_events[];
void wstag605_update();

void wstag605_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_data.progress == 0x11 && gamestate_flags.get_flag(0x40A6, 1)) {
            data->event = fieldstg_event_start(0x1FF);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag605_start(void *arg0) {
    WstagObject *obj = object_new(wstag605_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag605_funcs.setup();
    return obj;
}

void wstag605_event_511_end(void) {
    gamestate_flags.set_flag(0x8680, 1);
    gamestate_data.progress = 0x12;
}

void wstag605_event_510_end(void) {
    gamestate_flags.set_flag(0x40A6, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag605_setup(void) {
    fieldstg_stage.background_file = 0x484;
    fieldstg_stage.sprite_file = 0x04850000;
    fieldstg_stage.sprites = wstag605_sprites;
    fieldstg_stage.map_events = wstag605_map_events;
    fieldstg_stage.mask_file = 0x483;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1E800, 0x14000 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag605_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag605_actors;
    fieldstg_stage.battle_lists = &wstag605_battle_lists;
    fieldstg_stage.events = wstag605_events;
    fieldstg_attr.set_file(0, 0x04850001);
    fieldstg_attr.set_file(7, 0x04850002);
    fieldstg_attr.set_file(4, 0x04850003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag605_setup(void);

s16 D_WSTAG605_800A6060[66] = {
    FIELDSTG_EVENT_WALK(2, 625, 201, 5),
    FIELDSTG_EVENT_PLACE(150, 657, 185),
    FIELDSTG_EVENT_ANIM(150, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 150, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 150, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG605_800A60E4[72] = {
    FIELDSTG_EVENT_PLACE(2, 625, 201),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(150, 657, 185),
    FIELDSTG_EVENT_ANIM(150, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 150, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 150, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 600, 213, 1),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x24D, 1, 1, 7),
    FIELDSTG_EVENT_END,
    0x6004, /* padding, not read */
};
FieldstgListedBattle D_WSTAG605_800A6174 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A6180 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A618C = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A6198 = { 80, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A61A4 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A61B0 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A61BC = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG605_800A61C8 = { 75, 12, 0x60080000 };
FieldstgBattleList D_WSTAG605_800A61D4 = {
    3,
    { &D_WSTAG605_800A6174, &D_WSTAG605_800A6180, &D_WSTAG605_800A618C, &D_WSTAG605_800A6198, &D_WSTAG605_800A61A4,
        &D_WSTAG605_800A61B0, &D_WSTAG605_800A61BC, &D_WSTAG605_800A61C8 },
};
FieldstgListedBattle D_WSTAG605_800A61F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6204 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6210 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A621C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A624C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG605_800A6258 = {
    0,
    { &D_WSTAG605_800A61F8, &D_WSTAG605_800A6204, &D_WSTAG605_800A6210, &D_WSTAG605_800A621C, &D_WSTAG605_800A6228,
        &D_WSTAG605_800A6234, &D_WSTAG605_800A6240, &D_WSTAG605_800A624C },
};
FieldstgListedBattle D_WSTAG605_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6288 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6294 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A62A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A62AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A62B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A62D0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG605_800A62DC = {
    0,
    { &D_WSTAG605_800A627C, &D_WSTAG605_800A6288, &D_WSTAG605_800A6294, &D_WSTAG605_800A62A0, &D_WSTAG605_800A62AC,
        &D_WSTAG605_800A62B8, &D_WSTAG605_800A62C4, &D_WSTAG605_800A62D0 },
};
FieldstgListedBattle D_WSTAG605_800A6300 = { 7, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG605_800A630C = { 311, 19, 0x60880000 };
FieldstgListedBattle D_WSTAG605_800A6318 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6324 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6330 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A633C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6348 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG605_800A6354 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG605_800A6360 = {
    0,
    { &D_WSTAG605_800A6300, &D_WSTAG605_800A630C, &D_WSTAG605_800A6318, &D_WSTAG605_800A6324, &D_WSTAG605_800A6330,
        &D_WSTAG605_800A633C, &D_WSTAG605_800A6348, &D_WSTAG605_800A6354 },
};
FieldstgBattleLists wstag605_battle_lists = {
    46, 0, 0, { &D_WSTAG605_800A61D4, &D_WSTAG605_800A6258, &D_WSTAG605_800A62DC }, &D_WSTAG605_800A6360,
};
FieldstgVramPlace wstag605_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 362, 256, 168, 0, 336, 505 },
};
u16 D_WSTAG605_800A6410[4] = { 0x1C25, 0, 0xFFFF, 0 };
u16 D_WSTAG605_800A6418[6] = { 0x1C25, 1, 0xA04, 0, 0xFFFF, 0 };
u16 D_WSTAG605_800A6424[6] = { 0x905E, 1, 0xA04, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A6430[6] = { 0x1C25, 1, 0xA04, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG605_800A643C[2] = { { NULL, NULL, 332 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A6454[2] = { { NULL, NULL, 340 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A646C[2] = { { NULL, NULL, 335 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A6484[2] = { { NULL, NULL, 336 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A649C[2] = { { NULL, NULL, 337 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A64B4[2] = { { NULL, NULL, 338 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A64CC[2] = { { NULL, NULL, 339 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A64E4[2] = { { NULL, NULL, 333 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG605_800A64FC[4] = {
    { D_WSTAG605_800A6410, NULL, 334 }, { D_WSTAG605_800A6418, D_WSTAG605_800A6424, 762 },
    { D_WSTAG605_800A6430, NULL, 764 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG605_800A652C[2] = { { NULL, NULL, 764 }, { NULL, NULL, 0 } };
u16 D_WSTAG605_800A6544[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A654C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A6554[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A655C[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A6564[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A656C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A6574[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A657C[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A6584[4] = { 0x6011, 1, 0xFFFF, 0 };
u16 D_WSTAG605_800A658C[4] = { 0x6012, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG605_800A6594 = { D_WSTAG605_800A6544, D_WSTAG605_800A643C, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A65A8 = { D_WSTAG605_800A654C, D_WSTAG605_800A6454, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A65BC = { D_WSTAG605_800A6554, D_WSTAG605_800A646C, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A65D0 = { D_WSTAG605_800A655C, D_WSTAG605_800A6484, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A65E4 = { D_WSTAG605_800A6564, D_WSTAG605_800A649C, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A65F8 = { D_WSTAG605_800A656C, D_WSTAG605_800A64B4, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A660C = { D_WSTAG605_800A6574, D_WSTAG605_800A64CC, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A6620 = { D_WSTAG605_800A657C, D_WSTAG605_800A64E4, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A6634 = { D_WSTAG605_800A6584, D_WSTAG605_800A64FC, 150, 4, 657, 185, 1 };
FieldstgPlacedActor D_WSTAG605_800A6648 = { D_WSTAG605_800A658C, D_WSTAG605_800A652C, 150, 4, 657, 185, 1 };
FieldstgPlacedActor *wstag605_actors[11] = {
    &D_WSTAG605_800A6594, &D_WSTAG605_800A65A8, &D_WSTAG605_800A65BC, &D_WSTAG605_800A65D0, &D_WSTAG605_800A65E4,
    &D_WSTAG605_800A65F8, &D_WSTAG605_800A660C, &D_WSTAG605_800A6620, &D_WSTAG605_800A6634, &D_WSTAG605_800A6648,
    NULL,
};
FieldstgSprite wstag605_sprites[20] = {
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
FieldstgMapEvent wstag605_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x110, 0x8C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x3A8, 0xE4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x468, 0xFC, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag605_funcs = { wstag605_setup };
FieldstgEventDef wstag605_events[3] = {
    { 510, D_WSTAG605_800A6060, 0x01430013, NULL, wstag605_event_510_end },
    { 511, D_WSTAG605_800A60E4, 0x01430014, NULL, wstag605_event_511_end }, { -1, NULL, 0, NULL, NULL },
};
