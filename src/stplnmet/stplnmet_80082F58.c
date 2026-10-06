#include "common.h"
#include "object.h"
#include "cdload.h"
#include "pad.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "stplnmet.h"

/* STPLNMET.PRO: entering the player's name at the start of a game. This file sets up the display (the overlay's
 * entry point) and holds the scenery objects the main object (stplnmet_80085ECC.c) creates. */

/* An animated sprite object (size 0x68). */
typedef struct StplnmetAnim {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 unk_50;
    /* 0x54 */ s32 unk_54;
    /* 0x58 */ s32 unk_58;
    /* 0x5C */ s32 unk_5C;
    /* 0x60 */ s32 frame;  /* frame */
    /* 0x64 */ s32 frame_time; /* time of the last frame */
} StplnmetAnim; /* size 0x68 */

/* A frame of stplnmet_figure_frames (ended by sprite 0). */
typedef struct StplnmetFrame {
    /* 0x0 */ s32 delay; /* frames */
    /* 0x4 */ s32 sprite;
} StplnmetFrame; /* size 0x8 */

/* The message box (stplnmet_message_create, size 0x5C). */
typedef struct StplnmetMessage {
    /* 0x00 */ Object base;
    /* 0x50 */ StplnmetMain *main;
    /* 0x54 */ s32 box_frame; /* frame of the box's opening/closing */
    /* 0x58 */ s32 unk_58;
} StplnmetMessage; /* size 0x5C */

/* The message box's data block (8 bytes). */
typedef struct StplnmetMessageData {
    /* 0x0 */ MessageWindow *message; /* the message */
    /* 0x4 */ MessageWindow *next_cursor; /* the "next" cursor */
} StplnmetMessageData; /* size 0x8 */

extern s32 stplnmet_bg_wrap_x[2];
extern StplnmetFrame stplnmet_figure_frames[];

/* Centres the layer on `rect`. */
void stplnmet_setup_layer(Object *obj, StplnmetMain **data, GfxLayer *layer, RECT *rect) {
    layer->set_draw_offset(layer, rect->w / 2, rect->h / 2);
    layer->alloc_callbacks(layer, 5);
}

/* The overlay's first object: sets up the display and creates the main object. */
void stplnmet_update_main(Object *obj, StplnmetMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x19000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 1, 0x1000);
        layer->set_bg_color(layer, 1, 1, 1);
        stplnmet_setup_layer(obj, data, layer, &rect);
        gfx_module.funcs.create_layer(&rect, 3, 0x1001);
        gfx_module.funcs.move_layer(0x1001, 0x1000, 1);
        *data = stplnmet_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stplnmet_start(void) {
    return object_new(stplnmet_update_main, sizeof(Object), sizeof(StplnmetMain *));
}

void stplnmet_bg_update(StplnmetBg *obj) {
    Sprite spr;
    s32 i;
    s32 now;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->loaded != 0) {
            obj->step_times[0] = obj->step_times[1] = gfx_module.funcs.get_time();
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_RUN:
        sprite_init(&spr);
        spr.set_vram_pos(obj->tex_x, obj->tex_y);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        now = gfx_module.funcs.get_time();
        for (i = 0; i < 2; i++) {
            if (now - obj->step_times[i] > i + 1) {
                obj->sprite_pos[i].x -= 2;
                obj->sprite_pos[i].y += 1;
                if (obj->sprite_pos[i].x <= stplnmet_bg_wrap_x[i]) {
                    obj->sprite_pos[i].y = 0;
                    obj->sprite_pos[i].x = 0;
                }
                obj->step_times[i] = now;
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02C40000), i, obj->sprite_pos[i].x, obj->sprite_pos[i].y);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02C40000), 2, 0, 0);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Loads the background's texture to (x, y) in VRAM (StplnmetBg.load). */
void stplnmet_bg_load(StplnmetBg *obj, s32 x, s32 y) {
    Tim tim;

    obj->tex_x = x;
    obj->tex_y = y;
    tim_init(&tim);
    tim.set_image_pos(x, y);
    tim.load_all(cdload_module.get_subfile_by_id(0x08A70000));
    obj->loaded = 1;
}

/* StplnmetBg.set_layer. */
void stplnmet_bg_set_layer(StplnmetBg *obj, s32 layer, s32 depth) {
    obj->layer_id = layer;
    obj->ot_depth = depth;
}

StplnmetBg *stplnmet_bg_create(void) {
    StplnmetBg *obj = object_new(stplnmet_bg_update, sizeof(StplnmetBg), 0);

    obj->load = stplnmet_bg_load;
    obj->set_layer = stplnmet_bg_set_layer;
    return obj;
}

/* Five copies of an animated sprite (frames 0x18-0x23). */
void stplnmet_sparkle_update(StplnmetAnim *obj) {
    Sprite spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->frame = 0x18;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite_init(&spr);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_layer_id(0x1001, 6);
        if (gfx_module.funcs.get_time() - obj->frame_time >= 5) {
            obj->frame_time = gfx_module.funcs.get_time();
            if (++obj->frame >= 0x24) {
                obj->frame = 0x18;
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->frame, 0x33, 0);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->frame, 0x78, -0x14);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->frame, 0xA7, -0x58);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->frame, 0xF8, 0xD);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), obj->frame, 0x120, 0);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stplnmet_sparkle_create(void) {
    return object_new(stplnmet_sparkle_update, sizeof(StplnmetAnim), 0);
}

