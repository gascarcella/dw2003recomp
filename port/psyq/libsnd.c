/* port/psyq/libsnd.c: LIBSND, the 24 functions the game calls (docs/SOUND.md section 1), over the port's SPU core.
 * This file has the public calls, start-up, the VABs (header parsing, the SPU addresses of their samples, the body's
 * transfer) and the score table's lookup; the sequencer is libsnd_seq.c, the voice manager libsnd_voice.c, the SPU
 * side libsnd_spu.c (libsnd_internal.h has the overview). The game runs LIBSND with SS_NOTICK: it calls
 * SsSeqCalledTbyT itself from its vsync handler (gfx.c), 50 times a second (SsSetTickMode(0x1032)).
 *
 * The score table: SsSetTableSize(table, s_max, t_max) makes one array of s_max x t_max sequences, addressed as
 * access x t_max + sequence, and nothing checks the sequence against t_max: the game plays COMMON's SEP 0 with
 * sequences 0x19..0x1C, which are sequences 9..12 of access number 1, COMMON's second SEP (docs/SOUND.md section 1).
 * The port keeps that layout (in its own array; the game's table memory is not used).
 *
 * The VAB header stays in the game's buffer (LIBSND keeps a pointer to it, as on the PS1, and reads the program and
 * tone records from there); what LIBSND would write into it (each program's tone block, the VAGs' SPU addresses) is
 * kept in SndVab instead. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libsnd_internal.h"
#include "psyq/libetc.h"
#include "psyq/libsnd.h"

SndState snd;
static int snd_max_progs = 0x80; /* program records of the VAB opened last (LIBSND keeps one value for all) */
static void (*snd_call_hook)(const char *line);

/* ---- Tracing ---- */

int psyq_snd_in_vsync(void) {
    return snd.in_vsync;
}

void psyq_snd_set_call_hook(void (*hook)(const char *line)) {
    snd_call_hook = hook;
}

void snd_call(const char *fmt, ...) {
    char line[160];
    va_list ap;

    if (snd_call_hook == NULL && !PSYQ_TRACE_ON()) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    PSYQ_TRACE("%s", line);
    if (snd_call_hook != NULL) {
        snd_call_hook(line);
    }
}

/* ---- Shared lookups ---- */

SndSeq *snd_seq(int access, int seq) {
    int index = access * snd.t_max + seq;

    if (access < 0 || seq < 0 || index >= snd.s_max * snd.t_max || index >= SND_SEQS) {
        return NULL;
    }
    return &snd.seq[index];
}

const u8 *snd_prog(const SndVab *v, int prog) {
    return v->progs + (prog & 0xFF) * 16;
}

/* Tone record `index` (block x 16 + tone). An index past the header's records (only a stale selection could make one)
 * reads the last record instead of memory beyond the buffer. */
const u8 *snd_tone(const SndVab *v, int index) {
    int count = (v->head[0x12] | v->head[0x13] << 8) * 16;

    index &= 0xFFFF;
    if (index >= count) {
        index = count > 0 ? count - 1 : 0;
    }
    return v->tones + index * 32;
}

/* The VAB and program for a note, as LIBSND selects them: the VAB must be playable (its body sent) and the program
 * below the record count; the selection (and the program's tone block) stays for later calls. */
int snd_select(int vab, int prog) {
    const SndVab *v;

    if ((u16)vab >= SND_VABS || snd.vab[vab].state != 1 || prog >= snd_max_progs) {
        return -1;
    }
    v = &snd.vab[vab];
    snd.cur.vab = (s8)vab;
    snd.cur.prog = (s8)prog;
    snd.cur.vab_data = v;
    snd.cur.block = (s8)v->block[prog & 0x7F];
    return 0;
}

/* ---- The console's reset ---- */

void psyq_snd_reset(void) {
    snd_call("reset()"); /* the trace's mark: the SPU is cleared too (port/src/reset.c), and no vsync handler runs */
    memset(&snd, 0, sizeof(snd));
    snd_max_progs = 0x80;
    snd_spu_reset();
}

/* ---- Start-up ---- */

