#include "common.h"

#include "records.h"
#include "inn.h"
#include "object.h"
#include "gamestate.h"
#include "sound.h"
#include "gfx.h"
#include "message.h"
#include "cdload.h"
#include "pad.h"
#include "window_anim.h"
#include "overlay_common.h"
#include "psyq/libgpu.h"

/* An entry of inn_table, ended by map == 0. */
typedef struct InnEntry {
    /* 0x0 */ s32 map;     /* the inn's map (gamestate_data.funcs.get_map()) */
    /* 0x4 */ s16 name_id; /* its name: message number in ?SHTLNAM ("Asuka Inn", ...) */
    /* 0x6 */ s16 price;   /* price per party member */
} InnEntry; /* size 0x8 */

InnEntry inn_table[] = {
    { 0x20A, 1, 8 },
    { 0x223, 2, 12 },
    { 0x22F, 3, 16 },
    { 0x238, 4, 32 },
    { 0x23F, 5, 36 },
    { 0x25B, 6, 48 },
    { 0x25D, 7, 44 },
    { 0x263, 8, 80 },
    { 0x26F, 9, 84 },
    { 0x26D, 10, 88 },
    { 0x279, 11, 52 },
    { 0x292, 2, 56 },
    { 0x29D, 12, 60 },
    { 0x2A5, 4, 68 },
    { 0x2AC, 13, 72 },
    { 0x2C4, 6, 80 },
    { 0x2C6, 14, 76 },
    { 0x2CB, 8, 84 },
    { 0x2D6, 15, 88 },
    { 0x249, 21, 44 },
    { 0x2B3, 21, 76 },
    { 0, 1, 1 },
};

/* Object types created in this file: the Object header (base), then their own fields (the fade, inn_fade_create,
 * is overlay_common.h's Fade). */
struct Inn {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* gfx layer (inn_create) */
    /* 0x54 */ s32 ot_depth; /* its ordering table entry (1) */
    /* 0x58 */ s32 inn;    /* index into inn_table */
    /* 0x5C */ s32 cursor; /* cursor: 0 or 1 */
    /* 0x60 */ s32 party_count; /* party members (gamestate_data.funcs.get_party_member(i) >= 0) */
    /* 0x64 */ s32 saved_sound; /* sound_module.current, saved */
    /* 0x68 */ s32 saved_time; /* gfx_module.funcs.get_time(), saved */
    /* 0x6C */ WindowAnim title_anim;    /* the panels of the inn's name and the money */
    /* 0x7C */ WindowAnim question_anim; /* the price and stay / don't stay panel */
    /* 0x8C */ WindowAnim message_anim;  /* the "not enough BIT" panel */
    /* 0x9C */ s32 unk_9C;               /* unused; inn_create allocates 0xA0 */
}; /* size 0xA0 */

/* inn_update's data block (object_new's third argument). */
typedef struct InnData {
    /* 0x00 */ Fade *fade;
    /* 0x04 */ MessageWindow *name;       /* the inn's name */
    /* 0x08 */ MessageWindow *money;      /* the player's money */
    /* 0x0C */ MessageWindow *money_unit; /* "BIT" */
    /* 0x10 */ MessageWindow *question;   /* "Each Digimon N BIT", or "You don't have enough BIT!" */
    /* 0x14 */ MessageWindow *yes;        /* "Stay in Inn" */
    /* 0x18 */ MessageWindow *no;         /* "Do not stay in Inn" */
    /* 0x1C */ MessageCursor *cursor;
} InnData;     /* size 0x20 */

void inn_update_dialog(Inn *obj, InnData *data);
void inn_update(Inn *obj, InnData *data);
void inn_fade_start(Fade *obj, s32 arg1, s32 arg2);
void inn_fade_draw(Fade *obj);

void inn_heal_party(void) {
    s32 i;
    s32 j;
    s32 id;
    GamestateRecord *p;

    for (i = 0; i < 3; i++) {
        id = gamestate_data.funcs.get_party_member(i);
        if (id >= 0) {
            p = gamestate_data.funcs.get_record(id);
            p->stats.values[2] = p->stats.values[3];
            p->stats.values[4] = p->stats.values[5];
            for (j = 2; j >= 0; j--) {
                p->stats.penalties[j] = 0;
            }
        }
    }
}

