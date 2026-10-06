/* port/psyq/libsnd_seq.c: LIBSND's sequencer: the score table, SEP opening, the per-tick playback of every playing
 * sequence, its MIDI-like events and LIBSND's controls (NRPN loops, VAB attributes through data entry), play/stop,
 * volume and decrescendo.
 *
 * Time: a delta time is counted in tenths of a MIDI tick (LIBSND multiplies every delta by 10). A sequence opened at
 * `bpm` beats per minute with `resolution` ticks per beat advances resolution x bpm x 10 / (rate x 60) of them per
 * sequencer tick (rounded to nearest; `rate` = SsSetTickMode's ticks per second, 50 here); below one per tick it
 * instead waits rate x 600 / (resolution x bpm) ticks per unit. Each tick reads the events that fall in its step.
 *
 * Events (FORMATS "SEP"): note on (velocity 0 = off) 9n, control change Bn, program change Cn, pitch bend En (only its
 * MSB counts), and the meta events FF: 2F ends the track, any other is read as a tempo (3 bytes, no length byte).
 * Running status continues the last status byte, and after a meta event the running status is FF: a data byte that
 * follows one is read as a meta type (LIBSND's behaviour; on the disc every one of the 91 tempo events is followed by a
 * status byte, so it never shows). 8n note-offs and other statuses are skipped without their data (none on the disc). */
#include "libsnd_internal.h"

/* ---- The table ---- */

static SndSeq *seq_at(int s, int t) {
    return snd_seq(s, t);
}

static void seq_reset_entry(SndSeq *e) {
    e->flags = 0;
    e->fade_target = 0;
    e->fade_done = 0;
    e->fade_time = 0;
    e->fade_count = 0;
    e->vol_l = e->vol_r = 0x7F;
}

void snd_seq_table(int s_max, int t_max) {
    int s, t;

    snd.s_max = s_max;
    snd.t_max = t_max;
    for (s = s_max; s < 32; s++) {
        snd.open_mask |= 1u << s;
    }
    for (s = 0; s < s_max; s++) {
        for (t = 0; t < t_max; t++) {
            SndSeq *e = seq_at(s, t);
            if (e != NULL) {
                seq_reset_entry(e);
                e->vol_l2 = e->vol_r2 = 0x7F;
            }
        }
    }
}

/* ---- Reading the data ---- */

/* A delta time (a MIDI variable-length number) x 10, added to the elapsed time. */
static s32 read_delta(SndSeq *e) {
    u32 v = *e->pos++;
    s32 d;

    if (v == 0) {
        return 0;
    }
    if (v & 0x80) {
        u32 c;
        v &= 0x7F;
        do {
            c = *e->pos++;
            v = (v << 7) + (c & 0x7F);
        } while (c & 0x80);
    }
    d = (s32)(v * 10);
    e->elapsed += d;
    return d;
}

/* The tick step for the sequence's tempo (see the header). */
static void seq_set_step(SndSeq *e) {
    u32 per = (u32)e->resolution * (u32)e->bpm * 10;
    u32 rate60 = (u32)snd.tick_rate * 60;

    if (per < rate60) {
        e->frame_wait = (s16)((u32)snd.tick_rate * 600 / ((u32)e->resolution * (u32)e->bpm));
        e->step = (u16)e->frame_wait;
        return;
    }
    e->frame_wait = -1;
    e->step = (u16)(per / rate60);
    if ((u32)snd.tick_rate * 30 < per % rate60) {
        e->step++;
    }
}

/* One sequence of a SEP at `data` (the SEP's start for sequence 0, else where the previous one ended): its header
 * (FORMATS "SEP") and the first delta time. Returns the bytes it takes, or -1. */
