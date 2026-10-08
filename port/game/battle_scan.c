/* battle_scan.h: the command lengths of src/fightstg/fightstg_8008B630.c's readers (fightstg_script_update and the
 * fightstg_script_run_* functions), word by word. A command is one s16; its arguments follow:
 *   1 child    op                                 (op 0: the reaction results[3] + 1; op 5: the next of results[0..2])
 *   2 model    op, role, [arg unless op is 1, 2, 3, 4, 7, 8, 9]; then op 3: 4 (x, y, z, frames), op 4: 4 (rotation and
 *              one skipped), op 5 with arg 4: 1, op 7 and 8: 1 (frames)
 *   3 effect   op, id; op other than 1: 3 (position)
 *   4 stage    op, id; nothing more when the stage (id 0x38: the script's own) is -1, else op other than 1: 2 (fades)
 *   5 camera   frames, op; op 1: the setting, 15 words from op on; else 1 (preset)
 *   6 prop     op, id; op other than 1: 6 (position and rotation)
 *   7 flash    op, arg          10 sound    id, arg          11 wait     frames
 *   8, 9 and any other value: no arguments          0, 0xFF: the end
 * The waiting commands rewind and run again: the same words. */
#include "battle_scan.h"

/* The next word, or the end of the scan (ok 0) when the stream ran out. */
#define NEXT() (i < max_words ? s[i++] : (out->ok = 0, i = max_words + 1, 0))
#define SKIP(n) (i += (n))

void port_battle_scan(const int16_t *s, int max_words, int32_t stage, PortBattleScan *out) {
    int i = 0;
    out->ok = out->length = out->child = out->multi = out->sound = out->sound_arg = 0;
    while (i < max_words) {
        int cmd = NEXT();
        int op, id, arg;
        if (cmd == 0 || cmd == 0xFF) {
            out->ok = 1;
            out->length = i;
            return;
        }
        switch (cmd) {
        case 1:
            op = NEXT();
            if (op == 0 || op == 5) {
                out->child = 1;
                out->multi |= op == 5;
            }
            break;
        case 2:
            op = NEXT();
            SKIP(1); /* role */
            if (op == 3 || op == 4) {
                SKIP(4);
            } else if (op == 7 || op == 8) {
                SKIP(1); /* frames */
            } else if (op != 1 && op != 2 && op != 9) {
                arg = NEXT();
                if (op == 5 && arg == 4) {
                    SKIP(1); /* arg2 */
                }
            }
            break;
        case 3:
        case 6:
            op = NEXT();
            SKIP(1); /* id */
            if (op != 1) {
                SKIP(cmd == 3 ? 3 : 6); /* the position (and a prop's rotation) */
            }
            break;
        case 4:
            op = NEXT();
            id = NEXT();
            if ((id == 0x38 ? stage : id) != -1 && op != 1) {
                SKIP(2); /* the fades */
            }
            break;
        case 5:
            SKIP(1); /* frames */
            op = NEXT();
            SKIP(op == 1 ? 14 : 1); /* the setting's 14 words, or the preset */
            break;
        case 7:
            SKIP(2);
            break;
        case 10:
            id = NEXT();
            arg = NEXT();
            if ((id == 0x62 || id == 0x63) && out->sound == 0) {
                out->sound = id;
                out->sound_arg = arg;
            }
            break;
        case 11:
            SKIP(1);
            break;
        default:
            break;
        }
    }
    out->ok = 0; /* ran past the words given */
    out->length = 0;
}
