#!/usr/bin/env python3
"""The port's settings file (`--config FILE`, docs/LAUNCHER.md "Settings file"): the contract with the launcher.

Usage: tests/port/settings.py [--out DIR] [-j N] [--no-sdl]

Builds the headless port if needed (tests/port/run.py's build), then checks, with `--print-settings`:
  - the round trip: a file's effective settings, printed and loaded again from another directory, print the same;
  - the mods: every port/mods/<id>/mod.json equal to the game's registry (--print-mods) and copied beside the binary,
    the values read and resolved (defaults filled in), unknown mods and options logged, mods off under --script
    unless --script-mods;
  - the defaults (every key absent), relative paths resolved against the file's directory, null memory cards;
  - the command line overriding the file (--disc, --scale, --memcard1 none, --watchdog);
  - the errors: exit 64 with the key named (no schema, a newer schema, a wrong type, out of range, not JSON, a missing
    file, --print-settings without --config); an unknown key is logged and ignored;
the input and binding errors; with build/port-sdl, --input-test (offscreen) with the defaults and with
tests/port/settings/rebound.json (rebound keys and pads, chord hotkeys, the pause's round trip);
and, when the disc is there, that a run under `--config` (headless, cards in a scratch directory) replays new_game
with the bare run's log and record byte for byte, and creates the memory card files.
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
from run import DISC, SCRIPTS, Missing, build, tool_env  # noqa: E402  (the port test's build)

FAILURES = []


def check(cond, what):
    print(f"  {'ok  ' if cond else 'FAIL'} {what}")
    if not cond:
        FAILURES.append(what)


def run(binary, *args, cwd=None):
    proc = subprocess.run([str(binary), *map(str, args)], cwd=cwd or ROOT, capture_output=True, text=True,
                          timeout=120)
    return proc.returncode, proc.stdout, proc.stderr


def write(path, obj):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(obj if isinstance(obj, str) else json.dumps(obj, indent=2))
    return path


def printed(binary, config, *extra):
    rc, out, err = run(binary, "--config", config, "--print-settings", *extra)
    if rc != 0:
        raise RuntimeError(f"--print-settings {config}: exit {rc}: {err.strip()}")
    return out, json.loads(out), err


MODS = ROOT / "port/mods"
REBOUND = ROOT / "tests/port/settings/rebound.json"
OPTION_KEYS = {"id", "name", "description", "type", "default", "group", "applies", "min", "max", "step", "values"}


def mods_check(binary):
    """port/mods/<id>/mod.json against the game's registry (--print-mods), and the copies beside the binary."""
    rc, out, err = run(binary, "--print-mods")
    registry = {m["id"]: m for m in json.loads(out)} if rc == 0 else {}
    check(rc == 0 and registry, f"--print-mods: exit {rc}, {len(registry)} mod(s)")
    manifests = {}
    for path in sorted(MODS.glob("*/mod.json")):
        man = json.loads(path.read_text())
        manifests[man.get("id")] = man
        problems = []
        if path.parent.name != man.get("id"):
            problems.append(f"the directory is not the id {man.get('id')!r}")
        for key in ("schema", "id", "name", "version", "kind", "requires_port", "description", "options"):
            if key not in man:
                problems.append(f"no {key}")
        if man.get("schema") != 1:
            problems.append("schema is not 1")
        reg = registry.get(man.get("id"))
        if reg is None:
            problems.append("not in the game's registry")
        else:
            for key in ("version", "kind", "requires_port"):
                if man.get(key) != reg[key]:
                    problems.append(f"{key} {man.get(key)!r}, the game's {reg[key]!r}")
            got = [o.get("id") for o in man.get("options", [])]
            want = [o["id"] for o in reg["options"]]
            if got != want:
                problems.append(f"options {got}, the game's {want}")
            for mo, ro in zip(man.get("options", []), reg["options"]):
                where = f"option {mo.get('id')}"
                if set(mo) - OPTION_KEYS:
                    problems.append(f"{where}: unknown keys {sorted(set(mo) - OPTION_KEYS)}")
                if not mo.get("name") or not mo.get("description"):
                    problems.append(f"{where}: no name or description")
                for key in ("type", "default", "min", "max", "step"):
                    if mo.get(key) != ro.get(key):
                        problems.append(f"{where}: {key} {mo.get(key)!r}, the game's {ro.get(key)!r}")
                if mo.get("applies", "restart") != ro["applies"]:
                    problems.append(f"{where}: applies {mo.get('applies')!r}, the game's {ro['applies']!r}")
                if ro["type"] == "enum":
                    values = mo.get("values", [])
                    if [v.get("id") for v in values] != ro["values"] or not all(v.get("label") for v in values):
                        problems.append(f"{where}: values {values}, the game's ids {ro['values']} (each with a label)")
        check(not problems, f"{path.relative_to(ROOT)}: {'; '.join(problems) or 'equals the registry'}")
        copy = binary.parent / "mods" / path.parent.name / "mod.json"
        check(copy.exists() and copy.read_bytes() == path.read_bytes(), f"its copy beside the binary ({copy.parent})")
    missing = sorted(set(registry) - set(manifests))
    check(not missing, f"every mod of the registry has a manifest{': missing ' + ', '.join(missing) if missing else ''}")


