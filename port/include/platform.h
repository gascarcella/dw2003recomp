/* The platform layer (port/runtime/platform.c): the few places where the runtime needs the operating system beyond the
 * C library, each with a POSIX half and a Windows half (the project board's Windows 4; DECISIONS "Windows:
 * cross-built from Linux"). Everything else in port/runtime/ is plain C99 and calls these. Paths are UTF-8 narrow strings
 * on both (the Windows build's manifest selects the UTF-8 code page: port/windows/dw2003.manifest). */
#ifndef PORT_PLATFORM_H
#define PORT_PLATFORM_H

#include <stddef.h>
#include <stdio.h>

/* ---- Paths. A separator is '/' on both; the Windows half also takes '\'. */
int port_path_is_sep(char c);
int port_path_is_absolute(const char *path);       /* "/x" (both); "C:\x", "C:/x", "\\server\x" (Windows) */
const char *port_path_last_sep(const char *path);  /* the last separator, or NULL */
/* The canonical absolute path of an existing file (realpath / _fullpath) into out; 0 when it cannot be resolved. */
int port_path_canonical(const char *path, char *out, size_t out_size);
/* The per-user cache directory for this program ($XDG_CACHE_HOME or ~/.cache, then /<game id>-port; Windows:
 * %LOCALAPPDATA%\<game id>-port; PSXSTACK_GAME_ID) into out; 0 when there is none. Not created. */
int port_cache_dir(char *out, size_t out_size);
/* Creates `dir` and the missing directories above it; 0 when done (or it existed), -1 with errno set. */
int port_make_dirs(const char *dir);
/* Renames `tmp` onto `path`, replacing an existing file (rename / MoveFileExW); 0 or -1 with errno set. */
int port_file_replace(const char *tmp, const char *path);
long port_process_id(void);

/* ---- Files read by position (the disc image): a descriptor opened read-only in binary mode, not inherited. */
typedef struct PortFileInfo {
    long long size;
    int is_regular;
    long long mtime_sec; /* the last write, with nanoseconds where the system keeps them (100 ns on Windows) */
    long mtime_nsec;
    unsigned long long inode, device; /* the file's identity on its volume (the file index and the volume serial) */
} PortFileInfo;
int port_file_open_read(const char *path);               /* -1 with errno set */
int port_file_info(int fd, PortFileInfo *info);          /* 0 or -1 with errno set */
long long port_file_pread(int fd, void *buf, size_t n, long long offset); /* bytes read (0 at the end), -1 */
void port_file_close(int fd);

/* ---- stdio. Line buffering where the system has it (POSIX: _IOLBF); where it does not (UCRT treats _IOLBF as full
 * buffering), unbuffered, so that a crash loses no line of the log. */
void port_file_line_buffered(FILE *f);

/* ---- Time: a monotonic clock in nanoseconds, and sleeps on it. */
long long port_clock_ns(void);
void port_sleep_until_ns(long long when);  /* an absolute time of port_clock_ns; a signal never cuts it short */
void port_sleep_ms(int ms);

/* ---- The watchdog (pump.c): the run ends with status 4 after `sec` seconds without a kick. POSIX: SIGALRM, whose
 * handler is the crash report's (port_crash_watchdog_install); Windows: a thread that watches a deadline and calls
 * the report's port_crash_watchdog_fire (the main thread's registers and stack). port_watchdog_kick re-arms it. */
void port_watchdog_start(int sec);
void port_watchdog_kick(void);

#endif /* PORT_PLATFORM_H */
