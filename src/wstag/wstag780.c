#include "wstag.h"
#include "pad.h"

/* WSTAG780: stage 0x2D7 (fieldstg_stages). */

extern WstagFuncs wstag780_funcs;
void wstag780_gauge_update();
void wstag780_lamp_update();
void wstag780_sparks_update();
void wstag780_screen_update();
void wstag780_update();

/* A sprite that moves diagonally across the screen (wstag780_mover_create). */
typedef struct Wstag780Mover {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 up; /* moves up */
    /* 0x54 */ s32 right; /* moves right, drawn mirrored */
    /* 0x58 */ s32 still; /* stands still */
    /* 0x5C */ s32 variant;
    /* 0x60 */ s32 frame; /* frame */
    /* 0x64 */ s32 timer; /* timer, 0-7 */
    /* 0x68 */ s32 pos_x; /* x << 8 */
    /* 0x6C */ s32 pos_y; /* y << 8 */
} Wstag780Mover; /* size 0x70 */

/* A gauge that opens to frame 0x1C / 4, waits, then closes (wstag780_gauge_create). */
typedef struct Wstag780Gauge {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 frame; /* frame << 2 */
} Wstag780Gauge; /* size 0x54 */

/* A sparkle at one of six places (wstag780_spark_create), drawn through the layer's list. */
typedef struct Wstag780Spark {
    /* 0x00 */ Object base; /* base.key1: the place (D_WSTAG780_800A8420) */
    /* 0x50 */ s32 frame; /* frame, 0: none */
    /* 0x54 */ s32 x; /* x */
    /* 0x58 */ s32 y; /* y */
    /* 0x5C */ WstagAnim anim;
} Wstag780Spark; /* size 0x60 */

/* A place of Wstag780Spark: its animation and position. */
typedef struct Wstag780SparkPlace {
    /* 0x0 */ WstagAnimKey *keys;
    /* 0x4 */ WstagPos pos;
} Wstag780SparkPlace; /* size 0x8 */

/* An object that loops the animations of the sprites of types 1 and 2 (wstag780_lamp_create). */
typedef struct Wstag780Lamp {
    /* 0x00 */ Object base;
    /* 0x50 */ WstagSpriteAnim sprites[2];
} Wstag780Lamp; /* size 0x60 */

/* The data of wstag780_sparks_update's object. */
typedef struct Wstag780Data {
    /* 0x00 */ Wstag780Spark *sparks[6];
    /* 0x18 */ Wstag780Lamp *lamp;
} Wstag780Data; /* size 0x1C */

/* Three counters drawn as frames 0x41-0x46, which spawn Wstag780Mover objects (wstag780_board_create). */
typedef struct Wstag780Board {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 light_a; /* 0-2 */
    /* 0x54 */ s32 light_b; /* 0-2 */
    /* 0x58 */ s32 spawn_a; /* spawns movers */
    /* 0x5C */ s32 spawn_b; /* spawns movers */
} Wstag780Board; /* size 0x60 */

typedef struct Wstag780BoardData {
    /* 0x0 */ Wstag780Mover *movers[4];
} Wstag780BoardData; /* size 0x10 */

/* A key of Wstag780ScreenAnim's animations: time 0 jumps to key `value`. */
typedef struct Wstag780ScreenKey {
    /* 0x0 */ u8 value; /* D_WSTAG780_800A8520 index */
    /* 0x1 */ u8 time;
} Wstag780ScreenKey; /* size 0x2 */

/* One of Wstag780Screen's two animations (D_WSTAG780_800A8E14[id]). */
typedef struct Wstag780ScreenAnim {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 prev; /* restarts when id changes */
    /* 0x08 */ s32 key;
    /* 0x0C */ s32 value;
    /* 0x10 */ s32 time;
} Wstag780ScreenAnim; /* size 0x14 */

/* A picture of 40 textured quads (D_WSTAG780_800A8A50) whose kinds 0 and 1 take their texture from two animations
 * (wstag780_screen_create; message 0x322 switches the animations). */
typedef struct Wstag780Screen {
    /* 0x00 */ Object base;
    /* 0x50 */ Wstag780ScreenAnim anims[2];
} Wstag780Screen; /* size 0x78 */

/* A quad of Wstag780Screen. */
typedef struct Wstag780Quad {
    /* 0x00 */ s16 x0;
    /* 0x02 */ s16 y0;
    /* 0x04 */ s16 x1;
    /* 0x06 */ s16 y1;
    /* 0x08 */ s16 x2;
    /* 0x0A */ s16 y2;
    /* 0x0C */ s16 x3;
    /* 0x0E */ s16 y3;
    /* 0x10 */ s16 kind; /* 0, 1: Wstag780Screen's animation (1: a 0x14 texture, else 0x28); else the texture;
                          * >= 9: semi-transparent */
} Wstag780Quad; /* size 0x12 */

/* The data of the stage object (wstag780_update). */
typedef struct Wstag780StageData {
    /* 0x0 */ FieldstgEvent *event;
    /* 0x4 */ Wstag780Screen *screen;
    /* 0x8 */ Object *sparks;
} Wstag780StageData; /* size 0xC */

extern FieldstgVramPlace D_WSTAG780_800A8520[];
extern Wstag780Quad D_WSTAG780_800A8A50[40];
extern Wstag780ScreenKey *D_WSTAG780_800A8E14[];
extern u8 D_WSTAG780_800A83C0[2][2][2];
extern Wstag780SparkPlace D_WSTAG780_800A8420[];
extern WstagAnimKey *D_WSTAG780_800A8518[2];
Wstag780Lamp *wstag780_lamp_create(void);
extern GamestatePos D_WSTAG780_800A83C8[2][2][2];
extern FieldstgVramPlace wstag780_vram_places[];
extern FieldstgPlacedActor *wstag780_actors[];
extern FieldstgSprite wstag780_sprites[];
extern FieldstgEventDef wstag780_events[];

void wstag780_mover_draw(Wstag780Mover *obj) {
    Sprite spr;
    GamestatePos pos;

    sprite_init(&spr);
    pos.x = obj->pos_x >> 8;
    pos.y = obj->pos_y >> 8;
    spr.set_layer_id(0x1002, 2);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.set_pivot(pos.x, pos.y);
    if (obj->right != 0) {
        spr.set_scale(-0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x01C50000), obj->frame, pos.x, pos.y);
}

