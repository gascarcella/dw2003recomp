#include "wstag.h"

/* WSTAG545: stage 0x246 (fieldstg_stages). */

/* A mark over the sprite of type `type` at a place, shown (its animation played once) when Wstag545Board says so. */
typedef struct Wstag545Mark {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 x; /* x */
    /* 0x54 */ s32 y; /* y */
    /* 0x58 */ s32 type; /* the sprite's type */
    /* 0x5C */ WstagSpriteAnim sprite;
} Wstag545Mark; /* size 0x64 */

/* The sprite of type 1, which opens on message 0x35A (wstag545_lid_create). */
typedef struct Wstag545Lid {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagSpriteAnim sprite;
} Wstag545Lid; /* size 0x58 */

/* A board (the sprite of type 2) whose animations messages 0x35D-0x35F play, and which shows one of its five
 * marks on messages 0x360-0x364 (wstag545_board_create). */
typedef struct Wstag545Board {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mark; /* the mark to show (1-5), 0: none */
    /* 0x54 */ s32 silent; /* else a message plays sound 0x8004474A (cleared at the start, never set) */
    /* 0x58 */ WstagSpriteAnim sprite;
} Wstag545Board; /* size 0x60 */

typedef struct Wstag545BoardData {
    /* 0x00 */ Wstag545Mark *marks[5];
} Wstag545BoardData; /* size 0x14 */

/* A place of Wstag545Mark. */
typedef struct Wstag545Spawn {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
    /* 0x8 */ s32 type;
} Wstag545Spawn; /* size 0xC */

/* The data of the stage object (wstag545_update). */
typedef struct Wstag545Data {
    /* 0x0 */ Wstag545Lid *lid;
    /* 0x4 */ Wstag545Board *board;
    /* 0x8 */ FieldstgEvent *event;
} Wstag545Data; /* size 0xC */

extern WstagAnimKey D_WSTAG545_800A6D24[];
extern WstagAnimKey D_WSTAG545_800A6D5C[];
extern WstagAnimKey *D_WSTAG545_800A6DE8[];
extern WstagAnimKey *D_WSTAG545_800A6DEC[];
extern WstagAnimKey *D_WSTAG545_800A6DF0[];
extern Wstag545Spawn D_WSTAG545_800A6DF4[];
extern WstagFuncs wstag545_funcs;
extern FieldstgBattleLists wstag545_battle_lists;
extern FieldstgVramPlace wstag545_vram_places[];
extern FieldstgPlacedActor *wstag545_actors[];
extern FieldstgSprite wstag545_sprites[];
extern FieldstgMapEvent wstag545_map_events[];
extern FieldstgEventDef wstag545_events[];
void wstag545_lid_update(Wstag545Lid *obj);
void wstag545_update(WstagObject *obj, Wstag545Data *data);

s32 wstag545_sprite_anim_hold(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
            sa->anim.key--;
            key = &keys[sa->anim.key];
            sa->anim.time += key->time;
        }
        wstag545_sprite_anim_hold(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag545_mark_update(Wstag545Mark *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG545_800A6D24[0].time;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == obj->type) {
                obj->sprite.sprite = list;
                list->x = obj->x;
                list->y = obj->y;
            }
        }
        break;
    case OBJECT_STATE_RUN:
        obj->sprite.sprite->shown = 0;
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->sprite.anim.key = 0;
            obj->sprite.anim.time = D_WSTAG545_800A6D24[0].time;
            obj->base.set_step(obj, 1);
        }
        sprite = obj->sprite.sprite;
        sprite->shown = 1;
        sprite->sprite = wstag545_sprite_anim_hold(&obj->sprite, D_WSTAG545_800A6D24, 0);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Wstag545Mark *wstag545_mark_create(s32 type, s32 x, s32 y) {
    Wstag545Mark *obj = object_new(wstag545_mark_update, sizeof(Wstag545Mark), 0);

    obj->type = type;
    obj->x = x;
    obj->y = y;
    return obj;
}

