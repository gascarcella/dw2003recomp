#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "object.h"
#include "psyq/libgte.h"

/* A text line of the message window (6 of them at 0x5C), the arg1 of message_handle_char and of the
 * control-code handlers in message_control_handlers. */
typedef struct MessageLine {
    /* 0x0 */ u8 *text;
    /* 0x4 */ s16 capacity; /* size of the text buffer (message_copy_line_text reallocates it) */
    /* 0x6 */ s16 length; /* length of text */
    /* 0x8 */ s16 pos;   /* position in text */
    /* 0xA */ s16 is_sjis; /* 0: game glyph codes (message_set_line_text); else Shift-JIS (docs/FORMATS.md "Text") */
} MessageLine; /* size 0xC */

/* A glyph of a font (MessageFont.glyphs/glyphs_ext). */
typedef struct FontGlyph {
    /* 0x0 */ u8 tpage_x; /* texture page x / 64; 0xFF: no glyph */
    /* 0x1 */ u8 u;
    /* 0x2 */ u8 v;
    /* 0x3 */ u8 clut_x;
    /* 0x4 */ u8 clut_y;
    /* 0x5 */ u8 w;
    /* 0x6 */ u8 h;
    /* 0x7 */ s8 x_offset;
    /* 0x8 */ s8 y_offset;
    /* 0x9 */ u8 advance;
    /* 0xA */ u8 unk_A;
} FontGlyph; /* size 0xB */

/* An entry of a font's character tables (MessageFont.chars/chars_ext), ended by code 0xFFFF. */
typedef struct FontChar {
    /* 0x0 */ u16 sjis;  /* Shift-JIS code */
    /* 0x2 */ u8 glyph;  /* glyph code */
    /* 0x3 */ u8 pad_3;
} FontChar; /* size 0x4 */

/* The fonts' character tables (font.c): digits and letters, other characters. The overlays' name entry fonts
 * use them too. */
extern FontChar font_chars[];
extern FontChar font_chars_ext[];

/* An entry of the table at message_fonts (message_module.fonts; indexed 1-3): a font. */
typedef struct MessageFont {
    /* 0x00 */ u8 semi_trans; /* copied to MessageWindow.semi_trans by message_create_window; 0xFF: opaque */
    /* 0x01 */ s8 line_height; /* 14/11/9 */
    /* 0x02 */ u8 pad_02[0x2];
    /* 0x04 */ FontGlyph *glyphs; /* glyphs of characters 4 and up */
    /* 0x08 */ FontGlyph *glyphs_ext; /* ext_count more glyphs, from 1 (control code 1) */
    /* 0x0C */ FontChar *chars;  /* digits and letters (glyphs of glyphs) */
    /* 0x10 */ FontChar *chars_ext; /* other characters (glyphs of glyphs_ext) */
    /* 0x14 */ s16 glyph_end; /* 0xEA: codes 4..glyph_end - 1 are in glyphs */
    /* 0x16 */ s16 ext_count;
} MessageFont; /* size 0x18 */

/* message_module: message's table pointers and the function table {message_load_font, message_decode_char}. It is
 * 0x10 bytes, so -G8 files reach it with %hi/%lo; declared as a scalar it would be small data.
 * decode_char: message_decode_char is defined returning s16, but font's font_get_text_width uses the call's result as a
 * full word (no sign extension), so the entry returns s32; message's callers keep the result in an s16. */
typedef struct MessageModule {
    /* 0x0 */ MessageFont *fonts; /* message_fonts */
    /* 0x4 */ s32 *code_lengths;  /* message_code_lengths: length of each control code */
    /* 0x8 */ void (*load_font)(void); /* message_load_font */
    /* 0xC */ s32 (*decode_char)(u8 *text, u8 arg1, MessageFont *font); /* message_decode_char */
} MessageModule; /* size 0x10 */

extern MessageModule message_module;

