#include "common.h"

#include "fieldmenu.h"
#include "records.h"
#include "object.h"
#include "gamestate.h"
#include "sound.h"
#include "gfx.h"
#include "message.h"
#include "cdload.h"
#include "pad.h"
#include "window_anim.h"

/* A window's position and message: a party member's name (fieldmenu_name_pos), the labels (fieldmenu_label_pos) and
 * the values (fieldmenu_value_pos), each y + 46 per party member. fieldmenu_create_windows matches only with the labels
 * and values as arrays of their own, fieldmenu_show_member only reading the values as fieldmenu_name_pos[6 + i]
 * (past the end of fieldmenu_name_pos), so that is how both read them. */
typedef struct FieldmenuPos {
    /* 0x0 */ s32 message; /* message number (?SSTATUS) */
    /* 0x4 */ s32 x;
    /* 0x8 */ s32 y;
} FieldmenuPos; /* size 0xC */

FieldmenuPos fieldmenu_name_pos[1] = {
    { 0x0C, 0x37, 0x13 },
};
FieldmenuPos fieldmenu_label_pos[5] = {
    { 0x01, 0x10, 0x1C },
    { 0x02, 0x10, 0x25 },
    { 0x04, 0x3D, 0x25 },
    { 0x03, 0x10, 0x2E },
    { 0x04, 0x3D, 0x2E },
};
FieldmenuPos fieldmenu_value_pos[5] = {
    { 0x0D, 0x2C, 0x1C },
    { 0x0E, 0x3B, 0x25 },
    { 0x0E, 0x5E, 0x25 },
    { 0x0E, 0x3B, 0x2E },
    { 0x0E, 0x5E, 0x2E },
};
s32 fieldmenu_value_stats[5] = { 0, 2, 3, 4, 5 };                                /* the GamestateStats.values shown by the values */
s32 fieldmenu_option_messages[2][6] = { { 6, 7, 8, 9, 10, 0 }, { 6, 7, 8, 9, 10, 11 } }; /* the options' message numbers, without / with the sixth option */
s32 fieldmenu_anim_pictures[6] = { 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22 };          /* pictures of the state-2 animation */

/* A party member's windows. */
typedef struct FieldmenuMemberWindows {
    /* 0x00 */ MessageWindow *name;    /* name */
    /* 0x04 */ MessageWindow *labels[5]; /* labels */
    /* 0x18 */ MessageWindow *values[5]; /* values */
} FieldmenuMemberWindows; /* size 0x2C */

/* fieldmenu_update's data block (object_new's third argument). */
typedef struct FieldmenuWindows {
    /* 0x00 */ MessageWindow *help;       /* "(triangle) Button: Close Status" */
    /* 0x04 */ MessageWindow *money_unit; /* "BIT" */
    /* 0x08 */ MessageWindow *money;    /* the money */
#ifdef PC_PORT
    /* 0x0C */ MessageWindow *options[7]; /* the options; [6]: the global_save mod's SAVE entry (the PS1 has 6) */
#else
    /* 0x0C */ MessageWindow *options[6]; /* the options */
#endif
    /* 0x24 */ MessageCursor *cursor;    /* the cursor */
    /* 0x28 */ FieldmenuMemberWindows members[3];
} FieldmenuWindows; /* size 0xAC */

typedef struct Fieldmenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* gfx layer (fieldmenu_create) */
    /* 0x54 */ s32 ot_depth; /* its ordering table entry (1) */
    /* 0x58 */ s32 cursor;   /* the option under the cursor */
    /* 0x5C */ s32 option_count; /* 5 + extended */
    /* 0x60 */ s32 extended; /* the sixth option is there */
    /* 0x64 */ s32 map_enabled; /* the third option (MAP) can be chosen (fieldmenu_get_map_region() >= 0) */
    /* 0x68 */ s32 anim_palette; /* state 2's animation: the palette of the picture being faded in */
    /* 0x6C */ s32 anim_time; /* gfx_module.funcs.get_time() at the last step */
    /* 0x70 */ WindowAnim panel_anims[3]; /* per party member's panel; [0] also the title, [1] the options, [2] the money */
} Fieldmenu; /* size 0xA0 */

void fieldmenu_create_windows(Fieldmenu *obj, FieldmenuWindows *data);
void fieldmenu_show_member(Fieldmenu *obj, FieldmenuWindows *data, s32 slot, s32 show);
s32 fieldmenu_get_map_region(void);
void fieldmenu_update(Fieldmenu *obj, FieldmenuWindows *data);

