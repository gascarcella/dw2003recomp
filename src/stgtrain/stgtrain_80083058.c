#include "stgtrain.h"
#include "psyq/libgpu.h"

/* STGTRAIN.PRO (training at a gym): an animated sprite, the overlay's first object, the training screen (a party
 * member's stats and the trainer) and the fade. */

/* Each party Digimon's animation on the screen: 7 frames per Digimon, -1 ends early. */
extern s32 stgtrain_member_anims[][7];

void stgtrain_sprite_set_data(StgtrainSprite *obj, u8 *data, s32 offset) {
    obj->data = data;
    obj->offset = offset;
    obj->unk_84 = ((u16 *)data)[1];
    obj->unk_86 = ((u16 *)data)[2];
}

void stgtrain_sprite_set_anim(StgtrainSprite *obj, StgtrainSpriteAnim *anim) {
    if (anim != NULL) {
        obj->anim = anim;
        obj->frame = 0;
        obj->time = 0;
        obj->frames = anim->count;
        obj->done = 0;
    }
}

void stgtrain_sprite_set_layer(StgtrainSprite *obj, s32 id, s32 depth) {
    obj->layer_id = id;
    obj->ot_depth = depth;
}

void stgtrain_sprite_set_pos(StgtrainSprite *obj, s32 x, s32 y) {
    obj->x = x;
    obj->y = y;
}

void stgtrain_sprite_set_tex(StgtrainSprite *obj, s32 x, s32 y) {
    obj->tex_x = x;
    obj->tex_y = y;
}

void stgtrain_sprite_set_clut(StgtrainSprite *obj, s32 x, s32 y) {
    obj->clut_x = x;
    obj->clut_y = y;
}

void stgtrain_sprite_set_scale(StgtrainSprite *obj, s32 x, s32 y, s32 z) {
    obj->scale.vx = x;
    obj->scale.vy = y;
    obj->scale.vz = z;
    obj->transform = 1;
}

void stgtrain_sprite_set_pivot(StgtrainSprite *obj, s32 x, s32 y) {
    obj->pivot_x = x;
    obj->pivot_y = y;
}

void stgtrain_sprite_set_rotation(StgtrainSprite *obj, s16 x, s16 y, s16 z) {
    obj->rotation.vx = x;
    obj->rotation.vy = y;
    obj->rotation.vz = z;
    obj->transform = 1;
}

s32 stgtrain_sprite_is_done(StgtrainSprite *obj) {
    return obj->done & 0x7FFFFFFF;
}

void stgtrain_sprite_pause(StgtrainSprite *obj, s32 paused) {
    if (paused) {
        obj->done |= 0x80000000;
    } else {
        obj->done &= 0x7FFFFFFF;
    }
}

/* A part of a sprite frame: a rectangle of the texture. */
typedef struct StgtrainSpritePart {
    /* 0x00 */ u8 u;
    /* 0x01 */ u8 v;
    /* 0x02 */ u8 x;
    /* 0x03 */ u8 y;
    /* 0x04 */ u16 unk_4; /* bits 6-14: CLUT row; bit 15: semi-transparent */
    /* 0x06 */ u16 unk_6; /* bits 0-4: CLUT column (x16); bits 5-8: texture page bits */
    /* 0x08 */ u16 w;
    /* 0x0A */ u16 h;
    /* 0x0C */ u8 unk_C[8];
} StgtrainSpritePart; /* size 0x14 */

void stgtrain_sprite_update(StgtrainSprite *obj);

/* Plays the animation and draws the frame's parts, last to first, as sprites or, when rotated or scaled (about the
 * pivot), as textured quads. Shape ported from the US decomp (func_800828E8). */
