#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "cdload.h"
#include "pad.h"
#include "memcard.h"
#include "message.h"
#include "gamestate.h"
#include "psyq/libapi.h"
#include "psyq/libc2.h"

/* SHOCKTST.PRO: a debug test of the controller's vibration ("しんどうテスト"). It reads a pattern table from
 * the development PC (sim:C:\DEVELOP\DLSKDATA.TXT), converts it to DLSKDATA.BIN, and lets the tester edit and
 * play the steps on the two motors (pad_set_actuator). */

void shocktst_update_root();
void shocktst_update_editor();
void shocktst_update_loader();

/* A step of a motor's pattern. */
typedef struct ShockStep {
    /* 0x0 */ u8 time;  /* time (pad_set_actuator) */
    /* 0x1 */ u8 level; /* level */
} ShockStep; /* size 0x2 */

/* Object of shocktst_update_editor: the pattern editor. */
typedef struct ShockTest {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 *file;    /* file 0xBE */
    /* 0x54 */ s32 layer_id;  /* first argument of message_create_window */
    /* 0x58 */ s32 cursor_column; /* cursor column (motor) */
    /* 0x5C */ s32 cursor_row; /* cursor row */
    /* 0x60 */ s32 step_playing; /* step being played */
    /* 0x64 */ s32 time_left[2]; /* per motor: time left */
    /* 0x6C */ s32 state[2];  /* per motor: 0 = start, 1 = playing, -1 = done */
    /* 0x74 */ s32 step_shown; /* step shown */
    /* 0x78 */ s32 step_count; /* number of steps */
    /* 0x7C */ ShockStep *steps[2];  /* per motor: the steps */
    /* 0x84 */ u8 unk_84[0x4];
} ShockTest; /* size 0x88 */

/* Data of the pattern editor: its windows. */
typedef struct ShockTestData {
    /* 0x00 */ MessageWindow *step_number; /* step number */
    /* 0x04 */ MessageWindow *unk_04;
    /* 0x08 */ MessageWindow *names[2];  /* per motor: its name */
    /* 0x10 */ MessageWindow *times[2];  /* per motor: the step's time */
    /* 0x18 */ MessageWindow *levels[2]; /* per motor: the step's level */
    /* 0x20 */ MessageWindow *run;       /* "run the pattern" */
} ShockTestData; /* size 0x24 */

/* A row of the editor's cursor table shocktst_cursor_rows. */
typedef struct ShockRow {
    /* 0x00 */ s32 selectable[2]; /* per column: can be selected */
    /* 0x08 */ s32 highlight[4]; /* per column: highlight (shocktst_highlight_window); then the same once chosen */
} ShockRow; /* size 0x18 */

/* Object of shocktst_update_loader: loads and converts the pattern file. */
typedef struct ShockLoader {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *table;  /* the converted table (DLSKDATA.BIN) */
    /* 0x54 */ u8 *text;    /* the text file (DLSKDATA.TXT) */
    /* 0x58 */ u8 unk_58[0x4];
} ShockLoader; /* size 0x5C */

/* Data of the loader. */
typedef struct ShockLoaderData {
    /* 0x0 */ MessageWindow *title;
    /* 0x4 */ MessageWindow *stop_hint;
    /* 0x8 */ MessageWindow *back_hint;
    /* 0xC */ ShockTest *editor;
} ShockLoaderData; /* size 0x10 */

ShockRow shocktst_cursor_rows[4] = {
    { { 1, 0 }, { 1, 0, 0, 0 } },
    { { 1, 1 }, { 2, 3, 6, 7 } },
    { { 1, 1 }, { 4, 5, 8, 9 } },
    { { 1, 0 }, { 10, 0, 11, 0 } },
};

/* texts of the step and value windows */
u8 shocktst_labels[3][0x40] = {
    "\xC3\xB1\xE8\xE3\x01\x07\x02\x05\x01",
    "\xC7\xDE\xE8\xD2\x01\x07\x02\x05\x01",
    "\x65\x89\x56\x01\x07\x02\x05\x01",
};

