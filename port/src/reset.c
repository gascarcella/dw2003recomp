/* The console's reset (port_harness.h). Session 16 stub: T7 ("reset") owns this file. */
#include "port_harness.h"
#include "port_runtime.h"

jmp_buf port_reset_jmp;

void port_reset_request(void) {
    longjmp(port_reset_jmp, 1);
}

void port_reset_state(void) {
    port_fatal("reset: not implemented yet");
}
