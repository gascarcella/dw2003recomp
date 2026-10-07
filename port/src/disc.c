/* LIBCD's sector source over the user's BIN/CUE (port_harness.h; docs/PORT.md "Disc, memory cards and movies").
 *
 * port_disc_open takes the .cue (its first FILE line names the BIN, relative to the cue's directory; the disc is one
 * track, MODE2/2352, INDEX 01 00:00:00) or the .bin itself. Sector `lba` is the 2352 bytes at lba * 2352 in the BIN:
 * LBA 0 is the BIN's first sector, as CdIntToPos counts (it adds the 150-sector lead-in). The reader (a positional
 * read on a file descriptor kept open for the run; port/src/platform.c) is handed to the shim with
 * psyq_cd_set_reader; a sector past the BIN's end reads as missing (0).
 *
 * The SHA-1 check is the matching build's (scripts/setup.sh DISC_SHA1): the whole BIN must hash to the unpatched EU
 * disc's 457cb233..., else the run stops with status 1, naming both digests. --no-disc-check (check_sha1 0) skips it
 * and says so. Hashing the 692 MB costs ~4.5 s of CPU (~25 s from a cold page cache), so a successful check leaves a
 * stamp: one line "<sha1> <size> <mtime s>.<ns> <inode> <device> <canonical path>" per verified BIN in
 * $XDG_CACHE_HOME/dw2003-port/disc-stamps (~/.cache/dw2003-port/ without XDG_CACHE_HOME; Windows:
 * %LOCALAPPDATA%\dw2003-port\; port_cache_dir), outside the repository.
 * A run whose BIN has the same canonical path, size, mtime (with nanoseconds), inode and device skips the hash;
 * anything else hashes again (and a failed check never writes a stamp). The log line is the same either way, so a
 * run's output does not depend on the cache. An unwritable cache directory only costs the next run its shortcut.
 *
 * port_disc_set_speed selects LIBCD's timing model (psyq_cd_set_timing; port/psyq/libcd.c): "realistic" (the
 * default) or "instant". */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "port_harness.h"
#include "port_runtime.h"
#include "psyq.h"
#include "sha1.h"

#define DISC_PATH_MAX 4096

#define DISC_SHA1 "457cb233349ba841e03b33d8060f8fbcadd45cb3"
#define DISC_RAW_SECTOR 2352

static int disc_fd = -1;
static unsigned disc_sectors;

static int disc_read(unsigned lba, u8 *sector) {
    long long n;

    if (lba >= disc_sectors) {
        return 0;
    }
    n = port_file_pread(disc_fd, sector, DISC_RAW_SECTOR, (long long)lba * DISC_RAW_SECTOR);
    return n == DISC_RAW_SECTOR;
}

/* ---- the .cue ---- */

/* The BIN the cue's first FILE line names, resolved against the cue's directory, into `out`. */
static void disc_parse_cue(const char *cue, char *out, size_t out_size) {
    FILE *f = fopen(cue, "rb");
    char line[1024];
    int found = 0, track_ok = 0, tracks = 0;

    if (f == NULL) {
        port_fatal("disc: cannot open %s: %s", cue, strerror(errno));
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char *p = line, *name, *end;

        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (strncmp(p, "TRACK", 5) == 0) {
            tracks++;
            if (strstr(p, "MODE2/2352") != NULL) {
                track_ok = 1;
            }
            continue;
        }
        if (found || strncmp(p, "FILE", 4) != 0 || (p[4] != ' ' && p[4] != '\t')) {
            continue;
        }
        p += 5;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '"') {
            name = p + 1;
            end = strchr(name, '"');
        } else {
            name = p;
            end = strpbrk(name, " \t\r\n");
        }
        if (end == NULL || end == name) {
            port_fatal("disc: %s: cannot read the FILE line: %s", cue, line);
        }
        *end = '\0';
        if (port_path_is_absolute(name)) {
            snprintf(out, out_size, "%s", name);
        } else {
            const char *slash = port_path_last_sep(cue);
            int dir_len = slash != NULL ? (int)(slash - cue) + 1 : 0;

            snprintf(out, out_size, "%.*s%s", dir_len, cue, name);
        }
        found = 1;
    }
    fclose(f);
    if (!found) {
        port_fatal("disc: %s: no FILE line", cue);
    }
    if (tracks != 1 || !track_ok) {
        port_log("disc: warning: %s: expected one MODE2/2352 track (the EU disc), found %d track(s)%s", cue, tracks,
                 track_ok ? "" : " and no MODE2/2352");
    }
}

/* ---- the verification stamp ---- */

/* The stamp file's path into `out`; with `make_dir`, creates its directory (and the missing ones above it). */
static int disc_stamp_path(char *out, size_t out_size, int make_dir) {
    char dir[DISC_PATH_MAX];

    if (!port_cache_dir(dir, sizeof(dir))) {
        return 0;
    }
    if (make_dir && port_make_dirs(dir) != 0) {
        return 0;
    }
    return snprintf(out, out_size, "%s/disc-stamps", dir) < (int)out_size;
}

/* The stamp line for the open BIN: everything but the digest (the key). */
static void disc_stamp_key(const PortFileInfo *st, const char *canon, char *out, size_t out_size) {
    snprintf(out, out_size, "%lld %lld.%09ld %llu %llu %s", st->size, st->mtime_sec, st->mtime_nsec, st->inode,
             st->device, canon);
}

