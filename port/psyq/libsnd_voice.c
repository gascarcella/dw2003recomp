/* port/psyq/libsnd_voice.c: LIBSND's voice manager: which of the 24 voices a note gets, its volume and pitch, the
 * key-ons and key-offs, and the flush once a tick that writes them to the SPU.
 *
 * Nothing reaches the SPU when a note is keyed: the voice's registers are noted (SndShadow) with the key masks, and
 * snd_flush, at the start of every sequencer tick, writes the changed voices (pitch, volume, start address, ADSR:
 * LIBSPU's order, voice by voice), then KOFF, KON, EON and NON, always all eight halves. It also reads every voice's
 * envelope: a voice that was keyed off is reusable once its envelope reads 0, and LIBSND forgets a note once the
 * envelope has read 0 at 15 flushes running (the idle ring).
 *
 * The arithmetic is LIBSND's, step for step (the divisions truncate, the products are 32-bit; docs/SOUND.md "LIBSND"):
 *   velocity  v = note velocity x channel volume / 127 (sequence notes)
 *   volume    a = v x (VAB volume x 16383) / 16129, then x program volume x tone volume / 16129, then
 *             x sequence volume (L, R) / 127 (sequence notes); each pan (tone, program, note: 0..127, 64 centre)
 *             scales one side by pan / 63 or (127 - pan) / 63; sequence notes end squared: x / 16383.
 *   pitch     the note against the tone's centre note and fine tune: 2^(semitones / 12) from LIBSND's two tables
 *             (12 semitones, 128 fine steps), 0x1000 = the sample's own rate, at most 0x3FFF. */
#include <string.h>

#include "libsnd_internal.h"

/* LIBSND's pitch tables (the EXE's 0x8005C0E0 and 0x8005C0F8): one octave of semitones and one semitone in 128 steps,
 * as fractions of 0x10000 (0x8000 = 1). Not quite 2^(k/12) x 0x8000 rounded either way: the PS1's own numbers. */
static const u16 snd_semitone[12] = { 0x8000, 0x879C, 0x8FAC, 0x9837, 0xA145, 0xAADC,
                                      0xB504, 0xBFC8, 0xCB2F, 0xD744, 0xE411, 0xF1A1 };
static const u16 snd_finetone[128] = {
    0x8000, 0x800E, 0x801D, 0x802C, 0x803B, 0x804A, 0x8058, 0x8067, 0x8076, 0x8085, 0x8094, 0x80A3, 0x80B1, 0x80C0,
    0x80CF, 0x80DE, 0x80ED, 0x80FC, 0x810B, 0x811A, 0x8129, 0x8138, 0x8146, 0x8155, 0x8164, 0x8173, 0x8182, 0x8191,
    0x81A0, 0x81AF, 0x81BE, 0x81CD, 0x81DC, 0x81EB, 0x81FA, 0x8209, 0x8218, 0x8227, 0x8236, 0x8245, 0x8254, 0x8263,
    0x8272, 0x8282, 0x8291, 0x82A0, 0x82AF, 0x82BE, 0x82CD, 0x82DC, 0x82EB, 0x82FA, 0x830A, 0x8319, 0x8328, 0x8337,
    0x8346, 0x8355, 0x8364, 0x8374, 0x8383, 0x8392, 0x83A1, 0x83B0, 0x83C0, 0x83CF, 0x83DE, 0x83ED, 0x83FD, 0x840C,
    0x841B, 0x842A, 0x843A, 0x8449, 0x8458, 0x8468, 0x8477, 0x8486, 0x8495, 0x84A5, 0x84B4, 0x84C3, 0x84D3, 0x84E2,
    0x84F1, 0x8501, 0x8510, 0x8520, 0x852F, 0x853E, 0x854E, 0x855D, 0x856D, 0x857C, 0x858B, 0x859B, 0x85AA, 0x85BA,
    0x85C9, 0x85D9, 0x85E8, 0x85F8, 0x8607, 0x8617, 0x8626, 0x8636, 0x8645, 0x8655, 0x8664, 0x8674, 0x8683, 0x8693,
    0x86A2, 0x86B2, 0x86C1, 0x86D1, 0x86E0, 0x86F0, 0x8700, 0x870F, 0x871F, 0x872E, 0x873E, 0x874E, 0x875D, 0x876D,
    0x877D, 0x878C,
};

#define SND_NO_VOICE 0x63 /* the allocator's "none yet" (out of range for the callers) */

