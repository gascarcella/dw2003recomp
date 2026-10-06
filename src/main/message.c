#include "common.h"

#include "object.h"
#include "gamestate.h"
#include "sound.h"
#include "heap.h"
#include "cdload.h"
#include "pad.h"
#include "gfx.h"
#include "message.h"
#include "psyq/libc2.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"

/* message_draw_window's per-character state (on its stack), arg2 of the control-code handlers. */
typedef struct MessageDrawState {
    /* 0x00 */ union {
        u8 *ptr;
        SPRT *sprt;
        POLY_FT4 *ft4;
        DR_TPAGE *tpage;
    } packet; /* next primitive (gfx_get_packet) */
    /* 0x04 */ GfxLayer *layer;  /* layer */
    /* 0x08 */ u32 *ot_entry; /* ordering table entry */
    /* 0x0C */ struct FontGlyph *glyph;  /* the glyph to draw */
    /* 0x10 */ s32 second_line; /* where the page's second line starts: the next page starts there (message_code_newline) */
    /* 0x14 */ s16 code;   /* the character (message_decode_char's result) */
    /* 0x16 */ s16 type;   /* its high byte: 0/1 glyph, 2 control code, 3 unknown, 4 end */
    /* 0x18 */ s16 line_count;  /* lines started on the page */
} MessageDrawState;

/* The data block of the objects of message_create_cursor. */
typedef struct MessageCursorData {
    /* 0x0 */ MessageWindow *window;
} MessageCursorData; /* size 0x4 */

/* The frames of the cursor that message_cursor_update shows (2-byte Shift-JIS strings, in .sdata). */
extern u8 *message_cursor_frames[];

/* font's tables (font.c), for the fonts message_fonts. */
extern FontChar font_chars[];
extern FontChar font_chars_ext[];
extern FontGlyph font_glyphs_14[];
extern FontGlyph font_glyphs_14_ext[];
extern FontGlyph font_glyphs_11[];
extern FontGlyph font_glyphs_11_ext[];
extern FontGlyph font_glyphs_9[];
extern FontGlyph font_glyphs_9_ext[];

/* Header of RLEN data (magic "RLEN", then the unpacked size); the packed data follows. */
#define MESSAGE_RLEN_MAGIC 0x4E454C52

/* The object type created by message_box_frame_create (size 0x6C). */
typedef struct MessageBoxFrame {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id;
    /* 0x54 */ s32 close_time; /* time of the last close_frame step */
    /* 0x58 */ s32 close_frame; /* step 2 (closing): palette 0..4, then the frame ends */
    /* 0x5C */ s32 show_arrow;  /* the window waits for a button: draw the arrow */
    /* 0x60 */ s32 arrow_time;  /* time of the last arrow_frame step */
    /* 0x64 */ s32 arrow_frame; /* the arrow's palette, 0..4 */
    /* 0x68 */ s16 scale;       /* step 0 (opening): x scale, up to 0x1000 */
    /* 0x6A */ s16 scale_speed;
} MessageBoxFrame; /* size 0x6C */

/* The data block (object_new's third argument = its size) of the objects of message_box_create. */
typedef struct MessageBoxData {
    /* 0x0 */ MessageWindow *window;
    /* 0x4 */ MessageBoxFrame *frame;
} MessageBoxData;

/* A screen offset. */
typedef struct MessageOffset {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} MessageOffset;

/* An entry of the table at message_dialog_layouts (4 of them, indexed by the type of message_create_dialog's
 * objects). */
typedef struct MessageDialogLayout {
    /* 0x00 */ s32 tail_sprite; /* sprite of message_dialog_draw_frame's first part (the tail): 3, 5, 4, 6 */
    /* 0x04 */ MessageOffset tail_pos; /* message_dialog_draw_frame's parts, walked as an array: the tail, */
    /* 0x08 */ MessageOffset left_pos; /* the left edge (sprite 0; also where message_dialog_outline_create's
                                     * outlines start), */
    /* 0x0C */ MessageOffset fill_pos; /* the semi-transparent fill (width x height), */
    /* 0x10 */ MessageOffset right_pos; /* the right edge (sprite 2) */
    /* 0x14 */ MessageOffset name_pos; /* the first window (a speaker's name, control code 7) */
    /* 0x18 */ MessageOffset text_pos; /* the second window */
    /* 0x1C */ MessageOffset arrow_pos; /* message_dialog_draw_arrow's sprite */
} MessageDialogLayout; /* size 0x20 */

/* The object type created by message_dialog_outline_create (size 0xC0). */
typedef struct MessageDialogOutline {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 x;       /* the rectangle: dialog position + offset */
    /* 0x52 */ s16 y;
    /* 0x54 */ s16 pivot_x; /* scaled about this point (the dialog's position) */
    /* 0x56 */ s16 pivot_y;
    /* 0x58 */ s16 w;       /* the dialog's width + 0x20 */
    /* 0x5A */ s16 h;
    /* 0x5C */ s16 scale_speed; /* added to scale per frame: 0x199 opening, 0x333 closing */
    /* 0x5E */ u8 unk_5E[0x2];
    /* 0x60 */ s32 scale;    /* 0..0x1000 */
    /* 0x64 */ s32 layer_id;
    /* 0x68 */ s32 unk_68;
    /* 0x6C */ s16 offset_x; /* from the layout's left_pos (minus the width for types 0 and 1) */
    /* 0x6E */ s16 offset_y;
    /* 0x70 */ VECTOR scale_vec;
    /* 0x80 */ SVECTOR rotation;
    /* 0x88 */ s32 unk_88;
    /* 0x8C */ s32 unk_8C;
    /* 0x90 */ s32 unk_90;
    /* 0x94 */ u8 unk_94[0x4];
    /* 0x98 */ MATRIX matrix;
    /* 0xB8 */ s32 closing;  /* 0: grows to 0x1000, else shrinks to 0 (the dialog's step) */
    /* 0xBC */ s32 opened;   /* it reached full size (message_dialog_update waits for the third) */
} MessageDialogOutline; /* size 0xC0 */

/* The object type created by message_dialog_frame_create (size 0x60). */
typedef struct MessageDialogFrame {
    /* 0x00 */ Object base;
    /* 0x50 */ MessageDialog *dialog;
    /* 0x54 */ s32 arrow_frame; /* the arrow's palette, 0..3 */
    /* 0x58 */ s32 arrow_time;  /* time of the last arrow_frame step */
    /* 0x5C */ u8 show_arrow;   /* the text window waits for a button */
} MessageDialogFrame;

/* The data block (object_new's third argument = its size) of the objects of message_create_dialog. */
typedef struct MessageDialogData {
    /* 0x00 */ MessageDialogOutline *outlines[3];
    /* 0x0C */ MessageWindow *windows[2]; /* two windows */
    /* 0x14 */ MessageDialogFrame *frame;
} MessageDialogData; /* size 0x18 */

/* The buttons that close a message window, indexed by its base.substep (message_window_update). */
extern s32 message_wait_buttons[];

extern MessageDialogLayout message_dialog_layouts[];

/* The control-code handlers, indexed by the byte after the code's lead byte. */
extern s32 (*message_control_handlers[])(MessageWindow *obj, MessageLine *line, MessageDrawState *state);

/* "メッセージがせっていされていません" (no message has been set); in message_draw_window's .rodata for now */
extern const u8 message_no_text[];

