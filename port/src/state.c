/* The game-state probes (port_harness.h): what tests/replay/run.lua reads from PS1 RAM, read from the host's objects,
 * and gamestate_data's PS1 image, which a checkpoint hashes.
 *
 * A host object has the PS1's layout only up to its first pointer (pointers are 8 bytes at -m64; OverlayModule is 0x10
 * bytes on the PS1 and 0x18 here). port_state_read maps a PS1 address to a host byte only inside a layout-identical
 * range: the whole object for the EXE's sized data symbols whose -m64 size is their PS1 size (port_gen.py state
 * decides it the same way in the -m32 build), or the prefix listed below for the pointer-bearing objects the scripts
 * read, each checked by _Static_assert, or a field of the field table below (objects whose fields after a pointer the
 * scripts read, and an overlay's data). Anything else reads as unmapped (the caller makes that fatal). */
#include <stddef.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "sha1.h"

#include "fieldstg.h"
#include "gamestate.h"
#include "memcard.h"
#include "overlay.h"
#include "pad.h"

extern MemcardState memcard_state; /* src/main/memcard.c (not in memcard.h: STGMCARD declares it with its own type) */

/* ---- The layout-identical prefixes of the pointer-bearing objects */

/* overlay_module: stage and file (two s32 at 0 and 4), then the two function pointers. */
_Static_assert(offsetof(OverlayModule, stage) == 0x0 && offsetof(OverlayModule, file) == 0x4,
               "OverlayModule: stage/file moved");
_Static_assert(sizeof(((OverlayModule *)0)->file) == 4, "OverlayModule.file is an s32");
/* gamestate_data: every field before funcs (0x26FC on the PS1) is pointer-free: the last one, meter_random_count,
 * is at its PS1 offset, so nothing before it grew. funcs follows (at 0x2700 at -m64: 8-byte alignment). */
#define PORT_GAMESTATE_FUNCS 0x26FC
#define PORT_GAMESTATE_FUNC_COUNT 24
_Static_assert(offsetof(GamestateData, meter_random_count) == 0x26F8, "GamestateData: a field before funcs grew");
_Static_assert(sizeof(((GamestateData *)0)->meter_random_count) == 4, "GamestateData.meter_random_count is an s32");
_Static_assert(offsetof(GamestateData, funcs) >= PORT_GAMESTATE_FUNCS, "GamestateData.funcs moved");
_Static_assert(sizeof(GamestateFuncs) == PORT_GAMESTATE_FUNC_COUNT * sizeof(PortFn), "GamestateFuncs: 24 pointers");
_Static_assert(PORT_GAMESTATE_FUNCS + PORT_GAMESTATE_FUNC_COUNT * 4 == PORT_GAMESTATE_PS1_SIZE,
               "gamestate_data's PS1 size");
/* pad_random: index (an s32 at 0), then seed and next. */
_Static_assert(offsetof(PadRandom, index) == 0 && sizeof(((PadRandom *)0)->index) == 4, "PadRandom.index moved");

typedef struct PortPrefix {
    const void *host; /* the object */
    u32 length;       /* its layout-identical prefix */
    u32 addr;         /* its PS1 address (from port_exe_data, at the first use) */
} PortPrefix;

static PortPrefix port_prefixes[] = {
    { &overlay_module, 0x8, 0 },
    { &gamestate_data, PORT_GAMESTATE_FUNCS, 0 },
    { &pad_random, 0x4, 0 },
};
#define PORT_PREFIX_COUNT ((int)(sizeof(port_prefixes) / sizeof(port_prefixes[0])))

/* ---- The field table: single fields at their PS1 addresses, for objects whose PS1 layout the host does not keep
 * past a pointer (a field's PS1 offset is not its host offset) and for an overlay's data (mapped only while that
 * overlay is the current tier-1 one: on the PS1 the address holds another file's bytes otherwise). An entry is a PS1
 * range [addr, addr + size) and the host field it reads, of the same size (scalars and pointer-free members only).
 * The PS1 offsets are checked against offsetof where the host agrees: everywhere at -m32 (4-byte pointers), and the
 * distances between fields with no pointer between them at -m64. */

