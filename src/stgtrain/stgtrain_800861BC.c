#include "stgtrain.h"
#include "psyq/libgpu.h"

/* STGTRAIN.PRO, second file: a training session. The Digimon trains five rounds (StgtrainTrainee shows it), each
 * success raises a stat (and may lower another); a bonus round can follow. */

/* A stat change: base + random() % rand (rand 0: base alone). */
typedef struct StgtrainGain {
    /* 0x0 */ s32 base;
    /* 0x4 */ s32 rand;
} StgtrainGain; /* size 0x8 */

/* Raises of stats 6..11, by [6 if the training is 13 or more][bonus round * 3 + the trainer's level]. */
extern StgtrainGain stgtrain_stat_gains[12];
/* Losses of stats 6..11, by the trainer's level. */
extern StgtrainGain stgtrain_stat_losses[3];
/* Raises of stats 12..18 by gain class (RecordsDigimon.resist_gains) and stat size: [(class - 1) * 3 + 0..2]. */
extern StgtrainGain *stgtrain_resist_gain_tables[];
/* Raises of max HP/MP. */
extern StgtrainGain stgtrain_max_gains[9];

/* The session's data block (0x1C bytes). */
typedef struct StgtrainSessionData {
    /* 0x00 */ MessageWindow *result; /* result of the round */
    /* 0x04 */ MessageWindow *result_2;
    /* 0x08 */ MessageWindow *bonus_question; /* the bonus round's question */
    /* 0x0C */ MessageWindow *yes;    /* yes */
    /* 0x10 */ MessageWindow *no;     /* no */
    /* 0x14 */ MessageCursor *cursor; /* the cursor */
    /* 0x18 */ StgtrainTrainee *trainee;
} StgtrainSessionData; /* size 0x1C */

s32 stgtrain_session_raise_stat(StgtrainSession *obj, s32 type);
s32 stgtrain_session_lower_stat(StgtrainSession *obj, s32 type);
s32 stgtrain_session_raise_stat2(StgtrainSession *obj, s32 type);
s32 stgtrain_session_raise_max(StgtrainSession *obj, s32 type);
void stgtrain_session_run(StgtrainSession *obj, StgtrainSessionData *data);

/* Raises stat `type` (1..5: GamestateStats.stats[type - 1]) after a successful round; returns the gain. */
s32 stgtrain_session_raise_stat(StgtrainSession *obj, s32 type) {
    GamestateRecord *digimon = gamestate_data.funcs.get_record(obj->digimon);
    s32 row = 6;
    s16 *stat;
    s32 n;

    if ((u32)(type - 1) >= 5) {
        return 0;
    }
    stat = &digimon->stats.values[type + 5];
    if (obj->training < 13) {
        row = 0;
    }
    row += obj->bonus * 3 + obj->main->level;
    if (stgtrain_stat_gains[row].rand != 0) {
        n = stgtrain_stat_gains[row].base + pad_random.next() % stgtrain_stat_gains[row].rand;
    } else {
        n = stgtrain_stat_gains[row].base;
    }
    *stat += n;
    if (*stat >= 1000) {
        *stat = 999;
    }
    return n;
}

/* Lowers stat `type` (1..5) after a failed round, one time in two; returns the loss. */
s32 stgtrain_session_lower_stat(StgtrainSession *obj, s32 type) {
    GamestateRecord *digimon = gamestate_data.funcs.get_record(obj->digimon);
    s16 *stat;
    s32 row;
    s32 n;

    if ((u32)(type - 1) >= 5 || (pad_random.next() & 1)) {
        return 0;
    }
    stat = &digimon->stats.values[type + 5];
    row = obj->main->level;
    if (stgtrain_stat_losses[row].rand != 0) {
        n = stgtrain_stat_losses[row].base + pad_random.next() % stgtrain_stat_losses[row].rand;
    } else {
        n = stgtrain_stat_losses[row].base;
    }
    *stat -= n;
    if (*stat < 0) {
        *stat = 0;
    }
    return n;
}

