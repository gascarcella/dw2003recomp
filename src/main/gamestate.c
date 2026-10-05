#include "common.h"

#include "object.h"
#include "records.h"
#include "gamestate.h"
#include "heap.h"
#include "pad.h"
#include "cdload.h"
#include "sound.h"
#include "gfx.h"
#include "psyq/libc2.h"

/* Entry of gamestate_conditions (gamestate_check_condition), terminated by id 0xFF. */
typedef struct GamestateCondition {
    /* 0x0 */ u8 id;
    /* 0x1 */ u8 type; /* high nibble: which check function; low nibble: its first argument */
    /* 0x2 */ u8 arg;
} GamestateCondition; /* size 0x3 */

s32 gamestate_check_flags(u16 *list);
void gamestate_set_flag(s32, s32);
s32 gamestate_get_flag(u16 flag, u16 value);
void gamestate_set_flags(u16 *list);
void gamestate_update_map_flags(void);

GamestateFlags gamestate_flags = {
    { 0 },
    0,
    gamestate_check_flags,
    gamestate_set_flag,
    gamestate_get_flag,
    gamestate_set_flags,
    gamestate_update_map_flags,
};

GamestateCondition gamestate_conditions[] = {
    { 0x3A, 0x00, 0 }, { 0x96, 0x01, 0 }, { 0x00, 0x10, 0 }, { 0x01, 0x10, 1 },
    { 0x02, 0x10, 2 }, { 0x10, 0x11, 0 }, { 0x12, 0x11, 1 }, { 0x11, 0x11, 2 },
    { 0x0B, 0x11, 3 }, { 0x0D, 0x11, 4 }, { 0x0C, 0x11, 5 }, { 0x0F, 0x11, 6 },
    { 0x0E, 0x11, 7 }, { 0x42, 0x12, 0 }, { 0x44, 0x12, 1 }, { 0x43, 0x12, 2 },
    { 0x47, 0x12, 3 }, { 0x46, 0x12, 4 }, { 0x45, 0x12, 5 }, { 0x49, 0x12, 6 },
    { 0x48, 0x12, 7 }, { 0x37, 0x13, 0 }, { 0x39, 0x13, 1 }, { 0x38, 0x13, 2 },
    { 0x31, 0x13, 3 }, { 0x32, 0x13, 4 }, { 0x33, 0x13, 5 }, { 0x35, 0x13, 6 },
    { 0x34, 0x13, 7 }, { 0x4A, 0x14, 0 }, { 0x4C, 0x14, 1 }, { 0x4B, 0x14, 2 },
    { 0x4F, 0x14, 3 }, { 0x4E, 0x14, 4 }, { 0x4D, 0x14, 5 }, { 0x51, 0x14, 6 },
    { 0x50, 0x14, 7 }, { 0x25, 0x15, 0 }, { 0x26, 0x15, 1 }, { 0x27, 0x15, 2 },
    { 0x28, 0x15, 3 }, { 0x29, 0x15, 4 }, { 0x2E, 0x16, 0 }, { 0x30, 0x16, 1 },
    { 0x2F, 0x16, 2 }, { 0x2A, 0x16, 3 }, { 0x2B, 0x16, 4 }, { 0x2C, 0x16, 5 },
    { 0x36, 0x16, 6 }, { 0x2D, 0x16, 7 }, { 0x95, 0x17, 0 }, { 0x53, 0x18, 0 },
    { 0x55, 0x19, 0 }, { 0x74, 0x20, 0 }, { 0x75, 0x20, 1 }, { 0x76, 0x20, 2 },
    { 0x77, 0x20, 3 }, { 0x78, 0x20, 4 }, { 0x79, 0x20, 5 }, { 0x7A, 0x20, 6 },
    { 0x7B, 0x20, 7 }, { 0x7C, 0x20, 8 }, { 0x7D, 0x20, 9 }, { 0x8B, 0x21, 0 },
    { 0x8C, 0x21, 1 }, { 0x8D, 0x21, 2 }, { 0x8E, 0x21, 3 }, { 0x8F, 0x21, 4 },
    { 0x90, 0x21, 5 }, { 0x91, 0x21, 6 }, { 0x92, 0x21, 7 }, { 0x7E, 0x22, 0 },
    { 0x7F, 0x22, 1 }, { 0x80, 0x22, 2 }, { 0x81, 0x22, 3 }, { 0x82, 0x22, 4 },
    { 0x83, 0x22, 5 }, { 0x84, 0x22, 6 }, { 0x85, 0x22, 7 }, { 0x86, 0x22, 8 },
    { 0x87, 0x22, 9 }, { 0x06, 0x30, 0 }, { 0x03, 0x30, 1 }, { 0x0A, 0x30, 2 },
    { 0x07, 0x30, 3 }, { 0x04, 0x30, 4 }, { 0x05, 0x30, 5 }, { 0x09, 0x30, 6 },
    { 0x14, 0x30, 7 }, { 0x08, 0x30, 8 }, { 0x3F, 0x30, 9 }, { 0x40, 0x30, 10 },
    { 0x41, 0x30, 11 }, { 0x93, 0x30, 12 }, { 0x94, 0x30, 13 }, { 0x15, 0x30, 14 },
    { 0x16, 0x30, 15 }, { 0x17, 0x30, 16 }, { 0x18, 0x30, 17 }, { 0x19, 0x30, 18 },
    { 0x1A, 0x30, 19 }, { 0x1C, 0x30, 20 }, { 0x1D, 0x30, 21 }, { 0x21, 0x30, 22 },
    { 0x1E, 0x30, 23 }, { 0x1F, 0x30, 24 }, { 0x20, 0x30, 25 }, { 0x22, 0x30, 26 },
    { 0x23, 0x30, 27 }, { 0x24, 0x30, 28 }, { 0x3B, 0x30, 29 }, { 0x3C, 0x30, 30 },
    { 0x3D, 0x30, 31 }, { 0x3E, 0x30, 32 }, { 0x71, 0x40, 0 }, { 0x70, 0x40, 1 },
    { 0x73, 0x40, 2 }, { 0x72, 0x40, 3 }, { 0x54, 0x41, 0 }, { 0x13, 0x50, 0 },
    { 0xFF, 0x00, 0 },
};

