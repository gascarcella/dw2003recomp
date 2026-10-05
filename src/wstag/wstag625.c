#include "wstag.h"

/* WSTAG625: stage 0x256 (fieldstg_stages). */

/* One of the sprite animations of wstag625_anims_update (WstagAnimSlot with an s16 mode). */
typedef struct Wstag625Slot {
    /* 0x0 */ s16 entry; /* the animation (D_WSTAG625_800A67D4), 0: none */
    /* 0x2 */ s16 sets_sprite; /* 0: the animation sets the sprite's frame, else its sprite */
    /* 0x4 */ FieldstgSprite *sprite;
    /* 0x8 */ WstagAnim anim;
} Wstag625Slot; /* size 0xC */

/* An object that runs four sprite animations (types 1-4). */
typedef struct Wstag625Anims {
    /* 0x00 */ Object base;
    /* 0x50 */ Wstag625Slot slots[4];
} Wstag625Anims; /* size 0x80 */

extern WstagAnimEntry D_WSTAG625_800A67D4[4][4];
extern WstagFuncs wstag625_funcs;
extern FieldstgVramPlace wstag625_vram_places[];
extern FieldstgPlacedActor *wstag625_actors[];
extern FieldstgSprite wstag625_sprites[];
extern FieldstgMapEvent wstag625_map_events[];
extern FieldstgEventDef wstag625_events[];
void wstag625_update();
void wstag625_anims_update(Wstag625Anims *obj);

