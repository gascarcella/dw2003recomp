#include "common.h"
#include "object.h"
#include "heap.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "psyq/libc2.h"
#include "stdgname.h"

/* STDGNAME.PRO: renaming a party Digimon. The overlay's first object sets up the display; the party member
 * choice (stdgname_party_create) and the name entry (stdgname_entry_create) are run by the main object
 * of stdgname_80086184.c. */

/* A key of the keyboard: 1 = the first cell of a key, 0 = no key, < 0 = the next cells of a wide key (the
 * offset back to its first cell). */
typedef struct StdgnameKey {
    /* 0x0 */ s8 type;
    /* 0x1 */ u8 ch; /* index into the font's unk_0C */
} StdgnameKey; /* size 0x2 */

/* A page of the keyboard: 7 rows of 15 keys. */
typedef struct StdgnameKeyPage {
    /* 0x00 */ StdgnameKey k[7][15];
} StdgnameKeyPage; /* size 0xD2 */

/* A page's tabs: their messages. */
typedef struct StdgnameTabs {
    /* 0x0 */ s32 msg[3];
} StdgnameTabs; /* size 0xC */

/* stdgname_keyboard: the keyboard for the language (stdgname_entry_update). */
typedef struct StdgnameKeyboard {
    /* 0x0 */ s32 pages;
    /* 0x4 */ StdgnameTabs *titles; /* per page */
    /* 0x8 */ StdgnameKeyPage *keys;
} StdgnameKeyboard; /* size 0xC */

/* A sprite and where it goes. */
typedef struct StdgnameSprite {
    /* 0x0 */ s32 sprite;
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} StdgnameSprite; /* size 0xC */

/* Where the party members go (stdgname_member_pos[0]): the first one's position and scaling centre; the
 * others are 43 lines further down. The table has a second entry and ends with x = -1; the code reads only
 * the first. */
typedef struct StdgnameMemberPos {
    /* 0x0 */ s32 x;
    /* 0x4 */ s32 y;
    /* 0x8 */ s32 cx;
    /* 0xC */ s32 cy;
} StdgnameMemberPos; /* size 0x10 */

/* A window of the party member choice (stdgname_party_windows). */
typedef struct StdgnameWindow {
    /* 0x0 */ s32 message;
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} StdgnameWindow; /* size 0xC */

/* A frame of the party member choice (stdgname_party_frames, ended by sprite -1). */
typedef struct StdgnameFrame {
    /* 0x00 */ s32 sprite;
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ s32 x;
    /* 0x0C */ s32 y;
    /* 0x10 */ s32 cx; /* centre for the open/close scaling */
    /* 0x14 */ s32 cy;
    /* 0x18 */ s32 vertical; /* 0: scaled in x, else in y */
} StdgnameFrame; /* size 0x1C */

/* The party member choice (stdgname_party_create, size 0xE0). */
typedef struct StdgnameParty {
    /* 0x00 */ Object base;
    /* 0x50 */ StdgnameMain *main;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 glow_frame; /* cursor colour step */
    /* 0x5C */ struct {
        s32 frame;
        s32 time;
    } member_anims[3]; /* per member: its animation */
    /* 0x74 */ s32 unk_74;
    /* 0x78 */ s32 member_count; /* party members */
    /* 0x7C */ s32 cursor_frame; /* cursor colour step */
    /* 0x80 */ WindowAnim anims[6];
} StdgnameParty; /* size 0xE0 */

/* The name entry's data block (0x34 bytes): its text windows. */
typedef struct StdgnameNameEntryData {
    /* 0x00 */ MessageWindow *title;
    /* 0x04 */ MessageWindow *name;   /* the name */
    /* 0x08 */ MessageWindow *tabs[6];   /* the tabs */
    /* 0x20 */ MessageWindow *left_arrow;
    /* 0x24 */ MessageWindow *right_arrow;
    /* 0x28 */ MessageWindow *l1_window;
    /* 0x2C */ MessageWindow *r1_window;
    /* 0x30 */ MessageWindow *warning;
} StdgnameNameEntryData; /* size 0x34 */

extern MessageFont stdgname_name_font;
extern s32 stdgname_entry_anims[][7];
extern StdgnameSprite stdgname_key_buttons[5];
extern s32 stdgname_arrow_palettes[6];
extern StdgnameFrame stdgname_party_frames[];
extern StdgnameMemberPos stdgname_member_pos[3];
extern StdgnameWindow stdgname_party_windows[];
extern s32 stdgname_party_anims[][7];
extern StdgnameTabs stdgname_key_tabs_jpn[];
extern StdgnameKeyPage stdgname_key_pages_jpn[];
extern StdgnameTabs stdgname_key_tabs_latin[];
extern StdgnameKeyPage stdgname_key_pages_latin[];
StdgnameKeyboard stdgname_keyboard;