/* The inn's dialogue, one step per frame (obj->base.step): fade the windows in, ask (cursor: 0 = yes,
 * 1 = no), then either rest (pay party_count x the price, fade the screen out and in with the fade object,
 * heal the party with inn_heal_party) or say there isn't enough money, and close (state OBJECT_STATE_END). */
void inn_update_dialog(Inn *obj, InnData *data) {
    s32 cursor;

    switch (obj->base.step) {
    case 0:
    default:
        window_anim_start(&obj->title_anim, 1);
        obj->base.step++;
        break;
    case 1:
        if (window_anim_update(&obj->title_anim)) {
            data->name->set_text(data->name, cdload_module.files.get_file(records_language + 0x5C),
                                  inn_table[obj->inn].name_id);
            data->money->set_line_number(data->money, 0, gamestate_data.money);
            data->money->measure(data->money, 1);
            data->money_unit->set_text(data->money_unit, cdload_module.files.get_file(records_language + 0x5C), 0x10);
            window_anim_start(&obj->question_anim, 1);
            obj->base.step++;
        }
        break;
    case 2:
        if (window_anim_update(&obj->question_anim)) {
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x5C), 0x11);
            data->question->set_line_number(data->question, 1, inn_table[obj->inn].price);
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x5C), 0x12);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x5C), 0x13);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 3:
        cursor = obj->cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->cursor = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->cursor = 1;
        }
        if (cursor != obj->cursor) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0xB8, obj->cursor * 16 + 0x5F);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            if (obj->cursor != 0) {
                obj->base.step = 10;
            } else if (gamestate_data.money >= inn_table[obj->inn].price * obj->party_count) {
                obj->base.step = 20;
            } else {
                obj->base.step = 10;
                obj->base.substep = 1;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 10;
        }
        break;
    case 10:
        window_anim_start(&obj->question_anim, 0);
        data->question->set_visible(data->question, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
        break;
    case 11:
        if (window_anim_update(&obj->question_anim)) {
            if (obj->base.substep != 0) {
                window_anim_start(&obj->message_anim, 1);
                obj->base.step = 30;
            } else {
                window_anim_start(&obj->title_anim, 0);
                data->name->set_visible(data->name, 0);
                data->money->set_visible(data->money, 0);
                data->money_unit->set_visible(data->money_unit, 0);
                obj->base.step++;
            }
        }
        break;
    case 12:
        if (window_anim_update(&obj->title_anim)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 20:
        obj->saved_sound = sound_module.current;
        obj->saved_time = gfx_module.funcs.get_time();
        sound_module.play(0x4004000D);
        data->fade = inn_fade_create(obj->layer_id);
        data->fade->start(data->fade, 0, 20);
        obj->base.step++;
        break;
    case 21:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            obj->title_anim.level = 0;
            obj->question_anim.level = 0;
            data->name->set_visible(data->name, 0);
            data->money->set_visible(data->money, 0);
            data->money_unit->set_visible(data->money_unit, 0);
            data->question->set_visible(data->question, 0);
            data->yes->set_visible(data->yes, 0);
            data->no->set_visible(data->no, 0);
            data->cursor->show(data->cursor, 0);
            gamestate_data.money -= inn_table[obj->inn].price * obj->party_count;
            inn_heal_party();
            obj->base.step++;
        }
        break;
    case 22:
        if (gfx_module.funcs.get_time() - obj->saved_time > 240) {
            data->fade->start(data->fade, 1, 20);
            obj->base.step++;
        }
        break;
    case 23:
        if (data->fade->base.state == OBJECT_STATE_DONE) {
            sound_module.play(obj->saved_sound);
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 30:
        if (window_anim_update(&obj->message_anim)) {
            data->question->set_text(data->question, cdload_module.files.get_file(records_language + 0x5C), 0x14);
            obj->base.substep = 0;
            obj->base.step++;
        }
        break;
    case 31:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            data->question->set_visible(data->question, 0);
            window_anim_start(&obj->message_anim, 0);
            obj->base.step++;
        }
        break;
    case 32:
        if (window_anim_update(&obj->message_anim)) {
            window_anim_start(&obj->question_anim, 1);
            obj->base.step = 2;
        }
        break;
    }
}

/* The inn object's function: sets up its windows (state 0), then runs inn_update_dialog and draws the
 * three fading pictures (state 1). */
