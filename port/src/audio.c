/* The audio output (M3; port_harness.h "audio.c", docs/SOUND.md): the SPU core's samples, rendered once per vsync and
 * sent to a WAV file and to SDL3's audio device.
 *
 * The rendering is the emulated hardware's, not the device's: every vsync (pump.c, after the game's vsync callback,
 * where LIBSND's per-tick flush writes the SPU) port_audio_frame renders exactly that vsync's samples with spu_render,
 * whether anything listens or not, because LIBSND reads the voices' envelopes back (ENVX) to allocate voices: the game
 * must not run differently with or without --wav or a device. Vsync n (from 0) gets
 *     floor((n + 1) * 44100 / rate) - floor(n * 44100 / rate)
 * stereo frames, rate = --fps (the vsyncs per second the window paces to; 50 by default, PAL), or 50 with --fps 0
 * (unthrottled): 882 per vsync at 50, 735 at 60 (the NTSC patch's rate: SsSetTickMode(0x1000) then ticks 60 times a
 * second), and a fractional rate is spread exactly. So the audio always lasts as long as the vsyncs do at their nominal
 * pace, and its pitch never changes; a PAL game paced at 60 plays its music 20 % faster, as such a console would. The
 * samples depend only on the SPU's register writes and the vsync count: two runs give the same bytes.
 *
 * - `--wav FILE` (any build, headless too): the samples as a RIFF WAV, 44,100 Hz, 2 channels, signed 16-bit
 *   little-endian; the header's sizes are written at port_audio_close (port_exit): a run that is killed leaves them 0.
 *   Its length is (vsyncs rendered) x 882 x 4 bytes + the 44-byte header at 50 Hz.
 * - The device (a build with -DDW3_PORT_SDL=ON, window mode, unless --mute): an SDL3 audio stream (SDL resamples to the
 *   device's own format), fed the same samples. The window's pace (pump.c: vsync n due at n / fps seconds of
 *   CLOCK_MONOTONIC) and the device's sample clock are two clocks: they drift apart (a few hundred ppm on real hardware;
 *   a slow host or a pause makes it seconds). The queue (the frames put into the stream and not yet played, measured
 *   before each vsync's put: the low point of its sawtooth) is held between watermarks, without ever touching the
 *   game's timing or the samples (the WAV and the trace never change):
 *   . drift: a moving average of the queue (1/32 per vsync) above `high` speeds the stream's playback up by 0.2 %
 *     (SDL_SetAudioStreamFrequencyRatio, 1.002: SDL resamples; 3.5 cents, inaudible) until it is back at `target`;
 *     below `low` slows it down by 0.2 % the same way. 0.2 % corrects 2 ms a second, ten times any crystal's drift.
 *   . starved (the queue is empty when a vsync's samples arrive: the host was late, or the run starts): they are
 *     queued after `target` frames of silence, so the next vsync finds the queue at the target again (a gap, counted
 *     as a "refill" but at the start; the device played silence anyway).
 *   . flooded (above `max`: --fps 0, or a device that stalled and then went on at its own pace, which the 0.2 % would
 *     take a minute to absorb): the vsync's samples are dropped (counted), a 20 ms cut.
 *   With the device's period P (the frames it pulls at once, SDL_GetAudioDeviceFormat) and V = one vsync's frames:
 *   target = P + 2V, low = target - V, high = target + V, max = target + 3V (at 50 Hz with P = 1024: 63, 43, 83 and
 *   123 ms). Every 10 s and at the end, the queue's minimum, maximum and mean go to the log (stderr), with the ratio
 *   changes, refills and drops. SDL's `disk` driver (SDL_AUDIO_DRIVER=disk, SDL_AUDIO_DISK_OUTPUT_FILE) plays in real
 *   time into a file: a headless test of this path. Without a device (none on the host, or SDL fails to open one) the
 *   run goes on silent, logged. */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "port_harness.h"
