#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "records.h"
#include "psyq/libgpu.h"

/* CNTY_SEL.PRO: the language (country) select screen. The root object waits for the sound bank, loads
 * the screen's image file (0x8A3) and creates the menu, which shows a background, three panels and a
 * cursor over the languages and sets records_language from the choice. */

void cnty_sel_update_root();
void cnty_sel_update_bg();
void cnty_sel_update_cursor();
void cnty_sel_update_top_panel();
void cnty_sel_update_right_panel();
void cnty_sel_update_bottom_panel();
void cnty_sel_update_menu();

/* A frame of the cursor's animation (cnty_sel_cursor_frames, ended by image == 0xFF). */
typedef struct CntySelFrame {
    /* 0x0 */ s16 image; /* image; 0xFF: end */
    /* 0x2 */ s16 duration; /* duration */
} CntySelFrame; /* size 0x4 */

/* Where the cursor's animation is (cnty_sel_step_anim steps it). */
typedef struct CntySelTrack {
    /* 0x0 */ s16 frame; /* frame index */
    /* 0x2 */ s16 time_left; /* time left in the frame */
} CntySelTrack; /* size 0x4 */

/* A step of a panel's zoom (cnty_sel_top_panel_zoom, cnty_sel_right_panel_zoom): from start_scale to end_scale in `frames` frames. */
typedef struct CntySelZoom {
    /* 0x0 */ s32 frames; /* frames */
    /* 0x4 */ s16 end_scale; /* end scale */
    /* 0x6 */ s16 start_scale; /* start scale */
} CntySelZoom; /* size 0x8 */

/* The same with a padding word (cnty_sel_bottom_panel_zoom). */
typedef struct CntySelZoom2 {
    /* 0x0 */ s32 frames;
    /* 0x4 */ s16 end_scale;
    /* 0x6 */ s16 start_scale;
    /* 0x8 */ s32 unk_8;
} CntySelZoom2; /* size 0xC */

/* Object of cnty_sel_update_bg: the scrolling background; in state 2 it fades the screen out. */
typedef struct CntySelBg {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 scroll; /* scroll, 0..191 */
    /* 0x52 */ s16 fade_timer; /* fade-out timer */
} CntySelBg; /* size 0x54 */

/* Object of cnty_sel_update_cursor: the cursor. */
typedef struct CntySelCursor {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 frame_counter; /* frame counter, 0..31 */
    /* 0x54 */ s16 language; /* language under the cursor */
    /* 0x56 */ s16 image;  /* image */
    /* 0x58 */ CntySelTrack anim;
    /* 0x5C */ void (*set_cursor)(struct CntySelCursor *obj, s16 index); /* cnty_sel_set_cursor */
} CntySelCursor; /* size 0x60 */

/* Objects of cnty_sel_update_top_panel, cnty_sel_update_right_panel, cnty_sel_update_bottom_panel: a panel that zooms in. */
typedef struct CntySelPanel {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 zoom_step; /* zoom step */
    /* 0x52 */ s16 timer;  /* timer */
    /* 0x54 */ s16 scale_x; /* x scale */
    /* 0x56 */ s16 scale_y; /* y scale */
} CntySelPanel; /* size 0x58 */

/* Data of the menu object (cnty_sel_update_menu). */
typedef struct CntySelMenuData {
    /* 0x00 */ CntySelCursor *cursor;
    /* 0x04 */ CntySelPanel *right_panel;
    /* 0x08 */ CntySelPanel *top_panel;
    /* 0x0C */ CntySelPanel *bottom_panel;
    /* 0x10 */ CntySelBg *bg;
} CntySelMenuData; /* size 0x14 */

/* Object of cnty_sel_update_menu: the menu. */
typedef struct CntySelMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 choice; /* choice, 0..6 */
    /* 0x52 */ s16 timer;  /* timer */
} CntySelMenu; /* size 0x54 */