#if __SIZEOF_POINTER__ == 4
#define PORT_FIELD_AT_PS1(type, member, ps1) \
    _Static_assert(offsetof(type, member) == (ps1), #type "." #member ": not at its PS1 offset")
#else
#define PORT_FIELD_AT_PS1(type, member, ps1) _Static_assert(sizeof(((type *)0)->member) != 0, #type "." #member)
#endif

/* memcard_state (0x80048750, size 0x328; config/symbol_addrs.txt): run.lua's wait_mem reads state (+0x0), command
 * (+0x90) and done (+0x300); the other scalars come with them. file_name (+0xC) and files (DIRENTRY has a pointer)
 * move every later field on the host. */
#define PORT_MEMCARD_STATE 0x80048750u
#define PORT_MEMCARD_STATE_SIZE 0x328
PORT_FIELD_AT_PS1(MemcardState, header, 0x10);
PORT_FIELD_AT_PS1(MemcardState, command, 0x90);
PORT_FIELD_AT_PS1(MemcardState, file_count, 0xA4);
PORT_FIELD_AT_PS1(MemcardState, done, 0x300);
PORT_FIELD_AT_PS1(MemcardState, icon_frames, 0x314);
_Static_assert(offsetof(MemcardState, state) == 0 && offsetof(MemcardState, unk_008) == 0x8, "MemcardState: prefix");
_Static_assert(sizeof(MemcardHeader) == 0x80, "MemcardHeader: 0x80 bytes, pointer-free");
_Static_assert(offsetof(MemcardState, file_count) - offsetof(MemcardState, command) == 0x14,
               "MemcardState: command..file_count");
_Static_assert(offsetof(MemcardState, icon_frames) - offsetof(MemcardState, done) == 0x14,
               "MemcardState: done..icon_frames");

/* fieldstg_stage (FIELDSTG's 0x80099D80, size 0x80; config/fieldstg.symbols.txt): the scripts read menu_open (+0x50).
 * Its pointers (entry at +0x4 first) move the later fields on the host. */
#define PORT_FIELDSTG_STAGE 0x80099D80u
PORT_FIELD_AT_PS1(FieldstgStageState, menu_open, 0x50);
PORT_FIELD_AT_PS1(FieldstgStageState, return_dir, 0x6C);
_Static_assert(offsetof(FieldstgStageState, actor_busy) - offsetof(FieldstgStageState, menu_open) == 0x10,
               "FieldstgStageState: menu_open..actor_busy");
_Static_assert(offsetof(FieldstgStageState, event_text) - offsetof(FieldstgStageState, start_pos) == 0x1C,
               "FieldstgStageState: start_pos..event_text");

typedef struct PortField {
    u32 addr;            /* the PS1 address */
    u32 size;            /* the PS1 size */
    const void *host;    /* the host field */
    u32 host_size;       /* its host size (must be size) */
    const char *overlay; /* NULL: an EXE object; else the tier-1 overlay that must be current */
    const void *object;  /* the host object and its PS1 address: at -m32 the field must be at its PS1 offset */
    u32 object_addr;
} PortField;

#define PORT_FIELD(base, object, member, ps1, overlay)                                                      \
    { (base) + (ps1), sizeof((object).member), &(object).member, sizeof((object).member), overlay, &(object), \
      (base) }
#define PORT_MEMCARD_FIELD(member, ps1) PORT_FIELD(PORT_MEMCARD_STATE, memcard_state, member, ps1, NULL)
#define PORT_FIELDSTG_FIELD(member, ps1) PORT_FIELD(PORT_FIELDSTG_STAGE, fieldstg_stage, member, ps1, "FIELDSTG")

static const PortField port_fields[] = {
    PORT_MEMCARD_FIELD(state, 0x0),
    PORT_MEMCARD_FIELD(unk_004, 0x4),
    PORT_MEMCARD_FIELD(unk_008, 0x8),
    PORT_MEMCARD_FIELD(header, 0x10),
    PORT_MEMCARD_FIELD(command, 0x90),
    PORT_MEMCARD_FIELD(result, 0x94),
    PORT_MEMCARD_FIELD(retries, 0x98),
    PORT_MEMCARD_FIELD(max_retries, 0x9C),
    PORT_MEMCARD_FIELD(retry, 0xA0),
    PORT_MEMCARD_FIELD(file_count, 0xA4),
    PORT_MEMCARD_FIELD(done, 0x300),
    PORT_MEMCARD_FIELD(offset, 0x304),
    PORT_MEMCARD_FIELD(unk_308, 0x308),
    PORT_MEMCARD_FIELD(slot_size, 0x30C),
    PORT_MEMCARD_FIELD(part1_size, 0x310),
    PORT_MEMCARD_FIELD(icon_frames, 0x314),
    PORT_FIELDSTG_FIELD(code_file, 0x0),
    PORT_FIELDSTG_FIELD(background_file, 0x8),
    PORT_FIELDSTG_FIELD(sprite_file, 0xC),
    PORT_FIELDSTG_FIELD(mask_subfile, 0x18),
    PORT_FIELDSTG_FIELD(mask_file, 0x1C),
    PORT_FIELDSTG_FIELD(start_pos, 0x2C),
    PORT_FIELDSTG_FIELD(start_dir, 0x34),
    PORT_FIELDSTG_FIELD(color, 0x38),
    PORT_FIELDSTG_FIELD(music, 0x3C),
    PORT_FIELDSTG_FIELD(sound, 0x40),
    PORT_FIELDSTG_FIELD(talk_file, 0x44),
    PORT_FIELDSTG_FIELD(event_text, 0x48),
    PORT_FIELDSTG_FIELD(menu_open, 0x50),
    PORT_FIELDSTG_FIELD(title_shown, 0x54),
    PORT_FIELDSTG_FIELD(event_running, 0x58),
    PORT_FIELDSTG_FIELD(battle_starting, 0x5C),
    PORT_FIELDSTG_FIELD(actor_busy, 0x60),
    PORT_FIELDSTG_FIELD(return_pos, 0x64),
    PORT_FIELDSTG_FIELD(return_dir, 0x6C),
};
#define PORT_FIELD_COUNT ((int)(sizeof(port_fields) / sizeof(port_fields[0])))

static const PortExeData *port_exe_data_of(const void *host) {
    int i;
    for (i = 0; i < port_exe_data_count; i++) {
        if (port_exe_data[i].host == host) {
            return &port_exe_data[i];
        }
    }
    return NULL;
}

static void port_state_init(void) {
    static int done;
    int i;
    const PortExeData *d;
    if (done) {
        return;
    }
    for (i = 0; i < PORT_PREFIX_COUNT; i++) {
        d = port_exe_data_of(port_prefixes[i].host);
        if (d == NULL || port_prefixes[i].length > d->size) {
            port_fatal("state: prefix %d: not a sized EXE data symbol of config/symbol_addrs.txt", i);
        }
        port_prefixes[i].addr = d->addr;
    }
    d = port_exe_data_of(&memcard_state);
    if (d == NULL || d->addr != PORT_MEMCARD_STATE || d->size != PORT_MEMCARD_STATE_SIZE) {
        port_fatal("state: memcard_state is not at 0x%08X (size 0x%X) in config/symbol_addrs.txt", PORT_MEMCARD_STATE,
                   PORT_MEMCARD_STATE_SIZE);
    }
    for (i = 0; i < PORT_FIELD_COUNT; i++) {
        const PortField *f = &port_fields[i];
        if (f->size != f->host_size) {
            port_fatal("state: field 0x%08X: 0x%X bytes on the PS1, 0x%X here", f->addr, f->size, f->host_size);
        }
        if (sizeof(void *) == 4 && (u32)((const u8 *)f->host - (const u8 *)f->object) != f->addr - f->object_addr) {
            port_fatal("state: field 0x%08X: not at its PS1 offset at -m32", f->addr);
        }
    }
    d = port_exe_data_of(&gamestate_data);
    if (d->size != PORT_GAMESTATE_PS1_SIZE) {
        port_fatal("state: gamestate_data is 0x%X bytes in config/symbol_addrs.txt, 0x%X here", d->size,
                   PORT_GAMESTATE_PS1_SIZE);
    }
    done = 1;
}

/* ---- The probes */

s32 port_state_stage(void) {
    return overlay_module.stage;
}

s32 port_state_file(void) {
    return overlay_module.file;
}

s32 port_state_map(void) {
    return gamestate_data.map;
}

u32 port_state_slot1_word0(void) {
    return port_overlay_word0(1);
}

s32 port_state_random_index(void) {
    return pad_random.index;
}

/* The host byte of PS1 address `addr` for a `size`-byte read, or NULL outside a layout-identical range. */
static const u8 *port_state_map_addr(u32 addr, u32 size) {
    int i;
    port_state_init();
    for (i = 0; i < PORT_PREFIX_COUNT; i++) {
        const PortPrefix *p = &port_prefixes[i];
        if (addr >= p->addr && addr - p->addr + size <= p->length) {
            return (const u8 *)p->host + (addr - p->addr);
        }
    }
    for (i = 0; i < port_exe_data_count; i++) {
        const PortExeData *d = &port_exe_data[i];
        if (addr >= d->addr && addr - d->addr + size <= d->identical) {
            return (const u8 *)d->host + (addr - d->addr);
        }
    }
    for (i = 0; i < PORT_FIELD_COUNT; i++) {
        const PortField *f = &port_fields[i];
        if (addr >= f->addr && addr - f->addr + size <= f->size) {
            const PortOverlay *o = port_overlay_current(1);
            if (f->overlay != NULL && (o == NULL || strcmp(o->name, f->overlay) != 0)) {
                return NULL; /* the overlay's data is not in the slot */
            }
            return (const u8 *)f->host + (addr - f->addr);
        }
    }
    return NULL;
}

void *port_state_host(u32 addr, int size) {
    if (size != 1 && size != 2 && size != 4) {
        return NULL;
    }
    /* the mapped objects are the game's globals, writable; the map's pointers are const only for the readers */
    return (void *)(uintptr_t)port_state_map_addr(addr, (u32)size);
}

int port_state_read(u32 addr, int size, int is_signed, s32 *out) {
    const u8 *p;
    if (size != 1 && size != 2 && size != 4) {
        return 0;
    }
    p = port_state_map_addr(addr, (u32)size);
    if (p == NULL) {
        return 0;
    }
    if (size == 1) {
        *out = is_signed ? (s32)(s8)p[0] : (s32)p[0];
    } else if (size == 2) {
        u16 v;
        memcpy(&v, p, 2);
        *out = is_signed ? (s32)(s16)v : (s32)v;
    } else {
        u32 v;
        memcpy(&v, p, 4);
        *out = (s32)v;
    }
    return 1;
}

/* ---- gamestate_data's PS1 image and its hashes */

/* The PS1 address of an EXE function (port_exe_funcs, generated), 0 for NULL; fatal for anything else. */
static u32 port_state_exe_addr(PortFn fn, int index) {
    int i;
    if (fn == NULL) {
        return 0;
    }
    for (i = 0; i < port_exe_func_count; i++) {
        if (port_exe_funcs[i].fn == fn) {
            return port_exe_funcs[i].addr;
        }
    }
    port_fatal("state: gamestate_data.funcs[%d] is not an EXE function of config/symbol_addrs.txt", index);
}

void port_state_gamestate_image(u8 out[PORT_GAMESTATE_PS1_SIZE]) {
    const u8 *funcs = (const u8 *)&gamestate_data.funcs;
    int i;
    port_state_init();
    memcpy(out, &gamestate_data, PORT_GAMESTATE_FUNCS);
    for (i = 0; i < PORT_GAMESTATE_FUNC_COUNT; i++) {
        PortFn fn;
        u32 a;
        u8 *o = out + PORT_GAMESTATE_FUNCS + 4 * i;
        memcpy(&fn, funcs + (size_t)i * sizeof(PortFn), sizeof(fn));
        a = port_state_exe_addr(fn, i);
        o[0] = (u8)a;
        o[1] = (u8)(a >> 8);
        o[2] = (u8)(a >> 16);
        o[3] = (u8)(a >> 24);
    }
}

static void port_sha1_of(const u8 *data, size_t n, char hex[41]) {
    PortSha1 c;
    uint8_t digest[20];
    port_sha1_init(&c);
    port_sha1_update(&c, data, n);
    port_sha1_final(&c, digest);
    port_sha1_hex(digest, hex);
}

void port_state_gamestate_sha1(char full[41], char stable[41]) {
    static u8 image[PORT_GAMESTATE_PS1_SIZE];
    int i;
    port_state_gamestate_image(image);
    port_sha1_of(image, sizeof(image), full);
    for (i = 0; i < port_gamestate_volatile_count; i++) {
        const PortRange *r = &port_gamestate_volatile[i];
        if (r->lo < r->hi && r->hi <= sizeof(image)) {
            memset(image + r->lo, 0, r->hi - r->lo);
        } else {
            port_fatal("state: volatile range 0x%X..0x%X is outside gamestate_data", r->lo, r->hi);
        }
    }
    port_sha1_of(image, sizeof(image), stable);
}
