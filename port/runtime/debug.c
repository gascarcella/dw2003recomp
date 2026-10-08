/* The debug channel (port_harness.h; `--debug SOCKET`): a Unix stream socket on which a tool (tools/mcp, a script)
 * drives and inspects the running game: pause, step, wait, the pad, memory, screenshots, the hash, the reset. Nothing
 * of it exists without `--debug`: the bare binary, the replays and the goldens are unchanged (port_frame pays one
 * branch). The design: the game thread polls the socket itself, once per vsync (port_debug_frame, from port_frame
 * after the script's step and before the video) and continuously (50 times a second) while the pump holds the game
 * in its pause loop (port_debug_poll_paused), so every command runs between two vsyncs: reads and writes are
 * frame-consistent and a driven run stays deterministic. The pause is the pump's (shared with the window's pause
 * key: port_pump_pause_request/port_pump_resume_request); a deferred op (step, wait, pad with sync) is answered from
 * port_debug_frame when its condition completes, and until then no further request is read, so the answers stay in
 * order. The reset is requested after the reply went out (port_debug_frame's end, or port_debug_resumed after the
 * pause loop), never from inside the handler: port_reset_request longjmps. The channel's state (the socket, the pad
 * ownership, a pending op) is runtime state, outside the game: it survives the reset.
 *
 * Protocol (v1; the condensed spec: tools/mcp is the client):
 * - `--debug SOCKET` creates the socket (a stale one is unlinked), listens, accepts one client at a time (another is
 *   accepted once the first closed). It turns the watchdog off and, like --window and --script, the default frame cap
 *   (an explicit --max-frames N still applies). `--debug-hold` holds the game paused at the end of its first vsync
 *   until the client resumes it (a run reproducible from frame 1; otherwise an unthrottled headless game is ~150
 *   frames in by the time the client's first request lands).
 * - Transport: newline-delimited JSON, one request per line, one response per line, in order. The game never blocks
 *   on the socket except while paused (and then the window's events are still pumped).
 * - Request `{"id": <int>, "op": "<name>", ...args}`; response `{"id": <same>, "ok": true, ...result}` or
 *   `{"id": <same>, "ok": false, "error": "<text>"}`. Numbers are JSON integers or hex strings ("0x8001..."); byte
 *   strings are lowercase hex ("0a1b2c"). An unknown op or a malformed line gets an error (with `"id": null` when the
 *   line had no id). Deferred ops (`step`, `wait`, `pad` with `"sync": true`) are answered when their condition
 *   completes; the client sends nothing else meanwhile.
 * - Ops (args -> result):
 *   status: -> frame, stage, file, map, slot1_word0, paused (0/1), pace, rate, window (0/1), script (0/1),
 *     pad_owner ("debug"/"script"/"window"/"none"), pad (u16: the buttons last set).
 *   pause: -> frame. Holds the game at the next vsync boundary (right after this frame when handled from
 *     port_frame). Idempotent. resume: -> frame; idempotent.
 *   step: frames (>= 1, default 1) -> frame. Runs exactly `frames` vsyncs then pauses; deferred.
 *   peek: addr (host), len (1..65536) -> data (hex). Raw host memory; the range must be mapped readable (checked in
 *     /proc/self/maps: the game's globals, the arena, anything the process maps), else "unmapped": a bad address never
 *     faults the game. poke: addr, data (hex) -> len; the range must be mapped writable.
 *   peek_ps1: addr (PS1), len -> data. An arena address (>= 0x80082CB0, inside the arena: the slots and the heap)
 *     reads directly, any length; any other PS1 address only through the state map (game_state_read's ranges:
 *     layout-identical EXE objects, the prefixes, the field table), len 1, 2 or 4; else "unmapped".
 *     poke_ps1: addr, data -> len; the same mapping, written.
 *   pad: buttons (u16, PS1 bit order: SELECT=0, L3, R3, START, UP, RIGHT, DOWN, LEFT, L2, R2, L1, R1, TRIANGLE,
 *     CIRCLE, CROSS, SQUARE=15; active high), frames (>= 0; 0 = hold until the next pad op), release (>= 0, default
 *     0: frames of 0 after `frames`), sync (bool, default false) -> frame. The channel owns the pad from the first
 *     pad op until pad_free (window input no longer reaches it; a script keeps precedence: "script owns the pad");
 *     each vsync it calls psyq_pad_set(0, 1, value). With sync the answer waits until frames + release vsyncs ran,
 *     and a game that was paused when the op arrived is paused again then (a driven run stays at a known frame).
 *     pad_free: releases the pad to the window (or none).
 *   wait: one of addr (host) / ps1 (PS1 address) / ps1_stage / ps1_map, with size (1/2/4), signed (bool), value,
 *     timeout (frames, default 600) -> frame, hit (0/1). Runs until the read equals value (checked once before
 *     running, then after each vsync) or timeout frames passed; deferred; leaves the game paused. ps1_stage and
 *     ps1_map take only value (game_state_stage / game_state_map).
 *   screenshot: path -> w, h, path. The current display image (the one port_video_frame last converted, or
 *     converted now) as a binary PPM (P6), the bytes --screenshot writes; headless too.
 *   hash: -> frame, sha1, stable_sha1: gamestate_data's PS1 image, as a checkpoint hashes it.
 *   pace: fps (0 = unthrottled) -> pace (port_pace_set; the headless run is never paced).
 *   reset: -> frame; answers, then the console resets (port_reset_request) from the pump; the channel survives it,
 *     and a game that was paused is paused again at its first vsync after the reset (frame + 1).
 *   quit: status (default 0); answers, then port_exit(status, "debug quit").
 * - Pause semantics: `paused` means the pump holds the game between vsyncs in its pause loop; the pause key and the
 *   channel share that state. While paused the channel keeps answering peek/poke/screenshot/status/hash; step runs
 *   frames from there. A client that disconnects drops its pending op, frees the pad and resumes the game, so a dead
 *   tool never leaves the game held. */