ShockLoader *shocktst_create_loader(void);

void shocktst_update_root(Object *obj, ShockLoader **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0x5000);
        gfx_module.funcs.init_display(0x140, 0xF0, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x140;
        rect.h = 0xF0;
        layer = gfx_module.funcs.create_layer(&rect, 1, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = shocktst_create_loader();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *shocktst_start(void) {
    return object_new(shocktst_update_root, sizeof(Object), 4);
}

/* Highlights the window `sel` (shocktst_cursor_rows) and resets the others. */
void shocktst_highlight_window(ShockTest *obj, ShockTestData *data, s32 sel) {
    s32 i;

    data->step_number->set_palette(data->step_number, 0);
    for (i = 0; i < 2; i++) {
        data->times[i]->set_palette(data->times[i], 0);
        data->levels[i]->set_palette(data->levels[i], 0);
    }
    data->run->set_palette(data->run, 0);
    switch (sel) {
    case 1:
    default:
        data->step_number->set_palette(data->step_number, 3);
        break;
    case 2:
        data->times[0]->set_palette(data->times[0], 3);
        break;
    case 3:
        data->times[1]->set_palette(data->times[1], 3);
        break;
    case 4:
        data->levels[0]->set_palette(data->levels[0], 3);
        break;
    case 5:
        data->levels[1]->set_palette(data->levels[1], 3);
        break;
    case 10:
        data->run->set_palette(data->run, 3);
        break;
    case 0:
        data->step_number->set_palette(data->step_number, 1);
        break;
    case 6:
        data->times[0]->set_palette(data->times[0], 1);
        break;
    case 7:
        data->times[1]->set_palette(data->times[1], 1);
        break;
    case 8:
        data->levels[0]->set_palette(data->levels[0], 1);
        break;
    case 9:
        data->levels[1]->set_palette(data->levels[1], 1);
        break;
    case 11:
        data->run->set_palette(data->run, 1);
        break;
    }
}

void shocktst_show_step(ShockTest *obj, ShockTestData *data, s32 step);

/* Chooses the step shown. Returns 1 on button 13, -1 on button 14, else 0. */
s32 shocktst_choose_step(ShockTest *obj, ShockTestData *data) {
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        if (--obj->step_shown < 0) {
            obj->step_shown = 0;
        }
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        if (++obj->step_shown > obj->step_count - 1) {
            obj->step_shown = obj->step_count - 1;
        }
    }
    data->step_number->set_line_number(data->step_number, 1, obj->step_shown);
    shocktst_show_step(obj, data, obj->step_shown);
    if (PAD_PRESSED(13)) {
        return 1;
    }
    if (PAD_PRESSED(14)) {
        return -1;
    }
    return 0;
}

/* Shows step `step`: its number and both motors' values. */
void shocktst_show_step(ShockTest *obj, ShockTestData *data, s32 step) {
    s32 i;

    data->step_number->set_line_number(data->step_number, 1, step);
    for (i = 0; i < 2; i++) {
        data->levels[i]->set_line_number(data->levels[i], 1, obj->steps[i][step].level);
        data->times[i]->set_line_number(data->times[i], 1, obj->steps[i][step].time);
    }
}

/* Shows step `step` while it plays: the time left instead of the step's time. */
void shocktst_show_playing_step(ShockTest *obj, ShockTestData *data, s32 step) {
    s32 i;

    for (i = 0; i < 2; i++) {
        data->levels[i]->set_line_number(data->levels[i], 1, obj->steps[i][step].level);
        data->times[i]->set_line_number(data->times[i], 1, obj->time_left[i]);
    }
}

/* Plays step `step` on both motors. Returns 1 when both are done (or on button 14). */
s32 shocktst_play_step(ShockTest *obj, ShockTestData *data, s32 step) {
    s32 i;

    for (i = 0; i < 2; i++) {
        if (obj->state[i] == 0) {
            if (obj->steps[i][step].level != 0 && obj->steps[i][step].time != 0) {
                pad_state.set_actuator(0, i, obj->steps[i][step].time, obj->steps[i][step].level);
                obj->time_left[i] = obj->steps[i][step].time;
                obj->state[i] = 1;
            } else {
                obj->state[i] = -1;
            }
            data->levels[i]->set_line_number(data->levels[i], 1, obj->steps[i][step].level);
            data->times[i]->set_line_number(data->times[i], 1, obj->steps[i][step].time);
        } else if (obj->state[i] == 1) {
            if (--obj->time_left[i] < 0) {
                obj->time_left[i] = 0;
                obj->state[i] = -1;
            }
        }
    }
    if ((obj->state[0] == -1 && obj->state[1] == obj->state[0]) || PAD_PRESSED(14)) {
        for (i = 0; i < 2; i++) {
            pad_state.set_actuator(0, i, 0, 0);
        }
        shocktst_show_step(obj, data, obj->step_shown);
        return 1;
    }
    shocktst_show_playing_step(obj, data, obj->step_shown);
    return 0;
}

/* Plays all the steps (unk_60). Returns 1 at the end. */
s32 shocktst_play_all(ShockTest *obj, ShockTestData *data) {
    do {
        shocktst_show_step(obj, data, obj->step_playing);
        if (shocktst_play_step(obj, data, obj->step_playing) == 0) {
            if (obj->state[0] != -1 || obj->state[1] != -1) {
                shocktst_show_playing_step(obj, data, obj->step_playing);
                return 0;
            }
            obj->time_left[0] = obj->time_left[1] = 0;
            obj->state[0] = obj->state[1] = 0;
            obj->step_playing++;
        } else {
            obj->time_left[0] = obj->time_left[1] = 0;
            obj->state[0] = obj->state[1] = 0;
            obj->step_playing++;
        }
    } while (obj->step_playing < obj->step_count);
    shocktst_show_step(obj, data, obj->step_shown);
    return 1;
}

/* Moves the cursor over shocktst_cursor_rows. Returns 1 (a value chosen), -1 (run chosen), 2 (button 12), else 0. */
s32 shocktst_move_cursor(ShockTest *obj, ShockTestData *data) {
    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
        do {
            if (--obj->cursor_column < 0) {
                obj->cursor_column = 1;
            }
        } while (shocktst_cursor_rows[obj->cursor_row].selectable[obj->cursor_column] == 0);
    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
        do {
            if (++obj->cursor_column >= 2) {
                obj->cursor_column = 0;
            }
        } while (shocktst_cursor_rows[obj->cursor_row].selectable[obj->cursor_column] == 0);
    }
    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
        do {
            if (--obj->cursor_row < 0) {
                obj->cursor_row = 3;
            }
        } while (shocktst_cursor_rows[obj->cursor_row].selectable[obj->cursor_column] == 0);
    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
        do {
            if (++obj->cursor_row >= 4) {
                obj->cursor_row = 0;
            }
        } while (shocktst_cursor_rows[obj->cursor_row].selectable[obj->cursor_column] == 0);
    }
    if (PAD_PRESSED(13)) {
        shocktst_highlight_window(obj, data, shocktst_cursor_rows[obj->cursor_row].highlight[obj->cursor_column + 2]);
        if (obj->cursor_row == 3) {
            return -1;
        }
        return 1;
    }
    if (PAD_PRESSED(12)) {
        return 2;
    }
    shocktst_highlight_window(obj, data, shocktst_cursor_rows[obj->cursor_row].highlight[obj->cursor_column]);
    return 0;
}

