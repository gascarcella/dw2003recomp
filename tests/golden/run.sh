#!/usr/bin/env bash
# Layer 1: regenerate every golden family in the emulator and require the committed files to be reproduced exactly.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PY="$ROOT/tools/venv/bin/python"; [[ -x "$PY" ]] || PY=python3
exec "$PY" "$ROOT/tests/golden/oracle.py" check "$@"
