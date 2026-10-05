#ifndef WFIGHTMN_H
#define WFIGHTMN_H

/* The tier-2 battle overlays' functions that FIGHTSTG calls. FIGHTSTG loads WFIGHTMN (file 0x208, a normal battle)
 * or WFIGHTTS (file 0x209) at 0x800A5DE0 (TIER2_BASE in configure.py) and calls into the one loaded. FIGHTSTG
 * doesn't link against them (they link against it): config/fightstg.symbols.txt gives these addresses the
 * tier-2 overlays' names (absolute:True; configure.py defines them for FIGHTSTG's link), so both sides use the
 * same names. Parameter types are the ones FIGHTSTG's code matches with. */

#include "common.h"
#include "object.h"

/* WFIGHTMN (config/wfightmn.symbols.txt; src/wfightmn/wfightmn_800A6440.c). */
void wfightmn_check_regen(s32 member);
Object *wfightmn_main_create(void); /* the entry: creates the battle's main object */
void wfightmn_add_gauge(u8 side, s32 value);
void *wfightmn_tech_script_create(u8 side, s32 arg1);
s32 wfightmn_update_idle_anim(u8 side, s32 arg1); /* 1: the member is below a quarter of its HP */
void wfightmn_count_boss_hits(u8 side, s32 arg1);
void wfightmn_check_final_phase_end(u8 side);
#ifndef WFIGHTMN_CAP_DAMAGE_DEFINED
/* FIGHTSTG's callers pass the count sign-extended (an s16 parameter); WFIGHTMN's definition takes an s32 and uses
 * the register as passed (no re-extension), so its file defines WFIGHTMN_CAP_DAMAGE_DEFINED and skips this. */
s32 wfightmn_cap_damage(u8 side, s32 value, s16 count);
#endif

/* WFIGHTTS (config/wfightts.symbols.txt; src/wfightts/wfightts_800A67B8.c). */
Object *wfightts_main_create(void); /* the entry */

#endif /* WFIGHTMN_H */
