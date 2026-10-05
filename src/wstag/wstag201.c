#include "wstag.h"

/* WSTAG201: stage 0x270 (fieldstg_stages). */

extern WstagFuncs wstag201_funcs;
extern CVECTOR wstag201_color;
extern FieldstgBattleLists wstag201_battle_lists;
extern FieldstgVramPlace wstag201_vram_places[];
extern FieldstgPlacedActor *wstag201_actors[];
extern FieldstgSprite wstag201_sprites[];
extern FieldstgMapEvent wstag201_map_events[];

void wstag201_update(WstagObject *obj) {
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

WstagObject *wstag201_start(void *arg0) {
    WstagObject *obj = object_new(wstag201_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag201_funcs.setup();
    return obj;
}

void wstag201_setup(void) {
    fieldstg_stage.background_file = 0x521;
    fieldstg_stage.sprite_file = 0x05220000;
    fieldstg_stage.sprites = wstag201_sprites;
    fieldstg_stage.map_events = wstag201_map_events;
    fieldstg_stage.mask_file = 0x523;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x16B00, 0x28500 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag201_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag201_actors;
    fieldstg_stage.color = wstag201_color;
    fieldstg_stage.battle_lists = &wstag201_battle_lists;
    fieldstg_attr.set_file(0, 0x05220001);
    fieldstg_attr.set_file(7, 0x05220002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag201/nonmatchings/wstag201", wstag201_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag201_setup(void);

FieldstgListedBattle D_WSTAG201_800A5FE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A5FF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A5FFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6008 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6014 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6020 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A602C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6038 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG201_800A6044 = {
    0,
    { &D_WSTAG201_800A5FE4, &D_WSTAG201_800A5FF0, &D_WSTAG201_800A5FFC, &D_WSTAG201_800A6008, &D_WSTAG201_800A6014,
        &D_WSTAG201_800A6020, &D_WSTAG201_800A602C, &D_WSTAG201_800A6038 },
};
FieldstgListedBattle D_WSTAG201_800A6068 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6074 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6080 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A608C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6098 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A60A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A60B0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A60BC = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG201_800A60C8 = {
    0,
    { &D_WSTAG201_800A6068, &D_WSTAG201_800A6074, &D_WSTAG201_800A6080, &D_WSTAG201_800A608C, &D_WSTAG201_800A6098,
        &D_WSTAG201_800A60A4, &D_WSTAG201_800A60B0, &D_WSTAG201_800A60BC },
};
FieldstgListedBattle D_WSTAG201_800A60EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A60F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6104 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6110 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A611C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6128 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6134 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6140 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG201_800A614C = {
    0,
    { &D_WSTAG201_800A60EC, &D_WSTAG201_800A60F8, &D_WSTAG201_800A6104, &D_WSTAG201_800A6110, &D_WSTAG201_800A611C,
        &D_WSTAG201_800A6128, &D_WSTAG201_800A6134, &D_WSTAG201_800A6140 },
};
FieldstgListedBattle D_WSTAG201_800A6170 = { 195, 18, 0x60080000 };
FieldstgListedBattle D_WSTAG201_800A617C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6188 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A6194 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A61A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A61AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A61B8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG201_800A61C4 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG201_800A61D0 = {
    0,
    { &D_WSTAG201_800A6170, &D_WSTAG201_800A617C, &D_WSTAG201_800A6188, &D_WSTAG201_800A6194, &D_WSTAG201_800A61A0,
        &D_WSTAG201_800A61AC, &D_WSTAG201_800A61B8, &D_WSTAG201_800A61C4 },
};
FieldstgBattleLists wstag201_battle_lists = {
    142, 0, 0, { &D_WSTAG201_800A6044, &D_WSTAG201_800A60C8, &D_WSTAG201_800A614C }, &D_WSTAG201_800A61D0,
};
FieldstgVramPlace wstag201_vram_places[32] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 418, 346, 392, 90, 368, 509 }, { 384, 256, 398, 389, 312, 133, 368, 508 },
    { 384, 256, 406, 390, 344, 134, 320, 507 }, { 448, 256, 504, 289, 736, 33, 336, 507 },
    { 448, 256, 478, 304, 632, 48, 352, 507 }, { 448, 256, 448, 256, 512, 0, 368, 507 },
    { 448, 256, 456, 256, 544, 0, 320, 506 }, { 448, 256, 464, 256, 576, 0, 336, 506 },
    { 448, 256, 458, 320, 552, 64, 352, 506 }, { 448, 256, 466, 320, 584, 64, 368, 506 },
    { 448, 256, 472, 256, 608, 0, 320, 505 }, { 448, 256, 480, 256, 640, 0, 336, 505 },
    { 448, 256, 486, 256, 664, 0, 352, 505 }, { 384, 256, 426, 392, 424, 136, 368, 505 },
    { 448, 256, 486, 328, 664, 72, 320, 504 }, { 448, 256, 474, 336, 616, 80, 336, 504 },
    { 448, 256, 494, 340, 696, 84, 352, 504 }, { 448, 256, 502, 340, 728, 84, 368, 504 },
    { 448, 256, 448, 344, 512, 88, 320, 503 }, { 448, 256, 456, 352, 544, 96, 336, 503 },
    { 448, 256, 464, 352, 576, 96, 352, 503 }, { 448, 256, 482, 360, 648, 104, 368, 503 },
    { 448, 256, 472, 368, 608, 112, 320, 502 }, { 448, 256, 498, 372, 712, 116, 336, 502 },
    { 448, 256, 448, 376, 512, 120, 352, 502 }, { 448, 256, 456, 384, 544, 128, 368, 502 },
};
u16 D_WSTAG201_800A6410[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6418[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6420[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6428[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6430[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6438[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6440[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6448[8] = { 0x6026, 1, 0, 0, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A6458[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6460[8] = { 0x6026, 1, 0, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A6470[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A647C[8] = { 0x6026, 1, 1, 0, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A648C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6494[8] = { 0x6026, 1, 1, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A64A4[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A64B0[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A64B8[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG201_800A64C0[3] = {
    { D_WSTAG201_800A6410, NULL, 181 }, { D_WSTAG201_800A6418, NULL, 182 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A64E4[3] = {
    { D_WSTAG201_800A6420, NULL, 184 }, { D_WSTAG201_800A6428, NULL, 185 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A6508[3] = {
    { D_WSTAG201_800A6430, NULL, 187 }, { D_WSTAG201_800A6438, NULL, 188 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A652C[2] = { { NULL, NULL, 196 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6544[2] = { { NULL, NULL, 194 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A655C[2] = { { NULL, NULL, 202 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6574[2] = { { NULL, NULL, 200 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A658C[2] = { { NULL, NULL, 198 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A65A4[2] = { { NULL, NULL, 210 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A65BC[2] = { { NULL, NULL, 211 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A65D4[2] = { { NULL, NULL, 205 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A65EC[2] = { { NULL, NULL, 208 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6604[5] = {
    { D_WSTAG201_800A6440, NULL, 438 }, { D_WSTAG201_800A6448, D_WSTAG201_800A6458, 508 },
    { D_WSTAG201_800A6460, NULL, 509 }, { D_WSTAG201_800A6470, NULL, 522 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A6640[2] = { { NULL, NULL, 162 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6658[4] = {
    { D_WSTAG201_800A647C, D_WSTAG201_800A648C, 510 }, { D_WSTAG201_800A6494, NULL, 511 },
    { D_WSTAG201_800A64A4, NULL, 523 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A6688[3] = {
    { D_WSTAG201_800A64B0, NULL, 190 }, { D_WSTAG201_800A64B8, NULL, 191 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG201_800A66AC[2] = { { NULL, NULL, 204 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A66C4[2] = { { NULL, NULL, 206 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A66DC[2] = { { NULL, NULL, 207 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A66F4[2] = { { NULL, NULL, 209 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A670C[2] = { { NULL, NULL, 199 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6724[2] = { { NULL, NULL, 197 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A673C[2] = { { NULL, NULL, 203 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6754[2] = { { NULL, NULL, 201 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A676C[2] = { { NULL, NULL, 195 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6784[2] = { { NULL, NULL, 193 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A679C[2] = { { NULL, NULL, 183 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A67B4[2] = { { NULL, NULL, 186 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A67CC[2] = { { NULL, NULL, 189 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A67E4[2] = { { NULL, NULL, 192 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A67FC[2] = { { NULL, NULL, 212 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A6814[2] = { { NULL, NULL, 213 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG201_800A682C[2] = { { NULL, NULL, 214 }, { NULL, NULL, 0 } };
u16 D_WSTAG201_800A6844[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A684C[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6854[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A685C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6864[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6870[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A687C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6884[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6890[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6898[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68A0[6] = { 0x1A0A, 1, 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68AC[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68B8[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68C0[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68C8[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68D0[4] = { 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68D8[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A68E4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A68EC[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A68F8[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6900[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6908[6] = { 0x1A0A, 0, 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6914[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A691C[6] = { 0x701C, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG201_800A6928[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6930[6] = { 0x1A0A, 0, 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A693C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6944[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A694C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6954[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A695C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A6964[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG201_800A696C[4] = { 0x602B, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG201_800A6974 = { D_WSTAG201_800A6844, D_WSTAG201_800A64C0, 37, 4, 1088, 288, 3 };
FieldstgPlacedActor D_WSTAG201_800A6988 = { D_WSTAG201_800A684C, D_WSTAG201_800A64E4, 38, 5, 736, 392, 1 };
FieldstgPlacedActor D_WSTAG201_800A699C = { D_WSTAG201_800A6854, D_WSTAG201_800A6508, 39, 6, 555, 474, 1 };
FieldstgPlacedActor D_WSTAG201_800A69B0 = { D_WSTAG201_800A685C, D_WSTAG201_800A652C, 45, 7, 305, 616, 7 };
FieldstgPlacedActor D_WSTAG201_800A69C4 = { D_WSTAG201_800A6864, D_WSTAG201_800A6544, 45, 7, 268, 586, 7 };
FieldstgPlacedActor D_WSTAG201_800A69D8 = { D_WSTAG201_800A6870, D_WSTAG201_800A655C, 46, 8, 784, 264, 3 };
FieldstgPlacedActor D_WSTAG201_800A69EC = { D_WSTAG201_800A687C, D_WSTAG201_800A6574, 48, 9, 944, 584, 1 };
FieldstgPlacedActor D_WSTAG201_800A6A00 = { D_WSTAG201_800A6884, D_WSTAG201_800A658C, 48, 9, 944, 584, 1 };
FieldstgPlacedActor D_WSTAG201_800A6A14 = { D_WSTAG201_800A6890, D_WSTAG201_800A65A4, 50, 10, 563, 559, 5 };
FieldstgPlacedActor D_WSTAG201_800A6A28 = { D_WSTAG201_800A6898, D_WSTAG201_800A65BC, 56, 11, 905, 253, 5 };
FieldstgPlacedActor D_WSTAG201_800A6A3C = { D_WSTAG201_800A68A0, D_WSTAG201_800A65D4, 57, 12, 564, 558, 5 };
FieldstgPlacedActor D_WSTAG201_800A6A50 = { D_WSTAG201_800A68AC, D_WSTAG201_800A65EC, 58, 13, 595, 542, 1 };
FieldstgPlacedActor D_WSTAG201_800A6A64 = { D_WSTAG201_800A68B8, D_WSTAG201_800A6604, 101, 14, 257, 704, 1 };
FieldstgPlacedActor D_WSTAG201_800A6A78 = { D_WSTAG201_800A68C0, D_WSTAG201_800A6640, 102, 15, 905, 253, 5 };
FieldstgPlacedActor D_WSTAG201_800A6A8C = { D_WSTAG201_800A68C8, D_WSTAG201_800A6658, 103, 16, 225, 720, 5 };
FieldstgPlacedActor D_WSTAG201_800A6AA0 = { D_WSTAG201_800A68D0, D_WSTAG201_800A6688, 111, 17, 464, 640, 5 };
FieldstgPlacedActor D_WSTAG201_800A6AB4 = { D_WSTAG201_800A68D8, D_WSTAG201_800A66AC, 157, 18, 564, 558, 5 };
FieldstgPlacedActor D_WSTAG201_800A6AC8 = { D_WSTAG201_800A68E4, D_WSTAG201_800A66C4, 157, 18, 564, 558, 5 };
FieldstgPlacedActor D_WSTAG201_800A6ADC = { D_WSTAG201_800A68EC, D_WSTAG201_800A66DC, 158, 19, 595, 542, 1 };
FieldstgPlacedActor D_WSTAG201_800A6AF0 = { D_WSTAG201_800A68F8, D_WSTAG201_800A66F4, 158, 19, 595, 542, 1 };
FieldstgPlacedActor D_WSTAG201_800A6B04 = { D_WSTAG201_800A6900, D_WSTAG201_800A670C, 159, 20, 944, 584, 1 };
FieldstgPlacedActor D_WSTAG201_800A6B18 = { D_WSTAG201_800A6908, D_WSTAG201_800A6724, 159, 20, 944, 584, 1 };
FieldstgPlacedActor D_WSTAG201_800A6B2C = { D_WSTAG201_800A6914, D_WSTAG201_800A673C, 160, 21, 784, 264, 3 };
FieldstgPlacedActor D_WSTAG201_800A6B40 = { D_WSTAG201_800A691C, D_WSTAG201_800A6754, 160, 21, 784, 264, 3 };
FieldstgPlacedActor D_WSTAG201_800A6B54 = { D_WSTAG201_800A6928, D_WSTAG201_800A676C, 161, 22, 268, 586, 7 };
FieldstgPlacedActor D_WSTAG201_800A6B68 = { D_WSTAG201_800A6930, D_WSTAG201_800A6784, 161, 22, 268, 586, 7 };
FieldstgPlacedActor D_WSTAG201_800A6B7C = { D_WSTAG201_800A693C, D_WSTAG201_800A679C, 162, 23, 1088, 288, 3 };
FieldstgPlacedActor D_WSTAG201_800A6B90 = { D_WSTAG201_800A6944, D_WSTAG201_800A67B4, 174, 24, 736, 392, 1 };
FieldstgPlacedActor D_WSTAG201_800A6BA4 = { D_WSTAG201_800A694C, D_WSTAG201_800A67CC, 175, 25, 555, 474, 1 };
FieldstgPlacedActor D_WSTAG201_800A6BB8 = { D_WSTAG201_800A6954, D_WSTAG201_800A67E4, 176, 26, 464, 640, 1 };
FieldstgPlacedActor D_WSTAG201_800A6BCC = { D_WSTAG201_800A695C, D_WSTAG201_800A67FC, 375, 27, 736, 393, 1 };
FieldstgPlacedActor D_WSTAG201_800A6BE0 = { D_WSTAG201_800A6964, D_WSTAG201_800A6814, 377, 28, 464, 641, 5 };
FieldstgPlacedActor D_WSTAG201_800A6BF4 = { D_WSTAG201_800A696C, D_WSTAG201_800A682C, 379, 29, 596, 543, 1 };
FieldstgPlacedActor *wstag201_actors[34] = {
    &D_WSTAG201_800A6974, &D_WSTAG201_800A6988, &D_WSTAG201_800A699C, &D_WSTAG201_800A69B0, &D_WSTAG201_800A69C4,
    &D_WSTAG201_800A69D8, &D_WSTAG201_800A69EC, &D_WSTAG201_800A6A00, &D_WSTAG201_800A6A14, &D_WSTAG201_800A6A28,
    &D_WSTAG201_800A6A3C, &D_WSTAG201_800A6A50, &D_WSTAG201_800A6A64, &D_WSTAG201_800A6A78, &D_WSTAG201_800A6A8C,
    &D_WSTAG201_800A6AA0, &D_WSTAG201_800A6AB4, &D_WSTAG201_800A6AC8, &D_WSTAG201_800A6ADC, &D_WSTAG201_800A6AF0,
    &D_WSTAG201_800A6B04, &D_WSTAG201_800A6B18, &D_WSTAG201_800A6B2C, &D_WSTAG201_800A6B40, &D_WSTAG201_800A6B54,
    &D_WSTAG201_800A6B68, &D_WSTAG201_800A6B7C, &D_WSTAG201_800A6B90, &D_WSTAG201_800A6BA4, &D_WSTAG201_800A6BB8,
    &D_WSTAG201_800A6BCC, &D_WSTAG201_800A6BE0, &D_WSTAG201_800A6BF4, NULL,
};
FieldstgSprite wstag201_sprites[49] = {
    { 1, 0, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 120, 520, 0, 0 }, { 1, 0, 0x78, 2, 0x19, 0, 0, 0, 0, 0, 130, 528, 0, 0 },
    { 1, 0, 0x40, 2, 0x47, 2, 0, 3, 6, 0, 1048, 211, 0, 0 }, { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 6, 0, 437, 437, 0, 0 },
    { 1, 0, 0x40, 2, 0x48, 2, 0, 3, 6, 0, 533, 389, 0, 0 }, { 1, 0, 0x40, 2, 0x49, 2, 0, 3, 6, 0, 1231, 344, 0, 0 },
    { 1, 0, 0x40, 2, 0x4A, 2, 0, 3, 6, 0, 623, 331, 0, 0 }, { 1, 0, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 352, 629, 0, 0 },
    { 1, 0, 0x40, 2, 0x10, 0, 0, 0, 0, 0, 268, 384, 0, 0 }, { 1, 0, 0x48, 2, 0x11, 0, 0, 0, 0, 0, 274, 440, 0, 0 },
    { 1, 0, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 1044, 416, 0, 0 },
    { 1, 0, 0x40, 2, 0x13, 0, 0, 0, 0, 0, 1088, 384, 0, 0 }, { 1, 0, 0x40, 6, 0x47, 2, 0, 3, 6, 0, 430, 722, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 3, 6, 0, 501, 687, 0, 0 }, { 1, 0, 0x40, 6, 0x47, 2, 0, 3, 6, 0, 945, 160, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 3, 6, 0, 1096, 700, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 3, 6, 0, 1142, 723, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 483, 414, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 597, 695, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 642, 672, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 2, 0, 3, 6, 0, 754, 617, 0, 0 }, { 1, 0, 0x40, 6, 0x49, 2, 0, 3, 6, 0, 1023, 463, 0, 0 },
    { 1, 0, 0x40, 6, 0x4A, 2, 0, 3, 6, 0, 669, 308, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 145, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 290, 767, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 845, 621, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 1, 0x2C, 0x3B, 0xA, 0, 903, 713, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 40, 693, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 191, 816, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 441, 795, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1030, 595, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x46, 0xA, 0, 1148, 406, 0, 0 },
    { 1, 0x64, 0x40, 6, 0x14, 0, 0, 0, 0, 0, 277, 489, 0, 0 },
    { 1, 0x65, 0x40, 6, 0x15, 0, 0, 0, 0, 0, 446, 416, 0, 0 },
    { 1, 0x66, 0x40, 6, 0x16, 0, 0, 0, 0, 0, 542, 367, 0, 0 },
    { 1, 0x67, 0x40, 6, 0x17, 0, 0, 0, 0, 0, 638, 321, 0, 0 },
    { 1, 0x68, 0x40, 6, 0x18, 0, 0, 0, 0, 0, 1045, 458, 0, 0 }, { 1, 0, 0x68, 6, 6, 0, 0, 0, 0, 0, 879, 428, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4B, 1, 0x4B, 0x4E, 6, 0, 241, 396, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4F, 1, 0x4F, 0x52, 6, 0, 226, 450, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x53, 1, 0x53, 0x56, 6, 0, 1091, 316, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x57, 1, 0x57, 0x5A, 6, 0, 1103, 338, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 303, 495, 544, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 409, 416, 472, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 510, 375, 423, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 609, 327, 375, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 1051, 192, 246, 0 }, { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 1073, 463, 512, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag201_map_events[8] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x272, 0x308, 0xE0, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27E, 0xE0, 0x14E, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x178, 0x19C, 3, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x27C, 0x218, 0x14C, 3, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x279, 0x116, 0xDA, 3, 0x67, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x60, 0x190, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x275, 0xD8, 0x178, 5, 0x68, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag201_funcs = { wstag201_setup };