/* Edits *value: buttons 4/6 add/subtract 1 (10 when repeating), or toggle it between 0 and 1. Returns 1 on button 13,
 * -1 on button 14, else 0. */
s32 shocktst_edit_value(ShockTest *obj, ShockTestData *data, MessageWindow **windows, u8 *value, u8 toggle) {
    if (toggle) {
        if (PAD_PRESSED(4) || PAD_PRESSED(6)) {
            *value = 1 - *value;
        }
    } else if (PAD_PRESSED(4)) {
        *value += 1;
    } else if (PAD_REPEAT(4)) {
        *value += 10;
    } else if (PAD_PRESSED(6)) {
        *value -= 1;
    } else if (PAD_REPEAT(6)) {
        *value -= 10;
    }
    shocktst_show_step(obj, data, obj->step_shown);
    if (PAD_PRESSED(13)) {
        return 1;
    }
    if (PAD_PRESSED(14)) {
        return -1;
    }
    return 0;
}

/* Edits the value under the cursor. Returns 1 when done. */
s32 shocktst_edit(ShockTest *obj, ShockTestData *data) {
    switch (obj->cursor_row) {
    case 0:
    default:
        if (shocktst_choose_step(obj, data)) {
            return 1;
        }
        break;
    case 1:
        obj->base.substep = obj->cursor_column;
        if (shocktst_edit_value(obj, data, data->times, &obj->steps[obj->base.substep][obj->step_shown].time, 0)) {
            return 1;
        }
        break;
    case 2:
        obj->base.substep = obj->cursor_column;
        if (obj->base.substep != 0) {
            if (shocktst_edit_value(obj, data, data->times, &obj->steps[obj->base.substep][obj->step_shown].level, 0)) {
                return 1;
            }
        } else if (shocktst_edit_value(obj, data, data->times, &obj->steps[0][obj->step_shown].level, 1)) {
            return 1;
        }
        break;
    }
    return 0;
}

