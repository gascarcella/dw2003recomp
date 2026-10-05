#include "common.h"
#include "object.h"
#include "cdload.h"
#include "gfx.h"
#include "records.h"

/* WFIGHTMN's first file: an object that loads the battle's files and images. */

/* Loads the battle's images (files 0x456, 0x79E, 0x7AE, 0x7E1, 0x7E3) into VRAM, then the battle's text files
 * (0x286, 0x455 and five language-dependent ones). */
void wfightmn_loader_update(Object *obj) {
    switch (obj->state) {
    case OBJECT_STATE_INIT:
    default:
        obj->next_state(obj);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        switch (obj->step) {
        case 0:
        default:
            switch (obj->substep) {
            case 0:
            default:
                cdload_module.queue_file(0x456);
                cdload_module.queue_file(0x79E);
                cdload_module.queue_file(0x7AE);
                cdload_module.queue_file(0x7E1);
                cdload_module.queue_file(0x7E3);
                obj->next_substep(obj);
                break;
            case 1:
                if (cdload_module.is_loading(0x456) == 0) {
                    Tim tim;

                    tim_init(&tim);
                    tim.set_image_pos(0x200, 0);
                    tim.load_all(cdload_module.get_subfile_by_id(0x4560001));
                    tim.set_image_pos(0, 0xF4);
                    tim.load(cdload_module.get_subfile_by_id(0x4560000));
                    obj->next_substep(obj);
                }
                break;
            case 2:
                if (cdload_module.is_loading(0x79E) == 0) {
                    Tim tim;

                    tim_init(&tim);
                    tim.set_image_pos(0x140, 0x100);
                    tim.load_all(cdload_module.files.get_file(0x79E));
                    obj->next_substep(obj);
                }
                break;
            case 3:
                if (cdload_module.is_loading(0x7AE) == 0) {
                    Tim tim;

                    tim_init(&tim);
                    tim.set_image_pos(0x1C0, 0x100);
                    tim.load_all(cdload_module.files.get_file(0x7AE));
                    obj->next_substep(obj);
                }
                break;
            case 4:
                if (cdload_module.is_loading(0x7E1) == 0) {
                    Tim tim;

                    tim_init(&tim);
                    tim.set_image_pos(0x200, 0x100);
                    tim.load_all(cdload_module.files.get_file(0x7E1));
                    obj->next_substep(obj);
                }
                break;
            case 5:
                if (cdload_module.is_loading(0x7E3) == 0) {
                    Tim tim;

                    tim_init(&tim);
                    tim.set_image_pos(0x240, 0x100);
                    tim.load_all(cdload_module.files.get_file(0x7E3));
                    obj->next_substep(obj);
                }
                break;
            case 6:
                cdload_module.files.free_file(0x456);
                cdload_module.files.free_file(0x79E);
                cdload_module.files.free_file(0x7AE);
                cdload_module.files.free_file(0x7E1);
                cdload_module.files.free_file(0x7E3);
                obj->next_step(obj);
                break;
            }
            break;
        case 1:
            switch (obj->substep) {
            case 0:
            default:
                cdload_module.queue_file(0x286);
                cdload_module.queue_file(0x455);
                cdload_module.queue_file(records_language + 0x7F);
                cdload_module.queue_file(records_language + 0x4E);
                cdload_module.queue_file(records_language + 0xA2);
                cdload_module.queue_file(records_language + 0x9B);
                cdload_module.queue_file(records_language + 0x6A);
                cdload_module.queue_file(records_language + 0x63);
                obj->next_substep(obj);
                break;
            case 1:
                if (cdload_module.is_loading(0x286) == 0 && cdload_module.is_loading(0x455) == 0 &&
                    cdload_module.is_loading(records_language + 0x7F) == 0 && cdload_module.is_loading(records_language + 0x4E) == 0 &&
                    cdload_module.is_loading(records_language + 0xA2) == 0 && cdload_module.is_loading(records_language + 0x9B) == 0 &&
                    cdload_module.is_loading(records_language + 0x6A) == 0 && cdload_module.is_loading(records_language + 0x63) == 0) {
                    obj->set_state(obj, OBJECT_STATE_END);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates wfightmn_loader_update's object (0x54 bytes; called by wfightmn_main_update). */
Object *wfightmn_loader_create(void) {
    return object_new(wfightmn_loader_update, 0x54, 0);
}