void message_copy_line_text(MessageWindow *obj, MessageLine *line, u8 *text);
void message_set_line_number(MessageWindow *obj, s32 line, s32 value);
void message_set_ext_line_text(MessageWindow *obj, u8 *text, s32 line);
void message_copy_text(MessageWindow *obj, u8 *text);
void message_format_number(u8 *buf, s32 value);
void message_set_speed(MessageWindow *obj, s32 arg1);
void message_set_line_text(MessageWindow *obj, u8 *text, s32 arg2, s32 line);
void message_draw_window(MessageWindow *obj);
void message_find_page_end(MessageWindow *obj);
void message_insert_names(MessageWindow *obj);
s32 message_handle_char(MessageWindow *obj, MessageLine *line, MessageDrawState *state, s16 *pos);
void message_window_update(MessageWindow *obj);
void message_cursor_update(MessageCursor *obj, struct MessageCursorData *data);
void message_rlen_free(MessageRlen *obj);
void message_unpack_rlen(MessageRlen *obj);
void *message_rlen_unpack_now(MessageRlen *obj, s32 *data);
void message_rlen_unpack_start(MessageRlen *obj, s32 *data, s32 arg2);
void *message_rlen_get_data(MessageRlen *obj);
void message_rlen_update(MessageRlen *obj);
void message_box_frame_update(MessageBoxFrame *obj);
void message_box_draw_arrow(MessageBoxFrame *obj);
void message_box_update(Object *obj, struct MessageBoxData *data);
void message_dialog_draw_arrow(MessageDialogFrame *obj);
void message_dialog_draw_frame(MessageDialogFrame *obj);
void message_dialog_frame_update(MessageDialogFrame *obj);
void message_dialog_draw_outline(MessageDialogOutline *obj);
void message_dialog_update(struct MessageDialog *obj, struct MessageDialogData *data);
void message_dialog_move(struct MessageDialog *obj, s32 x, s32 y);
struct MessageDialogFrame *message_dialog_frame_create(struct MessageDialog *arg0);
void message_dialog_outline_update(MessageDialogOutline *obj);

void message_copy_line_text(MessageWindow *obj, MessageLine *line, u8 *text) {
    s16 size;

    if (text != NULL) {
        line->length = strlen(text);
        if (line->length == 0) {
            obj->visible = 0;
            return;
        }
        obj->visible = 1;
        line->is_sjis = 1;
        if (line->text != NULL && line->capacity <= line->length) {
            heap_funcs.free(line->text);
            line->text = NULL;
            line->capacity = 0;
        }
        if (line->text == NULL) {
            /* room for the terminator, rounded up to 4 */
            if (line->length & 3) {
                size = (line->length & ~3) + 8;
            } else {
                size = line->length + 4;
            }
            line->capacity = size;
            line->text = heap_funcs.alloc(size, 2);
        }
        heap_funcs.bzero(line->text, line->capacity);
        memcpy(line->text, text, line->length);
    } else {
        /* "Ｎｕｌｌメッセージがわたされました" (a null message was passed) */
        message_copy_line_text(obj, line, "\x82\x6D\x82\x95\x82\x8C\x82\x8C\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x82\xAA\x82\xED\x82\xBD\x82\xB3\x82\xEA\x82\xDC\x82\xB5\x82\xBD");
    }
    if (obj->speed == 0) {
        message_set_speed(obj, 0);
    }
}

void message_copy_text(MessageWindow *obj, u8 *text) {
    message_copy_line_text(obj, obj->lines, text);
}

void message_set_text(MessageWindow *obj, u8 *text, s32 arg2) {
    message_set_line_text(obj, text, arg2, 0);
}

/* Writes the decimal digits of `value` to `buf` (no terminator); "0" for value <= 0. */
void message_format_number(u8 *buf, s32 value) {
    s32 len;
    s32 div;
    s32 saved;
    s32 digit;

    if (value <= 0) {
        buf[0] = '0';
        return;
    }
    saved = value;
    len = 0;
    div = 10;
    do {
        value -= value % div;
        len++;
        div *= 10;
    } while (value != 0);
    value = saved;
    while (value != 0) {
        digit = value % 10;
        value -= digit;
        value /= 10;
        buf[--len] = digit + '0';
    }
}

void message_set_line_number(MessageWindow *obj, s32 line, s32 value) {
    u8 buf[16];
    s32 i;

    if (line < 0 || line >= 6) {
        /* "Ｄｉｇｉｔ：メッセージのワークばんごうがみたいおう" (Digit: message work number not supported) */
        message_copy_text(obj, "\x82\x63\x82\x89\x82\x87\x82\x89\x82\x94\x81\x46\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x82\xCC\x83\x8F\x81\x5B\x83\x4E\x82\xCE\x82\xF1\x82\xB2\x82\xA4\x82\xAA\x82\xDD\x82\xBD\x82\xA2\x82\xA8\x82\xA4");
        return;
    }
    for (i = 0; i < 16; i++) {
        buf[i] = 0;
    }
    message_format_number(buf, value);
    for (i = 0; buf[i] != 0; i++) {
        buf[i] -= 0x2C;
    }
    message_copy_line_text(obj, &obj->lines[line], buf);
    obj->lines[line].is_sjis = 0;
}

void message_set_ext_line_text(MessageWindow *obj, u8 *text, s32 line) {
    if (line < 1 || line > 5) {
        /* "ＥｘｔＭｅｓｓ：メッセージのワークばんごうがみたいおう" (ExtMess: message work number not supported) */
        message_copy_text(obj, "\x82\x64\x82\x98\x82\x94\x82\x6C\x82\x85\x82\x93\x82\x93\x81\x46\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x82\xCC\x83\x8F\x81\x5B\x83\x4E\x82\xCE\x82\xF1\x82\xB2\x82\xA4\x82\xAA\x82\xDD\x82\xBD\x82\xA2\x82\xA8\x82\xA4");
        return;
    }
    message_copy_line_text(obj, &obj->lines[line], text);
}

void message_set_line_text(MessageWindow *obj, u8 *text, s32 arg2, s32 line) {
    Font funcs;
    u8 *str;

    if (arg2 >= 0) {
        font_init(&funcs);
        str = funcs.get_entry(text, arg2);
        if (str == NULL) {
            return;
        }
        message_copy_line_text(obj, &obj->lines[line], str);
    } else {
        message_copy_line_text(obj, &obj->lines[line], text);
    }
    obj->lines[line].is_sjis = 0;
}

const u8 message_no_text[] = "\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x82\xAA\x82\xB9\x82\xC1\x82\xC4\x82\xA2\x82\xB3\x82\xEA\x82\xC4\x82\xA2\x82\xDC\x82\xB9\x82\xF1";

