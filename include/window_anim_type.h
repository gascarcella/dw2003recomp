#ifndef WINDOW_ANIM_TYPE_H
#define WINDOW_ANIM_TYPE_H

#include "common.h"

/* window_anim.h's type alone, for files with their own (non-static) copies of its functions, such as
 * the overlays' helper tables (STCRDDEK's stcrddek_util). */
typedef struct WindowAnim {
    /* 0x00 */ s32 duration; /* in frames */
    /* 0x04 */ s32 step;     /* added every frame */
    /* 0x08 */ s32 level;    /* 0..0x1000 */
    /* 0x0C */ s32 running;
} WindowAnim; /* size 0x10 */

#endif /* WINDOW_ANIM_TYPE_H */
