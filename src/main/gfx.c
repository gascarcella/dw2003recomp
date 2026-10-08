#include "common.h"

#include "records.h"
#include "gamestate.h"
#include "gfx.h"
#include "psyq/libetc.h"
#include "psyq/libgs.h"
#include "psyq/libsnd.h"
#include "heap.h"

/* $gp variables: this file is built with -G8, and ASPSX only uses $gp for variables defined in the
 * same file. gfx_initialized has an initializer, so it is in .sdata. */
s32 gfx_initialized = 0;
volatile s32 gfx_frame_pending; /* set by gfx_end_frame, cleared by the vsync callback */

extern s16 gfx_ot_lengths[]; /* ordering table length by depth: gfx_ot_lengths[bits - 1] = 1 << bits */

void gfx_vsync_callback(void);
void gfx_end_frame(s32 arg0);
void gfx_clear_callbacks(GfxLayer *obj);
void gfx_compact_ot(GfxLayer *obj);
GfxLayer *gfx_new_layer(DRAWENV *env, s32 bits);
void gfx_draw_layer(GfxLayer *obj);
void gfx_add_callback(GfxLayer *obj, void (*func)(void *, GfxLayer *, s32), void *data, s32 prio, s32 arg4);

/* The vsync callback. */
void gfx_vsync_callback(void) {
    if (records_60hz != 0) {
        gfx_module.time_fixed += 0x100;
        gfx_module.ticks_fixed += 0x100;
        gamestate_data.playtime_frames += 0x100;
    } else {
        gfx_module.time_fixed += 0x133;
        gfx_module.ticks_fixed += 0x133;
        gamestate_data.playtime_frames += 0x133;
    }
    if (gfx_module.vsync_callback != NULL) {
        gfx_module.vsync_callback(gfx_module.vsync_arg);
    }
    if (gfx_frame_pending != 0) {
        gfx_module.disp_buffer = !gfx_module.disp_buffer;
        PutDispEnv(&gfx_module.dispenvs[gfx_module.disp_buffer]);
    }
    SsSeqCalledTbyT();
    gfx_frame_pending = 0;
}

void gfx_set_vsync_callback(void) {
    VSyncCallback(gfx_vsync_callback);
}

/* Ends a frame: runs the layers' callbacks (if arg0), waits for the vsync, draws every layer, flips
 * the buffers, updates the frame counters and clears the layers' ordering tables. */
void gfx_end_frame(s32 arg0) {
    s32 i;
    s32 j;
    GfxLayer *obj;

    if (arg0 != 0) {
        for (j = 0; j < 30; j++) {
            obj = gfx_module.layers[j];
            if (obj != NULL) {
                if (obj->camera_saved != 0) {
                    obj->restore_camera(obj);
                }
                if (obj->world_screen_saved != 0) {
                    obj->restore_world_screen(obj);
                }
                obj->run_callbacks(obj);
            }
        }
    }
    DrawSync(0);
    gfx_frame_pending = 1;
    while (gfx_frame_pending != 0) {
        PLATFORM_WAIT();
    }
    if (gfx_module.packet != NULL) {
        for (i = 0; i < 30; i++) {
            if (gfx_module.layers[i] != NULL) {
                gfx_module.layers[i]->draw(gfx_module.layers[i]);
            }
        }
    }
    gfx_module.buffer = !gfx_module.buffer;
    if (records_60hz != 0) {
        gfx_module.frames_fixed += 0x100;
    } else {
        gfx_module.frames_fixed += 0x133;
    }
    gfx_module.frames = gfx_module.frames_fixed >> 8;
    gfx_module.time = gfx_module.time_fixed >> 8;
    gfx_module.frame_ticks = gfx_module.ticks_fixed >> 8;
    gfx_module.ticks_fixed &= 0xFF;
    gamestate_data.funcs.tick_playtime();
    gfx_module.packet = gfx_module.packet_buffers[gfx_module.buffer];
    for (i = 0; i < 30; i++) {
        if (gfx_module.layers[i] != NULL) {
            gfx_module.layers[i]->clear_ot(gfx_module.layers[i]);
        }
    }
}

s32 gfx_get_frames(void) {
    return gfx_module.frames;
}

s32 gfx_get_time(void) {
    return gfx_module.time;
}