s32 wstag545_sprite_anim_play_once(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
        wstag545_sprite_anim_play_once(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag545_lid_update(Wstag545Lid *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    FieldstgSprite *spr;
    s32 frame;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->sprite.anim.key = 0;
        obj->sprite.anim.time = D_WSTAG545_800A6D5C[0].time;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == 1) {
                obj->sprite.sprite = list;
            }
        }
        break;
    case OBJECT_STATE_RUN:
        spr = obj->sprite.sprite;
        spr->shown = 1;
        spr->sprite = 0x18;
        spr->frame = 9;
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->sprite.anim.key = 0;
            obj->sprite.anim.time = D_WSTAG545_800A6D5C[0].time;
            obj->base.set_step(obj, 1);
            sound_module.play(0x4C0002);
        }
        sprite = obj->sprite.sprite;
        frame = wstag545_sprite_anim_play_once(&obj->sprite, D_WSTAG545_800A6D5C, 0);
        if (frame == 0xFF) {
            obj->base.set_state(obj, OBJECT_STATE_RUN);
            sprite->frame = 9;
        } else {
            sprite->frame = frame;
        }
        sprite->shown = 1;
        sprite->sprite = 0x18;
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag545_lid_message(Object *obj, s32 id) {
    if (obj != NULL && id == 0x35A) {
        obj->set_state(obj, OBJECT_STATE_DONE);
    }
}

Wstag545Lid *wstag545_lid_create(s32 arg0) {
    return object_create(wstag545_lid_update, sizeof(Wstag545Lid), 0, arg0);
}