/* 1 if the stamp file holds `key` with the expected digest. */
static int disc_stamp_hit(const char *key) {
    char path[DISC_PATH_MAX], line[DISC_PATH_MAX + 128];
    FILE *f;
    int hit = 0;

    if (!disc_stamp_path(path, sizeof(path), 0) || (f = fopen(path, "rb")) == NULL) {
        return 0;
    }
    while (!hit && fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        hit = strncmp(line, DISC_SHA1 " ", 41) == 0 && strcmp(line + 41, key) == 0;
    }
    fclose(f);
    return hit;
}

/* The canonical path in a key or a stamp line: what follows its first `fields` space-separated fields. */
static const char *disc_stamp_canon(const char *s, int fields) {
    while (fields-- > 0 && s != NULL) {
        s = strchr(s, ' ');
        s = s != NULL ? s + 1 : NULL;
    }
    return s;
}

/* Adds `key` to the stamp file (keeping the other BINs' lines, dropping an older one for the same path), through
 * a temporary file and a rename. */
static void disc_stamp_write(const char *key) {
    char path[DISC_PATH_MAX], tmp[DISC_PATH_MAX + 16], line[DISC_PATH_MAX + 128];
    const char *canon = disc_stamp_canon(key, 4);
    FILE *in, *out;

    if (!disc_stamp_path(path, sizeof(path), 1)) {
        return;
    }
    snprintf(tmp, sizeof(tmp), "%s.%ld", path, port_process_id());
    if ((out = fopen(tmp, "wb")) == NULL) {
        return;
    }
    if ((in = fopen(path, "rb")) != NULL) {
        while (fgets(line, sizeof(line), in) != NULL) {
            const char *other;

            line[strcspn(line, "\n")] = '\0';
            other = disc_stamp_canon(line, 5);
            if (other != NULL && canon != NULL && strcmp(other, canon) == 0) {
                continue;
            }
            fprintf(out, "%s\n", line);
        }
        fclose(in);
    }
    fprintf(out, "%s %s\n", DISC_SHA1, key);
    if (fclose(out) != 0 || port_file_replace(tmp, path) != 0) {
        remove(tmp);
    }
}

/* ---- the check ---- */

static void disc_check_sha1(const char *bin, const PortFileInfo *st) {
    static u8 buf[DISC_RAW_SECTOR * 256];
    char canon[DISC_PATH_MAX], key[DISC_PATH_MAX + 96], hex[41];
    uint8_t digest[20];
    PortSha1 c;
    long long pos = 0;

    if (!port_path_canonical(bin, canon, sizeof(canon))) {
        snprintf(canon, sizeof(canon), "%s", bin);
    }
    disc_stamp_key(st, canon, key, sizeof(key));
    if (disc_stamp_hit(key)) {
        return;
    }
    port_sha1_init(&c);
    for (;;) {
        long long n = port_file_pread(disc_fd, buf, sizeof(buf), pos);

        if (n < 0) {
            port_fatal("disc: reading %s: %s", bin, strerror(errno));
        }
        if (n == 0) {
            break;
        }
        port_sha1_update(&c, buf, (size_t)n);
        pos += n;
    }
    port_sha1_final(&c, digest);
    port_sha1_hex(digest, hex);
    if (strcmp(hex, DISC_SHA1) != 0) {
        port_fatal("disc: %s is not the unpatched EU disc (SLES-03936): SHA-1 %s, expected %s "
                   "(--no-disc-check runs it anyway)",
                   bin, hex, DISC_SHA1);
    }
    disc_stamp_write(key);
}

/* `s` ends with `suffix`, letters compared without case (".cue", ".CUE"). */
static int disc_ends_with_nocase(const char *s, const char *suffix) {
    size_t n = strlen(s), m = strlen(suffix), i;
    if (n < m) {
        return 0;
    }
    for (i = 0; i < m; i++) {
        char a = s[n - m + i], b = suffix[i];
        if (a >= 'A' && a <= 'Z') {
            a = (char)(a - 'A' + 'a');
        }
        if (a != b) {
            return 0;
        }
    }
    return 1;
}

void port_disc_open(const char *path, int check_sha1) {
    char bin[DISC_PATH_MAX];
    PortFileInfo st;

    if (disc_ends_with_nocase(path, ".cue")) {
        disc_parse_cue(path, bin, sizeof(bin));
    } else {
        snprintf(bin, sizeof(bin), "%s", path);
    }
    disc_fd = port_file_open_read(bin);
    if (disc_fd < 0) {
        port_fatal("disc: cannot open %s: %s", bin, strerror(errno));
    }
    if (port_file_info(disc_fd, &st) != 0 || !st.is_regular) {
        port_fatal("disc: %s is not a regular file", bin);
    }
    if (st.size % DISC_RAW_SECTOR != 0) {
        port_log("disc: warning: %s: %lld bytes is not a whole number of 2352-byte sectors", bin, st.size);
    }
    disc_sectors = (unsigned)(st.size / DISC_RAW_SECTOR);
    if (check_sha1) {
        disc_check_sha1(bin, &st);
        port_log("disc: %s: %u sectors, SHA-1 %s (the EU disc)", bin, disc_sectors, DISC_SHA1);
    } else {
        port_log("disc: %s: %u sectors, SHA-1 not checked (--no-disc-check)", bin, disc_sectors);
    }
    psyq_cd_set_reader(disc_read);
}

int port_disc_set_speed(const char *speed) {
    if (strcmp(speed, "realistic") == 0) {
        psyq_cd_set_timing(PSYQ_CD_REALISTIC);
        return 1;
    }
    if (strcmp(speed, "instant") == 0) {
        psyq_cd_set_timing(PSYQ_CD_INSTANT);
        return 1;
    }
    return 0;
}
