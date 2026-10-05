#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"

/* Object.state: every update function switches on it. object_create leaves it 0; the update function sets itself
 * up and moves on (next_state) to 1; an object that has finished its job goes to 2 (its owner polls for it) and
 * whoever owns it sets 3 (set_state(obj, 3), heap_stop_object): heap_run_object then destroys it. */
#define OBJECT_STATE_INIT 0
#define OBJECT_STATE_RUN 1
#define OBJECT_STATE_DONE 2
#define OBJECT_STATE_END 3

/* The header of every object (object_new / object_create); each object type puts its own fields after it, from
 * 0x50 (`Object base;` as its first member). heap_run_object runs an object once per frame: update(obj, children)
 * unless it is paused, then the objects in its children block, which it stops when it is destroyed.
 * The step counters form four levels below `state`: setting or advancing a level (the methods at 0x28) clears the
 * levels below it. */
typedef struct Object {
    /* 0x00 */ s32 kind;  /* object_create's last argument; non-zero puts the object on heap_objects' list */
    /* 0x04 */ s32 key1;  /* kind, key1 and key2: what heap_find_object matches (-1: any); key1/key2 per type */
    /* 0x08 */ s32 key2;  /*   (FIELDSTG actors: ID and type; kind 0x17: x and y; FIGHTSTG models: record and side) */
    /* 0x0C */ s32 state; /* OBJECT_STATE_*: 0 init, 1 run, 2 done, 3 end */
    /* 0x10 */ s32 step;    /* the type's own steps inside a state */
    /* 0x14 */ s32 substep; /* steps inside a step */
    /* 0x18 */ s32 timer;   /* the innermost level: mostly a frame timer, sometimes a fourth step */
    /* 0x1C */ s32 paused;  /* in state 1, non-zero skips update and the children (a positive value becomes -1) */
    /* 0x20 */ s32 child_count; /* words in children */
    /* 0x24 */ void **children; /* the data block (zeroed, object_create's data_size): update's second argument,
                                 * a struct of child object pointers that run after the object (heap_run_children) */
    /* 0x28 */ void (*set_state)();    /* object_set_state: sets state, clears step, substep and timer */
    /* 0x2C */ void (*set_step)();     /* object_set_step */
    /* 0x30 */ void (*set_substep)();  /* object_set_substep */
    /* 0x34 */ void (*set_timer)();    /* object_set_timer */
    /* 0x38 */ void (*next_state)();   /* object_next_state: state + 1, clears the levels below */
    /* 0x3C */ void (*next_step)();    /* object_next_step */
    /* 0x40 */ void (*next_substep)(); /* object_next_substep */
    /* 0x44 */ void (*next_timer)();   /* object_next_timer */
    /* 0x48 */ void (*update)();       /* the type's update function, first argument of object_new */
    /* 0x4C */ void (*destroy)();      /* object_destroy */
} Object; /* size 0x50 */

void *object_new(void (*func)(), s32 size, s32 data_size);
void *object_create(void (*func)(), s32 size, s32 data_size, s32 kind);

#endif /* OBJECT_H */