int snd_seq_open(int s, int t, int vab, const u8 *data) {
    SndSeq *e = seq_at(s, t);
    const u8 *p = data;
    int head = 0, ch;
    u32 tempo, size, q, rem;

    if (e == NULL) {
        return -1;
    }
    e->play_count = 1;
    e->loop_active = 0;
    e->status = 0;
    e->channel = 0;
    e->rpn_count = e->nrpn_count = 0;
    e->nrpn_lsb = e->nrpn_msb = 0;
    e->loop_pending = 0;
    e->loop_count = 0;
    e->play_mode = 0;
    e->played = 0;
    e->frame_wait = 1;
    e->resolution = 0;
    e->vab = (s8)vab;
    e->step0 = 0;
    e->first_delta = 0;
    e->elapsed = 0;
    e->bpm0 = 0;
    e->delta = 0;
    e->mute = 0;
    for (ch = 0; ch < 16; ch++) {
        e->pan[ch] = 0x40;
        e->prog[ch] = (u8)ch;
        e->ch_vol[ch] = 0x7F;
    }
    if (t == 0) {
        if (p[0] == 'S' || p[0] == 'p') { /* "SEQp" or "pQES": the magic and the version */
            if (p[5] != 0) {
                PSYQ_TRACE("libsnd: SEP %d:%d: version %d", s, t, p[5]);
                return -1;
            }
            p += 8; /* and sequence 0's number */
            head = 8;
        }
    } else {
        p += 2; /* the sequence number */
        head = 2;
    }
    e->resolution = (s16)(p[0] << 8 | p[1]);
    tempo = (u32)p[2] << 16 | (u32)p[3] << 8 | p[4];
    p += 5;
    q = 60000000u / tempo;
    rem = 60000000u % tempo;
    e->bpm0 = (s32)((tempo >> 1) < rem ? q + 1 : q);
    e->bpm = e->bpm0;
    p += 2; /* the rhythm */
    size = (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3];
    p += 4;
    e->pos = p;
    e->first_delta = read_delta(e);
    e->delta = e->first_delta;
    e->loop = e->pos;
    e->start = e->pos;
    seq_set_step(e);
    e->step0 = e->step;
    return head + 11 + (int)size;
}

/* ---- Events ---- */

static s16 owner_of(int s, int t) {
    return (s16)(s | t << 8);
}

static void seq_finish(int s, int t, SndSeq *e);

/* The end of the track: again (SsSepPlay's count, 0 = endless), or the end (the notes keyed off now, the stop at
 * this same tick). */
static void seq_end_of_track(int s, int t, SndSeq *e) {
    e->played++;
    if (e->play_count == 0 || (s8)e->played < e->play_count) {
        e->elapsed = 0;
        e->loop_pending = 0;
        e->delta = 0;
        e->pos = e->start;
        if (e->play_count != 0) {
            e->loop = e->pos;
        }
        return;
    }
    seq_finish(s, t, e);
}

static void seq_finish(int s, int t, SndSeq *e) {
    e->flags &= ~(SND_SEQ_PLAYING | SND_SEQ_REPLAY | SND_SEQ_PAUSED);
    e->flags |= SND_SEQ_ENDED | SND_SEQ_STOPPED;
    e->play_mode = 0;
    e->loop = e->start;
    snd_owner_key_off(owner_of(s, t));
    e->delta = e->step;
}

/* A tempo meta event: the new tempo (not rounded, unlike the header's) and the step. */
static void seq_tempo(SndSeq *e) {
    u32 tempo = (u32)e->pos[0] << 16 | (u32)e->pos[1] << 8 | e->pos[2];

    e->pos += 3;
    e->bpm = (s32)(60000000 / (s32)tempo);
    seq_set_step(e);
    e->delta = read_delta(e);
}

/* NRPN attribute 15 / 16 (FORMATS "SEP"): the reverb type and depth. The other attributes (ADSR, vibrato, ...) are
 * not in any SEP on the disc. */
static void seq_attribute(int attr, int value) {
    switch (attr) {
    case 15:
        snd_reverb_type((s16)value);
        break;
    case 16:
        snd_reverb_depth((s16)value, (s16)value);
        break;
    default:
        PSYQ_TRACE("libsnd: NRPN attribute %d = %d (not modelled)", attr, value);
        break;
    }
}

/* Data entry (CC 6): an NRPN loop's count, or an attribute's value; else nothing. LIBSND looks the program up first,
 * which selects the VAB (a side effect later calls see). */
static void seq_data_entry(int s, int t, SndSeq *e, u8 value) {
    int prog = e->prog[e->channel];

    (void)s;
    (void)t;
    if ((u16)e->vab < SND_VABS && snd.vab[(u16)e->vab].state == 1) {
        snd_select(e->vab, prog);
    }
    if (e->loop_pending == 1 && e->loop_active == 0) {
        e->loop_count = value;
        e->loop_pending = 0;
        e->loop_active = 1;
    } else if (e->rpn_count == 2) {
        e->rpn_count = 0; /* RPN data (VAG attributes): no SEP on the disc sends RPNs */
    } else if (e->nrpn_count == 2) {
        seq_attribute(e->nrpn_lsb, value);
        e->delta = read_delta(e);
        e->nrpn_count = 0;
        return;
    }
    e->delta = read_delta(e);
}