s32 gamestate_money_check[] = { 800, 1600, 2700, 4000, 6000, 8700, 11500, 17500, 24000, 32000 };
s32 gamestate_money_add[] = { 100, 300, 600, 1000, 1600, 3000, 5000, 8500 };
s32 gamestate_money_sub[] = { 800, 1600, 2700, 4000, 6000, 8700, 11500, 17500, 24000, 32000 };
u8 gamestate_progress_ranges[][2] = {
    { 2, 18 }, { 4, 23 }, { 4, 44 }, { 20, 23 }, { 24, 37 }, { 39, 42 }, { 4, 99 }, { 7, 99 }, { 24, 99 }, { 22, 42 }, { 18, 99 },
    { 20, 99 }, { 15, 99 }, { 30, 99 }, { 5, 10 }, { 15, 18 }, { 20, 22 }, { 24, 26 }, { 27, 37 }, { 39, 40 }, { 37, 38 }, { 27, 36 },
    { 34, 38 }, { 27, 38 }, { 28, 37 }, { 27, 33 }, { 4, 38 }, { 4, 21 }, { 14, 38 }, { 17, 38 }, { 10, 13 }, { 29, 38 }, { 12, 22 },
};
s32 gamestate_party_stat_levels[] = { 60, 150, 210, 285, 378, 492, 630, 795, 990, 1218, 1482, 1785, 2049, 2277, 2472 };
extern u8 gamestate_parties[][3];
extern s32 gamestate_start_deck[40];
extern s16 gamestate_item_sets[][4];
extern u16 gamestate_item_set_bonuses[][6];

void gamestate_init_records();
void gamestate_add_stat_bonus(s16 *stats, s32 type, s32 value);
/* FIELDSTG's functions, called by address (the EXE can't link against an overlay; config/fieldstg.symbols.txt):
 * fieldstg_goto_map (a map's high byte picks its overlay, overlay_files), fieldstg_start_indexed_event,
 * fieldstg_open_inn and fieldstg_start_listed_battle_func. */
LATE_FUNC(1, 0x8008B770, void, func_8008B770, (s32, s32, s32, s32, s32)); /* in an overlay */
LATE_FUNC(1, 0x8008BFA4, void, func_8008BFA4, (s32)); /* in an overlay */
LATE_FUNC(1, 0x8008C000, void, func_8008C000, (void)); /* in an overlay (FIELDSTG: takes no arguments) */
extern void (*D_8009B6A4)(s32);            /* in an overlay */

s32 gamestate_test_bit(u8 *bits, s32 index, s32 set) {
    s32 byte = index >> 3;
    s32 mask = 1 << (index & 7);

    if (set) {
        return (bits[byte] & mask) != 0;
    }
    return (bits[byte] & mask) == 0;
}

void gamestate_set_bit(u8 *bits, s32 index, s32 set) {
    s32 byte = index >> 3;
    s32 mask = 1 << (index & 7);

    if (set) {
        bits[byte] |= mask;
        return;
    }
    bits[byte] &= ~mask;
}

s32 gamestate_cond_items(s32 arg0, s32 arg1) {
    s32 ret = 0;

    switch (arg0) {
    case 0:
        if ((gamestate_data.items[7] != 0 || gamestate_data.items_equipped[7] != 0)
            && (gamestate_data.items[89] != 0 || gamestate_data.items_equipped[89] != 0)
            && (gamestate_data.items[167] != 0 || gamestate_data.items_equipped[167] != 0)) {
            ret = 1;
        }
        break;
    case 1:
        if ((gamestate_data.items[105] != 0 || gamestate_data.items_equipped[105] != 0)
            && (gamestate_data.items[119] != 0 || gamestate_data.items_equipped[119] != 0)
            && (gamestate_data.items[131] != 0 || gamestate_data.items_equipped[131] != 0)
            && (gamestate_data.items[144] != 0 || gamestate_data.items_equipped[144] != 0)
            && (gamestate_data.items[156] != 0 || gamestate_data.items_equipped[156] != 0)) {
            ret = 1;
        }
        break;
    }
    return ret;
}

