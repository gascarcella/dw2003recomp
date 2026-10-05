#include "wstag.h"
#include "pad.h"

/* WSTAG220: stage 0x206 (fieldstg_stages). */

extern WstagFuncs wstag220_funcs;
extern const CVECTOR wstag220_color;
extern FieldstgVramPlace wstag220_vram_places[];
extern FieldstgPlacedActor *wstag220_actors[];
extern FieldstgSprite wstag220_sprites[];
extern FieldstgMapEvent wstag220_map_events[];
extern FieldstgEventDef wstag220_events[];
extern void (*D_WSTAG220_800A81B8)(WindowAnim *fade, s32 in);
extern s32 (*D_WSTAG220_800A81BC)(WindowAnim *fade);
void wstag220_choice_1512_update();
void wstag220_choice_1514_update();
void wstag220_choice_1516_update();

void wstag220_choice_1512_update(WstagChoice *obj, WstagChoiceData *data) {
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
            D_WSTAG220_800A81B8(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120036 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120036 + (records_language << 16)), 1);
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
                data->event = fieldstg_event_start(0x35);
            } else {
                data->event = fieldstg_event_start(0x5E9);
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
            D_WSTAG220_800A81B8(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
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

Object *wstag220_event_1512_start(void) {
    return object_new(wstag220_choice_1512_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag220_choice_1514_update(WstagChoice *obj, WstagChoiceData *data) {
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
            D_WSTAG220_800A81B8(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120037 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120037 + (records_language << 16)), 1);
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
                data->event = fieldstg_event_start(0x37);
            } else {
                data->event = fieldstg_event_start(0x5EB);
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
            D_WSTAG220_800A81B8(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
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

Object *wstag220_event_1514_start(void) {
    return object_new(wstag220_choice_1514_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag220_choice_1516_update(WstagChoice *obj, WstagChoiceData *data) {
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
            D_WSTAG220_800A81B8(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i], cdload_module.get_subfile_by_id(0x1120038 + (records_language << 16)),
                                              i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question, cdload_module.get_subfile_by_id(0x1120038 + (records_language << 16)), 1);
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
                data->event = fieldstg_event_start(0x38);
            } else {
                data->event = fieldstg_event_start(0x5ED);
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
            D_WSTAG220_800A81B8(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (D_WSTAG220_800A81BC(&obj->fade) != 0) {
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

Object *wstag220_event_1516_start(void) {
    return object_new(wstag220_choice_1516_update, sizeof(WstagChoice), sizeof(WstagChoiceData));
}

void wstag220_update(WstagObject *obj) {
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

WstagObject *wstag220_start(void *arg0) {
    WstagObject *obj = object_new(wstag220_update, sizeof(WstagObject), 0);

    obj->manager = arg0;
    wstag220_funcs.setup();
    return obj;
}

void wstag220_setup(void) {
    fieldstg_stage.background_file = 0x19C;
    fieldstg_stage.sprites = wstag220_sprites;
    fieldstg_stage.map_events = wstag220_map_events;
    fieldstg_stage.sprite_file = 0x019D0001;
    fieldstg_stage.mask_file = 0x2D6;
    fieldstg_stage.talk_file = records_language + 0xE8;
    fieldstg_stage.start_pos = (GamestatePos){ 0x10200, 0x15700 };
    fieldstg_stage.start_dir = 0;
    fieldstg_stage.vram_places = wstag220_vram_places;
    fieldstg_stage.music = 0x33;
    fieldstg_stage.sound = 0x60CC0000;
    fieldstg_stage.actors = wstag220_actors;
    fieldstg_stage.color = wstag220_color;
    fieldstg_stage.events = wstag220_events;
    fieldstg_attr.set_file(0, 0x019D0000);
    fieldstg_attr.set_file(7, 0x019D0002);
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

void wstag220_fade_start(WindowAnim *fade, s32 in) {
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

s32 wstag220_fade_update(WindowAnim *fade) {
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

const CVECTOR wstag220_color = { 0x54, 0x67, 0x96, 0 };

/* The stage's .data (tools/wstag_data.py). */
void wstag220_setup(void);

s16 D_WSTAG220_800A7330[67] = {
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 36, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 4, 32, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 7, 36, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG220_800A73B8[64] = {
    FIELDSTG_EVENT_WALK(2, 186, 248, 5),
    FIELDSTG_EVENT_PLACE(32, 220, 232),
    FIELDSTG_EVENT_ANIM(32, 1, 1),
    FIELDSTG_EVENT_ANIM(0x32D, 823, 2),
    FIELDSTG_EVENT_WAIT_WALK(2),
    FIELDSTG_EVENT_ANIM(2, 1, 5),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 824, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 2, 0x32D, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 3, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG220_800A7438[59] = {
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
s16 D_WSTAG220_800A74B0[67] = {
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
    FIELDSTG_EVENT_DIALOG(0, 5, 2, 4),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_ANIM(0x32D, 825, 2),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_DIALOG(0, 6, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG220_800A7538[11] = {
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 36, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG220_800A7550[11] = {
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
s16 D_WSTAG220_800A7568[11] = {
    FIELDSTG_EVENT_WAIT(60),
    FIELDSTG_EVENT_DIALOG(0, 1, 32, 2),
    FIELDSTG_EVENT_WAIT_DIALOG,
    FIELDSTG_EVENT_WAIT(30),
    FIELDSTG_EVENT_END,
};
FieldstgVramPlace wstag220_vram_places[20] = {
    { 512, 256, 540, 422, 112, 166, 560, 510 }, { 512, 256, 512, 256, 0, 0, 544, 510 },
    { 512, 256, 534, 312, 88, 56, 512, 509 }, { 512, 256, 520, 444, 32, 188, 528, 509 },
    { 512, 256, 528, 444, 64, 188, 544, 509 }, { 512, 256, 512, 444, 0, 188, 560, 509 },
    { 320, 256, 366, 376, 184, 120, 336, 503 }, { 320, 256, 374, 376, 216, 120, 352, 503 },
    { 320, 256, 320, 382, 0, 126, 368, 503 }, { 320, 256, 352, 384, 128, 128, 336, 502 },
    { 320, 256, 328, 382, 32, 126, 352, 502 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 256, 336, 414, 64, 158, 368, 502 }, { 320, 256, 344, 416, 96, 160, 320, 501 },
    { 320, 256, 358, 416, 152, 160, 336, 501 }, { 320, 256, 366, 416, 184, 160, 352, 501 },
    { 320, 256, 374, 416, 216, 160, 368, 501 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 },
};
u16 D_WSTAG220_800A76C0[4] = { 0x1A06, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A76C8[4] = { 0x1A06, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A76D0[6] = { 0x1A06, 1, 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A76DC[4] = { 0x9007, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A76E4[6] = { 0x1A06, 1, 0x700D, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A76F0[4] = { 0x9008, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A76F8[4] = { 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7700[4] = { 0x9007, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7708[4] = { 0x700D, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7710[4] = { 0x9008, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7718[4] = { 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7720[4] = { 0x9007, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7728[4] = { 0x700D, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7730[4] = { 0x9008, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7738[4] = { 0x1A07, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7740[4] = { 0x1A07, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7748[4] = { 0x1A07, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7750[4] = { 0x9005, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7758[4] = { 0x9005, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7760[4] = { 0x700D, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7768[4] = { 0x9005, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7770[4] = { 0x1A04, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7778[4] = { 0x1A04, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7780[4] = { 0x1A04, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7788[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7790[4] = { 0x7C00, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7798[4] = { 0x7C00, 1, 0xFFFF, 0 };
FieldstgTalk D_WSTAG220_800A77A0[4] = {
    { D_WSTAG220_800A76C0, D_WSTAG220_800A76C8, 2 }, { D_WSTAG220_800A76D0, D_WSTAG220_800A76DC, 1055 },
    { D_WSTAG220_800A76E4, D_WSTAG220_800A76F0, 1055 }, { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG220_800A77D0[3] = {
    { D_WSTAG220_800A76F8, D_WSTAG220_800A7700, 1060 }, { D_WSTAG220_800A7708, D_WSTAG220_800A7710, 1060 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG220_800A77F4[3] = {
    { D_WSTAG220_800A7718, D_WSTAG220_800A7720, 1055 }, { D_WSTAG220_800A7728, D_WSTAG220_800A7730, 1055 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG220_800A7818[3] = {
    { D_WSTAG220_800A7738, D_WSTAG220_800A7740, 3 }, { D_WSTAG220_800A7748, D_WSTAG220_800A7750, 1056 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG220_800A783C[2] = { { NULL, D_WSTAG220_800A7758, 1061 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A7854[2] = { { D_WSTAG220_800A7760, D_WSTAG220_800A7768, 1056 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A786C[2] = { { NULL, NULL, 1 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A7884[2] = { { NULL, NULL, 408 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A789C[2] = { { NULL, NULL, 399 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A78B4[2] = { { NULL, NULL, 400 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A78CC[2] = { { NULL, NULL, 401 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A78E4[2] = { { NULL, NULL, 402 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A78FC[2] = { { NULL, NULL, 403 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A7914[2] = { { NULL, NULL, 404 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A792C[2] = { { NULL, NULL, 405 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A7944[2] = { { NULL, NULL, 406 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A795C[3] = {
    { D_WSTAG220_800A7770, D_WSTAG220_800A7778, 4 }, { D_WSTAG220_800A7780, D_WSTAG220_800A7788, 241 },
    { NULL, NULL, 0 },
};
FieldstgTalk D_WSTAG220_800A7980[2] = { { NULL, D_WSTAG220_800A7790, 1062 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A7998[2] = { { NULL, NULL, 1057 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A79B0[2] = { { NULL, NULL, 1058 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A79C8[2] = { { NULL, D_WSTAG220_800A7798, 1059 }, { NULL, NULL, 0 } };
FieldstgTalk D_WSTAG220_800A79E0[2] = { { NULL, NULL, 407 }, { NULL, NULL, 0 } };
u16 D_WSTAG220_800A79F8[6] = { 0x7022, 1, 0x6016, 0, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A04[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A0C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A14[6] = { 0x6016, 0, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A20[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A28[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A30[4] = { 0x6004, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A38[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A40[4] = { 0x7015, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A48[4] = { 0x600C, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A50[4] = { 0x600E, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A58[4] = { 0x7016, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A60[4] = { 0x6016, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A68[4] = { 0x7018, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A70[4] = { 0x7019, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A78[4] = { 0x6026, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A80[6] = { 0x6016, 0, 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A8C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A94[4] = { 0x701E, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7A9C[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AA4[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AAC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AB4[4] = { 0x7022, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7ABC[4] = { 0x602B, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AC4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7ACC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AD4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7ADC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AE4[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AEC[4] = { 0x701A, 1, 0xFFFF, 0 };
u16 D_WSTAG220_800A7AF4[4] = { 0x701A, 1, 0xFFFF, 0 };
FieldstgPlacedActor D_WSTAG220_800A7AFC = { D_WSTAG220_800A79F8, D_WSTAG220_800A77A0, 32, 4, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B10 = { D_WSTAG220_800A7A04, D_WSTAG220_800A77D0, 32, 4, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B24 = { D_WSTAG220_800A7A0C, D_WSTAG220_800A77F4, 32, 4, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B38 = { D_WSTAG220_800A7A14, D_WSTAG220_800A7818, 36, 5, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B4C = { D_WSTAG220_800A7A20, D_WSTAG220_800A783C, 36, 5, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B60 = { D_WSTAG220_800A7A28, D_WSTAG220_800A7854, 36, 5, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG220_800A7B74 = { D_WSTAG220_800A7A30, D_WSTAG220_800A786C, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7B88 = { D_WSTAG220_800A7A38, D_WSTAG220_800A7884, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7B9C = { D_WSTAG220_800A7A40, D_WSTAG220_800A789C, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7BB0 = { D_WSTAG220_800A7A48, D_WSTAG220_800A78B4, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7BC4 = { D_WSTAG220_800A7A50, D_WSTAG220_800A78CC, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7BD8 = { D_WSTAG220_800A7A58, D_WSTAG220_800A78E4, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7BEC = { D_WSTAG220_800A7A60, D_WSTAG220_800A78FC, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7C00 = { D_WSTAG220_800A7A68, D_WSTAG220_800A7914, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7C14 = { D_WSTAG220_800A7A70, D_WSTAG220_800A792C, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7C28 = { D_WSTAG220_800A7A78, D_WSTAG220_800A7944, 50, 6, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7C3C = { D_WSTAG220_800A7A80, D_WSTAG220_800A795C, 67, 7, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG220_800A7C50 = { D_WSTAG220_800A7A8C, D_WSTAG220_800A7980, 67, 7, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG220_800A7C64 = { D_WSTAG220_800A7A94, NULL, 89, 8, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG220_800A7C78 = { D_WSTAG220_800A7A9C, NULL, 89, 8, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG220_800A7C8C = { D_WSTAG220_800A7AA4, NULL, 112, 9, 169, 224, 0 };
FieldstgPlacedActor D_WSTAG220_800A7CA0 = { D_WSTAG220_800A7AAC, NULL, 112, 9, 169, 224, 0 };
FieldstgPlacedActor D_WSTAG220_800A7CB4 = { D_WSTAG220_800A7AB4, NULL, 113, 10, 202, 241, 0 };
FieldstgPlacedActor D_WSTAG220_800A7CC8 = { D_WSTAG220_800A7ABC, NULL, 113, 10, 202, 241, 0 };
FieldstgPlacedActor D_WSTAG220_800A7CDC = { D_WSTAG220_800A7AC4, D_WSTAG220_800A7998, 157, 11, 187, 215, 1 };
FieldstgPlacedActor D_WSTAG220_800A7CF0 = { D_WSTAG220_800A7ACC, D_WSTAG220_800A79B0, 158, 12, 220, 232, 1 };
FieldstgPlacedActor D_WSTAG220_800A7D04 = { D_WSTAG220_800A7AD4, D_WSTAG220_800A79C8, 159, 13, 399, 248, 1 };
FieldstgPlacedActor D_WSTAG220_800A7D18 = { D_WSTAG220_800A7ADC, NULL, 160, 14, 300, 217, 5 };
FieldstgPlacedActor D_WSTAG220_800A7D2C = { D_WSTAG220_800A7AE4, D_WSTAG220_800A79E0, 161, 15, 65, 263, 7 };
FieldstgPlacedActor D_WSTAG220_800A7D40 = { D_WSTAG220_800A7AEC, NULL, 270, 16, 169, 224, 0 };
FieldstgPlacedActor D_WSTAG220_800A7D54 = { D_WSTAG220_800A7AF4, NULL, 271, 17, 202, 241, 0 };
FieldstgPlacedActor *wstag220_actors[32] = {
    &D_WSTAG220_800A7AFC, &D_WSTAG220_800A7B10, &D_WSTAG220_800A7B24, &D_WSTAG220_800A7B38, &D_WSTAG220_800A7B4C,
    &D_WSTAG220_800A7B60, &D_WSTAG220_800A7B74, &D_WSTAG220_800A7B88, &D_WSTAG220_800A7B9C, &D_WSTAG220_800A7BB0,
    &D_WSTAG220_800A7BC4, &D_WSTAG220_800A7BD8, &D_WSTAG220_800A7BEC, &D_WSTAG220_800A7C00, &D_WSTAG220_800A7C14,
    &D_WSTAG220_800A7C28, &D_WSTAG220_800A7C3C, &D_WSTAG220_800A7C50, &D_WSTAG220_800A7C64, &D_WSTAG220_800A7C78,
    &D_WSTAG220_800A7C8C, &D_WSTAG220_800A7CA0, &D_WSTAG220_800A7CB4, &D_WSTAG220_800A7CC8, &D_WSTAG220_800A7CDC,
    &D_WSTAG220_800A7CF0, &D_WSTAG220_800A7D04, &D_WSTAG220_800A7D18, &D_WSTAG220_800A7D2C, &D_WSTAG220_800A7D40,
    &D_WSTAG220_800A7D54, NULL,
};
FieldstgSprite wstag220_sprites[50] = {
    { 1, 0, 0x40, 2, 0x45, 2, 0, 2, 0xA, 0, 376, 237, 0, 0 }, { 1, 0, 0x40, 2, 5, 0, 0, 0, 0, 0, 368, 277, 0, 0 },
    { 1, 0, 0x40, 2, 6, 0, 0, 0, 0, 0, 0, 204, 0, 0 }, { 1, 0, 0x40, 2, 0x46, 2, 0, 1, 4, 0, 38, 197, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 110, 341, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 198, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 221, 148, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 235, 149, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 249, 297, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 271, 168, 0, 0 },
    { 1, 0, 0x40, 6, 0x1F, 1, 0x1F, 0x24, 8, 0, 398, 350, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 102, 333, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 200, 134, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 213, 145, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 242, 167, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 248, 230, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 254, 280, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 265, 153, 0, 0 },
    { 1, 0, 0x40, 6, 0x25, 1, 0x25, 0x2A, 8, 0, 391, 343, 0, 0 },
    { 1, 0, 0x40, 6, 0x2B, 0, 0, 0, 0, 0, 213, 151, 0, 0 }, { 1, 0, 0x40, 6, 0x2B, 0, 0, 0, 0, 0, 248, 286, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 104, 328, 0, 0 }, { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 197, 141, 0, 0 },
    { 1, 0, 0x40, 6, 0x2C, 0, 0, 0, 0, 0, 264, 158, 0, 0 }, { 1, 0, 0x40, 6, 0x2D, 0, 0, 0, 0, 0, 236, 159, 0, 0 },
    { 1, 0, 0x40, 6, 0x2D, 0, 0, 0, 0, 0, 391, 343, 0, 0 },
    { 1, 0, 0x40, 6, 0x32, 1, 0x32, 0x39, 0xE, 0, 318, 172, 0, 0 },
    { 1, 0, 0x40, 6, 0x2F, 1, 0x2F, 0x31, 4, 0, 305, 193, 0, 0 },
    { 1, 0, 0x40, 6, 0x3B, 1, 0x3B, 0x3F, 8, 0, 354, 209, 0, 0 },
    { 1, 0, 0x40, 6, 0x40, 2, 0, 9, 8, 0, 336, 217, 0, 0 },
    { 1, 0, 0x40, 6, 0x41, 2, 0, 2, 0x10, 0, 328, 232, 0, 0 },
    { 1, 0x64, 0x40, 6, 2, 0, 0, 0, 0, 0, 79, 186, 0, 0 }, { 1, 0, 0x40, 6, 0x42, 2, 0, 2, 6, 0, 136, 185, 0, 0 },
    { 1, 0, 0x40, 6, 0x43, 2, 0, 2, 0x10, 0, 306, 231, 0, 0 },
    { 1, 0, 0x40, 6, 0x44, 2, 0, 2, 0xA, 0, 321, 223, 0, 0 },
    { 1, 0, 0x40, 6, 0x47, 2, 0, 1, 4, 0, 140, 123, 0, 0 }, { 1, 0, 0x40, 6, 0x48, 2, 0, 1, 4, 0, 307, 122, 0, 0 },
    { 1, 0, 0x40, 6, 0x49, 2, 0, 1, 4, 0, 346, 144, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x4A, 1, 0x4A, 0x4C, 0xA, 0, 273, 297, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x56, 1, 0x56, 0x58, 0xA, 0, 215, 300, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x56, 1, 0x56, 0x58, 0xA, 0, 368, 358, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 92, 347, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 135, 312, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 150, 375, 0, 0 },
    { 1, 0, 0x40, 0xA, 0x59, 1, 0x59, 0x5B, 0xA, 0, 418, 326, 0, 0 },
    { 1, 0, 0x40, 4, 0, 0, 0, 0, 0, 0, 143, 287, 340, 0 }, { 1, 0, 0x40, 4, 1, 0, 0, 0, 0, 0, 326, 288, 340, 0 },
    { 1, 0, 0x40, 4, 4, 0, 0, 0, 0, 0, 398, 240, 263, 0 }, { 1, 0, 0x40, 4, 3, 0, 0, 0, 0, 0, 33, 201, 239, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
FieldstgMapEvent wstag220_map_events[3] = {
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x203, 0x2C8, 0x24C, 3, 0x64, 0, 0 },
    { 0xFFFF, 0, 0xFFFF, 0, 1, 0x200, 0x410, 0x200, 1, 0, 0, 0 }, { 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
WstagFuncs wstag220_funcs = { wstag220_setup };
void (*D_WSTAG220_800A81B8)(WindowAnim *fade, s32 in) = wstag220_fade_start;
s32 (*D_WSTAG220_800A81BC)(WindowAnim *fade) = wstag220_fade_update;
FieldstgEventDef wstag220_events[11] = {
    { 53, D_WSTAG220_800A7330, 0x0112000D, NULL, NULL }, { 54, D_WSTAG220_800A73B8, 0x0112000E, NULL, NULL },
    { 55, D_WSTAG220_800A7438, 0x0112000F, NULL, NULL }, { 56, D_WSTAG220_800A74B0, 0x01120010, NULL, NULL },
    { 1512, NULL, 0x01120036, (s32 (*)(void))wstag220_event_1512_start, NULL },
    { 1513, D_WSTAG220_800A7538, 0x01120031, NULL, NULL },
    { 1514, NULL, 0x01120037, (s32 (*)(void))wstag220_event_1514_start, NULL },
    { 1515, D_WSTAG220_800A7550, 0x01120032, NULL, NULL },
    { 1516, NULL, 0x01120038, (s32 (*)(void))wstag220_event_1516_start, NULL },
    { 1517, D_WSTAG220_800A7568, 0x01120033, NULL, NULL },
    { -1, NULL, 0, NULL, NULL },
};
