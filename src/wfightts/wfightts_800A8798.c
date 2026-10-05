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

/* WFIGHTTS's menu 4 (wfightts_main_update's menu of techniques). */

void wfightts_technique_menu_update();

extern u8 wfightts_technique_name_0[]; /* the table's first string (its padding bytes are not 0) */
u8 *wfightts_technique_names[62] = { /* technique names */
    wfightts_technique_name_0,
    "\x83\x45\x83\x46\x83\x43\x83\x67\x81\x7C\x82\x50",
    "\x83\x4B\x81\x5B\x83\x68",
    "\x82\x72\x83\x5F\x83\x81\x81\x5B\x83\x57",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x4F",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x50",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x51",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x52",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x53",
    "\x82\x6B\x83\x5F\x83\x81\x81\x5B\x83\x57\x81\x7C\x82\x54",
    "\x83\x66\x83\x62\x83\x68",
    "\x82\xA8\x82\xAB\x82\xA0\x82\xAA\x82\xE8",
    "\x83\x4C\x83\x81\x83\x7C\x81\x5B\x83\x59",
    "\x82\xA2\x82\xA9\x82\xE8",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x4F",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x50",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x51",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x52",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x53",
    "\x83\x5E\x83\x5E\x83\x4A\x83\x45\x81\x7C\x82\x54",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x4F",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x50",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x51",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x52",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x53",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x50\x81\x7C\x82\x54",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x4F",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x50",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x51",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x52",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x53",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x51\x81\x7C\x82\x54",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x4F",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x50",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x51",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x52",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x53",
    "\x83\x8F\x81\x46\x83\x75\x81\x46\x82\x52\x81\x7C\x82\x54",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x4F",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x50",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x51",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x52",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x53",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x45\x81\x7C\x82\x54",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x4F",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x50",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x51",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x52",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x53",
    "\x83\x8F\x81\x46\x83\x7D\x81\x46\x83\x4A\x81\x7C\x82\x54",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x4F",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x50",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x51",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x52",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x53",
    "\x83\x71\x83\x62\x83\x54\x83\x63\x81\x7C\x82\x54",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x4F",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x50",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x51",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x52",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x53",
    "\x83\x57\x83\x87\x83\x4F\x83\x8C\x83\x58\x81\x7C\x82\x54",
};
INCLUDE_RODATA("asm/wfightts/nonmatchings/wfightts_800A8798", wfightts_technique_name_0);
WfighttsMenuState wfightts_technique_menu_state; /* the menu's state */

/* Picks a technique of either side's Digimon (data block: 2 x 14 windows): left/right pick the side, up/down the
 * technique (10 at a time while 0x8000 is held), 0x2000 picks, 0x4000 cancels; the cursor's window blinks. */
void wfightts_technique_menu_update(WfighttsTechniqueMenu *obj, WfighttsColumnWindows *data) {
    FightstgSlots *models;
    s32 c;
    s32 n;
    WfighttsTechniqueColumn *col;
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
        for (n = 0; n < 2; n++) {
            col = &obj->columns[n];
            id = models->get_model_id(models, n * 16);
            if (wfightts_technique_menu_state.models[n] != id) {
                wfightts_technique_menu_state.models[n] = id;
                wfightts_technique_menu_state.cursors[n] = 0;
                wfightts_technique_menu_state.first_lines[n] = 0;
            }
            fightstg_models.get(id);
            flags = cdload_module.get_subfile_by_id(fightstg_models.record->anim_file);
            col->count = 0;
            for (k = 0; k < 62; k++) {
                if (flags[k] != 0) {
                    col->techniques[col->count] = k;
                    col->count++;
                }
            }
            col->lines = col->count < 15 ? col->count : 14;
            for (k = 0; k < col->lines; k++) {
                data->windows[n][k] = message_create_window(0x1005, 1, 180 - n * 160, 40 + k * 12);
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
                wfightts_technique_menu_state.column = 0;
                break;
            }
            if (pressed & 0x80) {
                wfightts_technique_menu_state.column = 1;
                break;
            }
            if (pressed & 0x10) {
                for (j = 0; j < step; j++) {
                    if (wfightts_technique_menu_state.cursors[wfightts_technique_menu_state.column] != 0) {
                        wfightts_technique_menu_state.cursors[wfightts_technique_menu_state.column]--;
                    } else if (wfightts_technique_menu_state.first_lines[wfightts_technique_menu_state.column] != 0) {
                        wfightts_technique_menu_state.first_lines[wfightts_technique_menu_state.column]--;
                    }
                }
            } else if (pressed & 0x40) {
                for (j = 0; j < step; j++) {
                    if (wfightts_technique_menu_state.cursors[wfightts_technique_menu_state.column] !=
                        obj->columns[wfightts_technique_menu_state.column].lines - 1) {
                        wfightts_technique_menu_state.cursors[wfightts_technique_menu_state.column]++;
                    } else if (wfightts_technique_menu_state.first_lines[wfightts_technique_menu_state.column] !=
                               obj->columns[wfightts_technique_menu_state.column].count -
                                   obj->columns[wfightts_technique_menu_state.column].lines) {
                        wfightts_technique_menu_state.first_lines[wfightts_technique_menu_state.column]++;
                    }
                }
            } else if (pressed & 0x4000) {
                *obj->column = -1;
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            } else if (pressed & 0x2000) {
                *obj->column = wfightts_technique_menu_state.column;
                *obj->result = obj->columns[wfightts_technique_menu_state.column]
                                    .techniques[wfightts_technique_menu_state.first_lines[wfightts_technique_menu_state.column] +
                                            wfightts_technique_menu_state.cursors[wfightts_technique_menu_state.column]] + 1;
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
        );
        for (c = 0; c < 2; c++) {
            for (i = 0; i < obj->columns[c].lines; i++) {
                if (wfightts_technique_menu_state.column == c && wfightts_technique_menu_state.cursors[c] == i &&
                    (gfx_module.funcs.get_time() & 8)) {
                    data->windows[c][i]->set_visible(data->windows[c][i], 0);
                } else {
                    data->windows[c][i]->set_visible(data->windows[c][i], 1);
                    data->windows[c][i]->copy_text(data->windows[c][i],
                                           wfightts_technique_names[obj->columns[c].techniques[i + wfightts_technique_menu_state.first_lines[c]]]);
                }
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WfighttsTechniqueMenu *wfightts_technique_menu_create(s32 *arg0, s32 *arg1) {
    WfighttsTechniqueMenu *obj = object_create(wfightts_technique_menu_update, sizeof(WfighttsTechniqueMenu), 0x70, 0xFFFF);

    obj->column = arg0;
    obj->result = arg1;
    return obj;
}