s32 gamestate_cond_digimon(s32 type, s32 arg) {
    s32 ret = 0;
    s32 i;
    s32 id;
    s32 sum;
    s32 j;
    s32 member;

    switch (type) {
    case 0:
        if (gamestate_data.party_set == arg) {
            ret = 1;
        }
        break;
    case 1:
        if (gamestate_data.digimon[arg].joined != 0) {
            ret = 1;
        }
        break;
    case 2:
        for (i = 0; i < 3; i++) {
            if (gamestate_data.funcs.get_party_member(i) == arg) {
                ret = 1;
                break;
            }
        }
        break;
    case 3:
        gamestate_data.digimon[arg].joined = arg + 3;
        ret = 1;
        break;
    case 4:
        if (gamestate_data.digimon[arg].joined != 0 && gamestate_data.digimon[arg].record.stats.values[0] >= 45) {
            ret = 1;
        }
        break;
    case 5:
        sum = 0;
        for (i = 0; i < 3; i++) {
            id = gamestate_data.funcs.get_party_member(i);
            if (id >= 0) {
                sum += gamestate_data.funcs.get_record(id)->stats.values[0];
            }
        }
        if (sum >= arg * 15 + 30) {
            ret = 1;
        }
        break;
    case 6:
        if (gamestate_data.digimon[arg].joined == 0) {
            ret = 1;
        }
        break;
    case 7:
        ret = 1;
        for (j = 0; j < 8; j++) {
            if (gamestate_data.digimon[j].joined != 0 && gamestate_data.digimon[j].record.stats.values[0] < 45) {
                ret = 0;
                break;
            }
        }
        break;
    case 8:
        for (i = 0; i < 3; i++) {
            member = gamestate_data.funcs.get_party_member(i);
            if (member >= 0) {
                gamestate_data.digimon[member].record.stats.values[2] = gamestate_data.digimon[member].record.stats.values[3];
                gamestate_data.digimon[member].record.stats.values[4] = gamestate_data.digimon[member].record.stats.values[5];
            }
        }
        ret = 1;
        break;
    case 9:
        if (gamestate_data.digimon[0].joined != 0 && gamestate_data.digimon[1].joined != 0
            && gamestate_data.digimon[2].joined != 0 && gamestate_data.digimon[3].joined != 0
            && gamestate_data.digimon[4].joined != 0 && gamestate_data.digimon[5].joined != 0
            && gamestate_data.digimon[6].joined != 0 && gamestate_data.digimon[7].joined != 0) {
            ret = 1;
        }
        break;
    }
    return ret;
}

s32 gamestate_change_money(s32 type, s32 i) {
    s32 ret = 0;

    switch (type) {
    case 0:
        if (gamestate_money_check[i] <= gamestate_data.money) {
            ret = 1;
        }
        break;
    case 1:
        gamestate_data.money += gamestate_money_add[i];
        if (gamestate_data.money > 9999999) {
            gamestate_data.money = 9999999;
        }
        break;
    case 2:
        gamestate_data.money -= gamestate_money_sub[i];
        if (gamestate_data.money < 0) {
            gamestate_data.money = 0;
        }
        break;
    }
    return ret;
}

s32 gamestate_cond_progress(s32 arg0, s32 i) {
    s32 value = gamestate_data.progress;
    s32 min = gamestate_progress_ranges[i][0];
    s32 max = gamestate_progress_ranges[i][1];

    return value >= min && value <= max;
}

s32 gamestate_cond_bits(s32 type, s32 arg) {
    s32 ret = 0;
    s32 set = 0;
    s32 clear = 0;
    s32 i;

    switch (type) {
    case 0:
        for (i = 39; i < 46; i++) {
            if (gamestate_test_bit(gamestate_data.flags_1C, i, 1)) {
                set++;
            } else {
                clear++;
            }
        }
        switch (arg) {
        case 0:
            if (set != 0) {
                ret = 1;
            }
            break;
        case 1:
            if (clear >= 2) {
                ret = 1;
            }
            break;
        case 2:
            if (clear == 1) {
                ret = 1;
            }
            break;
        }
        break;
    case 1:
        if (gamestate_test_bit(gamestate_data.flags_10, 13, 1) && gamestate_test_bit(gamestate_data.flags_10, 14, 1)
            && gamestate_test_bit(gamestate_data.flags_10, 15, 1) && gamestate_test_bit(gamestate_data.flags_10, 16, 1)) {
            ret = 1;
        }
        break;
    }
    return ret;
}

s32 gamestate_cond_object(s32 arg0, s32 arg1) {
    Object *obj = heap_objects.find(0x16, -1, -1);

    obj->set_step(obj, 3);
    return 1;
}

s32 gamestate_check_condition(s32 id, s32 value) {
    GamestateCondition *entry;
    s32 ret = 0;
    s32 sub;
    s32 arg;

    for (entry = gamestate_conditions; entry->id != 0xFF; entry++) {
        if (entry->id == id) {
            sub = entry->type & 0xF;
            arg = entry->arg;
            switch (entry->type & 0xF0) {
            case 0x00:
                ret = gamestate_cond_items(sub, arg);
                break;
            case 0x10:
                ret = gamestate_cond_digimon(sub, arg);
                break;
            case 0x20:
                ret = gamestate_change_money(sub, arg);
                break;
            case 0x30:
                ret = gamestate_cond_progress(sub, arg);
                break;
            case 0x40:
                ret = gamestate_cond_bits(sub, arg);
                break;
            case 0x50:
                ret = gamestate_cond_object(sub, arg);
                break;
            }
            break;
        }
    }
    return value == ret;
}

