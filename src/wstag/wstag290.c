#include "wstag.h"

/* WSTAG290: stage 0x215 (fieldstg_stages). */

extern WstagFuncs wstag290_funcs;
extern FieldstgBattleLists wstag290_battle_lists;
extern FieldstgVramPlace wstag290_vram_places[];
extern FieldstgPlacedActor *wstag290_actors[];
extern FieldstgSprite wstag290_sprites[];
extern FieldstgMapEvent wstag290_map_events[];

void wstag290_update(WstagObject *obj) {
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

WstagObject *wstag290_start(void *arg0) {
    WstagObject *obj = object_new(wstag290_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag290_funcs.setup();
    return obj;
}

void wstag290_setup(void) {
    fieldstg_stage.background_file = 0x2AB;
    fieldstg_stage.sprite_file = 0x02AC0000;
    fieldstg_stage.sprites = wstag290_sprites;
    fieldstg_stage.map_events = wstag290_map_events;
    fieldstg_stage.mask_file = 0x32E;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1E000, 0x1D400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag290_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag290_actors;
    fieldstg_stage.battle_lists = &wstag290_battle_lists;
    fieldstg_attr.set_file(0, 0x02AC0001);
    fieldstg_attr.set_file(7, 0x02AC0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag290_setup(void);

FieldstgListedBattle D_WSTAG290_800A5F7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5F88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5F94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5FA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5FAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5FB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG290_800A5FDC = {
    0,
    { &D_WSTAG290_800A5F7C, &D_WSTAG290_800A5F88, &D_WSTAG290_800A5F94, &D_WSTAG290_800A5FA0, &D_WSTAG290_800A5FAC,
        &D_WSTAG290_800A5FB8, &D_WSTAG290_800A5FC4, &D_WSTAG290_800A5FD0 },
};
FieldstgListedBattle D_WSTAG290_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6054 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG290_800A6060 = {
    0,
    { &D_WSTAG290_800A6000, &D_WSTAG290_800A600C, &D_WSTAG290_800A6018, &D_WSTAG290_800A6024, &D_WSTAG290_800A6030,
        &D_WSTAG290_800A603C, &D_WSTAG290_800A6048, &D_WSTAG290_800A6054 },
};
FieldstgListedBattle D_WSTAG290_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A60D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG290_800A60E4 = {
    0,
    { &D_WSTAG290_800A6084, &D_WSTAG290_800A6090, &D_WSTAG290_800A609C, &D_WSTAG290_800A60A8, &D_WSTAG290_800A60B4,
        &D_WSTAG290_800A60C0, &D_WSTAG290_800A60CC, &D_WSTAG290_800A60D8 },
};
FieldstgListedBattle D_WSTAG290_800A6108 = { 189, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG290_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG290_800A615C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG290_800A6168 = {
    0,
    { &D_WSTAG290_800A6108, &D_WSTAG290_800A6114, &D_WSTAG290_800A6120, &D_WSTAG290_800A612C, &D_WSTAG290_800A6138,
        &D_WSTAG290_800A6144, &D_WSTAG290_800A6150, &D_WSTAG290_800A615C },
};
FieldstgBattleLists wstag290_battle_lists = {
    135, 0, 0, { &D_WSTAG290_800A5FDC, &D_WSTAG290_800A6060, &D_WSTAG290_800A60E4 }, &D_WSTAG290_800A6168,
};
FieldstgVramPlace wstag290_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 441, 208, 185, 320, 497 }, { 384, 256, 396, 256, 304, 0, 336, 497 },
    { 384, 256, 404, 256, 336, 0, 368, 497 },
};
u16 D_WSTAG290_800A6238[6] = { 0x7400, 1, 0xC08, 1, 0xFFFF, 0 };
u16 D_WSTAG290_800A6244[6] = { 0x7400, 1, 0xC09, 1, 0xFFFF, 0 };
u16 D_WSTAG290_800A6250[6] = { 0x7400, 1, 0xC0A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG290_800A625C[2] = { { NULL, D_WSTAG290_800A6238, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG290_800A6274[2] = { { NULL, D_WSTAG290_800A6244, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG290_800A628C[2] = { { NULL, D_WSTAG290_800A6250, 169 }, { NULL, NULL, 0 } };
u16 D_WSTAG290_800A62A4[6] = { 0x6016, 1, 0xC08, 0, 0xFFFF, 0 };
u16 D_WSTAG290_800A62B0[6] = { 0x6016, 1, 0xC09, 0, 0xFFFF, 0 };
u16 D_WSTAG290_800A62BC[6] = { 0x6016, 1, 0xC0A, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG290_800A62C8 = { D_WSTAG290_800A62A4, D_WSTAG290_800A625C, 306, 4, 209, 353, 7 };
FieldstgPlacedActor D_WSTAG290_800A62DC = { D_WSTAG290_800A62B0, D_WSTAG290_800A6274, 307, 5, 419, 250, 1 };
FieldstgPlacedActor D_WSTAG290_800A62F0 = { D_WSTAG290_800A62BC, D_WSTAG290_800A628C, 308, 6, 544, 249, 7 };
FieldstgPlacedActor *wstag290_actors[4] = {
    &D_WSTAG290_800A62C8, &D_WSTAG290_800A62DC, &D_WSTAG290_800A62F0, NULL,
};
FieldstgSprite wstag290_sprites[35] = {
    { 1, 0, 0x40, 2, 0x40, 2, 0, 5, 8, 0, 171, 28, 0, 0 }, { 1, 0, 0x40, 2, 0x40, 2, 0, 5, 8, 0, 549, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 264, 45, 0, 0 }, { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 361, 94, 0, 0 },
    { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 614, 108, 0, 0 }, { 1, 0, 0x40, 2, 0x41, 2, 0, 5, 8, 0, 699, 185, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 0xD, 0x10, 0, 467, 144, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 0xD, 0x10, 0, 467, 144, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 0xD, 0x10, 0, 676, 280, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 0xD, 0x10, 0, 676, 280, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 8, 0x10, 0, 81, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x37, 2, 0, 8, 0x10, 0, 81, 194, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 8, 0x10, 0, 789, 254, 0, 0 },
    { 1, 0, 0x40, 2, 0x39, 2, 0, 8, 0x10, 0, 789, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 166, 188, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 262, 236, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 294, 252, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 326, 268, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 358, 284, 0, 0 }, { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 390, 300, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 6, 0, 421, 316, 0, 0 }, { 1, 0, 0x40, 6, 0x3B, 2, 0, 3, 6, 0, 197, 203, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 2, 0, 3, 6, 0, 229, 217, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 6, 0, 451, 328, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 6, 0, 487, 328, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 3, 6, 0, 517, 321, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 2, 0, 3, 6, 0, 549, 305, 0, 0 }, { 1, 0x64, 0x40, 6, 5, 0, 0, 0, 0, 0, 81, 237, 0, 0 },
    { 1, 0x65, 0x40, 6, 6, 0, 0, 0, 0, 0, 161, 70, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 114, 71, 123, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 59, 239, 290, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 572, 463, 490, 0 },
    { 1, 0, 0x68, 4, 3, 0, 0, 0, 0, 0, 656, 281, 368, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 209, 283, 336, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag290_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x214, 0x386, 0x34A, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21A, 0x3A8, 0x13C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x205, 0x368, 0x18C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag290_funcs = { wstag290_setup };
