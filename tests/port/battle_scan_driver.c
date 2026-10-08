/* tests/port/battle.py's driver for port/game/battle_scan.c: reads records { s32 words, s32 stage, s16 words[] } from
 * argv[1] and prints, per record, "ok length child multi sound sound_arg". */
#include <stdio.h>
#include <stdlib.h>

#include "battle_scan.h"

int main(int argc, char **argv) {
    FILE *f;
    int32_t head[2];
    if (argc != 2 || (f = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "usage: battle_scan_driver RECORDS\n");
        return 2;
    }
    while (fread(head, sizeof(head), 1, f) == 1) {
        int16_t *w = malloc(sizeof(int16_t) * (size_t)(head[0] > 0 ? head[0] : 1));
        PortBattleScan r;
        if (w == NULL || fread(w, sizeof(int16_t), (size_t)head[0], f) != (size_t)head[0]) {
            fprintf(stderr, "battle_scan_driver: short record\n");
            return 1;
        }
        port_battle_scan(w, head[0], head[1], &r);
        printf("%d %d %d %d %d %d\n", r.ok, r.length, r.child, r.multi, r.sound, r.sound_arg);
        free(w);
    }
    fclose(f);
    return 0;
}
