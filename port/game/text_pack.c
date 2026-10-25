/* The text pack (docs/PORT.md "Text packs"; written by tools/text_pack.py): translated entries laid over the English
 * text files (ES*.BIN, JPN ID + 2) as the game loads them. records_language cannot take another value (the text
 * files are JPN ID + language at 7 consecutive IDs, and several of the game's tables have 7 entries), so a
 * translation plays as English: preset_language's portuguese_br sets records_language 2 and opens pt_br.pack here.
 *
 * Every text file reaches the game through cdload's cache (cdload_queue_file / cdload_get_file /
 * cdload_get_subfile_by_id), which allocates a file's sectors and reads them whole; two hooks in src/main/cdload.c
 * make it rebuild a file the pack has entries for:
 *   - port_text_sectors(id): the buffer's size in sectors where cdload computes it (the allocation in
 *     cdload_queue_file, a buffer's end in cdload_free_above): the disc's sector count, or the rebuilt file's when
 *     that is larger. The read itself still reads the disc's sectors.
 *   - port_text_loaded(id, buffer): in cdload_update, when the read has ended and before the entry is marked loaded
 *     (state 2 -> 3, the only way to 3), so the game never sees the English bytes. The file is checked against the
 *     CRC-32 the pack was built against, rebuilt once to measure it (it must be the size the pack records, the size
 *     the buffer was allocated for), then rebuilt into the buffer; any mismatch leaves that file English.
 * The rebuild is tools/text_pack.py's (_rebuild): a table keeps its count and gets its entries in order, each 4-aligned,
 * each the pack's or the disc's (up to and with its first 0); a ?SDMG### container (JPN ID 0x112 and up) keeps its
 * sub-file count and order, its tables rebuilt, any other sub-file copied, each sub-file and the whole 4-aligned.
 * With nothing replaced it gives the disc's file back byte for byte (text_pack.py --selftest checks all 50). */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filetable.h"
#include "heap.h"
#include "port_runtime.h"
#include "psxstack/platform.h"
#include "text_pack.h"

#define TP_HEADER 20      /* "DW3T", u16 version, u16 language, u32 files, u32 entries, u32 blob bytes */
#define TP_FILE_REC 20    /* u16 file ID, u16 disc sectors, u32 disc CRC-32, u32 rebuilt size, u32 first entry, u32 entries */
#define TP_ENTRY_REC 16   /* u16 file ID, u16 table, u16 index, u16 0, u32 blob offset, u32 length (with the final 0) */
#define TP_VERSION 1
#define TP_LANGUAGE 2     /* the records_language whose files the pack covers */
#define TP_PLAIN 0xFFFF   /* the table of an entry in a file that is one table */
#define TP_DMG_FIRST 0x112
#define TP_MAX_REBUILT 0x80000
#define TP_MAX_COUNT 5000
#define TP_MAX_PACK (64 << 20)

typedef struct TpFile {
    u32 id, sectors, crc, size, first, count;
    int refused; /* its English file did not match once: left English (logged once) */
} TpFile;

typedef struct TpEntry {
    u32 table, index, length;
    const u8 *data;
} TpEntry;

static u8 *tp_pack;
static TpFile *tp_files;
static TpEntry *tp_entries;
static u32 tp_file_count;

static u32 tp_u16(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8;
}

static u32 tp_u32(const u8 *p) {
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}

static int tp_english_text(u32 id) {
    return id >= 1 && id <= 0x15E && (id - 1) % 7 == TP_LANGUAGE;
}

static u32 tp_crc32(const u8 *p, u32 n) {
    u32 crc = 0xFFFFFFFFu, k;
    while (n-- != 0) {
        crc ^= *p++;
        for (k = 0; k < 8; k++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
        }
    }
    return ~crc;
}