s32 wstag545_sprite_anim_advance(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 once, s32 depth) {
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
        if (once) {
            if (key->frame == 0xFF) {
                return 0xFF;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag545_sprite_anim_advance(sa, keys, once, depth + 1);
    }
    return key->frame;
}

void wstag545_board_update(Wstag545Board *obj, Wstag545BoardData *data) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 frame2;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            if (list->type == 2) {
                obj->sprite.anim.key = 0;
                obj->sprite.anim.time = D_WSTAG545_800A6DE8[0]->time;
                obj->sprite.sprite = list;
            }
        }
        for (i = 0; i < 5; i++) {
            data->marks[i] = wstag545_mark_create(D_WSTAG545_800A6DF4[i].type, D_WSTAG545_800A6DF4[i].x,
                                                    D_WSTAG545_800A6DF4[i].y);
        }
        obj->silent = 0;
        obj->mark = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite = obj->sprite.sprite;
        switch (obj->base.step) {
        case 0:
            sprite->shown = 0;
            break;
        case 1:
            if (obj->base.substep == 0) {
                obj->sprite.anim.key = 0;
                obj->sprite.anim.time = D_WSTAG545_800A6DE8[0]->time;
                obj->base.set_substep(obj, 1);
            }
            frame2 = wstag545_sprite_anim_advance(&obj->sprite, D_WSTAG545_800A6DE8[0], 1, 0);
            sprite->shown = 1;
            if (frame2 == 0xFF) {
                sprite->sprite = 8;
                obj->base.set_step(obj, 2);
            } else {
                sprite->sprite = frame2;
            }
            break;
        case 2:
        case 4:
            break;
        case 3:
            if (obj->base.substep == 0) {
                obj->sprite.anim.key = 0;
                obj->sprite.anim.time = D_WSTAG545_800A6DEC[0]->time;
                obj->base.set_substep(obj, 1);
            }
            frame = wstag545_sprite_anim_advance(&obj->sprite, D_WSTAG545_800A6DEC[0], 1, 0);
            sprite->shown = 1;
            if (frame == 0xFF) {
                sprite->sprite = 9;
                obj->base.set_step(obj, 4);
            } else {
                sprite->sprite = frame;
            }
            break;
        case 5:
            if (obj->base.substep == 0) {
                obj->sprite.anim.key = 0;
                obj->sprite.anim.time = D_WSTAG545_800A6DF0[0]->time;
                obj->base.set_substep(obj, 1);
                sound_module.play(0x4C0001);
            }
            frame = wstag545_sprite_anim_advance(&obj->sprite, D_WSTAG545_800A6DF0[0], 0, 0);
            sprite->shown = 1;
            sprite->sprite = frame;
            break;
        }
        if (obj->mark != 0) {
            data->marks[obj->mark - 1]->base.set_state(data->marks[obj->mark - 1], OBJECT_STATE_DONE);
            obj->mark = 0;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag545_board_message(void *arg0, s32 msg) {
    Wstag545Board *obj = arg0;

    if (obj != NULL) {
        if (obj->silent == 0) {
            sound_module.play(0x8004474A);
        }
        switch (msg) {
        case 0x35D:
            obj->base.set_step(obj, 1);
            break;
        case 0x35E:
            obj->base.set_step(obj, 3);
            break;
        case 0x35F:
            obj->base.set_step(obj, 5);
            break;
        case 0x360:
            obj->mark = 1;
            break;
        case 0x361:
            obj->mark = 2;
            break;
        case 0x362:
            obj->mark = 3;
            break;
        case 0x363:
            obj->mark = 4;
            break;
        case 0x364:
            obj->mark = 5;
            break;
        }
        obj->base.set_substep(obj, 0);
    }
}

Wstag545Board *wstag545_board_create(s32 arg0) {
    return object_create(wstag545_board_update, sizeof(Wstag545Board), sizeof(Wstag545BoardData), arg0);
}

void wstag545_update(WstagObject *obj, Wstag545Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        if (gamestate_flags.get_flag(0x4051, 0)) {
            data->lid = wstag545_lid_create(0x34F);
        }
        if (gamestate_data.progress == 0x1A && gamestate_flags.get_flag(0x4051, 1)) {
            data->event = fieldstg_event_start(0x2C7);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag545_start(void *arg0) {
    WstagObject *obj = object_new(wstag545_update, sizeof(WstagObject), sizeof(Wstag545Data));

    obj->manager = arg0;
    wstag545_funcs.setup();
    return obj;
}

void wstag545_event_710_end(void) {
    gamestate_flags.set_flag(0x4051, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag545_event_711_end(void) {
    gamestate_data.progress = 0x1B;
}

void wstag545_setup(void) {
    fieldstg_stage.background_file = 0x275;
    fieldstg_stage.sprite_file = 0x02760000;
    fieldstg_stage.sprites = wstag545_sprites;
    fieldstg_stage.map_events = wstag545_map_events;
    fieldstg_stage.mask_file = 0x3DF;
    fieldstg_stage.talk_file = records_language + 0xEF;
    fieldstg_stage.start_pos = (GamestatePos){ 0x18800, 0x24600 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag545_vram_places;
    fieldstg_stage.music = 0x13;
    fieldstg_stage.sound = 0x604C0000;
    fieldstg_stage.actors = wstag545_actors;
    fieldstg_stage.battle_lists = &wstag545_battle_lists;
    fieldstg_stage.events = wstag545_events;
    fieldstg_attr.set_file(0, 0x02760001);
    fieldstg_attr.set_file(7, 0x02760002);
    fieldstg_attr.set_file(4, 0x02760003);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag545_setup(void);

s16 D_WSTAG545_800A6B58[167] = {
    FIELDSTG_EVENT_WALK(2, 536, 452, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_ANIM(0x350, 860, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 616, 412, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 662, 388),
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 873, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x34F, 858, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(0x34F, 858, 2),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x350, 864, 2),
    FIELDSTG_EVENT_WAIT(72),
    FIELDSTG_EVENT_ANIM(0x350, 865, 2),
    FIELDSTG_EVENT_WAIT(36),
    FIELDSTG_EVENT_ANIM(0x350, 866, 2),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(0x350, 867, 2),
    FIELDSTG_EVENT_WAIT(48),
    FIELDSTG_EVENT_ANIM(0x350, 868, 2),
    FIELDSTG_EVENT_WAIT(96),
    FIELDSTG_EVENT_ANIM(0x350, 861, 2),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_ANIM(0x350, 862, 2),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_WAIT(42),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x350, 863, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG545_800A6CA8[62] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 666, 388),
    FIELDSTG_EVENT_PLACE(2, 616, 412),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 616, 360, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 680, 328, 5),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x245, 1800, 524, 3),
    FIELDSTG_EVENT_END,
};
WstagAnimKey D_WSTAG545_800A6D24[14] = {
    { 11, 4 }, { 12, 4 }, { 13, 7 }, { 14, 9 }, { 15, 10 }, { 16, 8 }, { 17, 4 }, { 18, 4 }, { 19, 4 }, { 20, 4 },
    { 21, 4 }, { 22, 4 }, { 23, 100 }, { 255, 0 },
};
WstagAnimKey D_WSTAG545_800A6D5C[19] = {
    { 9, 25 }, { 8, 4 }, { 7, 4 }, { 6, 4 }, { 5, 4 }, { 4, 4 }, { 3, 6 }, { 2, 7 }, { 1, 8 }, { 0, 8 }, { 1, 10 },
    { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 }, { 6, 10 }, { 7, 10 }, { 8, 10 }, { 255, 999 },
};
WstagAnimKey D_WSTAG545_800A6DA8[10] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 255, 999 },
};
WstagAnimKey D_WSTAG545_800A6DD0[3] = { { 10, 4 }, { 9, 10 }, { 255, 999 } };
WstagAnimKey D_WSTAG545_800A6DDC[3] = { { 10, 4 }, { 9, 4 }, { 255, 0 } };
WstagAnimKey *D_WSTAG545_800A6DE8[1] = { D_WSTAG545_800A6DA8 };
WstagAnimKey *D_WSTAG545_800A6DEC[1] = { D_WSTAG545_800A6DD0 };
WstagAnimKey *D_WSTAG545_800A6DF0[1] = { D_WSTAG545_800A6DDC };
Wstag545Spawn D_WSTAG545_800A6DF4[5] = {
    { 648, 471, 3 }, { 815, 368, 4 }, { 592, 287, 5 }, { 775, 457, 6 }, { 564, 369, 7 },
};
FieldstgListedBattle D_WSTAG545_800A6E30 = { 165, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E3C = { 165, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E48 = { 173, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E54 = { 173, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E60 = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E6C = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E78 = { 150, 10, 0x60080000 };
FieldstgListedBattle D_WSTAG545_800A6E84 = { 92, 10, 0x60080000 };
FieldstgBattleList D_WSTAG545_800A6E90 = {
    4,
    { &D_WSTAG545_800A6E30, &D_WSTAG545_800A6E3C, &D_WSTAG545_800A6E48, &D_WSTAG545_800A6E54, &D_WSTAG545_800A6E60,
        &D_WSTAG545_800A6E6C, &D_WSTAG545_800A6E78, &D_WSTAG545_800A6E84 },
};
FieldstgListedBattle D_WSTAG545_800A6EB4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6EC0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6ECC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6ED8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6EE4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6EF0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6EFC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F08 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG545_800A6F14 = {
    0,
    { &D_WSTAG545_800A6EB4, &D_WSTAG545_800A6EC0, &D_WSTAG545_800A6ECC, &D_WSTAG545_800A6ED8, &D_WSTAG545_800A6EE4,
        &D_WSTAG545_800A6EF0, &D_WSTAG545_800A6EFC, &D_WSTAG545_800A6F08 },
};
FieldstgListedBattle D_WSTAG545_800A6F38 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F44 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F50 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F5C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F68 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F74 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F80 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6F8C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG545_800A6F98 = {
    0,
    { &D_WSTAG545_800A6F38, &D_WSTAG545_800A6F44, &D_WSTAG545_800A6F50, &D_WSTAG545_800A6F5C, &D_WSTAG545_800A6F68,
        &D_WSTAG545_800A6F74, &D_WSTAG545_800A6F80, &D_WSTAG545_800A6F8C },
};
FieldstgListedBattle D_WSTAG545_800A6FBC = { 12, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG545_800A6FC8 = { 315, 10, 0x60880000 };
FieldstgListedBattle D_WSTAG545_800A6FD4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6FE0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6FEC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A6FF8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A7004 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG545_800A7010 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG545_800A701C = {
    0,
    { &D_WSTAG545_800A6FBC, &D_WSTAG545_800A6FC8, &D_WSTAG545_800A6FD4, &D_WSTAG545_800A6FE0, &D_WSTAG545_800A6FEC,
        &D_WSTAG545_800A6FF8, &D_WSTAG545_800A7004, &D_WSTAG545_800A7010 },
};
FieldstgBattleLists wstag545_battle_lists = {
    61, 0, 0, { &D_WSTAG545_800A6E90, &D_WSTAG545_800A6F14, &D_WSTAG545_800A6F98 }, &D_WSTAG545_800A701C,
};
FieldstgVramPlace wstag545_vram_places[8] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 424, 456, 416, 200, 336, 508 }, { 384, 256, 416, 456, 384, 200, 352, 508 },
};
u16 D_WSTAG545_800A70DC[8] = { 0x21B, 1, 0x8B09, 1, 0x7013, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG545_800A70EC[2] = { { NULL, D_WSTAG545_800A70DC, 612 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG545_800A7104[2] = { { NULL, NULL, 37 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG545_800A711C[2] = { { NULL, NULL, 300 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG545_800A7134[2] = { { NULL, NULL, 38 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG545_800A714C[2] = { { NULL, NULL, 272 }, { NULL, NULL, 0 } };
u16 D_WSTAG545_800A7164[4] = { 0x21B, 0, 0xFFFF, 0 };
u16 D_WSTAG545_800A716C[4] = { 0x601A, 1, 0xFFFF, 0 };
u16 D_WSTAG545_800A7174[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG545_800A717C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG545_800A7184[4] = { 0x6026, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG545_800A718C = { D_WSTAG545_800A7164, D_WSTAG545_800A70EC, 33, 4, 785, 697, 1 };
FieldstgPlacedActor D_WSTAG545_800A71A0 = { D_WSTAG545_800A716C, D_WSTAG545_800A7104, 69, 5, 559, 1145, 1 };
FieldstgPlacedActor D_WSTAG545_800A71B4 = { D_WSTAG545_800A7174, D_WSTAG545_800A711C, 69, 5, 559, 1145, 1 };
FieldstgPlacedActor D_WSTAG545_800A71C8 = { D_WSTAG545_800A717C, D_WSTAG545_800A7134, 69, 5, 559, 1145, 1 };
FieldstgPlacedActor D_WSTAG545_800A71DC = { D_WSTAG545_800A7184, D_WSTAG545_800A714C, 69, 5, 559, 1145, 1 };
FieldstgPlacedActor *wstag545_actors[6] = {
    &D_WSTAG545_800A718C, &D_WSTAG545_800A71A0, &D_WSTAG545_800A71B4, &D_WSTAG545_800A71C8, &D_WSTAG545_800A71DC,
    NULL,
};
FieldstgSprite wstag545_sprites[12] = {
    { 1, 0, 0x64, 2, 0x63, 0, 0, 0, 0, 0, 529, 465, 0, 0 }, { 1, 0, 0x64, 2, 0x61, 0, 0, 0, 0, 0, 427, 1040, 0, 0 },
    { 1, 0, 0x64, 2, 0x61, 0, 0, 0, 0, 0, 546, 1044, 0, 0 },
    { 1, 0, 0x64, 2, 0x62, 0, 0, 0, 0, 0, 502, 1051, 0, 0 }, { 0, 2, 0x80, 4, 0, 0, 0, 0, 0, 0, 701, 371, 371, 0 },
    { 0, 7, 0xF0, 4, 0xB, 0, 0, 0, 0, 0, 564, 369, 369, 0 },
    { 0, 5, 0xF0, 4, 0xB, 0, 0, 0, 0, 0, 592, 287, 287, 0 },
    { 0, 3, 0xF0, 4, 0xB, 0, 0, 0, 0, 0, 648, 471, 471, 0 },
    { 0, 6, 0xF0, 4, 0xB, 0, 0, 0, 0, 0, 775, 457, 457, 0 },
    { 0, 4, 0xF0, 4, 0xB, 0, 0, 0, 0, 0, 815, 368, 368, 0 },
    { 0, 1, 0x50, 4, 0x18, 0, 0, 0, 0, 0, 663, 340, 370, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag545_map_events[6] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x245, 0x708, 0x20C, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x245, 0x708, 0x2DC, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x245, 0x708, 0x3BC, 3, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x245, 0x708, 0x50E, 3, 0, 0, 0 },
    { 0x601A, 1, 0x4051, 0, 8, 0x2C6, 0, 0, 0, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag545_funcs = { wstag545_setup };
FieldstgEventDef wstag545_events[3] = {
    { 710, D_WSTAG545_800A6B58, 0x013C0021, NULL, wstag545_event_710_end },
    { 711, D_WSTAG545_800A6CA8, 0x013C001F, NULL, wstag545_event_711_end }, { -1, NULL, 0, NULL, NULL },
};