static u32 voice_bit(int v) {
    return 1u << v;
}

/* The score entry a note's owner names (NULL for SsUtKeyOn's notes or outside the table). */
static SndSeq *owner_seq(s16 owner) {
    return snd_seq(owner & 0xFF, ((int)owner & 0xFF00) >> 8);
}

/* A pan applied to one side: below 64 the right side shrinks, from 64 the left. */
static void pan_u32(u32 *l, u32 *r, u32 pan) {
    if (pan < 0x40) {
        *r = *r * pan / 63;
    } else {
        *l = *l * (0x7F - pan) / 63;
    }
}

/* ---- Pitch ---- */

/* The SPU pitch of `note` (fine: 1/128 semitone) for a tone recorded at `center` (+ `shift` / 128). */
static u16 pitch_from_note(int note, int fine, int center, int shift) {
    int sum = (s16)((shift & 0xFF) + fine);
    int semis_up = sum / 128;           /* toward zero */
    int rest = (s16)(sum - semis_up * 128);
    int frac = rest;
    int semis = note + semis_up - (center & 0xFF);
    int octave, step;
    u32 p;

    if (rest < 0) {
        frac = rest + 0x80;
        semis = semis - 1 + (s16)frac / 128;
    }
    semis = (s16)semis;
    octave = semis / 12;                /* toward zero, then corrected below */
    step = (s16)(semis - octave * 12);
    octave -= 2;
    if (step < 0) {
        step += 12;
        octave -= 1;
    }
    p = ((u32)snd_semitone[step] * snd_finetone[(s16)frac]) >> 16;
    if ((s16)octave >= 0) {
        return 0x3FFF;
    }
    octave = -(s16)octave;
    return (u16)((p + (1u << ((octave - 1) & 31))) >> (octave & 31)); /* the MIPS shifts use 5 bits */
}

/* A sequence note's pitch: the selected tone's centre, its fine tune at most 0x7F. */
static u16 note_pitch(void) {
    int shift = snd.cur.shift >= 0x80 ? 0x7F : snd.cur.shift;
    return pitch_from_note((s8)snd.cur.note, 0, snd.cur.center, shift);
}

/* A note with a fine offset, on the tone the selected block and snd.cur.tone name. */
static u16 note_pitch_fine(int note, int fine) {
    const u8 *tone = snd_tone(snd.cur.vab_data, (s16)(snd.cur.block * 16 + (s8)snd.cur.tone));
    return pitch_from_note((s16)note, (s16)fine, tone[4], tone[5]);
}

/* ---- Set-up and the flush ---- */

void snd_voices_init(int count) {
    int v;

    snd_spu_set_in_transfer(0);
    snd.release_add = 0;
    memset(snd.shadow, 0, sizeof(snd.shadow));
    snd.vab_count = 0;
    for (v = 0; v < SND_VABS; v++) {
        snd.vab[v].state = 0;
    }
    snd.nvoices = count >= SND_VOICES ? SND_VOICES : count;
    for (v = 0; v < snd.nvoices; v++) {
        SndVoice *vo = &snd.voice[v];
        vo->age = 0x18;
        vo->vag = 0xFF;
        vo->status = 0;
        vo->pitch = 0;
        vo->env = 0;
        vo->owner = -1;
        vo->block = 0;
        vo->prog = 0;
        vo->tone = 0xFF;
        vo->vel_raw = 0;
        vo->channel = 0;
        vo->pan = 0x40;
        vo->vel = 0;
        snd_spu_voice_attr(v, SND_ATTR_PITCH | SND_ATTR_VOL | SND_ATTR_ADDR | SND_ATTR_ADSR, 0, 0, 0x1000, 0x1000 >> 3,
                           0x80FF, 0x4000);
        snd_key_off_now(v);
    }
    /* The key-off mask's high half is left as it is: its voices 16..23 go out with the first flush. */
    snd.kon[0] = snd.kon[1] = 0;
    snd.koff[0] = 0;
    snd.eon[0] = snd.eon[1] = 0;
    snd.non[0] = snd.non[1] = 0;
    snd.mono = 0;
    snd_flush();
}