void inn_update(Inn *obj, InnData *data) {
    Sprite gfx;
    s32 map;
    s32 town;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        map = gamestate_data.funcs.get_map();
        for (town = 0; inn_table[town].map != 0; town++) {
            if (map == inn_table[town].map) {
                break;
            }
        }
        obj->inn = town;
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) >= 0) {
                obj->party_count++;
            }
        }
        data->name = message_create_window(obj->layer_id, 1, 0x1D, 0x14);
        data->money = message_create_window(obj->layer_id, 3, 0x117, 0x17);
        data->money_unit = message_create_window(obj->layer_id, 3, 0x11A, 0x17);
        data->question = message_create_window(obj->layer_id, 1, 0x79, 0x34);
        data->yes = message_create_window(obj->layer_id, 1, 0xC5, 0x5F);
        data->no = message_create_window(obj->layer_id, 1, 0xC5, 0x6F);
        data->cursor = message_create_cursor(obj->layer_id, 0, 0xB8, 0x5F);
        data->cursor->show(data->cursor, 0);
        obj->title_anim.duration = 10;
        obj->question_anim.duration = 10;
        obj->message_anim.duration = 10;
        break;
    case OBJECT_STATE_RUN:
        inn_update_dialog(obj, data);
        sprite_init(&gfx);
        gfx.set_vram_pos(0x140, 0);
        gfx.set_layer_id(obj->layer_id, obj->ot_depth);
        gfx.set_follow_scroll(0);
        if (obj->title_anim.level != 0) {
            if (obj->title_anim.level != 0x1000) {
                gfx.set_scale(obj->title_anim.level, 0x1000, 0x1000);
                gfx.set_pivot(0x57, 0x19);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x41, 0x16, 0x12);
            if (obj->title_anim.level != 0x1000) {
                gfx.set_pivot(0x140, 0x18);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x42, 0xD6, 0xF);
        }
        if (obj->question_anim.level != 0) {
            if (obj->question_anim.level != 0x1000) {
                gfx.set_scale(obj->question_anim.level, 0x1000, 0x1000);
                gfx.set_pivot(0x140, 0x41);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x43, 0x4C, 0x2E);
            if (obj->question_anim.level != 0x1000) {
                gfx.set_pivot(0x140, 0x6D);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x44, 0xAF, 0x59);
        }
        if (obj->message_anim.level != 0) {
            if (obj->message_anim.level != 0x1000) {
                gfx.set_scale(obj->message_anim.level, 0x1000, 0x1000);
                gfx.set_pivot(0x140, 0x41);
            }
            gfx.draw(cdload_module.get_subfile_by_id(0x2860000), 0x43, 0x4C, 0x2E);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

Inn *inn_create(s32 layer) {
    Inn *obj = object_new(inn_update, sizeof(Inn), sizeof(InnData));

    obj->layer_id = layer;
    obj->ot_depth = 1;
    return obj;
}

void inn_fade_start(Fade *obj, s32 arg1, s32 arg2) {
    obj->base.set_state(obj, OBJECT_STATE_RUN);
    obj->base.step = 1;
    obj->from_black = arg1;
    if (arg1 == 0) {
        obj->level = 0;
        obj->step = 0xFF00 / arg2;
    } else {
        obj->level = 0xFF00;
        obj->step = -(0xFF00 / arg2);
    }
}

/* Draws a full-screen (320x256) quad with subtractive blending (abr 2), each channel level >> 8. */
void inn_fade_draw(Fade *obj) {
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
    poly->x0 = poly->x2 = 0;
    poly->x1 = poly->x3 = 320;
    poly->y0 = poly->y1 = 0;
    poly->y2 = poly->y3 = 256;
    addPrim(ot, poly);
    tpage = (DR_TPAGE *)(poly + 1);
    setDrawTPage(tpage, 0, 1, getTPage(0, 2, 320, 0));
    addPrim(ot, tpage);
    gfx_module.funcs.set_packet(tpage + 1);
}

void inn_fade_update(Fade *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            break;
        }
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
        /* fallthrough */
    case OBJECT_STATE_DONE:
        inn_fade_draw(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

Fade *inn_fade_create(s32 arg0) {
    Fade *obj = object_new(inn_fade_update, sizeof(Fade), 0);

    obj->start = inn_fade_start;
    obj->layer_id = arg0;
    obj->ot_depth = 0;
    return obj;
}
