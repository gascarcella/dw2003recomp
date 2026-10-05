#include "common.h"
#include "object.h"
#include "gamestate.h"
#include "heap.h"
#include "gfx.h"
#include "sound.h"
#include "cdload.h"
#include "pad.h"
#include "message.h"
#include "records.h"
#include "fieldstg.h"

void fieldstg_lift_update();
void fieldstg_choice_update();
void fieldstg_stage_update();

s16 fieldstg_lift_steps[10] = {
    1, 2, 1, 0, -1, -2, -1, 0,
    1000, 0,
};

/* The lift of stage 528 (script object 826: fieldstg_lift_update, created by fieldstg_lift_create): moves two
 * entries of fieldstg_stage.sprites (types 3 and 2) and the player together by 0x7F. Messages 0x348/0x349
 * start it (state 2) up/down; flag 0x1C3D: it starts down. Sub-states of state 2: 0 start, 1 wait 30 frames,
 * 2 bounce (fieldstg_lift_steps), 3 move one pixel every other frame, 4 bounce, 5 stop. */
typedef struct FieldstgLift {
    /* 0x00 */ Object base;
    /* 0x50 */ FieldstgSprite *sprite_3; /* the entry of type 3 */
    /* 0x54 */ FieldstgSprite *sprite_2; /* the entry of type 2 */
    /* 0x58 */ s16 down; /* the next move goes down */
    /* 0x5A */ s16 wait; /* frames waited */
    /* 0x5C */ s16 step; /* index into fieldstg_lift_steps, then frames moved */
    /* 0x5E */ u8 unk_5E[0x2];
    /* 0x60 */ s16 start_y_3; /* sprite_3's y at the start of the move */
    /* 0x62 */ s16 start_y_2; /* sprite_2's */
    /* 0x64 */ s32 start_player_y; /* the player's (24.8) */
    /* 0x68 */ s16 low_y_3; /* sprite_3's y in the data: the lower position */
    /* 0x6A */ s16 low_y_2; /* sprite_2's */
} FieldstgLift; /* size 0x6C */

/* A question with two answers, each starting an event (fieldstg_choice_update, created by fieldstg_choice_start_0
 * ... fieldstg_choice_start_15, the callbacks of stage 528's events). States: 0 create the windows, 1 sub-states:
 * 0/1 fade in and show the texts, 2 move the cursor until the button, 10/11 close and fade out, 3 start the
 * answer's event, 4 wait for it, then end. */
typedef struct FieldstgChoice {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 index; /* 0..15: which creator made it, index into fieldstg_choices */
    /* 0x54 */ s32 cursor; /* answer (cursor) */
    /* 0x58 */ WindowAnim fade; /* fade (fieldstg_stage_funcs); its level scales the picture */
} FieldstgChoice; /* size 0x68 */

/* Data block of FieldstgChoice. */
typedef struct FieldstgChoiceData {
    /* 0x00 */ MessageWindow *question; /* the question */
    /* 0x04 */ MessageWindow *answers[2]; /* the answers */
    /* 0x0C */ MessageCursor *cursor; /* cursor */
    /* 0x10 */ FieldstgEvent *event; /* the event runner */
} FieldstgChoiceData; /* size 0x14 */

/* Entry of fieldstg_choices (one per FieldstgChoice.index). */
typedef struct FieldstgChoiceText {
    /* 0x0 */ s32 text; /* text sub-file ID (language added to the file ID) */
    /* 0x4 */ s16 first_event; /* event of the first answer */
    /* 0x6 */ s16 second_event; /* event of the second answer */
} FieldstgChoiceText; /* size 0x8 */

FieldstgChoiceText fieldstg_choices[16] = {
    { 0x1190002, 1333, 1334 }, { 0x1190005, 1338, 1339 }, { 0x1190009, 1343, 1344 }, { 0x119000D, 1348, 1349 },
    { 0x1190011, 1353, 1354 }, { 0x1190015, 1358, 1359 }, { 0x1190019, 1363, 1364 }, { 0x119001D, 1368, 1369 },
    { 0x1190021, 1373, 1374 }, { 0x1190025, 1378, 1379 }, { 0x1190029, 1383, 1384 }, { 0x119002D, 1388, 1389 },
    { 0x1190031, 1393, 1394 }, { 0x1190035, 1398, 1399 }, { 0x1190039, 1403, 1404 }, { 0x119003D, 1408, 1409 },
};

/* The object of stage 528, the one stage whose code is in FIELDSTG (fieldstg_stage_update, created by
 * fieldstg_stage_entry, its entry in fieldstg_stages; fieldstg_stage_funcs.setup is its setup): shows sprite
 * type 1 at some story points, creates the lift and starts the story event of fieldstg_flag_events. */
typedef struct FieldstgStage {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 manager; /* the entry's argument (the field manager) */
    /* 0x54 */ s32 next_event;
} FieldstgStage; /* size 0x58 */

