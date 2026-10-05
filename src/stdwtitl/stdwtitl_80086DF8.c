#include "common.h"
#include "gfx.h"
#include "cdload.h"
#include "sound.h"
#include "records.h"
#include "stdwtitl.h"

/* The stage module (stdwtitl_module): the title's pictures, fades and moving values. FIELDSTG and
 * STGMCARD end with the same module (the last four functions are the same code). */

/* Per language: the title's pictures and its sprite bank (sub-file IDs). */
typedef struct StdwtitlPictures {
    /* 0x0 */ s32 pictures; /* pictures (TIMs) */
    /* 0x4 */ s32 bank;  /* sprite bank, kept in stdwtitl_module.unk_00 */
} StdwtitlPictures; /* size 0x8 */

extern StdwtitlPictures stdwtitl_pictures[7];

/* Loads the title's pictures to VRAM. */
void stdwtitl_load_pictures(void) {
    Tim tim;

    tim_init(&tim);
    tim.set_image_pos(0x280, 0x100);
    tim.set_clut_pos(0, 0x1F0);
    tim.load_all(cdload_module.get_subfile_by_id(stdwtitl_pictures[records_language].pictures));
    stdwtitl_module.bank = stdwtitl_pictures[records_language].bank;
    tim.set_clut_pos(0, 0x1F3);
    tim.set_image_pos(0x300, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x08870004));
    tim.set_image_pos(0x340, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x08870005));
    tim.set_image_pos(0x380, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x08870006));
    tim.set_image_pos(0x3C0, 0);
    tim.load_all(cdload_module.get_subfile_by_id(0x08870007));
}

/* Starts opening (`open`) or closing; plays the matching sound (window_anim_start). */
void stdwtitl_start_fade(WindowAnim *anim, s32 open) {
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

/* Steps the fade; returns 1 once it is finished (or not running) (window_anim_update). */
s32 stdwtitl_update_fade(WindowAnim *anim) {
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

/* Starts moving `v` from `from` to `to` in `frames` frames. */
void stdwtitl_start_value(Tween *v, s32 from, s32 to, s32 frames) {
    if (from != to) {
        v->duration = frames;
        v->acc = from << 8;
        v->value = from;
        v->target = to;
        v->running = 1;
        v->step = ((to - from) << 8) / v->duration;
    }
}

/* Steps `v`; returns 1 once it reached its target (or isn't moving). */
s32 stdwtitl_step_value(Tween *v) {
    if (v->running == 0) {
        return 1;
    }
    v->acc += v->step;
    v->value = v->acc >> 8;
    if (v->step > 0) {
        if (v->value > v->target) {
            v->value = v->target;
            v->running = 0;
            return 1;
        }
    } else {
        if (v->value < v->target) {
            v->value = v->target;
            v->running = 0;
            return 1;
        }
    }
    return 0;
}

StdwtitlModule stdwtitl_module = {
    0,
    stdwtitl_load_pictures,
    stdwtitl_start_fade,
    stdwtitl_update_fade,
    stdwtitl_start_value,
    stdwtitl_step_value,
};

StdwtitlPictures stdwtitl_pictures[7] = {
    { 0x08860001, 0x08860000 },
    { 0x08A40001, 0x08A40000 },
    { 0x094D0001, 0x094D0000 },
    { 0x094D0001, 0x094D0000 },
    { 0x094D0001, 0x094D0000 },
    { 0x094D0001, 0x094D0000 },
    { 0x094D0001, 0x094D0000 },
};
