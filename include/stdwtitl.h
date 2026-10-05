#ifndef STDWTITL_H
#define STDWTITL_H

/* STDWTITL.PRO (the title screen): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "overlay_common.h"

/* stdwtitl_module: the stage module's state (the last file) and its function table. Other files reach
 * it through the struct. The same module, with other contents, ends STGMCARD and FIELDSTG. */
typedef struct StdwtitlModule {
    /* 0x00 */ s32 bank;   /* sub-file ID of the title's sprite bank (stdwtitl_load_pictures) */
    /* 0x04 */ void (*load_pictures)(void);                                     /* stdwtitl_load_pictures */
    /* 0x08 */ void (*start_fade)(WindowAnim *anim, s32 open);                 /* stdwtitl_start_fade */
    /* 0x0C */ s32 (*update_fade)(WindowAnim *anim);                           /* stdwtitl_update_fade */
    /* 0x10 */ void (*start_value)(Tween *v, s32 from, s32 to, s32 frames); /* stdwtitl_start_value */
    /* 0x14 */ s32 (*step_value)(Tween *v);                               /* stdwtitl_step_value */
} StdwtitlModule; /* size 0x18 */

extern StdwtitlModule stdwtitl_module;

/* The objects the title menu (stdwtitl_title_update) keeps in its data block and calls. */

/* A sprite animation of stdwtitl_glow_update. */
typedef struct StdwtitlGlowAnim {
    /* 0x0 */ s16 index; /* into stdwtitl_glow_frames[] */
    /* 0x2 */ s16 frame;
    /* 0x4 */ s32 done;
} StdwtitlGlowAnim; /* size 0x8 */

/* Object of stdwtitl_glow_update, created by stdwtitl_glow_create. */
typedef struct StdwtitlGlow {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 skip;   /* 0: animate, else show the last frames */
    /* 0x54 */ s32 shown;  /* shown */
    /* 0x58 */ StdwtitlGlowAnim anims[2];
    /* 0x68 */ s32 layer_id; /* 0x1000 */
    /* 0x6C */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x70 */ u8 unk_70[0x8];
    /* 0x78 */ void (*start)(struct StdwtitlGlow *);
} StdwtitlGlow; /* size 0x7C */

/* A piece of the title logo (stdwtitl_shine_update_jp, stdwtitl_shine_update_intl): an animation of sprite 4
 * over sprite 2 of the title's bank. */
typedef struct StdwtitlShine {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 skip;   /* 0: animate, else show the last frame */
    /* 0x54 */ s32 index;  /* index into the frame list */
    /* 0x58 */ s32 frame;  /* frame (-1: none) */
    /* 0x5C */ s32 show_base; /* show sprite 2 */
    /* 0x60 */ s32 layer_id; /* 0x1000 */
    /* 0x64 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x68 */ u8 unk_68[0x8];
    /* 0x70 */ void (*start)(struct StdwtitlShine *);
} StdwtitlShine; /* size 0x74 */

/* A piece of the title logo, sprite 1 of the title's bank sliding in (stdwtitl_slide_update_jp,
 * stdwtitl_slide_update_intl: two layouts, by language). */
typedef struct StdwtitlSlide {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 skip;   /* 0: slide in, else show it in place */
    /* 0x54 */ s32 frames_left; /* frames left */
    /* 0x58 */ s32 x;      /* x */
    /* 0x5C */ s32 y;      /* y */
    /* 0x60 */ s32 layer_id; /* 0x1000 */
    /* 0x64 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x68 */ u8 unk_68[0x8];
    /* 0x70 */ void (*start)(struct StdwtitlSlide *);
} StdwtitlSlide; /* size 0x74 */

/* A piece of the title logo, sprite 0 of the title's bank sliding in from the left (stdwtitl_slide_left_update_jp,
 * stdwtitl_slide_left_update_intl: two layouts, by language). */
typedef struct StdwtitlSlideLeft {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 skip;   /* 0: slide in, else show it in place */
    /* 0x54 */ s32 frames_left; /* frames left */
    /* 0x58 */ s32 x;      /* x */
    /* 0x5C */ s32 y;      /* y */
    /* 0x60 */ s32 layer_id; /* 0x1000 */
    /* 0x64 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x68 */ u8 unk_68[0x8];
    /* 0x70 */ void (*start)(struct StdwtitlSlideLeft *);
} StdwtitlSlideLeft; /* size 0x74 */

/* Object of stdwtitl_menu_update, created by stdwtitl_menu_create: "press start", then the two
 * choices of the title menu with a blinking cursor. */
typedef struct StdwtitlMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s16 skip_start; /* 0: wait for start first, else open the menu at once */
    /* 0x52 */ s16 result; /* result (stdwtitl_menu_get_result): 0 none, 1 or 2 the choice, 3 timed out */
    /* 0x54 */ s16 ticks;  /* ticks */
    /* 0x56 */ s16 is_open; /* the menu is open */
    /* 0x58 */ u8 cursor_shown; /* cursor shown */
    /* 0x59 */ u8 blink;  /* blinks, or cursor frame */
    /* 0x5A */ s16 cursor; /* cursor (2: none) */
    /* 0x5C */ s32 choice1_x; /* first choice: x, y */
    /* 0x60 */ s32 choice1_y;
    /* 0x64 */ u8 unk_64[0x8];
    /* 0x6C */ s32 choice2_x; /* second choice: x, y */
    /* 0x70 */ s32 choice2_y;
    /* 0x74 */ u8 unk_74[0x18];
    /* 0x8C */ s32 layer_id; /* 0x1000 */
    /* 0x90 */ s32 ot_depth; /* ordering table depth: set to 2, never read */
    /* 0x94 */ u8 unk_94[0x8];
    /* 0x9C */ void (*start)(struct StdwtitlMenu *);
    /* 0xA0 */ void (*show_press_start)(struct StdwtitlMenu *);
    /* 0xA4 */ s32 (*get_result)(struct StdwtitlMenu *); /* stdwtitl_menu_get_result (defined s16; the caller uses a word) */
} StdwtitlMenu; /* size 0xA8 */

/* Creators called from other files. */
Object *stdwtitl_picture_screen_create(void);
StdwtitlGlow *stdwtitl_glow_create(s32 arg0);
Object *stdwtitl_movie_screen_create(s32 movie);
StdwtitlShine *stdwtitl_shine_create_jp(s32 arg0);
StdwtitlShine *stdwtitl_shine_create_intl(s32 arg0);
Object *stdwtitl_picture_create(void);
Object *stdwtitl_title_screen_create(void);
StdwtitlSlide *stdwtitl_slide_create_jp(s32 arg0);
StdwtitlSlide *stdwtitl_slide_create_intl(s32 arg0);
StdwtitlSlideLeft *stdwtitl_slide_left_create_jp(s32 arg0);
StdwtitlSlideLeft *stdwtitl_slide_left_create_intl(s32 arg0);
StdwtitlMenu *stdwtitl_menu_create(s16 arg0);
Object *stdwtitl_title_create(Object *parent);

#endif /* STDWTITL_H */