/* The message window object, created by message_create_window (size 0x174), with its methods at 0x110. */
typedef struct MessageWindow {
    /* 0x000 */ Object base;
    /* 0x050 */ MessageFont *font;
    /* 0x054 */ s32 layer_id; /* gfx layer it draws in */
    /* 0x058 */ s32 ot_depth; /* ordering table entry of the layer */
    /* 0x05C */ MessageLine lines[6];
    /* 0x0A4 */ s16 shown_end;  /* text drawn up to here (grows by one per speed frames; message_find_page_end) */
    /* 0x0A6 */ s16 page_start; /* start of the page shown */
    /* 0x0A8 */ s16 speed_count; /* frames since the last character */
    /* 0x0AA */ s16 speed;      /* frames per character, 0: all at once (message_set_speed) */
    /* 0x0AC */ s16 vram_x;     /* the font's texture/CLUT base in VRAM (0x140, 0) */
    /* 0x0AE */ s16 vram_y;
    /* 0x0B0 */ s16 x;          /* position (message_set_pos) */
    /* 0x0B2 */ s16 y;
    /* 0x0B4 */ s16 fixed_w;    /* with fixed_size: advance per character */
    /* 0x0B6 */ s16 fixed_h;    /* with fixed_size: line height */
    /* 0x0B8 */ s16 pen_x;      /* where the next character goes, relative to x/y */
    /* 0x0BA */ s16 pen_y;
    /* 0x0BC */ s16 measured_width; /* text width (message_measure): lines start at x - measured_width */
    /* 0x0BE */ s8 semi_trans; /* semi-transparency rate; -1: opaque */
    /* 0x0BF */ u8 page_lines;  /* lines per page (control code 1) */
    /* 0x0C0 */ u8 palette;     /* added to the CLUT row */
    /* 0x0C1 */ u8 visible;     /* drawn (0 while it has no text) */
    /* 0x0C2 */ u8 fixed_size;  /* message_set_fixed_size */
    /* 0x0C3 */ u8 done;        /* the page is drawn to its end (message_is_done) */
    /* 0x0C4 */ u8 unk_C4;      /* set by message_set_unk_C4 (set_unk_C4), never read */
    /* 0x0C5 */ u8 pad_C5[0x3];
    /* 0x0C8 */ s32 char_sound; /* sound played per character shown (0: none) */
    /* 0x0CC */ s32 scaled; /* draw through matrix (message_set_scale) */
    /* 0x0D0 */ VECTOR scale;
    /* 0x0E0 */ s32 center_x;
    /* 0x0E4 */ s32 center_y;
    /* 0x0E8 */ SVECTOR rotation;
    /* 0x0F0 */ MATRIX matrix;
    /* 0x110 */ void (*copy_text)(struct MessageWindow *obj, u8 *text); /* message_copy_text */
    /* 0x114 */ void (*set_text)(struct MessageWindow *obj, u8 *text, s32 arg2); /* message_set_text */
    /* 0x118 */ void (*set_line_number)();                            /* message_set_line_number */
    /* 0x11C */ void (*set_ext_line_text)();                          /* message_set_ext_line_text */
    /* 0x120 */ void (*set_line_text)(struct MessageWindow *obj, u8 *text, s32 arg2, s32 line); /* message_set_line_text */
    /* 0x124 */ void (*draw_window)(struct MessageWindow *obj);         /* message_draw_window */
    /* 0x128 */ void (*find_page_end)(struct MessageWindow *obj);       /* message_find_page_end */
    /* 0x12C */ void (*set_font)(struct MessageWindow *obj, s32 arg1);  /* message_set_font */
    /* 0x130 */ void (*set_speed)(struct MessageWindow *obj, s32 arg1); /* message_set_speed */
    /* 0x134 */ void (*set_pos)(struct MessageWindow *obj, s16 arg1, s16 arg2); /* message_set_pos */
    /* 0x138 */ void (*set_palette)(struct MessageWindow *obj, u8 arg1); /* message_set_palette */
    /* 0x13C */ void (*set_semi_trans)(struct MessageWindow *obj, u8 arg1); /* message_set_semi_trans */
    /* 0x140 */ void (*set_fixed_size)(struct MessageWindow *obj, s16 arg1, s16 arg2); /* message_set_fixed_size */
    /* 0x144 */ void (*set_visible)(struct MessageWindow *obj, u8 arg1); /* message_set_visible */
    /* 0x148 */ void (*measure)(struct MessageWindow *obj, u8 arg1);    /* message_measure */
    /* 0x14C */ void (*insert_names)();                               /* message_insert_names */
    /* 0x150 */ void (*set_char_sound)(struct MessageWindow *obj, s32 arg1); /* message_set_char_sound */
    /* 0x154 */ void (*set_scale)(struct MessageWindow *obj, s32 arg1, s32 arg2); /* message_set_scale */
    /* 0x158 */ void (*set_center)(struct MessageWindow *obj, s32 arg1, s32 arg2); /* message_set_center */
    /* 0x15C */ void (*set_ot_depth)(struct MessageWindow *obj, s32 arg1); /* message_set_ot_depth */
    /* 0x160 */ void (*set_page_lines)(struct MessageWindow *obj, u8 arg1); /* message_set_page_lines */
    /* 0x164 */ void (*set_unk_C4)(struct MessageWindow *obj, u8 arg1); /* message_set_unk_C4 */
    /* 0x168 */ s32 (*is_done)(struct MessageWindow *obj);              /* message_is_done */
    /* 0x16C */ s32 (*is_visible)(struct MessageWindow *obj);           /* message_is_visible */
    /* 0x170 */ s32 (*is_waiting)(struct MessageWindow *obj);           /* message_is_waiting: waits for a button (control code 2) */
} MessageWindow; /* size 0x174 */

