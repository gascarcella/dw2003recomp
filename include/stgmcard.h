#ifndef STGMCARD_H
#define STGMCARD_H

/* STGMCARD.PRO (the memory card screen): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "overlay_common.h"

/* stgmcard_module: the stage module's state (the last file) and its function table, which the other
 * file calls through. The same module, with other contents, ends STDWTITL and FIELDSTG. */
typedef struct StgmcardModule {
    /* 0x00 */ s32 cursor_slot; /* the slot the cursor starts on (the header's last_slot) */
    /* 0x04 */ s32 slot; /* the slot being saved (its summary is header->slots[slot]) */
    /* 0x08 */ s32 prev_slot; /* the slot the cursor moves from */
    /* 0x0C */ struct StgmcardSaveHeader *header; /* buffer of header_size bytes: the save file's header */
    /* 0x10 */ struct GamestateData *save;   /* buffer of save_size bytes: a save slot (gamestate_data's first 0x26C4 bytes) */
    /* 0x14 */ s32 header_size;
    /* 0x18 */ s32 save_size;
    /* 0x1C */ void (*init_save_header)(void);                                /* stgmcard_init_save_header */
    /* 0x20 */ s32 (*is_loading)(void);                                       /* stgmcard_is_loading */
    /* 0x24 */ void (*free_buffers)(void);                                    /* stgmcard_free_buffers */
    /* 0x28 */ void (*start_fade)(WindowAnim *anim, s32 open);               /* stgmcard_start_fade */
    /* 0x2C */ s32 (*update_fade)(WindowAnim *anim);                         /* stgmcard_update_fade */
    /* 0x30 */ void (*start_value)(Tween *v, s32 from, s32 to, s32 frames); /* stgmcard_start_value */
    /* 0x34 */ s32 (*step_value)(Tween *v);                             /* stgmcard_step_value */
} StgmcardModule; /* size 0x38 */

extern StgmcardModule stgmcard_module;

#endif /* STGMCARD_H */