RECT cnty_sel_screen_rect = { 0, 0, 320, 240 };
RECT cnty_sel_vram_rect = { 0, 0, 1024, 512 };
RECT cnty_sel_fade_rect = { 0, -15, 320, 260 };
CntySelFrame cnty_sel_cursor_frames[5] = { { 8, 4 }, { 9, 4 }, { 10, 4 }, { 11, 30 }, { 0xFF, 999 } };
CntySelZoom cnty_sel_top_panel_zoom[2] = { { 10, 0x1000, 0 }, { 5, 0, 0x1000 } };
CntySelZoom cnty_sel_right_panel_zoom[2] = { { 10, 0x1000, 0 }, { 5, 0, 0x1000 } };
CntySelZoom2 cnty_sel_bottom_panel_zoom[2] = { { 10, 0x1000, 0, 0 }, { 5, 0, 0x1000, 0 } };
u8 cnty_sel_languages[7] = { 2, 3, 5, 4, 6, 1, 0 }; /* language of each choice */

CntySelMenu *cnty_sel_create_menu(void);

void cnty_sel_update_root(Object *obj, CntySelMenu **data) {
    Tim tim;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        if (sound_module.is_loading() == 0) {
#ifdef PC_PORT
            if (port_mod_preset_language) {
                /* preset_language (docs/LAUNCHER.md "Preset language"): the menu's choice (cnty_sel_update_menu's steps 7
                 * and 13) without the screen */
                records_language = port_preset_language;
                gamestate_data.funcs.set_next_map(records_language == 0 ? 0xE01 : 0xE02, 0);
                obj->next_state(obj);
                break;
            }
#endif
            gfx_module.reset();
            gfx_module.alloc_packet_buffers(0xA000);
            gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
            tim_init(&tim);
            tim.set_image_pos(0x280, 0);
            tim.set_clut_pos(0, 0x1F0);
            tim.load_all(cdload_module.get_subfile_by_id(0x08A30001));
            layer = gfx_module.funcs.create_layer(&cnty_sel_screen_rect, 3, 0x100);
            layer->set_bg_color(layer, 0x1F, 0x1F, 0x1F);
            *data = cnty_sel_create_menu();
            obj->next_state(obj);
            sound_module.play(0x60840002);
        }
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        sound_module.stop(0x60840002);
        break;
    }
}

Object *cnty_sel_start(void) {
    Object *obj;

    ClearImage2(&cnty_sel_vram_rect, 0, 0, 0);
    obj = object_new(cnty_sel_update_root, sizeof(Object), sizeof(CntySelMenu *));
    sound_module.load_extra_bank(0x21);
    return obj;
}

void cnty_sel_draw_bg(CntySelBg *obj) {
    Sprite spr;
    s32 y;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    y = obj->scroll / 2 % 96;
    spr.draw(cdload_module.get_subfile_by_id(0x08A30000), 0, y, y);
}

s32 cnty_sel_get_fade_level(s32 t) {
    if (t >= 30) {
        return 255;
    }
    return t * 255 / 30;
}