/* The object type created by message_create_cursor (size 0x98). It drives a message window
 * (message_create_window) through the window's methods. */
typedef struct MessageCursor {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id;
    /* 0x54 */ s32 ot_depth;
    /* 0x58 */ s32 x;
    /* 0x5C */ s32 y;
    /* 0x60 */ s32 palette;
    /* 0x64 */ s32 shown;
    /* 0x68 */ s32 dirty;  /* set when 0x58-0x64 changed */
    /* 0x6C */ s32 frame;    /* animation frame 0-4 (message_cursor_frames) */
    /* 0x70 */ s32 last_time; /* gfx time of the last frame change */
    /* 0x74 */ s32 delay;    /* time before the animation starts (0x20) */
    /* 0x78 */ s32 interval; /* time per animation frame (6) */
    /* 0x7C */ s32 stopped;  /* non-zero: shows frame 0 (message_cursor_stop) */
    /* 0x80 */ void (*show)(struct MessageCursor *obj, s32 arg1);             /* message_cursor_show */
    /* 0x84 */ void (*set_pos)(struct MessageCursor *obj, s32 arg1, s32 arg2); /* message_cursor_set_pos */
    /* 0x88 */ void (*set_palette)(struct MessageCursor *obj, s32 arg1);      /* message_cursor_set_palette */
    /* 0x8C */ void (*set_delay)(struct MessageCursor *obj, s32 arg1);        /* message_cursor_set_delay */
    /* 0x90 */ void (*set_interval)(struct MessageCursor *obj, s32 arg1);     /* message_cursor_set_interval */
    /* 0x94 */ void (*stop)(struct MessageCursor *obj, s32 arg1);             /* message_cursor_stop */
} MessageCursor; /* size 0x98 */

/* The object type created by message_rlen_create (size 0x84): unpacks RLEN data. */
typedef struct MessageRlen {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *data;   /* the data given to message_rlen_set_data */
    /* 0x54 */ void *packed; /* the packed bytes (data + 8) */
    /* 0x58 */ s32 is_packed; /* the data is RLEN-packed */
    /* 0x5C */ s32 size;    /* its unpacked size */
    /* 0x60 */ s32 buffer_size; /* size of the buffer at 0x64 */
    /* 0x64 */ void *buffer; /* the unpacked data (message_rlen_alloc_buffer) */
    /* 0x68 */ void *src;    /* unpacking: read position */
    /* 0x6C */ void *dst;    /* unpacking: write position */
    /* 0x70 */ s32 chunk_size; /* bytes unpacked per update (message_rlen_unpack_start; unpack_now: all) */
    /* 0x74 */ void *(*unpack_now)(struct MessageRlen *obj, s32 *data);       /* message_rlen_unpack_now */
    /* 0x78 */ void *(*get_data)(struct MessageRlen *obj);                    /* message_rlen_get_data */
    /* 0x7C */ void (*unpack_start)(struct MessageRlen *obj, s32 *data, s32 arg2); /* message_rlen_unpack_start */
    /* 0x80 */ void (*free)(struct MessageRlen *obj);                         /* message_rlen_free */
} MessageRlen; /* size 0x84 */

/* The object type created by message_create_dialog (size 0x6C). */
typedef struct MessageDialog {
    /* 0x00 */ Object base;
    /* 0x50 */ void *messages; /* the messages (message_create_dialog's arg3) */
    /* 0x54 */ s32 layer_id;
    /* 0x58 */ s32 type;   /* index of message_dialog_layouts */
    /* 0x5C */ s32 delay_count; /* frames counted before each of the three outlines (opening and closing) */
    /* 0x60 */ s16 x;
    /* 0x62 */ s16 y;
    /* 0x64 */ s16 width;  /* the text's width, 0x5F..0x8B */
    /* 0x66 */ s16 height; /* 0x3E */
    /* 0x68 */ void (*move)(struct MessageDialog *obj, s32 x, s32 y);   /* message_dialog_move */
} MessageDialog; /* size 0x6C */

MessageWindow *message_create_window(s16 arg0, s16 arg1, s16 arg2, s16 arg3);
MessageCursor *message_create_cursor(s16 arg0, s32 arg1, s16 arg2, s16 arg3);
MessageRlen *message_rlen_create(void);
Object *message_box_create(s32 arg0, u8 *text, s32 arg2);
MessageDialog *message_create_dialog(s32 arg0, s16 x, s16 y, void *arg3, s32 arg4, s32 type);

#endif /* MESSAGE_H */