void message_draw_window(MessageWindow *obj) {
    MessageDrawState state;
    SVECTOR out;
    SVECTOR v[4];
    MessageLine *line;
    u16 clut;
    u16 last_tpage;
    s32 rotated;
    GfxLayer *layer;
    FontGlyph *glyph;
    u16 tpage;
    s16 x;
    s16 y;
    u8 u;
    u8 v0;
    s32 n;
    s32 i;

    last_tpage = 0;
    tpage = 0;
    rotated = 0;
    if (obj->lines[0].text == NULL) {
        obj->visible = 0;
        return;
    }
    for (n = 5; n >= 0; n--) {
        obj->lines[n].pos = 0;
    }
    obj->done = 0;
    obj->pen_x = -obj->measured_width;
    state.line_count = 0;
    line = &obj->lines[0];
    obj->pen_y = 0;
    if (obj->scaled != 0) {
        if (obj->scale.vx == 0 && obj->scale.vy == 0) {
            return;
        }
        if (obj->scale.vx == 0x1000 && obj->scale.vy == 0x1000) {
            obj->scaled = 0;
        } else {
            rotated = 1;
            RotMatrixYXZ_gte(&obj->rotation, &obj->matrix);
            ScaleMatrix(&obj->matrix, &obj->scale);
        }
    }
    n = 0;
    state.layer = layer = gfx_module.funcs.get_layer(obj->layer_id);
    state.ot_entry = layer->get_ot_entry(layer, obj->ot_depth);
    state.packet.ptr = gfx_module.funcs.get_packet();
    line->pos = obj->page_start;
    while (n < obj->shown_end - obj->page_start) {
        if (obj->done != 0) {
            break;
        }
        state.code = message_module.decode_char(line->text + line->pos, line->is_sjis, obj->font);
        state.type = (state.code >> 8) & 0xFF;
        switch (message_handle_char(obj, line, &state, &line->pos)) {
        case 0:
        default:
            gfx_module.funcs.set_packet(state.packet.ptr);
            return;
        case 1:
            n++;
            break;
        case 4:
            n++;
            continue;
        case 2:
            break;
        case 3:
            continue;
        }
        if (state.glyph->tpage_x == 0xFF) {
            state.glyph = obj->font->glyphs;
        }
        glyph = state.glyph;
        x = obj->pen_x + (obj->x + glyph->x_offset);
        y = obj->pen_y + (obj->y + glyph->y_offset);
        clut = getClut(obj->vram_x + glyph->clut_x, obj->palette + (obj->vram_y + glyph->clut_y));
        u = glyph->u;
        v0 = glyph->v;
        if (obj->semi_trans == -1) {
            tpage = getTPage(0, 1, obj->vram_x + (glyph->tpage_x << 6), obj->vram_y);
        } else {
            tpage = getTPage(0, obj->semi_trans, obj->vram_x + (glyph->tpage_x << 6), obj->vram_y);
        }
        if (!rotated) {
            if (n == 0) {
                last_tpage = tpage;
            }
            if (tpage != last_tpage) {
                SetDrawTPage(state.packet.tpage, 0, 1, last_tpage);
                addPrim(state.ot_entry, state.packet.tpage);
                last_tpage = tpage;
                state.packet.tpage++;
            }
            setSprt(state.packet.sprt);
            if (obj->semi_trans != -1) {
                setSemiTrans(state.packet.sprt, 1);
            }
            state.packet.sprt->r0 = state.packet.sprt->g0 = state.packet.sprt->b0 = 0x80;
            state.packet.sprt->x0 = x;
            state.packet.sprt->y0 = y;
            state.packet.sprt->u0 = u;
            state.packet.sprt->v0 = v0;
            state.packet.sprt->w = state.glyph->w;
            state.packet.sprt->h = state.glyph->h;
            state.packet.sprt->clut = clut;
            addPrim(state.ot_entry, state.packet.sprt);
            state.packet.sprt++;
            SetDrawTPage(state.packet.tpage, 0, 1, tpage);
            addPrim(state.ot_entry, state.packet.tpage);
            state.packet.tpage++;
        } else {
            setPolyFT4(state.packet.ft4);
            state.packet.ft4->r0 = state.packet.ft4->g0 = state.packet.ft4->b0 = 0x80;
            v[0].vx = v[2].vx = x - obj->center_x;
            v[1].vx = v[3].vx = v[0].vx + state.glyph->w;
            v[0].vy = v[1].vy = y - obj->center_y;
            v[2].vy = v[3].vy = v[0].vy + state.glyph->h;
            v[0].vz = v[1].vz = v[2].vz = v[3].vz = 0;
            for (i = 0; i < 4; i++) {
                ApplyMatrixSV(&obj->matrix, &v[i], &out);
                /* x0/y0 ... x3/y3 are 8 bytes apart */
                (&state.packet.ft4->x0)[i * 4] = out.vx + obj->center_x;
                (&state.packet.ft4->y0)[i * 4] = out.vy + obj->center_y;
            }
            state.packet.ft4->u0 = state.packet.ft4->u2 = u;
            state.packet.ft4->u1 = state.packet.ft4->u3 = state.packet.ft4->u0 + state.glyph->w - 1;
            state.packet.ft4->v0 = state.packet.ft4->v1 = v0;
            state.packet.ft4->v2 = state.packet.ft4->v3 = state.packet.ft4->v0 + state.glyph->h - 1;
            if (obj->semi_trans != -1) {
                setSemiTrans(state.packet.ft4, 1);
            }
            state.packet.ft4->tpage = tpage;
            state.packet.ft4->clut = clut;
            addPrim(state.ot_entry, state.packet.ft4);
            state.packet.ft4++;
        }
        if (obj->fixed_size != 0) {
            obj->pen_x += obj->fixed_w;
        } else {
            obj->pen_x += state.glyph->advance + state.glyph->x_offset;
        }
    }
    if (!rotated) {
        SetDrawTPage(state.packet.tpage, 0, 1, tpage);
        addPrim(state.ot_entry, state.packet.tpage);
        state.packet.tpage++;
    }
    gfx_module.funcs.set_packet(state.packet.ptr);
}

void message_find_page_end(MessageWindow *obj) {
    s32 extra;
    s32 pos;
    s32 count;
    s32 loop;
    s32 c;
    u8 n;
    s16 code;

    if (obj->base.state != OBJECT_STATE_RUN) {
        return;
    }
    extra = 0;
    pos = obj->page_start;
    count = 0;
    loop = 1;
    while (loop) {
        code = message_module.decode_char(obj->lines[0].text + pos, obj->lines[0].is_sjis, obj->font);
        switch ((code >> 8) & 0xFF) {
        case 0:
        default:
            if (obj->lines[0].is_sjis != 0) {
                pos += 2;
            } else {
                pos += 1;
            }
            break;
        case 1:
            pos += 2;
            break;
        case 2:
            c = obj->lines[0].text[pos + 1];
            switch (c) {
            default:
                pos += message_module.code_lengths[c];
                break;
            case 1:
                if (++count < obj->page_lines) {
                    pos += message_module.code_lengths[1];
                } else {
                    loop = 0;
                }
                break;
            case 2:
                if (obj->lines[0].text[pos + 2] < 5) {
                    loop = 0;
                }
                pos += message_module.code_lengths[2];
                break;
            case 5:
                n = obj->lines[0].text[pos + 2];
                if (n < 6) {
                    extra += obj->lines[n].length;
                    pos += message_module.code_lengths[5];
                } else {
                    loop = 0;
                }
                break;
            case 3:
                loop = 0;
                break;
            }
            break;
        case 4:
            loop = 0;
            break;
        }
    }
    obj->shown_end = pos + extra;
}

void message_set_font(MessageWindow *obj, s32 arg1) {
    MessageFont *entry;

    if (arg1 < 1 || arg1 > 3) {
        arg1 = 1;
    }
    entry = &message_module.fonts[arg1];
    obj->font = entry;
    obj->semi_trans = entry->semi_trans;
}

void message_set_speed(MessageWindow *obj, s32 arg1) {
    if (arg1 <= 0) {
        obj->speed_count = 0;
        obj->speed = 0;
        obj->shown_end = obj->lines[0].length;
        return;
    }
    obj->shown_end = 0;
    obj->speed_count = 0;
    obj->speed = arg1;
    obj->page_start = 0;
}

void message_set_pos(MessageWindow *obj, s16 arg1, s16 arg2) {
    obj->x = arg1;
    obj->y = arg2;
}

void message_set_palette(MessageWindow *obj, u8 arg1) {
    obj->palette = arg1;
}

void message_set_semi_trans(MessageWindow *obj, u8 arg1) {
    obj->semi_trans = arg1;
}

void message_set_fixed_size(MessageWindow *obj, s16 arg1, s16 arg2) {
    if (arg1 != 0 || arg2 != 0) {
        obj->fixed_size = 1;
        obj->fixed_w = arg1;
        obj->fixed_h = arg2;
        return;
    }
    obj->fixed_size = 0;
    obj->fixed_w = 0;
    obj->fixed_h = 0;
}

void message_set_visible(MessageWindow *obj, u8 arg1) {
    if (obj->lines[0].length == 0) {
        obj->visible = 0;
        return;
    }
    obj->visible = arg1;
}

void message_measure(MessageWindow *obj, u8 arg1) {
    Font funcs;

    if (arg1 != 0) {
        font_init(&funcs);
        obj->measured_width = funcs.get_text_width(obj->lines, obj->font, obj->fixed_w);
        return;
    }
    obj->measured_width = 0;
}

void message_insert_names(MessageWindow *obj) {
    MessageLine *line = &obj->lines[0];
    s32 i;
    u8 n;
    s16 code;

    if (line->text == NULL) {
        return;
    }
    for (i = 0; i < obj->lines[0].length;) {
        code = message_module.decode_char(line->text + i, line->is_sjis, obj->font);
        switch ((code >> 8) & 0xFF) {
        case 0:
        case 3:
        default:
            if (line->is_sjis != 0) {
                i += 2;
            } else {
                i += 1;
            }
            break;
        case 1:
            i += 2;
            break;
        case 2:
            switch (line->text[i + 1]) {
            case 4:
                break;
            case 8:
                n = line->text[i + 2];
                message_set_ext_line_text(obj, gamestate_data.name, n);
                obj->lines[n].is_sjis = 0;
                i += message_module.code_lengths[8];
                break;
            default:
                i += message_module.code_lengths[line->text[i + 1]];
                break;
            }
            break;
        case 4:
            return;
        }
    }
}

