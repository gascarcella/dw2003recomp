/* Host build of src/fightstg/fightstg_80086A00.c: fightstg_enemy_check_condition is called before its definition (an
 * implicit int declaration, which the original's GCC 2.8.1 accepts and a modern gcc rejects as a conflicting type: its
 * u8/s16 parameters do not survive the default promotions). Declared here, forced in with -include, so the unit compiles
 * unchanged (as tests/host/fieldstg_protos.h does for FIELDSTG). */
#include "common.h"
s32 fightstg_enemy_check_condition(u8 type, s16 value);
