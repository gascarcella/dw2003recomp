#ifndef OVERLAY_H
#define OVERLAY_H

#include "common.h"
#include "object.h"

/* overlay_module: this module's state and its function table, which the overlays call through. */
typedef struct OverlayModule {
    /* 0x0 */ s32 stage;          /* stage overlay loaded at main_overlay_base (index into overlay_files) */
    /* 0x4 */ s32 file;           /* file ID loaded at main_file_base, or -1 */
    /* 0x8 */ void (*load_stage)(void); /* overlay_load_stage */
    /* 0xC */ void (*load_file)(s32); /* overlay_load_file */
} OverlayModule; /* size 0x10 */

extern OverlayModule overlay_module;

/* Creates the object that runs the current stage (overlay_run_object). */
Object *overlay_create_object(void);

#endif /* OVERLAY_H */
