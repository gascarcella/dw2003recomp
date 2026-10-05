#include "wstag.h"

/* WSTAG321: stage 0x28A (fieldstg_stages). */

extern WstagFuncs wstag321_funcs;
extern CVECTOR wstag321_color;
extern FieldstgBattleLists wstag321_battle_lists;
extern FieldstgVramPlace wstag321_vram_places[];
extern FieldstgPlacedActor *wstag321_actors[];
extern FieldstgSprite wstag321_sprites[];
extern FieldstgMapEvent wstag321_map_events[];

void wstag321_update(WstagObject *obj) {
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

WstagObject *wstag321_start(void *arg0) {
    WstagObject *obj = object_new(wstag321_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag321_funcs.setup();
    return obj;
}

void wstag321_setup(void) {
    fieldstg_stage.background_file = 0x53F;
    fieldstg_stage.sprite_file = 0x05400000;
    fieldstg_stage.sprites = wstag321_sprites;
    fieldstg_stage.map_events = wstag321_map_events;
    fieldstg_stage.mask_file = 0x53E;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x31100, 0x22700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag321_vram_places;
    fieldstg_stage.music = 6;
    fieldstg_stage.sound = 0x60180000;
    fieldstg_stage.actors = wstag321_actors;
    fieldstg_stage.color = wstag321_color;
    fieldstg_stage.battle_lists = &wstag321_battle_lists;
    fieldstg_attr.set_file(0, 0x05400001);
    fieldstg_attr.set_file(7, 0x05400002);
    fieldstg_attr.set_file(4, 0x05400003);
    fieldstg_attr.init_layer(0);
}

INCLUDE_RODATA("asm/wstag321/nonmatchings/wstag321", wstag321_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag321_setup(void);

FieldstgListedBattle D_WSTAG321_800A5FB4 = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FC0 = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FCC = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FD8 = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FE4 = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FF0 = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A5FFC = { 179, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG321_800A6008 = { 179, 7, 0x60080000 };
FieldstgBattleList D_WSTAG321_800A6014 = {
    3,
    { &D_WSTAG321_800A5FB4, &D_WSTAG321_800A5FC0, &D_WSTAG321_800A5FCC, &D_WSTAG321_800A5FD8, &D_WSTAG321_800A5FE4,
        &D_WSTAG321_800A5FF0, &D_WSTAG321_800A5FFC, &D_WSTAG321_800A6008 },
};
FieldstgListedBattle D_WSTAG321_800A6038 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6044 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6050 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A605C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A608C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG321_800A6098 = {
    0,
    { &D_WSTAG321_800A6038, &D_WSTAG321_800A6044, &D_WSTAG321_800A6050, &D_WSTAG321_800A605C, &D_WSTAG321_800A6068,
        &D_WSTAG321_800A6074, &D_WSTAG321_800A6080, &D_WSTAG321_800A608C },
};
FieldstgListedBattle D_WSTAG321_800A60BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A60C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A60D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A60E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6110 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG321_800A611C = {
    0,
    { &D_WSTAG321_800A60BC, &D_WSTAG321_800A60C8, &D_WSTAG321_800A60D4, &D_WSTAG321_800A60E0, &D_WSTAG321_800A60EC,
        &D_WSTAG321_800A60F8, &D_WSTAG321_800A6104, &D_WSTAG321_800A6110 },
};
FieldstgListedBattle D_WSTAG321_800A6140 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A614C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6158 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6164 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6170 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG321_800A6194 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG321_800A61A0 = {
    0,
    { &D_WSTAG321_800A6140, &D_WSTAG321_800A614C, &D_WSTAG321_800A6158, &D_WSTAG321_800A6164, &D_WSTAG321_800A6170,
        &D_WSTAG321_800A617C, &D_WSTAG321_800A6188, &D_WSTAG321_800A6194 },
};
FieldstgBattleLists wstag321_battle_lists = {
    123, 0, 0, { &D_WSTAG321_800A6014, &D_WSTAG321_800A6098, &D_WSTAG321_800A611C }, &D_WSTAG321_800A61A0,
};
FieldstgVramPlace wstag321_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 440, 256, 480, 0, 320, 500 }, { 384, 256, 414, 304, 376, 48, 352, 500 },
};
u16 D_WSTAG321_800A6260[8] = { 0x264, 1, 0x8B22, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG321_800A6270[8] = { 0x7091, 1, 0x265, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG321_800A6280[2] = { { NULL, D_WSTAG321_800A6260, 365 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG321_800A6298[2] = { { NULL, D_WSTAG321_800A6270, 371 }, { NULL, NULL, 0 } };
u16 D_WSTAG321_800A62B0[4] = { 0x264, 0, 0xFFFF, 0 };
u16 D_WSTAG321_800A62B8[4] = { 0x265, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG321_800A62C0 = { D_WSTAG321_800A62B0, D_WSTAG321_800A6280, 33, 4, 465, 169, 1 };
FieldstgPlacedActor D_WSTAG321_800A62D4 = { D_WSTAG321_800A62B8, D_WSTAG321_800A6298, 77, 5, 272, 217, 1 };
FieldstgPlacedActor *wstag321_actors[3] = { &D_WSTAG321_800A62C0, &D_WSTAG321_800A62D4, NULL };
FieldstgSprite wstag321_sprites[104] = {
    { 1, 0, 0x40, 2, 0x4D, 2, 0, 3, 4, 0, 1012, 671, 0, 0 }, { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 442, 116, 0, 0 },
    { 1, 0, 0x40, 2, 0x52, 2, 0, 3, 8, 0, 473, 132, 0, 0 }, { 1, 0, 0x40, 2, 0x53, 2, 0, 3, 4, 0, 299, 39, 0, 0 },
    { 1, 0, 0x40, 2, 0x5A, 0, 0, 0, 0, 0, 472, 396, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 417, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 449, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 481, 0, 0 },
    { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 513, 0, 0 }, { 1, 0, 0x40, 2, 0x5E, 2, 0, 5, 4, 0, 871, 544, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 1, 0x47, 0x4C, 4, 0, 735, 465, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 328, 658, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 616, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 760, 730, 0, 0 }, { 1, 0, 0x40, 6, 0x4E, 2, 0, 3, 6, 0, 904, 802, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 351, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 639, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 783, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x4F, 2, 0, 3, 6, 0, 927, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 324, 632, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 612, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 756, 704, 0, 0 }, { 1, 0, 0x40, 6, 0x51, 2, 0, 3, 6, 0, 900, 776, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 305, 643, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 593, 788, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 737, 715, 0, 0 }, { 1, 0, 0x40, 6, 0x50, 2, 0, 3, 6, 0, 881, 787, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 202, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 210, 420, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 218, 424, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 226, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 234, 432, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 242, 436, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 250, 440, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 258, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 266, 352, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 274, 356, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 282, 360, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 290, 364, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 298, 368, 0, 0 }, { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 306, 372, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 2, 0, 3, 8, 0, 314, 376, 0, 0 }, { 1, 0, 0x40, 6, 0x55, 2, 0, 3, 8, 0, 322, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 270, 444, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 278, 440, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 286, 436, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 294, 432, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 302, 428, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 310, 424, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 358, 400, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 366, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 374, 392, 0, 0 }, { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 382, 388, 0, 0 },
    { 1, 0, 0x40, 6, 0x56, 2, 0, 3, 8, 0, 390, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x57, 2, 0, 3, 8, 0, 349, 405, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 3, 4, 0, 439, 395, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 540, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x5B, 2, 0, 3, 4, 0, 612, 416, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 646, 530, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 688, 509, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 602, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 790, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 581, 0, 0 },
    { 1, 0, 0x40, 6, 0x5F, 2, 0, 1, 4, 0, 832, 725, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 358, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 400, 653, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 519, 754, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 535, 458, 0, 0 }, { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 560, 733, 0, 0 },
    { 1, 0, 0x40, 6, 0x60, 2, 0, 1, 4, 0, 576, 437, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 648, 724, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 690, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 792, 653, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 833, 674, 0, 0 }, { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 936, 725, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 1, 4, 0, 978, 746, 0, 0 }, { 1, 0, 0x40, 6, 0x62, 2, 0, 1, 4, 0, 654, 640, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x41, 0xA, 0, 43, 551, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x41, 0xA, 0, 250, 504, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x25, 0xA, 0, 151, 577, 0, 0 },
    { 1, 0, 0x40, 6, 0x1B, 1, 0x1B, 0x25, 0xA, 0, 260, 393, 0, 0 },
    { 1, 0, 0xA0, 6, 0, 0, 0, 0, 0, 0, 792, 381, 0, 0 }, { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 721, 691, 0, 0 },
    { 1, 0, 0x60, 6, 1, 0, 0, 0, 0, 0, 865, 763, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 369, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 401, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 433, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 465, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 5, 4, 0, 775, 497, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x18, 1, 0x18, 0x1A, 6, 0, 546, 665, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x18, 1, 0x18, 0x1A, 6, 0, 833, 798, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 545, 558, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2A, 1, 0x2A, 0x2D, 6, 0, 834, 701, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, -12, 814, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, 0, 364, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, 100, 758, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, 212, 702, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, 324, 646, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, -14, 713, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 99, 658, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 210, 602, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2E, 1, 0x2E, 0x31, 0xA, 0, 322, 546, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x26, 1, 0x26, 0x29, 6, 0, 11, 408, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag321_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27A, 0x242, 0x13E, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x28B, 0x88, 0x1B4, 5, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 0xC, 1 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E0, 0x240, 0xD8, 1, 0, 0xC, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x130, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x120, 0x140, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag321_funcs = { wstag321_setup };