void snd_flush(void) {
    u32 idle, mask, bits;
    int v, i;

    snd.ring_pos = (snd.ring_pos + 1) & 15;
    snd.idle_ring[snd.ring_pos] = 0;
    for (v = 0; v < snd.nvoices; v++) {
        snd.voice[v].env = snd_spu_envelope(v);
        if (snd.voice[v].env == 0) {
            snd.idle_ring[snd.ring_pos] |= voice_bit(v);
        }
    }
    /* A note whose envelope has read 0 for 15 flushes is over (the ring's last entry is not looked at). */
    idle = 0xFFFFFFFFu;
    for (i = 0; i < 15; i++) {
        idle &= snd.idle_ring[i];
    }
    for (v = 0; v < snd.nvoices; v++) {
        if (idle & voice_bit(v)) {
            snd.voice[v].status = 0;
        }
    }
    snd.kon[0] &= (u16)~snd.koff[0];
    snd.kon[1] &= (u16)~snd.koff[1];
    for (v = 0; v < SND_VOICES; v++) {
        SndShadow *sh = &snd.shadow[v];
        if (sh->dirty) {
            snd_spu_voice_attr(v, sh->dirty, sh->vol_l, sh->vol_r, sh->pitch, sh->addr, sh->adsr1, sh->adsr2);
        }
        sh->dirty = 0;
    }
    snd_spu_set_key(0, ((u32)(snd.koff[1] & 0xFF) << 16) | snd.koff[0]);
    snd_spu_set_key(1, ((u32)(snd.kon[1] & 0xFF) << 16) | snd.kon[0]);
    mask = 0xFFFFFFu >> (24 - snd.nvoices);
    bits = (((u32)snd.eon[1] << 16) | snd.eon[0]) & mask;
    snd_spu_set_voice_mask(SND_REG_EON, bits, ~mask & 0xFFFFFF);
    bits = (((u32)snd.non[1] << 16) | snd.non[0]) & mask;
    snd_spu_set_voice_mask(SND_REG_NON, bits, ~mask & 0xFFFFFF);
    snd.koff[0] = snd.koff[1] = 0;
    snd.kon[0] = snd.kon[1] = 0;
    snd.non[0] = snd.non[1] = 0;
}

/* ---- Allocation ---- */

/* The voice for a note of priority snd.cur.prio: the first one that is free with a silent envelope; else the one of
 * lowest priority below it; among equal priorities the quietest, then the oldest. snd.nvoices when none. */
static int snd_alloc(void) {
    int pick = SND_NO_VOICE, best = SND_NO_VOICE, candidates = 0;
    u16 best_prio = (u16)(s8)snd.cur.prio;
    u16 best_env = 0xFFFF, best_age = 0;
    int v;

    for (v = 0; v < snd.nvoices; v++) {
        const SndVoice *vo = &snd.voice[v];
        if (vo->status == 0 && vo->env == 0) {
            pick = v;
            break;
        }
        if (vo->prio < (s32)best_prio) {
            best_prio = (u16)vo->prio;
            best = v;
            best_env = vo->env;
            best_age = vo->age;
            candidates = 1;
        } else if (vo->prio == (s32)best_prio) {
            candidates++;
            if (vo->env < best_env) {
                best_age = vo->age;
                best_env = vo->env;
                best = v;
            } else if (vo->env == best_env && best_age < vo->age) {
                best_age = vo->age;
                best = v;
            }
        }
    }
    if (pick == SND_NO_VOICE) {
        pick = (candidates & 0xFF) ? best : snd.nvoices;
    }
    if (pick < snd.nvoices) {
        for (v = 0; v < snd.nvoices; v++) {
            snd.voice[v].age++;
        }
        snd.voice[pick].age = 0;
        snd.voice[pick].prio = (s8)snd.cur.prio;
    }
    return pick;
}

/* The voice taken: its envelope counts as sounding, its start address (the VAG's) and ADSR (the tone's, the release
 * slowed by snd.release_add) due at the next flush. */
static void snd_take(int voice) {
    SndShadow *sh = &snd.shadow[voice];
    const SndVab *vab = snd.cur.vab_data;
    const u8 *tone;
    int i, entry, rel;
    u16 adsr2;

    snd.voice[voice].env = 0x7FFF;
    for (i = 0; i < 16; i++) {
        snd.idle_ring[i] &= ~voice_bit(voice);
    }
    entry = snd.cur.vag == 0 ? 1 : snd.cur.vag - 1;
    sh->addr = vab->vag_end[entry & 0xFF];
    sh->dirty |= SND_ATTR_ADDR;
    tone = snd_tone(vab, snd.cur.block * 16 + snd.cur.tone);
    sh->adsr1 = (u16)(tone[0x10] | tone[0x11] << 8);
    adsr2 = (u16)(tone[0x12] | tone[0x13] << 8);
    rel = (s16)(snd.release_add + (adsr2 & 0x1F));
    if (rel >= 0x20) {
        rel = 0x1F;
    }
    sh->adsr2 = (u16)(rel | (adsr2 & 0xFFE0));
    sh->dirty |= SND_ATTR_ADSR;
}

