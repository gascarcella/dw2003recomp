#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "records.h"
#include "message.h"
#include "psyq/libc2.h"
#include "window_anim.h"
#include "stcrddek.h"

/* STCRDDEK.PRO: the name entry (renames a deck). The same file is in STDGNAME (0x80083378) and STPLNMET
 * (0x80083D70): a grid of characters (pages of 15 x 7) under the name, with OK/cancel and page buttons. */

/* A key of the character pages: type 1 = the first cell of a key, 0 = no key, < 0 = a next cell of a wide key
 * (the offset back to its first cell). */
typedef struct StcrddekNameKey {
    /* 0x0 */ s8 type;
    /* 0x1 */ u8 ch; /* index into the font's unk_0C */
} StcrddekNameKey; /* size 0x2 */

/* A character page: 7 rows of 15 keys. */
typedef struct StcrddekNamePage {
    /* 0x00 */ StcrddekNameKey k[7][15];
} StcrddekNamePage; /* size 0xD2 */

/* A page's tabs: their messages. */
typedef struct StcrddekNameTabs {
    /* 0x0 */ s32 msg[3];
} StcrddekNameTabs; /* size 0xC */

/* The character pages: Japanese has 3 (records_language == 0), the other languages 1. */
typedef struct StcrddekNamePages {
    /* 0x0 */ s32 count;
    /* 0x4 */ StcrddekNameTabs *labels; /* per page */
    /* 0x8 */ StcrddekNamePage *chars;
} StcrddekNamePages; /* size 0xC */

/* The name entry's windows (its data block). */
typedef struct StcrddekNameWindows {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ MessageWindow *name;   /* the name */
    /* 0x08 */ MessageWindow *tabs[3];   /* the page's tabs */
    /* 0x14 */ u8 unk_14[0xC];
    /* 0x20 */ MessageWindow *left_arrow;
    /* 0x24 */ MessageWindow *right_arrow;
    /* 0x28 */ MessageWindow *prev_label; /* previous page */
    /* 0x2C */ MessageWindow *next_label; /* next page */
    /* 0x30 */ MessageWindow *warning;
} StcrddekNameWindows; /* size 0x34 */

StcrddekNamePages stcrddek_name_pages;
extern MessageFont stcrddek_name_font; /* the name's font */
extern StcrddekNameTabs stcrddek_name_tabs_jpn[];
extern StcrddekNamePage stcrddek_name_pages_jpn[];
extern StcrddekNameTabs stcrddek_name_tabs_latin[];
extern StcrddekNamePage stcrddek_name_pages_latin[];

void stcrddek_draw_name_entry(StcrddekNameEntry *obj);
void stcrddek_run_name_entry(StcrddekNameEntry *obj, StcrddekNameWindows *data);

void stcrddek_create_name_windows(StcrddekNameEntry *obj, StcrddekNameWindows *data) {
    s32 i;

    data->title = message_create_window(obj->layer_id, 1, 0x20, 0x1A);
    data->title->set_palette(data->title, 4);
    data->name = message_create_window(obj->layer_id, 1, 0x4B, 0x40);
    data->name->set_fixed_size(data->name, 0x13, 0);
    data->name->font = &stcrddek_name_font;
    for (i = 0; i < 3; i++) {
        data->tabs[i] = message_create_window(obj->layer_id, 1, i * 0x4E + 0x2F, 0x5B);
        data->tabs[i]->set_ot_depth(data->tabs[i], obj->ot_depth - 1);
        data->tabs[i]->set_page_lines(data->tabs[i], 7);
        data->tabs[i]->set_fixed_size(data->tabs[i], 0xE, 0x12);
        data->tabs[i]->font = &stcrddek_name_font;
    }
    data->left_arrow = message_create_window(obj->layer_id, 1, 0xCE, 0xC6);
    data->right_arrow = message_create_window(obj->layer_id, 1, 0xE1, 0xC6);
    data->prev_label = message_create_window(obj->layer_id, 1, 0x13, 0x62);
    data->prev_label->set_ot_depth(data->prev_label, obj->ot_depth - 1);
    data->next_label = message_create_window(obj->layer_id, 1, 0x123, 0x62);
    data->next_label->set_ot_depth(data->next_label, obj->ot_depth - 1);
    data->warning = message_create_window(obj->layer_id, 1, 0x3E, 0x72);
}

/* Shows (or hides) the windows' texts. */
void stcrddek_show_name_windows(StcrddekNameEntry *obj, StcrddekNameWindows *data, s32 show) {
    s32 i;

    if (show) {
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x86), 1);
        data->name->copy_text(data->name, (u8 *)obj->name);
        data->name->set_palette(data->name, 1);
        for (i = 0; i < 3; i++) {
            data->tabs[i]->set_text(data->tabs[i], cdload_module.files.get_file(records_language + 0x86),
                                     stcrddek_name_pages.labels[obj->page].msg[i]);
            data->tabs[i]->set_palette(data->tabs[i], 1);
        }
        data->left_arrow->set_text(data->left_arrow, cdload_module.files.get_file(records_language + 0x86), 0xD);
        data->left_arrow->set_palette(data->left_arrow, 1);
        data->right_arrow->set_text(data->right_arrow, cdload_module.files.get_file(records_language + 0x86), 0xE);
        data->right_arrow->set_palette(data->right_arrow, 1);
        if (stcrddek_name_pages.count >= 2) {
            data->prev_label->set_text(data->prev_label, cdload_module.files.get_file(records_language + 0x86), 0x10);
            data->prev_label->set_palette(data->prev_label, 1);
            data->next_label->set_text(data->next_label, cdload_module.files.get_file(records_language + 0x86), 0x11);
            data->next_label->set_palette(data->next_label, 1);
        }
    } else {
        data->title->set_visible(data->title, 0);
        data->name->set_visible(data->name, 0);
        for (i = 0; i < 3; i++) {
            data->tabs[i]->set_visible(data->tabs[i], 0);
        }
        data->left_arrow->set_visible(data->left_arrow, 0);
        data->right_arrow->set_visible(data->right_arrow, 0);
        data->prev_label->set_visible(data->prev_label, 0);
        data->next_label->set_visible(data->next_label, 0);
    }
}


