#ifndef OVERLAY_COMMON_H
#define OVERLAY_COMMON_H

/* Types of the helpers that nearly every tier-1 overlay has its own byte-identical copy of (separate link units:
 * docs/MATCHING.md "C patterns"). CARDGAME still has local copies; the EXE's inn.c
 * defines Fade's functions (inn_fade_create).
 * WindowAnim (window_anim_type.h) is the EXE's window_anim type: overlays that define the pair after other
 * functions (STGMCARD's and STDWTITL's start_fade/update_fade) can't use window_anim.h's static copies. */

#include "common.h"
#include "object.h"
#include "window_anim_type.h"

/* A full-screen fade (<overlay>_fade_create, size 0x68; the same object as inn.c's inn_fade_create, STCRDDEK and
 * STCRDSHP put it on layer 0x1000): a POLY_F4 drawn with subtractive blending. */
typedef struct Fade {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 ot_depth; /* ordering table depth */
    /* 0x58 */ s32 from_black; /* 0: to black, else from black */
    /* 0x5C */ s32 level;  /* level, 8.8 fixed point (0..0xFF00) */
    /* 0x60 */ s32 step;   /* step per frame */
    /* 0x64 */ void (*start)(struct Fade *obj, s32 dir, s32 frames); /* <overlay>_fade_start: dir 0 fades to black, 1 back */
} Fade; /* size 0x68 */

/* A value moving linearly to a target over a number of frames (<overlay>_tween_start or _lerp_start starts it,
 * _tween_update or _lerp_update steps it). */
typedef struct Tween {
    /* 0x00 */ s32 duration; /* in frames */
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ s32 value;
    /* 0x0C */ s32 acc;      /* value, 24.8 fixed point */
    /* 0x10 */ s32 target;
    /* 0x14 */ s32 step;     /* 24.8 fixed point, added every frame */
    /* 0x18 */ s32 running;
} Tween; /* size 0x1C */

/* The stage module's helper functions, in the order every overlay's table has them: the whole table in
 * STCRDABM, STCRDDEK and STCRDSHP; the other overlays' tables put their own data first and keep their own types. */
typedef struct StageUtil {
    /* 0x00 */ void (*load_files)(void);
    /* 0x04 */ s32 (*is_loading)(void);
    /* 0x08 */ void (*window_anim_start)(WindowAnim *anim, s32 open);
    /* 0x0C */ s32 (*window_anim_update)(WindowAnim *anim);
    /* 0x10 */ void (*tween_start)(Tween *tween, s32 from, s32 to, s32 frames);
    /* 0x14 */ s32 (*tween_update)(Tween *tween);
} StageUtil; /* size 0x18 */

#endif /* OVERLAY_COMMON_H */