void stdgname_entry_run(StdgnameNameEntry *obj, StdgnameNameEntryData *data);
void stdgname_entry_update(StdgnameNameEntry *obj, StdgnameNameEntryData *data);
void stdgname_party_draw(StdgnameParty *obj, MessageWindow **data);
void stdgname_party_update(StdgnameParty *obj, MessageWindow **data);

/* The overlay's first object: sets up the display and creates the main object. */
void stdgname_update_main(Object *obj, StdgnameMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x5000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stdgname_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stdgname_start(void) {
    return object_new(stdgname_update_main, sizeof(Object), sizeof(StdgnameMain *));
}

void stdgname_fade_start(Fade *obj, s32 dir, s32 frames) {
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

void stdgname_fade_draw(Fade *obj) {
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

void stdgname_fade_update(Fade *obj) {
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
            stdgname_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stdgname_fade_create(void) {
    Fade *obj = object_new(stdgname_fade_update, sizeof(Fade), 0);

    obj->start = stdgname_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    return obj;
}

/* window_anim_start (include/window_anim.h), byte for byte. */
void stdgname_window_anim_start(WindowAnim *anim, s32 open) {
    anim->running = 1;
    if (open) {
        sound_module.play(0x40019);
        anim->step = 0x1000 / anim->duration;
        anim->level = 0;
    } else {
        sound_module.play(0x4001A);
        anim->level = 0x1000;
        anim->step = -(0x1000 / anim->duration * 2);
    }
}

/* window_anim_update (include/window_anim.h), byte for byte. */
s32 stdgname_window_anim_update(WindowAnim *anim) {
    if (anim->running == 0) {
        return 1;
    }
    anim->level += anim->step;
    if (anim->step > 0) {
        if (anim->level > 0x1000) {
            anim->level = 0x1000;
            anim->running = 0;
            return 1;
        }
    } else if (anim->level < 0) {
        anim->level = 0;
        anim->running = 0;
        return 1;
    }
    return 0;
}

/* Creates the name entry's text windows. */
void stdgname_entry_create_windows(StdgnameNameEntry *obj, StdgnameNameEntryData *data) {
    s32 i;
    MessageWindow *win;

    data->title = win = message_create_window(obj->layer_id, 1, 0x20, 0x1A);
    win->set_palette(win, 4);
    data->name = win = message_create_window(obj->layer_id, 1, 0x4B, 0x40);
    win->set_fixed_size(win, 0x13, 0);
    data->name->font = &stdgname_name_font;
    for (i = 0; i < 3; i++) {
        data->tabs[i] = win = message_create_window(obj->layer_id, 1, i * 0x4E + 0x2F, 0x5B);
        win->set_ot_depth(win, obj->ot_depth - 1);
        data->tabs[i]->set_page_lines(data->tabs[i], 7);
        data->tabs[i]->set_fixed_size(data->tabs[i], 0xE, 0x12);
        data->tabs[i]->font = &stdgname_name_font;
    }
    data->left_arrow = message_create_window(obj->layer_id, 1, 0xCE, 0xC6);
    data->right_arrow = message_create_window(obj->layer_id, 1, 0xE1, 0xC6);
    data->l1_window = win = message_create_window(obj->layer_id, 1, 0x13, 0x62);
    win->set_ot_depth(win, obj->ot_depth - 1);
    data->r1_window = win = message_create_window(obj->layer_id, 1, 0x123, 0x62);
    win->set_ot_depth(win, obj->ot_depth - 1);
    data->warning = message_create_window(obj->layer_id, 1, 0x3E, 0x72);
}

/* Shows (`show`) or hides the name entry's texts. */
void stdgname_entry_show_text(StdgnameNameEntry *obj, StdgnameNameEntryData *data, s32 show) {
    s32 i;

    if (show) {
        data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x86), 1);
        data->name->copy_text(data->name, (u8 *)obj->name);
        data->name->set_palette(data->name, 1);
        for (i = 0; i < 3; i++) {
            data->tabs[i]->set_text(data->tabs[i], cdload_module.files.get_file(records_language + 0x86),
                                     stdgname_keyboard.titles[obj->key_page].msg[i]);
            data->tabs[i]->set_palette(data->tabs[i], 1);
        }
        data->left_arrow->set_text(data->left_arrow, cdload_module.files.get_file(records_language + 0x86), 0xD);
        data->left_arrow->set_palette(data->left_arrow, 1);
        data->right_arrow->set_text(data->right_arrow, cdload_module.files.get_file(records_language + 0x86), 0xE);
        data->right_arrow->set_palette(data->right_arrow, 1);
        if (stdgname_keyboard.pages >= 2) {
            data->l1_window->set_text(data->l1_window, cdload_module.files.get_file(records_language + 0x86), 0x10);
            data->l1_window->set_palette(data->l1_window, 1);
            data->r1_window->set_text(data->r1_window, cdload_module.files.get_file(records_language + 0x86), 0x11);
            data->r1_window->set_palette(data->r1_window, 1);
        }
    } else {
        data->title->set_visible(data->title, 0);
        data->name->set_visible(data->name, 0);
        for (i = 0; i < 3; i++) {
            data->tabs[i]->set_visible(data->tabs[i], 0);
        }
        data->left_arrow->set_visible(data->left_arrow, 0);
        data->right_arrow->set_visible(data->right_arrow, 0);
        data->l1_window->set_visible(data->l1_window, 0);
        data->r1_window->set_visible(data->r1_window, 0);
    }
}

/* Draws the name entry: the keyboard cursor, the frame, the Digimon, the tabs and the buttons. */
void stdgname_entry_draw(StdgnameNameEntry *obj) {
    Sprite spr;
    s32 k;
    s32 i;

    sprite_init(&spr);
    spr.set_vram_pos(obj->tex_x, obj->tex_y);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (obj->keyboard_anim.level != 0) {
        if (obj->input_enabled != 0) {
            if (gfx_module.funcs.get_time() - obj->cursor_time >= 5) {
                obj->cursor_time = gfx_module.funcs.get_time();
                if (++obj->cursor_frame >= 4) {
                    obj->cursor_frame = 0;
                }
            }
            spr.set_palette(obj->cursor_frame);
            if (obj->key_column < 10 || obj->key_row < 3) {
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x27, obj->key_column * 14 + 0x2F + obj->key_column / 5 * 8,
                           obj->key_row * 18 + 0x5A);
            } else {
                for (k = 0; stdgname_keyboard.keys[obj->key_page].k[obj->key_row][obj->key_column + k].type != 1; k--) {
                }
                switch (obj->key_column + k + obj->key_row * 15) {
                case 0x64:
                default:
                    i = 0;
                    break;
                case 0x65:
                    i = 1;
                    break;
                case 0x46:
                    i = 2;
                    stdgname_key_buttons[2].sprite = records_language + 0x3C;
                    break;
                case 0x55:
                    i = 3;
                    stdgname_key_buttons[3].sprite = records_language + 0x44;
                    break;
                case 0x67:
                    i = 4;
                    stdgname_key_buttons[4].sprite = records_language + 0x4C;
                    break;
                }
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), stdgname_key_buttons[i].sprite, stdgname_key_buttons[i].x,
                           stdgname_key_buttons[i].y);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x28, obj->name_cursor * 19 + 0x4B, 0x40);
            spr.set_palette(0);
        }
        if (obj->keyboard_anim.level != 0x1000) {
            spr.set_scale(obj->keyboard_anim.level, 0x1000, 0x1000);
        }
        if (obj->keyboard_anim.level != 0x1000) {
            spr.set_pivot(0x18, 0x20);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x1D, 0x18, 0x15);
        if (obj->mode != 2) {
            if (obj->keyboard_anim.level != 0x1000) {
                spr.set_pivot(0x20, 0x3F);
            }
            if (obj->digimon != -1) {
                if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
                    obj->frame_time = gfx_module.funcs.get_time();
                    if (++obj->anim_frame >= 7 || stdgname_entry_anims[obj->digimon][obj->anim_frame] == -1) {
                        obj->anim_frame = 0;
                    }
                }
                spr.draw(cdload_module.get_subfile_by_id(0x07700000), stdgname_entry_anims[obj->digimon][obj->anim_frame], 0x22, 0x30);
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
        if (obj->keyboard_anim.level != 0x1000) {
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
        if (obj->keyboard_anim.level != 0x1000) {
            spr.set_scale(0x1000, obj->keyboard_anim.level, 0x1000);
        }
        if (stdgname_keyboard.pages >= 2) {
            if (gfx_module.funcs.get_time() - obj->arrows_time >= 7) {
                obj->arrows_time = gfx_module.funcs.get_time();
                if (++obj->arrows_frame >= 6) {
                    obj->arrows_frame = 0;
                }
            }
            spr.set_palette(stdgname_arrow_palettes[obj->arrows_frame]);
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
    if (obj->warning_anim.level != 0) {
        if (obj->warning_anim.level != 0x1000) {
            spr.set_scale(0x1000, obj->warning_anim.level, 0x1000);
            spr.set_pivot(0, 0x78);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x07700000), 0x26, 0, 0x64);
    }
}

#define STDGNAME_KEY(obj) (stdgname_keyboard.keys[(obj)->key_page].k[(obj)->key_row][(obj)->key_column])
#define STDGNAME_SWAP(c) ((u16)(((c) >> 8) | (((c) & 0xFF) << 8)))
#define STDGNAME_SPACE 0x4081 /* a full-width space (Shift-JIS 0x8140, byte-swapped) */

/* The name entry's input and its steps (base.step): move on the keyboard, type, delete, end. */
void stdgname_entry_run(StdgnameNameEntry *obj, StdgnameNameEntryData *data) {
    s32 old;
    s32 k;
    s32 i;
    s32 j;
    u16 c;
    u32 glyph; /* the typed character: its own u32 (DECISIONS "Independent EU-only project on the unpatched disc") */

    switch (obj->base.step) {
    case 0:
    default:
        stdgname_window_anim_start(&obj->keyboard_anim, 1);
        obj->base.step++;
        break;
    case 1:
        if (stdgname_window_anim_update(&obj->keyboard_anim)) {
            stdgname_entry_show_text(obj, data, 1);
            obj->input_enabled = 1;
            obj->base.step++;
        }
        break;
    case 2:
        if (PAD_PRESSED(3)) {
            sound_module.play(0x4001B);
            obj->key_column = 13;
            obj->key_row = 6;
            break;
        }
        if (stdgname_keyboard.pages >= 2) {
            old = obj->key_page;
            if (!PAD_HELD(0xB) && PAD_PRESSED(0xA)) {
                if (--obj->key_page < 0) {
                    obj->key_page = stdgname_keyboard.pages - 1;
                }
            } else if (!PAD_HELD(0xA) && PAD_PRESSED(0xB)) {
                if (++obj->key_page > stdgname_keyboard.pages - 1) {
                    obj->key_page = 0;
                }
            }
            if (old != obj->key_page) {
                sound_module.play(0x4001B);
                stdgname_entry_show_text(obj, data, 1);
                if (records_language == 0) {
                    if ((u32)obj->key_page < 2) {
                        while (STDGNAME_KEY(obj).type != 1) {
                            if (--obj->key_column < 0) {
                                obj->key_column = 14;
                            }
                        }
                    } else {
                        while (STDGNAME_KEY(obj).type == 0) {
                            if (--obj->key_row < 0) {
                                obj->key_row = 6;
                            }
                        }
                    }
                }
            }
        }
        if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
            if (STDGNAME_KEY(obj).type < 0) {
                obj->key_column += STDGNAME_KEY(obj).type;
            }
            do {
                if (--obj->key_column < 0) {
                    obj->key_column = 14;
                }
            } while (STDGNAME_KEY(obj).type != 1);
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
            if (STDGNAME_KEY(obj).type < 0) {
                obj->key_column += STDGNAME_KEY(obj).type;
            }
            do {
                if (++obj->key_column >= 15) {
                    obj->key_column = 0;
                }
            } while (STDGNAME_KEY(obj).type != 1);
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            do {
                if (--obj->key_row < 0) {
                    obj->key_row = 6;
                }
            } while (STDGNAME_KEY(obj).type == 0);
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            do {
                if (++obj->key_row >= 7) {
                    obj->key_row = 0;
                }
            } while (STDGNAME_KEY(obj).type == 0);
            sound_module.play(0x4001B);
        }
        if (PAD_PRESSED(0xD)) {
            for (k = 0; stdgname_keyboard.keys[obj->key_page].k[obj->key_row][obj->key_column + k].type != 1;
                 k--) {
            }
            i = obj->key_row * 15 + obj->key_column + k;
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
                obj->name[obj->name_cursor] = STDGNAME_SWAP(c);
                if (++obj->name_cursor > obj->length - 1) {
                    obj->name_cursor = obj->length - 1;
                }
                data->name->copy_text(data->name, (u8 *)obj->name);
                break;
            case 0x67:
                for (j = 0; j < obj->length; j++) {
                    if (obj->name[j] != STDGNAME_SPACE && obj->name[j] != 0) {
                        for (j = 19; j >= 0; j--) {
                            if (obj->name[j] != STDGNAME_SPACE) {
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
                glyph = data->name->font->chars[STDGNAME_KEY(obj).ch].sjis;
                obj->name[obj->name_cursor] = STDGNAME_SWAP(glyph);
                data->name->copy_text(data->name, (u8 *)obj->name);
                if (++obj->name_cursor > obj->length - 1) {
                    obj->name_cursor = obj->length - 1;
                    obj->key_column = 13;
                    obj->key_row = 6;
                }
                break;
            case 0x46:
                if (obj->name_cursor <= obj->length - 1 && obj->name[obj->name_cursor] == STDGNAME_SPACE) {
                    if (--obj->name_cursor < 0) {
                        obj->name_cursor = 0;
                    }
                }
                c = data->name->font->chars_ext[1].sjis;
                obj->name[obj->name_cursor] = STDGNAME_SWAP(c);
                data->name->copy_text(data->name, (u8 *)obj->name);
                break;
            }
        } else if (PAD_PRESSED(0xE)) {
            sound_module.play(0x800450BD);
            if (obj->name_cursor <= obj->length - 1 && obj->name[obj->name_cursor] == STDGNAME_SPACE) {
                if (--obj->name_cursor < 0) {
                    obj->name_cursor = 0;
                }
            }
            c = data->name->font->chars_ext[1].sjis;
            obj->name[obj->name_cursor] = STDGNAME_SWAP(c);
            data->name->copy_text(data->name, (u8 *)obj->name);
        }
        break;
    case 10:
        stdgname_entry_show_text(obj, data, 0);
        stdgname_window_anim_start(&obj->keyboard_anim, 0);
        obj->input_enabled = 0;
        obj->base.step++;
        break;
    case 11:
        if (stdgname_window_anim_update(&obj->keyboard_anim)) {
            obj->base.state = OBJECT_STATE_DONE;
        }
        break;
    case 20:
        obj->input_enabled = 0;
        stdgname_window_anim_start(&obj->warning_anim, 1);
        obj->base.step++;
        break;
    case 21:
        if (stdgname_window_anim_update(&obj->warning_anim)) {
            data->warning->set_text(data->warning, cdload_module.files.get_file(records_language + 0x86), 0x12);
            obj->base.step++;
        }
        break;
    case 22:
        if (PAD_PRESSED(0xD)) {
            data->warning->set_visible(data->warning, 0);
            stdgname_window_anim_start(&obj->warning_anim, 0);
            obj->base.step++;
        }
        break;
    case 23:
        if (stdgname_window_anim_update(&obj->warning_anim)) {
            obj->input_enabled = 1;
            obj->base.step = 2;
        }
        break;
    case 100:
        break;
    }
}

void stdgname_entry_update(StdgnameNameEntry *obj, StdgnameNameEntryData *data) {
    Tim tim;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        tim_init(&tim);
        tim.set_image_pos(obj->tex_x, obj->tex_y);
        tim.load_all(cdload_module.get_subfile_by_id(0x07710000));
        if (records_language == 0) {
            stdgname_keyboard.pages = 3;
            stdgname_keyboard.titles = stdgname_key_tabs_jpn;
            stdgname_keyboard.keys = stdgname_key_pages_jpn;
        } else {
            stdgname_keyboard.pages = 1;
            stdgname_keyboard.titles = stdgname_key_tabs_latin;
            stdgname_keyboard.keys = stdgname_key_pages_latin;
        }
        obj->unk_C0.duration = 10;
        obj->warning_anim.duration = 10;
        obj->keyboard_anim.duration = 10;
        stdgname_entry_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        stdgname_entry_run(obj, data);
        stdgname_entry_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void stdgname_entry_set_texture_pos(StdgnameNameEntry *obj, s32 x, s32 y) {
    obj->tex_x = x;
    obj->tex_y = y;
}

/* Converts `name` to the entry's two-byte characters and pads it with spaces. */
void stdgname_entry_set_name(StdgnameNameEntry *obj, u8 *name) {
    Font font;
    s32 i;

    font_init(&font);
    font.convert_text((u8 *)obj->name, name, 0);
    for (i = strlen((char *)obj->name) >> 1; i < obj->length; i++) {
        obj->name[i] = 0x4081;
    }
}

/* Writes the name to `dst` (StdgnameNameEntry.get_name), without the trailing and leading spaces. */
void stdgname_entry_get_name(StdgnameNameEntry *obj, u8 *dst) {
    Font font;
    s32 i;

    for (i = 0; i < obj->length * 2; i++) {
        dst[i] = 0;
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
    font.convert_text(dst, (u8 *)&obj->name[i], 1);
}

/* Closes the name entry (StdgnameNameEntry.close). */
void stdgname_entry_close(StdgnameNameEntry *obj) {
    obj->base.step = 10;
}

StdgnameNameEntry *stdgname_entry_create(GamestateRecord *digimon, s32 index) {
    StdgnameNameEntry *obj =
        object_new(stdgname_entry_update, sizeof(StdgnameNameEntry), sizeof(StdgnameNameEntryData));

    obj->get_name = stdgname_entry_get_name;
    obj->close = stdgname_entry_close;
    obj->layer_id = 0x1000;
    obj->ot_depth = 3;
    obj->mode = 1;
    obj->digimon = index;
    if (records_language == 0) {
        obj->length = 5;
    } else {
        obj->length = 8;
    }
    stdgname_entry_set_name(obj, (u8 *)digimon);
    stdgname_entry_set_texture_pos(obj, 0x280, 0x100);
    return obj;
}

/* Shows (`show`) or hides window `i` of the party member choice (2-4: the members' names). */
void stdgname_party_show_window(StdgnameParty *obj, MessageWindow **win, s32 i, s32 show) {
    s32 layer = 0;
    GamestateRecord *digimon;

    if (i < 5) {
        layer = obj->layer_id;
    }
    if (show) {
        if (*win == NULL) {
            *win = message_create_window(layer, 1, stdgname_party_windows[i].x, stdgname_party_windows[i].y);
            (*win)->set_ot_depth(*win, 1);
        }
        if ((u32)(i - 2) < 3) {
            digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(i - 2));
            (*win)->font = stdgname_funcs.font;
            (*win)->set_text(*win, (u8 *)digimon->name, -1);
        } else {
            (*win)->set_text(*win, cdload_module.files.get_file(records_language + 0x40), stdgname_party_windows[i].message);
        }
        (*win)->set_palette(*win, 0);
    } else if (*win != NULL) {
        (*win)->set_visible(*win, 0);
    }
}

/* Draws the party member choice: its frames, the members and the cursor. */
void stdgname_party_draw(StdgnameParty *obj, MessageWindow **data) {
    Sprite spr;
    s32 i;
    s32 m;
    s32 sx;
    s32 sy;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 2);
    spr.set_vram_pos(0x280, 0);
    for (i = 0; i < 3; i++) {
        if (stdgname_party_frames[i].sprite == -1) {
            break;
        }
        if (obj->anims[i].level != 0x1000) {
            sy = obj->anims[i].level;
            sx = 0x1000;
            if (stdgname_party_frames[i].vertical == 0) {
                sx = sy;
                sy = 0x1000;
            }
            spr.set_scale(sx, sy, 0x1000);
            if (stdgname_party_frames[i].sprite == 0x20) {
                spr.set_pivot(stdgname_party_frames[i].cx, stdgname_party_frames[i].cy + stdgname_funcs.member * 43);
            } else {
                spr.set_pivot(stdgname_party_frames[i].cx, stdgname_party_frames[i].cy);
            }
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        if (stdgname_party_frames[i].sprite == 0x20) {
            if ((gfx_module.funcs.get_time() & 3) == 3) {
                if (++obj->cursor_frame >= 16) {
                    obj->cursor_frame = 0;
                }
            }
            spr.set_palette(obj->cursor_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02880000), stdgname_party_frames[i].sprite, stdgname_party_frames[i].x, stdgname_party_frames[i].y + stdgname_funcs.member * 43);
            spr.set_palette(0);
        } else {
            spr.draw(cdload_module.get_subfile_by_id(0x02880000), stdgname_party_frames[i].sprite, stdgname_party_frames[i].x, stdgname_party_frames[i].y);
        }
    }
    for (i = 0; i < obj->member_count; i++) {
        m = gamestate_data.funcs.get_party_digimon(i);
        if (m >= 0 && obj->anims[i + 3].level != 0) {
            if (gfx_module.funcs.get_time() - obj->member_anims[i].time >= 13) {
                obj->member_anims[i].time = gfx_module.funcs.get_time();
                if (stdgname_party_anims[m][++obj->member_anims[i].frame] == -1) {
                    obj->member_anims[i].frame = 0;
                }
            }
            if (obj->anims[i + 3].level != 0x1000) {
                spr.set_scale(obj->anims[i + 3].level, 0x1000, 0x1000);
                spr.set_pivot(stdgname_member_pos[0].cx + 2, stdgname_member_pos[0].cy + 2);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02880000), stdgname_party_anims[m][obj->member_anims[i].frame],
                       stdgname_member_pos[0].x + 2, stdgname_member_pos[0].y + 2 + i * 43);
        }
    }
    if ((gfx_module.funcs.get_time() & 3) == 3) {
        if (++obj->glow_frame >= 14) {
            obj->glow_frame = 0;
        }
    }
    for (i = 0; i < obj->member_count; i++) {
        if (gamestate_data.funcs.get_party_digimon(i) >= 0 && obj->anims[i + 3].level != 0) {
            if (obj->anims[i + 3].level != 0x1000) {
                spr.set_scale(obj->anims[i + 3].level, 0x1000, 0x1000);
                spr.set_pivot(stdgname_member_pos[0].cx, stdgname_member_pos[0].cy + i * 43);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            spr.set_layer_id(obj->layer_id, 1);
            spr.set_palette(obj->glow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02880000), 0x1F, stdgname_member_pos[0].x, stdgname_member_pos[0].y + i * 43);
            spr.set_layer_id(obj->layer_id, 2);
            spr.set_palette(0);
            spr.draw(cdload_module.get_subfile_by_id(0x02880000), 0x1E, stdgname_member_pos[0].x, stdgname_member_pos[0].y + i * 43);
        }
    }
}

/* The party member choice's input; returns 1 when chosen or cancelled. */
s32 stdgname_party_input(StdgnameParty *obj, MessageWindow **data) {
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        if (--stdgname_funcs.member < 0) {
            stdgname_funcs.member = obj->member_count - 1;
        }
        sound_module.play(0x4001B);
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        if (++stdgname_funcs.member > obj->member_count - 1) {
            stdgname_funcs.member = 0;
        }
        sound_module.play(0x4001B);
    }
    if (PAD_PRESSED(0xD)) {
        sound_module.play(0x4001C);
        obj->main->member = stdgname_funcs.member;
        return 1;
    }
    if (PAD_PRESSED(0xE)) {
        sound_module.play(0x800450BD);
        obj->main->member = -1;
        obj->main->fade_out(obj->main);
        return 1;
    }
    return 0;
}

void stdgname_party_update(StdgnameParty *obj, MessageWindow **data) {
    s32 i;
    s32 j;
    s32 n;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 5; i >= 0; i--) {
            obj->anims[i].duration = 10;
        }
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_digimon(i) >= 0) {
                obj->member_count++;
            }
        }
        break;
    case OBJECT_STATE_RUN:
        n = 0;
        switch (obj->base.step) {
        case 0:
        default:
            stdgname_funcs.anim_start(&obj->anims[0], 1);
            stdgname_funcs.anim_start(&obj->anims[1], 1);
            obj->base.step++;
            break;
        case 1:
            n += stdgname_funcs.anim_update(&obj->anims[0]);
            n += stdgname_funcs.anim_update(&obj->anims[1]);
            if (n == 2) {
                stdgname_party_show_window(obj, &data[0], 0, 1);
                stdgname_party_show_window(obj, &data[1], 1, 1);
                stdgname_funcs.anim_start(&obj->anims[3], 1);
                obj->base.next_step(obj);
            }
            break;
        case 2:
            if (stdgname_funcs.anim_update(&obj->anims[obj->base.substep + 3]) != 0) {
                stdgname_party_show_window(obj, &data[obj->base.substep + 2], obj->base.substep + 2, 1);
                if (obj->base.substep < obj->member_count - 1) {
                    obj->base.substep++;
                    stdgname_funcs.anim_start(&obj->anims[obj->base.substep + 3], 1);
                } else {
                    stdgname_funcs.anim_start(&obj->anims[2], 1);
                    obj->base.next_step(obj);
                }
            }
            break;
        case 4:
            if (stdgname_party_input(obj, data) != 0) {
                obj->base.step++;
                for (i = 0; i < 6; i++) {
                    stdgname_funcs.anim_start(&obj->anims[i], 0);
                    stdgname_party_show_window(obj, &data[i], i, 0);
                }
            }
            break;
        case 3:
        case 5:
            if (stdgname_funcs.anim_update(&obj->anims[2]) != 0) {
                obj->base.step++;
            }
            break;
        case 6:
            for (j = 0; j < 6; j++) {
                n += stdgname_funcs.anim_update(&obj->anims[j]);
            }
            if (n == 6) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        stdgname_party_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *stdgname_party_create(StdgnameMain *main) {
    /* The data block: 10 window slots, of which party_update uses 0-5. */
    StdgnameParty *obj = object_new(stdgname_party_update, sizeof(StdgnameParty), 10 * sizeof(MessageWindow *));

    obj->main = main;
    obj->layer_id = 0x1000;
    return (Object *)obj;
}

/* The name entry's font: its glyphs (characters from 4, then 114 more from 1), with the EXE's character tables. */
FontGlyph stdgname_name_glyphs[230] = {
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

FontGlyph stdgname_name_glyphs_ext[114] = {
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

StdgnameTabs stdgname_key_tabs_jpn[3] = {
    { 2, 3, 4 },
    { 5, 6, 7 },
    { 8, 9, 10 },
};

StdgnameKeyPage stdgname_key_pages_jpn[3] = {
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

StdgnameTabs stdgname_key_tabs_latin[1] = {
    { 8, 9, 10 },
};

StdgnameKeyPage stdgname_key_pages_latin[1] = {
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

MessageFont stdgname_name_font = { 0xFF, 14, { 0, 0 }, stdgname_name_glyphs, stdgname_name_glyphs_ext, font_chars, font_chars_ext, 0xEA, 0x72 };

s32 stdgname_entry_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};

StdgnameSprite stdgname_key_buttons[5] = {
    { 44, 203, 195 },
    { 45, 222, 195 },
    { 60, 203, 153 },
    { 68, 203, 174 },
    { 76, 246, 195 },
};

s32 stdgname_arrow_palettes[6] = { 0, 1, 2, 3, 2, 1 };

StdgnameFrame stdgname_party_frames[15] = {
    { 29, 10, 24, 21, 24, 32, 0 },
    { 35, 10, 198, 192, 320, 206, 0 },
    { 32, 10, 28, 49, 109, 71, 1 },
    { 44, 10, 203, 195, 160, 153, 1 },
    { 45, 10, 222, 195, 160, 153, 1 },
    { 46, 10, 203, 153, 160, 153, 1 },
    { 47, 10, 203, 174, 160, 153, 1 },
    { 48, 10, 246, 195, 160, 153, 1 },
    { 50, 10, 10, 94, 160, 153, 1 },
    { 51, 10, 277, 94, 160, 153, 1 },
    { 37, 10, 29, 84, 160, 153, 1 },
    { 39, 10, 47, 90, 0, 0, 0 },
    { 40, 10, 75, 64, 0, 0, 0 },
    { 38, 5, 0, 100, 0, 120, 1 },
    { -1, 0, 0, 0, 0, 0, 0 },
};

StdgnameMemberPos stdgname_member_pos[3] = {
    { 32, 53, 32, 79 },
    { 32, 46, 32, 73 },
    { -1, 0, 0, 0 },
};

StdgnameWindow stdgname_party_windows[16] = {
    { 1, 32, 26 },
    { 2, 216, 200 },
    { 0, 75, 71 },
    { 0, 75, 114 },
    { 0, 75, 157 },
    { 3, 32, 26 },
    { 0, 75, 64 },
    { 16, 206, 198 },
    { 17, 225, 198 },
    { 13, 212, 155 },
    { 14, 213, 176 },
    { 15, 259, 197 },
    { 18, 20, 98 },
    { 19, 287, 98 },
    { 20, 77, 113 },
    { -1, 0, 0 },
};

s32 stdgname_party_anims[8][7] = {
    { 7, 8, 9, 10, 9, 8, -1 },
    { 14, 15, 16, 15, -1, -1, -1 },
    { 11, 12, 13, 12, -1, -1, -1 },
    { 3, 4, 5, 6, 5, 4, -1 },
    { 25, 26, 27, 28, 27, 26, -1 },
    { 0, 1, 2, 1, -1, -1, -1 },
    { 17, 18, 19, 20, 19, 18, -1 },
    { 21, 22, 23, 24, 23, 22, -1 },
};
