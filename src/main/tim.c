#include "common.h"

#include "cdload.h"
#include "gfx.h"
#include "heap.h"

/* $gp variable: this file is built with -G8, and ASPSX only uses $gp for variables defined in the
 * same file. It is in this object's .sbss (configure.py DATA_IN_C). */
Tim *tim_module;

void tim_set_module(Tim *obj);
void tim_set_image_pos(s32 arg0, s32 arg1);
void tim_set_clut_pos(s32 arg0, s32 arg1);
void tim_load(u32 *tim);
void tim_load_all(s32 *data);
void tim_set_buffer_size(s32 arg0);

void tim_set_module(Tim *obj) {
    tim_module = obj;
}

void tim_set_image_pos(s32 arg0, s32 arg1) {
    tim_module->image_x = arg0;
    tim_module->image_y = arg1;
}

void tim_set_clut_pos(s32 arg0, s32 arg1) {
    tim_module->clut_x = arg0;
    tim_module->clut_y = arg1;
}

/* Loads a TIM image (and its CLUT, for 4- and 8-bit images) to the VRAM positions in tim_module. */
void tim_load(u32 *tim) {
    RECT clut_rect;
    RECT rect;
    s32 flags;
    s32 has_clut;
    s32 pmode;

    tim++;
    flags = *tim++;
    has_clut = flags & 8;
    pmode = flags & 7;
    if (has_clut) {
        switch (pmode) {
            case 0:
            case 1:
                clut_rect.x = tim_module->clut_x;
                clut_rect.y = tim_module->clut_y;
                clut_rect.w = ((u16 *)tim)[4];
                clut_rect.h = ((u16 *)tim)[5];
                LoadImage(&clut_rect, tim + 3);
                break;
        }
        tim = (u32 *)((u8 *)tim + *tim);
    }
    rect.x = tim_module->image_x;
    rect.y = tim_module->image_y;
    rect.w = ((u16 *)tim)[4];
    rect.h = ((u16 *)tim)[5];
    LoadImage(&rect, tim + 3);
    tim_module->width = rect.w;
    tim_module->height = rect.h;
}

/* Loads every TIM of a container (cdload_get_subfile), each one 64 VRAM pixels right of the last;
 * an "RLEN" entry is run-length decoded into a buffer first (a byte n < 0x80: n literal bytes follow;
 * n >= 0x80: the next byte n & 0x7F times; 0 ends).
 * One variable per job: entry (call result, compared in v0), src (a1), dst (the decode cursor and the TIM passed
 * on, a0); `dst = buf` before `src += 8` gives the original's block layout. (The original calls
 * cdload_module.get_subfile without masking i to 16 bits, hence cdload.h's s32 index.) */
void tim_load_all(s32 *data) {
    u8 *buf;
    u8 *src;
    u8 *dst;
    u8 *entry;
    s32 i;
    s32 j;
    s32 n;
    u8 c;

    buf = heap_funcs.alloc(tim_module->buffer_size, 2);
    for (i = 0;; i++) {
        entry = cdload_module.get_subfile(i, data);
        if (entry == (u8 *)data) {
            break;
        }
        src = entry;
        if (*(u32 *)src != 0x4E454C52) {
            dst = src;
        } else {
            dst = buf;
            src += 8;
            while ((c = *src) != 0) {
                if (c & 0x80) {
                    n = c & 0x7F;
                    src++;
                    for (j = 0; j < n; j++) {
                        *dst++ = *src;
                    }
                    src++;
                } else {
                    n = *src++;
                    for (j = 0; j < n; j++) {
                        *dst++ = *src++;
                    }
                }
            }
            dst = buf;
        }
        tim_load((u32 *)dst);
        DrawSync(0);
        tim_module->image_x += 0x40;
    }
    heap_funcs.free(buf);
}

void tim_set_buffer_size(s32 arg0) {
    tim_module->buffer_size = arg0;
}

void tim_init(Tim *obj) {
    heap_funcs.bzero(obj, sizeof(Tim));
    obj->load = tim_load;
    obj->set_clut_pos = tim_set_clut_pos;
    obj->set_image_pos = tim_set_image_pos;
    obj->set_module = tim_set_module;
    obj->load_all = tim_load_all;
    obj->set_buffer_size = tim_set_buffer_size;
    tim_set_module(obj);
    obj->buffer_size = 0xA800;
}
