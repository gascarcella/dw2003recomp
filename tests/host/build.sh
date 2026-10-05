#!/usr/bin/env bash
# Builds the host-side replay harness: the game's C units for this machine (the port's compiler flags: -m64 -std=gnu99
# -fwrapv -fsigned-char -fno-strict-aliasing -DNON_MATCHING, INCLUDE_ASM empty, the GTE stubbed) plus the shims and the
# symbol table replay.py generated. Usage: tests/host/build.sh <symtab.c> <out binary>
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SYMTAB="$1"; OUT="$2"
UNITS=(src/main/pad.c src/main/memcard.c src/main/gamestate.c src/main/records.c src/main/card.c
       src/fightstg/fightstg_8008D3B4.c src/cardgame/cardgame_cpu.c src/stfgtrep/stfgtrep_80082E70.c
       src/stgtrain/stgtrain_800861BC.c src/stgtrain/stgtrain_80088100.c)
# The card game's rules and the CPU's helpers (card data, effects, game flow, check_condition): cardgame_cpu_choice, cardgame_rules;
# the board unit (80096950) for its data (cardgame_slot_origins: cardgame_deal_cards).
UNITS+=(src/cardgame/cardgame_80083E34.c src/cardgame/cardgame_80085DE8.c src/cardgame/cardgame_8009D6E0.c
        src/cardgame/cardgame_800954F8.c src/cardgame/cardgame_80096950.c)
# Item use and healing techniques out of battle (ststatus_items), the shops' helpers (shop_rules).
UNITS+=(src/ststatus/ststatus_8008E94C.c src/ststatus/ststatus_800937A4.c src/ststatus/ststatus_80099B6C.c
        src/stitshop/stitshop_800859C0.c src/stcrdshp/stcrdshp_80088E24.c)
# The object methods and the sprite renderer (gamestate_actions, sprite_draw).
UNITS+=(src/main/object.c src/main/sprite.c)
# The Digivolution Lab's main object (stgdglab_party).
UNITS+=(src/stgdglab/stgdglab_8008EB30.c)
# The battle's tier-2 overlay: first strike and the damage cap (fightstg_rules, appended cases), the battle's end (wfightmn_spoils).
UNITS+=(src/wfightmn/wfightmn_800A6440.c)
# FIELDSTG's battle table (fieldstg_battles, data only): fieldstg_start_battle (wfightmn_spoils).
UNITS+=(src/fieldstg/fieldstg_80083784.c)
CFLAGS=(-m64 -std=gnu99 -fwrapv -fsigned-char -fno-strict-aliasing -fno-stack-protector -DNON_MATCHING -w
        -I"$ROOT/tests/host" -I"$ROOT/include" -I"$ROOT")
mkdir -p "$(dirname "$OUT")"
OBJS=()
for u in "${UNITS[@]}"; do
    o="$(dirname "$OUT")/$(basename "$u" .c).o"
    gcc "${CFLAGS[@]}" -c "$ROOT/$u" -o "$o"
    OBJS+=("$o")
done
# heap.c gives heap_objects and its search (gamestate_cond_object): its scratchpad-stack switch (MIPS inline asm, in
# heap_run_object, never reached here) is compiled away, and its heap_funcs is made weak so the shims' libc heap wins.
o="$(dirname "$OUT")/heap.o"
gcc "${CFLAGS[@]}" -include "$ROOT/tests/host/no_asm.h" -D__asm__= -c "$ROOT/src/main/heap.c" -o "$o"
objcopy --weaken-symbol=heap_funcs "$o"
OBJS+=("$o")
# FIELDSTG's field unit for the encounter redraw (fieldstg_encounter): one function it calls before defining it gets its
# prototype forced in (tests/host/fieldstg_protos.h).
o="$(dirname "$OUT")/fieldstg_80087DB0.o"
gcc "${CFLAGS[@]}" -include "$ROOT/tests/host/fieldstg_protos.h" -c "$ROOT/src/fieldstg/fieldstg_80087DB0.c" -o "$o"
OBJS+=("$o")
# FIGHTSTG's first unit for the enemy's choice of action (wfightmn_enemy_ai): one function it calls before defining it gets
# its prototype forced in (tests/host/fightstg_protos.h).
o="$(dirname "$OUT")/fightstg_80086A00.o"
gcc "${CFLAGS[@]}" -include "$ROOT/tests/host/fightstg_protos.h" -c "$ROOT/src/fightstg/fightstg_80086A00.c" -o "$o"
OBJS+=("$o")
for u in tests/host/shims.c tests/host/layout.c tests/host/replay.c; do
    o="$(dirname "$OUT")/$(basename "$u" .c).o"
    gcc "${CFLAGS[@]}" -c "$ROOT/$u" -o "$o"
    OBJS+=("$o")
done
gcc "${CFLAGS[@]}" -c "$SYMTAB" -o "$(dirname "$OUT")/symtab.o"
# The units reference the rest of the game (gfx, sound, objects, Psy-Q): functions the goldens never reach. They resolve
# to address 0, so reaching one crashes the harness loudly instead of needing a stub each.
gcc -m64 -no-pie "${OBJS[@]}" "$(dirname "$OUT")/symtab.o" -Wl,--unresolved-symbols=ignore-all -o "$OUT"