s32 gamestate_check_progress(s32 value, s32 equal) {
    if (equal) {
        if (gamestate_data.progress == value) {
            return 1;
        }
    } else {
        if (gamestate_data.progress != value) {
            return 1;
        }
    }
    return 0;
}

s32 gamestate_check_item(s32 id, s32 flag) {
    if (flag) {
        if (gamestate_data.items[id] != 0 || gamestate_data.items_equipped[id] != 0) {
            return 1;
        }
    } else {
        if (gamestate_data.items[id] == 0 && gamestate_data.items_equipped[id] == 0) {
            return 1;
        }
    }
    return 0;
}

s32 gamestate_check_card(s32 id, s32 flag) {
    if (flag) {
        if (gamestate_data.cards[id] != 0) {
            return 1;
        }
    } else {
        if (gamestate_data.cards[id] == 0) {
            return 1;
        }
    }
    return 0;
}

s32 gamestate_check_party_stat(s32 i, s32 flag) {
    GamestateStats stats;
    s32 sum = 0;
    s32 n;
    s32 id;

    for (n = 0; n < 3; n++) {
        id = gamestate_data.funcs.get_party_member(n);
        if (id >= 0) {
            gamestate_data.funcs.get_stats(id, &stats);
            sum += stats.stats[5];
        }
    }
    if (flag) {
        if (sum >= gamestate_party_stat_levels[i]) {
            return 1;
        }
    } else {
        if (sum < gamestate_party_stat_levels[i]) {
            return 1;
        }
    }
    return 0;
}

/* Flag type 0x7E: indices 0..29 test route == index + 1, the others room == index - 29 (flag: equal, else not equal). */
s32 gamestate_check_route(s32 id, s32 flag) {
    if (id < 30) {
        if (flag) {
            if (gamestate_data.route == id + 1) {
                return 1;
            }
        } else {
            if (gamestate_data.route != id + 1) {
                return 1;
            }
        }
    } else {
        if (flag) {
            if (gamestate_data.room == id - 29) {
                return 1;
            }
        } else {
            if (gamestate_data.room != id - 29) {
                return 1;
            }
        }
    }
    return 0;
}

s32 gamestate_unequip_item(s32 i, s32 item) {
    GamestateRecord *rec = &gamestate_data.digimon[i].record;
    RecordsEquip *data = records_funcs.get_item(item)->data;
    s16 *items = rec->equipment;
    s32 j;

    for (j = 0; j < 6; j++) {
        if (items[j] == item) {
            if (data->slot == 7) {
                rec->equipment[2] = 0;
                rec->equipment[3] = 0;
            } else {
                items[j] = 0;
            }
            return 1;
        }
    }
    return 0;
}

void gamestate_change_item(s32 id, s32 add) {
    s32 i;

    if (add) {
        if (++gamestate_data.items[id] >= 100) {
            gamestate_data.items[id] = 99;
        }
    } else if (gamestate_data.items[id] != 0) {
        if (--gamestate_data.items[id] < 0) {
            gamestate_data.items[id] = 0;
        }
    } else if (gamestate_data.items_equipped[id] != 0) {
        /* The item is equipped: take it off a party member, else off any other Digimon. */
        for (i = 0; i < 3; i++) {
            if (gamestate_unequip_item(gamestate_data.funcs.get_party_member(i), id)) {
                goto unequipped;
            }
        }
        for (i = 0; i < 8; i++) {
            if (gamestate_data.digimon[i].joined >= 3 && gamestate_unequip_item(i, id)) {
                break;
            }
        }
    unequipped:
        gamestate_data.items_equipped[id]--;
    }
}

void gamestate_change_card(s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        gamestate_data.funcs.add_card(arg0, 1);
        return;
    }
    if (--gamestate_data.cards[arg0] < 0) {
        gamestate_data.cards[arg0] = 0;
    }
}

/* Flag types 0x76/0x78: a card game (map 0x700, CARDGAME) against opponent arg0 * 2 + arg1 + 1 (the map entry is
 * CardgameGame.opponent; two opponents per deck name). gamestate_update_map_flags sets flag 0x10 to the result. */
void gamestate_start_card_game(s32 arg0, s32 arg1) {
    LATE_CALL(func_8008B770)(0x700, arg0 * 2 + arg1 + 1, 0, 0, 0);
}

