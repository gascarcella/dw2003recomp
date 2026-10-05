#ifndef PSYQ_LIBC2_H
#define PSYQ_LIBC2_H

/* Our own declarations of the Psy-Q 4.7 LIBC2 functions the game uses (string.h/memory.h). The
 * game's memcpy calls are GCC's built-in (its argument evaluation order, overlay), which needs
 * a prototype compatible with the built-in: an unsigned length. */

#include "common.h"

s32 strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, s32 n);
void *memcpy(void *dst, const void *src, u32 n);
s32 strcspn(const char *s, const char *reject);
s32 atoi(const char *s);

#endif /* PSYQ_LIBC2_H */