/* The selected note on snd.cur.voice at `pitch`: its volume, the key-on, reverb as the tone's mode says. */
static void snd_key_on_now(u16 pitch) {
    const SndVab *vab = snd.cur.vab_data;
    SndShadow *sh = &snd.shadow[snd.cur.voice];
    s32 a;
    u32 b, l, r, lo, hi;
    int v = snd.cur.voice;

    a = (s32)(s8)snd.cur.vel * (s32)(vab->head[0x18] * 16383) / 16129;
    b = (u32)(a * (s8)snd.cur.prog_vol * (s8)snd.cur.tone_vol) / 16129;
    l = r = b;
    if (snd.cur.owner != SND_SE_OWNER && owner_seq(snd.cur.owner) != NULL) {
        const SndSeq *seq = owner_seq(snd.cur.owner);
        l = b * seq->vol_l / 127;
        r = b * seq->vol_r / 127;
    }
    pan_u32(&l, &r, snd.cur.tone_pan);
    pan_u32(&l, &r, snd.cur.prog_pan);
    pan_u32(&l, &r, snd.cur.pan);
    if (snd.mono == 1) {
        if (l < r) {
            l = r;
        } else {
            r = l;
        }
    }
    if (snd.cur.owner != SND_SE_OWNER) {
        l = l * l / 16383;
        r = r * r / 16383;
    }
    sh->pitch = pitch;
    sh->vol_l = (u16)l;
    sh->vol_r = (u16)r;
    sh->dirty |= SND_ATTR_VOL | SND_ATTR_PITCH;
    snd.voice[v].pitch = pitch;
    lo = v < 16 ? voice_bit(v) : 0;
    hi = v < 16 ? 0 : voice_bit(v - 16);
    if (snd.cur.mode & 4) {
        snd.eon[0] |= (u16)lo;
        snd.eon[1] |= (u16)hi;
    } else {
        snd.eon[0] &= (u16)~lo;
        snd.eon[1] &= (u16)~hi;
    }
    snd.non[0] &= (u16)~lo;
    snd.non[1] &= (u16)~hi;
    snd.kon[0] |= (u16)lo;
    snd.kon[1] |= (u16)hi;
    snd.koff[0] &= (u16)~snd.kon[0];
    snd.koff[1] &= (u16)~snd.kon[1];
}

/* Noise voices (a tone whose VAG is 0xFF): no VAB on this disc has one; the PS1 would key the noise generator. */
static void snd_noise(int voice, int on) {
    PSYQ_TRACE("libsnd: noise voice %d %s (not modelled)", voice, on ? "on" : "off");
}

/* ---- Key on / key off ---- */

void snd_key_off_now(int voice) {
    u32 lo = voice < 16 ? voice_bit(voice) : 0;
    u32 hi = voice < 16 ? 0 : voice_bit(voice - 16);

    snd.cur.voice = (s16)voice;
    snd.voice[voice].status = 0;
    snd.voice[voice].pitch = 0;
    snd.voice[voice].vag = 0;
    snd.koff[0] |= (u16)lo;
    snd.kon[0] &= (u16)~snd.koff[0];
    snd.koff[1] |= (u16)hi;
    snd.kon[1] &= (u16)~snd.koff[1];
}

/* A sequence note: every tone of the program whose key range holds it gets a voice. Returns the voices' bits, or -1
 * when a tone found none (or the VAB/program is unusable). */
