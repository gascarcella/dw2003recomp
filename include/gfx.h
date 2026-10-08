#ifndef GFX_H
#define GFX_H

#include "common.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"

typedef struct GfxRect {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 w;
    /* 0x6 */ s16 h;
} GfxRect;

struct GfxLayer;

/* A list node of GfxLayer (gfx_alloc_callbacks allocates them, gfx_run_callbacks runs them: func(data,
 * the layer, arg)). */
typedef struct GfxCallback {
    /* 0x00 */ s32 priority; /* priority (gfx_add_callback) */
    /* 0x04 */ s32 arg;      /* func's third argument (gfx_add_callback's last) */
    /* 0x08 */ void (*func)(void *data, struct GfxLayer *layer, s32 arg);
    /* 0x0C */ void *data;   /* data */
    /* 0x10 */ struct GfxCallback *next;
} GfxCallback; /* size 0x14 */

/* A drawing layer, created by gfx_new_layer (size 0x16C): a DRAWENV, two ordering tables, a list of
 * callbacks, matrices, and a table of its functions. Kept in the gfx slots (gfx_module.layers). */
typedef struct GfxLayer {
    /* 0x00 */ DRAWENV env;
    /* 0x5C */ u32 *ots[2];    /* ordering tables, one per buffer (index gfx_module.buffer) */
    /* 0x64 */ s32 ot_length;  /* ordering table length */
    /* 0x68 */ s32 ot_bits;    /* ordering table depth in bits */
    /* 0x6C */ s16 draw_x;     /* draw offset (DRAWENV.ofs; gfx_set_draw_offset) */
    /* 0x6E */ s16 draw_y;
    /* 0x70 */ s32 scroll_x;   /* scroll, 24.8 fixed point (gfx_set_scroll, gfx_add_scroll; gfx_get_view_rect) */
    /* 0x74 */ s32 scroll_y;
    /* 0x78 */ s32 callback_capacity; /* capacity of callbacks */
    /* 0x7C */ s32 callbacks_used; /* nodes used */
    /* 0x80 */ GfxCallback *callbacks; /* callbacks[0] is the list head */
    /* 0x84 */ s32 camera_saved; /* gfx_save_camera's flag: gfx_end_frame restores cameras[] before the callbacks */
    /* 0x88 */ s32 projection;   /* projection distance (GsSetProjection) */
    /* 0x8C */ MATRIX cameras[2]; /* per buffer, saved from / restored to GsWSMATRIX */
    /* 0xCC */ s32 light_saved; /* gfx_save_light's flag: restored likewise */
    /* 0xD0 */ MATRIX lights[2]; /* per buffer, saved from / restored to GsLIGHTWSMATRIX */
    /* 0x110 */ void (*set_clip_pos)(struct GfxLayer *, s32, s32);    /* gfx_set_clip_pos (s16 params; callers pass s32) */
    /* 0x114 */ void (*set_clip_size)(struct GfxLayer *, s32, s32);   /* gfx_set_clip_size (s16 params; callers pass s32) */
    /* 0x118 */ void (*set_draw_offset)(struct GfxLayer *, s32, s32); /* gfx_set_draw_offset (defined with u16 arguments; callers pass full words) */
    /* 0x11C */ void (*get_scroll)(struct GfxLayer *, s32 *);         /* gfx_get_scroll */
    /* 0x120 */ void (*set_scroll)(struct GfxLayer *, s32, s32);      /* gfx_set_scroll */
    /* 0x124 */ void (*add_scroll)(struct GfxLayer *, s32, s32);      /* gfx_add_scroll */
    /* 0x128 */ void (*get_view_rect)(struct GfxLayer *, GfxRect *);     /* gfx_get_view_rect */
    /* 0x12C */ void (*set_bg_color)(struct GfxLayer *, u8, u8, u8);  /* gfx_set_bg_color */
    /* 0x130 */ void (*draw)(struct GfxLayer *);                      /* gfx_draw_layer */
    /* 0x134 */ void (*clear_ot)(struct GfxLayer *);                  /* gfx_clear_ot */
    /* 0x138 */ u32 *(*get_ot_entry)(struct GfxLayer *, s32);         /* gfx_get_ot_entry */
    /* 0x13C */ u32 *(*get_ot_entry_z)(struct GfxLayer *, s32);       /* gfx_get_ot_entry_z */
    /* 0x140 */ u32 *(*get_ot)(struct GfxLayer *);                    /* gfx_get_ot */
    /* 0x144 */ s32 (*get_ot_bits)(struct GfxLayer *);                /* gfx_get_ot_bits */
    /* 0x148 */ void (*alloc_callbacks)(struct GfxLayer *, s32);      /* gfx_alloc_callbacks */
    /* 0x14C */ void (*add_callback)(struct GfxLayer *, void (*)(void *, struct GfxLayer *, s32), void *, s32, s32); /* gfx_add_callback */
    /* 0x150 */ void (*append_callback)(struct GfxLayer *, void (*)(void *, struct GfxLayer *, s32), void *); /* gfx_append_callback */
    /* 0x154 */ void (*run_callbacks)(struct GfxLayer *);             /* gfx_run_callbacks */
    /* 0x158 */ void (*restore_camera)(struct GfxLayer *);            /* gfx_restore_camera */
    /* 0x15C */ void (*save_camera)(struct GfxLayer *, s32, s32);     /* gfx_save_camera */
    /* 0x160 */ void (*restore_light)(struct GfxLayer *);      /* gfx_restore_light */
    /* 0x164 */ void (*save_light)(struct GfxLayer *, s32);    /* gfx_save_light */
    /* 0x168 */ void (*free)(struct GfxLayer *);                      /* gfx_free_layer */
} GfxLayer; /* size 0x16C */

