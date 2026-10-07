/* The crash report (docs/PORT.md "Crash report"): when the game dies (SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT), or
 * stops on a fatal error, a halt, an unimplemented part or the watchdog, one text file says what a tester cannot
 * tell us otherwise: the build, the vsync, the loaded overlays, the stage and map, the pad, the last lines of the
 * port's log, the fault's registers and the stack as addresses relative to the executable (scripts/symbolize.py
 * names them with the build's debug info). The launcher shows the file and puts it on the clipboard with its Copy.
 *
 * Everything the signal handler does is async-signal-safe: no malloc, no stdio, no locale; the text is formatted
 * into a static buffer by the small writers below and written with write(2). backtrace() is primed at init (its
 * first call may load libgcc_s). The handler runs on its own stack (sigaltstack: a stack overflow still reports) and
 * is one-shot (SA_RESETHAND): after the report the signal's default action ends the process, so the exit status the
 * launcher reads stays "killed by signal N", as without the handler.
 *
 * The file goes to --crash-dir (the launcher passes <settings dir>/crashes/), else the current directory, as
 * crash-<YYYYMMDD-HHMMSS>.txt (UTC; the time is computed here, not by localtime: that is not signal-safe). The last
 * line on stderr names it: `port: crash report: PATH`.
 *
 * DW3_PORT_CRASH_AT=VSYNC (tests/port/crash.py): a NULL write at that vsync, from port_crash_test_write, so that a
 * report's top frame symbolizes to a known function. */
#define _GNU_SOURCE
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <link.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"

#define CRASH_LOG_LINES 64
#define CRASH_LOG_LINE 200
#define CRASH_FRAMES 64
#define CRASH_TEXT (16 * 1024)
#define CRASH_PATH 1024

static char crash_dir[CRASH_PATH];
static char crash_platform[256];
static char crash_text[CRASH_TEXT];
static size_t crash_len;
static char crash_path[CRASH_PATH + 64];
static long crash_test_at = -1;
static volatile sig_atomic_t crash_reporting;

/* The main executable's load address (0 for a non-PIE binary) and its PT_LOAD range: a stack address inside it is
 * reported as exe+offset, which addr2line resolves against the unstripped build. */
static uintptr_t crash_exe_base, crash_exe_lo, crash_exe_hi;

/* The last lines of port_log, oldest first from crash_log_next. */
static char crash_log[CRASH_LOG_LINES][CRASH_LOG_LINE];
static int crash_log_next, crash_log_count;

/* ---- the async-signal-safe writers (into crash_text) ---- */

static void cw_str(const char *s) {
    size_t n = strlen(s);
    if (crash_len + n >= sizeof(crash_text)) {
        n = sizeof(crash_text) - 1 - crash_len;
    }
    memcpy(crash_text + crash_len, s, n);
    crash_len += n;
    crash_text[crash_len] = '\0';
}

static void cw_dec(long long v) {
    char buf[24];
    int i = sizeof(buf);
    unsigned long long u = v < 0 ? (unsigned long long)(-(v + 1)) + 1 : (unsigned long long)v;
    buf[--i] = '\0';
    do {
        buf[--i] = (char)('0' + u % 10);
        u /= 10;
    } while (u != 0);
    if (v < 0) {
        buf[--i] = '-';
    }
    cw_str(buf + i);
}

static void cw_hex(unsigned long long v, int digits) {
    char buf[24];
    int i = sizeof(buf);
    buf[--i] = '\0';
    do {
        buf[--i] = "0123456789abcdef"[v & 15];
        v >>= 4;
        digits--;
    } while (v != 0 || digits > 0);
    cw_str("0x");
    cw_str(buf + i);
}

/* A code address: exe+0x... when inside the executable, else the raw address with "?". */
static void cw_addr(uintptr_t a) {
    if (a >= crash_exe_lo && a < crash_exe_hi) {
        cw_str("exe+");
        cw_hex(a - crash_exe_base, 1);
    } else {
        cw_hex(a, 1);
        cw_str(" ?");
    }
}

/* The civil date of a Unix time (UTC), without localtime (Howard Hinnant's days-to-civil). */
static void crash_stamp(time_t t, char out[16]) {
    long long days = (long long)t / 86400, secs = (long long)t % 86400;
    long long z = days + 719468, era = (z >= 0 ? z : z - 146096) / 146097, doe = z - era * 146097;
    long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365, y = yoe + era * 400;
    long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
    long long d = doy - (153 * mp + 2) / 5 + 1, m = mp < 10 ? mp + 3 : mp - 9;
    int v[6], i;
    if (m <= 2) {
        y++;
    }
    v[0] = (int)y, v[1] = (int)m, v[2] = (int)d;
    v[3] = (int)(secs / 3600), v[4] = (int)(secs / 60 % 60), v[5] = (int)(secs % 60);
    for (i = 0; i < 6; i++) {
        int w = i == 0 ? 4 : 2, n = v[i];
        char *p = out + (i == 0 ? 0 : 4 + (i - 1) * 2 + (i >= 3 ? 1 : 0));
        while (w-- > 0) {
            p[w] = (char)('0' + n % 10);
            n /= 10;
        }
    }
    out[8] = '-';
    out[15] = '\0';
}

