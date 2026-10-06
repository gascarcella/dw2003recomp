#include "wstag.h"

/* WSTAG565: stage 0x24A (fieldstg_stages). */

extern WstagFuncs wstag565_funcs;
extern FieldstgBattleLists wstag565_battle_lists;
extern FieldstgVramPlace wstag565_vram_places[];
extern FieldstgPlacedActor *wstag565_actors[];
extern FieldstgSprite wstag565_sprites[];
extern FieldstgMapEvent wstag565_map_events[];
extern FieldstgEventDef wstag565_events[];
void wstag565_update();

void wstag565_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4029, 1) && gamestate_flags.get_flag(0x402A, 0)) {
            data->event = fieldstg_event_start(0x4F8);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag565_start(void *arg0) {
    WstagObject *obj = object_new(wstag565_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag565_funcs.setup();
    return obj;
}

void wstag565_event_1271_end(void) {
    gamestate_flags.set_flag(0x4029, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag565_event_1272_end(void) {
    gamestate_flags.set_flag(0x402A, 1);
    gamestate_flags.set_flag(0x8026, 1);
}

void wstag565_setup(void) {
    fieldstg_stage.background_file = 0x51A;
    fieldstg_stage.sprite_file = 0x051B0000;
    fieldstg_stage.sprites = wstag565_sprites;
    fieldstg_stage.map_events = wstag565_map_events;
    fieldstg_stage.mask_file = 0x519;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0xC700, 0x1E500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag565_vram_places;
    fieldstg_stage.music = 0x14;
    fieldstg_stage.sound = 0x60500000;
    fieldstg_stage.actors = wstag565_actors;
    fieldstg_stage.battle_lists = &wstag565_battle_lists;
    fieldstg_stage.events = wstag565_events;
    fieldstg_attr.set_file(0, 0x051B0001);
    fieldstg_attr.set_file(7, 0x051B0002);
    fieldstg_attr.set_file(4, 0x051B0003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag565_setup(void);

s16 D_WSTAG565_800A60BC[56] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 1017, 949, 7),
    FIELDSTG_EVENT_PLACE(125, 1049, 965),
    FIELDSTG_EVENT_ANIM(125, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 125, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x800A, /* padding, not read */
};
s16 D_WSTAG565_800A612C[66] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_PLACE(2, 1017, 949),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_PLACE(125, 1049, 965),
    FIELDSTG_EVENT_ANIM(125, 1, 3),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 125, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG565_800A61B0 = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61BC = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61C8 = { 149, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61D4 = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61E0 = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61EC = { 70, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A61F8 = { 67, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A6204 = { 67, 3, 0x60080000 };
FieldstgBattleList D_WSTAG565_800A6210 = {
    3,
    { &D_WSTAG565_800A61B0, &D_WSTAG565_800A61BC, &D_WSTAG565_800A61C8, &D_WSTAG565_800A61D4, &D_WSTAG565_800A61E0,
        &D_WSTAG565_800A61EC, &D_WSTAG565_800A61F8, &D_WSTAG565_800A6204 },
};
FieldstgListedBattle D_WSTAG565_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6288 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG565_800A6294 = {
    0,
    { &D_WSTAG565_800A6234, &D_WSTAG565_800A6240, &D_WSTAG565_800A624C, &D_WSTAG565_800A6258, &D_WSTAG565_800A6264,
        &D_WSTAG565_800A6270, &D_WSTAG565_800A627C, &D_WSTAG565_800A6288 },
};
FieldstgListedBattle D_WSTAG565_800A62B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A62D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A62DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A62E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A62F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A630C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG565_800A6318 = {
    0,
    { &D_WSTAG565_800A62B8, &D_WSTAG565_800A62C4, &D_WSTAG565_800A62D0, &D_WSTAG565_800A62DC, &D_WSTAG565_800A62E8,
        &D_WSTAG565_800A62F4, &D_WSTAG565_800A6300, &D_WSTAG565_800A630C },
};
FieldstgListedBattle D_WSTAG565_800A633C = { 267, 3, 0x60880000 };
FieldstgListedBattle D_WSTAG565_800A6348 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6354 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6360 = { 329, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A636C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG565_800A6384 = { 156, 3, 0x60080000 };
FieldstgListedBattle D_WSTAG565_800A6390 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG565_800A639C = {
    0,
    { &D_WSTAG565_800A633C, &D_WSTAG565_800A6348, &D_WSTAG565_800A6354, &D_WSTAG565_800A6360, &D_WSTAG565_800A636C,
        &D_WSTAG565_800A6378, &D_WSTAG565_800A6384, &D_WSTAG565_800A6390 },
};
FieldstgBattleLists wstag565_battle_lists = {
    34, 0, 0, { &D_WSTAG565_800A6210, &D_WSTAG565_800A6294, &D_WSTAG565_800A6318 }, &D_WSTAG565_800A639C,
};
FieldstgVramPlace wstag565_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 256, 216, 0, 336, 511 }, { 320, 256, 374, 288, 216, 32, 352, 511 },
    { 320, 256, 366, 370, 184, 114, 368, 511 }, { 320, 256, 354, 370, 136, 114, 336, 510 },
};
u16 D_WSTAG565_800A647C[8] = { 0x209, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG565_800A648C[8] = { 0x20A, 1, 0x708B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG565_800A649C[8] = { 0x20B, 1, 0x8245, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG565_800A64AC[4] = { 0x1C15, 0, 0xFFFF, 0 };
u16 D_WSTAG565_800A64B4[6] = { 0x9028, 1, 0x1C15, 1, 0xFFFF, 0 };
u16 D_WSTAG565_800A64C0[4] = { 0x1C15, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG565_800A64C8[2] = { { NULL, D_WSTAG565_800A647C, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG565_800A64E0[2] = { { NULL, D_WSTAG565_800A648C, 364 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG565_800A64F8[2] = { { NULL, D_WSTAG565_800A649C, 596 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG565_800A6510[3] = {
    { D_WSTAG565_800A64AC, D_WSTAG565_800A64B4, 707 }, { D_WSTAG565_800A64C0, NULL, 708 }, { NULL, NULL, 0 },
};
u16 D_WSTAG565_800A6534[4] = { 0x209, 0, 0xFFFF, 0 };
u16 D_WSTAG565_800A653C[4] = { 0x20A, 0, 0xFFFF, 0 };
u16 D_WSTAG565_800A6544[4] = { 0x20B, 0, 0xFFFF, 0 };
u16 D_WSTAG565_800A654C[6] = { 0x603, 1, 0x8026, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG565_800A6558 = { D_WSTAG565_800A6534, D_WSTAG565_800A64C8, 33, 4, 256, 561, 1 };
FieldstgPlacedActor D_WSTAG565_800A656C = { D_WSTAG565_800A653C, D_WSTAG565_800A64E0, 77, 5, 784, 393, 1 };
FieldstgPlacedActor D_WSTAG565_800A6580 = { D_WSTAG565_800A6544, D_WSTAG565_800A64F8, 78, 6, 593, 793, 1 };
FieldstgPlacedActor D_WSTAG565_800A6594 = { D_WSTAG565_800A654C, D_WSTAG565_800A6510, 125, 7, 1049, 965, 7 };
FieldstgPlacedActor *wstag565_actors[5] = {
    &D_WSTAG565_800A6558, &D_WSTAG565_800A656C, &D_WSTAG565_800A6580, &D_WSTAG565_800A6594, NULL,
};
FieldstgSprite wstag565_sprites[16] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 1234, 165, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 1279, 187, 0, 0 }, { 1, 0, 0x40, 6, 3, 1, 3, 8, 4, 0, 125, 286, 0, 0 },
    { 1, 0, 0x40, 6, 3, 1, 3, 8, 4, 0, 1409, 823, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 1497, 247, 283, 0 },
    { 1, 0, 0x6C, 4, 1, 0, 0, 0, 0, 0, 920, 640, 699, 0 }, { 1, 0, 0x57, 4, 2, 0, 0, 0, 0, 0, 1040, 220, 285, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 512, 512, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 432, 320, 320, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 496, 336, 336, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 584, 584, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 936, 900, 900, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1136, 168, 168, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1328, 896, 896, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1568, 479, 479, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag565_map_events[9] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x249, 0x294, 0x1F0, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24C, 0xA8, 0x3FC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24B, 0x90, 0xD0, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 2, 3 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 4, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 0x1E, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 8, 1 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag565_funcs = { wstag565_setup };
FieldstgEventDef wstag565_events[3] = {
    { 1271, D_WSTAG565_800A60BC, 0x013C0010, NULL, wstag565_event_1271_end },
    { 1272, D_WSTAG565_800A612C, 0x013C0011, NULL, wstag565_event_1272_end }, { -1, NULL, 0, NULL, NULL },
};
