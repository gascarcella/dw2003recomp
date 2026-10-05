#include "wstag.h"
#include "pad.h"

/* WSTAG924's second file: two answer-lists with a text per entry (WstagMenu; the same file as WSTAG935's
 * 0x800A60D8). A file of its own: its jump tables start 4 mod 8, right after the setup's CVECTOR. */

extern WstagFadeFuncs wstag924_funcs;
extern s32 D_WSTAG924_800A7A34[];
extern s32 D_WSTAG924_800A7A44[];
void wstag924_menu_1602_update();
void wstag924_menu_1604_update();

void wstag924_menu_1602_update(WstagMenu *obj, WstagMenuData *data) {
    Sprite spr;
    s32 prev;
    s32 i;
    s32 j;
    s32 k;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->fade.duration = 10;
        obj->text_fade.duration = 10;
        for (j = 0; j < 8; j++) {
            data->items[j] = message_create_window(0x1002, 1, 0xBD, 0x21 + j * 14);
            data->items[j]->set_ot_depth(data->items[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0xAF, 0x21);
        data->cursor->show(data->cursor, 0);
        data->text = message_create_window(0x1002, 1, 0x12, 0xB0);
        data->text->set_page_lines(data->text, 3);
        obj->count = 8;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            wstag924_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag924_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < obj->count; i++) {
                    data->items[i]->set_text(data->items[i], cdload_module.get_subfile_by_id(0x1580001 + (records_language << 16)),
                                            i + 1);
                }
                data->cursor->show(data->cursor, 1);
                obj->base.step++;
            }
            break;
        case 2:
            prev = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor > obj->count - 1) {
                    obj->cursor = obj->count - 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0xAF, obj->cursor * 14 + 0x21);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                if (obj->cursor == obj->count - 1) {
                    obj->base.step = 10;
                } else {
                    obj->base.step++;
                }
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                obj->base.step = 10;
            }
            break;
        case 3:
            data->cursor->stop(data->cursor, 1);
            data->cursor->set_palette(data->cursor, 7);
            wstag924_funcs.fade_start(&obj->text_fade, 1);
            obj->base.step++;
            break;
        case 4:
            if (wstag924_funcs.fade_update(&obj->text_fade) != 0) {
                data->text->set_text(data->text, cdload_module.get_subfile_by_id(0x1580001 + (records_language << 16)), obj->cursor + 9);
                data->text->set_speed(data->text, 6);
                obj->base.step++;
            }
            break;
        case 5:
            if (data->text->is_done(data->text) != 0) {
                obj->base.step++;
            } else if (data->text->is_waiting(data->text) != 0) {
                if (PAD_PRESSED(13)) {
                    sound_module.play(0x4001C);
                    obj->arrow = 0;
                } else {
                    obj->arrow = 1;
                }
            } else if (PAD_PRESSED(13)) {
                data->text->find_page_end(data->text);
            }
            break;
        case 6:
            wstag924_funcs.fade_start(&obj->text_fade, 0);
            data->text->set_visible(data->text, 0);
            obj->base.step++;
            break;
        case 7:
            if (wstag924_funcs.fade_update(&obj->text_fade) != 0) {
                data->cursor->stop(data->cursor, 0);
                data->cursor->set_palette(data->cursor, 0);
                obj->base.step = 2;
            }
            break;
        case 10:
            for (k = 0; k < obj->count; k++) {
                data->items[k]->set_visible(data->items[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            wstag924_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag924_funcs.fade_update(&obj->fade) != 0) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->arrow) {
            if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
                obj->arrow_time = gfx_module.funcs.get_time();
                if (++obj->arrow_frame >= 5) {
                    obj->arrow_frame = 0;
                }
            }
            spr.set_palette(obj->arrow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x124, 0xCD);
            spr.set_palette(0);
        }
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x56);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), D_WSTAG924_800A7A34[obj->count - 5], 0xA8, 0x18);
        }
        if (obj->text_fade.level != 0) {
            if (obj->text_fade.level != 0x1000) {
                spr.set_scale(obj->text_fade.level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x56);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
        wstag924_funcs.fade_start(&obj->fade, 1);
        wstag924_funcs.fade_update(&obj->fade);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag924_event_1602_start(void) {
    return object_new(wstag924_menu_1602_update, 0x84, 0x28);
}

void wstag924_menu_1604_update(WstagMenu *obj, WstagMenuData *data) {
    Sprite spr;
    s32 prev;
    s32 i;
    s32 j;
    s32 k;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->fade.duration = 10;
        obj->text_fade.duration = 10;
        for (j = 0; j < 8; j++) {
            data->items[j] = message_create_window(0x1002, 1, 0xBD, 0x21 + j * 14);
            data->items[j]->set_ot_depth(data->items[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0xAF, 0x21);
        data->cursor->show(data->cursor, 0);
        data->text = message_create_window(0x1002, 1, 0x12, 0xB0);
        data->text->set_page_lines(data->text, 3);
        obj->count = 8;
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            wstag924_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag924_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < obj->count; i++) {
                    data->items[i]->set_text(data->items[i], cdload_module.get_subfile_by_id(0x1580002 + (records_language << 16)),
                                            i + 1);
                }
                data->cursor->show(data->cursor, 1);
                obj->base.step++;
            }
            break;
        case 2:
            prev = obj->cursor;
            if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
                if (--obj->cursor < 0) {
                    obj->cursor = 0;
                }
            } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
                if (++obj->cursor > obj->count - 1) {
                    obj->cursor = obj->count - 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0xAF, obj->cursor * 14 + 0x21);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                if (obj->cursor == obj->count - 1) {
                    obj->base.step = 10;
                } else {
                    obj->base.step++;
                }
            } else if (PAD_PRESSED(14)) {
                sound_module.play(0x800450BD);
                obj->base.step = 10;
            }
            break;
        case 3:
            data->cursor->stop(data->cursor, 1);
            data->cursor->set_palette(data->cursor, 7);
            wstag924_funcs.fade_start(&obj->text_fade, 1);
            obj->base.step++;
            break;
        case 4:
            if (wstag924_funcs.fade_update(&obj->text_fade) != 0) {
                data->text->set_text(data->text, cdload_module.get_subfile_by_id(0x1580002 + (records_language << 16)), obj->cursor + 9);
                data->text->set_speed(data->text, 6);
                obj->base.step++;
            }
            break;
        case 5:
            if (data->text->is_done(data->text) != 0) {
                obj->base.step++;
            } else if (data->text->is_waiting(data->text) != 0) {
                if (PAD_PRESSED(13)) {
                    sound_module.play(0x4001C);
                    obj->arrow = 0;
                } else {
                    obj->arrow = 1;
                }
            } else if (PAD_PRESSED(13)) {
                data->text->find_page_end(data->text);
            }
            break;
        case 6:
            wstag924_funcs.fade_start(&obj->text_fade, 0);
            data->text->set_visible(data->text, 0);
            obj->base.step++;
            break;
        case 7:
            if (wstag924_funcs.fade_update(&obj->text_fade) != 0) {
                data->cursor->stop(data->cursor, 0);
                data->cursor->set_palette(data->cursor, 0);
                obj->base.step = 2;
            }
            break;
        case 10:
            for (k = 0; k < obj->count; k++) {
                data->items[k]->set_visible(data->items[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            wstag924_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag924_funcs.fade_update(&obj->fade) != 0) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->arrow) {
            if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
                obj->arrow_time = gfx_module.funcs.get_time();
                if (++obj->arrow_frame >= 5) {
                    obj->arrow_frame = 0;
                }
            }
            spr.set_palette(obj->arrow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x124, 0xCD);
            spr.set_palette(0);
        }
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x56);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), D_WSTAG924_800A7A44[obj->count - 5], 0xA8, 0x18);
        }
        if (obj->text_fade.level != 0) {
            if (obj->text_fade.level != 0x1000) {
                spr.set_scale(obj->text_fade.level, 0x1000, 0x1000);
                spr.set_pivot(0x140, 0x56);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
        wstag924_funcs.fade_start(&obj->fade, 1);
        wstag924_funcs.fade_update(&obj->fade);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag924_event_1604_start(void) {
    return object_new(wstag924_menu_1604_update, 0x84, 0x28);
}

/* The file's .data: the list frame's sprite per count - 5. */
s32 D_WSTAG924_800A7A34[4] = { 28, 27, 25, 36 };
s32 D_WSTAG924_800A7A44[4] = { 28, 27, 25, 36 };
