#include "wstag.h"

/* WSTAG942: stage 0x28E (fieldstg_stages_2d). */

extern WstagFuncs wstag942_funcs;
const CVECTOR wstag942_color = { 0x54, 0x67, 0x96, 0 };
extern FieldstgVramPlace wstag942_vram_places[];
extern FieldstgPlacedActor *wstag942_actors[];
extern FieldstgSprite wstag942_sprites[];
extern FieldstgMapEvent wstag942_map_events[];
extern FieldstgBattleLists wstag942_battle_lists;

void wstag942_update(WstagObject *obj) {
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

WstagObject *wstag942_start(void *arg0) {
    WstagObject *obj = object_new(wstag942_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag942_funcs.setup();
    return obj;
}

void wstag942_setup(void) {
    fieldstg_stage.background_file = 0x3A1;
    fieldstg_stage.sprite_file = 0x090B0000;
    fieldstg_stage.sprites = wstag942_sprites;
    fieldstg_stage.map_events = wstag942_map_events;
    fieldstg_stage.mask_file = 0x90A;
    fieldstg_stage.talk_file = records_language + 0x104;
    fieldstg_stage.start_pos = (GamestatePos){ 0x30700, 0x9A00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag942_vram_places;
    fieldstg_stage.music = 0x36;
    fieldstg_stage.sound = 0x60D80000;
    fieldstg_stage.actors = wstag942_actors;
    fieldstg_stage.color = wstag942_color;
    fieldstg_stage.battle_lists = &wstag942_battle_lists;
    fieldstg_attr.set_file(0, 0x090B0002);
    fieldstg_attr.set_file(7, 0x090B0003);
    fieldstg_attr.set_file(4, 0x090B0001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag942_setup(void);

FieldstgVramPlace wstag942_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 370, 337, 200, 81, 336, 511 }, { 320, 256, 370, 377, 200, 121, 352, 511 },
    { 384, 256, 416, 256, 384, 0, 368, 511 }, { 384, 256, 424, 256, 416, 0, 320, 510 },
};
u16 D_WSTAG942_800A6050[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A605C[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A6068[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A6074[8] = { 0x11, 0, 0x10, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A6084[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A6090[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A6098[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A60A4[4] = { 0x7841, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A60AC[4] = { 5, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A60B4[4] = { 5, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A60BC[6] = { 5, 1, 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A60C8[8] = { 5, 1, 0x8005, 0, 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A60D8[6] = { 0x8005, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG942_800A60E4[8] = { 5, 1, 0x8005, 1, 0x8192, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG942_800A60F4[2] = { { NULL, NULL, 33 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG942_800A610C[5] = {
    { D_WSTAG942_800A6050, D_WSTAG942_800A605C, 35 }, { D_WSTAG942_800A6068, D_WSTAG942_800A6074, 36 },
    { D_WSTAG942_800A6084, D_WSTAG942_800A6090, 33 }, { D_WSTAG942_800A6098, D_WSTAG942_800A60A4, 34 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG942_800A6148[2] = { { NULL, NULL, 120 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG942_800A6160[5] = {
    { D_WSTAG942_800A60AC, D_WSTAG942_800A60B4, 117 }, { D_WSTAG942_800A60BC, NULL, 119 },
    { D_WSTAG942_800A60C8, D_WSTAG942_800A60D8, 118 }, { D_WSTAG942_800A60E4, NULL, 119 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG942_800A619C[2] = { { NULL, NULL, 121 }, { NULL, NULL, 0 } };
u16 D_WSTAG942_800A61B4[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG942_800A61BC[4] = { 0x8192, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG942_800A61C4 = { D_WSTAG942_800A61B4, D_WSTAG942_800A60F4, 54, 4, 288, 424, 1 };
FieldstgPlacedActor D_WSTAG942_800A61D8 = { D_WSTAG942_800A61BC, D_WSTAG942_800A610C, 54, 4, 288, 424, 1 };
FieldstgPlacedActor D_WSTAG942_800A61EC = { NULL, D_WSTAG942_800A6148, 140, 5, 640, 593, 7 };
FieldstgPlacedActor D_WSTAG942_800A6200 = { NULL, D_WSTAG942_800A6160, 282, 6, 412, 642, 1 };
FieldstgPlacedActor D_WSTAG942_800A6214 = { NULL, D_WSTAG942_800A619C, 369, 7, 299, 282, 5 };
FieldstgPlacedActor *wstag942_actors[6] = {
    &D_WSTAG942_800A61C4, &D_WSTAG942_800A61D8, &D_WSTAG942_800A61EC, &D_WSTAG942_800A6200, &D_WSTAG942_800A6214,
    NULL,
};
FieldstgSprite wstag942_sprites[44] = {
    { 1, 0, 0x40, 2, 5, 1, 5, 0xA, 8, 0, 395, 85, 0, 0 }, { 1, 0, 0x40, 2, 5, 1, 5, 0xA, 8, 0, 698, 285, 0, 0 },
    { 1, 0, 0x50, 2, 0xB, 0, 0, 0, 0, 0, 688, 97, 0, 0 }, { 1, 0, 0x72, 2, 0xC, 0, 0, 0, 0, 0, 784, 47, 0, 0 },
    { 1, 0, 0x80, 2, 0xD, 0, 0, 0, 0, 0, 640, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 72, 562, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 271, 529, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 677, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 224, 557, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 309, 704, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 290, 716, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 698, 747, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 83, 463, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 47, 385, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 146, 531, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 37, 375, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 90, 472, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 1, 0x41, 0x43, 0xA, 0, 218, 638, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 75, 550, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 96, 549, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 182, 545, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 288, 706, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 310, 714, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 321, 691, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 494, 763, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 514, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 671, 111, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 799, 48, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 811, 36, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 1, 0x44, 0x46, 0xA, 0, 817, 43, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 496, 558, 575, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 509, 517, 543, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 514, 471, 489, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 676, 390, 407, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 276, 499, 511, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 224, 271, 271, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 256, 239, 239, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 320, 255, 255, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 390, 243, 243, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 464, 247, 247, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 528, 231, 231, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 592, 343, 343, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 656, 375, 375, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag942_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28C, 0x102, 0x3AC, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0xA, 0x2E3, 0x380, 0x70, 1, 0, 4, 1 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0x3C, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 8, 0, 0, 0, 0, 0 },
    { 0x8005, 1, 0xFFFF, 0, 7, 0xFFC0, 0xFFE8, 0, 0, 0, 0, 0 }, { 0x8005, 1, 0xFFFF, 0, 7, 0, 0x40, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag942_funcs = { wstag942_setup };
FieldstgListedBattle D_WSTAG942_800A6604 = { 42, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6610 = { 42, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A661C = { 42, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6628 = { 42, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6634 = { 43, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6640 = { 43, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A664C = { 43, 1, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6658 = { 43, 1, 0x60080000 };
FieldstgBattleList D_WSTAG942_800A6664 = {
    3,
    { &D_WSTAG942_800A6604, &D_WSTAG942_800A6610, &D_WSTAG942_800A661C, &D_WSTAG942_800A6628, &D_WSTAG942_800A6634,
        &D_WSTAG942_800A6640, &D_WSTAG942_800A664C, &D_WSTAG942_800A6658 },
};
FieldstgListedBattle D_WSTAG942_800A6688 = { 36, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A6694 = { 36, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66A0 = { 36, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66AC = { 36, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66B8 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66C4 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66D0 = { 37, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A66DC = { 37, 2, 0x60080000 };
FieldstgBattleList D_WSTAG942_800A66E8 = {
    3,
    { &D_WSTAG942_800A6688, &D_WSTAG942_800A6694, &D_WSTAG942_800A66A0, &D_WSTAG942_800A66AC, &D_WSTAG942_800A66B8,
        &D_WSTAG942_800A66C4, &D_WSTAG942_800A66D0, &D_WSTAG942_800A66DC },
};
FieldstgListedBattle D_WSTAG942_800A670C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6718 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6724 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6730 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A673C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6748 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6754 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A6760 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG942_800A676C = {
    0,
    { &D_WSTAG942_800A670C, &D_WSTAG942_800A6718, &D_WSTAG942_800A6724, &D_WSTAG942_800A6730, &D_WSTAG942_800A673C,
        &D_WSTAG942_800A6748, &D_WSTAG942_800A6754, &D_WSTAG942_800A6760 },
};
FieldstgListedBattle D_WSTAG942_800A6790 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A679C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A67A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A67B4 = { 327, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A67C0 = { 328, 2, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A67CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG942_800A67D8 = { 48, 13, 0x60080000 };
FieldstgListedBattle D_WSTAG942_800A67E4 = { 59, 2, 0x60080000 };
FieldstgBattleList D_WSTAG942_800A67F0 = {
    0,
    { &D_WSTAG942_800A6790, &D_WSTAG942_800A679C, &D_WSTAG942_800A67A8, &D_WSTAG942_800A67B4, &D_WSTAG942_800A67C0,
        &D_WSTAG942_800A67CC, &D_WSTAG942_800A67D8, &D_WSTAG942_800A67E4 },
};
FieldstgBattleLists wstag942_battle_lists = {
    381, 0, 0, { &D_WSTAG942_800A6664, &D_WSTAG942_800A66E8, &D_WSTAG942_800A676C }, &D_WSTAG942_800A67F0,
};
