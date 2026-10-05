#include "common.h"
#include "object.h"
#include "cdload.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "gfx.h"
#include "records.h"
#include "message.h"
#include "ststatus.h"

/* The techniques page (ststatus_create_tech_page): the healing techniques (0xB8..0xBC) a member's forms know,
 * used from the menu on one member or on the whole party. */

/* Per party member: its forms, their records and the healing techniques found in them. */
typedef struct StstatusTechMember {
    /* 0x00 */ s16 forms[3];  /* forms */
    /* 0x06 */ u8 pad_06[0x2];
    /* 0x08 */ GamestateForm form_records[3]; /* their records */
    /* 0x44 */ s16 techniques[5]; /* healing techniques */
    /* 0x4E */ u8 pad_4E[0x2];
    /* 0x50 */ s32 count;  /* their number */
} StstatusTechMember; /* size 0x54 */

/* The techniques page's data block (0xB4 bytes). */
typedef struct StstatusTechPageData {
    /* 0x00 */ MessageWindow *title;  /* title */
    /* 0x04 */ StstatusPanelWindows panels[3]; /* per party member: its panel */
    /* 0x88 */ MessageWindow *message; /* message, or technique */
    /* 0x8C */ MessageWindow *back_hint;
    /* 0x90 */ MessageWindow *mp_label; /* "MP" */
    /* 0x94 */ MessageWindow *mp_cost; /* MP cost */
    /* 0x98 */ MessageWindow *list_title; /* list title */
    /* 0x9C */ MessageWindow *techniques[5]; /* techniques */
    /* 0xB0 */ MessageCursor *cursor; /* cursor */
} StstatusTechPageData; /* size 0xB4 */

/* The techniques page (ststatus_create_tech_page, size 0x1F4). */
typedef struct StstatusTechPage {
    /* 0x000 */ Object base; /* base.substep: the state to return to after a message */
    /* 0x050 */ s32 parent;
    /* 0x054 */ s32 layer_id; /* layer */
    /* 0x058 */ s32 ot_depth; /* ordering table depth */
    /* 0x05C */ s32 member_count; /* party members */
    /* 0x060 */ s32 frames[3]; /* per member: sprite frame */
    /* 0x06C */ s32 frame_time; /* time of the last frame */
    /* 0x070 */ s32 member_cursor_frame; /* member cursors' frame, 0..7 */
    /* 0x074 */ s32 member_cursor_time; /* time of its last frame */
    /* 0x078 */ s32 member_cursor_shown; /* the member cursor is shown */
    /* 0x07C */ s32 user;   /* the member using the technique */
    /* 0x080 */ s32 target_cursor_shown; /* the target cursor is shown */
    /* 0x084 */ s32 target; /* the target */
    /* 0x088 */ s32 technique_cursor; /* technique cursor */
    /* 0x08C */ StstatusTechMember members[3];
    /* 0x188 */ s32 arrow_shown; /* the "next" arrow is shown */
    /* 0x18C */ s32 arrow_frame; /* its frame, 0..4 */
    /* 0x190 */ s32 arrow_time; /* time of its last frame */
    /* 0x194 */ WindowAnim member_panels[3]; /* member panels */
    /* 0x1C4 */ WindowAnim bars[2];    /* title, bottom bar */
    /* 0x1E4 */ WindowAnim list_anim; /* technique list */
} StstatusTechPage; /* size 0x1F4 */

s32 ststatus_tech_panel_stats[5] = { 0, 2, 3, 4, 5 };
/* The technique list's frame sprite, by the number of techniques listed (1..5). */
s32 ststatus_tech_list_frames[5] = { 57, 56, 55, 54, 45 };