/* NRPN MSB (CC 99): 20 marks the loop's start (the next data entry is its count), 30 its end (back to the start while
 * the count lasts, 127 = forever); any other value starts an attribute (with the LSB, CC 98). */
static void seq_nrpn_msb(SndSeq *e, u8 value) {
    switch (value) {
    case 20:
        e->nrpn_msb = value;
        e->loop_pending = 1;
        e->delta = read_delta(e);
        e->loop = e->pos;
        break;
    case 30:
        e->nrpn_msb = value;
        if (e->loop_count == 0) {
            e->loop_active = 0;
            e->delta = read_delta(e);
        } else if (e->loop_count < 0x7F) {
            e->loop_count--;
            e->delta = read_delta(e);
            if (e->loop_count != 0) {
                e->pos = e->loop;
            } else {
                e->loop_active = 0;
            }
        } else {
            read_delta(e);
            e->delta = 0;
            e->pos = e->loop;
        }
        break;
    default:
        e->nrpn_msb = value;
        e->nrpn_count++;
        e->delta = read_delta(e);
        break;
    }
}

static void seq_nrpn_lsb(SndSeq *e, u8 value) {
    if (e->nrpn_msb != 30 && e->nrpn_msb != 20 && e->nrpn_msb != 40) {
        e->nrpn_lsb = value;
        e->loop_pending = 0;
        e->nrpn_count++;
    }
    e->delta = read_delta(e);
}

static void seq_control(int s, int t, SndSeq *e, u8 cc) {
    u8 value = *e->pos++;
    int ch = e->channel;

    switch (cc) {
    case 0:
        e->vab = (s8)value;
        e->delta = read_delta(e);
        break;
    case 6:
        seq_data_entry(s, t, e, value);
        break;
    case 7:
        snd_set_volume(owner_of(s, t), e->vab, e->prog[ch], value, e->pan[ch]);
        e->ch_vol[ch] = value;
        e->delta = read_delta(e);
        break;
    case 10:
        snd_set_volume(owner_of(s, t), e->vab, e->prog[ch], (u16)e->ch_vol[ch], value);
        e->pan[ch] = value;
        e->delta = read_delta(e);
        break;
    case 98:
        seq_nrpn_lsb(e, value);
        break;
    case 99:
        seq_nrpn_msb(e, value);
        break;
    default:
        /* CC 11, 64, 91, 100, 101, 121 have handlers in LIBSND; none occurs on the disc (FORMATS "SEP") */
        e->delta = read_delta(e);
        break;
    }
}

static void seq_note(int s, int t, SndSeq *e, u8 note, u8 vel) {
    int ch = e->channel;

    if (vel != 0) {
        if (!((e->mute >> ch) & 1)) {
            snd_key_on(owner_of(s, t), e->vab, e->prog[ch], note, vel, e->pan[ch]);
        }
    } else {
        snd_key_off(owner_of(s, t), e->vab, e->prog[ch], note);
    }
}

