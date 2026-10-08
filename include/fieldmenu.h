#ifndef FIELDMENU_H
#define FIELDMENU_H

#include "common.h"

/* records_field_menu_choice: what fieldmenu_create's menu (fieldmenu_update) chose; STSTATUS opens that page
 * (ststatus_pages[extended][option]). STSTATUS reaches both words from one %hi, so it is one variable.
 * Defined in records.c (.sbss; this file stores to it without $gp, so it isn't defined here). */
typedef struct FieldmenuChoice {
    /* 0x0 */ s32 option; /* the option chosen (STSTATUS: the page) */
    /* 0x4 */ s32 extended; /* the sixth option is there: gamestate_data.items[0x192] was set (STSTATUS: the row) */
} FieldmenuChoice; /* size 0x8 */

extern FieldmenuChoice records_field_menu_choice;

struct Fieldmenu;

/* Creates the menu (fieldmenu_update) on `layer` with the cursor on `option`; returns it. */
struct Fieldmenu *fieldmenu_create(s32 layer, s32 option);

#endif /* FIELDMENU_H */