/* A sprite animated by the frames of stplnmet_figure_frames. */
void stplnmet_figure_update(StplnmetAnim *obj) {
    Sprite spr;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        sprite_init(&spr);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_layer_id(0x1001, 6);
        if (gfx_module.funcs.get_time() - obj->frame_time > stplnmet_figure_frames[obj->frame].delay) {
            obj->frame_time = gfx_module.funcs.get_time();
            obj->frame++;
            if (stplnmet_figure_frames[obj->frame].sprite == 0) {
                obj->frame = 0;
            }
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), stplnmet_figure_frames[obj->frame].sprite, 0, 0);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stplnmet_figure_create(void) {
    return object_new(stplnmet_figure_update, sizeof(StplnmetAnim), 0);
}

/* The message box: opens, shows a message with a blinking "next" cursor, closes on the button. */
void stplnmet_message_update(StplnmetMessage *obj, StplnmetMessageData *data) {
    Sprite spr;
    MessageWindow *win;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->message = win = message_create_window(0x1001, 1, 0x42, 0x3B);
        win->set_page_lines(win, 3);
        data->next_cursor = message_create_window(0x1001, 1, 0x42, 0x3B);
        obj->base.next_state(obj);
        obj->base.set_step(obj, 1);
        data->next_cursor->set_text(data->next_cursor, cdload_module.files.get_file(records_language + 0x8D), 2);
        data->next_cursor->set_palette(data->next_cursor, 1);
        data->next_cursor->set_visible(data->next_cursor, 0);
        data->message->set_char_sound(data->message, 0x800454C4);
        obj->base.substep = gfx_module.funcs.get_time();
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 1:
        default:
            if (gfx_module.funcs.get_time() - obj->base.substep >= 6) {
                obj->base.substep = gfx_module.funcs.get_time();
                if (++obj->box_frame >= 4) {
                    obj->box_frame = 3;
                    obj->base.set_step(obj, 2);
                    obj->base.substep = gfx_module.funcs.get_time();
                    data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x8D), 1);
                    data->message->set_palette(data->message, 1);
                    data->message->set_speed(data->message, 6);
                    data->next_cursor->set_visible(data->next_cursor, 1);
                }
            }
            break;
        case 2:
            if (data->message->is_done(data->message) != 0) {
                obj->base.set_step(obj, 3);
                obj->base.substep = gfx_module.funcs.get_time();
                obj->box_frame = 4;
                data->next_cursor->set_visible(data->next_cursor, 0);
                data->message->set_visible(data->message, 0);
            } else {
                data->next_cursor->set_pos(data->next_cursor, data->message->pen_x + 0x42, data->message->pen_y + 0x3B);
                if (data->message->is_waiting(data->message) != 0) {
                    if (data->next_cursor->is_visible(data->next_cursor) != 0) {
                        if (gfx_module.funcs.get_time() - obj->base.substep >= 0x11) {
                            obj->base.substep = gfx_module.funcs.get_time();
                            data->next_cursor->set_visible(data->next_cursor, 0);
                        }
                    } else if (gfx_module.funcs.get_time() - obj->base.substep >= 9) {
                        obj->base.substep = gfx_module.funcs.get_time();
                        data->next_cursor->set_visible(data->next_cursor, 1);
                    }
                } else {
                    data->next_cursor->set_visible(data->next_cursor, 1);
                    obj->base.substep = gfx_module.funcs.get_time();
                }
                if (PAD_PRESSED(0xD)) {
                    data->message->find_page_end(data->message);
                }
            }
            break;
        case 3:
            if (gfx_module.funcs.get_time() - obj->base.substep >= 6) {
                if (++obj->box_frame >= 8) {
                    obj->box_frame = 7;
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1001, 5);
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 0x38, 0x4B, 0xB8);
        spr.set_palette(obj->box_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x028A0000), 0xF, 0x38, 0x2F);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stplnmet_message_create(StplnmetMain *main) {
    StplnmetMessage *obj = object_new(stplnmet_message_update, sizeof(StplnmetMessage), sizeof(StplnmetMessageData));

    obj->main = main;
    return &obj->base;
}

s32 stplnmet_bg_wrap_x[2] = { -176, -190 };

StplnmetFrame stplnmet_figure_frames[29] = {
    { 4, 60 },
    { 4, 61 },
    { 4, 62 },
    { 4, 63 },
    { 4, 64 },
    { 4, 65 },
    { 4, 66 },
    { 4, 67 },
    { 4, 68 },
    { 80, 69 },
    { 4, 60 },
    { 4, 70 },
    { 4, 71 },
    { 4, 72 },
    { 4, 73 },
    { 4, 74 },
    { 4, 75 },
    { 4, 68 },
    { 4, 60 },
    { 4, 61 },
    { 4, 62 },
    { 4, 63 },
    { 4, 64 },
    { 4, 65 },
    { 4, 66 },
    { 4, 67 },
    { 4, 68 },
    { 56, 69 },
    { 0, 0 },
};
