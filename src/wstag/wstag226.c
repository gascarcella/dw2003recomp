#include "wstag.h"

/* WSTAG226: stage 0x276 (fieldstg_stages). */

extern WstagAnimKey *D_WSTAG226_800A68C0[3];
extern s8 D_WSTAG226_800A68CC[];
extern s8 D_WSTAG226_800A68D0[];
extern WstagPos D_WSTAG226_800A68D4[];
extern WstagFuncs wstag226_funcs;
extern FieldstgVramPlace wstag226_vram_places[];
extern FieldstgPlacedActor *wstag226_actors[];
extern FieldstgSprite wstag226_sprites[];
extern FieldstgMapEvent wstag226_map_events[];
extern FieldstgEventDef wstag226_events[];
void wstag226_sprite_anim_update(WstagSpriteAnimObject *obj);
void wstag226_update(WstagObject *obj, WstagEventData *data);

s32 wstag226_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        wstag226_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag226_sprite_anim_reset(WstagSpriteAnimObject *obj) {
    s32 i;

    for (i = 0; i < 3; i++) {
        obj->sprites[i].anim.key = 0;
        obj->sprites[i].anim.time = D_WSTAG226_800A68C0[i]->time;
    }
}

void wstag226_sprite_anim_update(WstagSpriteAnimObject *obj) {
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
        wstag226_sprite_anim_reset(obj);
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
                        list->y = obj->y + D_WSTAG226_800A68D0[n];
                        list->priority = obj->y + D_WSTAG226_800A68CC[n];
                        n++;
                    }
                }
                wstag226_sprite_anim_reset(obj);
                obj->base.next_substep(obj);
            case 1:
                done = 0;
                for (i = 0; i < 3; i++) {
                    sprite = obj->sprites[i].sprite;
                    frame = wstag226_sprite_anim_play_once(&obj->sprites[i], D_WSTAG226_800A68C0[i], 0);
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
                wstag226_sprite_anim_reset(obj);
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

void wstag226_sprite_anim_message(WstagSpriteAnimObject *obj, s32 which) {
    s32 i;

    if (obj == NULL) {
        return;
    }
    i = 0;
    if (which != 0) {
        if (which != 1) {
            return;
        }
        i = 1;
    }
    obj->x = D_WSTAG226_800A68D4[i].x;
    obj->y = D_WSTAG226_800A68D4[i].y;
    obj->base.set_step(obj, 1);
}

WstagSpriteAnimObject *wstag226_sprite_anim_create(s32 arg0) {
    return object_create(wstag226_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0, arg0);
}

WstagSpriteAnimObject *wstag226_sprite_anim_new(void) {
    return object_new(wstag226_sprite_anim_update, sizeof(WstagSpriteAnimObject), 0);
}

void wstag226_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->object = wstag226_sprite_anim_new();
        /* Evidence (class B, register priority only; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_data.progress == 0x25 && gamestate_flags.get_flag(0x4060, 1)) {
            data->event = fieldstg_event_start(0x3A7);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag226_start(void *arg0) {
    WstagObject *obj = object_new(wstag226_update, sizeof(WstagObject), sizeof(WstagEventData));

    obj->manager = arg0;
    wstag226_funcs.setup();
    return obj;
}

void wstag226_event_935_end(void) {
    gamestate_data.progress = 0x26;
}

void wstag226_setup(void) {
    fieldstg_stage.background_file = 0x4B7;
    fieldstg_stage.sprite_file = 0x04B80000;
    fieldstg_stage.sprites = wstag226_sprites;
    fieldstg_stage.map_events = wstag226_map_events;
    fieldstg_stage.mask_file = 0x4B6;
    fieldstg_stage.talk_file = records_language + 0xDA;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10100, 0x16D00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag226_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag226_actors;
    fieldstg_stage.events = wstag226_events;
    fieldstg_attr.set_file(0, 0x04B80001);
    fieldstg_attr.set_file(7, 0x04B80002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress != 0x26 || gamestate_flags.get_flag(0x1A0A, 0) != 0) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag226_setup(void);

s16 D_WSTAG226_800A6474[467] = {
    FIELDSTG_EVENT_PLACE(1, 88, 460),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_PLACE(157, 265, 397),
    FIELDSTG_EVENT_ANIM(157, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 120, 444, 5),
    FIELDSTG_EVENT_PLACE(12, 88, 460),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 160, 424, 5),
    FIELDSTG_EVENT_WALK(12, 128, 440, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x324, 805, 12),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_ANIM(0x324, 806, 12),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 240, 384, 5),
    FIELDSTG_EVENT_WALK(12, 208, 400, 5),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(1, 58, 7),
    FIELDSTG_EVENT_WALK(12, 208, 376, 4),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(1, 47, 7),
    FIELDSTG_EVENT_WALK(12, 254, 353, 5),
    FIELDSTG_EVENT_ANIM(0x323, 807, 1),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(12, 58, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 3),
    FIELDSTG_EVENT_ANIM(12, 58, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 12),
    FIELDSTG_EVENT_DIALOG(0, 2, 12, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 4),
    FIELDSTG_EVENT_ANIM(12, 7, 0),
    FIELDSTG_EVENT_ANIM(157, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_PLACE(11, 88, 460),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(11, 120, 444, 5),
    FIELDSTG_EVENT_PLACE(317, 88, 460),
    FIELDSTG_EVENT_ANIM(317, 1, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_WALK(11, 208, 400, 5),
    FIELDSTG_EVENT_ANIM(157, 1, 2),
    FIELDSTG_EVENT_WALK(317, 176, 416, 5),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(11, 1, 5),
    FIELDSTG_EVENT_ANIM(317, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 11),
    FIELDSTG_EVENT_DIALOG(0, 11, 11, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 5),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x324, 805, 12),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(157, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_ANIM(0x324, 806, 12),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 234, 373),
    FIELDSTG_EVENT_DIALOG(1, 3, 1, 3),
    FIELDSTG_EVENT_DIALOG(0, 4, 12, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WALK(11, 208, 354, 4),
    FIELDSTG_EVENT_ANIM(157, 1, 3),
    FIELDSTG_EVENT_WALK(317, 208, 400, 5),
    FIELDSTG_EVENT_WAIT_WALK(317),
    FIELDSTG_EVENT_ANIM(11, 1, 7),
    FIELDSTG_EVENT_WALK(317, 208, 384, 6),
    FIELDSTG_EVENT_WAIT_WALK(317),
    FIELDSTG_EVENT_DIALOG(0, 5, 11, 0),
    FIELDSTG_EVENT_ANIM(11, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 11, 0),
    FIELDSTG_EVENT_ANIM(11, 7, 6),
    FIELDSTG_EVENT_ANIM(157, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 11, 0),
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 317, 3),
    FIELDSTG_EVENT_ANIM(317, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 12, 2),
    FIELDSTG_EVENT_ANIM(1, 1, 4),
    FIELDSTG_EVENT_ANIM(12, 7, 1),
    FIELDSTG_EVENT_ANIM(157, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 7, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_ANIM(11, 1, 7),
    FIELDSTG_EVENT_ANIM(157, 1, 3),
    FIELDSTG_EVENT_ANIM(317, 1, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 88, 460, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 1),
    FIELDSTG_EVENT_ANIM(12, 1, 1),
    FIELDSTG_EVENT_ANIM(157, 1, 1),
    FIELDSTG_EVENT_ANIM(317, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_GOTO_MAP(0x270, 1000, 236, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG226_800A681C[5] = { { 70, 4 }, { 71, 4 }, { 70, 4 }, { 71, 4 }, { 255, 999 } };
WstagAnimKey D_WSTAG226_800A6830[20] = {
    { 300, 56 }, { 85, 6 }, { 86, 6 }, { 87, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 },
    { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 88, 4 }, { 89, 4 }, { 90, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG226_800A6880[16] = {
    { 300, 16 }, { 72, 4 }, { 73, 4 }, { 74, 4 }, { 73, 4 }, { 75, 6 }, { 76, 6 }, { 77, 14 }, { 78, 114 },
    { 79, 6 }, { 80, 6 }, { 81, 6 }, { 82, 6 }, { 83, 6 }, { 84, 8 }, { 255, 999 },
};
WstagAnimKey *D_WSTAG226_800A68C0[3] = { D_WSTAG226_800A681C, D_WSTAG226_800A6880, D_WSTAG226_800A6830 };
s8 D_WSTAG226_800A68CC[4] = { 30, 30, 30, 0 };
s8 D_WSTAG226_800A68D0[4] = { 0, -47, -47, 0 };
WstagPos D_WSTAG226_800A68D4[2] = { { 336, 274 }, { 112, 274 } };
FieldstgVramPlace wstag226_vram_places[14] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 352, 384, 128, 128, 368, 510 }, { 320, 256, 360, 384, 160, 128, 352, 509 },
    { 320, 256, 352, 352, 128, 96, 368, 509 }, { 320, 256, 338, 408, 72, 152, 336, 508 },
    { 320, 256, 346, 408, 104, 152, 352, 508 }, { 320, 256, 360, 352, 160, 96, 368, 508 },
    { 320, 256, 330, 400, 40, 144, 352, 507 }, { 320, 256, 330, 368, 40, 112, 368, 507 },
};
u16 D_WSTAG226_800A69BC[4] = { 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG226_800A69C4[4] = { 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A69CC[4] = { 0, 0, 0xFFFF, 0 };
u16 D_WSTAG226_800A69D4[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A69DC[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A69E4[4] = { 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG226_800A69EC[4] = { 0x1A0A, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG226_800A69F4[3] = {
    { D_WSTAG226_800A69BC, NULL, 436 }, { D_WSTAG226_800A69C4, NULL, 521 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG226_800A6A18[2] = { { NULL, NULL, 91 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG226_800A6A30[2] = { { NULL, NULL, 93 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG226_800A6A48[3] = {
    { D_WSTAG226_800A69CC, D_WSTAG226_800A69D4, 513 }, { D_WSTAG226_800A69DC, NULL, 514 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG226_800A6A6C[2] = { { NULL, NULL, 90 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG226_800A6A84[2] = { { NULL, NULL, 92 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG226_800A6A9C[3] = {
    { D_WSTAG226_800A69E4, NULL, 506 }, { D_WSTAG226_800A69EC, NULL, 520 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG226_800A6AC0[2] = { { NULL, NULL, 507 }, { NULL, NULL, 0 } };
u16 D_WSTAG226_800A6AD8[6] = { 0x6025, 1, 0x4060, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6AE4[6] = { 0x6025, 1, 0x4060, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6AF0[6] = { 0x6025, 1, 0x4060, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6AFC[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B04[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B10[6] = { 0x882B, 1, 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B1C[6] = { 0x6026, 1, 0x1A0A, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B28[6] = { 0x1A0A, 0, 0x701C, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B34[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B3C[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B44[6] = { 0x6026, 1, 0x1A0A, 0, 0xFFFF, 0 };
u16 D_WSTAG226_800A6B50[6] = { 0x6025, 1, 0x4060, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG226_800A6B5C = { D_WSTAG226_800A6AD8, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG226_800A6B70 = { D_WSTAG226_800A6AE4, NULL, 11, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG226_800A6B84 = { D_WSTAG226_800A6AF0, NULL, 12, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG226_800A6B98 = { D_WSTAG226_800A6AFC, D_WSTAG226_800A69F4, 12, 6, 208, 376, 7 };
FieldstgPlacedActor D_WSTAG226_800A6BAC = { D_WSTAG226_800A6B04, D_WSTAG226_800A6A18, 32, 7, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG226_800A6BC0 = { D_WSTAG226_800A6B10, D_WSTAG226_800A6A30, 32, 7, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG226_800A6BD4 = { D_WSTAG226_800A6B1C, D_WSTAG226_800A6A48, 104, 8, 255, 352, 7 };
FieldstgPlacedActor D_WSTAG226_800A6BE8 = { D_WSTAG226_800A6B28, D_WSTAG226_800A6A6C, 157, 9, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG226_800A6BFC = { D_WSTAG226_800A6B34, D_WSTAG226_800A6A84, 157, 9, 265, 397, 3 };
FieldstgPlacedActor D_WSTAG226_800A6C10 = { D_WSTAG226_800A6B3C, D_WSTAG226_800A6A9C, 178, 10, 277, 363, 3 };
FieldstgPlacedActor D_WSTAG226_800A6C24 = { D_WSTAG226_800A6B44, D_WSTAG226_800A6AC0, 317, 11, 255, 352, 7 };
FieldstgPlacedActor D_WSTAG226_800A6C38 = { D_WSTAG226_800A6B50, NULL, 317, 11, 0, 0, 1 };
FieldstgPlacedActor *wstag226_actors[13] = {
    &D_WSTAG226_800A6B5C, &D_WSTAG226_800A6B70, &D_WSTAG226_800A6B84, &D_WSTAG226_800A6B98, &D_WSTAG226_800A6BAC,
    &D_WSTAG226_800A6BC0, &D_WSTAG226_800A6BD4, &D_WSTAG226_800A6BE8, &D_WSTAG226_800A6BFC, &D_WSTAG226_800A6C10,
    &D_WSTAG226_800A6C24, &D_WSTAG226_800A6C38, NULL,
};
FieldstgSprite wstag226_sprites[12] = {
    { 0, 1, 0x50, 6, 0x46, 0, 0, 0, 0, 0, 336, 274, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 116, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 228, 282, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 340, 258, 0, 0 },
    { 1, 0, 0x40, 6, 0x3F, 1, 0x3F, 0x42, 6, 0, 355, 346, 0, 0 },
    { 0, 2, 0x50, 4, 0x48, 0, 2, 0, 0, 0, 336, 227, 301, 0 },
    { 0, 3, 0x50, 4, 0x55, 0, 0, 0, 0, 0, 336, 227, 294, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 116, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 228, 298, 324, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 340, 274, 300, 0 },
    { 1, 0, 0x40, 4, 0x3B, 1, 0x3B, 0x3E, 6, 0, 355, 362, 388, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag226_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x273, 0x2DA, 0x17E, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag226_funcs = { wstag226_setup };
FieldstgEventDef wstag226_events[2] = {
    { 935, D_WSTAG226_800A6474, 0x01120029, NULL, wstag226_event_935_end }, { -1, NULL, 0, NULL, NULL },
};
