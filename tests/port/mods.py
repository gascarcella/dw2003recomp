#!/usr/bin/env python3
"""The port's mods that change the game's behaviour, run with the mod on (docs/LAUNCHER.md "Mod runtime").

Usage: tests/port/mods.py [check] [--out DIR] [-j N] [--only MOD ...]
       tests/port/mods.py record [--out DIR]     # rewrite tests/port/mods/expected/ from this build (after a review)
The mods run N at a time (-j, default $DW3_JOBS, else every mod at once): each in its own process (this script with
--only MOD, its output printed whole, in the order below) and its own directory under --out; the port is built once
first. DW3_JOBS=1 (scripts/test.sh's default) runs them one after another.

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
global_save (scripts/global_save.json): first_battle_save's route to the lab after the first battle (map 0x206, not
an inn's map), the field menu's SAVE entry, STGMCARD's save, Back to the field, a reset and Continue:
  - the slot (gamestate_data's first 0x26C4 bytes, the volatile ranges aside) is the same at `saved_anywhere`,
    `back_after_save` and `loaded_anywhere`, on map 0x206; the load resumes the map as a return to it (map_is_new 0,
    prev_map 0xC00) and the card's slot tail carries the mod's record ("GSAV" at slot offset 0x26C4);
  - with `restore_map_state` off the load is the game's own (map_is_new 1);
  - with the mod off the menu has no SAVE entry: the script's SAVE press opens STATUS instead and its wait for
    STGMCARD times out.
tests/saves/run.py loads the card the mod writes in the emulator (the PS1 game ignores the record).
preset_language (no script of its own):
  - each of the eight languages, booted through the debug channel (tools/mcp/game.py), reaches the opening's map
    (0xE02; 0xE01 for Japanese) with its records_language (portuguese_br: 2, English), without the language select
    screen; portuguese_br looks for its text pack beside the binary; with the mod off the game is still on that screen
    (map 0x1600) 600 frames in;
  - port/game/text_pack.c, compiled alone with tests/port/text_pack_driver.c, rebuilds all 50 English files of a pack
    with two entries in each as tools/text_pack.py does, byte for byte;
  - portuguese_br's text pack (docs/PORT.md "Text packs"): an invalid pack is logged and the game boots in English; a
    test pack (tools/text_pack.py: the default name, the deck names, a long entry that grows ?SCRDSHP to two sectors,
    an unreached DMG entry) through new_game.json's route from the opening to the title's New Game: the new game's
    name and deck names are the pack's, and both files in cdload's cache are text_pack.py's rebuild byte for byte;
  - French through new_game.json's route from the opening (its CNTY_SEL steps replaced by a wait for map 0xE02) to
    the first field map: the opening well before the emulator's (which goes through the screen), and the new game's
    deck names (gamestate_init_records, at the title's New Game) are the French text file's.
party_xp: first_battle_save's route to back_on_field (the first battle: party member 0 fights alone, 4 experience),
the mod on with its defaults (share 50) and with share 100, and off: the checkpoints up to battle_won have the
emulator's stable hashes (the mod acts in the report only); from battle_won to battle_end member 0 gains 4 in every
run and members 1 and 2 gain 2, 4 and 0, and the money 50. Catch-up and knocked-out members are not reached by this battle.
xp_boost: the same route with the mod on: exp 3 and bits 2.5 (12 experience for member 0, 125 bits instead of 50);
exp and bits 10 with `manual` and party_xp's default share (40, 20, 20 and 500 bits); and 10 without `manual`
(capped at the sliders' 5x: 20 and 250 bits). Each run logs two boosts (the experience and the bits); the first battle
gives no form experience (member 0 fights as its own form), so `form_exp` is not reached by a scripted run.
level_up_recover: the same route with xp_boost's exp 10 (`manual`: 40 experience, member 0 from level 1 to 3), with
the mod on: member 0 ends at its new max HP and MP, one recover logged; and off: it keeps the battle's HP (93) and MP
under the raised maximums, nothing logged.
widescreen (no script of its own): first_battle_save to back_on_field with the mod on (`field` too, its default), in
the headless build (no GPU renderer): the frame log equals the run without the mod (the battle's canvas changes nothing
the game does, and the field's hooks and the extended VRAM do nothing without the canvas; the renderer's pictures are
tests/port/render_gpu.py's), and the run logs once that the picture stays 4:3.
random_encounters (no script of its own): first_battle_save's route to map 0x21D (all encounter area but its ramps), then
back and forth across its plaza until encounter_timer runs out, with the mod off and on (memory cards: the route saves):
  - off: the walk ends in a random battle (map 0x600 after 0x21D);
  - on: the story's first battle (an event's) still starts; the walk completes on 0x21D with no battle, each skipped
    encounter logged on map 0x21D, and the frame log equals the mod-off run's up to the first skip (the mod changes
    nothing before the roll). The toggle is a key, not reached
    by a script.
fast_transitions (scripts/global_save.json): global_save's run with global_save alone (off) and with the mod at speed
normal, fast and instant (memory card 1: the run saves and loads):
  - fast and instant: the run completes with its checkpoints and the slot's bytes are the same from the menu to the
    load (the save and the load are the game's own); the speed is logged;
  - fewer frames from the field menu's opening to back on the field after the save (the menu's windows and picture,
    STGMCARD's slides, bar and fade) and from map 0x203 to 0x204 (the map's title, the walk out, the closing window):
    off > fast > instant;
  - normal: the frame log and the record equal the mod-off run's (nothing changes);
  - instant twice: the same frame log and record (a run with the mod is deterministic with itself; it differs from
    the mod-off run's after the first transition, since the random generator steps once a frame).
fast_travel (global_save.json's route to the field menu on the lab, map 0x206, area 20 Asuka City): MAP, SQUARE twice
(the Amaterasu map, nothing visited there, and back), CROSS on the area under the cursor (the lab's own, Asuka City),
TRIANGLE twice (the page, then the menu), START once back on the field (the menu saves the player's place), with the
mod on and off:
  - on: the pick sets field_map 0x200 and player_pos to the table's (186824, 102500) and is logged; leaving the menus
    enters Asuka City (map 0x200 after 0x1000) at that place (player_pos when the menu opens again); both region
    switches upload the other region's images and back without a crash;
  - off: the presses change nothing: back on the lab (map 0x206), nothing logged.
training_success (first_battle_save's route to map 0x21D's trainer, then member 0's TP set to 1 and one Punch
session at level 0: the first party member, Punch, level 0, YES until the TP is spent; CROSS through the result and
the refused trainings after it; first_battle_save's way out), with the mod on (bonus_round `always`), with
bonus_round `game`, and off (memory cards: the route saves):
  - on: the round chance and the bonus round's are logged as 75 -> 100; Power gains 3 rounds of 1..2 and the won
    bonus round's 2 (5..8), last_bonus_training 1 (Punch), TP 0;
  - game: the rounds' chance 75 -> 100, the bonus round's stays 75;
  - off: the session runs (TP 0), nothing logged.
Exit codes: 0 pass, 1 fail, 2 something missing.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
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


def widescreen(binary, out):
    """widescreen: the battle and the field with the mod on in the headless build: the game's frame log unchanged,
    the software picture's 4:3 logged once."""
    print("mods: widescreen (first_battle_save to back_on_field, the headless build: no GPU renderer)")
    fbs = json.loads((SCRIPTS / "first_battle_save.json").read_text())["steps"]
    j = next(k for k, s in enumerate(fbs) if s.get("type") == "checkpoint" and s.get("name") == "back_on_field")
    script = derived(out, "ws", fbs[:j + 1])
    rc_on, log_on, _, err = run_port(binary, out, "ws_on", script, settings(out, "ws", {"widescreen": {"enabled": True}}),
                                     True)
    rc_off, log_off, _, _ = run_port(binary, out, "ws_off", script)
    said = err.count("widescreen: needs the GPU renderer")
    check(rc_on == rc_off == 0 and log_on == log_off and len(log_on) > 0,
          f"the frame log with the mod equals the one without (exit {rc_on}, {rc_off})")
    check("mods: widescreen on" in err and said == 1, f"the 4:3 picture logged once ({said} times)")


