#include "common.h"
#include "cdload.h"
#include "object.h"
#include "heap.h"

extern HeapBlock *D_8005CB50; /* start of the heap (0x800AB800) */

Object *heap_find_next_object(void);
Object *heap_run_object(Object *obj);

void heap_free(void *ptr);
void heap_bzero(void *, s32);
void *heap_try_alloc(u32 size, s32 state);
void *heap_try_alloc_top(u32 size, s32 state);
void *heap_alloc(s32 size, s32 arg1);

/* The unit block sizes are rounded to. PS1: 4. Host: 8 (PC_PORT), so that every block's data is 8-aligned like its
 * pointers: the heap starts 8-aligned in the arena (0x800AB800's offset) and the header (two pointers and the state)
 * is 24 bytes. In the -m32 build (DW3_PORT_M32) the header is 12 bytes and the data 4-aligned, which is what its
 * pointers need; the sizes are rounded the same way in both builds. */
#ifndef PC_PORT
#define HEAP_ALIGN 4
#else
#define HEAP_ALIGN 8
_Static_assert(sizeof(HeapBlock) % sizeof(void *) == 0, "PC_PORT: the heap block header keeps the data pointer-aligned");
#endif

/* Frees a block and merges it with free neighbours. */
void heap_free(void *ptr) {
    HeapBlock *block = (HeapBlock *)ptr - 1;
    HeapBlock *prev;
    HeapBlock *next;

    if (ptr != NULL) {
        prev = block->prev;
        next = block->next;
        block->state = 0;
        if (next->state == 0) {
            block->next = next->next;
            next->next->prev = block;
        }
        if (prev->state == 0) {
            prev->next = block->next;
            block->next->prev = prev;
        }
    }
}

void heap_nop(void) {
}

/* Frees every block whose state is `state`; the list ends at a block with state 1. */
void heap_free_state(s32 state) {
    HeapBlock *block = heap_funcs.first;

    while (block->state != 1) {
        if (block->state == state) {
            heap_free(block + 1);
        }
        block = block->next;
    }
}

/* Makes the whole heap one free block, from D_8005CB50 up to a terminator block below 0x801FF000. */
void heap_init(void) {
    HeapBlock *first;
    HeapBlock *last;

    heap_funcs.end = HEAP_END(HeapBlock *);
    last = HEAP_END(HeapBlock *) - 1;
    first = D_8005CB50;
    heap_funcs.first = first;
    heap_funcs.size = HEAP_SIZE_FROM(first);
    first->prev = first;
    first->next = last;
    first->state = 0;
    last->prev = first;
    last->state = 1;
    last->next = heap_funcs.end;
}

/* Zeroes `size` bytes, a word at a time when `size` is a multiple of 4. */
void heap_bzero(void *dst, s32 size) {
    s32 i;

    if (size & 3) {
        s8 *p = dst;

        for (i = 0; i < size; i++) {
            *p++ = 0;
        }
    } else {
        s32 *p = dst;

        size >>= 2;
        for (i = 0; i < size; i++) {
            *p++ = 0;
        }
    }
}

void heap_memset(s8 *dst, s8 value, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        *dst++ = value;
    }
}

/* First-fit allocation: takes the first free block that is large enough and splits it when the
 * rest can hold another block. The new block gets `state`. */
void *heap_try_alloc(u32 size, s32 state) {
    HeapBlock *block;
    HeapBlock *rest;
    u32 avail;
    u32 split_min;

    size = (size + HEAP_ALIGN - 1) / HEAP_ALIGN * HEAP_ALIGN;
    split_min = size + sizeof(HeapBlock) + 8; /* split only if the rest exceeds a header + 8 bytes */
    for (block = heap_funcs.first; block->state != 1; block = block->next) {
        if (block->state == 0) {
            avail = (u8 *)block->next - (u8 *)block - sizeof(HeapBlock);
            if (avail >= size) {
                if (avail > split_min) {
                    rest = (HeapBlock *)((u8 *)block + size + sizeof(HeapBlock));
                    rest->prev = block;
                    rest->next = block->next;
                    rest->state = 0;
                    block->next->prev = rest;
                    block->next = rest;
                }
                block->state = state;
                return block + 1;
            }
        }
    }
    return NULL;
}

/* Like heap_try_alloc, but takes the last free block that is large enough and allocates from its
 * top end. */
void *heap_try_alloc_top(u32 size, s32 state) {
    HeapBlock *block;
    HeapBlock *prev;
    HeapBlock *new;
    u32 avail;

    size = (size + HEAP_ALIGN - 1) / HEAP_ALIGN * HEAP_ALIGN + sizeof(HeapBlock);
    for (block = heap_funcs.end - 1; heap_funcs.first != block; block = block->prev) {
        prev = block->prev;
        if (prev->state == 0) {
            avail = (u8 *)block - (u8 *)prev;
            if (size == avail) { /* exact fit: hand out the free block whole */
                new = prev;
                new->state = state;
                return new + 1;
            }
            if (size < avail) {
                new = (HeapBlock *)((u8 *)block - size);
                new->prev = prev;
                new->next = block;
                new->state = state;
                block->prev->next = new;
                block->prev = new;
                return new + 1;
            }
        }
    }
    return NULL;
}

