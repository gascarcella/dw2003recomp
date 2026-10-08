/* The platform layer (port/include/platform.h): the runtime's operating-system calls, a POSIX half and a Windows
 * half. The POSIX half is exactly what the runtime did before the Windows build (the same calls, flags and messages,
 * so a Linux run's output is unchanged). The Windows half uses the wide (UTF-16) system calls where it opens or
 * renames files by name, converting from UTF-8 itself, so those work whatever the process's code page; the rest of
 * the runtime's fopen calls rely on the UTF-8 code page the executable's manifest selects (port/windows/). */
#ifndef _WIN32
#define _GNU_SOURCE /* realpath, clock_nanosleep, pread */
#define _FILE_OFFSET_BITS 64 /* the -m32 build: 64-bit offsets into the disc image */
#endif
#include "platform.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "port_runtime.h"

#ifdef _WIN32

/* ======================================================================================================== Windows */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

/* UTF-8 to UTF-16 into a caller's buffer; 0 when it does not fit or is not UTF-8. */
static int win_wide(const char *s, wchar_t *out, int out_len) {
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, out, out_len);
    if (n <= 0) {
        errno = EINVAL;
        return 0;
    }
    return 1;
}

static void win_errno(DWORD err) {
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        errno = ENOENT;
        break;
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
        errno = EACCES;
        break;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        errno = EEXIST;
        break;
    default:
        errno = EIO;
        break;
    }
}

int port_path_is_sep(char c) {
    return c == '/' || c == '\\';
}

int port_path_is_absolute(const char *path) {
    if (port_path_is_sep(path[0])) {
        return 1; /* \x (the current drive) and \\server\share\x */
    }
    return ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':' &&
           port_path_is_sep(path[2]);
}

const char *port_path_last_sep(const char *path) {
    const char *last = NULL;
    for (; *path != '\0'; path++) {
        if (port_path_is_sep(*path)) {
            last = path;
        }
    }
    return last;
}