def random_encounters(binary, out):
    """random_encounters: a walk on map 0x21D long enough for a random battle, the mod off and on (the docstring)."""
    print("mods: random_encounters (first_battle_save to map 0x21D, then back and forth across its plaza)")
    fbs = json.loads((SCRIPTS / "first_battle_save.json").read_text())["steps"]
    j = next(k for k, s in enumerate(fbs) if s.get("until", {}).get("map") == "0x21D")
    # down the ramps to the plaza (encounter area 2), then 80 legs of ~48 frames: encounter_timer's 926 (since the new
    # game) runs out about 2,300 frames in
    walk = [{"type": "walk", "x": 1460, "y": 245}, {"type": "walk", "x": 1335, "y": 375}]
    walk += [{"type": "walk", "x": x, "y": y, "tol": 6} for _ in range(40) for x, y in ((1170, 470), (1335, 375))]
    script = derived(out, "re", fbs[:j + 1] + walk + [{"type": "checkpoint", "name": "walked"}])
    runs = {}
    for label, mods in (("re_off", {}), ("re_on", {"random_encounters": {"enabled": True}})):
        cfg = out / f"{label}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": str(out / f"{label}_1.mcd"), "memcard2": str(out / f"{label}_2.mcd"),
                                   "mods": mods}))
        for card in (out / f"{label}_1.mcd", out / f"{label}_2.mcd"):
            card.unlink(missing_ok=True)
        runs[label] = run_port(binary, out, label, script, cfg, True)
    rc, log_off, rec, err = runs["re_off"]
    maps = [m["map"] for m in rec["map_sequence"]] if rec else []
    check(rc == 5 and maps[-2:] == [0x21D, 0x600], f"off: the walk on map 0x21D ends in a random battle (map 0x600) "
          f"(exit {rc}, the last maps {', '.join(hex(m) for m in maps[-2:])})")
    rc, log_on, rec, err = runs["re_on"]
    maps = [m["map"] for m in rec["map_sequence"]] if rec else []
    skips = [line.split("frame ", 1)[1] for line in err.splitlines() if "random encounters: frame" in line]
    walked = maps.index(0x21D) if 0x21D in maps else len(maps)
    check(0x600 in maps[:walked], "on: the story's first battle (an event's, map 0x200) still starts")
    check(rc == 0 and maps[-1] == 0x21D and 0x600 not in maps[walked:] and skips and
          all("map 0x21D," in s for s in skips),
          f"on: the walk completes on map 0x21D with no battle, {len(skips)} encounter(s) skipped (exit {rc}; "
          f"{'; '.join(skips)})")
    if not skips:
        return
    first = int(skips[0].split(":")[0])
    on_lines, off_lines = log_on.decode().splitlines(), log_off.decode().splitlines()
    differ = next((k for k, (a, b) in enumerate(zip(on_lines, off_lines)) if a != b), None)
    at = int(on_lines[differ - 1].split()[1]) if differ else None
    check(at is not None and at >= first, f"the frame log equals the mod-off run's up to the first skip (frame {first}; "
          f"the first difference after frame {at})")


GS_SCRIPT = MODS / "scripts/global_save.json"
GS_FIELDS = {"map": 0x26C4, "prev_map": 0x26CC, "map_is_new": 0x26D8, "field_last_map": 0x26DC}


def gs_field(dump, name):
    return int.from_bytes(dump[GS_FIELDS[name]:GS_FIELDS[name] + 4], "little", signed=True)


def gs_stable_slot(dump):
    """gamestate_data's slot bytes (4..0x26C4) with layer 2's volatile ranges zeroed."""
    from replay import VOLATILE_RANGES
    stable = bytearray(dump)
    for lo, hi in VOLATILE_RANGES:
        stable[lo:hi] = bytes(hi - lo)
    return bytes(stable[4:0x26C4])