void message_set_char_sound(MessageWindow *obj, s32 arg1) {
    obj->char_sound = arg1;
}

void message_set_scale(MessageWindow *obj, s32 arg1, s32 arg2) {
    obj->scale.vz = 0x1000;
    obj->scale.vx = arg1;
    obj->scale.vy = arg2;
    obj->scaled = 1;
}

void message_set_center(MessageWindow *obj, s32 arg1, s32 arg2) {
    obj->center_x = arg1;
    obj->center_y = arg2;
}

void message_set_ot_depth(MessageWindow *obj, s32 arg1) {
    obj->ot_depth = arg1;
}

void message_set_page_lines(MessageWindow *obj, u8 arg1) {
    obj->page_lines = arg1;
}

void func_8001A458(MessageWindow *obj, u8 arg1) {
    obj->unk_C4 = arg1;
}

s32 message_is_done(MessageWindow *obj) {
    return obj->done;
}

s32 message_is_visible(MessageWindow *obj) {
    return obj->visible;
}

s32 message_is_waiting(MessageWindow *obj) {
    return obj->base.step == 1;
}

s32 message_handle_char(MessageWindow *obj, MessageLine *line, MessageDrawState *state, s16 *pos) {
    MessageFont *font;
    s32 code;
    s32 ret;
    s32 n;

    switch (state->type) {
    case 2:
        if (line->is_sjis != 0) {
            if ((u8)state->code == 1) {
                ret = message_control_handlers[1](obj, line, state);
                *pos += 1;
            } else {
                return message_control_handlers[0](obj, line, state);
            }
        } else {
            code = line->text[*pos + 1];
            if (message_control_handlers[code] == NULL) {
                return message_control_handlers[0](obj, line, state);
            }
            ret = message_control_handlers[code](obj, line, state);
            if (ret & 0x8000) {
                *pos += message_module.code_lengths[code];
            }
        }
        return ret & ~0x8000;
    case 0:
        state->glyph = &obj->font->glyphs[state->code - 4];
        if (line->is_sjis != 0) {
            *pos += 2;
        } else {
            *pos += 1;
        }
        break;
    case 1:
        font = obj->font;
        n = (u8)state->code;
        if (n <= font->ext_count && n > 0) {
            state->glyph = &font->glyphs_ext[n - 1];
        } else {
            state->glyph = obj->font->glyphs;
        }
        *pos += 2;
        break;
    case 4:
        obj->done = 1;
        *pos += 1;
        return 3;
    default:
        state->glyph = obj->font->glyphs;
        if (line->is_sjis != 0) {
            *pos += 2;
        } else {
            *pos += 1;
        }
        break;
    }
    return 1;
}

s32 message_code_end(MessageWindow *obj, MessageLine *line) {
    switch (line->text[line->pos + 1]) {
    case 0:
    default:
        message_find_page_end(obj);
        break;
    case 7:
        obj->page_start = line->pos + 2;
        break;
    }
    return 0;
}

s32 message_code_newline(MessageWindow *obj, MessageLine *line, MessageDrawState *state) {
    if (++state->line_count == 1) {
        state->second_line = line->pos + 2;
    } else if (state->line_count >= obj->page_lines) {
        obj->page_start = state->second_line;
        if (obj->page_lines >= 2) {
            obj->page_lines--;
            message_find_page_end(obj);
            obj->page_lines++;
        }
        return 0x8000;
    }
    obj->pen_x = 0;
    if (obj->fixed_size != 0) {
        obj->pen_y += obj->fixed_h;
    } else {
        obj->pen_y += obj->font->line_height;
    }
    return 0x8004;
}

s32 message_code_wait_button(MessageWindow *obj, MessageLine *line) {
    if (line->text[line->pos + 2] < 5) {
        obj->base.state = OBJECT_STATE_DONE;
        obj->base.step = 1;
        if (line->text[line->pos + 2] < 1 || line->text[line->pos + 2] > 4) {
            obj->base.substep = 0;
        } else {
            obj->base.substep = line->text[line->pos + 2];
        }
        obj->base.timer = line->pos + 2;
        return 0;
    }
    return 0x8003;
}

s32 message_code_instant(MessageWindow *obj, MessageLine *line) {
    s16 code;
    u8 c;

    if (obj->speed != 0) {
        obj->shown_end = line->pos + 2;
    } else {
        obj->shown_end = line->length;
    }
    while (line->pos < line->length) {
        code = message_module.decode_char(line->text + line->pos, line->is_sjis, obj->font);
        switch ((code >> 8) & 0xFF) {
        case 0:
        case 1:
        default:
            obj->page_start = line->pos;
            line->pos = line->length + 1;
            break;
        case 2:
            c = line->text[line->pos + 1];
            if (c == 5 || c == 8) {
                obj->page_start = line->pos;
                line->pos = line->length + 1;
            } else {
                line->pos += message_module.code_lengths[c];
            }
            break;
        case 4:
            obj->page_start = line->pos - 1;
            return 3;
        }
    }
    return 0;
}

s32 message_code_nop(MessageWindow *obj, MessageLine *line, MessageDrawState *state) {
    return 0x8003;
}

s32 message_code_insert_line(MessageWindow *obj, MessageLine *line, MessageDrawState *state) {
    s32 n = line->text[line->pos + 2];
    s16 code;
    s32 ret;

    if (obj->lines[n].text == NULL) {
        /* "メッセージがせっていされていません" (no message has been set) */
        message_set_ext_line_text(obj, (u8 *)message_no_text, n);
        return 0x8003;
    }
    if (obj->lines[n].pos >= obj->lines[n].length) {
        return 0x8003;
    }
    code = message_module.decode_char(obj->lines[n].text + obj->lines[n].pos, obj->lines[n].is_sjis, obj->font);
    state->code = code;
    state->type = (code >> 8) & 0xFF;
    if (state->type == 2) {
        return 0x8000;
    }
    if (obj->speed != 0) {
        return message_handle_char(obj, &obj->lines[n], state, &obj->lines[n].pos);
    }
    ret = message_handle_char(obj, &obj->lines[n], state, &obj->lines[n].pos);
    if (ret == 1) {
        return 2;
    }
    return ret;
}

s32 message_code_pause(MessageWindow *obj, MessageLine *line) {
    if (line->text[line->pos + 2] < 0xFF) {
        obj->base.state = OBJECT_STATE_DONE;
        obj->base.step = 0;
        obj->base.substep = line->text[line->pos + 2];
        line->text[line->pos + 2] = 0xFF;
        obj->speed_count = 0;
        return 0x8000;
    }
    return 0x8003;
}

s32 message_code_player_name(MessageWindow *obj, MessageLine *line) {
    if (line->text[line->pos + 2] < 6) {
        message_set_line_text(obj, gamestate_data.name, -1, line->text[line->pos + 2]);
        line->text[line->pos + 2] = 6;
    }
    return 0x8003;
}

void message_window_update(MessageWindow *obj) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->visible == 0) {
            break;
        }
#ifdef PC_PORT
        if (port_mod_skip_dialogues != 0 && obj->speed > 0 && obj->done == 0) {
            message_find_page_end(obj); /* skip_dialogues: the page at once, as a confirm press shows it */
        }
#endif
        if (obj->speed > 0 && obj->done == 0 && ++obj->speed_count > obj->speed) {
            obj->speed_count = 0;
            obj->shown_end++;
            if (obj->char_sound != 0) {
                sound_module.play(obj->char_sound);
            }
        }
        message_draw_window(obj);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        case 0:
        default:
            message_draw_window(obj);
            if (++obj->speed_count > obj->base.substep) {
                obj->speed_count = 0;
                obj->base.set_state(obj, OBJECT_STATE_RUN);
            }
            break;
        case 1:
