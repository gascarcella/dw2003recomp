#include "common.h"

#include "cdload.h"
#include "filetable.h"
#include "overlay.h"
#include "gamestate.h"
#include "main.h"
#include "object.h"
#include "psyq/libc2.h"

/* The tier-1 overlays share one address (0x80082CB0), so their entry points are plain addresses. */
#define OVERLAY_ENTRY(addr) ((s32 (*)(void))(addr))

/* Per stage (gamestate_data.funcs.get_map() >> 8): the overlay's entry point. */
s32 (*overlay_entries[])(void) = {
    NULL,                       /*  0 */
    NULL,                       /*  1 */
    OVERLAY_ENTRY(0x80087580),  /*  2 FIELDSTG */
    OVERLAY_ENTRY(0x80087580),  /*  3 FIELDSTG */
    OVERLAY_ENTRY(0x800832E8),  /*  4 STCRDDEK */
    OVERLAY_ENTRY(0x80083118),  /*  5 STPLNMET */
    OVERLAY_ENTRY(0x80087160),  /*  6 FIGHTSTG */
    OVERLAY_ENTRY(0x80095E2C),  /*  7 CARDGAME */
    OVERLAY_ENTRY(0x80084410),  /*  8 SOUNDTST */
    OVERLAY_ENTRY(0x80082E98),  /*  9 SHOCKTST */
    OVERLAY_ENTRY(0x80083AC0),  /* 10 STGTRAIN */
    OVERLAY_ENTRY(0x80083084),  /* 11 STDGNAME */
    OVERLAY_ENTRY(0x80082DC8),  /* 12 STGMCARD */
    OVERLAY_ENTRY(0x80083040),  /* 13 STGDGLAB */
    OVERLAY_ENTRY(0x80083028),  /* 14 STDWTITL */
    OVERLAY_ENTRY(0x80083318),  /* 15 STITSHOP */
    OVERLAY_ENTRY(0x80083654),  /* 16 STSTATUS */
    NULL,                       /* 17 */
    OVERLAY_ENTRY(0x80083BD0),  /* 18 STCRDABM */
    OVERLAY_ENTRY(0x80087CD4),  /* 19 STCRDSHP */
    OVERLAY_ENTRY(0x80082F6C),  /* 20 STFGTREP */
    OVERLAY_ENTRY(0x80084A30),  /* 21 STAGSLCT */
    OVERLAY_ENTRY(0x80082E8C),  /* 22 CNTY_SEL */
};

/* Per stage: the overlay's file ID. */
s32 overlay_files[] = {
    0,                          /*  0 */
    0,                          /*  1 */
    0x166,                      /*  2 FIELDSTG.PRO */
    0x166,                      /*  3 FIELDSTG.PRO */
    0x1CA,                      /*  4 STCRDDEK.PRO */
    0x206,                      /*  5 STPLNMET.PRO */
    0x167,                      /*  6 FIGHTSTG.PRO */
    0x161,                      /*  7 CARDGAME.PRO */
    0x169,                      /*  8 SOUNDTST.PRO */
    0x168,                      /*  9 SHOCKTST.PRO */
    0x1F2,                      /* 10 STGTRAIN.PRO */
    0x162,                      /* 11 STDGNAME.PRO */
    0x1EE,                      /* 12 STGMCARD.PRO */
    0x1EC,                      /* 13 STGDGLAB.PRO */
    0x1D0,                      /* 14 STDWTITL.PRO */
    0x205,                      /* 15 STITSHOP.PRO */
    0x207,                      /* 16 STSTATUS.PRO */
    0,                          /* 17 */
    0x185,                      /* 18 STCRDABM.PRO */
    0x1CE,                      /* 19 STCRDSHP.PRO */
    0x1D1,                      /* 20 STFGTREP.PRO */
    0x184,                      /* 21 STAGSLCT.PRO */
    0x165,                      /* 22 CNTY_SEL.PRO */
};

void overlay_run_object(Object *obj, s32 *result);

/* The object that runs the current stage: loads its overlay (overlay_load_stage) and calls the
 * overlay's entry point, whose result goes to *result. */
void overlay_run_object(Object *obj, s32 *result) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        overlay_module.load_stage();
        *result = overlay_entries[gamestate_data.funcs.get_map() >> 8]();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (gamestate_data.funcs.is_map_changing()) {
            obj->set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *overlay_create_object(void) {
    return object_new(overlay_run_object, 0x50, 4);
}

/* Loads the current stage's overlay (overlay_files) to main_overlay_base, unless it is already there. */
void overlay_load_stage(void) {
    s32 stage = gamestate_data.funcs.get_map() >> 8;
    u8 *src;

    if (overlay_module.stage != stage) {
        overlay_module.stage = stage;
        overlay_module.file = -1;
        src = cdload_module.files.get_file(overlay_files[stage]);
        memcpy(main_overlay_base, src, filetable_funcs.get_sectors(overlay_files[stage]) << 11);
    }
}

/* Copies file `id` (already loaded by cdload) to main_file_base, unless it is already there. */
void overlay_load_file(s32 id) {
    u8 *src;

    if (overlay_module.file != id) {
        overlay_module.file = id;
        src = cdload_module.files.get_file(id);
        memcpy(main_file_base, src, filetable_funcs.get_sectors(id) << 11);
    }
}

OverlayModule overlay_module = { 0, 0, overlay_load_stage, overlay_load_file };
