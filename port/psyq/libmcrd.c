/* port/psyq/libmcrd.c: LIBMCRD. No memory card in the M1 skeleton: every asynchronous command is accepted (1) and
 * MemCardSync reports it finished with "no card" (result 1, McErrCardNotExist); the synchronous ones return the
 * same error. memcard.c then reports "no card" to its callers (memcard_wait_exist returns result + 1 = 2).
 *
 * Assumption to verify (M4, against the emulator): the command numbers MemCardSync reports in *cmds (memcard.c
 * stores them without reading them back) and that MemCardSync's result for a missing card is 1 for every command
 * (not, say, 2 "invalid card" for the file functions). */
#include "psyq_internal.h"
#include "psyq/libmcrd.h"

#define MCRD_NO_CARD 1

/* Command numbers (Psy-Q's McFunc*): 1 Exist, 2 Accept, 3 ReadFile, 4 WriteFile, 7 GetDirentry. */
static s32 psyq_mcrd_command;
static int psyq_mcrd_pending;

static s32 psyq_mcrd_start(const char *who, s32 chan, s32 command) {
    PSYQ_TRACE("%s chan %x", who, chan);
    psyq_mcrd_command = command;
    psyq_mcrd_pending = 1;
    return 1;
}

void MemCardInit(long val) {
    PSYQ_TRACE("MemCardInit %ld", val);
}

void MemCardStart(void) {
    PSYQ_TRACE("MemCardStart");
}

s32 MemCardExist(s32 chan) {
    return psyq_mcrd_start("MemCardExist", chan, 1);
}

s32 MemCardAccept(s32 chan) {
    return psyq_mcrd_start("MemCardAccept", chan, 2);
}

s32 MemCardReadFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) {
    PSYQ_TRACE("MemCardReadFile %s ofs %d bytes %d to %u", file, offset, bytes, PSYQ_PTR(addr));
    return psyq_mcrd_start("MemCardReadFile", chan, 3);
}

s32 MemCardWriteFile(s32 chan, char *file, u32 *addr, s32 offset, s32 bytes) {
    PSYQ_TRACE("MemCardWriteFile %s ofs %d bytes %d from %u", file, offset, bytes, PSYQ_PTR(addr));
    return psyq_mcrd_start("MemCardWriteFile", chan, 4);
}

/* Synchronous: the error at once. */
s32 MemCardCreateFile(s32 chan, char *file, s32 blocks) {
    PSYQ_TRACE("MemCardCreateFile chan %x %s blocks %d", chan, file, blocks);
    return MCRD_NO_CARD;
}

s32 MemCardFormat(s32 chan) {
    PSYQ_TRACE("MemCardFormat chan %x", chan);
    return MCRD_NO_CARD;
}

s32 MemCardUnformat(s32 chan) {
    PSYQ_TRACE("MemCardUnformat chan %x", chan);
    return MCRD_NO_CARD;
}

/* mode 0 waits, 1 polls: 1 = the command finished (cmds/result filled), 0 = still running, -1 = none pending. */
s32 MemCardSync(s32 mode, s32 *cmds, s32 *result) {
    (void)mode;
    if (!psyq_mcrd_pending) {
        return -1;
    }
    psyq_mcrd_pending = 0;
    if (cmds != NULL) {
        *cmds = psyq_mcrd_command;
    }
    if (result != NULL) {
        *result = MCRD_NO_CARD;
    }
    PSYQ_TRACE("MemCardSync: command %d done, no card", psyq_mcrd_command);
    return 1;
}

s32 MemCardGetDirentry(s32 chan, char *name, DIRENTRY *dir, s32 *files, s32 offset, s32 max) {
    PSYQ_TRACE("MemCardGetDirentry chan %x %s ofs %d max %d", chan, name, offset, max);
    (void)dir;
    if (files != NULL) {
        *files = 0;
    }
    return MCRD_NO_CARD;
}

/* LIBMCRD's part of the console's reset (psyq.h). Session 16 stub: T6 ("memcard") implements it. */
void psyq_mcrd_reset(void) {
}