void wstag780_mover_update(Wstag780Mover *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->still == 0) {
            if (obj->right != 0) {
                obj->pos_x += 0x500;
            } else {
                obj->pos_x -= 0x500;
            }
            if (obj->up != 0) {
                obj->pos_y -= 0x280;
            } else {
                obj->pos_y += 0x280;
            }
        }
        obj->timer += gfx_module.funcs.get_frame_ticks();
        if (obj->timer >= 8) {
            obj->timer = 0;
        }
        if (obj->variant != 0 && obj->still != 0) {
            obj->frame = 0x3B;
        } else {
            obj->frame = D_WSTAG780_800A83C0[obj->variant][obj->up][obj->timer >> 2];
        }
        if (obj->right != 0) {
            if (obj->pos_x >= 0x28000) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
        } else if (obj->pos_x <= 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        wstag780_mover_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Wstag780Mover *wstag780_mover_create(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    Wstag780Mover *obj = object_new(wstag780_mover_update, sizeof(Wstag780Mover), 0);

    obj->right = arg1;
    obj->up = arg0;
    obj->still = arg2;
    obj->variant = arg3;
    obj->pos_x = D_WSTAG780_800A83C8[arg0][arg1][arg2].x << 8;
    obj->pos_y = D_WSTAG780_800A83C8[arg0][arg1][arg2].y << 8;
    if (arg2 != 0 && arg3 != 0) {
        obj->pos_x += 0x3200;
        obj->pos_y -= 0x1900;
    }
    return obj;
}

void wstag780_gauge_draw(Wstag780Gauge *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x1002, 7);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.draw(cdload_module.get_subfile_by_id(0x01C50000), obj->frame >> 2, 0xDA, 0x47);
}

void wstag780_gauge_update(Wstag780Gauge *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame = 0xC;
        if (gamestate_data.progress == 0x27) {
            sound_module.play(0x80A4203C);
        } else {
            sound_module.play(0xA40004);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->frame += gfx_module.funcs.get_frame_ticks();
            if (obj->frame >= 0x1C) {
                obj->frame = 0x1C;
                obj->base.next_step(obj);
            }
            break;
        case 1:
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep >= 0x8C) {
                obj->base.next_step(obj);
            }
            break;
        case 2:
            obj->frame -= gfx_module.funcs.get_frame_ticks();
            if (obj->frame < 0xD) {
                obj->frame = 0xC;
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        wstag780_gauge_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag780_gauge_create(s32 arg0) {
    return object_create(wstag780_gauge_update, 0x54, 0, arg0);
}

s32 wstag780_anim_loop(WstagAnim *anim, WstagAnimKey *keys, s32 depth) {
    WstagAnimKey *key = &keys[anim->key];
    s32 step = gfx_module.funcs.get_frame_ticks();

    if (step >= 5) {
        step = 4;
    }
    if (depth == 0) {
        anim->time -= step;
    }
    if (anim->time <= 0) {
        key++;
        anim->key++;
        anim->time += key->time;
        if (key->frame == 0xFF) {
            key = keys;
            anim->key = 0;
            anim->time += key->time;
        }
        wstag780_anim_loop(anim, keys, depth + 1);
    }
    return key->frame;
}

void wstag780_spark_draw(Wstag780Spark *obj, GfxLayer *layer, s32 which) {
    Sprite spr;

    if (obj->base.state == OBJECT_STATE_RUN) {
        sprite_init(&spr);
        spr.set_vram_pos(0x140, 0x100);
        spr.set_layer(layer, 4);
        spr.set_palette(0);
        spr.draw(cdload_module.get_subfile_by_id(fieldstg_stage.sprite_file), obj->frame, obj->x, obj->y);
    }
}

void wstag780_spark_update(Wstag780Spark *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->x = D_WSTAG780_800A8420[obj->base.key1].pos.x;
        obj->y = D_WSTAG780_800A8420[obj->base.key1].pos.y;
        obj->anim.key = pad_random.next() & 1;
        obj->anim.time = pad_random.next() % 3 + 2;
        obj->frame = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->frame = wstag780_anim_loop(&obj->anim, D_WSTAG780_800A8420[obj->base.key1].keys, 0);
        if (obj->frame != 0) {
            layer->add_callback(layer, (WstagDrawCallback)wstag780_spark_draw, obj, obj->y, 0);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Wstag780Spark *wstag780_spark_create(s32 i) {
    Wstag780Spark *obj = object_new(wstag780_spark_update, sizeof(Wstag780Spark), 0);

    obj->base.key1 = i;
    return obj;
}

s32 wstag780_sprite_anim_loop(WstagSpriteAnim *sa, WstagAnimKey *keys, s32 depth) {
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
            key = keys;
            sa->anim.key = 0;
            sa->anim.time += key->time;
        }
        wstag780_sprite_anim_loop(sa, keys, depth + 1);
    }
    return key->frame;
}

void wstag780_lamp_update(Wstag780Lamp *obj) {
    FieldstgSprite *list;
    FieldstgSprite *sprite;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->sprites[0].anim.key = 0;
        obj->sprites[0].anim.time = D_WSTAG780_800A8518[0]->time;
        obj->sprites[1].anim.key = 0;
        obj->sprites[1].anim.time = D_WSTAG780_800A8518[1]->time;
        for (list = fieldstg_stage.sprites; list->present != 0; list++) {
            switch (list->type) {
            case 1:
                obj->sprites[0].sprite = list;
                break;
            case 2:
                obj->sprites[1].sprite = list;
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            sprite = obj->sprites[i].sprite;
            sprite->shown = 1;
            sprite->sprite = wstag780_sprite_anim_loop(&obj->sprites[i], D_WSTAG780_800A8518[i], 0);
            sprite->frame = 0;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Wstag780Lamp *wstag780_lamp_create(void) {
    return object_new(wstag780_lamp_update, sizeof(Wstag780Lamp), 0);
}

void wstag780_sparks_update(Object *obj, Wstag780Data *data) {
    s32 i;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 6; i++) {
            data->sparks[i] = wstag780_spark_create(i);
        }
        data->lamp = wstag780_lamp_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag780_sparks_new(void) {
    return object_new(wstag780_sparks_update, sizeof(Object), sizeof(Wstag780Data));
}

void wstag780_board_spawn_movers(Wstag780Board *obj, Wstag780BoardData *data) {
    s32 r;
    s32 r2;

    if (obj->spawn_a != 0) {
        r = pad_random.next() & 1;
        if (data->movers[0] == NULL) {
            data->movers[0] = wstag780_mover_create(0, 0, 0, r);
        }
        if (data->movers[3] == NULL) {
            data->movers[3] = wstag780_mover_create(1, 1, 0, 0);
        }
    }
    if (obj->spawn_b != 0) {
        r2 = pad_random.next() & 1;
        if (data->movers[0] == NULL) {
            data->movers[0] = wstag780_mover_create(0, 0, 1, 0);
        }
        if (data->movers[3] == NULL) {
            data->movers[3] = wstag780_mover_create(0, 0, 1, 1);
        }
        if (data->movers[1] == NULL) {
            data->movers[1] = wstag780_mover_create(1, 0, 0, 0);
        }
        if (data->movers[2] == NULL) {
            data->movers[2] = wstag780_mover_create(0, 1, 0, r2);
        }
    }
}

void wstag780_board_draw(Wstag780Board *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x1002, 2);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0, 0x1F0);
    spr.set_palette((gfx_module.funcs.get_time() >> 1) & 1);
    switch (obj->light_a) {
    case 0:
    default:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x44, 0xC6, 0xB3);
        break;
    case 1:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x45, 0xC6, 0xBC);
        break;
    case 2:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x46, 0xC6, 0xC4);
        break;
    }
    switch (obj->light_a) {
    case 0:
    default:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x44, 0x156, 0xFB);
        break;
    case 1:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x45, 0x156, 0x104);
        break;
    case 2:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x46, 0x156, 0x10C);
        break;
    }
    switch (obj->light_b) {
    case 0:
    default:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x41, 0xEA, 0xB3);
        break;
    case 1:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x42, 0xEA, 0xBC);
        break;
    case 2:
        spr.draw(cdload_module.get_subfile_by_id(0x01C50000), 0x43, 0xEA, 0xC4);
        break;
    }
}