/* SsInit: LIBSPU's set-up, the reverb work area cleared, LIBSND's volumes (main 0x3FFF, the rest 0) and voices. */
void SsInit(void) {
    static const u16 control[16] = { 0x3FFF, 0x3FFF }; /* main volume; reverb depth, keys, PMON/NON/EON, ENDX 0 */
    int i;

    snd_call("SsInit()");
    snd_spu_init();
    snd_spu_clear_reverb_area();
    for (i = 0; i < 16; i++) {
        spu_write16(SND_REG_MVOL_L + 2 * (u32)i, control[i]);
    }
    snd_voices_init(SND_VOICES);
    snd.tick_rate = 60;
    snd.open_mask = 0;
    snd.lock = 0;
}

void SsStart2(void) {
    snd_call("SsStart2()");
    /* SS_NOTICK: nothing to start (no timer interrupt; the game calls SsSeqCalledTbyT) */
}

void SsSetTableSize(u8 *table, s16 s_max, s16 t_max) {
    snd_call("SsSetTableSize(%d, %d)", s_max, t_max);
    (void)table;
    if (s_max * t_max > SND_SEQS || s_max > 32) {
        port_unimplemented("SsSetTableSize: a table larger than the port's");
    }
    snd_seq_table(s_max, t_max);
}

/* The tick rate: 0x1000 = SS_NOTICK (the caller ticks), the low bits the rate (6 and more: per second; 0 and 5: the
 * video mode's, 1: 60, 2: 240, 3: 120, 4: 50, as LIBSND maps them). */
void SsSetTickMode(s32 tick_mode) {
    int code = (tick_mode & 0x1000) ? (tick_mode & 0xFFF) : tick_mode;
    int video;

    snd_call("SsSetTickMode(0x%x)", (unsigned)tick_mode);
    video = SetVideoMode(0); /* LIBETC has no getter here: read the mode and put it back */
    SetVideoMode(video);
    if (code >= 6) {
        snd.tick_rate = code;
    } else if (code == 1) {
        snd.tick_rate = 60;
    } else if (code == 2) {
        snd.tick_rate = 240;
    } else if (code == 3) {
        snd.tick_rate = 120;
    } else if (code == 4) {
        snd.tick_rate = 50;
    } else if (code >= 0) {
        snd.tick_rate = video == MODE_PAL ? 50 : 60;
    } else {
        snd.tick_rate = 60;
    }
}

/* The sequencer's tick, from the game's vsync handler: the DMA completion that was due, the flush (what the previous
 * tick and the game keyed), then every playing sequence. */
void SsSeqCalledTbyT(void) {
    snd.in_vsync = 1;
    snd_spu_vsync();
    if (snd.lock != 1) {
        snd.lock = 1;
        snd_flush();
        snd_seq_tick();
        snd.lock = 0;
    }
    snd.in_vsync = 0;
}

/* ---- Volumes ---- */

void SsSetMVol(s16 voll, s16 volr) {
    snd_call("SsSetMVol(%d, %d)", voll, volr);
    snd_spu_set_main_volume((u16)(voll * 0x81), (u16)(volr * 0x81));
}

/* Serial input 0 is the CD (1, external, has no SPU path here): attribute 0 = mix it in, 1 = through reverb. */
void SsSetSerialAttr(char s_num, char attr, char mode) {
    snd_call("SsSetSerialAttr(%d, %d, %d)", s_num, attr, mode);
    if (s_num == 0 && attr == 0) {
        snd_spu_set_cd_mix(mode != 0);
    } else {
        PSYQ_TRACE("libsnd: SsSetSerialAttr %d %d not modelled", s_num, attr);
    }
}

void SsSetSerialVol(char s_num, s16 voll, s16 volr) {
    snd_call("SsSetSerialVol(%d, %d, %d)", s_num, voll, volr);
    if (voll >= 0x80) {
        voll = 0x7F;
    }
    if (volr >= 0x80) {
        volr = 0x7F;
    }
    if (s_num == 0) {
        snd_spu_set_cd_volume((u16)(voll * 0x102), (u16)(volr * 0x102));
    }
}

/* ---- VABs ---- */

static s16 vab_fail(SndVab *v) {
    v->state = 0;
    snd.vab_count--;
    snd_spu_set_in_transfer(0);
    return -1;
}

