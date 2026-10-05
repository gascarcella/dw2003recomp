#include "wstag.h"

/* WSTAG306: stage 0x287 (fieldstg_stages). */

extern WstagAnimKey D_WSTAG306_800A6738[];
extern WstagFuncs wstag306_funcs;
extern FieldstgVramPlace wstag306_vram_places[];
extern FieldstgPlacedActor *wstag306_actors[];
extern FieldstgSprite wstag306_sprites[];
extern FieldstgMapEvent wstag306_map_events[];
extern FieldstgEventDef wstag306_events[];
void wstag306_sound_anim_update();
void wstag306_update(WstagObject *obj, WstagObjEventData *data);

s32 wstag306_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        sa->anim.time -= step;
    }
    if (sa->anim.time <= 0) {
        key++;
        sa->anim.key++;
        sa->anim.time += key->time;
        if (key->frame == 0xFF) {
            return 0xFF;
        }
        wstag306_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag306_sound_anim_update(WstagSoundAnimObject *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            if (sprite->type == 1) {
                obj->sprite.anim.key = 0;
                obj->sprite.anim.time = D_WSTAG306_800A6738[0].time;
                obj->sprite.sprite = sprite;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        spr = obj->sprite.sprite;
        switch (obj->base.step) {
        case 0:
        default:
            spr->sprite = 5;
            spr->shown = 1;
            break;
        case 1:
            spr->shown = 1;
            frame = wstag306_sprite_anim_play_once(&obj->sprite, D_WSTAG306_800A6738, 0);
            if (frame != 0xFF) {
                spr->sprite = frame;
            } else {
                spr->sprite = 10;
                obj->base.next_step(obj);
                sound_module.key_off(0xA0042FCB, obj->voice);
            }
            break;
        case 2:
            spr->shown = 1;
            spr->sprite = 10;
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag306_sound_anim_message(WstagSoundAnimObject *obj, s32 msg) {
    if (obj != NULL && msg == 0x335) {
        obj->voice = sound_module.play(0xA0042FCB);
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG306_800A6738[0].time;
        obj->base.set_step(obj, 1);
    }
}

Object *wstag306_sound_anim_create(s32 arg0) {
    return object_create(wstag306_sound_anim_update, sizeof(WstagSoundAnimObject), 0, arg0);
}

void wstag306_update(WstagObject *obj, WstagObjEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        switch (gamestate_data.progress) {
        case 0x25:
            data->event = fieldstg_event_start(0x3A2);
            break;
        case 0x27:
            data->event = fieldstg_event_start(0x3D4);
            break;
        }
        if (gamestate_data.progress < 0x27) {
            data->object = wstag306_sound_anim_create(0x353);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag306_start(void *arg0) {
    WstagObject *obj = object_new(wstag306_update, sizeof(WstagObject), sizeof(WstagObjEventData));

    obj->manager = arg0;
    wstag306_funcs.setup();
    return obj;
}

void wstag306_setup(void) {
    fieldstg_stage.background_file = 0x545;
    fieldstg_stage.sprite_file = 0x05460000;
    fieldstg_stage.sprites = wstag306_sprites;
    fieldstg_stage.map_events = wstag306_map_events;
    fieldstg_stage.mask_file = 0x544;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x13900, 0x11B00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag306_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag306_actors;
    fieldstg_stage.events = wstag306_events;
    fieldstg_attr.set_file(0, 0x05460001);
    fieldstg_attr.set_file(7, 0x05460002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag306_setup(void);

s16 D_WSTAG306_800A62FC[160] = {
    FIELDSTG_EVENT_PLACE(1, 79, 401),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 328, 277, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x353, 821, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 400, 240, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x288, 88, 524, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG306_800A643C[39] = {
    FIELDSTG_EVENT_WALK(2, 392, 245, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x288, 88, 524, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG306_800A648C[341] = {
    FIELDSTG_EVENT_PLACE(1, 79, 400),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_PLACE(157, 360, 277),
    FIELDSTG_EVENT_ANIM(157, 1, 5),
    FIELDSTG_EVENT_PLACE(158, 328, 260),
    FIELDSTG_EVENT_ANIM(158, 1, 5),
    FIELDSTG_EVENT_PLACE(281, 335, 296),
    FIELDSTG_EVENT_ANIM(281, 1, 5),
    FIELDSTG_EVENT_PLACE(314, 288, 272),
    FIELDSTG_EVENT_ANIM(314, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 208, 336, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(157, 1, 1),
    FIELDSTG_EVENT_ANIM(158, 1, 1),
    FIELDSTG_EVENT_ANIM(281, 1, 1),
    FIELDSTG_EVENT_ANIM(314, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 279, 300, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 310, 283),
    FIELDSTG_EVENT_ANIM(281, 1, 2),
    FIELDSTG_EVENT_ANIM(314, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 2, 314, 0),
    FIELDSTG_EVENT_DIALOG(0, 1, 281, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 4, 158, 0),
    FIELDSTG_EVENT_DIALOG(0, 3, 157, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 281, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_WALK(1, 319, 280, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 369, 255, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(157, 1, 5),
    FIELDSTG_EVENT_ANIM(158, 1, 5),
    FIELDSTG_EVENT_ANIM(281, 1, 5),
    FIELDSTG_EVENT_ANIM(314, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 319, 280),
    FIELDSTG_EVENT_DIALOG(1, 10, 314, 0),
    FIELDSTG_EVENT_DIALOG(0, 9, 281, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 12, 158, 0),
    FIELDSTG_EVENT_DIALOG(0, 11, 157, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_ANIM(1, 9, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 416, 232, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x288, 96, 520, 5),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG306_800A6738[6] = { { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 255, 999 } };
FieldstgVramPlace wstag306_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 396, 216, 140, 320, 507 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 384, 256, 410, 256, 360, 0, 336, 507 }, { 384, 256, 418, 256, 392, 0, 352, 507 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 384, 256, 384, 276, 256, 20, 336, 506 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 384, 256, 392, 278, 288, 22, 352, 506 },
};
FieldstgTalk D_WSTAG306_800A6870[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6888[2] = { { NULL, NULL, 6 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A68A0[2] = { { NULL, NULL, 505 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A68B8[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A68D0[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A68E8[2] = { { NULL, NULL, 4 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6900[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6918[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6930[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6948[2] = { { NULL, NULL, 524 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG306_800A6960[2] = { { NULL, NULL, 5 }, { NULL, NULL, 0 } };
u16 D_WSTAG306_800A6978[4] = { 0x6025, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A6980[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A6988[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A6990[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A6998[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69A0[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69A8[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69B0[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69B8[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69C0[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69C8[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69D0[4] = { 0x7094, 1, 0xFFFF, 0 };
u16 D_WSTAG306_800A69D8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG306_800A69E0 = { D_WSTAG306_800A6978, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG306_800A69F4 = { D_WSTAG306_800A6980, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG306_800A6A08 = { D_WSTAG306_800A6988, D_WSTAG306_800A6870, 63, 5, 121, 260, 7 };
FieldstgPlacedActor D_WSTAG306_800A6A1C = { D_WSTAG306_800A6990, D_WSTAG306_800A6888, 157, 6, 463, 264, 1 };
FieldstgPlacedActor D_WSTAG306_800A6A30 = { D_WSTAG306_800A6998, D_WSTAG306_800A68A0, 158, 7, 367, 312, 5 };
FieldstgPlacedActor D_WSTAG306_800A6A44 = { D_WSTAG306_800A69A0, D_WSTAG306_800A68B8, 198, 8, 185, 229, 7 };
FieldstgPlacedActor D_WSTAG306_800A6A58 = { D_WSTAG306_800A69A8, D_WSTAG306_800A68D0, 199, 9, 249, 197, 7 };
FieldstgPlacedActor D_WSTAG306_800A6A6C = { D_WSTAG306_800A69B0, D_WSTAG306_800A68E8, 281, 10, 428, 278, 1 };
FieldstgPlacedActor D_WSTAG306_800A6A80 = { D_WSTAG306_800A69B8, D_WSTAG306_800A6900, 289, 11, 376, 188, 1 };
FieldstgPlacedActor D_WSTAG306_800A6A94 = { D_WSTAG306_800A69C0, D_WSTAG306_800A6918, 290, 12, 360, 380, 3 };
FieldstgPlacedActor D_WSTAG306_800A6AA8 = { D_WSTAG306_800A69C8, D_WSTAG306_800A6930, 291, 13, 424, 348, 3 };
FieldstgPlacedActor D_WSTAG306_800A6ABC = { D_WSTAG306_800A69D0, D_WSTAG306_800A6948, 292, 14, 488, 316, 3 };
FieldstgPlacedActor D_WSTAG306_800A6AD0 = { D_WSTAG306_800A69D8, D_WSTAG306_800A6960, 314, 15, 451, 290, 1 };
FieldstgPlacedActor *wstag306_actors[14] = {
    &D_WSTAG306_800A69E0, &D_WSTAG306_800A69F4, &D_WSTAG306_800A6A08, &D_WSTAG306_800A6A1C, &D_WSTAG306_800A6A30,
    &D_WSTAG306_800A6A44, &D_WSTAG306_800A6A58, &D_WSTAG306_800A6A6C, &D_WSTAG306_800A6A80, &D_WSTAG306_800A6A94,
    &D_WSTAG306_800A6AA8, &D_WSTAG306_800A6ABC, &D_WSTAG306_800A6AD0, NULL,
};
FieldstgSprite wstag306_sprites[10] = {
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x3B, 6, 0, 374, 136, 0, 0 },
    { 1, 0, 0x40, 6, 0x3C, 1, 0x3C, 0x45, 8, 0, 408, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x46, 1, 0x46, 0x57, 8, 0, 418, 102, 0, 0 },
    { 0, 1, 0x40, 6, 5, 0, 5, 0xA, 4, 0, 382, 196, 0, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 453, 185, 199, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 449, 190, 204, 0 }, { 1, 0, 0x58, 4, 2, 0, 0, 0, 0, 0, 351, 136, 218, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 215, 302, 353, 0 }, { 1, 0, 0x48, 4, 4, 0, 0, 0, 0, 0, 144, 262, 322, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag306_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x286, 0x398, 0xCC, 1, 0, 0, 0 },
    { 0x701A, 1, 0xFFFF, 0, 8, 0x3C6, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag306_funcs = { wstag306_setup };
FieldstgEventDef wstag306_events[4] = {
    { 930, D_WSTAG306_800A62FC, 0x0127000F, NULL, NULL }, { 966, D_WSTAG306_800A643C, 0x01270015, NULL, NULL },
    { 980, D_WSTAG306_800A648C, 0x01270017, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
