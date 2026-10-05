#!/bin/bash
# Cloud sessions only (user-approved, session 6): install the pinned toolchain (scripts/setup.sh,
# idempotent; ~4 min on a cold container) and prepare the disc (scripts/worktree_init.sh) from the data
# checkout, when the session has one beside this repo (scripts/gamedata_dir.sh: ../dw2003-gamedata or
# $DW3_GAMEDATA). Without it the emulator and disc steps skip. Local and worktree sessions are untouched.
set -euo pipefail
[[ "${CLAUDE_CODE_REMOTE:-}" == "true" ]] || exit 0
cd "$CLAUDE_PROJECT_DIR"
scripts/setup.sh binutils mkpsxiso venv gcc objdiff ext redux >&2
scripts/worktree_init.sh >&2
