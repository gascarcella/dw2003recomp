"""pad_random: the 4,096-entry random table and its index (src/main/pad.c, include/pad.h).

pad_random_seed(s) sets index = s & 0xFFF; pad_random_next() advances the index (mod 4096) and returns
pad_random_table[index] (a u16; the overlays use the whole word). docs/MECHANICS.md section 1.
"""
from oracle import Call, Case, Read

COMMENT = ("pad_random_seed(seed) then 32 x pad_random_next(): the returned values and the index after each call. "
           "The 'table' case records the SHA-1 of pad_random_table as the game holds it in RAM.")
SEEDS = [0, 1, 2, 0x7FF, 0xFFE, 0xFFF, 0x1000, 0x1001, 0x12345, 0x7FFFFFFF, 0xFFFFFFFF]
DRAWS = 32


def cases(sym):
    index = Read("pad_random", 0, 4, "pad_random.index (s32)")
    out = [Case("table", [Call("pad_random_seed", [0], "void",
                               [Read("pad_random_table", 0, 0x2000, "pad_random_table[4096] (u16)")])],
                saves=[("pad_random", 4)], comment="the table itself, as a SHA-1 of its 8,192 bytes in RAM")]
    for seed in SEEDS:
        calls = [Call("pad_random_seed", [seed], "void", [index], comment="index = seed & 0xFFF")]
        calls += [Call("pad_random_next", [], "u16", [index]) for _ in range(DRAWS)]
        out.append(Case(f"seed_{seed:#x}", calls, saves=[("pad_random", 4)],
                        comment=f"seed {seed:#x}: the index after seeding, then {DRAWS} draws"))
    return out