def global_save(binary, out):
    """global_save: a save from the field menu on a map that is not an inn's, Back, a reset and its load (the docstring)."""
    print("mods: global_save (tests/port/mods/scripts/global_save.json, a save from the lab and its load)")
    runs = {}
    for label, restore in (("gs_on", True), ("gs_norestore", False)):
        cfg = out / f"{label}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": f"{label}_1.mcd", "memcard2": None,
                                   "mods": {"global_save": {"enabled": True, "restore_map_state": restore}}}))
        (out / f"{label}_1.mcd").unlink(missing_ok=True)
        dumps = out / f"{label}_dumps"
        dumps.mkdir()
        rc, _, rec, err = run_port(binary, out, label, GS_SCRIPT, cfg, True, {"DW3_PORT_CHECKPOINT_DIR": str(dumps)})
        if rc != 0 or rec is None:
            print("    " + "\n    ".join(err.splitlines()[-5:]))
        d = {f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in dumps.glob("cp*.bin")}
        runs[label] = (rc, rec, d, err)
    rc, rec, d, err = runs["gs_on"]
    cps = {c["name"]: c for c in rec["checkpoints"]} if rec else {}
    names = ("menu_open", "saved_anywhere", "back_after_save", "loaded_anywhere", "loaded_settled")
    check(rc == 0 and all(n in d for n in names), f"the run completes with its checkpoints (exit {rc})")
    if not all(n in d for n in names):
        return
    check(cps["saved_anywhere"]["stage"] == 12 and cps["saved_anywhere"]["map"] == 0xC00 and
          all(cps[n]["map"] == 0x206 and cps[n]["stage"] == 2 for n in ("menu_open", "back_after_save", "loaded_anywhere")),
          "SAVE on the lab (map 0x206) opens STGMCARD as map 0xC00; Back and the load return to the lab")
    check(gs_stable_slot(d["saved_anywhere"]) == gs_stable_slot(d["loaded_anywhere"]) ==
          gs_stable_slot(d["back_after_save"]) == gs_stable_slot(d["menu_open"]),
          "the slot's bytes are the same when the menu opened, saved, back on the field and loaded")
    # loaded_anywhere is taken as FIELDSTG loads, before its fieldstg_update_main sets map_is_new; loaded_settled after
    check(gs_field(d["loaded_settled"], "map_is_new") == 0 and gs_field(d["loaded_settled"], "prev_map") == 0xC00 and
          gs_field(d["loaded_settled"], "field_last_map") == 0x206 and
          "the map's state restored (map 0x206" in err and "the map's state saved with the slot (map 0x206" in err,
          "the load resumes the lab as a return to it (map_is_new 0 once FIELDSTG runs; the record saved and restored)")
    card = (out / "gs_on_1.mcd").read_bytes()
    check(card[0x2000 + 0x300 + 0x26C4:][:5] == b"GSAV\x01",
          "the card's slot 1 tail (block 1, file offset 0x300 + 0x26C4) carries the record, version 1")
    rc2, rec2, d2, err2 = runs["gs_norestore"]
    check(rc2 == 0 and "loaded_settled" in d2 and gs_field(d2["loaded_settled"], "map_is_new") == 1 and
          gs_stable_slot(d2["loaded_anywhere"]) == gs_stable_slot(d["saved_anywhere"]) and
          "not restored (restore_map_state off)" in err2,
          f"restore_map_state off: the same slot, the map entered fresh (map_is_new 1) (exit {rc2})")
    rc3, _, rec3, err3 = run_port(binary, out, "gs_off", GS_SCRIPT)
    check(rc3 == 5 and rec3 is not None and "wait_stage) timed out" in err3 and
          all(c["name"] != "saved_anywhere" for c in rec3["checkpoints"]) and rec3["map_sequence"][-1]["map"] == 0x1000,
          f"with the mod off the menu has no SAVE: the press opens STATUS (map 0x1000) and the wait for STGMCARD times out (exit {rc3})")


FT_SPEEDS = ("normal", "fast", "instant")

# gamestate_data's field_map and player_pos (x, y) in a checkpoint dump
TR_FIELDS = {"field_map": 0x34, "x": 0x38, "y": 0x3C}


def tr_field(dump, name):
    return int.from_bytes(dump[TR_FIELDS[name]:TR_FIELDS[name] + 4], "little", signed=True)


def fast_travel(binary, out):
    """fast_travel: MAP from the lab's field menu, Asuka City picked, the menus left (the docstring)."""
    print("mods: fast_travel (global_save.json's route to the lab's field menu, MAP, Asuka City picked)")
    gs = json.loads(GS_SCRIPT.read_text())["steps"]
    j = next(k for k, s in enumerate(gs) if s.get("type") == "checkpoint" and s.get("name") == "menu_open")
    tap = {"type": "press", "frames": 2, "release": 40}
    steps = gs[:j + 1] + [
        {"type": "press", "buttons": ["DOWN"], "frames": 2, "release": 8, "repeat": 2,
         "comment": "cursor 0 (ITEMS) to 2 (MAP)"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 8, "until": {"type": "wait_stage", "stage": 16},
         "timeout": 1500, "comment": "MAP: STSTATUS (map 0x1000) opens the map page"},
        {"type": "wait_frames", "frames": 400, "comment": "the files load and the page sets up: the cursor on the current area"},
        dict(tap, buttons=["SQUARE"], comment="the Amaterasu map"),
        dict(tap, buttons=["SQUARE"], comment="the Asuka map again"),
        dict(tap, buttons=["CROSS"], comment="the area under the cursor: Asuka City"),
        {"type": "checkpoint", "name": "picked"},
        dict(tap, buttons=["TRIANGLE"], comment="close the page: the field menu"),
        {"type": "press", "buttons": ["TRIANGLE"], "frames": 2, "release": 8, "until": {"type": "wait_stage", "stage": 2},
         "timeout": 1500, "comment": "leave the menu: back to the field"},
        {"type": "wait_frames", "frames": 400, "comment": "the map title fades; the menu needs no event running"},
        {"type": "press", "buttons": ["START"], "frames": 2, "release": 150, "comment": "the field menu again"},
        {"type": "checkpoint", "name": "menu_again"},
    ]
    script = derived(out, "tr", steps)
    runs = {}
    for label, mods in (("tr_on", {"fast_travel": {"enabled": True}}), ("tr_off", {})):
        cfg = out / f"{label}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": str(out / f"{label}_1.mcd"), "memcard2": str(out / f"{label}_2.mcd"),
                                   "mods": mods}))
        dumps = out / f"{label}_dumps"
        dumps.mkdir()
        rc, _, rec, err = run_port(binary, out, label, script, cfg, True, {"DW3_PORT_CHECKPOINT_DIR": str(dumps)})
        if rc != 0 or rec is None:
            print("    " + "\n    ".join(err.splitlines()[-5:]))
        d = {f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in dumps.glob("cp*.bin")}
        runs[label] = (rc, rec, d, err)
    rc, rec, d, err = runs["tr_on"]
    check(rc == 0 and "picked" in d and "menu_again" in d, f"on: the run completes with its checkpoints (exit {rc})")
    if "picked" not in d or "menu_again" not in d:
        return
    maps = [m["map"] for m in rec["map_sequence"]]
    picked = (tr_field(d["picked"], "field_map"), tr_field(d["picked"], "x"), tr_field(d["picked"], "y"))
    check(picked == (0x200, 186824, 102500) and "area 20 of region 0: map 0x200" in err,
          f"on: the pick sets field_map 0x200 and player_pos (186824, 102500), logged ({picked[0]:#x}, {picked[1:]})")
    again = (tr_field(d["menu_again"], "field_map"), tr_field(d["menu_again"], "x"), tr_field(d["menu_again"], "y"))
    check(maps[-2:] == [0x1000, 0x200] and again == (0x200, 186824, 102500),
          f"on: leaving the menus enters Asuka City (map 0x200 after 0x1000) at the destination "
          f"({', '.join(hex(m) for m in maps[-2:])}; {again[0]:#x} {again[1:]})")
    rc, rec, d, err = runs["tr_off"]
    maps = [m["map"] for m in rec["map_sequence"]] if rec else []
    check(rc == 0 and maps[-2:] == [0x1000, 0x206] and "menu_again" in d and
          tr_field(d["menu_again"], "field_map") == 0x206 and "fast travel:" not in err,
          f"off: the presses change nothing, back on the lab (map 0x206) (exit {rc}; "
          f"{', '.join(hex(m) for m in maps[-2:])})")


