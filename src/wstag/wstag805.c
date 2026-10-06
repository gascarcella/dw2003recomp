#include "wstag.h"

/* WSTAG805: stage 0x2DC (fieldstg_stages). */

/* A key of Wstag805Part's animations: a frame, its time and a move added each frame (played once). */
typedef struct Wstag805Key {
    /* 0x0 */ s16 frame;
    /* 0x2 */ s16 time;
    /* 0x4 */ s32 delta;
} Wstag805Key; /* size 0x8 */

/* One of Wstag805Drop's three animations. */
typedef struct Wstag805Part {
    /* 0x0 */ s32 pos;   /* the moves of the keys, summed */
    /* 0x4 */ s16 frame; /* 0: hidden */
    /* 0x6 */ s16 palette; /* the sprite's palette (Sprite.set_palette) */
    /* 0x8 */ WstagAnim anim;
} Wstag805Part; /* size 0xC */

/* A drop that waits at its place, then falls (up or down: up) when Wstag805Rain lets it go (state 2). */
typedef struct Wstag805Drop {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 x; /* x */
    /* 0x54 */ s32 y; /* y */
    /* 0x58 */ s32 pos_y; /* y, 24.8 */
    /* 0x5C */ s32 up; /* moves up */
    /* 0x60 */ Wstag805Part parts[3];
} Wstag805Drop; /* size 0x84 */

/* A place of Wstag805Drop. */
typedef struct Wstag805Spawn {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
    /* 0x8 */ s32 dir;
} Wstag805Spawn; /* size 0xC */

/* The object that makes the 27 drops fall one by one at the times of D_WSTAG805_800A71E0 (message 0x335). */
typedef struct Wstag805Rain {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 next; /* the next drop */
    /* 0x52 */ s16 timer; /* timer */
} Wstag805Rain; /* size 0x54 */

typedef struct Wstag805RainData {
    /* 0x00 */ Wstag805Drop *drops[27];
} Wstag805RainData; /* size 0x6C */

/* An object over the sprites of types 1-4 (message 0x35B switches it to its second look). */
typedef struct Wstag805Lamp {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagSpriteAnim sprites[4];
} Wstag805Lamp; /* size 0x70 */

/* The data of the stage object (wstag805_update). */
typedef struct Wstag805Data {
    /* 0x0 */ Wstag805Rain *rain;
    /* 0x4 */ Wstag805Lamp *lamp;
    /* 0x8 */ FieldstgEvent *event;
} Wstag805Data; /* size 0xC */

extern Wstag805Spawn D_WSTAG805_800A709C[];
extern s16 D_WSTAG805_800A71E0[];
extern WstagAnimKeyB D_WSTAG805_800A7218[];
extern WstagAnimKeyB D_WSTAG805_800A7250[];
extern WstagAnimKeyB D_WSTAG805_800A726C[];
extern WstagAnimKeyB D_WSTAG805_800A7288[];
extern WstagAnimKeyB *D_WSTAG805_800A72A8[4];
extern Wstag805Key D_WSTAG805_800A72B8[];
extern Wstag805Key D_WSTAG805_800A7300[];
extern Wstag805Key D_WSTAG805_800A7348[];
extern Wstag805Key D_WSTAG805_800A73A0[];
extern WstagFuncs wstag805_funcs;
const CVECTOR wstag805_color = { 0x80, 0x80, 0x80, 0 };
extern FieldstgBattleLists wstag805_battle_lists;
extern FieldstgVramPlace wstag805_vram_places[];
extern FieldstgPlacedActor *wstag805_actors[];
extern FieldstgSprite wstag805_sprites[];
extern FieldstgEventDef wstag805_events[];
void wstag805_lamp_update(Wstag805Lamp *obj);
void wstag805_drop_update(Wstag805Drop *obj);
Wstag805Drop *wstag805_drop_create(s32 x, s32 y, s32 dir);
void wstag805_update(WstagObject *obj, Wstag805Data *data);

