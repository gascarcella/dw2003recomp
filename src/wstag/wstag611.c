#include "wstag.h"

/* WSTAG611: stage 0x2BD (fieldstg_stages). */

extern WstagFuncs wstag611_funcs;
extern FieldstgBattleLists wstag611_battle_lists;
extern FieldstgVramPlace wstag611_vram_places[];
extern FieldstgPlacedActor *wstag611_actors[];
extern FieldstgSprite wstag611_sprites[];
extern FieldstgMapEvent wstag611_map_events[];

void wstag611_update(WstagObject *obj) {
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

WstagObject *wstag611_start(void *arg0) {
    WstagObject *obj = object_new(wstag611_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag611_funcs.setup();
    return obj;
}

void wstag611_setup(void) {
    fieldstg_stage.background_file = 0x61A;
    fieldstg_stage.sprite_file = 0x061B0000;
    fieldstg_stage.sprites = wstag611_sprites;
    fieldstg_stage.map_events = wstag611_map_events;
    fieldstg_stage.mask_file = 0x619;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0xBD00, 0x25800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag611_vram_places;
    fieldstg_stage.music = 0x3A;
    fieldstg_stage.sound = 0x60E80000;
    fieldstg_stage.actors = wstag611_actors;
    fieldstg_stage.battle_lists = &wstag611_battle_lists;
    fieldstg_attr.set_file(0, 0x061B0001);
    fieldstg_attr.set_file(7, 0x061B0002);
    fieldstg_attr.set_file(4, 0x061B0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag611_setup(void);

FieldstgListedBattle D_WSTAG611_800A5F90 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5F9C = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FA8 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FB4 = { 116, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FC0 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FCC = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FD8 = { 164, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG611_800A5FE4 = { 164, 12, 0x60080000 };
FieldstgBattleList D_WSTAG611_800A5FF0 = {
    2,
    { &D_WSTAG611_800A5F90, &D_WSTAG611_800A5F9C, &D_WSTAG611_800A5FA8, &D_WSTAG611_800A5FB4, &D_WSTAG611_800A5FC0,
        &D_WSTAG611_800A5FCC, &D_WSTAG611_800A5FD8, &D_WSTAG611_800A5FE4 },
};
FieldstgListedBattle D_WSTAG611_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6068 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG611_800A6074 = {
    0,
    { &D_WSTAG611_800A6014, &D_WSTAG611_800A6020, &D_WSTAG611_800A602C, &D_WSTAG611_800A6038, &D_WSTAG611_800A6044,
        &D_WSTAG611_800A6050, &D_WSTAG611_800A605C, &D_WSTAG611_800A6068 },
};
FieldstgListedBattle D_WSTAG611_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A60EC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG611_800A60F8 = {
    0,
    { &D_WSTAG611_800A6098, &D_WSTAG611_800A60A4, &D_WSTAG611_800A60B0, &D_WSTAG611_800A60BC, &D_WSTAG611_800A60C8,
        &D_WSTAG611_800A60D4, &D_WSTAG611_800A60E0, &D_WSTAG611_800A60EC },
};
FieldstgListedBattle D_WSTAG611_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG611_800A6170 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG611_800A617C = {
    0,
    { &D_WSTAG611_800A611C, &D_WSTAG611_800A6128, &D_WSTAG611_800A6134, &D_WSTAG611_800A6140, &D_WSTAG611_800A614C,
        &D_WSTAG611_800A6158, &D_WSTAG611_800A6164, &D_WSTAG611_800A6170 },
};
FieldstgBattleLists wstag611_battle_lists = {
    112, 0, 0, { &D_WSTAG611_800A5FF0, &D_WSTAG611_800A6074, &D_WSTAG611_800A60F8 }, &D_WSTAG611_800A617C,
};
FieldstgVramPlace wstag611_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 418, 374, 392, 118, 368, 511 }, { 384, 256, 434, 326, 456, 70, 336, 510 },
    { 384, 256, 434, 366, 456, 110, 352, 510 }, { 384, 256, 410, 374, 360, 118, 368, 510 },
};
FieldstgTalk D_WSTAG611_800A625C[2] = { { NULL, NULL, 280 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG611_800A6274[2] = { { NULL, NULL, 278 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG611_800A628C[2] = { { NULL, NULL, 279 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG611_800A62A4[2] = { { NULL, NULL, 279 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG611_800A62BC[2] = { { NULL, NULL, 277 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG611_800A62D4[2] = { { NULL, NULL, 277 }, { NULL, NULL, 0 } };
u16 D_WSTAG611_800A62EC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG611_800A62F8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG611_800A6304[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG611_800A6310[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG611_800A6318[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG611_800A6324[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG611_800A632C = { D_WSTAG611_800A62EC, D_WSTAG611_800A625C, 45, 4, 625, 171, 1 };
FieldstgPlacedActor D_WSTAG611_800A6340 = { D_WSTAG611_800A62F8, D_WSTAG611_800A6274, 51, 5, 529, 567, 7 };
FieldstgPlacedActor D_WSTAG611_800A6354 = { D_WSTAG611_800A6304, D_WSTAG611_800A628C, 157, 6, 625, 171, 1 };
FieldstgPlacedActor D_WSTAG611_800A6368 = { D_WSTAG611_800A6310, D_WSTAG611_800A62A4, 157, 6, 625, 171, 1 };
FieldstgPlacedActor D_WSTAG611_800A637C = { D_WSTAG611_800A6318, D_WSTAG611_800A62BC, 158, 7, 529, 567, 7 };
FieldstgPlacedActor D_WSTAG611_800A6390 = { D_WSTAG611_800A6324, D_WSTAG611_800A62D4, 158, 7, 529, 567, 7 };
FieldstgPlacedActor *wstag611_actors[7] = {
    &D_WSTAG611_800A632C, &D_WSTAG611_800A6340, &D_WSTAG611_800A6354, &D_WSTAG611_800A6368, &D_WSTAG611_800A637C,
    &D_WSTAG611_800A6390, NULL,
};
FieldstgSprite wstag611_sprites[20] = {
    { 1, 0, 0x80, 2, 1, 0, 0, 0, 0, 0, 512, 0, 0, 0 }, { 1, 0, 0x80, 2, 2, 0, 0, 0, 0, 0, 463, 0, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 4, 0, 180, 506, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x37, 4, 0, 249, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3D, 4, 0, 160, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3D, 4, 0, 229, 368, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 388, 395, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 429, 415, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 441, 565, 0, 0 }, { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 468, 435, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 2, 0, 1, 4, 0, 482, 585, 0, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 460, 138, 584, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 492, 154, 584, 0 },
    { 1, 0, 0x40, 4, 0x3F, 2, 0, 1, 4, 0, 586, 201, 584, 0 }, { 1, 0, 0x65, 4, 0, 0, 0, 0, 0, 0, 84, 484, 584, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 489, 136, 190, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 509, 136, 198, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 525, 136, 207, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 541, 136, 215, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag611_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2B7, 0x88, 0x22C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2BE, 0x3F0, 0x2F0, 3, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag611_funcs = { wstag611_setup };
