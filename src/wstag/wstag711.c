#include "wstag.h"

/* WSTAG711: stage 0x2CD (fieldstg_stages). */

extern WstagFuncs wstag711_funcs;
extern FieldstgBattleLists wstag711_battle_lists;
extern FieldstgVramPlace wstag711_vram_places[];
extern FieldstgPlacedActor *wstag711_actors[];
extern FieldstgSprite wstag711_sprites[];
extern FieldstgMapEvent wstag711_map_events[];

void wstag711_update(WstagObject *obj) {
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

WstagObject *wstag711_start(void *arg0) {
    WstagObject *obj = object_new(wstag711_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag711_funcs.setup();
    return obj;
}

void wstag711_setup(void) {
    fieldstg_stage.background_file = 0x65D;
    fieldstg_stage.sprite_file = 0x065E0000;
    fieldstg_stage.sprites = wstag711_sprites;
    fieldstg_stage.map_events = wstag711_map_events;
    fieldstg_stage.mask_file = 0x65C;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x42D00, 0x25F00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag711_vram_places;
    fieldstg_stage.music = 0x2F;
    fieldstg_stage.sound = 0x60BC0000;
    fieldstg_stage.actors = wstag711_actors;
    fieldstg_stage.battle_lists = &wstag711_battle_lists;
    fieldstg_attr.set_file(0, 0x065E0001);
    fieldstg_attr.set_file(7, 0x065E0002);
    fieldstg_attr.set_file(4, 0x065E0003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag711_setup(void);

FieldstgListedBattle D_WSTAG711_800A5F94 = { 163, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FA0 = { 163, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FAC = { 163, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FB8 = { 163, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FC4 = { 137, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FD0 = { 137, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FDC = { 137, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A5FE8 = { 137, 4, 0x60080000 };
FieldstgBattleList D_WSTAG711_800A5FF4 = {
    3,
    { &D_WSTAG711_800A5F94, &D_WSTAG711_800A5FA0, &D_WSTAG711_800A5FAC, &D_WSTAG711_800A5FB8, &D_WSTAG711_800A5FC4,
        &D_WSTAG711_800A5FD0, &D_WSTAG711_800A5FDC, &D_WSTAG711_800A5FE8 },
};
FieldstgListedBattle D_WSTAG711_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG711_800A6078 = {
    0,
    { &D_WSTAG711_800A6018, &D_WSTAG711_800A6024, &D_WSTAG711_800A6030, &D_WSTAG711_800A603C, &D_WSTAG711_800A6048,
        &D_WSTAG711_800A6054, &D_WSTAG711_800A6060, &D_WSTAG711_800A606C },
};
FieldstgListedBattle D_WSTAG711_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG711_800A60FC = {
    0,
    { &D_WSTAG711_800A609C, &D_WSTAG711_800A60A8, &D_WSTAG711_800A60B4, &D_WSTAG711_800A60C0, &D_WSTAG711_800A60CC,
        &D_WSTAG711_800A60D8, &D_WSTAG711_800A60E4, &D_WSTAG711_800A60F0 },
};
FieldstgListedBattle D_WSTAG711_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6144 = { 333, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG711_800A6168 = { 175, 4, 0x60080000 };
FieldstgListedBattle D_WSTAG711_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG711_800A6180 = {
    0,
    { &D_WSTAG711_800A6120, &D_WSTAG711_800A612C, &D_WSTAG711_800A6138, &D_WSTAG711_800A6144, &D_WSTAG711_800A6150,
        &D_WSTAG711_800A615C, &D_WSTAG711_800A6168, &D_WSTAG711_800A6174 },
};
FieldstgBattleLists wstag711_battle_lists = {
    117, 0, 0, { &D_WSTAG711_800A5FF4, &D_WSTAG711_800A6078, &D_WSTAG711_800A60FC }, &D_WSTAG711_800A6180,
};
FieldstgVramPlace wstag711_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 412, 287, 368, 31, 352, 511 }, { 384, 256, 420, 327, 400, 71, 368, 511 },
    { 320, 256, 374, 449, 216, 193, 352, 510 }, { 384, 256, 422, 287, 408, 31, 368, 510 },
    { 384, 256, 430, 293, 440, 37, 320, 509 }, { 384, 256, 438, 293, 472, 37, 336, 509 },
    { 384, 256, 430, 325, 440, 69, 352, 509 }, { 384, 256, 438, 325, 472, 69, 368, 509 },
    { 384, 256, 412, 327, 368, 71, 320, 508 },
};
u16 D_WSTAG711_800A62B0[4] = { 0x868F, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A62B8[6] = { 0x868F, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG711_800A62C4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A62CC[8] = { 0x848B, 0, 0x868F, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A62DC[8] = { 0x868F, 0, 0, 1, 0x848B, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A62EC[10] = {
    0x7013, 1, 0x868F, 1, 0x868E, 0, 0x848B, 0,
    0xFFFF, 0,
};
FieldstgTalk D_WSTAG711_800A6300[5] = {
    { D_WSTAG711_800A62B0, NULL, 770 }, { D_WSTAG711_800A62B8, D_WSTAG711_800A62C4, 771 },
    { D_WSTAG711_800A62CC, NULL, 772 }, { D_WSTAG711_800A62DC, D_WSTAG711_800A62EC, 773 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG711_800A633C[2] = { { NULL, NULL, 648 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A6354[2] = { { NULL, NULL, 654 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A636C[2] = { { NULL, NULL, 650 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A6384[2] = { { NULL, NULL, 652 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A639C[2] = { { NULL, NULL, 655 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A63B4[2] = { { NULL, NULL, 651 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A63CC[2] = { { NULL, NULL, 651 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A63E4[2] = { { NULL, NULL, 649 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A63FC[2] = { { NULL, NULL, 649 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A6414[2] = { { NULL, NULL, 653 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A642C[2] = { { NULL, NULL, 653 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG711_800A6444[2] = { { NULL, NULL, 656 }, { NULL, NULL, 0 } };
u16 D_WSTAG711_800A645C[10] = {
    0x7048, 1, 0x7050, 1, 0x868E, 1, 0x868F, 0,
    0xFFFF, 0,
};
u16 D_WSTAG711_800A6470[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A647C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A6484[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A6490[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A649C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A64A4[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG711_800A64B0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A64B8[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG711_800A64C4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A64CC[6] = { 0x701E, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG711_800A64D8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG711_800A64E0[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG711_800A64E8 = { D_WSTAG711_800A645C, D_WSTAG711_800A6300, 31, 4, 255, 706, 7 };
FieldstgPlacedActor D_WSTAG711_800A64FC = { D_WSTAG711_800A6470, D_WSTAG711_800A633C, 45, 5, 1214, 517, 1 };
FieldstgPlacedActor D_WSTAG711_800A6510 = { D_WSTAG711_800A647C, D_WSTAG711_800A6354, 45, 5, 768, 369, 3 };
FieldstgPlacedActor D_WSTAG711_800A6524 = { D_WSTAG711_800A6484, D_WSTAG711_800A636C, 48, 6, 1089, 234, 7 };
FieldstgPlacedActor D_WSTAG711_800A6538 = { D_WSTAG711_800A6490, D_WSTAG711_800A6384, 49, 7, 465, 345, 1 };
FieldstgPlacedActor D_WSTAG711_800A654C = { D_WSTAG711_800A649C, D_WSTAG711_800A639C, 66, 8, 463, 193, 1 };
FieldstgPlacedActor D_WSTAG711_800A6560 = { D_WSTAG711_800A64A4, D_WSTAG711_800A63B4, 157, 9, 1089, 234, 7 };
FieldstgPlacedActor D_WSTAG711_800A6574 = { D_WSTAG711_800A64B0, D_WSTAG711_800A63CC, 157, 9, 1089, 234, 7 };
FieldstgPlacedActor D_WSTAG711_800A6588 = { D_WSTAG711_800A64B8, D_WSTAG711_800A63E4, 158, 10, 1214, 577, 1 };
FieldstgPlacedActor D_WSTAG711_800A659C = { D_WSTAG711_800A64C4, D_WSTAG711_800A63FC, 158, 10, 1214, 517, 1 };
FieldstgPlacedActor D_WSTAG711_800A65B0 = { D_WSTAG711_800A64CC, D_WSTAG711_800A6414, 159, 11, 465, 345, 1 };
FieldstgPlacedActor D_WSTAG711_800A65C4 = { D_WSTAG711_800A64D8, D_WSTAG711_800A642C, 159, 11, 465, 345, 1 };
FieldstgPlacedActor D_WSTAG711_800A65D8 = { D_WSTAG711_800A64E0, D_WSTAG711_800A6444, 180, 12, 1089, 234, 7 };
FieldstgPlacedActor *wstag711_actors[14] = {
    &D_WSTAG711_800A64E8, &D_WSTAG711_800A64FC, &D_WSTAG711_800A6510, &D_WSTAG711_800A6524, &D_WSTAG711_800A6538,
    &D_WSTAG711_800A654C, &D_WSTAG711_800A6560, &D_WSTAG711_800A6574, &D_WSTAG711_800A6588, &D_WSTAG711_800A659C,
    &D_WSTAG711_800A65B0, &D_WSTAG711_800A65C4, &D_WSTAG711_800A65D8, NULL,
};
FieldstgSprite wstag711_sprites[40] = {
    { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 603, 223, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 1, 6, 0, 952, 485, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 567, 357, 0, 0 }, { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 604, 375, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 6, 0, 857, 450, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 6, 0, 529, 660, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 198, 395, 0, 0 }, { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 221, 591, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 1084, 623, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 140, 499, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 476, 616, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 569, 552, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 6, 0, 827, 661, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 408, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 479, 678, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 772, 301, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 869, 528, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 919, 263, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 929, 414, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 1166, 732, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 1, 6, 0, 1218, 758, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 986, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 1025, 319, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 1, 6, 0, 1195, 732, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 1, 6, 0, 522, 632, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 2, 0, 1, 6, 0, 599, 379, 0, 0 }, { 1, 0, 0xC4, 4, 0, 0, 0, 0, 0, 0, 413, 523, 715, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 699, 190, 233, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 138, 383, 410, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 161, 328, 364, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 542, 196, 217, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 208, 311, 311, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 672, 719, 719, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 688, 455, 455, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 720, 743, 743, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 736, 479, 479, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 864, 175, 175, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 928, 351, 351, 0 },
    { 1, 0xFF, 0x64, 4, 0x38, 0, 0, 0, 0, 0, 944, 167, 167, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag711_map_events[18] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2CE, 0x390, 0x1E8, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2C9, 0x80, 0x90, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2D0, 0x390, 0x118, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x20F, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x21F, 0x190, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x34F, 0x106, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x35F, 0x16F, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x440, 0x161, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x430, 0x1C8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x3EF, 0x2C9, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x3E1, 0x32F, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x150, 0x258, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x15F, 0x2BE, 0, 0, 0, 0 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 5, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0xB, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 3, 1 },
    { 0x7094, 1, 0xFFFF, 0, 9, 0x2E8, 0x240, 0xD0, 1, 0, 0x17, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag711_funcs = { wstag711_setup };