int port_path_canonical(const char *path, char *out, size_t out_size) {
    wchar_t w[4096], full[4096];
    DWORD n;
    if (!win_wide(path, w, 4096)) {
        return 0;
    }
    n = GetFullPathNameW(w, 4096, full, NULL);
    if (n == 0 || n >= 4096 || GetFileAttributesW(full) == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return WideCharToMultiByte(CP_UTF8, 0, full, -1, out, (int)out_size, NULL, NULL) > 0;
}

int port_cache_dir(char *out, size_t out_size) {
    const char *base = getenv("LOCALAPPDATA");
    if (base == NULL || !port_path_is_absolute(base)) {
        return 0;
    }
    return snprintf(out, out_size, "%s\\" PSXSTACK_GAME_ID "-port", base) < (int)out_size;
}

int port_make_dirs(const char *dir) {
    wchar_t w[4096];
    wchar_t *p = w;
    if (!win_wide(dir, w, 4096)) {
        return -1;
    }
    if (((w[0] >= L'A' && w[0] <= L'Z') || (w[0] >= L'a' && w[0] <= L'z')) && w[1] == L':') {
        p = w + 2; /* the drive */
    } else if (w[0] == L'\\' && w[1] == L'\\') {
        /* \\server\share: the share is not a directory to create */
        int seps = 0;
        for (p = w + 2; *p != L'\0' && seps < 2; p++) {
            seps += *p == L'\\' || *p == L'/';
        }
    }
    for (; *p != L'\0'; p++) {
        if ((*p == L'\\' || *p == L'/') && p != w) {
            *p = L'\0';
            if (_wmkdir(w) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = L'\\';
        }
    }
    if (_wmkdir(w) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

int port_file_replace(const char *tmp, const char *path) {
    wchar_t wt[4096], wp[4096];
    if (!win_wide(tmp, wt, 4096) || !win_wide(path, wp, 4096)) {
        return -1;
    }
    if (!MoveFileExW(wt, wp, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        win_errno(GetLastError());
        return -1;
    }
    return 0;
}

long port_process_id(void) {
    return (long)GetCurrentProcessId();
}

int port_file_open_read(const char *path) {
    wchar_t w[4096];
    if (!win_wide(path, w, 4096)) {
        return -1;
    }
    return _wopen(w, _O_RDONLY | _O_BINARY | _O_NOINHERIT);
}

int port_file_info(int fd, PortFileInfo *info) {
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    BY_HANDLE_FILE_INFORMATION bh;
    unsigned long long t;
    if (h == INVALID_HANDLE_VALUE || !GetFileInformationByHandle(h, &bh)) {
        win_errno(GetLastError());
        return -1;
    }
    info->size = (long long)(((unsigned long long)bh.nFileSizeHigh << 32) | bh.nFileSizeLow);
    info->is_regular = !(bh.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && GetFileType(h) == FILE_TYPE_DISK;
    /* FILETIME: 100 ns units since 1601-01-01; Unix time since 1970-01-01 */
    t = ((unsigned long long)bh.ftLastWriteTime.dwHighDateTime << 32) | bh.ftLastWriteTime.dwLowDateTime;
    t -= 116444736000000000ULL;
    info->mtime_sec = (long long)(t / 10000000ULL);
    info->mtime_nsec = (long)(t % 10000000ULL) * 100;
    info->inode = ((unsigned long long)bh.nFileIndexHigh << 32) | bh.nFileIndexLow;
    info->device = bh.dwVolumeSerialNumber;
    return 0;
}

long long port_file_pread(int fd, void *buf, size_t n, long long offset) {
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    OVERLAPPED ov;
    DWORD got = 0;
    if (h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    memset(&ov, 0, sizeof(ov));
    ov.Offset = (DWORD)(offset & 0xFFFFFFFFu);
    ov.OffsetHigh = (DWORD)((unsigned long long)offset >> 32);
    if (!ReadFile(h, buf, n > 0x7FFFFFFF ? 0x7FFFFFFF : (DWORD)n, &got, &ov)) {
        DWORD err = GetLastError();
        if (err == ERROR_HANDLE_EOF) {
            return 0;
        }
        win_errno(err);
        return -1;
    }
    return (long long)got;
}

void port_file_close(int fd) {
    _close(fd);
}

void port_file_line_buffered(FILE *f) {
    setvbuf(f, NULL, _IONBF, 0); /* UCRT has no line buffering (_IOLBF is _IOFBF there) */
}

static LARGE_INTEGER win_qpc_freq;

long long port_clock_ns(void) {
    LARGE_INTEGER c;
    if (win_qpc_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&win_qpc_freq);
    }
    QueryPerformanceCounter(&c);
    return (c.QuadPart / win_qpc_freq.QuadPart) * 1000000000LL +
           (c.QuadPart % win_qpc_freq.QuadPart) * 1000000000LL / win_qpc_freq.QuadPart;
}

/* A waitable timer, high resolution where Windows has it (10 1803 or newer; the plain one wakes at the scheduler's
 * 15.6 ms granularity); under Wine a 2 ms wait measured 2.009 ms. */
static HANDLE win_timer;

void port_sleep_until_ns(long long when) {
    if (win_timer == NULL) {
        win_timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (win_timer == NULL) {
            win_timer = CreateWaitableTimerW(NULL, FALSE, NULL);
        }
    }
    for (;;) {
        long long left = when - port_clock_ns();
        LARGE_INTEGER due;
        if (left <= 0) {
            return;
        }
        if (win_timer == NULL) {
            Sleep((DWORD)((left + 999999) / 1000000));
            continue;
        }
        due.QuadPart = -(left + 99) / 100; /* relative, 100 ns units */
        if (!SetWaitableTimer(win_timer, &due, 0, NULL, NULL, FALSE)) {
            Sleep(1);
            continue;
        }
        WaitForSingleObject(win_timer, INFINITE);
    }
}

void port_sleep_ms(int ms) {
    Sleep((DWORD)ms);
}

/* The watchdog: a thread that compares the clock with the deadline the kicks move forward. As SIGALRM's handler
 * does on POSIX, it writes the crash report (the main thread's registers and stack, crash.c's
 * port_crash_watchdog_fire) and exits 4. */
static volatile LONG64 win_watchdog_deadline_ms;
static int win_watchdog_sec;

static DWORD WINAPI win_watchdog_thread(LPVOID arg) {
    (void)arg;
    for (;;) {
        Sleep(250);
        if ((LONG64)GetTickCount64() > win_watchdog_deadline_ms) {
            port_crash_watchdog_fire();
        }
    }
    return 0;
}

void port_watchdog_start(int sec) {
    HANDLE t;
    port_crash_watchdog_install(); /* the main thread's handle, for the report */
    win_watchdog_sec = sec;
    win_watchdog_deadline_ms = (LONG64)GetTickCount64() + (LONG64)sec * 1000;
    t = CreateThread(NULL, 0, win_watchdog_thread, NULL, 0, NULL);
    if (t == NULL) {
        port_fatal("watchdog: CreateThread failed");
    }
    CloseHandle(t);
}

void port_watchdog_kick(void) {
    if (win_watchdog_sec > 0) {
        win_watchdog_deadline_ms = (LONG64)GetTickCount64() + (LONG64)win_watchdog_sec * 1000;
    }
}

#else

/* ========================================================================================================== POSIX */
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int port_path_is_sep(char c) {
    return c == '/';
}

int port_path_is_absolute(const char *path) {
    return path[0] == '/';
}

const char *port_path_last_sep(const char *path) {
    return strrchr(path, '/');
}

int port_path_canonical(const char *path, char *out, size_t out_size) {
    char canon[PATH_MAX];
    if (realpath(path, canon) == NULL || strlen(canon) >= out_size) {
        return 0;
    }
    strcpy(out, canon);
    return 1;
}

int port_cache_dir(char *out, size_t out_size) {
    const char *xdg = getenv("XDG_CACHE_HOME");
    const char *home = getenv("HOME");
    int n;
    if (xdg != NULL && xdg[0] == '/') {
        n = snprintf(out, out_size, "%s/" PSXSTACK_GAME_ID "-port", xdg);
    } else if (home != NULL && home[0] == '/') {
        n = snprintf(out, out_size, "%s/.cache/" PSXSTACK_GAME_ID "-port", home);
    } else {
        return 0;
    }
    return n < (int)out_size;
}

int port_make_dirs(const char *dir) {
    char buf[PATH_MAX];
    char *p;
    if (strlen(dir) >= sizeof(buf)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(buf, dir);
    for (p = buf + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(buf, 0755);
            *p = '/';
        }
    }
    if (mkdir(buf, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

int port_file_replace(const char *tmp, const char *path) {
    return rename(tmp, path);
}

long port_process_id(void) {
    return (long)getpid();
}

int port_file_open_read(const char *path) {
    return open(path, O_RDONLY | O_CLOEXEC);
}

int port_file_info(int fd, PortFileInfo *info) {
    struct stat st;
    if (fstat(fd, &st) != 0) {
        return -1;
    }
    info->size = (long long)st.st_size;
    info->is_regular = S_ISREG(st.st_mode);
    info->mtime_sec = (long long)st.st_mtim.tv_sec;
    info->mtime_nsec = (long)st.st_mtim.tv_nsec;
    info->inode = (unsigned long long)st.st_ino;
    info->device = (unsigned long long)st.st_dev;
    return 0;
}

long long port_file_pread(int fd, void *buf, size_t n, long long offset) {
    for (;;) {
        ssize_t r = pread(fd, buf, n, (off_t)offset);
        if (r < 0 && errno == EINTR) {
            continue;
        }
        return (long long)r;
    }
}

void port_file_close(int fd) {
    close(fd);
}

void port_file_line_buffered(FILE *f) {
    setvbuf(f, NULL, _IOLBF, 0);
}

long long port_clock_ns(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000000000LL + now.tv_nsec;
}

void port_sleep_until_ns(long long when) {
    struct timespec due;
    due.tv_sec = (time_t)(when / 1000000000LL);
    due.tv_nsec = (long)(when % 1000000000LL);
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &due, NULL) == EINTR) {
        /* the watchdog's SIGALRM, or another signal: wait on */
    }
}

void port_sleep_ms(int ms) {
    struct timespec tick;
    tick.tv_sec = ms / 1000;
    tick.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&tick, NULL);
}

static int posix_watchdog_sec;

void port_watchdog_start(int sec) {
    posix_watchdog_sec = sec;
    port_crash_watchdog_install(); /* SIGALRM writes the crash report (the registers show where it spins), exits 4 */
    alarm((unsigned)sec);
}

void port_watchdog_kick(void) {
    if (posix_watchdog_sec > 0) {
        alarm((unsigned)posix_watchdog_sec);
    }
}

#endif