/* Creates the windows: the title, the options and their cursor, the money, and each party member's. */
void fieldmenu_create_windows(Fieldmenu *obj, FieldmenuWindows *data) {
    FieldmenuPos *pos;
    s32 i;
    s32 j;

    data->help = message_create_window(obj->layer_id, 1, 0x98, 0x13);
    for (j = 0; j < obj->option_count; j++) {
        data->options[j] = message_create_window(obj->layer_id, 1, 0xBD, j * 14 + 0x31);
    }
    data->cursor = message_create_cursor(obj->layer_id, 0, 0xB0, obj->cursor * 14 + 0x31);
    data->cursor->show(data->cursor, 0);
    data->money_unit = message_create_window(obj->layer_id, 3, 0x46, 0xA6);
    data->money = message_create_window(obj->layer_id, 3, 0x42, 0xA6);
    for (i = 0; i < 3; i++) {
        pos = fieldmenu_name_pos;
        data->members[i].name = message_create_window(obj->layer_id, 1, pos->x, pos->y + i * 46);
        for (j = 0; j < 5; j++) {
            pos = &fieldmenu_label_pos[j];
            data->members[i].labels[j] = message_create_window(obj->layer_id, 3, pos->x, pos->y + i * 46);
        }
        for (j = 0; j < 5; j++) {
            pos = &fieldmenu_value_pos[j];
            data->members[i].values[j] = message_create_window(obj->layer_id, 3, pos->x, pos->y + i * 46);
        }
    }
}

/* Shows party member `slot`'s name, level, HP and MP (or the empty-slot texts), or closes them. */
void fieldmenu_show_member(Fieldmenu *obj, FieldmenuWindows *data, s32 slot, s32 show) {
    GamestateStats stats;
    s32 id;
    u8 *name;
    s32 i;

    if (show) {
        id = gamestate_data.funcs.get_party_member(slot);
        if (id >= 0) {
            name = gamestate_data.funcs.get_record(id)->name;
            gamestate_data.funcs.get_stats(id, &stats);
            data->members[slot].name->set_text(data->members[slot].name, name, -1);
            for (i = 0; i < 5; i++) {
                data->members[slot].labels[i]->set_text(data->members[slot].labels[i],
                                                      cdload_module.files.get_file(records_language + 0xB0), fieldmenu_label_pos[i].message);
                data->members[slot].values[i]->set_line_number(data->members[slot].values[i], 0, stats.values[fieldmenu_value_stats[i]]);
                data->members[slot].values[i]->measure(data->members[slot].values[i], 1);
            }
        } else {
            data->members[slot].name->set_text(data->members[slot].name, cdload_module.files.get_file(records_language + 0xB0), 0xC);
            for (i = 0; i < 5; i++) {
                data->members[slot].values[i]->set_text(data->members[slot].values[i],
                                                      cdload_module.files.get_file(records_language + 0xB0), fieldmenu_name_pos[i + 6].message);
                data->members[slot].values[i]->measure(data->members[slot].values[i], 1);
            }
        }
    } else {
        data->members[slot].name->set_visible(data->members[slot].name, 0);
        for (i = 0; i < 5; i++) {
            data->members[slot].labels[i]->set_visible(data->members[slot].labels[i], 0);
            data->members[slot].values[i]->set_visible(data->members[slot].values[i], 0);
        }
    }
}

/* The MAP option's region for the current map (the field map when on map 0x1000): -1 from map 0x2D7 on (no MAP:
 * the option is greyed), 1 from 0x270, else 0. */
s32 fieldmenu_get_map_region(void) {
    s32 x = gamestate_data.funcs.get_map();

    if (x == 0x1000) {
        x = gamestate_data.field_map;
    }
    if (x >= 0x2D7) {
        return -1;
    }
    return x >= 0x270;
}

/* The object's function. State 0 sets up; state 1 runs the menu (obj->base.step: open the windows,
 * choose, close them) and draws its frames; state 2 plays the picture animation, then changes the map. */