/* Raises stat `type` (8..14: GamestateStats.resists[type - 8]) by the Digimon's gain class for it and the stat's
 * size; returns the gain. */
s32 stgtrain_session_raise_stat2(StgtrainSession *obj, s32 type) {
    GamestateRecord *digimon;
    s16 *stat;
    RecordsDigimon *info;
    StgtrainGain *gains;
    s32 row;
    s32 n;

    if ((u32)(type - 8) >= 7) {
        return 0;
    }
    digimon = gamestate_data.funcs.get_record(obj->digimon);
    info = &records_digimon[obj->digimon];
    stat = &digimon->stats.values[type + 4];
    if (*stat < 100) {
        gains = stgtrain_resist_gain_tables[(info->resist_gains[type - 8] - 1) * 3];
    } else if (*stat < 300) {
        gains = stgtrain_resist_gain_tables[(info->resist_gains[type - 8] - 1) * 3 + 1];
    } else {
        gains = stgtrain_resist_gain_tables[(info->resist_gains[type - 8] - 1) * 3 + 2];
    }
    /* row = 0 in an else, not set first: set first, it goes into the bonus test's delay slot (fill_simple) before
     * fill_eager handles the gain tests, and reorg then counts row's a0 as live at the third gain branch, so the
     * table's lui (a0) can't take the second test's delay slot as in the original. */
    if (obj->bonus != 0) {
        row = 2;
        if (obj->training < 13) {
            row = 1;
        }
    } else {
        row = 0;
    }
    row += obj->main->level * 3;
    if (gains[row].rand != 0) {
        n = gains[row].base + pad_random.next() % gains[row].rand;
    } else {
        n = gains[row].base;
    }
    *stat += n;
    if (*stat >= 1000) {
        *stat = 999;
    }
    return n;
}

/* Raises max HP (type 15) or max MP (16); returns the gain. */
s32 stgtrain_session_raise_max(StgtrainSession *obj, s32 type) {
    GamestateRecord *digimon;
    s16 *stat;
    s32 row;
    s32 n;

    if ((u32)(type - 15) >= 2) {
        return 0;
    }
    digimon = gamestate_data.funcs.get_record(obj->digimon);
    if (type == 15) {
        stat = &digimon->stats.values[3];
    } else {
        stat = &digimon->stats.values[5];
    }
    row = 0;
    if (obj->bonus != 0) {
        row = 2;
        if (obj->training < 13) {
            row = 1;
        }
    }
    row += obj->main->level * 3;
    if (stgtrain_max_gains[row].rand != 0) {
        n = stgtrain_max_gains[row].base + pad_random.next() % stgtrain_max_gains[row].rand;
    } else {
        n = stgtrain_max_gains[row].base;
    }
    *stat += n;
    if (*stat >= 10000) {
        *stat = 9999;
    }
    return n;
}



