#ifndef HEAP_H
#define HEAP_H

#include "common.h"
#include "object.h"

/* Header in front of every heap block; the caller's pointer points just past it. The blocks form
 * a list from heap_funcs.first to a terminator block (state 1) just below heap_funcs.end. */
typedef struct HeapBlock {
    /* 0x0 */ struct HeapBlock *prev;
    /* 0x4 */ struct HeapBlock *next;
    /* 0x8 */ s32 state; /* 0 = free, 1 = terminator, else the owner's tag */
} HeapBlock;

/* heap_funcs: the heap's state and its function table, which game code calls through. Callers
 * must go through the struct: GCC 2.8 assumes a struct field and a scalar global never alias,
 * which changes instruction order. */
typedef struct HeapFuncs {
    /* 0x00 */ u32 size;
    /* 0x04 */ HeapBlock *first;
    /* 0x08 */ HeapBlock *end;
    /* 0x0C */ void (*init)(void);                          /* heap_init */
    /* 0x10 */ void (*free)(void *ptr);                     /* heap_free */
    /* 0x14 */ void (*free_state)(s32 state);               /* heap_free_state */
    /* 0x18 */ void *(*alloc)(s32 size, s32 state);         /* heap_alloc */
    /* 0x1C */ void *(*alloc_top)(s32 size, s32 state);     /* heap_alloc_top */
    /* 0x20 */ void *(*alloc_zero)(s32 size, s32 state);    /* heap_alloc_zero */
    /* 0x24 */ void (*bzero)(void *dst, s32 size);          /* heap_bzero */
    /* 0x28 */ void (*memset)(u8 *dst, u8 value, s32 count);  /* heap_memset */
    /* 0x2C */ void (*set_state)(void *ptr, s32 arg1);      /* heap_set_state */
    /* 0x30 */ void (*nop)(void);                           /* heap_nop (empty) */
} HeapFuncs; /* size 0x34 */

extern HeapFuncs heap_funcs;

/* heap_objects: a list of up to 100 objects (those object_create gives a kind), the filter (Object.kind, key1,
 * key2; -1 = any) and cursor of the search in heap_find_next_object, and the list's function table. */
typedef struct HeapObjects {
    /* 0x000 */ Object *objects[100];
    /* 0x190 */ s32 filter[3];
    /* 0x19C */ s32 cursor;
    /* 0x1A0 */ void (*clear)(void);                                  /* heap_clear_objects */
    /* 0x1A4 */ void (*add)(Object *obj);                        /* heap_add_object */
    /* 0x1A8 */ void (*remove)(Object *obj);                     /* heap_remove_object */
    /* 0x1AC */ Object *(*find)(s32 kind, s32 key1, s32 key2);    /* heap_find_object */
    /* 0x1B0 */ Object *(*find_next)(void);                      /* heap_find_next_object */
    /* 0x1B4 */ void (*run_children)(Object *obj);               /* heap_run_children */
    /* 0x1B8 */ Object *(*try_run)(Object *obj);            /* heap_try_run_object */
    /* 0x1BC */ void (*stop)(Object *obj);                       /* heap_stop_object */
} HeapObjects; /* size 0x1C0 */

extern HeapObjects heap_objects;

#endif /* HEAP_H */