/* The header at `addr` as VAB `vab_id` (-1: the first free one) whose body will go to SPU address `sbaddr`: the
 * programs' tone blocks and the VAGs' addresses (the size table, 256 entries after the tone records, in units of 8
 * bytes from version 5, else 4). -1 when a transfer is still running, the id is taken, the header is not a VAB or the
 * body would not fit. */
s16 SsVabOpenHeadSticky(u8 *addr, s16 vab_id, u32 sbaddr) {
    SndVab *v;
    const u8 *sizes;
    u32 version, unit, total, sum;
    int id, i, programs, vags, blocks;

    snd_call("SsVabOpenHeadSticky(%d, 0x%x)", vab_id, (unsigned)sbaddr);
    if (snd_spu_in_transfer()) {
        return -1;
    }
    snd_spu_set_in_transfer(1);
    if (vab_id >= SND_VABS) {
        snd_spu_set_in_transfer(0);
        return -1;
    }
    if (vab_id == -1) {
        for (id = 0; id < SND_VABS && snd.vab[id].state != 0; id++) {
        }
        if (id == SND_VABS) {
            snd_spu_set_in_transfer(0);
            return -1;
        }
    } else {
        id = vab_id;
        if (snd.vab[id].state != 0) {
            snd_spu_set_in_transfer(0);
            return -1;
        }
    }
    v = &snd.vab[id];
    v->state = 1; /* reserved while it is read */
    snd.vab_count++;
    if (addr[1] != 'B' || addr[2] != 'A' || addr[3] != 'V') {
        return vab_fail(v);
    }
    version = (u32)(addr[4] | addr[5] << 8 | addr[6] << 16 | (u32)addr[7] << 24);
    snd_max_progs = (addr[0] == 'p' && version >= 5) ? 0x80 : 0x40;
    programs = addr[0x12] | addr[0x13] << 8;
    if (snd_max_progs < programs) {
        return vab_fail(v);
    }
    v->head = addr;
    v->progs = addr + 0x20;
    v->nprogs = snd_max_progs;
    v->tones = v->progs + snd_max_progs * 16;
    blocks = 0;
    for (i = 0; i < snd_max_progs; i++) {
        v->block[i] = (u8)blocks;
        if (v->progs[i * 16] != 0) {
            blocks++;
        }
    }
    sizes = v->tones + programs * 16 * 32;
    vags = addr[0x16];
    unit = version >= 5 ? 8 : 4;
    total = 0;
    for (i = 0; i <= vags; i++) {
        total += (u32)(sizes[2 * i] | sizes[2 * i + 1] << 8) * unit;
    }
    if (sbaddr + ((total + 0x3F) & ~0x3Fu) > 0x80000) {
        return vab_fail(v);
    }
    v->spu_addr = sbaddr;
    sum = 0;
    for (i = 0; i <= vags; i++) {
        sum += (u32)(sizes[2 * i] | sizes[2 * i + 1] << 8) * unit;
        v->vag_end[i] = (u16)((sbaddr + sum) >> 3);
    }
    v->body_size = sum;
    v->state = 2;
    return (s16)id;
}

/* The body (the VAGs, back to back) to the VAB's SPU address, by DMA; SsVabTransCompleted reports its end. */
s16 SsVabTransBody(u8 *addr, s16 vab_id) {
    SndVab *v;

    snd_call("SsVabTransBody(%d)", vab_id);
    if ((u16)vab_id < SND_VABS && snd.vab[vab_id].state == 2) {
        v = &snd.vab[vab_id];
        if (v->spu_addr - 0x1010 <= 0x7EFE8) {
            snd_spu_transfer(addr, v->body_size, v->spu_addr);
            v->state = 1;
            return vab_id;
        }
    }
    snd_spu_set_in_transfer(0);
    return -1;
}

/* 1 once the body's DMA has completed (immediate_flag 1: waits for it). */
s16 SsVabTransCompleted(s16 immediate_flag) {
    int done = snd_spu_transfer_done(immediate_flag == 1);

    snd_call("SsVabTransCompleted(%d) = %d", immediate_flag, done);
    return (s16)done;
}

void SsVabClose(s16 vab_id) {
    snd_call("SsVabClose(%d)", vab_id);
    if ((u16)vab_id < SND_VABS && snd.vab[vab_id].state != 0 && snd.vab[vab_id].state < 3) {
        snd.vab[vab_id].state = 0;
        snd.vab_count--;
        if (snd_spu_in_transfer()) {
            snd_spu_set_in_transfer(0);
        }
    }
}

