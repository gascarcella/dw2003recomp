/* The per-frame log and the run's record (port_harness.h), the port's side of tests/replay/run.lua's vsync listener.
 *
 * Every frame (port_framelog_frame, once per vsync tick) samples overlay_module (stage, file) and gamestate_data.map
 * and keeps the overlay and map sequences as run.lua does (an entry on every change; frame 1 always records), takes
 * the frame's primitive-stream hash (psyq_gpu_take_hash) and writes one log line. Events get their own lines: a file
 * copied into a slot, a checkpoint (gamestate_data's PS1 image hashed as the emulator's dump is), an input change, the
 * exit. At exit the record (JSON, the keys of tests/replay's records) gets the checkpoints and the sequences.
 *
 * The log is a function of the game's state only (no time, no host address), so two runs, and the -m32 and -m64
 * builds, write the same bytes. port/README.md documents its lines. */
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

typedef struct PortOverlayStep {
    long frame;
    s32 stage, file;
} PortOverlayStep;
typedef struct PortMapStep {
    long frame;
    u32 map;
} PortMapStep;
typedef struct PortCheckpoint {
    char *name;
    long frame;
    s32 stage;
    u32 map;
    s32 random_index;
    char sha1[41], stable[41];
} PortCheckpoint;
typedef struct PortInput {
    long frame;
    u16 buttons;
} PortInput;

static FILE *port_log_file;
static char *port_record_path;
static PortOverlayStep *port_overlay_seq;
static PortMapStep *port_map_seq;
static PortCheckpoint *port_checkpoints;
static PortInput *port_inputs;
static size_t port_overlay_n, port_map_n, port_checkpoint_n, port_input_n;
static size_t port_overlay_cap, port_map_cap, port_checkpoint_cap, port_input_cap;
static int port_sampled, port_inputs_used;
static s32 port_last_stage, port_last_file;
static u32 port_last_map;
static u16 port_last_buttons;

/* The PS1 pad's button names by bit (PCSX.CONSTS.PAD.BUTTON, as run.lua records them). */
static const char *const port_button_names[16] = {
    "SELECT", "L3", "R3", "START", "UP", "RIGHT", "DOWN", "LEFT",
    "L2", "R2", "L1", "R1", "TRIANGLE", "CIRCLE", "CROSS", "SQUARE",
};

/* A growing array: returns its new last element (of `size` bytes), zeroed. */
static void *port_append(void *data_ptr, size_t *n, size_t *cap, size_t size) {
    void **data = data_ptr;
    if (*n == *cap) {
        *cap = *cap ? *cap * 2 : 16;
        *data = realloc(*data, *cap * size);
        if (*data == NULL) {
            port_fatal("framelog: out of memory");
        }
    }
    memset((char *)*data + *n * size, 0, size);
    return (char *)*data + (*n)++ * size;
}

static void port_logf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void port_logf(const char *fmt, ...) {
    va_list ap;
    if (port_log_file == NULL) {
        return;
    }
    va_start(ap, fmt);
    vfprintf(port_log_file, fmt, ap);
    va_end(ap);
    fputc('\n', port_log_file);
}

void port_framelog_open(const char *log_path, const char *record_path) {
    if (log_path != NULL) {
        port_log_file = fopen(log_path, "wb"); /* "b": the same bytes on Windows (no CRLF) */
        if (port_log_file == NULL) {
            port_fatal("framelog: --log %s: cannot open", log_path);
        }
        port_file_line_buffered(port_log_file); /* a crashed run's log ends at its last line */
        port_logf("# dw2003 port frame log 1 (port/README.md)");
    }
    if (record_path != NULL) {
        FILE *f = fopen(record_path, "wb"); /* fail now rather than at exit */
        if (f == NULL) {
            port_fatal("framelog: --record %s: cannot open", record_path);
        }
        fclose(f);
        port_record_path = strdup(record_path);
        if (port_record_path == NULL) {
            port_fatal("framelog: out of memory");
        }
    }
}

void port_framelog_frame(void) {
    s32 stage = port_state_stage(), file = port_state_file();
    u32 map = (u32)port_state_map();
    u32 prims;
    u32 hash = psyq_gpu_take_hash(&prims);
    port_logf("F %ld st %d fl %d map 0x%X prims %u hash %08x", port_frames, stage, file, map, prims, hash);
    if (!port_sampled || stage != port_last_stage || file != port_last_file) {
        *(PortOverlayStep *)port_append(&port_overlay_seq, &port_overlay_n, &port_overlay_cap,
                                        sizeof(*port_overlay_seq)) = (PortOverlayStep){ port_frames, stage, file };
        port_last_stage = stage;
        port_last_file = file;
        port_logf("S %ld stage %d file %d", port_frames, stage, file);
    }
    if (!port_sampled || map != port_last_map) {
        *(PortMapStep *)port_append(&port_map_seq, &port_map_n, &port_map_cap, sizeof(*port_map_seq)) =
            (PortMapStep){ port_frames, map };
        port_last_map = map;
        port_logf("M %ld map 0x%X", port_frames, map);
    }
    port_sampled = 1;
}

void port_framelog_overlay_load(int tier, s32 file, const char *name, u32 word0, u32 size) {
    port_logf("L %ld tier %d file 0x%X %s word0 0x%08X size 0x%X", port_frames, tier, file,
              name != NULL ? name : "(data)", word0, size);
}