s32 wstag625_slot_anim_advance(Wstag625Slot *slot, WstagAnimKey *keys, s32 once, s32 depth) {
    WstagAnimKey *key = &keys[slot->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        slot->anim.time -= step;
    }
    if (slot->anim.time <= 0) {
        key++;
        slot->anim.key++;
        slot->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            slot->anim.key = 0;
            slot->anim.time += key->time;
        }
        wstag625_slot_anim_advance(slot, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag625_anims_update(Wstag625Anims *obj) {
    FieldstgSprite *sprite;
    s32 i;
    s32 frame;
    s32 n;
    FieldstgSprite *spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        n = 0;
        for (spr = fieldstg_stage.sprites; spr->present != 0; spr++) {
            if (spr->type - 1 < 4U) {
                obj->slots[n++].sprite = spr;
            }
        }
        obj->slots[0].sets_sprite = 0;
        obj->slots[1].sets_sprite = 0;
        obj->slots[2].sets_sprite = 0;
        obj->slots[3].sets_sprite = 1;
        obj->slots[0].entry = 2;
        obj->slots[1].entry = 2;
        obj->slots[2].entry = 1;
        obj->slots[3].entry = 1;
        obj->slots[0].anim.key = 0;
        obj->slots[0].anim.time = D_WSTAG625_800A67D4[0][2].keys[0].time;
        obj->slots[1].anim.key = 0;
        obj->slots[1].anim.time = D_WSTAG625_800A67D4[1][2].keys[0].time;
        obj->slots[2].anim.key = 0;
        obj->slots[2].anim.time = D_WSTAG625_800A67D4[2][1].keys[0].time;
        obj->slots[3].anim.key = 0;
        obj->slots[3].anim.time = D_WSTAG625_800A67D4[3][1].keys[0].time;
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 4; i++) {
            sprite = obj->slots[i].sprite;
            if (obj->slots[i].entry != 0) {
                frame = wstag625_slot_anim_advance(&obj->slots[i], D_WSTAG625_800A67D4[i][obj->slots[i].entry].keys,
                                               D_WSTAG625_800A67D4[i][obj->slots[i].entry].once, 0);
                switch (frame) {
                case 0xFF:
                    if (D_WSTAG625_800A67D4[i][obj->slots[i].entry].next == 0) {
                        obj->slots[i].entry = 0;
                        sprite->shown = 0;
                    } else {
                        obj->slots[i].entry = D_WSTAG625_800A67D4[i][obj->slots[i].entry].next;
                        obj->slots[i].anim.key = 0;
                        obj->slots[i].anim.time = D_WSTAG625_800A67D4[i][obj->slots[i].entry].keys[0].time;
                    }
                    break;
                case 0x12C:
                    sprite->shown = 0;
                    break;
                default:
                    sprite->shown = 1;
                    if (obj->slots[i].sets_sprite == 0) {
                        sprite->frame = frame;
                    } else {
                        sprite->sprite = frame;
                    }
                    break;
                }
            } else {
                sprite->shown = 0;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag625_anims_create(s32 arg0) {
    return object_create(wstag625_anims_update, 0x80, 0, arg0);
}

void wstag625_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_data.progress == 0x15 && gamestate_flags.get_flag(0x403D, 0)) {
            data->event = fieldstg_event_start(0x228);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag625_start(void *arg0) {
    WstagObject *obj = object_new(wstag625_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag625_funcs.setup();
    return obj;
}

void wstag625_event_552_end(void) {
    gamestate_flags.set_flag(0x403D, 1);
    gamestate_flags.set_flag(0x8191, 1);
}

void wstag625_setup(void) {
    fieldstg_stage.background_file = 0x48B;
    fieldstg_stage.sprite_file = 0x048C0000;
    fieldstg_stage.sprites = wstag625_sprites;
    fieldstg_stage.map_events = wstag625_map_events;
    fieldstg_stage.mask_file = 0x48A;
    fieldstg_stage.talk_file = records_language + 0xD3;
    fieldstg_stage.start_pos = (GamestatePos){ 0x20200, 0x17600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag625_vram_places;
    fieldstg_stage.music = 0x15;
    fieldstg_stage.sound = 0x60540000;
    fieldstg_stage.actors = wstag625_actors;
    fieldstg_stage.events = wstag625_events;
    fieldstg_attr.set_file(0, 0x048C0001);
    fieldstg_attr.set_file(7, 0x048C0002);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag625_setup(void);

s16 D_WSTAG625_800A63A4[363] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 430, 321),
    FIELDSTG_EVENT_PLACE(2, 552, 396),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_PLACE(11, 472, 381),
    FIELDSTG_EVENT_ANIM(11, 1, 4),
    FIELDSTG_EVENT_PLACE(101, 432, 260),
    FIELDSTG_EVENT_ANIM(101, 1, 7),
    FIELDSTG_EVENT_PLACE(102, 494, 232),
    FIELDSTG_EVENT_ANIM(102, 1, 5),
    FIELDSTG_EVENT_PLACE(103, 386, 416),
    FIELDSTG_EVENT_ANIM(103, 1, 3),
    FIELDSTG_EVENT_PLACE(317, 449, 392),
    FIELDSTG_EVENT_ANIM(317, 1, 4),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(1, 1, 102, 1),
    FIELDSTG_EVENT_DIALOG(0, 2, 103, 2),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 101, 3),
    FIELDSTG_EVENT_ANIM(101, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 19, 101, 3),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(1, 18, 103, 2),
    FIELDSTG_EVENT_DIALOG(0, 4, 102, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 101, 1),
    FIELDSTG_EVENT_ANIM(101, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 20, 101, 1),
    FIELDSTG_EVENT_ANIM(101, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(101, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 7, 103, 2),
    FIELDSTG_EVENT_DIALOG(0, 6, 102, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(102, 1, 5),
    FIELDSTG_EVENT_ANIM(103, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 101, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 103, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 480, 432, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 0),
    FIELDSTG_EVENT_ANIM(317, 1, 0),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 416, 400, 3),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(317, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 103, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 103),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 103),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 103, 1),
    FIELDSTG_EVENT_ANIM(103, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 842, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_MOVE(0, 430, 321),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 101, 3),
    FIELDSTG_EVENT_ANIM(101, 1, 1),
    FIELDSTG_EVENT_ANIM(102, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 103, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(101, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 101, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 11, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 2),
    FIELDSTG_EVENT_WALK(2, 456, 420, 6),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 500, 420, 6),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x254, 724, 196, 7),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG625_800A667C[9] = {
    { 0, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 0, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 255, 0 },
};
WstagAnimKey D_WSTAG625_800A66A0[19] = {
    { 0, 6 }, { 0, 6 }, { 0, 6 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 6 }, { 6, 6 }, { 7, 6 }, { 6, 6 },
    { 5, 6 }, { 4, 6 }, { 3, 6 }, { 2, 6 }, { 1, 6 }, { 0, 6 }, { 0, 6 }, { 255, 999 },
};
WstagAnimKey D_WSTAG625_800A66EC[9] = {
    { 0, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 0, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 255, 0 },
};
WstagAnimKey D_WSTAG625_800A6710[5] = { { 0, 12 }, { 1, 12 }, { 2, 12 }, { 1, 12 }, { 255, 0 } };
WstagAnimKey D_WSTAG625_800A6724[10] = {
    { 2, 12 }, { 3, 12 }, { 4, 12 }, { 5, 12 }, { 6, 12 }, { 7, 12 }, { 8, 12 }, { 9, 12 }, { 10, 12 },
    { 255, 999 },
};
WstagAnimKey D_WSTAG625_800A674C[5] = { { 9, 12 }, { 10, 12 }, { 9, 12 }, { 8, 12 }, { 255, 0 } };
WstagAnimKey D_WSTAG625_800A6760[18] = {
    { 300, 90 }, { 0, 6 }, { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 3, 6 }, { 4, 6 }, { 3, 6 }, { 4, 6 }, { 3, 6 },
    { 2, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 2, 6 }, { 1, 6 }, { 255, 999 },
};
WstagAnimKey D_WSTAG625_800A67A8[11] = {
    { 300, 108 }, { 50, 6 }, { 51, 6 }, { 52, 6 }, { 53, 6 }, { 54, 6 }, { 56, 6 }, { 57, 6 }, { 58, 6 }, { 59, 6 },
    { 255, 999 },
};
WstagAnimEntry D_WSTAG625_800A67D4[4][4] = {
    { { NULL, 0, 0 }, { D_WSTAG625_800A667C, 0, 0 }, { D_WSTAG625_800A66A0, 1, 3 }, { D_WSTAG625_800A66EC, 0, 0 } },
    { { NULL, 0, 0 }, { D_WSTAG625_800A6710, 0, 0 }, { D_WSTAG625_800A6724, 1, 3 }, { D_WSTAG625_800A674C, 0, 0 } },
    { { NULL, 0, 0 }, { D_WSTAG625_800A6760, 1, 0 }, { NULL, 0, 0 }, { NULL, 0, 0 } },
    { { NULL, 0, 0 }, { D_WSTAG625_800A67A8, 1, 0 }, { NULL, 0, 0 }, { NULL, 0, 0 } },
};
FieldstgVramPlace wstag625_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 354, 400, 136, 144, 352, 502 }, { 320, 256, 372, 336, 208, 80, 352, 501 },
    { 320, 256, 332, 376, 48, 120, 368, 501 }, { 320, 256, 338, 376, 72, 120, 320, 500 },
    { 320, 256, 362, 400, 168, 144, 352, 500 }, { 320, 256, 320, 408, 0, 152, 368, 500 },
    { 320, 256, 346, 408, 104, 152, 320, 499 }, { 320, 256, 338, 416, 72, 160, 352, 499 },
};
FieldstgTalk D_WSTAG625_800A6934[2] = { { NULL, NULL, 774 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A694C[2] = { { NULL, NULL, 775 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A6964[2] = { { NULL, NULL, 775 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A697C[2] = { { NULL, NULL, 772 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A6994[2] = { { NULL, NULL, 772 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A69AC[2] = { { NULL, NULL, 773 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A69C4[2] = { { NULL, NULL, 773 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG625_800A69DC[2] = { { NULL, NULL, 771 }, { NULL, NULL, 0 } };
u16 D_WSTAG625_800A69F4[4] = { 0x7007, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A69FC[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A04[4] = { 0x6015, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A0C[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A14[4] = { 0x6015, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A1C[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A24[4] = { 0x6015, 1, 0xFFFF, 0 };
u16 D_WSTAG625_800A6A2C[4] = { 0x7007, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG625_800A6A34 = { D_WSTAG625_800A69F4, D_WSTAG625_800A6934, 11, 4, 472, 381, 1 };
FieldstgPlacedActor D_WSTAG625_800A6A48 = { D_WSTAG625_800A69FC, D_WSTAG625_800A694C, 101, 5, 385, 416, 3 };
FieldstgPlacedActor D_WSTAG625_800A6A5C = { D_WSTAG625_800A6A04, D_WSTAG625_800A6964, 101, 5, 385, 416, 3 };
FieldstgPlacedActor D_WSTAG625_800A6A70 = { D_WSTAG625_800A6A0C, D_WSTAG625_800A697C, 102, 6, 496, 232, 7 };
FieldstgPlacedActor D_WSTAG625_800A6A84 = { D_WSTAG625_800A6A14, D_WSTAG625_800A6994, 102, 6, 496, 232, 7 };
FieldstgPlacedActor D_WSTAG625_800A6A98 = { D_WSTAG625_800A6A1C, D_WSTAG625_800A69AC, 103, 7, 400, 232, 3 };
FieldstgPlacedActor D_WSTAG625_800A6AAC = { D_WSTAG625_800A6A24, D_WSTAG625_800A69C4, 103, 7, 400, 232, 3 };
FieldstgPlacedActor D_WSTAG625_800A6AC0 = { NULL, NULL, 154, 8, 641, 350, 0 };
FieldstgPlacedActor D_WSTAG625_800A6AD4 = { NULL, NULL, 155, 9, 593, 325, 1 };
FieldstgPlacedActor D_WSTAG625_800A6AE8 = { NULL, NULL, 156, 10, 545, 301, 2 };
FieldstgPlacedActor D_WSTAG625_800A6AFC = { D_WSTAG625_800A6A2C, D_WSTAG625_800A69DC, 317, 11, 449, 392, 5 };
FieldstgPlacedActor *wstag625_actors[12] = {
    &D_WSTAG625_800A6A34, &D_WSTAG625_800A6A48, &D_WSTAG625_800A6A5C, &D_WSTAG625_800A6A70, &D_WSTAG625_800A6A84,
    &D_WSTAG625_800A6A98, &D_WSTAG625_800A6AAC, &D_WSTAG625_800A6AC0, &D_WSTAG625_800A6AD4, &D_WSTAG625_800A6AE8,
    &D_WSTAG625_800A6AFC, NULL,
};
FieldstgSprite wstag625_sprites[13] = {
    { 0, 1, 0x40, 2, 0x3D, 0, 0, 0, 0, 0, 335, 351, 0, 0 }, { 0, 2, 0x40, 2, 0x3E, 0, 0, 0, 0, 0, 345, 372, 0, 0 },
    { 0, 3, 0x40, 2, 0x3C, 0, 0, 0, 0, 0, 477, 356, 0, 0 }, { 0, 4, 0x40, 2, 0x32, 0, 0, 0, 0, 0, 489, 350, 0, 0 },
    { 1, 0, 0x40, 2, 0x5F, 2, 0, 3, 6, 0, 521, 212, 0, 0 }, { 1, 0, 0x40, 2, 0x5F, 2, 0, 3, 6, 0, 569, 236, 0, 0 },
    { 1, 0, 0x40, 2, 0x60, 2, 0, 3, 6, 0, 617, 260, 0, 0 }, { 1, 0, 0x40, 2, 0x62, 2, 0, 5, 6, 0, 392, 163, 0, 0 },
    { 1, 0, 0x40, 2, 0x63, 2, 0, 5, 6, 0, 369, 205, 0, 0 }, { 1, 0, 0x40, 6, 0x5E, 2, 0, 3, 6, 0, 360, 389, 0, 0 },
    { 1, 0, 0x40, 6, 0x61, 2, 0, 5, 6, 0, 361, 186, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 377, 371, 399, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag625_map_events[4] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x254, 0x2D4, 0xC4, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 3, 5, 0x1D0, 0xF8, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 2, 5, 0x1E0, 0x150, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag625_funcs = { wstag625_setup };
FieldstgEventDef wstag625_events[2] = {
    { 552, D_WSTAG625_800A63A4, 0x01430018, NULL, wstag625_event_552_end }, { -1, NULL, 0, NULL, NULL },
};
