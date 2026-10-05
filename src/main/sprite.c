#include "common.h"

#include "gfx.h"
#include "heap.h"

/* A cell of a sprite bank (sprite_draw): a rectangle of a 4- or 8-bit texture and its CLUT. */
typedef struct SpriteCell {
    /* 0x0 */ s16 u; /* in texture pixels */
    /* 0x2 */ u8 v;
    /* 0x3 */ u8 unk_3;
    /* 0x4 */ s16 w;
    /* 0x6 */ s16 h;
    /* 0x8 */ s16 clut_x; /* CLUT x, added to sprite_current's */
    /* 0xA */ s16 clut_y; /* CLUT y, added to sprite_current's */
    /* 0xC */ s16 is_8bit; /* non-zero: 8-bit texture */
} SpriteCell; /* size 0xE */

/* $gp variable: this file is built with -G8, and ASPSX only uses $gp for variables defined in the
 * same file. It is in this object's .sbss (configure.py DATA_IN_C). */
Sprite *sprite_current;

void sprite_set_current(Sprite *obj);
void sprite_set_vram_pos(s32 arg0, s32 arg1);
void sprite_set_clut8_pos(s32 arg0, s32 arg1);
void sprite_set_palette(s32 arg0);
void sprite_set_layer(GfxLayer *arg0, s32 arg1);
void sprite_set_layer_id(s32 arg0, s32 arg1);
void sprite_draw(s32 *bank, s32 id, s32 x, s32 y);
void sprite_set_scale(s32 arg0, s32 arg1, s32 arg2);
void sprite_set_rotation(s16 arg0, s16 arg1, s16 arg2);
void sprite_set_pivot(s32 arg0, s32 arg1);
void sprite_set_follow_scroll(s32 arg0);
void sprite_set_color(CVECTOR *color);

void sprite_set_current(Sprite *obj) {
    sprite_current = obj;
}

void sprite_set_vram_pos(s32 arg0, s32 arg1) {
    sprite_current->vram_x = arg0;
    sprite_current->vram_y = arg1;
    sprite_current->clut_x = arg0;
    sprite_current->clut_y = arg1;
}

void sprite_set_clut8_pos(s32 arg0, s32 arg1) {
    sprite_current->clut8_x = arg0;
    sprite_current->clut8_y = arg1 - 0x100;
}

void sprite_set_palette(s32 arg0) {
    sprite_current->palette = arg0;
}

void sprite_set_layer(GfxLayer *arg0, s32 arg1) {
    sprite_current->layer = arg0;
    sprite_current->ot_entry = arg0->get_ot_entry(arg0, arg1);
}

void sprite_set_layer_id(s32 arg0, s32 arg1) {
    sprite_set_layer(gfx_module.funcs.get_layer(arg0), arg1);
}

/* Draws frame `id` of sprite bank `bank` at (x, y) with the settings in sprite_current: one SPRT per cell, or
 * one POLY_FT4 per cell through the rotation/scale matrix. The bank starts with the offsets of its
 * cells (SpriteCell) and of its frame IDs (bytes), then of each frame: {count, CLUT y, blend
 * mode (-1: opaque)} and `count` {cell, x, y}. */