/* Checks the pack's records (as tools/text_pack.py parse_pack does); NULL when valid, else why not. */
static const char *tp_check(const u8 *d, u32 len) {
    u32 nf, ne, nb, eo, bo, i, j, next = 0;
    if (len < TP_HEADER || memcmp(d, "DW3T", 4) != 0) {
        return "no DW3T header";
    }
    if (tp_u16(d + 4) != TP_VERSION || tp_u16(d + 6) != TP_LANGUAGE) {
        return "another version or language";
    }
    nf = tp_u32(d + 8), ne = tp_u32(d + 12), nb = tp_u32(d + 16);
    if (nf == 0 || nf > 50 || ne > len || nb > len) {
        return "its counts are wrong";
    }
    eo = TP_HEADER + TP_FILE_REC * nf;
    bo = eo + TP_ENTRY_REC * ne;
    if ((unsigned long long)bo + nb != len) {
        return "its size is not the header's";
    }
    tp_files = calloc(nf, sizeof(*tp_files));
    tp_entries = calloc(ne != 0 ? ne : 1, sizeof(*tp_entries));
    if (tp_files == NULL || tp_entries == NULL) {
        return "out of memory";
    }
    for (i = 0; i < nf; i++) {
        const u8 *r = d + TP_HEADER + TP_FILE_REC * i;
        TpFile *f = &tp_files[i];
        f->id = tp_u16(r), f->sectors = tp_u16(r + 2), f->crc = tp_u32(r + 4), f->size = tp_u32(r + 8);
        f->first = tp_u32(r + 12), f->count = tp_u32(r + 16);
        if (!tp_english_text(f->id) || (i != 0 && f->id <= tp_files[i - 1].id)) {
            return "a file record that is no English text file, or not in order";
        }
        if ((u32)filetable_funcs.get_sectors((s32)f->id) != f->sectors) {
            return "a file's sector count is not this game's";
        }
        if (f->first != next || f->count == 0 || f->count > ne - next || f->size == 0 || f->size > TP_MAX_REBUILT) {
            return "a file record's entries or size are wrong";
        }
        next += f->count;
        for (j = f->first; j < f->first + f->count; j++) {
            const u8 *e = d + eo + TP_ENTRY_REC * j;
            TpEntry *x = &tp_entries[j];
            u32 off = tp_u32(e + 8);
            x->table = tp_u16(e + 2), x->index = tp_u16(e + 4), x->length = tp_u32(e + 12);
            if (tp_u16(e) != f->id || tp_u16(e + 6) != 0 ||
                (j != f->first && (x->table < x[-1].table || (x->table == x[-1].table && x->index <= x[-1].index)))) {
                return "an entry record that is not its file's, or not in order";
            }
            if (x->length == 0 || off > nb || x->length > nb - off || d[bo + off + x->length - 1] != 0) {
                return "an entry outside the blob, or not ending in 0";
            }
            x->data = d + bo + off;
        }
    }
    if (next != ne) {
        return "entry records that no file owns";
    }
    tp_file_count = nf;
    return NULL;
}

int port_text_pack_open(const char *name) {
    char path[4096];
    const char *test = getenv("DW3_PORT_TEXT_PACK"), *why;
    FILE *fp;
    long len;
    if (test != NULL && test[0] != '\0') {
        snprintf(path, sizeof(path), "%s", test);
    } else {
        const char *sep;
        if (!port_exe_path(path, sizeof(path)) || (sep = port_path_last_sep(path)) == NULL) {
            port_log("text pack: the binary's directory is unknown: no %s, the game plays in English", name);
            return 0;
        }
        snprintf(path + (sep - path) + 1, sizeof(path) - (size_t)(sep - path) - 1, "mods/preset_language/%s", name);
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        port_log("text pack: %s: %s: the game plays in English", path, strerror(errno));
        return 0;
    }
    if (fseek(fp, 0, SEEK_END) != 0 || (len = ftell(fp)) <= 0 || len > TP_MAX_PACK || fseek(fp, 0, SEEK_SET) != 0 ||
        (tp_pack = malloc((size_t)len)) == NULL || fread(tp_pack, 1, (size_t)len, fp) != (size_t)len) {
        fclose(fp);
        port_log("text pack: %s cannot be read (empty, over 64 MB or a read error): the game plays in English", path);
        free(tp_pack);
        tp_pack = NULL;
        return 0;
    }
    fclose(fp);
    if ((why = tp_check(tp_pack, (u32)len)) != NULL) {
        port_log("text pack: %s is not a valid pack (%s): the game plays in English", path, why);
        free(tp_pack), free(tp_files), free(tp_entries);
        tp_pack = NULL, tp_files = NULL, tp_entries = NULL, tp_file_count = 0;
        return 0;
    }
    port_log("text pack: %s: %u entries in %u English text files, replaced as the game loads them", path,
             (unsigned)(tp_files[tp_file_count - 1].first + tp_files[tp_file_count - 1].count), (unsigned)tp_file_count);
    return 1;
}

