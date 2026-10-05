#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "stdwtitl.h"

void stdwtitl_slide_draw_jp(StdwtitlSlide *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_clut8_pos(0, 0x1F0);
    spr.set_vram_pos(0x280, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 1, obj->x, obj->y);
}

void stdwtitl_slide_update_jp(StdwtitlSlide *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->skip == 0) {
            obj->x = 0x20F;
            obj->y = 0x22;
            obj->base.next_state(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->frames_left = 10;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->x = obj->frames_left * 32 + 0xCF;
            if (obj->frames_left-- <= 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            stdwtitl_slide_draw_jp(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->x = 0xCF;
            obj->y = 0x22;
            obj->base.next_step(obj);
        }
        stdwtitl_slide_draw_jp(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void stdwtitl_slide_start(StdwtitlSlide *obj) {
    if (obj->base.state == OBJECT_STATE_RUN) {
        obj->base.set_step(obj, 1);
    }
}

StdwtitlSlide *stdwtitl_slide_create_jp(s32 arg0) {
    StdwtitlSlide *obj = object_new(stdwtitl_slide_update_jp, sizeof(StdwtitlSlide), 0);

    obj->start = stdwtitl_slide_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip = arg0;
    return obj;
}

void stdwtitl_slide_draw_intl(StdwtitlSlide *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_clut8_pos(0, 0x1F0);
    spr.set_vram_pos(0x280, 0x100);
    spr.draw(cdload_module.get_subfile_by_id(stdwtitl_module.bank), 1, obj->x, obj->y);
}

void stdwtitl_slide_update_intl(StdwtitlSlide *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->x = 0x153;
        obj->y = 0x6F;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            obj->frames_left = 10;
            obj->base.next_step(obj);
            /* fallthrough */
        case 2:
            obj->x = obj->frames_left * 32 + 0x13;
            if (obj->frames_left-- <= 0) {
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            stdwtitl_slide_draw_intl(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->x = 0x13;
            obj->y = 0x6F;
            obj->base.next_step(obj);
        }
        stdwtitl_slide_draw_intl(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

StdwtitlSlide *stdwtitl_slide_create_intl(s32 arg0) {
    StdwtitlSlide *obj = object_new(stdwtitl_slide_update_intl, sizeof(StdwtitlSlide), 0);

    obj->start = stdwtitl_slide_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->skip = arg0;
    return obj;
}
