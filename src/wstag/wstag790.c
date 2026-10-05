#include "wstag.h"

/* WSTAG790: stage 0x2D9 (fieldstg_stages). */

/* An object that plays a sequence over the stage's sprites of types 2-9. */
typedef struct Wstag790Sequence {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgSprite *sprites[8];
} Wstag790Sequence; /* size 0x70 */

extern WstagFuncs wstag790_funcs;
extern FieldstgVramPlace wstag790_vram_places[];
extern FieldstgPlacedActor *wstag790_actors[];
extern FieldstgSprite wstag790_sprites[];
extern FieldstgEventDef wstag790_events[];
void wstag790_sequence_update(Wstag790Sequence *obj);
/* An object that changes the frame of the sprite of type 1 when it is told to. */
typedef struct Wstag790Glow {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 frame; /* frame */
    /* 0x54 */ s32 unk_54;
    /* 0x58 */ s32 unk_58;
} Wstag790Glow; /* size 0x5C */

void wstag790_glow_update(Wstag790Glow *obj);
/* The data of the stage object: an event and two more objects. */
typedef struct Wstag790Data {
    /* 0x0 */ FieldstgEvent *event;
    /* 0x4 */ void *glow;
    /* 0x8 */ void *sequence;
} Wstag790Data; /* size 0xC */

void wstag790_update(WstagObject *obj, Wstag790Data *data);
Wstag790Sequence *wstag790_sequence_create(s32 arg0);

