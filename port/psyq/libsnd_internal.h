/* port/psyq/libsnd_internal.h: what LIBSND's files share. Our own LIBSND over the port's SPU core (port/include/spu.h):
 * libsnd.c the public calls and the VABs, libsnd_seq.c the sequencer, libsnd_voice.c the voice manager, libsnd_spu.c
 * the SPU side (what LIBSPU does on the PS1; the game never calls LIBSPU, so there is no LIBSPU API here).
 *
 * Provenance (docs/SOUND.md section 7 "Provenance"): the behaviour is the PS1's, learnt from the EXE's LIBSND/LIBSPU
 * disassembly (constants, tables, formulas, the order of operations) and from the emulator's SPU write traces
 * (tests/sound); the code is ours. The trace oracle compares every SPU store, so the arithmetic (volumes, pitches,
 * tempo), the voice the allocator picks and the order of the register writes are the PS1's to the bit. */
#ifndef PORT_LIBSND_INTERNAL_H
#define PORT_LIBSND_INTERNAL_H

#include "psyq_internal.h"
#include "spu.h"

#define SND_VOICES 24
#define SND_VABS 16
#define SND_SEQS 512 /* score entries: SsSetTableSize's s_max x t_max (the game: 6 x 16) */
#define SND_SE_OWNER 0x21 /* the owner of SsUtKeyOn's notes, where a sequence note has access | sequence << 8 */

/* ---- The SPU side (libsnd_spu.c). Registers are spu.h's offsets from 0x1F801C00. ---- */
enum {
    SND_REG_MVOL_L = 0x180, SND_REG_MVOL_R = 0x182, SND_REG_RVOL_L = 0x184, SND_REG_RVOL_R = 0x186,
    SND_REG_KON = 0x188, SND_REG_KOFF = 0x18C, SND_REG_PMON = 0x190, SND_REG_NON = 0x194, SND_REG_EON = 0x198,
    SND_REG_ENDX = 0x19C, SND_REG_ESA = 0x1A2, SND_REG_TSA = 0x1A6, SND_REG_FIFO = 0x1A8, SND_REG_ATTR = 0x1AA,
    SND_REG_XFER_CTRL = 0x1AC, SND_REG_CDVOL_L = 0x1B0, SND_REG_CDVOL_R = 0x1B2, SND_REG_EXTVOL_L = 0x1B4,
    SND_REG_EXTVOL_R = 0x1B6, SND_REG_REVERB = 0x1C0,
};

void snd_spu_reset(void);
void snd_spu_init(void);               /* the hardware set-up at SsInit: registers, voices, the silent block */
void snd_spu_clear_reverb_area(void);  /* SsInit: the largest work area zeroed by DMA, one wait per block */
void snd_spu_set_key(int on, u32 voices);                    /* KON (on) or KOFF, 24 voice bits */
void snd_spu_set_voice_mask(int reg, u32 bits, u32 keep);    /* EON or NON: `bits`, the register's bits in `keep` */
u16 snd_spu_envelope(int voice);                             /* ENVX */
/* The voice registers a flush (or a direct set-up) writes, in LIBSPU's order: pitch, volume L/R, start address,
 * ADSR (`what`: SND_ATTR_* bits). */
#define SND_ATTR_VOL 1
#define SND_ATTR_PITCH 2
#define SND_ATTR_ADDR 4
#define SND_ATTR_ADSR 8
void snd_spu_voice_attr(int voice, int what, u16 vol_l, u16 vol_r, u16 pitch, u16 addr8, u16 adsr1, u16 adsr2);
void snd_spu_set_main_volume(u16 left, u16 right);
void snd_spu_set_cd_volume(u16 left, u16 right);
void snd_spu_set_cd_mix(int on);
int snd_spu_set_reverb_type(int type);   /* the preset's registers and work area; -1 for a bad type */
void snd_spu_set_reverb_depth(s16 left, s16 right);
void snd_spu_reverb_enable(int on);
/* The body transfer: SsVabTransBody's DMA to `addr` and its completion (snd_spu_transfer_done polls it). */
void snd_spu_transfer(const u8 *data, u32 bytes, u32 addr);
int snd_spu_transfer_done(int wait);
void snd_spu_vsync(void);                /* a vsync has begun: a DMA that outlasts its frame completes */
int snd_spu_in_transfer(void);
void snd_spu_set_in_transfer(int on);