/* ---- the report ---- */

static const char *crash_signal_name(int sig) {
    switch (sig) {
    case SIGSEGV: return "SIGSEGV";
    case SIGBUS: return "SIGBUS";
    case SIGFPE: return "SIGFPE";
    case SIGILL: return "SIGILL";
    case SIGABRT: return "SIGABRT";
    case SIGALRM: return "SIGALRM";
    default: return "signal";
    }
}

static void crash_write_context(const char *kind, int status) {
    const PortOverlay *o1 = port_overlay_current(1), *o2 = port_overlay_current(2);
    int i, n;
    cw_str("dw2003 crash report\n");
    cw_str("kind: ");
    cw_str(kind);
    cw_str("\nstatus: ");
    cw_dec(status);
    cw_str("\nbuild: ");
    cw_str(port_version);
    cw_str(" (");
    cw_str(port_commit);
    cw_str(")\nplatform: ");
    cw_str(crash_platform);
    cw_str("\nvsync: ");
    cw_dec(port_frames);
    cw_str("\nrate: ");
    cw_dec(port_rate);
    cw_str(" Hz\noverlay tier 1: ");
    cw_str(o1 != NULL ? o1->name : "(none)");
    cw_str("\noverlay tier 2: ");
    cw_str(o2 != NULL ? o2->name : "(none)");
    cw_str("\nstage: ");
    cw_dec(port_state_stage());
    cw_str("\nfile: ");
    cw_hex((u32)port_state_file(), 1);
    cw_str("\nmap: ");
    cw_dec(port_state_map());
    cw_str("\npad: ");
    cw_hex(psyq_pad_get(0), 4);
    cw_str("\nscript: ");
    cw_str(port_script_active ? "yes" : "no");
    cw_str("\n\nlog (last ");
    n = crash_log_count;
    cw_dec(n);
    cw_str(" lines):\n");
    for (i = 0; i < n; i++) {
        int k = (crash_log_next - n + i + CRASH_LOG_LINES) % CRASH_LOG_LINES;
        cw_str("  ");
        cw_str(crash_log[k]);
        cw_str("\n");
    }
}

static void crash_write_registers(const ucontext_t *uc) {
#if defined(__x86_64__)
    cw_str("pc: ");
    cw_addr((uintptr_t)uc->uc_mcontext.gregs[REG_RIP]);
    cw_str("\nsp: ");
    cw_hex((unsigned long long)uc->uc_mcontext.gregs[REG_RSP], 1);
    cw_str("\n");
#elif defined(__aarch64__)
    cw_str("pc: ");
    cw_addr((uintptr_t)uc->uc_mcontext.pc);
    cw_str("\nsp: ");
    cw_hex((unsigned long long)uc->uc_mcontext.sp, 1);
    cw_str("\n");
#elif defined(__i386__)
    cw_str("pc: ");
    cw_addr((uintptr_t)uc->uc_mcontext.gregs[REG_EIP]);
    cw_str("\nsp: ");
    cw_hex((unsigned long long)uc->uc_mcontext.gregs[REG_ESP], 1);
    cw_str("\n");
#else
    (void)uc;
    cw_str("pc: (no registers on this architecture)\n");
#endif
}

static void crash_write_stack(void) {
    void *frames[CRASH_FRAMES];
    int n = backtrace(frames, CRASH_FRAMES), i;
    cw_str("\nstack (return addresses; frame 0 is the reporter):\n");
    for (i = 0; i < n; i++) {
        cw_str("  #");
        cw_dec(i);
        cw_str(" ");
        cw_addr((uintptr_t)frames[i]);
        cw_str("\n");
    }
}

/* Writes crash_text to a new file in crash_dir, names it on stderr. */
static void crash_write_file(void) {
    char stamp[16];
    int fd;
    size_t n = strlen(crash_dir), off = 0;
    crash_stamp(time(NULL), stamp);
    memcpy(crash_path, crash_dir, n);
    if (n > 0 && crash_path[n - 1] != '/') {
        crash_path[n++] = '/';
    }
    memcpy(crash_path + n, "crash-", 6);
    memcpy(crash_path + n + 6, stamp, 15);
    memcpy(crash_path + n + 21, ".txt", 5);
    fd = open(crash_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) {
        static const char msg[] = "port: crash report: could not be written\n";
        if (write(2, msg, sizeof(msg) - 1) < 0) {
            /* nothing to do */
        }
        return;
    }
    while (off < crash_len) {
        ssize_t w = write(fd, crash_text + off, crash_len - off);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        off += (size_t)w;
    }
    close(fd);
    {
        static const char head[] = "port: crash report: ";
        if (write(2, head, sizeof(head) - 1) < 0 || write(2, crash_path, strlen(crash_path)) < 0 ||
            write(2, "\n", 1) < 0) {
            /* nothing to do */
        }
    }
}

