#include "common.h"

#include "records.h"
#include "cdload.h"
#include "filetable.h"
#include "heap.h"
#include "psyq/libsnd.h"
#include "sound.h"

/* Sub-file `n` of file `file`, as cdload takes it. */
#define SOUND_SUBFILE(file, n) (((file) << 16) | (n))

/* The sound banks (SoundBank; the first has two SEPs). */
s32 sound_bank_01[] = { 0x180, 0x173, SOUND_SUBFILE(0x173, 0), SOUND_SUBFILE(0x180, 0), SOUND_SUBFILE(0x173, 1), SOUND_SUBFILE(0x173, 2), 0 };
SoundBank sound_bank_02 = { 0x177, 0x16A, SOUND_SUBFILE(0x16A, 0), SOUND_SUBFILE(0x177, 0), { SOUND_SUBFILE(0x16A, 1), 0 } };
SoundBank sound_bank_03 = { 0x472, 0x470, SOUND_SUBFILE(0x470, 0), SOUND_SUBFILE(0x472, 0), { SOUND_SUBFILE(0x470, 1), 0 } };
SoundBank sound_bank_04 = { 0x178, 0x16B, SOUND_SUBFILE(0x16B, 0), SOUND_SUBFILE(0x178, 0), { SOUND_SUBFILE(0x16B, 1), 0 } };
SoundBank sound_bank_05 = { 0x179, 0x16C, SOUND_SUBFILE(0x16C, 0), SOUND_SUBFILE(0x179, 0), { SOUND_SUBFILE(0x16C, 1), 0 } };
SoundBank sound_bank_06 = { 0x211, 0x20F, SOUND_SUBFILE(0x20F, 0), SOUND_SUBFILE(0x211, 0), { SOUND_SUBFILE(0x20F, 1), 0 } };
SoundBank sound_bank_07 = { 0x17A, 0x16D, SOUND_SUBFILE(0x16D, 0), SOUND_SUBFILE(0x17A, 0), { SOUND_SUBFILE(0x16D, 1), 0 } };
SoundBank sound_bank_08 = { 0x17B, 0x16E, SOUND_SUBFILE(0x16E, 0), SOUND_SUBFILE(0x17B, 0), { SOUND_SUBFILE(0x16E, 1), 0 } };
SoundBank sound_bank_09 = { 0x17C, 0x16F, SOUND_SUBFILE(0x16F, 0), SOUND_SUBFILE(0x17C, 0), { SOUND_SUBFILE(0x16F, 1), 0 } };
SoundBank sound_bank_10 = { 0x219, 0x216, SOUND_SUBFILE(0x216, 0), SOUND_SUBFILE(0x219, 0), { SOUND_SUBFILE(0x216, 1), 0 } };
SoundBank sound_bank_11 = { 0x17D, 0x170, SOUND_SUBFILE(0x170, 0), SOUND_SUBFILE(0x17D, 0), { SOUND_SUBFILE(0x170, 1), 0 } };
SoundBank sound_bank_12 = { 0x17E, 0x171, SOUND_SUBFILE(0x171, 0), SOUND_SUBFILE(0x17E, 0), { SOUND_SUBFILE(0x171, 1), 0 } };
SoundBank sound_bank_13 = { 0x22B, 0x225, SOUND_SUBFILE(0x225, 0), SOUND_SUBFILE(0x22B, 0), { SOUND_SUBFILE(0x225, 1), 0 } };
SoundBank sound_bank_14 = { 0x238, 0x237, SOUND_SUBFILE(0x237, 0), SOUND_SUBFILE(0x238, 0), { SOUND_SUBFILE(0x237, 1), 0 } };
SoundBank sound_bank_15 = { 0x242, 0x241, SOUND_SUBFILE(0x241, 0), SOUND_SUBFILE(0x242, 0), { SOUND_SUBFILE(0x241, 1), 0 } };
SoundBank sound_bank_16 = { 0x247, 0x246, SOUND_SUBFILE(0x246, 0), SOUND_SUBFILE(0x247, 0), { SOUND_SUBFILE(0x246, 1), 0 } };
SoundBank sound_bank_17 = { 0x25B, 0x25A, SOUND_SUBFILE(0x25A, 0), SOUND_SUBFILE(0x25B, 0), { SOUND_SUBFILE(0x25A, 1), 0 } };
SoundBank sound_bank_18 = { 0x261, 0x260, SOUND_SUBFILE(0x260, 0), SOUND_SUBFILE(0x261, 0), { SOUND_SUBFILE(0x260, 1), 0 } };
SoundBank sound_bank_19 = { 0x299, 0x298, SOUND_SUBFILE(0x298, 0), SOUND_SUBFILE(0x299, 0), { SOUND_SUBFILE(0x298, 1), 0 } };
SoundBank sound_bank_20 = { 0x274, 0x273, SOUND_SUBFILE(0x273, 0), SOUND_SUBFILE(0x274, 0), { SOUND_SUBFILE(0x273, 1), 0 } };
SoundBank sound_bank_21 = { 0x2CC, 0x2C8, SOUND_SUBFILE(0x2C8, 0), SOUND_SUBFILE(0x2CC, 0), { SOUND_SUBFILE(0x2C8, 1), 0 } };
SoundBank sound_bank_22 = { 0x214, 0x213, SOUND_SUBFILE(0x213, 0), SOUND_SUBFILE(0x214, 0), { SOUND_SUBFILE(0x213, 1), 0 } };
SoundBank sound_bank_23 = { 0x334, 0x333, SOUND_SUBFILE(0x333, 0), SOUND_SUBFILE(0x334, 0), { SOUND_SUBFILE(0x333, 1), 0 } };
SoundBank sound_bank_24 = { 0x22C, 0x227, SOUND_SUBFILE(0x227, 0), SOUND_SUBFILE(0x22C, 0), { SOUND_SUBFILE(0x227, 1), 0 } };
SoundBank sound_bank_25 = { 0x2CD, 0x2C9, SOUND_SUBFILE(0x2C9, 0), SOUND_SUBFILE(0x2CD, 0), { SOUND_SUBFILE(0x2C9, 1), 0 } };
SoundBank sound_bank_26 = { 0x2CE, 0x2CA, SOUND_SUBFILE(0x2CA, 0), SOUND_SUBFILE(0x2CE, 0), { SOUND_SUBFILE(0x2CA, 1), 0 } };
SoundBank sound_bank_27 = { 0x17F, 0x172, SOUND_SUBFILE(0x172, 0), SOUND_SUBFILE(0x17F, 0), { SOUND_SUBFILE(0x172, 1), 0 } };
SoundBank sound_bank_28 = { 0x438, 0x436, SOUND_SUBFILE(0x436, 0), SOUND_SUBFILE(0x438, 0), { SOUND_SUBFILE(0x436, 1), 0 } };
SoundBank sound_bank_29 = { 0x27C, 0x27B, SOUND_SUBFILE(0x27B, 0), SOUND_SUBFILE(0x27C, 0), { SOUND_SUBFILE(0x27B, 1), 0 } };
SoundBank sound_bank_30 = { 0x2CF, 0x2CB, SOUND_SUBFILE(0x2CB, 0), SOUND_SUBFILE(0x2CF, 0), { SOUND_SUBFILE(0x2CB, 1), 0 } };
SoundBank sound_bank_31 = { 0x212, 0x210, SOUND_SUBFILE(0x210, 0), SOUND_SUBFILE(0x212, 0), { SOUND_SUBFILE(0x210, 1), 0 } };
SoundBank sound_bank_32 = { 0x203, 0x201, SOUND_SUBFILE(0x201, 0), SOUND_SUBFILE(0x203, 0), { SOUND_SUBFILE(0x201, 1), 0 } };
SoundBank sound_bank_33 = { 0x2DF, 0x2DE, SOUND_SUBFILE(0x2DE, 0), SOUND_SUBFILE(0x2DF, 0), { SOUND_SUBFILE(0x2DE, 1), 0 } };
SoundBank sound_bank_34 = { 0x22D, 0x228, SOUND_SUBFILE(0x228, 0), SOUND_SUBFILE(0x22D, 0), { SOUND_SUBFILE(0x228, 1), 0 } };
SoundBank sound_bank_35 = { 0x473, 0x471, SOUND_SUBFILE(0x471, 0), SOUND_SUBFILE(0x473, 0), { SOUND_SUBFILE(0x471, 1), 0 } };
SoundBank sound_bank_36 = { 0x560, 0x55F, SOUND_SUBFILE(0x55F, 0), SOUND_SUBFILE(0x560, 0), { SOUND_SUBFILE(0x55F, 1), 0 } };
SoundBank sound_bank_37 = { 0x7CA, 0x7AC, SOUND_SUBFILE(0x7AC, 0), SOUND_SUBFILE(0x7CA, 0), { SOUND_SUBFILE(0x7AC, 1), 0 } };
SoundBank sound_bank_38 = { 0x7CB, 0x7C7, SOUND_SUBFILE(0x7C7, 0), SOUND_SUBFILE(0x7CB, 0), { SOUND_SUBFILE(0x7C7, 1), 0 } };
SoundBank sound_bank_39 = { 0x45C, 0x45B, SOUND_SUBFILE(0x45B, 0), SOUND_SUBFILE(0x45C, 0), { SOUND_SUBFILE(0x45B, 1), 0 } };
SoundBank sound_bank_40 = { 0x4CB, 0x4CA, SOUND_SUBFILE(0x4CA, 0), SOUND_SUBFILE(0x4CB, 0), { SOUND_SUBFILE(0x4CA, 1), 0 } };
SoundBank sound_bank_41 = { 0x181, 0x174, SOUND_SUBFILE(0x174, 0), SOUND_SUBFILE(0x181, 0), { SOUND_SUBFILE(0x174, 1), 0 } };
SoundBank sound_bank_42 = { 0x360, 0x35E, SOUND_SUBFILE(0x35E, 0), SOUND_SUBFILE(0x360, 0), { SOUND_SUBFILE(0x35E, 1), 0 } };
SoundBank sound_bank_43 = { 0x361, 0x35F, SOUND_SUBFILE(0x35F, 0), SOUND_SUBFILE(0x361, 0), { SOUND_SUBFILE(0x35F, 1), 0 } };
SoundBank sound_bank_44 = { 0x22E, 0x229, SOUND_SUBFILE(0x229, 0), SOUND_SUBFILE(0x22E, 0), { SOUND_SUBFILE(0x229, 1), 0 } };
SoundBank sound_bank_45 = { 0x182, 0x175, SOUND_SUBFILE(0x175, 0), SOUND_SUBFILE(0x182, 0), { SOUND_SUBFILE(0x175, 1), 0 } };
SoundBank sound_bank_46 = { 0x381, 0x37F, SOUND_SUBFILE(0x37F, 0), SOUND_SUBFILE(0x381, 0), { SOUND_SUBFILE(0x37F, 1), 0 } };
SoundBank sound_bank_47 = { 0x382, 0x380, SOUND_SUBFILE(0x380, 0), SOUND_SUBFILE(0x382, 0), { SOUND_SUBFILE(0x380, 1), 0 } };
SoundBank sound_bank_48 = { 0x3AF, 0x3AA, SOUND_SUBFILE(0x3AA, 0), SOUND_SUBFILE(0x3AF, 0), { SOUND_SUBFILE(0x3AA, 1), 0 } };
SoundBank sound_bank_49 = { 0x3B0, 0x3AB, SOUND_SUBFILE(0x3AB, 0), SOUND_SUBFILE(0x3B0, 0), { SOUND_SUBFILE(0x3AB, 1), 0 } };
SoundBank sound_bank_50 = { 0x183, 0x176, SOUND_SUBFILE(0x176, 0), SOUND_SUBFILE(0x183, 0), { SOUND_SUBFILE(0x176, 1), 0 } };
SoundBank sound_bank_51 = { 0x204, 0x202, SOUND_SUBFILE(0x202, 0), SOUND_SUBFILE(0x204, 0), { SOUND_SUBFILE(0x202, 1), 0 } };
SoundBank sound_bank_52 = { 0x3B1, 0x3AC, SOUND_SUBFILE(0x3AC, 0), SOUND_SUBFILE(0x3B1, 0), { SOUND_SUBFILE(0x3AC, 1), 0 } };
SoundBank sound_bank_53 = { 0x3B2, 0x3AD, SOUND_SUBFILE(0x3AD, 0), SOUND_SUBFILE(0x3B2, 0), { SOUND_SUBFILE(0x3AD, 1), 0 } };
SoundBank sound_bank_54 = { 0x3B3, 0x3AE, SOUND_SUBFILE(0x3AE, 0), SOUND_SUBFILE(0x3B3, 0), { SOUND_SUBFILE(0x3AE, 1), 0 } };
SoundBank sound_bank_55 = { 0x3BE, 0x3BB, SOUND_SUBFILE(0x3BB, 0), SOUND_SUBFILE(0x3BE, 0), { SOUND_SUBFILE(0x3BB, 1), 0 } };
SoundBank sound_bank_56 = { 0x3BF, 0x3BC, SOUND_SUBFILE(0x3BC, 0), SOUND_SUBFILE(0x3BF, 0), { SOUND_SUBFILE(0x3BC, 1), 0 } };
SoundBank sound_bank_57 = { 0x3C0, 0x3BD, SOUND_SUBFILE(0x3BD, 0), SOUND_SUBFILE(0x3C0, 0), { SOUND_SUBFILE(0x3BD, 1), 0 } };
SoundBank sound_bank_58 = { 0x3F6, 0x3F0, SOUND_SUBFILE(0x3F0, 0), SOUND_SUBFILE(0x3F6, 0), { SOUND_SUBFILE(0x3F0, 1), 0 } };
SoundBank sound_bank_59 = { 0x3F7, 0x3F1, SOUND_SUBFILE(0x3F1, 0), SOUND_SUBFILE(0x3F7, 0), { SOUND_SUBFILE(0x3F1, 1), 0 } };
SoundBank sound_bank_60 = { 0x3F8, 0x3F2, SOUND_SUBFILE(0x3F2, 0), SOUND_SUBFILE(0x3F8, 0), { SOUND_SUBFILE(0x3F2, 1), 0 } };
SoundBank sound_bank_61 = { 0x3F9, 0x3F3, SOUND_SUBFILE(0x3F3, 0), SOUND_SUBFILE(0x3F9, 0), { SOUND_SUBFILE(0x3F3, 1), 0 } };
SoundBank sound_bank_62 = { 0x3FA, 0x3F4, SOUND_SUBFILE(0x3F4, 0), SOUND_SUBFILE(0x3FA, 0), { SOUND_SUBFILE(0x3F4, 1), 0 } };
SoundBank sound_bank_63 = { 0x3FB, 0x3F5, SOUND_SUBFILE(0x3F5, 0), SOUND_SUBFILE(0x3FB, 0), { SOUND_SUBFILE(0x3F5, 1), 0 } };
SoundBank sound_bank_64 = { 0x439, 0x437, SOUND_SUBFILE(0x437, 0), SOUND_SUBFILE(0x439, 0), { SOUND_SUBFILE(0x437, 1), 0 } };
SoundBank sound_bank_65 = { 0x63A, 0x639, SOUND_SUBFILE(0x639, 0), SOUND_SUBFILE(0x63A, 0), { SOUND_SUBFILE(0x639, 1), 0 } };
SoundBank sound_bank_66 = { 0x22F, 0x22A, SOUND_SUBFILE(0x22A, 0), SOUND_SUBFILE(0x22F, 0), { SOUND_SUBFILE(0x22A, 1), 0 } };
SoundBank sound_bank_67 = { 0x7A2, 0x7A1, SOUND_SUBFILE(0x7A1, 0), SOUND_SUBFILE(0x7A2, 0), { SOUND_SUBFILE(0x7A1, 1), 0 } };
SoundBank sound_bank_68 = { 0x838, 0x837, SOUND_SUBFILE(0x837, 0), SOUND_SUBFILE(0x838, 0), { SOUND_SUBFILE(0x837, 1), 0 } };
SoundBank sound_bank_69 = { 0x732, 0x730, SOUND_SUBFILE(0x730, 0), SOUND_SUBFILE(0x732, 0), { SOUND_SUBFILE(0x730, 1), 0 } };
SoundBank sound_bank_70 = { 0x733, 0x731, SOUND_SUBFILE(0x731, 0), SOUND_SUBFILE(0x733, 0), { SOUND_SUBFILE(0x731, 1), 0 } };
SoundBank sound_bank_71 = { 0x89B, 0x89A, SOUND_SUBFILE(0x89A, 1), SOUND_SUBFILE(0x89B, 0), { SOUND_SUBFILE(0x89A, 0), 0 } };

