/* LIBCD's sector source over the user's BIN/CUE (port_harness.h). Step-0 stub of session 16: T1 ("cd") owns this
 * file and replaces it. */
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"

void port_disc_open(const char *path, int check_sha1) {
    (void)check_sha1;
    port_fatal("disc: --disc %s: not implemented yet", path);
}

int port_disc_set_speed(const char *speed) {
    return strcmp(speed, "instant") == 0 || strcmp(speed, "realistic") == 0;
}
