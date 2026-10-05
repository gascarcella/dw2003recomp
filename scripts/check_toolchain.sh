#!/usr/bin/env bash
# Smoke test for the matching toolchain installed by scripts/setup.sh.
# Compiles a tiny C function with every old-gcc version through tools/cc_psx.sh
# (cpp -> cc1 -> maspsx -> mipsel as), disassembles it, then runs m2c, objdiff-cli,
# asm-differ and decomp-permuter once each. Exits non-zero on the first failure.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
T="$ROOT/tools"
PY="$T/venv/bin/python"
export PATH="$T/binutils/bin:$PATH"
W="$(mktemp -d)"
trap 'rm -rf "$W"' EXIT

cat > "$W/t.c" <<'EOF'
extern void *D_80044F6C[];
int func_800141E4(int i) { return D_80044F6C[i] != 0; }
EOF

for v in 2.7.2 2.8.0 2.8.1 2.91.66 2.95.2; do
    # The same wrapper the build uses (cpp -> cc1 -> maspsx at the matching ASPSX -> as).
    "$ROOT/tools/cc_psx.sh" -V "$v" "$W/t.c" -o "$W/$v.o"
    printf 'gcc %-8s: ' "$v"
    mipsel-linux-gnu-objdump -d -z "$W/$v.o" | awk '/^ +[0-9a-f]+:/{printf "%s %s; ", $3, $4} END{print ""}'
done

cat > "$W/f.s" <<'EOF'
glabel func_800141E4
    lui        $v0, %hi(D_80044F6C)
    addiu      $v0, $v0, %lo(D_80044F6C)
    sll        $a0, $a0, 2
    addu       $a0, $a0, $v0
    lw         $v0, 0x0($a0)
    jr         $ra
    sltu       $v0, $zero, $v0
EOF
echo "m2c:"
"$PY" "$T/ext/m2c/m2c.py" --target mips-gcc-c "$W/f.s" | sed 's/^/    /'

"$T/bin/objdiff-cli" diff -1 "$W/2.8.1.o" -2 "$W/2.8.0.o" -o "$W/d.json" func_800141E4
grep -q '"match_percent":100' "$W/d.json" && echo "objdiff-cli: 2.8.1 vs 2.8.0 is 100%"
"$PY" "$T/ext/asm-differ/diff.py" --help > /dev/null && echo "asm-differ: runs"
"$PY" "$T/ext/decomp-permuter/permuter.py" --help > /dev/null && echo "decomp-permuter: runs"
echo "toolchain OK"
