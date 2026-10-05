#include "wstag.h"

/* WSTAG650: stage 0x25B (fieldstg_stages). */

extern WstagFuncs wstag650_funcs;
extern FieldstgBattleLists wstag650_battle_lists;
extern FieldstgVramPlace wstag650_vram_places[];
extern FieldstgPlacedActor *wstag650_actors[];
extern FieldstgSprite wstag650_sprites[];
extern FieldstgMapEvent wstag650_map_events[];
extern FieldstgEventDef wstag650_events[];

void wstag650_update(WstagObject *obj) {
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

WstagObject *wstag650_start(void *arg0) {
    WstagObject *obj = object_new(wstag650_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag650_funcs.setup();
    return obj;
}

void wstag650_event_1458_end(void) {
    gamestate_flags.set_flag(0x7C15, 1);
}

void wstag650_setup(void) {
    fieldstg_stage.background_file = 0x54D;
    fieldstg_stage.sprite_file = 0x054E0000;
    fieldstg_stage.sprites = wstag650_sprites;
    fieldstg_stage.map_events = wstag650_map_events;
    fieldstg_stage.mask_file = 0x54C;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0xEF00, 0x25800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag650_vram_places;
    fieldstg_stage.music = 0x16;
    fieldstg_stage.sound = 0x60580000;
    fieldstg_stage.actors = wstag650_actors;
    fieldstg_stage.battle_lists = &wstag650_battle_lists;
    fieldstg_stage.events = wstag650_events;
    fieldstg_attr.set_file(0, 0x054E0001);
    fieldstg_attr.set_file(7, 0x054E0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0xF && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag650_setup(void);

s16 D_WSTAG650_800A5FFC[58] = {
    FIELDSTG_EVENT_WALK(2, 215, 297, 3),
    FIELDSTG_EVENT_PLACE(21, 183, 281),
    FIELDSTG_EVENT_ANIM(21, 1, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 7),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC13, 0, 0, 0),
    FIELDSTG_EVENT_END,
    0x323, /* padding, not read */
};
FieldstgListedBattle D_WSTAG650_800A6070 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A607C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6088 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6094 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A60AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A60B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A60C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG650_800A60D0 = {
    3,
    { &D_WSTAG650_800A6070, &D_WSTAG650_800A607C, &D_WSTAG650_800A6088, &D_WSTAG650_800A6094, &D_WSTAG650_800A60A0,
        &D_WSTAG650_800A60AC, &D_WSTAG650_800A60B8, &D_WSTAG650_800A60C4 },
};
FieldstgListedBattle D_WSTAG650_800A60F4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6100 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A610C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6118 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A613C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6148 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG650_800A6154 = {
    0,
    { &D_WSTAG650_800A60F4, &D_WSTAG650_800A6100, &D_WSTAG650_800A610C, &D_WSTAG650_800A6118, &D_WSTAG650_800A6124,
        &D_WSTAG650_800A6130, &D_WSTAG650_800A613C, &D_WSTAG650_800A6148 },
};
FieldstgListedBattle D_WSTAG650_800A6178 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6184 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6190 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A619C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A61A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A61B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A61C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A61CC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG650_800A61D8 = {
    0,
    { &D_WSTAG650_800A6178, &D_WSTAG650_800A6184, &D_WSTAG650_800A6190, &D_WSTAG650_800A619C, &D_WSTAG650_800A61A8,
        &D_WSTAG650_800A61B4, &D_WSTAG650_800A61C0, &D_WSTAG650_800A61CC },
};
FieldstgListedBattle D_WSTAG650_800A61FC = { 216, 20, 0x600C0000 };
FieldstgListedBattle D_WSTAG650_800A6208 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6214 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6220 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A622C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6238 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6244 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG650_800A6250 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG650_800A625C = {
    0,
    { &D_WSTAG650_800A61FC, &D_WSTAG650_800A6208, &D_WSTAG650_800A6214, &D_WSTAG650_800A6220, &D_WSTAG650_800A622C,
        &D_WSTAG650_800A6238, &D_WSTAG650_800A6244, &D_WSTAG650_800A6250 },
};
FieldstgBattleLists wstag650_battle_lists = {
    158, 0, 0, { &D_WSTAG650_800A60D0, &D_WSTAG650_800A6154, &D_WSTAG650_800A61D8 }, &D_WSTAG650_800A625C,
};
FieldstgVramPlace wstag650_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 356, 423, 144, 167, 352, 511 }, { 320, 256, 336, 376, 64, 120, 368, 511 },
    { 320, 256, 320, 395, 0, 139, 352, 510 }, { 320, 256, 364, 421, 176, 165, 368, 510 },
    { 320, 256, 372, 421, 208, 165, 352, 509 }, { 320, 256, 348, 423, 112, 167, 368, 509 },
    { 320, 256, 330, 424, 40, 168, 352, 508 },
};
u16 D_WSTAG650_800A636C[4] = { 0x7A2D, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6374[4] = { 0x9012, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A637C[4] = { 0x7A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6384[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A638C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6394[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A63A0[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A63AC[4] = { 0x761A, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A63B4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A63BC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A63C8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A63D0[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A63DC[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A63E8[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A63F0[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A63F8[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6404[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6410[4] = { 0x761A, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6418[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6420[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6428[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6434[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6440[4] = { 0x761A, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6448[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6450[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6458[6] = { 0, 1, 0xE10, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6464[6] = { 0x7400, 1, 0xE10, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6470[8] = { 0, 1, 0xE10, 1, 0x720D, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6480[8] = { 0, 1, 0xE10, 1, 0x720D, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6490[4] = { 0x781A, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6498[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A64A0[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A64AC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A64B4[8] = { 0x10, 1, 0x9201, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A64C4[10] = {
    0x9201, 1, 0x11, 0, 0x7013, 1, 0x10, 0,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A64D8[8] = { 0x10, 1, 0x9201, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A64E8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A64F4[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A64FC[6] = { 0, 1, 0x7209, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A6508[6] = { 0, 1, 0x7209, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6514[4] = { 0x761A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG650_800A651C[2] = { { NULL, D_WSTAG650_800A636C, 363 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A6534[2] = { { NULL, D_WSTAG650_800A6374, 360 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A654C[2] = { { NULL, D_WSTAG650_800A637C, 727 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A6564[2] = { { NULL, NULL, 39 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A657C[2] = { { NULL, NULL, 351 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A6594[2] = { { NULL, NULL, 352 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A65AC[2] = { { NULL, NULL, 353 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A65C4[2] = { { NULL, NULL, 352 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A65DC[2] = { { NULL, NULL, 40 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A65F4[2] = { { NULL, NULL, 348 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A660C[2] = { { NULL, NULL, 349 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A6624[2] = { { NULL, NULL, 349 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A663C[2] = { { NULL, NULL, 350 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A6654[4] = {
    { D_WSTAG650_800A6384, D_WSTAG650_800A638C, 205 }, { D_WSTAG650_800A6394, NULL, 209 },
    { D_WSTAG650_800A63A0, D_WSTAG650_800A63AC, 210 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A6684[4] = {
    { D_WSTAG650_800A63B4, NULL, 205 }, { D_WSTAG650_800A63BC, D_WSTAG650_800A63C8, 215 },
    { D_WSTAG650_800A63D0, D_WSTAG650_800A63DC, 216 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A66B4[4] = {
    { D_WSTAG650_800A63E8, D_WSTAG650_800A63F0, 206 }, { D_WSTAG650_800A63F8, NULL, 209 },
    { D_WSTAG650_800A6404, D_WSTAG650_800A6410, 210 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A66E4[4] = {
    { D_WSTAG650_800A6418, D_WSTAG650_800A6420, 207 }, { D_WSTAG650_800A6428, NULL, 209 },
    { D_WSTAG650_800A6434, D_WSTAG650_800A6440, 210 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A6714[5] = {
    { D_WSTAG650_800A6448, D_WSTAG650_800A6450, 211 }, { D_WSTAG650_800A6458, D_WSTAG650_800A6464, 212 },
    { D_WSTAG650_800A6470, NULL, 213 }, { D_WSTAG650_800A6480, D_WSTAG650_800A6490, 214 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A6750[5] = {
    { D_WSTAG650_800A6498, NULL, 205 }, { D_WSTAG650_800A64A0, D_WSTAG650_800A64AC, 217 },
    { D_WSTAG650_800A64B4, D_WSTAG650_800A64C4, 218 }, { D_WSTAG650_800A64D8, D_WSTAG650_800A64E8, 219 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG650_800A678C[2] = { { NULL, NULL, 644 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG650_800A67A4[4] = {
    { D_WSTAG650_800A64F4, NULL, 208 }, { D_WSTAG650_800A64FC, NULL, 208 },
    { D_WSTAG650_800A6508, D_WSTAG650_800A6514, 208 }, { NULL, NULL, 0 },
};
u16 D_WSTAG650_800A67D4[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A67DC[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A67E4[4] = { 0x6011, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A67EC[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A67F4[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A67FC[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6804[4] = { 0x6010, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A680C[4] = { 0x6011, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6814[4] = { 0x6012, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A681C[4] = { 0x7017, 1, 0xFFFF, 0 };
u16 D_WSTAG650_800A6824[10] = {
    0x11, 0, 0x7003, 1, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A6838[10] = {
    0x8192, 1, 0x11, 1, 0x700A, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A684C[10] = {
    0x8192, 1, 0x11, 0, 0x7004, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A6860[10] = {
    0x8192, 1, 0x11, 0, 0x6026, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A6874[10] = {
    0x8192, 1, 0x11, 0, 0x8012, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A6888[10] = {
    0x8192, 1, 0x11, 1, 0x8012, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG650_800A689C[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG650_800A68AC[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG650_800A68B4 = { NULL, D_WSTAG650_800A651C, 20, 4, 321, 593, 1 };
FieldstgPlacedActor D_WSTAG650_800A68C8 = { NULL, D_WSTAG650_800A6534, 21, 5, 183, 281, 7 };
FieldstgPlacedActor D_WSTAG650_800A68DC = { NULL, D_WSTAG650_800A654C, 23, 6, 528, 305, 7 };
FieldstgPlacedActor D_WSTAG650_800A68F0 = { D_WSTAG650_800A67D4, D_WSTAG650_800A6564, 32, 7, 353, 442, 1 };
FieldstgPlacedActor D_WSTAG650_800A6904 = { D_WSTAG650_800A67DC, D_WSTAG650_800A657C, 32, 7, 353, 442, 1 };
FieldstgPlacedActor D_WSTAG650_800A6918 = { D_WSTAG650_800A67E4, D_WSTAG650_800A6594, 32, 7, 353, 442, 1 };
FieldstgPlacedActor D_WSTAG650_800A692C = { D_WSTAG650_800A67EC, D_WSTAG650_800A65AC, 32, 7, 353, 442, 1 };
FieldstgPlacedActor D_WSTAG650_800A6940 = { D_WSTAG650_800A67F4, D_WSTAG650_800A65C4, 32, 7, 353, 442, 1 };
FieldstgPlacedActor D_WSTAG650_800A6954 = { D_WSTAG650_800A67FC, D_WSTAG650_800A65DC, 48, 8, 537, 493, 7 };
FieldstgPlacedActor D_WSTAG650_800A6968 = { D_WSTAG650_800A6804, D_WSTAG650_800A65F4, 48, 8, 537, 493, 7 };
FieldstgPlacedActor D_WSTAG650_800A697C = { D_WSTAG650_800A680C, D_WSTAG650_800A660C, 48, 8, 537, 493, 7 };
FieldstgPlacedActor D_WSTAG650_800A6990 = { D_WSTAG650_800A6814, D_WSTAG650_800A6624, 48, 8, 537, 493, 7 };
FieldstgPlacedActor D_WSTAG650_800A69A4 = { D_WSTAG650_800A681C, D_WSTAG650_800A663C, 48, 8, 537, 493, 7 };
FieldstgPlacedActor D_WSTAG650_800A69B8 = { D_WSTAG650_800A6824, D_WSTAG650_800A6654, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A69CC = { D_WSTAG650_800A6838, D_WSTAG650_800A6684, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A69E0 = { D_WSTAG650_800A684C, D_WSTAG650_800A66B4, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A69F4 = { D_WSTAG650_800A6860, D_WSTAG650_800A66E4, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A6A08 = { D_WSTAG650_800A6874, D_WSTAG650_800A6714, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A6A1C = { D_WSTAG650_800A6888, D_WSTAG650_800A6750, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A6A30 = { D_WSTAG650_800A689C, D_WSTAG650_800A678C, 51, 9, 401, 529, 7 };
FieldstgPlacedActor D_WSTAG650_800A6A44 = { D_WSTAG650_800A68AC, D_WSTAG650_800A67A4, 157, 10, 401, 529, 7 };
FieldstgPlacedActor *wstag650_actors[22] = {
    &D_WSTAG650_800A68B4, &D_WSTAG650_800A68C8, &D_WSTAG650_800A68DC, &D_WSTAG650_800A68F0, &D_WSTAG650_800A6904,
    &D_WSTAG650_800A6918, &D_WSTAG650_800A692C, &D_WSTAG650_800A6940, &D_WSTAG650_800A6954, &D_WSTAG650_800A6968,
    &D_WSTAG650_800A697C, &D_WSTAG650_800A6990, &D_WSTAG650_800A69A4, &D_WSTAG650_800A69B8, &D_WSTAG650_800A69CC,
    &D_WSTAG650_800A69E0, &D_WSTAG650_800A69F4, &D_WSTAG650_800A6A08, &D_WSTAG650_800A6A1C, &D_WSTAG650_800A6A30,
    &D_WSTAG650_800A6A44, NULL,
};
FieldstgSprite wstag650_sprites[20] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 7, 0xE, 0, 252, 269, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 7, 0xE, 0, 482, 625, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 0xE, 0, 195, 353, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 0xE, 0, 339, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 7, 0xE, 0, 539, 364, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 7, 0xE, 0, 366, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 7, 0xE, 0, 475, 430, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 2, 0, 0xB, 0xA, 0, 164, 326, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 0xB, 0xA, 0, 164, 354, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 0xB, 0xA, 0, 508, 337, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 0xB, 0xA, 0, 512, 364, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 2, 0, 0xB, 0xA, 0, 568, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 0xB, 0xA, 0, 481, 426, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 0xB, 0xA, 0, 453, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 0xB, 0xA, 0, 346, 480, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 2, 0, 0xB, 0xA, 0, 342, 488, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 311, 96, 151, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 318, 579, 600, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 335, 572, 595, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag650_map_events[17] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25C, 0xA8, 0x1B4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25A, 0x298, 0x104, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0xFD, 0xD0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0xEC, 0x118, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x133, 0xC8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x142, 0x150, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x192, 0x148, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x1A2, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x1C2, 0x1A0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x1D2, 0x1E8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x25C, 0x160, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x24C, 0x1A8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x1AC, 0x228, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x19C, 0x270, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 5, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 9, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag650_funcs = { wstag650_setup };
FieldstgEventDef wstag650_events[2] = {
    { 1458, D_WSTAG650_800A5FFC, 0x0143001E, NULL, wstag650_event_1458_end }, { -1, NULL, 0, NULL, NULL },
};