void sprite_draw(s32 *bank, s32 id, s32 x, s32 y) {
    s32 ofs[2];
    SVECTOR out;
    SVECTOR v[4];
    SpriteCell *cells;
    s32 count;
    s32 clut_y;
    s32 semi;
    s32 abr;
    s32 transform;
    s32 i;
    s32 j;
    u8 *ids;
    s16 *frame;
    SpriteCell *cell;
    u8 *packet;

    transform = 0;
    i = 0;
    ids = (u8 *)bank + bank[1];
    cells = (SpriteCell *)((u8 *)bank + bank[0]);
    while (ids[i] != id) {
        i++;
    }
    frame = (s16 *)((u8 *)bank + bank[i + 2]);
    /* Scaled or rotated: POLY_FT4s through matrix, else SPRTs. (rotation.vx and rotation.vy are
     * tested as one word.) */
    if (sprite_current->scale.vx != 0x1000 || sprite_current->scale.vy != sprite_current->scale.vx
        || sprite_current->scale.vz != sprite_current->scale.vy || *(s32 *)&sprite_current->rotation != 0
        || sprite_current->rotation.vz != 0) {
        transform = 1;
        if (sprite_current->matrix_dirty != 0) {
            RotMatrixYXZ_gte(&sprite_current->rotation, &sprite_current->matrix);
            ScaleMatrix(&sprite_current->matrix, &sprite_current->scale);
        }
    }
    count = *frame++;
    clut_y = *frame++;
    abr = *frame++;
    if (abr == -1) {
        semi = 0;
        abr = 0;
    } else {
        semi = 1;
    }
    if (sprite_current->follow_scroll != 0) {
        sprite_current->layer->get_scroll(sprite_current->layer, ofs);
    } else {
        ofs[0] = 0;
        ofs[1] = 0;
    }
    /* The cells are drawn last to first. */
    frame += (count - 1) * 3;
    packet = gfx_module.funcs.get_packet();
    if (!transform) {
        u16 clut;
        u16 tpage;
        u16 prev;

        tpage = 0;
        prev = 0;
        for (i = 0; i < count; i++) {
            cell = &cells[frame[0]];
            clut = getClut((cell->is_8bit ? sprite_current->clut8_x : sprite_current->clut_x) + cell->clut_x,
                           (cell->is_8bit ? sprite_current->clut8_y : sprite_current->clut_y) + cell->clut_y + clut_y
                               + sprite_current->palette);
            tpage = getTPage(cell->is_8bit, abr,
                             sprite_current->vram_x + (cell->is_8bit ? cell->u / 2 : cell->u / 4),
                             sprite_current->vram_y);
            if (i == 0) {
                prev = tpage;
            }
            /* A DR_TPAGE for the cells drawn before (the ordering table runs backwards). */
            if (prev != tpage) {
                SetDrawTPage((DR_TPAGE *)packet, 0, 1, prev);
                addPrim(sprite_current->ot_entry, packet);
                packet += sizeof(DR_TPAGE);
                prev = tpage;
            }
            *(CVECTOR *)&((SPRT *)packet)->r0 = sprite_current->color;
            setSprt((SPRT *)packet);
            if (semi) {
                setSemiTrans((SPRT *)packet, 1);
            }
            ((SPRT *)packet)->x0 = frame[1] + x - ofs[0];
            ((SPRT *)packet)->y0 = frame[2] + y - ofs[1];
            if (cell->is_8bit) {
                ((SPRT *)packet)->u0 = cell->u & 0x7F;
            } else {
                ((SPRT *)packet)->u0 = cell->u;
            }
            ((SPRT *)packet)->v0 = cell->v;
            ((SPRT *)packet)->w = cell->w;
            ((SPRT *)packet)->h = cell->h;
            ((SPRT *)packet)->clut = clut;
            frame -= 3;
            addPrim(sprite_current->ot_entry, packet);
            packet += sizeof(SPRT);
        }
        SetDrawTPage((DR_TPAGE *)packet, 0, 1, tpage);
        addPrim(sprite_current->ot_entry, packet);
        packet += sizeof(DR_TPAGE);
    } else {
        u16 clut;
        u16 tpage;
        u8 u;
        u8 tv;

        for (i = 0; i < count; i++) {
            cell = &cells[frame[0]];
            clut = getClut((cell->is_8bit ? sprite_current->clut8_x : sprite_current->clut_x) + cell->clut_x,
                           (cell->is_8bit ? sprite_current->clut8_y : sprite_current->clut_y) + cell->clut_y + clut_y
                               + sprite_current->palette);
            tpage = getTPage(cell->is_8bit, abr,
                             sprite_current->vram_x + (cell->is_8bit ? cell->u / 2 : cell->u / 4),
                             sprite_current->vram_y);
            *(CVECTOR *)&((POLY_FT4 *)packet)->r0 = sprite_current->color;
            setPolyFT4((POLY_FT4 *)packet);
            if (semi) {
                setSemiTrans((POLY_FT4 *)packet, 1);
            }
            /* The corners relative to (pivot_x, pivot_y), transformed by matrix. */
            v[0].vx = v[2].vx = frame[1] + x - sprite_current->pivot_x;
            v[1].vx = v[3].vx = v[0].vx + cell->w;
            v[0].vy = v[1].vy = frame[2] + y - sprite_current->pivot_y;
            v[2].vy = v[3].vy = v[0].vy + cell->h;
            v[0].vz = v[1].vz = v[2].vz = v[3].vz = 0;
            for (j = 0; j < 4; j++) {
                ApplyMatrixSV(&sprite_current->matrix, &v[j], &out);
                (&((POLY_FT4 *)packet)->x0)[j * 4] = (out.vx - ofs[0]) + sprite_current->pivot_x;
                (&((POLY_FT4 *)packet)->y0)[j * 4] = (out.vy - ofs[1]) + sprite_current->pivot_y;
            }
            if (cell->is_8bit) {
                u = cell->u & 0x7F;
            } else {
                u = cell->u;
            }
            ((POLY_FT4 *)packet)->u0 = ((POLY_FT4 *)packet)->u2 = u;
            ((POLY_FT4 *)packet)->u1 = ((POLY_FT4 *)packet)->u3 = u + cell->w - 1;
            tv = cell->v;
            ((POLY_FT4 *)packet)->v0 = ((POLY_FT4 *)packet)->v1 = tv;
            ((POLY_FT4 *)packet)->v2 = ((POLY_FT4 *)packet)->v3 = tv + cell->h - 1;
            ((POLY_FT4 *)packet)->tpage = tpage;
            ((POLY_FT4 *)packet)->clut = clut;
            addPrim(sprite_current->ot_entry, packet);
            packet += sizeof(POLY_FT4);
            frame -= 3;
        }
    }
    gfx_module.funcs.set_packet(packet);
}

