#!/usr/bin/env python3
"""Layer-2 replay tests: boot the disc in PCSX-Redux, feed a pad script, hash gamestate_data at named checkpoints.
The runner is psxstack's (psxstack/tools/replay/emulator.py and run.lua; GAME_CONTRACT.md "6. Tests"): this file
is this game's configuration of it (the emulator, the disc, the scripts and expected files, the probes
tests/replay/probes.lua, the stable hash's volatile ranges) and the names the other tests import from here.

Usage:
  tests/replay/replay.py run <script.json> [--record] [--repeat N] [--bios openbios|retail] [--speed S] [--interpreter]
                             [--iso CUE] [--prelude LUA] [--expected-dir DIR] [--out DIR] [-v]
  tests/replay/replay.py check [--interpreter] [--iso CUE] [-j N] [script.json ...]   # every script with an expected file (the test entry point), N at a time
  tests/replay/replay.py boot [--bios ...] [--iso CUE] [--frames N]                    # the boot check (scripts/check_emulator.sh)

A script (tests/replay/scripts/<name>.json) is a list of steps (see tests/README.md); the runner executes it in the
emulator and writes result.json plus one gamestate_data dump per checkpoint. The driver hashes the dumps (SHA-1),
builds the record and compares it with tests/replay/expected/<name>.json, or writes that file with --record.
--repeat N runs the script N times and requires identical records (determinism).

Exit codes: 0 pass, 1 mismatch or emulator failure, 2 usage / missing tool.
"""
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "psxstack/tools/replay"))
import emulator  # noqa: E402  (psxstack's runner)
from emulator import (CHECKPOINT_FIELDS, RUN_LUA, bios_path, check_tools, compare, cross_core_view,  # noqa: E402,F401
                      emulator_info, load_script, lua_literal, parse_ints, replay_env, run_once, sha1_file)


def gamedata_dir():
    """The optional data checkout (scripts/gamedata_dir.sh, same lookup): $DW3_GAMEDATA, tools/local.env, this
    checkout if it has gamedata/, or the sibling ../dw2003-gamedata. None when there is none."""
    d = os.environ.get("DW3_GAMEDATA")
    if not d and (ROOT / "tools/local.env").is_file():
        for line in (ROOT / "tools/local.env").read_text().splitlines():
            if line.startswith("DW3_GAMEDATA="):
                d = line.split("=", 1)[1].strip().strip("\"'")
                break
    if not d and (ROOT / "gamedata").is_dir():
        d = str(ROOT)
    if not d and (ROOT.parent / "dw2003-gamedata/gamedata").is_dir():
        d = str(ROOT.parent / "dw2003-gamedata")
    return Path(d) if d and Path(d).is_dir() else None


GD = gamedata_dir()
RETAIL_BIOS = (GD or ROOT) / "gamedata/bios/scph7502.bin"   # cross-check only: committed goldens come from OpenBIOS
# Extra emulator flags of --interpreter (the core tools/coverage.py needs); scripts must reach the same checkpoints on it.
EMU_INTERPRETER = ("-interpreter",)
# Byte ranges of gamestate_data (include/gamestate.h) that change every frame, with the frame of arrival, or with the
# pad_random index (which advances about once per frame in the field); they are
# zeroed for gamestate_sha1_stable so the port can compare a checkpoint reached a few frames earlier or later.
# A literal: tools/port_gen.py and tools/port_inputs.py read it from this file's source (one definition for the
# emulator's records and the port's).
VOLATILE_RANGES = ((0x00, 0x01),   # checksum: 0 until a save is loaded, then the slot's, which covers the playtime
                   (0x30, 0x34),   # encounter_timer
                   (0x48, 0x54),   # playtime_frames, playtime[4]
                   (0x26EC, 0x26F0))  # spot_target: drawn from pad_random on entering a map with spots (0x21D), and the
                                      # RNG index depends on the frame count (core- and port-dependent); 0 before that

CFG = emulator.configure(
    root=ROOT, game_json=ROOT / "port/game/game.json", redux_dir=ROOT / "tools/redux", iso=ROOT / "iso/dw2003.cue",
    scripts_dir=ROOT / "tests/replay/scripts", expected_dir=ROOT / "tests/replay/expected",
    probes=ROOT / "tests/replay/probes.lua", volatile_ranges=VOLATILE_RANGES, retail_bios=RETAIL_BIOS,
    tree_paths=("src", "include", "config"), interpreter_args=EMU_INTERPRETER)
# The paths the other tests use (tests/port/*.py, tests/sound/spu_trace.py, tests/saves/run.py, tools/coverage.py).
REDUX, REDUX_VERSION, OPENBIOS = CFG.redux, CFG.redux_version, CFG.openbios
ISO, SCRIPTS, EXPECTED = CFG.iso, CFG.scripts_dir, CFG.expected_dir


def main():
    return emulator.main()


if __name__ == "__main__":
    sys.exit(main())
