#include "wstag.h"

/* WSTAG795: stage 0x2DA (fieldstg_stages). */

extern WstagFuncs wstag795_funcs;
extern s16 D_WSTAG795_800A6A7C[];
void wstag795_countdown_update(WstagObject *obj, WstagEventData *data);
/* The data of the stage object: one more object, the door and an event. */
typedef struct Wstag795Data {
    /* 0x0 */ void *countdown;
    /* 0x4 */ void *door;
    /* 0x8 */ FieldstgEvent *event;
} Wstag795Data; /* size 0xC */

void wstag795_update(WstagObject *obj, Wstag795Data *data);
WstagDoorObject *wstag795_door_new(void);
extern WstagAnimKeyB D_WSTAG795_800A6A1C[];
extern WstagAnimKeyB D_WSTAG795_800A6A28[];
extern WstagAnimKeyB D_WSTAG795_800A6A54[];
extern WstagAnimKeyB *D_WSTAG795_800A6A70[];
extern FieldstgBattleLists wstag795_battle_lists;
extern FieldstgVramPlace wstag795_vram_places[];
extern FieldstgPlacedActor *wstag795_actors[];
extern FieldstgSprite wstag795_sprites[];
extern FieldstgMapEvent wstag795_map_events[];
extern FieldstgEventDef wstag795_events[];
void wstag795_door_update(WstagDoorObject *obj);

