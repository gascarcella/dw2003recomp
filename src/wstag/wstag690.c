#include "wstag.h"

/* WSTAG690: stage 0x261 (fieldstg_stages). */

extern WstagFuncs wstag690_funcs;
extern FieldstgBattleLists wstag690_battle_lists;
extern FieldstgVramPlace wstag690_vram_places[];
extern FieldstgPlacedActor *wstag690_actors[];
extern FieldstgSprite wstag690_sprites[];
extern FieldstgMapEvent wstag690_map_events[];

void wstag690_update(WstagObject *obj) {
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

WstagObject *wstag690_start(void *arg0) {
    WstagObject *obj = object_new(wstag690_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag690_funcs.setup();
    return obj;
}

void wstag690_setup(void) {
    fieldstg_stage.background_file = 0x640;
    fieldstg_stage.sprite_file = 0x06410000;
    fieldstg_stage.sprites = wstag690_sprites;
    fieldstg_stage.map_events = wstag690_map_events;
    fieldstg_stage.mask_file = 0x63F;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x4BE00, 0x2DA00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag690_vram_places;
    fieldstg_stage.music = 0x3D;
    fieldstg_stage.sound = 0x60F40000;
    fieldstg_stage.actors = wstag690_actors;
    fieldstg_stage.battle_lists = &wstag690_battle_lists;
    fieldstg_attr.set_file(0, 0x06410001);
    fieldstg_attr.set_file(7, 0x06410002);
    fieldstg_attr.set_file(4, 0x06410003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag690_setup(void);

FieldstgListedBattle D_WSTAG690_800A5F94 = { 153, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FA0 = { 153, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FAC = { 153, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FB8 = { 111, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FC4 = { 111, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FD0 = { 111, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FDC = { 112, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A5FE8 = { 112, 4, 0x60080000 };
FieldstgBattleList D_WSTAG690_800A5FF4 = {
    3,
    { &D_WSTAG690_800A5F94, &D_WSTAG690_800A5FA0, &D_WSTAG690_800A5FAC, &D_WSTAG690_800A5FB8, &D_WSTAG690_800A5FC4,
        &D_WSTAG690_800A5FD0, &D_WSTAG690_800A5FDC, &D_WSTAG690_800A5FE8 },
};
FieldstgListedBattle D_WSTAG690_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG690_800A6078 = {
    0,
    { &D_WSTAG690_800A6018, &D_WSTAG690_800A6024, &D_WSTAG690_800A6030, &D_WSTAG690_800A603C, &D_WSTAG690_800A6048,
        &D_WSTAG690_800A6054, &D_WSTAG690_800A6060, &D_WSTAG690_800A606C },
};
FieldstgListedBattle D_WSTAG690_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG690_800A60FC = {
    0,
    { &D_WSTAG690_800A609C, &D_WSTAG690_800A60A8, &D_WSTAG690_800A60B4, &D_WSTAG690_800A60C0, &D_WSTAG690_800A60CC,
        &D_WSTAG690_800A60D8, &D_WSTAG690_800A60E4, &D_WSTAG690_800A60F0 },
};
FieldstgListedBattle D_WSTAG690_800A6120 = { 217, 4, 0x600C0000 };
FieldstgListedBattle D_WSTAG690_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6144 = { 333, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG690_800A6168 = { 168, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG690_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG690_800A6180 = {
    0,
    { &D_WSTAG690_800A6120, &D_WSTAG690_800A612C, &D_WSTAG690_800A6138, &D_WSTAG690_800A6144, &D_WSTAG690_800A6150,
        &D_WSTAG690_800A615C, &D_WSTAG690_800A6168, &D_WSTAG690_800A6174 },
};
FieldstgBattleLists wstag690_battle_lists = {
    81, 0, 0, { &D_WSTAG690_800A5FF4, &D_WSTAG690_800A6078, &D_WSTAG690_800A60FC }, &D_WSTAG690_800A6180,
};
FieldstgVramPlace wstag690_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 328, 216, 72, 352, 510 }, { 320, 256, 374, 256, 216, 0, 368, 510 },
    { 320, 256, 374, 360, 216, 104, 352, 509 }, { 320, 256, 374, 296, 216, 40, 368, 509 },
};
u16 D_WSTAG690_800A6260[8] = { 0x220, 1, 0x846D, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A6270[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6278[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A6280[6] = { 0x7206, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A628C[8] = { 0x7206, 1, 0x8014, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A629C[4] = { 0x761B, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A62A4[10] = {
    0x7206, 1, 0x8014, 1, 0x7208, 0, 0, 1,
    0xFFFF, 0,
};
u16 D_WSTAG690_800A62B8[12] = {
    0x7206, 1, 0x8014, 1, 0x7208, 1, 0xE11, 0,
    0, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A62D0[6] = { 0x7400, 1, 0xE11, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A62DC[14] = {
    0xE11, 1, 0x720A, 0, 0x7206, 1, 0x8014, 1,
    0x7208, 1, 0, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A62F8[14] = {
    0x7206, 1, 0x8014, 1, 0x7208, 1, 0xE11, 1,
    0x720A, 1, 0, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A6314[4] = { 0x781B, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A631C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6324[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A632C[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6338[8] = { 0x7206, 1, 0, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6348[4] = { 0x761B, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A6350[10] = {
    0, 1, 0x7208, 0, 0x8014, 1, 0x7206, 1,
    0xFFFF, 0,
};
u16 D_WSTAG690_800A6364[12] = {
    0x7206, 1, 0xE11, 0, 0, 1, 0x8014, 1,
    0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A637C[6] = { 0x7400, 1, 0xE11, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A6388[14] = {
    0, 1, 0x8014, 1, 0x7208, 1, 0x7206, 1,
    0xE11, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG690_800A63A4[14] = {
    0x7206, 1, 0xE11, 1, 0x720A, 1, 0, 1,
    0x8014, 1, 0x7208, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A63C0[4] = { 0x781B, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A63C8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A63D0[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A63DC[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A63E4[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A63F0[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A63FC[8] = { 0x221, 1, 0x822B, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A640C[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6414[6] = { 0, 1, 0x7206, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6420[8] = { 0, 1, 0x7206, 1, 0x8014, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6430[4] = { 0x761B, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A6438[10] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 0,
    0xFFFF, 0,
};
u16 D_WSTAG690_800A644C[12] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE11, 0, 0xFFFF, 0,
};
u16 D_WSTAG690_800A6464[4] = { 0xE11, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A646C[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE11, 1, 0x720A, 0, 0xFFFF, 0,
};
u16 D_WSTAG690_800A6488[14] = {
    0, 1, 0x7206, 1, 0x8014, 1, 0x7208, 1,
    0xE11, 1, 0x720A, 1, 0xFFFF, 0,
};
u16 D_WSTAG690_800A64A4[4] = { 0x781B, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG690_800A64AC[2] = { { NULL, D_WSTAG690_800A6260, 613 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG690_800A64C4[8] = {
    { D_WSTAG690_800A6270, D_WSTAG690_800A6278, 1 }, { D_WSTAG690_800A6280, NULL, 5 },
    { D_WSTAG690_800A628C, D_WSTAG690_800A629C, 6 }, { D_WSTAG690_800A62A4, NULL, 7 },
    { D_WSTAG690_800A62B8, D_WSTAG690_800A62D0, 8 }, { D_WSTAG690_800A62DC, NULL, 9 },
    { D_WSTAG690_800A62F8, D_WSTAG690_800A6314, 10 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG690_800A6524[2] = { { NULL, NULL, 660 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG690_800A653C[8] = {
    { D_WSTAG690_800A631C, D_WSTAG690_800A6324, 2 }, { D_WSTAG690_800A632C, NULL, 5 },
    { D_WSTAG690_800A6338, D_WSTAG690_800A6348, 6 }, { D_WSTAG690_800A6350, NULL, 7 },
    { D_WSTAG690_800A6364, D_WSTAG690_800A637C, 8 }, { D_WSTAG690_800A6388, NULL, 9 },
    { D_WSTAG690_800A63A4, D_WSTAG690_800A63C0, 10 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG690_800A659C[4] = {
    { D_WSTAG690_800A63C8, NULL, 1 }, { D_WSTAG690_800A63D0, D_WSTAG690_800A63DC, 11 },
    { D_WSTAG690_800A63E4, D_WSTAG690_800A63F0, 12 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG690_800A65CC[2] = { { NULL, D_WSTAG690_800A63FC, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG690_800A65E4[8] = {
    { D_WSTAG690_800A640C, NULL, 3 }, { D_WSTAG690_800A6414, NULL, 3 },
    { D_WSTAG690_800A6420, D_WSTAG690_800A6430, 3 }, { D_WSTAG690_800A6438, NULL, 3 },
    { D_WSTAG690_800A644C, D_WSTAG690_800A6464, 3 }, { D_WSTAG690_800A646C, NULL, 3 },
    { D_WSTAG690_800A6488, D_WSTAG690_800A64A4, 3 }, { NULL, NULL, 0 },
};
u16 D_WSTAG690_800A6644[4] = { 0x220, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A664C[8] = { 0x8192, 1, 0x11, 0, 0x7004, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A665C[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A666C[8] = { 0x8192, 1, 0x11, 0, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A667C[8] = { 0x8192, 1, 0x11, 1, 0x7009, 1, 0xFFFF, 0 };
u16 D_WSTAG690_800A668C[4] = { 0x221, 0, 0xFFFF, 0 };
u16 D_WSTAG690_800A6694[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG690_800A669C = { D_WSTAG690_800A6644, D_WSTAG690_800A64AC, 33, 4, 1281, 353, 1 };
FieldstgPlacedActor D_WSTAG690_800A66B0 = { D_WSTAG690_800A664C, D_WSTAG690_800A64C4, 50, 5, 617, 589, 7 };
FieldstgPlacedActor D_WSTAG690_800A66C4 = { D_WSTAG690_800A665C, D_WSTAG690_800A6524, 50, 5, 617, 589, 7 };
FieldstgPlacedActor D_WSTAG690_800A66D8 = { D_WSTAG690_800A666C, D_WSTAG690_800A653C, 50, 5, 617, 589, 7 };
FieldstgPlacedActor D_WSTAG690_800A66EC = { D_WSTAG690_800A667C, D_WSTAG690_800A659C, 50, 5, 617, 589, 7 };
FieldstgPlacedActor D_WSTAG690_800A6700 = { D_WSTAG690_800A668C, D_WSTAG690_800A65CC, 77, 6, 241, 409, 1 };
FieldstgPlacedActor D_WSTAG690_800A6714 = { D_WSTAG690_800A6694, D_WSTAG690_800A65E4, 157, 7, 617, 589, 7 };
FieldstgPlacedActor *wstag690_actors[8] = {
    &D_WSTAG690_800A669C, &D_WSTAG690_800A66B0, &D_WSTAG690_800A66C4, &D_WSTAG690_800A66D8, &D_WSTAG690_800A66EC,
    &D_WSTAG690_800A6700, &D_WSTAG690_800A6714, NULL,
};
FieldstgSprite wstag690_sprites[27] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 323, 826, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 6, 0, 690, 795, 0, 0 },
    { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 455, 285, 0, 0 }, { 1, 0, 0x40, 2, 0, 1, 0, 5, 4, 0, 749, 532, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 210, 424, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 277, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 788, 245, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 960, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 850, 660, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 939, 699, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 950, 624, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x3C, 6, 0, 997, 651, 0, 0 },
    { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 8, 0, 893, 649, 0, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 130, 97, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 434, 412, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 480, 645, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 611, 294, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 628, 630, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 893, 695, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 1101, 650, 896, 0 },
    { 1, 0, 0xC8, 4, 0x33, 1, 0x33, 0x36, 8, 0, 1232, -8, 896, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 527, 527, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 512, 703, 703, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 816, 375, 375, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 991, 207, 207, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 1088, 207, 207, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag690_map_events[14] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x265, 0x4F0, 0x2B8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x262, 0xA0, 0x1D0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x262, 0xA0, 0x240, 7, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x16, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 2, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E9, 0xB0, 0xF8, 7, 0, 5, 2 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xC, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x27F, 0x280, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x26E, 0x2E8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x4C0, 0x1A0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x4B0, 0x208, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 4, 7, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 4, 0xA, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag690_funcs = { wstag690_setup };