s32 gamestate_get_flag(u16 flag, u16 value) {
    u16 type = (flag >> 8) & ~1;
    u16 index = flag & 0x1FF;

    if (type == 0x00) {
        return gamestate_test_bit(gamestate_flags.map_flags, index, value);
    } else if (type == 0x02) {
        return gamestate_test_bit(gamestate_data.flags, index, value);
    } else if (type == 0x04) {
        return gamestate_test_bit(gamestate_data.flags_04, index, value);
    } else if (type == 0x06) {
        return gamestate_test_bit(gamestate_data.flags_06, index, value);
    } else if (type == 0x08) {
        return gamestate_test_bit(gamestate_data.flags_08, index, value);
    } else if (type == 0x0A) {
        return gamestate_test_bit(gamestate_data.flags_0A, index, value);
    } else if (type == 0x0C) {
        return gamestate_test_bit(gamestate_data.flags_0C, index, value);
    } else if (type == 0x0E) {
        return gamestate_test_bit(gamestate_data.flags_0E, index, value);
    } else if (type == 0x10) {
        return gamestate_test_bit(gamestate_data.flags_10, index, value);
    } else if (type == 0x18) {
        return gamestate_test_bit(gamestate_data.flags_18, index, value);
    } else if (type == 0x1A) {
        return gamestate_test_bit(gamestate_data.flags_1A, index, value);
    } else if (type == 0x1C) {
        return gamestate_test_bit(gamestate_data.flags_1C, index, value);
    } else if (type == 0x20) {
        return gamestate_test_bit(gamestate_data.flags_20, index, value);
    } else if (type == 0x40) {
        return gamestate_test_bit(gamestate_data.flags_40, index, value);
    } else if (type == 0x60) {
        return gamestate_check_progress(index, value);
    } else if (type == 0x70) {
        return gamestate_check_condition(index, value);
    } else if (type == 0x72) {
        return gamestate_check_party_stat(index, value);
    } else if (type == 0x7E) {
        return gamestate_check_route(index, value);
    } else if (type >= 0x80 && type <= 0x8E) {
        return gamestate_check_item(index, value);
    } else if (type == 0x92) {
        return gamestate_check_card(index, value);
    }
    return 1;
}

void gamestate_set_flag(s32 flag, s32 value) {
    u16 type = (flag >> 8) & ~1;
    u16 index = flag & 0x1FF;

    if (type == 0x00) {
        gamestate_set_bit(gamestate_flags.map_flags, index, value);
    }
    if (type == 0x02) {
        gamestate_set_bit(gamestate_data.flags, index, value);
    }
    if (type == 0x04) {
        gamestate_set_bit(gamestate_data.flags_04, index, value);
    }
    if (type == 0x06) {
        gamestate_set_bit(gamestate_data.flags_06, index, value);
    }
    if (type == 0x08) {
        gamestate_set_bit(gamestate_data.flags_08, index, value);
    }
    if (type == 0x0A) {
        gamestate_set_bit(gamestate_data.flags_0A, index, value);
    }
    if (type == 0x0C) {
        gamestate_set_bit(gamestate_data.flags_0C, index, value);
    }
    if (type == 0x0E) {
        gamestate_set_bit(gamestate_data.flags_0E, index, value);
    }
    if (type == 0x10) {
        gamestate_set_bit(gamestate_data.flags_10, index, value);
    }
    if (type == 0x18) {
        gamestate_set_bit(gamestate_data.flags_18, index, value);
    }
    if (type == 0x1A) {
        gamestate_set_bit(gamestate_data.flags_1A, index, value);
    }
    if (type == 0x1C) {
        gamestate_set_bit(gamestate_data.flags_1C, index, value);
    }
    if (type == 0x20) {
        gamestate_set_bit(gamestate_data.flags_20, index, value);
    }
    if (type == 0x40) {
        gamestate_set_bit(gamestate_data.flags_40, index, value);
    }
    if (type == 0x70) {
        gamestate_check_condition(index, 1);
    }
    if (type == 0x74) {
        D_8009B6A4(index);
    }
    if (type == 0x76) {
        gamestate_start_card_game(index, 0);
    }
    if (type == 0x78) {
        gamestate_start_card_game(index, 1);
    }
    if (type >= 0x80 && type <= 0x8E) {
        gamestate_change_item(index, value);
    }
    if (type == 0x90) {
        LATE_CALL(func_8008BFA4)(index);
    }
    if (type == 0x92) {
        gamestate_change_card(index, value);
    }
    if (type == 0x94) {
        LATE_CALL(func_8008B770)(0xA00, index, 0, 0, 0);
    }
    if (type == 0x7A) {
        if (index < 30) {
            LATE_CALL(func_8008B770)(0xF00, index, 0, 0, 0);
        } else if ((index >= 0x31 && index <= 0x43) || (index >= 0x46 && index <= 0x4A)) {
            LATE_CALL(func_8008B770)(0x1300, index, 0, 0, 0);
        } else {
            LATE_CALL(func_8008C000)();
        }
    }
    if (type == 0x7C) {
        if (index == 0) {
            LATE_CALL(func_8008B770)(0xD00, 0, 0, 0, 0);
        } else if (index == 1) {
            LATE_CALL(func_8008B770)(0xB00, 0, 0, 0, 0);
        }
    }
}

/* Lists are (flag, value) pairs ending with 0xFFFF. */
s32 gamestate_check_flags(u16 *list) {
    u16 flag;

    while ((flag = *list) != 0xFFFF) {
        list++;
        if (gamestate_get_flag(flag, *list++) == 0) {
            return 0;
        }
    }
    return 1;
}

void gamestate_set_flags(u16 *list) {
    u16 flag;

    while ((flag = *list) != 0xFFFF) {
        list++;
        gamestate_set_flag(flag, *list++);
    }
}