/* Collects the healing techniques of member `member`'s forms (from the third form ID on); returns how many. */
s32 ststatus_tech_collect(StstatusTechPage *obj, s32 member) {
    StstatusTechMember *m;
    GamestateForm *rec;
    s32 digimon;
    s32 i;
    s32 j;
    s32 k;
    s32 n;
    s32 found;
    s32 tech;
    s16 *p;

    digimon = gamestate_data.funcs.get_party_member(member);
    i = 0;
    n = 0;
    gamestate_data.funcs.get_chosen_forms(digimon, obj->members[member].forms);
    for (; i < 3; i++) {
        m = &obj->members[member];
        if (obj->members[member].forms[i] >= 3) {
            rec = &m->form_records[i];
            gamestate_data.funcs.get_form(digimon, obj->members[member].forms[i], rec);
            for (j = 0; j < 6; j++) {
                tech = rec->techniques[j] & 0x1FFF;
                if (tech >= 0xB8 && tech < 0xBD) {
                    k = 0;
                    found = 0;
                    for (p = m->techniques; k < 5; k++, p++) {
                        if (tech == *p) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        obj->members[member].techniques[n++] = tech;
                        if (n >= 5) {
                            return n;
                        }
                    }
                }
            }
        }
    }
    return n;
}

/* The technique under the cursor, or 0 if the member hasn't the MP for it. */
s32 ststatus_tech_get_usable(StstatusTechPage *obj) {
    GamestateRecord *rec;
    s16 tech;

    rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->user));
    tech = obj->members[obj->user].techniques[obj->technique_cursor];
    if (records_techniques[tech - 1].mp_cost <= rec->stats.values[4]) {
        return tech;
    }
    return 0;
}

/* How much the technique heals when Digimon `digimon` uses it. */
s32 ststatus_calc_heal(s32 digimon, s32 tech) {
    GamestateStats stats;
    RecordsTechnique *t;
    s32 power;

    gamestate_data.funcs.get_stats(digimon, &stats);
    t = &records_techniques[tech - 1];
    power = t->effect_power;
    return (power << 6) + power * stats.stats[3] / 8;
}

void ststatus_tech_show_member(StstatusTechPage *obj, StstatusTechPageData *data, s32 member, s32 show);

/* Uses the technique: heals the target, or the whole party; returns 1 if it did anything. */
s32 ststatus_tech_use(StstatusTechPage *obj, StstatusTechPageData *data, s32 sound) {
    s32 digimon[2]; /* the user, the target (an array: the original keeps both on the stack) */
    s32 tech;
    GamestateRecord *user;
    GamestateRecord *rec;
    s32 heal;
    s32 i;
    s32 healed;
    s32 full;

    tech = obj->members[obj->user].techniques[obj->technique_cursor];
    digimon[0] = gamestate_data.funcs.get_party_member(obj->user);
    user = gamestate_data.funcs.get_record(digimon[0]);
    heal = ststatus_calc_heal(digimon[0], tech);
    if (records_techniques[tech - 1].target == 3) {
        digimon[1] = gamestate_data.funcs.get_party_member(obj->target);
        rec = gamestate_data.funcs.get_record(digimon[1]);
        if (rec->stats.values[2] < rec->stats.values[3]) {
            user->stats.values[4] -= records_techniques[tech - 1].mp_cost;
            rec->stats.values[2] += heal;
            if (rec->stats.values[3] < rec->stats.values[2]) {
                rec->stats.values[2] = rec->stats.values[3];
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x52);
            } else {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x53);
                data->message->set_line_number(data->message, 1, heal);
            }
            for (i = 0; i < obj->member_count; i++) {
                ststatus_tech_show_member(obj, data, i, 1);
            }
            sound_module.play(0x40014);
            return 1;
        }
    } else {
        i = 0;
        healed = 0;
        full = 0;
        for (; i < obj->member_count; i++) {
            digimon[1] = gamestate_data.funcs.get_party_member(i);
            if (digimon[1] >= 0) {
                rec = gamestate_data.funcs.get_record(digimon[1]);
                if (rec->stats.values[2] < rec->stats.values[3]) {
                    rec->stats.values[2] += heal;
                    if (rec->stats.values[3] < rec->stats.values[2]) {
                        rec->stats.values[2] = rec->stats.values[3];
                        full++;
                    }
                    healed = 1;
                } else {
                    full++;
                }
            }
        }
        if (healed) {
            user->stats.values[4] -= records_techniques[tech - 1].mp_cost;
            for (i = 0; i < obj->member_count; i++) {
                ststatus_tech_show_member(obj, data, i, 1);
            }
            if (full == obj->member_count) {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x52);
            } else {
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x53);
                data->message->set_line_number(data->message, 1, heal);
            }
            sound_module.play(0x40014);
            return 1;
        }
    }
    sound_module.play(sound);
    data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x6A);
    return 0;
}