/* The gfx module's table of functions (gfx_module.funcs). */
typedef struct GfxFuncs {
    /* 0x00 */ void *(*get_packet)(void);                       /* gfx_get_packet */
    /* 0x04 */ void (*set_packet)(void *);                      /* gfx_set_packet */
    /* 0x08 */ void (*free_packet_buffers)(void);               /* gfx_free_packet_buffers */
    /* 0x0C */ void (*set_vsync_callback)(void);                /* gfx_set_vsync_callback */
    /* 0x10 */ void (*end_frame)(s32);                          /* gfx_end_frame */
    /* 0x14 */ GfxLayer *(*create_layer)(RECT *, s32, s32);  /* gfx_create_layer */
    /* 0x18 */ s32 (*delete_layer)(s32 id);                     /* gfx_delete_layer */
    /* 0x1C */ void (*init_display)(s32, s32, s32, s32);        /* gfx_init_display */
    /* 0x20 */ void (*set_display_area)(s32, s32, s32, s32);    /* gfx_set_display_area */
    /* 0x24 */ GfxLayer *(*get_layer)(s32 id);               /* gfx_get_layer */
    /* 0x28 */ void (*move_layer)(s32, s32, s32);               /* gfx_move_layer */
    /* 0x2C */ s32 (*get_frames)(void);                         /* gfx_get_frames */
    /* 0x30 */ s32 (*get_time)(void);                           /* gfx_get_time */
    /* 0x34 */ s32 (*get_frame_ticks)(void);                    /* gfx_get_frame_ticks */
} GfxFuncs;

