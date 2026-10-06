/* port/psyq/libsnd.c: LIBSND. No sound in the M1 skeleton (DECISIONS: our own SPU core and LIBSND come later): every
 * call records and reports success. The identifiers the game keeps (VAB ids, SEP access numbers, voices) are
 * what the PS1 would hand out for a successful call, so that sound.c's bookkeeping stays consistent. */
#include "psyq_internal.h"
#include "psyq/libsnd.h"

static s16 psyq_snd_sep_next; /* the next SEP access number SsSepOpen hands out (the PS1 reuses closed ones) */
static s16 psyq_snd_voice_next;

/* The console's reset (psyq.c psyq_reset): the identifiers start over (no SEP open, no voice used). */
void psyq_snd_reset(void) {
    psyq_snd_sep_next = 0;
    psyq_snd_voice_next = 0;
}

void SsInit(void) {
    PSYQ_TRACE("SsInit");
}

void SsStart2(void) {
    PSYQ_TRACE("SsStart2");
}

void SsSetTableSize(u8 *table, s16 s_max, s16 t_max) {
    PSYQ_TRACE("SsSetTableSize %u %d x %d", PSYQ_PTR(table), s_max, t_max);
}

void SsSetTickMode(s32 tick_mode) {
    PSYQ_TRACE("SsSetTickMode %x", tick_mode);
}

/* The sequencer tick the game runs from its vsync callback (tick mode 0x1000/0x1032 = called by the user). Nothing
 * to advance without a sequencer; not traced, it runs every frame. */
void SsSeqCalledTbyT(void) {
}

void SsSetMVol(s16 voll, s16 volr) {
    PSYQ_TRACE("SsSetMVol %d,%d", voll, volr);
}

void SsSetSerialAttr(char s_num, char attr, char mode) {
    PSYQ_TRACE("SsSetSerialAttr %d %d %d", s_num, attr, mode);
}

void SsSetSerialVol(char s_num, s16 voll, s16 volr) {
    PSYQ_TRACE("SsSetSerialVol %d %d,%d", s_num, voll, volr);
}

/* Success: the VAB id the caller asked for (sound.c passes its entry index); -1 would be a failure. */
s16 SsVabOpenHeadSticky(u8 *addr, s16 vab_id, u32 sbaddr) {
    PSYQ_TRACE("SsVabOpenHeadSticky %u id %d spu %x", PSYQ_PTR(addr), vab_id, sbaddr);
    return vab_id;
}

s16 SsVabTransBody(u8 *addr, s16 vab_id) {
    PSYQ_TRACE("SsVabTransBody %u id %d", PSYQ_PTR(addr), vab_id);
    return vab_id;
}

/* 1 = the transfer is complete. */
s16 SsVabTransCompleted(s16 immediate_flag) {
    PSYQ_TRACE("SsVabTransCompleted %d", immediate_flag);
    return 1;
}

void SsVabClose(s16 vab_id) {
    PSYQ_TRACE("SsVabClose %d", vab_id);
}

/* Success: an access number (0..); -1 would be "no free slot". */
s16 SsSepOpen(u32 *addr, s16 vab_id, s16 seq_num) {
    s16 access = psyq_snd_sep_next;

    PSYQ_TRACE("SsSepOpen %u vab %d seqs %d -> %d", PSYQ_PTR(addr), vab_id, seq_num, access);
    psyq_snd_sep_next = (s16)((psyq_snd_sep_next + 1) & 0x7F);
    return access;
}

void SsSepPlay(s16 access_num, s16 seq_num, char play_mode, s16 l_count) {
    PSYQ_TRACE("SsSepPlay %d:%d mode %d loops %d", access_num, seq_num, play_mode, l_count);
}

void SsSepStop(s16 access_num, s16 seq_num) {
    PSYQ_TRACE("SsSepStop %d:%d", access_num, seq_num);
}

void SsSepClose(s16 access_num) {
    PSYQ_TRACE("SsSepClose %d", access_num);
}

void SsSepSetVol(s16 access_num, s16 seq_num, s16 voll, s16 volr) {
    PSYQ_TRACE("SsSepSetVol %d:%d %d,%d", access_num, seq_num, voll, volr);
}

void SsSepSetDecrescendo(s16 access_num, s16 seq_num, s16 vol, s32 v_time) {
    PSYQ_TRACE("SsSepSetDecrescendo %d:%d vol %d time %d", access_num, seq_num, vol, v_time);
}

/* Returns the previous type (0: off). */
s16 SsUtSetReverbType(s16 type) {
    PSYQ_TRACE("SsUtSetReverbType %d", type);
    return 0;
}

void SsUtSetReverbDepth(s16 ldepth, s16 rdepth) {
    PSYQ_TRACE("SsUtSetReverbDepth %d,%d", ldepth, rdepth);
}

void SsUtReverbOn(void) {
    PSYQ_TRACE("SsUtReverbOn");
}

void SsUtAllKeyOff(s16 mode) {
    PSYQ_TRACE("SsUtAllKeyOff %d", mode);
}

/* Success: a voice number (0..23); -1 would be "no voice". */
s16 SsUtKeyOn(s16 vabId, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr) {
    s16 voice = psyq_snd_voice_next;

    PSYQ_TRACE("SsUtKeyOn vab %d prog %d tone %d note %d fine %d vol %d,%d -> voice %d", vabId, prog, tone, note,
               fine, voll, volr, voice);
    psyq_snd_voice_next = (s16)((psyq_snd_voice_next + 1) % 24);
    return voice;
}

/* 0 = ok. */
s16 SsUtKeyOff(s16 voice, s16 vabId, s16 prog, s16 tone, s16 note) {
    PSYQ_TRACE("SsUtKeyOff voice %d vab %d prog %d tone %d note %d", voice, vabId, prog, tone, note);
    return 0;
}