void stgtrain_sprite_update(StgtrainSprite *obj) {
    SVECTOR out;
    SVECTOR pts[4];
    StgtrainSpriteFrame *frame;
    u8 *list;
    StgtrainSpritePart *part;
    s32 n;
    s32 count;
    s32 i;
    s32 j;
    s32 k;
    s32 state;
    u16 info;
    u16 w;
    u16 h;
    u16 tpage;
    u16 prev;
    u16 clut;
    s32 transform;
    u8 u;
    u8 v;
    s32 x;
    s32 y;
    void *prim;

    state = obj->base.state;
    switch (state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->time = gfx_module.funcs.get_time();
        break;
    case OBJECT_STATE_RUN:
        if (obj->data == NULL) {
            break;
        }
        if (obj->anim == NULL) {
            break;
        }
        frame = obj->anim->frames;
        frame += obj->frame;
        if (obj->done >= 0 && gfx_module.funcs.get_time() - obj->time > frame->duration) {
            obj->time = gfx_module.funcs.get_time();
            if (++obj->frame >= obj->frames - 1) {
                obj->frame = obj->frames - 1;
                obj->done = 1;
            }
            /* FAKE: frame first holds the animation, then its frames, then the frame (the US decomp's shape). The
             * original loads obj->anim straight into the spilled `frame` and steps it in place (reload register
             * t7); `frame = obj->anim->frames + obj->frame` or `&...frames[obj->frame]` build it in v1 (99.48%). */
            frame = (StgtrainSpriteFrame *)obj->anim;
            frame = ((StgtrainSpriteAnim *)frame)->frames;
            frame += obj->frame;
        }
        list = obj->data;
        list += obj->offset;
        count = frame->list;
        for (i = 0; i < count; i++) {
            n = *(s32 *)list;
            list += sizeof(s32);
            for (j = 0; j < n; j++) {
                list += sizeof(StgtrainSpritePart);
            }
        }
        tpage = 0;
        prev = 0;
        transform = 0;
        if (obj->transform) {
            if (obj->scale.vx == 0 && obj->scale.vy == 0) {
                break;
            }
            if (obj->scale.vx == 0x1000 && obj->scale.vy == 0x1000) {
                obj->transform = 0;
            } else {
                transform = 1;
                RotMatrixYXZ_gte(&obj->rotation, &obj->matrix);
                ScaleMatrix(&obj->matrix, &obj->scale);
            }
        }
        obj->layer = gfx_module.funcs.get_layer(obj->layer_id);
        obj->ot = obj->layer->get_ot_entry(obj->layer, obj->ot_depth);
        prim = gfx_module.funcs.get_packet();
        n = *(s32 *)list;
        list += sizeof(s32);
        for (i = 0; i < n; i++) {
            list += sizeof(StgtrainSpritePart);
        }
        part = (StgtrainSpritePart *)list;
        for (i = 0; i < n; i++) {
            part--;
            info = part->unk_6;
            clut = getClut(obj->clut_x + (info & 0x1F) * 16, obj->clut_y + ((part->unk_4 & 0x7FC0) >> 6));
            tpage = getTPage(info >> 7, info >> 5, obj->tex_x + (info & 0x1F) * 64, obj->tex_y);
            u = part->u;
            x = part->x;
            y = part->y;
            v = part->v;
            w = part->w;
            h = part->h;
            if (!transform) {
                if (i == 0) {
                    prev = tpage;
                }
                if (prev != tpage) {
                    SetDrawTPage(prim, 0, 1, prev);
                    addPrim(obj->ot, prim);
                    prim = (DR_TPAGE *)prim + 1;
                    prev = tpage;
                }
                setSprt((SPRT *)prim);
                if ((s16)part->unk_4 & 0x8000) {
                    setSemiTrans((SPRT *)prim, 1);
                }
                setRGB0((SPRT *)prim, 0x80, 0x80, 0x80);
                ((SPRT *)prim)->x0 = x + (frame->x + obj->x);
                ((SPRT *)prim)->y0 = y + (frame->y + obj->y);
                ((SPRT *)prim)->u0 = u;
                ((SPRT *)prim)->v0 = v;
                ((SPRT *)prim)->w = w;
                ((SPRT *)prim)->h = h;
                ((SPRT *)prim)->clut = clut;
                addPrim(obj->ot, prim);
                prim = (SPRT *)prim + 1;
            } else {
                setPolyFT4((POLY_FT4 *)prim);
                if ((s16)part->unk_4 & 0x8000) {
                    setSemiTrans((POLY_FT4 *)prim, 1);
                }
                setRGB0((POLY_FT4 *)prim, 0x80, 0x80, 0x80);
                pts[0].vx = pts[2].vx = x + (frame->x + obj->x) - obj->pivot_x;
                pts[1].vx = pts[3].vx = pts[0].vx + w;
                pts[0].vy = pts[1].vy = y + (frame->y + obj->y) - obj->pivot_y;
                pts[2].vy = pts[3].vy = pts[0].vy + h;
                pts[0].vz = pts[1].vz = pts[2].vz = pts[3].vz = 0;
                for (k = 0; k < 4; k++) {
                    ApplyMatrixSV(&obj->matrix, &pts[k], &out);
                    (&((POLY_FT4 *)prim)->x0)[k * 4] = out.vx + obj->pivot_x;
                    (&((POLY_FT4 *)prim)->y0)[k * 4] = out.vy + obj->pivot_y;
                }
                ((POLY_FT4 *)prim)->u0 = ((POLY_FT4 *)prim)->u2 = u;
                ((POLY_FT4 *)prim)->u1 = ((POLY_FT4 *)prim)->u3 = w + u - 1;
                ((POLY_FT4 *)prim)->v0 = ((POLY_FT4 *)prim)->v1 = v;
                ((POLY_FT4 *)prim)->v2 = ((POLY_FT4 *)prim)->v3 = h + v - 1;
                ((POLY_FT4 *)prim)->tpage = tpage;
                ((POLY_FT4 *)prim)->clut = clut;
                addPrim(obj->ot, prim);
                prim = (POLY_FT4 *)prim + 1;
            }
        }
        if (!transform) {
            SetDrawTPage(prim, 0, 1, tpage);
            addPrim(obj->ot, prim);
            prim = (DR_TPAGE *)prim + 1;
        }
        gfx_module.funcs.set_packet(prim);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgtrainSprite *stgtrain_sprite_create(void) {
    StgtrainSprite *obj = object_new(stgtrain_sprite_update, sizeof(StgtrainSprite), 0);

    obj->set_data = stgtrain_sprite_set_data;
    obj->set_anim = stgtrain_sprite_set_anim;
    obj->set_pos = stgtrain_sprite_set_pos;
    obj->set_tex = stgtrain_sprite_set_tex;
    obj->set_layer = stgtrain_sprite_set_layer;
    obj->set_clut = stgtrain_sprite_set_clut;
    obj->set_scale = stgtrain_sprite_set_scale;
    obj->set_pivot = stgtrain_sprite_set_pivot;
    obj->set_rotation = stgtrain_sprite_set_rotation;
    obj->is_done = stgtrain_sprite_is_done;
    obj->pause = stgtrain_sprite_pause;
    return obj;
}

StgtrainMain *stgtrain_main_create(void);

/* The overlay's first object: sets up the display (a full-screen layer, a 168x88 one at 123,83 above it, and a
 * full-screen one on top) and creates the training screen. */
void stgtrain_update_main(Object *obj, StgtrainMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x14000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        rect.x = 123;
        rect.y = 83;
        rect.w = 168;
        rect.h = 88;
        gfx_module.funcs.create_layer(&rect, 3, 0x1001);
        gfx_module.funcs.move_layer(0x1001, 0x1000, 1);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        gfx_module.funcs.create_layer(&rect, 3, 0x1002);
        gfx_module.funcs.move_layer(0x1002, 0x1001, 1);
        *data = stgtrain_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stgtrain_start(void) {
    return object_new(stgtrain_update_main, sizeof(Object), 4);
}

/* The training screen's data block (0x80 bytes): its text windows and the objects it creates. */
typedef struct StgtrainMainData {
    /* 0x00 */ MessageWindow *name;
    /* 0x04 */ MessageWindow *level[2];  /* label, value */
    /* 0x0C */ MessageWindow *hp[3];     /* label, value, max */
    /* 0x18 */ MessageWindow *mp[3];     /* label, value, max */
    /* 0x24 */ MessageWindow *slash[2];  /* between the values and the maxima */
    /* 0x2C */ MessageWindow *stats[6];  /* stats 6..11 */
    /* 0x44 */ MessageWindow *resists[7]; /* stats 12..18 */
    /* 0x60 */ MessageWindow *tp[2]; /* label, stat 1 */
    /* 0x68 */ MessageWindow *back_hint;
    /* 0x6C */ MessageWindow *question;
    /* 0x70 */ StgtrainChoice *choice;
    /* 0x74 */ StgtrainLevel *level_menu;
    /* 0x78 */ StgtrainSession *session;
    /* 0x7C */ Fade *fade;
} StgtrainMainData; /* size 0x80 */

void stgtrain_main_create_windows(StgtrainMain *obj, StgtrainMainData *data) {
    s32 i;

    data->name = message_create_window(obj->layer, 1, 0x33, 0x13);
    data->level[0] = message_create_window(obj->layer, 3, 0x33, 0x22);
    data->level[1] = message_create_window(obj->layer, 3, 0x50, 0x22);
    data->hp[0] = message_create_window(obj->layer, 3, 0x33, 0x2C);
    data->hp[1] = message_create_window(obj->layer, 3, 0x5D, 0x2C);
    data->hp[2] = message_create_window(obj->layer, 3, 0x80, 0x2C);
    data->mp[0] = message_create_window(obj->layer, 3, 0x33, 0x35);
    data->mp[1] = message_create_window(obj->layer, 3, 0x5D, 0x35);
    data->mp[2] = message_create_window(obj->layer, 3, 0x80, 0x35);
    data->slash[0] = message_create_window(obj->layer, 3, 0x5F, 0x2C);
    data->slash[1] = message_create_window(obj->layer, 3, 0x5F, 0x35);
    for (i = 0; i < 6; i++) {
        data->stats[i] = message_create_window(obj->layer, 1, 0x33, 0x4F + i * 14);
    }
    for (i = 0; i < 7; i++) {
        data->resists[i] = message_create_window(obj->layer, 1, 0x5D, 0x4F + i * 14);
    }
    data->tp[0] = message_create_window(obj->layer, 1, 0x10, 0xBB);
    data->tp[1] = message_create_window(obj->layer, 1, 0x2C, 0xBB);
    data->back_hint = message_create_window(obj->layer, 1, 0xA1, 0x17);
    data->question = message_create_window(obj->layer, 1, 0xAE, 0x49);
}

/* Shows (or hides) the member's name, level, HP and MP. */
void stgtrain_main_show_status(StgtrainMain *obj, StgtrainMainData *data, s32 show) {
    GamestateStats stats;
    GamestateRecord *digimon;
    s32 id;
    s32 i;

    if (show) {
        id = gamestate_data.funcs.get_party_member(obj->member);
        digimon = gamestate_data.funcs.get_record(id);
        gamestate_data.funcs.get_stats(id, &stats);
        data->name->set_text(data->name, (u8 *)digimon, -1);
        data->level[0]->set_text(data->level[0], cdload_module.files.get_file(records_language + 0x10B), 1);
        data->level[1]->set_line_number(data->level[1], 0, stats.values[0]);
        data->level[1]->measure(data->level[1], 1);
        data->hp[0]->set_text(data->hp[0], cdload_module.files.get_file(records_language + 0x10B), 2);
        data->hp[1]->set_line_number(data->hp[1], 0, stats.values[2]);
        data->hp[2]->set_line_number(data->hp[2], 0, stats.values[3]);
        for (i = 1; i < 3; i++) {
            data->hp[i]->set_palette(data->hp[i], 0);
            data->hp[i]->measure(data->hp[i], 1);
        }
        data->mp[0]->set_text(data->mp[0], cdload_module.files.get_file(records_language + 0x10B), 3);
        data->mp[1]->set_line_number(data->mp[1], 0, stats.values[4]);
        data->mp[2]->set_line_number(data->mp[2], 0, stats.values[5]);
        for (i = 1; i < 3; i++) {
            data->mp[i]->set_palette(data->mp[i], 0);
            data->mp[i]->measure(data->mp[i], 1);
        }
        for (i = 0; i < 2; i++) {
            data->slash[i]->set_text(data->slash[i], cdload_module.files.get_file(records_language + 0x10B), 0x43);
        }
    } else {
        data->name->set_visible(data->name, 0);
        for (i = 0; i < 2; i++) {
            data->level[i]->set_visible(data->level[i], 0);
            data->slash[i]->set_visible(data->slash[i], 0);
        }
        for (i = 0; i < 3; i++) {
            data->hp[i]->set_visible(data->hp[i], 0);
        }
        for (i = 0; i < 3; i++) {
            data->mp[i]->set_visible(data->mp[i], 0);
        }
    }
}

/* Shows (or hides) the member's other stats; those lowered (unk_26) in another colour. */
void stgtrain_main_show_stats(StgtrainMain *obj, StgtrainMainData *data, s32 show) {
    GamestateStats stats;
    s32 i;

    if (show) {
        gamestate_data.funcs.get_stats(gamestate_data.funcs.get_party_member(obj->member), &stats);
        for (i = 0; i < 6; i++) {
            data->stats[i]->set_line_number(data->stats[i], 0, stats.stats[i]);
            data->stats[i]->measure(data->stats[i], 1);
            data->stats[i]->set_palette(data->stats[i], 0);
        }
        if (stats.penalties[0] != 0) {
            data->stats[0]->set_palette(data->stats[0], 6);
        }
        if (stats.penalties[1] != 0) {
            data->stats[1]->set_palette(data->stats[1], 6);
        }
        if (stats.penalties[2] != 0) {
            data->stats[4]->set_palette(data->stats[4], 6);
        }
        for (i = 0; i < 7; i++) {
            data->resists[i]->set_line_number(data->resists[i], 0, stats.resists[i]);
            data->resists[i]->measure(data->resists[i], 1);
            data->resists[i]->set_palette(data->resists[i], 0);
        }
    } else {
        for (i = 0; i < 6; i++) {
            data->stats[i]->set_visible(data->stats[i], 0);
        }
        for (i = 0; i < 7; i++) {
            data->resists[i]->set_visible(data->resists[i], 0);
        }
    }
}

void stgtrain_main_show_tp(StgtrainMain *obj, StgtrainMainData *data, s32 show) {
    GamestateStats stats;
    s32 i;

    if (show) {
        gamestate_data.funcs.get_stats(gamestate_data.funcs.get_party_member(obj->member), &stats);
        data->tp[0]->set_text(data->tp[0], cdload_module.files.get_file(records_language + 0x10B), 4);
        data->tp[1]->set_line_number(data->tp[1], 0, stats.values[1]);
        data->tp[1]->measure(data->tp[1], 1);
    } else {
        for (i = 0; i < 2; i++) {
            data->tp[i]->set_visible(data->tp[i], 0);
        }
    }
}

/* Shows the member's stats after a training, those that changed from `old` in colour 1 (up) or 5 (down);
 * `old` NULL just shows them (StgtrainMain.show_change). */
void stgtrain_main_show_change(StgtrainMain *obj, GamestateStats *old) {
    GamestateStats stats;
    StgtrainMainData *data = (StgtrainMainData *)obj->base.children;
    s32 id;
    s32 i;

    if (old == NULL) {
        stgtrain_main_show_status(obj, data, 1);
        stgtrain_main_show_stats(obj, data, 1);
        return;
    }
    id = gamestate_data.funcs.get_party_member(obj->member);
    gamestate_data.funcs.get_record(id);
    gamestate_data.funcs.get_stats(id, &stats);
    data->hp[0]->set_text(data->hp[0], cdload_module.files.get_file(records_language + 0x10B), 2);
    data->hp[1]->set_line_number(data->hp[1], 0, stats.values[2]);
    data->hp[2]->set_line_number(data->hp[2], 0, stats.values[3]);
    for (i = 1; i < 3; i++) {
        data->hp[i]->measure(data->hp[i], 1);
    }
    if (old->values[3] < stats.values[3]) {
        data->hp[2]->set_palette(data->hp[2], 1);
    } else if (stats.values[3] < old->values[3]) {
        data->hp[2]->set_palette(data->hp[2], 5);
    } else {
        data->hp[2]->set_palette(data->hp[2], 0);
    }
    /* The window passed is data->mp[3], i.e. slash[0] (i is 3 after the loop): an original slip. */
    data->mp[0]->set_text(data->mp[i], cdload_module.files.get_file(records_language + 0x10B), 3);
    data->mp[1]->set_line_number(data->mp[1], 0, stats.values[4]);
    data->mp[2]->set_line_number(data->mp[2], 0, stats.values[5]);
    for (i = 1; i < 3; i++) {
        data->mp[i]->measure(data->mp[i], 1);
    }
    if (old->values[5] < stats.values[5]) {
        data->mp[2]->set_palette(data->mp[2], 1);
    } else if (stats.values[5] < old->values[5]) {
        data->mp[2]->set_palette(data->mp[2], 5);
    } else {
        data->mp[2]->set_palette(data->mp[2], 0);
    }
    for (i = 0; i < 2; i++) {
        data->slash[i]->set_text(data->slash[i], cdload_module.files.get_file(records_language + 0x10B), 0x43);
    }
    if (stats.penalties[0] != 0) {
        data->stats[0]->set_palette(data->stats[0], 6);
    }
    if (stats.penalties[1] != 0) {
        data->stats[1]->set_palette(data->stats[1], 6);
    }
    if (stats.penalties[2] != 0) {
        data->stats[4]->set_palette(data->stats[4], 6);
    }
    for (i = 0; i < 6; i++) {
        data->stats[i]->set_line_number(data->stats[i], 0, *(stats.stats + i));
        data->stats[i]->measure(data->stats[i], 1);
        if (old->stats[i] < *(stats.stats + i)) {
            data->stats[i]->set_palette(data->stats[i], 1);
        } else if (*(stats.stats + i) < old->stats[i]) {
            data->stats[i]->set_palette(data->stats[i], 5);
        }
    }
    for (i = 0; i < 7; i++) {
        data->resists[i]->set_line_number(data->resists[i], 0, *(stats.resists + i));
        data->resists[i]->measure(data->resists[i], 1);
        if (old->resists[i] < *(stats.resists + i)) {
            data->resists[i]->set_palette(data->resists[i], 1);
        } else if (*(stats.resists + i) < old->resists[i]) {
            data->resists[i]->set_palette(data->resists[i], 5);
        }
    }
}

/* Draws the screen: the background, the panels as their window animations open them, the party Digimon, the
 * arrows and the trainer. */
void stgtrain_main_draw(StgtrainMain *obj) {
    Sprite spr;
    s32 i;
    s32 digimon;

    for (i = 0; i < obj->members; i++) {
        if (gfx_module.funcs.get_time() - obj->member_anims[i].time >= 13) {
            obj->member_anims[i].time = gfx_module.funcs.get_time();
            obj->member_anims[i].frame++;
            digimon = gamestate_data.funcs.get_party_member(i);
            if (obj->member_anims[i].frame >= 7 || stgtrain_member_anims[digimon][obj->member_anims[i].frame] == -1) {
                obj->member_anims[i].frame = 0;
            }
        }
    }
    digimon = gamestate_data.funcs.get_party_member(obj->member);
    if (gfx_module.funcs.get_time() - obj->time >= 13) {
        obj->time = gfx_module.funcs.get_time();
        obj->frame++;
        if (obj->frame >= 7 || stgtrain_member_anims[digimon][obj->frame] == -1) {
            obj->frame = 0;
        }
    }
    sprite_init(&spr);
    spr.set_layer_id(obj->layer, obj->ot_depth);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x2B);
        } else {
            spr.set_vram_pos(0x240, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), stgtrain_member_anims[digimon][obj->frame], 0x10, 0x16);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xE, 0, 0xF);
        spr.set_vram_pos(0x240, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x1F, 0, 0xF);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0x7F);
        }
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xF, 0, 0x4B);
        spr.set_vram_pos(0x240, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x20, 0, 0x4B);
    }
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xC1);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x21, 0, 0xB6);
    }
    if (obj->anims[5].level != 0) {
        for (i = 0; i < obj->members; i++) {
            if (obj->anims[5].level != 0x1000) {
                spr.set_scale(obj->anims[5].level, 0x1000, 0x1000);
                spr.set_pivot(0xB4 + i * 0x30, 0x8A);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), stgtrain_member_anims[digimon][obj->member_anims[i].frame], 0xA4 + i * 0x30,
                       0x81);
        }
    }
    if (obj->cursor != 0) {
        if (gfx_module.funcs.get_time() - obj->cursor_time >= 11) {
            obj->cursor_time = gfx_module.funcs.get_time();
            obj->cursor_frame++;
            if (obj->cursor_frame >= 4) {
                obj->cursor_frame = 0;
            }
        }
        spr.set_layer_id(obj->layer, obj->ot_depth - 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_palette(obj->cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xD, obj->member * 0x30 + 0xA4, 0x65);
        spr.set_palette(0);
    }
    if (obj->anims[6].level != 0) {
        if (obj->anims[6].level != 0x1000) {
            spr.set_scale(obj->anims[6].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x8A);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_layer_id(obj->layer, obj->ot_depth - 1);
        spr.set_vram_pos(0x140, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x10, 0x8F, 0x5F);
        spr.set_layer_id(obj->layer, obj->ot_depth);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x11, 0x8F, 0x5F);
        spr.set_vram_pos(0x240, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x24, 0x8F, 0x5F);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x1D);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x27, 0x8F, 0xF);
    }
    if (obj->anims[4].level != 0) {
        if (obj->anims[4].level != 0x1000) {
            spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x4E);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x22, 0x92, 0x43);
    }
    sprite_init(&spr);
    spr.set_layer_id(obj->layer, 7);
    spr.set_vram_pos(0x240, 0x100);
    if (obj->scroll_wait != 0) {
        obj->scroll++;
        obj->scroll = obj->scroll < 0x60 ? obj->scroll : 0;
        obj->scroll_wait = 0;
    } else {
        obj->scroll_wait = 1;
    }
    spr.draw(cdload_module.get_subfile_by_id(0x028C0003), obj->background, obj->scroll, obj->scroll);
}

