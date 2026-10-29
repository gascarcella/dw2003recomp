#!/usr/bin/env python3
"""Field events and menus that the layer-2 replays do not reach, run on the port alone (no emulator expectation).
Usage: tests/port/field.py [--out DIR] [--only CASE ...] [--wine [--exe PATH]]
Needs build/port/dw2003 (tests/port/run.py builds it) and the disc. Each case is first_battle_save's steps up to a
checkpoint, then its own steps (derived at run time, so the route stays the one the emulator recorded); the port must
reach the case's last checkpoint with exit 0, and no crash report may be written. A case with a check also has its
checkpoints' gamestate_data dumps (DW3_PORT_CHECKPOINT_DIR) checked.
--wine: the same cases with the Windows build (--exe, default build/port-win/dw2003.exe; scripts/build_windows.sh)
through `wine` (headless: SDL's dummy drivers, the prefix in build/wine-prefix/).
  lobby_lab_exit (issue #98): from the `asuka_lobby` checkpoint, the player walks to the Main Lobby's exit towards the
    lab (map-event tile 7 of WSTAG210, at (672..719, 568..591)) before progress 3, which runs event 30: the player is
    turned back with a prompt icon that the script closes twice (message 0x326 to object 0x323). The second close finds
    no icon, starts a new one and closes it before it found the player: fieldstg_icon_draw dereferenced its NULL actor
    (tests/host/FINDINGS.md 11). The port must survive the event and reach `turned_back`.
  items_list (issue #116): from the `card_case` checkpoint, the field menu's Items (STSTATUS's items page) and its
    first category's list: the counts window finishes opening two frames before the list puts its cursor's item in
    the page, and ststatus_items_draw drew the icon of item 0 through records_get_item's NULL (tests/host/FINDINGS.md
    12). The port must open the list and use a Power Charge (`power_charge_used`).
  swamp_dive (issue #120): from the `status_open` checkpoint (STSTATUS, entered through the field menu), progress 15
    and gamestate_data's field map and position are set so that Back returns to map 0x234 (WSTAG450) on its
    Submarimon dive (map event 4, type 10). The warp ends the party's actors before it leaves the map, and
    fieldstg_manager_leave_map read the player through their cleared slot (tests/host/FINDINGS.md 13). The port must
    reach map 0x2E5 (`dived`).
  swamp_flies (issue #119): the same way into BIOS Swampland (map 0x236, WSTAG460) below progress 15, where seven
    flies are created and type 4 has no sprite (tests/host/FINDINGS.md 14). The port must run 100 frames there
    (`flies_seen`).
  level_ups (issue #130): at the `battle_won` checkpoint (the report loaded, the experience not added yet), Digimon 0's
    (Kotemon's, member 0) experience is set to 50000, so the report's stfgtrep_add_exp takes it from level 1 to 39 in
    one call: 38 level-ups, each drawing the seven resists. Its Water resist is gain class 5, whose residue 3 read
    stfgtrep_resist_gains[8], past the table (tests/host/FINDINGS.md 2): the 0.6.0 Windows build added 383 there and
    ended at Water 999. Each resist must gain 0..2 per level-up (the table's range) from `battle_won` to `battle_end`.
    The Linux 0.6.0 build read a 2 there, which this bound cannot tell from a real gain; the Windows build's run
    (--wine) is the one that catches it.
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build/port/dw2003"
RUNNER = []          # the command prefix of a run (--wine: ["wine"])
RUNNER_ENV = {}      # its environment additions (--wine: the prefix, no debug output, SDL's dummy drivers)
DISC = ROOT / "iso/dw2003.cue"
SCRIPTS = ROOT / "tests/replay/scripts"
CASES = {
    "lobby_lab_exit": ("first_battle_save", "asuka_lobby", [
        # down the ramp, then the corridor to the bottom-right exit (positions from the attribute map's layer 0)
        {"type": "walk", "x": 600, "y": 440, "tol": 4, "timeout": 1500},
        {"type": "walk", "x": 580, "y": 470, "tol": 4, "timeout": 1500},
        {"type": "walk", "x": 580, "y": 510, "tol": 4, "timeout": 1500},
        {"type": "walk", "x": 620, "y": 540, "tol": 4, "timeout": 1500},
        {"type": "walk", "x": 640, "y": 560, "tol": 4, "timeout": 1500},
        {"type": "press", "buttons": ["RIGHT", "DOWN"], "frames": 300, "release": 1, "timeout": 1500,
         "until": {"type": "wait_mem", "addr": "0x80099DD8", "size": 4, "value": 1},
         "comment": "on towards the exit until fieldstg_stage.event_running: map-event tile 7 (flag 0x6002 set before progress 3) starts event 30"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 30, "timeout": 3000,
         "until": {"type": "wait_mem", "addr": "0x80099DD8", "size": 4, "value": 0},
         "comment": "through the event (the icon, the walk back, the second close, the dialogue) until it ends"},
        {"type": "checkpoint", "name": "turned_back"},
    ]),
    "items_list": ("first_battle_save", "card_case", [
        {"type": "press", "buttons": ["START"], "frames": 2, "release": 10, "timeout": 600,
         "until": {"type": "wait_mem", "addr": "0x80099DD0", "size": 4, "value": 1},
         "comment": "open the field menu until fieldstg_stage.menu_open, as first_battle_save's status_open does"},
        {"type": "wait_frames", "frames": 30, "comment": "the menu's opening animation, the cursor on option 0 (Items)"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 10, "until": {"type": "wait_stage", "stage": 16},
         "timeout": 600, "comment": "confirm: STSTATUS on the items page (ststatus_pages[1][0])"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 20, "timeout": 3000,
         "until": {"type": "wait_mem", "addr": "0x80048DDB", "size": 1, "value": 3},
         "comment": "CROSS until a Power Charge is used (gamestate_data.items[43] 4 -> 3; the leader is not at full HP): "
                    "the first category (usable items) opens its list and the counts, then the Power Charge, the member"},
        {"type": "checkpoint", "name": "power_charge_used"},
    ]),
    "swamp_dive": ("first_battle_save", "status_open", [
        {"type": "write_mem", "addr": "0x8004B370", "value": 15,
         "comment": "gamestate_data.progress 15: Submarimon's dives (map events with flag 0x7093: condition 0x93, "
                    "progress 15..99) are open"},
        {"type": "write_mem", "addr": "0x80048D68", "value": "0x234",
         "comment": "gamestate_data.field_map: back from the menu on map 0x234 (WSTAG450), not 0x211"},
        {"type": "write_mem", "addr": "0x80048D6C", "data": "0038030000c4010001000000",
         "comment": "player_pos (824, 452) << 8 and player_dir 1: on its dive's region (attribute layer 7 value 0xA4, "
                    "index 4, x 784..863, y 432..471: map event 4, type 10 to map 0x2E5)"},
        {"type": "press", "buttons": ["TRIANGLE"], "frames": 2, "release": 10,
         "until": {"type": "wait_stage", "stage": 2}, "timeout": 1500,
         "comment": "Back from the status screen to the field, as first_battle_save does"},
        {"type": "wait_map", "map": "0x234", "timeout": 300},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 10,
         "until": {"type": "wait_map", "map": "0x2E5"}, "timeout": 3000,
         "comment": "dive (CROSS on the warp's icon): the warp ends the party's actors, then leaves the map through "
                    "fieldstg_manager_leave_map"},
        {"type": "checkpoint", "name": "dived"},
    ]),
    "swamp_flies": ("first_battle_save", "status_open", [
        {"type": "write_mem", "addr": "0x80048D68", "value": "0x236",
         "comment": "gamestate_data.field_map: back from the menu on map 0x236 (WSTAG460, BIOS Swampland); progress "
                    "stays below 15, so its fly swarm is created"},
        {"type": "write_mem", "addr": "0x80048D6C", "data": "00d600000084000001000000",
         "comment": "player_pos (214, 132) << 8 and player_dir 1: where WSTAG455's event 360 enters the map"},
        {"type": "press", "buttons": ["TRIANGLE"], "frames": 2, "release": 10,
         "until": {"type": "wait_stage", "stage": 2}, "timeout": 1500,
         "comment": "Back from the status screen to the field, as first_battle_save does"},
        {"type": "wait_map", "map": "0x236", "timeout": 300},
        {"type": "wait_frames", "frames": 100, "comment": "the swarm's seven flies move (type 4 has no sprite)"},
        {"type": "checkpoint", "name": "flies_seen"},
    ]),
    "level_ups": ("first_battle_save", "battle_won", [
        {"type": "write_mem", "addr": "0x800494B4", "size": 4, "value": 50000,
         "comment": "gamestate_data.digimon[0].record.exp (0x75C + 0xC + 0x18): the report levels it from 1 to 39"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 8, "until": {"type": "wait_stage", "stage": 2},
         "timeout": 3000, "comment": "through the report, as first_battle_save does"},
        {"type": "checkpoint", "name": "battle_end"},
    ]),
}
LU_RECORD = 0x75C + 0xC   # gamestate_data.digimon[0].record in a dump: level at +0x1C, the resists at +0x34
LU_RESISTS = ("Fire", "Water", "Ice", "Wind", "Thunder", "Machine", "Dark")


def s16(dump, at):
    return int.from_bytes(dump[at:at + 2], "little", signed=True)


def check_level_ups(dumps):
    """level_ups: Digimon 0 from level 1 to 31..40 (the resists are drawn up to 40), each resist +0..2 a level."""
    won, end = dumps.get("battle_won"), dumps.get("battle_end")
    if won is None or end is None:
        return ["no battle_won or battle_end dump"]
    lv0, lv1 = s16(won, LU_RECORD + 0x1C), s16(end, LU_RECORD + 0x1C)
    if lv0 != 1 or not 30 < lv1 <= 40:
        return [f"Digimon 0 went from level {lv0} to {lv1}, want 1 to 31..40"]
    ups = lv1 - lv0
    problems = []
    for i, name in enumerate(LU_RESISTS):
        a, b = s16(won, LU_RECORD + 0x34 + 2 * i), s16(end, LU_RECORD + 0x34 + 2 * i)
        if not 0 <= b - a <= 2 * ups:
            problems.append(f"{name} {a} -> {b} in {ups} level-ups (the table gives 0..2 each)")
    return problems


CHECKS = {"level_ups": check_level_ups}


def derive(out, name):
    base, checkpoint, steps = CASES[name]
    src = json.loads((SCRIPTS / f"{base}.json").read_text())
    prefix = []
    for step in src["steps"]:
        prefix.append(step)
        if step.get("type") == "checkpoint" and step.get("name") == checkpoint:
            break
    else:
        raise RuntimeError(f"{base}: no checkpoint {checkpoint}")
    path = out / f"{name}.script.json"
    path.write_text(json.dumps({"name": name, "max_frames": 40000, "default_timeout": 3000,
                                "steps": prefix + steps}, indent=1))
    return path, steps[-1]["name"]


def run_case(out, name):
    script, last = derive(out, name)
    log, record = out / f"{name}.log", out / f"{name}.json"
    dumps = out / f"{name}_dumps"
    shutil.rmtree(dumps, ignore_errors=True)
    dumps.mkdir()
    cmd = [*RUNNER, str(BINARY), "--disc", str(DISC), "--script", str(script), "--log", str(log), "--record", str(record)]
    proc = subprocess.run(cmd, cwd=out, capture_output=True, text=True, errors="replace", timeout=600,
                          env={**os.environ, **RUNNER_ENV, "DW3_PORT_CHECKPOINT_DIR": str(dumps)})
    (out / f"{name}.stderr").write_text(proc.stderr)
    problems = []
    if proc.returncode != 0:
        problems.append(f"exit {proc.returncode}: {proc.stderr.strip().splitlines()[-1] if proc.stderr.strip() else ''}")
    reports = sorted(out.glob("crash-*.txt"))
    if reports:
        problems.append("crash report " + ", ".join(r.name for r in reports))
    rec = json.loads(record.read_text()) if record.exists() and record.stat().st_size else {}
    names = [c.get("name") for c in rec.get("checkpoints", [])]
    if last not in names:
        problems.append(f"checkpoint {last} not reached (reached: {names})")
    elif name in CHECKS:
        problems += CHECKS[name]({f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in dumps.glob("cp*.bin")})
    return problems


def main():
    global BINARY
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=None)
    ap.add_argument("--only", nargs="*", default=None)
    ap.add_argument("--wine", action="store_true", help="the Windows build through wine")
    ap.add_argument("--exe", default=None, help="with --wine: the Windows build (default build/port-win/dw2003.exe)")
    args = ap.parse_args()
    if args.wine:
        BINARY = Path(args.exe).resolve() if args.exe else ROOT / "build/port-win/dw2003.exe"
        if not BINARY.is_file():
            print(f"field test: no {BINARY} (scripts/build_windows.sh)")
            return 2
        if shutil.which("wine") is None:
            print("field test: no wine on PATH")
            return 2
        prefix = ROOT / "build/wine-prefix"
        prefix.mkdir(parents=True, exist_ok=True)
        RUNNER[:] = ["wine"]
        RUNNER_ENV.update(WINEPREFIX=str(prefix), WINEDEBUG="-all", SDL_VIDEO_DRIVER="dummy", SDL_AUDIO_DRIVER="dummy")
    elif not BINARY.is_file():
        print(f"field test: no {BINARY.relative_to(ROOT)} (tests/port/run.py builds it)")
        return 2
    if not DISC.is_file():
        print(f"field test: no {DISC.relative_to(ROOT)}")
        return 2
    out = Path(args.out) if args.out else ROOT / ("build/port-test/field-wine" if args.wine else "build/port-test/field")
    out.mkdir(parents=True, exist_ok=True)
    failed = 0
    for name in args.only or CASES:
        problems = run_case(out, name)
        print(f"field test: {name}: " + ("ok" if not problems else "FAIL: " + "; ".join(problems)))
        failed += bool(problems)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
