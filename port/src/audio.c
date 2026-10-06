/* The audio output (port_harness.h). M3 step-0 stub: T-audio owns this file and replaces it. */
#include "port_harness.h"
#include "port_runtime.h"

void port_audio_open(int device, const char *wav_path) {
    (void)device;
    if (wav_path != NULL) {
        port_fatal("--wav %s: not implemented yet", wav_path);
    }
}

void port_audio_frame(void) {
}

void port_audio_close(void) {
}