#ifdef PC_PORT
            /* skip_dialogues: a wait for the confirm button (every wait in the disc's text) goes on by itself */
            if (port_mod_skip_dialogues != 0 && message_wait_buttons[obj->base.substep] == 13) {
                obj->lines[0].text[obj->base.timer] = 5;
                obj->speed_count = obj->speed;
                obj->base.set_state(obj, OBJECT_STATE_RUN);
            } else
#endif
            if (PAD_PRESSED(message_wait_buttons[obj->base.substep])) {
                obj->lines[0].text[obj->base.timer] = 5;
                obj->speed_count = obj->speed;
                obj->base.set_state(obj, OBJECT_STATE_RUN);
            }
            message_draw_window(obj);
            break;
        }
        break;
    case OBJECT_STATE_END:
        for (i = 0; i < 6; i++) {
            if (obj->lines[i].text != NULL) {
                heap_funcs.free(obj->lines[i].text);
            }
        }
        break;
    }
}

MessageWindow *message_create_window(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    MessageWindow *obj = object_new(message_window_update, sizeof(MessageWindow), 0);

    obj->copy_text = message_copy_text;
    obj->set_text = message_set_text;
    obj->set_line_number = message_set_line_number;
    obj->set_ext_line_text = message_set_ext_line_text;
    obj->set_line_text = message_set_line_text;
    obj->draw_window = message_draw_window;
    obj->find_page_end = message_find_page_end;
    obj->set_font = message_set_font;
    obj->set_speed = message_set_speed;
    obj->set_pos = message_set_pos;
    obj->set_palette = message_set_palette;
    obj->set_semi_trans = message_set_semi_trans;
    obj->set_fixed_size = message_set_fixed_size;
    obj->set_visible = message_set_visible;
    obj->measure = message_measure;
    obj->insert_names = message_insert_names;
    obj->set_char_sound = message_set_char_sound;
    obj->set_scale = message_set_scale;
    obj->set_center = message_set_center;
    obj->set_ot_depth = message_set_ot_depth;
    obj->set_page_lines = message_set_page_lines;
    obj->unk_164 = func_8001A458;
    obj->is_done = message_is_done;
    obj->is_visible = message_is_visible;
    obj->is_waiting = message_is_waiting;
    if (arg1 < 1 || arg1 > 3) {
        arg1 = 1;
    }
    obj->layer_id = arg0;
    obj->font = &message_module.fonts[arg1];
    obj->x = arg2;
    obj->vram_x = 0x140;
    obj->y = arg3;
    obj->vram_y = 0;
    obj->page_lines = 1;
    obj->semi_trans = obj->font->semi_trans;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    return obj;
}

void message_cursor_show(MessageCursor *obj, s32 arg1) {
    obj->shown = arg1;
    if (arg1 == 0) {
        obj->base.step = 0;
        obj->last_time = 0;
    }
    obj->dirty = 1;
}

void message_cursor_set_pos(MessageCursor *obj, s32 arg1, s32 arg2) {
    obj->x = arg1;
    obj->y = arg2;
    obj->dirty = 1;
}

void message_cursor_set_palette(MessageCursor *obj, s32 arg1) {
    obj->palette = arg1;
    obj->dirty = 1;
}

void message_cursor_set_delay(MessageCursor *obj, s32 arg1) {
    obj->delay = arg1;
}

void message_cursor_set_interval(MessageCursor *obj, s32 arg1) {
    obj->interval = arg1;
}

void message_cursor_stop(MessageCursor *obj, s32 arg1) {
    obj->stopped = arg1;
}

void message_cursor_update(MessageCursor *obj, MessageCursorData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (data->window == NULL) {
            data->window = message_create_window(obj->layer_id, 1, obj->x, obj->y);
        }
        data->window->copy_text(data->window, message_cursor_frames[obj->frame]);
        data->window->set_visible(data->window, obj->shown);
        data->window->set_ot_depth(data->window, obj->ot_depth);
        obj->last_time = gfx_module.funcs.get_time();
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->dirty != 0) {
            data->window->set_visible(data->window, obj->shown);
            data->window->set_pos(data->window, obj->x, obj->y);
            data->window->set_palette(data->window, obj->palette);
            obj->dirty = 0;
        }
        if (obj->shown == 0) {
            break;
        }
        if (obj->stopped != 0) {
            if (obj->frame != 0) {
                obj->frame = 0;
                data->window->copy_text(data->window, message_cursor_frames[0]);
            }
        } else if (obj->base.step == 0) {
            if ((gfx_module.funcs.get_time() - obj->last_time) / obj->delay != 0) {
                obj->last_time = gfx_module.funcs.get_time();
                obj->frame = 1;
                data->window->copy_text(data->window, message_cursor_frames[1]);
                obj->base.step = 1;
            }
        } else if ((gfx_module.funcs.get_time() - obj->last_time) / obj->interval != 0) {
            obj->last_time = gfx_module.funcs.get_time();
            if (++obj->frame >= 5) {
                obj->frame = 0;
                obj->base.step = 0;
            }
            data->window->copy_text(data->window, message_cursor_frames[obj->frame]);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

MessageCursor *message_create_cursor(s16 arg0, s32 arg1, s16 arg2, s16 arg3) {
    MessageCursor *obj = object_new(message_cursor_update, sizeof(MessageCursor), sizeof(MessageCursorData));

    obj->layer_id = arg0;
    obj->ot_depth = arg1;
    obj->x = arg2;
    obj->y = arg3;
    obj->shown = 1;
    obj->delay = 0x20;
    obj->interval = 6;
    obj->show = message_cursor_show;
    obj->set_pos = message_cursor_set_pos;
    obj->set_delay = message_cursor_set_delay;
    obj->set_interval = message_cursor_set_interval;
    obj->set_palette = message_cursor_set_palette;
    obj->stop = message_cursor_stop;
    return obj;
}

void message_rlen_free(MessageRlen *obj) {
    if (obj->buffer != NULL) {
        heap_funcs.free(obj->buffer);
    }
    obj->buffer = NULL;
    obj->buffer_size = 0;
}

void message_rlen_set_data(MessageRlen *obj, s32 *data) {
    obj->data = data;
    if (data[0] == MESSAGE_RLEN_MAGIC) {
        obj->is_packed = 1;
        obj->size = data[1];
    } else {
        obj->is_packed = 0;
        obj->size = 0;
    }
    data += 2;
    obj->packed = data;
    obj->src = data;
}

void message_rlen_alloc_buffer(MessageRlen *obj) {
    if (obj->size > obj->buffer_size) {
        if (obj->buffer != NULL) {
            heap_funcs.free(obj->buffer);
        }
        obj->buffer = heap_funcs.alloc(obj->size, 2);
        obj->buffer_size = obj->size;
    }
    obj->dst = obj->buffer;
}

void message_unpack_rlen(MessageRlen *obj) {
    s32 total = 0;
    s32 stopped = 0;
    u8 *src = obj->src;
    u8 *dst = obj->dst;
    s32 i;
    s32 n;

    while (*src != 0) {
        if (*src & 0x80) {
            n = *src & 0x7F;
            src++;
            for (i = 0; i < n; i++) {
                *dst++ = *src;
            }
            src++;
            total += n;
        } else {
            n = *src++;
            for (i = 0; i < n; i++) {
                *dst++ = *src++;
            }
            total += n;
        }
        if (total >= obj->chunk_size) {
            stopped = 1;
            obj->src = src;
            obj->dst = dst;
            break;
        }
    }
    if (!stopped) {
        obj->base.set_step(obj, 0);
    }
}

void *message_rlen_unpack_now(MessageRlen *obj, s32 *data) {
    obj->chunk_size = 0x10000000;
    message_rlen_set_data(obj, data);
    if (obj->is_packed != 0) {
        message_rlen_alloc_buffer(obj);
        message_unpack_rlen(obj);
        return obj->buffer;
    }
    return data;
}

void message_rlen_unpack_start(MessageRlen *obj, s32 *data, s32 arg2) {
    obj->chunk_size = arg2;
    message_rlen_set_data(obj, data);
    if (obj->is_packed != 0) {
        message_rlen_alloc_buffer(obj);
        obj->base.set_step(obj, 1);
    }
}

void *message_rlen_get_data(MessageRlen *obj) {
    if (obj->base.step != 0) {
        return NULL;
    }
    if (obj->is_packed != 0) {
        return obj->buffer;
    }
    return obj->data;
}

void message_rlen_update(MessageRlen *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            break;
        case 1:
            message_unpack_rlen(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->buffer != NULL) {
            heap_funcs.free(obj->buffer);
        }
        break;
    }
}