/* One event. Returns 1 at the end of the track. */
static int seq_event(int s, int t, SndSeq *e) {
    u8 b = *e->pos++;
    u8 kind, x, y;

    if (b & 0x80) {
        e->channel = b & 0xF;
        kind = b & 0xF0;
        switch (kind) {
        case 0x90:
            e->status = kind;
            x = *e->pos++;
            y = *e->pos++;
            e->delta = read_delta(e);
            seq_note(s, t, e, x, y);
            return 0;
        case 0xB0:
            e->status = kind;
            x = *e->pos++;
            seq_control(s, t, e, x);
            return 0;
        case 0xC0:
            e->status = kind;
            e->prog[e->channel] = *e->pos++;
            e->delta = read_delta(e);
            return 0;
        case 0xE0:
            e->status = kind;
            e->pos++; /* the LSB */
            break;
        case 0xF0:
            e->status = 0xFF;
            x = *e->pos++;
            if (x == 0x2F) {
                seq_end_of_track(s, t, e);
                return 1;
            }
            seq_tempo(e);
            return 0;
        default:
            return 0;
        }
    } else {
        switch (e->status) {
        case 0x90:
            y = *e->pos++;
            e->delta = read_delta(e);
            seq_note(s, t, e, b, y);
            return 0;
        case 0xB0:
            seq_control(s, t, e, b);
            return 0;
        case 0xC0:
            e->prog[e->channel] = b;
            e->delta = read_delta(e);
            return 0;
        case 0xE0:
            break; /* b was the LSB */
        case 0xFF:
            if (b == 0x2F) {
                seq_end_of_track(s, t, e);
                return 1;
            }
            seq_tempo(e);
            return 0;
        default:
            return 0;
        }
    }
    /* Pitch bend: the MSB */
    x = *e->pos++;
    snd_pitch_bend(owner_of(s, t), e->vab, e->prog[e->channel], x);
    e->delta = read_delta(e);
    return 0;
}

/* One tick of a playing sequence: the events whose time has come. */
static void seq_play(int s, int t, SndSeq *e) {
    s32 acc;

    if (e->delta - (s16)e->step > 0) {
        if (e->frame_wait > 0) {
            e->frame_wait--;
        } else if (e->frame_wait == 0) {
            e->frame_wait = (s16)e->step;
            e->delta--;
        } else {
            e->delta -= (s16)e->step;
        }
        return;
    }
    acc = e->delta;
    do {
        do {
            seq_event(s, t, e);
        } while (e->delta == 0);
        acc += e->delta;
    } while (acc < (s16)e->step);
    e->delta = acc - (s16)e->step;
}

/* ---- Decrescendo ---- */

static void seq_fade(int s, int t, SndSeq *e) {
    s32 change;
    int l, r;

    e->fade_count++;
    if (e->fade_time < e->fade_count) {
        e->flags &= ~SND_SEQ_CRESCENDO;
    } else {
        change = e->fade_target * e->fade_count / e->fade_time - e->fade_done;
        if (change != 0) {
            e->fade_done = (s16)(e->fade_done + change);
            l = e->vol_l + change;
            if (l >= 0x80) {
                l = 0x7F;
            }
            if (l < 0) {
                l = 0;
            }
            r = e->vol_r + change;
            if (r >= 0x80) {
                r = 0x7F;
            }
            if (r < 0) {
                r = 0;
            }
            snd_set_seq_volume(owner_of(s, t), (u16)l, (u16)r);
            if ((l == 0x7F && r == 0x7F) || (l == 0 && r == 0)) {
                e->flags &= ~SND_SEQ_CRESCENDO;
            }
        }
    }
    e->vol_l2 = e->vol_l;
    e->vol_r2 = e->vol_r;
}

/* ---- Play, stop, close ---- */

void snd_seq_stop(int s, int t) {
    SndSeq *e = seq_at(s, t);
    int ch;

    if (e == NULL) {
        return;
    }
    e->flags &= ~(SND_SEQ_PLAYING | SND_SEQ_PAUSED | SND_SEQ_REPLAY | 0x400);
    e->flags |= SND_SEQ_STOPPED;
    snd_owner_key_off(owner_of(s, t));
    snd.release_add = 0;
    e->play_mode = 0;
    e->elapsed = 0;
    e->loop_pending = 0;
    e->rpn_count = 0;
    e->nrpn_lsb = e->nrpn_msb = 0;
    e->nrpn_count = 0;
    e->channel = 0;
    e->played = 0;
    e->loop_count = 0;
    e->loop_active = 0;
    e->status = 0;
    e->delta = e->first_delta;
    e->bpm = e->bpm0;
    e->step = e->step0;
    e->pos = e->start;
    e->loop = e->start;
    for (ch = 0; ch < 16; ch++) {
        e->prog[ch] = (u8)ch;
        e->pan[ch] = 0x40;
        e->ch_vol[ch] = 0x7F;
    }
    e->vol_l2 = e->vol_r2 = 0x7F;
}