s32 gfx_get_frame_ticks(void) {
    return gfx_module.frame_ticks;
}

/* First call: initializes the buffer index. Later calls: deletes every layer and clears the
 * buffers' state. */
void gfx_reset(void) {
    s32 i;

    if (gfx_initialized != 0) {
        for (i = 0; i < 30; i++) {
            if (gfx_module.layers[i] != NULL) {
                gfx_module.funcs.delete_layer(gfx_module.layer_ids[i]);
                i--;
            }
        }
        heap_funcs.bzero(&gfx_module.packet, OFFSETOF(GfxModule, dispenvs) - OFFSETOF(GfxModule, packet));
        return;
    }
    gfx_module.buffer = 1;
    gfx_module.disp_buffer = 0;
    gfx_initialized = 1;
}

/* Allocates the two packet buffers. */
void gfx_alloc_packet_buffers(s32 size) {
    gfx_module.packet_buffer_size = size;
    gfx_module.packet_buffers[0] = heap_funcs.alloc_top(size, 2);
    gfx_module.packet_buffers[1] = heap_funcs.alloc_top(size, 2);
    gfx_module.packet = gfx_module.packet_buffers[gfx_module.buffer];
}

void *gfx_get_packet(void) {
    return gfx_module.packet;
}

void gfx_set_packet(void *arg0) {
    gfx_module.packet = arg0;
}

void gfx_free_packet_buffers(void) {
    if (gfx_module.packet_buffers[0] != NULL) {
        heap_funcs.free(gfx_module.packet_buffers[0]);
    }
    if (gfx_module.packet_buffers[1] != NULL) {
        heap_funcs.free(gfx_module.packet_buffers[1]);
    }
    gfx_module.packet_buffers[0] = NULL;
    gfx_module.packet_buffers[1] = NULL;
}

/* Sets up the two display buffers (side by side, one above the other, or 320x480 interlaced 24-bit). */
void gfx_init_display(s32 w, s32 h, s32 arg2, s32 arg3) {
    if (arg2 != 0) {
        if (arg3 != 0) {
            SetDefDispEnv(&gfx_module.dispenvs[0], 0, 0, 320, 480);
            gfx_module.dispenvs[0].isinter = 1;
            gfx_module.dispenvs[0].isrgb24 = 1;
            SetDefDispEnv(&gfx_module.dispenvs[1], 480, 0, 320, 480);
            gfx_module.dispenvs[1].isinter = 1;
            gfx_module.dispenvs[1].isrgb24 = 1;
        } else {
            SetDefDispEnv(&gfx_module.dispenvs[0], 0, 0, w, h);
            SetDefDispEnv(&gfx_module.dispenvs[1], w, 0, w, h);
        }
    } else {
        SetDefDispEnv(&gfx_module.dispenvs[0], 0, 0, w, h);
        SetDefDispEnv(&gfx_module.dispenvs[1], 0, 256, w, h);
    }
    if (main_screen_pos != 0) {
        gfx_module.dispenvs[0].screen.y = 24;
        gfx_module.dispenvs[1].screen.y = 24;
    }
    GsInit3D();
    SetGeomOffset(0, 0);
}

/* Sets both display environments to the same area. */
void gfx_set_display_area(s32 x, s32 y, s32 w, s32 h) {
    SetDefDispEnv(&gfx_module.dispenvs[0], x, y, w, h);
    SetDefDispEnv(&gfx_module.dispenvs[1], x, y, w, h);
}

/* Returns the object with this ID from the 30 slots, or NULL. */
GfxLayer *gfx_get_layer(s32 id) {
    s32 i;

    for (i = 0; i < 30; i++) {
        if (gfx_module.layers[i] != NULL && gfx_module.layer_ids[i] == id) {
            return gfx_module.layers[i];
        }
    }
    return NULL;
}

/* Returns the slot of the layer with this ID (ID 0: the first free slot), or -1. */
s32 gfx_find_slot(s32 id) {
    s32 i;

    for (i = 0; i < 30; i++) {
        if (id != 0) {
            if (gfx_module.layers[i] != NULL && gfx_module.layer_ids[i] == id) {
                return i;
            }
        } else if (gfx_module.layers[i] == NULL) {
            return i;
        }
    }
    return -1;
}