def fast_transitions(binary, out):
    """fast_transitions: global_save's script with global_save alone and with the mod at each speed (the docstring)."""
    print("mods: fast_transitions (tests/port/mods/scripts/global_save.json with global_save, the mod off and at each speed)")
    runs = {}
    for label, speed in (("ft_off", None), ("ft_normal", "normal"), ("ft_fast", "fast"), ("ft_instant", "instant"),
                         ("ft_instant2", "instant")):
        mods = {"global_save": {"enabled": True}}
        if speed:
            mods["fast_transitions"] = {"enabled": True, "speed": speed}
        cfg = out / f"{label}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": f"{label}_1.mcd", "memcard2": None, "mods": mods}))
        (out / f"{label}_1.mcd").unlink(missing_ok=True)
        dumps = out / f"{label}_dumps"
        dumps.mkdir()
        rc, log, rec, err = run_port(binary, out, label, GS_SCRIPT, cfg, True, {"DW3_PORT_CHECKPOINT_DIR": str(dumps)})
        if rc != 0 or rec is None:
            print("    " + "\n    ".join(err.splitlines()[-5:]))
        d = {f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in dumps.glob("cp*.bin")}
        runs[label] = (rc, log, rec, d, err)
    names = ("menu_open", "saved_anywhere", "back_after_save", "loaded_anywhere", "loaded_settled")
    span = {}  # label -> (frames from menu_open to back_after_save, frames from map 0x203 to 0x204)
    for label in ("ft_off", "ft_fast", "ft_instant"):
        rc, log, rec, d, err = runs[label]
        ok = rc == 0 and rec is not None and all(n in d for n in names)
        check(ok and gs_stable_slot(d["saved_anywhere"]) == gs_stable_slot(d["loaded_anywhere"]) ==
              gs_stable_slot(d["back_after_save"]) == gs_stable_slot(d["menu_open"]),
              f"{label}: the save and its load complete (exit {rc}) with the same slot bytes from the menu to the load")
        if not ok:
            return
        cps = {c["name"]: c["frame"] for c in rec["checkpoints"]}
        maps = [(m["map"], m["frame"]) for m in rec["map_sequence"]]
        walk = next(b[1] - a[1] for a, b in zip(maps, maps[1:]) if (a[0], b[0]) == (0x203, 0x204))
        span[label] = (cps["back_after_save"] - cps["menu_open"], walk)
    for label, speed in (("ft_fast", "fast"), ("ft_instant", "instant")):
        check(f"fast transitions: {speed}" in runs[label][4], f"{label}: the speed logged")
    (off_save, off_map), (fast_save, fast_map), (inst_save, inst_map) = span["ft_off"], span["ft_fast"], span["ft_instant"]
    check(off_save > fast_save > inst_save, f"the field menu, the save screen and back: {off_save} frames off, "
          f"{fast_save} fast, {inst_save} instant")
    check(off_map > fast_map > inst_map, f"map 0x203 to 0x204 (a walk out, the map's change and title): {off_map} "
          f"frames off, {fast_map} fast, {inst_map} instant")
    check(runs["ft_normal"][1] == runs["ft_off"][1] and runs["ft_normal"][2] == runs["ft_off"][2],
          "speed normal: the frame log and the record equal the mod-off run's")
    check(runs["ft_instant2"][1] == runs["ft_instant"][1] and runs["ft_instant2"][2] == runs["ft_instant"][2],
          "instant twice: the same frame log and record (deterministic)")


PL_CODES = {"english": 2, "french": 3, "german": 5, "italian": 4, "spanish": 6, "usa": 1, "japanese": 0,
            "portuguese_br": 2}
PL_DECKS = 0x628  # gamestate_data.decks[3]: GamestateDeck (0x66 bytes), its name first
NEW_GAME = SCRIPTS / "new_game.json"
NG_EXPECTED = ROOT / "tests/replay/expected/new_game.json"


def pl_boot(binary, out, lang, env=None, label=None):
    """One boot through the debug channel with preset_language on (lang) or off (None): (map, records_language,
    frame, the log) once the opening's map is reached, or after 600 frames on the language select screen."""
    sys.path.insert(0, str(ROOT / "tools"))
    import mcp_game  # noqa: E402
    from mcp_game import Game  # the debug channel's client (stdlib only)
    mods = {"preset_language": {"enabled": True, "language": lang}} if lang else {}
    cfg = settings(out, label or f"pl_{lang or 'off'}", mods)
    rl = mcp_game.symbols(binary).host("records_language").addr
    g = Game.spawn([str(binary), "--config", str(cfg), "--cd-speed", "instant"], cwd=str(ROOT), hold=True,
                   env=dict(os.environ, **(env or {})))
    try:
        r = g.wait(ps1_map=0xE01 if lang == "japanese" else 0xE02, timeout=600)
        st = g.status()
        return st["map"], int.from_bytes(g.peek(rl, 4), "little", signed=True), r["frame"], g.log_text(0)
    finally:
        g.stop()


