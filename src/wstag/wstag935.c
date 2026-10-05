#include "wstag.h"
#include "pad.h"

/* WSTAG935: stage 0x280 (fieldstg_stages_2d). */

extern WstagFadeFuncs wstag935_funcs;
extern s32 D_WSTAG935_800A7B64[];
extern s32 D_WSTAG935_800A7B74[];
extern FieldstgVramPlace wstag935_vram_places[];
extern FieldstgPlacedActor *wstag935_actors[];
extern FieldstgSprite wstag935_sprites[];
extern FieldstgMapEvent wstag935_map_events[];
extern FieldstgEventDef wstag935_events[];
void wstag935_menu_1616_update();
void wstag935_menu_1618_update();

void wstag935_update(WstagObject *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

WstagObject *wstag935_start(void *arg0) {
    WstagObject *obj = object_new(wstag935_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag935_funcs.setup();
    return obj;
}

void wstag935_setup(void) {
    fieldstg_stage.background_file = 0x1AE;
    fieldstg_stage.sprite_file = 0x08FD0000;
    fieldstg_stage.sprites = wstag935_sprites;
    fieldstg_stage.map_events = wstag935_map_events;
    fieldstg_stage.mask_file = 0x8FC;
    fieldstg_stage.talk_file = records_language + 0xFD;
    fieldstg_stage.start_pos = (GamestatePos){ 0x1A400, 0xC800 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag935_vram_places;
    fieldstg_stage.music = 8;
    fieldstg_stage.sound = 0x60200000;
    fieldstg_stage.actors = wstag935_actors;
    fieldstg_stage.events = wstag935_events;
    fieldstg_attr.set_file(0, 0x08FD0001);
    fieldstg_attr.set_file(7, 0x08FD0002);
    fieldstg_attr.init_layer(0);
}

void wstag935_fade_start(WindowAnim *fade, s32 in) {
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

s32 wstag935_fade_update(WindowAnim *fade) {
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

void wstag935_menu_1616_update(WstagMenu *obj, WstagMenuData *data) {
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
            wstag935_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag935_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < obj->count; i++) {
                    data->items[i]->set_text(data->items[i], cdload_module.get_subfile_by_id(0x1580008 + (records_language << 16)),
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
            wstag935_funcs.fade_start(&obj->text_fade, 1);
            obj->base.step++;
            break;
        case 4:
            if (wstag935_funcs.fade_update(&obj->text_fade) != 0) {
                data->text->set_text(data->text, cdload_module.get_subfile_by_id(0x1580008 + (records_language << 16)), obj->cursor + 9);
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
            wstag935_funcs.fade_start(&obj->text_fade, 0);
            data->text->set_visible(data->text, 0);
            obj->base.step++;
            break;
        case 7:
            if (wstag935_funcs.fade_update(&obj->text_fade) != 0) {
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
            wstag935_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag935_funcs.fade_update(&obj->fade) != 0) {
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
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), D_WSTAG935_800A7B64[obj->count - 5], 0xA8, 0x18);
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
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag935_event_1616_start(void) {
    return object_new(wstag935_menu_1616_update, 0x84, 0x28);
}

void wstag935_menu_1618_update(WstagMenu *obj, WstagMenuData *data) {
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
            wstag935_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (wstag935_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < obj->count; i++) {
                    data->items[i]->set_text(data->items[i], cdload_module.get_subfile_by_id(0x1580009 + (records_language << 16)),
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
            wstag935_funcs.fade_start(&obj->text_fade, 1);
            obj->base.step++;
            break;
        case 4:
            if (wstag935_funcs.fade_update(&obj->text_fade) != 0) {
                data->text->set_text(data->text, cdload_module.get_subfile_by_id(0x1580009 + (records_language << 16)), obj->cursor + 9);
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
            wstag935_funcs.fade_start(&obj->text_fade, 0);
            data->text->set_visible(data->text, 0);
            obj->base.step++;
            break;
        case 7:
            if (wstag935_funcs.fade_update(&obj->text_fade) != 0) {
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
            wstag935_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (wstag935_funcs.fade_update(&obj->fade) != 0) {
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
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), D_WSTAG935_800A7B74[obj->count - 5], 0xA8, 0x18);
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
        wstag935_funcs.fade_start(&obj->fade, 1);
        wstag935_funcs.fade_update(&obj->fade);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Object *wstag935_event_1618_start(void) {
    return object_new(wstag935_menu_1618_update, 0x84, 0x28);
}

/* The stage's .data (tools/wstag_data.py). */
void wstag935_setup(void);

FieldstgVramPlace wstag935_vram_places[15] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 320, 472, 0, 216, 352, 507 }, { 384, 256, 432, 345, 448, 89, 368, 507 },
    { 384, 256, 416, 349, 384, 93, 336, 506 }, { 384, 256, 424, 349, 416, 93, 352, 506 },
    { 384, 256, 400, 369, 320, 113, 368, 506 }, { 384, 256, 408, 369, 352, 113, 320, 505 },
    { 384, 256, 436, 256, 464, 0, 336, 505 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG935_800A74B8[4] = { 0x9073, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A74C0[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A74CC[6] = { 0x11, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A74D8[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A74E4[8] = { 0x11, 0, 0x10, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A74F4[6] = { 0x11, 0, 3, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7500[4] = { 3, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7508[6] = { 0x11, 0, 3, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7514[4] = { 0x7824, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A751C[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7528[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7534[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7540[8] = { 0x11, 0, 0x10, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7550[6] = { 0x11, 0, 1, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A755C[4] = { 1, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7564[6] = { 0x11, 0, 1, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7570[4] = { 0x782D, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7578[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7584[6] = { 0x11, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7590[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A759C[8] = { 0x11, 0, 0x10, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A75AC[6] = { 0x11, 0, 2, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A75B8[4] = { 2, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A75C0[6] = { 2, 1, 0x11, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A75CC[4] = { 0x7844, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A75D4[4] = { 0x9072, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A75DC[6] = { 0x11, 1, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A75E8[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A75F4[6] = { 0x11, 1, 0x10, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7600[8] = { 0x11, 0, 0, 0, 0x10, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7610[6] = { 0x11, 0, 0, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A761C[4] = { 0, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7624[6] = { 0x11, 0, 0, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7630[4] = { 0x7834, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7638[4] = { 0x7A43, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG935_800A7640[2] = { { NULL, D_WSTAG935_800A74B8, 67 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A7658[2] = { { NULL, NULL, 81 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A7670[5] = {
    { D_WSTAG935_800A74C0, D_WSTAG935_800A74CC, 83 }, { D_WSTAG935_800A74D8, D_WSTAG935_800A74E4, 84 },
    { D_WSTAG935_800A74F4, D_WSTAG935_800A7500, 81 }, { D_WSTAG935_800A7508, D_WSTAG935_800A7514, 82 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG935_800A76AC[2] = { { NULL, NULL, 73 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A76C4[5] = {
    { D_WSTAG935_800A751C, D_WSTAG935_800A7528, 75 }, { D_WSTAG935_800A7534, D_WSTAG935_800A7540, 76 },
    { D_WSTAG935_800A7550, D_WSTAG935_800A755C, 73 }, { D_WSTAG935_800A7564, D_WSTAG935_800A7570, 74 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG935_800A7700[2] = { { NULL, NULL, 77 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A7718[5] = {
    { D_WSTAG935_800A7578, D_WSTAG935_800A7584, 79 }, { D_WSTAG935_800A7590, D_WSTAG935_800A759C, 80 },
    { D_WSTAG935_800A75AC, D_WSTAG935_800A75B8, 77 }, { D_WSTAG935_800A75C0, D_WSTAG935_800A75CC, 78 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG935_800A7754[2] = { { NULL, D_WSTAG935_800A75D4, 68 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A776C[2] = { { NULL, NULL, 69 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A7784[5] = {
    { D_WSTAG935_800A75DC, D_WSTAG935_800A75E8, 71 }, { D_WSTAG935_800A75F4, D_WSTAG935_800A7600, 72 },
    { D_WSTAG935_800A7610, D_WSTAG935_800A761C, 69 }, { D_WSTAG935_800A7624, D_WSTAG935_800A7630, 70 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG935_800A77C0[2] = { { NULL, D_WSTAG935_800A7638, 66 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG935_800A77D8[2] = { { NULL, NULL, 137 }, { NULL, NULL, 0 } };
u16 D_WSTAG935_800A77F0[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A77F8[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7800[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7808[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7810[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7818[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7820[4] = { 0x8192, 0, 0xFFFF, 0 };
u16 D_WSTAG935_800A7828[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7830[4] = { 0x8192, 1, 0xFFFF, 0 };
u16 D_WSTAG935_800A7838[4] = { 0x8192, 0, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG935_800A7840 = { NULL, D_WSTAG935_800A7640, 46, 4, 255, 176, 3 };
FieldstgPlacedActor D_WSTAG935_800A7854 = { D_WSTAG935_800A77F0, D_WSTAG935_800A7658, 47, 5, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG935_800A7868 = { D_WSTAG935_800A77F8, D_WSTAG935_800A7670, 47, 5, 173, 231, 7 };
FieldstgPlacedActor D_WSTAG935_800A787C = { D_WSTAG935_800A7800, D_WSTAG935_800A76AC, 49, 6, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG935_800A7890 = { D_WSTAG935_800A7808, D_WSTAG935_800A76C4, 49, 6, 198, 219, 7 };
FieldstgPlacedActor D_WSTAG935_800A78A4 = { D_WSTAG935_800A7810, D_WSTAG935_800A7700, 50, 7, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG935_800A78B8 = { D_WSTAG935_800A7818, D_WSTAG935_800A7718, 50, 7, 224, 257, 3 };
FieldstgPlacedActor D_WSTAG935_800A78CC = { NULL, D_WSTAG935_800A7754, 51, 8, 348, 147, 1 };
FieldstgPlacedActor D_WSTAG935_800A78E0 = { D_WSTAG935_800A7820, D_WSTAG935_800A776C, 52, 9, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG935_800A78F4 = { D_WSTAG935_800A7828, D_WSTAG935_800A7784, 52, 9, 250, 244, 3 };
FieldstgPlacedActor D_WSTAG935_800A7908 = { D_WSTAG935_800A7830, D_WSTAG935_800A77C0, 206, 10, 379, 147, 7 };
FieldstgPlacedActor D_WSTAG935_800A791C = { D_WSTAG935_800A7838, D_WSTAG935_800A77D8, 206, 10, 379, 147, 7 };
FieldstgPlacedActor D_WSTAG935_800A7930 = { NULL, NULL, 220, 11, 379, 163, 7 };
FieldstgPlacedActor D_WSTAG935_800A7944 = { NULL, NULL, 225, 12, 347, 158, 3 };
FieldstgPlacedActor *wstag935_actors[15] = {
    &D_WSTAG935_800A7840, &D_WSTAG935_800A7854, &D_WSTAG935_800A7868, &D_WSTAG935_800A787C, &D_WSTAG935_800A7890,
    &D_WSTAG935_800A78A4, &D_WSTAG935_800A78B8, &D_WSTAG935_800A78CC, &D_WSTAG935_800A78E0, &D_WSTAG935_800A78F4,
    &D_WSTAG935_800A7908, &D_WSTAG935_800A791C, &D_WSTAG935_800A7930, &D_WSTAG935_800A7944, NULL,
};
FieldstgSprite wstag935_sprites[19] = {
    { 1, 0, 0x40, 6, 0x3B, 2, 0, 5, 8, 0, 428, 75, 0, 0 }, { 1, 0, 0x40, 6, 0x3D, 2, 0, 3, 8, 0, 65, 176, 0, 0 },
    { 1, 0, 0x40, 6, 0x3A, 2, 0, 3, 8, 0, 425, 67, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 8, 0, 428, 75, 0, 0 },
    { 1, 0, 0x40, 4, 0x3C, 2, 0, 3, 8, 0, 470, 149, 176, 0 }, { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 86, 239, 258, 0 },
    { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 160, 220, 248, 0 }, { 1, 0, 0x40, 4, 2, 0, 0, 0, 0, 0, 192, 204, 232, 0 },
    { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 320, 201, 231, 0 }, { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 291, 185, 209, 0 },
    { 1, 0, 0x40, 4, 5, 0, 0, 0, 0, 0, 352, 147, 168, 0 }, { 1, 0, 0x40, 4, 6, 0, 0, 0, 0, 0, 371, 140, 160, 0 },
    { 1, 0, 0x40, 4, 7, 0, 0, 0, 0, 0, 336, 139, 160, 0 }, { 1, 0, 0x40, 4, 8, 0, 0, 0, 0, 0, 385, 131, 153, 0 },
    { 1, 0, 0x40, 4, 9, 0, 0, 0, 0, 0, 320, 131, 153, 0 }, { 1, 0, 0x40, 4, 0xA, 0, 0, 0, 0, 0, 401, 123, 144, 0 },
    { 1, 0, 0x40, 4, 0xB, 0, 0, 0, 0, 0, 304, 117, 144, 0 },
    { 1, 0, 0x40, 4, 0xC, 0, 0, 0, 0, 0, 464, 149, 176, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag935_map_events[2] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x271, 0x1F0, 0xA0, 7, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFadeFuncs wstag935_funcs = { wstag935_setup, wstag935_fade_start, wstag935_fade_update };
FieldstgEventDef wstag935_events[3] = {
    { 1616, NULL, 0x01580008, (s32 (*)(void))wstag935_event_1616_start, NULL },
    { 1618, NULL, 0x01580009, (s32 (*)(void))wstag935_event_1618_start, NULL }, { -1, NULL, 0, NULL, NULL },
};
s32 D_WSTAG935_800A7B64[4] = { 28, 27, 25, 36 };
s32 D_WSTAG935_800A7B74[4] = { 28, 27, 25, 36 };