/* Applies round `round`'s result to the stats and shows it. */
void stgtrain_session_apply_round(StgtrainSession *obj, s32 round) {
    StgtrainSessionData *data = (StgtrainSessionData *)obj->base.children;
    StgtrainMenuEntry *entry = stgtrain_module.funcs.find_menu_entry(obj->map, obj->training);

    if (obj->results[round] != 0 && entry->stat != 0) {
        if (entry->stat >= 1 && entry->stat <= 5) {
            obj->gains[round] = stgtrain_session_raise_stat(obj, entry->stat);
        } else {
            obj->gains[round] = stgtrain_session_raise_stat2(obj, entry->stat);
            if (entry->stat_2 != 0) {
                if (entry->stat_2 >= 1 && entry->stat_2 <= 5) {
                    obj->losses[round] = stgtrain_session_lower_stat(obj, entry->stat_2);
                } else {
                    obj->losses[round] = stgtrain_session_raise_max(obj, entry->stat_2);
                }
            }
        }
    }
    if (obj->results[round] != 0) {
        if (entry->stat >= 1 && entry->stat <= 5) {
            data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), 0x5C);
            data->result->set_line_text(data->result, cdload_module.files.get_file(records_language + 0x10B), entry->stat + 0x46, 1);
            data->result->set_line_number(data->result, 2, obj->gains[round]);
            data->result->set_palette(data->result, 1);
        } else {
            data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), 0x5C);
            data->result->set_line_text(data->result, cdload_module.files.get_file(records_language + 0x10B), entry->stat + 0x4D, 1);
            data->result->set_line_number(data->result, 2, obj->gains[round]);
            data->result->set_palette(data->result, 1);
            if (entry->stat_2 == 0) {
                return;
            }
            if (entry->stat_2 >= 1 && entry->stat_2 <= 5) {
                if (obj->losses[round] != 0) {
                    data->result_2->set_text(data->result_2, cdload_module.files.get_file(records_language + 0x10B), 0x5D);
                    data->result_2->set_line_text(data->result_2, cdload_module.files.get_file(records_language + 0x10B), entry->stat_2 + 0x46, 1);
                    data->result_2->set_line_number(data->result_2, 2, obj->losses[round]);
                    data->result_2->set_palette(data->result_2, 5);
                    return;
                }
            } else {
                data->result_2->set_text(data->result_2, cdload_module.files.get_file(records_language + 0x10B), 0x5C);
                data->result_2->set_line_text(data->result_2, cdload_module.files.get_file(records_language + 0x10B), entry->stat_2 + 0x44, 1);
                data->result_2->set_line_number(data->result_2, 2, obj->losses[round]);
                data->result_2->set_palette(data->result_2, 1);
                return;
            }
        }
    } else {
        data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), 0x46);
        data->result->set_palette(data->result, 0);
    }
    data->result_2->set_visible(data->result_2, 0);
}

/* The success bonus of the Digimon's accessories (items 0x151: 3, 0x152: 6). */
s32 stgtrain_session_get_bonus(StgtrainSession *obj) {
    GamestateRecord *digimon = gamestate_data.funcs.get_record(obj->digimon);

    if (digimon->equipment[4] == 0x151 || digimon->equipment[5] == 0x151) {
        return 3;
    }
    if (digimon->equipment[4] == 0x152 || digimon->equipment[5] == 0x152) {
        return 6;
    }
    return 0;
}

void stgtrain_session_create_windows(StgtrainSession *obj, StgtrainSessionData *data) {
    data->result = message_create_window(obj->layer, 1, 0x74, 0xC0);
    data->result->set_page_lines(data->result, 2);
    data->result_2 = message_create_window(obj->layer, 1, 0x74, 0xCE);
    data->bonus_question = message_create_window(0x1002, 1, 0xA2, 0x75);
    data->yes = message_create_window(0x1002, 1, 0xA2, 0x91);
    data->no = message_create_window(0x1002, 1, 0xA2, 0xA1);
    data->cursor = message_create_cursor(0x1002, obj->ot_depth - 1, 0x94, 0x91);
    data->cursor->show(data->cursor, 0);
}

