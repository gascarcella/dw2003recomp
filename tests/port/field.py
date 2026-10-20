#!/usr/bin/env python3
"""Field event scripts that the layer-2 replays do not reach, run on the port alone (no emulator expectation).
Usage: tests/port/field.py [--out DIR]
Needs build/port/dw2003 (tests/port/run.py builds it) and the disc. Each case is first_battle_save's steps up to a
checkpoint, then its own steps (derived at run time, so the route stays the one the emulator recorded); the port must
reach the case's last checkpoint with exit 0, and no crash report may be written.
  lobby_lab_exit (issue #98): from the `asuka_lobby` checkpoint, the player walks to the Main Lobby's exit towards the
    lab (map-event tile 7 of WSTAG210, at (672..719, 568..591)) before progress 3, which runs event 30: the player is
    turned back with a prompt icon that the script closes twice (message 0x326 to object 0x323). The second close finds
    no icon, starts a new one and closes it before it found the player: fieldstg_icon_draw dereferenced its NULL actor
    (tests/host/FINDINGS.md 11). The port must survive the event and reach `turned_back`.
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "build/port/dw2003"
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
}


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
    cmd = [str(BINARY), "--disc", str(DISC), "--script", str(script), "--log", str(log), "--record", str(record)]
    proc = subprocess.run(cmd, cwd=out, capture_output=True, text=True, timeout=600)
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
    return problems


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=None)
    ap.add_argument("--only", nargs="*", default=None)
    args = ap.parse_args()
    if not BINARY.is_file():
        print(f"field test: no {BINARY.relative_to(ROOT)} (tests/port/run.py builds it)")
        return 2
    if not DISC.is_file():
        print(f"field test: no {DISC.relative_to(ROOT)}")
        return 2
    out = Path(args.out) if args.out else ROOT / "build/port-test/field"
    out.mkdir(parents=True, exist_ok=True)
    failed = 0
    for name in args.only or CASES:
        problems = run_case(out, name)
        print(f"field test: {name}: " + ("ok" if not problems else "FAIL: " + "; ".join(problems)))
        failed += bool(problems)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
