/* psxstack/types.h: the PS1-style integer names (s8..u64, f32, f64) for the stack's own C, with the same definitions
 * the game's common.h uses, so that either header may come first: both share the PSXSTACK_TYPES_H guard (the game's
 * common.h wraps its typedefs in it). The stack's public headers (hooks.h, game.h) use <stdint.h> types; the runtime's
 * C uses these names, as the shim does. */
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

#endif /* PSXSTACK_TYPES_H */