/* An entry of fieldstg_flag_events (ended by progress == -1): the event that starts at a story point when two
 * flags are set. */
typedef struct FieldstgFlagEvent {
    /* 0x0 */ s32 progress; /* gamestate_data.progress */
    /* 0x4 */ s32 flag_0; /* flag that must be set (type 0) */
    /* 0x8 */ s32 flag_1; /* flag that must be set (type 1) */
    /* 0xC */ s16 event; /* event */
    /* 0xE */ s16 next_event; /* event started on the next update */
} FieldstgFlagEvent; /* size 0x10 */

FieldstgFlagEvent fieldstg_flag_events[17] = {
    { 5, 1024, 28788, 1331, 1332 }, { 8, 1025, 28788, 1336, 1337 }, { 12, 1026, 28789, 1341, 1342 },
    { 14, 1027, 28789, 1346, 1347 }, { 16, 1028, 28790, 1351, 1352 }, { 22, 1029, 28790, 1356, 1357 },
    { 24, 1030, 28791, 1361, 1362 }, { 26, 1031, 28791, 1366, 1367 }, { 28, 1032, 28792, 1371, 1372 },
    { 30, 1033, 28792, 1376, 1377 }, { 31, 1034, 28793, 1381, 1382 }, { 34, 1035, 28793, 1386, 1387 },
    { 36, 1036, 28794, 1391, 1392 }, { 37, 1037, 28795, 1396, 1397 }, { 38, 1038, 28796, 1401, 1402 },
    { 39, 1039, 28797, 1406, 1407 }, { -1, 0, 0, 0, 0 },
};

/* Data block of FieldstgStage. */
typedef struct FieldstgStageData {
    /* 0x0 */ FieldstgLift *lift;
    /* 0x4 */ FieldstgEvent *event; /* the event runner */
} FieldstgStageData; /* size 0x8 */