void gamestate_update_map_flags(void) {
    s32 i;

    if (gamestate_data.map_is_new != 0) {
        for (i = 0; i < 3; i++) {
            gamestate_flags.map_flags[i] = 0;
        }
        gamestate_set_flag(0x12, 0);
    }
    if (gamestate_data.funcs.get_prev_map() == 0x700) {
        gamestate_set_flag(0x11, 1);
        gamestate_set_flag(0x12, 1);
        if (gamestate_flags.card_game_won) {
            gamestate_set_flag(0x10, 1);
        } else {
            gamestate_set_flag(0x10, 0);
        }
        gamestate_flags.card_game_won = 0;
    }
}

void gamestate_new_game(void) {
    heap_funcs.bzero(&gamestate_data, 0x26C4);
    gamestate_data.map = 0x1600;
    gamestate_data.next_map = 0x1600;
    gamestate_data.digivolve_demo = 1;
    gamestate_data.countdown[0] = 1;
    gamestate_data.countdown[1] = 8;
    gamestate_data.countdown[3] = 60;
    gamestate_data.map_entry = 0;
    gamestate_data.countdown[2] = 0;
    gamestate_data.unk_000C = -1;
    gamestate_init_records();
    gamestate_data.encounter_timer = (pad_random.next() & 0x1FF) + 0x200;
}

void gamestate_change_map(void) {
    if (gamestate_data.next_map != 0) {
        gamestate_data.prev_map = gamestate_data.map;
        gamestate_data.map = gamestate_data.next_map;
        gamestate_data.next_map = 0;
    }
}

s32 gamestate_get_prev_map(void) {
    return gamestate_data.prev_map;
}

s32 gamestate_get_map(void) {
    return gamestate_data.map;
}

s32 gamestate_get_map_entry(void) {
    return gamestate_data.map_entry;
}

void gamestate_set_next_map(s32 arg0, s32 arg1) {
    gamestate_data.next_map = arg0;
    gamestate_data.map_entry = arg1;
}

s32 gamestate_is_map_changing(void) {
    return gamestate_data.next_map != 0;
}

void gamestate_init_records(void) {
    Font funcs;
    RecordsDigimon *src;
    s32 i;
    s32 j;
    u16 *dst;
    u16 *s;

    font_init(&funcs);
    strcpy(gamestate_data.name, funcs.get_entry(cdload_module.files.get_file(records_language + 0x86), 11));
    gamestate_data.party[0] = -1;
    gamestate_data.party[1] = -1;
    gamestate_data.party[2] = -1;
    for (j = 0; j < 3; j++) {
        strcpy(gamestate_data.decks[j].name, funcs.get_entry(cdload_module.files.get_file(records_language + 0x32), j + 0x16));
    }
    gamestate_data.funcs.init_cards();
    for (i = 0; i < 8; i++) {
        src = &records_digimon[i];
        strcpy(gamestate_data.digimon[i].record.name,
               funcs.get_entry(cdload_module.files.get_file(records_language + 0x4E), src->name_id));
        gamestate_data.digimon[i].record.stats.values[0] = 1;
        gamestate_data.digimon[i].record.stats.values[2] = gamestate_data.digimon[i].record.stats.values[3] = src->start_hp;
        gamestate_data.digimon[i].record.stats.values[4] = gamestate_data.digimon[i].record.stats.values[5] = src->start_mp;
        dst = gamestate_data.digimon[i].record.stats.stats;
        s = src->stats;
        for (j = 0; j < 6; j++) {
            *dst++ = *s++;
        }
        s = src->resists;
        for (j = 0; j < 7; j++) {
            *dst++ = *s++;
        }
    }
}

s32 gamestate_get_party_member(u32 i) {
    return (i < 3) ? gamestate_data.party[i] : -1;
}

void gamestate_set_party(s32 arg0) {
    s32 i;
    s32 id;

    for (i = 0; i < 3; i++) {
        id = gamestate_parties[arg0][i];
        gamestate_data.party[i] = id;
        gamestate_data.digimon[id].joined = id + 3;
    }
    gamestate_data.party_set = arg0;
}

void gamestate_add_card(s32 arg0, s32 arg1) {
    gamestate_data.cards_obtained[arg0] = 1;
    if ((gamestate_data.cards[arg0] += arg1) >= 10) {
        gamestate_data.cards[arg0] = 9;
    }
}

void gamestate_init_cards(void) {
    s32 i;
    s32 j;

    for (i = 0; i < 40; i++) {
        gamestate_add_card(gamestate_start_deck[i], 1);
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 40; i++) {
            gamestate_data.decks[j].cards[i] = gamestate_start_deck[i];
        }
    }
}

void gamestate_reset_playtime(void) {
    gamestate_data.playtime[3] = 0;
    gamestate_data.playtime[2] = 0;
    gamestate_data.playtime[1] = 0;
    gamestate_data.playtime[0] = 0;
    gamestate_data.playtime_frames = 0;
}

void gamestate_tick_playtime(void) {
    if ((gamestate_data.playtime_frames >> 8) >= 60) {
        gamestate_data.playtime_frames &= 0xFF;
        if (++gamestate_data.playtime[2] >= 60) {
            gamestate_data.playtime[2] = 0;
            if (++gamestate_data.playtime[1] >= 60) {
                gamestate_data.playtime[1] = 0;
                if (++gamestate_data.playtime[0] >= 1000) {
                    gamestate_data.playtime[0] = 999;
                    gamestate_data.playtime[1] = 59;
                    gamestate_data.playtime[2] = 59;
                    gamestate_data.playtime[3] = 1;
                }
            }
        }
    }
}