/* The training screen's steps (StgtrainMain.base.step): open the panels, choose a party member, train, close. */
void stgtrain_main_run(StgtrainMain *obj, StgtrainMainData *data) {
    s32 member;
    s32 i;

    switch (obj->base.step) {
    case 0:
    default:
        stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
        stgtrain_module.funcs.anim_start(&obj->anims[3], 1);
        obj->base.step++;
        break;
    case 1:
        stgtrain_module.funcs.anim_update(&obj->anims[3]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[0])) {
            stgtrain_main_show_status(obj, data, 1);
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0x10B), 5);
            stgtrain_module.funcs.anim_start(&obj->anims[1], 1);
            stgtrain_module.funcs.anim_start(&obj->anims[4], 1);
            obj->base.step++;
        }
        break;
    case 2:
        stgtrain_module.funcs.anim_update(&obj->anims[4]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[1])) {
            stgtrain_main_show_stats(obj, data, 1);
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x10B), 6);
            stgtrain_module.funcs.anim_start(&obj->anims[2], 1);
            stgtrain_module.funcs.anim_start(&obj->anims[6], 1);
            obj->base.step++;
        }
        break;
    case 3:
        stgtrain_module.funcs.anim_update(&obj->anims[6]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[2])) {
            stgtrain_main_show_tp(obj, data, 1);
            stgtrain_module.funcs.anim_start(&obj->anims[5], 1);
            obj->base.step++;
        }
        break;
    case 4:
        if (stgtrain_module.funcs.anim_update(&obj->anims[5])) {
            obj->cursor = 1;
            obj->base.step++;
        }
        break;
    case 5:
        member = obj->member;
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            obj->member--;
            if (obj->member < 0) {
                obj->member = 0;
            }
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            obj->member++;
            if (obj->member > obj->members - 1) {
                obj->member = obj->members - 1;
            }
        }
        if (member != obj->member) {
            sound_module.play(0x4001B);
            stgtrain_main_show_status(obj, data, 1);
            stgtrain_main_show_stats(obj, data, 1);
            stgtrain_main_show_tp(obj, data, 1);
            obj->frame = 0;
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->base.step = 20;
            if (data->choice != NULL) {
                data->choice->base.state = OBJECT_STATE_END;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        }
        break;
    case 10:
        if (data->choice == NULL) {
            data->choice = stgtrain_choice_create(obj);
            obj->base.step++;
        }
        break;
    case 11:
        if (data->choice == NULL) {
            obj->base.step = 30;
        } else if (data->choice->base.state == OBJECT_STATE_DONE) {
            obj->training = 0;
            data->choice->base.state = OBJECT_STATE_END;
            obj->base.step = 25;
        }
        break;
    case 20:
        obj->anims[5].level = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[6], 0);
        obj->cursor = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[4], 0);
        data->question->set_visible(data->question, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[3], 0);
        data->back_hint->set_visible(data->back_hint, 0);
        obj->base.step++;
        break;
    case 21:
        stgtrain_module.funcs.anim_update(&obj->anims[6]);
        stgtrain_module.funcs.anim_update(&obj->anims[4]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            obj->base.step = 10;
        }
        break;
    case 25:
        stgtrain_module.funcs.anim_start(&obj->anims[3], 1);
        obj->base.step++;
        break;
    case 26:
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0x10B), 5);
            stgtrain_module.funcs.anim_start(&obj->anims[4], 1);
            obj->base.step++;
        }
        break;
    case 27:
        if (stgtrain_module.funcs.anim_update(&obj->anims[4])) {
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x10B), 6);
            stgtrain_module.funcs.anim_start(&obj->anims[6], 1);
            obj->base.step++;
        }
        break;
    case 28:
        if (stgtrain_module.funcs.anim_update(&obj->anims[6])) {
            stgtrain_module.funcs.anim_start(&obj->anims[5], 1);
            obj->base.step = 4;
        }
        break;
    case 30:
        if (data->level_menu == NULL) {
            data->level_menu = stgtrain_level_create(obj);
            obj->base.step++;
            stgtrain_module.funcs.file_load(obj->training);
        }
        break;
    case 31:
        if (data->level_menu->base.step == 35) {
            if (data->session != NULL) {
                data->session->base.state = OBJECT_STATE_END;
            } else {
                obj->base.step = data->level_menu->base.step;
                stgtrain_main_show_tp(obj, data, 1);
            }
        } else if (data->level_menu->base.state == OBJECT_STATE_DONE) {
            data->level_menu->base.state = OBJECT_STATE_END;
            obj->base.step = 10;
        }
        break;
    case 35:
        if (stgtrain_module.funcs.file_get() != NULL && data->session == NULL) {
            data->session = stgtrain_session_create(obj, gamestate_data.funcs.get_party_member(obj->member), obj->training);
            obj->base.step++;
        }
        break;
    case 36:
        if (data->session == NULL) {
            data->level_menu->close(data->level_menu);
            obj->base.step++;
        }
        break;
    case 37:
        if (data->level_menu == NULL) {
            stgtrain_main_show_stats(obj, data, 1);
            obj->base.step = 25;
        }
        break;
    case 50:
        data->fade = stgtrain_fade_create();
        data->fade->start(data->fade, 0, 30);
        obj->anims[5].level = 0;
        for (i = 0; i < 3; i++) {
            stgtrain_module.funcs.anim_start(&obj->anims[i], 0);
        }
        stgtrain_main_show_status(obj, data, 0);
        stgtrain_main_show_stats(obj, data, 0);
        stgtrain_main_show_tp(obj, data, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[6], 0);
        obj->cursor = 0;
        stgtrain_module.funcs.anim_start(&obj->anims[4], 0);
        data->question->set_visible(data->question, 0);
        stgtrain_module.funcs.anim_start(&obj->anims[3], 0);
        data->back_hint->set_visible(data->back_hint, 0);
        obj->base.step++;
        break;
    case 51:
        for (i = 0; i < 3; i++) {
            stgtrain_module.funcs.anim_update(&obj->anims[i]);
        }
        stgtrain_module.funcs.anim_update(&obj->anims[6]);
        stgtrain_module.funcs.anim_update(&obj->anims[4]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            obj->base.step++;
        }
        break;
    case 52:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void stgtrain_main_update(StgtrainMain *obj, StgtrainMainData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            stgtrain_module.funcs.load();
            cdload_module.queue_file(records_language + 0x10B);
            sound_module.load_extra_bank(0x21);
            obj->base.step++;
            break;
        case 1:
            if (cdload_module.is_loading(records_language + 0x10B) == 0) {
                obj->base.next_state(obj);
                stgtrain_main_create_windows(obj, data);
                for (i = 0; i < 3; i++) {
                    if (gamestate_data.funcs.get_party_member(i) != -1) {
                        obj->members++;
                    }
                }
                for (i = 2; i >= 0; i--) {
                    obj->anims[i].duration = 10;
                }
                obj->anims[3].duration = 10;
                obj->anims[4].duration = 10;
                obj->anims[6].duration = 10;
                obj->anims[5].duration = 8;
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        stgtrain_main_run(obj, data);
        stgtrain_main_draw(obj);
        break;
    case OBJECT_STATE_DONE:
        if (sound_module.is_loading() == 0) {
            sound_module.play(0x60840002);
            obj->base.set_state(obj, OBJECT_STATE_RUN);
        }
        break;
    case OBJECT_STATE_END:
        sound_module.stop(0x60840002);
        gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
        break;
    }
}

