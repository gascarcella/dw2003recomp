#include "wstag.h"

/* WSTAG531: stage 0x2AF (fieldstg_stages). */

extern WstagFuncs wstag531_funcs;
extern FieldstgBattleLists wstag531_battle_lists;
extern FieldstgVramPlace wstag531_vram_places[];
extern FieldstgPlacedActor *wstag531_actors[];
extern FieldstgSprite wstag531_sprites[];
extern FieldstgMapEvent wstag531_map_events[];
extern FieldstgEventDef wstag531_events[];

void wstag531_update(WstagObject *obj) {
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

WstagObject *wstag531_start(void *arg0) {
    WstagObject *obj = object_new(wstag531_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag531_funcs.setup();
    return obj;
}

void wstag531_setup(void) {
    fieldstg_stage.background_file = 0x74D;
    fieldstg_stage.sprite_file = 0x074E0000;
    fieldstg_stage.sprites = wstag531_sprites;
    fieldstg_stage.map_events = wstag531_map_events;
    fieldstg_stage.mask_file = 0x74C;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFB00, 0xED00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag531_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.actors = wstag531_actors;
    fieldstg_stage.events = wstag531_events;
    fieldstg_stage.battle_lists = &wstag531_battle_lists;
    fieldstg_attr.set_file(0, 0x074E0001);
    fieldstg_attr.set_file(7, 0x074E0002);
    fieldstg_attr.set_file(4, 0x074E0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag531_setup(void);

FieldstgListedBattle D_WSTAG531_800A5F98 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FA4 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FB0 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FBC = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FC8 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FD4 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FE0 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A5FEC = { 166, 6, 0x60080000 };
FieldstgBattleList D_WSTAG531_800A5FF8 = {
    5,
    { &D_WSTAG531_800A5F98, &D_WSTAG531_800A5FA4, &D_WSTAG531_800A5FB0, &D_WSTAG531_800A5FBC, &D_WSTAG531_800A5FC8,
        &D_WSTAG531_800A5FD4, &D_WSTAG531_800A5FE0, &D_WSTAG531_800A5FEC },
};
FieldstgListedBattle D_WSTAG531_800A601C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6028 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6034 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6040 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A604C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6058 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6064 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A6070 = { 0, 6, 0x60080000 };
FieldstgBattleList D_WSTAG531_800A607C = {
    0,
    { &D_WSTAG531_800A601C, &D_WSTAG531_800A6028, &D_WSTAG531_800A6034, &D_WSTAG531_800A6040, &D_WSTAG531_800A604C,
        &D_WSTAG531_800A6058, &D_WSTAG531_800A6064, &D_WSTAG531_800A6070 },
};
FieldstgListedBattle D_WSTAG531_800A60A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60E8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A60F4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG531_800A6100 = {
    0,
    { &D_WSTAG531_800A60A0, &D_WSTAG531_800A60AC, &D_WSTAG531_800A60B8, &D_WSTAG531_800A60C4, &D_WSTAG531_800A60D0,
        &D_WSTAG531_800A60DC, &D_WSTAG531_800A60E8, &D_WSTAG531_800A60F4 },
};
FieldstgListedBattle D_WSTAG531_800A6124 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A6130 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A613C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A6148 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A6154 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A6160 = { 166, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG531_800A616C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG531_800A6178 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG531_800A6184 = {
    0,
    { &D_WSTAG531_800A6124, &D_WSTAG531_800A6130, &D_WSTAG531_800A613C, &D_WSTAG531_800A6148, &D_WSTAG531_800A6154,
        &D_WSTAG531_800A6160, &D_WSTAG531_800A616C, &D_WSTAG531_800A6178 },
};
FieldstgBattleLists wstag531_battle_lists = {
    79, 0, 0, { &D_WSTAG531_800A5FF8, &D_WSTAG531_800A607C, &D_WSTAG531_800A6100 }, &D_WSTAG531_800A6184,
};
FieldstgVramPlace wstag531_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 336, 506 }, { 320, 256, 360, 320, 160, 64, 352, 506 },
    { 320, 256, 320, 349, 0, 93, 368, 506 },
};
u16 D_WSTAG531_800A6254[8] = { 0x21E, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG531_800A6264[2] = { { NULL, D_WSTAG531_800A6254, 382 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG531_800A627C[2] = { { NULL, NULL, 177 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG531_800A6294[2] = { { NULL, NULL, 179 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG531_800A62AC[2] = { { NULL, NULL, 180 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG531_800A62C4[2] = { { NULL, NULL, 178 }, { NULL, NULL, 0 } };
u16 D_WSTAG531_800A62DC[4] = { 0x21E, 0, 0xFFFF, 0 };
u16 D_WSTAG531_800A62E4[4] = { 0x701D, 1, 0xFFFF, 0 };
u16 D_WSTAG531_800A62EC[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG531_800A62F4[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG531_800A62FC[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG531_800A6304 = { D_WSTAG531_800A62DC, D_WSTAG531_800A6264, 33, 4, 671, 176, 1 };
FieldstgPlacedActor D_WSTAG531_800A6318 = { D_WSTAG531_800A62E4, D_WSTAG531_800A627C, 69, 5, 402, 153, 5 };
FieldstgPlacedActor D_WSTAG531_800A632C = { D_WSTAG531_800A62EC, D_WSTAG531_800A6294, 69, 5, 402, 153, 5 };
FieldstgPlacedActor D_WSTAG531_800A6340 = { D_WSTAG531_800A62F4, D_WSTAG531_800A62AC, 69, 5, 402, 153, 5 };
FieldstgPlacedActor D_WSTAG531_800A6354 = { D_WSTAG531_800A62FC, D_WSTAG531_800A62C4, 157, 6, 402, 153, 5 };
FieldstgPlacedActor *wstag531_actors[6] = {
    &D_WSTAG531_800A6304, &D_WSTAG531_800A6318, &D_WSTAG531_800A632C, &D_WSTAG531_800A6340, &D_WSTAG531_800A6354,
    NULL,
};
FieldstgSprite wstag531_sprites[41] = {
    { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 477, 371, 0, 0 }, { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 508, 386, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 512, 353, 0, 0 }, { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 543, 368, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 574, 419, 0, 0 }, { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 604, 434, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 608, 401, 0, 0 }, { 1, 0, 0x40, 2, 0x39, 2, 0, 3, 4, 0, 639, 416, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 438, 68, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 469, 367, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 500, 383, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 502, 120, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 504, 349, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 522, 110, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 535, 364, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 566, 415, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 596, 431, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 600, 397, 0, 0 },
    { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 631, 412, 0, 0 }, { 1, 0, 0x40, 2, 0x3A, 2, 0, 3, 4, 0, 822, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 6, 0, 788, 446, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 6, 0, 768, 423, 0, 0 }, { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 426, 82, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 446, 72, 0, 0 }, { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 510, 124, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 530, 114, 0, 0 }, { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 747, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 830, 392, 0, 0 }, { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 830, 475, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 4, 0, 913, 433, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 4, 0, 418, 79, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 4, 0, 739, 430, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 4, 0, 822, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 4, 0, 905, 430, 0, 0 }, { 1, 0, 0x40, 6, 0x3B, 2, 0, 3, 4, 0, 514, 60, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 3, 4, 0, 510, 56, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 4, 0, 429, 28, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 3, 4, 0, 423, 20, 0, 0 }, { 1, 0, 0x50, 4, 0, 0, 0, 0, 0, 0, 474, 362, 406, 0 },
    { 1, 0, 0x50, 4, 1, 0, 0, 0, 0, 0, 571, 410, 456, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag531_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2A9, 0x3C8, 0xC4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B0, 0x98, 0x8C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x221, 0xF0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x211, 0x158, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag531_funcs = { wstag531_setup };
FieldstgEventDef wstag531_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