#ifdef _WIN32
/* The Windows build has no channel yet (a Unix socket, poll and /proc/self/maps are POSIX): --debug is refused; the
 * pump's hooks are no-ops (port_debug_active stays 0, so none is reached). */
#include <stdio.h>
#include <stdlib.h>

#include "port_harness.h"

int port_debug_active;
int port_debug_pad_owned;

void port_debug_open(const char *path) {
    fprintf(stderr, "port: --debug %s: the debug channel is not available on Windows\n", path);
    exit(64);
}
void port_debug_frame(void) {
}
void port_debug_poll_paused(void) {
}
void port_debug_resumed(void) {
}
void port_debug_close(void) {
}
#else
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "json.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

int port_debug_active;
int port_debug_pad_owned;

#define DEBUG_DATA_MAX 65536                        /* bytes per peek/poke */
#define DEBUG_LINE_MAX (DEBUG_DATA_MAX * 2 + 4096)   /* a poke's hex and the rest of its line */
#define DEBUG_REPLY_MAX (DEBUG_DATA_MAX * 2 + 4096)  /* a peek's hex and the rest of its reply */

static const char *debug_path;
static int debug_listen_fd = -1, debug_client_fd = -1;
static char *debug_line;  /* the partial line read so far */
static size_t debug_line_n;
static char *debug_reply; /* the response being built */

/* A request's id: a JSON integer, or none (then "null" in the answer). */
typedef struct DebugReq {
    int has_id;
    long long id;
} DebugReq;

/* The deferred op in flight (at most one; nothing else is read meanwhile). */
typedef enum DebugPending {
    DEBUG_NONE,
    DEBUG_STEP,
    DEBUG_WAIT,
    DEBUG_PAD_SYNC,
} DebugPending;
static DebugPending debug_pending;
static DebugReq debug_pending_req;
static long debug_step_left;

typedef enum DebugWaitKind {
    DEBUG_WAIT_HOST,
    DEBUG_WAIT_PS1,
    DEBUG_WAIT_STAGE,
    DEBUG_WAIT_MAP,
} DebugWaitKind;
typedef struct DebugWait {
    DebugWaitKind kind;
    unsigned long long addr;
    int size, is_signed;
    long long value;
    long timeout, elapsed;
} DebugWait;
static DebugWait debug_wait;

/* The pad: `buttons` for `hold` more vsyncs (or forever), then 0 for `release` vsyncs, then 0. Applied once per
 * frame (debug_pad_applied_frame), at the op itself and at every later port_debug_frame. */
static u16 debug_pad_buttons;
static long debug_pad_hold, debug_pad_release;
static int debug_pad_forever;
static int debug_pad_repause; /* the sync pad op found the game paused: pause it again when done */
static long debug_pad_applied_frame = -1;

static int debug_reset_pending, debug_reset_repause; /* a reset asked for; it was asked for while paused */

/* ---- The socket */