/* gfx_module: the gfx module's state and its function table (game code calls through funcs). */
typedef struct GfxModule {
    /* 0x00 */ void (*vsync_callback)(s32); /* called with vsync_arg on every vsync (gfx_vsync_callback) */
    /* 0x04 */ s32 vsync_arg;
    /* 0x08 */ s32 frames; /* frames (frames_fixed >> 8): gfx_get_frames */
    /* 0x0C */ s32 frames_fixed; /* 24.8: +1 per frame on 60 Hz, +1.2 on 50 Hz */
    /* 0x10 */ s32 time;   /* time_fixed >> 8: gfx_get_time */
    /* 0x14 */ s32 time_fixed; /* 24.8: +1 per vsync on 60 Hz, +1.2 on 50 Hz */
    /* 0x18 */ s32 frame_ticks; /* whole ticks of ticks_fixed since the last frame: gfx_get_frame_ticks */
    /* 0x1C */ s32 ticks_fixed; /* 24.8: like time_fixed, whole part taken every frame */
    /* 0x20 */ void *packet;    /* packet_buffers[buffer]: where the next primitive goes */
    /* 0x24 */ void *packet_buffers[2]; /* two buffers of packet_buffer_size bytes */
    /* 0x2C */ s32 packet_buffer_size;
    /* 0x30 */ s32 disp_buffer; /* DISPENV shown (toggled on vsync in interlaced mode) */
    /* 0x34 */ s32 buffer;      /* current buffer, 0 or 1 */
    /* 0x38 */ DISPENV dispenvs[2];
    /* 0x60 */ GfxLayer *layers[30]; /* slots: an object and its ID */
    /* 0xD8 */ s32 layer_ids[30];
    /* 0x150 */ void (*reset)(void);     /* gfx_reset */
    /* 0x154 */ void (*alloc_packet_buffers)(s32 size); /* gfx_alloc_packet_buffers */
    /* 0x158 */ GfxFuncs funcs;
} GfxModule; /* size 0x190 */

extern GfxModule gfx_module;

/* Matrices in libgs's .bss (psyq/libgs/bss): GsWSMATRIX the world-screen matrix (GsSetRefView2; GfxLayer.cameras),
 * GsLIGHTWSMATRIX the flat-light matrix (GsSetFlatLight, port/psyq/libgs.c; GfxLayer.lights); gfx saves and
 * restores both per buffer, FIGHTSTG composes its models with them. */
extern MATRIX GsLIGHTWSMATRIX;
extern MATRIX GsWSMATRIX;

/* An identity MATRIX in the EXE data block at 0x8004DC10 (defined in message.c, the start of its .data; owner open;
 * no EXE code reads it). FIGHTSTG points its models at it. */
extern MATRIX message_identity_matrix;

/* message.h's types, for Font. */
struct MessageLine;
struct MessageFont;

/* An object set up by sprite_init (size 0xA0): parameters and a table of its functions. */
typedef struct Sprite {
    /* 0x00 */ GfxLayer *layer;
    /* 0x04 */ u32 *ot_entry; /* an ordering table entry of layer */
    /* 0x08 */ s32 vram_x; /* texture position (sprite_set_vram_pos also sets clut_x/clut_y to it) */
    /* 0x0C */ s32 vram_y;
    /* 0x10 */ s32 clut_x;
    /* 0x14 */ s32 clut_y;
    /* 0x18 */ s32 clut8_x; /* CLUT position for 8-bit cells (clut_x/clut_y: 4-bit) */
    /* 0x1C */ s32 clut8_y;
    /* 0x20 */ s32 palette; /* added to the CLUT row */
    /* 0x24 */ s32 follow_scroll; /* non-zero (default): sprite_draw subtracts the layer's scroll */
    /* 0x28 */ CVECTOR color;
    /* 0x2C */ s32 matrix_dirty; /* set by sprite_set_scale/sprite_set_rotation: sprite_draw rebuilds matrix */
    /* 0x30 */ s32 pivot_x;
    /* 0x34 */ s32 pivot_y;
    /* 0x38 */ VECTOR scale;   /* scale (ONE = 0x1000) */
    /* 0x48 */ SVECTOR rotation; /* rotation */
    /* 0x50 */ MATRIX matrix;  /* rotation and scale as a matrix */
    /* 0x70 */ void (*set_current)(struct Sprite *);
    /* 0x74 */ void (*set_vram_pos)(s32, s32);
    /* 0x78 */ void (*set_clut8_pos)(s32, s32);
    /* 0x7C */ void (*set_layer_id)(s32, s32);
    /* 0x80 */ void (*set_layer)(GfxLayer *, s32);
    /* 0x84 */ void (*draw)(s32 *bank, s32 id, s32 x, s32 y);
    /* 0x88 */ void (*set_palette)(s32);
    /* 0x8C */ void (*set_scale)(s32, s32, s32);
    /* 0x90 */ void (*set_rotation)(s16, s16, s16);
    /* 0x94 */ void (*set_pivot)(s32, s32);
    /* 0x98 */ void (*set_follow_scroll)(s32);
    /* 0x9C */ void (*set_color)(CVECTOR *);
} Sprite; /* size 0xA0 */

