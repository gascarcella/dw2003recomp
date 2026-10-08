/* The battle scripts' command streams scanned without running them (battle_scan.c; the battle_animations mod,
 * docs/LAUNCHER.md "Disable battle animations"). Dependency-free: tests/port/battle.py compiles it alone and checks it against an
 * independent reading of every script on the disc. */
#ifndef PORT_BATTLE_SCAN_H
#define PORT_BATTLE_SCAN_H

#include <stdint.h>

typedef struct PortBattleScan {
    int ok;        /* the stream reaches its end (command 0 or 0xFF) within the words given */
    int length;    /* words up to and including the end command */
    int child;     /* it has a child command (1 with op 0, the target's reaction, or op 5, a multi-hit's next) */
    int multi;     /* it has a child command with op 5 */
    int sound;     /* the first hit-sound command (10 with id 0x62: per results[0]; 0x63: per results[3]); 0: none */
    int sound_arg; /* that command's argument */
} PortBattleScan;

/* Scans `stream` (at most `max_words` s16 words) as fightstg_script_update would run it. `stage` is the script's
 * FightstgScript.stage: command 4 reads no fade times when its stage (0x38: this one) is -1. */
void port_battle_scan(const int16_t *stream, int max_words, int32_t stage, PortBattleScan *out);

#endif /* PORT_BATTLE_SCAN_H */