int snd_key_on(s16 owner, s16 vab, s16 prog, u8 note, u16 vel, u16 pan) {
    const SndSeq *seq = owner_seq(owner);
    const u8 *rec;
    u8 vags[16], tones[16];
    int n = 0, k, i, result = 0;

    if (snd_select(vab, prog) != 0 || (owner != SND_SE_OWNER && seq == NULL)) {
        return -1;
    }
    snd.cur.owner = owner;
    snd.cur.note = note;
    snd.cur.fine = 0;
    if (owner == SND_SE_OWNER) {
        snd.cur.vel = (u8)vel;
    } else {
        snd.cur.vel = (u8)((s32)vel * seq->ch_vol[seq->channel] / 127);
    }
    snd.cur.pan = (u8)pan;
    rec = snd_prog(snd.cur.vab_data, prog);
    snd.cur.prog_vol = rec[1];
    snd.cur.prog_pan = rec[4];
    snd.cur.tones = rec[0];
    if (!(snd.cur.block < (s32)(u16)(snd.cur.vab_data->head[0x12] | snd.cur.vab_data->head[0x13] << 8))) {
        return -1;
    }
    if (vel == 0) {
        return snd_key_off(owner, vab, prog, note);
    }
    for (k = 0; k < (s8)snd.cur.tones && n < 16; k++) {
        const u8 *t = snd_tone(snd.cur.vab_data, snd.cur.block * 16 + k);
        if ((s8)snd.cur.note >= t[6] && t[7] >= (s8)snd.cur.note) {
            vags[n] = t[0x16];
            tones[n] = (u8)k;
            n++;
        }
    }
    for (i = 0; i < n; i++) {
        const u8 *t;
        int v;

        snd.cur.vag = vags[i];
        snd.cur.tone = tones[i];
        t = snd_tone(snd.cur.vab_data, (u16)((s8)snd.cur.tone + snd.cur.block * 16));
        snd.cur.prio = t[0];
        snd.cur.tone_vol = t[2];
        snd.cur.tone_pan = t[3];
        snd.cur.center = t[4];
        snd.cur.shift = t[5];
        snd.cur.mode = t[1];
        v = snd_alloc();
        snd.cur.voice = (s16)v;
        if (v < snd.nvoices) {
            SndVoice *vo = &snd.voice[v];
            vo->status = 1;
            vo->age = 0;
            vo->owner = owner;
            vo->vab = snd.cur.vab;
            vo->block = snd.cur.block;
            vo->prog = prog;
            if (owner != SND_SE_OWNER) {
                vo->vel_raw = (s16)vel;
                vo->channel = seq->channel;
            }
            vo->pan = (u8)pan;
            vo->vel = (s8)snd.cur.vel;
            vo->tone = (s8)snd.cur.tone;
            vo->note = note;
            vo->prio = (s8)snd.cur.prio;
            vo->vag = snd.cur.vag;
            snd_take(v);
            if (snd.cur.vag == 0xFF) {
                snd_noise(v, 1);
            } else {
                snd_key_on_now(note_pitch());
            }
            result |= (int)voice_bit(snd.cur.voice);
        } else {
            result = -1;
        }
    }
    return result;
}

/* A sequence note's end: every voice still holding it (owner, VAB, program, note) is keyed off. */
int snd_key_off(s16 owner, s16 vab, s16 prog, u8 note) {
    int v, count = 0;

    for (v = 0; v < snd.nvoices; v++) {
        const SndVoice *vo = &snd.voice[v];
        if ((u16)vo->note == note && vo->prog == prog && vo->owner == owner && vo->vab == vab) {
            if (vo->vag == 0xFF) {
                count++;
                snd_noise(v, 0);
                v++; /* LIBSND skips the next voice here */
            } else {
                snd_key_off_now(v);
                count++;
            }
        }
    }
    return count;
}

void snd_owner_key_off(s16 owner) {
    int v;

    for (v = 0; v < snd.nvoices; v++) {
        if (snd.voice[v].owner == owner) {
            snd_key_off_now(v);
        }
    }
}

/* ---- Volume changes of sounding notes ---- */

/* A sequence's volume: stored (at most 127) and applied to its notes' voices. LIBSND selects each voice's VAB with the
 * voice's tone block as the program (its own slip), which only matters for the stale selection it leaves. */