void wstag790_sequence_update(Wstag790Sequence *obj) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 8; i++) {
            obj->sprites[i] = fieldstg_sprites_find_first(i + 2);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
                sound_module.play(0x8100383C);
                obj->base.next_substep(obj);
            case 1:
                if (obj->base.timer & 1) {
                    obj->sprites[0]->y++;
                    obj->sprites[1]->y++;
                    obj->sprites[2]->y++;
                    obj->sprites[5]->y++;
                }
                if (++obj->base.timer != 8) {
                    break;
                }
                obj->base.next_substep(obj);
            case 2:
                if (obj->base.timer & 1) {
                    obj->sprites[1]->y++;
                    obj->sprites[5]->y++;
                }
                if (++obj->base.timer != 0x10) {
                    break;
                }
                obj->base.next_substep(obj);
            case 3:
                if (obj->base.timer == 0) {
                    obj->sprites[3]->shown = 1;
                    sound_module.play(0x01000002);
                    obj->base.next_timer(obj);
                }
                if (obj->sprites[3]->frame == 0xF) {
                    obj->base.next_substep(obj);
                }
                break;
            case 4:
                if (obj->base.timer == 0) {
                    obj->sprites[3]->shown = 0;
                    obj->sprites[4]->shown = 1;
                    obj->sprites[5]->shown = 0;
                    obj->sprites[6]->shown = 1;
                    obj->base.next_timer(obj);
                }
                if (obj->sprites[6]->frame == 0xF) {
                    obj->sprites[6]->shown = 0;
                    obj->sprites[7]->shown = 1;
                    obj->base.next_substep(obj);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag790_sequence_message(Object *obj, s32 msg) {
    if (obj != NULL && msg == 0x32C) {
        obj->set_step(obj, 1);
    }
}

/* arg0 (0x329, the object's id) is not used. */
Wstag790Sequence *wstag790_sequence_create(s32 arg0) {
    return object_create(wstag790_sequence_update, sizeof(Wstag790Sequence), 0, 0x329);
}



void wstag790_glow_update(Wstag790Glow *obj) {
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            if (obj->base.substep == 0) {
                obj->frame = (obj->base.timer >> 2) + 5;
                if (++obj->base.timer >= 0x14) {
                    sound_module.play(0x8100303C);
                    obj->base.next_substep(obj);
                    break;
                }
                sprite = fieldstg_sprites_find_first(1);
                if (sprite != NULL) {
                    sprite->sprite = obj->frame;
                }
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag790_glow_message(Object *obj, s32 msg) {
    if (obj != NULL && msg == 0x32E) {
        obj->set_step(obj, 1);
    }
}


Object *wstag790_glow_create(s32 arg0) {
    return object_create(wstag790_glow_update, 0x5C, 0, arg0);
}

void wstag790_update(WstagObject *obj, Wstag790Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->sequence = wstag790_sequence_create(0x329);
        data->glow = wstag790_glow_create(0x328);
        if (gamestate_data.progress == 1) {
            data->event = fieldstg_event_start(4);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag790_start(void *arg0) {
    WstagObject *obj = object_new(wstag790_update, sizeof(WstagObject), sizeof(Wstag790Data));

    obj->manager = arg0;
    wstag790_funcs.setup();
    return obj;
}

void wstag790_setup(void) {
    fieldstg_stage.background_file = 0x1C8;
    fieldstg_stage.sprites = wstag790_sprites;
    fieldstg_stage.sprite_file = 0x01C90001;
    fieldstg_stage.mask_file = 0x331;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x15900, 0x11700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag790_vram_places;
    fieldstg_stage.music = 0x40;
    fieldstg_stage.actors = wstag790_actors;
    fieldstg_stage.sound = 0x61000001;
    fieldstg_stage.events = wstag790_events;
    fieldstg_attr.set_file(0, 0x01C90000);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag790_setup(void);

s16 D_WSTAG790_800A6454[351] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 343, 289),
    FIELDSTG_EVENT_PLACE(1, 0, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_PLACE(11, 0, 0),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_PLACE(12, 0, 0),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_PLACE(13, 394, 246),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_ANIM(0x328, 813, 808),
    FIELDSTG_EVENT_ANIM(0x329, 813, 808),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(1, 229, 345),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 343, 289, 0),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 411, 255, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_ANIM(13, 2, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 41, 7),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(1, 42, 7),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 13),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 806, 13),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 13, 0),
    FIELDSTG_EVENT_ANIM(13, 7, 7),
    FIELDSTG_EVENT_ANIM(0x329, 811, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(1, 411, 255),
    FIELDSTG_EVENT_WALK(1, 469, 225, 5),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_PLACE(1, 0, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 0),
    FIELDSTG_EVENT_ANIM(13, 2, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(11, 235, 337),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(11, 254, 328, 5),
    FIELDSTG_EVENT_PLACE(12, 235, 337),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_WALK(11, 369, 271, 5),
    FIELDSTG_EVENT_WALK(12, 346, 283, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x328, 814, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(11, 51, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 7, 11, 2),
    FIELDSTG_EVENT_ANIM(11, 12, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x329, 812, 1),
    FIELDSTG_EVENT_WAIT(300),
    FIELDSTG_EVENT_GOTO_MAP(0xE03, 368, 287, 1),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag790_vram_places[10] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 358, 464, 152, 208, 368, 507 }, { 384, 256, 438, 304, 472, 48, 368, 506 },
    { 384, 256, 436, 419, 464, 163, 368, 505 }, { 384, 256, 422, 304, 408, 48, 368, 504 },
};
u16 D_WSTAG790_800A67B4[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG790_800A67BC[4] = { 0x6001, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG790_800A67C4 = { NULL, NULL, 1, 4, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG790_800A67D8 = { D_WSTAG790_800A67B4, NULL, 11, 5, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG790_800A67EC = { NULL, NULL, 12, 6, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG790_800A6800 = { D_WSTAG790_800A67BC, NULL, 13, 7, 0, 0, 0 };
FieldstgPlacedActor *wstag790_actors[5] = {
    &D_WSTAG790_800A67C4, &D_WSTAG790_800A67D8, &D_WSTAG790_800A67EC, &D_WSTAG790_800A6800, NULL,
};
FieldstgSprite wstag790_sprites[19] = {
    { 1, 4, 0x40, 2, 0xC, 0, 0, 0, 0, 0, 479, 117, 0, 0 }, { 1, 7, 0x40, 2, 0x39, 0, 0, 0, 0, 0, 407, 126, 0, 0 },
    { 0, 8, 0x40, 2, 0x34, 2, 0, 0xF, 5, 0, 407, 138, 0, 0 },
    { 0, 9, 0x40, 2, 0x38, 2, 0xE, 0xF, 4, 0xE, 407, 138, 0, 0 },
    { 1, 3, 0x90, 2, 0xB, 0, 0, 0, 0, 0, 415, 79, 0, 0 }, { 1, 2, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 387, 76, 0, 0 },
    { 0, 5, 0x40, 2, 0x33, 2, 0, 0xF, 4, 0, 428, 172, 0, 0 },
    { 0, 6, 0x40, 2, 0x37, 2, 0xE, 0xF, 4, 0xE, 428, 172, 0, 0 },
    { 1, 1, 0x40, 2, 4, 0, 0, 0, 0, 0, 418, 191, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 2, 0, 254, 146, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 2, 0, 442, 241, 0, 0 }, { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 2, 0, 538, 288, 0, 0 },
    { 1, 0, 0x40, 6, 0x36, 0, 0, 7, 0xA, 0, 361, 205, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x35, 0, 0, 0, 0, 0, 150, 0, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 398, 279, 296, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 430, 247, 264, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 397, 263, 281, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 429, 232, 247, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag790_funcs = { wstag790_setup };
FieldstgEventDef wstag790_events[2] = {
    { 4, D_WSTAG790_800A6454, 0x014A0003, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
