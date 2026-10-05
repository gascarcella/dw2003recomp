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

/* Destroys an object: ends the objects whose pointers are in its data block, frees the block, takes
 * the object off the list and frees it. */
void object_destroy(Object *obj) {
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
        /* The block is scanned as pointers by object_destroy; the callers size it in sizeof(T *) units. */
        obj->child_count = data_size / sizeof(void *);
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
