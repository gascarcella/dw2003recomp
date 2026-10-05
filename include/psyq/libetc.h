#ifndef PSYQ_LIBETC_H
#define PSYQ_LIBETC_H

/* Our own declarations of the Psy-Q 4.7 LIBETC interface, added as the game needs them. */

#include "common.h"

#define MODE_NTSC 0
#define MODE_PAL 1

int VSyncCallback(void (*f)(void));
int VSync(int mode);
int ResetCallback(void);
int SetVideoMode(int mode);

#endif /* PSYQ_LIBETC_H */