s32 gamestate_get_party_digimon(u32 i) {
    return (i < 3) ? gamestate_data.digimon[gamestate_data.party[i]].joined - 3 : -1;
}

void gamestate_set_stat(s32 i, u32 stat, s16 value) {
    s16 *p = gamestate_data.digimon[i].record.stats.values;

    if (stat < 19) {
        p += stat;
        *p = value;
        if (value < 0) {
            *p = 0;
        } else if (stat < 2) {
            if (value >= 100) {
                *p = 99;
            }
        } else if (stat >= 2 && stat <= 5) {
            if (value >= 10000) {
                *p = 9999;
            }
        } else {
            if (value >= 1000) {
                *p = 999;
            }
        }
    }
}

void gamestate_add_stat(s32 i, u32 stat, s32 value) {
    s16 *p = gamestate_data.digimon[i].record.stats.values;

    if (stat < 19) {
        p += stat;
        *p += value;
        if (*p < 0) {
            *p = 0;
        } else if (stat < 2) {
            if (*p >= 100) {
                *p = 99;
            }
        } else if (stat >= 2 && stat <= 5) {
            if (*p >= 10000) {
                *p = 9999;
            }
        } else {
            if (*p >= 1000) {
                *p = 999;
            }
        }
    }
}

/* Copies party member i's stats into out, adds its items' bonuses (and a bonus for one item set), and
 * takes off penalties. Reaching the stat arrays as *(array + k) keeps k as the first addend (the original's). */
void gamestate_get_stats(s32 i, GamestateStats *out) {
    s16 *items;
    RecordsItem *entry;
    void *data;
    u8 type;
    s16 stat;
    s16 value;
    s32 j;
    s32 k;

    *out = gamestate_data.digimon[i].record.stats;
    items = gamestate_data.digimon[i].record.equipment;
    for (j = 0; j < 6; j++) {
        if (items[j] > 0) {
            entry = records_funcs.get_item(items[j]);
            type = entry->type;
            data = entry->data;
            if (type >= 2 && type <= 14) {
                out->stats[0] += ((RecordsWeapon *)data)->power;
                if (out->stats[0] >= 1000) {
                    out->stats[0] = 999;
                }
                for (k = 0; k < 2; k++) {
                    stat = *(((RecordsWeapon *)data)->bonus_stats + k);
                    value = ((RecordsWeapon *)data)->bonus_values[k];
                    if (stat != 0) {
                        gamestate_add_stat_bonus(out->values, stat, value);
                    }
                }
            } else if (type >= 15 && type <= 20) {
                out->stats[1] += ((RecordsArmor *)data)->guard;
                if (out->stats[1] >= 1000) {
                    out->stats[1] = 999;
                }
                for (k = 0; k < 2; k++) {
                    stat = *(((RecordsArmor *)data)->bonus_stats + k);
                    value = ((RecordsArmor *)data)->bonus_values[k];
                    if (stat != 0) {
                        gamestate_add_stat_bonus(out->values, stat, value);
                    }
                }
            } else if (type >= 21 && type <= 24) {
                stat = ((RecordsAccessory *)data)->bonus_stat;
                value = ((RecordsAccessory *)data)->bonus_value;
                if (stat != 0) {
                    gamestate_add_stat_bonus(out->values, stat, value);
                }
            } else {
                continue;
            }
            out->stats[5] += ((RecordsEquip *)data)->charisma;
            if (out->stats[5] >= 1000) {
                out->stats[5] = 999;
            }
        }
    }
    out->stats[0] -= out->penalties[0];
    if (out->stats[0] < 0) {
        out->stats[0] = 0;
    }
    out->stats[1] -= out->penalties[1];
    if (out->stats[1] < 0) {
        out->stats[1] = 0;
    }
    out->stats[4] -= out->penalties[2];
    if (out->stats[4] < 0) {
        out->stats[4] = 0;
    }
    if (items[0] == gamestate_item_sets[i][0] && items[1] == gamestate_item_sets[i][1] && items[2] == gamestate_item_sets[i][2]
        && items[3] == gamestate_item_sets[i][3]) {
        for (j = 0; j < 6; j++) {
            *(out->values + 6 + j) += gamestate_item_set_bonuses[i][j];
        }
    }
}

void gamestate_add_stat_bonus(s16 *stats, s32 type, s32 value) {
    s32 i;

    if (type == 7) {
        for (i = 0; i < 6; i++) {
            stats[i + 6] += value;
            if (stats[i + 6] >= 1000) {
                stats[i + 6] = 999;
            }
        }
    } else if (type >= 1 && type <= 6) {
        stats[type + 5] += value;
        if (stats[type + 5] >= 1000) {
            stats[type + 5] = 999;
        }
    } else if (type >= 8 && type <= 14) {
        stats[type + 4] += value;
        if (stats[type + 4] >= 1000) {
            stats[type + 4] = 999;
        }
    }
}

s32 gamestate_find_form(s32 i, s32 id) {
    s32 j;

    for (j = 0; j < 44; j++) {
        if (gamestate_data.digimon[i].record.forms[j].id >= 3
            && gamestate_data.digimon[i].record.forms[j].id == id) {
            return j;
        }
    }
    return -1;
}

