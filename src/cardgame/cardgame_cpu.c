#include "common.h"
#include "object.h"
#include "gfx.h"
#include "cardgame.h"

/* The CPU player: cardgame_cpu_choose_card (called by the game flow) and its helpers; cardgame_cpu_get_score is
 * also the game's method get_score (a value for a set of cards). */

s32 cardgame_cpu_get_score(CardgameGame *game, s32 side, s32 mask);
s32 cardgame_mark_selectable_cards(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 arg3, s32 arg4);
s32 cardgame_mark_selectable_slots(CardgameGame *game, CardgameBoard *board, s32 arg2, s32 mask);
s32 cardgame_is_card_playable(CardgameGame *game, s32 card);
s32 cardgame_get_card_data(s32 card, u32 field, s32 i); /* defined returning u8; callers use the full word */

s32 cardgame_cpu_can_use_effect(CardgameGame *game, CardgameBoard *board, s32 card) {
    CardPicture pic;
    s32 done = 0;
    s32 played;

    switch (card) {
    case 0x80:
    case 0x81:
        if (cardgame_mark_selectable_cards(game, board, 1, 4, 0xFF) != 0) {
            done = 1;
        }
        break;
    case 0x83:
        if (game->turn != 0) {
            done = 1;
        }
        break;
    case 0x84:
        if (game->turn != 0) {
            played = game->turns[game->turn - 1].card;
            card_init(&pic);
            pic.select(game->card_ids[played] + 1);
            if (pic.record[0] == 6) {
                done = 1;
            }
        }
        break;
    case 0x72:
        if (cardgame_mark_selectable_cards(game, board, 0, 3, 0xFF) != 0) {
            done = 1;
        }
        break;
    case 0x82:
        if (cardgame_mark_selectable_cards(game, board, 1, 3, 0xFF) != 0) {
            done = 1;
        }
        break;
    case 0x71:
        if (cardgame_mark_selectable_cards(game, board, 1, 2, 0xFF) != 0) {
            done = 1;
        }
        break;
    case 0x7F:
        if (cardgame_mark_selectable_cards(game, board, 0, 2, 0xFF) != 0) {
            done = 1;
        }
        break;
    case 0x96:
        if (cardgame_mark_selectable_cards(game, board, 1, 2, 0xFD) != 0) {
            done = 1;
        }
        break;
    case 0x85:
    case 0x8E:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x1FC) != 0) {
            done = 1;
        }
        break;
    case 0x87:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x180) != 0) {
            done = 1;
        }
        break;
    case 0x88:
    case 0x8F:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x2FC) != 0) {
            done = 1;
        }
        break;
    case 0x89:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x2BC) != 0) {
            done = 1;
        }
        break;
    case 0x8A:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x280) != 0) {
            done = 1;
        }
        break;
    case 0x8B:
    case 0x90:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x3FC) != 0) {
            done = 1;
        }
        break;
    case 0x8C:
    case 0x95:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x380) != 0) {
            done = 1;
        }
        break;
    case 0x91:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x3F8) != 0) {
            done = 1;
        }
        break;
    case 0x92:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x3F4) != 0) {
            done = 1;
        }
        break;
    case 0x93:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x310) != 0) {
            done = 1;
        }
        break;
    case 0x94:
        if (cardgame_mark_selectable_slots(game, board, 1, 0x3DC) == 0) {
            break;
        }
        /* fall through */
    case 0x70:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_cpu_get_score(CardgameGame *game, s32 side, s32 mask) {
    CardgameSortEntry list[6];
    CardgameSortEntry tmp;
    CardPicture pic;
    s32 rest_a = 0;
    s32 rest_b = 0;
    s32 sum_a = 0;
    s32 sum_b = 0;
    s32 count = game->slots[side].count;
    s32 cur_a;
    s32 cur_b;
    s32 skip;
    s32 i;
    s32 j;

    card_init(&pic);
    for (i = 0; i < count; i++) {
        list[i].id = game->card_ids[game->slots[side].slots[i].card];
        list[i].index = i;
    }
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (list[i].id > list[j].id) {
                tmp = list[i];
                list[i] = list[j];
                list[j] = tmp;
            }
        }
    }
    j = 0;
    i = (mask >> list[0].index) & 1;
    cur_a = game->slots[side].slots[list[0].index].attack;
    cur_b = game->slots[side].slots[list[0].index].hp;
    for (skip = 0; i < count - 1; i++, skip = 0) {
        pic.select(list[i].id + 1);
        if ((mask >> list[i + 1].index) & 1) {
            if (i == count - 2) {
                break;
            }
            skip = 1;
        }
        if (*(s16 *)(pic.record + 10) != 0 && list[i].id == list[i + 1 + skip].id) {
            j++;
            cur_a += game->slots[side].slots[list[i + skip].index].attack;
            cur_b += game->slots[side].slots[list[i + skip].index].hp;
        } else if (j < 2) {
            j = 0;
            if (i + 1 + skip < count) {
                cur_a = game->slots[side].slots[list[i + 1 + skip].index].attack;
                cur_b = game->slots[side].slots[list[i + 1 + skip].index].hp;
            } else {
                cur_a = 0;
                cur_b = 0;
            }
        } else {
            sum_a += cur_a;
            sum_b += cur_b;
            if (i + 1 + skip < count) {
                cur_a = game->slots[side].slots[list[i + 1 + skip].index].attack;
                cur_b = game->slots[side].slots[list[i + 1 + skip].index].hp;
            } else {
                cur_a = 0;
                cur_b = 0;
            }
            if (j >= 3) {
                sum_a += 20;
                sum_b += 20;
                j = 0;
                break;
            }
            j = 0;
        }
        if (skip) {
            i++;
        }
    }
    if (j >= 2) {
        sum_a += cur_a;
        sum_b += cur_b;
        if (j >= 3) {
            sum_a += 20;
            sum_b += 20;
        }
    }
    for (i = 0; i < count; i++) {
        if (!((mask >> i) & 1)) {
            rest_a += game->slots[side].slots[i].attack;
            rest_b += game->slots[side].slots[i].hp;
        }
    }
    return rest_a + rest_b + sum_a + sum_b;
}