#include "port_runtime.h"
#include "spu.h"

#ifdef DW3_PORT_SDL
#include <SDL3/SDL.h>
#endif

#define AUDIO_WAV_HEADER 44
#define AUDIO_MAX_FRAMES SPU_RATE /* one vsync's frames at 1 fps, the slowest pace --fps allows */

static int16_t audio_buf[AUDIO_MAX_FRAMES * 2];
static long audio_rate;       /* vsyncs per second the samples follow */
static long long audio_vsync; /* vsyncs rendered */
static FILE *audio_wav;
static const char *audio_wav_path;
static unsigned long long audio_wav_bytes; /* sample bytes written */
static int audio_wav_full;

static void audio_put_le(unsigned char *p, uint32_t v, int bytes) {
    int i;
    for (i = 0; i < bytes; i++) {
        p[i] = (unsigned char)(v >> (8 * i));
    }
}

/* The RIFF header for `data_bytes` of samples. */
static void audio_wav_header(unsigned char h[AUDIO_WAV_HEADER], uint32_t data_bytes) {
    memcpy(h, "RIFF", 4);
    audio_put_le(h + 4, 36 + data_bytes, 4);
    memcpy(h + 8, "WAVEfmt ", 8);
    audio_put_le(h + 16, 16, 4);           /* the fmt chunk's size */
    audio_put_le(h + 20, 1, 2);            /* PCM */
    audio_put_le(h + 22, 2, 2);            /* channels */
    audio_put_le(h + 24, SPU_RATE, 4);     /* frames per second */
    audio_put_le(h + 28, SPU_RATE * 4, 4); /* bytes per second */
    audio_put_le(h + 32, 4, 2);            /* bytes per frame */
    audio_put_le(h + 34, 16, 2);           /* bits per sample */
    memcpy(h + 36, "data", 4);
    audio_put_le(h + 40, data_bytes, 4);
}

static void audio_wav_write(const int16_t *s, int frames) {
    static unsigned char out[AUDIO_MAX_FRAMES * 4];
    size_t bytes = (size_t)frames * 4;
    int i;
    if (audio_wav_bytes + bytes > 0xFFFFFFFFull - 36) {
        if (!audio_wav_full) {
            audio_wav_full = 1;
            port_log("audio: --wav %s: a WAV holds at most 4 GB (6.7 hours); the rest is not written", audio_wav_path);
        }
        return;
    }
    for (i = 0; i < frames * 2; i++) {
        audio_put_le(out + 2 * i, (uint16_t)s[i], 2);
    }
    if (fwrite(out, 1, bytes, audio_wav) != bytes) {
        port_fatal("audio: --wav %s: cannot write: %s", audio_wav_path, strerror(errno));
    }
    audio_wav_bytes += bytes;
}

/* ---- The device */
#ifdef DW3_PORT_SDL
#define AUDIO_RATIO_STEP 0.002f
#define AUDIO_REPORT_SEC 10

static SDL_AudioStream *audio_stream;
static int audio_vsync_frames; /* V: one vsync's frames, nominal */
static int audio_period;       /* P: the frames the device pulls at once */
static int audio_target, audio_low, audio_high, audio_max;
static double audio_avg;  /* the queue's moving average (frames) */
static int audio_nudge;   /* -1 slower, 0, +1 faster */
static int audio_started; /* the first vsync's samples were queued */
static struct {
    long long vsyncs;
    int min, max;
    double sum;
} audio_win, audio_all; /* the queue (frames, before each put): since the last report, and over the run */
static long audio_refills, audio_drops, audio_nudges;
static long long audio_dropped_frames, audio_silence_frames;

