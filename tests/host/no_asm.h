/* tests/host/build.sh compiles src/main/heap.c with this header and -D__asm__= : the MIPS inline asm of
 * include/port.h's PORT_SCRATCHPAD_STACK_ENTER/LEAVE (`__asm__ volatile(...)`, the scratchpad stack of heap_run_object,
 * which the goldens never run; the harness builds without PC_PORT, so port.h keeps the PS1 form) becomes an empty
 * statement on the host. `volatile` as a qualifier (not followed by a parenthesis) is untouched. */
#define volatile(...)
