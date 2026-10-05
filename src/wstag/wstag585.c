#include "wstag.h"

/* WSTAG585: stage 0x24E (fieldstg_stages). */

extern WstagFuncs wstag585_funcs;
extern FieldstgBattleLists wstag585_battle_lists;
extern FieldstgVramPlace wstag585_vram_places[];
extern FieldstgPlacedActor *wstag585_actors[];
extern FieldstgSprite wstag585_sprites[];
extern FieldstgMapEvent wstag585_map_events[];
extern FieldstgEventDef wstag585_events[];
void wstag585_update();

void wstag585_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_flags.get_flag(0x4033, 0) && gamestate_flags.get_flag(0x1C20, 1)) {
            data->event = fieldstg_event_start(0x1CC);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag585_start(void *arg0) {
    WstagObject *obj = object_new(wstag585_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag585_funcs.setup();
    return obj;
}

void wstag585_event_460_end(void) {
    gamestate_flags.set_flag(0x4033, 1);
}

void wstag585_setup(void) {
    fieldstg_stage.background_file = 0x504;
    fieldstg_stage.sprite_file = 0x05050000;
    fieldstg_stage.sprites = wstag585_sprites;
    fieldstg_stage.map_events = wstag585_map_events;
    fieldstg_stage.mask_file = 0x503;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13200, 0x15B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag585_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag585_actors;
    fieldstg_stage.events = wstag585_events;
    fieldstg_stage.battle_lists = &wstag585_battle_lists;
    fieldstg_attr.set_file(0, 0x05050001);
    fieldstg_attr.set_file(7, 0x05050002);
    fieldstg_attr.set_file(4, 0x05050003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag585_setup(void);

s16 D_WSTAG585_800A602C[56] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 239, 384),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 277, 366, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG585_800A609C = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60A8 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60B4 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60C0 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60CC = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60D8 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60E4 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG585_800A60F0 = { 75, 12, 0x60080000 };
FieldstgBattleList D_WSTAG585_800A60FC = {
    4,
    { &D_WSTAG585_800A609C, &D_WSTAG585_800A60A8, &D_WSTAG585_800A60B4, &D_WSTAG585_800A60C0, &D_WSTAG585_800A60CC,
        &D_WSTAG585_800A60D8, &D_WSTAG585_800A60E4, &D_WSTAG585_800A60F0 },
};
FieldstgListedBattle D_WSTAG585_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG585_800A6180 = {
    0,
    { &D_WSTAG585_800A6120, &D_WSTAG585_800A612C, &D_WSTAG585_800A6138, &D_WSTAG585_800A6144, &D_WSTAG585_800A6150,
        &D_WSTAG585_800A615C, &D_WSTAG585_800A6168, &D_WSTAG585_800A6174 },
};
FieldstgListedBattle D_WSTAG585_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A61F8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG585_800A6204 = {
    0,
    { &D_WSTAG585_800A61A4, &D_WSTAG585_800A61B0, &D_WSTAG585_800A61BC, &D_WSTAG585_800A61C8, &D_WSTAG585_800A61D4,
        &D_WSTAG585_800A61E0, &D_WSTAG585_800A61EC, &D_WSTAG585_800A61F8 },
};
FieldstgListedBattle D_WSTAG585_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG585_800A627C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG585_800A6288 = {
    0,
    { &D_WSTAG585_800A6228, &D_WSTAG585_800A6234, &D_WSTAG585_800A6240, &D_WSTAG585_800A624C, &D_WSTAG585_800A6258,
        &D_WSTAG585_800A6264, &D_WSTAG585_800A6270, &D_WSTAG585_800A627C },
};
FieldstgBattleLists wstag585_battle_lists = {
    42, 0, 0, { &D_WSTAG585_800A60FC, &D_WSTAG585_800A6180, &D_WSTAG585_800A6204 }, &D_WSTAG585_800A6288,
};
FieldstgVramPlace wstag585_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 256, 216, 0, 336, 511 },
};
u16 D_WSTAG585_800A6338[8] = { 0x20D, 1, 0x822F, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG585_800A6348[2] = { { NULL, D_WSTAG585_800A6338, 598 }, { NULL, NULL, 0 } };
u16 D_WSTAG585_800A6360[6] = { 0x20D, 0, 0x1C20, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG585_800A636C = { D_WSTAG585_800A6360, D_WSTAG585_800A6348, 33, 4, 379, 318, 1 };
FieldstgPlacedActor *wstag585_actors[2] = { &D_WSTAG585_800A636C, NULL };
FieldstgSprite wstag585_sprites[4] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 277, 252, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 6, 0, 405, 317, 0, 0 },
    { 1, 0, 0x40, 6, 0, 1, 0, 0xB, 4, 0, 214, 250, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag585_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x358, 0x37C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag585_funcs = { wstag585_setup };
FieldstgEventDef wstag585_events[2] = {
    { 460, D_WSTAG585_800A602C, 0x013C0007, NULL, wstag585_event_460_end }, { -1, NULL, 0, NULL, NULL },
};