/* Removes slot i; the later slots move down. */
void gfx_remove_slot(s32 i) {
    for (; i < 29; i++) {
        gfx_module.layers[i] = gfx_module.layers[i + 1];
        gfx_module.layer_ids[i] = gfx_module.layer_ids[i + 1];
    }
}

/* Inserts a layer at slot i; the later slots move up. */
void gfx_insert_slot(s32 i, GfxLayer *obj, s32 id) {
    s32 j;

    for (j = 29; j != i; j--) {
        gfx_module.layers[j] = gfx_module.layers[j - 1];
        gfx_module.layer_ids[j] = gfx_module.layer_ids[j - 1];
    }
    gfx_module.layers[i] = obj;
    gfx_module.layer_ids[i] = id;
}

/* Creates a layer drawing to rect, in the first free slot. */
GfxLayer *gfx_create_layer(RECT *rect, s32 arg1, s32 id) {
    DRAWENV env;
    s32 i;

    i = gfx_find_slot(0);
    if (i != -1) {
        SetDefDrawEnv(&env, rect->x, rect->y, rect->w, rect->h);
        gfx_module.layer_ids[i] = id;
        return gfx_module.layers[i] = gfx_new_layer(&env, arg1);
    }
    return NULL;
}

s32 gfx_delete_layer(s32 id) {
    s32 i;

    i = gfx_find_slot(id);
    if (i != -1) {
        gfx_module.layers[i]->free(gfx_module.layers[i]);
        heap_funcs.free(gfx_module.layers[i]);
        gfx_remove_slot(i);
        return 1;
    }
    return 0;
}

/* Moves layer id to the slot of layer id2 plus offset (drawing order). */
void gfx_move_layer(s32 id, s32 id2, s32 offset) {
    s32 i;
    s32 j;
    s32 pos;
    GfxLayer *obj;
    s32 obj_id;

    i = gfx_find_slot(id);
    j = gfx_find_slot(id2);
    if (i == -1 || j == -1) {
        return;
    }
    pos = j + offset;
    obj = gfx_module.layers[i];
    obj_id = gfx_module.layer_ids[i];
    if (pos <= 0) {
        pos = 0;
    }
    gfx_remove_slot(i);
    gfx_insert_slot(pos, obj, obj_id);
}

/* Clears the current ordering table. */
void gfx_clear_ot(GfxLayer *obj) {
    ClearOTagR(obj->ots[gfx_module.buffer], obj->ot_length);
}

/* Shortens the current ordering table's chains: an entry that links to the entry just below it
 * links directly past the run of empty entries instead. */
void gfx_compact_ot(GfxLayer *obj) {
    u32 *ot;
    u32 *p;
    u32 *q;
    u32 mask = 0xFFFFFF;

    ot = obj->ots[gfx_module.buffer];
    p = ot + obj->ot_length - 1;
    while (p != ot) {
        q = p - 1;
        if ((*p & mask) == (PTR_TO_U32(q) & mask)) {
            while ((*q & mask) == (PTR_TO_U32(q - 1) & mask)) {
                q--;
            }
            *p = PTR_TO_U32(q) & mask;
        }
        p = q;
    }
}

/* Draws the layer: links a DR_ENV for its DRAWENV (moved down 256 lines for buffer 1) at the end
 * of the current ordering table and draws the table. */
void gfx_draw_layer(GfxLayer *obj) {
    DRAWENV env;
    DR_ENV *dr;
    u32 *ot;

    env = obj->env;
    env.ofs[0] = obj->draw_x;
    env.ofs[1] = obj->draw_y;
    gfx_compact_ot(obj);
    dr = gfx_module.packet;
    ot = obj->ots[gfx_module.buffer] + obj->ot_length - 1;
    if (gfx_module.buffer != 0) {
        env.clip.y += 256;
        env.ofs[1] += 256;
    }
    SetDrawEnv(dr, &env);
    addPrim(ot, dr);
    dr++;
    gfx_module.packet = dr;
    DrawOTag(ot);
}

u32 *gfx_get_ot_entry(GfxLayer *obj, s32 z) {
    return obj->ots[gfx_module.buffer] + z;
}

u32 *gfx_get_ot_entry_z(GfxLayer *obj, s32 z) {
    z >>= 16 - obj->ot_bits;
    return obj->ots[gfx_module.buffer] + z;
}

