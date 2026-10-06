/* The per-frame log and the run's record (port_harness.h). Step-0 stub of session 16: T2 ("log") owns this file
 * and replaces it. */
#include "port_harness.h"
#include "port_runtime.h"

void port_framelog_open(const char *log_path, const char *record_path) {
    if (log_path != NULL || record_path != NULL) {
        port_fatal("framelog: --log/--record: not implemented yet");
    }
}

void port_framelog_frame(void) {
}

void port_framelog_checkpoint(const char *name) {
    (void)name;
}

void port_framelog_close(int status, const char *reason) {
    (void)status;
    (void)reason;
}
