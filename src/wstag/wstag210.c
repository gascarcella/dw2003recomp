#include "wstag.h"
#include "pad.h"

/* WSTAG210: stage 0x203 (fieldstg_stages). */

extern WstagFadeFuncs wstag210_funcs;
extern s32 D_WSTAG210_800A7F84[];
extern const CVECTOR wstag210_color;
extern FieldstgBattleLists wstag210_battle_lists;
extern FieldstgVramPlace wstag210_vram_places[];
extern FieldstgPlacedActor *wstag210_actors[];
extern FieldstgSprite wstag210_sprites[];
extern FieldstgMapEvent wstag210_map_events[];
extern FieldstgEventDef wstag210_events[];
void wstag210_update();
void wstag210_menu_update();
void wstag210_choice_9_update();
void wstag210_choice_1510_update();

void wstag210_menu_update(WstagMenu *obj, WstagMenuData *data) {
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
            wstag210_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < obj->count; i++) {
                    data->items[i]->set_text(data->items[i], cdload_module.get_subfile_by_id(0x1120001 + (records_language << 16)),
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
            wstag210_funcs.fade_start(&obj->text_fade, 1);
            obj->base.step++;
            break;
        case 4:
            if (wstag210_funcs.fade_update(&obj->text_fade) != 0) {
                data->text->set_text(data->text, cdload_module.get_subfile_by_id(0x1120001 + (records_language << 16)), obj->cursor + 9);
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
            wstag210_funcs.fade_start(&obj->text_fade, 0);
            data->text->set_visible(data->text, 0);
            obj->base.step++;
            break;
        case 7:
            if (wstag210_funcs.fade_update(&obj->text_fade) != 0) {
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
            wstag210_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
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
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), D_WSTAG210_800A7F84[obj->count - 5], 0xA8, 0x18);
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
        wstag210_funcs.fade_start(&obj->fade, 1);
        wstag210_funcs.fade_update(&obj->fade);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag210_event_8_start(void) {
    return object_new(wstag210_menu_update, sizeof(WstagMenu), sizeof(WstagMenuData));
}

void wstag210_choice_9_update(WstagChoice *obj, WstagChoiceData *data) {
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
        for (j = 0; j < 2; j++) {
            data->answers[j] = message_create_window(0x1002, 1, 0x1C, 0xBE + j * 14);
            data->answers[j]->set_ot_depth(data->answers[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0x12, 0xBE);
        data->cursor->show(data->cursor, 0);
        data->question = message_create_window(0x1002, 1, 0x12, 0xB0);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            wstag210_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120002 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120002 + (records_language << 16)), 1);
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
                if (++obj->cursor >= 2) {
                    obj->cursor = 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0x12, obj->cursor * 14 + 0xBE);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 10;
                obj->base.substep = 1;
            }
            break;
        case 3:
            if (obj->cursor == 0) {
                data->event = fieldstg_event_start(0xD);
            } else {
                data->event = fieldstg_event_start(0xE);
            }
            obj->base.step++;
            break;
        case 4:
            if (data->event == NULL) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 10:
            for (k = 0; k < 2; k++) {
                data->answers[k]->set_visible(data->answers[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            data->question->set_visible(data->question, 0);
            wstag210_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
                if (obj->base.substep == 1) {
                    obj->base.step = 3;
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0, 0xC3);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag210_event_9_start(void) {
    return object_new(wstag210_choice_9_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag210_choice_1510_update(WstagChoice *obj, WstagChoiceData *data) {
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
        for (j = 0; j < 2; j++) {
            data->answers[j] = message_create_window(0x1002, 1, 0x1C, 0xBE + j * 14);
            data->answers[j]->set_ot_depth(data->answers[j], 1);
        }
        data->cursor = message_create_cursor(0x1002, 1, 0x12, 0xBE);
        data->cursor->show(data->cursor, 0);
        data->question = message_create_window(0x1002, 1, 0x12, 0xB0);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            wstag210_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120035 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120035 + (records_language << 16)), 1);
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
                if (++obj->cursor >= 2) {
                    obj->cursor = 1;
                }
            }
            if (prev != obj->cursor) {
                sound_module.play(0x8004513E);
                data->cursor->set_pos(data->cursor, 0x12, obj->cursor * 14 + 0xBE);
            } else if (PAD_PRESSED(13)) {
                sound_module.play(0x8004503C);
                obj->base.step = 10;
                obj->base.substep = 1;
            }
            break;
        case 3:
            if (obj->cursor == 0) {
                data->event = fieldstg_event_start(0x3A);
            } else {
                data->event = fieldstg_event_start(0x5E7);
            }
            obj->base.step++;
            break;
        case 4:
            if (data->event == NULL) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 10:
            for (k = 0; k < 2; k++) {
                data->answers[k]->set_visible(data->answers[k], 0);
            }
            data->cursor->show(data->cursor, 0);
            data->question->set_visible(data->question, 0);
            wstag210_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag210_funcs.fade_update(&obj->fade) != 0) {
                if (obj->base.substep == 1) {
                    obj->base.step = 3;
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        }
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 2);
        spr.set_vram_pos(0x140, 0);
        spr.set_follow_scroll(0);
        if (obj->fade.level != 0) {
            if (obj->fade.level != 0x1000) {
                spr.set_scale(obj->fade.level, 0x1000, 0x1000);
                spr.set_pivot(0, 0xC3);
            }
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x45, 0, 0xAC);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag210_event_1510_start(void) {
    return object_new(wstag210_choice_1510_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag210_update(WstagObject *obj, WstagEventData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        /* Evidence (class B, register priority only; DECISIONS "LOOP_BLOCK audit"): without the block only the
         * obj, data and flags-base saved registers differ, also with scheduling off. */
        LOOP_BLOCK(if (gamestate_data.progress == 0xD && gamestate_flags.get_flag(0x1C0B, 0)) {
            data->event = fieldstg_event_start(0x136);
        } else if (gamestate_data.progress == 0x17 && gamestate_flags.get_flag(0x4050, 1)) {
            data->event = fieldstg_event_start(0x2AD);
        });
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag210_start(void *arg0) {
    WstagObject *obj = object_new(wstag210_update, sizeof(WstagObject), 4);

    obj->manager = arg0;
    wstag210_funcs.setup();
    return obj;
}

void wstag210_event_310_end(void) {
    gamestate_flags.set_flag(0x4005, 1);
    gamestate_flags.set_flag(0x1C0B, 1);
}

void wstag210_event_685_end(void) {
    gamestate_flags.set_flag(0x1C26, 0);
    gamestate_data.progress = 0x18;
}

void wstag210_setup(void) {
    fieldstg_stage.background_file = 0x19A;
    fieldstg_stage.sprite_file = 0x019B0000;
    fieldstg_stage.sprites = wstag210_sprites;
    fieldstg_stage.map_events = wstag210_map_events;
    fieldstg_stage.mask_file = 0x322;
    fieldstg_stage.talk_file = records_language + 0xC5;
    fieldstg_stage.start_pos = (GamestatePos){ 0x11C00, 0x13400 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag210_vram_places;
    fieldstg_stage.music = 5;
    fieldstg_stage.sound = 0x60140000;
    fieldstg_stage.actors = wstag210_actors;
    fieldstg_stage.color = wstag210_color;
    fieldstg_stage.events = wstag210_events;
    fieldstg_stage.battle_lists = &wstag210_battle_lists;
    fieldstg_attr.set_file(0, 0x019B0001);
    fieldstg_attr.set_file(7, 0x019B0002);
    fieldstg_attr.init_layer(0);
    if (gamestate_data.progress >= 0x14 && gamestate_data.progress < 0x18) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
    if (gamestate_data.progress >= 0x27 && gamestate_data.progress < 0x29) {
        fieldstg_stage.music = 0x1F;
        fieldstg_stage.sound = 0x607C0000;
    }
}

void wstag210_fade_start(WindowAnim *fade, s32 in) {
    fade->running = 1;
    if (in) {
        sound_module.play(0x40019);
        fade->step = 0x1000 / fade->duration;
        fade->level = 0;
    } else {
        sound_module.play(0x4001A);
        fade->level = 0x1000;
        fade->step = -(0x1000 / fade->duration * 2);
    }
}

s32 wstag210_fade_update(WindowAnim *fade) {
    if (fade->running == 0) {
        return 1;
    }
    fade->level += fade->step;
    if (fade->step > 0) {
        if (fade->level > 0x1000) {
            fade->level = 0x1000;
            fade->running = 0;
            return 1;
        }
    } else if (fade->level < 0) {
        fade->level = 0;
        fade->running = 0;
        return 1;
    }
    return 0;
}

const CVECTOR wstag210_color = { 0x54, 0x67, 0x96, 0 };

/* The stage's .data (tools/wstag_data.py). */
void wstag210_setup(void);

s16 D_WSTAG210_800A77EC[44] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 352, 272, 5),
    FIELDSTG_EVENT_PLACE(32, 384, 255),
    FIELDSTG_EVENT_ANIM(32, 1, 1),
    FIELDSTG_EVENT_PLACE(36, 434, 247),
    FIELDSTG_EVENT_ANIM(36, 1, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_GOTO_MAP(0x204, 352, 272, 5),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7844[62] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_WALK(2, 352, 272, 5),
    FIELDSTG_EVENT_PLACE(32, 384, 255),
    FIELDSTG_EVENT_ANIM(32, 1, 1),
    FIELDSTG_EVENT_PLACE(36, 434, 247),
    FIELDSTG_EVENT_ANIM(36, 1, 7),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 336, 280, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A78C0[50] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 178, 359, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x5DE0, /* padding, not read */
};
s16 D_WSTAG210_800A7924[54] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 2),
    FIELDSTG_EVENT_ANIM(2, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 639, 557, 3),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
    0x4A5F, /* padding, not read */
};
s16 D_WSTAG210_800A7990[59] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 32, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7A08[191] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 106),
    FIELDSTG_EVENT_PLACE(32, 384, 255),
    FIELDSTG_EVENT_ANIM(32, 1, 1),
    FIELDSTG_EVENT_PLACE(36, 434, 247),
    FIELDSTG_EVENT_ANIM(36, 1, 7),
    FIELDSTG_EVENT_PLACE(45, 671, 376),
    FIELDSTG_EVENT_ANIM(45, 1, 7),
    FIELDSTG_EVENT_PLACE(49, 376, 213),
    FIELDSTG_EVENT_ANIM(49, 1, 7),
    FIELDSTG_EVENT_PLACE(54, 535, 307),
    FIELDSTG_EVENT_ANIM(54, 1, 1),
    FIELDSTG_EVENT_PLACE(55, 328, 260),
    FIELDSTG_EVENT_ANIM(55, 1, 6),
    FIELDSTG_EVENT_PLACE(65, 290, 345),
    FIELDSTG_EVENT_ANIM(65, 1, 5),
    FIELDSTG_EVENT_PLACE(106, 317, 311),
    FIELDSTG_EVENT_ANIM(106, 1, 3),
    FIELDSTG_EVENT_PLACE(107, 285, 295),
    FIELDSTG_EVENT_ANIM(107, 1, 7),
    FIELDSTG_EVENT_WAIT(120),
    FIELDSTG_EVENT_DIALOG(0, 1, 106, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 107, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 106, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 107, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 106, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 107, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 107),
    FIELDSTG_EVENT_ANIM(107, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(106, 1, 4),
    FIELDSTG_EVENT_WALK(107, 285, 216, 4),
    FIELDSTG_EVENT_WAIT_WALK(107),
    FIELDSTG_EVENT_WALK(107, 330, 196, 5),
    FIELDSTG_EVENT_WAIT_WALK(107),
    FIELDSTG_EVENT_ANIM(0x32D, 847, 107),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_WALK(107, 394, 164, 5),
    FIELDSTG_EVENT_WAIT_WALK(107),
    FIELDSTG_EVENT_ANIM(0x32D, 853, 107),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_CAMERA_FOLLOW(0, 106),
    FIELDSTG_EVENT_PLACE(107, 0, 0),
    FIELDSTG_EVENT_ANIM(107, 1, 0),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 106, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7B88[45] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 106),
    FIELDSTG_EVENT_ANIM(106, 1, 1),
    FIELDSTG_EVENT_ANIM(0x323, 805, 106),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 106),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(106, 232, 332, 5),
    FIELDSTG_EVENT_WAIT_WALK(106),
    FIELDSTG_EVENT_ANIM(106, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 106, 3),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(106, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7BE4[45] = {
    FIELDSTG_EVENT_CAMERA_FOLLOW(1, 106),
    FIELDSTG_EVENT_ANIM(106, 1, 7),
    FIELDSTG_EVENT_ANIM(0x323, 805, 106),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 106),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(106, 616, 540, 3),
    FIELDSTG_EVENT_WAIT_WALK(106),
    FIELDSTG_EVENT_ANIM(106, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 106, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(106, 1, 3),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7C40[405] = {
    FIELDSTG_EVENT_PLACE(2, 394, 164),
    FIELDSTG_EVENT_ANIM(2, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 847, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 330, 196, 1),
    FIELDSTG_EVENT_PLACE(179, 394, 164),
    FIELDSTG_EVENT_ANIM(179, 1, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 298, 212, 5),
    FIELDSTG_EVENT_WALK(179, 330, 196, 1),
    FIELDSTG_EVENT_WAIT_WALK(179),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(179, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 853, 2),
    FIELDSTG_EVENT_WAIT(6),
    FIELDSTG_EVENT_WALK(2, 282, 220, 1),
    FIELDSTG_EVENT_WALK(179, 314, 204, 1),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_ANIM(179, 1, 1),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 5),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WALK(2, 282, 252, 4),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_DIALOG(0, 2, 179, 2),
    FIELDSTG_EVENT_WALK(179, 282, 220, 0),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 8, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 9, 2, 1),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 807, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 805, 2),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_ANIM(0x323, 806, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 10, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x323, 805, 179),
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_ANIM(0x323, 806, 179),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(179, 1, 0),
    FIELDSTG_EVENT_WAIT_WALK(179),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 11, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 12, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 13, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 14, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 15, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 16, 2, 1),
    FIELDSTG_EVENT_ANIM(2, 7, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_ANIM(2, 1, 4),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 17, 179, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(2, 1, 0),
    FIELDSTG_EVENT_ANIM(179, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WALK(2, 282, 304, 0),
    FIELDSTG_EVENT_WALK(179, 330, 196, 5),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_WALK(2, 154, 368, 1),
    FIELDSTG_EVENT_WALK(179, 394, 164, 5),
    FIELDSTG_EVENT_ANIM(0x32D, 847, 2),
    FIELDSTG_EVENT_WAIT(12),
    FIELDSTG_EVENT_GOTO_MAP(0x200, 1000, 236, 1),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG210_800A7F6C[11] = {
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s32 D_WSTAG210_800A7F84[4] = { 28, 27, 25, 36 };
FieldstgListedBattle D_WSTAG210_800A7F94 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FA0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FAC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FB8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FC4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FD0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FDC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A7FE8 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG210_800A7FF4 = {
    0,
    { &D_WSTAG210_800A7F94, &D_WSTAG210_800A7FA0, &D_WSTAG210_800A7FAC, &D_WSTAG210_800A7FB8, &D_WSTAG210_800A7FC4,
        &D_WSTAG210_800A7FD0, &D_WSTAG210_800A7FDC, &D_WSTAG210_800A7FE8 },
};
FieldstgListedBattle D_WSTAG210_800A8018 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8024 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8030 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A803C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8048 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8054 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8060 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A806C = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG210_800A8078 = {
    0,
    { &D_WSTAG210_800A8018, &D_WSTAG210_800A8024, &D_WSTAG210_800A8030, &D_WSTAG210_800A803C, &D_WSTAG210_800A8048,
        &D_WSTAG210_800A8054, &D_WSTAG210_800A8060, &D_WSTAG210_800A806C },
};
FieldstgListedBattle D_WSTAG210_800A809C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80A8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80B4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80C0 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80CC = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80D8 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80E4 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A80F0 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG210_800A80FC = {
    0,
    { &D_WSTAG210_800A809C, &D_WSTAG210_800A80A8, &D_WSTAG210_800A80B4, &D_WSTAG210_800A80C0, &D_WSTAG210_800A80CC,
        &D_WSTAG210_800A80D8, &D_WSTAG210_800A80E4, &D_WSTAG210_800A80F0 },
};
FieldstgListedBattle D_WSTAG210_800A8120 = { 188, 15, 0x60080000 };
FieldstgListedBattle D_WSTAG210_800A812C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8138 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8144 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8150 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A815C = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8168 = { 0, 0, 0x60040000 };
FieldstgListedBattle D_WSTAG210_800A8174 = { 0, 0, 0x60040000 };
FieldstgBattleList D_WSTAG210_800A8180 = {
    0,
    { &D_WSTAG210_800A8120, &D_WSTAG210_800A812C, &D_WSTAG210_800A8138, &D_WSTAG210_800A8144, &D_WSTAG210_800A8150,
        &D_WSTAG210_800A815C, &D_WSTAG210_800A8168, &D_WSTAG210_800A8174 },
};
FieldstgBattleLists wstag210_battle_lists = {
    132, 0, 0, { &D_WSTAG210_800A7FF4, &D_WSTAG210_800A8078, &D_WSTAG210_800A80FC }, &D_WSTAG210_800A8180,
};
FieldstgVramPlace wstag210_vram_places[31] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 384, 256, 436, 256, 464, 0, 368, 504 }, { 448, 256, 488, 366, 672, 110, 320, 503 },
    { 320, 256, 376, 313, 224, 57, 336, 503 }, { 384, 256, 438, 441, 472, 185, 352, 503 },
    { 448, 256, 480, 366, 640, 110, 368, 503 }, { 448, 256, 448, 366, 512, 110, 320, 502 },
    { 384, 256, 416, 416, 384, 160, 336, 502 }, { 384, 256, 416, 456, 384, 200, 352, 502 },
    { 448, 256, 472, 366, 608, 110, 368, 502 }, { 448, 256, 456, 366, 544, 110, 320, 501 },
    { 448, 256, 456, 406, 544, 150, 352, 501 }, { 448, 256, 488, 438, 672, 182, 368, 501 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 448, 256, 464, 406, 576, 150, 320, 500 },
    { 448, 256, 472, 406, 608, 150, 336, 500 }, { 448, 256, 480, 406, 640, 150, 352, 500 },
    { 320, 256, 368, 313, 192, 57, 368, 500 }, { 448, 256, 448, 438, 512, 182, 336, 499 },
    { 448, 256, 456, 438, 544, 182, 352, 499 }, { 448, 256, 464, 438, 576, 182, 368, 499 },
    { 448, 256, 472, 438, 608, 182, 320, 498 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 360, 313, 160, 57, 336, 498 },
};
u16 D_WSTAG210_800A83B0[4] = { 0x1A00, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A83B8[4] = { 0x1A00, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83C0[4] = { 0x1A00, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83C8[4] = { 0x9004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83D0[4] = { 0x901A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83D8[4] = { 0x901A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83E0[4] = { 0x901A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83E8[4] = { 0x1A00, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A83F0[4] = { 0x1A00, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A83F8[4] = { 0x1A00, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8400[4] = { 0x9003, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8408[4] = { 0x9003, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8410[4] = { 0x9003, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8418[4] = { 0x9003, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8420[4] = { 0x1A16, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8428[4] = { 0x1A16, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8430[4] = { 0x1A16, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG210_800A8438[3] = {
    { D_WSTAG210_800A83B0, D_WSTAG210_800A83B8, 103 }, { D_WSTAG210_800A83C0, D_WSTAG210_800A83C8, 1158 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG210_800A845C[2] = { { NULL, D_WSTAG210_800A83D0, 103 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8474[2] = { { NULL, D_WSTAG210_800A83D8, 1065 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A848C[2] = { { NULL, D_WSTAG210_800A83E0, 1067 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A84A4[3] = {
    { D_WSTAG210_800A83E8, D_WSTAG210_800A83F0, 102 }, { D_WSTAG210_800A83F8, D_WSTAG210_800A8400, 52 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG210_800A84C8[2] = { { NULL, D_WSTAG210_800A8408, 52 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A84E0[2] = { { NULL, D_WSTAG210_800A8410, 53 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A84F8[2] = { { NULL, D_WSTAG210_800A8418, 55 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8510[2] = { { NULL, NULL, 17 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8528[2] = { { NULL, NULL, 329 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8540[2] = { { NULL, NULL, 420 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8558[2] = { { NULL, NULL, 422 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8570[2] = { { NULL, NULL, 421 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8588[2] = { { NULL, NULL, 423 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A85A0[2] = { { NULL, NULL, 424 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A85B8[2] = { { NULL, NULL, 425 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A85D0[2] = { { NULL, NULL, 430 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A85E8[2] = { { NULL, NULL, 426 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8600[2] = { { NULL, NULL, 427 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8618[2] = { { NULL, NULL, 428 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8630[2] = { { NULL, NULL, 14 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8648[2] = { { NULL, NULL, 328 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8660[2] = { { NULL, NULL, 409 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8678[2] = { { NULL, NULL, 16 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8690[2] = { { NULL, NULL, 327 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A86A8[2] = { { NULL, NULL, 398 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A86C0[2] = { { NULL, NULL, 13 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A86D8[2] = { { NULL, NULL, 331 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A86F0[2] = { { NULL, NULL, 442 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8708[2] = { { NULL, NULL, 444 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8720[2] = { { NULL, NULL, 443 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8738[2] = { { NULL, NULL, 445 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8750[2] = { { NULL, NULL, 446 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8768[2] = { { NULL, NULL, 447 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8780[2] = { { NULL, NULL, 452 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8798[2] = { { NULL, NULL, 448 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A87B0[2] = { { NULL, NULL, 449 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A87C8[2] = { { NULL, NULL, 450 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A87E0[2] = { { NULL, NULL, 240 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A87F8[2] = { { NULL, NULL, 332 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8810[2] = { { NULL, NULL, 453 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8828[2] = { { NULL, NULL, 239 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8840[2] = { { NULL, NULL, 330 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8858[2] = { { NULL, NULL, 431 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8870[2] = { { NULL, NULL, 465 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8888[2] = { { NULL, NULL, 467 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A88A0[2] = { { NULL, NULL, 468 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A88B8[2] = { { NULL, NULL, 466 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A88D0[2] = { { NULL, NULL, 469 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A88E8[2] = { { NULL, NULL, 474 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8900[2] = { { NULL, NULL, 470 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8918[2] = { { NULL, NULL, 471 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8930[2] = { { NULL, NULL, 472 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8948[2] = { { NULL, NULL, 478 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8960[2] = { { NULL, NULL, 479 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8978[2] = { { NULL, NULL, 477 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8990[2] = { { NULL, NULL, 476 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A89A8[2] = { { NULL, NULL, 480 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A89C0[2] = { { NULL, NULL, 485 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A89D8[2] = { { NULL, NULL, 481 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A89F0[2] = { { NULL, NULL, 482 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A08[2] = { { NULL, NULL, 483 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A20[2] = { { NULL, NULL, 1066 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A38[2] = { { NULL, NULL, 54 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A50[2] = { { NULL, NULL, 429 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A68[2] = { { NULL, NULL, 451 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A80[2] = { { NULL, NULL, 473 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8A98[2] = { { NULL, NULL, 484 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8AB0[3] = {
    { D_WSTAG210_800A8420, D_WSTAG210_800A8428, 79 }, { D_WSTAG210_800A8430, NULL, 1160 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG210_800A8AD4[2] = { { NULL, NULL, 676 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8AEC[2] = { { NULL, NULL, 675 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B04[2] = { { NULL, NULL, 56 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B1C[2] = { { NULL, NULL, 58 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B34[2] = { { NULL, NULL, 59 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B4C[2] = { { NULL, NULL, 57 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B64[2] = { { NULL, NULL, 60 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B7C[2] = { { NULL, NULL, 61 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8B94[2] = { { NULL, NULL, 65 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8BAC[2] = { { NULL, NULL, 62 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8BC4[2] = { { NULL, NULL, 63 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG210_800A8BDC[2] = { { NULL, NULL, 64 }, { NULL, NULL, 0 } };
u16 D_WSTAG210_800A8BF4[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8BFC[6] = { 0x7022, 1, 0x6016, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C08[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C10[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C18[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C20[6] = { 0x7022, 1, 0x6016, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C2C[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C34[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C3C[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C44[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C4C[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C54[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C5C[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C64[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C6C[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C74[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C7C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C84[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C8C[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C94[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8C9C[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CA4[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CAC[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CB4[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CBC[6] = { 0x6002, 1, 0x8000, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CC8[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CD0[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CD8[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CE0[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CE8[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CF0[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8CF8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D00[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D08[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D10[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D18[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D20[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D28[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D30[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D38[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D40[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D48[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D50[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D58[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D60[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D68[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D70[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D78[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D80[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D88[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D90[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8D98[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DA0[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DA8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DB0[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DB8[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DC0[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DC8[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DD0[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DD8[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DE0[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DE8[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DF0[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8DF8[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E00[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E08[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E10[6] = { 0x600D, 1, 0x1C0D, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E1C[6] = { 0x600D, 1, 0x1C0D, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E28[6] = { 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E34[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E3C[6] = { 0x7009, 1, 0x701A, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E48[4] = { 0x6002, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E50[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E58[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E60[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E68[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E70[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E78[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E80[8] = { 0x1A15, 1, 0x600C, 1, 0x800F, 0, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E90[6] = { 0x600D, 1, 0x1C0D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8E9C[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EA4[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EAC[4] = { 0x6017, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EB4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EBC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EC4[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8ECC[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8ED4[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EDC[4] = { 0x600D, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EE4[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EEC[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EF4[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8EFC[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8F04[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8F0C[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG210_800A8F14[4] = { 0x6017, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG210_800A8F1C = { D_WSTAG210_800A8BF4, D_WSTAG210_800A8438, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG210_800A8F30 = { D_WSTAG210_800A8BFC, D_WSTAG210_800A845C, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG210_800A8F44 = { D_WSTAG210_800A8C08, D_WSTAG210_800A8474, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG210_800A8F58 = { D_WSTAG210_800A8C10, D_WSTAG210_800A848C, 32, 4, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG210_800A8F6C = { D_WSTAG210_800A8C18, D_WSTAG210_800A84A4, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG210_800A8F80 = { D_WSTAG210_800A8C20, D_WSTAG210_800A84C8, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG210_800A8F94 = { D_WSTAG210_800A8C2C, D_WSTAG210_800A84E0, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG210_800A8FA8 = { D_WSTAG210_800A8C34, D_WSTAG210_800A84F8, 36, 5, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG210_800A8FBC = { D_WSTAG210_800A8C3C, D_WSTAG210_800A8510, 45, 6, 684, 363, 7 };
FieldstgPlacedActor D_WSTAG210_800A8FD0 = { D_WSTAG210_800A8C44, D_WSTAG210_800A8528, 45, 6, 684, 363, 7 };
FieldstgPlacedActor D_WSTAG210_800A8FE4 = { D_WSTAG210_800A8C4C, D_WSTAG210_800A8540, 45, 6, 684, 363, 7 };
FieldstgPlacedActor D_WSTAG210_800A8FF8 = { D_WSTAG210_800A8C54, D_WSTAG210_800A8558, 45, 6, 684, 363, 7 };
FieldstgPlacedActor D_WSTAG210_800A900C = { D_WSTAG210_800A8C5C, D_WSTAG210_800A8570, 45, 6, 529, 505, 5 };
FieldstgPlacedActor D_WSTAG210_800A9020 = { D_WSTAG210_800A8C64, D_WSTAG210_800A8588, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9034 = { D_WSTAG210_800A8C6C, D_WSTAG210_800A85A0, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9048 = { D_WSTAG210_800A8C74, D_WSTAG210_800A85B8, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A905C = { D_WSTAG210_800A8C7C, D_WSTAG210_800A85D0, 45, 6, 305, 353, 7 };
FieldstgPlacedActor D_WSTAG210_800A9070 = { D_WSTAG210_800A8C84, D_WSTAG210_800A85E8, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9084 = { D_WSTAG210_800A8C8C, D_WSTAG210_800A8600, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9098 = { D_WSTAG210_800A8C94, D_WSTAG210_800A8618, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A90AC = { D_WSTAG210_800A8C9C, NULL, 45, 6, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A90C0 = { D_WSTAG210_800A8CA4, D_WSTAG210_800A8630, 46, 7, 391, 359, 1 };
FieldstgPlacedActor D_WSTAG210_800A90D4 = { D_WSTAG210_800A8CAC, D_WSTAG210_800A8648, 46, 7, 391, 359, 1 };
FieldstgPlacedActor D_WSTAG210_800A90E8 = { D_WSTAG210_800A8CB4, D_WSTAG210_800A8660, 46, 7, 391, 359, 1 };
FieldstgPlacedActor D_WSTAG210_800A90FC = { D_WSTAG210_800A8CBC, D_WSTAG210_800A8678, 47, 8, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG210_800A9110 = { D_WSTAG210_800A8CC8, D_WSTAG210_800A8690, 47, 8, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG210_800A9124 = { D_WSTAG210_800A8CD0, D_WSTAG210_800A86A8, 47, 8, 617, 501, 1 };
FieldstgPlacedActor D_WSTAG210_800A9138 = { D_WSTAG210_800A8CD8, D_WSTAG210_800A86C0, 49, 9, 361, 221, 7 };
FieldstgPlacedActor D_WSTAG210_800A914C = { D_WSTAG210_800A8CE0, D_WSTAG210_800A86D8, 49, 9, 361, 221, 7 };
FieldstgPlacedActor D_WSTAG210_800A9160 = { D_WSTAG210_800A8CE8, D_WSTAG210_800A86F0, 49, 9, 361, 221, 7 };
FieldstgPlacedActor D_WSTAG210_800A9174 = { D_WSTAG210_800A8CF0, D_WSTAG210_800A8708, 49, 9, 361, 221, 7 };
FieldstgPlacedActor D_WSTAG210_800A9188 = { D_WSTAG210_800A8CF8, D_WSTAG210_800A8720, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A919C = { D_WSTAG210_800A8D00, D_WSTAG210_800A8738, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A91B0 = { D_WSTAG210_800A8D08, D_WSTAG210_800A8750, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A91C4 = { D_WSTAG210_800A8D10, D_WSTAG210_800A8768, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A91D8 = { D_WSTAG210_800A8D18, D_WSTAG210_800A8780, 49, 9, 391, 359, 7 };
FieldstgPlacedActor D_WSTAG210_800A91EC = { D_WSTAG210_800A8D20, D_WSTAG210_800A8798, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A9200 = { D_WSTAG210_800A8D28, D_WSTAG210_800A87B0, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A9214 = { D_WSTAG210_800A8D30, D_WSTAG210_800A87C8, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A9228 = { D_WSTAG210_800A8D38, NULL, 49, 9, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A923C = { D_WSTAG210_800A8D40, D_WSTAG210_800A87E0, 51, 10, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9250 = { D_WSTAG210_800A8D48, D_WSTAG210_800A87F8, 51, 10, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9264 = { D_WSTAG210_800A8D50, D_WSTAG210_800A8810, 51, 10, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A9278 = { D_WSTAG210_800A8D58, D_WSTAG210_800A8828, 52, 11, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A928C = { D_WSTAG210_800A8D60, D_WSTAG210_800A8840, 52, 11, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A92A0 = { D_WSTAG210_800A8D68, D_WSTAG210_800A8858, 52, 11, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A92B4 = { D_WSTAG210_800A8D70, D_WSTAG210_800A8870, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A92C8 = { D_WSTAG210_800A8D78, D_WSTAG210_800A8888, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A92DC = { D_WSTAG210_800A8D80, D_WSTAG210_800A88A0, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A92F0 = { D_WSTAG210_800A8D88, D_WSTAG210_800A88B8, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9304 = { D_WSTAG210_800A8D90, D_WSTAG210_800A88D0, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9318 = { D_WSTAG210_800A8D98, D_WSTAG210_800A88E8, 54, 12, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A932C = { D_WSTAG210_800A8DA0, D_WSTAG210_800A8900, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9340 = { D_WSTAG210_800A8DA8, D_WSTAG210_800A8918, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9354 = { D_WSTAG210_800A8DB0, D_WSTAG210_800A8930, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9368 = { D_WSTAG210_800A8DB8, NULL, 54, 12, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A937C = { D_WSTAG210_800A8DC0, D_WSTAG210_800A8948, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A9390 = { D_WSTAG210_800A8DC8, D_WSTAG210_800A8960, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A93A4 = { D_WSTAG210_800A8DD0, D_WSTAG210_800A8978, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A93B8 = { D_WSTAG210_800A8DD8, D_WSTAG210_800A8990, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A93CC = { D_WSTAG210_800A8DE0, D_WSTAG210_800A89A8, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A93E0 = { D_WSTAG210_800A8DE8, D_WSTAG210_800A89C0, 55, 13, 684, 363, 3 };
FieldstgPlacedActor D_WSTAG210_800A93F4 = { D_WSTAG210_800A8DF0, D_WSTAG210_800A89D8, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A9408 = { D_WSTAG210_800A8DF8, D_WSTAG210_800A89F0, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A941C = { D_WSTAG210_800A8E00, D_WSTAG210_800A8A08, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A9430 = { D_WSTAG210_800A8E08, NULL, 55, 13, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A9444 = { D_WSTAG210_800A8E10, NULL, 106, 14, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG210_800A9458 = { D_WSTAG210_800A8E1C, NULL, 107, 15, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG210_800A946C = { D_WSTAG210_800A8E28, NULL, 112, 16, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG210_800A9480 = { D_WSTAG210_800A8E34, NULL, 112, 16, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG210_800A9494 = { D_WSTAG210_800A8E3C, NULL, 113, 17, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG210_800A94A8 = { D_WSTAG210_800A8E48, NULL, 113, 17, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG210_800A94BC = { D_WSTAG210_800A8E50, D_WSTAG210_800A8A20, 157, 18, 384, 255, 1 };
FieldstgPlacedActor D_WSTAG210_800A94D0 = { D_WSTAG210_800A8E58, D_WSTAG210_800A8A38, 158, 19, 434, 247, 7 };
FieldstgPlacedActor D_WSTAG210_800A94E4 = { D_WSTAG210_800A8E60, D_WSTAG210_800A8A50, 159, 20, 529, 505, 7 };
FieldstgPlacedActor D_WSTAG210_800A94F8 = { D_WSTAG210_800A8E68, D_WSTAG210_800A8A68, 160, 21, 193, 257, 7 };
FieldstgPlacedActor D_WSTAG210_800A950C = { D_WSTAG210_800A8E70, D_WSTAG210_800A8A80, 162, 22, 535, 307, 1 };
FieldstgPlacedActor D_WSTAG210_800A9520 = { D_WSTAG210_800A8E78, D_WSTAG210_800A8A98, 174, 23, 328, 260, 6 };
FieldstgPlacedActor D_WSTAG210_800A9534 = { D_WSTAG210_800A8E80, D_WSTAG210_800A8AB0, 178, 24, 285, 295, 5 };
FieldstgPlacedActor D_WSTAG210_800A9548 = { D_WSTAG210_800A8E90, NULL, 178, 24, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG210_800A955C = { D_WSTAG210_800A8E9C, D_WSTAG210_800A8AD4, 178, 24, 285, 295, 7 };
FieldstgPlacedActor D_WSTAG210_800A9570 = { D_WSTAG210_800A8EA4, D_WSTAG210_800A8AEC, 179, 25, 308, 305, 3 };
FieldstgPlacedActor D_WSTAG210_800A9584 = { D_WSTAG210_800A8EAC, NULL, 179, 25, 0, 0, 1 };
FieldstgPlacedActor D_WSTAG210_800A9598 = { D_WSTAG210_800A8EB4, NULL, 270, 26, 360, 268, 1 };
FieldstgPlacedActor D_WSTAG210_800A95AC = { D_WSTAG210_800A8EBC, NULL, 271, 27, 450, 254, 7 };
FieldstgPlacedActor D_WSTAG210_800A95C0 = { D_WSTAG210_800A8EC4, D_WSTAG210_800A8B04, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A95D4 = { D_WSTAG210_800A8ECC, D_WSTAG210_800A8B1C, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A95E8 = { D_WSTAG210_800A8ED4, D_WSTAG210_800A8B34, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A95FC = { D_WSTAG210_800A8EDC, D_WSTAG210_800A8B4C, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9610 = { D_WSTAG210_800A8EE4, D_WSTAG210_800A8B64, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9624 = { D_WSTAG210_800A8EEC, D_WSTAG210_800A8B7C, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9638 = { D_WSTAG210_800A8EF4, D_WSTAG210_800A8B94, 373, 28, 617, 501, 5 };
FieldstgPlacedActor D_WSTAG210_800A964C = { D_WSTAG210_800A8EFC, D_WSTAG210_800A8BAC, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9660 = { D_WSTAG210_800A8F04, D_WSTAG210_800A8BC4, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9674 = { D_WSTAG210_800A8F0C, D_WSTAG210_800A8BDC, 373, 28, 305, 353, 5 };
FieldstgPlacedActor D_WSTAG210_800A9688 = { D_WSTAG210_800A8F14, NULL, 373, 28, 305, 353, 5 };
FieldstgPlacedActor *wstag210_actors[97] = {
    &D_WSTAG210_800A8F1C, &D_WSTAG210_800A8F30, &D_WSTAG210_800A8F44, &D_WSTAG210_800A8F58, &D_WSTAG210_800A8F6C,
    &D_WSTAG210_800A8F80, &D_WSTAG210_800A8F94, &D_WSTAG210_800A8FA8, &D_WSTAG210_800A8FBC, &D_WSTAG210_800A8FD0,
    &D_WSTAG210_800A8FE4, &D_WSTAG210_800A8FF8, &D_WSTAG210_800A900C, &D_WSTAG210_800A9020, &D_WSTAG210_800A9034,
    &D_WSTAG210_800A9048, &D_WSTAG210_800A905C, &D_WSTAG210_800A9070, &D_WSTAG210_800A9084, &D_WSTAG210_800A9098,
    &D_WSTAG210_800A90AC, &D_WSTAG210_800A90C0, &D_WSTAG210_800A90D4, &D_WSTAG210_800A90E8, &D_WSTAG210_800A90FC,
    &D_WSTAG210_800A9110, &D_WSTAG210_800A9124, &D_WSTAG210_800A9138, &D_WSTAG210_800A914C, &D_WSTAG210_800A9160,
    &D_WSTAG210_800A9174, &D_WSTAG210_800A9188, &D_WSTAG210_800A919C, &D_WSTAG210_800A91B0, &D_WSTAG210_800A91C4,
    &D_WSTAG210_800A91D8, &D_WSTAG210_800A91EC, &D_WSTAG210_800A9200, &D_WSTAG210_800A9214, &D_WSTAG210_800A9228,
    &D_WSTAG210_800A923C, &D_WSTAG210_800A9250, &D_WSTAG210_800A9264, &D_WSTAG210_800A9278, &D_WSTAG210_800A928C,
    &D_WSTAG210_800A92A0, &D_WSTAG210_800A92B4, &D_WSTAG210_800A92C8, &D_WSTAG210_800A92DC, &D_WSTAG210_800A92F0,
    &D_WSTAG210_800A9304, &D_WSTAG210_800A9318, &D_WSTAG210_800A932C, &D_WSTAG210_800A9340, &D_WSTAG210_800A9354,
    &D_WSTAG210_800A9368, &D_WSTAG210_800A937C, &D_WSTAG210_800A9390, &D_WSTAG210_800A93A4, &D_WSTAG210_800A93B8,
    &D_WSTAG210_800A93CC, &D_WSTAG210_800A93E0, &D_WSTAG210_800A93F4, &D_WSTAG210_800A9408, &D_WSTAG210_800A941C,
    &D_WSTAG210_800A9430, &D_WSTAG210_800A9444, &D_WSTAG210_800A9458, &D_WSTAG210_800A946C, &D_WSTAG210_800A9480,
    &D_WSTAG210_800A9494, &D_WSTAG210_800A94A8, &D_WSTAG210_800A94BC, &D_WSTAG210_800A94D0, &D_WSTAG210_800A94E4,
    &D_WSTAG210_800A94F8, &D_WSTAG210_800A950C, &D_WSTAG210_800A9520, &D_WSTAG210_800A9534, &D_WSTAG210_800A9548,
    &D_WSTAG210_800A955C, &D_WSTAG210_800A9570, &D_WSTAG210_800A9584, &D_WSTAG210_800A9598, &D_WSTAG210_800A95AC,
    &D_WSTAG210_800A95C0, &D_WSTAG210_800A95D4, &D_WSTAG210_800A95E8, &D_WSTAG210_800A95FC, &D_WSTAG210_800A9610,
    &D_WSTAG210_800A9624, &D_WSTAG210_800A9638, &D_WSTAG210_800A964C, &D_WSTAG210_800A9660, &D_WSTAG210_800A9674,
    &D_WSTAG210_800A9688, NULL,
};
FieldstgSprite wstag210_sprites[105] = {
    { 1, 0, 0x40, 2, 0x36, 2, 0, 1, 4, 0, 243, 69, 0, 0 }, { 1, 0, 0x40, 2, 0x37, 2, 0, 1, 4, 0, 802, 278, 0, 0 },
    { 1, 0, 0x40, 2, 0x33, 2, 0, 1, 4, 0, 457, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 339, 212, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 372, 196, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 403, 244, 0, 0 },
    { 1, 0, 0x40, 2, 0x32, 2, 0, 3, 0x16, 0, 435, 228, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 386, 117, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 546, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x55, 2, 0, 7, 0x12, 0, 674, 197, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 573, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x56, 2, 0, 7, 0x12, 0, 613, 209, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 151, 129, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 411, 111, 0, 0 },
    { 1, 0, 0x40, 2, 0x57, 2, 0, 7, 0x12, 0, 649, 200, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 10, 105, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 42, 121, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 74, 137, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 290, 69, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 322, 85, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 354, 101, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 482, 165, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 514, 181, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 706, 213, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 738, 229, 0, 0 },
    { 1, 0, 0x40, 6, 0x55, 2, 0, 7, 0x12, 0, 770, 245, 0, 0 },
    { 1, 0, 0x40, 6, 0x57, 2, 0, 7, 0x12, 0, 119, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 135, 232, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 155, 222, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 175, 212, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 195, 202, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 215, 192, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 235, 182, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 255, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 275, 162, 0, 0 },
    { 1, 0, 0x40, 6, 0x58, 2, 0, 7, 0x12, 0, 295, 152, 0, 0 },
    { 1, 0x66, 0x40, 6, 4, 0, 0, 0, 0, 0, 335, 141, 0, 0 }, { 1, 0x65, 0x40, 6, 5, 0, 0, 0, 0, 0, 511, 229, 0, 0 },
    { 1, 0x64, 0x40, 6, 6, 0, 0, 0, 0, 0, 736, 326, 0, 0 }, { 1, 0, 0x40, 6, 0x34, 2, 0, 3, 4, 0, 114, 179, 0, 0 },
    { 1, 0, 0x40, 6, 0x35, 2, 0, 3, 4, 0, 264, 130, 0, 0 }, { 1, 0, 0x40, 6, 0x59, 2, 0, 3, 4, 0, 429, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x51, 1, 0x51, 0x54, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 6, 0x4D, 1, 0x4D, 0x50, 8, 0, 585, 335, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 28, 519, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 81, 492, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 183, 442, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 237, 424, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 656, 492, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x38, 1, 0x38, 0x3A, 0xA, 0, 752, 431, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 55, 506, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 100, 493, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 151, 461, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 202, 445, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 296, 424, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3B, 1, 0x3B, 0x3D, 0xA, 0, 657, 504, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 259, 428, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 366, 433, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 582, 411, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 661, 442, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 674, 537, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 694, 538, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x3E, 1, 0x3E, 0x40, 0xA, 0, 778, 417, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 449, 517, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 501, 544, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 530, 355, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 546, 354, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 636, 592, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x41, 1, 0x41, 0x43, 0xA, 0, 659, 600, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 87, 303, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 130, 322, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x44, 1, 0x44, 0x46, 0xA, 0, 682, 614, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 61, 292, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 113, 328, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 474, 531, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x47, 1, 0x47, 0x49, 0xA, 0, 529, 554, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 437, 427, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 455, 421, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 608, 405, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 626, 396, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 654, 525, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 691, 424, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 144, 110, 160, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 359, 148, 193, 0 },
    { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 535, 236, 281, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 759, 332, 376, 0 },
    { 1, 0, 0x50, 4, 7, 0, 0, 0, 0, 0, 441, 345, 450, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 464, 444, 496, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 595, 442, 478, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 384, 259, 279, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 401, 250, 271, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 368, 251, 270, 0 },
    { 1, 0, 0x40, 4, 0xD, 0, 0, 0, 0, 0, 417, 243, 264, 0 },
    { 1, 0, 0x40, 4, 0xE, 0, 0, 0, 0, 0, 352, 242, 262, 0 },
    { 1, 0, 0x40, 4, 0xF, 0, 0, 0, 0, 0, 433, 233, 255, 0 },
    { 1, 0, 0x40, 4, 0x10, 0, 0, 0, 0, 0, 336, 229, 255, 0 },
    { 1, 0, 0x40, 4, 0x11, 0, 0, 0, 0, 0, 449, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x12, 0, 0, 0, 0, 0, 320, 227, 247, 0 },
    { 1, 0, 0x40, 4, 0x13, 0, 0, 0, 0, 0, 337, 219, 240, 0 },
    { 1, 0, 0x40, 4, 0x14, 0, 0, 0, 0, 0, 353, 208, 231, 0 },
    { 1, 0, 0x40, 4, 0x15, 0, 0, 0, 0, 0, 369, 203, 223, 0 },
    { 1, 0, 0x40, 4, 0x16, 0, 0, 0, 0, 0, 385, 192, 215, 0 },
    { 1, 0, 0x40, 4, 0x17, 0, 0, 0, 0, 0, 401, 187, 207, 0 },
    { 1, 0, 0x40, 4, 0x18, 0, 0, 0, 0, 0, 499, 317, 334, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag210_map_events[11] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x200, 0x3E8, 0xEC, 1, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x206, 0x70, 0xF0, 7, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x207, 0x58, 0x1CC, 5, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x208, 0x69, 0x134, 5, 0x65, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x214, 0x1F8, 0x2BC, 5, 0x66, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x217, 0x1D8, 0x1D4, 3, 0, 0, 0 },
    { 0x6002, 1, 0xFFFF, 0, 8, 0x14, 0, 0, 0, 0, 0, 0 }, { 0x6002, 1, 0xFFFF, 0, 8, 0x1E, 0, 0, 0, 0, 0, 0 },
    { 0x600D, 1, 0xFFFF, 0, 8, 0x140, 0, 0, 0, 0, 0, 0 }, { 0x600D, 1, 0xFFFF, 0, 8, 0x14A, 0, 0, 0, 0, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFadeFuncs wstag210_funcs = { wstag210_setup, wstag210_fade_start, wstag210_fade_update };
FieldstgEventDef wstag210_events[14] = {
    { 8, NULL, 0x01120001, (s32 (*)(void))wstag210_event_8_start, NULL },
    { 9, NULL, 0x01120002, (s32 (*)(void))wstag210_event_9_start, NULL },
    { 13, D_WSTAG210_800A77EC, 0x01120006, NULL, NULL }, { 14, D_WSTAG210_800A7844, 0x01120007, NULL, NULL },
    { 20, D_WSTAG210_800A78C0, 0x01120008, NULL, NULL }, { 30, D_WSTAG210_800A7924, 0x01120009, NULL, NULL },
    { 58, D_WSTAG210_800A7990, 0x01120012, NULL, NULL },
    { 310, D_WSTAG210_800A7A08, 0x0112001C, NULL, wstag210_event_310_end },
    { 320, D_WSTAG210_800A7B88, 0x0112001D, NULL, NULL }, { 330, D_WSTAG210_800A7BE4, 0x0112001E, NULL, NULL },
    { 685, D_WSTAG210_800A7C40, 0x01120022, NULL, wstag210_event_685_end },
    { 1510, NULL, 0x01120035, (s32 (*)(void))wstag210_event_1510_start, NULL },
    { 1511, D_WSTAG210_800A7F6C, 0x01120030, NULL, NULL },
    { -1, NULL, 0, NULL, NULL },
};