static void audio_stat_add(int q) {
    if (audio_win.vsyncs == 0 || q < audio_win.min) {
        audio_win.min = q;
    }
    if (audio_win.vsyncs == 0 || q > audio_win.max) {
        audio_win.max = q;
    }
    audio_win.sum += q;
    audio_win.vsyncs++;
    if (audio_all.vsyncs == 0 || q < audio_all.min) {
        audio_all.min = q;
    }
    if (audio_all.vsyncs == 0 || q > audio_all.max) {
        audio_all.max = q;
    }
    audio_all.sum += q;
    audio_all.vsyncs++;
}

static double audio_ms(double frames) {
    return frames * 1000.0 / SPU_RATE;
}

static void audio_report(const char *what, long long vsyncs, int min, int max, double sum) {
    port_log("audio: %s: queue min %.1f ms, max %.1f ms, mean %.1f ms over %lld vsyncs (band %.1f..%.1f ms, target "
             "%.1f); ratio %s, %ld ratio changes, %ld refills (%.1f ms of silence), %ld drops (%.1f ms)",
             what, audio_ms(min), audio_ms(max), vsyncs > 0 ? audio_ms(sum / (double)vsyncs) : 0.0, vsyncs,
             audio_ms(audio_low), audio_ms(audio_high), audio_ms(audio_target),
             audio_nudge > 0 ? "1.002" : audio_nudge < 0 ? "0.998" : "1", audio_nudges, audio_refills,
             audio_ms((double)audio_silence_frames), audio_drops, audio_ms((double)audio_dropped_frames));
}

static void audio_device_open(void) {
    SDL_AudioSpec spec, dev;
    int frames = 0;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        port_log("audio: no audio (SDL_InitSubSystem: %s); the run goes on silent", SDL_GetError());
        return;
    }
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = SPU_RATE;
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (audio_stream == NULL) {
        port_log("audio: no audio device (%s); the run goes on silent", SDL_GetError());
        return;
    }
    if (!SDL_GetAudioDeviceFormat(SDL_GetAudioStreamDevice(audio_stream), &dev, &frames) || frames <= 0) {
        dev.format = 0;
        dev.channels = 0;
        dev.freq = 0;
        frames = 1024;
    }
    audio_vsync_frames = (int)((SPU_RATE + audio_rate - 1) / audio_rate);
    /* the device's period in our frames (it pulls `frames` of its own rate) */
    audio_period = dev.freq > 0 ? (int)((long long)frames * SPU_RATE / dev.freq) + 1 : frames;
    audio_target = audio_period + 2 * audio_vsync_frames;
    audio_low = audio_target - audio_vsync_frames;
    audio_high = audio_target + audio_vsync_frames;
    audio_max = audio_target + 3 * audio_vsync_frames;
    audio_avg = audio_target;
    port_log("audio: SDL audio driver %s, device %d Hz %d channels, %d frames a period; queue target %.1f ms (band "
             "%.1f..%.1f, drop above %.1f)", SDL_GetCurrentAudioDriver(), dev.freq, dev.channels, frames,
             audio_ms(audio_target), audio_ms(audio_low), audio_ms(audio_high), audio_ms(audio_max));
    SDL_ResumeAudioStreamDevice(audio_stream);
}

static void audio_put(const void *data, int frames) {
    if (!SDL_PutAudioStreamData(audio_stream, data, frames * 4)) {
        port_log("audio: SDL_PutAudioStreamData: %s", SDL_GetError());
    }
}