static void crash_handler(int sig, siginfo_t *info, void *ctx) {
    if (crash_reporting) {
        return; /* a second fault while reporting: the default action takes over (SA_RESETHAND) */
    }
    crash_reporting = 1;
    crash_len = 0;
    crash_text[0] = '\0';
    crash_write_context(sig == SIGALRM ? "watchdog" : "crash", sig == SIGALRM ? 4 : -sig);
    cw_str("\nsignal: ");
    cw_str(crash_signal_name(sig));
    cw_str(" (");
    cw_dec(sig);
    cw_str(")\nfault address: ");
    cw_hex((unsigned long long)(uintptr_t)(info != NULL ? info->si_addr : NULL), 1);
    cw_str("\n");
    crash_write_registers((const ucontext_t *)ctx);
    crash_write_stack();
    crash_write_file();
    if (sig == SIGALRM) {
        static const char msg[] = "port: watchdog: no port_wait() for the watchdog's time: the game spins in a loop "
                                  "without a PLATFORM_WAIT hook (cdload_load_file?); exiting 4\n";
        if (write(2, msg, sizeof(msg) - 1) < 0) {
            /* nothing to do */
        }
        _exit(4);
    }
    /* SA_RESETHAND restored the default action and SA_NODEFER left the signal unblocked: raising it again ends the
     * process with that signal (a fault would re-execute and die too; a signal sent from outside would not), so the
     * exit status is the signal's, as without the handler. */
    raise(sig);
}

static int crash_phdr(struct dl_phdr_info *info, size_t size, void *data) {
    int i;
    (void)size;
    (void)data;
    /* the first entry is the main executable */
    crash_exe_base = info->dlpi_addr;
    crash_exe_lo = (uintptr_t)-1;
    crash_exe_hi = 0;
    for (i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *p = &info->dlpi_phdr[i];
        if (p->p_type == PT_LOAD && (p->p_flags & PF_X)) {
            uintptr_t lo = info->dlpi_addr + p->p_vaddr, hi = lo + p->p_memsz;
            if (lo < crash_exe_lo) {
                crash_exe_lo = lo;
            }
            if (hi > crash_exe_hi) {
                crash_exe_hi = hi;
            }
        }
    }
    return 1;
}

void port_crash_init(const char *dir) {
    static const int sigs[] = { SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT };
    static char alt_stack[64 * 1024];
    struct utsname u;
    stack_t ss;
    struct sigaction sa;
    void *frames[4];
    size_t i;
    const char *env;

    if (dir == NULL || *dir == '\0') {
        dir = ".";
    }
    if (strlen(dir) >= sizeof(crash_dir)) {
        port_fatal("--crash-dir: the path is too long");
    }
    strcpy(crash_dir, dir);
    if (mkdir(crash_dir, 0755) != 0 && errno != EEXIST) {
        port_fatal("--crash-dir %s: cannot create: %s", crash_dir, strerror(errno));
    }
    if (uname(&u) == 0) {
        snprintf(crash_platform, sizeof(crash_platform), "%s %s %s", u.sysname, u.release, u.machine);
    } else {
        strcpy(crash_platform, "unknown");
    }
    dl_iterate_phdr(crash_phdr, NULL);
    backtrace(frames, 4); /* primes libgcc's unwinder: the handler's call then loads nothing */

    ss.ss_sp = alt_stack;
    ss.ss_size = sizeof(alt_stack);
    ss.ss_flags = 0;
    if (sigaltstack(&ss, NULL) != 0) {
        port_fatal("crash: sigaltstack: %s", strerror(errno));
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    for (i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++) {
        sigaction(sigs[i], &sa, NULL);
    }
    env = getenv("DW3_PORT_CRASH_AT");
    if (env != NULL && *env != '\0') {
        crash_test_at = strtol(env, NULL, 10);
    }
}

void port_crash_watchdog_install(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
}

void port_crash_log_line(const char *line) {
    size_t n = strlen(line);
    if (n >= CRASH_LOG_LINE) {
        n = CRASH_LOG_LINE - 1;
    }
    memcpy(crash_log[crash_log_next], line, n);
    crash_log[crash_log_next][n] = '\0';
    crash_log_next = (crash_log_next + 1) % CRASH_LOG_LINES;
    if (crash_log_count < CRASH_LOG_LINES) {
        crash_log_count++;
    }
}

void port_crash_report(const char *kind, int status, const char *detail) {
    if (crash_reporting) {
        return;
    }
    crash_reporting = 1;
    crash_len = 0;
    crash_text[0] = '\0';
    crash_write_context(kind, status);
    cw_str("\nreason: ");
    cw_str(detail != NULL ? detail : "");
    cw_str("\n");
    crash_write_stack();
    crash_write_file();
    crash_reporting = 0;
}

const char *port_crash_last_path(void) {
    return crash_path[0] != '\0' ? crash_path : NULL;
}

/* The test hook's fault, in a function of its own so that the report's pc names it. */
__attribute__((noinline)) void port_crash_test_write(long frame) {
    volatile int *null_pointer = NULL;
    port_log("crash test: DW3_PORT_CRASH_AT=%ld: writing through a NULL pointer", frame);
    *null_pointer = (int)frame;
}

void port_crash_frame(long frame) {
    if (crash_test_at >= 0 && frame == crash_test_at) {
        port_crash_test_write(frame);
    }
}
