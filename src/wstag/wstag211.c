#include "wstag.h"

/* WSTAG211: stage 0x273 (fieldstg_stages). */

extern WstagFuncs wstag211_funcs;
extern CVECTOR wstag211_color;
extern FieldstgBattleLists wstag211_battle_lists;
extern FieldstgVramPlace wstag211_vram_places[];
extern FieldstgPlacedActor *wstag211_actors[];
extern FieldstgSprite wstag211_sprites[];
extern FieldstgMapEvent wstag211_map_events[];
extern FieldstgEventDef wstag211_events[];

void wstag211_update(WstagObject *obj) {
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

WstagObject *wstag211_start(void *arg0) {
    WstagObject *obj = object_new(wstag211_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag211_funcs.setup();
    return obj;
}

void wstag211_event_925_end(void) {
    gamestate_flags.set_flag(0xC23, 1);
    gamestate_flags.set_flag(0x40A4, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag211_setup(void) {
    fieldstg_stage.background_file = 0x4A3;
    fieldstg_stage.sprite_file = 0x04A40000;
    fieldstg_stage.sprites = wstag211_sprites;
    fieldstg_stage.map_events = wstag211_map_events;
    fieldstg_stage.mask_file = 0x4A2;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x14B00, 0x13400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag211_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag211_actors;
    fieldstg_stage.color = wstag211_color;
    fieldstg_stage.battle_lists = &wstag211_battle_lists;
    fieldstg_stage.events = wstag211_events;
    fieldstg_attr.set_file(0, 0x04A40001);
    fieldstg_attr.set_file(7, 0x04A40002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag211/nonmatchings/wstag211", wstag211_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag211_setup(void);

s16 D_WSTAG211_800A6050[92] = {
    FIELDSTG_EVENT_WALK(2, 288, 225, 5),
    FIELDSTG_EVENT_PLACE(194, 329, 205),
    FIELDSTG_EVENT_ANIM(194, 1, 1),
    FIELDSTG_EVENT_PLACE(195, 352, 217),
    FIELDSTG_EVENT_ANIM(195, 1, 1),
    FIELDSTG_EVENT_PLACE(196, 304, 192),
    FIELDSTG_EVENT_ANIM(196, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 194),
    FIELDSTG_EVENT_ANIM(0x324, 805, 195),
    FIELDSTG_EVENT_ANIM(0x325, 805, 196),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 194),
    FIELDSTG_EVENT_ANIM(0x324, 806, 195),
    FIELDSTG_EVENT_ANIM(0x325, 806, 196),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(194, 321, 209, 1),
    FIELDSTG_EVENT_WAIT_WALK(194),
    FIELDSTG_EVENT_ANIM(194, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 194, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x6961, /* padding, not read */
};
FieldstgListedBattle D_WSTAG211_800A6108 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6114 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6120 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A612C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A615C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG211_800A6168 = {
    0,
    { &D_WSTAG211_800A6108, &D_WSTAG211_800A6114, &D_WSTAG211_800A6120, &D_WSTAG211_800A612C, &D_WSTAG211_800A6138,
        &D_WSTAG211_800A6144, &D_WSTAG211_800A6150, &D_WSTAG211_800A615C },
};
FieldstgListedBattle D_WSTAG211_800A618C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6198 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A61E0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG211_800A61EC = {
    0,
    { &D_WSTAG211_800A618C, &D_WSTAG211_800A6198, &D_WSTAG211_800A61A4, &D_WSTAG211_800A61B0, &D_WSTAG211_800A61BC,
        &D_WSTAG211_800A61C8, &D_WSTAG211_800A61D4, &D_WSTAG211_800A61E0 },
};
FieldstgListedBattle D_WSTAG211_800A6210 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A621C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6228 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6234 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6240 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A624C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6258 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A6264 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG211_800A6270 = {
    0,
    { &D_WSTAG211_800A6210, &D_WSTAG211_800A621C, &D_WSTAG211_800A6228, &D_WSTAG211_800A6234, &D_WSTAG211_800A6240,
        &D_WSTAG211_800A624C, &D_WSTAG211_800A6258, &D_WSTAG211_800A6264 },
};
FieldstgListedBattle D_WSTAG211_800A6294 = { 195, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG211_800A62A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62C4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62D0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62DC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG211_800A62E8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG211_800A62F4 = {
    0,
    { &D_WSTAG211_800A6294, &D_WSTAG211_800A62A0, &D_WSTAG211_800A62AC, &D_WSTAG211_800A62B8, &D_WSTAG211_800A62C4,
        &D_WSTAG211_800A62D0, &D_WSTAG211_800A62DC, &D_WSTAG211_800A62E8 },
};
FieldstgBattleLists wstag211_battle_lists = {
    143, 0, 0, { &D_WSTAG211_800A6168, &D_WSTAG211_800A61EC, &D_WSTAG211_800A6270 }, &D_WSTAG211_800A62F4,
};
FieldstgVramPlace wstag211_vram_places[33] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 496, 352, 704, 96, 368, 504 }, { 448, 256, 448, 369, 512, 113, 320, 503 },
    { 384, 256, 408, 432, 352, 176, 336, 503 }, { 448, 256, 502, 256, 728, 0, 352, 503 },
    { 448, 256, 502, 304, 728, 48, 368, 503 }, { 448, 256, 504, 352, 736, 96, 320, 502 },
    { 448, 256, 456, 369, 544, 113, 336, 502 }, { 448, 256, 472, 372, 608, 116, 352, 502 },
    { 448, 256, 480, 372, 640, 116, 368, 502 }, { 448, 256, 488, 372, 672, 116, 320, 501 },
    { 448, 256, 456, 409, 544, 153, 336, 501 }, { 448, 256, 464, 340, 576, 84, 352, 501 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 448, 256, 464, 428, 576, 172, 368, 501 },
    { 448, 256, 492, 432, 688, 176, 320, 500 }, { 448, 256, 500, 432, 720, 176, 336, 500 },
    { 448, 256, 472, 436, 608, 180, 352, 500 }, { 448, 256, 480, 436, 640, 180, 368, 500 },
    { 448, 256, 456, 441, 544, 185, 320, 499 }, { 448, 256, 448, 449, 512, 193, 336, 499 },
    { 448, 256, 464, 460, 576, 204, 352, 499 }, { 448, 256, 464, 388, 576, 132, 368, 499 },
    { 448, 256, 496, 392, 704, 136, 320, 498 }, { 448, 256, 448, 409, 512, 153, 336, 498 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG211_800A6544[6] = { 0x7400, 1, 0xC23, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6550[6] = { 0x7400, 1, 0xC23, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A655C[6] = { 0x7400, 1, 0xC23, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG211_800A6568[2] = { { NULL, NULL, 51 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6580[2] = { { NULL, NULL, 54 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6598[2] = { { NULL, NULL, 55 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A65B0[2] = { { NULL, NULL, 58 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A65C8[2] = { { NULL, NULL, 67 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A65E0[2] = { { NULL, NULL, 69 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A65F8[2] = { { NULL, NULL, 71 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6610[2] = { { NULL, NULL, 66 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6628[2] = { { NULL, NULL, 62 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6640[2] = { { NULL, NULL, 63 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6658[2] = { { NULL, NULL, 59 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6670[2] = { { NULL, NULL, 75 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6688[2] = { { NULL, NULL, 76 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A66A0[2] = { { NULL, NULL, 73 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A66B8[2] = { { NULL, NULL, 52 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A66D0[2] = { { NULL, NULL, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A66E8[2] = { { NULL, NULL, 56 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6700[2] = { { NULL, NULL, 57 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6718[2] = { { NULL, NULL, 60 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6730[2] = { { NULL, NULL, 61 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6748[2] = { { NULL, NULL, 64 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6760[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6778[2] = { { NULL, NULL, 68 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6790[2] = { { NULL, NULL, 70 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A67A8[2] = { { NULL, NULL, 72 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A67C0[2] = { { NULL, NULL, 74 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A67D8[2] = { { NULL, D_WSTAG211_800A6544, 439 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A67F0[2] = { { NULL, D_WSTAG211_800A6550, 440 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG211_800A6808[2] = { { NULL, D_WSTAG211_800A655C, 441 }, { NULL, NULL, 0 } };
u16 D_WSTAG211_800A6820[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A682C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6834[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6840[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6848[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6850[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6858[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6860[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6868[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6870[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A687C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6888[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6890[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6898[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68A0[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68A8[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68B4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68BC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68C8[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A68D4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68DC[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A68E8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A68F0[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A68FC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6904[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A6910[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6918[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6920[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6928[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6930[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6938[6] = { 0xC23, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A6944[6] = { 0x6025, 1, 0xC23, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A6950[6] = { 0xC23, 0, 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG211_800A695C[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A6968[6] = { 0x7094, 1, 0x6026, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A6974[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG211_800A6980[6] = { 0x6026, 0, 0x7094, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG211_800A698C = { D_WSTAG211_800A6820, D_WSTAG211_800A6568, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG211_800A69A0 = { D_WSTAG211_800A682C, D_WSTAG211_800A6580, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG211_800A69B4 = { D_WSTAG211_800A6834, D_WSTAG211_800A6598, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG211_800A69C8 = { D_WSTAG211_800A6840, D_WSTAG211_800A65B0, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG211_800A69DC = { D_WSTAG211_800A6848, D_WSTAG211_800A65C8, 37, 6, 304, 193, 7 };
FieldstgPlacedActor D_WSTAG211_800A69F0 = { D_WSTAG211_800A6850, D_WSTAG211_800A65E0, 38, 7, 305, 353, 1 };
FieldstgPlacedActor D_WSTAG211_800A6A04 = { D_WSTAG211_800A6858, D_WSTAG211_800A65F8, 39, 8, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG211_800A6A18 = { D_WSTAG211_800A6860, D_WSTAG211_800A6610, 45, 9, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG211_800A6A2C = { D_WSTAG211_800A6868, D_WSTAG211_800A6628, 48, 10, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG211_800A6A40 = { D_WSTAG211_800A6870, D_WSTAG211_800A6640, 49, 11, 328, 260, 5 };
FieldstgPlacedActor D_WSTAG211_800A6A54 = { D_WSTAG211_800A687C, D_WSTAG211_800A6658, 50, 12, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG211_800A6A68 = { D_WSTAG211_800A6888, D_WSTAG211_800A6670, 55, 13, 391, 359, 1 };
FieldstgPlacedActor D_WSTAG211_800A6A7C = { D_WSTAG211_800A6890, D_WSTAG211_800A6688, 57, 14, 684, 363, 7 };
FieldstgPlacedActor D_WSTAG211_800A6A90 = { D_WSTAG211_800A6898, D_WSTAG211_800A66A0, 111, 15, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG211_800A6AA4 = { D_WSTAG211_800A68A0, NULL, 112, 16, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG211_800A6AB8 = { D_WSTAG211_800A68A8, NULL, 112, 16, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG211_800A6ACC = { D_WSTAG211_800A68B4, NULL, 113, 17, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG211_800A6AE0 = { D_WSTAG211_800A68BC, NULL, 113, 17, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG211_800A6AF4 = { D_WSTAG211_800A68C8, D_WSTAG211_800A66B8, 157, 18, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG211_800A6B08 = { D_WSTAG211_800A68D4, D_WSTAG211_800A66D0, 157, 18, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG211_800A6B1C = { D_WSTAG211_800A68DC, D_WSTAG211_800A66E8, 158, 19, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG211_800A6B30 = { D_WSTAG211_800A68E8, D_WSTAG211_800A6700, 158, 19, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG211_800A6B44 = { D_WSTAG211_800A68F0, D_WSTAG211_800A6718, 159, 20, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG211_800A6B58 = { D_WSTAG211_800A68FC, D_WSTAG211_800A6730, 159, 20, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG211_800A6B6C = { D_WSTAG211_800A6904, D_WSTAG211_800A6748, 160, 21, 328, 260, 5 };
FieldstgPlacedActor D_WSTAG211_800A6B80 = { D_WSTAG211_800A6910, D_WSTAG211_800A6760, 160, 21, 328, 260, 5 };
FieldstgPlacedActor D_WSTAG211_800A6B94 = { D_WSTAG211_800A6918, D_WSTAG211_800A6778, 161, 22, 304, 193, 7 };
FieldstgPlacedActor D_WSTAG211_800A6BA8 = { D_WSTAG211_800A6920, D_WSTAG211_800A6790, 162, 23, 305, 303, 1 };
FieldstgPlacedActor D_WSTAG211_800A6BBC = { D_WSTAG211_800A6928, D_WSTAG211_800A67A8, 174, 24, 193, 257, 5 };
FieldstgPlacedActor D_WSTAG211_800A6BD0 = { D_WSTAG211_800A6930, D_WSTAG211_800A67C0, 175, 25, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG211_800A6BE4 = { D_WSTAG211_800A6938, D_WSTAG211_800A67D8, 194, 26, 329, 205, 1 };
FieldstgPlacedActor D_WSTAG211_800A6BF8 = { D_WSTAG211_800A6944, D_WSTAG211_800A67F0, 195, 27, 352, 217, 1 };
FieldstgPlacedActor D_WSTAG211_800A6C0C = { D_WSTAG211_800A6950, D_WSTAG211_800A6808, 196, 28, 304, 192, 1 };
FieldstgPlacedActor D_WSTAG211_800A6C20 = { D_WSTAG211_800A695C, NULL, 270, 29, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG211_800A6C34 = { D_WSTAG211_800A6968, NULL, 270, 29, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG211_800A6C48 = { D_WSTAG211_800A6974, NULL, 271, 30, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG211_800A6C5C = { D_WSTAG211_800A6980, NULL, 271, 30, 450, 254, 7 };
FieldstgPlacedActor *wstag211_actors[38] = {
    &D_WSTAG211_800A698C, &D_WSTAG211_800A69A0, &D_WSTAG211_800A69B4, &D_WSTAG211_800A69C8, &D_WSTAG211_800A69DC,
    &D_WSTAG211_800A69F0, &D_WSTAG211_800A6A04, &D_WSTAG211_800A6A18, &D_WSTAG211_800A6A2C, &D_WSTAG211_800A6A40,
    &D_WSTAG211_800A6A54, &D_WSTAG211_800A6A68, &D_WSTAG211_800A6A7C, &D_WSTAG211_800A6A90, &D_WSTAG211_800A6AA4,
    &D_WSTAG211_800A6AB8, &D_WSTAG211_800A6ACC, &D_WSTAG211_800A6AE0, &D_WSTAG211_800A6AF4, &D_WSTAG211_800A6B08,
    &D_WSTAG211_800A6B1C, &D_WSTAG211_800A6B30, &D_WSTAG211_800A6B44, &D_WSTAG211_800A6B58, &D_WSTAG211_800A6B6C,
    &D_WSTAG211_800A6B80, &D_WSTAG211_800A6B94, &D_WSTAG211_800A6BA8, &D_WSTAG211_800A6BBC, &D_WSTAG211_800A6BD0,
    &D_WSTAG211_800A6BE4, &D_WSTAG211_800A6BF8, &D_WSTAG211_800A6C0C, &D_WSTAG211_800A6C20, &D_WSTAG211_800A6C34,
    &D_WSTAG211_800A6C48, &D_WSTAG211_800A6C5C, NULL,
};
FieldstgSprite wstag211_sprites[73] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 4, 0, 243, 69, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 4, 0, 802, 278, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 4, 0, 457, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 339, 212, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 372, 196, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 403, 244, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 435, 228, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 386, 117, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 546, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 674, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 573, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 613, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 151, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 411, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 649, 200, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x47, 0xA, 0, 76, 320, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x47, 0xA, 0, 561, 589, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x47, 0xA, 0, 710, 468, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x52, 0xA, 0, 225, 445, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x52, 0xA, 0, 422, 462, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x52, 0xA, 0, 748, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 10, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 42, 121, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 74, 137, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 290, 69, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 322, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 354, 101, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 482, 165, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 514, 181, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 706, 213, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 738, 229, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 770, 245, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 7, 0x12, 0, 119, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 135, 232, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 155, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 175, 212, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 195, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 215, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 235, 182, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 255, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 275, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 295, 152, 0, 0 },
    { 1, 0x64, 0x40, 6, 4, 0, 0, 0, 0, 0, 335, 141, 0, 0 }, { 1, 0x65, 0x40, 6, 5, 0, 0, 0, 0, 0, 511, 229, 0, 0 },
    { 1, 0x66, 0x40, 6, 6, 0, 0, 0, 0, 0, 736, 326, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 114, 179, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 264, 130, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 429, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x5E, 1, 0x5E, 0x61, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 6, 0x5A, 1, 0x5A, 0x5D, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 144, 110, 160, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 359, 148, 193, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 535, 236, 281, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 759, 332, 376, 0 },
    { 1, 0, 0x50, 4, 7, 0, 0, 0, 0, 0, 441, 345, 450, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 464, 444, 496, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 595, 442, 478, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 384, 259, 279, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 401, 250, 271, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 368, 251, 270, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 417, 243, 264, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 352, 242, 262, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 433, 233, 255, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 336, 229, 255, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 449, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 320, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 337, 219, 240, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 353, 208, 231, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 369, 203, 223, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 385, 192, 215, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 401, 187, 207, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 499, 317, 334, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag211_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x270, 0x3E8, 0xEC, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x275, 0x70, 0xF0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x276, 0x58, 0x1CC, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x277, 0x69, 0x134, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x283, 0x1F8, 0x2BC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x286, 0x1D8, 0x1D4, 3, 0, 0, 0 },
    { 0x6025, 1, 0x40A4, 0, 8, 0x39D, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag211_funcs = { wstag211_setup };
FieldstgEventDef wstag211_events[2] = {
    { 925, D_WSTAG211_800A6050, 0x01120028, NULL, wstag211_event_925_end }, { -1, NULL, 0, NULL, NULL },
};