s16 snd_set_seq_volume(s16 owner, u16 left, u16 right) {
    SndSeq *seq = owner_seq(owner);
    int v;

    if (seq == NULL) {
        return owner;
    }
    seq->vol_l = left >= 0x7F ? 0x7F : left;
    seq->vol_r = right >= 0x7F ? 0x7F : right;
    for (v = 0; v < snd.nvoices; v++) {
        SndVoice *vo = &snd.voice[v];
        const SndVab *vab;
        const u8 *tone, *prog;
        s32 vel, a;
        u32 b, l, r;

        if (vo->owner != owner || vo->vab != seq->vab) {
            continue;
        }
        snd_select(vo->vab, vo->block);
        vab = snd.cur.vab_data;
        tone = snd_tone(vab, vo->block * 16 + vo->tone);
        prog = snd_prog(vab, vo->prog);
        vel = vo->vel_raw * seq->ch_vol[vo->channel] / 127;
        a = (s32)vab->head[0x18] * (vel * 16383) / 16129;
        b = (u32)(a * prog[1] * tone[2]) / 16129;
        l = b * seq->vol_l / 127;
        r = b * seq->vol_r / 127;
        pan_u32(&l, &r, tone[3]);
        l &= 0xFFFF;
        r &= 0xFFFF;
        pan_u32(&l, &r, prog[4]);
        l &= 0xFFFF;
        r &= 0xFFFF;
        pan_u32(&l, &r, vo->pan);
        l &= 0xFFFF;
        r &= 0xFFFF;
        if (snd.mono == 1) {
            if (l < r) {
                l = r;
            } else {
                r = l;
            }
        }
        snd.shadow[v].vol_l = (u16)((s32)(l * l) / 16383);
        snd.shadow[v].vol_r = (u16)((s32)(r * r) / 16383);
        snd.shadow[v].dirty |= SND_ATTR_VOL;
    }
    return owner;
}

/* Control change 7 (volume) or 10 (pan) of a channel: its program's sounding notes get `vol` and `pan`. */
int snd_set_volume(s16 owner, s16 vab, s16 prog, u16 vol, u16 pan) {
    SndSeq *seq = owner_seq(owner);
    int v, count = 0;

    snd_select(vab, prog);
    snd.cur.owner = owner;
    if (pan == 0) {
        pan = 1;
    }
    if (vol == 0) {
        vol = 1;
    }
    for (v = 0; v < snd.nvoices; v++) {
        SndVoice *vo = &snd.voice[v];
        const SndVab *vab_data = snd.cur.vab_data;
        const u8 *tone;
        s32 vv, a;
        u32 b, l, r;

        if (vo->owner != owner || vo->prog != prog || vo->vab != vab || seq == NULL) {
            continue;
        }
        if (seq->ch_vol[seq->channel] != (s16)vol && seq->ch_vol[seq->channel] == 0) {
            seq->ch_vol[seq->channel] = 1;
        }
        tone = snd_tone(vab_data, vo->block * 16 + vo->tone);
        vv = vo->vel_raw * (s32)vol / 127;
        a = vv * (s32)(vab_data->head[0x18] * 16383) / 16129;
        b = (u32)(a * snd_prog(vab_data, prog)[1] * tone[2]) / 16129;
        l = b * seq->vol_l / 127;
        r = b * seq->vol_r / 127;
        pan_u32(&l, &r, tone[3]);
        pan_u32(&l, &r, snd_prog(vab_data, vo->prog)[4]);
        pan_u32(&l, &r, pan & 0xFF);
        if (snd.mono == 1) {
            if (l < r) {
                l = r;
            } else {
                r = l;
            }
        }
        snd.shadow[v].vol_l = (u16)(l * l / 16383);
        snd.shadow[v].vol_r = (u16)(r * r / 16383);
        snd.shadow[v].dirty |= SND_ATTR_VOL;
        count++;
    }
    return count;
}

/* ---- Pitch bend ---- */

/* `msb` 0..127 (64 = none) bends the channel's notes by up to the tone's range (pbmax up, pbmin down, semitones). */
int snd_pitch_bend(s16 owner, s16 vab, s16 prog, int msb) {
    int v, count = 0;
    s16 bend = (s16)(msb - 0x40);

    snd_select(vab, prog);
    snd.cur.owner = owner;
    for (v = 0; v < snd.nvoices; v++) {
        SndVoice *vo = &snd.voice[v];
        const u8 *tone;
        int note, fine, p, q;

        if (vo->owner != owner || vo->vab != vab || vo->prog != prog) {
            continue;
        }
        note = (u16)vo->note;
        tone = snd_tone(snd.cur.vab_data, (u16)((u16)vo->tone + snd.cur.block * 16));
        if (bend > 0) {
            p = bend * tone[0xD];
            q = p / 63;
            note += q;
            fine = (p - q * 63) * 2;
        } else if (bend < 0) {
            p = bend * tone[0xC];
            q = p / 64;
            note = note + q - 1;
            fine = (p - q * 64) * 2 + 0x7F;
        } else {
            fine = 0;
        }
        snd.cur.voice = (s16)v;
        snd.cur.tone = (u8)vo->tone;
        snd.shadow[v].pitch = note_pitch_fine((s16)(note & 0xFFFF), (s16)(fine & 0xFFFF));
        snd.shadow[v].dirty |= SND_ATTR_PITCH;
        count++;
    }
    return count;
}