# The text pack's run: entries of two files the new game reads at the title's New Game (gamestate_init_records):
# ?SNAMEDT entry 11 (the default name) and ?SCRDSHP entries 0x16-0x18 (the deck names), plus a long entry 0x19 that
# grows CRDSHP past its one sector, and one DMG200 entry (table 1) the route does not reach.
TP_NAMEDT, TP_CRDSHP, TP_DMG200 = 0x86 + 2, 0x32 + 2, 0x112 + 2
TP_NAME, TP_DECKS = "Tiago", ["Pasta 1", "Pasta 2", "Pasta 3"]
CDLOAD_ENTRY = 24  # the -m64 CdloadEntry: state s16, marked s16, id s32, last_use s32, 4 padding, buffer pointer


def pl_text_pack_driver(binary, out, text_pack, enc):
    """port/game/text_pack.c against tools/text_pack.py on all 50 English files: a pack with an entry in each (entry 1
    and the last of a table; a container's first table's entry 1 and its last table's last entry), each file run
    through port_text_loaded by tests/port/text_pack_driver.c (compiled with text_pack.c alone)."""
    import disc_files
    import dump_text
    work = out / "tp_driver"
    work.mkdir()
    driver = work / "text_pack_driver"
    gen = Path(binary).resolve().parent / "gen/include"
    subprocess.run(["gcc", "-std=gnu99", "-O2", "-Wall", "-Wextra", "-Werror", "-DPC_PORT", "-DNON_MATCHING",
                    "-I", str(gen), "-iquote", str(ROOT / "include"), "-iquote", str(ROOT), "-I", str(ROOT / "psxstack/include"),
                    "-I", str(ROOT / "psxstack/include/psxstack"), "-I", str(ROOT / "port/game"),
                    str(ROOT / "port/game/text_pack.c"), str(ROOT / "tests/port/text_pack_driver.c"), "-o", str(driver)],
                   check=True)
    ids = list(range(3, 0x15F, 7))
    entries = {}
    for fid in ids:
        tabs = dump_text.tables_of(disc_files.read(fid))
        (t0, o0), (t1, o1) = tabs[0], tabs[-1]
        entries[(fid, text_pack.PLAIN if t0 is None else t0, 1)] = enc(f"Primeira {fid:#x}")
        entries[(fid, text_pack.PLAIN if t1 is None else t1, len(o1) - 1)] = enc(f"Última {fid:#x} " * (fid % 9 * 40))
    text_pack.write_pack(work / "all.pack", entries)
    secs = b"".join(e.sectors.to_bytes(2, "little") for e in disc_files.files())
    (work / "sectors.bin").write_bytes(secs)
    for fid in ids:
        (work / f"{fid}.in").write_bytes(disc_files.read(fid))
    proc = subprocess.run([str(driver), str(work / "all.pack"), str(work / "sectors.bin"), str(work)] + [str(f) for f in ids],
                          capture_output=True, text=True)
    (work / "driver.stderr").write_text(proc.stderr)
    got_secs = dict(tuple(map(int, line.split())) for line in proc.stdout.split("\n") if line)
    wrong = []
    for fid in ids:
        want = text_pack.rebuild(fid, entries)
        n = max(disc_files.files()[fid].sectors, -(-len(want) // 2048))
        res = work / f"{fid}.out"
        if got_secs.get(fid) != n or not res.exists() or res.read_bytes() != want + bytes(n * 2048 - len(want)):
            wrong.append(hex(fid))
    grown = sum(1 for fid in ids if got_secs.get(fid, 0) > disc_files.files()[fid].sectors)
    check(proc.returncode == 0 and not wrong and proc.stderr.count("entries replaced") == len(ids),
          f"port/game/text_pack.c rebuilds all {len(ids)} English files (11 containers, {grown} grown past their "
          f"sectors) as tools/text_pack.py does, byte for byte{': not ' + ', '.join(wrong) if wrong else ''}")


def pl_text_pack(binary, out):
    """portuguese_br's text pack (docs/PORT.md "Text packs"): a test pack written by tools/text_pack.py, the opening
    to the title's New Game through the debug channel with the route's script, and what the game read."""
    sys.path.insert(0, str(ROOT / "tools"))
    import mcp_game  # noqa: E402
    import text_pack
    from mcp_game import Game
    enc = text_pack._encoder()
    entries = {(TP_NAMEDT, text_pack.PLAIN, 11): enc(TP_NAME), (TP_CRDSHP, text_pack.PLAIN, 0x19): enc("Escolha " * 300),
               (TP_DMG200, 1, 2): enc("Ação.")}
    entries.update({(TP_CRDSHP, text_pack.PLAIN, 0x16 + j): enc(n) for j, n in enumerate(TP_DECKS)})
    pack = out / "pt_br.pack"
    text_pack.write_pack(pack, entries)
    pl_text_pack_driver(binary, out, text_pack, enc)
    # an invalid pack: logged, and the game boots in English
    bad = out / "bad.pack"
    bad.write_bytes(b"DW3T" + bytes(40))
    m, rl, frame, log = pl_boot(binary, out, "portuguese_br", {"DW3_PORT_TEXT_PACK": str(bad)}, "pl_bad_pack")
    check(m == 0xE02 and rl == 2 and "is not a valid pack" in log and "plays in English" in log,
          f"portuguese_br, an invalid pack: logged, the opening's map {m:#x}, records_language {rl}")
    # new_game.json from the opening to New Game, the pack named by DW3_PORT_TEXT_PACK
    steps = json.loads(NEW_GAME.read_text())["steps"]
    names = [s.get("name") for s in steps]
    script = derived(out, "pl_text_pack", [{"type": "wait_map", "map": "0xE02", "timeout": 3000}] +
                     steps[3:names.index("new_game_field") + 1] + [{"type": "wait_frames", "frames": 3000}])
    cfg = settings(out, "pl_text_pack", {"preset_language": {"enabled": True, "language": "portuguese_br"}})
    syms = mcp_game.symbols(binary)
    gs, cd = syms.host("gamestate_data").addr, syms.host("cdload_module").addr
    name = enc(TP_NAME)
    g = Game.spawn([str(binary), "--config", str(cfg), "--script", str(script), "--script-mods"], cwd=str(ROOT),
                   hold=True, env=dict(os.environ, DW3_PORT_TEXT_PACK=str(pack)))
    try:
        # the last deck name is the new game's last write from ?SCRDSHP (the default name is written at boot);
        # hold: the game waits for this request, so the wait sees the write while ?SCRDSHP is still in the cache
        last = enc(TP_DECKS[-1])
        r = g.wait(addr=gs + PL_DECKS + 0x66 * 2, value=int.from_bytes(last[:4], "little"), timeout=6000)
        g.pause()
        got_name = g.peek(gs + 0x54, 0x18).split(b"\0")[0] + b"\0"
        got_decks = [g.peek(gs + PL_DECKS + 0x66 * j, 0x16).split(b"\0")[0] + b"\0" for j in range(3)]
        cache = g.peek(cd + 8, 64 * CDLOAD_ENTRY)
        loaded = {}
        for k in range(64):
            state, fid = int.from_bytes(cache[k * 24:k * 24 + 2], "little"), int.from_bytes(cache[k * 24 + 4:k * 24 + 8], "little")
            if state == 3 and fid in (TP_NAMEDT, TP_CRDSHP):
                loaded[fid] = int.from_bytes(cache[k * 24 + 16:k * 24 + 24], "little")
        rebuilt = {}
        for fid, buf in loaded.items():
            want = text_pack.rebuild(fid, entries)
            sectors = -(-len(want) // 2048)
            rebuilt[fid] = (g.peek(buf, sectors * 2048) == want + bytes(sectors * 2048 - len(want)), sectors)
        log = g.log_text(0)
    finally:
        g.stop()
    check(r.get("hit") and got_name == name and got_decks == [enc(n) for n in TP_DECKS],
          "portuguese_br with the pack: the title's New Game gives the pack's default name and deck names (?SNAMEDT "
          "11, ?SCRDSHP 0x16-0x18)")
    check(set(rebuilt) == {TP_NAMEDT, TP_CRDSHP} and all(ok for ok, _ in rebuilt.values()) and
          rebuilt[TP_CRDSHP][1] == 2,
          f"the two files in cdload's cache are tools/text_pack.py's rebuild byte for byte (CRDSHP in "
          f"{rebuilt.get(TP_CRDSHP, (0, 0))[1]} sectors, 1 on the disc): {rebuilt}")
    check("text pack: " + str(pack) in log and "file 0x034: 4 entries replaced" in log and
          "file 0x088: 1 entries replaced" in log, "the pack's opening and both rebuilds logged")


def preset_language(binary, out):
    """preset_language: every language boots past the language select screen with its records_language, and a new
    game in French starts with the French text (the docstring)."""
    print("mods: preset_language (boots through the debug channel; new_game's route from the opening in French)")
    for lang, code in PL_CODES.items():
        m, rl, frame, log = pl_boot(binary, out, lang)
        want = 0xE01 if code == 0 else 0xE02
        check(m == want and rl == code, f"{lang}: the opening's map {m:#x} (want {want:#x}) at frame {frame}, "
              f"records_language {rl} (want {code}), no language select screen")
        if lang == "portuguese_br":   # its pack is looked for beside the binary (none in a build tree: English)
            where = str(Path(binary).resolve().parent / "mods/preset_language/pt_br.pack")
            check("text pack: " + where in log, f"{lang}: the text pack looked for at {where}")
    pl_text_pack(binary, out)
    m, rl, frame, _ = pl_boot(binary, out, None)
    check(m == 0x1600 and rl == 2, f"the mod off: still on the language select screen (map {m:#x}) after {frame} "
          f"frames, records_language {rl}")
    # new_game.json from the opening on: its first three steps (CNTY_SEL, its checkpoint, the START that confirms
    # English) become a wait for the opening's map, and the route stops at the first field map's checkpoint
    steps = json.loads(NEW_GAME.read_text())["steps"]
    names = [s.get("name") for s in steps]
    script = derived(out, "preset_language", [{"type": "wait_map", "map": "0xE02", "timeout": 3000}] +
                     steps[3:names.index("new_game_field") + 1])
    cfg = settings(out, "pl_new_game", {"preset_language": {"enabled": True, "language": "french"}})
    dumps = out / "pl_dumps"
    dumps.mkdir()
    rc, _, rec, err = run_port(binary, out, "pl_new_game", script, cfg, True, {"DW3_PORT_CHECKPOINT_DIR": str(dumps)})
    if rc != 0 or rec is None:
        print("    " + "\n    ".join(err.splitlines()[-5:]))
    cps = {c["name"]: c for c in rec["checkpoints"]} if rec else {}
    emu = {c["name"]: c for c in json.loads(NG_EXPECTED.read_text())["checkpoints"]}
    check(rc == 0 and "opening_movie" in cps and "new_game_field" in cps and
          cps["opening_movie"]["frame"] + 100 < emu["opening_movie"]["frame"],
          f"French: new_game's route from the opening to the first field map (exit {rc}); the opening at frame "
          f"{cps.get('opening_movie', {}).get('frame')} (the emulator's, through the screen: {emu['opening_movie']['frame']})")
    dump = next(dumps.glob("cp*_new_game_field.bin"), None)
    if dump is None:
        check(False, "French: the new_game_field checkpoint's dump")
        return
    sys.path.insert(0, str(ROOT / "tools"))
    import disc_files
    import dump_text
    text = disc_files.read(0x32 + PL_CODES["french"])
    offs = dump_text.table_offsets(text)
    want = [text[offs[0x16 + j]:].split(b"\0")[0] for j in range(3)]
    d = dump.read_bytes()
    got = [d[PL_DECKS + 0x66 * j:][:0x16].split(b"\0")[0] for j in range(3)]
    check(got == want, f"French: the new game's deck names are the French text file's (0x35, entries 0x16-0x18): {got}")


PX_GAINS = {"px_on": (4, 2, 2), "px_full": (4, 4, 4), "px_off": (4, 0, 0)}
FIRST_BATTLE_MONEY = 50  # stfgtrep_rewards[records_battle_results.battle].money of the first battle


def px_party_exp(dump):
    """The party's (Digimon, exp) from a gamestate_data dump: party[3] at 0x70, digimon[i].record.exp at
    0x75C + i * 0x3DC + 0xC + 0x18."""
    party = [int.from_bytes(dump[0x70 + 4 * k:0x74 + 4 * k], "little", signed=True) for k in range(3)]
    return [(d, int.from_bytes(dump[0x780 + d * 0x3DC:0x784 + d * 0x3DC], "little", signed=True)) for d in party]


def gs_money(dump):
    return int.from_bytes(dump[0x6C:0x70], "little", signed=True)


def first_battle_runs(binary, out, runs):
    """first_battle_save's route to back_on_field once per (label, mods) of `runs` (mods None: off). Yields (label,
    mods, the party's exp gains and the money gain from battle_won to battle_end, stderr, the battle_end dump) for the
    runs that reach back_on_field with battle_start and battle_won at the emulator's stable hashes (the mods act in the
    report only)."""
    fbs = json.loads((SCRIPTS / "first_battle_save.json").read_text())["steps"]
    j = next(k for k, s in enumerate(fbs) if s.get("type") == "checkpoint" and s.get("name") == "back_on_field")
    script = derived(out, "first_battle", fbs[:j + 1])
    emu = {c["name"]: c for c in json.loads(FBS_EXPECTED.read_text())["checkpoints"]}
    for label, mods in runs:
        dumps = out / f"{label}_dumps"
        dumps.mkdir()
        cfg = settings(out, label, mods) if mods else None
        rc, _, rec, err = run_port(binary, out, label, script, cfg, mods is not None,
                                   {"DW3_PORT_CHECKPOINT_DIR": str(dumps)})
        d = {f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in dumps.glob("cp*.bin")}
        if rc != 0 or "battle_end" not in d:
            check(False, f"{label}: the run reaches back_on_field (exit {rc})")
            print("    " + "\n    ".join(err.splitlines()[-5:]))
            continue
        cps = {c["name"]: c for c in rec["checkpoints"]}
        before = ("battle_start", "battle_won")
        check(all(cps[n]["gamestate_sha1_stable"] == emu[n]["gamestate_sha1_stable"] for n in before),
              f"{label}: battle_start and battle_won have the emulator's stable hashes")
        won, end = px_party_exp(d["battle_won"]), px_party_exp(d["battle_end"])
        gains = tuple(e[1] - w[1] for w, e in zip(won, end))
        yield label, mods, won, gains, gs_money(d["battle_end"]) - gs_money(d["battle_won"]), err, d["battle_end"]


def party_xp(binary, out):
    """party_xp: the first battle's report with the mod on (share 50, share 100) and off (the docstring)."""
    print("mods: party_xp (first_battle_save to back_on_field: the first battle, one member of three fights)")
    runs = (("px_on", {"party_xp": {"enabled": True}}),
            ("px_full", {"party_xp": {"enabled": True, "share": 100, "catch_up": True}}),
            ("px_off", None))
    for label, mods, won, gains, money, err, _ in first_battle_runs(binary, out, runs):
        shares = err.count("party xp: frame")
        check(gains == PX_GAINS[label] and shares == (3 if mods else 0),
              f"{label}: the party (Digimon {', '.join(str(w[0]) for w in won)}) gains {gains}, "
              f"want {PX_GAINS[label]}; {shares} share(s) logged")
        check(money == FIRST_BATTLE_MONEY, f"{label}: the money gain {money}, want {FIRST_BATTLE_MONEY}")


# label -> (the party's exp gains, the money gain)
XB_WANT = {"xb_on": ((12, 0, 0), 125), "xb_party": ((40, 20, 20), 500), "xb_capped": ((20, 0, 0), 250)}


def xp_boost(binary, out):
    """xp_boost: the first battle's report with the mod on (the docstring)."""
    print("mods: xp_boost (first_battle_save to back_on_field: the first battle, 4 experience and 50 bits)")
    runs = (("xb_on", {"xp_boost": {"enabled": True, "exp": 3, "bits": 2.5}}),
            ("xb_party", {"xp_boost": {"enabled": True, "exp": 10, "bits": 10, "manual": True},
                          "party_xp": {"enabled": True}}),
            ("xb_capped", {"xp_boost": {"enabled": True, "exp": 10, "bits": 10}}))
    for label, mods, won, gains, money, err, _ in first_battle_runs(binary, out, runs):
        want = XB_WANT[label]
        boosts = [line.split(": ", 2)[2] for line in err.splitlines() if "xp boost: frame" in line]
        check((gains, money) == want and len(boosts) == 2,
              f"{label}: the party gains {gains} and {money} bits, want {want[0]} and {want[1]} "
              f"({'; '.join(boosts)})")


def lr_values(dump, digimon):
    """digimon[i].record.stats.values[0..5] (level, TP, HP, max HP, MP, max MP) from a gamestate_data dump: at
    0x75C + i * 0x3DC + 0xC + 0x1C."""
    at = 0x784 + digimon * 0x3DC
    return tuple(int.from_bytes(dump[at + 2 * k:at + 2 * k + 2], "little", signed=True) for k in range(6))


def level_up_recover(binary, out):
    """level_up_recover: the first battle's report with xp_boost's 10x (40 experience: member 0 levels up), with
    the mod on and off (the docstring)."""
    print("mods: level_up_recover (first_battle_save to back_on_field: the first battle, 10x experience)")
    boost = {"xp_boost": {"enabled": True, "exp": 10, "manual": True}}
    runs = (("lr_on", dict(boost, level_up_recover={"enabled": True})), ("lr_off", boost))
    for label, mods, won, gains, money, err, end in first_battle_runs(binary, out, runs):
        lv, _, hp, max_hp, mp, max_mp = lr_values(end, won[0][0])
        logged = err.count("level up recover: frame")
        if "level_up_recover" in mods:
            check(lv > 1 and (hp, mp) == (max_hp, max_mp) and logged == 1,
                  f"{label}: member 0 at level {lv}, HP {hp}/{max_hp}, MP {mp}/{max_mp} (full); {logged} recover(s) logged")
        else:
            check(lv > 1 and hp < max_hp and logged == 0,
                  f"{label}: member 0 at level {lv}, HP {hp}/{max_hp}, MP {mp}/{max_mp} (the battle's HP kept)")


TS_TP = 0x80048D34 + 0x784 + 2   # gamestate_data.digimon[0].record.stats.values[1] (TP): member 0 is Digimon 0
TS_POWER = 0x790                 # its stats.stats[0] (Power) in a dump
TS_LAST_BONUS = 0xB34            # its last_bonus_training in a dump


def ts_byte_le(dump, at, size):
    return int.from_bytes(dump[at:at + size], "little", signed=True)


def training_success(binary, out):
    """training_success: one Punch session at level 0 on map 0x21D's trainer, the mod on and off (the docstring)."""
    print("mods: training_success (first_battle_save to map 0x21D's trainer, one Punch session at level 0)")
    fbs = json.loads((SCRIPTS / "first_battle_save.json").read_text())["steps"]
    j = next(k for k, s in enumerate(fbs) if s.get("type") == "checkpoint" and s.get("name") == "train_open")
    session = [
        {"type": "write_mem", "addr": hex(TS_TP), "size": 2, "value": 1,
         "comment": "TP 1: one level-0 training (stgtrain_tp_costs[0]), the next one refused"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 60,
         "until": {"type": "wait_mem", "addr": hex(TS_TP), "size": 2, "value": 0}, "timeout": 3000,
         "comment": "the first party member, Punch (cursor 0), level 0, YES: the TP spent, the session starts"},
        {"type": "checkpoint", "name": "train_start"},
        {"type": "press", "buttons": ["CROSS"], "frames": 2, "release": 60, "repeat": 30,
         "comment": "the rounds play by themselves; CROSS shows the result and closes it; the later presses start "
                    "another training, refused (no TP), and close the warning"},
    ]
    # first_battle_save's way out of STGTRAIN (from its 14 CROSS presses on), whichever screen the presses ended on
    script = derived(out, "ts", fbs[:j + 1] + session + fbs[j + 2:])
    for label, mods in (("ts_on", {"training_success": {"enabled": True}}),
                        ("ts_game", {"training_success": {"enabled": True, "bonus_round": "game"}}), ("ts_off", {})):
        cfg = out / f"{label}.settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False},
                                   "memcard1": str(out / f"{label}_1.mcd"), "memcard2": str(out / f"{label}_2.mcd"),
                                   "mods": mods}))
        d = out / f"{label}_dumps"
        d.mkdir()
        rc, _, rec, err = run_port(binary, out, label, script, cfg, True, {"DW3_PORT_CHECKPOINT_DIR": str(d)})
        cps = {f.name.split("_", 1)[1][:-4]: f.read_bytes() for f in d.glob("cp*.bin")}
        chances = [line.split("frame ", 1)[1].split(": ", 1)[1] for line in err.splitlines()
                   if "training success: frame" in line]
        if rc != 0 or "train_left" not in cps:
            check(False, f"{label}: the run leaves STGTRAIN after the session (exit {rc})")
            print("    " + "\n    ".join(err.splitlines()[-5:]))
            continue
        start, end = cps["train_start"], cps["train_left"]
        gain = ts_byte_le(end, TS_POWER, 2) - ts_byte_le(start, TS_POWER, 2)
        bonus = ts_byte_le(end, TS_LAST_BONUS, 1)
        tp = ts_byte_le(end, TS_TP - 0x80048D34, 2)
        if label == "ts_game":
            check(chances == ["round chance 75 -> 100", "bonus round chance 75 -> 75"] and tp == 0 and gain >= 3,
                  f"{label}: the rounds succeed (Power +{gain}), the bonus round keeps the game's chance "
                  f"(last_bonus_training {bonus}); chances {chances}")
        elif mods:
            # three rounds of 1 + r2 and the won bonus round's 2 (stgtrain_stat_gains rows 0 and 3)
            check(chances == ["round chance 75 -> 100", "bonus round chance 75 -> 100"] and bonus == 1 and
                  5 <= gain <= 8 and tp == 0,
                  f"{label}: every round succeeds: Power +{gain} (want 5..8), last_bonus_training {bonus} (want 1, "
                  f"Punch), TP {tp}; chances {chances}")
        else:
            check(not chances and tp == 0 and 0 <= gain <= 8,
                  f"{label}: the game's session: Power +{gain}, last_bonus_training {bonus}, TP {tp}, nothing logged")


