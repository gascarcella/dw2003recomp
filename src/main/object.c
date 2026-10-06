#include "common.h"

#include "object.h"
#include "heap.h"

void object_set_state(Object *obj, s32 value) {
    obj->state = value;
    obj->step = 0;
    obj->substep = 0;
    obj->timer = 0;
}

void object_set_step(Object *obj, s32 value) {
    obj->step = value;
    obj->substep = 0;
    obj->timer = 0;
}

void object_set_substep(Object *obj, s32 value) {
    obj->substep = value;
    obj->timer = 0;
}

void object_set_timer(Object *obj, s32 value) {
    obj->timer = value;
}

void object_next_state(Object *obj) {
    obj->step = 0;
    obj->substep = 0;
    obj->timer = 0;
    obj->state++;
}

void object_next_step(Object *obj) {
    obj->substep = 0;
    obj->timer = 0;
    obj->step++;
}

void object_next_substep(Object *obj) {
    obj->timer = 0;
    obj->substep++;
}

void object_next_timer(Object *obj) {
    obj->timer++;
}

#ifdef PC_PORT
/* PC_PORT: FINDINGS 9c: the PS1's object_destroy stops every non-zero word of the data block as an object. About a
 * dozen data blocks mix other fields with the pointers, and an object kept in an s32 field (FieldstgEventData.started)
 * fills half of an 8-byte slot, so the host cannot read the block as an array of pointers. Instead it finds the
 * objects in it: object_live() says whether a value is a live object, which every word the PS1 stops is (stopping
 * anything else would call set_state through garbage there). */
#include <stddef.h>
#include <string.h>

void object_destroy(Object *obj);

/* `p` as an object that is alive: in the heap, its block's header neither free (0) nor the terminator (1) and linked
 * both ways, and object_create's methods in it. The reads go through memcpy: `p` may be any 4-aligned value (host heap
 * blocks are 8-aligned, heap.c's HEAP_ALIGN). Deterministic: it reads only the arena. */
static Object *object_live(const u8 *p) {
    HeapBlock block;
    u8 *back;
    void (*method)();

    if (((uintptr_t)p & 3) != 0 || p < port_heap_start + sizeof(HeapBlock) || p > port_heap_end - sizeof(Object)) {
        return NULL;
    }
    memcpy(&block, p - sizeof(HeapBlock), sizeof(block));
    if (block.state == 0 || block.state == 1 || (u8 *)block.next < p + sizeof(Object)
        || (u8 *)block.next > port_heap_end - sizeof(HeapBlock)) {
        return NULL;
    }
    memcpy(&back, (u8 *)block.next + offsetof(HeapBlock, prev), sizeof(back));
    if (back != p - sizeof(HeapBlock)) {
        return NULL;
    }
    memcpy(&method, p + offsetof(Object, set_state), sizeof(method));
    if (method != (void (*)())object_set_state) {
        return NULL;
    }
    memcpy(&method, p + offsetof(Object, destroy), sizeof(method));
    if (method != (void (*)())object_destroy) {
        return NULL;
    }
    return (Object *)p;
}

/* A 4-byte word of the block as an object: a PS1-style heap address (an object stored with PTR_TO_S32), or the low half
 * of a host pointer (an object returned through an s32, e.g. FieldstgEventDef.start: the whole pointer, since the
 * non-PIE arena lies below 4 GB). */
static Object *object_child_word(u32 w) {
    if (w == 0) {
        return NULL;
    }
    if (w >= PORT_HEAP_START_ADDR && w - PORT_HEAP_START_ADDR < (u32)(port_heap_end - port_heap_start)) {
        return object_live(HEAP_ADDR(w));
    }
    return object_live((const u8 *)(uintptr_t)w);
}
#endif

/* Destroys an object: ends the objects whose pointers are in its data block, frees the block, takes
 * the object off the list and frees it. */
void object_destroy(Object *obj) {
#ifdef PC_PORT
    /* PC_PORT: FINDINGS 9c: the block's bytes in order, as the PS1 scans its words: at each pointer-sized slot of the block
     * (where the host puts a pointer field) a live object is stopped and both halves are done; otherwise each 4-byte
     * word is tried on its own (an s32 that holds an object). A value that is not a live object is skipped where the
     * PS1 would call set_state through it (a zero, a non-pointer field, an object already destroyed). */
    u8 *data = (u8 *)obj->children;
    uintptr_t v;
    u32 w;
    Object *child;
    s32 ofs;

    if (obj->data_size != 0) {
        for (ofs = 0; ofs + 4 <= obj->data_size; ofs += 4) {
            if (ofs % (s32)sizeof(v) == 0 && ofs + (s32)sizeof(v) <= obj->data_size) {
                memcpy(&v, data + ofs, sizeof(v));
                if (v != 0 && (child = object_live((const u8 *)v)) != NULL) {
                    heap_objects.stop(child);
                    ofs += (s32)sizeof(v) - 4; /* the pointer's other half, at -m64 (none at -m32) */
                    continue;
                }
            }
            memcpy(&w, data + ofs, sizeof(w));
            if ((child = object_child_word(w)) != NULL) {
                heap_objects.stop(child);
            }
        }
        heap_funcs.free(obj->children);
    }
#else
    s32 i;
    Object **words;

    if (obj->child_count != 0) {
        words = (Object **)obj->children;
        for (i = 0; i < obj->child_count; i++) {
            if (words[i] != NULL) {
                heap_objects.stop(words[i]);
            }
        }
        heap_funcs.free(obj->children);
    }
#endif
    heap_objects.remove(obj);
    heap_funcs.free(obj);
}

/* Creates an object of `size` bytes with a zeroed data block of `data_size` bytes (children, child_count words),
 * and puts it on the object list when `kind` is non-zero. */
void *object_create(void (*func)(), s32 size, s32 data_size, s32 kind) {
    Object *obj = heap_funcs.alloc_zero(size, 2);

    if (data_size != 0) {
        obj->children = heap_funcs.alloc_zero(data_size, 2);
#ifndef PC_PORT
        obj->child_count = data_size / 4;
#else
        /* The block is run as pointers by heap_run_children; the callers size it in sizeof(T *) units. */
        obj->child_count = data_size / sizeof(void *);
        obj->data_size = data_size; /* PC_PORT: FINDINGS 9c: object_destroy scans the block's bytes */
#endif
    }
    obj->set_state = object_set_state;
    obj->set_step = object_set_step;
    obj->set_substep = object_set_substep;
    obj->set_timer = object_set_timer;
    obj->next_state = object_next_state;
    obj->next_step = object_next_step;
    obj->next_substep = object_next_substep;
    obj->next_timer = object_next_timer;
    obj->update = func;
    obj->destroy = object_destroy;
    if (kind != 0) {
        obj->kind = kind;
        heap_objects.add(obj);
    }
    return obj;
}

void *object_new(void (*func)(), s32 size, s32 data_size) {
    return object_create(func, size, data_size, 0);
}