s32 gamestate_get_chosen_forms(s32 i, s16 *out) {
    s32 count;
    s32 j;
    s32 k;

    for (j = 0, count = 0; j < 3; j++) {
        if (gamestate_data.digimon[i].record.chosen_forms[j] >= 3) {
            k = gamestate_find_form(i, gamestate_data.digimon[i].record.chosen_forms[j]);
            if (k >= 0 && gamestate_data.digimon[i].record.forms[k].id >= 3) {
                out[count++] = gamestate_data.digimon[i].record.forms[k].id;
            }
        }
    }
    for (j = count; j < 3; j++) {
        out[j] = -1;
    }
    return count;
}

void gamestate_set_chosen_forms(s32 i, s16 *ids) {
    s32 j;
    s32 k;

    for (j = 0; j < 3; j++) {
        k = gamestate_find_form(i, ids[j]);
        if (k >= 0) {
            gamestate_data.digimon[i].record.chosen_forms[j] = gamestate_data.digimon[i].record.forms[k].id;
        } else {
            gamestate_data.digimon[i].record.chosen_forms[j] = -1;
        }
    }
}

s32 gamestate_list_forms(s32 i, s16 *out) {
    s32 j;
    s32 count;
    s32 found;

    for (j = 0, count = 0; j < 44; j++) {
        if (gamestate_data.digimon[i].record.forms[j].id >= 3) {
            out[count++] = gamestate_data.digimon[i].record.forms[j].id;
        }
    }
    found = count;
    for (; count < 44; count++) {
        out[count] = 0;
    }
    return found;
}

s32 gamestate_add_form(s32 i, s32 id) {
    s32 j;
    s32 slot;

    if (gamestate_find_form(i, id) != -1) {
        return 0;
    }
    for (j = 0, slot = -1; j < 44; j++) {
        if (gamestate_data.digimon[i].record.forms[j].id == 0) {
            slot = j;
            break;
        }
    }
    if (slot == -1) {
        return 0;
    }
    records_get_digimon_func(id);
    gamestate_data.digimon[i].record.forms[slot].id = id;
    return gamestate_data.digimon[i].record.forms[slot].level = 1;
}

s32 gamestate_get_form(s32 i, s32 id, GamestateForm *out) {
    s32 j = gamestate_find_form(i, id);

    if (j != -1) {
        *out = gamestate_data.digimon[i].record.forms[j];
    }
    return j;
}

s32 gamestate_put_form(s32 i, s32 id, GamestateForm *in) {
    s32 j = gamestate_find_form(i, id);

    if (j != -1) {
        gamestate_data.digimon[i].record.forms[j] = *in;
    }
    return j;
}

GamestateRecord *gamestate_get_record(s32 i) {
    return &gamestate_data.digimon[i].record;
}

/* Only its function table (funcs) has an initializer. gamestate_get_party_member and gamestate_get_party_digimon take a u32
 * (their code compares it unsigned); the table's callers pass an s32. */
GamestateData gamestate_data = {
    .funcs = {
        gamestate_new_game,
        gamestate_change_map,
        gamestate_get_map,
        gamestate_get_map_entry,
        gamestate_set_next_map,
        gamestate_is_map_changing,
        gamestate_get_prev_map,
        (s32 (*)(s32))gamestate_get_party_member,
        gamestate_set_party,
        gamestate_add_card,
        gamestate_init_cards,
        (s32 (*)())gamestate_get_party_digimon,
        (void (*)())gamestate_set_stat, /* takes an s16 */
        gamestate_add_stat,
        gamestate_get_stats,
        gamestate_get_chosen_forms,
        gamestate_set_chosen_forms,
        gamestate_list_forms,
        gamestate_add_form,
        gamestate_get_form,
        gamestate_put_form,
        gamestate_get_record,
        gamestate_reset_playtime,
        gamestate_tick_playtime,
    },
};

u8 gamestate_parties[][3] = { { 0, 6, 7 }, { 2, 3, 6 }, { 1, 5, 7 } };
s32 gamestate_start_deck[40] = {
    24, 24, 24, 45, 45, 50, 59, 60, 95, 99,
    100, 100, 102, 122, 138, 141, 141, 143, 145, 181,
    184, 185, 185, 188, 221, 224, 228, 230, 230, 230,
    267, 268, 268, 270, 274, 303, 313, 314, 314, 314,
};
s16 gamestate_item_sets[][4] = {
    { 243, 269, 98, 284 }, { 242, 258, 112, 285 }, { 222, 256, 124, 282 }, { 244, 271, 163, 286 },
    { 211, 268, 203, 288 }, { 245, 270, 149, 287 }, { 221, 257, 137, 283 }, { 232, 255, 136, 281 },
};
u16 gamestate_item_set_bonuses[][6] = {
    { 4, 4, 4, 4, 4, 20 }, { 10, 10, 0, 0, 0, 20 },
    { 10, 0, 0, 0, 10, 20 }, { 0, 10, 10, 0, 0, 20 },
    { 10, 0, 0, 10, 0, 20 }, { 0, 0, 0, 10, 10, 20 },
    { 10, 0, 10, 0, 0, 20 }, { 0, 10, 0, 10, 0, 20 },
};