void sprite_set_scale(s32 arg0, s32 arg1, s32 arg2) {
    sprite_current->scale.vx = arg0;
    sprite_current->scale.vy = arg1;
    sprite_current->scale.vz = arg2;
    sprite_current->matrix_dirty = 1;
}

void sprite_set_rotation(s16 arg0, s16 arg1, s16 arg2) {
    sprite_current->rotation.vx = arg0;
    sprite_current->rotation.vy = arg1;
    sprite_current->rotation.vz = arg2;
    sprite_current->matrix_dirty = 1;
}

void sprite_set_pivot(s32 arg0, s32 arg1) {
    sprite_current->pivot_x = arg0;
    sprite_current->pivot_y = arg1;
}

void sprite_set_follow_scroll(s32 arg0) {
    sprite_current->follow_scroll = arg0;
}

void sprite_set_color(CVECTOR *color) {
    sprite_current->color = *color;
}

void sprite_init(Sprite *obj) {
    heap_funcs.bzero(obj, sizeof(Sprite));
    obj->scale.vx = 0x1000;
    obj->scale.vy = 0x1000;
    obj->scale.vz = 0x1000;
    obj->follow_scroll = 1;
    obj->color.r = obj->color.g = obj->color.b = 0x80;
    obj->set_vram_pos = sprite_set_vram_pos;
    obj->draw = sprite_draw;
    obj->set_clut8_pos = sprite_set_clut8_pos;
    obj->set_layer_id = sprite_set_layer_id;
    obj->set_layer = sprite_set_layer;
    obj->set_current = sprite_set_current;
    obj->set_palette = sprite_set_palette;
    obj->set_scale = sprite_set_scale;
    obj->set_rotation = sprite_set_rotation;
    obj->set_pivot = sprite_set_pivot;
    obj->set_follow_scroll = sprite_set_follow_scroll;
    obj->set_color = sprite_set_color;
    sprite_set_current(obj);
}
