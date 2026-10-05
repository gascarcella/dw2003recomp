#include "wstag.h"

/* WSTAG521: stage 0x2AD (fieldstg_stages). */

extern WstagFuncs wstag521_funcs;
extern FieldstgBattleLists wstag521_battle_lists;
extern FieldstgVramPlace wstag521_vram_places[];
extern FieldstgPlacedActor *wstag521_actors[];
extern FieldstgSprite wstag521_sprites[];
extern FieldstgMapEvent wstag521_map_events[];
extern FieldstgEventDef wstag521_events[];
void wstag521_update();

void wstag521_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0x1C && gamestate_flags.get_flag(0x4052, 1)) {
            data->event = fieldstg_event_start(0x2EF);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag521_start(void *arg0) {
    WstagObject *obj = object_new(wstag521_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag521_funcs.setup();
    return obj;
}

void wstag521_event_750_end(void) {
    gamestate_flags.set_flag(0x4052, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag521_event_751_end(void) {
    gamestate_data.progress = 0x1D;
    gamestate_flags.set_flag(0x8017, 1);
}

void wstag521_setup(void) {
    fieldstg_stage.background_file = 0x5C9;
    fieldstg_stage.sprite_file = 0x05CA0000;
    fieldstg_stage.sprites = wstag521_sprites;
    fieldstg_stage.map_events = wstag521_map_events;
    fieldstg_stage.mask_file = 0x5C8;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x14600, 0xEA00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag521_vram_places;
    fieldstg_stage.music = 0x12;
    fieldstg_stage.sound = 0x60480000;
    fieldstg_stage.actors = wstag521_actors;
    fieldstg_stage.battle_lists = &wstag521_battle_lists;
    fieldstg_stage.events = wstag521_events;
    fieldstg_attr.set_file(0, 0x05CA0001);
    fieldstg_attr.set_file(7, 0x05CA0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag521_setup(void);

s16 D_WSTAG521_800A609C[114] = {
    FIELDSTG_EVENT_WALK(2, 441, 213, 3),
    FIELDSTG_EVENT_PLACE(210, 417, 201),
    FIELDSTG_EVENT_ANIM(210, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 210, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG521_800A6180[90] = {
    FIELDSTG_EVENT_PLACE(2, 441, 213),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(316, 417, 201),
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
    FIELDSTG_EVENT_WALK(2, 425, 205, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(2, 465, 225, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x2AB, 568, 140, 1),
    FIELDSTG_EVENT_END,
};
FieldstgListedBattle D_WSTAG521_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6264 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6270 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A627C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6288 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG521_800A6294 = {
    0,
    { &D_WSTAG521_800A6234, &D_WSTAG521_800A6240, &D_WSTAG521_800A624C, &D_WSTAG521_800A6258, &D_WSTAG521_800A6264,
        &D_WSTAG521_800A6270, &D_WSTAG521_800A627C, &D_WSTAG521_800A6288 },
};
FieldstgListedBattle D_WSTAG521_800A62B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A62D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A62DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A62E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A62F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6300 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A630C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG521_800A6318 = {
    0,
    { &D_WSTAG521_800A62B8, &D_WSTAG521_800A62C4, &D_WSTAG521_800A62D0, &D_WSTAG521_800A62DC, &D_WSTAG521_800A62E8,
        &D_WSTAG521_800A62F4, &D_WSTAG521_800A6300, &D_WSTAG521_800A630C },
};
FieldstgListedBattle D_WSTAG521_800A633C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6348 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6354 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6360 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A636C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6378 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6384 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6390 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG521_800A639C = {
    0,
    { &D_WSTAG521_800A633C, &D_WSTAG521_800A6348, &D_WSTAG521_800A6354, &D_WSTAG521_800A6360, &D_WSTAG521_800A636C,
        &D_WSTAG521_800A6378, &D_WSTAG521_800A6384, &D_WSTAG521_800A6390 },
};
FieldstgListedBattle D_WSTAG521_800A63C0 = { 16, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG521_800A63CC = { 306, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG521_800A63D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A63E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A63F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A63FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG521_800A6414 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG521_800A6420 = {
    0,
    { &D_WSTAG521_800A63C0, &D_WSTAG521_800A63CC, &D_WSTAG521_800A63D8, &D_WSTAG521_800A63E4, &D_WSTAG521_800A63F0,
        &D_WSTAG521_800A63FC, &D_WSTAG521_800A6408, &D_WSTAG521_800A6414 },
};
FieldstgBattleLists wstag521_battle_lists = {
    151, 0, 0, { &D_WSTAG521_800A6294, &D_WSTAG521_800A6318, &D_WSTAG521_800A639C }, &D_WSTAG521_800A6420,
};
FieldstgVramPlace wstag521_vram_places[22] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 368, 413, 192, 157, 352, 511 }, { 320, 256, 372, 373, 208, 117, 368, 511 },
    { 320, 256, 320, 414, 0, 158, 352, 510 }, { 320, 256, 342, 422, 88, 166, 368, 510 },
    { 320, 256, 350, 422, 120, 166, 336, 509 }, { 320, 256, 328, 434, 32, 178, 352, 509 },
    { 320, 256, 362, 373, 168, 117, 368, 509 }, { 320, 256, 344, 382, 96, 126, 336, 508 },
    { 320, 256, 352, 382, 128, 126, 352, 508 }, { 320, 256, 334, 394, 56, 138, 368, 508 },
    { 320, 256, 358, 445, 152, 189, 336, 507 }, { 320, 256, 366, 445, 184, 189, 352, 507 },
    { 320, 256, 374, 445, 216, 189, 368, 507 }, { 320, 256, 334, 346, 56, 90, 336, 506 },
    { 320, 256, 320, 446, 0, 190, 352, 506 }, { 320, 256, 378, 256, 232, 0, 368, 506 },
};
u16 D_WSTAG521_800A65C0[4] = { 0x9049, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG521_800A65C8[2] = { { NULL, NULL, 287 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A65E0[2] = { { NULL, NULL, 293 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A65F8[2] = { { NULL, NULL, 290 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6610[2] = { { NULL, NULL, 298 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6628[2] = { { NULL, NULL, 296 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6640[2] = { { NULL, NULL, 297 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6658[2] = { { NULL, NULL, 295 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6670[2] = { { NULL, NULL, 289 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6688[2] = { { NULL, NULL, 291 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A66A0[2] = { { NULL, NULL, 292 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A66B8[2] = { { NULL, NULL, 288 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A66D0[2] = { { NULL, NULL, 286 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A66E8[2] = { { NULL, NULL, 294 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6700[2] = { { NULL, D_WSTAG521_800A65C0, 285 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG521_800A6718[2] = { { NULL, NULL, 512 }, { NULL, NULL, 0 } };
u16 D_WSTAG521_800A6730[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A673C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A6748[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A6754[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A675C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A6764[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A676C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A6774[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG521_800A6780[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A6788[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG521_800A6794[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A679C[6] = { 0x703D, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG521_800A67A8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A67B0[4] = { 0x4052, 0, 0xFFFF, 0 };
u16 D_WSTAG521_800A67B8[4] = { 0x601C, 1, 0xFFFF, 0 };
u16 D_WSTAG521_800A67C0[4] = { 0x601C, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG521_800A67C8 = { D_WSTAG521_800A6730, D_WSTAG521_800A65C8, 46, 4, 517, 289, 1 };
FieldstgPlacedActor D_WSTAG521_800A67DC = { D_WSTAG521_800A673C, D_WSTAG521_800A65E0, 49, 5, 321, 193, 3 };
FieldstgPlacedActor D_WSTAG521_800A67F0 = { D_WSTAG521_800A6748, D_WSTAG521_800A65F8, 57, 6, 241, 217, 3 };
FieldstgPlacedActor D_WSTAG521_800A6804 = { D_WSTAG521_800A6754, D_WSTAG521_800A6610, 64, 7, 241, 217, 3 };
FieldstgPlacedActor D_WSTAG521_800A6818 = { D_WSTAG521_800A675C, D_WSTAG521_800A6628, 65, 8, 517, 289, 3 };
FieldstgPlacedActor D_WSTAG521_800A682C = { D_WSTAG521_800A6764, D_WSTAG521_800A6640, 66, 9, 321, 193, 3 };
FieldstgPlacedActor D_WSTAG521_800A6840 = { D_WSTAG521_800A676C, D_WSTAG521_800A6658, 144, 10, 417, 201, 3 };
FieldstgPlacedActor D_WSTAG521_800A6854 = { NULL, NULL, 147, 11, 198, 184, 6 };
FieldstgPlacedActor D_WSTAG521_800A6868 = { NULL, NULL, 148, 12, 223, 172, 7 };
FieldstgPlacedActor D_WSTAG521_800A687C = { NULL, NULL, 149, 13, 248, 160, 7 };
FieldstgPlacedActor D_WSTAG521_800A6890 = { D_WSTAG521_800A6774, D_WSTAG521_800A6670, 157, 14, 241, 217, 3 };
FieldstgPlacedActor D_WSTAG521_800A68A4 = { D_WSTAG521_800A6780, D_WSTAG521_800A6688, 157, 14, 241, 217, 3 };
FieldstgPlacedActor D_WSTAG521_800A68B8 = { D_WSTAG521_800A6788, D_WSTAG521_800A66A0, 158, 15, 321, 193, 3 };
FieldstgPlacedActor D_WSTAG521_800A68CC = { D_WSTAG521_800A6794, D_WSTAG521_800A66B8, 158, 15, 321, 193, 3 };
FieldstgPlacedActor D_WSTAG521_800A68E0 = { D_WSTAG521_800A679C, D_WSTAG521_800A66D0, 159, 16, 517, 289, 1 };
FieldstgPlacedActor D_WSTAG521_800A68F4 = { D_WSTAG521_800A67A8, D_WSTAG521_800A66E8, 159, 16, 517, 289, 1 };
FieldstgPlacedActor D_WSTAG521_800A6908 = { D_WSTAG521_800A67B0, D_WSTAG521_800A6700, 210, 17, 417, 201, 3 };
FieldstgPlacedActor D_WSTAG521_800A691C = { D_WSTAG521_800A67B8, D_WSTAG521_800A6718, 281, 18, 520, 252, 3 };
FieldstgPlacedActor D_WSTAG521_800A6930 = { D_WSTAG521_800A67C0, NULL, 316, 19, 0, 0, 1 };
FieldstgPlacedActor *wstag521_actors[20] = {
    &D_WSTAG521_800A67C8, &D_WSTAG521_800A67DC, &D_WSTAG521_800A67F0, &D_WSTAG521_800A6804, &D_WSTAG521_800A6818,
    &D_WSTAG521_800A682C, &D_WSTAG521_800A6840, &D_WSTAG521_800A6854, &D_WSTAG521_800A6868, &D_WSTAG521_800A687C,
    &D_WSTAG521_800A6890, &D_WSTAG521_800A68A4, &D_WSTAG521_800A68B8, &D_WSTAG521_800A68CC, &D_WSTAG521_800A68E0,
    &D_WSTAG521_800A68F4, &D_WSTAG521_800A6908, &D_WSTAG521_800A691C, &D_WSTAG521_800A6930, NULL,
};
FieldstgSprite wstag521_sprites[16] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 323, 114, 0, 0 }, { 1, 0, 0xA0, 6, 0xA, 0, 0, 0, 0, 0, 367, 110, 0, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 205, 181, 199, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 237, 165, 183, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 270, 149, 167, 0 }, { 1, 0, 0x40, 4, 0x33, 2, 0, 1, 4, 0, 186, 161, 199, 0 },
    { 1, 0, 0x40, 4, 0x33, 2, 0, 1, 4, 0, 218, 145, 183, 0 },
    { 1, 0, 0x40, 4, 0x33, 2, 0, 1, 4, 0, 252, 129, 167, 0 }, { 1, 0, 0x58, 4, 0, 0, 0, 0, 0, 0, 521, 189, 272, 0 },
    { 1, 0, 0xA0, 4, 1, 0, 0, 0, 0, 0, 367, 110, 256, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 230, 243, 272, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 259, 221, 255, 0 }, { 1, 0, 0x58, 4, 4, 0, 0, 0, 0, 0, 171, 118, 198, 0 },
    { 1, 0, 0x55, 4, 5, 0, 0, 0, 0, 0, 138, 106, 182, 0 }, { 1, 0, 0x5C, 4, 6, 0, 0, 0, 0, 0, 263, 68, 153, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag521_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AB, 0x32A, 0x9C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2AB, 0x238, 0x8C, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x1CE, 0xE6, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x1BE, 0x12E, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x220, 0x150, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x230, 0x1B8, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag521_funcs = { wstag521_setup };
FieldstgEventDef wstag521_events[3] = {
    { 750, D_WSTAG521_800A609C, 0x013C0022, NULL, wstag521_event_750_end },
    { 751, D_WSTAG521_800A6180, 0x013C001E, NULL, wstag521_event_751_end }, { -1, NULL, 0, NULL, NULL },
};
