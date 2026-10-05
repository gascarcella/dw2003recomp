/* tests/host/build.sh compiles src/main/heap.c with this header and -D__asm__= : its MIPS inline asm
 * (`__asm__ volatile(...)`, the scratchpad stack of heap_run_object, which the goldens never run) becomes an empty
 * statement on the host. `volatile` as a qualifier (not followed by a parenthesis) is untouched. */
#define volatile(...)