/* Draws the "next" arrow, the five rounds' marks and the panels. */
void stgtrain_session_draw(StgtrainSession *obj) {
    Sprite spr;
    s32 i;

    if (obj->arrow != 0) {
        sprite_init(&spr);
        spr.set_layer_id(0x1002, 3);
        spr.set_vram_pos(0x140, 0);
        if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
            obj->arrow_time = gfx_module.funcs.get_time();
            obj->arrow_frame++;
            if (obj->arrow_frame >= 5) {
                obj->arrow_frame = 0;
            }
        }
        spr.set_palette(obj->arrow_frame);
        spr.draw(cdload_module.get_subfile_by_id(0x02860000), 0xA, 0x124, 0xCD);
    }
    sprite_init(&spr);
    spr.set_vram_pos(0x240, 0x100);
    spr.set_layer_id(obj->layer, obj->ot_depth);
    if (obj->bonus_running != 0) {
        if (gfx_module.funcs.get_time() - obj->blink_time >= 3) {
            obj->blink_time = gfx_module.funcs.get_time();
            obj->blink = 1 - obj->blink;
        }
        spr.set_palette(obj->blink + 1);
    }
    for (i = 0; i < 5; i++) {
        if (obj->bonus != 0 && i == 3) {
            spr.set_palette(3);
        }
        if (obj->results[i] == 1) {
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x45, 0x80 + i * 0x11, 0x58);
        } else if (obj->results[i] == 0) {
            spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x46, 0x80 + i * 0x11, 0x58);
        }
    }
    spr.set_palette(0);
    if (obj->anims[0].level != 0) {
        if (obj->anims[0].level != 0x1000) {
            spr.set_scale(obj->anims[0].level, obj->anims[0].level, 0x1000);
            spr.set_pivot(0xCF, 0x7F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x28, 0x72, 0x4B);
    }
    if (obj->anims[1].level != 0) {
        if (obj->anims[1].level != 0x1000) {
            spr.set_scale(obj->anims[1].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0xCD);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x23, 0x46, 0xBA);
    }
    spr.set_layer_id(0x1002, obj->ot_depth);
    if (obj->anims[2].level != 0) {
        if (obj->anims[2].level != 0x1000) {
            spr.set_scale(obj->anims[2].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x7B);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x26, 0x85, 0x70);
    }
    if (obj->anims[3].level != 0) {
        if (obj->anims[3].level != 0x1000) {
            spr.set_scale(obj->anims[3].level, 0x1000, 0x1000);
            spr.set_pivot(0x140, 0x9F);
        }
        spr.draw(cdload_module.get_subfile_by_id(0x028C0003), 0x1D, 0x82, 0x8B);
    }
}

/* A stat by its menu type: types 1-5 are unk_0C[0..4], types 8-14 are unk_18[0..6]. The original reads them with
 * the offset added to the struct base (`addu obj,k`), which only a separate function gives (inlined: its parameters are
 * fresh pseudos); every index form compiles to `addu k,obj`. Judgement call (DECISIONS "Inline stat accessors"). */
static inline s16 stgtrain_stat_low(GamestateStats *stats, s32 type) {
    return stats->stats[type - 1];
}

static inline s16 stgtrain_stat_high(GamestateStats *stats, s32 type) {
    return stats->resists[type - 8];
}

/* The session's steps (base.step): open, play the rounds (base.substep of base.timer), show the result,
 * offer the bonus round, close. */
