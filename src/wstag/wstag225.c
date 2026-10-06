#include "wstag.h"

/* WSTAG225: stage 0x207 (fieldstg_stages). */

extern WstagAnimKey *D_WSTAG225_800A662C[3];
extern WstagFuncs wstag225_funcs;
extern s8 D_WSTAG225_800A6638[];
extern s8 D_WSTAG225_800A663C[];
extern WstagPos D_WSTAG225_800A6640[];
extern FieldstgVramPlace wstag225_vram_places[];
extern FieldstgPlacedActor *wstag225_actors[];
extern FieldstgSprite wstag225_sprites[];
extern FieldstgMapEvent wstag225_map_events[];
extern FieldstgEventDef wstag225_events[];
void wstag225_sprite_anim_update(WstagSpriteAnimObject *obj);
void wstag225_update();

s32 wstag225_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        wstag225_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag225_sprite_anim_reset(WstagSpriteAnimObject *obj) {
    s32 i;

    for (i = 0; i < 3; i++) {
        obj->sprites[i].anim.key = 0;
        obj->sprites[i].anim.time = D_WSTAG225_800A662C[i]->time;
    }
}

void wstag225_sprite_anim_update(WstagSpriteAnimObject *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 n;
    s32 i;
    s32 j;
    s32 done;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        wstag225_sprite_anim_reset(obj);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
                list = fieldstg_stage.sprites;
                for (n = 0; list->present != 0; list++) {
                    if (list->type >= 1 && list->type <= 3) {
                        obj->sprites[n].sprite = list;
                        list->x = obj->x;
                        list->y = obj->y + D_WSTAG225_800A663C[n];
                        list->priority = obj->y + D_WSTAG225_800A6638[n];
                        n++;
                    }
                }
                sound_module.play(0xCC0001);
                obj->base.next_substep(obj);
            case 1:
                done = 0;
                for (i = 0; i < 3; i++) {
                    sprite = obj->sprites[i].sprite;
                    frame = wstag225_sprite_anim_play_once(&obj->sprites[i], D_WSTAG225_800A662C[i], 0);
                    switch (frame) {
                    case 0xFF:
                        done++;
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    case 0x12C:
                        sprite->shown = 0;
                        sprite->sprite = 0;
                        break;
                    default:
                        sprite->shown = 1;
                        sprite->sprite = frame;
                        break;
                    }
                }
                if (done < 3) {
                    break;
                }
                obj->base.next_substep(obj);
            case 2:
                for (j = 0; j < 3; j++) {
                    obj->sprites[j].sprite->shown = 0;
                }
                wstag225_sprite_anim_reset(obj);
                obj->base.set_step(obj, 0);
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

void wstag225_sprite_anim_message(WstagSpriteAnimObject *obj, s32 msg) {
    s32 i;

    if (obj == NULL) {
        return;
    }
    i = 0;
    if (msg != 0x346) {
        if (msg != 0x347) {
            return;
        }
        i = 1;
    }
    obj->x = D_WSTAG225_800A6640[i].x;
    obj->y = D_WSTAG225_800A6640[i].y;
    obj->base.set_step(obj, 1);
}

WstagSpriteAnimObject *wstag225_sprite_anim_create(s32 arg0) {
    return object_create(wstag225_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0, arg0);
}

WstagSpriteAnimObject *wstag225_sprite_anim_new(void) {
    return object_new(wstag225_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0);
}

void wstag225_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (gamestate_data.progress == 0x1) {
            data->event = fieldstg_event_start(0x5);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag225_start(void *arg0) {
    WstagObject *obj = object_new(wstag225_update, sizeof(WstagObject), sizeof(FieldstgEvent *));

    obj->manager = arg0;
    wstag225_funcs.setup();
    return obj;
}

void wstag225_event_5_end(void) {
    gamestate_data.progress = 0x2;
}

void wstag225_setup(void) {
    fieldstg_stage.background_file = 0x19E;
    fieldstg_stage.sprite_file = 0x019F0000;
    fieldstg_stage.sprites = wstag225_sprites;
    fieldstg_stage.map_events = wstag225_map_events;
    fieldstg_stage.mask_file = 0x3CD;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0xFD00, 0x17700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag225_vram_places;
    fieldstg_stage.music = 0x33;
    fieldstg_stage.sound = 0x60CC0000;
    fieldstg_stage.actors = wstag225_actors;
    fieldstg_stage.events = wstag225_events;
    fieldstg_attr.set_file(0, 0x019F0001);
    fieldstg_attr.set_file(7, 0x019F0002);
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
void wstag225_setup(void);

s16 D_WSTAG225_800A6460[147] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 286, 295),
    FIELDSTG_EVENT_PLACE(1, 0, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 0),
    FIELDSTG_EVENT_PLACE(13, 271, 401),
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_ANIM(0x32F, 837, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32F, 838, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_PLACE(1, 368, 287),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 336, 304, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 58, 1),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 304, 352, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_WALK(1, 240, 384, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 13, 2),
    FIELDSTG_EVENT_ANIM(13, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(13, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 1),
    FIELDSTG_EVENT_ANIM(1, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(13, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 80, 464, 7),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x203, 730, 382, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG225_800A6588[5] = { { 70, 4 }, { 71, 4 }, { 70, 4 }, { 71, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG225_800A659C[20] = {
    { 300, 56 }, { 85, 6 }, { 86, 6 }, { 87, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 },
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG225_800A65EC[16] = {
    { 300, 16 }, { 72, 4 }, { 73, 4 }, { 74, 4 }, { 73, 4 }, { 75, 6 }, { 76, 6 }, { 77, 14 }, { 78, 114 },
    { 79, 6 }, { 80, 6 }, { 81, 6 }, { 82, 6 }, { 83, 6 }, { 84, 8 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG225_800A662C[3] = { D_WSTAG225_800A6588, D_WSTAG225_800A65EC, D_WSTAG225_800A659C };
s8 D_WSTAG225_800A6638[4] = { 30, 30, 30, 0 };
s8 D_WSTAG225_800A663C[4] = { 0, -47, -47, 0 };
WstagPos D_WSTAG225_800A6640[2] = { { 336, 274 }, { 112, 274 } };
FieldstgVramPlace wstag225_vram_places[11] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 338, 464, 72, 208, 368, 510 }, { 320, 256, 368, 432, 192, 176, 352, 509 },
    { 320, 256, 330, 440, 40, 184, 368, 509 }, { 320, 256, 354, 464, 136, 208, 336, 508 },
    { 320, 256, 346, 464, 104, 208, 352, 508 },
};
FieldstgTalk D_WSTAG225_800A66F8[2] = { { NULL, NULL, 28 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6710[2] = { { NULL, NULL, 410 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6728[2] = { { NULL, NULL, 411 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6740[2] = { { NULL, NULL, 413 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6758[2] = { { NULL, NULL, 417 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6770[2] = { { NULL, NULL, 418 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6788[2] = { { NULL, NULL, 412 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A67A0[2] = { { NULL, NULL, 414 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A67B8[2] = { { NULL, NULL, 415 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A67D0[2] = { { NULL, NULL, 419 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A67E8[2] = { { NULL, NULL, 433 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6800[2] = { { NULL, NULL, 416 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG225_800A6818[2] = { { NULL, NULL, 432 }, { NULL, NULL, 0 } };
u16 D_WSTAG225_800A6830[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6838[4] = { 0x6001, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6840[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6848[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6850[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6858[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6860[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6868[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6870[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6878[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6880[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6888[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6890[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A6898[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A68A0[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG225_800A68A8[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG225_800A68B0 = { D_WSTAG225_800A6830, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG225_800A68C4 = { D_WSTAG225_800A6838, NULL, 13, 5, 0, 0, 3 };
FieldstgPlacedActor D_WSTAG225_800A68D8 = { D_WSTAG225_800A6840, D_WSTAG225_800A66F8, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A68EC = { D_WSTAG225_800A6848, D_WSTAG225_800A6710, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6900 = { D_WSTAG225_800A6850, D_WSTAG225_800A6728, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6914 = { D_WSTAG225_800A6858, D_WSTAG225_800A6740, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6928 = { D_WSTAG225_800A6860, D_WSTAG225_800A6758, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A693C = { D_WSTAG225_800A6868, D_WSTAG225_800A6770, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6950 = { D_WSTAG225_800A6870, D_WSTAG225_800A6788, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6964 = { D_WSTAG225_800A6878, D_WSTAG225_800A67A0, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A6978 = { D_WSTAG225_800A6880, D_WSTAG225_800A67B8, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A698C = { D_WSTAG225_800A6888, D_WSTAG225_800A67D0, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A69A0 = { D_WSTAG225_800A6890, D_WSTAG225_800A67E8, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A69B4 = { D_WSTAG225_800A6898, D_WSTAG225_800A6800, 32, 6, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG225_800A69C8 = { D_WSTAG225_800A68A0, NULL, 106, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG225_800A69DC = { D_WSTAG225_800A68A8, D_WSTAG225_800A6818, 157, 8, 265, 397, 3 };
FieldstgPlacedActor *wstag225_actors[17] = {
    &D_WSTAG225_800A68B0, &D_WSTAG225_800A68C4, &D_WSTAG225_800A68D8, &D_WSTAG225_800A68EC, &D_WSTAG225_800A6900,
    &D_WSTAG225_800A6914, &D_WSTAG225_800A6928, &D_WSTAG225_800A693C, &D_WSTAG225_800A6950, &D_WSTAG225_800A6964,
    &D_WSTAG225_800A6978, &D_WSTAG225_800A698C, &D_WSTAG225_800A69A0, &D_WSTAG225_800A69B4, &D_WSTAG225_800A69C8,
    &D_WSTAG225_800A69DC, NULL,
};
FieldstgSprite wstag225_sprites[12] = {
    { 0, 1, 0x50, 6, 0x46, 0, 0, 0, 0, 0, 112, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 116, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 228, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 340, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 355, 346, 0, 0 },
    { 0, 2, 0x50, 4, 0x48, 0, 0, 0, 0, 0, 112, 227, 301, 0 },
    { 0, 3, 0x50, 4, 0x55, 0, 0, 0, 0, 0, 112, 227, 294, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 116, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 228, 298, 324, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 340, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 355, 362, 388, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag225_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x203, 0x2DA, 0x17E, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag225_funcs = { wstag225_setup };
FieldstgEventDef wstag225_events[2] = {
    { 5, D_WSTAG225_800A6460, 0x01120000, NULL, wstag225_event_5_end }, { -1, NULL, 0, NULL, NULL },
};
