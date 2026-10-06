#ifndef MAIN_H
#define MAIN_H

#include "common.h"

/* In main's .rodata (INCLUDE_RODATA main_overlay_base): load addresses. */
extern u8 *const main_overlay_base;  /* 0x80082CB0: where the stage overlays run (overlay) */
extern u32 *const main_file_base; /* 0x800A5DE0: the boot screen TIM, later a file copied by overlay_load_file */

#endif /* MAIN_H */
