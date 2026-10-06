#!/usr/bin/env python3
"""The port's settings file (`--config FILE`, docs/LAUNCHER_MODS_PLAN.md 4.3): the contract with the launcher.

Usage: tests/port/settings.py [--out DIR] [-j N]

Builds the headless port if needed (tests/port/run.py's build), then checks, with `--print-settings`:
  - the round trip: a file's effective settings, printed and loaded again from another directory, print the same;
  - the defaults (every key absent), relative paths resolved against the file's directory, null memory cards;
  - the command line overriding the file (--disc, --scale, --memcard1 none, --watchdog);
  - the errors: exit 64 with the key named (no schema, a newer schema, a wrong type, out of range, not JSON, a missing
    file, --print-settings without --config); an unknown key is logged and ignored;
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


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", help="scratch directory (default build/port-test/settings)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None)
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
    check(s["input"] == full["input"] and s["mods"] == full["mods"] and s["launcher"] == full["launcher"],
          "input, mods and launcher printed back as they are")

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
    else:
        print("settings: no disc image: the run under --config is skipped")

    print(f"settings test: {'FAIL (' + str(len(FAILURES)) + ')' if FAILURES else 'pass'}")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    sys.exit(main())
