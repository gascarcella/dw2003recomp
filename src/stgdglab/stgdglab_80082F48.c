#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "stgdglab.h"

/* STGDGLAB.PRO: the root object (sets up the display and creates the main object), the fade, and the lab's
 * chart (stgdglab_menu's entry 2): one of stgdglab_funcs.charts's charts of requirements, checked against a
 * list of IDs. */

/* The chart (size 0x29C). */
struct StgdglabChart {
    /* 0x000 */ Object base;
    /* 0x050 */ StgdglabMain *main;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table entry */
    /* 0x05C */ s32 unk_5C;
    /* 0x060 */ s16 ids[44];    /* the IDs there are */
    /* 0x0B8 */ s32 id_count;   /* how many */
    /* 0x0BC */ s32 chart;      /* the chart (index into stgdglab_funcs.charts) */
    /* 0x0C0 */ s32 chart_row;  /* the chart's row */
    /* 0x0C4 */ s32 row_count;
    /* 0x0C8 */ s32 next_row;
    /* 0x0CC */ s32 rows_used;  /* rows of found in use */
    /* 0x0D0 */ s32 found[4][4][5]; /* per row and requirement: the IDs found */
    /* 0x210 */ s32 row_met[4];
    /* 0x220 */ s32 row_found[4]; /* per row: IDs found */
    /* 0x230 */ WindowAnim anims[5];
    /* 0x280 */ s32 left_arrow_shown; /* left arrow shown */
    /* 0x284 */ s32 right_arrow_shown; /* right arrow shown */
    /* 0x288 */ s32 blink_time; /* time of the last blink */
    /* 0x28C */ s32 blink;   /* blink, 0..3 */
    /* 0x290 */ s32 cursor_id; /* cursor (ID) */
    /* 0x294 */ s32 cursor_row; /* cursor (row of found) */
    /* 0x298 */ s32 unk_298;
}; /* StgdglabChart, size 0x29C */

/* Its data block (0x1C bytes): its windows. */
typedef struct StgdglabChartData {
    /* 0x00 */ MessageWindow *title;  /* title and the Digimon's name */
    /* 0x04 */ MessageWindow *page_number; /* page number */
    /* 0x08 */ MessageWindow *row_total;
    /* 0x0C */ MessageWindow *prev_label; /* previous page */
    /* 0x10 */ MessageWindow *next_label; /* next page */
    /* 0x14 */ MessageWindow *id_name; /* the ID's name */
    /* 0x18 */ MessageWindow *description; /* its description */
} StgdglabChartData; /* size 0x1C */

/* Per chart: the Digimon it leads to. */
s32 stgdglab_chart_icons[8] = { 383, 385, 384, 3, 145, 366, 373, 31 };

void stgdglab_chart_update(StgdglabChart *obj, StgdglabChartData *data);

/* The overlay's first object: sets up the display and creates the main object. */
void stgdglab_update_main(Object *obj, StgdglabMain **data) {
    RECT rect;
    GfxLayer *layer;

    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        gfx_module.reset();
        gfx_module.alloc_packet_buffers(0xF000);
        gfx_module.funcs.init_display(320, 240, 0, 0);
        rect.x = 0;
        rect.y = 0;
        rect.w = 320;
        rect.h = 240;
        layer = gfx_module.funcs.create_layer(&rect, 3, 0x1000);
        layer->set_bg_color(layer, 0, 0, 0);
        *data = stgdglab_main_create();
        obj->next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* The overlay's entry point (overlay_entries). */
Object *stgdglab_start(void) {
    return object_new(stgdglab_update_main, sizeof(Object), sizeof(StgdglabMain *));
}

void stgdglab_fade_start(Fade *obj, s32 dir, s32 frames) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = dir;
    if (dir == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / frames;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / frames);
    }
}

