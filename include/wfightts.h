#ifndef WFIGHTTS_H
#define WFIGHTTS_H

/* WFIGHTTS (the test battle overlay, file 0x209): types and functions its files share. Six files: the main
 * object (wfightts_800A67B8.c) and one per menu it opens (config/wfightts.yaml). */

#include "common.h"
#include "object.h"
#include "message.h"

/* The objects of wfightts_digimon_menu_update and wfightts_camera_menu_update: two-column menus. They write the column
 * chosen (or -1) to *column and the entry to *result. */
typedef struct WfighttsColumnMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *column; /* the creator's results */
    /* 0x54 */ s32 *result; /* (cleared on creation) */
} WfighttsColumnMenu; /* size 0x58 */

/* The data block of the two-column menus (wfightts_digimon_menu_update, 800A8798, 800A8D34; the size their creators
 * pass): 14 windows per column. */
typedef struct WfighttsColumnWindows {
    /* 0x00 */ MessageWindow *windows[2][14];
} WfighttsColumnWindows; /* size 0x70 */

/* The state of the two-column menus wfightts_technique_menu_update (wfightts_technique_menu_state) and wfightts_script_menu_update
 * (wfightts_script_menu_state): kept between battles while the side's Digimon is the same. */
typedef struct WfighttsMenuState {
    /* 0x00 */ s32 models[2]; /* per column: the Digimon (model ID) listed */
    /* 0x08 */ s32 column;    /* the column */
    /* 0x0C */ s32 cursors[2]; /* per column: the cursor */
    /* 0x14 */ s32 first_lines[2]; /* per column: the first line shown */
} WfighttsMenuState; /* size 0x1C */

/* wfightts_stage_menu_update's object: a list of 55 entries (the stages, wfightts_stage_names) shown 14 at a time;
 * writes the one chosen (from 1) or -1 to *result. */
typedef struct WfighttsStageMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *result; /* the creator's result */
} WfighttsStageMenu; /* size 0x54 */

/* A column of wfightts_technique_menu_update's menu: the techniques the side's Digimon has. */
typedef struct WfighttsTechniqueColumn {
    /* 0x00 */ s32 techniques[62]; /* technique indices */
    /* 0xF8 */ s32 count;      /* their count */
    /* 0xFC */ s32 lines;      /* lines shown (at most 14) */
} WfighttsTechniqueColumn; /* size 0x100 */

/* wfightts_technique_menu_update's object: a two-column technique menu (one column per side). */
typedef struct WfighttsTechniqueMenu {
    /* 0x000 */ Object base;
    /* 0x050 */ WfighttsTechniqueColumn columns[2];
    /* 0x250 */ s32 *column;  /* the creator's results: the column chosen (or -1) */
    /* 0x254 */ s32 *result;  /* the technique chosen (from 1) */
} WfighttsTechniqueMenu; /* size 0x258 */

/* A column of wfightts_script_menu_update's menu: the entries (from 1) the side's Digimon's script record flags. */
typedef struct WfighttsScriptColumn {
    /* 0x00 */ s32 entries[19]; /* entries (0: none) */
    /* 0x4C */ s32 count;      /* their count */
    /* 0x50 */ s32 lines;      /* lines shown (at most 14) */
} WfighttsScriptColumn; /* size 0x54 */

/* wfightts_script_menu_update's object: a two-column menu (one column per side). */
typedef struct WfighttsScriptMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ WfighttsScriptColumn columns[2];
    /* 0xF8 */ s32 *column; /* the creator's results: the column chosen (or -1) */
    /* 0xFC */ s32 *result; /* the entry chosen (from 0) */
} WfighttsScriptMenu; /* size 0x100 */

/* The menus' creators (the main object, wfightts_main_update, opens them). */
WfighttsColumnMenu *wfightts_digimon_menu_create(s32 *arg0, s32 *result);
WfighttsColumnMenu *wfightts_camera_menu_create(s32 *arg0, s32 *result);
WfighttsStageMenu *wfightts_stage_menu_create(s32 *arg0);
WfighttsTechniqueMenu *wfightts_technique_menu_create(s32 *arg0, s32 *arg1);
WfighttsScriptMenu *wfightts_script_menu_create(s32 *arg0, s32 *arg1);

#endif /* WFIGHTTS_H */
