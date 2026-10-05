#include "wstag.h"

/* WSTAG315: stage 0x21A (fieldstg_stages). */

extern WstagFuncs wstag315_funcs;
extern FieldstgBattleLists wstag315_battle_lists;
extern FieldstgVramPlace wstag315_vram_places[];
extern FieldstgPlacedActor *wstag315_actors[];
extern FieldstgSprite wstag315_sprites[];
extern FieldstgMapEvent wstag315_map_events[];

void wstag315_update(WstagObject *obj) {
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

WstagObject *wstag315_start(void *arg0) {
    WstagObject *obj = object_new(wstag315_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag315_funcs.setup();
    return obj;
}

void wstag315_setup(void) {
    fieldstg_stage.background_file = 0x747;
    fieldstg_stage.sprite_file = 0x07480000;
    fieldstg_stage.sprites = wstag315_sprites;
    fieldstg_stage.map_events = wstag315_map_events;
    fieldstg_stage.mask_file = 0x746;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13B00, 0x1B900 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag315_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag315_actors;
    fieldstg_stage.battle_lists = &wstag315_battle_lists;
    fieldstg_attr.set_file(0, 0x07480001);
    fieldstg_attr.set_file(7, 0x07480002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag315_setup(void);

FieldstgListedBattle D_WSTAG315_800A5F7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5F88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5F94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5FA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5FAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5FB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A5FD0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG315_800A5FDC = {
    0,
    { &D_WSTAG315_800A5F7C, &D_WSTAG315_800A5F88, &D_WSTAG315_800A5F94, &D_WSTAG315_800A5FA0, &D_WSTAG315_800A5FAC,
        &D_WSTAG315_800A5FB8, &D_WSTAG315_800A5FC4, &D_WSTAG315_800A5FD0 },
};
FieldstgListedBattle D_WSTAG315_800A6000 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A600C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6054 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG315_800A6060 = {
    0,
    { &D_WSTAG315_800A6000, &D_WSTAG315_800A600C, &D_WSTAG315_800A6018, &D_WSTAG315_800A6024, &D_WSTAG315_800A6030,
        &D_WSTAG315_800A603C, &D_WSTAG315_800A6048, &D_WSTAG315_800A6054 },
};
FieldstgListedBattle D_WSTAG315_800A6084 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6090 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A60D8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG315_800A60E4 = {
    0,
    { &D_WSTAG315_800A6084, &D_WSTAG315_800A6090, &D_WSTAG315_800A609C, &D_WSTAG315_800A60A8, &D_WSTAG315_800A60B4,
        &D_WSTAG315_800A60C0, &D_WSTAG315_800A60CC, &D_WSTAG315_800A60D8 },
};
FieldstgListedBattle D_WSTAG315_800A6108 = { 192, 12, 0x60080000 };
FieldstgListedBattle D_WSTAG315_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG315_800A615C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG315_800A6168 = {
    0,
    { &D_WSTAG315_800A6108, &D_WSTAG315_800A6114, &D_WSTAG315_800A6120, &D_WSTAG315_800A612C, &D_WSTAG315_800A6138,
        &D_WSTAG315_800A6144, &D_WSTAG315_800A6150, &D_WSTAG315_800A615C },
};
FieldstgBattleLists wstag315_battle_lists = {
    138, 0, 0, { &D_WSTAG315_800A5FDC, &D_WSTAG315_800A6060, &D_WSTAG315_800A60E4 }, &D_WSTAG315_800A6168,
};
FieldstgVramPlace wstag315_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 376, 373, 224, 117, 368, 511 }, { 384, 256, 440, 256, 480, 0, 368, 510 },
    { 384, 256, 424, 280, 416, 24, 368, 509 }, { 320, 256, 368, 341, 192, 85, 368, 508 },
    { 320, 256, 368, 381, 192, 125, 336, 507 }, { 320, 256, 344, 398, 96, 142, 352, 507 },
    { 320, 256, 370, 421, 200, 165, 368, 507 }, { 320, 256, 370, 461, 200, 205, 336, 506 },
    { 384, 256, 416, 256, 384, 0, 352, 506 },
};
u16 D_WSTAG315_800A6298[8] = { 0x211, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62A8[8] = { 0x708B, 1, 0x212, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62B8[8] = { 0x213, 1, 0x8246, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62C8[6] = { 0x7400, 1, 0xC14, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62D4[6] = { 0x7400, 1, 0xC15, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62E0[6] = { 0x7400, 1, 0xC16, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62EC[6] = { 0x7400, 1, 0xC15, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A62F8[6] = { 0x7400, 1, 0xC17, 1, 0xFFFF, 0 };
u16 D_WSTAG315_800A6304[6] = { 0x7400, 1, 0xC18, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG315_800A6310[2] = { { NULL, D_WSTAG315_800A6298, 694 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A6328[2] = { { NULL, D_WSTAG315_800A62A8, 1161 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A6340[2] = { { NULL, D_WSTAG315_800A62B8, 1104 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A6358[2] = { { NULL, D_WSTAG315_800A62C8, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A6370[2] = { { NULL, D_WSTAG315_800A62D4, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A6388[2] = { { NULL, D_WSTAG315_800A62E0, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A63A0[2] = { { NULL, D_WSTAG315_800A62EC, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A63B8[2] = { { NULL, D_WSTAG315_800A62F8, 169 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG315_800A63D0[2] = { { NULL, D_WSTAG315_800A6304, 169 }, { NULL, NULL, 0 } };
u16 D_WSTAG315_800A63E8[4] = { 0x211, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A63F0[4] = { 0x212, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A63F8[4] = { 0x213, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A6400[6] = { 0x6016, 1, 0xC14, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A640C[6] = { 0x6016, 1, 0xC15, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A6418[6] = { 0x6016, 1, 0xC16, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A6424[6] = { 0x6016, 1, 0xC15, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A6430[6] = { 0x6016, 1, 0xC17, 0, 0xFFFF, 0 };
u16 D_WSTAG315_800A643C[6] = { 0x6016, 1, 0xC18, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG315_800A6448 = { D_WSTAG315_800A63E8, D_WSTAG315_800A6310, 33, 4, 144, 647, 1 };
FieldstgPlacedActor D_WSTAG315_800A645C = { D_WSTAG315_800A63F0, D_WSTAG315_800A6328, 77, 5, 209, 201, 1 };
FieldstgPlacedActor D_WSTAG315_800A6470 = { D_WSTAG315_800A63F8, D_WSTAG315_800A6340, 78, 6, 768, 490, 1 };
FieldstgPlacedActor D_WSTAG315_800A6484 = { D_WSTAG315_800A6400, D_WSTAG315_800A6358, 306, 7, 236, 720, 1 };
FieldstgPlacedActor D_WSTAG315_800A6498 = { D_WSTAG315_800A640C, D_WSTAG315_800A6370, 307, 8, 419, 666, 1 };
FieldstgPlacedActor D_WSTAG315_800A64AC = { D_WSTAG315_800A6418, D_WSTAG315_800A6388, 308, 9, 377, 310, 7 };
FieldstgPlacedActor D_WSTAG315_800A64C0 = { D_WSTAG315_800A6424, D_WSTAG315_800A63A0, 309, 10, 446, 679, 1 };
FieldstgPlacedActor D_WSTAG315_800A64D4 = { D_WSTAG315_800A6430, D_WSTAG315_800A63B8, 310, 11, 736, 233, 1 };
FieldstgPlacedActor D_WSTAG315_800A64E8 = { D_WSTAG315_800A643C, D_WSTAG315_800A63D0, 311, 12, 593, 608, 1 };
FieldstgPlacedActor *wstag315_actors[10] = {
    &D_WSTAG315_800A6448, &D_WSTAG315_800A645C, &D_WSTAG315_800A6470, &D_WSTAG315_800A6484, &D_WSTAG315_800A6498,
    &D_WSTAG315_800A64AC, &D_WSTAG315_800A64C0, &D_WSTAG315_800A64D4, &D_WSTAG315_800A64E8, NULL,
};
FieldstgSprite wstag315_sprites[27] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 191, 124, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 383, 219, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 735, 142, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 807, 178, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 855, 202, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 4, 0, 927, 238, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 4, 0, 492, 156, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 70, 458, 0, 0 },
    { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 110, 478, 0, 0 }, { 1, 0, 0x40, 2, 0x34, 2, 0, 3, 4, 0, 150, 498, 0, 0 },
    { 1, 0, 0x40, 2, 0x35, 2, 0, 3, 4, 0, 380, 474, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 3, 4, 0, 584, 338, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 0xB, 4, 0, 569, 365, 0, 0 }, { 1, 0, 0x4E, 2, 5, 0, 0, 0, 0, 0, 640, 434, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 704, 448, 0, 0 }, { 1, 0, 0x40, 6, 0x36, 2, 0, 3, 4, 0, 336, 602, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 0xB, 4, 0, 702, 441, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 0xB, 4, 0, 838, 509, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 737, 176, 214, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 392, 265, 292, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 419, 258, 304, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 544, 550, 583, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 185, 681, 707, 0 },
    { 1, 0, 0x60, 4, 7, 0, 0, 0, 0, 0, 704, 89, 176, 0 }, { 1, 0, 0x60, 4, 8, 0, 0, 0, 0, 0, 512, 89, 174, 0 },
    { 1, 0, 0x48, 4, 9, 0, 0, 0, 0, 0, 233, 359, 422, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag315_map_events[5] = {
    { 0x703F, 1, 0xFFFF, 0, 1, 0x21B, 0x338, 0x21C, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x215, 0x78, 0x124, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 8, 0x19E, 0x1C0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 8, 0x1AE, 0x248, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag315_funcs = { wstag315_setup };