void cardgame_cpu_keep_best_target(CardgameGame *game, s32 side) {
    s32 best = 0xFFF;
    s32 besti = 0;
    s8 *flags;
    s32 i;
    s32 v;

    if (side == 1) {
        flags = &game->selectable[6];
    } else {
        flags = &game->selectable[0];
    }
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            v = cardgame_cpu_get_score(game, side, 1 << i);
            if (best >= v) {
                best = v;
                flags[besti] = 0;
                besti = i;
                flags[i] = 1;
            } else {
                flags[i] = 0;
            }
        }
    }
}

s32 cardgame_cpu_set_target(CardgameGame *game, s32 side) {
    s32 found = 0;
    s8 *flags;
    s32 i;

    if (side == 1) {
        flags = &game->selectable[6];
    } else {
        flags = &game->selectable[0];
    }
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            found = 1;
            game->turns[game->turn].target = game->slots[side].slots[i].id;
            break;
        }
    }
    return found;
}

s32 cardgame_cpu_set_target_kind_3(CardgameGame *game, s32 side) {
    CardPicture pic;
    s8 *flags;
    s32 i;

    card_init(&pic);
    if (side == 1) {
        flags = &game->selectable[6];
    } else {
        flags = &game->selectable[0];
    }
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            pic.select(game->card_ids[game->slots[side].slots[i].card] + 1);
            if (pic.record[0] != 3) {
                flags[i] = 0;
            }
        }
    }
    return cardgame_cpu_set_target(game, side);
}

s32 cardgame_cpu_set_target_kind_4(CardgameGame *game, s32 side) {
    CardPicture pic;
    s8 *flags;
    s32 i;

    card_init(&pic);
    if (side == 1) {
        flags = &game->selectable[6];
    } else {
        flags = &game->selectable[0];
    }
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            pic.select(game->card_ids[game->slots[side].slots[i].card] + 1);
            if (pic.record[0] != 4) {
                flags[i] = 0;
            }
        }
    }
    return cardgame_cpu_set_target(game, side);
}

s32 cardgame_cpu_is_target_within(CardgameGame *game, s32 side, s32 i, s32 max) {
    s32 ok = 0;

    if (*(game->selectable + i) != 0) {
        ok = max >= game->slots[side].slots[i].hp;
    }
    return ok;
}

s32 cardgame_cpu_filter_targets(CardgameGame *game, s32 side, s32 max, s32 keep) {
    s32 result = 0;
    s32 last = 0;
    s8 *flags;
    s32 i;
    s32 any;

    if (side == 1) {
        flags = &game->selectable[6];
    } else {
        flags = &game->selectable[0];
    }
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            last = i;
            if (max < game->slots[side].slots[i].hp) {
                flags[i] = 0;
            }
        }
    }
    any = 0;
    for (i = 0; i < game->slots[side].count; i++) {
        if (flags[i] != 0) {
            any = 1;
            break;
        }
    }
    if (any == 0) {
        if (keep == 0) {
            flags[last] = 1;
        }
        result = 1;
    }
    return result;
}

