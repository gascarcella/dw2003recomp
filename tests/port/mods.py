#!/usr/bin/env python3
"""The port's mods that change the game's behaviour, run with the mod on (docs/LAUNCHER.md "Mod runtime").

Usage: tests/port/mods.py [check] [--out DIR] [-j N]
       tests/port/mods.py record [--out DIR]     # rewrite tests/port/mods/expected/ from this build (after a review)

A run with a mod on has its own expected results (the random generator steps once a frame: any skipping moves later
rolls), and the emulator has no mods, so they are the port's, committed in tests/port/mods/expected/ and checked here
together with what the mod must keep from the emulator's run. The scripts are tests/port/mods/scripts/*.json, run
with a settings file that enables the mod and --script-mods.

skip_dialogues (scripts/skip_dialogues.json; on from the start: DW3_PORT_SKIP_DIALOGUES=1): first_battle_save's route
with no press through the scenes and one press per NPC talk and per choice, each choice after 300 idle frames:
  - two runs give the same log and record, whose cross-core view equals expected/skip_dialogues.json (the choice
    checkpoints `tamer_choice` and `inn_choice` are still on their maps: the choices waited);
  - every checkpoint it shares with first_battle_save has the emulator's stable gamestate hash
    (tests/replay/expected/first_battle_save.json): skipping the text changes no game logic on this route;
  - with the mod off the same script stops at its first scene (the intro's first dialogue waits for a press);
  - the first battle with one CROSS every 100 frames needs fewer presses with the mod on than off (the battle's
    message waits go on by themselves; first_battle_save's route to the battle for the mod-off run);
  - `fast_forward_waits` on: the same log and record (only the pace changes), and fast-forward is asked for during
    the scenes' events.
battle_animations: first_battle_save itself with the mod on, `hit_reaction` on and off: the emulator's
cross-core view (the battle's rules run before its animations), and the first battle's three actions cut, the last
one keeping its KO reaction. tests/port/battle.py checks the scripts on the disc that the cut relies on.
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
sys.path.insert(0, str(ROOT / "tests/port"))
sys.path.insert(0, str(ROOT / "tests/replay"))
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)
from replay import compare, cross_core_view  # noqa: E402

MODS = ROOT / "tests/port/mods"
SD_SCRIPT = MODS / "scripts/skip_dialogues.json"
SD_EXPECTED = MODS / "expected/skip_dialogues.json"
FBS_EXPECTED = ROOT / "tests/replay/expected/first_battle_save.json"
BATTLE_PRESS = {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 100,
                "until": {"type": "wait_stage", "stage": 20}, "timeout": 30000,
                "comment": "one CROSS every 102 frames until the battle report (STFGTREP)"}

FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def settings(out, name, mods):
    path = out / f"{name}.settings.json"
    path.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                "memcard1": None, "memcard2": None, "mods": mods}, indent=1))
    return path


def run_port(binary, out, label, script, config=None, mods_on=False, env=None):
    """One run; returns (exit code, log bytes, record dict or None, stderr)."""
    log, record = out / f"{label}.log", out / f"{label}.json"
    cmd = [str(binary), "--script", str(script), "--log", str(log), "--record", str(record)]
    cmd += ["--config", str(config)] if config else ["--disc", str(DISC)]
    if mods_on:
        cmd.append("--script-mods")
    proc = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, **(env or {})), capture_output=True, text=True,
                          timeout=600)
    (out / f"{label}.stderr").write_text(proc.stderr)
    rec = json.loads(record.read_text()) if record.exists() and record.stat().st_size else None
    return proc.returncode, log.read_bytes() if log.exists() else b"", rec, proc.stderr


def derived(out, name, steps):
    path = out / f"{name}.script.json"
    path.write_text(json.dumps({"name": name, "max_frames": 120000, "steps": steps}, indent=1))
    return path


def battle_presses(log):
    """The CROSS presses (I lines) from the checkpoint battle_start to the end of the log."""
    n, on = 0, False
    for line in log.decode().splitlines():
        p = line.split()
        if p and p[0] == "C" and p[2] == "battle_start":
            on = True
        elif on and p and p[0] == "I" and int(p[3], 16) & 0x4000:
            n += 1
    return n


def skip_dialogues(binary, out, record):
    print("mods: skip_dialogues (tests/port/mods/scripts/skip_dialogues.json, on from the start)")
    on = settings(out, "sd_on", {"skip_dialogues": {"enabled": True}})
    env = {"DW3_PORT_SKIP_DIALOGUES": "1"}
    rc1, log1, rec1, err1 = run_port(binary, out, "sd_1", SD_SCRIPT, on, True, env)
    rc2, log2, rec2, _ = run_port(binary, out, "sd_2", SD_SCRIPT, on, True, env)
    check(rc1 == 0 and rc2 == 0 and rec1 is not None, f"two runs complete (exit {rc1}, {rc2})")
    if rec1 is None:
        print("    " + "\n    ".join(err1.splitlines()[-5:]))
        return
    check(log1 == log2 and rec1 == rec2, "the two runs' logs and records are identical")
    if record:
        SD_EXPECTED.parent.mkdir(parents=True, exist_ok=True)
        SD_EXPECTED.write_text(json.dumps(rec1, indent=1) + "\n")
        print(f"  recorded {SD_EXPECTED.relative_to(ROOT)}")
    expected = json.loads(SD_EXPECTED.read_text())
    diffs = compare(cross_core_view(expected), cross_core_view(rec1))
    check(not diffs, f"the cross-core view equals {SD_EXPECTED.relative_to(ROOT)}" + ("".join(f"\n      {d}" for d in diffs)))
    cps = {c["name"]: c for c in rec1["checkpoints"]}
    check(cps.get("tamer_choice", {}).get("map") == 0x203 and cps.get("inn_choice", {}).get("map") == 0x20A,
          "after 300 idle frames the choices are still open (tamer_choice on map 0x203, inn_choice on 0x20A)")
    emu = {c["name"]: c for c in json.loads(FBS_EXPECTED.read_text())["checkpoints"]}
    shared = [n for n in cps if n in emu]
    same = [n for n in shared if cps[n]["gamestate_sha1_stable"] == emu[n]["gamestate_sha1_stable"]]
    check(len(shared) >= 6 and same == shared,
          f"the {len(shared)} checkpoints shared with first_battle_save have the emulator's stable hashes "
          f"({', '.join(n + ('' if n in same else ' DIFFERS') for n in shared)}): no game logic changed")
    fewer = [f"{n} {emu[n]['frame'] - cps[n]['frame']:+d}" for n in shared]
    print(f"    frames saved against the emulator's mod-off run: {', '.join(fewer)}")

    rc, _, rec, err = run_port(binary, out, "sd_off", SD_SCRIPT)
    check(rc == 5 and rec is not None and not rec["checkpoints"] and "(wait_map) timed out" in err
          and rec["map_sequence"][-1]["map"] == 0x2D7,
          f"with the mod off the script stops at the intro's first dialogue (exit {rc})")

    steps = json.loads(SD_SCRIPT.read_text())["steps"]
    i = next(k for k, s in enumerate(steps) if s.get("type") == "checkpoint" and s.get("name") == "battle_start")
    rc_on, log_on, _, _ = run_port(binary, out, "battle_on", derived(out, "battle_on", steps[:i + 1] + [BATTLE_PRESS]),
                                   on, True, env)
    fbs = json.loads((SCRIPTS / "first_battle_save.json").read_text())["steps"]
    j = next(k for k, s in enumerate(fbs) if s.get("type") == "checkpoint" and s.get("name") == "battle_start")
    rc_off, log_off, _, _ = run_port(binary, out, "battle_off", derived(out, "battle_off", fbs[:j + 1] + [BATTLE_PRESS]))
    p_on, p_off = battle_presses(log_on), battle_presses(log_off)
    check(rc_on == 0 and rc_off == 0 and 0 < p_on < p_off,
          f"the first battle, one CROSS every 102 frames: {p_on} presses with the mod on, {p_off} off "
          f"(the battle's messages go on by themselves)")

    ffw = settings(out, "sd_ffw", {"skip_dialogues": {"enabled": True, "fast_forward_waits": True}})
    rc, log, rec, err = run_port(binary, out, "sd_ffw", SD_SCRIPT, ffw, True, env)
    asked = err.count("(a cutscene's waits)")
    check(rc == 0 and log == log1 and rec == rec1 and asked > 0,
          f"fast_forward_waits: the same log and record, fast-forward asked for {asked} time(s) during the events")


def battle_animations(binary, out):
    """battle_animations: first_battle_save itself with the mod on (both settings of hit_reaction) must give the
    emulator's cross-core view (the battle's rules ran before the animations: the outcome, the save, the reload, the
    shops are the same), with the first battle's three actions cut (the last one a knock-out, which keeps its KO
    reaction in both settings)."""
    print("mods: battle_animations (first_battle_save, the mod on)")
    emu = json.loads(FBS_EXPECTED.read_text())
    for hit in (True, False):
        cfg = out / f"ba_{hit}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": f"ba_{hit}_1.mcd", "memcard2": f"ba_{hit}_2.mcd",
                                   "mods": {"battle_animations": {"enabled": True, "hit_reaction": hit}}}))
        for card in (out / f"ba_{hit}_1.mcd", out / f"ba_{hit}_2.mcd"):
            card.unlink(missing_ok=True)
        rc, _, rec, err = run_port(binary, out, f"ba_{hit}", SCRIPTS / "first_battle_save.json", cfg, True,
                                   {"DW3_PORT_RESET_CHECK": "1"})
        diffs = compare(cross_core_view(emu), cross_core_view(rec)) if rec else ["no record"]
        cuts = [l.split("cut", 1)[1] for l in err.splitlines() if "battle animations: frame" in l]
        battle = [c for c in cuts]
        ko = bool(battle) and "reaction 3" in battle[-1]
        reactions = sum("the target's reaction" in c for c in battle)
        check(rc == 0 and not diffs, f"hit_reaction {str(hit).lower()}: the emulator's cross-core view (all "
              f"{len(emu['checkpoints'])} checkpoints, sequences){'' if not diffs else ': ' + '; '.join(diffs[:2])}")
        want = len(battle) if hit else 1
        check(len(battle) == 3 and ko and reactions == want,
              f"hit_reaction {str(hit).lower()}: the first battle's {len(battle)} actions cut, {reactions} kept as the "
              f"target's reaction, the knock-out's KO reaction kept")
        if rec:
            cps = {c["name"]: c for c in rec["checkpoints"]}
            e = {c["name"]: c for c in emu["checkpoints"]}
            print(f"    the battle took {cps['battle_won']['frame'] - cps['battle_start']['frame']} frames "
                  f"(emulator, the mod off: {e['battle_won']['frame'] - e['battle_start']['frame']})")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("mode", nargs="?", default="check", choices=("check", "record"))
    ap.add_argument("--out", help="scratch directory (default build/port-test/mods)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None)
    args = ap.parse_args()
    try:
        if not DISC.exists():
            raise Missing(f"no disc image ({DISC.relative_to(ROOT)})")
        env = tool_env()
    except Missing as e:
        print(f"mods: {e}", file=sys.stderr)
        return 2
    out = (Path(args.out) if args.out else ROOT / "build/port-test/mods").resolve()
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    binary = build("build/port", [], args.jobs, env)
    skip_dialogues(binary, out, args.mode == "record")
    battle_animations(binary, out)
    print(f"mods test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
