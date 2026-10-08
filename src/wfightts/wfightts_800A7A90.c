#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "pad.h"
#include "message.h"
#include "sound.h"
#include "gamestate.h"
#include "records.h"
#include "fightstg.h"
#include "wfightts.h"

/* WFIGHTTS's menu 1 (wfightts_main_update's menu of Digimon). */

void wfightts_digimon_menu_update();

s32 wfightts_digimon_menu_column = 0;           /* wfightts_digimon_menu_update's column */
s32 wfightts_digimon_menu_cursors[2] = { 0, 0 }; /* its cursor per column (0..13) */
s32 wfightts_digimon_menu_first_lines[2] = { 0, 0 }; /* its first entry shown per column */

/* A two-column menu of file 0x1CC's records (fightstg_models; column 1 from record 0x37), 14 lines each
 * (data block: 2 x 14 windows): left/right pick the column, up/down the entry (10 at a time while 0x8000 is held),
 * 0x2000 picks (column to *column, the record's ID to *result), 0x4000 cancels; the cursor's window blinks. */
void wfightts_digimon_menu_update(WfighttsColumnMenu *obj, WfighttsColumnWindows *data) {
    s32 pressed;
    s32 step;
    s32 c;
    s32 i;
    s32 j;
    s32 k;
    s32 n;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (c = 0; c < 2; c++) {
            for (i = 0; i < 14; i++) {
                data->windows[c][i] = message_create_window(0x1005, 1, 180 - c * 160, 40 + i * 12);
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class C, block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the pad's handling is
         * left by breaks; the original places the blocks that leave it out of line, before case 1's code. */
        LOOP_BLOCK(
            pressed = pad_state.get_pressed(0) | pad_state.get_repeat(0);
            if (pad_state.get_held(0) & 0x8000) {
                step = 10;
            } else {
                step = 1;
            }
            if (pressed & 0x20) {
                wfightts_digimon_menu_column = 0;
                break;
            }
            if (pressed & 0x80) {
                wfightts_digimon_menu_column = 1;
                break;
            }
            if (pressed & 0x10) {
                for (j = 0; j < step; j++) {
                    if (wfightts_digimon_menu_cursors[wfightts_digimon_menu_column] != 0) {
                        wfightts_digimon_menu_cursors[wfightts_digimon_menu_column]--;
                    } else if (wfightts_digimon_menu_first_lines[wfightts_digimon_menu_column] != 0) {
                        wfightts_digimon_menu_first_lines[wfightts_digimon_menu_column]--;
                    }
                }
                break;
            }
            if (pressed & 0x40) {
                for (j = 0; j < step; j++) {
                    if (wfightts_digimon_menu_cursors[wfightts_digimon_menu_column] != 13) {
                        wfightts_digimon_menu_cursors[wfightts_digimon_menu_column]++;
                    } else if (wfightts_digimon_menu_first_lines[wfightts_digimon_menu_column] !=
                               (wfightts_digimon_menu_column != 0 ? 179 : 41)) {
                        wfightts_digimon_menu_first_lines[wfightts_digimon_menu_column]++;
                    }
                }
                break;
            }
            if (pressed & 0x2000) {
                *obj->column = wfightts_digimon_menu_column;
                if (wfightts_digimon_menu_column == 0) {
                    fightstg_models.select(wfightts_digimon_menu_first_lines[0] + wfightts_digimon_menu_cursors[0]);
                } else {
                    fightstg_models.select(wfightts_digimon_menu_first_lines[1] + wfightts_digimon_menu_cursors[1] + 0x37);
                }
                *obj->result = fightstg_models.id;
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            if (pressed & 0x4000) {
                *obj->column = -1;
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
        );
        for (k = 0; k < 14; k++) {
            if (wfightts_digimon_menu_column == 0 && wfightts_digimon_menu_cursors[0] == k && (gfx_module.funcs.get_time() & 8)) {
                data->windows[0][k]->set_visible(data->windows[0][k], 0);
            } else {
                fightstg_models.select(k + wfightts_digimon_menu_first_lines[0]);
                data->windows[0][k]->set_visible(data->windows[0][k], 1);
                data->windows[0][k]->set_text(data->windows[0][k], cdload_module.files.get_file(records_language + 0x4E),
                                              fightstg_models.kind);
            }
        }
        for (n = 0; n < 14; n++) {
            /* The cursor's line: hidden while the blink is off, else selected as the other lines are. */
            if (wfightts_digimon_menu_column == 1 && wfightts_digimon_menu_cursors[1] == n) {
                if (gfx_module.funcs.get_time() & 8) {
                    data->windows[1][n]->set_visible(data->windows[1][n], 0);
                    continue;
                }
                fightstg_models.select(n + wfightts_digimon_menu_first_lines[1] + 0x37);
            } else {
                fightstg_models.select(n + wfightts_digimon_menu_first_lines[1] + 0x37);
            }
            data->windows[1][n]->set_visible(data->windows[1][n], 1);
            data->windows[1][n]->set_text(data->windows[1][n], cdload_module.files.get_file(records_language + 0x4E),
                                          fightstg_models.kind);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WfighttsColumnMenu *wfightts_digimon_menu_create(s32 *arg0, s32 *result) {
    WfighttsColumnMenu *obj = object_new(wfightts_digimon_menu_update, sizeof(WfighttsColumnMenu), sizeof(WfighttsColumnWindows));

    obj->column = arg0;
    obj->result = result;
    *result = 0;
    return obj;
}