/* ---- SsUtKeyOn / SsUtKeyOff / SsUtAllKeyOff ---- */

int snd_ut_key_on(s16 vab, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr) {
    const u8 *rec, *t;
    int v;

    if (snd.lock == 1) {
        return -1;
    }
    snd.lock = 1;
    if (snd_select(vab, prog) != 0) {
        snd.lock = 0;
        return -1;
    }
    snd.cur.owner = SND_SE_OWNER;
    snd.cur.note = (u8)note;
    snd.cur.fine = (u8)fine;
    snd.cur.tone = (u8)tone;
    if (voll == volr) {
        snd.cur.pan = 0x40;
        snd.cur.vel = (u8)voll;
    } else if (volr < voll) {
        snd.cur.pan = (u8)(((s32)volr << 6) / voll);
        snd.cur.vel = (u8)voll;
    } else {
        snd.cur.vel = (u8)volr;
        snd.cur.pan = (u8)(0x7F - ((s32)voll << 6) / volr);
    }
    rec = snd_prog(snd.cur.vab_data, prog);
    snd.cur.prog_vol = rec[1];
    snd.cur.prog_pan = rec[4];
    snd.cur.tones = rec[0];
    t = snd_tone(snd.cur.vab_data, (s16)((s8)snd.cur.tone + snd.cur.block * 16));
    snd.cur.prio = t[0];
    snd.cur.vag = (u16)(t[0x16] | t[0x17] << 8);
    snd.cur.tone_vol = t[2];
    snd.cur.tone_pan = t[3];
    snd.cur.center = t[4];
    snd.cur.shift = t[5];
    snd.cur.mode = t[1];
    if ((s16)snd.cur.vag == 0 || (v = snd_alloc() & 0xFF) == snd.nvoices) {
        snd.lock = 0;
        return -1;
    }
    snd.cur.voice = (s16)v;
    snd.voice[v].owner = SND_SE_OWNER;
    snd.voice[v].vab = vab;
    snd.voice[v].prog = prog;
    snd.voice[v].block = snd.cur.block;
    snd.voice[v].vag = snd.cur.vag;
    snd.voice[v].note = note;
    snd.voice[v].tone = (s8)snd.cur.tone;
    snd.voice[v].vel = (s8)snd.cur.vel;
    snd.voice[v].status = 1;
    snd.voice[v].age = 0;
    snd.voice[v].pan = snd.cur.pan;
    snd_take(v);
    if ((s16)snd.cur.vag == 0xFF) {
        snd_noise(v, 1);
    } else {
        snd_key_on_now(note_pitch_fine(note, fine));
    }
    snd.lock = 0;
    return v;
}

/* LIBSND keys the voice off whatever it holds now (the arguments only pick the noise case). */
int snd_ut_key_off(s16 voice, s16 vab, s16 prog, s16 tone, s16 note) {
    const SndVoice *vo;

    if (snd.lock == 1) {
        return -1;
    }
    snd.lock = 1;
    if ((u16)voice >= SND_VOICES) {
        snd.lock = 0;
        return -1;
    }
    vo = &snd.voice[voice];
    if (vo->vab == vab && vo->prog == prog && vo->tone == tone && vo->note == note && vo->vag == 0xFF) {
        snd_noise(voice, 0);
    } else {
        snd_key_off_now(voice);
    }
    snd.lock = 0;
    return 0;
}

/* Every voice silenced at once (its registers written now, not at the flush) and keyed off. */
void snd_all_key_off(void) {
    int v;

    for (v = 0; v < snd.nvoices; v++) {
        SndVoice *vo = &snd.voice[v];
        vo->age = 0x18;
        vo->env = 0;
        vo->owner = 0xFF;
        vo->block = 0;
        vo->prog = 0;
        vo->tone = 0xFF;
        vo->vel = 0;
        snd_spu_voice_attr(v, SND_ATTR_PITCH | SND_ATTR_VOL | SND_ATTR_ADDR | SND_ATTR_ADSR, 0, 0, 0x1000, 0x1000 >> 3,
                           0x80FF, 0x4000);
        snd_key_off_now(v);
    }
}