void stgdglab_fade_draw(Fade *obj) {
    GfxLayer *layer;
    u32 *ot;
    POLY_F4 *poly;
    DR_TPAGE *tpage;

    layer = gfx_module.funcs.get_layer(obj->layer_id);
    ot = layer->get_ot_entry(layer, obj->ot_depth);
    poly = gfx_module.funcs.get_packet();
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->r0 = poly->g0 = poly->b0 = obj->level >> 8;
    setXYWH(poly, 0, 0, 320, 256);
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void stgdglab_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step != 0) {
            obj->level += obj->step;
            if (obj->from_black == 0) {
                if (obj->level > 0xFF00) {
                    obj->level = 0xFF00;
                    obj->base.state = OBJECT_STATE_DONE;
                }
            } else if (obj->level < 0) {
                obj->level = 0;
                obj->base.state = OBJECT_STATE_DONE;
            }
        case OBJECT_STATE_DONE:
            stgdglab_fade_draw(obj);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *stgdglab_fade_create(void) {
    Fade *obj = object_new(stgdglab_fade_update, sizeof(Fade), 0);

    obj->start = stgdglab_fade_start;
    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    return obj;
}

/* Fills found[row][slot] with the requirement's IDs found in ids (and counts them in row_found[row]);
 * clears row_met[row] when fewer than the requirement's count are there. Returns 5, or 0 for a bad col. */
s32 stgdglab_chart_check(StgdglabChart *obj, s32 row, s32 col, s32 slot) {
    s32 k;
    s32 j;
    s32 table;
    s16 *id;
    s16 v;
    s32 *found;

    if ((u32)col >= 5) {
        return 0;
    }
    table = obj->chart;
    id = stgdglab_funcs.charts[table][row * 4 + col].ids;
    obj->row_found[row] = 0;
    found = obj->found[row][slot];
    for (k = 0; k < 5; k++) {
        found[k] = 0;
        v = id[k];
        if (v > 0) {
            for (j = 0; j < obj->id_count; j++) {
                if (obj->ids[j] == v) {
                    found[k] = obj->ids[j];
                    obj->row_found[row]++;
                    break;
                }
            }
        }
    }
    if (obj->row_found[row] < stgdglab_funcs.charts[table][row * 4 + col].count) {
        obj->row_met[row] = 0;
    }
    return 5;
}

/* 1 if the requirement's first ID is in ids. */
s32 stgdglab_chart_has_first(StgdglabChart *obj, s32 row, s32 col) {
    s32 k;
    s32 j;
    s16 id;
    s16 *ids;

    if ((u32)col >= 5) {
        return 0;
    }
    id = 0;
    ids = stgdglab_funcs.charts[obj->chart][row * 4 + col].ids;
    for (k = 0; k < 5; k++) {
        if (ids[k] > 0) {
            id = ids[k];
            break;
        }
    }
    if (id != 0) {
        for (j = 0; j < obj->id_count; j++) {
            if (obj->ids[j] == id) {
                return 1;
            }
        }
    }
    return 0;
}

/* How many of the row's four requirements have their first ID in ids. */
s32 stgdglab_chart_count_row(StgdglabChart *obj, u32 row) {
    s32 col;
    s32 n;

    if (row >= 4) {
        return 0;
    }
    col = 0;
    n = 0;
    for (; col < 4; col++) {
        n += stgdglab_chart_has_first(obj, row, col);
    }
    return n;
}

/* Puts the cursor on the first ID found in row 0. */
void stgdglab_chart_reset_cursor(StgdglabChart *obj) {
    obj->cursor_row = 0;
    obj->cursor_id = 0;
    while (obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id] == 0) {
        if (++obj->cursor_id >= 5) {
            obj->cursor_id = 4;
            break;
        }
    }
}

/* Draws the chart: the ID under the cursor, the cursor and arrows (blinking), the frames, the lines between the
 * IDs found (palette 1 for a complete row, 2 when every row is), then the IDs' sprites. */
