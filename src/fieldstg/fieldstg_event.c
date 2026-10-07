#include "common.h"
#include "object.h"
#include "heap.h"
#include "cdload.h"
#include "records.h"
#include "fieldstg.h"

void fieldstg_event_update();

/* Data block of FieldstgEvent (its children). */
typedef struct FieldstgEventData {
    /* 0x00 */ Object *started; /* what the event definition's start returned: a child object the event waits for */
    /* 0x04 */ struct FieldstgDialog *dialogs[3]; /* message windows (fieldstg_dialog_create) */
    /* 0x10 */ Object *script_objects[10]; /* script objects the script started (fieldstg_start_script_object) */
} FieldstgEventData; /* size 0x38 */

FieldstgActor *fieldstg_event_find_actor(FieldstgEvent *obj, s32 id) {
    s32 i;

    if (id < 0x320) {
        for (i = 0; i < 30; i++) {
            if (obj->actors[i].id == 0) {
                break;
            }
            if (obj->actors[i].id == id) {
                return obj->actors[i].actor;
            }
        }
    }
    return NULL;
}

s32 fieldstg_event_send_message(FieldstgEvent *obj, s16 *cmd, FieldstgEventData *data) {
    s32 id = cmd[1];
    s32 arg1 = cmd[2];
    s32 arg2 = cmd[3];
    FieldstgActor *actor;
    Object *other;
    s32 i;

    if (id < 0x320) {
        actor = fieldstg_event_find_actor(obj, id);
        if (actor != NULL) {
            actor->play_anim(actor, arg1, arg2);
        }
    } else {
        other = heap_objects.find(id, -1, -1);
        if (other == NULL) {
            for (i = 0; i < 10; i++) {
                if (data->script_objects[i] == NULL) {
                    data->script_objects[i] = fieldstg_start_script_object(id);
                    if (data->script_objects[i] != NULL) {
                        other = data->script_objects[i];
                    }
                    break;
                }
            }
        }
        if (other != NULL) {
            fieldstg_send_script_object(other, id, arg1, arg2);
        }
    }
    return 4;
}

/* Runs the event's script (FieldstgEvent.script_pos): commands of 16-bit words, high byte the command,
 * low byte its variant. */
void fieldstg_event_update(FieldstgEvent *obj, FieldstgEventData *data) {
    FieldstgActor *actor;
    Object *other;
    s16 *script;
    s32 running;
    s16 sub;
    s32 slot;
    s32 id;
    s32 arg2;
    s32 arg1;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (fieldstg_stage.event_text != 0 && cdload_module.is_loading(fieldstg_stage.event_text >> 16) != 0) {
            break;
        }
        if (heap_objects.find(7, -1, -1)->state != OBJECT_STATE_RUN) {
            break;
        }
        other = heap_objects.find(5, -1, -1);
        for (i = 0; other != NULL; other = heap_objects.find_next()) {
            actor = (FieldstgActor *)other;
            obj->actors[i].actor = actor;
            obj->actors[i].id = actor->base.key1;
            i++;
            actor->start_walk_to(actor);
            if (i == 30) {
                break;
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->script_pos != NULL) {
            script = obj->script_pos;
            running = 1;
            do {
                sub = *script & 0xFF;
                switch (*script >> 8) {
                case 0:
                default:
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    running = 0;
                    break;
                case 1:
                    switch (sub) {
                    case 0:
                    default:
                        actor = fieldstg_event_find_actor(obj, script[1]);
                        if (actor != NULL) {
                            actor->pos.x = script[2] << 8;
                            actor->pos.y = script[3] << 8;
                        }
                        script += 4;
                        break;
                    case 1:
                        script += fieldstg_event_send_message(obj, script, data);
                        break;
                    case 2:
                        actor = fieldstg_event_find_actor(obj, script[1]);
                        if (actor != NULL) {
                            actor->set_walk_target(actor, script[2], script[3], script[4]);
                        }
                        script += 5;
                        break;
                    }
                    break;
                case 2:
                    slot = script[1];
                    arg1 = script[2];
                    id = script[3];
                    arg2 = script[4];
                    if (arg2 != 4) {
                        actor = fieldstg_event_find_actor(obj, id);
                        if (actor != NULL) {
                            data->dialogs[slot] = fieldstg_dialog_create(actor, arg1, arg2, 0);
                        }
                    } else {
                        data->dialogs[slot] = fieldstg_dialog_create(NULL, arg1, 4, 1);
                    }
                    script += 5;
                    break;
                case 3:
                    switch (sub) {
                    case 0:
                    default:
                        if (obj->wait == 0) {
                            obj->wait = script[1];
                        }
                        if (--obj->wait != 0) {
                            running = 0;
                        } else {
                            script += 2;
                        }
                        break;
                    case 1:
                        if (data->dialogs[0] != 0) {
                            running = 0;
                        } else {
                            script += 1;
                        }
                        break;
                    case 2:
                        actor = fieldstg_event_find_actor(obj, script[1]);
                        if (actor != NULL && actor->is_walking(actor) != 0) {
                            running = 0;
                        } else {
                            script += 2;
                        }
                        break;
                    case 3:
                        actor = fieldstg_event_find_actor(obj, script[1]);
                        if (actor == NULL || actor->is_anim_done(actor) != 0) {
                            script += 2;
                        } else {
                            running = 0;
                        }
                        break;
                    case 4:
                        fieldstg_goto_map(script[1], -1, script[2] << 8, script[3] << 8, script[4]);
                        obj->base.set_state(obj, OBJECT_STATE_DONE);
                        running = 0;
                        break;
                    }
                    break;
                case 6:
                    switch (sub) {
                    case 0:
                    default:
                        fieldstg_camera_follow(script[1], script[2]);
                        script += 3;
                        break;
                    case 1:
                        fieldstg_camera_move_to(script[1], script[2], script[3]);
                        script += 4;
                        break;
                    }
                    break;
                }
            } while (running);
            obj->script_pos = script;
            break;
        }
        switch (obj->base.step) {
        case 0:
        default:
            data->started = obj->start();
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            if (data->started == NULL) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        for (i = 0; i < 30; i++) {
            if (obj->actors[i].id == 0) {
                break;
            }
            obj->actors[i].actor->reset_control(obj->actors[i].actor);
        }
        fieldstg_stage.event_running = 0;
        if (obj->end != NULL) {
            obj->end();
        }
        break;
    }
}

FieldstgEvent *fieldstg_event_start(s32 id) {
    FieldstgEvent *obj = object_new(fieldstg_event_update, sizeof(FieldstgEvent), sizeof(FieldstgEventData));
    FieldstgEventDef *event;
    Object *player;
    s32 text;

    for (event = fieldstg_stage.events; event->id != -1; event++) {
        if (event->id == id) {
            obj->id = id;
            obj->script_pos = event->script;
            obj->start = event->start;
            obj->end = event->end;
            text = event->text;
            fieldstg_stage.event_text = text;
            if (text != 0) {
                fieldstg_stage.event_text = text + (records_language << 16);
                cdload_module.queue_file(event->text >> 16);
            }
            fieldstg_stage.event_running = 1;
            if ((u32)(id - 8000) >= 1000) {
                player = heap_objects.find(5, -1, 0);
                if (player != NULL && player->state == OBJECT_STATE_RUN) {
                    player->set_step(player, 1);
                }
            }
            break;
        }
    }
    if (fieldstg_stage.event_running == 0) {
        obj->base.set_state(obj, OBJECT_STATE_END);
    }
    return obj;
}