s32 cardgame_cpu_set_cheapest_target(CardgameGame *game) {
    CardPicture pic;
    s32 min = 500;
    s32 best = 500;
    s8 *flags = game->selectable;
    s32 found = 0;
    s32 i;

    card_init(&pic);
    for (i = 0; i < game->players[1].hand_count; i++) {
        if (flags[i] != 0) {
            found = 1;
            pic.select(game->card_ids[game->players[1].hand[i]] + 1);
            if (*(s16 *)(pic.record + 8) < min) {
                min = *(s16 *)(pic.record + 8);
                best = i;
            }
        }
    }
    if (found == 1) {
        game->turns[game->turn].target = game->players[1].hand[best];
    }
    return found;
}

s32 cardgame_cpu_get_card_damage(CardgameGame *game, s32 card) {
    s32 value = 0;

    switch (card) {
    case 0x12:
        value = 60;
        break;
    case 0x14:
        value = 15;
        break;
    case 0x38:
        value = 10;
        break;
    case 0x15:
    case 0x2E:
        value = 30;
        break;
    }
    return value;
}

s32 cardgame_cpu_get_card_heal(CardgameGame *game, s32 card) {
    s32 value = 0;

    switch (card) {
    case 0x2:
        value = 15;
        break;
    case 0xC:
        value = 50;
        break;
    case 0xF:
        value = 20;
        break;
    case 0x3:
    case 0x28:
        value = 30;
        break;
    case 0x3A:
        value = 10;
        break;
    }
    return value;
}

s32 cardgame_cpu_choose_target(CardgameGame *game, CardgameBoard *board, s32 card) {
    s32 done = 0;

    switch (card) {
    case 39:
        if (cardgame_cpu_set_cheapest_target(game) != 0) {
            done = 1;
        }
        break;
    case 7:
    case 9:
    case 26:
    case 55:
    case 57:
        cardgame_cpu_keep_best_target(game, 0);
        if (cardgame_cpu_set_target(game, 0) != 0) {
            done = 1;
        }
        break;
    case 21:
    case 46:
        cardgame_cpu_filter_targets(game, 0, 30, 0);
        cardgame_cpu_keep_best_target(game, 0);
        if (cardgame_cpu_set_target(game, 0) != 0) {
            done = 1;
        }
        break;
    case 56:
        cardgame_cpu_filter_targets(game, 0, 10, 0);
        cardgame_cpu_keep_best_target(game, 0);
        if (cardgame_cpu_set_target(game, 0) != 0) {
            done = 1;
        }
        break;
    case 40:
        if (cardgame_cpu_set_target(game, 1) != 0) {
            done = 1;
        }
        break;
    case 3:
    case 15:
    case 58:
    case 59:
        if (game->slots[1].count != 0) {
            done = 1;
            game->turns[game->turn].target = game->slots[1].slots[0].id;
        }
        break;
    default:
        done = 1;
        break;
    }
    return done;
}

s32 cardgame_cpu_choose_counter_target(CardgameGame *game, s32 card1, s32 card2, s32 cpu) {
    s32 slot = 0;
    s32 done = 0;
    s32 value;
    s32 bonus;
    s32 i;

    if (cpu == 0) {
        value = cardgame_cpu_get_card_damage(game, card2);
        bonus = cardgame_cpu_get_card_heal(game, card1);
    } else {
        value = cardgame_cpu_get_card_damage(game, card1);
        bonus = cardgame_cpu_get_card_heal(game, card2);
    }
    switch (game->turns[game->turn - 1].target_kind) {
    case 0:
        for (i = 0; i < 12; i++) {
            if (i < 6) {
                if (game->turns[game->turn - 1].target == game->slots[0].slots[i].id) {
                    slot = i;
                    break;
                }
            } else if (game->turns[game->turn - 1].target == game->slots[1].slots[i - 6].id) {
                slot = i - 6;
                break;
            }
        }
        if (cpu == 0) {
            if (cardgame_cpu_is_target_within(game, 0, slot, value) != 0) {
                done = 1;
                game->turns[game->turn].target = game->slots[0].slots[slot].id;
            }
        } else if (cardgame_cpu_is_target_within(game, 1, slot, value) != 0) {
            if (cardgame_cpu_is_target_within(game, 1, slot, value - bonus) == 0) {
                done = 1;
                game->turns[game->turn].target = game->slots[1].slots[slot].id;
            }
        }
        break;
    case 1:
        if (cpu == 0) {
            cardgame_cpu_filter_targets(game, 0, value, 0);
            cardgame_cpu_keep_best_target(game, 0);
            if (cardgame_cpu_set_target(game, 0) != 0) {
                done = 1;
            }
        }
        break;
    case 3:
        if (cpu == 0) {
            cardgame_cpu_filter_targets(game, 0, value, 0);
            cardgame_cpu_keep_best_target(game, 0);
            if (cardgame_cpu_set_target(game, 0) != 0) {
                done = 1;
            }
        } else if (cardgame_cpu_filter_targets(game, 1, value, 0) != 0) {
            if (cardgame_cpu_set_target(game, 1) != 0) {
                done = 1;
            }
        }
        break;
    case 6:
        if (cpu == 0) {
            cardgame_cpu_filter_targets(game, 0, value, 0);
            cardgame_cpu_keep_best_target(game, 0);
            if (cardgame_cpu_set_target_kind_3(game, 0) != 0) {
                done = 1;
            }
        }
        break;
    case 7:
        if (cpu != 0) {
            cardgame_cpu_keep_best_target(game, 1);
            if (cardgame_cpu_filter_targets(game, 1, value, 0) != 0) {
                if (cardgame_cpu_set_target_kind_4(game, 1) != 0) {
                    done = 1;
                }
            }
        }
        break;
    }
    return done;
}

