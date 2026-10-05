#ifndef WINDOW_ANIM_H
#define WINDOW_ANIM_H

#include "common.h"
#include "sound.h"

/* A window's open/close animation: `level` goes 0 -> 0x1000 over `duration` frames when opening, and
 * back twice as fast when closing; the caller draws the window frame scaled by level / 0x1000.
 * The functions are static, so each file that includes this header has its own copy, at the top of
 * the file: inn.c (0x80010F4C, 0x80010FE0) and fieldmenu.c (0x800120B4, 0x80012148)
 * are byte-identical. */
#include "window_anim_type.h"

/* Starts opening (`open`) or closing; plays the matching sound. */
static void window_anim_start(WindowAnim *anim, s32 open) {
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

/* Steps the animation; returns 1 once it is finished (or not running). */
static s32 window_anim_update(WindowAnim *anim) {
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

#endif /* WINDOW_ANIM_H */
