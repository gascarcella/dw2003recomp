#include "wstag.h"

/* WSTAG725: stage 0x268 (fieldstg_stages). */

extern WstagFuncs wstag725_funcs;
extern FieldstgBattleLists wstag725_battle_lists;
extern FieldstgVramPlace wstag725_vram_places[];
extern FieldstgPlacedActor *wstag725_actors[];
extern FieldstgSprite wstag725_sprites[];
extern FieldstgMapEvent wstag725_map_events[];

void wstag725_update(WstagObject *obj) {
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

WstagObject *wstag725_start(void *arg0) {
    WstagObject *obj = object_new(wstag725_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag725_funcs.setup();
    return obj;
}

void wstag725_setup(void) {
    fieldstg_stage.background_file = 0x6A3;
    fieldstg_stage.sprite_file = 0x06A40000;
    fieldstg_stage.sprites = wstag725_sprites;
    fieldstg_stage.map_events = wstag725_map_events;
    fieldstg_stage.mask_file = 0x6A2;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x37900, 0x10500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag725_vram_places;
    fieldstg_stage.music = 0x3F;
    fieldstg_stage.sound = 0x60FC0000;
    fieldstg_stage.actors = wstag725_actors;
    fieldstg_stage.battle_lists = &wstag725_battle_lists;
    fieldstg_attr.set_file(0, 0x06A40001);
    fieldstg_attr.set_file(7, 0x06A40002);
    fieldstg_attr.set_file(4, 0x06A40003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag725_setup(void);

FieldstgListedBattle D_WSTAG725_800A5F94 = { 161, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FA0 = { 161, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FAC = { 161, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FB8 = { 161, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FC4 = { 110, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FD0 = { 110, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FDC = { 110, 7, 0x60080000 };
FieldstgListedBattle D_WSTAG725_800A5FE8 = { 110, 7, 0x60080000 };
FieldstgBattleList D_WSTAG725_800A5FF4 = {
    3,
    { &D_WSTAG725_800A5F94, &D_WSTAG725_800A5FA0, &D_WSTAG725_800A5FAC, &D_WSTAG725_800A5FB8, &D_WSTAG725_800A5FC4,
        &D_WSTAG725_800A5FD0, &D_WSTAG725_800A5FDC, &D_WSTAG725_800A5FE8 },
};
FieldstgListedBattle D_WSTAG725_800A6018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A603C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A606C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG725_800A6078 = {
    0,
    { &D_WSTAG725_800A6018, &D_WSTAG725_800A6024, &D_WSTAG725_800A6030, &D_WSTAG725_800A603C, &D_WSTAG725_800A6048,
        &D_WSTAG725_800A6054, &D_WSTAG725_800A6060, &D_WSTAG725_800A606C },
};
FieldstgListedBattle D_WSTAG725_800A609C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A60F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG725_800A60FC = {
    0,
    { &D_WSTAG725_800A609C, &D_WSTAG725_800A60A8, &D_WSTAG725_800A60B4, &D_WSTAG725_800A60C0, &D_WSTAG725_800A60CC,
        &D_WSTAG725_800A60D8, &D_WSTAG725_800A60E4, &D_WSTAG725_800A60F0 },
};
FieldstgListedBattle D_WSTAG725_800A6120 = { 223, 7, 0x600C0000 };
FieldstgListedBattle D_WSTAG725_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A615C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG725_800A6174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG725_800A6180 = {
    0,
    { &D_WSTAG725_800A6120, &D_WSTAG725_800A612C, &D_WSTAG725_800A6138, &D_WSTAG725_800A6144, &D_WSTAG725_800A6150,
        &D_WSTAG725_800A615C, &D_WSTAG725_800A6168, &D_WSTAG725_800A6174 },
};
FieldstgBattleLists wstag725_battle_lists = {
    86, 0, 0, { &D_WSTAG725_800A5FF4, &D_WSTAG725_800A6078, &D_WSTAG725_800A60FC }, &D_WSTAG725_800A6180,
};
FieldstgVramPlace wstag725_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 322, 136, 66, 352, 511 }, { 320, 256, 368, 288, 192, 32, 368, 511 },
    { 320, 256, 362, 328, 168, 72, 352, 510 }, { 320, 256, 370, 328, 200, 72, 368, 510 },
};
u16 D_WSTAG725_800A6260[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6268[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6270[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A627C[6] = { 0, 1, 0x720A, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6288[4] = { 0x7621, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6290[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6298[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A62A4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A62AC[6] = { 0x10, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A62B8[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A62C4[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A62CC[6] = { 0x10, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A62D8[4] = { 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A62E0[8] = { 0x10, 1, 0x9219, 0, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A62F0[10] = {
    0x9219, 1, 0x11, 0, 0x7013, 1, 0x10, 0,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A6304[8] = { 0x10, 1, 0x9219, 1, 0x11, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6314[6] = { 0x11, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6320[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6328[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6330[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A633C[6] = { 0, 1, 0x720A, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6348[4] = { 0x7621, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6350[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6358[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6360[6] = { 0, 1, 0xE17, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A636C[6] = { 0xE17, 1, 0x7400, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6378[8] = { 0, 1, 0xE17, 1, 0x720E, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A6388[8] = { 0, 1, 0xE17, 1, 0x720E, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6398[4] = { 0x7821, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A63A0[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A63A8[6] = { 0, 1, 0x720A, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A63B4[6] = { 0, 1, 0x720A, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A63C0[4] = { 0x7621, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG725_800A63C8[2] = { { NULL, NULL, 717 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A63E0[2] = { { NULL, NULL, 720 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A63F8[2] = { { NULL, NULL, 718 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A6410[2] = { { NULL, NULL, 719 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A6428[4] = {
    { D_WSTAG725_800A6260, D_WSTAG725_800A6268, 416 }, { D_WSTAG725_800A6270, NULL, 419 },
    { D_WSTAG725_800A627C, D_WSTAG725_800A6288, 420 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A6458[2] = { { NULL, NULL, 666 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A6470[4] = {
    { D_WSTAG725_800A6290, NULL, 416 }, { D_WSTAG725_800A6298, D_WSTAG725_800A62A4, 425 },
    { D_WSTAG725_800A62AC, D_WSTAG725_800A62B8, 426 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A64A0[5] = {
    { D_WSTAG725_800A62C4, NULL, 416 }, { D_WSTAG725_800A62CC, D_WSTAG725_800A62D8, 427 },
    { D_WSTAG725_800A62E0, D_WSTAG725_800A62F0, 428 }, { D_WSTAG725_800A6304, D_WSTAG725_800A6314, 429 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A64DC[4] = {
    { D_WSTAG725_800A6320, D_WSTAG725_800A6328, 417 }, { D_WSTAG725_800A6330, NULL, 419 },
    { D_WSTAG725_800A633C, D_WSTAG725_800A6348, 420 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A650C[5] = {
    { D_WSTAG725_800A6350, D_WSTAG725_800A6358, 421 }, { D_WSTAG725_800A6360, D_WSTAG725_800A636C, 422 },
    { D_WSTAG725_800A6378, NULL, 423 }, { D_WSTAG725_800A6388, D_WSTAG725_800A6398, 424 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A6548[4] = {
    { D_WSTAG725_800A63A0, NULL, 418 }, { D_WSTAG725_800A63A8, NULL, 419 },
    { D_WSTAG725_800A63B4, D_WSTAG725_800A63C0, 420 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG725_800A6578[2] = { { NULL, NULL, 721 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A6590[2] = { { NULL, NULL, 724 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A65A8[2] = { { NULL, NULL, 722 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG725_800A65C0[2] = { { NULL, NULL, 723 }, { NULL, NULL, 0 } };
u16 D_WSTAG725_800A65D8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A65E0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A65E8[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A65F0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A65F8[10] = {
    0x8192, 1, 0x11, 0, 0x7004, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A660C[8] = { 0x8192, 0, 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG725_800A661C[10] = {
    0x11, 1, 0x700A, 1, 0x8192, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A6630[10] = {
    0x11, 1, 0x8012, 1, 0x8192, 1, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A6644[10] = {
    0x8192, 1, 0x11, 0, 0x6026, 1, 0x8012, 0,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A6658[10] = {
    0x8012, 1, 0x8192, 1, 0x11, 0, 0x7022, 1,
    0xFFFF, 0,
};
u16 D_WSTAG725_800A666C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6674[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A667C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A6684[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG725_800A668C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG725_800A6694 = { D_WSTAG725_800A65D8, D_WSTAG725_800A63C8, 34, 4, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG725_800A66A8 = { D_WSTAG725_800A65E0, D_WSTAG725_800A63E0, 34, 4, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG725_800A66BC = { D_WSTAG725_800A65E8, D_WSTAG725_800A63F8, 34, 4, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG725_800A66D0 = { D_WSTAG725_800A65F0, D_WSTAG725_800A6410, 34, 4, 161, 321, 1 };
FieldstgPlacedActor D_WSTAG725_800A66E4 = { D_WSTAG725_800A65F8, D_WSTAG725_800A6428, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A66F8 = { D_WSTAG725_800A660C, D_WSTAG725_800A6458, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A670C = { D_WSTAG725_800A661C, D_WSTAG725_800A6470, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A6720 = { D_WSTAG725_800A6630, D_WSTAG725_800A64A0, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A6734 = { D_WSTAG725_800A6644, D_WSTAG725_800A64DC, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A6748 = { D_WSTAG725_800A6658, D_WSTAG725_800A650C, 51, 5, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A675C = { D_WSTAG725_800A666C, D_WSTAG725_800A6548, 157, 6, 816, 209, 1 };
FieldstgPlacedActor D_WSTAG725_800A6770 = { D_WSTAG725_800A6674, D_WSTAG725_800A6578, 181, 7, 692, 265, 3 };
FieldstgPlacedActor D_WSTAG725_800A6784 = { D_WSTAG725_800A667C, D_WSTAG725_800A6590, 181, 7, 495, 75, 7 };
FieldstgPlacedActor D_WSTAG725_800A6798 = { D_WSTAG725_800A6684, D_WSTAG725_800A65A8, 181, 7, 692, 265, 3 };
FieldstgPlacedActor D_WSTAG725_800A67AC = { D_WSTAG725_800A668C, D_WSTAG725_800A65C0, 181, 7, 692, 265, 3 };
FieldstgPlacedActor *wstag725_actors[16] = {
    &D_WSTAG725_800A6694, &D_WSTAG725_800A66A8, &D_WSTAG725_800A66BC, &D_WSTAG725_800A66D0, &D_WSTAG725_800A66E4,
    &D_WSTAG725_800A66F8, &D_WSTAG725_800A670C, &D_WSTAG725_800A6720, &D_WSTAG725_800A6734, &D_WSTAG725_800A6748,
    &D_WSTAG725_800A675C, &D_WSTAG725_800A6770, &D_WSTAG725_800A6784, &D_WSTAG725_800A6798, &D_WSTAG725_800A67AC,
    NULL,
};
FieldstgSprite wstag725_sprites[29] = {
    { 1, 0, 0x40, 2, 0x33, 2, 0, 3, 6, 0, 807, 124, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 647, 322, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 714, 356, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 755, 312, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 773, 350, 0, 0 }, { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 830, 322, 0, 0 },
    { 1, 0, 0x40, 2, 0x36, 2, 0, 3, 6, 0, 831, 297, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 289, 211, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 6, 0, 317, 214, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 3, 4, 0, 762, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 287, 282, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 482, 16, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 566, 355, 0, 0 }, { 1, 0, 0x40, 6, 0x33, 2, 0, 3, 6, 0, 630, 94, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 243, 466, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 435, 24, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 467, 302, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 6, 0, 909, 158, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 330, 362, 0, 0 }, { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 339, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 6, 0, 658, 77, 0, 0 }, { 1, 0, 0x40, 6, 0x37, 3, 0, 9, 8, 0, 105, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 3, 0, 9, 8, 0, 143, 271, 0, 0 }, { 1, 0, 0x40, 4, 0x38, 3, 0, 9, 8, 0, 77, 305, 328, 0 },
    { 1, 0, 0x44, 4, 0, 0, 0, 0, 0, 0, 635, 206, 246, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 464, 183, 220, 0 },
    { 1, 0, 0x42, 4, 2, 0, 0, 0, 0, 0, 61, 264, 328, 0 }, { 1, 0, 0x50, 4, 3, 0, 0, 0, 0, 0, 800, 104, 175, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag725_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x26A, 0x508, 0x54, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x269, 0xA8, 0xF4, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x265, 0x220, 0x2C0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 6, 0x200, 0x60, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 6, 0x210, 0xC8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 9, 0x220, 0x100, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 9, 0x210, 0x198, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag725_funcs = { wstag725_setup };
