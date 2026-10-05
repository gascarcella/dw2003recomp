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

/* WFIGHTTS's menu 5 (wfightts_main_update's menu of scripts). */

void wfightts_script_menu_update();

extern const u8 wfightts_script_name_0[16]; /* the table's first string (its padding bytes are not 0) */
u8 *wfightts_script_names[20] = { /* entry names */
    (u8 *)wfightts_script_name_0,
    "\x83\x45\x83\x46\x83\x43\x83\x67",
    "\x82\x72\x83\x5F\x83\x81\x81\x5B\x83\x57",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57",
    "\x83\x66\x83\x62\x83\x68",
    "\x83\x4B\x81\x5B\x83\x68",
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A",
    "\x83\x71\x83\x62\x83\x54\x83\x63",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x46\x82\x52",
    "\x83\x41\x83\x43\x83\x65\x83\x80\x81\x46\x83\x5F",
    "\x83\x74\x83\x42\x81\x5B\x83\x8B\x83\x68\x81\x46\x83\x52",
    "\x83\x58\x83\x65\x81\x5B\x83\x5E\x83\x58\x81\x46\x83\x5F",
    "\x83\x58\x83\x65\x81\x5B\x83\x5E\x83\x58\x81\x46\x83\x5F\x83\x41",
    "\x82\x6F\x82\x71\x82\x6E\x81\x46\x82\xC7\x82\xAD",
};
/* the table's first string, as an array: the bytes after its NUL are the non-zero fill psylink left at
 * the end of the original object's .rodata, so a string literal cannot reproduce them */
const u8 wfightts_script_name_0[16] = { 0x81, 0x7C, 0x82, 0x6D, 0x82, 0x6E, 0x82, 0x6D, 0x82, 0x64, 0x81, 0x7C, 0x00, 0x00, 0x84, 0x8E };
WfighttsMenuState wfightts_script_menu_state; /* the menu's state */

/* wfightts_technique_menu_update's menu for the entries flagged in each side's Digimon's script record (data block:
 * 2 x 14 windows); a column whose record has none lists entry 0. */
void wfightts_script_menu_update(WfighttsScriptMenu *obj, WfighttsColumnWindows *data) {
    FightstgSlots *models;
    WfighttsScriptColumn *col;
    s32 c;
    s32 i;
    s32 j;
    s32 k;
    s32 id;
    s32 *flags;
    s32 pressed;
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        models = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        for (c = 0; c < 2; c++) {
            col = &obj->columns[c];
            id = models->get_model_id(models, c * 16);
            if (wfightts_script_menu_state.models[c] != id) {
                wfightts_script_menu_state.models[c] = id;
                wfightts_script_menu_state.cursors[c] = 0;
                wfightts_script_menu_state.first_lines[c] = 0;
            }
            fightstg_models.get(id);
            if (fightstg_models.record->script_file != 0) {
                flags = cdload_module.get_subfile_by_id(fightstg_models.record->script_file);
                if (flags[0] == 0) {
                    col->entries[0] = 0;
                    col->count = 1;
                    col->lines = 1;
                } else {
                    col->count = 0;
                    for (k = 0; k < 19; k++) {
                        if (flags[k] != 0) {
                            col->entries[col->count] = k + 1;
                            col->count++;
                        }
                    }
                }
                col->lines = col->count < 15 ? col->count : 14;
            } else {
                col->entries[0] = 0;
                col->count = 1;
                col->lines = 1;
            }
            for (k = 0; k < col->lines; k++) {
                data->windows[c][k] = message_create_window(0x1005, 1, 180 - c * 160, 40 + k * 12);
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class C, block placement; DECISIONS "LOOP_BLOCK audit"): the original places the column and cancel
         * blocks out of line, before case 1's code. */
        LOOP_BLOCK(
            pressed = pad_state.get_pressed(0) | pad_state.get_repeat(0);
            if (pad_state.get_held(0) & 0x8000) {
                step = 10;
            } else {
                step = 1;
            }
            if (pressed & 0x20) {
                wfightts_script_menu_state.column = 0;
                break;
            }
            if (pressed & 0x80) {
                wfightts_script_menu_state.column = 1;
                break;
            }
            if (pressed & 0x10) {
                for (j = 0; j < step; j++) {
                    if (wfightts_script_menu_state.cursors[wfightts_script_menu_state.column] != 0) {
                        wfightts_script_menu_state.cursors[wfightts_script_menu_state.column]--;
                    } else if (wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column] != 0) {
                        wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column]--;
                    }
                }
            } else if (pressed & 0x40) {
                for (j = 0; j < step; j++) {
                    if (wfightts_script_menu_state.cursors[wfightts_script_menu_state.column] !=
                        obj->columns[wfightts_script_menu_state.column].lines - 1) {
                        wfightts_script_menu_state.cursors[wfightts_script_menu_state.column]++;
                    } else if (wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column] !=
                               obj->columns[wfightts_script_menu_state.column].count -
                                   obj->columns[wfightts_script_menu_state.column].lines) {
                        wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column]++;
                    }
                }
            } else if (pressed & 0x4000) {
                *obj->column = -1;
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            } else if (pressed & 0x2000) {
                if (obj->columns[wfightts_script_menu_state.column]
                        .entries[wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column] +
                                wfightts_script_menu_state.cursors[wfightts_script_menu_state.column]] != 0) {
                    *obj->column = wfightts_script_menu_state.column;
                    *obj->result = obj->columns[wfightts_script_menu_state.column]
                                       .entries[wfightts_script_menu_state.first_lines[wfightts_script_menu_state.column] +
                                               wfightts_script_menu_state.cursors[wfightts_script_menu_state.column]] - 1;
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
        );
        for (c = 0; c < 2; c++) {
            for (i = 0; i < obj->columns[c].lines; i++) {
                if (wfightts_script_menu_state.column == c && wfightts_script_menu_state.cursors[c] == i &&
                    (gfx_module.funcs.get_time() & 8)) {
                    data->windows[c][i]->set_visible(data->windows[c][i], 0);
                } else {
                    data->windows[c][i]->set_visible(data->windows[c][i], 1);
                    data->windows[c][i]->copy_text(data->windows[c][i],
                                           wfightts_script_names[obj->columns[c].entries[i + wfightts_script_menu_state.first_lines[c]]]);
                }
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WfighttsScriptMenu *wfightts_script_menu_create(s32 *arg0, s32 *arg1) {
    WfighttsScriptMenu *obj = object_create(wfightts_script_menu_update, sizeof(WfighttsScriptMenu), sizeof(WfighttsColumnWindows), 0xFFFF);

    obj->column = arg0;
    obj->result = arg1;
    return obj;
}