u32 *gfx_get_ot(GfxLayer *obj) {
    return obj->ots[gfx_module.buffer];
}

s32 gfx_get_ot_bits(GfxLayer *obj) {
    return obj->ot_bits;
}

void gfx_set_bg_color(GfxLayer *obj, u8 arg1, u8 arg2, u8 arg3) {
    obj->env.r0 = arg1;
    obj->env.b0 = arg3;
    obj->env.g0 = arg2;
    if ((arg3 | (arg1 | arg2)) & 0xFF) {
        obj->env.isbg = 1;
        return;
    }
    obj->env.isbg = 0;
}

void gfx_set_draw_offset(GfxLayer *obj, u16 arg1, u16 arg2) {
    obj->draw_x = arg1;
    obj->draw_y = arg2;
}

void gfx_get_scroll(GfxLayer *obj, s32 *out) {
    out[0] = obj->scroll_x >> 8;
    out[1] = obj->scroll_y >> 8;
}

void gfx_set_scroll(GfxLayer *obj, s32 x, s32 y) {
    obj->scroll_x = x;
    obj->scroll_y = y;
}

void gfx_add_scroll(GfxLayer *obj, s32 dx, s32 dy) {
    obj->scroll_x += dx;
    obj->scroll_y += dy;
}

void gfx_set_clip_pos(GfxLayer *obj, s16 arg1, s16 arg2) {
    obj->env.clip.x = arg1;
    obj->env.clip.y = arg2;
}

void gfx_set_clip_size(GfxLayer *obj, s16 arg1, s16 arg2) {
    obj->env.clip.w = arg1;
    obj->env.clip.h = arg2;
}

void gfx_get_view_rect(GfxLayer *obj, GfxRect *out) {
    out->x = obj->env.clip.x - obj->draw_x + (obj->scroll_x >> 8);
    out->y = obj->env.clip.y - obj->draw_y + (obj->scroll_y >> 8);
    out->w = obj->env.clip.w;
    out->h = obj->env.clip.h;
}

/* Frees the layer's ordering tables and callback nodes. */
void gfx_free_layer(GfxLayer *obj) {
    DrawSync(0);
    heap_funcs.free(obj->ots[0]);
    heap_funcs.free(obj->ots[1]);
    if (obj->callback_capacity != 0) {
        heap_funcs.free(obj->callbacks);
    }
}

void gfx_clear_callbacks(GfxLayer *obj) {
    obj->callbacks->priority = 0x7FFFFFFF;
    obj->callbacks->arg = 0;
    obj->callbacks->func = NULL;
    obj->callbacks->data = 0;
    obj->callbacks->next = NULL;
    obj->callbacks_used = 1;
}

void gfx_alloc_callbacks(GfxLayer *obj, s32 count) {
    obj->callbacks = heap_funcs.alloc(count * sizeof(GfxCallback), 2);
    obj->callback_capacity = count;
    gfx_clear_callbacks(obj);
}

/* Adds a callback node, keeping the list sorted by priority, highest first. */
void gfx_add_callback(GfxLayer *obj, void (*func)(void *, GfxLayer *, s32), void *data, s32 prio, s32 arg4) {
    GfxCallback *node;
    GfxCallback *cur;
    GfxCallback *prev;
    s32 i;

    if (obj->callbacks_used < obj->callback_capacity) {
        cur = obj->callbacks;
        node = &cur[obj->callbacks_used];
        node->func = func;
        node->data = data;
        node->priority = prio;
        node->arg = arg4;
        prev = NULL;
        if (obj->callbacks_used != 0) {
            for (i = 0; i < obj->callback_capacity; i++) {
                if (cur->priority < prio) {
                    cur = prev;
                    break;
                }
                if (cur->next == NULL) {
                    break;
                }
                prev = cur;
                cur = cur->next;
            }
            if (cur->next == NULL) {
                cur->next = node;
                node->next = NULL;
            } else {
                node->next = cur->next;
                cur->next = node;
            }
        } else {
            node->next = NULL;
        }
        obj->callbacks_used++;
    }
}

/* Adds a callback node after the last one added. */
void gfx_append_callback(GfxLayer *obj, void (*func)(void *, GfxLayer *, s32), void *data) {
    GfxCallback *node;

    if (obj->callbacks_used < obj->callback_capacity) {
        node = &obj->callbacks[obj->callbacks_used];
        node->func = func;
        node->data = data;
        node->priority = 0;
        node->arg = 0;
        node->next = NULL;
        if (obj->callbacks_used != 0) {
            node[-1].next = node;
        }
        obj->callbacks_used++;
    }
}