MessageRlen *message_rlen_create(void) {
    MessageRlen *obj = object_new(message_rlen_update, sizeof(MessageRlen), 0);

    obj->unpack_now = message_rlen_unpack_now;
    obj->free = message_rlen_free;
    obj->unpack_start = message_rlen_unpack_start;
    obj->get_data = message_rlen_get_data;
    return obj;
}

void message_box_draw_frame(MessageBoxFrame *obj) {
    Sprite spr;
    s32 i;

    for (i = 0; i < 2; i++) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, 0);
        spr.set_vram_pos(0x140, 0);
        if (obj->base.step == 0) {
            spr.set_scale(obj->scale, 0x1000, 0x1000);
            spr.set_pivot(0x140 - i * 0x140, 0xC4);
        } else if (obj->base.step == 2) {
            spr.set_palette(obj->close_frame);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x2860000), i + 8, 0, 0xA6);
    }
}

void message_box_draw_arrow(MessageBoxFrame *obj) {
    Sprite spr;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, 0);
    spr.set_vram_pos(0x140, 0);
    if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
        obj->arrow_time = gfx_module.funcs.get_time();
        if (++obj->arrow_frame >= 5) {
            obj->arrow_frame = 0;
        }
    }
    spr.set_palette(obj->arrow_frame);
    spr.draw(cdload_module.get_subfile_by_id(0x2860000), 0xA, 0x124, 0xCD);
}

void message_box_frame_update(MessageBoxFrame *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->scale_speed = 0x199;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            obj->scale += obj->scale_speed;
            if (obj->scale > 0x1000) {
                obj->scale = 0x1000;
                obj->base.next_step(obj);
            }
            break;
        case 1:
            if (obj->show_arrow != 0) {
                message_box_draw_arrow(obj);
            }
            break;
        case 2:
            if (gfx_module.funcs.get_time() - obj->close_time >= 3) {
                obj->close_time = gfx_module.funcs.get_time();
                if (++obj->close_frame >= 5) {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    return;
                }
            }
            break;
        }
        message_box_draw_frame(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

MessageBoxFrame *message_box_frame_create(s32 arg0) {
    MessageBoxFrame *obj = object_new(message_box_frame_update, sizeof(MessageBoxFrame), 0);

    obj->layer_id = arg0;
    sound_module.play(0x40019);
    return obj;
}

void message_box_update(Object *obj, MessageBoxData *data) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->step) {
        case 0:
        default:
            if (data->window->is_visible(data->window) == 0 && data->frame->base.step == 1) {
                data->window->set_visible(data->window, 1);
                obj->step++;
            }
            break;
        case 1:
            if (data->window->is_done(data->window) != 0) {
                data->frame->base.step = 2;
                data->window->set_visible(data->window, 0);
                obj->step++;
                sound_module.play(0x4001A);
            } else {
                if (PAD_PRESSED(13)) {
                    data->window->find_page_end(data->window);
                }
                if (data->window->is_waiting(data->window) != 0) {
                    data->frame->show_arrow = 1;
                } else {
                    data->frame->show_arrow = 0;
                }
            }
            break;
        case 2:
            if (data->frame == NULL) {
                obj->set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *message_box_create(s32 arg0, u8 *text, s32 arg2) {
    Object *obj = object_new(message_box_update, sizeof(Object), sizeof(MessageBoxData));
    MessageBoxData *data = (MessageBoxData *)obj->children;

    data->window = message_create_window(arg0, 1, 0x12, 0xB0);
    data->window->set_page_lines(data->window, 3);
    data->window->set_text(data->window, text, arg2);
    data->window->set_visible(data->window, 0);
    data->window->set_speed(data->window, 6);
    data->frame = message_box_frame_create(arg0);
    return obj;
}

/* Draws the dialog's "more" arrow, animated every 6 ticks (4 frames). */
void message_dialog_draw_arrow(MessageDialogFrame *obj) {
    Sprite spr;
    SVECTOR unused; /* unused, but the original's frame has room for it */
    s32 dx;
    MessageOffset *arrow;
    MessageOffset *ofs = &message_dialog_layouts[obj->dialog->type].arrow_pos;

    if (obj->dialog->type == 2 || obj->dialog->type == 3) {
        dx = obj->dialog->width - 14;
    } else {
        dx = ofs->x;
    }
    if (obj->show_arrow != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->dialog->layer_id, 0);
        spr.set_vram_pos(0x140, 0);
        /* FAKE: a copy of ofs read only for the y below (the US decomp's shape, drawTalkBoxArrow). Its extra
         * reference puts ofs in s2 and dx in s3; with ofs->y read directly global.c allocates dx first (99.66%).
         * No natural form found: a ternary, a switch, a type local, a layout pointer, s16 dx, dx = 0, an early
         * return, declaration orders (permuter 2 x 30 min: only dead references to ofs). */
        arrow = ofs;
        if (gfx_module.funcs.get_time() - obj->arrow_time >= 6) {
            obj->arrow_time = gfx_module.funcs.get_time();
            if (++obj->arrow_frame >= 4) {
                obj->arrow_frame = 0;
            }
        }
        spr.set_palette(obj->arrow_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x2860000), 7, obj->dialog->x + dx, obj->dialog->y + arrow->y);
    }
}

void message_dialog_draw_frame(MessageDialogFrame *obj) {
    Sprite spr;
    SVECTOR v[4];
    GfxLayer *layer;
    u32 *ot;
    POLY_FT4 *poly;
    MessageOffset *pos;
    s32 type;
    s32 dx;
    s32 x;
    s32 i;

    type = obj->dialog->type;
    dx = 0;
    if (type == 0 || type == 1) {
        dx = obj->dialog->width;
    }
    pos = &message_dialog_layouts[type].tail_pos;
    sprite_init(&spr);
    spr.set_layer_id(obj->dialog->layer_id, 0);
    spr.set_vram_pos(0x140, 0);
    for (i = 0; i < 4; pos++, i++) {
        switch (i) {
        case 0:
            spr.draw(cdload_module.get_subfile_by_id(0x2860000), message_dialog_layouts[type].tail_sprite,
                       obj->dialog->x + pos->x, obj->dialog->y + pos->y);
            break;
        case 1:
            spr.draw(cdload_module.get_subfile_by_id(0x2860000), 0,
                       obj->dialog->x + pos->x - dx, obj->dialog->y + pos->y);
            break;
        case 3:
            if (type == 0 || type == 1) {
                spr.draw(cdload_module.get_subfile_by_id(0x2860000), 2,
                           obj->dialog->x + pos->x, obj->dialog->y + pos->y);
            } else {
                x = obj->dialog->width - 14;
                spr.draw(cdload_module.get_subfile_by_id(0x2860000), 2,
                           obj->dialog->x + x, obj->dialog->y + pos->y);
            }
            break;
        }
    }
    layer = gfx_module.funcs.get_layer(obj->dialog->layer_id);
    ot = layer->get_ot_entry(layer, 0);
    pos = &message_dialog_layouts[type].fill_pos;
    v[0].vx = v[2].vx = obj->dialog->x + pos->x - dx;
    v[1].vx = v[3].vx = v[0].vx + obj->dialog->width;
    v[0].vy = v[1].vy = obj->dialog->y + pos->y;
    v[2].vy = v[3].vy = v[0].vy + obj->dialog->height;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 0;
    poly = gfx_module.funcs.get_packet();
    setPolyFT4(poly);
    setRGB0(poly, 0x80, 0x80, 0x80);
    poly->tpage = 0x45;
    poly->clut = 0x2C57;
    setSemiTrans(poly, 1);
    setXY4(poly, v[0].vx, v[0].vy, v[1].vx, v[1].vy, v[2].vx, v[2].vy, v[3].vx, v[3].vy);
    setUV4(poly, 0xBC, 0, 0xC7, 0, 0xBC, 0x3E, 0xC7, 0x3E);
    addPrim(ot, poly);
    gfx_module.funcs.set_packet(poly + 1);
}

