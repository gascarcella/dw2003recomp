#include "wstag.h"

/* WSTAG660: stage 0x25D (fieldstg_stages). */

extern WstagFuncs wstag660_funcs;
extern CVECTOR wstag660_color;
extern FieldstgVramPlace wstag660_vram_places[];
extern FieldstgPlacedActor *wstag660_actors[];
extern FieldstgSprite wstag660_sprites[];
extern FieldstgMapEvent wstag660_map_events[];
extern FieldstgEventDef wstag660_events[];

void wstag660_update(WstagObject *obj) {
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

WstagObject *wstag660_start(void *arg0) {
    WstagObject *obj = object_new(wstag660_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag660_funcs.setup();
    return obj;
}

void wstag660_setup(void) {
    fieldstg_stage.background_file = 0x4C6;
    fieldstg_stage.sprite_file = 0x04C70000;
    fieldstg_stage.sprites = wstag660_sprites;
    fieldstg_stage.map_events = wstag660_map_events;
    fieldstg_stage.mask_file = 0x4C5;
    fieldstg_stage.talk_file = records_language + 0xCC;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1F800, 0x2E200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag660_vram_places;
    fieldstg_stage.music = 0x17;
    fieldstg_stage.sound = 0x605C0000;
    fieldstg_stage.actors = wstag660_actors;
    fieldstg_stage.color = wstag660_color;
    fieldstg_stage.events = wstag660_events;
    fieldstg_attr.set_file(0, 0x04C70001);
    fieldstg_attr.set_file(7, 0x04C70002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0xF && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

INCLUDE_RODATA("asm/wstag660/nonmatchings/wstag660", wstag660_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag660_setup(void);

s16 D_WSTAG660_800A5FE8[176] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 390, 800, 5),
    FIELDSTG_EVENT_PLACE(69, 295, 614),
    FIELDSTG_EVENT_ANIM(69, 1, 7),
    FIELDSTG_EVENT_PLACE(70, 480, 721),
    FIELDSTG_EVENT_ANIM(70, 1, 1),
    FIELDSTG_EVENT_PLACE(71, 513, 737),
    FIELDSTG_EVENT_ANIM(71, 1, 1),
    FIELDSTG_EVENT_PLACE(72, 545, 753),
    FIELDSTG_EVENT_ANIM(72, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 445, 800, 6),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 400, 642),
    FIELDSTG_EVENT_ANIM(0x323, 805, 69),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 69),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 69, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 69, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 514, 714),
    FIELDSTG_EVENT_ANIM(70, 1, 2),
    FIELDSTG_EVENT_ANIM(72, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 70, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 71, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 72, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(70, 1, 1),
    FIELDSTG_EVENT_ANIM(72, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x248, 496, 104, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG660_800A6148[58] = {
    FIELDSTG_EVENT_WALK(2, 1086, 516, 5),
    FIELDSTG_EVENT_PLACE(21, 1119, 502),
    FIELDSTG_EVENT_ANIM(21, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 21, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(21, 54, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 885, 2),
    FIELDSTG_EVENT_WAIT_ANIM(21),
    FIELDSTG_EVENT_ANIM(21, 55, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_GOTO_MAP(0xC06, 0, 0, 0),
    FIELDSTG_EVENT_END,
    0x6004, /* padding, not read */
};
FieldstgVramPlace wstag660_vram_places[22] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 448, 256, 484, 432, 656, 176, 336, 507 }, { 384, 256, 384, 463, 256, 207, 352, 507 },
    { 448, 256, 498, 296, 712, 40, 368, 507 }, { 448, 256, 482, 306, 648, 50, 336, 506 },
    { 448, 256, 494, 256, 696, 0, 352, 506 }, { 320, 256, 368, 462, 192, 206, 368, 506 },
    { 448, 256, 448, 312, 512, 56, 320, 505 }, { 448, 256, 492, 432, 688, 176, 336, 505 },
    { 448, 256, 470, 324, 600, 68, 352, 505 }, { 448, 256, 456, 333, 544, 77, 368, 505 },
    { 448, 256, 492, 336, 688, 80, 320, 504 }, { 448, 256, 500, 336, 720, 80, 336, 504 },
    { 448, 256, 478, 338, 632, 82, 352, 504 }, { 448, 256, 500, 432, 720, 176, 368, 504 },
    { 448, 256, 464, 434, 576, 178, 320, 503 }, { 448, 256, 472, 434, 608, 178, 336, 503 },
};
u16 D_WSTAG660_800A631C[4] = { 0x7A24, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6324[4] = { 0x9014, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A632C[4] = { 0x7A0C, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6334[4] = { 0x7A0B, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A633C[4] = { 0x7C00, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG660_800A6344[2] = { { NULL, D_WSTAG660_800A631C, 5 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A635C[2] = { { NULL, D_WSTAG660_800A6324, 2 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6374[2] = { { NULL, D_WSTAG660_800A632C, 243 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A638C[2] = { { NULL, D_WSTAG660_800A6334, 242 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A63A4[2] = { { NULL, D_WSTAG660_800A633C, 76 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A63BC[2] = { { NULL, NULL, 341 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A63D4[2] = { { NULL, NULL, 340 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A63EC[2] = { { NULL, NULL, 337 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6404[2] = { { NULL, NULL, 338 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A641C[2] = { { NULL, NULL, 327 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6434[2] = { { NULL, NULL, 331 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A644C[2] = { { NULL, NULL, 328 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6464[2] = { { NULL, NULL, 329 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A647C[2] = { { NULL, NULL, 332 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6494[2] = { { NULL, NULL, 336 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A64AC[2] = { { NULL, NULL, 333 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A64C4[2] = { { NULL, NULL, 334 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A64DC[2] = { { NULL, NULL, 325 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A64F4[2] = { { NULL, NULL, 322 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A650C[2] = { { NULL, NULL, 324 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6524[2] = { { NULL, NULL, 326 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A653C[2] = { { NULL, NULL, 323 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6554[2] = { { NULL, NULL, 330 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A656C[2] = { { NULL, NULL, 335 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG660_800A6584[2] = { { NULL, NULL, 339 }, { NULL, NULL, 0 } };
u16 D_WSTAG660_800A659C[4] = { 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65A4[4] = { 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65AC[4] = { 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65B4[4] = { 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65BC[4] = { 0x7008, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65C4[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65CC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65D4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65DC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65E4[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65EC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65F4[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A65FC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6604[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A660C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6614[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A661C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6624[4] = { 0x1C26, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A662C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6634[4] = { 0x1C26, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A663C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6644[4] = { 0x1C26, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A664C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6654[4] = { 0x1C26, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A665C[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6664[4] = { 0x600F, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A666C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A6674[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG660_800A667C[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG660_800A6684 = { D_WSTAG660_800A659C, D_WSTAG660_800A6344, 20, 4, 908, 596, 7 };
FieldstgPlacedActor D_WSTAG660_800A6698 = { D_WSTAG660_800A65A4, D_WSTAG660_800A635C, 21, 5, 1119, 502, 1 };
FieldstgPlacedActor D_WSTAG660_800A66AC = { D_WSTAG660_800A65AC, D_WSTAG660_800A6374, 22, 6, 834, 470, 7 };
FieldstgPlacedActor D_WSTAG660_800A66C0 = { D_WSTAG660_800A65B4, D_WSTAG660_800A638C, 23, 7, 690, 493, 1 };
FieldstgPlacedActor D_WSTAG660_800A66D4 = { D_WSTAG660_800A65BC, D_WSTAG660_800A63A4, 24, 8, 1329, 411, 1 };
FieldstgPlacedActor D_WSTAG660_800A66E8 = { D_WSTAG660_800A65C4, D_WSTAG660_800A63BC, 45, 9, 897, 289, 5 };
FieldstgPlacedActor D_WSTAG660_800A66FC = { D_WSTAG660_800A65CC, D_WSTAG660_800A63D4, 45, 9, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG660_800A6710 = { D_WSTAG660_800A65D4, D_WSTAG660_800A63EC, 45, 9, 897, 289, 5 };
FieldstgPlacedActor D_WSTAG660_800A6724 = { D_WSTAG660_800A65DC, D_WSTAG660_800A6404, 45, 9, 897, 289, 5 };
FieldstgPlacedActor D_WSTAG660_800A6738 = { D_WSTAG660_800A65E4, D_WSTAG660_800A641C, 54, 10, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG660_800A674C = { D_WSTAG660_800A65EC, D_WSTAG660_800A6434, 54, 10, 897, 289, 5 };
FieldstgPlacedActor D_WSTAG660_800A6760 = { D_WSTAG660_800A65F4, D_WSTAG660_800A644C, 54, 10, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG660_800A6774 = { D_WSTAG660_800A65FC, D_WSTAG660_800A6464, 54, 10, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG660_800A6788 = { D_WSTAG660_800A6604, D_WSTAG660_800A647C, 58, 11, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG660_800A679C = { D_WSTAG660_800A660C, D_WSTAG660_800A6494, 58, 11, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG660_800A67B0 = { D_WSTAG660_800A6614, D_WSTAG660_800A64AC, 58, 11, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG660_800A67C4 = { D_WSTAG660_800A661C, D_WSTAG660_800A64C4, 58, 11, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG660_800A67D8 = { D_WSTAG660_800A6624, NULL, 69, 12, 295, 614, 7 };
FieldstgPlacedActor D_WSTAG660_800A67EC = { D_WSTAG660_800A662C, D_WSTAG660_800A64DC, 69, 12, 834, 470, 7 };
FieldstgPlacedActor D_WSTAG660_800A6800 = { D_WSTAG660_800A6634, NULL, 70, 13, 480, 721, 1 };
FieldstgPlacedActor D_WSTAG660_800A6814 = { D_WSTAG660_800A663C, D_WSTAG660_800A64F4, 70, 13, 1123, 499, 1 };
FieldstgPlacedActor D_WSTAG660_800A6828 = { D_WSTAG660_800A6644, NULL, 71, 14, 513, 737, 1 };
FieldstgPlacedActor D_WSTAG660_800A683C = { D_WSTAG660_800A664C, D_WSTAG660_800A650C, 71, 14, 690, 493, 1 };
FieldstgPlacedActor D_WSTAG660_800A6850 = { D_WSTAG660_800A6654, NULL, 72, 15, 545, 753, 1 };
FieldstgPlacedActor D_WSTAG660_800A6864 = { D_WSTAG660_800A665C, D_WSTAG660_800A6524, 72, 15, 908, 596, 7 };
FieldstgPlacedActor D_WSTAG660_800A6878 = { D_WSTAG660_800A6664, D_WSTAG660_800A653C, 73, 16, 1329, 411, 1 };
FieldstgPlacedActor D_WSTAG660_800A688C = { D_WSTAG660_800A666C, D_WSTAG660_800A6554, 157, 17, 993, 531, 7 };
FieldstgPlacedActor D_WSTAG660_800A68A0 = { D_WSTAG660_800A6674, D_WSTAG660_800A656C, 158, 18, 767, 529, 3 };
FieldstgPlacedActor D_WSTAG660_800A68B4 = { D_WSTAG660_800A667C, D_WSTAG660_800A6584, 159, 19, 897, 289, 5 };
FieldstgPlacedActor *wstag660_actors[30] = {
    &D_WSTAG660_800A6684, &D_WSTAG660_800A6698, &D_WSTAG660_800A66AC, &D_WSTAG660_800A66C0, &D_WSTAG660_800A66D4,
    &D_WSTAG660_800A66E8, &D_WSTAG660_800A66FC, &D_WSTAG660_800A6710, &D_WSTAG660_800A6724, &D_WSTAG660_800A6738,
    &D_WSTAG660_800A674C, &D_WSTAG660_800A6760, &D_WSTAG660_800A6774, &D_WSTAG660_800A6788, &D_WSTAG660_800A679C,
    &D_WSTAG660_800A67B0, &D_WSTAG660_800A67C4, &D_WSTAG660_800A67D8, &D_WSTAG660_800A67EC, &D_WSTAG660_800A6800,
    &D_WSTAG660_800A6814, &D_WSTAG660_800A6828, &D_WSTAG660_800A683C, &D_WSTAG660_800A6850, &D_WSTAG660_800A6864,
    &D_WSTAG660_800A6878, &D_WSTAG660_800A688C, &D_WSTAG660_800A68A0, &D_WSTAG660_800A68B4, NULL,
};
FieldstgSprite wstag660_sprites[70] = {
    { 1, 0, 0xC8, 2, 0x5A, 1, 0x5A, 0x61, 0xA, 0, 1314, 270, 0, 0 },
    { 1, 0, 0x48, 2, 0, 0, 0, 0, 0, 0, 444, 419, 0, 0 }, { 1, 0, 0x58, 2, 0xD, 0, 0, 0, 0, 0, 683, 345, 0, 0 },
    { 1, 0, 0x4C, 2, 0xE, 0, 0, 0, 0, 0, 947, 492, 0, 0 }, { 1, 0, 0x60, 2, 0x10, 0, 0, 0, 0, 0, 640, 515, 0, 0 },
    { 1, 0, 0x74, 2, 0x11, 0, 0, 0, 0, 0, 624, 524, 0, 0 }, { 1, 0, 0x44, 2, 0x12, 0, 0, 0, 0, 0, 304, 556, 0, 0 },
    { 1, 0, 0x40, 2, 7, 1, 7, 0xC, 4, 0, 379, 317, 0, 0 }, { 1, 0, 0x40, 2, 7, 1, 7, 0xC, 4, 0, 924, 587, 0, 0 },
    { 1, 0, 0x41, 2, 3, 0, 0, 0, 0, 0, 960, 418, 0, 0 }, { 1, 0, 0x62, 2, 0xF, 0, 0, 0, 0, 0, 938, 609, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 2, 0, 5, 8, 0, 1378, 348, 0, 0 },
    { 1, 0, 0x40, 6, 0x33, 2, 0, 5, 8, 0, 1391, 358, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 8, 0, 1250, 357, 0, 0 },
    { 1, 0, 0x40, 6, 0x34, 2, 0, 5, 8, 0, 1263, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x38, 4, 0, 1217, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x38, 4, 0, 1417, 367, 0, 0 },
    { 1, 0, 0x40, 6, 0x39, 2, 0, 3, 6, 0, 1310, 259, 0, 0 },
    { 1, 0, 0xC8, 6, 0x57, 1, 0x57, 0x59, 6, 0, 1144, 428, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x3E, 0xA, 0, 939, 765, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x3E, 0xA, 0, 1005, 684, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x3E, 0xA, 0, 1163, 605, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x3E, 0xA, 0, 1324, 525, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x41, 0xA, 0, 971, 701, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x41, 0xA, 0, 1013, 599, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x41, 0xA, 0, 1160, 527, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 337, 398, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 411, 485, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 489, 399, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 701, 294, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 815, 743, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 995, 695, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 1025, 674, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 1079, 362, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 1139, 536, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 1182, 595, 0, 0 },
    { 1, 0, 0x40, 6, 0x42, 1, 0x42, 0x44, 0xA, 0, 1306, 536, 0, 0 },
    { 1, 0, 0x40, 6, 0x45, 1, 0x45, 0x47, 0xA, 0, 551, 531, 0, 0 },
    { 1, 0, 0x40, 6, 0x45, 1, 0x45, 0x47, 0xA, 0, 733, 621, 0, 0 },
    { 1, 0, 0x40, 6, 0x45, 1, 0x45, 0x47, 0xA, 0, 873, 609, 0, 0 },
    { 1, 0, 0x40, 6, 0x45, 1, 0x45, 0x47, 0xA, 0, 886, 745, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x4A, 0xA, 0, 345, 477, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x4A, 0xA, 0, 575, 542, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x4A, 0xA, 0, 814, 580, 0, 0 },
    { 1, 0, 0x40, 6, 0x48, 1, 0x48, 0x4A, 0xA, 0, 896, 701, 0, 0 },
    { 1, 0, 0x40, 6, 0x4B, 1, 0x4B, 0x4D, 0xA, 0, 869, 689, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 273, 423, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 289, 417, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 297, 409, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 310, 410, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 510, 386, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 531, 377, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 719, 281, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 804, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 817, 561, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 922, 500, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 964, 753, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 977, 754, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 1101, 351, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 1171, 513, 0, 0 },
    { 1, 0, 0x40, 6, 0x4E, 1, 0x4E, 0x50, 0xA, 0, 1174, 506, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0x51, 1, 0x51, 0x53, 6, 0, 799, 318, 0, 0 },
    { 1, 0, 0xC8, 0xA, 0x54, 1, 0x54, 0x56, 6, 0, 1052, 371, 0, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 944, 515, 569, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 582, 424, 454, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 1056, 269, 327, 0 }, { 1, 0, 0x41, 4, 4, 0, 0, 0, 0, 0, 369, 703, 751, 0 },
    { 1, 0, 0x41, 4, 4, 0, 0, 0, 0, 0, 449, 743, 791, 0 }, { 1, 0, 0x60, 4, 2, 0, 0, 0, 0, 0, 914, 233, 303, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag660_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x25E, 0x100, 0x220, 5, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x248, 0x1F0, 0x68, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 4, 0x170, 0x268, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 4, 0x180, 0x2B0, 0, 0, 0, 0 }, { 0x1C26, 1, 0xFFFF, 0, 8, 0x19F, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag660_funcs = { wstag660_setup };
FieldstgEventDef wstag660_events[3] = {
    { 1231, D_WSTAG660_800A6148, 0x01430007, NULL, NULL }, { 415, D_WSTAG660_800A5FE8, 0x0143000E, NULL, NULL },
    { -1, NULL, 0, NULL, NULL },
};