void stgtrain_session_run(StgtrainSession *obj, StgtrainSessionData *data) {
    StgtrainMenuEntry *entry;
    s32 sum[2];  /* gains, losses */
    s32 shown[2];
    s32 choice;
    s32 se;
    s32 i;
    s16 stat;
    GamestateRecord *digimon;

    switch (obj->base.step) {
    case 0:
    default:
        stgtrain_module.funcs.anim_start(&obj->anims[0], 1);
        stgtrain_module.funcs.anim_start(&obj->anims[1], 1);
        data->trainee->grow(data->trainee);
        obj->base.step++;
        break;
    case 1:
        stgtrain_module.funcs.anim_update(&obj->anims[1]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[0]) && (data->trainee->state & 1)) {
            obj->base.timer = 2;
            obj->base.step++;
            data->trainee->start_round(data->trainee);
            data->trainee->set_chance(data->trainee, stgtrain_session_get_bonus(obj) + 75);
        }
        break;
    case 2:
        obj->results[obj->base.substep] = data->trainee->get_result(data->trainee);
        if (obj->results[obj->base.substep] != -1) {
            if (obj->results[obj->base.substep] != 0) {
                sound_module.play(0x840001);
            } else {
                sound_module.play(0x840000);
            }
            stgtrain_session_apply_round(obj, obj->base.substep);
            obj->base.substep++;
            if (obj->base.timer < obj->base.substep) {
                if (obj->base.timer == 2) {
                    if (obj->results[0] != 0 && obj->results[1] != 0 && obj->results[2] != 0) {
                        obj->base.step = 10;
                        break;
                    }
                    if (obj->training >= 13) {
                        obj->base.timer = 4;
                        data->trainee->start_round(data->trainee);
                        break;
                    }
                }
                obj->base.set_step(obj, 3);
                break;
            }
            data->trainee->start_round(data->trainee);
        }
        break;
    case 3:
        data->trainee->end(data->trainee);
        obj->base.step++;
        break;
    case 4:
        if (data->trainee->state & 1) {
            obj->arrow = 1;
            obj->base.step++;
        }
        break;
    case 5:
        if (PAD_PRESSED(13)) {
            sound_module.play(0x4001C);
            obj->arrow = 0;
            entry = stgtrain_module.funcs.find_menu_entry(obj->map, obj->training);
            if (entry->stat != 0) {
                sum[0] = 0;
                for (i = 0; i < 5; i++) {
                    sum[0] += obj->gains[i];
                }
                if (entry->stat >= 8 && entry->stat <= 14) {
                    if (entry->stat_2 != 0) {
                        sum[1] = 0;
                        for (i = 0; i < 5; i++) {
                            sum[1] += obj->losses[i];
                        }
                    }
                } else {
                    sum[1] = 0;
                }
            }
            if (sum[0] == 0 && sum[1] == 0) {
                data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), 0x71);
            } else {
                for (i = 0; i < 2; i++) {
                    shown[i] = sum[i];
                }
                if (sum[0] != 0) {
                    if (entry->stat >= 1 && entry->stat <= 5) {
                        stat = stgtrain_stat_low(&obj->old, entry->stat);
                        if (stat + sum[0] >= 1000) {
                            shown[0] = 999 - stat;
                        } else {
                            shown[0] = sum[0];
                        }
                    } else {
                        stat = stgtrain_stat_high(&obj->old, entry->stat);
                        if (stat + sum[0] >= 1000) {
                            shown[0] = 999 - stat;
                        } else {
                            shown[0] = sum[0];
                        }
                    }
                }
                if (sum[1] != 0) {
                    if (entry->stat_2 >= 1 && entry->stat_2 <= 5) {
                        stat = stgtrain_stat_low(&obj->old, entry->stat_2);
                        if (stat - sum[1] < 0) {
                            shown[1] = stat;
                        } else {
                            shown[1] = sum[1];
                        }
                    } else if (entry->stat_2 >= 8 && entry->stat_2 <= 14) {
                        stat = stgtrain_stat_high(&obj->old, entry->stat_2);
                        if (stat + sum[1] >= 1000) {
                            shown[1] = 999 - stat;
                        } else {
                            shown[1] = sum[1];
                        }
                    } else if (entry->stat_2 == 15) {
                        stat = obj->old.values[3];
                        if (stat + sum[1] >= 10000) {
                            shown[1] = 9999 - stat;
                        } else {
                            shown[1] = sum[1];
                        }
                    } else if (entry->stat_2 == 16) {
                        stat = obj->old.values[5];
                        if (stat + sum[1] >= 10000) {
                            shown[1] = 9999 - stat;
                        } else {
                            shown[1] = sum[1];
                        }
                    }
                }
                if (entry->stat >= 1 && entry->stat <= 5) {
                    data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), entry->stat + 0x5D);
                    data->result->set_line_number(data->result, 1, shown[0]);
                } else if (sum[1] != 0) {
                    data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), entry->stat + 0x62);
                    data->result->set_line_number(data->result, 1, shown[0]);
                    data->result->set_line_number(data->result, 2, shown[1]);
                } else {
                    data->result->set_text(data->result, cdload_module.files.get_file(records_language + 0x10B), entry->stat + 0x5B);
                    data->result->set_line_number(data->result, 1, shown[0]);
                }
            }
            data->result->set_palette(data->result, 0);
            data->result->set_speed(data->result, 6);
            data->result_2->set_visible(data->result_2, 0);
            obj->base.step++;
            obj->main->show_change(obj->main, &obj->old);
        }
        break;
    case 6:
        if (data->result->is_done(data->result) != 0) {
            sound_module.play(0x4001C);
            obj->base.step = 50;
            obj->arrow = 0;
            obj->main->show_change(obj->main, NULL);
        } else if (data->result->is_waiting(data->result) != 0) {
            obj->arrow = 1;
            if (PAD_PRESSED(13)) {
                sound_module.play(0x4001C);
                obj->arrow = 0;
            }
        } else {
            obj->arrow = 0;
            if (PAD_PRESSED(13)) {
                data->result->find_page_end(data->result);
            }
        }
        break;
    case 10:
        if (obj->training < 13) {
            obj->base.step = 20;
            obj->bonus_running = 1;
            data->trainee->start_round(data->trainee);
            obj->sound = sound_module.play(0xA084603C);
            break;
        }
        obj->base.step++;
        break;
    case 11:
        stgtrain_module.funcs.anim_start(&obj->anims[2], 1);
        obj->base.step++;
        break;
    case 12:
        if (stgtrain_module.funcs.anim_update(&obj->anims[2])) {
            data->bonus_question->set_text(data->bonus_question, cdload_module.files.get_file(records_language + 0x10B), 0x11);
            stgtrain_module.funcs.anim_start(&obj->anims[3], 1);
            obj->base.step++;
        }
        break;
    case 13:
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            data->yes->set_text(data->yes, cdload_module.files.get_file(records_language + 0x10B), 0xC);
            data->no->set_text(data->no, cdload_module.files.get_file(records_language + 0x10B), 0xD);
            data->cursor->set_pos(data->cursor, 0x94, obj->choice * 0x10 + 0x91);
            data->cursor->show(data->cursor, 1);
            obj->base.step++;
        }
        break;
    case 14:
        choice = obj->choice;
        if (PAD_PRESSED(4) || PAD_REPEAT(4)) {
            obj->choice = 0;
        } else if (PAD_PRESSED(6) || PAD_REPEAT(6)) {
            obj->choice = 1;
        }
        if (choice != obj->choice) {
            sound_module.play(0x8004513E);
            data->cursor->set_pos(data->cursor, 0x94, obj->choice * 0x10 + 0x91);
        } else if (PAD_PRESSED(13)) {
            sound_module.play(0x8004503C);
            if (obj->choice == 0) {
                obj->base.timer = 0;
                obj->base.step++;
            } else {
                obj->base.timer = 4;
                obj->base.step++;
            }
        } else if (PAD_PRESSED(14)) {
            sound_module.play(0x800450BD);
            obj->base.timer = 4;
            obj->base.step++;
        }
        break;
    case 15:
        stgtrain_module.funcs.anim_start(&obj->anims[2], 0);
        stgtrain_module.funcs.anim_start(&obj->anims[3], 0);
        data->bonus_question->set_visible(data->bonus_question, 0);
        data->yes->set_visible(data->yes, 0);
        data->no->set_visible(data->no, 0);
        data->cursor->show(data->cursor, 0);
        obj->base.step++;
        break;
    case 16:
        stgtrain_module.funcs.anim_update(&obj->anims[2]);
        if (stgtrain_module.funcs.anim_update(&obj->anims[3])) {
            if (obj->base.timer == 0) {
                obj->base.step = 20;
                obj->bonus_running = 1;
                data->trainee->start_round(data->trainee);
                obj->sound = sound_module.play(0xA084603C);
                digimon = gamestate_data.funcs.get_record(obj->digimon);
                if (digimon->last_bonus_training != 0 && digimon->last_bonus_training == obj->training) {
                    data->trainee->set_chance(data->trainee, 0); /* not the same bonus training twice in a row */
                } else {
                    digimon->last_bonus_training = 0;
                    data->trainee->set_chance(data->trainee, 50);
                }
            } else {
                data->trainee->start_round(data->trainee);
                obj->base.step = 2;
            }
        }
        break;
    case 20:
        obj->results[3] = data->trainee->get_result(data->trainee);
        if (obj->results[3] != -1) {
            digimon = gamestate_data.funcs.get_record(obj->digimon);
            if (obj->results[3] != 0) {
                digimon->last_bonus_training = obj->training;
                obj->bonus = 1;
            } else {
                digimon->last_bonus_training = 0;
            }
            if (obj->results[3] != 0) {
                sound_module.play(0x840001);
            } else {
                sound_module.play(0x840000);
            }
            sound_module.key_off(0xA084603C, obj->sound);
            stgtrain_session_apply_round(obj, 3);
            obj->arrow = 1;
            obj->bonus_running = 0;
            obj->base.set_step(obj, 3);
        }
        break;
    case 50:
        obj->base.state = OBJECT_STATE_END;
        break;
    }
}