/* Allocates with heap_try_alloc, running cdload_module.free_oldest until it succeeds. */
void *heap_alloc(s32 size, s32 arg1) {
    void *ptr;

    while ((ptr = heap_try_alloc(size, arg1)) == NULL) {
        cdload_module.free_oldest();
    }
    return ptr;
}

/* Same as heap_alloc with heap_try_alloc_top. */
void *heap_alloc_top(s32 size, s32 arg1) {
    void *ptr;

    while ((ptr = heap_try_alloc_top(size, arg1)) == NULL) {
        cdload_module.free_oldest();
    }
    return ptr;
}

void *heap_alloc_zero(s32 size, s32 arg1) {
    void *ptr = heap_alloc(size, arg1);

    heap_bzero(ptr, size);
    return ptr;
}

void heap_set_state(void *ptr, s32 arg1) {
    HeapBlock *block = (HeapBlock *)ptr - 1;

    if (arg1 != 0) {
        block->state = 4;
        return;
    }
    block->state = 2;
}

/* heap_memset's own code sign-extends `value` (s8) while its callers pass a u8 (pad's 0xFF). */
HeapFuncs heap_funcs = {
    0,
    NULL,
    NULL,
    heap_init,
    heap_free,
    heap_free_state,
    heap_alloc,
    heap_alloc_top,
    heap_alloc_zero,
    heap_bzero,
    (void (*)(u8 *, u8, s32))heap_memset,
    heap_set_state,
    heap_nop,
};

/* Empties heap_objects. */
void heap_clear_objects(void) {
    s32 i;

    for (i = 99; i >= 0; i--) {
        heap_objects.objects[i] = NULL;
    }
}

/* Puts `obj` in the first empty slot of heap_objects. */
void heap_add_object(Object *obj) {
    s32 i;

    for (i = 0; i < 100; i++) {
        if (heap_objects.objects[i] == NULL) {
            heap_objects.objects[i] = obj;
            return;
        }
    }
}

/* Removes `obj` from heap_objects. */
void heap_remove_object(Object *obj) {
    s32 i;

    for (i = 0; i < 100; i++) {
        if (heap_objects.objects[i] == obj) {
            heap_objects.objects[i] = NULL;
            return;
        }
    }
}

/* Returns the next object (from the cursor on) that matches the filter; -1 = any. */
Object *heap_find_next_object(void) {
    s32 i;
    Object *obj;

    for (i = heap_objects.cursor; i < 100; i++) {
        obj = heap_objects.objects[i];
        if (obj != NULL
            && (heap_objects.filter[0] == -1 || obj->kind == heap_objects.filter[0])
            && (heap_objects.filter[1] == -1 || obj->key1 == heap_objects.filter[1])
            && (heap_objects.filter[2] == -1 || obj->key2 == heap_objects.filter[2])) {
            heap_objects.cursor = i + 1;
            return heap_objects.objects[i];
        }
    }
    return NULL;
}

/* Sets the search filter and finds the first matching object in heap_objects. */
Object *heap_find_object(s32 kind, s32 key1, s32 key2) {
    heap_objects.filter[0] = kind;
    heap_objects.filter[1] = key1;
    heap_objects.filter[2] = key2;
    heap_objects.cursor = 0;
    return heap_find_next_object();
}

/* Runs one object for a frame (on the scratchpad stack): update(obj, children), unless it is paused
 * (state 1 with `paused` set: a positive value becomes -1). An object in state 3 (OBJECT_STATE_END) ends:
 * destroy(obj), returns NULL. Otherwise run_children (heap_run_children: its children) runs, except while
 * paused. Returns the object. */
Object *heap_run_object(Object *obj) {
    s32 end = obj->state == OBJECT_STATE_END;

    PORT_SCRATCHPAD_STACK_ENTER(0x1F8003FC);
    if (obj->state == OBJECT_STATE_RUN && obj->paused != 0) {
        if (obj->paused > 0) {
            obj->paused = -1;
        }
    } else {
        obj->update(obj, obj->children);
    }
    PORT_SCRATCHPAD_STACK_LEAVE();
    if (!end) {
        if (obj->state != OBJECT_STATE_RUN || obj->paused == 0) {
            heap_objects.run_children(obj);
        }
    } else {
        obj->destroy(obj);
        obj = NULL;
    }
    return obj;
}

/* Runs heap_run_object on each of `obj`'s children, keeping what it returns. */
void heap_run_children(Object *obj) {
    s32 count = obj->child_count;
    void **children = obj->children;
    s32 i;

    for (i = 0; i < count; i++) {
        if (children[i] != NULL) {
            children[i] = heap_run_object(children[i]);
        }
    }
}

Object *heap_try_run_object(Object *obj) {
    if (obj != NULL) {
        return heap_run_object(obj);
    }
    return NULL;
}

void heap_stop_object(Object *obj) {
    if (obj != NULL) {
        obj->set_state(obj, OBJECT_STATE_END);
        heap_objects.try_run(obj);
    }
}

HeapObjects heap_objects = {
    { NULL },
    { 0, 0, 0 },
    0,
    heap_clear_objects,
    heap_add_object,
    heap_remove_object,
    heap_find_object,
    heap_find_next_object,
    heap_run_children,
    heap_try_run_object,
    heap_stop_object,
};