static void debug_disconnect(const char *why) {
    port_log("debug: client disconnected (%s)", why);
    close(debug_client_fd);
    debug_client_fd = -1;
    debug_line_n = 0;
    debug_pending = DEBUG_NONE; /* its answer has no one to go to */
    if (port_debug_pad_owned) {
        port_debug_pad_owned = 0;
        psyq_pad_set(0, 1, 0);
        port_framelog_input(0);
    }
    port_pump_resume_request(); /* a dead tool never leaves the game held */
}

/* Writes the whole reply; the client's fd is non-blocking, so a full pipe waits (at most a second per try). */
static void debug_send(const char *s, size_t n) {
    while (n > 0 && debug_client_fd >= 0) {
        ssize_t w = write(debug_client_fd, s, n);
        if (w > 0) {
            s += w;
            n -= (size_t)w;
        } else if (w < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            struct pollfd p = { debug_client_fd, POLLOUT, 0 };
            if (poll(&p, 1, 1000) <= 0) {
                debug_disconnect("the client does not read");
            }
        } else if (w < 0 && errno == EINTR) {
            continue;
        } else {
            debug_disconnect("write failed");
        }
    }
}

static size_t debug_reply_head(const DebugReq *req, int ok) {
    int n;
    if (req->has_id) {
        n = snprintf(debug_reply, DEBUG_REPLY_MAX, "{\"id\": %lld, \"ok\": %s", req->id, ok ? "true" : "false");
    } else {
        n = snprintf(debug_reply, DEBUG_REPLY_MAX, "{\"id\": null, \"ok\": %s", ok ? "true" : "false");
    }
    return (size_t)n;
}

/* `{"id": .., "ok": true, <fmt>}` (fmt: the result's fields, NULL for none). */
static void debug_ok(const DebugReq *req, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void debug_ok(const DebugReq *req, const char *fmt, ...) {
    size_t n = debug_reply_head(req, 1);
    va_list ap;
    if (fmt != NULL) {
        n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, ", ");
        va_start(ap, fmt);
        n += (size_t)vsnprintf(debug_reply + n, DEBUG_REPLY_MAX - n, fmt, ap);
        va_end(ap);
    }
    n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, "}\n");
    debug_send(debug_reply, n);
}

/* `{"id": .., "ok": true, "data": "<hex>"}` */
static void debug_ok_data(const DebugReq *req, const u8 *data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    size_t n = debug_reply_head(req, 1), i;
    n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, ", \"data\": \"");
    for (i = 0; i < len; i++) {
        debug_reply[n++] = hex[data[i] >> 4];
        debug_reply[n++] = hex[data[i] & 15];
    }
    n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, "\"}\n");
    debug_send(debug_reply, n);
}

/* `{"id": .., "ok": false, "error": "<text>"}`; the text is ours (plain ASCII, no quotes to escape). */
static void debug_error(const DebugReq *req, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void debug_error(const DebugReq *req, const char *fmt, ...) {
    size_t n = debug_reply_head(req, 0);
    va_list ap;
    n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, ", \"error\": \"");
    va_start(ap, fmt);
    n += (size_t)vsnprintf(debug_reply + n, DEBUG_REPLY_MAX - n, fmt, ap);
    va_end(ap);
    n += (size_t)snprintf(debug_reply + n, DEBUG_REPLY_MAX - n, "\"}\n");
    debug_send(debug_reply, n);
}