/* A sprite of the name entry's buttons (stcrddek_name_buttons). */
typedef struct StcrddekNameButton {
    /* 0x0 */ s32 image; /* image */
    /* 0x4 */ s32 x;     /* x */
    /* 0x8 */ s32 y;     /* y */
} StcrddekNameButton; /* size 0xC */

extern s32 stcrddek_name_portrait_anims[][7]; /* animations, -1 ends one early */
extern StcrddekNameButton stcrddek_name_buttons[5];
extern s32 stcrddek_name_arrow_palettes[6];

void stcrddek_draw_name_entry(StcrddekNameEntry *obj) {
    Sprite spr;
    s32 k;
    s32 i;

    sprite_init(&spr);
    spr.set_vram_pos(obj->image_pos, obj->image_y);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->anims[1].level != 0) {
        if (obj->cursor_shown != 0) {
            if (gfx_module.funcs.get_time() - obj->cursor_time >= 5) {
                obj->cursor_time = gfx_module.funcs.get_time();
                if (++obj->cursor_frame >= 4) {
                    obj->cursor_frame = 0;
                }
            }
            spr.set_palette(obj->cursor_frame);
            if (obj->cursor_column < 10 || obj->cursor_row < 3) {
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x27, obj->cursor_column * 14 + 0x2F + obj->cursor_column / 5 * 8,
                           obj->cursor_row * 18 + 0x5A);
            } else {
                for (k = 0; stcrddek_name_pages.chars[obj->page].k[obj->cursor_row][obj->cursor_column + k].type != 1; k--) {
                }
                switch (obj->cursor_column + k + obj->cursor_row * 15) {
                case 0x64:
                default:
                    i = 0;
                    break;
                case 0x65:
                    i = 1;
                    break;
                case 0x46:
                    i = 2;
                    stcrddek_name_buttons[2].image = records_language + 0x3C;
                    break;
                case 0x55:
                    i = 3;
                    stcrddek_name_buttons[3].image = records_language + 0x44;
                    break;
                case 0x67:
                    i = 4;
                    stcrddek_name_buttons[4].image = records_language + 0x4C;
                    break;
                }
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), stcrddek_name_buttons[i].image, stcrddek_name_buttons[i].x,
                           stcrddek_name_buttons[i].y);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x28, obj->name_cursor * 19 + 0x4B, 0x40);
            spr.set_palette(0);
        }
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        }
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x18, 0x20);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x1D, 0x18, 0x15);
        if (obj->mode != 2) {
            if (obj->anims[1].level != 0x1000) {
                spr.set_pivot(0x20, 0x3F);
            }
            if (obj->portrait_anim != -1) {
                if (gfx_module.funcs.get_time() - obj->portrait_time >= 13) {
                    obj->portrait_time = gfx_module.funcs.get_time();
                    if (++obj->portrait_frame >= 7 || stcrddek_name_portrait_anims[obj->portrait_anim][obj->portrait_frame] == -1) {
                        obj->portrait_frame = 0;
                    }
                }
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), stcrddek_name_portrait_anims[obj->portrait_anim][obj->portrait_frame], 0x22, 0x30);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x36, 0x20, 0x2E);
            }
            if (gfx_module.funcs.get_time() - obj->glow_time >= 5) {
                obj->glow_time = gfx_module.funcs.get_time();
                if (++obj->glow_frame >= 14) {
                    obj->glow_frame = 0;
                }
            }
            spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
            spr.set_palette(obj->glow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x1F, 0x20, 0x2E);
            spr.set_palette(0);
            spr.set_layer_id(obj->layer_id, obj->ot_depth);
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x1E, 0x20, 0x2E);
        }
        if (obj->anims[1].level != 0x1000) {
            spr.set_pivot(0x20, 0x49);
        }
        if (obj->mode != 2) {
            if (records_language == 0) {
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x34, 0x4B, 0x40);
            } else {
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x37, 0x4B, 0x40);
            }
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x35, 0x4B, 0x40);
        }
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(0x1000, obj->anims[1].level, 0x1000);
        }
        if (stcrddek_name_pages.count >= 2) {
            if (gfx_module.funcs.get_time() - obj->arrows_time >= 7) {
                obj->arrows_time = gfx_module.funcs.get_time();
                if (++obj->arrows_frame >= 6) {
                    obj->arrows_frame = 0;
                }
            }
            spr.set_palette(stcrddek_name_arrow_palettes[obj->arrows_frame]);
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x32, 0xA, 0x5E);
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x33, 0x119, 0x5E);
            spr.set_palette(0);
        }
        spr.set_palette(4);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x2C, 0xCB, 0xC3);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x2D, 0xDE, 0xC3);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), records_language + 0x3C, 0xCB, 0x99);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), records_language + 0x44, 0xCB, 0xAE);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), records_language + 0x4C, 0xF6, 0xC3);
        spr.set_palette(0);
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x25, 0x1D, 0x54);
    }
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(0x1000, obj->anims[2].level, 0x1000);
            spr.set_pivot(0, 0x78);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x26, 0, 0x64);
    }
}