/* Creates the training screen; its background depends on the gym (the current stage). */
StgtrainMain *stgtrain_main_create(void) {
    StgtrainMain *obj = object_new(stgtrain_main_update, sizeof(StgtrainMain), sizeof(StgtrainMainData));

    obj->show_change = stgtrain_main_show_change;
    obj->layer = 0x1000;
    obj->ot_depth = 6;
    switch (gamestate_data.funcs.get_prev_map()) {
    case 0x200: /* a stage below 0x23D with the default background: the code keeps no trace of its value, but
                 * without it the comparison tree's root is 0x28C instead of 0x267 */
        obj->background = 0x2D;
        break;
    case 0x23D:
        obj->background = 0x2E;
        break;
    case 0x24B:
        obj->background = 0x2F;
        break;
    case 0x267:
        obj->background = 0x30;
        break;
    case 0x28C:
        obj->background = 0x31;
        break;
    case 0x2AA:
        obj->background = 0x32;
        break;
    case 0x2B5:
        obj->background = 0x33;
        break;
    case 0x2CF:
        obj->background = 0x34;
        break;
    default:
        obj->background = 0x2D;
        break;
    }
    return obj;
}

void stgtrain_fade_start(Fade *obj, s32 dir, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = dir;
    if (dir == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void stgtrain_fade_draw(Fade *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, obj->ot_depth);
    poly = gfx_module.funcs.get_packet();
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->r0 = poly->g0 = poly->b0 = obj->level >> 8;
    setXYWH(poly, 0, 0, 320, 256);
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stgtrain_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step != 0) {
            obj->level += obj->step;
            if (obj->from_black == 0) {
                if (obj->level > 0xFF00) {
                    obj->level = 0xFF00;
                    obj->base.state = OBJECT_STATE_DONE;
                }
            } else if (obj->level < 0) {
                obj->level = 0;
                obj->base.state = OBJECT_STATE_DONE;
            }
        case OBJECT_STATE_DONE:
            stgtrain_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stgtrain_fade_create(void) {
    Fade *obj = object_new(stgtrain_fade_update, sizeof(Fade), 0);

    obj->start = stgtrain_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    return obj;
}

s32 stgtrain_member_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};