void wstag780_board_update(Wstag780Board *obj, Wstag780BoardData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->light_a = 2;
            obj->light_b = 0;
            obj->spawn_a = 1;
            obj->spawn_b = 0;
            break;
        case 1:
            if (obj->light_a == 2) {
                obj->light_a = 1;
            }
            if (obj->light_b == 2) {
                obj->light_b = 1;
            }
            obj->base.substep += gfx_module.funcs.get_frame_ticks();
            if (obj->base.substep >= 0x79) {
                if (obj->light_a == 0) {
                    obj->light_a = 2;
                }
                if (obj->light_a == 1) {
                    obj->light_a = 0;
                }
                if (obj->light_b == 0) {
                    obj->light_b = 2;
                }
                if (obj->light_b == 1) {
                    obj->light_b = 0;
                }
                obj->base.next_step(obj);
            }
            obj->spawn_a = 0;
            obj->spawn_b = 0;
            break;
        case 2:
            obj->spawn_a = 0;
            obj->spawn_b = 1;
            break;
        }
        wstag780_board_draw(obj);
        wstag780_board_spawn_movers(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag780_board_message(Wstag780Board *obj, s32 msg) {
    if (obj != NULL) {
        switch (msg) {
        case 0x324:
            obj->base.set_step(obj, 0);
            break;
        case 0x320:
            obj->base.set_step(obj, 1);
            break;
        }
    }
}

Wstag780Board *wstag780_board_create(s32 arg0) {
    return object_create(wstag780_board_update, sizeof(Wstag780Board), sizeof(Wstag780BoardData), arg0);
}

void wstag780_screen_draw(Wstag780Screen *obj) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x1002);
    u32 *ot = layer->get_ot_entry(layer, 2);
    s32 ofs[2];
    POLY_FT4 *poly;
    FieldstgVramPlace *vram;
    s32 size;
    s32 i;
    Wstag780Quad *quad;

    layer->get_scroll(layer, ofs);
    poly = gfx_module.funcs.get_packet();
    for (i = 0; i < 40; i++) {
        quad = &D_WSTAG780_800A8A50[i];
        switch (quad->kind) {
        case 0:
        case 1:
            vram = &D_WSTAG780_800A8520[obj->anims[quad->kind].value];
            break;
        default:
            vram = &D_WSTAG780_800A8520[quad->kind];
            break;
        }
        size = 0x28;
        if (quad->kind == 1) {
            size = 0x14;
        }
        setPolyFT4(poly);
        if (quad->kind >= 9) {
            poly->code = 0x2E;
        }
        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = quad->x0 - ofs[0];
        poly->x1 = quad->x1 - ofs[0];
        poly->x2 = quad->x2 - ofs[0];
        poly->x3 = quad->x3 - ofs[0];
        poly->y0 = quad->y0 - ofs[1];
        poly->y1 = quad->y1 - ofs[1];
        poly->y2 = quad->y2 - ofs[1];
        poly->y3 = quad->y3 - ofs[1];
        poly->u0 = vram->u;
        poly->u1 = vram->u + size;
        poly->u2 = vram->u;
        poly->u3 = vram->u + size;
        poly->v0 = vram->v;
        poly->v1 = vram->v;
        poly->v2 = vram->v + size;
        poly->v3 = vram->v + size;
        poly->tpage = getTPage(0, 0, vram->tpage_x, vram->tpage_y);
        poly->clut = getClut(vram->clut_x, vram->clut_y);
        addPrim(ot, poly);
        poly++;
    }
    gfx_module.funcs.set_packet(poly);
}

