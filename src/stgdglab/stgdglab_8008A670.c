#include "common.h"
#include "object.h"
#include "gfx.h"
#include "stgdglab.h"

/* STGDGLAB.PRO: a scroll bar object (the same code as STCRDDEK's and STSTATUS's; only the ordering table
 * entry set by the create differs from STCRDDEK's). */

void stgdglab_set_bar_x(StgdglabBar *obj, s32 x, s32 width) {
    obj->x = x;
    obj->w = width;
}

void stgdglab_set_bar_range(StgdglabBar *obj, s32 top, s32 bottom) {
    obj->top = top;
    obj->bottom = bottom;
    obj->range_set = 1;
}

void stgdglab_set_bar_lines(StgdglabBar *obj, s32 shown, s32 lines) {
    obj->shown = shown;
    obj->lines = lines;
    obj->lines_set = 1;
}

void stgdglab_set_bar_line(StgdglabBar *obj, s32 line) {
    obj->first_line = line;
}

void stgdglab_update_bar(StgdglabBar *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    s32 range;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->range_set != 0 && obj->lines_set != 0) {
            range = (obj->bottom - obj->top) << 8;
            obj->h = range / obj->lines * obj->shown;
            obj->line_height = range / obj->lines;
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        layer = gfx_module.funcs.get_layer(obj->layer_id);
        ot = layer->get_ot_entry(layer, obj->ot_depth);
        poly = gfx_module.funcs.get_packet();
        if (obj->first_line < obj->lines - 1) {
            obj->y = obj->top + ((obj->first_line * obj->line_height) >> 8);
            if (obj->bottom - (obj->h >> 8) < obj->y) {
                obj->y = obj->bottom - (obj->h >> 8);
            }
        } else {
            obj->y = obj->bottom - (obj->h >> 8);
        }
        setPolyF4(poly);
        poly->r0 = poly->g0 = poly->b0 = 0xFF;
        poly->x0 = poly->x2 = obj->x;
        poly->x1 = poly->x3 = obj->x + obj->w;
        poly->y0 = poly->y1 = obj->y;
        poly->y2 = poly->y3 = obj->y + (obj->h >> 8);
        addPrim(ot, poly);
        gfx_module.funcs.set_packet(poly + 1);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgdglabBar *stgdglab_bar_create(void) {
    StgdglabBar *obj = object_new(stgdglab_update_bar, sizeof(StgdglabBar), 0);

    obj->set_x = stgdglab_set_bar_x;
    obj->set_range = stgdglab_set_bar_range;
    obj->set_lines = stgdglab_set_bar_lines;
    obj->set_line = stgdglab_set_bar_line;
    obj->layer_id = 0x1000;
    obj->ot_depth = 0;
    return obj;
}
