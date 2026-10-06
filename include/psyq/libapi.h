#ifndef PSYQ_LIBAPI_H
#define PSYQ_LIBAPI_H

/* Our own declarations of the Psy-Q 4.7 LIBAPI functions the game uses (BIOS file I/O), added as the game
 * needs them (no Sony headers are used; DECISIONS "Code only: no game data, SDK or BIOS in the repo"). */

#include "common.h"

s32 open(char *name, s32 mode);
s32 read(s32 fd, void *buf, s32 n);
s32 write(s32 fd, void *buf, s32 n);
s32 close(s32 fd);

#endif /* PSYQ_LIBAPI_H */