void wstag805_rain_update(Wstag805Rain *obj, Wstag805RainData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 27; i++) {
            data->drops[i] = wstag805_drop_create(D_WSTAG805_800A709C[i].x, D_WSTAG805_800A709C[i].y,
                                                    D_WSTAG805_800A709C[i].dir);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->timer >= D_WSTAG805_800A71E0[obj->next] && obj->timer < D_WSTAG805_800A71E0[obj->next + 1]) {
            data->drops[obj->next]->base.set_state(data->drops[obj->next], OBJECT_STATE_DONE);
            obj->next++;
            if (D_WSTAG805_800A71E0[obj->next] == 0x1000) {
                obj->base.set_state(obj, OBJECT_STATE_RUN);
            }
        }
        obj->timer += gfx_module.funcs.get_frame_ticks();
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag805_rain_message(Wstag805Rain *obj, s32 msg) {
    if (obj != NULL && msg == 0x335) {
        obj->next = 0;
        obj->timer = 0;
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}

Wstag805Rain *wstag805_rain_create(s32 arg0) {
    return object_create(wstag805_rain_update, sizeof(Wstag805Rain), sizeof(Wstag805RainData), arg0);
}

s32 wstag805_sprite_anim_loop_b(WstagSpriteAnim *sa, WstagAnimKeyB *keys, s32 depth) {
    WstagAnimKeyB *key = &keys[sa->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();
    s32 next;

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
            next = key->unk_3;
            key = &keys[next];
            sa->anim.key = next;
            sa->anim.time += key->time;
        }
        wstag805_sprite_anim_loop_b(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag805_lamp_update(Wstag805Lamp *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 frame;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            switch (list->type) {
            case 1:
                obj->sprites[0].anim.key = 0;
                obj->sprites[0].anim.time = D_WSTAG805_800A72A8[0]->time;
                obj->sprites[0].sprite = list;
                break;
            case 2:
                obj->sprites[1].anim.key = 0;
                obj->sprites[1].anim.time = D_WSTAG805_800A72A8[1]->time;
                obj->sprites[1].sprite = list;
                break;
            case 3:
                obj->sprites[2].anim.key = 0;
                obj->sprites[2].anim.time = D_WSTAG805_800A72A8[2]->time;
                obj->sprites[2].sprite = list;
                break;
            case 4:
                obj->sprites[3].anim.key = 0;
                obj->sprites[3].anim.time = D_WSTAG805_800A72A8[3]->time;
                obj->sprites[3].sprite = list;
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 4; i++) {
            sprite = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                sprite->shown = 1;
                sprite->sprite = 0x24;
                sprite->frame = 0;
                break;
            case 1:
                sprite->shown = 0;
                break;
            case 2:
                sprite->shown = 0;
                break;
            case 3:
                sprite->shown = 1;
                sprite->sprite = 0x13;
                sprite->frame = wstag805_sprite_anim_loop_b(&obj->sprites[3], D_WSTAG805_800A7250, 0);
                break;
            }
        }
        break;
    case OBJECT_STATE_DONE:
        for (i = 0; i < 4; i++) {
            sprite = obj->sprites[i].sprite;
            switch (i) {
            case 0:
                sprite->shown = 1;
                sprite->sprite = wstag805_sprite_anim_loop_b(&obj->sprites[0], D_WSTAG805_800A7218, 0);
                sprite->frame = 0;
                break;
            case 1:
                sprite->shown = 1;
                sprite->sprite = 0x15;
                sprite->frame = wstag805_sprite_anim_loop_b(&obj->sprites[1], D_WSTAG805_800A726C, 0);
                break;
            case 2:
                sprite->shown = 1;
                sprite->sprite = 0x16;
                frame = wstag805_sprite_anim_loop_b(&obj->sprites[2], D_WSTAG805_800A7288, 0);
                if (frame == 0x12C) {
                    sprite->frame = 0;
                } else {
                    sprite->frame = frame;
                }
                break;
            case 3:
                sprite->shown = 0;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void wstag805_lamp_message(Wstag805Lamp *obj, s32 msg) {
    if (obj != NULL && msg == 0x35B) {
        sound_module.play(0x340004);
        obj->base.set_state(obj, OBJECT_STATE_DONE);
    }
}

Wstag805Lamp *wstag805_lamp_create(s32 arg0) {
    return object_create(wstag805_lamp_update, sizeof(Wstag805Lamp), 0, arg0);
}

s32 wstag805_part_anim_advance(Wstag805Part *part, Wstag805Key *keys, s32 once, s32 depth) {
    Wstag805Key *key = &keys[part->anim.key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        part->anim.time -= step;
    }
    if (once) {
        part->pos += key->delta;
    }
    if (part->anim.time <= 0) {
        key++;
        part->anim.key++;
        part->anim.time += key->time;
        if (once) {
            if (key->frame == 0xFF) {
                part->anim.key--;
                key--;
                part->anim.time += key->time;
            }
        } else if (key->frame == 0xFF) {
            key = keys;
            part->anim.key = 0;
            part->anim.time += key->time;
        }
        wstag805_part_anim_advance(part, keys, once, depth + 1);
    }
    return key->frame;
}

s32 wstag805_is_on_screen(s32 x, s32 y, s32 w, s32 h) {
    GfxRect view;
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);

    layer->get_view_rect(layer, &view);
    if (x + w < view.x) {
        return 0;
    }
    if (view.x + view.w < x) {
        return 0;
    }
    if (y + h < view.y) {
        return 0;
    }
    return view.y + view.h >= y;
}

void wstag805_drop_draw(Wstag805Drop *obj, GfxLayer *layer, s32 which) {
    Sprite spr;
    Wstag805Part *part = &obj->parts[which];

    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_layer(layer, 0xA);
    spr.set_palette(part->palette);
    spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), part->frame, obj->x, obj->y);
}

void wstag805_drop_update(Wstag805Drop *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    s32 y;
    s32 pos;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            obj->parts[0].anim.key = 0;
            obj->parts[0].anim.time = D_WSTAG805_800A72B8[0].time;
            obj->parts[1].anim.key = 0;
            obj->parts[1].anim.time = D_WSTAG805_800A73A0[0].time;
            obj->parts[2].anim.key = 0;
            obj->parts[2].anim.time = D_WSTAG805_800A7300[0].time;
            obj->base.set_step(obj, 1);
        }
        obj->parts[0].frame = wstag805_part_anim_advance(&obj->parts[0], D_WSTAG805_800A72B8, 0, 0);
        obj->parts[0].palette = wstag805_part_anim_advance(&obj->parts[2], D_WSTAG805_800A7300, 0, 0);
        obj->parts[1].frame = 0x28;
        obj->parts[1].palette = wstag805_part_anim_advance(&obj->parts[1], D_WSTAG805_800A73A0, 0, 0);
        if (obj->parts[0].frame != 0 && wstag805_is_on_screen(obj->x, obj->y, 0x20, 0x64)) {
            wstag805_drop_draw(obj, layer, 0);
        }
        if (obj->parts[1].frame != 0 && wstag805_is_on_screen(obj->x, obj->y, 0x20, 0x64)) {
            wstag805_drop_draw(obj, layer, 1);
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->parts[0].anim.key = 0;
            obj->parts[0].anim.time = D_WSTAG805_800A7348[0].time;
            obj->pos_y = obj->y << 8;
            obj->base.set_step(obj, 1);
            sound_module.play(0x800421BF);
        }
        obj->parts[0].frame = wstag805_part_anim_advance(&obj->parts[0], D_WSTAG805_800A7348, 1, 0);
        obj->parts[0].palette = 0;
        if (obj->y >= -0x64 && obj->y <= 0x3E8) {
            pos = obj->parts[0].pos;
            y = obj->pos_y;
            obj->pos_y = obj->up != 0 ? y - pos : y + pos;
            obj->y = obj->pos_y >> 8;
        }
        if (obj->parts[0].frame != 0 && wstag805_is_on_screen(obj->x, obj->y, 0x20, 0x64)) {
            wstag805_drop_draw(obj, layer, 0);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Wstag805Drop *wstag805_drop_create(s32 x, s32 y, s32 dir) {
    Wstag805Drop *obj = object_new(wstag805_drop_update, sizeof(Wstag805Drop), 0);

    obj->x = x;
    obj->y = y;
    obj->up = dir;
    return obj;
}

void wstag805_update(WstagObject *obj, Wstag805Data *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        data->rain = wstag805_rain_create(0x349);
        data->lamp = wstag805_lamp_create(0x348);
        /* Evidence (class A1, sched1 barrier; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original schedules the lamp store
         * before the block, as before a loop start. */
        LOOP_BLOCK(if (gamestate_flags.get_flag(0x4048, 0) && gamestate_flags.get_flag(0x4046, 0)) {
            data->event = fieldstg_event_start(0x370);
        } else if (gamestate_flags.get_flag(0x4048, 0) && gamestate_flags.get_flag(0x4046, 1) && gamestate_flags.get_flag(0x4064, 0)) {
            data->event = fieldstg_event_start(0x371);
        } else if (gamestate_flags.get_flag(0x4048, 1) && gamestate_flags.get_flag(0x4046, 0) && gamestate_flags.get_flag(0x4065, 0)) {
            data->event = fieldstg_event_start(0x372);
        });
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag805_start(void *arg0) {
    WstagObject *obj = object_new(wstag805_update, sizeof(WstagObject), sizeof(Wstag805Data));

    obj->manager = arg0;
    wstag805_funcs.setup();
    return obj;
}

void wstag805_event_880_end(void) {
    gamestate_flags.set_flag(0x4046, 1);
    gamestate_flags.set_flag(0x7400, 1);
}

void wstag805_event_881_end(void) {
    gamestate_flags.set_flag(0x8AF0, 1);
}

void wstag805_event_882_end(void) {
    gamestate_flags.set_flag(0x4065, 1);
}

void wstag805_setup(void) {
    fieldstg_stage.background_file = 0x671;
    fieldstg_stage.sprite_file = 0x06720000;
    fieldstg_stage.sprites = wstag805_sprites;
    fieldstg_stage.mask_file = 0x670;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x2A000, 0x18C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag805_vram_places;
    fieldstg_stage.music = 0xD;
    fieldstg_stage.sound = 0x60340000;
    fieldstg_stage.actors = wstag805_actors;
    fieldstg_stage.color = wstag805_color;
    fieldstg_stage.battle_lists = &wstag805_battle_lists;
    fieldstg_stage.events = wstag805_events;
    fieldstg_attr.set_file(0, 0x06720001);
    fieldstg_attr.init_layer(0);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag805_setup(void);

s16 D_WSTAG805_800A6D40[137] = {
    FIELDSTG_EVENT_PLACE(1, 692, 408),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_PLACE(210, 360, 197),
    FIELDSTG_EVENT_ANIM(210, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 648, 386, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 584, 310, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WALK(1, 472, 254, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_DIALOG(0, 5, 210, 3),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 392, 214, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 3),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 210),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(210, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 806, 210),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 210, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 210, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG805_800A6E54[197] = {
    FIELDSTG_EVENT_PLACE(1, 392, 214),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_PLACE(316, 371, 202),
    FIELDSTG_EVENT_ANIM(316, 1, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 378, 207, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_PLACE(316, 0, 0),
    FIELDSTG_EVENT_ANIM(316, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 7, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x348, 859, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 873, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(1, 352, 194, 3),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 807, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 805, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 224, 178),
    FIELDSTG_EVENT_ANIM(0x349, 821, 1),
    FIELDSTG_EVENT_WAIT(300),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 1, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 882, 1),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 1),
    FIELDSTG_EVENT_GOTO_MAP(0xE04, 1, 1, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG805_800A6FE0[94] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 210),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_PLACE(210, 360, 197),
    FIELDSTG_EVENT_ANIM(210, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(180),
    FIELDSTG_EVENT_DIALOG(0, 4, 210, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x348, 859, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 210, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 873, 2),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_CAMERA_MOVE(0, 224, 178),
    FIELDSTG_EVENT_ANIM(0x349, 821, 2),
    FIELDSTG_EVENT_WAIT(300),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 210),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 210, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_DIALOG(0, 3, 210, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(0x32D, 882, 2),
    FIELDSTG_EVENT_WAIT(150),
    FIELDSTG_EVENT_ANIM(0x32D, 883, 2),
    FIELDSTG_EVENT_GOTO_MAP(0xE04, 350, 202, 1),
    FIELDSTG_EVENT_END,
};
Wstag805Spawn D_WSTAG805_800A709C[27] = {
    { 8, 140, 0 }, { 28, 300, 1 }, { 48, 120, 0 }, { 68, 280, 1 }, { 88, 100, 0 }, { 108, 260, 1 }, { 128, 80, 0 },
    { 148, 240, 1 }, { 168, 60, 0 }, { 188, 220, 1 }, { 208, 40, 1 }, { 228, 200, 0 }, { 248, 20, 1 },
    { 288, 0, 1 }, { 328, -20, 1 }, { 348, 140, 1 }, { 368, -40, 0 }, { 388, 120, 1 }, { 408, -60, 0 },
    { 428, 100, 1 }, { 448, -80, 0 }, { 468, 80, 1 }, { 508, 60, 1 }, { 548, 40, 1 }, { 588, 20, 1 }, { 628, 0, 1 },
    { 668, -20, 1 },
};
s16 D_WSTAG805_800A71E0[28] = {
    0, 4, 12, 16, 24, 28, 36, 40,
    48, 52, 60, 64, 72, 84, 96, 100,
    108, 112, 120, 124, 132, 136, 148, 160,
    172, 184, 196, 4096,
};
WstagAnimKeyB D_WSTAG805_800A7218[14] = {
    { 23, 4, 0 }, { 24, 4, 0 }, { 25, 4, 0 }, { 26, 4, 0 }, { 27, 4, 0 }, { 28, 4, 0 }, { 29, 4, 0 }, { 30, 4, 0 },
    { 31, 4, 0 }, { 32, 4, 0 }, { 33, 4, 0 }, { 34, 8, 0 }, { 35, 8, 0 }, { 255, 0, 0xB },
};
WstagAnimKeyB D_WSTAG805_800A7250[7] = {
    { 0, 8, 0 }, { 1, 8, 0 }, { 2, 8, 0 }, { 3, 8, 0 }, { 4, 8, 0 }, { 5, 8, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG805_800A726C[7] = {
    { 0, 8, 0 }, { 1, 8, 0 }, { 2, 8, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 0 },
};
WstagAnimKeyB D_WSTAG805_800A7288[8] = {
    { 300, 4, 0 }, { 0, 8, 0 }, { 1, 8, 0 }, { 2, 8, 0 }, { 3, 6, 0 }, { 4, 6, 0 }, { 5, 6, 0 }, { 255, 0, 1 },
};
WstagAnimKeyB *D_WSTAG805_800A72A8[4] = {
    D_WSTAG805_800A7218, D_WSTAG805_800A726C, D_WSTAG805_800A7288, D_WSTAG805_800A7250,
};
Wstag805Key D_WSTAG805_800A72B8[9] = {
    { 42, 4, 0 }, { 42, 4, 0 }, { 43, 4, 0 }, { 43, 4, 0 }, { 44, 4, 0 }, { 44, 4, 0 }, { 45, 4, 0 }, { 45, 4, 0 },
    { 255, 0, 0 },
};
Wstag805Key D_WSTAG805_800A7300[9] = {
    { 0, 4, 0 }, { 1, 4, 0 }, { 0, 4, 0 }, { 1, 4, 0 }, { 0, 4, 0 }, { 1, 4, 0 }, { 0, 4, 0 }, { 1, 4, 0 },
    { 255, 0, 0 },
};
Wstag805Key D_WSTAG805_800A7348[11] = {
    { 11, 4, 0 }, { 12, 4, 0 }, { 13, 4, 0 }, { 14, 4, 0 }, { 15, 4, 0 }, { 16, 4, 0 }, { 17, 4, 0 }, { 17, 4, 0 },
    { 18, 30, 64 }, { 18, 30, 0 }, { 255, 999, 0 },
};
Wstag805Key D_WSTAG805_800A73A0[7] = {
    { 0, 8, 0 }, { 1, 8, 0 }, { 2, 8, 0 }, { 3, 8, 0 }, { 4, 8, 0 }, { 5, 8, 0 }, { 255, 0, 0 },
};
FieldstgListedBattle D_WSTAG805_800A73D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A73E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A73F0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A73FC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7408 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7414 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7420 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A742C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG805_800A7438 = {
    0,
    { &D_WSTAG805_800A73D8, &D_WSTAG805_800A73E4, &D_WSTAG805_800A73F0, &D_WSTAG805_800A73FC, &D_WSTAG805_800A7408,
        &D_WSTAG805_800A7414, &D_WSTAG805_800A7420, &D_WSTAG805_800A742C },
};
FieldstgListedBattle D_WSTAG805_800A745C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7468 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7474 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7480 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A748C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7498 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A74A4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A74B0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG805_800A74BC = {
    0,
    { &D_WSTAG805_800A745C, &D_WSTAG805_800A7468, &D_WSTAG805_800A7474, &D_WSTAG805_800A7480, &D_WSTAG805_800A748C,
        &D_WSTAG805_800A7498, &D_WSTAG805_800A74A4, &D_WSTAG805_800A74B0 },
};
FieldstgListedBattle D_WSTAG805_800A74E0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A74EC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A74F8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7504 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7510 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A751C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7528 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7534 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG805_800A7540 = {
    0,
    { &D_WSTAG805_800A74E0, &D_WSTAG805_800A74EC, &D_WSTAG805_800A74F8, &D_WSTAG805_800A7504, &D_WSTAG805_800A7510,
        &D_WSTAG805_800A751C, &D_WSTAG805_800A7528, &D_WSTAG805_800A7534 },
};
FieldstgListedBattle D_WSTAG805_800A7564 = { 22, 18, 0x608C0000 };
FieldstgListedBattle D_WSTAG805_800A7570 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A757C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7588 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A7594 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A75A0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A75AC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG805_800A75B8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG805_800A75C4 = {
    0,
    { &D_WSTAG805_800A7564, &D_WSTAG805_800A7570, &D_WSTAG805_800A757C, &D_WSTAG805_800A7588, &D_WSTAG805_800A7594,
        &D_WSTAG805_800A75A0, &D_WSTAG805_800A75AC, &D_WSTAG805_800A75B8 },
};
FieldstgBattleLists wstag805_battle_lists = {
    140, 0, 0, { &D_WSTAG805_800A7438, &D_WSTAG805_800A74BC, &D_WSTAG805_800A7540 }, &D_WSTAG805_800A75C4,
};
FieldstgVramPlace wstag805_vram_places[9] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 345, 288, 100, 32, 320, 501 }, { 320, 256, 327, 256, 28, 0, 336, 501 },
    { 320, 256, 324, 472, 16, 216, 352, 501 },
};
u16 D_WSTAG805_800A7694[4] = { 0x6020, 1, 0xFFFF, 0 };
u16 D_WSTAG805_800A769C[6] = { 0x6020, 1, 0x4046, 0, 0xFFFF, 0 };
u16 D_WSTAG805_800A76A8[4] = { 0x6020, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG805_800A76B0 = { D_WSTAG805_800A7694, NULL, 1, 4, 0, 0, 0 };
FieldstgPlacedActor D_WSTAG805_800A76C4 = { D_WSTAG805_800A769C, NULL, 210, 5, 360, 197, 3 };
FieldstgPlacedActor D_WSTAG805_800A76D8 = { D_WSTAG805_800A76A8, NULL, 316, 6, 0, 0, 1 };
FieldstgPlacedActor *wstag805_actors[4] = {
    &D_WSTAG805_800A76B0, &D_WSTAG805_800A76C4, &D_WSTAG805_800A76D8, NULL,
};
FieldstgSprite wstag805_sprites[5] = {
    { 1, 1, 0xFF, 6, 0x13, 0, 0, 0, 0, 0, 235, 78, 0, 0 }, { 1, 2, 0xFF, 6, 0x15, 0, 0, 0, 0, 0, 235, 78, 0, 0 },
    { 1, 3, 0xFF, 6, 0x16, 0, 0, 0, 0, 0, 235, 78, 0, 0 }, { 1, 4, 0xFF, 6, 0x17, 0, 0, 0, 0, 0, 235, 78, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag805_funcs = { wstag805_setup };
FieldstgEventDef wstag805_events[4] = {
    { 880, D_WSTAG805_800A6D40, 0x01510003, NULL, wstag805_event_880_end },
    { 881, D_WSTAG805_800A6E54, 0x01510004, NULL, wstag805_event_881_end },
    { 882, D_WSTAG805_800A6FE0, 0x01510005, NULL, wstag805_event_882_end }, { -1, NULL, 0, NULL, NULL },
};