FF_ON, FF_OFF = 100, 25  # DW3_PORT_FAST_FORWARD's pattern (vsyncs on, off)


def ff_stretches(err):
    """The fast-forward log lines -> [(on, frame, seconds)]."""
    out = []
    for line in err.splitlines():
        if line.startswith("port: fast-forward: on at frame ") or line.startswith("port: fast-forward: off at frame "):
            words = line.split()
            out.append((words[2] == "on", int(words[5].rstrip(",")), float(words[6])))
    return out


def fast_forward_check(sdl, env, out):
    """Fast-forward (docs/LAUNCHER.md "Fast-forward") in the window (offscreen): new_game with the mod on and the test
    pattern; the log and the record are the bare run's; every off stretch keeps PAL's pace from its first vsync (no
    stall: the schedule starts over), the on stretches run at 4x or faster, the audio is muted, the presents capped."""
    print("settings: fast-forward (build/port-sdl, offscreen): new_game with DW3_PORT_FAST_FORWARD=%d:%d" % (FF_ON, FF_OFF))
    c = out / "ff"
    script = SCRIPTS / "new_game.json"
    for speed, mute in (("4x", True), ("unlimited", False)):
        cfg = write(c / f"{speed}.json", {"schema": 1, "disc": {"path": str(DISC)}, "memcard1": None, "memcard2": None,
                                          "mods": {"fast_forward": {"enabled": True, "speed": speed, "mute": mute}}})
        cmd = [str(sdl), "--config", str(cfg), "--script", str(script), "--script-mods",
               "--log", str(c / f"{speed}.log"), "--record", str(c / f"{speed}.json")]
        if speed != "4x":
            cmd += ["--max-frames", "400"]
        proc = subprocess.run(cmd, cwd=ROOT, env=dict(env, DW3_PORT_FAST_FORWARD=f"{FF_ON}:{FF_OFF}"),
                              capture_output=True, text=True, timeout=300)
        st = ff_stretches(proc.stderr)
        offs = [b[2] - a[2] for a, b in zip(st, st[1:]) if not a[0] and b[0]]
        ons = [b[2] - a[2] for a, b in zip(st, st[1:]) if a[0] and not b[0]]
        paced = FF_OFF / 50
        check(proc.returncode == 0 and len(offs) >= 3, f"{speed}: exit {proc.returncode}, {len(st)} changes")
        # each off stretch is FF_OFF vsyncs at 50: 0.5 s; a schedule that did not start over makes it 2 s
        check(offs and all(paced * 0.8 <= t <= paced + 0.4 for t in offs),
              f"{speed}: every off stretch {paced:.2f} s at PAL's pace, no stall "
              f"({min(offs, default=0):.3f}..{max(offs, default=0):.3f} s)")
        if speed == "4x":
            fastest = (FF_ON - 1) / 200
            check(ons and min(ons) >= fastest * 0.9, f"4x: no on stretch faster than 200 vsyncs a second "
                  f"({min(ons, default=0):.3f}..{max(ons, default=0):.3f} s for {FF_ON - 1} vsyncs)")
            same = (c / "4x.log").read_bytes() == (out / "bare.log").read_bytes() and \
                (c / "4x.json").read_bytes() == (out / "bare.json").read_bytes()
            check(same, "4x: the log and the record are the bare run's")
            muted = [l for l in proc.stderr.splitlines() if "vsyncs muted (fast-forward)" in l]
            presented = [l for l in proc.stderr.splitlines() if "frames presented in" in l]
            frames = json.loads((c / "4x.json").read_text()).get("frames", 0)
            n_presented = int(presented[0].split("window: ")[1].split()[0]) if presented else -1
            check(bool(muted) and 0 < n_presented < frames,
                  f"4x: audio muted ({muted[0].split('audio: ')[1] if muted else 'no report'}), presents capped "
                  f"({n_presented} of {frames} vsyncs)")
        else:
            check(ons and min(ons) < (FF_ON - 1) / 200, f"unlimited: on stretches faster than 4x ({min(ons, default=0):.3f} s)")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", help="scratch directory (default build/port-test/settings)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None)
    ap.add_argument("--no-sdl", action="store_true", help="skip the input self-test even when build/port-sdl exists")
    args = ap.parse_args()
    try:
        env = tool_env()
    except Missing as e:
        print(f"settings test: {e}", file=sys.stderr)
        return 2
    out = (Path(args.out) if args.out else ROOT / "build/port-test/settings").resolve()
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    binary = build("build/port", [], args.jobs, env)

    print("settings: defaults, relative paths")
    a = out / "a"
    write(a / "settings.json", {"schema": 1})
    _, s, _ = printed(binary, a / "settings.json")
    check(s["disc"]["path"] == "" and s["video"] == {"window": True, "scale": 2, "fullscreen": False, "refresh": 50}
          and s["audio"] == {"mute": False} and s["watchdog"] == 0, "the defaults of an empty file")
    check(s["memcard1"] == str(a / "card1.mcd") and s["memcard2"] == str(a / "card2.mcd"),
          "the cards default to card1.mcd and card2.mcd beside the file")
    full = {
        "schema": 1,
        "disc": {"path": "../discs/dw2003.cue", "sha1": "457cb233" + "0" * 32},
        "video": {"window": False, "scale": 3, "fullscreen": True, "refresh": 60},
        "audio": {"mute": True},
        "memcard1": "cards/one.mcd",
        "memcard2": None,
        "watchdog": 30,
        "input": {"keyboard": {"cross": "X"}, "hotkeys": {"pause": "P"}},
        "mods": {"fast_forward": {"enabled": True}},
        "launcher": {"window": [10, 20], "note": "tab\tquote\" é"},
    }
    write(a / "full.json", full)
    text1, s, _ = printed(binary, a / "full.json")
    check(s["disc"]["path"] == str(a / "../discs/dw2003.cue") and s["memcard1"] == str(a / "cards/one.mcd"),
          "relative paths start at the file's directory")
    check(s["memcard2"] is None, "memcard2 null: no card")
    check(s["video"] == full["video"] and s["audio"] == full["audio"] and s["watchdog"] == 30
          and s["disc"]["sha1"] == full["disc"]["sha1"], "every value read")
    check(s["input"] == full["input"] and s["launcher"] == full["launcher"], "input and launcher printed back as they are")
    check(s["mods"]["fast_forward"] == {"enabled": True, "hold": "Tab", "toggle": "", "speed": "4x", "mute": True},
          "mods printed resolved: the manifest's defaults filled in")

    print("settings: the round trip")
    b = write(out / "b/elsewhere/settings.json", text1)
    text2, _, _ = printed(binary, b)
    check(text1 == text2, "printed, loaded from another directory, printed again: the same text")

    print("settings: the command line overrides the file")
    _, s, _ = printed(binary, a / "full.json", "--disc", "x.cue", "--scale", "5", "--memcard1", "none",
                      "--memcard2", "two.mcd", "--watchdog", "7")
    check(s["disc"]["path"] == str(ROOT / "x.cue") and s["video"]["scale"] == 5 and s["video"]["window"]
          and s["memcard1"] is None and s["memcard2"] == str(ROOT / "two.mcd") and s["watchdog"] == 7,
          "--disc (from the current directory), --scale (implies the window), --memcard1 none, --memcard2, --watchdog")

    print("settings: the mods and their manifests")
    mods_check(binary)
    _, s, err = printed(binary, write(out / "mods.json", {"schema": 1, "mods": {
        "fast_forward": {"enabled": True, "speed": "unlimited", "mute": False, "hold": ["F3", ["pad:guide", "pad:north"]],
                         "turbo": 1},
        "someone_elses": {"enabled": True, "x": [1, 2]}}}))
    check(s["mods"]["fast_forward"] == {"enabled": True, "hold": ["F3", ["pad:guide", "pad:north"]], "toggle": "",
                                        "speed": "unlimited", "mute": False}, "a mod's values read")
    check(s["mods"]["someone_elses"] == {"enabled": True, "x": [1, 2]} and
          "mods.someone_elses: no such mod in this build, ignored" in err, "an unknown mod: logged, printed back")
    check("mods.fast_forward.turbo: unknown option, ignored" in err, "an unknown option: logged and ignored")
    _, s, _ = printed(binary, a / "settings.json")
    check(s["mods"]["fast_forward"]["enabled"] is False, "a mod absent from the settings is off")

    print("settings: errors (exit 64, the key named)")
    bad = {
        "no schema": ({}, "schema: missing"),
        "a newer schema": ({"schema": 2}, "newer launcher"),
        "a wrong type": ({"schema": 1, "video": {"scale": "3"}}, "video.scale: a number, not a string"),
        "out of range": ({"schema": 1, "video": {"scale": 17}}, "video.scale: an integer from 1 to 16"),
        "refresh 55": ({"schema": 1, "video": {"refresh": 55}}, "video.refresh: 50 (PAL) or 60"),
        "a card path not a string": ({"schema": 1, "memcard1": 3}, "memcard1: a path"),
        "mods not an object": ({"schema": 1, "mods": []}, "mods: an object, not an array"),
        "not JSON": ('{"schema": 1,}', "not JSON"),
        "not an object": ("[1]", "the settings are an object"),
        "a bad enum": ({"schema": 1, "mods": {"fast_forward": {"speed": "5x"}}}, "mods.fast_forward.speed: one of"),
        "enabled not a bool": ({"schema": 1, "mods": {"fast_forward": {"enabled": 1}}},
                               "mods.fast_forward.enabled: true or false"),
        "a binding not a binding": ({"schema": 1, "mods": {"fast_forward": {"hold": 3}}},
                                    "mods.fast_forward.hold: a binding"),
        "an unknown gamepad input": ({"schema": 1, "mods": {"fast_forward": {"hold": "pad:nosuch"}}},
                                     "\"pad:nosuch\": not a gamepad input"),
        "a chord too long": ({"schema": 1, "input": {"hotkeys": {"pause": [["A", "B", "C", "D", "E"]]}}},
                             "input.hotkeys.pause: a trigger is an input name or a chord"),
        "a gamepad map name": ({"schema": 1, "input": {"gamepad": {"cross": "a"}}},
                               "input.gamepad.cross: \"a\": not a gamepad input"),
    }
    for what, (content, message) in bad.items():
        p = write(out / "bad" / (what.replace(" ", "_") + ".json"), content)
        rc, _, err = run(binary, "--config", p, "--print-settings")
        last = err.strip().splitlines()[-1].split(".json: ", 1)[-1] if err.strip() else ""
        check(rc == 64 and message in err, f"{what}: exit {rc}, {last!r}")
    rc, _, err = run(binary, "--config", out / "missing.json", "--print-settings")
    check(rc == 64 and "cannot read" in err, "a missing file: exit 64")
    rc, _, err = run(binary, "--print-settings")
    check(rc == 64 and "needs --config" in err, "--print-settings without --config: exit 64")
    p = write(out / "unknown.json", {"schema": 1, "frobnicate": 1, "video": {"vsync": True}})
    rc, _, err = run(binary, "--config", p, "--print-settings")
    check(rc == 0 and "frobnicate: unknown key, ignored" in err and "video.vsync: unknown key, ignored" in err,
          "unknown keys: logged and ignored")

    if DISC.exists():
        print("settings: a run under --config replays new_game as the bare binary does")
        script = SCRIPTS / "new_game.json"
        c = out / "run"
        cfg = write(c / "settings.json", {"schema": 1, "disc": {"path": os.path.relpath(DISC, c)},
                                          "video": {"window": False}})
        bare = [binary, "--disc", DISC, "--script", script]
        rc1, _, err1 = run(*bare, "--log", out / "bare.log", "--record", out / "bare.json")
        rc2, _, err2 = run(binary, "--config", cfg, "--script", script, "--log", out / "config.log",
                           "--record", out / "config.json")
        check(rc1 == 0 and rc2 == 0, f"both runs exit 0 ({rc1}, {rc2})")
        same = (out / "bare.log").read_bytes() == (out / "config.log").read_bytes() and \
            (out / "bare.json").read_bytes() == (out / "config.json").read_bytes()
        check(same, "the same log and record")
        cards = [c / "card1.mcd", c / "card2.mcd"]
        check(all(p.exists() and p.stat().st_size == 0x20000 for p in cards),
              "card1.mcd and card2.mcd created beside the file (128 KB each)")
        check("watchdog 0 s" in err2, "the watchdog off under --config")
        modcfg = write(c / "mods.json", {"schema": 1, "disc": {"path": os.path.relpath(DISC, c)},
                                         "video": {"window": False}, "mods": {"fast_forward": {"enabled": True}}})
        rc3, _, err3 = run(binary, "--config", modcfg, "--script", script, "--log", out / "mods_off.log")
        rc4, _, err4 = run(binary, "--config", modcfg, "--script", script, "--script-mods", "--log", out / "mods_on.log")
        check(rc3 == 0 and "mods: fast_forward on" not in err3 and
              (out / "mods_off.log").read_bytes() == (out / "bare.log").read_bytes(),
              "under --script the settings' mods are off (and the log is the bare run's)")
        check(rc4 == 0 and "mods: fast_forward on" in err4, "--script-mods keeps them on")
    else:
        print("settings: no disc image: the run under --config is skipped")

    sdl = ROOT / "build/port-sdl/dw2003"
    if sdl.exists() and not args.no_sdl:
        print("settings: the window's input self-test (build/port-sdl, offscreen): the defaults, then rebound keys")
        env_sdl = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_AUDIO_DRIVER="dummy")
        for label, extra in (("defaults", []), ("rebound", ["--config", REBOUND])):
            proc = subprocess.run([str(sdl), "--input-test", "--fps", "0", *extra], cwd=ROOT, env=env_sdl,
                                  capture_output=True, text=True, timeout=120)
            lines = [l for l in proc.stderr.splitlines() if "input test:" in l]
            check(proc.returncode == 0 and any("the pause: passed" in l for l in lines),
                  f"{label}: exit {proc.returncode}: {'; '.join(l.split('input test: ')[1] for l in lines)}")
        if DISC.exists():
            fast_forward_check(sdl, env_sdl, out)
    else:
        print("settings: no build/port-sdl (or --no-sdl): the input self-test is skipped")

    print(f"settings test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
