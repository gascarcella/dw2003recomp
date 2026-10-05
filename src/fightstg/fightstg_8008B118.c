#include "common.h"

#include "gfx.h"
#include "object.h"
#include "pad.h"
#include "psyq/libgpu.h"
#include "fightstg.h"

/* A rectangle of a model's texture that changes (eyes, mouth): where it is and its 3 frames, all
 * relative to the texture's VRAM position. */
typedef struct FightstgTextureAnimRecord {
    /* 0x0 */ u8 x;
    /* 0x1 */ u8 y;
    /* 0x2 */ u8 w; /* 0 = unused */
    /* 0x3 */ u8 h;
    /* 0x4 */ u8 uv[3][2]; /* source of each frame */
} FightstgTextureAnimRecord; /* size 0xA */

typedef struct FightstgTextureAnimSlot {
    /* 0x00 */ s32 in_use;
    /* 0x04 */ s32 frame; /* frame shown */
    /* 0x08 */ FightstgTextureAnimRecord rec;
} FightstgTextureAnimSlot; /* size 0x14 */

/* fightstg_texture_anim_create's object: animates parts of a model's texture by copying rectangles in
 * VRAM (DR_MOVE): slots 0-1 blink or follow the animation, slots 2-15 cycle through their frames. */
typedef struct FightstgTextureAnim {
    /* 0x000 */ Object base;
    /* 0x050 */ FightstgModel *model;
    /* 0x054 */ FightstgPos texture_pos;
    /* 0x05C */ s32 slot_count; /* slots in use */
    /* 0x060 */ FightstgTextureAnimSlot slots[16];
    /* 0x1A0 */ s32 blink_timer; /* blink timer */
    /* 0x1A4 */ s32 time;
} FightstgTextureAnim; /* size 0x1A8 */

void fightstg_texture_anim_update(FightstgTextureAnim *obj);

/* Frame of slots 0-1 forced by the model's animation: 1 (animation 2), 2 (animations 4-11), else 0. */
s32 fightstg_texture_anim_get_forced_frame(FightstgTextureAnim *obj) {
    switch (obj->model->anim) {
    case 4 ... 11:
        return 2;
    case 2:
        return 1;
    }
    return 0;
}

/* Fills a DR_MOVE that shows frame `frame` of slot `i`. */
void fightstg_texture_anim_set_move(FightstgTextureAnim *obj, DR_MOVE *p, s32 i, s32 frame) {
    RECT rect;
    FightstgPos pos;

    rect.x = obj->slots[i].rec.uv[frame][0] + obj->texture_pos.x;
    rect.y = obj->slots[i].rec.uv[frame][1] + obj->texture_pos.y;
    rect.w = obj->slots[i].rec.w;
    rect.h = obj->slots[i].rec.h;
    pos.x = obj->slots[i].rec.x + obj->texture_pos.x;
    pos.y = obj->slots[i].rec.y + obj->texture_pos.y;
    SetDrawMove(p, &rect, pos.x, pos.y);
}

void fightstg_texture_anim_update(FightstgTextureAnim *obj) {
    s32 eyes;
    s32 frame;
    s32 i;
    GfxLayer *layer;
    u32 *ot;
    DR_MOVE *p;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->slot_count == 0) {
            obj->base.set_state(obj, OBJECT_STATE_END);
            break;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        eyes = fightstg_texture_anim_get_forced_frame(obj);
        if (eyes == 0) {
            switch (obj->blink_timer >> 1) {
            case 0:
                obj->blink_timer = (pad_random.next() & 0x7F) + 60;
            default:
                eyes = 0;
                break;
            case 1:
            case 2:
            case 5:
            case 6:
                eyes = 1;
                break;
            case 3:
            case 4:
                eyes = 2;
                break;
            }
            obj->blink_timer -= fightstg_battle.frames;
            if (obj->blink_timer < 0) {
                obj->blink_timer = 0;
            }
        } else {
            obj->blink_timer = 0;
        }
        obj->time += fightstg_battle.frames;
        frame = obj->time % 18 / 6;
        layer = gfx_module.funcs.get_layer(0x1000);
        ot = layer->get_ot_entry(layer, 0);
        p = gfx_module.funcs.get_packet();
        for (i = 0; i < 2; i++) {
            if (obj->slots[i].in_use != 0 && obj->slots[i].frame != eyes) {
                obj->slots[i].frame = eyes;
                fightstg_texture_anim_set_move(obj, p, i, eyes);
                addPrim(ot, p);
                p++;
            }
        }
        for (i = 2; i < 16; i++) {
            if (obj->slots[i].in_use != 0 && obj->slots[i].frame != frame) {
                obj->slots[i].frame = frame;
                fightstg_texture_anim_set_move(obj, p, i, frame);
                addPrim(ot, p);
                p++;
            }
        }
        gfx_module.funcs.set_packet(p);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the texture animation of model `model` from records fightstg_models.get_texture_anim(id) (0xFF-terminated). */
FightstgTextureAnim *fightstg_texture_anim_create(FightstgModel *model, s32 id) {
    FightstgTextureAnim *obj;
    FightstgTextureAnimRecord *rec;
    s32 i;

    if (id == 0) {
        return NULL;
    }
    obj = object_new(fightstg_texture_anim_update, sizeof(FightstgTextureAnim), 0);
    obj->model = model;
    obj->texture_pos = model->texture_pos;
    rec = fightstg_models.get_texture_anim(id);
    for (i = 15; i >= 0; i--) {
        obj->slots[i].in_use = 0;
    }
    for (i = 0; i < 16; i++) {
        if (rec[i].x == 0xFF) {
            break;
        }
        if (rec[i].w != 0) {
            obj->slots[i].rec = rec[i];
            obj->slots[i].in_use = 1;
            obj->slot_count++;
        }
    }
    return obj;
}