/* Creates the page's text windows. */
void ststatus_tech_create_windows(StstatusTechPage *obj, StstatusTechPageData *data) {
    s32 i;
    s32 j;
    StstatusLayout *l;
    StstatusLayout *p;

    p = &ststatus_module.party_layout[11];
    data->title = message_create_window(obj->layer_id, 1, p->x, p->y);
    for (i = 0; i < 3; i++) {
        p = ststatus_module.party_layout;
        data->panels[i].name = message_create_window(obj->layer_id, 1, p->x, p->y + i * 0x2E);
        l = &ststatus_module.party_layout[1];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].labels[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
        l = &ststatus_module.party_layout[6];
        for (j = 0; j < 5; j++, l++) {
            data->panels[i].values[j] = message_create_window(obj->layer_id, 3, l->x, l->y + i * 0x2E);
        }
    }
    l = ststatus_module.party_layout;
    p = &l[12];
    data->message = message_create_window(obj->layer_id, 1, p->x, p->y);
    data->message->set_page_lines(data->message, 2);
    data->back_hint = message_create_window(obj->layer_id, 1, p->x, p->y + 0xE);
    p = &l[13];
    data->mp_label = message_create_window(obj->layer_id, 1, p->x, p->y);
    p = &l[14];
    data->mp_cost = message_create_window(obj->layer_id, 1, p->x, p->y);
    data->list_title = message_create_window(obj->layer_id, 1, 0x9C, 0x31);
    for (j = 0; j < 5; j++) {
        data->techniques[j] = message_create_window(obj->layer_id, 1, 0xB1, j * 0xE + 0x41);
    }
    data->cursor = message_create_cursor(obj->layer_id, obj->ot_depth - 1, 0xA6, obj->technique_cursor * 0xE + 0x41);
    data->cursor->show(data->cursor, 0);
}

/* Shows (fills in) or hides a member's panel. */
void ststatus_tech_show_member(StstatusTechPage *obj, StstatusTechPageData *data, s32 member, s32 show) {
    GamestateStats stats;
    s32 digimon;
    s32 i;
    MessageWindow *win;
    StstatusLayout *l;

    if (show) {
        digimon = gamestate_data.funcs.get_party_member(member);
        gamestate_data.funcs.get_stats(digimon, &stats);
        data->panels[member].name->set_text(data->panels[member].name,
                                             (u8 *)gamestate_data.funcs.get_record(digimon), -1);
        l = &ststatus_module.party_layout[1];
        for (i = 0; i < 5; i++, l++) {
            data->panels[member].labels[i]->set_text(data->panels[member].labels[i],
                                                    cdload_module.files.get_file(records_language + 0xB0), l->text);
        }
        for (i = 0; i < 5; i++) {
            win = data->panels[member].values[i];
            win->set_line_number(win, 0, stats.values[ststatus_tech_panel_stats[i]]);
            data->panels[member].values[i]->measure(data->panels[member].values[i], 1);
        }
    } else {
        data->panels[member].name->set_visible(data->panels[member].name, 0);
        for (i = 0; i < 5; i++) {
            data->panels[member].labels[i]->set_visible(data->panels[member].labels[i], 0);
        }
        for (i = 0; i < 5; i++) {
            data->panels[member].values[i]->set_visible(data->panels[member].values[i], 0);
        }
    }
}