/* motor names */
u8 *shocktst_motor_names[2] = { "\x82\xB1\x82\xA4\x82\xBB\x82\xAD", "\x82\xC4\x82\xA2\x82\xBB\x82\xAD" };

void shocktst_update_editor(ShockTest *obj, ShockTestData *data) {
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->layer_id = 0x1000;
        data->step_number = message_create_window(obj->layer_id, 1, 0x28, 0x3C);
        obj->file = cdload_module.files.get_file(0xBE);
        data->step_number->set_text(data->step_number, shocktst_labels[0], -1);
        data->step_number->set_line_number(data->step_number, 1, obj->step_shown);
        for (i = 0; i < 2; i++) {
            data->names[i] = message_create_window(obj->layer_id, 1, i * 100 + 60, 0x50);
            data->names[i]->copy_text(data->names[i], shocktst_motor_names[i]);
            data->times[i] = message_create_window(obj->layer_id, 1, i * 100 + 60, 0x64);
            data->times[i]->set_text(data->times[i], shocktst_labels[1], -1);
            data->times[i]->set_line_number(data->times[i], 1, obj->steps[i][0].time);
            data->levels[i] = message_create_window(obj->layer_id, 1, i * 100 + 60, 0x78);
            data->levels[i]->set_text(data->levels[i], shocktst_labels[2], -1);
            data->levels[i]->set_line_number(data->levels[i], 1, obj->steps[i][0].level);
        }
        data->run = message_create_window(obj->layer_id, 1, 0x28, 0x8C);
        /* "パターンじっこう" (run the pattern) */
        data->run->copy_text(data->run, "\x83\x70\x83\x5E\x81\x5B\x83\x93\x82\xB6\x82\xC1\x82\xB1\x82\xA4");
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        default:
            obj->base.set_step(obj, 0);
            /* fallthrough */
        case 0:
            switch (shocktst_move_cursor(obj, data)) {
            case 2:
                obj->base.set_step(obj, 4);
                break;
            case 1:
                obj->base.next_step(obj);
                break;
            case -1:
                if ((pad_state.get_held(0) >> pad_state.get_button_map(0, 10)) & 1) {
                    obj->base.set_step(obj, 2);
                } else {
                    obj->base.set_step(obj, 3);
                }
                obj->step_playing = 0;
                obj->time_left[1] = 0;
                obj->time_left[0] = 0;
                obj->state[1] = 0;
                obj->state[0] = 0;
                break;
            }
            break;
        case 1:
            if (shocktst_edit(obj, data)) {
                obj->base.set_step(obj, 0);
            }
            break;
        case 2:
            if (shocktst_play_all(obj, data)) {
                obj->base.set_step(obj, 0);
            }
            break;
        case 3:
            if (shocktst_play_step(obj, data, obj->step_shown)) {
                obj->base.set_step(obj, 0);
            }
            break;
        case 4:
            if (memcard_funcs.wait_exist(0)) {
                obj->base.set_step(obj, 0);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        heap_funcs.free(obj->steps[0]);
        heap_funcs.free(obj->steps[1]);
        for (j = 0; j < 2; j++) {
            pad_state.set_actuator(0, j, 0, 0);
        }
        break;
    }
}

