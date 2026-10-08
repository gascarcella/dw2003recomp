#ifndef STPLNMET_H
#define STPLNMET_H

/* STPLNMET.PRO (entering the player's name): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "message.h"
#include "overlay_common.h"

/* The name entry (stplnmet_entry_create, size 0xF8): STDGNAME's StdgnameNameEntry without its unk_98, and with
 * a pause method. The name being typed (two-byte characters) and a keyboard of 15 x 7 keys. */
typedef struct StplnmetNameEntry {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 tex_x;  /* texture position x */
    /* 0x60 */ s32 tex_y;  /* texture position y */
    /* 0x64 */ s32 digimon; /* the Digimon's index, -1: none */
    /* 0x68 */ s32 anim_frame; /* animation frame */
    /* 0x6C */ s32 frame_time; /* time of the last frame */
    /* 0x70 */ s32 glow_frame;
    /* 0x74 */ s32 glow_time;
    /* 0x78 */ u16 name[12];   /* the name */
    /* 0x90 */ s32 name_cursor; /* cursor in the name */
    /* 0x94 */ s32 length; /* name length (5, or 8 outside Japan) */
    /* 0x98 */ s32 key_column; /* keyboard column */
    /* 0x9C */ s32 key_row; /* keyboard row */
    /* 0xA0 */ s32 cursor_frame;
    /* 0xA4 */ s32 cursor_time;
    /* 0xA8 */ s32 input_enabled; /* input enabled */
    /* 0xAC */ s32 key_page; /* keyboard page */
    /* 0xB0 */ s32 arrows_frame;
    /* 0xB4 */ s32 arrows_time;
    /* 0xB8 */ s32 unk_B8;
    /* 0xBC */ WindowAnim unused_anim; /* only its duration is set (10): nothing starts it */
    /* 0xCC */ WindowAnim keyboard_anim;
    /* 0xDC */ WindowAnim warning_anim;
    /* 0xEC */ void (*get_name)(struct StplnmetNameEntry *obj, u8 *dst);  /* stplnmet_entry_get_name */
    /* 0xF0 */ void (*pause)(struct StplnmetNameEntry *obj, s32 pause); /* stplnmet_entry_pause */
    /* 0xF4 */ void (*close)(struct StplnmetNameEntry *obj);          /* stplnmet_entry_close */
} StplnmetNameEntry; /* size 0xF8 */

/* The scrolling background (stplnmet_bg_create, size 0x84). */
typedef struct StplnmetBg {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 tex_x;  /* texture position x */
    /* 0x54 */ s32 tex_y;  /* texture position y */
    /* 0x58 */ s32 loaded; /* the texture is loaded */
    /* 0x5C */ s32 layer_id; /* layer */
    /* 0x60 */ s32 ot_depth; /* ordering table depth */
    /* 0x64 */ s32 step_times[2]; /* per scrolling sprite: time of its last step */
    /* 0x6C */ struct {
        s32 x;
        s32 y;
    } sprite_pos[2]; /* per scrolling sprite: its position */
    /* 0x7C */ void (*load)(struct StplnmetBg *obj, s32 x, s32 y);         /* stplnmet_bg_load */
    /* 0x80 */ void (*set_layer)(struct StplnmetBg *obj, s32 layer, s32 depth); /* stplnmet_bg_set_layer */
} StplnmetBg; /* size 0x84 */

/* The main object (stplnmet_main_create, size 0x6C). */
typedef struct StplnmetMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table depth */
    /* 0x58 */ s32 tab;    /* the step's tab (0 name, 1 partner, 2 confirmation) */
    /* 0x5C */ WindowAnim frame_anim; /* the frame */
} StplnmetMain; /* size 0x6C */

/* A Digimon's animation: its sprites, ended by -1 (stplnmet_funcs.anims). */
typedef struct StplnmetDigimonAnim {
    /* 0x00 */ s32 sprites[7];
} StplnmetDigimonAnim; /* size 0x1C */

/* stplnmet_funcs: the partner Digimon's tables and the overlay's helpers, called through this table. */
typedef struct StplnmetFuncs {
    /* 0x00 */ StplnmetDigimonAnim *anims; /* stplnmet_digimon_anims: per Digimon */
    /* 0x04 */ s32 (*partners)[3]; /* stplnmet_partner_sets: the three sets of three partner Digimon */
    /* 0x08 */ void (*load)(void);                                              /* stplnmet_load_files */
    /* 0x0C */ s32 (*is_loading)(void);                                         /* stplnmet_is_loading */
    /* 0x10 */ void (*anim_start)(WindowAnim *anim, s32 open);          /* stplnmet_anim_start */
    /* 0x14 */ s32 (*anim_update)(WindowAnim *anim);                    /* stplnmet_anim_update */
    /* 0x18 */ void (*tween_start)(Tween *obj, s32 from, s32 to, s32 frames); /* stplnmet_tween_start */
    /* 0x1C */ s32 (*tween_update)(Tween *obj);                         /* stplnmet_tween_update */
} StplnmetFuncs; /* size 0x20 */

extern StplnmetFuncs stplnmet_funcs;

/* stplnmet_80082F58.c */
StplnmetBg *stplnmet_bg_create(void);
Object *stplnmet_sparkle_create(void);
Object *stplnmet_figure_create(void);
Object *stplnmet_message_create(StplnmetMain *main);

/* stplnmet_80083D70.c */
StplnmetNameEntry *stplnmet_entry_create(u8 *name);

/* stplnmet_80085ECC.c */
StplnmetMain *stplnmet_main_create(void);

#endif /* STPLNMET_H */