void message_dialog_frame_update(MessageDialogFrame *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.set_state(obj, OBJECT_STATE_DONE);
        break;
    case OBJECT_STATE_RUN:
        message_dialog_draw_arrow(obj);
        message_dialog_draw_frame(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

MessageDialogFrame *message_dialog_frame_create(MessageDialog *arg0) {
    MessageDialogFrame *obj = object_new(message_dialog_frame_update, sizeof(MessageDialogFrame), 0);

    obj->dialog = arg0;
    return obj;
}

void message_dialog_draw_outline(MessageDialogOutline *obj) {
    SVECTOR out[4];
    SVECTOR v[4];
    GfxLayer *layer;
    u32 *ot;
    LINE_F2 *line;
    s32 i;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, 0);
    if (obj->closing == 0) {
        obj->scale += obj->scale_speed;
        if (obj->scale > 0x1000) {
            obj->scale = 0x1000;
            obj->opened = 1;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
    } else {
        obj->scale -= obj->scale_speed;
        if (obj->scale < 0) {
            obj->scale = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
    }
    obj->scale_vec.vz = 0;
    obj->scale_vec.vx = obj->scale_vec.vy = obj->scale;
    RotMatrixYXZ_gte(&obj->rotation, &obj->matrix);
    ScaleMatrix(&obj->matrix, &obj->scale_vec);
    v[0].vx = v[2].vx = obj->x - obj->pivot_x;
    v[1].vx = v[3].vx = v[0].vx + obj->w;
    v[0].vy = v[1].vy = obj->y - obj->pivot_y;
    v[2].vy = v[3].vy = v[0].vy + obj->h;
    v[0].vz = v[1].vz = v[2].vz = v[3].vz = 0;
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(&obj->matrix, &v[i], &out[i]);
        out[i].vx += obj->pivot_x;
        out[i].vy += obj->pivot_y;
    }
    line = gfx_module.funcs.get_packet();
    for (i = 0; i < 4; i++) {
        s32 next[4] = { 1, 3, 0, 2 };

        setLineF2(line);
        setRGB0(line, 0, 0, 0xFF);
        line->x0 = out[i].vx;
        line->y0 = out[i].vy;
        line->x1 = out[next[i]].vx;
        line->y1 = out[next[i]].vy;
        addPrim(ot, line);
        line++;
    }
    gfx_module.funcs.set_packet(line);
}

void message_dialog_outline_update(MessageDialogOutline *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->unk_68 = 0;
        if (obj->closing == 0) {
            obj->scale = 0;
        } else {
            obj->scale = 0x1000;
        }
        obj->unk_90 = 0;
        obj->unk_88 = obj->pivot_x;
        obj->unk_8C = obj->pivot_y;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        message_dialog_draw_outline(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

MessageDialogOutline *message_dialog_outline_create(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s32 arg5) {
    MessageDialogOutline *obj;
    s16 offset = arg3;

    obj = object_new(message_dialog_outline_update, sizeof(MessageDialogOutline), 0);

    obj->layer_id = arg0;
    obj->pivot_x = arg1;
    obj->pivot_y = arg2;
    obj->w = arg3 + 0x20;
    obj->h = arg4;
    if (arg5 == 2 || arg5 == 3) {
        offset = 0;
    }
    obj->offset_x = message_dialog_layouts[arg5].left_pos.x - offset;
    obj->offset_y = message_dialog_layouts[arg5].left_pos.y;
    obj->x = arg1 + obj->offset_x;
    obj->y = arg2 + obj->offset_y;
    return obj;
}

void message_dialog_move(MessageDialog *obj, s32 x, s32 y) {
    s32 type;
    MessageDialogData *data = (MessageDialogData *)obj->base.children;
    MessageDialogOutline *child;
    s32 i;
    s32 wx;
    s32 wy;

    obj->x = x;
    obj->y = y;
    for (i = 0; i < 3; i++) {
        child = data->outlines[i];
        if (child != NULL && child->base.state == OBJECT_STATE_RUN) {
            child->x = child->offset_x + x;
            child->y = child->offset_y + y;
        }
    }
    type = obj->type;
    if (data->windows[0] != NULL) {
        wx = obj->x + message_dialog_layouts[type].name_pos.x;
        wy = obj->y + message_dialog_layouts[type].name_pos.y;
        if (type == 0 || type == 1) {
            wx -= obj->width;
        }
        data->windows[0]->set_pos(data->windows[0], wx, wy);
    }
    if (data->windows[1] != NULL) {
        wx = obj->x + message_dialog_layouts[type].text_pos.x;
        wy = obj->y + message_dialog_layouts[type].text_pos.y;
        if (type == 0 || type == 1) {
            wx -= obj->width;
        }
        data->windows[1]->set_pos(data->windows[1], wx, wy);
    }
}

void message_dialog_update(MessageDialog *obj, MessageDialogData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.set_state(obj, OBJECT_STATE_DONE);
        break;
    case OBJECT_STATE_RUN:
        if (data->windows[1]->is_done(data->windows[1]) != 0) {
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            obj->base.set_step(obj, 1);
            data->frame->base.set_state(data->frame, OBJECT_STATE_DONE);
            data->windows[0]->set_visible(data->windows[0], 0);
            data->windows[1]->set_visible(data->windows[1], 0);
        } else if (PAD_PRESSED(13)) {
            data->windows[1]->find_page_end(data->windows[1]);
        }
        if (data->windows[1]->is_waiting(data->windows[1]) != 0) {
            data->frame->show_arrow = 1;
        } else {
            data->frame->show_arrow = 0;
        }
        break;
    case OBJECT_STATE_DONE: {
        s32 delays[3] = { 0, 2, 4 };

        switch (obj->base.substep) {
        case 0:
        default:
            if (obj->base.step == 0) {
                sound_module.play(0x40019);
            } else {
                sound_module.play(0x4001A);
            }
        case 1:
        case 2:
            if (obj->delay_count++ >= delays[obj->base.substep]) {
                data->outlines[obj->base.substep] = message_dialog_outline_create(obj->layer_id, obj->x, obj->y, obj->width, obj->height, obj->type);
                data->outlines[obj->base.substep]->closing = obj->base.step;
                if (obj->base.step == 0) {
                    data->outlines[obj->base.substep]->scale_speed = 0x199;
                } else {
                    data->outlines[obj->base.substep]->scale_speed = 0x333;
                }
                obj->base.next_substep(obj);
                obj->delay_count = 0;
            }
            break;
        case 3:
            if (obj->base.step == 0) {
                if (data->outlines[2]->opened != 0) {
                    obj->base.set_state(obj, OBJECT_STATE_RUN);
                    data->frame->base.set_state(data->frame, OBJECT_STATE_RUN);
                    data->windows[0]->set_visible(data->windows[0], 1);
                    data->windows[1]->set_visible(data->windows[1], 1);
                }
            } else if (obj->base.step == 1) {
                if (data->outlines[2] == NULL) {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
            break;
        }
        break;
    }
    case OBJECT_STATE_END:
        break;
    }
}

MessageDialog *message_create_dialog(s32 arg0, s16 x, s16 y, void *arg3, s32 arg4, s32 type) {
    Font funcs;
    char buf[32];
    MessageDialog *obj;
    MessageDialogData *data;
    u8 *text;
    s32 end;
    s32 width;
    s32 i;

    obj = object_new(message_dialog_update, sizeof(MessageDialog), sizeof(MessageDialogData));
    data = (MessageDialogData *)obj->base.children;
    obj->layer_id = arg0;
    obj->x = x;
    obj->y = y;
    obj->move = message_dialog_move;
    obj->type = type;
    font_init(&funcs);
    i = 0;
    data->windows[0] = message_create_window(obj->layer_id, 2, obj->x + message_dialog_layouts[type].name_pos.x, obj->y + message_dialog_layouts[type].name_pos.y);
    data->windows[0]->set_page_lines(data->windows[0], 3);
    data->windows[1] = message_create_window(obj->layer_id, 1, obj->x + message_dialog_layouts[type].text_pos.x, obj->y + message_dialog_layouts[type].text_pos.y);
    data->windows[1]->set_page_lines(data->windows[1], 3);
    obj->messages = arg3;
    text = funcs.get_entry(arg3, arg4);
    /* A leading control code 7 (02 07 name 02 07?) puts its text in the first window. */
    if (text[i] == 2 && text[i + 1] == 7) {
        end = i + 2;
        while (text[end] != 2 && text[end + 1] != 7) {
            end++;
        }
        heap_funcs.bzero(buf, sizeof(buf));
        if (text[i + 2] == 2 && text[i + 3] == 9) {
            strcpy(buf, gamestate_data.name);
        } else {
            strncpy(buf, &text[i + 2], end - (i + 2));
        }
        data->windows[0]->set_text(data->windows[0], buf, -1);
        data->windows[1]->set_text(data->windows[1], &text[end + 2], -1);
    } else {
        data->windows[1]->set_text(data->windows[1], text, -1);
    }
    data->windows[1]->set_speed(data->windows[1], 6);
    data->windows[1]->insert_names(data->windows[1]);
    width = funcs.get_text_width(data->windows[1]->lines, data->windows[1]->font, 0);
    if (width < 0x5F) {
        width = 0x5F;
    } else if (width >= 0x8C) {
        width = 0x8B;
    }
    obj->width = width;
    obj->height = 0x3E;
    if (type == 0 || type == 1) {
        data->windows[0]->set_pos(data->windows[0], data->windows[0]->x - obj->width, data->windows[0]->y);
        data->windows[1]->set_pos(data->windows[1], data->windows[1]->x - obj->width, data->windows[1]->y);
    }
    for (i = 0; i < 2; i++) {
        data->windows[i]->set_palette(data->windows[i], 2);
        data->windows[i]->set_visible(data->windows[i], 0);
    }
    data->frame = message_dialog_frame_create(obj);
    return obj;
}

void message_load_font(void) {
    Tim img;

    tim_init(&img);
    img.set_image_pos(0x140, 0);
    img.load_all(cdload_module.get_subfile_by_id(0x2870000));
    cdload_module.files.free_file(0x287);
    cdload_module.queue_file(0x286);
}

s16 message_decode_char(u8 *text, u8 arg1, MessageFont *font) {
    FontChar *table;
    s32 i;
    u16 c;

    if (text[0] == 0) {
        return 0x400;
    }
    if (arg1 != 0) {
        if (text[0] < 4) {
            return (text[0] << 8) | text[1];
        }
        if (text[0] == '\n') {
            return 0x201;
        }
        c = text[0] << 8;
        c |= text[1];
        if (c >= 0x824F && c <= 0x8394) {
            table = font->chars;
            for (i = 4; table[i].sjis != 0xFFFF; i++) {
                if (c == table[i].sjis) {
                    return table[i].glyph;
                }
            }
        } else {
            table = font->chars_ext;
            for (i = 0; table[i].sjis != 0xFFFF; i++) {
                if (c == table[i].sjis) {
                    return table[i].glyph | 0x100;
                }
            }
        }
    } else {
        if (text[0] == 1) {
            if (text[1] > font->ext_count) {
                return ((font->ext_count + 1) & 0xFF) | 0x100;
            }
            return (text[0] << 8) | text[1];
        }
        if (text[0] < 4) {
            return (text[0] << 8) | text[1];
        }
        if (text[0] < font->glyph_end) {
            return text[0];
        }
    }
    return 0x300;
}

/* The data block at 0x8004DC10 (DECISIONS "Data in C, split per object": owner open; placed at the start of message's .data,
 * where it fits the link order): a zero VECTOR, an identity MATRIX (D_8004DC20: FIGHTSTG points its models at it; no
 * EXE code reads any of them) and the matrices scaling x, y and both by 2. In .data: explicitly zero-initialized. */
VECTOR D_8004DC10 = { 0, 0, 0, 0 };
MATRIX D_8004DC20 = { { { 0x1000, 0, 0 }, { 0, 0x1000, 0 }, { 0, 0, 0x1000 } }, { 0, 0, 0 } };
MATRIX D_8004DC40 = { { { 0x2000, 0, 0 }, { 0, 0x1000, 0 }, { 0, 0, 0x1000 } }, { 0, 0, 0 } };
MATRIX D_8004DC60 = { { { 0x1000, 0, 0 }, { 0, 0x2000, 0 }, { 0, 0, 0x1000 } }, { 0, 0, 0 } };
MATRIX D_8004DC80 = { { { 0x2000, 0, 0 }, { 0, 0x2000, 0 }, { 0, 0, 0x1000 } }, { 0, 0, 0 } };

/* The handlers that take only (obj, line) ignore the state. */
#define MESSAGE_HANDLER(func) ((s32 (*)(MessageWindow *, MessageLine *, MessageDrawState *))(func))

s32 (*message_control_handlers[])(MessageWindow *obj, MessageLine *line, MessageDrawState *state) = {
    MESSAGE_HANDLER(message_code_end),
    message_code_newline,
    MESSAGE_HANDLER(message_code_wait_button),
    MESSAGE_HANDLER(message_code_instant),
    message_code_nop,
    message_code_insert_line,
    MESSAGE_HANDLER(message_code_pause),
    MESSAGE_HANDLER(message_code_end),
    MESSAGE_HANDLER(message_code_player_name),
    MESSAGE_HANDLER(message_code_end),
    NULL,
};

s32 message_wait_buttons[] = { 13, 12, 13, 14, 15 };

u8 *message_cursor_frames[] = { "\x81\x85", "\x81\x86", "\x81\x87", "\x81\x88", "\x81\x87" };

MessageDialogLayout message_dialog_layouts[] = {
    { 3, { 0, -16 }, { 0, -74 }, { 16, -74 }, { 16, -74 }, { 6, -71 }, { 6, -58 }, { 16, -38 } },
    { 5, { 0, 0 }, { 0, 12 }, { 16, 12 }, { 16, 12 }, { 6, 15 }, { 6, 28 }, { 16, 48 } },
    { 4, { -16, -16 }, { -26, -74 }, { -10, -74 }, { 125, -74 }, { -20, -71 }, { -20, -58 }, { 125, -38 } },
    { 6, { -20, 0 }, { -26, 12 }, { -10, 12 }, { 125, 12 }, { -20, 15 }, { -20, 28 }, { 125, 48 } },
};

/* The fonts (message_module.fonts, indexed 1-3); their tables are font's. */
MessageFont message_fonts[] = {
    { 0xFF, 0, { 0, 0 }, NULL, NULL, NULL, NULL, 0, 0 },
    { 0xFF, 14, { 0, 0 }, font_glyphs_14, font_glyphs_14_ext, font_chars, font_chars_ext, 0xEA, 0x72 },
    { 0xFF, 11, { 0, 0 }, font_glyphs_11, font_glyphs_11_ext, font_chars, font_chars_ext, 0xEA, 0x72 },
    { 0xFF, 9, { 0, 0 }, font_glyphs_9, font_glyphs_9_ext, font_chars, font_chars_ext, 0xEA, 0x72 },
};

/* Length of each control code. */
s32 message_code_lengths[] = { 1, 2, 3, 2, 5, 3, 3, 2, 3, 2, 0 };

MessageModule message_module = { message_fonts, message_code_lengths, message_load_font, (s32 (*)(u8 *, u8, MessageFont *))message_decode_char };
