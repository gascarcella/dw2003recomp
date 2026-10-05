#ifndef PSYQ_GTEMAC_H
#define PSYQ_GTEMAC_H

/* GTE inline macros, as Psy-Q's inline_c.h writes them (the same names and registers: $12-$14 as
 * scratch, two nops before each command), and at the end the composite ones of Psy-Q's gtemac.h.
 * Written from the game's code (FIGHTSTG's model, mesh and battle functions); no SDK file is used.
 * GTE commands are emitted as raw cop2 words. One header for every unit: add macros here. */

/* Rotation matrix (and translation) from a MATRIX. */
#define gte_SetRotMatrix(r0)                                                                       \
    __asm__ volatile("lw $12, 0(%0);"                                                              \
                     "lw $13, 4(%0);"                                                              \
                     "ctc2 $12, $0;"                                                               \
                     "ctc2 $13, $1;"                                                               \
                     "lw $12, 8(%0);"                                                              \
                     "lw $13, 12(%0);"                                                             \
                     "lw $14, 16(%0);"                                                             \
                     "ctc2 $12, $2;"                                                               \
                     "ctc2 $13, $3;"                                                               \
                     "ctc2 $14, $4"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* Translation vector from a MATRIX. */
#define gte_SetTransMatrix(r0)                                                                     \
    __asm__ volatile("lw $12, 20(%0);"                                                             \
                     "lw $13, 24(%0);"                                                             \
                     "ctc2 $12, $5;"                                                               \
                     "lw $14, 28(%0);"                                                             \
                     "ctc2 $13, $6;"                                                               \
                     "ctc2 $14, $7"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* Light matrix from a MATRIX. */
#define gte_SetLightMatrix(r0)                                                                     \
    __asm__ volatile("lw $12, 0(%0);"                                                              \
                     "lw $13, 4(%0);"                                                              \
                     "ctc2 $12, $8;"                                                               \
                     "ctc2 $13, $9;"                                                               \
                     "lw $12, 8(%0);"                                                              \
                     "lw $13, 12(%0);"                                                             \
                     "lw $14, 16(%0);"                                                             \
                     "ctc2 $12, $10;"                                                              \
                     "ctc2 $13, $11;"                                                              \
                     "ctc2 $14, $12"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* V0 from a VECTOR (its low halfwords). */
#define gte_ldlv0(r0)                                                                              \
    __asm__ volatile("lhu $13, 4(%0);"                                                             \
                     "lhu $12, 0(%0);"                                                             \
                     "sll $13, $13, 16;"                                                           \
                     "or $12, $12, $13;"                                                           \
                     "mtc2 $12, $0;"                                                               \
                     "lwc2 $1, 8(%0)"                                                              \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13")

/* IR1-3 from a column of a MATRIX. */
#define gte_ldclmv(r0)                                                                             \
    __asm__ volatile("lhu $12, 0(%0);"                                                             \
                     "lhu $13, 6(%0);"                                                             \
                     "lhu $14, 12(%0);"                                                            \
                     "mtc2 $12, $9;"                                                               \
                     "mtc2 $13, $10;"                                                              \
                     "mtc2 $14, $11"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* V0 from an SVECTOR. */
#define gte_ldv0(r0) __asm__ volatile("lwc2 $0, 0(%0);" "lwc2 $1, 4(%0)" : : "r"(r0))

/* V0 from an SVECTOR that may not be word-aligned (lwl/lwr; no Psy-Q name known). */
#define gte_ldv0_u(r0)                                                                             \
    __asm__ volatile("lwl $12, 3(%0);"                                                             \
                     "lwr $12, 0(%0);"                                                             \
                     "lhu $13, 4(%0);"                                                             \
                     "mtc2 $12, $0;"                                                               \
                     "mtc2 $13, $1"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13")

/* IR0 (interpolation factor) from a value. */
#define gte_lddp(r0) __asm__ volatile("mtc2 %0, $8" : : "r"(r0))

/* IR1-3 from an SVECTOR. */
#define gte_ldsv(r0)                                                                               \
    __asm__ volatile("lhu $12, 0(%0);"                                                             \
                     "lhu $13, 2(%0);"                                                             \
                     "lhu $14, 4(%0);"                                                             \
                     "mtc2 $12, $9;"                                                               \
                     "mtc2 $13, $10;"                                                              \
                     "mtc2 $14, $11"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* GPF (sf = 1): IR1-3 = IR0 * IR1-3 >> 12. */
#define gte_gpf12() __asm__ volatile("nop;" "nop;" ".word 0x4B98003D")

/* MVMVA: rotation matrix x IR1-3 (sf = 1, no translation). */
#define gte_rtir() __asm__ volatile("nop;" "nop;" ".word 0x4A49E012")

/* MVMVA: rotation matrix x V0 + translation (sf = 1). */
#define gte_rt() __asm__ volatile("nop;" "nop;" ".word 0x4A480012")

/* NCS: normal colour of V0. */
#define gte_ncs() __asm__ volatile("nop;" "nop;" ".word 0x4AC8041E")

/* SXY0-2 from three words. */
#define gte_ldsxy3(r0, r1, r2)                                                                     \
    __asm__ volatile("mtc2 %0, $12;" "mtc2 %2, $14;" "mtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))

/* SZ1-3 from three values. */
#define gte_ldsz3(r0, r1, r2)                                                                      \
    __asm__ volatile("mtc2 %0, $17;" "mtc2 %1, $18;" "mtc2 %2, $19" : : "r"(r0), "r"(r1), "r"(r2))

/* SZ0-3 from four values. */
#define gte_ldsz4(r0, r1, r2, r3)                                                                  \
    __asm__ volatile("mtc2 %0, $16;" "mtc2 %1, $17;" "mtc2 %2, $18;" "mtc2 %3, $19"                \
                     :                                                                             \
                     : "r"(r0), "r"(r1), "r"(r2), "r"(r3))

/* NCLIP: normal clipping of SXY0-2 (MAC0). */
#define gte_nclip() __asm__ volatile("nop;" "nop;" ".word 0x4B400006")

/* AVSZ3 / AVSZ4: average Z (OTZ). */
#define gte_avsz3() __asm__ volatile("nop;" "nop;" ".word 0x4B58002D")
#define gte_avsz4() __asm__ volatile("nop;" "nop;" ".word 0x4B68002E")

/* RTPS: perspective transform of V0. */
#define gte_rtps() __asm__ volatile("nop;" "nop;" ".word 0x4A180001")

/* MVMVA: rotation matrix x V0, no translation (MAC1-3). */
#define gte_rtv0() __asm__ volatile("nop;" "nop;" ".word 0x4A486012")

/* SXY2 to a word. */
#define gte_stsxy(r0) __asm__ volatile("swc2 $14, 0(%0)" : : "r"(r0) : "memory")

/* IR1-3 to an SVECTOR. */
#define gte_stsv(r0)                                                                               \
    __asm__ volatile("mfc2 $12, $9;"                                                               \
                     "mfc2 $13, $10;"                                                              \
                     "mfc2 $14, $11;"                                                              \
                     "sh $12, 0(%0);"                                                              \
                     "sh $13, 2(%0);"                                                              \
                     "sh $14, 4(%0)"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14", "memory")

/* IR1-3 to a column of a MATRIX. */
#define gte_stclmv(r0)                                                                             \
    __asm__ volatile("mfc2 $12, $9;"                                                               \
                     "mfc2 $13, $10;"                                                              \
                     "mfc2 $14, $11;"                                                              \
                     "sh $12, 0(%0);"                                                              \
                     "sh $13, 6(%0);"                                                              \
                     "sh $14, 12(%0)"                                                              \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14", "memory")

/* MAC0 (NCLIP's result) to a word. */
#define gte_stopz(r0) __asm__ volatile("swc2 $24, 0(%0)" : : "r"(r0) : "memory")

/* OTZ to a word. */
#define gte_stotz(r0) __asm__ volatile("swc2 $7, 0(%0)" : : "r"(r0) : "memory")

/* RGB2 to a CVECTOR. */
#define gte_strgb(r0) __asm__ volatile("swc2 $22, 0(%0)" : : "r"(r0) : "memory")

/* MAC1-3 to a VECTOR. */
#define gte_stlvnl(r0)                                                                             \
    __asm__ volatile("swc2 $25, 0(%0);" "swc2 $26, 4(%0);" "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

/* SZ3 / 4 (the average-Z scale for one point) to a word. */
#define gte_stszotz(r0)                                                                            \
    __asm__ volatile("mfc2 $12, $19;" "nop;" "sra $12, $12, 2;" "sw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

/* FLAG (control register 31). */
#define gte_stflg(r0)                                                                              \
    __asm__ volatile("cfc2 $12, $31;" "nop;" "sw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

/* Composite macros (Psy-Q's gtemac.h). */

/* r3 = r1 x r2 (rotation parts). */
#define gte_MulMatrix0(r1, r2, r3)                                                                 \
    {                                                                                              \
        gte_SetRotMatrix(r1);                                                                      \
        gte_ldclmv(r2);                                                                            \
        gte_rtir();                                                                                \
        gte_stclmv(r3);                                                                            \
        gte_ldclmv((char *)(r2) + 2);                                                              \
        gte_rtir();                                                                                \
        gte_stclmv((char *)(r3) + 2);                                                              \
        gte_ldclmv((char *)(r2) + 4);                                                              \
        gte_rtir();                                                                                \
        gte_stclmv((char *)(r3) + 4);                                                              \
    }

/* r3 = r1 x r2, translation included. */
#define gte_CompMatrix(r1, r2, r3)                                                                 \
    {                                                                                              \
        gte_MulMatrix0(r1, r2, r3);                                                                \
        gte_SetTransMatrix(r1);                                                                    \
        gte_ldlv0((r2)->t);                                                                        \
        gte_rt();                                                                                  \
        gte_stlvnl((r3)->t);                                                                       \
    }

#endif /* PSYQ_GTEMAC_H */