/* Copies the converted table's steps into the editor. */
void shocktst_copy_steps(ShockTest *obj, s32 *table) {
    u8 *times = (u8 *)table + table[2];
    u8 *levels = (u8 *)table + table[3];
    s32 i;

    for (i = 0; i < obj->step_count; i++) {
        obj->steps[0][i].time = times[0];
        obj->steps[1][i].time = times[1];
        times += 2;
        obj->steps[0][i].level = levels[0];
        obj->steps[1][i].level = levels[1];
        levels += 2;
    }
}

ShockTest *shocktst_create_editor(s32 count) {
    ShockTest *obj = object_new(shocktst_update_editor, sizeof(ShockTest), sizeof(ShockTestData));

    obj->step_count = count;
    obj->steps[0] = heap_funcs.alloc_zero(count * 2, 2);
    obj->steps[1] = heap_funcs.alloc_zero(obj->step_count * 2, 2);
    return obj;
}

char *shocktst_data_path = "sim:C:\\DEVELOP\\DLSKDATA.TXT";

/* Converts the text table (unk_54) into unk_50 and writes it to the PC as DLSKDATA.BIN. The text: the number of
 * steps on the first line, then lines of tab-separated fields up to a '/'; lines whose third field is 1 hold a
 * step: four values, motor 0's time and level, motor 1's time and level. The table: the count, the offsets of the
 * types (s32 each), of the time pairs and of the level pairs, then those arrays. */
#ifdef NON_MATCHING
/* 89%: register allocation; the original keeps times + 2 and levels + 2 as extra pointers (loop givs): each step
 * copies a reduced register R into times (times = R; store at R - 1; R += 2), i.e. RTL `tmp = times + 2; times = tmp`
 * back to back (loop.c's basic_induction_var follows a register source into the previous insn). Not found (wip-10):
 * u8 or ShockStep pointers, post/pre-increment, times[-1], (times += 2)[-1], an index k, a next-pointer local.
 * last-rest (final): the rest follows from those two givs. With them the original has no saved register left for
 * the hoisted "\t" high part and the constant 9, so reload rematerializes them at each use (the lui a3 / li a3,9
 * before each strcspn and test); ours keeps them in s8/s7. loop.c only makes the givs when the RTL has
 * `next = times + 2; times = next` back to back with next used afterwards (`nt = times + 1; times = nt;
 * nt[-1].unk_1 = vals[2];` does it: "giv at 348 combined with giv at 340"), but then rejects them as not worth
 * while (-496 vs 115): next is a user variable, so it pays copy_cost (strength_reduce: !replaceable &&
 * REG_USERVAR_P). The original's next was a compiler temporary; every expression form tried (pre/post-increment
 * values, assignment values, casts, statement order) collapses into a single `times += 2` in cse. */