static void audio_device_frame(const int16_t *s, int frames) {
    static const int16_t silence[2048 * 2];
    int q = SDL_GetAudioStreamQueued(audio_stream);
    int nudge = audio_nudge;
    q = q > 0 ? q / 4 : 0;
    if (audio_started) {
        audio_stat_add(q);
    }
    if (q > audio_max) {
        audio_drops++;
        audio_dropped_frames += frames;
    } else {
        if (q == 0) {
            /* the next vsync's queue (before its put) lands on the target: this put's frames are one vsync's */
            int fill = audio_target;
            if (audio_started) {
                audio_refills++; /* the first vsync's fill is the start, not a refill */
                audio_silence_frames += fill;
            }
            while (fill > 0) {
                int n = fill < 2048 ? fill : 2048;
                audio_put(silence, n);
                fill -= n;
            }
            audio_avg = audio_target;
            nudge = 0;
        }
        audio_put(s, frames);
    }
    audio_started = 1;
    audio_avg += ((double)q - audio_avg) / 32.0;
    if (audio_avg > audio_high) {
        nudge = 1;
    } else if (audio_avg < audio_low) {
        nudge = -1;
    } else if ((nudge > 0 && audio_avg <= audio_target) || (nudge < 0 && audio_avg >= audio_target)) {
        nudge = 0;
    }
    if (nudge != audio_nudge) {
        audio_nudge = nudge;
        audio_nudges++;
        SDL_SetAudioStreamFrequencyRatio(audio_stream, 1.0f + (float)nudge * AUDIO_RATIO_STEP);
    }
    if (audio_win.vsyncs >= (long long)audio_rate * AUDIO_REPORT_SEC) {
        char what[48];
        snprintf(what, sizeof(what), "vsync %lld", audio_vsync);
        audio_report(what, audio_win.vsyncs, audio_win.min, audio_win.max, audio_win.sum);
        audio_win.vsyncs = 0;
        audio_win.sum = 0;
    }
}

static void audio_device_close(void) {
    if (audio_stream != NULL) {
        audio_report("end", audio_all.vsyncs, audio_all.min, audio_all.max, audio_all.sum);
        SDL_DestroyAudioStream(audio_stream);
        audio_stream = NULL;
    }
}
#else
static void audio_device_open(void) {
    port_log("audio: this build has no audio device (configure with -DDW3_PORT_SDL=ON); the run goes on silent");
}

static void audio_device_close(void) {
}
#endif

void port_audio_open(int device, const char *wav_path) {
    unsigned char h[AUDIO_WAV_HEADER];
    audio_rate = port_fps > 0 ? port_fps : 50;
    if (audio_rate > SPU_RATE) {
        audio_rate = SPU_RATE;
    }
    if (wav_path != NULL) {
        audio_wav_path = wav_path;
        audio_wav = fopen(wav_path, "wb");
        if (audio_wav == NULL) {
            port_fatal("audio: --wav %s: cannot open: %s", wav_path, strerror(errno));
        }
        audio_wav_header(h, 0);
        if (fwrite(h, 1, sizeof(h), audio_wav) != sizeof(h)) {
            port_fatal("audio: --wav %s: cannot write: %s", wav_path, strerror(errno));
        }
    }
    if (device) {
        audio_device_open();
    }
}

void port_audio_frame(void) {
    long long n = audio_vsync;
    int frames = (int)((n + 1) * SPU_RATE / audio_rate - n * SPU_RATE / audio_rate);
    spu_render(audio_buf, frames);
    audio_vsync++;
    if (audio_wav != NULL) {
        audio_wav_write(audio_buf, frames);
    }
#ifdef DW3_PORT_SDL
    if (audio_stream != NULL) {
        audio_device_frame(audio_buf, frames);
    }
#endif
}

void port_audio_close(void) {
    audio_device_close();
    if (audio_wav != NULL) {
        unsigned char h[AUDIO_WAV_HEADER];
        audio_wav_header(h, (uint32_t)audio_wav_bytes);
        if (fseek(audio_wav, 0, SEEK_SET) != 0 || fwrite(h, 1, sizeof(h), audio_wav) != sizeof(h) ||
            fclose(audio_wav) != 0) {
            audio_wav = NULL;
            port_fatal("audio: --wav %s: cannot finish the header: %s", audio_wav_path, strerror(errno));
        }
        audio_wav = NULL;
        port_log("audio: --wav %s: %lld vsyncs, %llu frames (%.2f s)", audio_wav_path, audio_vsync,
                 audio_wav_bytes / 4, (double)audio_wav_bytes / 4 / SPU_RATE);
    }
}