void stgtrain_session_update(StgtrainSession *obj, StgtrainSessionData *data) {
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        stgtrain_session_create_windows(obj, data);
        obj->anims[0].duration = 10;
        obj->anims[1].duration = 10;
        obj->anims[2].duration = 10;
        obj->anims[3].duration = 10;
        for (i = 0; i < 5; i++) {
            obj->results[i] = -1;
        }
        data->trainee = stgtrain_trainee_create(obj->digimon, obj->training, obj->layer, obj->ot_depth - 3);
        data->trainee->set_tex(data->trainee, 0x300, 0);
        data->trainee->set_clut(data->trainee, 0x2C0, 0);
        data->trainee->pause(data->trainee);
        data->trainee->set_scale(data->trainee, 0);
        gamestate_data.funcs.get_stats(obj->digimon, &obj->old);
        break;
    case OBJECT_STATE_RUN:
        stgtrain_session_run(obj, data);
        stgtrain_session_draw(obj);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

StgtrainSession *stgtrain_session_create(StgtrainMain *main, s32 digimon, s32 training) {
    StgtrainSession *obj = object_new(stgtrain_session_update, sizeof(StgtrainSession), sizeof(StgtrainSessionData));

    obj->layer = 0x1000;
    obj->ot_depth = 6;
    obj->main = main;
    obj->digimon = digimon;
    obj->training = training;
    obj->map = gamestate_data.funcs.get_map_entry();
    return obj;
}

StgtrainGain stgtrain_stat_gains[12] = {
    { 1, 2 }, { 7, 2 }, { 14, 3 }, { 2, 0 }, { 10, 0 }, { 20, 0 },
    { 1, 2 }, { 6, 3 }, { 11, 5 }, { 4, 0 }, { 22, 0 }, { 48, 0 },
};
StgtrainGain stgtrain_stat_losses[3] = {
    { 1, 0 }, { 2, 3 }, { 4, 3 },
};
StgtrainGain stgtrain_resist_gains_0[9] = {
    { 1, 0 }, { 1, 0 }, { 2, 0 }, { 4, 2 }, { 8, 0 }, { 12, 0 }, { 8, 2 }, { 20, 0 }, { 30, 0 },
};
StgtrainGain stgtrain_resist_gains_1[9] = {
    { 1, 2 }, { 2, 0 }, { 3, 0 }, { 6, 3 }, { 12, 0 }, { 18, 0 }, { 12, 4 }, { 30, 0 }, { 40, 0 },
};
StgtrainGain stgtrain_resist_gains_2[9] = {
    { 2, 0 }, { 2, 0 }, { 4, 0 }, { 8, 4 }, { 12, 0 }, { 25, 0 }, { 16, 5 }, { 30, 0 }, { 50, 0 },
};
StgtrainGain *stgtrain_resist_gain_tables[15] = {
    stgtrain_resist_gains_0, stgtrain_resist_gains_1, stgtrain_resist_gains_1, stgtrain_resist_gains_0, stgtrain_resist_gains_1,
    stgtrain_resist_gains_1, stgtrain_resist_gains_1, stgtrain_resist_gains_1, stgtrain_resist_gains_1, stgtrain_resist_gains_2,
    stgtrain_resist_gains_2, stgtrain_resist_gains_1, stgtrain_resist_gains_2, stgtrain_resist_gains_2, stgtrain_resist_gains_1,
};
StgtrainGain stgtrain_max_gains[9] = {
    { 1, 2 }, { 2, 0 }, { 4, 0 }, { 6, 4 }, { 10, 0 }, { 20, 0 }, { 13, 5 }, { 20, 0 }, { 40, 0 },
};