/* Sound bank by id (sound_load_bank); 0 is none. */
SoundBank *sound_banks[] = {
    NULL, (SoundBank *)sound_bank_01, &sound_bank_02, &sound_bank_03,
    &sound_bank_04, &sound_bank_05, &sound_bank_06, &sound_bank_07,
    &sound_bank_08, &sound_bank_09, &sound_bank_10, &sound_bank_11,
    &sound_bank_12, &sound_bank_13, &sound_bank_14, &sound_bank_15,
    &sound_bank_16, &sound_bank_17, &sound_bank_18, &sound_bank_19,
    &sound_bank_20, &sound_bank_21, &sound_bank_22, &sound_bank_23,
    &sound_bank_24, &sound_bank_25, &sound_bank_26, &sound_bank_27,
    &sound_bank_28, &sound_bank_29, &sound_bank_30, &sound_bank_31,
    &sound_bank_32, &sound_bank_33, &sound_bank_34, &sound_bank_35,
    &sound_bank_36, &sound_bank_37, &sound_bank_38, &sound_bank_39,
    &sound_bank_40, &sound_bank_41, &sound_bank_42, &sound_bank_43,
    &sound_bank_44, &sound_bank_45, &sound_bank_46, &sound_bank_47,
    &sound_bank_48, &sound_bank_49, &sound_bank_50, &sound_bank_51,
    &sound_bank_52, &sound_bank_53, &sound_bank_54, &sound_bank_55,
    &sound_bank_56, &sound_bank_57, &sound_bank_58, &sound_bank_59,
    &sound_bank_60, &sound_bank_61, &sound_bank_62, &sound_bank_63,
    &sound_bank_64, &sound_bank_65, &sound_bank_66, &sound_bank_67,
    &sound_bank_68, &sound_bank_69, &sound_bank_70, &sound_bank_71,
};