void stgdglab_chart_draw(StgdglabChart *obj, StgdglabChartData *data) {
    Sprite spr;
    s32 complete;
    s32 id;
    s32 sprite;
    s32 row;
    s32 r;
    s32 col;
    s32 seen;
    s32 row2;
    s32 col2;

    complete = 0;
    if (obj->row_met[0] != 0 && obj->row_met[1] != 0 && obj->row_met[2] != 0) {
        complete = obj->row_met[3] != 0;
    }
    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->anims[1].level != 0) {
        id = obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id];
        spr.set_vram_pos(0x140, 0x100);
        spr.set_clut8_pos(0x280, 0);
        spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
        if (obj->cursor_row < 2) {
            spr.set_pivot(0, 0xC2);
            if (id != 0) {
                sprite = stgdglab_funcs.get_sprite(id);
                spr.draw(cdload_module.get_subfile_by_id(0x02C50001), sprite, 0x14, 0xAF);
            }
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x42, 0, 0xA8);
        } else {
            spr.set_pivot(0, 0x4C);
            if (id != 0) {
                sprite = stgdglab_funcs.get_sprite(id);
                spr.draw(cdload_module.get_subfile_by_id(0x02C50001), sprite, 0x14, 0x39);
            }
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x42, 0, 0x32);
        }
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x280, 0x100);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->blink_time >= 8) {
        obj->blink_time = gfx_module.funcs.get_time();
        if (++obj->blink >= 4) {
            obj->blink = 0;
        }
    }
    spr.set_palette(obj->blink);
    if (obj->anims[4].level == 0x1000) {
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x47, obj->cursor_id * 0x28 + 0x4A, obj->cursor_row * 0x2E + 0x32);
    }
    if (obj->left_arrow_shown != 0) {
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x45, 0x17, 0xC9);
    }
    if (obj->right_arrow_shown != 0) {
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x46, 0x110, 0xC9);
    }
    spr.set_palette(0);
    if (obj->anims[0].level != 0x1000) {
        spr.set_scale(obj->anims[0].level, 0x1000, 0x1000);
        spr.set_pivot(0, 0x1F);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x44, 0, 0x11);
    if (obj->anims[0].level != 0x1000) {
        spr.set_pivot(0x140, 0x1F);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x43, 0xF0, 0x11);
    if (obj->anims[4].level != 0x1000) {
        spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
        spr.set_pivot(0x44, 0x42);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    if (obj->found[obj->chart_row][0][0] != -1) {
        if (complete) {
            spr.set_palette(2);
        } else if (obj->row_met[obj->chart_row] != 0) {
            spr.set_palette(1);
        }
        for (row = 0; row < 4; row++) {
            for (col = 4, seen = 0; col >= 0; col--) {
                id = obj->found[obj->chart_row][row][col];
                if (id != 0) {
                    if (stgdglab_funcs.get_sprite(id) != -1) {
                        seen = 1;
                        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3F, col * 0x28 + 0x4A, row * 0x2E + 0x32);
                    }
                } else if (seen) {
                    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x40, col * 0x28 + 0x4A, row * 0x2E + 0x32);
                }
            }
        }
    } else {
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3F, 0x4A, 0x32);
    }
    if (complete) {
        spr.set_palette(2);
    } else {
        spr.set_palette(0);
    }
    if (obj->anims[3].level != 0x1000) {
        spr.set_scale(0x1000, obj->anims[3].level, 0x1000);
        spr.set_pivot(0x40, 0x3F);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    for (r = obj->rows_used - 2; r >= 0; r--) {
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3C, 0x3D, r * 0x2E + 0x3F);
    }
    if (obj->anims[2].level != 0x1000) {
        spr.set_scale(0x1000, obj->anims[2].level, 0x1000);
        spr.set_pivot(0x3D, 0x42);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x3D, 0x14, 0x32);
    spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x40, 0x24, 0x32);
    sprite_init(&spr);
    spr.set_vram_pos(0x140, 0x100);
    spr.set_clut8_pos(0x280, 0);
    spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
    if (obj->anims[2].level != 0x1000) {
        spr.set_scale(0x1000, obj->anims[2].level, 0x1000);
        spr.set_pivot(0x3D, 0x42);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    id = stgdglab_chart_icons[obj->chart];
    spr.draw(cdload_module.get_subfile_by_id(0x02C50001), stgdglab_funcs.get_sprite(id), 0x14, 0x32);
    if (obj->anims[4].level != 0x1000) {
        spr.set_scale(obj->anims[4].level, 0x1000, 0x1000);
        spr.set_pivot(0x44, 0x42);
    } else {
        spr.set_scale(0x1000, 0x1000, 0x1000);
    }
    if (obj->found[obj->chart_row][0][0] != -1) {
        for (row2 = 0; row2 < 4; row2++) {
            for (col2 = 4; col2 >= 0; col2--) {
                id = obj->found[obj->chart_row][row2][col2];
                if (id != 0) {
                    sprite = stgdglab_funcs.get_sprite(id);
                    if (sprite != -1) {
                        spr.draw(cdload_module.get_subfile_by_id(0x02C50001), sprite, col2 * 0x28 + 0x4A, row2 * 0x2E + 0x32);
                    }
                }
            }
        }
    } else {
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x02C50000), 0x49, 0x4A, 0x32);
    }
}