void gfx_run_callbacks(GfxLayer *obj) {
    GfxCallback *node;

    if (obj->callbacks_used != 0) {
        node = obj->callbacks;
        do {
            if (node->func != NULL) {
                node->func(node->data, obj, node->arg);
            }
            node = node->next;
        } while (node != NULL);
        gfx_clear_callbacks(obj);
    }
}

void gfx_save_camera(GfxLayer *obj, s32 arg1, s32 h) {
    obj->camera_saved = arg1;
    if (arg1 != 0) {
        obj->projection = h;
        obj->cameras[gfx_module.buffer] = GsWSMATRIX;
    }
}

void gfx_restore_camera(GfxLayer *obj) {
    GsSetProjection(obj->projection);
    GsWSMATRIX = obj->cameras[gfx_module.buffer];
}

void gfx_save_world_screen(GfxLayer *obj, s32 arg1) {
    obj->world_screen_saved = arg1;
    if (arg1 != 0) {
        obj->world_screens[gfx_module.buffer] = GsLIGHTWSMATRIX;
    }
}

void gfx_restore_world_screen(GfxLayer *obj) {
    GsLIGHTWSMATRIX = obj->world_screens[gfx_module.buffer];
}

/* Creates a layer with this DRAWENV and two ordering tables of 2^bits entries. */
GfxLayer *gfx_new_layer(DRAWENV *env, s32 bits) {
    GfxLayer *obj;

    obj = heap_funcs.alloc_zero(sizeof(GfxLayer), 2);
    obj->env = *env;
    obj->ot_bits = bits;
    obj->ot_length = gfx_ot_lengths[bits - 1];
    obj->ots[0] = heap_funcs.alloc(obj->ot_length << 2, 2);
    obj->ots[1] = heap_funcs.alloc(obj->ot_length << 2, 2);
    ClearOTagR(obj->ots[0], obj->ot_length);
    ClearOTagR(obj->ots[1], obj->ot_length);
    obj->set_bg_color = gfx_set_bg_color;
    obj->draw = gfx_draw_layer;
    obj->clear_ot = gfx_clear_ot;
    obj->get_ot_entry = gfx_get_ot_entry;
    obj->get_ot_entry_z = gfx_get_ot_entry_z;
    obj->free = gfx_free_layer;
    obj->set_clip_pos = (void (*)(GfxLayer *, s32, s32))gfx_set_clip_pos;
    obj->set_clip_size = (void (*)(GfxLayer *, s32, s32))gfx_set_clip_size;
    obj->set_draw_offset = (void (*)(GfxLayer *, s32, s32))gfx_set_draw_offset;
    obj->set_scroll = gfx_set_scroll;
    obj->add_scroll = gfx_add_scroll;
    obj->get_scroll = gfx_get_scroll;
    obj->get_view_rect = gfx_get_view_rect;
    obj->get_ot = gfx_get_ot;
    obj->get_ot_bits = gfx_get_ot_bits;
    obj->alloc_callbacks = gfx_alloc_callbacks;
    obj->add_callback = gfx_add_callback;
    obj->append_callback = gfx_append_callback;
    obj->run_callbacks = gfx_run_callbacks;
    obj->save_camera = gfx_save_camera;
    obj->restore_camera = gfx_restore_camera;
    obj->save_world_screen = gfx_save_world_screen;
    obj->restore_world_screen = gfx_restore_world_screen;
    return obj;
}

GfxModule gfx_module = {
    .reset = gfx_reset,
    .alloc_packet_buffers = gfx_alloc_packet_buffers,
    .funcs = {
        gfx_get_packet,
        gfx_set_packet,
        gfx_free_packet_buffers,
        gfx_set_vsync_callback,
        gfx_end_frame,
        gfx_create_layer,
        gfx_delete_layer,
        gfx_init_display,
        gfx_set_display_area,
        gfx_get_layer,
        gfx_move_layer,
        gfx_get_frames,
        gfx_get_time,
        gfx_get_frame_ticks,
    },
};

s16 gfx_ot_lengths[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