/* Mode 1 plays (`count` times, 0 = endless) with the sequence's volume applied to its notes; mode 0 pauses. */
void snd_seq_play(int s, int t, int mode, int count) {
    SndSeq *e = seq_at(s, t);

    if (e == NULL) {
        return;
    }
    e->pos = e->start;
    e->loop = e->start;
    e->flags &= ~(SND_SEQ_ENDED | SND_SEQ_STOPPED);
    e->play_count = (s8)count;
    if (mode == 1) {
        e->flags |= SND_SEQ_PLAYING;
        e->play_mode = (u8)mode;
        e->played = 0;
        snd_set_seq_volume(owner_of(s, t), e->vol_l, e->vol_r);
    } else if (mode == 0) {
        e->flags |= SND_SEQ_PAUSED;
    }
}

/* SsSepClose: sequence 0's volume set to 0 (its notes' volumes too) and its notes keyed off, the access number free,
 * every sequence's state cleared. */
void snd_seq_close(int s) {
    int t;

    snd_set_seq_volume(owner_of(s, 0), 0, 0);
    snd_owner_key_off(owner_of(s, 0));
    snd.open_mask &= ~(1u << s);
    for (t = 0; t < snd.t_max; t++) {
        SndSeq *e = seq_at(s, t);
        if (e != NULL) {
            seq_reset_entry(e);
        }
    }
}

/* SsSepSetVol: applied to the notes only while the sequence is playing and nothing else (flags exactly 1). */
void snd_seq_set_volume(int s, int t, s16 left, s16 right) {
    SndSeq *e = seq_at(s, t);

    if (e == NULL) {
        return;
    }
    if (e->flags != SND_SEQ_PLAYING) {
        e->vol_l = (u16)left;
        e->vol_r = (u16)right;
        return;
    }
    snd_set_seq_volume(owner_of(s, t), (u16)left, (u16)right);
}

/* SsSepSetDecrescendo: the volume falls by `vol` over `time` ticks (from the next tick on). */
void snd_seq_decrescendo(int s, int t, int vol, int time) {
    SndSeq *e = seq_at(s, t);

    if (e == NULL) {
        return;
    }
    if (!(e->flags & SND_SEQ_STOPPED) && !(e->flags & 0x100) && (s16)-vol != 0) {
        e->fade_target = (s16)-vol;
        e->fade_time = time;
        e->fade_count = 0;
        e->fade_done = 0;
    }
    e->flags |= SND_SEQ_DECRESCENDO;
    e->flags &= ~SND_SEQ_CRESCENDO;
}

/* The sequencer's tick (after the flush): every playing sequence of every open SEP, then the stops. */
void snd_seq_tick(void) {
    int s, t;

    for (s = 0; s < snd.s_max; s++) {
        if (!(snd.open_mask & (1u << s))) {
            continue;
        }
        for (t = 0; t < snd.t_max; t++) {
            SndSeq *e = seq_at(s, t);
            if (e == NULL) {
                continue;
            }
            if (e->flags & SND_SEQ_PLAYING) {
                seq_play(s, t, e);
                if (e->flags & SND_SEQ_CRESCENDO) {
                    seq_fade(s, t, e);
                }
                if (e->flags & SND_SEQ_DECRESCENDO) {
                    seq_fade(s, t, e);
                }
            }
            if (e->flags & SND_SEQ_PAUSED) {
                snd_owner_key_off(owner_of(s, t));
                e->play_mode = 0;
                e->flags &= ~SND_SEQ_PAUSED;
            }
            if (e->flags & SND_SEQ_REPLAY) {
                e->play_mode = 1;
                e->flags &= ~SND_SEQ_REPLAY;
            }
            if (e->flags & SND_SEQ_STOPPED) {
                snd_seq_stop(s, t);
                e->flags = 0;
            }
        }
    }
}

/* ---- Reverb (SsUtSetReverbType/Depth; also NRPN attributes 15 and 16) ---- */

s16 snd_reverb_type(s16 type) {
    int clear = 0;
    s16 t = type;

    if (type & 0x8000) {
        clear = 1;
        t = (s16)-type;
    }
    if ((u16)t >= 10) {
        return -1;
    }
    if (t == 0) {
        snd_spu_reverb_enable(0);
    }
    snd_spu_set_reverb_type(clear ? (t | 0x100) : t);
    return t;
}

void snd_reverb_depth(s16 left, s16 right) {
    snd_spu_set_reverb_depth((s16)(left * 0x7FFF / 127), (s16)(right * 0x7FFF / 127));
}
