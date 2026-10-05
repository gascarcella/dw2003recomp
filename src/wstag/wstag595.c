#include "wstag.h"

/* WSTAG595: stage 0x250 (fieldstg_stages). */

extern WstagFuncs wstag595_funcs;
extern FieldstgBattleLists wstag595_battle_lists;
extern FieldstgVramPlace wstag595_vram_places[];
extern FieldstgPlacedActor *wstag595_actors[];
extern FieldstgSprite wstag595_sprites[];
extern FieldstgMapEvent wstag595_map_events[];

void wstag595_update(WstagObject *obj) {
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

WstagObject *wstag595_start(void *arg0) {
    WstagObject *obj = object_new(wstag595_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag595_funcs.setup();
    return obj;
}

void wstag595_setup(void) {
    fieldstg_stage.background_file = 0x507;
    fieldstg_stage.sprite_file = 0x05080000;
    fieldstg_stage.sprites = wstag595_sprites;
    fieldstg_stage.map_events = wstag595_map_events;
    fieldstg_stage.mask_file = 0x506;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13200, 0x15B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag595_vram_places;
    fieldstg_stage.music = 0x39;
    fieldstg_stage.sound = 0x60E40000;
    fieldstg_stage.actors = wstag595_actors;
    fieldstg_stage.battle_lists = &wstag595_battle_lists;
    fieldstg_attr.set_file(0, 0x05080001);
    fieldstg_attr.set_file(7, 0x05080002);
    fieldstg_attr.set_file(4, 0x05080003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag595_setup(void);

FieldstgListedBattle D_WSTAG595_800A5F94 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FA0 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FAC = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FB8 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FC4 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FD0 = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FDC = { 75, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG595_800A5FE8 = { 75, 12, 0x60080000 };
FieldstgBattleList D_WSTAG595_800A5FF4 = {
    4,
    { &D_WSTAG595_800A5F94, &D_WSTAG595_800A5FA0, &D_WSTAG595_800A5FAC, &D_WSTAG595_800A5FB8, &D_WSTAG595_800A5FC4,
        &D_WSTAG595_800A5FD0, &D_WSTAG595_800A5FDC, &D_WSTAG595_800A5FE8 },
};
FieldstgListedBattle D_WSTAG595_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG595_800A6078 = {
    0,
    { &D_WSTAG595_800A6018, &D_WSTAG595_800A6024, &D_WSTAG595_800A6030, &D_WSTAG595_800A603C, &D_WSTAG595_800A6048,
        &D_WSTAG595_800A6054, &D_WSTAG595_800A6060, &D_WSTAG595_800A606C },
};
FieldstgListedBattle D_WSTAG595_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG595_800A60FC = {
    0,
    { &D_WSTAG595_800A609C, &D_WSTAG595_800A60A8, &D_WSTAG595_800A60B4, &D_WSTAG595_800A60C0, &D_WSTAG595_800A60CC,
        &D_WSTAG595_800A60D8, &D_WSTAG595_800A60E4, &D_WSTAG595_800A60F0 },
};
FieldstgListedBattle D_WSTAG595_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG595_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG595_800A6180 = {
    0,
    { &D_WSTAG595_800A6120, &D_WSTAG595_800A612C, &D_WSTAG595_800A6138, &D_WSTAG595_800A6144, &D_WSTAG595_800A6150,
        &D_WSTAG595_800A615C, &D_WSTAG595_800A6168, &D_WSTAG595_800A6174 },
};
FieldstgBattleLists wstag595_battle_lists = {
    44, 0, 0, { &D_WSTAG595_800A5FF4, &D_WSTAG595_800A6078, &D_WSTAG595_800A60FC }, &D_WSTAG595_800A6180,
};
FieldstgVramPlace wstag595_vram_places[7] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 256, 216, 0, 336, 511 },
};
u16 D_WSTAG595_800A6230[8] = { 0x210, 1, 0x8231, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG595_800A6240[2] = { { NULL, D_WSTAG595_800A6230, 600 }, { NULL, NULL, 0 } };
u16 D_WSTAG595_800A6258[6] = { 0x210, 0, 0x1C24, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG595_800A6264 = { D_WSTAG595_800A6258, D_WSTAG595_800A6240, 33, 4, 379, 318, 1 };
FieldstgPlacedActor *wstag595_actors[2] = { &D_WSTAG595_800A6264, NULL };
FieldstgSprite wstag595_sprites[4] = {
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 6, 0, 277, 252, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 6, 0, 405, 317, 0, 0 },
    { 1, 0, 0x40, 6, 0, 1, 0, 0xB, 4, 0, 214, 250, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag595_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x24D, 0x408, 0x1A4, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag595_funcs = { wstag595_setup };