/* The three entries' buffers for the VAB header file, and their VAB bodies' SPU addresses. */
s32 sound_buffer_0[0xE000 / 4];
s32 sound_buffer_1[0xA000 / 4];
s32 sound_buffer_2[0xA000 / 4];
s32 *sound_buffers[3] = { sound_buffer_0, sound_buffer_1, sound_buffer_2 };
s32 sound_spu_addrs[3] = { 0x1010, 0x49C10, 0x62410 };

s32 sound_is_loading(void);
void sound_load_bank(s32 index, s32 id);
void sound_update_loading(void);

/* Index of the entry whose id is `id`, or -1. */
s32 sound_find_entry(s32 id) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (sound_module.entries[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* Plays sound `key`: (key >> 18) & 0x7F = entry id; bit 31 set: a note of the VAB (prog, tone, note
 * as in sound_key_off), clear: SEP (key >> 8) & 0xFF, sequence key & 0xFF. Bit 30: a "current" sound
 * that replaces the previous one (current). Returns the voice for a note, else -1; 0 on failure. */
s32 sound_play(s32 key) {
    s32 id = (key >> 18) & 0x7F;
    s32 is_note = (key >> 31) & 1;
    s32 replace = (key >> 30) & 1;
    s32 prog = (key >> 11) & 0x7F;
    s32 tone = (key >> 7) & 0xF;
    s32 note = key & 0x7F;
    s32 sep = (key >> 8) & 0xFF;
    s32 seq = key & 0xFF;
    s32 index = sound_find_entry(id);
    s32 voice = -1;

    if (index == -1) {
        return 0;
    }
    if (index != 0) {
        sound_module.extra_entry = index;
    }
    if (replace) {
        if (sound_module.current == key) {
            return 0;
        }
        if (sound_module.current != 0) {
            sound_module.stop(sound_module.current);
        }
        sound_module.current = key;
    }
    if (is_note) {
        voice = SsUtKeyOn(sound_module.entries[index].vab_id, prog, tone, note, 0, 0x7F, 0x7F);
    } else {
        SsSepStop(sound_module.entries[index].seps[sep], seq);
        SsSepSetVol(sound_module.entries[index].seps[sep], seq, 0x7F, 0x7F);
        SsSepPlay(sound_module.entries[index].seps[sep], seq, 1, 1);
    }
    return voice;
}

/* Stops every SEP sequence and every note. */
void sound_stop_all(void) {
    SoundEntry *entry;
    s32 i;
    s32 j;
    s32 seq;

    for (i = 0; i < 3; i++) {
        entry = &sound_module.entries[i];
        if (entry->vab_id != -1) {
            for (j = 0; j < entry->sep_count; j++) {
                for (seq = 0; seq < 16; seq++) {
                    SsSepStop(entry->seps[j], seq);
                }
            }
        }
    }
    SsUtAllKeyOff(0);
    sound_module.current = 0;
}

/* Stops SEP sound `key` (as in sound_play; notes are left alone). */
void sound_stop(s32 key) {
    s32 id = (key >> 18) & 0x7F;
    s32 is_note = (key >> 31) & 1;
    s32 sep = (key >> 8) & 0xFF;
    s32 seq = key & 0xFF;
    s32 index = sound_find_entry(id);

    if (index == -1 || is_note) {
        return;
    }
    SsSepStop(sound_module.entries[index].seps[sep], seq);
    if (sound_module.current == key) {
        sound_module.current = 0;
    }
}

/* Fades out SEP sound `key` (over 50 ticks at 50 Hz, 60 at 60 Hz). */
void sound_fade_out(s32 key) {
    s32 id = (key >> 18) & 0x7F;
    s32 is_note = (key >> 31) & 1;
    s32 sep = (key >> 8) & 0xFF;
    s32 seq = key & 0xFF;
    s32 index = sound_find_entry(id);

    if (index == -1 || is_note) {
        return;
    }
    SsSepSetDecrescendo(sound_module.entries[index].seps[sep], seq, 0x80, records_60hz != 0 ? 60 : 50);
    if (sound_module.current == key) {
        sound_module.current = 0;
    }
}

s32 sound_is_loading(void) {
    return sound_module.loading.step != 0;
}

/* Frees entry `index` and starts loading sound bank `id` into it (sound_update_loading does the rest). */
void sound_load_bank(s32 index, s32 id) {
    SoundEntry *entry = &sound_module.entries[index];
    SoundBankLoad *load = &sound_module.loading;
    s32 i;
    s32 seq;

    entry->id = id;
    if (entry->vab_id != -1) {
        for (i = 0; i < entry->sep_count; i++) {
            for (seq = 0; seq < 16; seq++) {
                SsSepStop(entry->seps[i], seq);
            }
            SsSepClose(entry->seps[i]);
        }
        SsVabClose(entry->vab_id);
        entry->vab_id = -1;
    }
    load->step = 1;
    load->bank = sound_banks[id];
    load->entry = index;
    cdload_module.queue_file(load->bank->header_file);
}

/* Unless `id` is already in entry 1 or 2, loads it into the one not used last. */
void sound_load_extra_bank(s32 id) {
    if (sound_module.entries[1].id == id || sound_module.entries[2].id == id) {
        return;
    }
    if (sound_module.extra_entry == 1) {
        sound_load_bank(2, id);
        sound_module.extra_entry = 2;
    } else {
        sound_load_bank(1, id);
        sound_module.extra_entry = 1;
    }
}

/* Called every frame: loads the bank sound_load_bank picked, one step per call (loading.step: 1 = wait
 * for the header file, copy it and open the VAB header; 2 = wait for the body file and transfer it
 * to the SPU; 3 = wait for the transfer, then open the SEPs). */
void sound_update_loading(void) {
    SoundBankLoad *load = &sound_module.loading;
    s32 index = load->entry;
    SoundEntry *entry = &sound_module.entries[index];
    s32 *src;
    s32 *dst;
    s32 count;
    s32 i;
    s32 j;

    switch (load->step) {
    case 0:
        return;
    case 1:
        if (cdload_module.is_loading(load->bank->header_file)) {
            return;
        }
        src = (s32 *)cdload_module.files.get_file(load->bank->header_file);
        dst = entry->header_buffer;
        count = filetable_funcs.get_sectors(load->bank->header_file) << 9;
        for (i = 0; i < count; i++) {
            *dst++ = *src++;
        }
        cdload_module.files.free_file(load->bank->header_file);
        entry->vab_id = SsVabOpenHeadSticky(cdload_module.get_subfile(load->bank->vab_header, entry->header_buffer), index,
                                            entry->spu_addr);
        cdload_module.queue_file(load->bank->body_file);
        load->step++;
    case 2:
        if (cdload_module.is_loading(load->bank->body_file)) {
            return;
        }
        heap_funcs.set_state(cdload_module.files.get_file(load->bank->body_file), 1);
        entry->vab_id = SsVabTransBody(cdload_module.get_subfile_by_id(load->bank->vab_body), entry->vab_id);
        load->step++;
    case 3:
        if (!SsVabTransCompleted(0)) {
            return;
        }
        heap_funcs.set_state(cdload_module.files.get_file(load->bank->body_file), 0);
        cdload_module.files.free_file(load->bank->body_file);
        for (j = 0; load->bank->seps[j] != 0; j++) {
            entry->seps[j] = SsSepOpen(cdload_module.get_subfile(load->bank->seps[j], entry->header_buffer),
                                         entry->vab_id, 16);
        }
        entry->sep_count = j;
        load->step = 0;
    }
}

s16 sound_key_on(s32 index, s16 prog, s16 note) {
    return SsUtKeyOn(sound_module.entries[index].vab_id, prog, 0, note, 0, 0x7F, 0x7F);
}

/* `key` packs (entry id << 18) | (prog << 11) | (tone << 7) | note. */
void sound_key_off(s32 key, s16 voice) {
    s32 id = (key >> 18) & 0x7F;
    s32 prog = (key >> 11) & 0x7F;
    s32 tone = (key >> 7) & 0xF;
    s32 note = key & 0x7F;
    s32 index = sound_find_entry(id);

    if (voice != -1 && index != -1) {
        SsUtKeyOff(voice, sound_module.entries[index].vab_id, prog, tone, note);
    }
}

/* Start-up: libsnd with sound_module.seq_table as its SEQ/SEP table, the three entries' buffers,
 * then sound bank 1 into entry 0, loaded before returning. */
void sound_init(void) {
    s32 i;

    SsSetTableSize(sound_module.seq_table, 6, 16);
    if (records_60hz != 0) {
        SsSetTickMode(0x1000);
    } else {
        SsSetTickMode(0x1032);
    }
    SsStart2();
    SsSetMVol(0x7F, 0x7F);
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
    SsUtSetReverbType(3);
    SsUtSetReverbDepth(0, 0);
    SsUtReverbOn();
    for (i = 0; i < 3; i++) {
        sound_module.entries[i].vab_id = -1;
        sound_module.entries[i].sep_count = 0;
        sound_module.entries[i].header_buffer = sound_buffers[i];
        sound_module.entries[i].spu_addr = sound_spu_addrs[i];
    }
    sound_module.loading.entry = 0;
    sound_module.loading.step = 0;
    sound_module.loading.bank = NULL;
    sound_load_bank(0, 1);
    while (sound_is_loading()) {
        cdload_module.update();
        sound_update_loading();
    }
}

SoundModule sound_module = {
    { 0 },
    { { 0 } },
    0,
    0,
    { NULL, 0, 0 },
    sound_init,
    sound_play,
    sound_key_on,
    sound_key_off,
    sound_load_extra_bank,
    sound_load_bank,
    sound_update_loading,
    sound_is_loading,
    sound_stop_all,
    sound_stop,
    sound_fade_out,
};
