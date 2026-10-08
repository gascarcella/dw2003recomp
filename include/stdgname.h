#ifndef STDGNAME_H
#define STDGNAME_H

/* STDGNAME.PRO (renaming a party Digimon): types, module structs and functions shared by its files. */

#include "common.h"
#include "object.h"
#include "message.h"
#include "gamestate.h"
#include "overlay_common.h"

/* stdgname_funcs: the overlay's font, the party member being renamed, and the functions of the
 * second file (stdgname_80086184.c), which the first calls through. */
typedef struct StdgnameFuncs {
    /* 0x00 */ MessageFont *font; /* stdgname_name_font_2 */
    /* 0x04 */ s32 member;        /* party member (0..2) */
    /* 0x08 */ void (*load)(void);
    /* 0x0C */ s32 (*is_loading)(void);
    /* 0x10 */ void (*anim_start)(WindowAnim *anim, s32 open);
    /* 0x14 */ s32 (*anim_update)(WindowAnim *anim);
} StdgnameFuncs; /* size 0x18 */

extern StdgnameFuncs stdgname_funcs;

/* The overlay's main object (stdgname_main_create, size 0x6C). */
typedef struct StdgnameMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 layer_id; /* layer */
    /* 0x54 */ s32 unk_54;
    /* 0x58 */ s32 scroll; /* background scroll */
    /* 0x5C */ s32 odd_frame; /* the scroll moves every other frame */
    /* 0x60 */ s32 member; /* party member chosen, -1: cancelled */
    /* 0x64 */ s32 unk_64;
    /* 0x68 */ void (*fade_out)(struct StdgnameMain *obj); /* stdgname_main_fade_out: fade out */
} StdgnameMain; /* size 0x6C */

/* The name entry (stdgname_entry_create, size 0xF8): the name being typed (two-byte characters) and a
 * keyboard of 15 x 7 keys. */
typedef struct StdgnameNameEntry {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 mode;
    /* 0x54 */ s32 layer_id; /* layer */
    /* 0x58 */ s32 ot_depth; /* ordering table depth */
    /* 0x5C */ s32 tex_x;  /* texture position x */
    /* 0x60 */ s32 tex_y;  /* texture position y */
    /* 0x64 */ s32 digimon; /* the Digimon's index */
    /* 0x68 */ s32 anim_frame; /* animation frame */
    /* 0x6C */ s32 frame_time; /* time of the last frame */
    /* 0x70 */ s32 glow_frame;
    /* 0x74 */ s32 glow_time;
    /* 0x78 */ u16 name[12];   /* the name */
    /* 0x90 */ s32 name_cursor; /* cursor in the name */
    /* 0x94 */ s32 length; /* name length (5, or 8 outside Japan) */
    /* 0x98 */ s32 unk_98;
    /* 0x9C */ s32 key_column; /* keyboard column */
    /* 0xA0 */ s32 key_row; /* keyboard row */
    /* 0xA4 */ s32 cursor_frame;
    /* 0xA8 */ s32 cursor_time;
    /* 0xAC */ s32 input_enabled; /* input enabled */
    /* 0xB0 */ s32 key_page; /* keyboard page */
    /* 0xB4 */ s32 arrows_frame;
    /* 0xB8 */ s32 arrows_time;
    /* 0xBC */ s32 unk_BC;
    /* 0xC0 */ WindowAnim unused_anim; /* only its duration is set (10): nothing starts it */
    /* 0xD0 */ WindowAnim keyboard_anim;
    /* 0xE0 */ WindowAnim warning_anim;
    /* 0xF0 */ void (*get_name)(struct StdgnameNameEntry *obj, u8 *dst); /* stdgname_entry_get_name */
    /* 0xF4 */ void (*close)(struct StdgnameNameEntry *obj);          /* stdgname_entry_close */
} StdgnameNameEntry; /* size 0xF8 */

/* The main object's data block (0x10 bytes). */
typedef struct StdgnameMainData {
    /* 0x0 */ Object *party;         /* the party member choice (stdgname_party_create) */
    /* 0x4 */ StdgnameNameEntry *name_entry; /* the name entry */
    /* 0x8 */ Fade *fade;        /* the fade */
    /* 0xC */ Object *waited; /* the done state waits for it, then runs again; nothing here fills it */
} StdgnameMainData; /* size 0x10 */

/* stdgname_80082F8C.c */
Fade *stdgname_fade_create(void);
StdgnameNameEntry *stdgname_entry_create(GamestateRecord *digimon, s32 index);
Object *stdgname_party_create(StdgnameMain *main);

/* stdgname_80086184.c */
StdgnameMain *stdgname_main_create(void);

#endif /* STDGNAME_H */