#define STCRDDEK_NAME_KEY(obj) (stcrddek_name_pages.chars[(obj)->page].k[(obj)->cursor_row][(obj)->cursor_column])
#define STCRDDEK_NAME_SWAP(c) ((u16)(((c) >> 8) | (((c) & 0xFF) << 8)))
#define STCRDDEK_NAME_SPACE 0x4081 /* a full-width space (Shift-JIS 0x8140, byte-swapped) */

/* The name entry's input and its steps (base.step): move on the keyboard, type, delete, end. */
void stcrddek_run_name_entry(StcrddekNameEntry *obj, StcrddekNameWindows *data) {
    s32 old;
    s32 k;
    s32 i;
    s32 j;
    u16 c;
    u32 glyph; /* the typed character: its own u32 (DECISIONS "The US decomp as a reference") */

    switch (obj->base.step) {
    case 0:
    default:
        window_anim_start(&obj->anims[1], 1);
        obj->base.step++;
        break;
    case 1:
        if (window_anim_update(&obj->anims[1])) {
            stcrddek_show_name_windows(obj, data, 1);
            obj->cursor_shown = 1;
            obj->base.step++;
        }
        break;
    case 2:
        if (PAD_PRESSED(3)) {
            sound_module.play(0x4001B);
            obj->cursor_column = 13;
            obj->cursor_row = 6;
            break;
        }
        if (stcrddek_name_pages.count >= 2) {
            old = obj->page;
            if (!PAD_HELD(0xB) && PAD_PRESSED(0xA)) {
                if (--obj->page < 0) {
                    obj->page = stcrddek_name_pages.count - 1;
                }
            } else if (!PAD_HELD(0xA) && PAD_PRESSED(0xB)) {
                if (++obj->page > stcrddek_name_pages.count - 1) {
                    obj->page = 0;
                }
            }
            if (old != obj->page) {
                sound_module.play(0x4001B);
                stcrddek_show_name_windows(obj, data, 1);
                if (records_language == 0) {
                    if ((u32)obj->page < 2) {
                        while (STCRDDEK_NAME_KEY(obj).type != 1) {
                            if (--obj->cursor_column < 0) {
                                obj->cursor_column = 14;
                            }
                        }
                    } else {
                        while (STCRDDEK_NAME_KEY(obj).type == 0) {
                            if (--obj->cursor_row < 0) {
                                obj->cursor_row = 6;
                            }
                        }
                    }
                }
            }
        }
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            if (STCRDDEK_NAME_KEY(obj).type < 0) {
                obj->cursor_column += STCRDDEK_NAME_KEY(obj).type;
            }
            do {
                if (--obj->cursor_column < 0) {
                    obj->cursor_column = 14;
                }
            } while (STCRDDEK_NAME_KEY(obj).type != 1);
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            if (STCRDDEK_NAME_KEY(obj).type < 0) {
                obj->cursor_column += STCRDDEK_NAME_KEY(obj).type;
            }
            do {
                if (++obj->cursor_column >= 15) {
                    obj->cursor_column = 0;
                }
            } while (STCRDDEK_NAME_KEY(obj).type != 1);
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            do {
                if (--obj->cursor_row < 0) {
                    obj->cursor_row = 6;
                }
            } while (STCRDDEK_NAME_KEY(obj).type == 0);
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            do {
                if (++obj->cursor_row >= 7) {
                    obj->cursor_row = 0;
                }
            } while (STCRDDEK_NAME_KEY(obj).type == 0);
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(0xD)) {
            for (k = 0; stcrddek_name_pages.chars[obj->page].k[obj->cursor_row][obj->cursor_column + k].type != 1;
                 k--) {
            }
            i = obj->cursor_row * 15 + obj->cursor_column + k;
            sound_module.play(0x4001C);
            switch (i) {
            case 0x64:
                if (--obj->name_cursor < 0) {
                    obj->name_cursor = 0;
                }
                break;
            case 0x65:
                if (++obj->name_cursor > obj->length - 1) {
                    obj->name_cursor = obj->length - 1;
                }
                break;
            case 0x55:
                c = data->name->font->chars_ext[1].sjis;
                obj->name[obj->name_cursor] = STCRDDEK_NAME_SWAP(c);
                if (++obj->name_cursor > obj->length - 1) {
                    obj->name_cursor = obj->length - 1;
                }
                data->name->copy_text(data->name, (u8 *)obj->name);
                break;
            case 0x67:
                for (j = 0; j < obj->length; j++) {
                    if (obj->name[j] != STCRDDEK_NAME_SPACE && obj->name[j] != 0) {
                        for (j = 19; j >= 0; j--) {
                            if (obj->name[j] != STCRDDEK_NAME_SPACE) {
                                obj->base.step = 100;
                                return;
                            }
                            obj->name[j] = 0;
                        }
                    }
                }
                obj->base.step = 20;
                break;
            default:
                glyph = data->name->font->chars[STCRDDEK_NAME_KEY(obj).ch].sjis;
                obj->name[obj->name_cursor] = STCRDDEK_NAME_SWAP(glyph);
                data->name->copy_text(data->name, (u8 *)obj->name);
                if (++obj->name_cursor > obj->length - 1) {
                    obj->name_cursor = obj->length - 1;
                    obj->cursor_column = 13;
                    obj->cursor_row = 6;
                }
                break;
            case 0x46:
                if (obj->name_cursor <= obj->length - 1 && obj->name[obj->name_cursor] == STCRDDEK_NAME_SPACE) {
                    if (--obj->name_cursor < 0) {
                        obj->name_cursor = 0;
                    }
                }
                c = data->name->font->chars_ext[1].sjis;
                obj->name[obj->name_cursor] = STCRDDEK_NAME_SWAP(c);
                data->name->copy_text(data->name, (u8 *)obj->name);
                break;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            if (obj->name_cursor <= obj->length - 1 && obj->name[obj->name_cursor] == STCRDDEK_NAME_SPACE) {
                if (--obj->name_cursor < 0) {
                    obj->name_cursor = 0;
                }
            }
            c = data->name->font->chars_ext[1].sjis;
            obj->name[obj->name_cursor] = STCRDDEK_NAME_SWAP(c);
            data->name->copy_text(data->name, (u8 *)obj->name);
        }
        break;
    case 10:
        stcrddek_show_name_windows(obj, data, 0);
        window_anim_start(&obj->anims[1], 0);
        obj->cursor_shown = 0;
        obj->base.step++;
        break;
    case 11:
        if (window_anim_update(&obj->anims[1])) {
            obj->base.state = OBJECT_STATE_DONE;
        }
        break;
    case 20:
        obj->cursor_shown = 0;
        window_anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 21:
        if (window_anim_update(&obj->anims[2])) {
            data->warning->set_text(data->warning, cdload_module.files.get_file(records_language + 0x86), 0x12);
            obj->base.step++;
        }
        break;
    case 22:
        if (PAD_PRESSED(0xD)) {
            data->warning->set_visible(data->warning, 0);
            window_anim_start(&obj->anims[2], 0);
            obj->base.step++;
        }
        break;
    case 23:
        if (window_anim_update(&obj->anims[2])) {
            obj->cursor_shown = 1;
            obj->base.step = 2;
        }
        break;
    case 100:
        break;
    }
}