void wstag780_screen_update(Wstag780Screen *obj) {
    Wstag780ScreenAnim *anim;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->anims[0].id = 1;
        obj->anims[0].prev = -1;
        obj->anims[1].id = 4;
        obj->anims[1].prev = -1;
        break;
    case OBJECT_STATE_RUN:
        for (i = 0; i < 2; i++) {
            anim = &obj->anims[i];
            if (anim->id != anim->prev) {
                anim->prev = anim->id;
                anim->key = 0;
                anim->value = 0;
                anim->time = 0;
            }
            anim->time -= gfx_module.funcs.get_frame_ticks();
            if (anim->time <= 0) {
                anim->key++;
                if (D_WSTAG780_800A8E14[anim->id][anim->key].time == 0) {
                    anim->key = D_WSTAG780_800A8E14[anim->id][anim->key].value;
                }
                anim->value = D_WSTAG780_800A8E14[anim->id][anim->key].value;
                anim->time = D_WSTAG780_800A8E14[anim->id][anim->key].time;
            }
        }
        wstag780_screen_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void wstag780_screen_message(Wstag780Screen *obj, s32 msg) {
    if (msg == 0x322) {
        obj->anims[0].id = 2;
        obj->anims[1].id = 5;
        sound_module.play(0xA40006);
    }
}

Wstag780Screen *wstag780_screen_create(s32 arg0) {
    return object_create(wstag780_screen_update, sizeof(Wstag780Screen), 0, arg0);
}

void wstag780_update(WstagObject *obj, Wstag780StageData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->screen = wstag780_screen_create(0x321);
        if (gamestate_data.progress == 0) {
            data->event = fieldstg_event_start(0);
        } else if (gamestate_data.progress == 0x17) {
            data->event = fieldstg_event_start(0x2AC);
        } else if (gamestate_data.progress == 0x1B) {
            data->event = fieldstg_event_start(0x2E6);
            data->sparks = wstag780_sparks_new();
        } else if (gamestate_data.progress == 0x20) {
            data->event = fieldstg_event_start(0x375);
        } else if (gamestate_data.progress == 0x27) {
            data->event = fieldstg_event_start(0x3CB);
        } else if (gamestate_data.progress == 0x2B) {
            data->event = fieldstg_event_start(0x5DC);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag780_start(void *arg0) {
    WstagObject *obj = object_new(wstag780_update, sizeof(WstagObject), sizeof(Wstag780StageData));

    obj->manager = arg0;
    wstag780_funcs.setup();
    return obj;
}

void wstag780_event_0_end(void) {
    gamestate_data.progress = 0;
}

void wstag780_event_885_end(void) {
    gamestate_flags.set_flag(0x4066, 1);
}

void wstag780_event_1500_end(void) {
    gamestate_data.progress = 0x2D;
}

void wstag780_setup(void) {
    fieldstg_stage.background_file = 0x1C4;
    fieldstg_stage.sprite_file = 0x01C50000;
    fieldstg_stage.sprites = wstag780_sprites;
    fieldstg_stage.mask_file = 0x320;
    fieldstg_stage.talk_file = records_language + 0xF6;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1C700, 0x12C00 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag780_vram_places;
    fieldstg_stage.sound = 0x60A40000;
    fieldstg_stage.actors = wstag780_actors;
    fieldstg_stage.music = 0x29;
    fieldstg_stage.events = wstag780_events;
    fieldstg_attr.set_file(0, 0x01C50001);
    fieldstg_attr.init_layer(0);
    switch (gamestate_data.progress) {
    case 27:
        fieldstg_stage.music = 0x29;
        fieldstg_stage.sound = 0x60A40001;
        break;
    case 32:
        fieldstg_stage.music = 0x29;
        fieldstg_stage.sound = 0x60A40002;
        break;
    case 39:
        fieldstg_stage.music = 0x29;
        fieldstg_stage.sound = 0x60A40003;
        break;
    }
}

/* The stage's .data (tools/wstag_data.py). */
void wstag780_setup(void);

s16 D_WSTAG780_800A7798[473] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_PLACE(1, 176, 152),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_PLACE(16, 304, 176),
    FIELDSTG_EVENT_ANIM(16, 1, 7),
    FIELDSTG_EVENT_ANIM(0x320, 804, 1),
    FIELDSTG_EVENT_ANIM(0x321, 801, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 43, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(1, 192, 144, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 57, 1),
    FIELDSTG_EVENT_ANIM(0x320, 800, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WALK(16, 640, 480, 7),
    FIELDSTG_EVENT_WAIT(360),
    FIELDSTG_EVENT_WALK(1, 208, 136, 5),
    FIELDSTG_EVENT_PLACE(16, 0, 0),
    FIELDSTG_EVENT_ANIM(16, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 46, 5),
    FIELDSTG_EVENT_ANIM(0x321, 802, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 256, 152, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 4),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_ANIM(0x321, 801, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_ANIM(0x322, 803, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_PLACE(11, 240, 120),
    FIELDSTG_EVENT_WALK(11, 192, 144, 1),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_PLACE(12, 240, 120),
    FIELDSTG_EVENT_WALK(12, 218, 132, 7),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 6, 12, 2),
    FIELDSTG_EVENT_DIALOG(0, 5, 11, 3),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 6),
    FIELDSTG_EVENT_ANIM(12, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 7, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 49, 3),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 8, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 12, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_WALK(11, 224, 152, 6),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 11, 2),
    FIELDSTG_EVENT_ANIM(11, 7, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 12, 2),
    FIELDSTG_EVENT_ANIM(11, 12, 6),
    FIELDSTG_EVENT_ANIM(12, 52, 0),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(12, 53, 0),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(12, 9, 0),
    FIELDSTG_EVENT_WAIT_ANIM(12),
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(12, 176, 152, 6),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(12, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 7, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 6),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 9, 2),
    FIELDSTG_EVENT_ANIM(11, 9, 6),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 592, 312, 7),
    FIELDSTG_EVENT_ANIM(11, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(11, 592, 312, 7),
    FIELDSTG_EVENT_WALK(12, 592, 312, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0x2D8, 640, 256, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG780_800A7B4C[148] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 192, 144),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(16, 303, 193),
    FIELDSTG_EVENT_ANIM(16, 1, 7),
    FIELDSTG_EVENT_PLACE(45, 208, 128),
    FIELDSTG_EVENT_ANIM(45, 1, 3),
    FIELDSTG_EVENT_PLACE(47, 160, 152),
    FIELDSTG_EVENT_ANIM(47, 1, 4),
    FIELDSTG_EVENT_PLACE(49, 256, 153),
    FIELDSTG_EVENT_ANIM(49, 1, 5),
    FIELDSTG_EVENT_PLACE(53, 216, 148),
    FIELDSTG_EVENT_ANIM(53, 1, 3),
    FIELDSTG_EVENT_PLACE(56, 336, 176),
    FIELDSTG_EVENT_ANIM(56, 1, 7),
    FIELDSTG_EVENT_ANIM(0x320, 804, 2),
    FIELDSTG_EVENT_ANIM(0x321, 801, 801),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x321, 802, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x321, 801, 801),
    FIELDSTG_EVENT_ANIM(0x323, 805, 53),
    FIELDSTG_EVENT_ANIM(0x324, 805, 47),
    FIELDSTG_EVENT_ANIM(0x325, 805, 45),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 45),
    FIELDSTG_EVENT_ANIM(0x324, 806, 47),
    FIELDSTG_EVENT_ANIM(0x325, 806, 53),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 3, 47, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 4, 53, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_GOTO_MAP(0x203, 330, 196, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG780_800A7C74[156] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 224, 176),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(16, 240, 160),
    FIELDSTG_EVENT_ANIM(16, 1, 7),
    FIELDSTG_EVENT_PLACE(45, 176, 200),
    FIELDSTG_EVENT_ANIM(45, 1, 0),
    FIELDSTG_EVENT_PLACE(47, 137, 172),
    FIELDSTG_EVENT_ANIM(47, 1, 1),
    FIELDSTG_EVENT_PLACE(53, 191, 136),
    FIELDSTG_EVENT_ANIM(53, 1, 0),
    FIELDSTG_EVENT_PLACE(56, 271, 192),
    FIELDSTG_EVENT_ANIM(56, 1, 7),
    FIELDSTG_EVENT_ANIM(0x321, 801, 2),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(0x323, 805, 45),
    FIELDSTG_EVENT_ANIM(0x324, 805, 16),
    FIELDSTG_EVENT_ANIM(0x325, 805, 56),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 45),
    FIELDSTG_EVENT_ANIM(0x324, 806, 16),
    FIELDSTG_EVENT_ANIM(0x325, 806, 56),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 45, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 2, 16, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 3, 56, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(56, 1, 7),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(56, 1, 0),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(56, 1, 7),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(56, 1, 6),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_ANIM(56, 1, 7),
    FIELDSTG_EVENT_WAIT(24),
    FIELDSTG_EVENT_DIALOG(0, 4, 56, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_GOTO_MAP(0x29C, 384, 218, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG780_800A7DAC[44] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 224, 160),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x321, 801, 801),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x321, 802, 801),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_GOTO_MAP(0x26D, 350, 202, 7),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG780_800A7E04[263] = {
    FIELDSTG_EVENT_CAMERA_MOVE(1, 224, 176),
    FIELDSTG_EVENT_PLACE(2, 0, 0),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_PLACE(272, 16, 256),
    FIELDSTG_EVENT_ANIM(272, 1, 5),
    FIELDSTG_EVENT_PLACE(273, 16, 128),
    FIELDSTG_EVENT_ANIM(273, 1, 7),
    FIELDSTG_EVENT_PLACE(274, 464, 256),
    FIELDSTG_EVENT_ANIM(274, 1, 3),
    FIELDSTG_EVENT_ANIM(0x321, 801, 801),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_WALK(272, 192, 168, 5),
    FIELDSTG_EVENT_WAIT_WALK(272),
    FIELDSTG_EVENT_ANIM(272, 1, 0),
    FIELDSTG_EVENT_WALK(273, 176, 208, 1),
    FIELDSTG_EVENT_WALK(274, 256, 152, 3),
    FIELDSTG_EVENT_WAIT_WALK(274),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 0),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_ANIM(274, 1, 7),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_ANIM(273, 1, 0),
    FIELDSTG_EVENT_ANIM(274, 1, 0),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 0),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_ANIM(274, 1, 7),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 7),
    FIELDSTG_EVENT_ANIM(273, 1, 2),
    FIELDSTG_EVENT_ANIM(274, 1, 6),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 0),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_ANIM(274, 1, 7),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(0x322, 803, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_PLACE(157, 240, 120),
    FIELDSTG_EVENT_ANIM(157, 1, 1),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_WALK(157, 231, 124, 1),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_ANIM(0x323, 805, 157),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(0x323, 806, 157),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_DIALOG(0, 1, 157, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(157, 1, 5),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_WALK(157, 240, 120, 5),
    FIELDSTG_EVENT_ANIM(0x322, 803, 2),
    FIELDSTG_EVENT_WAIT_WALK(157),
    FIELDSTG_EVENT_PLACE(157, 0, 0),
    FIELDSTG_EVENT_ANIM(157, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 272),
    FIELDSTG_EVENT_ANIM(0x324, 805, 273),
    FIELDSTG_EVENT_ANIM(0x325, 805, 274),
    FIELDSTG_EVENT_WAIT(90),
    FIELDSTG_EVENT_ANIM(272, 1, 1),
    FIELDSTG_EVENT_ANIM(273, 1, 1),
    FIELDSTG_EVENT_ANIM(274, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 806, 272),
    FIELDSTG_EVENT_ANIM(0x324, 806, 273),
    FIELDSTG_EVENT_ANIM(0x325, 806, 274),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_ANIM(272, 1, 5),
    FIELDSTG_EVENT_ANIM(273, 1, 5),
    FIELDSTG_EVENT_ANIM(274, 1, 3),
    FIELDSTG_EVENT_WAIT(18),
    FIELDSTG_EVENT_GOTO_MAP(0x272, 256, 480, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG780_800A8014[469] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 1),
    FIELDSTG_EVENT_PLACE(1, 176, 152),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_PLACE(16, 304, 176),
    FIELDSTG_EVENT_ANIM(16, 1, 7),
    FIELDSTG_EVENT_ANIM(0x320, 804, 1),
    FIELDSTG_EVENT_ANIM(0x321, 801, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 43, 5),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 42, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(1, 192, 144, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 2, 1, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 57, 1),
    FIELDSTG_EVENT_ANIM(0x320, 800, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(1, 42, 1),
    FIELDSTG_EVENT_WALK(16, 640, 480, 0),
    FIELDSTG_EVENT_WAIT(360),
    FIELDSTG_EVENT_WALK(1, 208, 136, 5),
    FIELDSTG_EVENT_ANIM(16, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_ANIM(1, 46, 5),
    FIELDSTG_EVENT_ANIM(0x321, 802, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 256, 152, 5),
    FIELDSTG_EVENT_WAIT_WALK(1),
    FIELDSTG_EVENT_DIALOG(0, 3, 1, 4),
    FIELDSTG_EVENT_ANIM(1, 1, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_ANIM(0x321, 801, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 4, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_ANIM(0x322, 803, 1),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 41, 1),
    FIELDSTG_EVENT_PLACE(11, 240, 120),
    FIELDSTG_EVENT_WALK(11, 192, 144, 1),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_PLACE(12, 240, 120),
    FIELDSTG_EVENT_WALK(12, 218, 132, 7),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(1, 48, 1),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(1, 6, 12, 2),
    FIELDSTG_EVENT_DIALOG(0, 5, 11, 3),
    FIELDSTG_EVENT_ANIM(1, 1, 1),
    FIELDSTG_EVENT_ANIM(11, 7, 6),
    FIELDSTG_EVENT_ANIM(12, 7, 7),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 3),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 7, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 49, 3),
    FIELDSTG_EVENT_WAIT_ANIM(1),
    FIELDSTG_EVENT_ANIM(1, 12, 3),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 8, 1, 2),
    FIELDSTG_EVENT_ANIM(1, 12, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_WALK(11, 224, 152, 6),
    FIELDSTG_EVENT_WAIT_WALK(11),
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 11, 2),
    FIELDSTG_EVENT_ANIM(11, 7, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 12, 2),
    FIELDSTG_EVENT_ANIM(11, 12, 6),
    FIELDSTG_EVENT_ANIM(12, 52, 0),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(12, 53, 0),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(12, 9, 0),
    FIELDSTG_EVENT_WAIT_ANIM(12),
    FIELDSTG_EVENT_ANIM(12, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(12, 176, 152, 6),
    FIELDSTG_EVENT_WAIT_WALK(12),
    FIELDSTG_EVENT_ANIM(12, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 12, 2),
    FIELDSTG_EVENT_ANIM(12, 7, 6),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_ANIM(12, 1, 6),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(1, 9, 2),
    FIELDSTG_EVENT_ANIM(11, 9, 6),
    FIELDSTG_EVENT_WAIT_ANIM(11),
    FIELDSTG_EVENT_ANIM(1, 1, 2),
    FIELDSTG_EVENT_ANIM(11, 1, 6),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(1, 592, 312, 7),
    FIELDSTG_EVENT_ANIM(11, 1, 7),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(11, 592, 312, 7),
    FIELDSTG_EVENT_WALK(12, 592, 312, 7),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_GOTO_MAP(0xE03, 0, 0, 1),
    FIELDSTG_EVENT_END,
};
u8 D_WSTAG780_800A83C0[2][2][2] = { { { 0x32, 0x34 }, { 0x35, 0x37 } }, { { 0x38, 0x39 }, { 0x38, 0x39 } } };
GamestatePos D_WSTAG780_800A83C8[2][2][2] = {
    { { { 640, 72 }, { 432, 176 } }, { { 0, 224 }, { 0, 0 } } },
    { { { 448, 392 }, { 0, 0 } }, { { 112, 392 }, { 0, 0 } } },
};
WstagAnimKey D_WSTAG780_800A8408[3] = { { 50, 4 }, { 51, 4 }, { 255, 0 } };
WstagAnimKey D_WSTAG780_800A8414[3] = { { 53, 4 }, { 54, 4 }, { 255, 0 } };
Wstag780SparkPlace D_WSTAG780_800A8420[10] = {
    { D_WSTAG780_800A8414, { 20, 130 } }, { D_WSTAG780_800A8414, { 80, 180 } },
    { D_WSTAG780_800A8414, { 120, 230 } }, { D_WSTAG780_800A8414, { 170, 280 } },
    { D_WSTAG780_800A8408, { 350, 210 } }, { D_WSTAG780_800A8408, { 300, 240 } }, { NULL, { 0, 0 } },
    { NULL, { 0, 0 } }, { NULL, { 0, 0 } }, { NULL, { 0, 0 } },
};
WstagAnimKey D_WSTAG780_800A8470[22] = {
    { 10, 12 }, { 11, 12 }, { 12, 12 }, { 13, 12 }, { 14, 12 }, { 15, 20 }, { 16, 4 }, { 17, 4 }, { 15, 4 },
    { 13, 4 }, { 18, 4 }, { 11, 4 }, { 13, 4 }, { 15, 4 }, { 13, 4 }, { 16, 4 }, { 16, 4 }, { 15, 4 }, { 18, 4 },
    { 17, 4 }, { 11, 4 }, { 255, 0 },
};
WstagAnimKey D_WSTAG780_800A84C8[20] = {
    { 18, 12 }, { 19, 12 }, { 20, 12 }, { 21, 12 }, { 22, 12 }, { 23, 20 }, { 24, 4 }, { 25, 4 }, { 22, 4 },
    { 19, 4 }, { 21, 4 }, { 18, 4 }, { 23, 4 }, { 20, 4 }, { 19, 4 }, { 25, 4 }, { 22, 4 }, { 19, 4 }, { 18, 4 },
    { 255, 0 },
};
WstagAnimKey *D_WSTAG780_800A8518[2] = { D_WSTAG780_800A8470, D_WSTAG780_800A84C8 };
FieldstgVramPlace D_WSTAG780_800A8520[83] = {
    { 384, 256, 434, 304, 456, 48, 368, 478 }, { 384, 256, 384, 332, 256, 76, 368, 478 },
    { 384, 256, 394, 332, 296, 76, 368, 478 }, { 384, 256, 404, 344, 336, 88, 368, 478 },
    { 384, 256, 414, 344, 376, 88, 368, 478 }, { 384, 256, 424, 344, 416, 88, 368, 478 },
    { 384, 256, 434, 344, 456, 88, 368, 478 }, { 384, 256, 384, 372, 256, 116, 368, 478 },
    { 384, 256, 394, 372, 296, 116, 368, 478 }, { 384, 256, 404, 384, 336, 128, 368, 478 },
    { 384, 256, 414, 384, 376, 128, 368, 478 }, { 384, 256, 424, 384, 416, 128, 368, 478 },
    { 384, 256, 434, 384, 456, 128, 368, 477 }, { 384, 256, 384, 412, 256, 156, 368, 477 },
    { 384, 256, 394, 412, 296, 156, 368, 477 }, { 384, 256, 404, 424, 336, 168, 368, 477 },
    { 384, 256, 414, 424, 376, 168, 368, 477 }, { 384, 256, 424, 424, 416, 168, 368, 477 },
    { 384, 256, 434, 424, 456, 168, 368, 477 }, { 384, 256, 384, 452, 256, 196, 368, 477 },
    { 384, 256, 394, 452, 296, 196, 368, 477 }, { 384, 256, 404, 464, 336, 208, 368, 477 },
    { 384, 256, 414, 464, 376, 208, 368, 477 }, { 384, 256, 424, 464, 416, 208, 368, 477 },
    { 384, 256, 434, 464, 456, 208, 368, 477 }, { 448, 256, 448, 256, 512, 0, 368, 477 },
    { 448, 256, 458, 256, 552, 0, 368, 476 }, { 448, 256, 468, 256, 592, 0, 368, 476 },
    { 448, 256, 478, 256, 632, 0, 368, 476 }, { 448, 256, 488, 256, 672, 0, 368, 476 },
    { 448, 256, 498, 256, 712, 0, 368, 475 }, { 448, 256, 448, 296, 512, 40, 368, 474 },
    { 448, 256, 458, 296, 552, 40, 368, 474 }, { 448, 256, 468, 296, 592, 40, 368, 474 },
    { 448, 256, 478, 296, 632, 40, 368, 474 }, { 448, 256, 488, 296, 672, 40, 368, 474 },
    { 448, 256, 498, 296, 712, 40, 368, 474 }, { 448, 256, 448, 336, 512, 80, 368, 474 },
    { 320, 256, 362, 472, 168, 216, 368, 478 }, { 448, 256, 506, 336, 744, 80, 368, 478 },
    { 448, 256, 506, 356, 744, 100, 368, 478 }, { 448, 256, 478, 376, 632, 120, 368, 478 },
    { 448, 256, 483, 376, 652, 120, 368, 478 }, { 448, 256, 506, 376, 744, 120, 368, 478 },
    { 448, 256, 478, 396, 632, 140, 368, 478 }, { 448, 256, 483, 396, 652, 140, 368, 478 },
    { 448, 256, 506, 396, 744, 140, 368, 478 }, { 448, 256, 488, 400, 672, 144, 368, 478 },
    { 448, 256, 493, 400, 692, 144, 368, 478 }, { 448, 256, 498, 400, 712, 144, 368, 478 },
    { 448, 256, 464, 404, 576, 148, 368, 477 }, { 448, 256, 469, 404, 596, 148, 368, 477 },
    { 448, 256, 448, 408, 512, 152, 368, 477 }, { 448, 256, 453, 408, 532, 152, 368, 477 },
    { 448, 256, 458, 408, 552, 152, 368, 477 }, { 448, 256, 474, 416, 616, 160, 368, 477 },
    { 448, 256, 479, 416, 636, 160, 368, 477 }, { 448, 256, 503, 416, 732, 160, 368, 477 },
    { 448, 256, 484, 420, 656, 164, 368, 477 }, { 448, 256, 489, 420, 676, 164, 368, 477 },
    { 448, 256, 494, 420, 696, 164, 368, 477 }, { 448, 256, 463, 424, 572, 168, 368, 477 },
    { 448, 256, 468, 424, 592, 168, 368, 477 }, { 448, 256, 448, 428, 512, 172, 368, 477 },
    { 448, 256, 453, 428, 532, 172, 368, 476 }, { 448, 256, 458, 428, 552, 172, 368, 476 },
    { 448, 256, 473, 436, 612, 180, 368, 476 }, { 448, 256, 478, 436, 632, 180, 368, 476 },
    { 448, 256, 499, 436, 716, 180, 368, 475 }, { 448, 256, 504, 436, 736, 180, 368, 474 },
    { 448, 256, 483, 440, 652, 184, 368, 474 }, { 448, 256, 488, 440, 672, 184, 368, 474 },
    { 448, 256, 493, 440, 692, 184, 368, 474 }, { 448, 256, 463, 444, 572, 188, 368, 474 },
    { 448, 256, 468, 444, 592, 188, 368, 474 }, { 448, 256, 448, 448, 512, 192, 368, 474 },
    { 448, 256, 453, 448, 532, 192, 368, 473 }, { 448, 256, 458, 448, 552, 192, 368, 473 },
    { 448, 256, 473, 456, 612, 200, 368, 473 }, { 448, 256, 478, 456, 632, 200, 368, 473 },
    { 384, 256, 416, 256, 384, 0, 368, 502 }, { 384, 256, 404, 256, 336, 0, 368, 502 },
    { 448, 256, 471, 376, 604, 120, 368, 500 },
};
Wstag780Quad D_WSTAG780_800A8A50[40] = {
    { 121, 76, 168, 53, 121, 123, 168, 100, 80 }, { 121, 77, 160, 58, 121, 116, 160, 97, 0 },
    { 272, 92, 305, 108, 272, 123, 305, 139, 81 }, { 277, 96, 304, 109, 277, 122, 304, 135, 0 },
    { 29, 53, 56, 66, 29, 81, 56, 94, 81 }, { 33, 59, 54, 69, 33, 81, 54, 91, 0 },
    { 60, 68, 87, 81, 60, 97, 87, 110, 81 }, { 64, 74, 85, 84, 64, 96, 85, 106, 0 },
    { 60, 99, 87, 112, 60, 128, 87, 141, 81 }, { 64, 105, 85, 115, 64, 127, 85, 137, 0 },
    { 29, 84, 56, 97, 29, 112, 56, 125, 81 }, { 33, 90, 54, 100, 33, 112, 54, 122, 0 },
    { 335, 109, 351, 101, 335, 123, 351, 115, 82 }, { 335, 109, 347, 103, 335, 119, 347, 113, 1 },
    { 351, 101, 367, 93, 351, 115, 367, 107, 82 }, { 351, 101, 363, 95, 351, 111, 363, 105, 1 },
    { 367, 93, 383, 85, 367, 107, 383, 99, 82 }, { 367, 93, 379, 87, 367, 103, 379, 97, 1 },
    { 383, 85, 399, 77, 383, 99, 399, 91, 82 }, { 383, 85, 395, 79, 383, 95, 395, 89, 1 },
    { 335, 125, 351, 117, 335, 139, 351, 131, 82 }, { 335, 125, 347, 119, 335, 135, 347, 129, 1 },
    { 351, 117, 367, 109, 351, 131, 367, 123, 82 }, { 351, 117, 363, 111, 351, 127, 363, 121, 1 },
    { 367, 109, 383, 101, 367, 123, 383, 115, 82 }, { 367, 109, 379, 103, 367, 119, 379, 113, 1 },
    { 383, 101, 399, 93, 383, 115, 399, 107, 82 }, { 383, 101, 395, 95, 383, 111, 395, 105, 1 },
    { 148, 91, 164, 83, 148, 105, 164, 97, 82 }, { 148, 91, 160, 85, 148, 101, 160, 95, 1 },
    { 164, 83, 180, 75, 164, 97, 180, 89, 82 }, { 164, 83, 176, 77, 164, 93, 176, 87, 1 },
    { 180, 75, 196, 67, 180, 89, 196, 81, 82 }, { 180, 75, 192, 69, 180, 85, 192, 79, 1 },
    { 148, 107, 164, 99, 148, 121, 164, 113, 82 }, { 148, 107, 160, 101, 148, 117, 160, 111, 1 },
    { 164, 99, 180, 91, 164, 113, 180, 105, 82 }, { 164, 99, 176, 93, 164, 109, 176, 103, 1 },
    { 180, 91, 196, 83, 180, 105, 196, 97, 82 }, { 180, 91, 192, 85, 180, 101, 192, 95, 1 },
};
Wstag780ScreenKey D_WSTAG780_800A8D20[14] = {
    { 0, 4 }, { 1, 4 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 4 }, { 9, 4 }, { 0xA, 4 },
    { 0xB, 4 }, { 0, 0 }, { 0, 0 },
};
Wstag780ScreenKey D_WSTAG780_800A8D3C[8] = {
    { 0x1F, 8 }, { 0x20, 8 }, { 0x21, 8 }, { 0x22, 8 }, { 0x23, 8 }, { 0x24, 8 }, { 0x25, 8 }, { 0, 0 },
};
Wstag780ScreenKey D_WSTAG780_800A8D4C[32] = {
    { 0xC, 0xA }, { 0xD, 4 }, { 0xE, 4 }, { 0xF, 4 }, { 0x10, 4 }, { 0x11, 4 }, { 0x12, 4 }, { 0x13, 8 },
    { 0x14, 8 }, { 0x13, 8 }, { 0x14, 8 }, { 0x13, 8 }, { 0x14, 8 }, { 0x13, 4 }, { 0x15, 8 }, { 0x16, 4 },
    { 0x17, 4 }, { 0x18, 4 }, { 0x19, 4 }, { 0x1A, 0x1A }, { 0x1A, 0x1A }, { 0x1B, 0x1A }, { 0x1A, 0xC },
    { 0x1C, 0xC }, { 0x1D, 0xC }, { 0x1D, 0xC }, { 0x1E, 0xC }, { 0x1D, 0xC }, { 0x1C, 0xC }, { 0x1A, 0xC },
    { 0x13, 0 }, { 0, 0 },
};
Wstag780ScreenKey D_WSTAG780_800A8D8C[14] = {
    { 0x26, 4 }, { 0x27, 4 }, { 0x28, 4 }, { 0x29, 4 }, { 0x2A, 4 }, { 0x2B, 4 }, { 0x2C, 4 }, { 0x2D, 4 },
    { 0x2E, 4 }, { 0x2F, 4 }, { 0x30, 4 }, { 0x31, 4 }, { 0, 0 }, { 0, 0 },
};
Wstag780ScreenKey D_WSTAG780_800A8DA8[8] = {
    { 0x45, 4 }, { 0x46, 4 }, { 0x47, 4 }, { 0x48, 4 }, { 0x49, 4 }, { 0x4A, 4 }, { 0x4B, 4 }, { 0, 0 },
};
Wstag780ScreenKey D_WSTAG780_800A8DB8[46] = {
    { 0x32, 0xA }, { 0x33, 4 }, { 0x34, 4 }, { 0x35, 4 }, { 0x36, 4 }, { 0x37, 4 }, { 0x38, 4 }, { 0x39, 8 },
    { 0x3A, 8 }, { 0x39, 8 }, { 0x3A, 8 }, { 0x39, 8 }, { 0x3A, 8 }, { 0x39, 4 }, { 0x3B, 8 }, { 0x3C, 4 },
    { 0x3D, 4 }, { 0x3E, 4 }, { 0x3F, 4 }, { 0x40, 0x1A }, { 0x40, 0x1A }, { 0x41, 0x1A }, { 0x40, 0xC },
    { 0x42, 0xC }, { 0x43, 0xC }, { 0x43, 0xC }, { 0x44, 0xC }, { 0x43, 0xC }, { 0x42, 0xC }, { 0x40, 0xC },
    { 0x4C, 0x78 }, { 0x4D, 0x78 }, { 0x40, 0x1A }, { 0x40, 0x1A }, { 0x41, 0x1A }, { 0x40, 0xC }, { 0x42, 0xC },
    { 0x43, 0xC }, { 0x43, 0xC }, { 0x44, 0xC }, { 0x43, 0xC }, { 0x42, 0xC }, { 0x40, 0xC }, { 0x4E, 0x78 },
    { 0x4F, 0x78 }, { 0x13, 0 },
};
Wstag780ScreenKey *D_WSTAG780_800A8E14[7] = {
    D_WSTAG780_800A8D20, D_WSTAG780_800A8D3C, D_WSTAG780_800A8D4C, D_WSTAG780_800A8D8C, D_WSTAG780_800A8DA8,
    D_WSTAG780_800A8DB8, NULL,
};
FieldstgVramPlace wstag780_vram_places[20] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 374, 436, 216, 180, 368, 492 }, { 320, 256, 354, 472, 136, 216, 368, 491 },
    { 448, 256, 490, 336, 680, 80, 368, 490 }, { 384, 256, 438, 256, 472, 0, 368, 489 },
    { 320, 256, 334, 394, 56, 138, 368, 488 }, { 448, 256, 458, 336, 552, 80, 368, 487 },
    { 448, 256, 466, 336, 584, 80, 368, 486 }, { 448, 256, 474, 336, 616, 80, 368, 485 },
    { 448, 256, 482, 336, 648, 80, 368, 484 }, { 448, 256, 498, 336, 712, 80, 368, 483 },
    { 448, 256, 490, 368, 680, 112, 368, 482 }, { 448, 256, 498, 368, 712, 112, 368, 481 },
    { 448, 256, 448, 376, 512, 120, 368, 480 }, { 448, 256, 456, 376, 544, 120, 368, 479 },
};
FieldstgTalk D_WSTAG780_800A8F70[3] = { { NULL, NULL, 1 }, { NULL, NULL, 1 }, { NULL, NULL, 0 } };
u16 D_WSTAG780_800A8F94[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8F9C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FA4[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FAC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FB4[4] = { 0x6000, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FBC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FC4[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FCC[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FD4[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FDC[4] = { 0x6027, 1, 0xFFFF, 0 };
u16 D_WSTAG780_800A8FE4[4] = { 0x6027, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG780_800A8FEC = { D_WSTAG780_800A8F94, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9000 = { D_WSTAG780_800A8F9C, NULL, 1, 4, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9014 = { D_WSTAG780_800A8FA4, NULL, 11, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9028 = { D_WSTAG780_800A8FAC, NULL, 11, 5, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A903C = { D_WSTAG780_800A8FB4, NULL, 12, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9050 = { D_WSTAG780_800A8FBC, NULL, 12, 6, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9064 = { NULL, D_WSTAG780_800A8F70, 16, 7, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9078 = { NULL, NULL, 45, 8, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A908C = { NULL, NULL, 47, 9, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A90A0 = { NULL, NULL, 49, 10, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A90B4 = { NULL, NULL, 53, 11, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A90C8 = { NULL, NULL, 56, 12, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A90DC = { D_WSTAG780_800A8FC4, NULL, 157, 13, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A90F0 = { D_WSTAG780_800A8FCC, NULL, 158, 14, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9104 = { D_WSTAG780_800A8FD4, NULL, 272, 15, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A9118 = { D_WSTAG780_800A8FDC, NULL, 273, 16, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG780_800A912C = { D_WSTAG780_800A8FE4, NULL, 274, 17, 0, 0, 1 };
FieldstgPlacedActor *wstag780_actors[18] = {
    &D_WSTAG780_800A8FEC, &D_WSTAG780_800A9000, &D_WSTAG780_800A9014, &D_WSTAG780_800A9028, &D_WSTAG780_800A903C,
    &D_WSTAG780_800A9050, &D_WSTAG780_800A9064, &D_WSTAG780_800A9078, &D_WSTAG780_800A908C, &D_WSTAG780_800A90A0,
    &D_WSTAG780_800A90B4, &D_WSTAG780_800A90C8, &D_WSTAG780_800A90DC, &D_WSTAG780_800A90F0, &D_WSTAG780_800A9104,
    &D_WSTAG780_800A9118, &D_WSTAG780_800A912C, NULL,
};
FieldstgSprite wstag780_sprites[7] = {
    { 0, 1, 0x40, 2, 0xA, 0, 0, 0, 0, 0, 198, 179, 0, 0 }, { 0, 2, 0x40, 2, 0x12, 0, 0, 0, 0, 0, 342, 251, 0, 0 },
    { 0, 0, 0x80, 6, 0x32, 0, 0, 0, 0, 0, 363, 206, 0, 0 }, { 0, 0, 0x80, 6, 0x35, 0, 0, 0, 0, 0, 136, 235, 0, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 190, 156, 231, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 346, 229, 303, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag780_funcs = { wstag780_setup };
FieldstgEventDef wstag780_events[7] = {
    { 0, D_WSTAG780_800A7798, 0x014A0000, NULL, wstag780_event_0_end },
    { 684, D_WSTAG780_800A7B4C, 0x014A0020, NULL, NULL }, { 742, D_WSTAG780_800A7C74, 0x014A0021, NULL, NULL },
    { 885, D_WSTAG780_800A7DAC, 0x014A0022, NULL, wstag780_event_885_end },
    { 971, D_WSTAG780_800A7E04, 0x014A0023, NULL, NULL },
    { 1500, D_WSTAG780_800A8014, 0x014A0024, NULL, wstag780_event_1500_end }, { -1, NULL, 0, NULL, NULL },
};
