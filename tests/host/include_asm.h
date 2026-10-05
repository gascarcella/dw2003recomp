/* Host build: the matching build's INCLUDE_ASM pulls split assembly in; here a non-matching function is simply absent
 * (the goldens only call functions that compile from C). */
#define INCLUDE_ASM(path, name)
#define INCLUDE_RODATA(path, name)
