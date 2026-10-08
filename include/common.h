#ifndef COMMON_H
#define COMMON_H

/* The PC port's runtime defines the same names (port/include/psxstack/types.h) under this guard, so that either header
 * may come first in a host translation unit; the PS1 build never sees that header. */
#ifndef PSXSTACK_TYPES_H
#define PSXSTACK_TYPES_H
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#endif

#ifndef NULL
#define NULL 0
#endif

#include "include_asm.h"

/* A block the original wrapped in a loop construct (very likely a macro expanding to do/while(0)): its loop notes
 * are scheduling barriers and move exits out of line. Use only with a comment naming that evidence
 * (DECISIONS "LOOP_BLOCK: loop-scoped blocks as a named macro"). */
#define LOOP_BLOCK(body...) do { body } while (0)
#define LOOP_BARRIER() do { } while (0)

/* The byte offset of `field` in `type`, as a constant (the classic null-pointer form; the host uses the builtin).
 * For a partial clear or copy of a struct whose prefix holds pointers (its size differs on the PC port). */
#ifdef PC_PORT
#define OFFSETOF(type, field) __builtin_offsetof(type, field)
#else
#define OFFSETOF(type, field) ((u32) & ((type *)0)->field)
#endif

/* The PC port's hook macros (PLATFORM_WAIT, PTR_ADD, SLOT_FUNC, ...): each is the plain PS1 code unless PC_PORT is
 * defined. */
#include "port.h"

#endif /* COMMON_H */