/* Moves the cursor by `dir` rows, skipping the rows with nothing found; 1 if it moved. */
s32 stgdglab_chart_move_cursor(StgdglabChart *obj, s32 dir) {
    s32 old = obj->cursor_row;
    s32 i;

    do {
        obj->cursor_row += dir;
        if (obj->cursor_row < 0) {
            obj->cursor_row = 0;
        } else if (obj->cursor_row > obj->rows_used - 1) {
            obj->cursor_row = obj->rows_used - 1;
        }
        i = 0;
        while (obj->found[obj->chart_row][obj->cursor_row][i] == 0) {
            if (++i >= 5) {
                break;
            }
        }
    } while (obj->found[obj->chart_row][obj->cursor_row][i] == 0);
    if (old != obj->cursor_row) {
        obj->cursor_id = i;
        return 1;
    }
    return 0;
}

/* The chart's object: builds the rows (state 0), runs the cursor and the row change (steps 0-8), the page
 * change (state 2, substeps), and draws. */
void stgdglab_chart_update(StgdglabChart *obj, StgdglabChartData *data) {
    s32 row;
    s32 col;
    s32 found;
    s32 slot;
    s32 n;
    s32 old_col;
    s32 old_row;
    s32 id;
    s32 msg;
    GamestateRecord *digimon;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->main->is_menu_running(obj->main) == 0) {
            return;
        }
        obj->base.next_state(obj);
        row = 0;
        obj->chart = gamestate_data.funcs.get_party_member(obj->main->member);
        obj->id_count = gamestate_data.funcs.list_forms(obj->chart, obj->ids);
        data->title = message_create_window(obj->layer_id, 1, 0x14, 0x19);
        data->page_number = message_create_window(obj->layer_id, 1, 0xFF, 0x19);
        data->row_total = message_create_window(obj->layer_id, 1, 0x101, 0x19);
        data->prev_label = message_create_window(obj->layer_id, 1, 0x27, 0xC4);
        data->prev_label->set_ot_depth(data->prev_label, 2);
        data->next_label = message_create_window(obj->layer_id, 1, 0x115, 0xC4);
        data->next_label->set_ot_depth(data->next_label, 2);
        data->id_name = message_create_window(obj->layer_id, 1, 0x14, 0x19);
        data->id_name->set_ot_depth(data->id_name, row);
        data->description = message_create_window(obj->layer_id, 1, 0x14, 0x19);
        data->description->set_ot_depth(data->description, row);
        for (; row < 4; row++) {
            obj->row_met[row] = 1;
            col = 0;
            slot = 0;
            found = 0;
            for (; col < 4; col++) {
                n = stgdglab_chart_has_first(obj, row, col);
                found += n;
                if (n != 0) {
                    stgdglab_chart_check(obj, row, col, slot);
                    slot++;
                } else if (stgdglab_funcs.charts[obj->chart][row * 4 + col].count != 0) {
                    obj->row_met[row] = 0;
                }
            }
            if (found == 0) {
                obj->found[row][0][0] = -1;
            }
        }
        obj->row_count = 4;
        obj->rows_used = stgdglab_chart_count_row(obj, obj->chart_row);
        stgdglab_chart_reset_cursor(obj);
        obj->anims[0].duration = 8;
        obj->anims[1].duration = 8;
        obj->anims[2].duration = 8;
        obj->anims[4].duration = 8;
        obj->anims[3].duration = 8;
        obj->left_arrow_shown = 0;
        obj->right_arrow_shown = 1;
        stgdglab_funcs.window_anim_start(&obj->anims[0], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[2], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
        stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
        return;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            stgdglab_funcs.window_anim_update(&obj->anims[0]);
            if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
                digimon = gamestate_data.funcs.get_record(obj->chart);
                data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0x39), 0x27);
                data->title->set_line_text(data->title, digimon->name, -1, 1);
                data->page_number->set_line_number(data->page_number, 0, obj->chart_row + 1);
                data->page_number->measure(data->page_number, 1);
                data->row_total->set_text(data->row_total, cdload_module.files.get_file(records_language + 0x39), 0x28);
                data->prev_label->set_text(data->prev_label, cdload_module.files.get_file(records_language + 0x39), 0x29);
                data->prev_label->set_visible(data->prev_label, 0);
                data->next_label->set_text(data->next_label, cdload_module.files.get_file(records_language + 0x39), 0x2A);
                stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
                obj->base.step++;
            }
            break;
        case 1:
            if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
                stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
                obj->base.step++;
            }
            break;
        case 2:
            if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
                obj->base.step++;
            }
            break;
        case 3:
            obj->next_row = obj->chart_row;
            if ((!PAD_HELD(0xB) && PAD_PRESSED(0xA)) || (!PAD_HELD(0xB) && PAD_REPEAT(0xA))) {
                if (--obj->next_row < 0) {
                    obj->next_row = 0;
                }
            } else if ((!PAD_HELD(0xA) && PAD_PRESSED(0xB)) || (!PAD_HELD(0xA) && PAD_REPEAT(0xB))) {
                if (++obj->next_row > obj->row_count - 1) {
                    obj->next_row = obj->row_count - 1;
                }
            }
            if (obj->next_row != obj->chart_row) {
                sound_module.play(0x4001B);
                data->page_number->set_line_number(data->page_number, 0, obj->next_row + 1);
                data->page_number->measure(data->page_number, 1);
                if (obj->next_row == 0) {
                    data->prev_label->set_visible(data->prev_label, 0);
                    obj->left_arrow_shown = 0;
                } else if (obj->next_row == obj->row_count - 1) {
                    data->next_label->set_visible(data->next_label, 0);
                    obj->right_arrow_shown = 0;
                } else {
                    data->prev_label->set_visible(data->prev_label, 1);
                    data->next_label->set_visible(data->next_label, 1);
                    obj->left_arrow_shown = 1;
                    obj->right_arrow_shown = 1;
                }
                obj->base.state = OBJECT_STATE_DONE;
                obj->base.substep = 0;
            } else if (PAD_PRESSED(0xE)) {
                sound_module.play(0x800450BD);
                obj->base.state = OBJECT_STATE_DONE;
                obj->base.step = 7;
                obj->base.substep = 0;
                obj->base.timer = 1;
                obj->left_arrow_shown = 0;
                obj->right_arrow_shown = 0;
                data->prev_label->set_visible(data->prev_label, 0);
                data->next_label->set_visible(data->next_label, 0);
            } else if (obj->found[obj->chart_row][0][0] != -1) {
                old_col = obj->cursor_id;
                old_row = obj->cursor_row;
                if (obj->rows_used >= 2) {
                    if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                        stgdglab_chart_move_cursor(obj, -1);
                    } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                        stgdglab_chart_move_cursor(obj, 1);
                    }
                }
                if (old_row == obj->cursor_row) {
                    if (PAD_PRESSED(7) || PAD_REPEAT(7)) {
                        do {
                            if (--obj->cursor_id < 0) {
                                obj->cursor_id = 0;
                                break;
                            }
                        } while (obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id] == 0);
                        if (obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id] == 0) {
                            obj->cursor_id = old_col;
                        }
                    } else if (PAD_PRESSED(5) || PAD_REPEAT(5)) {
                        do {
                            if (++obj->cursor_id >= 5) {
                                obj->cursor_id = 4;
                                break;
                            }
                        } while (obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id] == 0);
                        if (obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id] == 0) {
                            obj->cursor_id = old_col;
                        }
                    }
                }
                if (old_col != obj->cursor_id || old_row != obj->cursor_row) {
                    sound_module.play(0x4001B);
                } else if (PAD_PRESSED(0xD)) {
                    sound_module.play(0x4001C);
                    stgdglab_funcs.window_anim_start(&obj->anims[1], 1);
                    obj->base.step++;
                }
            }
            break;
        case 4:
            if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
                id = obj->found[obj->chart_row][obj->cursor_row][obj->cursor_id];
                msg = records_get_digimon_func(id)->name_id;
                if (id != 0) {
                    if (obj->cursor_row < 2) {
                        data->id_name->set_pos(data->id_name, 0x3A, 0xAD);
                        data->description->set_pos(data->description, 0x3A, 0xBC);
                    } else {
                        data->id_name->set_pos(data->id_name, 0x3A, 0x37);
                        data->description->set_pos(data->description, 0x3A, 0x46);
                    }
                    data->id_name->set_text(data->id_name, cdload_module.files.get_file(records_language + 0x4E), msg);
                    data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x47), msg);
                }
                obj->base.step++;
            }
            break;
        case 5:
            if (PAD_PRESSED(0xD)) {
                stgdglab_funcs.window_anim_start(&obj->anims[1], 0);
                data->id_name->set_visible(data->id_name, 0);
                data->description->set_visible(data->description, 0);
                obj->base.step++;
            }
            break;
        case 6:
            if (stgdglab_funcs.window_anim_update(&obj->anims[1])) {
                obj->base.step = 3;
            }
            break;
        case 7:
            stgdglab_funcs.window_anim_start(&obj->anims[0], 0);
            stgdglab_funcs.window_anim_start(&obj->anims[2], 0);
            data->title->set_visible(data->title, 0);
            data->page_number->set_visible(data->page_number, 0);
            data->row_total->set_visible(data->row_total, 0);
            obj->base.step++;
            break;
        case 8:
            stgdglab_funcs.window_anim_update(&obj->anims[0]);
            if (stgdglab_funcs.window_anim_update(&obj->anims[2])) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        stgdglab_chart_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.substep) {
        case 0:
        default:
            stgdglab_funcs.window_anim_start(&obj->anims[4], 0);
            obj->base.substep++;
            break;
        case 1:
            if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
                stgdglab_funcs.window_anim_start(&obj->anims[3], 0);
                obj->chart_row = obj->next_row;
                obj->base.substep++;
            }
            break;
        case 2:
            if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
                if (obj->base.timer == 0) {
                    stgdglab_chart_reset_cursor(obj);
                    stgdglab_funcs.window_anim_start(&obj->anims[3], 1);
                    stgdglab_funcs.window_anim_start(&obj->anims[4], 1);
                    obj->rows_used = stgdglab_chart_count_row(obj, obj->chart_row);
                    obj->base.substep++;
                } else {
                    obj->base.state = OBJECT_STATE_RUN;
                }
            }
            break;
        case 3:
            if (stgdglab_funcs.window_anim_update(&obj->anims[3])) {
                obj->base.substep++;
            }
            break;
        case 4:
            if (stgdglab_funcs.window_anim_update(&obj->anims[4])) {
                obj->base.state = OBJECT_STATE_RUN;
            }
            break;
        }
        stgdglab_chart_draw(obj, data);
        break;
    case OBJECT_STATE_END:
        return;
    }
}

StgdglabChart *stgdglab_chart_create(StgdglabMain *main) {
    StgdglabChart *obj = object_new(stgdglab_chart_update, sizeof(StgdglabChart), sizeof(StgdglabChartData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 2;
    obj->main = main;
    main->close_menu(main);
    return obj;
}
