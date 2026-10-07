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
  - each of the seven languages, booted through the debug channel (tools/mcp/game.py), reaches the opening's map
    (0xE02; 0xE01 for Japanese) with its records_language, without the language select screen; with the mod off the
    game is still on that screen (map 0x1600) 600 frames in;
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


PL_CODES = {"english": 2, "french": 3, "german": 5, "italian": 4, "spanish": 6, "usa": 1, "japanese": 0}
PL_DECKS = 0x628  # gamestate_data.decks[3]: GamestateDeck (0x66 bytes), its name first
NEW_GAME = SCRIPTS / "new_game.json"
NG_EXPECTED = ROOT / "tests/replay/expected/new_game.json"


def pl_boot(binary, out, lang):
    """One boot through the debug channel with preset_language on (lang) or off (None): (map, records_language,
    frame) once the opening's map is reached, or after 600 frames on the language select screen."""
    sys.path.insert(0, str(ROOT / "tools/mcp"))
    from game import Game  # the debug channel's client (stdlib only)
    from symbols import Symbols
    mods = {"preset_language": {"enabled": True, "language": lang}} if lang else {}
    cfg = settings(out, f"pl_{lang or 'off'}", mods)
    rl = Symbols(ROOT, binary).host("records_language").addr
    g = Game.spawn([str(binary), "--config", str(cfg), "--cd-speed", "instant"], cwd=str(ROOT), hold=True)
    try:
        r = g.wait(ps1_map=0xE01 if lang == "japanese" else 0xE02, timeout=600)
        st = g.status()
        return st["map"], int.from_bytes(g.peek(rl, 4), "little", signed=True), r["frame"]
    finally:
        g.stop()


def preset_language(binary, out):
    """preset_language: every language boots past the language select screen with its records_language, and a new
    game in French starts with the French text (the docstring)."""
    print("mods: preset_language (boots through the debug channel; new_game's route from the opening in French)")
    for lang, code in PL_CODES.items():
        m, rl, frame = pl_boot(binary, out, lang)
        want = 0xE01 if code == 0 else 0xE02
        check(m == want and rl == code, f"{lang}: the opening's map {m:#x} (want {want:#x}) at frame {frame}, "
              f"records_language {rl} (want {code}), no language select screen")
    m, rl, frame = pl_boot(binary, out, None)
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
    mods, the party's exp gains and the money gain from battle_won to battle_end, stderr) for the runs that reach
    back_on_field with battle_start and battle_won at the emulator's stable hashes (the mods act in the report only)."""
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
        yield label, mods, won, gains, gs_money(d["battle_end"]) - gs_money(d["battle_won"]), err


def party_xp(binary, out):
    """party_xp: the first battle's report with the mod on (share 50, share 100) and off (the docstring)."""
    print("mods: party_xp (first_battle_save to back_on_field: the first battle, one member of three fights)")
    runs = (("px_on", {"party_xp": {"enabled": True}}),
            ("px_full", {"party_xp": {"enabled": True, "share": 100, "catch_up": True}}),
            ("px_off", None))
    for label, mods, won, gains, money, err in first_battle_runs(binary, out, runs):
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
    for label, mods, won, gains, money, err in first_battle_runs(binary, out, runs):
        want = XB_WANT[label]
        boosts = [line.split(": ", 2)[2] for line in err.splitlines() if "xp boost: frame" in line]
        check((gains, money) == want and len(boosts) == 2,
              f"{label}: the party gains {gains} and {money} bits, want {want[0]} and {want[1]} "
              f"({'; '.join(boosts)})")


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
    global_save(binary, out)
    preset_language(binary, out)
    party_xp(binary, out)
    xp_boost(binary, out)
    print(f"mods test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