/* ---- The VABs (libsnd.c) ---- */
typedef struct SndVab {
    u8 state;           /* 0 closed, 2 header open, 1 body sent (playable), as LIBSND numbers them */
    const u8 *head;     /* the VAB header, in the caller's buffer (read only) */
    const u8 *progs;    /* its program records (16 bytes each) */
    const u8 *tones;    /* its tone records (32 bytes, 16 per program with tones) */
    int nprogs;         /* program records: 128 (64 before version 5) */
    u32 spu_addr;       /* where the body goes */
    u32 body_size;      /* the VAG sizes' sum, rounded up to 64 bytes */
    u8 block[128];      /* program -> its tone block: the number of programs with tones before it */
    u16 vag_end[256];   /* SPU address / 8 where VAG n ends (n = 0..vags; VAG n starts at vag_end[n - 1]) */
} SndVab;

/* ---- The voices (libsnd_voice.c) ---- */
typedef struct SndVoice {
    u16 vag;      /* the sample keyed (0 after key off; 0xFF would be noise) */
    u16 age;      /* allocations since the voice was taken: the allocator's tie-break */
    u16 pitch;    /* the pitch keyed (0 after key off) */
    u16 env;      /* ENVX at the last flush; 0x7FFF when taken */
    s16 vel_raw;  /* a sequence note's velocity before the channel volume */
    u8 pan;       /* the note's pan */
    s16 channel;  /* a sequence note's channel */
    s16 note;
    s16 owner;    /* access | sequence << 8, SND_SE_OWNER, or -1 / 0xFF */
    s16 block;    /* the program's tone block */
    s16 prog;
    s16 tone;
    s16 vab;
    s16 prio;     /* the tone's priority */
    s8 status;    /* 1 keyed, 0 free or released (the envelope decides when it is reusable) */
    s16 vel;      /* the velocity after the channel volume */
} SndVoice;

/* The registers a voice is due to get at the next flush (SND_ATTR_* in `dirty`). */
typedef struct SndShadow {
    u16 vol_l, vol_r, pitch, addr, adsr1, adsr2;
    u8 dirty;
} SndShadow;

/* The note being keyed and the VAB selected for it: LIBSND keeps them between calls, and some calls use what an
 * earlier one selected, so they are global state here too. */
typedef struct SndCur {
    const SndVab *vab_data; /* the VAB selected (snd_select's last success) */
    s8 vab;
    s8 prog;
    s8 block;     /* the program's tone block */
    u8 tones;     /* the program's tone count */
    u8 prog_vol, prog_pan;
    s16 owner;
    u8 note, fine;
    u8 vel, pan;
    u8 tone;
    u8 tone_vol, tone_pan, prio, center, shift, mode;
    u16 vag;
    s16 voice;
} SndCur;

/* ---- The score table (libsnd_seq.c): one entry per (access number, sequence) ---- */
typedef struct SndSeq {
    const u8 *pos;      /* the next event */
    const u8 *start;    /* the first event (after the first delta time) */
    const u8 *loop;     /* the loop point (an NRPN loop's start, or the start) */
    u32 flags;          /* SND_SEQ_* */
    u8 play_mode;
    u8 loop_active;     /* an NRPN loop is running */
    u8 status;          /* the running status (0xFF after a meta event) */
    u8 channel;
    u8 rpn_count, nrpn_count;
    u8 nrpn_lsb, nrpn_msb;
    u8 loop_pending;    /* NRPN 20 seen: the next data entry is the loop count */
    u8 loop_count;
    s8 play_count;      /* SsSepPlay's l_count (0: endless) */
    u8 played;
    s8 vab;
    u8 pan[16], prog[16];
    s16 ch_vol[16];
    u16 mute;           /* channels whose notes are not keyed */
    s16 resolution;     /* ticks per quarter note */
    s16 frame_wait;     /* slow tempos: vsyncs to wait (-1: fast) */
    u16 step, step0;    /* fast tempos: delta units per vsync (now, as opened) */
    u16 vol_l, vol_r;
    u16 vol_l2, vol_r2;
    s32 first_delta;
    s32 elapsed;
    s32 bpm, bpm0;
    s32 delta;          /* delta units (10 per MIDI tick) to the next event */
    s16 fade_target, fade_done; /* decrescendo: the volume change and how much of it is done */
    s32 fade_time, fade_count;
} SndSeq;