static TpFile *tp_find(s32 id) {
    u32 lo = 0, hi = tp_file_count;
    if (id < 0 || !tp_english_text((u32)id)) {
        return NULL;
    }
    while (lo < hi) {
        u32 mid = (lo + hi) / 2;
        if (tp_files[mid].id == (u32)id) {
            return &tp_files[mid];
        }
        if (tp_files[mid].id < (u32)id) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return NULL;
}

s32 port_text_sectors(s32 id) {
    s32 sectors = filetable_funcs.get_sectors(id);
    TpFile *f = tp_find(id);
    if (f != NULL && (s32)((f->size + 2047) >> 11) > sectors) {
        return (s32)((f->size + 2047) >> 11);
    }
    return sectors;
}

/* ---- The rebuild: once with dst NULL to measure, once into the buffer (never past cap). */
typedef struct TpOut {
    const u8 *src; /* the English file as read: its sectors, zeros after its end */
    u32 len;
    u8 *dst;
    u32 cap, pos;
    const TpEntry *next, *end; /* the file's pack entries not used yet, by (table, index) */
} TpOut;

static s32 tp_s32(const TpOut *o, u32 at, int *ok) {
    if ((unsigned long long)at + 4 > o->len) {
        *ok = 0;
        return 0;
    }
    return (s32)tp_u32(o->src + at);
}

static void tp_put(TpOut *o, const u8 *p, u32 n, int *ok) {
    if (o->dst != NULL) {
        if ((unsigned long long)o->pos + n > o->cap) {
            *ok = 0;
            return;
        }
        if (p != NULL) {
            memcpy(o->dst + o->pos, p, n);
        } else {
            memset(o->dst + o->pos, 0, n);
        }
    }
    o->pos += n;
}

static void tp_set(TpOut *o, u32 at, u32 v) {
    if (o->dst != NULL && at + 4 <= o->cap) {
        o->dst[at] = (u8)v, o->dst[at + 1] = (u8)(v >> 8), o->dst[at + 2] = (u8)(v >> 16), o->dst[at + 3] = (u8)(v >> 24);
    }
}

static void tp_pad4(TpOut *o, int *ok) {
    tp_put(o, NULL, (0u - o->pos) & 3, ok);
}

/* The counted string table at `base` (shape 3): its count, or 0 when it is not one (text_pack.py _table_count). */
static s32 tp_table_count(const TpOut *o, u32 base) {
    int ok = 1;
    s32 n;
    if ((unsigned long long)base + 8 > o->len) {
        return 0;
    }
    n = tp_s32(o, base, &ok);
    if (n <= 0 || n >= TP_MAX_COUNT || (unsigned long long)base + 4 + 4 * (u32)n > o->len ||
        tp_s32(o, base + 4, &ok) != 4 + 4 * n) {
        return 0;
    }
    return n;
}

static void tp_table(TpOut *o, u32 base, u32 table, int *ok) {
    s32 n = tp_table_count(o, base), i;
    u32 head = o->pos;
    if (n == 0) {
        *ok = 0;
        return;
    }
    tp_put(o, NULL, 4 + 4 * (u32)n, ok);
    tp_set(o, head, (u32)n);
    for (i = 0; i < n && *ok; i++) {
        tp_pad4(o, ok);
        tp_set(o, head + 4 + 4 * (u32)i, o->pos - head);
        if (o->next < o->end && o->next->table == table && o->next->index == (u32)i) {
            tp_put(o, o->next->data, o->next->length, ok);
            o->next++;
        } else {
            s32 at = (s32)base + tp_s32(o, base + 4 + 4 * (u32)i, ok);
            const u8 *z = at >= 0 && (u32)at < o->len ? memchr(o->src + at, 0, o->len - (u32)at) : NULL;
            if (z == NULL) {
                *ok = 0;
                return;
            }
            tp_put(o, o->src + at, (u32)(z - (o->src + at)) + 1, ok);
        }
    }
    if (o->next < o->end && o->next->table == table) {
        *ok = 0; /* an index past the table's count */
    }
}

/* text_pack.py _rebuild: 1 when the file rebuilt (o->pos its size), 0 when the file or the pack's entries for it do
 * not fit its shape. */
static int tp_rebuild(TpOut *o, u32 id) {
    int ok = 1;
    s32 h, k, n;
    u32 last = 0;
    if (id - TP_LANGUAGE < TP_DMG_FIRST) {
        tp_table(o, 0, TP_PLAIN, &ok);
        return ok && o->next == o->end;
    }
    h = tp_s32(o, 0, &ok);
    n = h > 0 ? h / 4 : 0;
    if (!ok || n <= 0 || n >= TP_MAX_COUNT || 4 * (u32)n > o->len) {
        return 0;
    }
    for (k = 0; k < n; k++) { /* the used offsets inside the file and increasing */
        s32 s = tp_s32(o, 4 * (u32)k, &ok);
        if (s != 0) {
            if (s < 4 * n || (u32)s >= o->len || (u32)s <= last) {
                return 0;
            }
            last = (u32)s;
        }
    }
    tp_put(o, NULL, 4 * (u32)n, &ok);
    for (k = 0; k < n && ok; k++) {
        u32 s = (u32)tp_s32(o, 4 * (u32)k, &ok);
        if (s == 0) {
            if (o->next < o->end && o->next->table == (u32)k) {
                return 0; /* an entry in an empty slot */
            }
            continue;
        }
        if (o->next < o->end && o->next->table < (u32)k) {
            return 0; /* an entry in a sub-file before this one that was no table */
        }
        tp_pad4(o, &ok);
        tp_set(o, 4 * (u32)k, o->pos);
        if (tp_table_count(o, s) != 0) {
            tp_table(o, s, (u32)k, &ok);
        } else {
            u32 to = o->len;
            s32 j;
            for (j = k + 1; j < n; j++) {
                s32 t = tp_s32(o, 4 * (u32)j, &ok);
                if (t != 0) {
                    to = (u32)t;
                    break;
                }
            }
            if (o->next < o->end && o->next->table == (u32)k) {
                return 0;
            }
            tp_put(o, o->src + s, to - s, &ok);
        }
    }
    tp_pad4(o, &ok);
    return ok && o->next == o->end;
}

void port_text_loaded(s32 id, void *buffer) {
    TpFile *f = tp_find(id);
    TpOut o;
    u32 bytes, cap, block, crc;
    u8 *copy;
    if (f == NULL || f->refused) {
        return;
    }
    bytes = f->sectors << 11;
    cap = (u32)port_text_sectors(id) << 11;
    /* never past the heap block cdload allocated, whatever made it (a save state from a run without the pack loads
     * buffers allocated at the disc's size): the block's data ends where the next block's header starts */
    block = (u32)((u8 *)((HeapBlock *)buffer - 1)->next - (u8 *)buffer);
    if (block < cap) {
        cap = block;
    }
    if (cap < f->size) {
        port_log("text pack: frame %ld: file 0x%03X's buffer has %u bytes, not the %u it rebuilds to: left in English",
                 port_frames, (unsigned)id, (unsigned)cap, (unsigned)f->size);
        return;
    }
    crc = tp_crc32(buffer, bytes);
    if (crc != f->crc) {
        port_log("text pack: frame %ld: file 0x%03X is not the English file the pack was built against (CRC-32 "
                 "%08X, the pack's %08X): left in English", port_frames, (unsigned)id, (unsigned)crc, (unsigned)f->crc);
        f->refused = 1;
        return;
    }
    copy = malloc(bytes);
    if (copy == NULL) {
        port_log("text pack: frame %ld: file 0x%03X: out of memory: left in English", port_frames, (unsigned)id);
        return;
    }
    memcpy(copy, buffer, bytes);
    memset(&o, 0, sizeof(o));
    o.src = copy, o.len = bytes;
    o.next = &tp_entries[f->first], o.end = &tp_entries[f->first + f->count];
    if (!tp_rebuild(&o, (u32)id) || o.pos != f->size) {
        port_log("text pack: frame %ld: file 0x%03X does not rebuild to the pack's %u bytes (%s): left in English",
                 port_frames, (unsigned)id, (unsigned)f->size, o.pos != f->size ? "another size" : "entries it lacks");
        f->refused = 1;
        free(copy);
        return;
    }
    o.dst = buffer, o.cap = cap, o.pos = 0;
    o.next = &tp_entries[f->first];
    if (!tp_rebuild(&o, (u32)id) || o.pos != f->size) { /* cannot happen after the measure: the same pass */
        memcpy(buffer, copy, bytes);
        port_log("text pack: frame %ld: file 0x%03X: the rebuild failed: left in English", port_frames, (unsigned)id);
        f->refused = 1;
        free(copy);
        return;
    }
    memset((u8 *)buffer + o.pos, 0, cap - o.pos);
    free(copy);
    port_log("text pack: frame %ld: file 0x%03X: %u entries replaced (%u -> %u bytes)", port_frames, (unsigned)id,
             (unsigned)f->count, (unsigned)bytes, (unsigned)f->size);
}
