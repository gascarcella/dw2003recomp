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

/* WFIGHTTS's menu 2 (wfightts_main_update's menu of camera entries). */

void wfightts_camera_menu_update();

s32 wfightts_camera_menu_column = 0;           /* wfightts_camera_menu_update's column */
s32 wfightts_camera_menu_cursors[2] = { 0, 0 }; /* its cursor per column */
extern const u8 wfightts_camera_menu_text_0[16]; /* the table's first string (its padding bytes are not 0) */
u8 *wfightts_camera_menu_text[12] = { /* its first column's text */
    (u8 *)wfightts_camera_menu_text_0,
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4\x81\x7C\x82\x51",
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4\x81\x7C\x82\x52",
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4\x81\x7C\x82\x53",
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4\x81\x7C\x82\x54",
    "\x82\xBD\x82\xBD\x82\xA9\x82\xA4\x81\x7C\x82\x55",
    "\x83\x74\x83\x46\x83\x43\x83\x58\x83\x41\x83\x62\x83\x76",
    "\x82\x72\x83\x74\x83\x46\x83\x43\x83\x58\x83\x41\x83\x62\x83\x76",
    "\x83\x5F\x83\x81\x81\x5B\x83\x57",
    "\x83\x4C\x83\x81\x83\x7C\x81\x5B\x83\x59",
    "\x82\xA9\x82\xA2\x82\xD3\x82\xAD",
    "\x83\x66\x83\x62\x83\x68",
};
/* the table's first string, as an array: the bytes after its NUL are the non-zero fill psylink left at
 * the end of the original object's .rodata, so a string literal cannot reproduce them */
const u8 wfightts_camera_menu_text_0[16] = { 0x82, 0xBD, 0x82, 0xBD, 0x82, 0xA9, 0x82, 0xA4, 0x81, 0x7C, 0x82, 0x50, 0x00, 0x00, 0x62, 0x10 };
u8 *wfightts_camera_menu_text_2[3] = { /* its second column's text (entries 8..10 of the first) */
    "\x83\x5F\x83\x81\x81\x5B\x83\x57",
    "\x83\x4C\x83\x81\x83\x7C\x81\x5B\x83\x59",
    "\x82\xA9\x82\xA2\x82\xD3\x82\xAD",
};

/* A two-column menu (data block: 12 + 3 windows): left/right pick the column, up/down the entry, 0x2000 picks,
 * 0x4000 cancels; the cursor's window blinks. */
void wfightts_camera_menu_update(WfighttsColumnMenu *obj, MessageWindow **windows) {
    s32 i;
    s32 j;
    s32 k;
    s32 pressed;
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        for (i = 0; i < 12; i++) {
            windows[i] = message_create_window(0x1005, 1, 180, 40 + i * 12);
        }
        for (i = 0; i < 3; i++) {
            windows[12 + i] = message_create_window(0x1005, 1, 20, 40 + i * 12);
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class C, block placement; DECISIONS "LOOP_BLOCK audit"): the original places the column blocks out
         * of line, before case 1's code. */
        LOOP_BLOCK(
            pressed = pad_state.get_pressed(0) | pad_state.get_repeat(0);
            if (pad_state.get_held(0) & 0x800) {
                step = 10;
            } else {
                step = 1;
            }
            if (pressed & 0x20) {
                wfightts_camera_menu_column = 0;
                break;
            }
            if (pressed & 0x80) {
                wfightts_camera_menu_column = 1;
                break;
            }
            if (pressed & 0x10) {
                for (j = 0; j < step; j++) {
                    if (wfightts_camera_menu_cursors[wfightts_camera_menu_column] != 0) {
                        wfightts_camera_menu_cursors[wfightts_camera_menu_column]--;
                    }
                }
            } else if (pressed & 0x40) {
                for (j = 0; j < step; j++) {
                    if (wfightts_camera_menu_cursors[wfightts_camera_menu_column] != (wfightts_camera_menu_column != 0 ? 2 : 11)) {
                        wfightts_camera_menu_cursors[wfightts_camera_menu_column]++;
                    }
                }
            } else {
                if (pressed & 0x2000) {
                    *obj->column = wfightts_camera_menu_column;
                    *obj->result = wfightts_camera_menu_cursors[wfightts_camera_menu_column];
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
                if (pressed & 0x4000) {
                    *obj->column = -1;
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
        );
        for (k = 0; k < 12; k++) {
            if (wfightts_camera_menu_column == 0 && wfightts_camera_menu_cursors[0] == k && (gfx_module.funcs.get_time() & 8)) {
                windows[k]->set_visible(windows[k], 0);
            } else {
                windows[k]->set_visible(windows[k], 1);
                windows[k]->copy_text(windows[k], wfightts_camera_menu_text[k]);
            }
        }
        for (k = 0; k < 3; k++) {
            if (wfightts_camera_menu_column == 1 && wfightts_camera_menu_cursors[1] == k && (gfx_module.funcs.get_time() & 8)) {
                windows[12 + k]->set_visible(windows[12 + k], 0);
            } else {
                windows[12 + k]->set_visible(windows[12 + k], 1);
                windows[12 + k]->copy_text(windows[12 + k], wfightts_camera_menu_text_2[k]);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WfighttsColumnMenu *wfightts_camera_menu_create(s32 *arg0, s32 *result) {
    WfighttsColumnMenu *obj = object_new(wfightts_camera_menu_update, sizeof(WfighttsColumnMenu), 15 * sizeof(MessageWindow *));

    obj->column = arg0;
    obj->result = result;
    *result = 0;
    return obj;
}