void shocktst_convert_table(ShockLoader *obj) {
    u8 vals[4];
    s32 count;
    u8 *p;
    s32 *hdr;
    s32 *types;
    ShockStep *times;
    ShockStep *levels;
    s32 type;
    s32 len;
    s32 n;
    s32 i;
    s32 fd;

    p = obj->text;
    hdr = obj->table;
    count = atoi(p);
    while (*p != '\n') {
        p++;
    }
    *hdr = count;
    hdr++;
    *hdr = 0x10;
    hdr++;
    *hdr = hdr[-1] + count * 4;
    hdr++;
    *hdr = hdr[-1] + count * 2;
    levels = times = (ShockStep *)(types = obj->table);
    p++;
    types = (s32 *)((u8 *)types + types[1]);
    levels = (ShockStep *)((u8 *)levels + ((s32 *)levels)[3]);
    times = (ShockStep *)((u8 *)times + ((s32 *)times)[2]);
    while (*p != '/') {
        p += strcspn(p, "\t");
        while (*p == '\t') {
            p++;
        }
        p += strcspn(p, "\t");
        while (*p == '\t') {
            p++;
        }
        len = strcspn(p, "\t");
        type = atoi(p);
        p += len;
        while (*p == '\t') {
            p++;
        }
        if (type == 1) {
            for (i = 0; i < 4; i++) {
                n = strcspn(p, "\t\n");
                vals[i] = atoi(p);
                p += n;
                while (*p == '\t' || *p == '\n') {
                    p++;
                }
            }
            *types++ = type;
            times->time = vals[0];
            times->level = vals[2];
            times++;
            levels->time = vals[1];
            levels->level = vals[3];
            levels++;
        } else {
            p += strcspn(p, "\n");
            while (*p == '\n') {
                p++;
            }
        }
    }
    fd = open("sim:C:\\DEVELOP\\DLSKDATA.BIN", 0x200);
    if (fd != -1) {
        write(fd, obj->table, count * 8 + 0x10);
        close(fd);
    }
}
#else
INCLUDE_ASM("asm/shocktst/nonmatchings/shocktst_80082DA0", shocktst_convert_table);
#endif

void shocktst_update_loader(ShockLoader *obj, ShockLoaderData *data) {
    s32 fd;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        data->title = message_create_window(0x1000, 0, 0x14, 0x1E);
        /* "しんどうテスト" (vibration test) */
        data->title->copy_text(data->title, "\x82\xB5\x82\xF1\x82\xC7\x82\xA4\x83\x65\x83\x58\x83\x67");
        data->stop_hint = message_create_window(0x1000, 1, 0xDC, 0xB4);
        /* "×：じっこうていし" (cross: stop) */
        data->stop_hint->copy_text(data->stop_hint, "\x81\x7E\x81\x46\x82\xB6\x82\xC1\x82\xB1\x82\xA4\x82\xC4\x82\xA2\x82\xB5");
        data->back_hint = message_create_window(0x1000, 1, 0xDC, 0xC8);
        /* "ＳＴＡＲＴ：もどる" (START: back) */
        data->back_hint->copy_text(data->back_hint, "\x82\x72\x82\x73\x82\x60\x82\x71\x82\x73\x81\x46\x82\xE0\x82\xC7\x82\xE9");
        obj->text = heap_funcs.alloc_zero(0x4000, 2);
        obj->table = heap_funcs.alloc_zero(0x4000, 2);
        if (obj->text == NULL || obj->table == NULL) {
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        fd = open(shocktst_data_path, 1);
        if (fd == -1) {
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        read(fd, obj->text, 0x4000);
        close(fd);
        shocktst_convert_table(obj);
        if (obj->table != NULL) {
            data->editor = shocktst_create_editor(*obj->table);
            shocktst_copy_steps(data->editor, obj->table);
        } else {
            data->editor = shocktst_create_editor(10);
        }
        break;
    case OBJECT_STATE_RUN:
        if (PAD_PRESSED(3)) {
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->table != NULL) {
            heap_funcs.free(obj->table);
        }
        if (obj->text != NULL) {
            heap_funcs.free(obj->text);
        }
        gamestate_data.funcs.set_next_map(0x1500, 0);
        break;
    }
}

ShockLoader *shocktst_create_loader(void) {
    return object_new(shocktst_update_loader, sizeof(ShockLoader), sizeof(ShockLoaderData));
}