s32 wstag795_sprite_anim_advance_b(WstagSpriteAnim *sa, WstagAnimKeyB *keys, s32 once, s32 depth) {
    WstagAnimKeyB *key = &keys[sa->anim.key];
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
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag795_sprite_anim_advance_b(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag795_door_update(WstagDoorObject *obj) {
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 i;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (sprite = fieldstg_stage.sprites; sprite->present != 0; sprite++) {
            switch (sprite->type) {
            case 1:
                obj->sprites[0].anim.key = 0;
                obj->sprites[0].anim.time = D_WSTAG795_800A6A70[0][0].time;
                obj->sprites[0].sprite = sprite;
                break;
            case 2:
                obj->sprites[1].anim.key = 0;
                obj->sprites[1].anim.time = D_WSTAG795_800A6A70[2][0].time;
                obj->sprites[1].sprite = sprite;
                break;
            }
        }
        obj->opening = 0;
        if (obj->starts_open == 0) {
            obj->base.next_state(obj);
        } else {
            obj->sprites[0].anim.key = 0;
            obj->sprites[0].anim.time = D_WSTAG795_800A6A28[0].time;
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            spr = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                spr->shown = 1;
                spr->sprite = 0x46;
                spr->frame = wstag795_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG795_800A6A1C, 0, 0);
                break;
            case 1:
                spr->shown = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
        for (i = 0; i < 2; i++) {
            spr = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                spr->shown = 1;
                if (obj->opening != 0) {
                    frame = wstag795_sprite_anim_advance_b(&obj->sprites[0], D_WSTAG795_800A6A28, 1, 0);
                    if (frame == 0xFF) {
                        spr->sprite = 0x50;
                        obj->opening = 0;
                    } else {
                        spr->sprite = frame;
                    }
                } else {
                    spr->sprite = 0x50;
                }
                spr->frame = 0;
                break;
            case 1:
                spr->shown = 1;
                spr->sprite = 0x51;
                spr->frame = wstag795_sprite_anim_advance_b(&obj->sprites[1], D_WSTAG795_800A6A54, 0, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag795_door_message(WstagDoorObject *obj, s32 msg) {
    if (obj != NULL && msg == 0x35B) {
        obj->opening = 1;
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG795_800A6A28[0].time;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}


Object *wstag795_door_create(s32 arg0) {
    return object_create(wstag795_door_update, sizeof(WstagDoorObject), 0, arg0);
}

WstagDoorObject *wstag795_door_new(void) {
    WstagDoorObject *obj = object_new(wstag795_door_update, sizeof(WstagDoorObject), 0);

    obj->starts_open = 1;
    return obj;
}

/* Draws the countdown (gamestate_data.countdown): a frame and its three digits. */
void wstag795_countdown_draw(WstagObject *obj) {
    Sprite spr;
    GamestatePos pos;
    GfxLayer *layer;
    s32 i;

    layer = gfx_module.funcs.get_layer(0x1002);
    i = 0;
    layer->get_scroll(layer, &pos.x);
    sprite_init(&spr);
    spr.set_layer(layer, 0);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.draw(cdload_module.get_subfile_by_id(0x06F60000), 1, pos.x + 0xE0, pos.y + 0x16);
    pos.y += 0x19;
    for (; i < 3; i++) {
        spr.draw(cdload_module.get_subfile_by_id(0x06F60000), gamestate_data.countdown[i] + 2, pos.x + D_WSTAG795_800A6A7C[i],
                   pos.y);
    }
}

void wstag795_countdown_update(WstagObject *obj, WstagEventData *data) {
    u8 *timer;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class A1, sched1 barrier; DECISIONS "LOOP_BLOCK audit"): the original's temporaries in the digit
         * carries come from sched1's order (without sched1 both forms are the same). */
        LOOP_BLOCK(if (gamestate_flags.get_flag(0x4043, 0) == 0 && fieldstg_stage.event_running == 0
            && fieldstg_stage.battle_starting == 0 && fieldstg_stage.title_shown == 0 && fieldstg_stage.menu_open == 0
            && fieldstg_stage.actor_busy == 0) {
            wstag795_countdown_draw(obj);
            timer = gamestate_data.countdown;
            if (timer[0] != 0 || timer[1] != 0 || timer[2] != 0) {
                timer[3] -= gfx_module.funcs.get_frame_ticks();
                if (timer[3] > 60) {
                    timer[2]--;
                    timer[3] += 60;
                    sound_module.play(0x800452C6);
                }
                if (timer[2] >= 10) {
                    timer[2] = 9;
                    timer[1]--;
                }
                if (timer[1] >= 10) {
                    timer[1] = 9;
                    timer[0]--;
                }
            } else {
                data->event = fieldstg_event_start(0x5E1);
                obj->base.next_state(obj);
            }
        });
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}


Object *wstag795_countdown_new(void) {
    return object_new(wstag795_countdown_update, sizeof(WstagObject), sizeof(FieldstgEvent *)); /* data: the event only */
}

void wstag795_update(WstagObject *obj, Wstag795Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->countdown = wstag795_countdown_new();
        if (gamestate_flags.get_flag(0x4063, 0)) {
            data->door = wstag795_door_create(0x344);
        } else {
            data->door = wstag795_door_new();
        }
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4043, 0) && gamestate_data.progress == 0x20) {
            data->event = fieldstg_event_start(0x33E);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag795_start(void *arg0) {
    WstagObject *obj = object_new(wstag795_update, sizeof(WstagObject), sizeof(Wstag795Data));

    obj->manager = arg0;
    wstag795_funcs.setup();
    return obj;
}

void wstag795_event_830_end(void) {
    gamestate_flags.set_flag(0x4043, 1);
}

void wstag795_event_840_end(void) {
    gamestate_flags.set_flag(0x1C05, 1);
    gamestate_flags.set_flag(0x4063, 1);
}

void wstag795_setup(void) {
    fieldstg_stage.background_file = 0x721;
    fieldstg_stage.sprite_file = 0x06F60000;
    fieldstg_stage.sprites = wstag795_sprites;
    fieldstg_stage.map_events = wstag795_map_events;
    fieldstg_stage.mask_file = 0x6F2;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x4B200, 0x3AA00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag795_vram_places;
    fieldstg_stage.music = 0xD;
    fieldstg_stage.sound = 0x60340000;
    fieldstg_stage.actors = wstag795_actors;
    fieldstg_stage.battle_lists = &wstag795_battle_lists;
    fieldstg_stage.events = wstag795_events;
    fieldstg_attr.set_file(0, 0x06F60001);
    fieldstg_attr.set_file(7, 0x06F60002);
    fieldstg_attr.set_file(4, 0x06F60003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag795_setup(void);

s16 D_WSTAG795_800A6844[63] = {
    FIELDSTG_EVENT_PLACE(2, 1119, 893),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG795_800A68C4[129] = {
    FIELDSTG_EVENT_WALK(2, 449, 903, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 887, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x344, 859, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 895, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 7, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 416, 889, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG795_800A69C8[42] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_ANIM(2, 7, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_GOTO_MAP(0x26D, 1, 1, 1),
    FIELDSTG_EVENT_END,
};
WstagAnimKeyB D_WSTAG795_800A6A1C[3] = { { 0, 8, 0 }, { 1, 8, 0 }, { 255, 0, 0 } };
WstagAnimKeyB D_WSTAG795_800A6A28[11] = {
    { 71, 4, 0 }, { 72, 4, 0 }, { 73, 4, 0 }, { 74, 4, 0 }, { 75, 4, 0 }, { 76, 4, 0 }, { 77, 4, 0 }, { 78, 4, 0 },
    { 79, 4, 0 }, { 80, 4, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG795_800A6A54[7] = {
    { 0, 6, 0 }, { 1, 6, 0 }, { 2, 6, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB *D_WSTAG795_800A6A70[3] = { D_WSTAG795_800A6A1C, D_WSTAG795_800A6A28, D_WSTAG795_800A6A54 };
s16 D_WSTAG795_800A6A7C[4] = { 226, 253, 280, 0 };
FieldstgListedBattle D_WSTAG795_800A6A84 = { 124, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6A90 = { 124, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6A9C = { 124, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6AA8 = { 125, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6AB4 = { 125, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6AC0 = { 125, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6ACC = { 123, 16, 0x60080000 };
FieldstgListedBattle D_WSTAG795_800A6AD8 = { 123, 16, 0x60080000 };
FieldstgBattleList D_WSTAG795_800A6AE4 = {
    3,
    { &D_WSTAG795_800A6A84, &D_WSTAG795_800A6A90, &D_WSTAG795_800A6A9C, &D_WSTAG795_800A6AA8, &D_WSTAG795_800A6AB4,
        &D_WSTAG795_800A6AC0, &D_WSTAG795_800A6ACC, &D_WSTAG795_800A6AD8 },
};
FieldstgListedBattle D_WSTAG795_800A6B08 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B14 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B20 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B2C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B5C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG795_800A6B68 = {
    0,
    { &D_WSTAG795_800A6B08, &D_WSTAG795_800A6B14, &D_WSTAG795_800A6B20, &D_WSTAG795_800A6B2C, &D_WSTAG795_800A6B38,
        &D_WSTAG795_800A6B44, &D_WSTAG795_800A6B50, &D_WSTAG795_800A6B5C },
};
FieldstgListedBattle D_WSTAG795_800A6B8C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6B98 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BA4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BB0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BBC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BC8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6BE0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG795_800A6BEC = {
    0,
    { &D_WSTAG795_800A6B8C, &D_WSTAG795_800A6B98, &D_WSTAG795_800A6BA4, &D_WSTAG795_800A6BB0, &D_WSTAG795_800A6BBC,
        &D_WSTAG795_800A6BC8, &D_WSTAG795_800A6BD4, &D_WSTAG795_800A6BE0 },
};
FieldstgListedBattle D_WSTAG795_800A6C10 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C1C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C28 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C34 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C40 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C4C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C58 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG795_800A6C64 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG795_800A6C70 = {
    0,
    { &D_WSTAG795_800A6C10, &D_WSTAG795_800A6C1C, &D_WSTAG795_800A6C28, &D_WSTAG795_800A6C34, &D_WSTAG795_800A6C40,
        &D_WSTAG795_800A6C4C, &D_WSTAG795_800A6C58, &D_WSTAG795_800A6C64 },
};
FieldstgBattleLists wstag795_battle_lists = {
    90, 0, 0, { &D_WSTAG795_800A6AE4, &D_WSTAG795_800A6B68, &D_WSTAG795_800A6BEC }, &D_WSTAG795_800A6C70,
};
FieldstgVramPlace wstag795_vram_places[18] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 332, 410, 48, 154, 320, 505 }, { 320, 256, 320, 418, 0, 162, 336, 505 },
    { 320, 256, 338, 441, 72, 185, 352, 505 }, { 320, 256, 344, 441, 96, 185, 368, 505 },
    { 320, 256, 350, 441, 120, 185, 320, 504 }, { 320, 256, 356, 441, 144, 185, 336, 504 },
    { 320, 256, 326, 442, 24, 186, 352, 504 }, { 320, 256, 332, 442, 48, 186, 368, 504 },
    { 320, 256, 362, 448, 168, 192, 320, 503 }, { 320, 256, 368, 448, 192, 192, 336, 503 },
    { 320, 256, 374, 448, 216, 192, 352, 503 }, { 320, 256, 320, 450, 0, 194, 368, 503 },
};
u16 D_WSTAG795_800A6DD0[6] = { 0x239, 1, 0x84D5, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6DDC[4] = { 0x23A, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6DE4[6] = { 0x23B, 1, 0x822B, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6DF0[4] = { 0x23C, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6DF8[4] = { 0x23D, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E00[6] = { 0x23E, 1, 0x88A5, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E0C[6] = { 0x23F, 1, 0x84AE, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E18[4] = { 0x240, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E20[6] = { 0x241, 1, 0x822C, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E2C[6] = { 0x242, 1, 0x8464, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E38[6] = { 0x243, 1, 0x8497, 1, 0xFFFF, 0 };
u16 D_WSTAG795_800A6E44[6] = { 0x244, 1, 0x8B22, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG795_800A6E50[2] = { { NULL, D_WSTAG795_800A6DD0, 513 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6E68[2] = { { NULL, D_WSTAG795_800A6DDC, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6E80[2] = { { NULL, D_WSTAG795_800A6DE4, 382 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6E98[2] = { { NULL, D_WSTAG795_800A6DF0, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6EB0[2] = { { NULL, D_WSTAG795_800A6DF8, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6EC8[2] = { { NULL, D_WSTAG795_800A6E00, 498 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6EE0[2] = { { NULL, D_WSTAG795_800A6E0C, 511 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6EF8[2] = { { NULL, D_WSTAG795_800A6E18, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6F10[2] = { { NULL, D_WSTAG795_800A6E20, 381 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6F28[2] = { { NULL, D_WSTAG795_800A6E2C, 489 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6F40[2] = { { NULL, D_WSTAG795_800A6E38, 497 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG795_800A6F58[2] = { { NULL, D_WSTAG795_800A6E44, 399 }, { NULL, NULL, 0 } };
u16 D_WSTAG795_800A6F70[4] = { 0x239, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6F78[4] = { 0x23A, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6F80[4] = { 0x23B, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6F88[4] = { 0x23C, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6F90[4] = { 0x23D, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6F98[4] = { 0x23E, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FA0[4] = { 0x23F, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FA8[4] = { 0x240, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FB0[4] = { 0x241, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FB8[4] = { 0x242, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FC0[4] = { 0x243, 0, 0xFFFF, 0 };
u16 D_WSTAG795_800A6FC8[4] = { 0x244, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG795_800A6FD0 = { D_WSTAG795_800A6F70, D_WSTAG795_800A6E50, 33, 4, 961, 678, 1 };
FieldstgPlacedActor D_WSTAG795_800A6FE4 = { D_WSTAG795_800A6F78, D_WSTAG795_800A6E68, 77, 5, 737, 855, 1 };
FieldstgPlacedActor D_WSTAG795_800A6FF8 = { D_WSTAG795_800A6F80, D_WSTAG795_800A6E80, 78, 6, 640, 903, 1 };
FieldstgPlacedActor D_WSTAG795_800A700C = { D_WSTAG795_800A6F88, D_WSTAG795_800A6E98, 79, 7, 545, 950, 1 };
FieldstgPlacedActor D_WSTAG795_800A7020 = { D_WSTAG795_800A6F90, D_WSTAG795_800A6EB0, 80, 8, 768, 998, 1 };
FieldstgPlacedActor D_WSTAG795_800A7034 = { D_WSTAG795_800A6F98, D_WSTAG795_800A6EC8, 81, 9, 400, 558, 1 };
FieldstgPlacedActor D_WSTAG795_800A7048 = { D_WSTAG795_800A6FA0, D_WSTAG795_800A6EE0, 82, 10, 784, 366, 1 };
FieldstgPlacedActor D_WSTAG795_800A705C = { D_WSTAG795_800A6FA8, D_WSTAG795_800A6EF8, 83, 11, 432, 478, 1 };
FieldstgPlacedActor D_WSTAG795_800A7070 = { D_WSTAG795_800A6FB0, D_WSTAG795_800A6F10, 84, 12, 672, 1047, 1 };
FieldstgPlacedActor D_WSTAG795_800A7084 = { D_WSTAG795_800A6FB8, D_WSTAG795_800A6F28, 85, 13, 335, 527, 1 };
FieldstgPlacedActor D_WSTAG795_800A7098 = { D_WSTAG795_800A6FC0, D_WSTAG795_800A6F40, 86, 14, 160, 823, 1 };
FieldstgPlacedActor D_WSTAG795_800A70AC = { D_WSTAG795_800A6FC8, D_WSTAG795_800A6F58, 87, 15, 205, 526, 1 };
FieldstgPlacedActor *wstag795_actors[13] = {
    &D_WSTAG795_800A6FD0, &D_WSTAG795_800A6FE4, &D_WSTAG795_800A6FF8, &D_WSTAG795_800A700C, &D_WSTAG795_800A7020,
    &D_WSTAG795_800A7034, &D_WSTAG795_800A7048, &D_WSTAG795_800A705C, &D_WSTAG795_800A7070, &D_WSTAG795_800A7084,
    &D_WSTAG795_800A7098, &D_WSTAG795_800A70AC, NULL,
};
FieldstgSprite wstag795_sprites[22] = {
    { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 153, 151, 0, 0 }, { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 166, 158, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 185, 167, 0, 0 }, { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 198, 174, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 217, 183, 0, 0 }, { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 230, 190, 0, 0 },
    { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 249, 200, 0, 0 }, { 1, 0, 0x40, 2, 0x38, 2, 0, 5, 4, 0, 262, 206, 0, 0 },
    { 1, 2, 0x64, 6, 0x51, 0, 0, 0, 0, 0, 414, 834, 0, 0 }, { 1, 1, 0x64, 6, 0x46, 0, 0, 0, 0, 0, 414, 834, 0, 0 },
    { 1, 0, 0x40, 6, 0x37, 2, 0, 5, 6, 0, 1117, 845, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x36, 6, 0, 1121, 849, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 226, 115, 0, 0 }, { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 239, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 258, 131, 0, 0 }, { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 271, 137, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 289, 147, 0, 0 }, { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 303, 154, 0, 0 },
    { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 322, 163, 0, 0 }, { 1, 0, 0x40, 6, 0x38, 2, 0, 5, 4, 0, 335, 170, 0, 0 },
    { 1, 0x64, 0x7A, 6, 0, 0, 0, 0, 0, 0, 142, 67, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag795_map_events[7] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DB, 0x208, 0xCC, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DB, 0x168, 0x11C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DB, 0x128, 0x19C, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x2DB, 0x128, 0x35C, 7, 0, 0, 0 },
    { 0x1C0A, 1, 0xFFFF, 0, 1, 0x2DC, 0x2B4, 0x198, 3, 0x64, 0, 0 },
    { 0x6020, 1, 0x4063, 0, 8, 0x348, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag795_funcs = { wstag795_setup };
FieldstgEventDef wstag795_events[4] = {
    { 830, D_WSTAG795_800A6844, 0x014A001A, NULL, wstag795_event_830_end },
    { 840, D_WSTAG795_800A68C4, 0x014A001B, NULL, wstag795_event_840_end },
    { 1505, D_WSTAG795_800A69C8, 0x014A0025, NULL, NULL }, { -1, NULL, 0, NULL, NULL },
};