void stcrddek_update_name_entry(StcrddekNameEntry *obj, StcrddekNameWindows *data) {
    Tim tim;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        tim_init(&tim);
        tim.set_image_pos(obj->image_pos, obj->image_y);
        tim.load_all(cdload_module.get_subfile_by_id(0x07710000));
        if (records_language == 0) {
            stcrddek_name_pages.count = 3;
            stcrddek_name_pages.labels = stcrddek_name_tabs_jpn;
            stcrddek_name_pages.chars = stcrddek_name_pages_jpn;
        } else {
            stcrddek_name_pages.count = 1;
            stcrddek_name_pages.labels = stcrddek_name_tabs_latin;
            stcrddek_name_pages.chars = stcrddek_name_pages_latin;
        }
        obj->anims[0].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[1].duration = 10;
        stcrddek_create_name_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        stcrddek_run_name_entry(obj, data);
        stcrddek_draw_name_entry(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void stcrddek_set_name_vram(StcrddekNameEntry *obj, s32 x, s32 y) {
    obj->image_pos = x;
    obj->image_y = y;
}

/* Copies `name` in, padded with full-width spaces. */
void stcrddek_set_name(StcrddekNameEntry *obj, u8 *name) {
    Font font;
    s32 i;

    font_init(&font);
    font.convert_text((u8 *)obj->name, name, 0);
    for (i = strlen((char *)obj->name) >> 1; i < obj->length; i++) {
        obj->name[i] = 0x4081;
    }
}

/* Copies the name out without its padding. */
void stcrddek_get_name(StcrddekNameEntry *obj, u8 *name) {
    Font font;
    s32 i;

    for (i = 0; i < obj->length * 2; i++) {
        name[i] = 0;
    }
    for (i = obj->length - 1; i >= 0; i--) {
        if (obj->name[i] != 0x4081) {
            break;
        }
        obj->name[i] = 0;
    }
    for (i = 0; i < obj->length; i++) {
        if (obj->name[i] != 0x4081) {
            break;
        }
    }
    font_init(&font);
    font.convert_text(name, (u8 *)&obj->name[i], 1);
}

void stcrddek_close_name_entry(StcrddekNameEntry *obj) {
    obj->base.step = 10;
}

StcrddekNameEntry *stcrddek_create_name_entry(u8 *name) {
    StcrddekNameEntry *obj = object_new(stcrddek_update_name_entry, sizeof(StcrddekNameEntry), sizeof(StcrddekNameWindows));

    obj->get_name = stcrddek_get_name;
    obj->close = stcrddek_close_name_entry;
    obj->layer_id = 0x1000;
    obj->ot_depth = 3;
    obj->mode = 2;
    obj->length = 10;
    obj->portrait_anim = -1;
    stcrddek_set_name(obj, name);
    stcrddek_set_name_vram(obj, 0x280, 0x100);
    return obj;
}

/* The name entry's font: its glyphs (characters from 4, then 114 more from 1), with the EXE's character tables. */
FontGlyph stcrddek_name_glyphs[230] = {
    { 0x01, 0x60, 0x12, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x68, 0x12, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x70, 0x12, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x78, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x80, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x88, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x90, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x98, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xA0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xA8, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xB0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xB8, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xC0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xC8, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xD0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xD8, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xE0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xE8, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x20, 0x3F, 0x30, 0xE8, 4, 12, 5, 3, 3, 12 },
    { 0x01, 0xF0, 0x15, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x00, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x08, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x10, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x18, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x20, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x28, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x30, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x38, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x40, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x48, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x50, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x58, 0x1E, 0x30, 0xE8, 8, 12, 5, 3, 6, 12 },
    { 0x01, 0x60, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x68, 0x1E, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x70, 0x1E, 0x30, 0xE8, 8, 12, 5, 3, 6, 12 },
    { 0x01, 0x78, 0x21, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x80, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x88, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x90, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x98, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xA0, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xA8, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xB0, 0x21, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xB8, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x24, 0x3F, 0x30, 0xE8, 4, 12, 7, 3, 1, 12 },
    { 0x01, 0x7C, 0xB9, 0x30, 0xE8, 4, 12, 5, 3, 4, 12 },
    { 0x01, 0xC0, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x00, 0xF8, 0x3E, 0x30, 0xE8, 4, 12, 6, 3, 3, 12 },
    { 0x01, 0xC8, 0x21, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0xD0, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xD8, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xE0, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xE8, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xD4, 0x78, 0x30, 0xE8, 4, 12, 6, 3, 4, 12 },
    { 0x01, 0xF0, 0x21, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xCC, 0x47, 0x30, 0xE8, 4, 12, 6, 3, 4, 12 },
    { 0x01, 0x00, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 6, 12 },
    { 0x01, 0x08, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x10, 0x2A, 0x30, 0xE8, 8, 12, 4, 3, 6, 12 },
    { 0x01, 0x18, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x20, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0x28, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x00, 0xD8, 0x4A, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xE4, 0x4A, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xF0, 0x4A, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0x00, 0x4C, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x0C, 0x4C, 0x30, 0xE8, 12, 12, 4, 3, 6, 12 },
    { 0x00, 0x18, 0x4C, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x24, 0x4C, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0x30, 0x4C, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x3C, 0x4C, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x9C, 0x4D, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xA8, 0x4D, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x48, 0x4E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x54, 0x54, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x60, 0x54, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x6C, 0x54, 0x30, 0xE8, 12, 12, 3, 3, 6, 12 },
    { 0x00, 0x78, 0x54, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x84, 0x54, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x90, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xB4, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xC0, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xCC, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xD8, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xE4, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xF0, 0x56, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x00, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x0C, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x18, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x24, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x30, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x3C, 0x58, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x9C, 0x59, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xA8, 0x59, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x48, 0x5A, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x54, 0x60, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x60, 0x60, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x6C, 0x60, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x78, 0x60, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x84, 0x60, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x90, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xB4, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xC0, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xCC, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xD8, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xE4, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xF0, 0x62, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x00, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x0C, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x18, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x24, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 11, 12 },
    { 0x00, 0x30, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x3C, 0x64, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x9C, 0x65, 0x30, 0xE8, 12, 12, 3, 3, 10, 9 },
    { 0x00, 0xA8, 0x65, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x48, 0x66, 0x30, 0xE8, 12, 12, 2, 3, 10, 12 },
    { 0x00, 0x54, 0x6C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x60, 0x6C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x6C, 0x6C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x78, 0x6C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x84, 0x6C, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x90, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xB4, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xC0, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xCC, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xD8, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xE4, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xF0, 0x6E, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x00, 0x70, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x0C, 0x70, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x18, 0x70, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x24, 0x70, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x30, 0x70, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x3C, 0x70, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x9C, 0x71, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xA8, 0x71, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x48, 0x72, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x54, 0x78, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x60, 0x78, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x6C, 0x78, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x78, 0x78, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x84, 0x78, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x90, 0x7A, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xB4, 0x7A, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0xC0, 0x7A, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xCC, 0x7A, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0xD8, 0x7A, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xE4, 0x7A, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0xF0, 0x7A, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x00, 0x7C, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x0C, 0x7C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x18, 0x7C, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0x24, 0x7C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x30, 0x7C, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x3C, 0x7C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x9C, 0x7D, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xA8, 0x7D, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x48, 0x7E, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x54, 0x84, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x60, 0x84, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x6C, 0x84, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x78, 0x84, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x84, 0x84, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x90, 0x86, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xB4, 0x86, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0x58, 0xC9, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xB8, 0x53, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xDC, 0x53, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xD0, 0x53, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xE8, 0x53, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x00, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x0C, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x18, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x24, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x30, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x3C, 0x88, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x9C, 0x89, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xA8, 0x89, 0x30, 0xE8, 12, 12, 3, 3, 7, 12 },
    { 0x00, 0x48, 0x8A, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x54, 0x90, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x60, 0x90, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x6C, 0x90, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0x30, 0x2A, 0x30, 0xE8, 8, 12, 4, 3, 5, 12 },
    { 0x01, 0x38, 0x2A, 0x30, 0xE8, 8, 12, 5, 3, 6, 12 },
    { 0x00, 0x78, 0x90, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x84, 0x90, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x90, 0x92, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xB4, 0x92, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x01, 0x00, 0x57, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x01, 0x0C, 0x57, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0xB4, 0xAA, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0x58, 0xE6, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xD0, 0x47, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x00, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x0C, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x18, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x24, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x30, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x3C, 0x94, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x9C, 0x95, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0xA8, 0x95, 0x30, 0xE8, 12, 12, 2, 3, 10, 12 },
    { 0x00, 0x48, 0x96, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x54, 0x9C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x60, 0x9C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x6C, 0x9C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x78, 0x9C, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x84, 0x9C, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x90, 0x9E, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0xB4, 0x9E, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xC4, 0x53, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x01, 0x58, 0xF2, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x18, 0xF3, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x24, 0xF3, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0xD4, 0xC1, 0x30, 0xE8, 12, 12, 4, 3, 6, 12 },
    { 0x00, 0x00, 0xA0, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0x0C, 0xA0, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x18, 0xA0, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0x24, 0xA0, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x30, 0xA0, 0x30, 0xE8, 12, 12, 4, 3, 8, 12 },
    { 0x00, 0x3C, 0xA0, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x9C, 0xA1, 0x30, 0xE8, 12, 12, 4, 3, 7, 12 },
    { 0x00, 0xA8, 0xA1, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x48, 0xA2, 0x30, 0xE8, 12, 12, 3, 3, 8, 12 },
    { 0x00, 0x54, 0xA8, 0x30, 0xE8, 12, 12, 3, 3, 9, 12 },
    { 0x00, 0x60, 0xA8, 0x30, 0xE8, 12, 12, 3, 3, 10, 12 },
    { 0x01, 0x88, 0x09, 0x30, 0xE8, 8, 12, 4, 3, 7, 12 },
    { 0x01, 0xA0, 0x09, 0x30, 0xE8, 8, 12, 4, 3, 7, 12 },
    { 0x01, 0xC8, 0x9C, 0x30, 0xE8, 4, 12, 5, 3, 4, 12 },
    { 0x01, 0xB0, 0x09, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x00, 0x60, 0x3C, 0x30, 0xE8, 12, 12, 4, 3, 9, 12 },
};

FontGlyph stcrddek_name_glyphs_ext[114] = {
    { 0x00, 0x84, 0x3C, 0x30, 0xE8, 8, 12, 3, 3, 6, 12 },
    { 0x00, 0xF4, 0x20, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x00, 0xF4, 0x2C, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x78, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 8, 12 },
    { 0x01, 0x80, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x88, 0x09, 0x30, 0xE8, 8, 12, 4, 3, 7, 12 },
    { 0x01, 0x90, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x98, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0xA0, 0x09, 0x30, 0xE8, 8, 12, 4, 3, 7, 12 },
    { 0x01, 0xC8, 0x9C, 0x30, 0xE8, 4, 12, 5, 3, 4, 12 },
    { 0x01, 0x3C, 0x54, 0x30, 0xE8, 4, 12, 3, 3, 4, 12 },
    { 0x01, 0xA8, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0xB0, 0x09, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xB8, 0x09, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xC0, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 6, 12 },
    { 0x01, 0xC8, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 6, 12 },
    { 0x00, 0x60, 0x3C, 0x30, 0xE8, 12, 12, 4, 3, 9, 12 },
    { 0x00, 0x6C, 0x3C, 0x30, 0xE8, 12, 12, 3, 3, 12, 12 },
    { 0x01, 0xA4, 0xCD, 0x30, 0xE8, 8, 12, 3, 3, 12, 12 },
    { 0x01, 0xD0, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 11, 12 },
    { 0x01, 0x78, 0x53, 0x30, 0xE8, 4, 12, 3, 3, 4, 12 },
    { 0x01, 0xF8, 0xA8, 0x30, 0xE8, 4, 12, 3, 3, 4, 12 },
    { 0x01, 0x1C, 0x99, 0x30, 0xE8, 4, 12, 3, 3, 4, 12 },
    { 0x01, 0x1C, 0x8D, 0x30, 0xE8, 4, 12, 3, 3, 4, 12 },
    { 0x01, 0xD8, 0x09, 0x30, 0xE8, 8, 12, 3, 3, 8, 12 },
    { 0x01, 0xE0, 0x09, 0x30, 0xE8, 8, 12, 5, 3, 5, 12 },
    { 0x01, 0xE8, 0x09, 0x30, 0xE0, 12, 12, 3, 3, 7, 12 },
    { 0x01, 0x08, 0xF2, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x00, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 6, 12 },
    { 0x01, 0x08, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 6, 12 },
    { 0x01, 0x10, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x18, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x20, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x28, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x00, 0x78, 0x3C, 0x30, 0xE8, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xEC, 0x3E, 0x30, 0xE8, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0x90, 0x3E, 0x30, 0x99, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xBC, 0x3E, 0x30, 0x98, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xC8, 0x3E, 0x30, 0x97, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xD4, 0x3E, 0x30, 0x96, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xE0, 0x3E, 0x30, 0x95, 12, 12, 3, 3, 12, 12 },
    { 0x01, 0x30, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 12, 12 },
    { 0x01, 0x38, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 8, 12 },
    { 0x00, 0x9C, 0x41, 0x30, 0x94, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xA8, 0x41, 0x30, 0x93, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0x54, 0x48, 0x30, 0x92, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0x60, 0x48, 0x30, 0xE0, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x6C, 0x48, 0x30, 0x91, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0x78, 0x48, 0x30, 0x90, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0x84, 0x48, 0x30, 0xE0, 12, 12, 3, 3, 10, 12 },
    { 0x00, 0x90, 0x4A, 0x30, 0x8F, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xB4, 0x4A, 0x30, 0xE0, 12, 12, 3, 3, 11, 12 },
    { 0x00, 0xC0, 0x4A, 0x30, 0x8E, 12, 12, 3, 3, 12, 12 },
    { 0x00, 0xCC, 0x4A, 0x30, 0x8D, 12, 12, 3, 3, 12, 12 },
    { 0x01, 0x40, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x48, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 7, 12 },
    { 0x01, 0x50, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 8, 12 },
    { 0x01, 0x58, 0x12, 0x30, 0xE8, 8, 12, 3, 3, 8, 12 },
    { 0x01, 0xC8, 0x87, 0x30, 0xE8, 4, 12, 0, 0, 12, 12 },
    { 0x00, 0x38, 0xF3, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x6C, 0xA8, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x74, 0xA8, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x7C, 0xA8, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0xF4, 0x14, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x28, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xBC, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xF4, 0x09, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x00, 0xAC, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x9C, 0xC9, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x80, 0xB5, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xAC, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xC4, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x94, 0xC9, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xC4, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x78, 0xF1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x10, 0xAC, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x78, 0xD9, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xC0, 0x9C, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x00, 0xF2, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x40, 0xF3, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x38, 0xA6, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xB4, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x40, 0xAF, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x50, 0xA8, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x50, 0xB4, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x38, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xA4, 0x97, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x10, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x08, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x48, 0xAF, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x80, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xA4, 0xA3, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xCC, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xC0, 0x87, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xA4, 0x8B, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x10, 0xF2, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xA4, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xB4, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x30, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x78, 0xE5, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x50, 0xC0, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x78, 0xCD, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xCC, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x50, 0xEA, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x50, 0xDE, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x00, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x08, 0xAC, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x18, 0x3F, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xAC, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xBC, 0xC1, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0x18, 0xF2, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xCC, 0x9C, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x01, 0xCC, 0x87, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
    { 0x00, 0x30, 0xF3, 0x30, 0xE8, 8, 12, 0, 0, 12, 12 },
};

StcrddekNameTabs stcrddek_name_tabs_jpn[3] = {
    { 2, 3, 4 },
    { 5, 6, 7 },
    { 8, 9, 10 },
};

StcrddekNamePage stcrddek_name_pages_jpn[3] = {
    { {
        { { 1, 67 }, { 1, 69 }, { 1, 71 }, { 1, 73 }, { 1, 75 }, { 1, 133 }, { 0, 0 }, { 1, 135 },
          { 0, 0 }, { 1, 137 }, { 1, 114 }, { 1, 117 }, { 1, 120 }, { 1, 123 }, { 1, 126 } },
        { { 1, 76 }, { 1, 78 }, { 1, 80 }, { 1, 82 }, { 1, 84 }, { 1, 138 }, { 1, 139 }, { 1, 140 },
          { 1, 141 }, { 1, 142 }, { 1, 66 }, { 1, 68 }, { 1, 70 }, { 1, 72 }, { 1, 74 } },
        { { 1, 86 }, { 1, 88 }, { 1, 90 }, { 1, 92 }, { 1, 94 }, { 1, 144 }, { 0, 0 }, { 1, 145 },
          { 0, 0 }, { 1, 146 }, { 1, 132 }, { 1, 134 }, { 1, 136 }, { 1, 100 }, { 0, 0 } },
        { { 1, 96 }, { 1, 98 }, { 1, 101 }, { 1, 103 }, { 1, 105 }, { 1, 77 }, { 1, 79 }, { 1, 81 },
          { 1, 83 }, { 1, 85 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 1, 107 }, { 1, 108 }, { 1, 109 }, { 1, 110 }, { 1, 111 }, { 1, 87 }, { 1, 89 }, { 1, 91 },
          { 1, 93 }, { 1, 95 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 112 }, { 1, 115 }, { 1, 118 }, { 1, 121 }, { 1, 124 }, { 1, 97 }, { 1, 99 }, { 1, 102 },
          { 1, 104 }, { 1, 106 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 127 }, { 1, 128 }, { 1, 129 }, { 1, 130 }, { 1, 131 }, { 1, 113 }, { 1, 116 }, { 1, 119 },
          { 1, 122 }, { 1, 125 }, { 1, 0 }, { 1, 0 }, { -1, 0 }, { 1, 0 }, { -1, 0 } },
    } },
    { {
        { { 1, 148 }, { 1, 150 }, { 1, 152 }, { 1, 154 }, { 1, 156 }, { 1, 214 }, { 0, 0 }, { 1, 216 },
          { 0, 0 }, { 1, 218 }, { 1, 195 }, { 1, 198 }, { 1, 201 }, { 1, 204 }, { 1, 207 } },
        { { 1, 157 }, { 1, 159 }, { 1, 161 }, { 1, 163 }, { 1, 165 }, { 1, 219 }, { 1, 220 }, { 1, 221 },
          { 1, 222 }, { 1, 223 }, { 1, 147 }, { 1, 149 }, { 1, 151 }, { 1, 153 }, { 1, 155 } },
        { { 1, 167 }, { 1, 169 }, { 1, 171 }, { 1, 173 }, { 1, 175 }, { 1, 225 }, { 0, 0 }, { 1, 226 },
          { 0, 0 }, { 1, 227 }, { 1, 213 }, { 1, 215 }, { 1, 217 }, { 1, 181 }, { 1, 228 } },
        { { 1, 177 }, { 1, 179 }, { 1, 182 }, { 1, 184 }, { 1, 186 }, { 1, 158 }, { 1, 160 }, { 1, 162 },
          { 1, 164 }, { 1, 166 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 1, 188 }, { 1, 189 }, { 1, 190 }, { 1, 191 }, { 1, 192 }, { 1, 168 }, { 1, 170 }, { 1, 172 },
          { 1, 174 }, { 1, 176 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 193 }, { 1, 196 }, { 1, 199 }, { 1, 202 }, { 1, 205 }, { 1, 178 }, { 1, 180 }, { 1, 183 },
          { 1, 185 }, { 1, 187 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 208 }, { 1, 209 }, { 1, 210 }, { 1, 211 }, { 1, 212 }, { 1, 194 }, { 1, 197 }, { 1, 200 },
          { 1, 203 }, { 1, 206 }, { 1, 0 }, { 1, 0 }, { -1, 0 }, { 1, 0 }, { -1, 0 } },
    } },
    { {
        { { 1, 14 }, { 1, 15 }, { 1, 16 }, { 1, 17 }, { 1, 18 }, { 1, 40 }, { 1, 41 }, { 1, 42 },
          { 1, 43 }, { 1, 44 }, { 1, 4 }, { 1, 5 }, { 1, 6 }, { 1, 7 }, { 1, 8 } },
        { { 1, 19 }, { 1, 20 }, { 1, 21 }, { 1, 22 }, { 1, 23 }, { 1, 45 }, { 1, 46 }, { 1, 47 },
          { 1, 48 }, { 1, 49 }, { 1, 9 }, { 1, 10 }, { 1, 11 }, { 1, 12 }, { 1, 13 } },
        { { 1, 24 }, { 1, 25 }, { 1, 26 }, { 1, 27 }, { 1, 28 }, { 1, 50 }, { 1, 51 }, { 1, 52 },
          { 1, 53 }, { 1, 54 }, { 1, 232 }, { 1, 233 }, { 1, 229 }, { 1, 230 }, { 1, 231 } },
        { { 1, 29 }, { 1, 30 }, { 1, 31 }, { 1, 32 }, { 1, 33 }, { 1, 55 }, { 1, 56 }, { 1, 57 },
          { 1, 58 }, { 1, 59 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 1, 34 }, { 1, 35 }, { 1, 36 }, { 1, 37 }, { 1, 38 }, { 1, 60 }, { 1, 61 }, { 1, 62 },
          { 1, 63 }, { 1, 64 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 39 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 1, 65 }, { 0, 0 }, { 0, 0 },
          { 0, 0 }, { 0, 0 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
          { 0, 0 }, { 0, 0 }, { 1, 0 }, { 1, 0 }, { -1, 0 }, { 1, 0 }, { -1, 0 } },
    } },
};

StcrddekNameTabs stcrddek_name_tabs_latin[1] = {
    { 8, 9, 10 },
};

StcrddekNamePage stcrddek_name_pages_latin[1] = {
    { {
        { { 1, 14 }, { 1, 15 }, { 1, 16 }, { 1, 17 }, { 1, 18 }, { 1, 40 }, { 1, 41 }, { 1, 42 },
          { 1, 43 }, { 1, 44 }, { 1, 4 }, { 1, 5 }, { 1, 6 }, { 1, 7 }, { 1, 8 } },
        { { 1, 19 }, { 1, 20 }, { 1, 21 }, { 1, 22 }, { 1, 23 }, { 1, 45 }, { 1, 46 }, { 1, 47 },
          { 1, 48 }, { 1, 49 }, { 1, 9 }, { 1, 10 }, { 1, 11 }, { 1, 12 }, { 1, 13 } },
        { { 1, 24 }, { 1, 25 }, { 1, 26 }, { 1, 27 }, { 1, 28 }, { 1, 50 }, { 1, 51 }, { 1, 52 },
          { 1, 53 }, { 1, 54 }, { 1, 232 }, { 1, 233 }, { 1, 229 }, { 1, 230 }, { 1, 231 } },
        { { 1, 29 }, { 1, 30 }, { 1, 31 }, { 1, 32 }, { 1, 33 }, { 1, 55 }, { 1, 56 }, { 1, 57 },
          { 1, 58 }, { 1, 59 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { 1, 34 }, { 1, 35 }, { 1, 36 }, { 1, 37 }, { 1, 38 }, { 1, 60 }, { 1, 61 }, { 1, 62 },
          { 1, 63 }, { 1, 64 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 1, 39 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 1, 65 }, { 0, 0 }, { 0, 0 },
          { 0, 0 }, { 0, 0 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 } },
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
          { 0, 0 }, { 0, 0 }, { 1, 0 }, { 1, 0 }, { -1, 0 }, { 1, 0 }, { -1, 0 } },
    } },
};

MessageFont stcrddek_name_font = { 0xFF, 14, { 0, 0 }, stcrddek_name_glyphs, stcrddek_name_glyphs_ext, font_chars, font_chars_ext, 0xEA, 0x72 };

s32 stcrddek_name_portrait_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};

StcrddekNameButton stcrddek_name_buttons[5] = {
    { 44, 203, 195 },
    { 45, 222, 195 },
    { 60, 203, 153 },
    { 68, 203, 174 },
    { 76, 246, 195 },
};

s32 stcrddek_name_arrow_palettes[6] = { 0, 1, 2, 3, 2, 1 };