s32 cardgame_cpu_find_kind(CardgameGame *game, s32 kind) {
    s32 i;

    for (i = 0; i < game->players[1].hand_count; i++) {
        if (game->cpu_cards[game->players[1].hand[i] - 40].kind == kind) {
            break;
        }
    }
    return i;
}

s32 cardgame_cpu_choose_card(CardgameGame *game, CardgameBoard *board) {
    CardPicture pic;
    s32 chosen = 0;
    s32 i;
    s32 j;
    s32 card;
    s32 kind;
    s32 start;
    s32 found;
    s32 last;
    u8 *played;
    u8 *data;
    s32 flag;
    s32 value0;
    s32 value1;

    for (i = 0; i < game->players[1].hand_count; i++) {
        game->marked[i] = 0;
    }
    if (game->turn == 0) {
        if (game->phase == 5) {
            start = cardgame_cpu_find_kind(game, 1);
            kind = 1;
        } else {
            start = cardgame_cpu_find_kind(game, 3);
            value0 = cardgame_cpu_get_score(game, 0, 0);
            value1 = cardgame_cpu_get_score(game, 1, 0);
            kind = 3;
            if ((game->swap_count & 1) || value0 < value1) {
                return 0;
            }
        }
        for (i = start; i < game->players[1].hand_count; i++) {
            card = game->players[1].hand[i];
            if (game->cpu_cards[card - 40].kind == kind && cardgame_is_card_playable(game, card) != 0) {
                s32 id = game->card_ids[card];

                if (cardgame_cpu_can_use_effect(game, board, cardgame_get_card_data(id, 0, 0)) != 0 &&
                    cardgame_cpu_choose_target(game, board, id) != 0) {
                    game->marked[i] = 1;
                    chosen = 1;
                    break;
                }
            }
        }
    } else {
        found = 0;
        for (j = 0; game->cpu_counter_ids[j] != 0xFF; j++) {
            if (game->cpu_counter_ids[j] == game->card_ids[game->turns[game->turn - 1].card]) {
                found = 1;
                break;
            }
        }
        if (found) {
            for (i = cardgame_cpu_find_kind(game, 4); i < game->players[1].hand_count; i++) {
                card = game->players[1].hand[i];
                if (cardgame_is_card_playable(game, card) != 0) {
                    s32 id = game->card_ids[card];

                    if (cardgame_cpu_can_use_effect(game, board, cardgame_get_card_data(id, 0, 0)) != 0 &&
                        cardgame_cpu_choose_target(game, board, id) != 0) {
                        game->marked[i] = 1;
                        chosen = 1;
                        break;
                    }
                }
            }
        }
        if (chosen == 0) {
            card_init(&pic);
            last = game->turns[game->turn - 1].card;
            pic.select(game->card_ids[last] + 1);
            played = pic.record;
            if (played[3] == 3 || played[3] == 9) {
                start = cardgame_cpu_find_kind(game, 3);
                kind = 3;
                for (i = start; i < game->players[1].hand_count; i++) {
                    card = game->players[1].hand[i];
                    if ((game->cpu_cards[card - 40].kind == kind || game->cpu_cards[card - 40].kind == 4) &&
                        game->cpu_cards[card - 40].unk_01 != 0) {
                        pic.select(game->card_ids[card] + 1);
                        data = pic.record;
                        if (played[3] == 3) {
                            if (data[3] != 9) {
                                continue;
                            }
                            flag = 0;
                        } else {
                            if (data[3] != 3) {
                                continue;
                            }
                            flag = 1;
                        }
                        if (cardgame_is_card_playable(game, card) != 0 &&
                            cardgame_cpu_can_use_effect(game, board, cardgame_get_card_data(game->card_ids[card], 0, 0)) != 0 &&
                            cardgame_cpu_choose_counter_target(game, last, card, flag) != 0) {
                            game->marked[i] = 1;
                            chosen = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return chosen;
}