#define SND_SEQ_PLAYING 0x01
#define SND_SEQ_PAUSED 0x02
#define SND_SEQ_STOPPED 0x04
#define SND_SEQ_REPLAY 0x08
#define SND_SEQ_CRESCENDO 0x10
#define SND_SEQ_DECRESCENDO 0x20
#define SND_SEQ_ENDED 0x200

typedef struct SndState {
    int lock;                /* a sequencer tick or a key call is running */
    int in_vsync;            /* SsSeqCalledTbyT (the vsync handler) is running: the trace's tick */
    u32 open_mask;           /* access numbers in use (and the ones beyond s_max) */
    int s_max, t_max;
    int tick_rate;           /* sequencer ticks per second */
    SndSeq seq[SND_SEQS];
    SndVab vab[SND_VABS];
    int vab_count;
    int nvoices;
    SndVoice voice[SND_VOICES];
    SndShadow shadow[SND_VOICES];
    u16 kon[2], koff[2], eon[2], non[2]; /* pending key-ons and key-offs, the reverb and noise voices: low/high */
    u32 idle_ring[16];       /* per flush, the voices whose envelope read 0 */
    int ring_pos;
    int release_add;         /* added to every ADSR release rate (the damper; 0) */
    int mono;
    SndCur cur;
} SndState;

extern SndState snd;

/* libsnd.c */
SndSeq *snd_seq(int access, int seq);   /* NULL outside the table */
int snd_select(int vab, int prog);      /* LIBSND's VAB/program selection for a note: 0, or -1 */
const u8 *snd_tone(const SndVab *v, int index);
const u8 *snd_prog(const SndVab *v, int prog);

/* libsnd_voice.c */
void snd_voices_init(int count);
void snd_flush(void);
int snd_key_on(s16 owner, s16 vab, s16 prog, u8 note, u16 vel, u16 pan);
int snd_key_off(s16 owner, s16 vab, s16 prog, u8 note);
void snd_key_off_now(int voice);
void snd_owner_key_off(s16 owner);
s16 snd_set_seq_volume(s16 owner, u16 left, u16 right);
int snd_set_volume(s16 owner, s16 vab, s16 prog, u16 vol, u16 pan);
int snd_pitch_bend(s16 owner, s16 vab, s16 prog, int msb);
int snd_ut_key_on(s16 vab, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);
int snd_ut_key_off(s16 voice, s16 vab, s16 prog, s16 tone, s16 note);
void snd_all_key_off(void);

/* libsnd_seq.c */
void snd_seq_table(int s_max, int t_max);
int snd_seq_open(int access, int seq, int vab, const u8 *data); /* bytes used, or -1 */
void snd_seq_tick(void);
void snd_seq_play(int access, int seq, int mode, int count);
void snd_seq_stop(int access, int seq);
void snd_seq_close(int access);
void snd_seq_set_volume(int access, int seq, s16 left, s16 right);
void snd_seq_decrescendo(int access, int seq, int vol, int time);
s16 snd_reverb_type(s16 type);
void snd_reverb_depth(s16 left, s16 right);

/* One line per call the game makes, to psyq_snd_set_call_hook's hook (psyq_internal.h) and the shim's trace. */
void snd_call(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* PORT_LIBSND_INTERNAL_H */