/* Shows (fills in) or hides the member's technique list. */
void ststatus_tech_show_list(StstatusTechPage *obj, StstatusTechPageData *data, s32 show) {
    s32 i;

    if (show) {
        data->list_title->set_text(data->list_title, cdload_module.files.get_file(records_language + 0xB0), 9);
        for (i = 0; i < obj->members[obj->user].count; i++) {
            data->techniques[i]->set_text(data->techniques[i], cdload_module.files.get_file(records_language + 0xA2),
                                     obj->members[obj->user].techniques[i] & 0x1FFF);
        }
    } else {
        data->list_title->set_visible(data->list_title, 0);
        for (i = 0; i < obj->members[obj->user].count; i++) {
            data->techniques[i]->set_visible(data->techniques[i], 0);
        }
    }
}

/* Shows (fills in) or hides the technique under the cursor and its MP cost. */
void ststatus_tech_show_technique(StstatusTechPage *obj, StstatusTechPageData *data, s32 show) {
    s32 tech;

    if (show) {
        tech = obj->members[obj->user].techniques[obj->technique_cursor] & 0x1FFF;
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0x9B), tech);
        data->mp_label->set_text(data->mp_label, cdload_module.files.get_file(records_language + 0xB0), 3);
        data->mp_cost->set_line_number(data->mp_cost, 0, records_techniques[tech - 1].mp_cost);
        data->mp_cost->measure(data->mp_cost, 1);
    } else {
        data->message->set_visible(data->message, 0);
        data->mp_label->set_visible(data->mp_label, 0);
        data->mp_cost->set_visible(data->mp_cost, 0);
    }
}

/* Draws the members' sprites, the frames, the "next" arrow, the cursors and the technique list's frame. */
void ststatus_tech_draw(StstatusTechPage *obj) {
    Sprite spr;
    s32 i;
    s32 digimon;

    sprite_init(&spr);
    spr.set_layer_id(obj->layer_id, obj->ot_depth);
    if (gfx_module.funcs.get_time() - obj->frame_time >= 13) {
        obj->frame_time = gfx_module.funcs.get_time();
        for (i = 0; i < obj->member_count; i++) {
            digimon = gamestate_data.funcs.get_party_member(i);
            obj->frames[i]++;
            if (ststatus_module.anims[digimon].frame[obj->frames[i]] == -1 || obj->frames[i] >= 7) {
                obj->frames[i] = 0;
            }
        }
    }
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
            }
            digimon = gamestate_data.funcs.get_party_member(i);
            spr.set_vram_pos(0x280, 0x100);
            spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_module.anims[digimon].frame[obj->frames[i]],
                       0x6B, i * 0x2E + 0x13);
        }
    }
    spr.set_vram_pos(0x140, 0);
    for (i = 0; i < obj->member_count; i++) {
        if (obj->member_panels[i].level != 0) {
            if (obj->member_panels[i].level != 0x1000) {
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.set_scale(obj->member_panels[i].level, obj->member_panels[i].level, 0x1000);
                spr.set_pivot(0x7C, i * 0x2E + 0x27);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.set_scale(obj->member_panels[i].level, 0x1000, 0x1000);
                spr.set_pivot(0, i * 0x2E + 0x25);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            } else {
                spr.set_scale(0x1000, 0x1000, 0x1000);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x15, 0, i * 0x2E + 0x11);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x16, 0x67, i * 0x2E + 0x13);
                spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x17, 0, i * 0x2E + 0x11);
            }
        }
    }
    if (obj->bars[0].level != 0) {
        if (obj->bars[0].level != 0x1000) {
            spr.set_scale(obj->bars[0].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x19);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0x18, 0x22, 0xD);
    }
    if (obj->bars[1].level != 0) {
        if (obj->arrow_shown != 0) {
            if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
                obj->arrow_time = gfx_module.funcs.get_time();
                if (++obj->arrow_frame >= 5) {
                    obj->arrow_frame = 0;
                }
            }
            spr.set_vram_pos(0x140, 0);
            spr.set_palette(obj->arrow_frame);
            spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x123, 0xD6);
            spr.set_palette(0);
        }
        if (obj->bars[1].level != 0x1000) {
            spr.set_scale(obj->bars[1].level, 0x1000, 0x1000);
            spr.set_pivot(0, 0xD3);
        } else {
            spr.set_scale(0x1000, 0x1000, 0x1000);
        }
        spr.set_vram_pos(0x280, 0x100);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x20, 0, 0xC2);
    }
    if (gfx_module.funcs.get_time() - obj->member_cursor_time >= 9) {
        obj->member_cursor_time = gfx_module.funcs.get_time();
        if (++obj->member_cursor_frame >= 8) {
            obj->member_cursor_frame = 0;
        }
    }
    if (obj->target_cursor_shown != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.set_vram_pos(0x280, 0x100);
        spr.set_palette(obj->member_cursor_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->target * 0x2E + 0x11);
    }
    if (obj->member_cursor_shown != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth - 1);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->base.step >= 30) {
            spr.set_palette(8);
        } else {
            spr.set_palette(obj->member_cursor_frame);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), 0x1E, 0, obj->user * 0x2E + 0x11);
    }
    if (obj->list_anim.level != 0) {
        sprite_init(&spr);
        spr.set_layer_id(obj->layer_id, obj->ot_depth);
        spr.set_vram_pos(0x280, 0x100);
        if (obj->list_anim.level != 0x1000) {
            spr.set_scale(obj->list_anim.level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x63);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x04040000), ststatus_tech_list_frames[obj->members[obj->user].count - 1], 0x94,
                   0x2F);
    }
}