void port_framelog_checkpoint(const char *name) {
    PortCheckpoint *c;
    c = port_append(&port_checkpoints, &port_checkpoint_n, &port_checkpoint_cap, sizeof(*port_checkpoints));
    c->name = strdup(name);
    if (c->name == NULL) {
        port_fatal("framelog: out of memory");
    }
    c->frame = port_frames;
    c->stage = port_state_stage();
    c->map = (u32)port_state_map();
    c->random_index = port_state_random_index();
    port_state_gamestate_sha1(c->sha1, c->stable);
    port_logf("C %ld %s stage %d map 0x%X rnd %d sha1 %s stable %s", c->frame, c->name, c->stage, c->map,
              c->random_index, c->sha1, c->stable);
}

/* The console's reset (port/src/reset.c): an event line; the sequences go on (run.lua's listener keeps its last
 * stage/file/map across PCSX-Redux's hardResetEmulator, so the next frame records the cleared RAM's (0, 0) and map 0
 * as a change), and so do the checkpoints and the input trace. */
void port_framelog_reset(void) {
    port_logf("R %ld reset", port_frames);
}

void port_framelog_input(u16 buttons) {
    port_inputs_used = 1;
    if (buttons == port_last_buttons) {
        return;
    }
    *(PortInput *)port_append(&port_inputs, &port_input_n, &port_input_cap, sizeof(*port_inputs)) =
        (PortInput){ port_frames, buttons };
    port_last_buttons = buttons;
    port_logf("I %ld buttons 0x%04X", port_frames, buttons);
}

u16 port_framelog_last_input(void) {
    return port_last_buttons;
}

/* ---- The record (JSON) */

static void port_json_string(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') {
            fprintf(f, "\\%c", c);
        } else if (c < 0x20) {
            fprintf(f, "\\u%04x", c);
        } else {
            fputc(c, f);
        }
    }
    fputc('"', f);
}

/* The buttons of an input as run.lua lists them: names sorted by byte order (Lua's table.sort). */
static void port_json_buttons(FILE *f, u16 buttons) {
    const char *names[16];
    int n = 0, i, j;
    for (i = 0; i < 16; i++) {
        if (buttons & (1u << i)) {
            for (j = n; j > 0 && strcmp(names[j - 1], port_button_names[i]) > 0; j--) {
                names[j] = names[j - 1];
            }
            names[j] = port_button_names[i];
            n++;
        }
    }
    fputc('[', f);
    for (i = 0; i < n; i++) {
        fputs(i ? ", " : "", f);
        port_json_string(f, names[i]);
    }
    fputc(']', f);
}

static void port_write_record(int status, const char *reason) {
    FILE *f = fopen(port_record_path, "wb");
    size_t i;
    if (f == NULL) {
        port_log("framelog: --record %s: cannot write", port_record_path);
        return;
    }
    fprintf(f, "{\n \"runner\": \"port\",\n \"status\": %d,\n \"reason\": ", status);
    port_json_string(f, reason != NULL ? reason : "");
    fprintf(f, ",\n \"frames\": %ld,\n \"checkpoints\": [", port_frames);
    for (i = 0; i < port_checkpoint_n; i++) {
        const PortCheckpoint *c = &port_checkpoints[i];
        fprintf(f, "%s\n  {\"name\": ", i ? "," : "");
        port_json_string(f, c->name);
        fprintf(f,
                ", \"frame\": %ld, \"stage\": %d, \"map\": %u, \"random_index\": %d, \"gamestate_sha1\": \"%s\", "
                "\"gamestate_sha1_stable\": \"%s\"}",
                c->frame, c->stage, c->map, c->random_index, c->sha1, c->stable);
    }
    fprintf(f, "%s],\n \"overlay_sequence\": [", port_checkpoint_n ? "\n " : "");
    for (i = 0; i < port_overlay_n; i++) {
        fprintf(f, "%s\n  {\"frame\": %ld, \"stage\": %d, \"file\": %d}", i ? "," : "", port_overlay_seq[i].frame,
                port_overlay_seq[i].stage, port_overlay_seq[i].file);
    }
    fprintf(f, "%s],\n \"map_sequence\": [", port_overlay_n ? "\n " : "");
    for (i = 0; i < port_map_n; i++) {
        fprintf(f, "%s\n  {\"frame\": %ld, \"map\": %u}", i ? "," : "", port_map_seq[i].frame, port_map_seq[i].map);
    }
    fprintf(f, "%s]", port_map_n ? "\n " : "");
    if (port_inputs_used) {
        fputs(",\n \"inputs\": [", f);
        for (i = 0; i < port_input_n; i++) {
            fprintf(f, "%s\n  {\"frame\": %ld, \"buttons\": ", i ? "," : "", port_inputs[i].frame);
            port_json_buttons(f, port_inputs[i].buttons);
            fputc('}', f);
        }
        fprintf(f, "%s]", port_input_n ? "\n " : "");
    }
    fputs("\n}\n", f);
    if (fclose(f) != 0) {
        port_log("framelog: --record %s: write error", port_record_path);
    }
}

void port_framelog_close(int status, const char *reason) {
    port_logf("X %ld status %d %s", port_frames, status, reason != NULL ? reason : "");
    if (port_log_file != NULL) {
        fclose(port_log_file);
        port_log_file = NULL;
    }
    if (port_record_path != NULL) {
        port_write_record(status, reason);
    }
}
