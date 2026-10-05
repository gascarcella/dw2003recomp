#include "wstag.h"

/* WSTAG230: stage 0x208 (fieldstg_stages). */

extern WstagFuncs wstag230_funcs;
extern FieldstgVramPlace wstag230_vram_places[];
extern FieldstgPlacedActor *wstag230_actors[];
extern FieldstgSprite wstag230_sprites[];
extern FieldstgMapEvent wstag230_map_events[];

void wstag230_update(WstagObject *obj) {
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

WstagObject *wstag230_start(void *arg0) {
    WstagObject *obj = object_new(wstag230_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag230_funcs.setup();
    return obj;
}

void wstag230_setup(void) {
    fieldstg_stage.background_file = 0x1A0;
    fieldstg_stage.sprite_file = 0x01A10000;
    fieldstg_stage.sprites = wstag230_sprites;
    fieldstg_stage.map_events = wstag230_map_events;
    fieldstg_stage.mask_file = 0x396;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0xDE00, 0xDE00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag230_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag230_actors;
    fieldstg_attr.set_file(0, 0x01A10001);
    fieldstg_attr.set_file(7, 0x01A10003);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x14 && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag230_setup(void);

FieldstgVramPlace wstag230_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 372, 328, 208, 72, 368, 511 }, { 320, 256, 348, 353, 112, 97, 352, 510 },
    { 320, 256, 356, 353, 144, 97, 368, 510 }, { 320, 256, 372, 368, 208, 112, 336, 509 },
    { 320, 256, 320, 373, 0, 117, 352, 509 }, { 320, 256, 328, 373, 32, 117, 368, 509 },
    { 320, 256, 336, 373, 64, 117, 336, 508 }, { 320, 256, 364, 385, 176, 129, 352, 508 },
    { 320, 256, 344, 393, 96, 137, 368, 508 },
};
FieldstgTalk D_WSTAG230_800A60A0[2] = { { NULL, NULL, 457 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A60B8[2] = { { NULL, NULL, 434 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A60D0[2] = { { NULL, NULL, 435 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A60E8[2] = { { NULL, NULL, 337 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6100[2] = { { NULL, NULL, 436 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6118[2] = { { NULL, NULL, 437 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6130[2] = { { NULL, NULL, 438 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6148[2] = { { NULL, NULL, 439 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6160[2] = { { NULL, NULL, 440 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6178[2] = { { NULL, NULL, 441 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6190[2] = { { NULL, NULL, 454 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A61A8[2] = { { NULL, NULL, 456 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A61C0[2] = { { NULL, NULL, 458 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A61D8[2] = { { NULL, NULL, 464 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A61F0[2] = { { NULL, NULL, 459 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6208[2] = { { NULL, NULL, 461 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6220[2] = { { NULL, NULL, 475 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6238[2] = { { NULL, NULL, 494 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6250[2] = { { NULL, NULL, 495 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6268[2] = { { NULL, NULL, 460 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6280[2] = { { NULL, NULL, 462 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6298[2] = { { NULL, NULL, 463 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A62B0[2] = { { NULL, NULL, 497 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A62C8[2] = { { NULL, NULL, 498 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A62E0[2] = { { NULL, NULL, 504 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A62F8[2] = { { NULL, NULL, 499 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6310[2] = { { NULL, NULL, 501 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6328[2] = { { NULL, NULL, 505 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6340[2] = { { NULL, NULL, 506 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6358[2] = { { NULL, NULL, 507 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6370[2] = { { NULL, NULL, 500 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6388[2] = { { NULL, NULL, 502 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A63A0[2] = { { NULL, NULL, 503 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A63B8[2] = { { NULL, NULL, 509 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A63D0[2] = { { NULL, NULL, 508 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A63E8[2] = { { NULL, NULL, 496 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG230_800A6400[2] = { { NULL, NULL, 455 }, { NULL, NULL, 0 } };
u16 D_WSTAG230_800A6418[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6420[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6428[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6430[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6438[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6440[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6448[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6450[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6458[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6460[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6468[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6470[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6478[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6480[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6488[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6490[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6498[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64A0[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64A8[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64B0[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64B8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64C0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64C8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64D0[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64D8[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64E0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64E8[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64F0[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A64F8[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6500[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6508[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6510[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6518[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6520[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6528[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6530[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6538[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6540[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6548[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6550[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6558[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG230_800A6560[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG230_800A6568 = { D_WSTAG230_800A6418, D_WSTAG230_800A60A0, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A657C = { D_WSTAG230_800A6420, D_WSTAG230_800A60B8, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A6590 = { D_WSTAG230_800A6428, D_WSTAG230_800A60D0, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A65A4 = { D_WSTAG230_800A6430, D_WSTAG230_800A60E8, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A65B8 = { D_WSTAG230_800A6438, D_WSTAG230_800A6100, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A65CC = { D_WSTAG230_800A6440, D_WSTAG230_800A6118, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A65E0 = { D_WSTAG230_800A6448, D_WSTAG230_800A6130, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A65F4 = { D_WSTAG230_800A6450, D_WSTAG230_800A6148, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A6608 = { D_WSTAG230_800A6458, D_WSTAG230_800A6160, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A661C = { D_WSTAG230_800A6460, D_WSTAG230_800A6178, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A6630 = { D_WSTAG230_800A6468, D_WSTAG230_800A6190, 32, 4, 277, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A6644 = { D_WSTAG230_800A6470, D_WSTAG230_800A61A8, 32, 4, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A6658 = { D_WSTAG230_800A6478, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG230_800A666C = { D_WSTAG230_800A6480, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG230_800A6680 = { D_WSTAG230_800A6488, NULL, 36, 5, 240, 329, 3 };
FieldstgPlacedActor D_WSTAG230_800A6694 = { D_WSTAG230_800A6490, D_WSTAG230_800A61C0, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A66A8 = { D_WSTAG230_800A6498, D_WSTAG230_800A61D8, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A66BC = { D_WSTAG230_800A64A0, D_WSTAG230_800A61F0, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A66D0 = { D_WSTAG230_800A64A8, D_WSTAG230_800A6208, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A66E4 = { D_WSTAG230_800A64B0, D_WSTAG230_800A6220, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A66F8 = { D_WSTAG230_800A64B8, D_WSTAG230_800A6238, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A670C = { D_WSTAG230_800A64C0, D_WSTAG230_800A6250, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A6720 = { D_WSTAG230_800A64C8, D_WSTAG230_800A6268, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A6734 = { D_WSTAG230_800A64D0, D_WSTAG230_800A6280, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A6748 = { D_WSTAG230_800A64D8, D_WSTAG230_800A6298, 52, 6, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A675C = { D_WSTAG230_800A64E0, D_WSTAG230_800A62B0, 52, 6, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6770 = { D_WSTAG230_800A64E8, D_WSTAG230_800A62C8, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6784 = { D_WSTAG230_800A64F0, D_WSTAG230_800A62E0, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6798 = { D_WSTAG230_800A64F8, D_WSTAG230_800A62F8, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A67AC = { D_WSTAG230_800A6500, D_WSTAG230_800A6310, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A67C0 = { D_WSTAG230_800A6508, D_WSTAG230_800A6328, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A67D4 = { D_WSTAG230_800A6510, D_WSTAG230_800A6340, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A67E8 = { D_WSTAG230_800A6518, D_WSTAG230_800A6358, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A67FC = { D_WSTAG230_800A6520, D_WSTAG230_800A6370, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6810 = { D_WSTAG230_800A6528, D_WSTAG230_800A6388, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6824 = { D_WSTAG230_800A6530, D_WSTAG230_800A63A0, 57, 7, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6838 = { D_WSTAG230_800A6538, D_WSTAG230_800A63B8, 57, 7, 240, 327, 3 };
FieldstgPlacedActor D_WSTAG230_800A684C = { D_WSTAG230_800A6540, NULL, 106, 8, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG230_800A6860 = { D_WSTAG230_800A6548, D_WSTAG230_800A63D0, 157, 9, 237, 202, 3 };
FieldstgPlacedActor D_WSTAG230_800A6874 = { D_WSTAG230_800A6550, D_WSTAG230_800A63E8, 158, 10, 352, 239, 1 };
FieldstgPlacedActor D_WSTAG230_800A6888 = { D_WSTAG230_800A6558, D_WSTAG230_800A6400, 159, 11, 272, 344, 7 };
FieldstgPlacedActor D_WSTAG230_800A689C = { D_WSTAG230_800A6560, NULL, 160, 12, 240, 329, 3 };
FieldstgPlacedActor *wstag230_actors[43] = {
    &D_WSTAG230_800A6568, &D_WSTAG230_800A657C, &D_WSTAG230_800A6590, &D_WSTAG230_800A65A4, &D_WSTAG230_800A65B8,
    &D_WSTAG230_800A65CC, &D_WSTAG230_800A65E0, &D_WSTAG230_800A65F4, &D_WSTAG230_800A6608, &D_WSTAG230_800A661C,
    &D_WSTAG230_800A6630, &D_WSTAG230_800A6644, &D_WSTAG230_800A6658, &D_WSTAG230_800A666C, &D_WSTAG230_800A6680,
    &D_WSTAG230_800A6694, &D_WSTAG230_800A66A8, &D_WSTAG230_800A66BC, &D_WSTAG230_800A66D0, &D_WSTAG230_800A66E4,
    &D_WSTAG230_800A66F8, &D_WSTAG230_800A670C, &D_WSTAG230_800A6720, &D_WSTAG230_800A6734, &D_WSTAG230_800A6748,
    &D_WSTAG230_800A675C, &D_WSTAG230_800A6770, &D_WSTAG230_800A6784, &D_WSTAG230_800A6798, &D_WSTAG230_800A67AC,
    &D_WSTAG230_800A67C0, &D_WSTAG230_800A67D4, &D_WSTAG230_800A67E8, &D_WSTAG230_800A67FC, &D_WSTAG230_800A6810,
    &D_WSTAG230_800A6824, &D_WSTAG230_800A6838, &D_WSTAG230_800A684C, &D_WSTAG230_800A6860, &D_WSTAG230_800A6874,
    &D_WSTAG230_800A6888, &D_WSTAG230_800A689C, NULL,
};
FieldstgSprite wstag230_sprites[24] = {
    { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 192, 256, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 144, 104, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 168, 92, 0, 0 }, { 1, 0, 0x40, 6, 0x32, 2, 0, 1, 4, 0, 224, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 127, 144, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 159, 128, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 80, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 191, 112, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 64, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 0xB, 8, 0, 223, 96, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 196, 289, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 0xB, 8, 0, 226, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 266, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 0xB, 8, 0, 212, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 196, 275, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 2, 0, 0xB, 8, 0, 226, 260, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 1, 0x37, 0x39, 0xA, 0, 200, 290, 0, 0 },
    { 1, 0, 0x40, 6, 2, 0, 0, 0, 0, 0, 256, 336, 0, 0 },
    { 1, 0, 0x40, 4, 0x3A, 1, 0x3A, 0x3C, 0xA, 0, 256, 286, 315, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 304, 312, 329, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 256, 280, 315, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag230_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x203, 0x200, 0x11C, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag230_funcs = { wstag230_setup };
