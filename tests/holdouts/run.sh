#!/usr/bin/env bash
# Holdout validation by run (tests/holdouts/run.py; tests/README.md "Holdout validation"): the -DNON_MATCHING build of
# the holdouts in a scratch disc image, replayed through the layer-2 scripts. Not part of scripts/test.sh (~11 min; ~2 min with --no-probe).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
exec "$ROOT/tools/venv/bin/python" "$ROOT/tests/holdouts/run.py" "$@"
