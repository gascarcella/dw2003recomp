#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cardgame.h"

/* A full-screen colour fade (CardgameFade): created by the game flow (cardgame_8009D6E0.c). */

/* The fade's rectangle: the whole screen. */
extern RECT cardgame_fade_rect;

void cardgame_fade_draw(CardgameFade *obj, RECT rect) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(0x100);
    ot = layer->get_ot_entry(layer, 0);
    poly = gfx_module.funcs.get_packet();
    setRGB0(poly, obj->color[0], obj->color[1], obj->color[2]);
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->x0 = rect.x;
    poly->x1 = rect.x + rect.w;
    poly->x2 = rect.x;
    poly->x3 = rect.x + rect.w;
    poly->y0 = rect.y;
    poly->y1 = rect.y;
    poly->y2 = rect.y + rect.h;
    poly->y3 = rect.y + rect.h;
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, obj->abr, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void cardgame_fade_step(CardgameFade *obj) {
    s32 i;

    obj->timer -= gfx_module.funcs.get_frame_ticks();
    if (obj->timer > 0) {
        for (i = 0; i < 3; i++) {
            obj->color[i] = obj->to[i] - (obj->to[i] - obj->from[i]) * obj->timer / obj->duration;
        }
    } else if (obj->close) {
        obj->state = 2;
    } else {
        obj->state = 0;
        obj->color[0] = obj->to[0];
        obj->color[1] = obj->to[1];
        obj->color[2] = obj->to[2];
    }
}

void cardgame_fade_set_color(CardgameFade *obj, u8 r, u8 g, u8 b) {
    obj->color[0] = r;
    obj->color[1] = g;
    obj->color[2] = b;
}

void cardgame_fade_start(CardgameFade *obj, u8 r, u8 g, u8 b, s32 ticks, s32 close) {
    s32 i;

    for (i = 0; i < 3; i++) {
        obj->from[i] = obj->color[i];
    }
    obj->to[0] = r;
    obj->to[1] = g;
    obj->to[2] = b;
    obj->timer = ticks;
    obj->duration = ticks;
    obj->state = 1;
    obj->close = close;
}

s32 cardgame_fade_is_idle(CardgameFade *obj) {
    return obj->state == 0;
}

void cardgame_fade_close(CardgameFade *obj) {
    obj->state = 2;
}

void cardgame_fade_update(CardgameFade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->state == 1) {
            cardgame_fade_step(obj);
        }
        cardgame_fade_draw(obj, cardgame_fade_rect);
        if (obj->state == 2) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

CardgameFade *cardgame_fade_create(u8 abr) {
    CardgameFade *obj = object_new(cardgame_fade_update, sizeof(CardgameFade), sizeof(Object *));

    obj->set_color = cardgame_fade_set_color;
    obj->start = cardgame_fade_start;
    obj->is_idle = cardgame_fade_is_idle;
    obj->abr = abr;
    obj->close_fade = cardgame_fade_close;
    return obj;
}

/* .data (address order) */

RECT cardgame_fade_rect = { 0, -15, 320, 260 };