/* An object set up by font_init (size 0xC): a table of its functions. */
typedef struct Font {
    /* 0x0 */ void *(*get_entry)(void *data, s32 i);                      /* font_get_entry */
    /* 0x4 */ s32 (*get_text_width)(struct MessageLine *line, struct MessageFont *font, s16 width); /* font_get_text_width */
    /* 0x8 */ void (*convert_text)(u8 *dst, u8 *src, s32 mode);              /* font_convert_text */
} Font; /* size 0xC */

/* An object set up by tim_init (size 0x34): parameters and a table of its functions. */
typedef struct Tim {
    /* 0x00 */ s16 width;  /* size of the last image loaded by tim_load */
    /* 0x02 */ s16 height;
    /* 0x04 */ u8 unk_04[0x4];
    /* 0x08 */ s32 image_x; /* VRAM position for images */
    /* 0x0C */ s32 image_y;
    /* 0x10 */ s32 clut_x; /* VRAM position for CLUTs */
    /* 0x14 */ s32 clut_y;
    /* 0x18 */ s32 buffer_size;
    /* 0x1C */ void (*set_module)(struct Tim *);
    /* 0x20 */ void (*load)(u32 *);
    /* 0x24 */ void (*set_image_pos)(s32, s32);
    /* 0x28 */ void (*set_clut_pos)(s32, s32);
    /* 0x2C */ void (*load_all)(s32 *data);
    /* 0x30 */ void (*set_buffer_size)(s32);
} Tim; /* size 0x34 */

/* An object set up by card_init (size 0x54): parameters and a table of its functions. Draws card pictures
 * (STCRDABM, STCRDDEK, STCRDSHP: select(card) loads the card's record, load_image() its image into VRAM). */
typedef struct CardPicture {
    /* 0x00 */ u8 *record; /* a 0x62C-byte record (card_select) */
    /* 0x04 */ s32 image_x;
    /* 0x08 */ s32 image_y;
    /* 0x0C */ s32 clut_x;
    /* 0x10 */ s32 clut_y;
    /* 0x14 */ s32 cell_x;
    /* 0x18 */ s32 cell_y;
    /* 0x1C */ s32 clut_stride;
    /* 0x20 */ s32 semi_trans;
    /* 0x24 */ GfxLayer *layer;
    /* 0x28 */ u32 *ot_entry; /* an ordering table entry of layer */
    /* 0x2C */ void (*select)(s32);      /* card_select: selects record n (a card; 64 per file) */
    /* 0x30 */ void (*load_image)(void); /* card_load_image: uploads the record's image */
    /* 0x34 */ void (*draw)(s32, s32);   /* card_draw: draws the 32x32 sprite at (x, y) */
    /* 0x38 */ void (*set_layer)(s32, s32); /* card_set_layer: layer and ordering table entry */
    /* 0x3C */ void (*set_image_pos)(s32, s32); /* card_set_image_pos: image position in VRAM */
    /* 0x40 */ void (*set_clut_pos)(s32, s32); /* card_set_clut_pos: CLUT position in VRAM */
    /* 0x44 */ void (*set_cell)(s32, s32); /* card_set_cell: cell (column, row) */
    /* 0x48 */ void (*set_clut_stride)(s32); /* card_set_clut_stride */
    /* 0x4C */ void (*set_semi_trans)(s32); /* card_set_semi_trans: semi-transparent */
    /* 0x50 */ s32 (*get_class)(void);   /* card_get_class: the selected card's class (card_classes) */
} CardPicture; /* size 0x54 */

void sprite_init(Sprite *obj);
void font_init(Font *obj);
void tim_init(Tim *obj);
void card_init(CardPicture *obj);

#endif /* GFX_H */