# Every mod's test, the longest first (the parallel run's critical path): name -> function(binary, out, record).
MODS_TESTS = {
    "preset_language": lambda binary, out, record: preset_language(binary, out),
    "skip_dialogues": skip_dialogues,
    "fast_transitions": lambda binary, out, record: fast_transitions(binary, out),
    "random_encounters": lambda binary, out, record: random_encounters(binary, out),
    "party_xp": lambda binary, out, record: party_xp(binary, out),
    "xp_boost": lambda binary, out, record: xp_boost(binary, out),
    "level_up_recover": lambda binary, out, record: level_up_recover(binary, out),
    "global_save": lambda binary, out, record: global_save(binary, out),
    "battle_animations": lambda binary, out, record: battle_animations(binary, out),
    "widescreen": lambda binary, out, record: widescreen(binary, out),
    "fast_travel": lambda binary, out, record: fast_travel(binary, out),
    "training_success": lambda binary, out, record: training_success(binary, out),
}


def run_child(name, mode, out, binary):
    """One mod's test in its own process (--only); returns (name, exit code, its output)."""
    cmd = [sys.executable, str(Path(__file__).resolve()), mode, "--only", name, "--out", str(out / name),
           "--binary", str(binary)]
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    return name, proc.returncode, proc.stdout + (proc.stderr if proc.returncode not in (0, 1) else "")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("mode", nargs="?", default="check", choices=("check", "record"))
    ap.add_argument("--out", help="scratch directory (default build/port-test/mods)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None,
                    help="mods run at once, and the build's jobs (default: $DW3_JOBS, else every mod at once)")
    ap.add_argument("--only", action="append", choices=sorted(MODS_TESTS), metavar="MOD",
                    help="only these mods' tests, in this process one after another (repeatable)")
    ap.add_argument("--binary", help=argparse.SUPPRESS)   # the parallel run's children: the port built by the parent
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
    binary = Path(args.binary) if args.binary else build("build/port", [], args.jobs, env)
    if args.only:
        for name in args.only:
            MODS_TESTS[name](binary, out, args.mode == "record")
    else:
        names = list(MODS_TESTS)
        with ThreadPoolExecutor(max_workers=min(args.jobs or len(names), len(names))) as pool:
            for name, rc, text in pool.map(lambda n: run_child(n, args.mode, out, binary), names):
                print(text, end="", flush=True)
                if rc != 0:
                    FAILURES.append(f"{name}: exit {rc}")
                    if rc != 1:
                        print(f"  FAIL {name}: exit {rc}")
    if not args.binary:   # a child's summary is its parent's
        print(f"mods test: {'FAIL (' + str(len(FAILURES)) + (' check(s))' if args.only else ' mod(s))') if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