/* ---- SEPs ---- */

/* `seq_num` sequences from `addr` under the first free access number, playing VAB `vab_id`. (The trace line gives the
 * SEP's place as its offset from the VAB's header: tests/port/sound.py finds the bank's files with it.) */
s16 SsSepOpen(u32 *addr, s16 vab_id, s16 seq_num) {
    const u8 *p = (const u8 *)addr;
    long rel = (u16)vab_id < SND_VABS && snd.vab[vab_id].head != NULL ? (long)(p - snd.vab[vab_id].head) : 0;
    int access = 0, t, used;

    if (snd.open_mask == 0xFFFFFFFFu) {
        snd_call("SsSepOpen(head+%ld, %d, %d) = -1", rel, vab_id, seq_num);
        return -1;
    }
    while (access < 32 && (snd.open_mask & (1u << access))) {
        access++;
    }
    snd.open_mask |= 1u << access;
    for (t = 0; t < seq_num; t++) {
        used = snd_seq_open(access, t, vab_id, p);
        if (used == -1) {
            snd_call("SsSepOpen(head+%ld, %d, %d) = -1", rel, vab_id, seq_num);
            return -1;
        }
        p += used;
    }
    snd_call("SsSepOpen(head+%ld, %d, %d) = %d", rel, vab_id, seq_num, access);
    return (s16)access;
}

void SsSepPlay(s16 access_num, s16 seq_num, char play_mode, s16 l_count) {
    snd_call("SsSepPlay(%d, %d, %d, %d)", access_num, seq_num, play_mode, l_count);
    snd_seq_play(access_num, seq_num, play_mode, l_count);
}

void SsSepStop(s16 access_num, s16 seq_num) {
    snd_call("SsSepStop(%d, %d)", access_num, seq_num);
    snd_seq_stop(access_num, seq_num);
}

void SsSepClose(s16 access_num) {
    snd_call("SsSepClose(%d)", access_num);
    snd_seq_close(access_num);
}

void SsSepSetVol(s16 access_num, s16 seq_num, s16 voll, s16 volr) {
    snd_call("SsSepSetVol(%d, %d, %d, %d)", access_num, seq_num, voll, volr);
    snd_seq_set_volume(access_num, seq_num, voll, volr);
}

void SsSepSetDecrescendo(s16 access_num, s16 seq_num, s16 vol, s32 v_time) {
    snd_call("SsSepSetDecrescendo(%d, %d, %d, %d)", access_num, seq_num, vol, v_time);
    snd_seq_decrescendo(access_num, seq_num, vol, v_time);
}

/* ---- Utilities ---- */

/* Returns the type set (-1 for a bad one). */
s16 SsUtSetReverbType(s16 type) {
    snd_call("SsUtSetReverbType(%d)", type);
    return snd_reverb_type(type);
}

void SsUtSetReverbDepth(s16 ldepth, s16 rdepth) {
    snd_call("SsUtSetReverbDepth(%d, %d)", ldepth, rdepth);
    snd_reverb_depth(ldepth, rdepth);
}

void SsUtReverbOn(void) {
    snd_call("SsUtReverbOn()");
    snd_spu_reverb_enable(1);
}

void SsUtAllKeyOff(s16 mode) {
    snd_call("SsUtAllKeyOff(%d)", mode);
    snd_all_key_off();
}

/* A note of VAB `vabId` outside any sequence: the voice it got, or -1. */
s16 SsUtKeyOn(s16 vabId, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr) {
    snd_call("SsUtKeyOn(%d, %d, %d, %d, %d, %d, %d)", vabId, prog, tone, note, fine, voll, volr);
    return (s16)snd_ut_key_on(vabId, prog, tone, note, fine, voll, volr);
}

s16 SsUtKeyOff(s16 voice, s16 vabId, s16 prog, s16 tone, s16 note) {
    snd_call("SsUtKeyOff(%d, %d, %d, %d, %d)", voice, vabId, prog, tone, note);
    return (s16)snd_ut_key_off(voice, vabId, prog, tone, note);
}
