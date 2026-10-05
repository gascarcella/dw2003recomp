#include "wstag.h"

/* WSTAG530: stage 0x242 (fieldstg_stages). */

extern WstagFuncs wstag530_funcs;
extern FieldstgBattleLists wstag530_battle_lists;
extern FieldstgVramPlace wstag530_vram_places[];
extern FieldstgPlacedActor *wstag530_actors[];
extern FieldstgSprite wstag530_sprites[];
extern FieldstgMapEvent wstag530_map_events[];
extern FieldstgMapEvent wstag530_map_events2[];
extern FieldstgEventDef wstag530_events[];

void wstag530_update(WstagObject *obj) {
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

WstagObject *wstag530_start(void *arg0) {
    WstagObject *obj = object_new(wstag530_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag530_funcs.setup();
    return obj;
}

void wstag530_setup(void) {
    fieldstg_stage.background_file = 0x278;
    fieldstg_stage.sprite_file = 0x02790000;
    fieldstg_stage.sprites = wstag530_sprites;
    fieldstg_stage.mask_file = 0x3DD;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10800, 0xED00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag530_vram_places;
    fieldstg_stage.music = 0x11;
    fieldstg_stage.sound = 0x60440000;
    fieldstg_stage.actors = wstag530_actors;
    fieldstg_stage.events = wstag530_events;
    fieldstg_stage.battle_lists = &wstag530_battle_lists;
    fieldstg_attr.set_file(0, 0x02790001);
    fieldstg_attr.set_file(7, 0x02790002);
    fieldstg_attr.set_file(4, 0x02790003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress < 0x1A) {
        fieldstg_stage.map_events = wstag530_map_events;
    } else {
        fieldstg_stage.map_events = wstag530_map_events2;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag530_setup(void);

FieldstgListedBattle D_WSTAG530_800A5FC4 = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A5FD0 = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A5FDC = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A5FE8 = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A5FF4 = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6000 = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A600C = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6018 = { 165, 6, 0x60080000 };
FieldstgBattleList D_WSTAG530_800A6024 = {
    5,
    { &D_WSTAG530_800A5FC4, &D_WSTAG530_800A5FD0, &D_WSTAG530_800A5FDC, &D_WSTAG530_800A5FE8, &D_WSTAG530_800A5FF4,
        &D_WSTAG530_800A6000, &D_WSTAG530_800A600C, &D_WSTAG530_800A6018 },
};
FieldstgListedBattle D_WSTAG530_800A6048 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6054 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6060 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A606C = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6078 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6084 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6090 = { 0, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A609C = { 0, 6, 0x60080000 };
FieldstgBattleList D_WSTAG530_800A60A8 = {
    0,
    { &D_WSTAG530_800A6048, &D_WSTAG530_800A6054, &D_WSTAG530_800A6060, &D_WSTAG530_800A606C, &D_WSTAG530_800A6078,
        &D_WSTAG530_800A6084, &D_WSTAG530_800A6090, &D_WSTAG530_800A609C },
};
FieldstgListedBattle D_WSTAG530_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A60F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A60FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6120 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG530_800A612C = {
    0,
    { &D_WSTAG530_800A60CC, &D_WSTAG530_800A60D8, &D_WSTAG530_800A60E4, &D_WSTAG530_800A60F0, &D_WSTAG530_800A60FC,
        &D_WSTAG530_800A6108, &D_WSTAG530_800A6114, &D_WSTAG530_800A6120 },
};
FieldstgListedBattle D_WSTAG530_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6174 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A6180 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A618C = { 165, 6, 0x60080000 };
FieldstgListedBattle D_WSTAG530_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG530_800A61A4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG530_800A61B0 = {
    0,
    { &D_WSTAG530_800A6150, &D_WSTAG530_800A615C, &D_WSTAG530_800A6168, &D_WSTAG530_800A6174, &D_WSTAG530_800A6180,
        &D_WSTAG530_800A618C, &D_WSTAG530_800A6198, &D_WSTAG530_800A61A4 },
};
FieldstgBattleLists wstag530_battle_lists = {
    58, 0, 0, { &D_WSTAG530_800A6024, &D_WSTAG530_800A60A8, &D_WSTAG530_800A612C }, &D_WSTAG530_800A61B0,
};
FieldstgVramPlace wstag530_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 256, 224, 0, 336, 506 },
};
u16 D_WSTAG530_800A6260[8] = { 0x205, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG530_800A6270[2] = { { NULL, D_WSTAG530_800A6260, 365 }, { NULL, NULL, 0 } };
u16 D_WSTAG530_800A6288[4] = { 0x205, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG530_800A6290 = { D_WSTAG530_800A6288, D_WSTAG530_800A6270, 33, 4, 671, 176, 1 };
FieldstgPlacedActor *wstag530_actors[2] = { &D_WSTAG530_800A6290, NULL };
FieldstgSprite wstag530_sprites[41] = {
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
FieldstgMapEvent wstag530_map_events[5] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23C, 0x3C8, 0xC4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x243, 0x98, 0x8C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x221, 0xF0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x211, 0x158, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag530_map_events2[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x23C, 0x3C8, 0xC4, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x244, 0x98, 0x8C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x221, 0xF0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x211, 0x158, 0, 0, 0, 0 }, { 0xF, 0, 0xFFFF, 0, 8, 0x2328, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag530_funcs = { wstag530_setup };
FieldstgEventDef wstag530_events[2] = {
    { 9000, NULL, 0, fieldstg_start_battle_5, NULL }, { -1, NULL, 0, NULL, NULL },
};