/* Darkens the screen (cnty_sel_fade_rect) by `level` per channel: a quad with subtractive blending (abr 2). */
void cnty_sel_draw_fade(s32 level) {
    GfxLayer *layer = gfx_module.funcs.get_layer(0x100);
    u32 *ot = layer->get_ot_entry(layer, 0);
    POLY_F4 *poly = gfx_module.funcs.get_packet();
    DR_TPAGE *tpage;

    setlen(poly, 5);
    poly->r0 = poly->g0 = poly->b0 = level;
    setcode(poly, 0x2A); /* POLY_F4, semi-transparent */
    poly->x0 = cnty_sel_fade_rect.x;
    poly->x1 = cnty_sel_fade_rect.x + cnty_sel_fade_rect.w;
    poly->x2 = cnty_sel_fade_rect.x;
    poly->x3 = cnty_sel_fade_rect.x + cnty_sel_fade_rect.w;
    poly->y0 = cnty_sel_fade_rect.y;
    poly->y1 = cnty_sel_fade_rect.y;
    poly->y2 = cnty_sel_fade_rect.y + cnty_sel_fade_rect.h;
    poly->y3 = cnty_sel_fade_rect.y + cnty_sel_fade_rect.h;
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void cnty_sel_update_bg(CntySelBg *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->scroll = 0;
        break;
    case OBJECT_STATE_RUN:
        obj->scroll++;
        obj->scroll %= 192;
        cnty_sel_draw_bg(obj);
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->fade_timer = 0;
            obj->base.step = 1;
        }
        obj->scroll++;
        obj->scroll %= 192;
        cnty_sel_draw_bg(obj);
        cnty_sel_draw_fade(cnty_sel_get_fade_level(obj->fade_timer++));
        if (obj->fade_timer >= 30) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

CntySelBg *cnty_sel_create_bg(void) {
    return object_new(cnty_sel_update_bg, sizeof(CntySelBg), 0);
}

s16 cnty_sel_step_anim(CntySelTrack *track, CntySelFrame *frames, s32 depth) {
    CntySelFrame *frame = &frames[track->frame];
    s32 ticks = gfx_module.funcs.get_frame_ticks();

    if (ticks > 4) {
        ticks = 4;
    }
    if (depth == 0) {
        track->time_left -= ticks;
    }
    if (track->time_left <= 0) {
        frame++;
        track->frame++;
        track->time_left += frame->duration;
        if (frame->image == 0xFF) {
            return 0xFF;
        }
        cnty_sel_step_anim(track, frames, depth + 1);
    }
    return frame->image;
}

void cnty_sel_draw_cursor(CntySelCursor *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_palette(obj->image);
    spr.draw(cdload_module.get_subfile_by_id(0x08A30000), obj->language + 4, 0, 0);
}

void cnty_sel_set_cursor(CntySelCursor *obj, s16 index) {
    obj->language = index;
}

void cnty_sel_update_cursor(CntySelCursor *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        obj->frame_counter++;
        obj->frame_counter %= 32;
        obj->image = obj->frame_counter / 4;
        cnty_sel_draw_cursor(obj);
        break;
    case OBJECT_STATE_DONE:
        if (obj->base.step == 0) {
            obj->anim.frame = 0;
            obj->anim.time_left = cnty_sel_cursor_frames[0].duration;
            obj->base.step++;
        }
        obj->image = cnty_sel_step_anim(&obj->anim, cnty_sel_cursor_frames, 0);
        if (obj->image != 0xFF) {
            cnty_sel_draw_cursor(obj);
        } else {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

CntySelCursor *cnty_sel_create_cursor(void) {
    CntySelCursor *obj = object_new(cnty_sel_update_cursor, sizeof(CntySelCursor), 0);

    obj->set_cursor = cnty_sel_set_cursor;
    return obj;
}

s32 cnty_sel_get_top_panel_scale(CntySelPanel *obj, s32 step) {
    if (obj->timer >= cnty_sel_top_panel_zoom[step].frames) {
        return cnty_sel_top_panel_zoom[step].end_scale;
    }
    return cnty_sel_top_panel_zoom[step].start_scale
           + (cnty_sel_top_panel_zoom[step].end_scale - cnty_sel_top_panel_zoom[step].start_scale) * obj->timer
                 / cnty_sel_top_panel_zoom[step].frames;
}

void cnty_sel_draw_top_panel(CntySelPanel *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_pivot(0x94, 0);
    spr.set_scale(obj->scale_x, obj->scale_y, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x08A30000), 2, 0x94, 0);
}

void cnty_sel_update_top_panel(CntySelPanel *obj) {
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->scale_x = 0x1000;
        obj->scale_y = 0;
        obj->zoom_step = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            step = obj->zoom_step;
            if (cnty_sel_top_panel_zoom[step].frames < obj->timer++) {
                obj->zoom_step ^= 1;
                obj->base.set_step(obj, 0);
                obj->scale_y = cnty_sel_top_panel_zoom[step].end_scale;
            } else {
                obj->scale_y = cnty_sel_get_top_panel_scale(obj, step);
            }
            break;
        }
        cnty_sel_draw_top_panel(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->timer = 0;
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->base.set_step(obj, 1);
        cnty_sel_draw_top_panel(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

CntySelPanel *cnty_sel_create_top_panel(void) {
    return object_new(cnty_sel_update_top_panel, sizeof(CntySelPanel), 0);
}

s32 cnty_sel_get_right_panel_scale(CntySelPanel *obj, s32 step) {
    if (obj->timer >= cnty_sel_right_panel_zoom[step].frames) {
        return cnty_sel_right_panel_zoom[step].end_scale;
    }
    return cnty_sel_right_panel_zoom[step].start_scale
           + (cnty_sel_right_panel_zoom[step].end_scale - cnty_sel_right_panel_zoom[step].start_scale) * obj->timer
                 / cnty_sel_right_panel_zoom[step].frames;
}

void cnty_sel_draw_right_panel(CntySelPanel *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_pivot(0x140, 0x14);
    spr.set_scale(obj->scale_x, obj->scale_y, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x08A30000), 1, 0x140, 0x14);
}

void cnty_sel_update_right_panel(CntySelPanel *obj) {
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->scale_y = 0x1000;
        obj->scale_x = 0;
        obj->zoom_step = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            step = obj->zoom_step;
            if (cnty_sel_right_panel_zoom[step].frames < obj->timer++) {
                obj->zoom_step ^= 1;
                obj->base.set_step(obj, 0);
                obj->scale_x = cnty_sel_right_panel_zoom[step].end_scale;
            } else {
                obj->scale_x = cnty_sel_get_right_panel_scale(obj, step);
            }
            break;
        }
        cnty_sel_draw_right_panel(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->timer = 0;
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->base.set_step(obj, 1);
        cnty_sel_draw_right_panel(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

CntySelPanel *cnty_sel_create_right_panel(void) {
    return object_new(cnty_sel_update_right_panel, sizeof(CntySelPanel), 0);
}

s32 cnty_sel_get_bottom_panel_scale(CntySelPanel *obj, s32 step) {
    if (obj->timer >= cnty_sel_bottom_panel_zoom[step].frames) {
        return cnty_sel_bottom_panel_zoom[step].end_scale;
    }
    return cnty_sel_bottom_panel_zoom[step].start_scale
           + (cnty_sel_bottom_panel_zoom[step].end_scale - cnty_sel_bottom_panel_zoom[step].start_scale) * obj->timer
                 / cnty_sel_bottom_panel_zoom[step].frames;
}

void cnty_sel_draw_bottom_panel(CntySelPanel *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(0x100, 1);
    spr.set_vram_pos(0x280, 0);
    spr.set_clut8_pos(0, 0x1F0);
    spr.set_pivot(0, 0x9E);
    spr.set_scale(obj->scale_x, obj->scale_y, 0x1000);
    spr.draw(cdload_module.get_subfile_by_id(0x08A30000), 3, 0, 0x9E);
}

void cnty_sel_update_bottom_panel(CntySelPanel *obj) {
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->scale_y = 0x1000;
        obj->scale_x = 0;
        obj->zoom_step = 0;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            step = obj->zoom_step;
            if (cnty_sel_bottom_panel_zoom[step].frames < obj->timer++) {
                obj->zoom_step ^= 1;
                obj->base.set_step(obj, 0);
                obj->scale_x = cnty_sel_bottom_panel_zoom[step].end_scale;
            } else {
                obj->scale_x = cnty_sel_get_bottom_panel_scale(obj, step);
            }
            break;
        }
        cnty_sel_draw_bottom_panel(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->timer = 0;
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->base.set_step(obj, 1);
        cnty_sel_draw_bottom_panel(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

CntySelPanel *cnty_sel_create_bottom_panel(void) {
    return object_new(cnty_sel_update_bottom_panel, sizeof(CntySelPanel), 0);
}

void cnty_sel_update_menu(CntySelMenu *obj, CntySelMenuData *data) {
    s32 text;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->bg = cnty_sel_create_bg();
        data->top_panel = cnty_sel_create_top_panel();
        data->right_panel = cnty_sel_create_right_panel();
        data->bottom_panel = cnty_sel_create_bottom_panel();
        obj->choice = 0;
        obj->timer = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            data->right_panel->base.set_state(data->right_panel, OBJECT_STATE_DONE);
            obj->timer = 0;
            obj->base.next_step(obj);
            break;
        case 1:
            if (obj->timer++ >= 6) {
                obj->base.next_step(obj);
            }
            break;
        case 2:
            obj->timer = 0;
            data->bottom_panel->base.set_state(data->bottom_panel, OBJECT_STATE_DONE);
            obj->base.next_step(obj);
            break;
        case 3:
            if (obj->timer++ >= 11) {
                obj->base.next_step(obj);
            }
            break;
        case 4:
            obj->timer = 0;
            data->top_panel->base.set_state(data->top_panel, OBJECT_STATE_DONE);
            obj->base.next_step(obj);
            break;
        case 5:
            if (obj->timer++ >= 6) {
                obj->base.next_step(obj);
            }
            break;
        case 6:
            data->cursor = cnty_sel_create_cursor();
            obj->base.next_step(obj);
            break;
        case 7:
            if (PAD_PRESSED(3)) {
                sound_module.play(0x4001C);
                data->cursor->base.set_state(data->cursor, OBJECT_STATE_DONE);
                obj->base.next_step(obj);
                obj->timer = 0;
                records_language = cnty_sel_languages[obj->choice];
            } else {
                switch (obj->base.substep) {
                case 0:
                default:
                    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                        if (obj->choice > 0) {
                            sound_module.play(0x4001B);
                            obj->choice--;
                        }
                    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                        if (obj->choice < 4) {
                            sound_module.play(0x4001B);
                            obj->choice++;
                        }
                    } else if (PAD_PRESSED(7)) {
                        /* nothing (the calls are kept) */
                    }
                    break;
                case 1:
                    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                        if (obj->choice != 6) {
                            sound_module.play(0x4001B);
                        }
                        obj->choice = 6;
                    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                        if (obj->choice == 6) {
                            sound_module.play(0x4001B);
                            obj->choice = 5;
                        } else {
                            obj->choice = 1;
                            sound_module.play(0x4001B);
                            obj->base.set_substep(obj, 0);
                        }
                    }
                    break;
                }
            }
            data->cursor->set_cursor(data->cursor, obj->choice);
            break;
        case 8:
            if (++obj->timer >= 43) {
                obj->base.next_step(obj);
            }
            break;
        case 9:
            data->right_panel->base.set_state(data->right_panel, OBJECT_STATE_DONE);
            data->bottom_panel->base.set_state(data->bottom_panel, OBJECT_STATE_DONE);
            data->top_panel->base.set_state(data->top_panel, OBJECT_STATE_DONE);
            obj->timer = 0;
            obj->base.next_step(obj);
            break;
        case 10:
            if (++obj->timer >= 6) {
                obj->base.next_step(obj);
            }
            break;
        case 11:
            data->bg->base.set_state(data->bg, OBJECT_STATE_DONE);
            obj->base.next_step(obj);
            break;
        case 12:
            if (++obj->timer >= 31) {
                obj->base.next_step(obj);
            }
            break;
        case 13:
            text = 0xE02;
            if (records_language == 0) {
                text = 0xE01;
            }
            gamestate_data.funcs.set_next_map(text, 0);
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

CntySelMenu *cnty_sel_create_menu(void) {
    return object_new(cnty_sel_update_menu, sizeof(CntySelMenu), sizeof(CntySelMenuData));
}