void port_debug_open(const char *path) {
    struct sockaddr_un addr;
    if (strlen(path) >= sizeof(addr.sun_path)) {
        port_fatal("--debug: the socket path is longer than %zu bytes: %s", sizeof(addr.sun_path) - 1, path);
    }
    debug_path = path;
    debug_line = malloc(DEBUG_LINE_MAX);
    debug_reply = malloc(DEBUG_REPLY_MAX);
    if (debug_line == NULL || debug_reply == NULL) {
        port_fatal("--debug: out of memory");
    }
    debug_listen_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (debug_listen_fd < 0) {
        port_fatal("--debug: socket: %s", strerror(errno));
    }
    unlink(path); /* a stale socket of an earlier run */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if (bind(debug_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0 || listen(debug_listen_fd, 1) != 0) {
        port_fatal("--debug: cannot listen on %s: %s", path, strerror(errno));
    }
    port_debug_active = 1;
    port_log("debug: listening on %s", path);
}

void port_debug_close(void) {
    if (debug_client_fd >= 0) {
        close(debug_client_fd);
        debug_client_fd = -1;
    }
    if (debug_listen_fd >= 0) {
        close(debug_listen_fd);
        debug_listen_fd = -1;
        unlink(debug_path);
    }
}

/* ---- Arguments */

/* An integer field: a JSON integer or a hex string ("0x...", at most 16 digits). 1 found, 0 absent, -1 bad (an error
 * was answered). */
static int debug_int(const DebugReq *req, const PortJson *obj, const char *key, long long *out) {
    const PortJson *v = port_json_get(obj, key);
    if (v == NULL) {
        return 0;
    }
    if (v->type == PORT_JSON_NUMBER) {
        if (v->number != floor(v->number) || fabs(v->number) > 9007199254740992.0) {
            debug_error(req, "%s: not an integer", key);
            return -1;
        }
        *out = (long long)v->number;
        return 1;
    }
    if (v->type == PORT_JSON_STRING && strncmp(v->string, "0x", 2) == 0 && v->string[2] != '\0' &&
        strlen(v->string) <= 2 + 16 && strspn(v->string + 2, "0123456789abcdefABCDEF") == strlen(v->string + 2)) {
        *out = (long long)strtoull(v->string + 2, NULL, 16);
        return 1;
    }
    debug_error(req, "%s: a number or a hex string (\"0x...\", at most 16 digits) expected", key);
    return -1;
}

/* A required integer; `*out` within [min, max]. 1, or 0 with the error answered. */
static int debug_int_req(const DebugReq *req, const PortJson *obj, const char *key, long long min, long long max,
                         long long *out) {
    int r = debug_int(req, obj, key, out);
    if (r == 0) {
        debug_error(req, "%s missing", key);
        return 0;
    }
    if (r < 0) {
        return 0;
    }
    if (*out < min || *out > max) {
        debug_error(req, "%s: %lld out of range", key, *out);
        return 0;
    }
    return 1;
}

/* An optional integer with a default; 1, or 0 with the error answered. */
static int debug_int_opt(const DebugReq *req, const PortJson *obj, const char *key, long long dflt, long long min,
                         long long max, long long *out) {
    int r = debug_int(req, obj, key, out);
    if (r == 0) {
        *out = dflt;
        return 1;
    }
    if (r < 0) {
        return 0;
    }
    if (*out < min || *out > max) {
        debug_error(req, "%s: %lld out of range", key, *out);
        return 0;
    }
    return 1;
}

/* An optional boolean (default false); 1, or 0 with the error answered. */
static int debug_bool_opt(const DebugReq *req, const PortJson *obj, const char *key, int *out) {
    const PortJson *v = port_json_get(obj, key);
    *out = 0;
    if (v == NULL) {
        return 1;
    }
    if (v->type != PORT_JSON_BOOL) {
        debug_error(req, "%s: true or false expected", key);
        return 0;
    }
    *out = v->boolean;
    return 1;
}

/* `data`: a hex string of 1..DEBUG_DATA_MAX bytes, decoded into `out` (`*len` bytes); 0 with the error answered. */
static int debug_hex_data(const DebugReq *req, const PortJson *obj, u8 *out, size_t *len) {
    const PortJson *v = port_json_get(obj, "data");
    size_t n, i;
    if (v == NULL || v->type != PORT_JSON_STRING) {
        debug_error(req, "data (a hex string) missing");
        return 0;
    }
    n = strlen(v->string);
    if (n == 0 || n % 2 != 0 || n / 2 > DEBUG_DATA_MAX || strspn(v->string, "0123456789abcdefABCDEF") != n) {
        debug_error(req, "data: an even number of hex digits (1..%d bytes) expected", DEBUG_DATA_MAX);
        return 0;
    }
    for (i = 0; i < n; i += 2) {
        char pair[3] = { v->string[i], v->string[i + 1], '\0' };
        out[i / 2] = (u8)strtoul(pair, NULL, 16);
    }
    *len = n / 2;
    return 1;
}

/* ---- Memory */

/* Whether [addr, addr + len) lies inside one mapping of this process that is readable (and writable, for a poke):
 * /proc/self/maps, read at every access (a few KB; mappings come and go with malloc). A miss is "unmapped" to the
 * client instead of a fault in the game. */
static int debug_host_mapped(unsigned long long addr, size_t len, int write) {
    FILE *f = fopen("/proc/self/maps", "r");
    char line[512];
    int ok = 0;
    if (f == NULL || addr + len < addr) {
        if (f != NULL) {
            fclose(f);
        }
        return 0;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        unsigned long long lo, hi;
        char perms[8];
        if (sscanf(line, "%llx-%llx %7s", &lo, &hi, perms) == 3 && addr >= lo && addr + len <= hi) {
            ok = perms[0] == 'r' && (!write || perms[1] == 'w');
            break;
        }
    }
    fclose(f);
    return ok;
}

/* The host bytes of PS1 address `addr` for `len` bytes: the arena directly, any other address through the state map
 * (1, 2 or 4 bytes); NULL when unmapped. */
static u8 *debug_ps1_ptr(u32 addr, size_t len) {
    if (addr >= PORT_SLOT1_BASE && addr - PORT_SLOT1_BASE < PORT_ARENA_SIZE &&
        len <= PORT_ARENA_SIZE - (addr - PORT_SLOT1_BASE)) {
        return (u8 *)port_arena_base() + (addr - PORT_SLOT1_BASE);
    }
    if (len == 1 || len == 2 || len == 4) {
        return game_state_host(addr, (int)len);
    }
    return NULL;
}

/* A wait's read: 1 with the value (sign-extended as asked; a 4-byte unsigned read as a u32), 0 when unmapped. */
static int debug_wait_read(const DebugWait *w, long long *out) {
    u8 buf[4];
    const u8 *p;
    if (w->kind == DEBUG_WAIT_STAGE) {
        *out = game_state_stage();
        return 1;
    }
    if (w->kind == DEBUG_WAIT_MAP) {
        *out = (u32)game_state_map();
        return 1;
    }
    if (w->kind == DEBUG_WAIT_HOST) {
        if (!debug_host_mapped(w->addr, (size_t)w->size, 0)) {
            return 0;
        }
        p = (const u8 *)(uintptr_t)w->addr;
    } else {
        p = debug_ps1_ptr((u32)w->addr, (size_t)w->size);
        if (p == NULL) {
            return 0;
        }
    }
    memcpy(buf, p, (size_t)w->size);
    if (w->size == 1) {
        *out = w->is_signed ? (long long)(s8)buf[0] : (long long)buf[0];
    } else if (w->size == 2) {
        u16 v;
        memcpy(&v, buf, 2);
        *out = w->is_signed ? (long long)(s16)v : (long long)v;
    } else {
        u32 v;
        memcpy(&v, buf, 4);
        *out = w->is_signed ? (long long)(s32)v : (long long)v;
    }
    return 1;
}

static int debug_wait_hit(void) {
    long long v;
    return debug_wait_read(&debug_wait, &v) && v == debug_wait.value;
}

/* ---- The pad */

static void debug_pad_apply(void) {
    u16 v = 0;
    if (debug_pad_forever) {
        v = debug_pad_buttons;
    } else if (debug_pad_hold > 0) {
        v = debug_pad_buttons;
        debug_pad_hold--;
    } else if (debug_pad_release > 0) {
        debug_pad_release--;
    }
    psyq_pad_set(0, 1, v);
    port_framelog_input(v);
    debug_pad_applied_frame = port_frames;
}

/* Once per vsync: a sync whose frames + release vsyncs all ran is answered, then the pad gets this vsync's value
 * (unless the op itself applied it at this frame: a pad op handled at frame F sets the pad the game reads during
 * F + 1). */
static void debug_pad_frame(void) {
    if (!port_debug_pad_owned) {
        return;
    }
    if (debug_pending == DEBUG_PAD_SYNC && !debug_pad_forever && debug_pad_hold == 0 && debug_pad_release == 0) {
        debug_pending = DEBUG_NONE;
        if (debug_pad_repause) {
            port_pump_pause_request(); /* a press from the paused game leaves it paused: the tool's next call finds
                                        * the frame it was answered at, not thousands of unthrottled frames later */
        }
        debug_ok(&debug_pending_req, "\"frame\": %ld", port_frames);
    }
    if (debug_pad_applied_frame != port_frames) {
        debug_pad_apply();
    }
}

/* ---- The ops */

static const char *debug_pad_owner(void) {
    if (port_script_active) {
        return "script";
    }
    if (port_debug_pad_owned) {
        return "debug";
    }
    return port_window ? "window" : "none";
}

static void debug_op_status(const DebugReq *req) {
    debug_ok(req,
             "\"frame\": %ld, \"stage\": %d, \"file\": %d, \"map\": %u, \"slot1_word0\": %u, \"paused\": %d, "
             "\"pace\": %ld, \"rate\": %ld, \"window\": %d, \"script\": %d, \"pad_owner\": \"%s\", \"pad\": %u",
             port_frames, game_state_stage(), game_state_file(), (u32)game_state_map(), port_state_slot1_word0(),
             port_pump_paused(), port_pace_get(), port_rate, port_window != 0, port_script_active != 0,
             debug_pad_owner(), port_framelog_last_input());
}

static void debug_op_step(const DebugReq *req, const PortJson *obj) {
    long long frames;
    if (!debug_int_opt(req, obj, "frames", 1, 1, 0x7FFFFFFF, &frames)) {
        return;
    }
    debug_pending = DEBUG_STEP;
    debug_pending_req = *req;
    debug_step_left = (long)frames;
    port_pump_resume_request();
}

static void debug_op_peek(const DebugReq *req, const PortJson *obj, int ps1) {
    long long addr, len;
    const u8 *p;
    if (!debug_int_req(req, obj, "addr", 0, ps1 ? 0xFFFFFFFFLL : LLONG_MAX, &addr) ||
        !debug_int_req(req, obj, "len", 1, DEBUG_DATA_MAX, &len)) {
        return;
    }
    if (ps1) {
        p = debug_ps1_ptr((u32)addr, (size_t)len);
    } else {
        p = debug_host_mapped((unsigned long long)addr, (size_t)len, 0) ? (const u8 *)(uintptr_t)addr : NULL;
    }
    if (p == NULL) {
        debug_error(req, "unmapped");
        return;
    }
    debug_ok_data(req, p, (size_t)len);
}

static void debug_op_poke(const DebugReq *req, const PortJson *obj, int ps1) {
    static u8 data[DEBUG_DATA_MAX];
    long long addr;
    size_t len;
    u8 *p;
    if (!debug_int_req(req, obj, "addr", 0, ps1 ? 0xFFFFFFFFLL : LLONG_MAX, &addr) ||
        !debug_hex_data(req, obj, data, &len)) {
        return;
    }
    if (ps1) {
        p = debug_ps1_ptr((u32)addr, len);
    } else {
        p = debug_host_mapped((unsigned long long)addr, len, 1) ? (u8 *)(uintptr_t)addr : NULL;
    }
    if (p == NULL) {
        debug_error(req, "unmapped");
        return;
    }
    memcpy(p, data, len);
    debug_ok(req, "\"len\": %zu", len);
}

static void debug_op_pad(const DebugReq *req, const PortJson *obj) {
    long long buttons, frames, release;
    int sync;
    if (port_script_active) {
        debug_error(req, "script owns the pad");
        return;
    }
    if (!debug_int_req(req, obj, "buttons", 0, 0xFFFF, &buttons) ||
        !debug_int_opt(req, obj, "frames", 0, 0, 0x7FFFFFFF, &frames) ||
        !debug_int_opt(req, obj, "release", 0, 0, 0x7FFFFFFF, &release) || !debug_bool_opt(req, obj, "sync", &sync)) {
        return;
    }
    port_debug_pad_owned = 1;
    debug_pad_buttons = (u16)buttons;
    debug_pad_forever = frames == 0;
    debug_pad_hold = (long)frames;
    debug_pad_release = (long)release;
    debug_pad_apply();
    if (sync && !debug_pad_forever) {
        debug_pending = DEBUG_PAD_SYNC;
        debug_pending_req = *req;
        debug_pad_repause = port_pump_paused();
        port_pump_resume_request();
        return;
    }
    debug_ok(req, "\"frame\": %ld", port_frames);
}

static void debug_op_pad_free(const DebugReq *req) {
    if (port_debug_pad_owned) {
        port_debug_pad_owned = 0;
        psyq_pad_set(0, 1, 0);
        port_framelog_input(0);
    }
    debug_ok(req, NULL);
}

static void debug_op_wait(const DebugReq *req, const PortJson *obj) {
    DebugWait w;
    long long v, size, timeout;
    int r;
    memset(&w, 0, sizeof(w));
    if (port_json_get(obj, "ps1_stage") != NULL) {
        w.kind = DEBUG_WAIT_STAGE;
    } else if (port_json_get(obj, "ps1_map") != NULL) {
        w.kind = DEBUG_WAIT_MAP;
    } else if ((r = debug_int(req, obj, "ps1", &v)) != 0) {
        if (r < 0 || v < 0 || v > 0xFFFFFFFFLL) {
            if (r > 0) {
                debug_error(req, "ps1: not a 32-bit address");
            }
            return;
        }
        w.kind = DEBUG_WAIT_PS1;
        w.addr = (unsigned long long)v;
    } else if ((r = debug_int(req, obj, "addr", &v)) != 0) {
        if (r < 0) {
            return;
        }
        w.kind = DEBUG_WAIT_HOST;
        w.addr = (unsigned long long)v;
    } else {
        debug_error(req, "one of addr, ps1, ps1_stage, ps1_map expected");
        return;
    }
    if (!debug_int_opt(req, obj, "size", 4, 1, 4, &size) || !debug_bool_opt(req, obj, "signed", &w.is_signed) ||
        !debug_int_req(req, obj, "value", LLONG_MIN, LLONG_MAX, &v) ||
        !debug_int_opt(req, obj, "timeout", 600, 0, 0x7FFFFFFF, &timeout)) {
        return;
    }
    if (size != 1 && size != 2 && size != 4) {
        debug_error(req, "size: 1, 2 or 4");
        return;
    }
    w.size = (int)size;
    w.value = v;
    w.timeout = (long)timeout;
    if (!debug_wait_read(&w, &v)) {
        debug_error(req, "unmapped");
        return;
    }
    if (v == w.value) {
        port_pump_pause_request(); /* already there: the game stays (or is) paused */
        debug_ok(req, "\"frame\": %ld, \"hit\": 1", port_frames);
        return;
    }
    debug_wait = w;
    debug_pending = DEBUG_WAIT;
    debug_pending_req = *req;
    port_pump_resume_request();
}

static void debug_op_screenshot(const DebugReq *req, const PortJson *obj) {
    const PortJson *path = port_json_get(obj, "path");
    int w, h;
    if (path == NULL || path->type != PORT_JSON_STRING || path->string[0] == '\0') {
        debug_error(req, "path (a string) missing");
        return;
    }
    if (strpbrk(path->string, "\"\\\n\r\t") != NULL) {
        debug_error(req, "path: no quotes, backslashes or control characters");
        return;
    }
    if (!port_video_screenshot_now(path->string, &w, &h)) {
        debug_error(req, "cannot write the file");
        return;
    }
    debug_ok(req, "\"w\": %d, \"h\": %d, \"path\": \"%s\"", w, h, path->string);
}

static void debug_op_hash(const DebugReq *req) {
    char full[41], stable[41];
    port_state_sha1(full, stable);
    debug_ok(req, "\"frame\": %ld, \"sha1\": \"%s\", \"stable_sha1\": \"%s\"", port_frames, full, stable);
}

static void debug_op_pace(const DebugReq *req, const PortJson *obj) {
    long long fps;
    if (!debug_int_req(req, obj, "fps", 0, 100000, &fps)) {
        return;
    }
    port_pace_set((long)fps);
    debug_ok(req, "\"pace\": %ld", port_pace_get());
}

static void debug_op_reset(const DebugReq *req) {
    debug_ok(req, "\"frame\": %ld", port_frames);
    debug_reset_pending = 1;        /* done by the pump once this handler returned (port_reset_request longjmps) */
    debug_reset_repause = port_pump_paused();
    port_pump_resume_request();     /* out of the pause loop first, if paused */
}

/* The reset itself (the reply went out): a game that was paused is paused again at its first vsync after it. */
static void debug_reset(void) {
    debug_reset_pending = 0;
    if (debug_reset_repause) {
        port_pump_pause_request();
    }
    port_reset_request();
}

static void debug_op_quit(const DebugReq *req, const PortJson *obj) {
    long long status;
    if (!debug_int_opt(req, obj, "status", 0, 0, 255, &status)) {
        return;
    }
    debug_ok(req, NULL);
    port_exit((int)status, "debug quit");
}

static void debug_handle(const char *line, size_t n) {
    char err[256];
    PortJson *root = port_json_parse(line, n, err, sizeof(err));
    DebugReq req = { 0, 0 };
    const PortJson *id, *op;
    if (root == NULL) {
        debug_error(&req, "malformed request");
        return;
    }
    id = port_json_get(root, "id");
    if (id != NULL && id->type == PORT_JSON_NUMBER && id->number == floor(id->number) &&
        fabs(id->number) <= 9007199254740992.0) {
        req.has_id = 1;
        req.id = (long long)id->number;
    }
    op = port_json_get(root, "op");
    if (root->type != PORT_JSON_OBJECT || op == NULL || op->type != PORT_JSON_STRING) {
        debug_error(&req, "op (a string) missing");
    } else if (strcmp(op->string, "status") == 0) {
        debug_op_status(&req);
    } else if (strcmp(op->string, "pause") == 0) {
        port_pump_pause_request();
        debug_ok(&req, "\"frame\": %ld", port_frames);
    } else if (strcmp(op->string, "resume") == 0) {
        port_pump_resume_request();
        debug_ok(&req, "\"frame\": %ld", port_frames);
    } else if (strcmp(op->string, "step") == 0) {
        debug_op_step(&req, root);
    } else if (strcmp(op->string, "peek") == 0) {
        debug_op_peek(&req, root, 0);
    } else if (strcmp(op->string, "poke") == 0) {
        debug_op_poke(&req, root, 0);
    } else if (strcmp(op->string, "peek_ps1") == 0) {
        debug_op_peek(&req, root, 1);
    } else if (strcmp(op->string, "poke_ps1") == 0) {
        debug_op_poke(&req, root, 1);
    } else if (strcmp(op->string, "pad") == 0) {
        debug_op_pad(&req, root);
    } else if (strcmp(op->string, "pad_free") == 0) {
        debug_op_pad_free(&req);
    } else if (strcmp(op->string, "wait") == 0) {
        debug_op_wait(&req, root);
    } else if (strcmp(op->string, "screenshot") == 0) {
        debug_op_screenshot(&req, root);
    } else if (strcmp(op->string, "hash") == 0) {
        debug_op_hash(&req);
    } else if (strcmp(op->string, "pace") == 0) {
        debug_op_pace(&req, root);
    } else if (strcmp(op->string, "reset") == 0) {
        debug_op_reset(&req);
    } else if (strcmp(op->string, "quit") == 0) {
        debug_op_quit(&req, root);
    } else {
        debug_error(&req, "unknown op");
    }
    port_json_free(root);
}

/* ---- Polling: accept a client, read what arrived, handle the complete lines (until a deferred op is in flight). */

static void debug_poll(void) {
    if (debug_client_fd < 0) {
        int fd = accept(debug_listen_fd, NULL, NULL);
        if (fd < 0) {
            return;
        }
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
        fcntl(fd, F_SETFD, FD_CLOEXEC);
        debug_client_fd = fd;
        debug_line_n = 0;
        port_log("debug: client connected (frame %ld)", port_frames);
    }
    while (debug_client_fd >= 0 && debug_pending == DEBUG_NONE) {
        char *nl = memchr(debug_line, '\n', debug_line_n);
        ssize_t r;
        if (nl != NULL) {
            size_t n = (size_t)(nl - debug_line), rest = debug_line_n - n - 1;
            if (n > 0 && debug_line[n - 1] == '\r') {
                n--;
            }
            debug_handle(debug_line, n);
            memmove(debug_line, nl + 1, rest);
            debug_line_n = rest;
            continue;
        }
        if (debug_line_n == DEBUG_LINE_MAX) {
            DebugReq none = { 0, 0 };
            debug_error(&none, "request line longer than %d bytes", DEBUG_LINE_MAX);
            debug_line_n = 0; /* the rest of that line is skipped up to its newline below */
        }
        r = read(debug_client_fd, debug_line + debug_line_n, DEBUG_LINE_MAX - debug_line_n);
        if (r > 0) {
            debug_line_n += (size_t)r;
        } else if (r == 0) {
            debug_disconnect("closed");
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else if (errno != EINTR) {
            debug_disconnect(strerror(errno));
        }
    }
}

/* ---- The pump's hooks */

void port_debug_frame(void) {
    debug_pad_frame();
    if (debug_pending == DEBUG_STEP) {
        if (--debug_step_left <= 0) {
            debug_pending = DEBUG_NONE;
            port_pump_pause_request();
            debug_ok(&debug_pending_req, "\"frame\": %ld", port_frames);
        }
    } else if (debug_pending == DEBUG_WAIT) {
        int hit = debug_wait_hit();
        debug_wait.elapsed++;
        if (hit || debug_wait.elapsed >= debug_wait.timeout) {
            debug_pending = DEBUG_NONE;
            port_pump_pause_request();
            debug_ok(&debug_pending_req, "\"frame\": %ld, \"hit\": %d", port_frames, hit);
        }
    }
    debug_poll();
    if (debug_reset_pending) {
        debug_reset();
    }
}

void port_debug_poll_paused(void) {
    debug_poll();
}

void port_debug_resumed(void) {
    if (debug_reset_pending) {
        debug_reset();
    }
}
#endif /* _WIN32 */