void fieldmenu_update(Fieldmenu *obj, FieldmenuWindows *data) {
    Sprite gfx;
    Sprite gfx2;
    s32 cursor;
    s32 done;
    s32 level;
    s32 i;
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->panel_anims[0].duration = obj->panel_anims[1].duration = obj->panel_anims[2].duration = 10;
        window_anim_start(&obj->panel_anims[0], 1);
        window_anim_start(&obj->panel_anims[1], 1);
        window_anim_start(&obj->panel_anims[2], 1);
        if (gamestate_data.items[0x192] != 0) {
            D_8005CCF0.extended = 1;
            obj->extended = 1;
        } else {
            D_8005CCF0.extended = 0;
        }
        obj->option_count = obj->extended + 5;
#ifdef PC_PORT
        if (port_mod_global_save != 0 && gamestate_data.funcs.get_map() != 0x1000) {
            obj->option_count++; /* global_save: a SAVE entry last, in the field's menu only (docs/LAUNCHER.md) */
        }
#endif
        if (fieldmenu_get_map_region() >= 0) {
            obj->map_enabled = 1;
        } else {
            obj->map_enabled = 0;
        }
        fieldmenu_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (window_anim_update(&obj->panel_anims[0])) {
                fieldmenu_show_member(obj, data, 0, 1);
                data->help->set_text(data->help, cdload_module.files.get_file(records_language + 0xB0), 0x13);
                sound_module.play(0x40019);
                obj->base.step++;
            }
            break;
        case 1:
            if (window_anim_update(&obj->panel_anims[1])) {
                fieldmenu_show_member(obj, data, 1, 1);
                for (obj->base.timer = 0; obj->base.timer < obj->option_count; obj->base.timer++) {
#ifdef PC_PORT
                    if (obj->base.timer == obj->extended + 5) {
                        /* global_save's SAVE entry: the save screen's title (?SMEMCRD entry 1, in every language) */
                        data->options[obj->base.timer]->set_text(data->options[obj->base.timer],
                                                                cdload_module.files.get_file(records_language + 0x78), 1);
                        continue;
                    }
#endif
                    data->options[obj->base.timer]->set_text(data->options[obj->base.timer],
                                                            cdload_module.files.get_file(records_language + 0xB0),
                                                            fieldmenu_option_messages[obj->extended][obj->base.timer]);
                }
                sound_module.play(0x40019);
                if (obj->map_enabled == 0) {
                    data->options[2]->set_palette(data->options[2], 7);
                }
                obj->base.step++;
            }
            break;
        case 2:
            if (window_anim_update(&obj->panel_anims[2])) {
                fieldmenu_show_member(obj, data, 2, 1);
                data->money_unit->set_text(data->money_unit, cdload_module.files.get_file(records_language + 0xB0), 5);
                data->money->set_line_number(data->money, 0, gamestate_data.money);
                data->money->measure(data->money, 1);
                data->cursor->show(data->cursor, 1);
                obj->base.step++;
            }
            break;
        case 3:
            cursor = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor > obj->option_count - 1) {
                    obj->cursor = obj->option_count - 1;
                }
            }
            if (cursor != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0xB0, obj->cursor * 14 + 0x31);
                break;
            }
            done = 0;
            if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                if (obj->map_enabled == 0 && obj->cursor == 2) {
                    break;
                }
                done = 1;
                if (gamestate_data.funcs.get_map() == 0x1000) {
                    obj->base.substep = 1;
                } else {
                    obj->base.substep = 0;
                }
                D_8005CCF0.option = obj->cursor;
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                done = 1;
                if (gamestate_data.funcs.get_map() == 0x1000) {
                    obj->base.substep = 0;
                } else {
                    obj->base.substep = 1;
                }
            }
            if (done) {
                window_anim_start(&obj->panel_anims[0], 0);
                window_anim_start(&obj->panel_anims[1], 0);
                window_anim_start(&obj->panel_anims[2], 0);
                fieldmenu_show_member(obj, data, 2, 0);
                data->money_unit->set_visible(data->money_unit, 0);
                data->money->set_visible(data->money, 0);
                data->cursor->show(data->cursor, 0);
                obj->base.step++;
            }
            break;
        case 4:
            if (window_anim_update(&obj->panel_anims[2])) {
                fieldmenu_show_member(obj, data, 1, 0);
                for (obj->base.timer = 0; obj->base.timer < obj->option_count; obj->base.timer++) {
                    data->options[obj->base.timer]->set_visible(data->options[obj->base.timer], 0);
                }
                sound_module.play(0x4001A);
                obj->base.step++;
            }
            break;
        case 5:
            if (window_anim_update(&obj->panel_anims[1])) {
                fieldmenu_show_member(obj, data, 0, 0);
                data->help->set_visible(data->help, 0);
                sound_module.play(0x4001A);
                obj->base.step++;
            }
            break;
        case 6:
            if (window_anim_update(&obj->panel_anims[0])) {
                if (obj->base.substep == 0) {
                    obj->base.set_state(obj, OBJECT_STATE_DONE);
                    obj->anim_time = gfx_module.funcs.get_time();
                } else {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                }
            }
            break;
        }
        sprite_init(&gfx);
        gfx.set_vram_pos(0x140, 0);
        gfx.set_layer_id(obj->layer_id, obj->ot_depth);
        gfx.set_follow_scroll(0);
        for (i = 0; i < 3; i++) {
            level = obj->panel_anims[i].level;
            if (level != 0) {
                if (level != 0x1000) {
                    gfx.set_scale(level, 0x1000, 0x1000);
                    gfx.set_pivot(0, i * 46 + 0x25);
                } else {
                    gfx.set_scale(0x1000, 0x1000, 0x1000);
                }
                gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x15, 0, i * 46 + 0x11);
                gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x17, 0, i * 46 + 0x11);
            }
        }
        if (obj->panel_anims[0].level != 0) {
            if (obj->panel_anims[0].level != 0x1000) {
                gfx.set_scale(obj->panel_anims[0].level, 0x1000, 0x1000);
                gfx.set_pivot(0x140, 0x19);
            } else {
                gfx.set_scale(0x1000, 0x1000, 0x1000);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x18, 0x22, 0xD);
        }
        if (obj->panel_anims[1].level != 0) {
            if (obj->panel_anims[1].level != 0x1000) {
                gfx.set_scale(obj->panel_anims[1].level, 0x1000, 0x1000);
                gfx.set_pivot(0x140, 0x52);
            } else {
                gfx.set_scale(0x1000, 0x1000, 0x1000);
            }
#ifdef PC_PORT
            if (obj->option_count == 7) {
                /* global_save's entry with the card case's: the bank has panels for 5 and 6 entries only (a 22 px top,
                 * 14 px rows, a 20 px bottom), so the 6-entry one (98 px) is stretched to 112 px from its top */
                gfx.set_scale(obj->panel_anims[1].level, 0x1249, 0x1000);
                gfx.set_pivot(0x140, 0x28);
                gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x1B, 0xA8, 0x28);
            } else {
                gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x1C - (obj->option_count - 5), 0xA8, 0x28);
            }