void fieldstg_lift_update(FieldstgLift *obj) {
    FieldstgSprite *entry;
    FieldstgSprite *first;
    FieldstgSprite *second;
    FieldstgActor *player;
    s32 step;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (entry = fieldstg_stage.sprites; entry->present != 0; entry++) {
            switch (entry->type) {
            case 2:
                obj->sprite_2 = entry;
                obj->low_y_2 = entry->y;
                if (obj->down != 0) {
                    entry->y -= 0x7F;
                }
                entry->shown = 0;
                break;
            case 3:
                obj->sprite_3 = entry;
                obj->low_y_3 = entry->y;
                if (obj->down != 0) {
                    entry->y -= 0x7F;
                }
                entry->shown = 1;
                break;
            }
        }
        obj->down = 0;
        break;
    case OBJECT_STATE_RUN:
        break;
    case OBJECT_STATE_DONE:
        first = obj->sprite_3;
        second = obj->sprite_2;
        player = (FieldstgActor *)heap_objects.find(5, -1, 0);
        switch (obj->base.step) {
        case 0:
        default:
            second->shown = 1;
            obj->wait = 0;
            obj->start_y_3 = first->y;
            obj->start_y_2 = second->y;
            obj->start_player_y = player->pos.y;
            sound_module.play(0x8004103C);
            obj->base.next_step(obj);
            break;
        case 1:
            obj->wait += gfx_module.funcs.get_frame_ticks();
            if (obj->wait >= 30) {
                obj->step = 0;
                obj->base.next_step(obj);
                sound_module.play(0x01080001);
            }
            break;
        case 2:
        case 4:
            step = fieldstg_lift_steps[obj->step];
            if (step != 1000) {
                first->y = obj->start_y_3 + step;
                second->y = obj->start_y_2 + step;
                player->pos.y = obj->start_player_y + step;
                obj->step++;
            } else {
                obj->base.next_step(obj);
                obj->step = 0;
            }
            break;
        case 3:
            obj->step++;
            if (obj->step >= 0xFE) {
                if (obj->down != 0) {
                    first->y = obj->low_y_3;
                    second->y = obj->low_y_2;
                    player->pos.y = obj->start_player_y + 0x7F00;
                } else {
                    first->y = obj->low_y_3 - 0x7F;
                    second->y = obj->low_y_2 - 0x7F;
                    player->pos.y = obj->start_player_y - 0x7F00;
                }
                obj->start_y_3 = first->y;
                obj->start_y_2 = second->y;
                obj->start_player_y = player->pos.y;
                obj->base.next_step(obj);
                obj->step = 0;
            } else if (obj->step & 1) {
                if (obj->down != 0) {
                    first->y++;
                    second->y++;
                    player->pos.y += 0x100;
                } else {
                    first->y--;
                    second->y--;
                    player->pos.y -= 0x100;
                }
            }
            break;
        case 5:
            second->shown = 0;
            obj->base.set_state(obj, OBJECT_STATE_RUN);
            obj->down ^= 1;
            break;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void fieldstg_lift_message(FieldstgLift *obj, s32 cmd) {
    if (obj != NULL) {
        switch (cmd) {
        case 0x348:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            obj->down = 0;
            break;
        case 0x349:
            obj->base.set_state(obj, OBJECT_STATE_DONE);
            obj->down = 1;
            break;
        }
    }
}

FieldstgLift *fieldstg_lift_create(s32 id) {
    FieldstgLift *obj = object_create(fieldstg_lift_update, sizeof(FieldstgLift), 0, id);

    if (gamestate_flags.get_flag(0x1C3D, 1) != 0) {
        obj->down = 1;
    } else {
        obj->down = 0;
    }
    return obj;
}

void fieldstg_choice_update(FieldstgChoice *obj, FieldstgChoiceData *data) {
    Sprite spr;
    s32 prev;
    s32 event;
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
            fieldstg_stage_funcs.fade_start(&obj->fade, 1);
            obj->base.step++;
            break;
        case 1:
            if (fieldstg_stage_funcs.fade_update(&obj->fade) != 0) {
                for (i = 0; i < 2; i++) {
                    data->answers[i]->set_text(data->answers[i],
                                             cdload_module.get_subfile_by_id(fieldstg_choices[obj->index].text + (records_language << 16)),
                                             i + 2);
                }
                data->cursor->show(data->cursor, 1);
                data->question->set_text(data->question,
                                      cdload_module.get_subfile_by_id(fieldstg_choices[obj->index].text + (records_language << 16)), 1);
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
            }
            break;
        case 3:
            if (obj->cursor == 0) {
                event = fieldstg_choices[obj->index].first_event;
            } else {
                event = fieldstg_choices[obj->index].second_event;
            }
            data->event = fieldstg_event_start(event);
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
            fieldstg_stage_funcs.fade_start(&obj->fade, 0);
            obj->base.step++;
            break;
        case 11:
            if (fieldstg_stage_funcs.fade_update(&obj->fade) != 0) {
                obj->base.step = 3;
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

void fieldstg_choice_start_0(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 0;
}

void fieldstg_choice_start_1(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 1;
}

void fieldstg_choice_start_2(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 2;
}

void fieldstg_choice_start_3(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 3;
}

void fieldstg_choice_start_4(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 4;
}

void fieldstg_choice_start_5(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 5;
}

void fieldstg_choice_start_6(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 6;
}

void fieldstg_choice_start_7(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 7;
}

void fieldstg_choice_start_8(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 8;
}

void fieldstg_choice_start_9(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 9;
}

void fieldstg_choice_start_10(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 10;
}

void fieldstg_choice_start_11(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 11;
}

void fieldstg_choice_start_12(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 12;
}

void fieldstg_choice_start_13(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 13;
}

void fieldstg_choice_start_14(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 14;
}

void fieldstg_choice_start_15(void) {
    FieldstgChoice *obj = object_new(fieldstg_choice_update, sizeof(FieldstgChoice), 0x14);

    obj->index = 15;
}

void fieldstg_stage_update(FieldstgStage *obj, FieldstgStageData *data) {
    FieldstgSprite *entry;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->next_event = 0;
        for (entry = fieldstg_stage.sprites; entry->present != 0; entry++) {
            if (entry->type == 1) {
                break;
            }
        }
        switch (gamestate_data.progress) {
        case 5:
        case 8:
        case 12:
        case 14:
        case 16:
        case 22:
        case 24:
        case 26:
        case 28:
        case 30:
        case 31:
        case 34:
        case 36:
        case 37:
        case 38:
        case 39:
            entry->shown = 1;
            break;
        default:
            entry->shown = 0;
            break;
        }
        data->lift = fieldstg_lift_create(0x33A);
        for (i = 0; fieldstg_flag_events[i].progress != -1; i++) {
            if (gamestate_data.progress == fieldstg_flag_events[i].progress
                && gamestate_flags.get_flag(fieldstg_flag_events[i].flag_0, 0) != 0
                && gamestate_flags.get_flag(fieldstg_flag_events[i].flag_1, 1) != 0) {
                data->event = fieldstg_event_start(fieldstg_flag_events[i].event);
                obj->next_event = fieldstg_flag_events[i].next_event;
                break;
            }
        }
        break;
    case OBJECT_STATE_RUN:
        if (data->event == NULL && obj->next_event != 0) {
            data->event = fieldstg_event_start(obj->next_event);
            obj->next_event = 0;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FieldstgStage *fieldstg_stage_entry(s32 manager) {
    FieldstgStage *obj = object_new(fieldstg_stage_update, sizeof(FieldstgStage), 8);

    obj->manager = manager;
    fieldstg_stage_funcs.setup();
    return obj;
}
