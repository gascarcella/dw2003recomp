#include "wstag.h"

/* WSTAG205: stage 0x202 (fieldstg_stages). */

/* A key of the animations of wstag205_sprites_update: frame (0: back to the first key) and how long it is shown. */
typedef struct Wstag205Key {
    /* 0x0 */ u8 frame;
    /* 0x1 */ u8 time;
} Wstag205Key; /* size 0x2 */

/* A running animation of wstag205_sprites_update. */
typedef struct Wstag205Anim {
    /* 0x0 */ s32 frame;
    /* 0x4 */ s32 key;
    /* 0x8 */ s32 time;
} Wstag205Anim; /* size 0xC */

/* An object that draws the stage's animated sprites (D_WSTAG205_800A6684) near the view. */
typedef struct Wstag205Sprites {
    /* 0x00 */ Object base;
    /* 0x50 */ Wstag205Anim anims[10];
} Wstag205Sprites; /* size 0xC8 */

/* A sprite drawn by it, 0-terminated (x 0). */
typedef struct Wstag205Sprite {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ u8 anim; /* index of Wstag205Sprites.anims */
    /* 0x5 */ u8 flip;
} Wstag205Sprite; /* size 0x6 */

extern Wstag205Key *D_WSTAG205_800A6660[];
extern Wstag205Sprite D_WSTAG205_800A6684[];
extern WstagFuncs wstag205_funcs;
extern CVECTOR wstag205_color;
extern FieldstgBattleLists wstag205_battle_lists;
extern FieldstgVramPlace wstag205_vram_places[];
extern FieldstgPlacedActor *wstag205_actors[];
extern FieldstgSprite wstag205_sprites[];
extern FieldstgMapEvent wstag205_map_events[];
extern FieldstgEventDef wstag205_events[];
void wstag205_update();
void wstag205_sprites_update(Wstag205Sprites *obj);