#else
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x1C - obj->extended, 0xA8, 0x28);
#endif
        }
        if (obj->panel_anims[2].level != 0) {
            if (obj->panel_anims[2].level != 0x1000) {
                gfx.set_scale(obj->panel_anims[2].level, 0x1000, 0x1000);
                gfx.set_pivot(0, 0xA8);
            } else {
                gfx.set_scale(0x1000, 0x1000, 0x1000);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x1A, 0, 0x9E);
        }
        break;
    case OBJECT_STATE_DONE:
        switch (obj->base.step) {
        default:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            /* fallthrough */
        case 0:
        case 1:
        case 2:
            if (gfx_module.funcs.get_time() - obj->anim_time >= 2) {
                obj->anim_time = gfx_module.funcs.get_time();
                if (++obj->anim_palette >= 8) {
                    if (++obj->base.step == 3) {
                        obj->anim_palette = 8;
                    } else {
                        obj->anim_palette = 0;
                    }
                }
            }
            break;
        case 3:
        case 4:
        case 5:
            if (gfx_module.funcs.get_time() - obj->anim_time >= 2) {
                obj->anim_time = gfx_module.funcs.get_time();
                if (++obj->anim_palette >= 16) {
                    if (++obj->base.step == 6) {
                        obj->anim_palette = 15;
                    } else {
                        obj->anim_palette = 8;
                    }
                }
            }
            break;
        case 6:
#ifdef PC_PORT
            if (obj->option_count == obj->extended + 6 && obj->cursor == obj->extended + 5) {
                port_global_save_open(); /* global_save's SAVE: STGMCARD in save mode instead of STSTATUS */
                obj->base.step++;
                break;
            }
#endif
            if (gamestate_data.funcs.get_map() == 0x1000) {
                gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
            } else {
                gamestate_data.funcs.set_next_map(0x1000, 0);
                D_8005CCF0.option = obj->cursor;
            }
            obj->base.step++;
            break;
        case 7:
            break;
        }
        sprite_init(&gfx2);
        gfx2.set_vram_pos(0x140, 0);
        gfx2.set_layer_id(obj->layer_id, obj->ot_depth);
        gfx2.set_follow_scroll(0);
        for (j = 0; j <= obj->base.step; j++) {
            if (j == 6) {
                break;
            }
            if (j == obj->base.step) {
                gfx2.set_palette(obj->anim_palette);
            } else if (j < 3) {
                gfx2.set_palette(7);
            } else {
                gfx2.set_palette(15);
            }
            gfx2.draw(cdload_module.get_subfile_by_id(0x2860000), fieldmenu_anim_pictures[j], 0, 0);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fieldmenu *fieldmenu_create(s32 layer, s32 option) {
    Fieldmenu *obj = object_new(fieldmenu_update, sizeof(Fieldmenu), sizeof(FieldmenuWindows));

    obj->layer_id = layer;
    obj->ot_depth = 1;
    obj->cursor = option;
    return obj;
}