/* The page's states: open, pick a member, pick a technique (and a target), use it, close. */
void ststatus_tech_run(StstatusTechPage *obj, StstatusTechPageData *data) {
    s32 old_member;
    s32 old;
    s32 tech;

    switch (obj->base.step) {
    case 0:
    default:
        ststatus_module.window_anim_start(&obj->member_panels[0], 1);
        ststatus_module.window_anim_start(&obj->bars[0], 1);
        if (obj->member_count == 1) {
            ststatus_module.window_anim_start(&obj->bars[1], 1);
        }
        obj->base.step = obj->member_count;
        break;
    case 1:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->bars[0]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            ststatus_tech_show_member(obj, data, 0, 1);
            obj->base.step = 10;
        }
        break;
    case 2:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            ststatus_tech_show_member(obj, data, 0, 1);
            obj->base.step = 4;
        }
        break;
    case 4:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_tech_show_member(obj, data, 1, 1);
            obj->base.step = 10;
        }
        break;
    case 3:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            ststatus_tech_show_member(obj, data, 0, 1);
            obj->base.step = 5;
        }
        break;
    case 5:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[2], 1);
            ststatus_module.window_anim_start(&obj->bars[1], 1);
            ststatus_tech_show_member(obj, data, 1, 1);
            obj->base.step++;
        }
        break;
    case 6:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_tech_show_member(obj, data, 2, 1);
            obj->base.step = 10;
        }
        break;
    case 10:
        obj->member_cursor_shown = 1;
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x28);
        data->back_hint->set_text(data->back_hint, cdload_module.files.get_file(records_language + 0xB0), 0x15);
        obj->base.step++;
        break;
    case 11:
        old_member = obj->user;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->user < 0) {
                obj->user = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->user > obj->member_count - 1) {
                obj->user = obj->member_count - 1;
            }
        }
        if (old_member != obj->user) {
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->base.step = 15;
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 50;
        }
        break;
    case 15:
        if (obj->members[obj->user].count != 0) {
            obj->list_anim.duration = 8;
            ststatus_module.window_anim_start(&obj->list_anim, 1);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2A);
            data->message->set_visible(data->message, 0);
            data->back_hint->set_visible(data->back_hint, 0);
            obj->base.step = 30;
        } else {
            obj->base.step = 16;
        }
        break;
    case 16:
        obj->member_cursor_shown = 0;
        data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x29);
        data->back_hint->set_visible(data->back_hint, 0);
        obj->arrow_shown = 1;
        obj->base.step++;
        break;
    case 17:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->member_cursor_shown = 1;
            data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x28);
            data->back_hint->set_visible(data->back_hint, 1);
            obj->arrow_shown = 0;
            obj->base.step = 11;
        }
        break;
    case 30:
        if (ststatus_module.window_anim_update(&obj->list_anim)) {
            obj->technique_cursor = 0;
            ststatus_tech_show_list(obj, data, 1);
            ststatus_tech_show_technique(obj, data, 1);
            data->cursor->set_pos(data->cursor, 0xA6, obj->technique_cursor * 0xE + 0x41);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 31:
        old = obj->technique_cursor;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->technique_cursor < 0) {
                obj->technique_cursor = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->technique_cursor > obj->members[obj->user].count - 1) {
                obj->technique_cursor = obj->members[obj->user].count - 1;
            }
        }
        if (old != obj->technique_cursor) {
            data->cursor->set_pos(data->cursor, 0xA6, obj->technique_cursor * 0xE + 0x41);
            ststatus_tech_show_technique(obj, data, 1);
            sound_module.play(0x8004513E);
        } else if (PAD_PRESSED(13)) {
            tech = ststatus_tech_get_usable(obj);
            if (tech != 0) {
                if (records_techniques[tech - 1].target == 3) {
                    sound_module.play(0x8004503C);
                    obj->target_cursor_shown = 1;
                    data->cursor->set_palette(data->cursor, 7);
                    data->cursor->stop(data->cursor, 1);
                    data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2B);
                    obj->base.step++;
                } else {
                    data->cursor->set_palette(data->cursor, 7);
                    data->cursor->stop(data->cursor, 1);
                    ststatus_tech_show_technique(obj, data, 0);
                    obj->base.substep = obj->base.step;
                    ststatus_tech_use(obj, data, 0x8004503C);
                    obj->base.step = 36;
                    obj->arrow_shown = 1;
                }
            } else {
                sound_module.play(0x8004503C);
                data->cursor->set_palette(data->cursor, 7);
                data->cursor->stop(data->cursor, 1);
                ststatus_tech_show_technique(obj, data, 0);
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x69);
                obj->base.step = 35;
                obj->arrow_shown = 1;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            ststatus_module.window_anim_start(&obj->list_anim, 0);
            ststatus_tech_show_list(obj, data, 0);
            ststatus_tech_show_technique(obj, data, 0);
            data->cursor->show(data->cursor, 0);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x27);
            obj->base.step = 40;
        }
        break;
    case 32:
        old = obj->target;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            if (--obj->target < 0) {
                obj->target = 0;
            }
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            if (++obj->target > obj->member_count - 1) {
                obj->target = obj->member_count - 1;
            }
        }
        if (old != obj->target) {
            sound_module.play(0x4001B);
        } else if (PAD_PRESSED(13)) {
            ststatus_tech_show_technique(obj, data, 0);
            if (ststatus_tech_get_usable(obj) != 0) {
                obj->target_cursor_shown = 0;
                obj->base.substep = obj->base.step;
                ststatus_tech_use(obj, data, 0x4001C);
                obj->base.step = 36;
                obj->arrow_shown = 1;
            } else {
                sound_module.play(0x4001C);
                obj->member_cursor_shown = 0;
                obj->target_cursor_shown = 0;
                data->message->set_text(data->message, cdload_module.files.get_file(records_language + 0xB0), 0x69);
                obj->base.step = 35;
                obj->arrow_shown = 1;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.step = 31;
            obj->target_cursor_shown = 0;
            data->cursor->set_palette(data->cursor, 0);
            data->cursor->stop(data->cursor, 0);
            data->title->set_text(data->title, cdload_module.files.get_file(records_language + 0xB0), 0x2A);
        }
        break;
    case 35:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            ststatus_tech_show_technique(obj, data, 1);
            data->cursor->set_palette(data->cursor, 0);
            data->cursor->stop(data->cursor, 0);
            obj->base.step = 31;
            obj->arrow_shown = 0;
        }
        break;
    case 36:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            ststatus_tech_show_technique(obj, data, 1);
            obj->base.step = obj->base.substep;
            if (obj->base.substep == 31) {
                data->cursor->set_palette(data->cursor, 0);
                data->cursor->stop(data->cursor, 0);
            } else {
                obj->target_cursor_shown = 1;
            }
            obj->arrow_shown = 0;
        }
        break;
    case 40:
        if (ststatus_module.window_anim_update(&obj->list_anim)) {
            obj->base.set_step(obj, 10);
        }
        break;
    case 50:
        obj->member_cursor_shown = 0;
        data->back_hint->set_visible(data->back_hint, 0);
        ststatus_module.window_anim_start(&obj->bars[1], 0);
        ststatus_tech_show_technique(obj, data, 0);
        ststatus_module.window_anim_start(&obj->member_panels[obj->member_count - 1], 0);
        ststatus_tech_show_member(obj, data, obj->member_count - 1, 0);
        if (obj->member_count == 1) {
            data->title->set_visible(data->title, 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
        }
        obj->base.step = obj->member_count + 50;
        break;
    case 51:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        ststatus_module.window_anim_update(&obj->bars[0]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    case 52:
        ststatus_module.window_anim_update(&obj->member_panels[1]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_tech_show_member(obj, data, 0, 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            obj->base.step = 54;
        }
        break;
    case 53:
        ststatus_module.window_anim_update(&obj->member_panels[2]);
        if (ststatus_module.window_anim_update(&obj->bars[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[1], 0);
            ststatus_tech_show_member(obj, data, 1, 0);
            obj->base.step = 55;
        }
        break;
    case 55:
        if (ststatus_module.window_anim_update(&obj->member_panels[1])) {
            ststatus_module.window_anim_start(&obj->member_panels[0], 0);
            ststatus_module.window_anim_start(&obj->bars[0], 0);
            data->title->set_visible(data->title, 0);
            ststatus_tech_show_member(obj, data, 0, 0);
            obj->base.step++;
        }
        break;
    case 54:
    case 56:
        ststatus_module.window_anim_update(&obj->member_panels[0]);
        if (ststatus_module.window_anim_update(&obj->bars[0])) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}


void ststatus_tech_update(StstatusTechPage *obj, StstatusTechPageData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) >= 0) {
                obj->member_count++;
            }
        }
        for (i = 0; i < obj->member_count; i++) {
            obj->member_panels[i].duration = 10;
            ststatus_module.window_anim_start(&obj->member_panels[i], 1);
        }
        for (i = 0; i < 2; i++) {
            obj->bars[i].duration = 10;
            ststatus_module.window_anim_start(&obj->bars[i], 1);
        }
        for (i = 0; i < obj->member_count; i++) {
            obj->members[i].count = ststatus_tech_collect(obj, i);
        }
        ststatus_tech_create_windows(obj, data);
        break;
    case OBJECT_STATE_RUN:
        ststatus_tech_run(obj, data);
        ststatus_tech_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Creates the techniques page. */
StstatusTechPage *ststatus_create_tech_page(s32 arg0) {
    StstatusTechPage *obj = object_new(ststatus_tech_update, sizeof(StstatusTechPage), sizeof(StstatusTechPageData));

    obj->layer_id = 0x1000;
    obj->ot_depth = 6;
    obj->parent = arg0;
    return obj;
}
