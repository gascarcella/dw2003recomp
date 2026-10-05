#ifndef INN_H
#define INN_H

#include "common.h"

/* The inn object (its fields are inn.c's own). */
typedef struct Inn Inn;

/* Creates the inn object (its windows on layer `layer`). */
Inn *inn_create(s32 layer);

struct Fade;

/* Creates a full-screen fade (overlay_common.h's Fade, the overlays' <overlay>_fade_create) on `layer`. */
struct Fade *inn_fade_create(s32 layer);

#endif /* INN_H */
