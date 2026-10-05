#include "common.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "records.h"
#include "stcrddek.h"

/* The overlay's helper table stcrddek_util and its functions (the same file ends STCRDSHP, STCRDABM,
 * STGMCARD, STITSHOP, STGDGLAB and other menus; only the files loaded differ). */


/* Uploads the screen's image (0x0642) and starts loading the files the screen needs. */
void stcrddek_load_files(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x06420000));
    cdload_module.queue_file(0x7F6);
    cdload_module.queue_file(0x7F7);
    cdload_module.queue_file(0x7F8);
    cdload_module.queue_file(0x7F9);
    cdload_module.queue_file(0x7FA);
    cdload_module.queue_file(records_language + 0x16);
    cdload_module.queue_file(records_language + 0x1D);
    cdload_module.queue_file(records_language + 0x32);
    cdload_module.queue_file(records_language + 0x86);
    cdload_module.queue_file(0x771);
}

/* 1 while one of the language files (or 0x771) is still loading. */
s32 stcrddek_is_loading(void) {
    if (cdload_module.is_loading(records_language + 0x16)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x1D)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x32)) {
        return 1;
    }
    if (cdload_module.is_loading(records_language + 0x86)) {
        return 1;
    }
    return cdload_module.is_loading(0x771) != 0;
}

/* window_anim.h's functions, as non-static copies for the table. */
void stcrddek_window_anim_start(WindowAnim *anim, s32 open) {
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

s32 stcrddek_window_anim_update(WindowAnim *anim) {
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

void stcrddek_lerp_start(Tween *lerp, s32 from, s32 to, s32 frames) {
    if (from != to) {
        lerp->duration = frames;
        lerp->acc = from << 8;
        lerp->value = from;
        lerp->target = to;
        lerp->running = 1;
        lerp->step = ((to - from) << 8) / lerp->duration;
    }
}

/* Steps the value; returns 1 once it is at the end (or not running). */
s32 stcrddek_lerp_update(Tween *lerp) {
    if (lerp->running == 0) {
        return 1;
    }
    lerp->acc += lerp->step;
    lerp->value = lerp->acc >> 8;
    if (lerp->step > 0) {
        if (lerp->value > lerp->target) {
            lerp->value = lerp->target;
            lerp->running = 0;
            return 1;
        }
    } else if (lerp->value < lerp->target) {
        lerp->value = lerp->target;
        lerp->running = 0;
        return 1;
    }
    return 0;
}

StageUtil stcrddek_util = {
    stcrddek_load_files,
    stcrddek_is_loading,
    stcrddek_window_anim_start,
    stcrddek_window_anim_update,
    stcrddek_lerp_start,
    stcrddek_lerp_update,
};