void wstag205_sprites_update(Wstag205Sprites *obj) {
    Sprite spr;
    GfxRect view;
    GfxLayer *layer;
    s32 i;
    s32 x;
    s32 y;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 0xA);
        spr.set_vram_pos(0x140, 0x100);
        spr.set_clut8_pos(0, 0x1F0);
        for (i = 0; i < 9; i++) {
            obj->anims[i].time += gfx_module.funcs.get_frame_ticks();
            while (1) {
                if (D_WSTAG205_800A6660[i][obj->anims[i].key].frame == 0) {
                    obj->anims[i].key = 0;
                }
                if (D_WSTAG205_800A6660[i][obj->anims[i].key].time >= obj->anims[i].time) {
                    break;
                }
                obj->anims[i].time -= D_WSTAG205_800A6660[i][obj->anims[i].key].time;
                obj->anims[i].key++;
            }
            obj->anims[i].frame = D_WSTAG205_800A6660[i][obj->anims[i].key].frame;
        }
        layer = gfx_module.funcs.get_layer(0x1002);
        layer->get_view_rect(layer, &view);
        view.w += view.x;
        view.h += view.y;
        i = 0;
        while (1) {
            if (D_WSTAG205_800A6684[i].x == 0) {
                break;
            }
            x = D_WSTAG205_800A6684[i].x;
            y = D_WSTAG205_800A6684[i].y;
            if (x >= view.x - 0x40 && view.w + 0x40 >= x && y >= view.y - 0x40 && view.h + 0x40 >= y) {
                if (D_WSTAG205_800A6684[i].flip == 0) {
                    spr.set_scale(0x1000, 0x1000, 0x1000);
                } else {
                    spr.set_pivot(x, y);
                    spr.set_scale(-0x1000, 0x1000, 0x1000);
                }
                spr.draw(cdload_module.get_subfile_by_id(0x01990000), obj->anims[D_WSTAG205_800A6684[i].anim].frame, x, y);
            }
            i++;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag205_sprites_new(void) {
    return object_new(wstag205_sprites_update, 0xC8, 0);
}

void wstag205_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 6 && gamestate_flags.get_flag(0x4006, 1) && gamestate_flags.get_flag(0x4016, 0)) {
            data->event = fieldstg_event_start(0x65);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag205_start(void *arg0) {
    WstagObject *obj = object_new(wstag205_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag205_funcs.setup();
    return obj;
}

void wstag205_event_100_end(void) {
    gamestate_flags.set_flag(0x4006, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag205_event_101_end(void) {
    gamestate_flags.set_flag(0x4016, 1);
}

void wstag205_setup(void) {
    fieldstg_stage.background_file = 0x198;
    fieldstg_stage.sprite_file = 0x01990000;
    fieldstg_stage.sprites = wstag205_sprites;
    fieldstg_stage.map_events = wstag205_map_events;
    fieldstg_stage.mask_file = 0x2C3;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2DC00, 0xF200 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag205_vram_places;
    fieldstg_stage.music = 4;
    fieldstg_stage.sound = 0x60100000;
    fieldstg_stage.actors = wstag205_actors;
    fieldstg_stage.color = wstag205_color;
    fieldstg_stage.events = wstag205_events;
    fieldstg_stage.battle_lists = &wstag205_battle_lists;
    fieldstg_attr.set_file(0, 0x01990002);
    fieldstg_attr.set_file(7, 0x01990001);
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

INCLUDE_RODATA("asm/wstag205/nonmatchings/wstag205", wstag205_color);

/* The stage's .data (tools/wstag_data.py). */
void wstag205_setup(void);

s16 D_WSTAG205_800A63D0[171] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 655, 287, 5),
    FIELDSTG_EVENT_PLACE(102, 791, 220),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 719, 257, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 102),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 102),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 102),
    FIELDSTG_EVENT_DIALOG(0, 1, 102, 2),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(102, 751, 240, 1),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG205_800A6528[112] = {
    FIELDSTG_EVENT_PLACE(2, 719, 257),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_PLACE(102, 751, 240),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 102, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(102, 719, 240, 2),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WALK(102, 703, 248, 1),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WALK(102, 558, 319, 1),
    FIELDSTG_EVENT_WAIT_WALK(102),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_PLACE(102, 0, 0),
    FIELDSTG_EVENT_ANIM(102, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
Wstag205Key D_WSTAG205_800A6608[4] = { { 0x32, 0xA }, { 0x33, 0xA }, { 0x34, 0xA }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6610[4] = { { 0x35, 0xA }, { 0x36, 0xA }, { 0x37, 0xA }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6618[4] = { { 0x38, 0xA }, { 0x39, 0xA }, { 0x3A, 0xA }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6620[6] = { { 0x48, 6 }, { 0x49, 6 }, { 0x4A, 6 }, { 0x4B, 6 }, { 0, 0 }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A662C[4] = { { 0x3E, 6 }, { 0x3F, 6 }, { 0x40, 6 }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6634[6] = { { 0x50, 4 }, { 0x51, 4 }, { 0x52, 4 }, { 0x53, 4 }, { 0, 0 }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6640[6] = { { 0x4C, 6 }, { 0x4D, 6 }, { 0x4E, 6 }, { 0x4F, 6 }, { 0, 0 }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A664C[4] = { { 0x41, 6 }, { 0x42, 6 }, { 0x43, 6 }, { 0, 0 } };
Wstag205Key D_WSTAG205_800A6654[6] = { { 0x54, 4 }, { 0x55, 4 }, { 0x56, 4 }, { 0x57, 4 }, { 0, 0 }, { 0, 0 } };
Wstag205Key *D_WSTAG205_800A6660[9] = {
    D_WSTAG205_800A6608, D_WSTAG205_800A6610, D_WSTAG205_800A6618, D_WSTAG205_800A6620, D_WSTAG205_800A662C,
    D_WSTAG205_800A6634, D_WSTAG205_800A6640, D_WSTAG205_800A664C, D_WSTAG205_800A6654,
};
Wstag205Sprite D_WSTAG205_800A6684[137] = {
    { 40, 421, 0, 0 }, { 56, 371, 0, 0 }, { 143, 262, 0, 0 }, { 294, 343, 0, 0 }, { 381, 98, 0, 0 },
    { 636, 426, 0, 0 }, { 671, 411, 0, 0 }, { 743, 394, 0, 0 }, { 878, 424, 0, 0 }, { 1218, 291, 0, 0 },
    { 1293, 272, 0, 0 }, { 1194, 303, 0, 0 }, { 1216, 358, 0, 0 }, { 1132, 375, 0, 0 }, { 1278, 350, 0, 0 },
    { 1292, 355, 0, 0 }, { 1203, 489, 0, 0 }, { 1109, 501, 0, 0 }, { 1122, 458, 0, 0 }, { 977, 455, 0, 0 },
    { 839, 399, 0, 0 }, { 799, 401, 0, 0 }, { 555, 229, 0, 0 }, { 488, 261, 0, 0 }, { 597, 427, 0, 0 },
    { 527, 463, 0, 0 }, { 430, 298, 0, 0 }, { 375, 325, 0, 0 }, { 1331, 398, 0, 0 }, { 1253, 421, 0, 0 },
    { 8, 381, 1, 0 }, { 31, 379, 1, 0 }, { 64, 414, 1, 0 }, { 87, 412, 1, 0 }, { 125, 263, 1, 0 },
    { 276, 344, 1, 0 }, { 288, 353, 1, 0 }, { 425, 62, 1, 0 }, { 457, 292, 1, 0 }, { 617, 425, 1, 0 },
    { 631, 141, 1, 0 }, { 695, 404, 1, 0 }, { 718, 402, 1, 0 }, { 860, 425, 1, 0 }, { 868, 466, 1, 0 },
    { 995, 192, 1, 0 }, { 1239, 282, 1, 0 }, { 1166, 317, 1, 0 }, { 1264, 278, 1, 0 }, { 1141, 331, 1, 0 },
    { 1189, 369, 1, 0 }, { 1166, 368, 1, 0 }, { 1327, 342, 1, 0 }, { 1302, 341, 1, 0 }, { 1151, 487, 1, 0 },
    { 1136, 484, 1, 0 }, { 1003, 444, 1, 0 }, { 972, 469, 1, 0 }, { 789, 390, 1, 0 }, { 815, 402, 1, 0 },
    { 522, 252, 1, 0 }, { 505, 261, 1, 0 }, { 546, 464, 1, 0 }, { 551, 453, 1, 0 }, { 406, 303, 1, 0 },
    { 384, 318, 1, 0 }, { 1306, 414, 1, 0 }, { 1283, 408, 1, 0 }, { 34, 367, 2, 0 }, { 50, 413, 2, 0 },
    { 90, 400, 2, 0 }, { 137, 272, 2, 0 }, { 377, 107, 2, 0 }, { 410, 59, 2, 0 }, { 444, 290, 2, 0 },
    { 560, 452, 2, 0 }, { 618, 139, 2, 0 }, { 681, 403, 2, 0 }, { 721, 390, 2, 0 }, { 855, 464, 2, 0 },
    { 872, 434, 2, 0 }, { 982, 190, 2, 0 }, { 1271, 285, 2, 0 }, { 1115, 337, 2, 0 }, { 1199, 354, 2, 0 },
    { 1144, 361, 2, 0 }, { 1320, 325, 2, 0 }, { 1313, 356, 2, 0 }, { 1172, 487, 2, 0 }, { 1126, 501, 2, 0 },
    { 1011, 451, 2, 0 }, { 993, 464, 2, 0 }, { 816, 390, 2, 0 }, { 772, 401, 2, 0 }, { 538, 252, 2, 0 },
    { 543, 243, 2, 0 }, { 582, 437, 2, 0 }, { 576, 452, 2, 0 }, { 386, 309, 2, 0 }, { 378, 332, 2, 0 },
    { 1303, 404, 2, 0 }, { 1269, 405, 2, 0 }, { 37, 265, 3, 0 }, { 77, 285, 3, 0 }, { 117, 306, 3, 0 },
    { 157, 326, 3, 0 }, { 197, 345, 3, 0 }, { 237, 364, 3, 0 }, { 277, 385, 3, 0 }, { 479, 486, 3, 0 },
    { 518, 505, 3, 0 }, { 36, 292, 4, 0 }, { 76, 312, 4, 0 }, { 116, 333, 4, 0 }, { 156, 353, 4, 0 },
    { 196, 372, 4, 0 }, { 236, 391, 4, 0 }, { 478, 515, 4, 0 }, { 517, 532, 4, 0 }, { 614, 92, 4, 1 },
    { 28, 339, 5, 0 }, { 68, 359, 5, 0 }, { 108, 380, 5, 0 }, { 148, 400, 5, 0 }, { 188, 419, 5, 0 },
    { 228, 438, 5, 0 }, { 469, 562, 5, 0 }, { 509, 580, 5, 0 }, { 621, 124, 5, 1 }, { 942, 206, 6, 0 },
    { 469, 110, 6, 1 }, { 943, 255, 7, 0 }, { 469, 137, 7, 1 }, { 1004, 164, 8, 0 }, { 406, 48, 8, 1 },
    { 476, 160, 8, 1 }, { 0, 0, 0, 0 },
};
FieldstgListedBattle D_WSTAG205_800A69BC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A69C8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A69D4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A69E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A69EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A69F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A04 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A10 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG205_800A6A1C = {
    3,
    { &D_WSTAG205_800A69BC, &D_WSTAG205_800A69C8, &D_WSTAG205_800A69D4, &D_WSTAG205_800A69E0, &D_WSTAG205_800A69EC,
        &D_WSTAG205_800A69F8, &D_WSTAG205_800A6A04, &D_WSTAG205_800A6A10 },
};
FieldstgListedBattle D_WSTAG205_800A6A40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A64 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A70 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A7C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A88 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6A94 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG205_800A6AA0 = {
    0,
    { &D_WSTAG205_800A6A40, &D_WSTAG205_800A6A4C, &D_WSTAG205_800A6A58, &D_WSTAG205_800A6A64, &D_WSTAG205_800A6A70,
        &D_WSTAG205_800A6A7C, &D_WSTAG205_800A6A88, &D_WSTAG205_800A6A94 },
};
FieldstgListedBattle D_WSTAG205_800A6AC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6AD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6ADC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6AE8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6AF4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B00 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B0C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B18 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG205_800A6B24 = {
    0,
    { &D_WSTAG205_800A6AC4, &D_WSTAG205_800A6AD0, &D_WSTAG205_800A6ADC, &D_WSTAG205_800A6AE8, &D_WSTAG205_800A6AF4,
        &D_WSTAG205_800A6B00, &D_WSTAG205_800A6B0C, &D_WSTAG205_800A6B18 },
};
FieldstgListedBattle D_WSTAG205_800A6B48 = { 263, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG205_800A6B54 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B60 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B6C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B78 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B84 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B90 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG205_800A6B9C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG205_800A6BA8 = {
    0,
    { &D_WSTAG205_800A6B48, &D_WSTAG205_800A6B54, &D_WSTAG205_800A6B60, &D_WSTAG205_800A6B6C, &D_WSTAG205_800A6B78,
        &D_WSTAG205_800A6B84, &D_WSTAG205_800A6B90, &D_WSTAG205_800A6B9C },
};
FieldstgBattleLists wstag205_battle_lists = {
    168, 0, 0, { &D_WSTAG205_800A6A1C, &D_WSTAG205_800A6AA0, &D_WSTAG205_800A6B24 }, &D_WSTAG205_800A6BA8,
};
FieldstgVramPlace wstag205_vram_places[13] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 360, 388, 160, 132, 336, 511 }, { 384, 256, 384, 324, 256, 68, 352, 511 },
    { 384, 256, 392, 324, 288, 68, 368, 511 }, { 384, 256, 434, 458, 456, 202, 320, 510 },
    { 384, 256, 400, 328, 320, 72, 336, 510 }, { 448, 256, 490, 280, 680, 24, 352, 510 },
    { 448, 256, 498, 280, 712, 24, 368, 510 },
};
u16 D_WSTAG205_800A6CB8[8] = { 0x206, 1, 0x846C, 1, 0x7013, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6CC8[4] = { 0x1A19, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG205_800A6CD0[2] = { { NULL, D_WSTAG205_800A6CB8, 1100 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6CE8[2] = { { NULL, NULL, 731 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D00[2] = { { NULL, NULL, 732 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D18[2] = { { NULL, NULL, 733 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D30[2] = { { NULL, NULL, 734 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D48[2] = { { NULL, NULL, 740 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D60[2] = { { NULL, NULL, 736 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D78[2] = { { NULL, NULL, 737 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6D90[2] = { { NULL, NULL, 738 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6DA8[2] = { { NULL, NULL, 26 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6DC0[2] = { { NULL, NULL, 27 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6DD8[2] = { { NULL, NULL, 741 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6DF0[2] = { { NULL, NULL, 743 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E08[2] = { { NULL, NULL, 742 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E20[2] = { { NULL, NULL, 744 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E38[2] = { { NULL, NULL, 750 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E50[2] = { { NULL, NULL, 746 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E68[2] = { { NULL, NULL, 747 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E80[2] = { { NULL, NULL, 748 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6E98[2] = { { NULL, D_WSTAG205_800A6CC8, 80 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6EB0[2] = { { NULL, NULL, 739 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG205_800A6EC8[2] = { { NULL, NULL, 749 }, { NULL, NULL, 0 } };
u16 D_WSTAG205_800A6EE0[4] = { 0x206, 0, 0xFFFF, 0 };
u16 D_WSTAG205_800A6EE8[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6EF0[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6EF8[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F00[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F08[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F10[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F18[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F20[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F28[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F30[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F38[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F40[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F48[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F50[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F58[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F60[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F68[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F70[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F78[6] = { 0x6006, 1, 0x4016, 0, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F84[8] = { 0x6008, 1, 0x1A18, 1, 0x1A19, 0, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F94[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG205_800A6F9C[4] = { 0x7005, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG205_800A6FA4 = { D_WSTAG205_800A6EE0, D_WSTAG205_800A6CD0, 33, 4, 304, 160, 1 };
FieldstgPlacedActor D_WSTAG205_800A6FB8 = { D_WSTAG205_800A6EE8, D_WSTAG205_800A6CE8, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A6FCC = { D_WSTAG205_800A6EF0, D_WSTAG205_800A6D00, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A6FE0 = { D_WSTAG205_800A6EF8, D_WSTAG205_800A6D18, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A6FF4 = { D_WSTAG205_800A6F00, D_WSTAG205_800A6D30, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A7008 = { D_WSTAG205_800A6F08, D_WSTAG205_800A6D48, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A701C = { D_WSTAG205_800A6F10, D_WSTAG205_800A6D60, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A7030 = { D_WSTAG205_800A6F18, D_WSTAG205_800A6D78, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A7044 = { D_WSTAG205_800A6F20, D_WSTAG205_800A6D90, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A7058 = { D_WSTAG205_800A6F28, D_WSTAG205_800A6DA8, 37, 5, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A706C = { D_WSTAG205_800A6F30, D_WSTAG205_800A6DC0, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A7080 = { D_WSTAG205_800A6F38, D_WSTAG205_800A6DD8, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A7094 = { D_WSTAG205_800A6F40, D_WSTAG205_800A6DF0, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A70A8 = { D_WSTAG205_800A6F48, D_WSTAG205_800A6E08, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A70BC = { D_WSTAG205_800A6F50, D_WSTAG205_800A6E20, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A70D0 = { D_WSTAG205_800A6F58, D_WSTAG205_800A6E38, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A70E4 = { D_WSTAG205_800A6F60, D_WSTAG205_800A6E50, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A70F8 = { D_WSTAG205_800A6F68, D_WSTAG205_800A6E68, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A710C = { D_WSTAG205_800A6F70, D_WSTAG205_800A6E80, 38, 6, 848, 240, 1 };
FieldstgPlacedActor D_WSTAG205_800A7120 = { D_WSTAG205_800A6F78, NULL, 102, 7, 792, 221, 1 };
FieldstgPlacedActor D_WSTAG205_800A7134 = { D_WSTAG205_800A6F84, D_WSTAG205_800A6E98, 114, 8, 1144, 405, 1 };
FieldstgPlacedActor D_WSTAG205_800A7148 = { D_WSTAG205_800A6F94, D_WSTAG205_800A6EB0, 157, 9, 736, 184, 1 };
FieldstgPlacedActor D_WSTAG205_800A715C = { D_WSTAG205_800A6F9C, D_WSTAG205_800A6EC8, 158, 10, 848, 240, 1 };
FieldstgPlacedActor *wstag205_actors[24] = {
    &D_WSTAG205_800A6FA4, &D_WSTAG205_800A6FB8, &D_WSTAG205_800A6FCC, &D_WSTAG205_800A6FE0, &D_WSTAG205_800A6FF4,
    &D_WSTAG205_800A7008, &D_WSTAG205_800A701C, &D_WSTAG205_800A7030, &D_WSTAG205_800A7044, &D_WSTAG205_800A7058,
    &D_WSTAG205_800A706C, &D_WSTAG205_800A7080, &D_WSTAG205_800A7094, &D_WSTAG205_800A70A8, &D_WSTAG205_800A70BC,
    &D_WSTAG205_800A70D0, &D_WSTAG205_800A70E4, &D_WSTAG205_800A70F8, &D_WSTAG205_800A710C, &D_WSTAG205_800A7120,
    &D_WSTAG205_800A7134, &D_WSTAG205_800A7148, &D_WSTAG205_800A715C, NULL,
};
FieldstgSprite wstag205_sprites[103] = {
    { 1, 0, 0x80, 2, 0, 0, 0, 0, 0, 0, 384, 384, 0, 0 }, { 1, 0, 0x40, 2, 1, 0, 0, 0, 0, 0, 128, 475, 0, 0 },
    { 1, 0, 0x40, 2, 2, 0, 0, 0, 0, 0, 240, 499, 0, 0 }, { 1, 0, 0x40, 2, 3, 0, 0, 0, 0, 0, 252, 384, 0, 0 },
    { 1, 0, 0x80, 2, 4, 0, 0, 0, 0, 0, 512, 307, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 496, 371, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 248, 352, 0, 0 }, { 1, 0, 0x40, 2, 0x10, 0, 0, 0, 0, 0, 104, 512, 0, 0 },
    { 1, 0, 0x80, 2, 7, 0, 0, 0, 0, 0, 896, 128, 0, 0 }, { 1, 0, 0x80, 2, 8, 0, 0, 0, 0, 0, 256, 384, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 8, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 11, 459, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 22, 447, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 142, 254, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 344, 345, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 410, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 417, 596, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 507, 261, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 606, 427, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 680, 401, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 712, 393, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x34, 0xA, 0, 985, 188, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 37, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 357, 335, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 444, 299, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 457, 287, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 540, 244, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 554, 453, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 782, 396, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 817, 391, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 942, 475, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 990, 449, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1109, 334, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1122, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 1, 0x35, 0x37, 0xA, 0, 1250, 276, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 102, 206, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 113, 248, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 416, 303, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 422, 57, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 574, 418, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 633, 141, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 654, 397, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 887, 427, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 928, 444, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1034, 516, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 1, 0x38, 0x3A, 0xA, 0, 1181, 488, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 168, 451, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 392, 601, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 440, 70, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 440, 555, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 510, 455, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 1085, 507, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 44, 434, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 381, 116, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 397, 206, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 415, 603, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 479, 284, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 541, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 545, 250, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 555, 460, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 711, 403, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 781, 404, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 843, 392, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 977, 194, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 985, 457, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1114, 365, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1144, 486, 0, 0 },
    { 1, 0, 0x40, 6, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 1218, 456, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 30, 340, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 70, 360, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 110, 380, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 150, 400, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 190, 420, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 470, 560, 0, 0 },
    { 1, 0, 0x40, 6, 0x50, 1, 0x50, 0x53, 4, 0, 510, 580, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 1, 0x58, 0x5B, 4, 0, 585, 125, 0, 0 },
    { 1, 0, 0x40, 6, 0x5C, 1, 0x5C, 0x5F, 4, 0, 363, 49, 0, 0 },
    { 1, 0, 0x40, 6, 0x5C, 1, 0x5C, 0x5F, 4, 0, 435, 161, 0, 0 },
    { 1, 0, 0x40, 6, 0x54, 1, 0x54, 0x57, 4, 0, 1009, 162, 0, 0 },
    { 1, 0x64, 0x80, 6, 9, 0, 0, 0, 0, 0, 758, 96, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 6, 0, 943, 296, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x2F, 1, 0x2F, 0x31, 6, 0, 1019, 140, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x42, 1, 0x42, 0x44, 6, 0, 582, 78, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x45, 1, 0x45, 0x47, 6, 0, 358, 18, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 37, 267, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 77, 287, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 117, 307, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 157, 327, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 196, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 236, 367, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 276, 387, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 477, 487, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x48, 1, 0x48, 0x4B, 6, 0, 517, 507, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x50, 1, 0x50, 0x53, 4, 0, 230, 440, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x60, 1, 0x60, 0x63, 6, 0, 422, 110, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4C, 1, 0x4C, 0x4F, 6, 0, 942, 205, 0, 0 },
    { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 224, 177, 200, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 600, 214, 273, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 1046, 371, 407, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 1062, 315, 337, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 902, 275, 295, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 846, 177, 226, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag205_map_events[8] = {
    { 0x7007, 0, 0xFFFF, 0, 1, 0x200, 0x60, 0x31C, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x21D, 0x5E2, 0xE0, 1, 0, 0, 0 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E1, 0x1D0, 0x154, 7, 0, 2, 1 },
    { 0x7093, 1, 0xFFFF, 0, 0xA, 0x2E1, 0xE0, 0x110, 7, 0, 2, 1 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 3, 0x110, 0x100, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 3, 0x120, 0xC8, 0, 0, 0, 0 }, { 0x6006, 1, 0x4006, 0, 8, 0x64, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag205_funcs = { wstag205_setup };
FieldstgEventDef wstag205_events[3] = {
    { 100, D_WSTAG205_800A63D0, 0x01120015, NULL, wstag205_event_100_end },
    { 101, D_WSTAG205_800A6528, 0x01120016, NULL, wstag205_event_101_end }, { -1, NULL, 0, NULL, NULL },
};
