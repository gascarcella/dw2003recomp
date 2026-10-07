#!/usr/bin/env python3
"""Save round trips between the PC port and the emulator (tests/README.md layer 3; docs/PORT.md "Disc, memory cards and movies").

Usage: tests/saves/run.py [--out DIR] [--keep] [--card-port MCD] [--card-emulator MCD] [-j N]

Two cuts of layer 2's tests/replay/scripts/first_battle_save.json, made at run time (no script of their own, no
expected file of their own):
  save  its steps up to the `saved` checkpoint and back to the field (boot, registration, the first battle, the walk to
        the Asuka Inn, STGMCARD's save to card 1 slot 1)
  load  its steps from after the `reset` to the `loaded` checkpoint (boot, title, Continue, STGMCARD's load, the inn)
1. Both sides save: the port (build/port/dw2003 --memcard1 FILE, a new card) and the emulator (PCSX-Redux, a new card,
   OpenBIOS) each run `save`; each `saved` checkpoint must have first_battle_save's expected stable hash.
2. Each side loads the other's card (a copy): port -> emulator and emulator -> port; each `loaded` checkpoint must have
   first_battle_save's expected stable hash (tests/replay/expected/first_battle_save.json).
3. The two cards' formats (tests/saves/cards.py): the directory, the save file's Sony header and icons, the game's
   header and slot, byte for byte except the fields named in cards.py (play time, the checksums that cover it, the
   stale buffer tails, directory frame 63), and every checksum valid (directory frames, the game's two XOR sums).
A third card, from the global_save mod (docs/LAUNCHER.md "Save anywhere"; tests/port/mods/scripts/global_save.json cut
at its `saved_anywhere` checkpoint and back to the field): the port saves from the lab (map 0x206, not an inn's map)
with the mod on, and the emulator loads that card with the same `load` cut: its `loaded` checkpoint must be on map
0x206 with the slot's bytes (4..0x26C4, the volatile ranges aside) equal to the port's at `saved_anywhere`, and the
card must pass the single-card format checks (the mod's record sits in the slot's unused tail, which the game never
reads).
--card-port / --card-emulator skip that side's save run and use the given card (a corrupted copy, to see the checks
fail). Outputs go to build/saves-test/ (--out): each run's log/record, the cards, the checkpoint dumps; --keep keeps
them on a pass (they are removed otherwise, but for the cards). Exit 0 pass, 1 fail, 2 something missing.
The two save runs (the emulator's ~27,000 frames are most of the ~1 min) run in parallel, then the two loads.
"""
import argparse
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests/replay"))
import replay  # noqa: E402  (the layer-2 runner's helpers: the Lua step table, VOLATILE_RANGES, the emulator's paths)
sys.path.insert(0, str(ROOT / "tests/saves"))
import cards  # noqa: E402  (the card format checks)

# tests/port/run.py builds the port; loaded by path (this file is a run.py too).
_spec = importlib.util.spec_from_file_location("port_test", ROOT / "tests/port/run.py")
port_test = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(port_test)

SOURCE = ROOT / "tests/replay/scripts/first_battle_save.json"
ANYWHERE = ROOT / "tests/port/mods/scripts/global_save.json"
ANYWHERE_END = "saved_anywhere"
EXPECTED = ROOT / "tests/replay/expected/first_battle_save.json"
DISC = ROOT / "iso/dw2003.cue"
# The cuts, by checkpoint and step type, so that a step added elsewhere in the source script does not move them.
SAVE_END = "saved"       # the save cut runs to this checkpoint, then to the next wait_stage (back on the field)
LOAD_FROM = "reset"      # the load cut starts after this step (a fresh boot instead of the reset)
LOAD_END = "loaded"
RUN_TIMEOUT = 600


def cut_scripts(source):
    """The save and load cuts of first_battle_save (the step dicts unchanged)."""
    steps = source["steps"]
    names = [s.get("name") if s["type"] == "checkpoint" else None for s in steps]
    saved = names.index(SAVE_END)
    # after `saved`: the window, CROSS, then TRIANGLE until FIELDSTG (stage 2) is back; the card is complete by then
    save_end = next(i for i in range(saved + 1, len(steps))
                    if steps[i].get("until", {}).get("type") == "wait_stage" or steps[i]["type"] == "wait_stage")
    reset = next(i for i, s in enumerate(steps) if s["type"] == LOAD_FROM)
    loaded = names.index(LOAD_END)
    if not saved < save_end < reset < loaded:
        raise RuntimeError(f"{SOURCE.relative_to(ROOT)}: the steps are not save, back to the field, reset, load")
    common = {"default_timeout": source.get("default_timeout", 3000)}
    save = dict(name="saves_save", max_frames=40000, steps=steps[:save_end + 1], **common)
    load = dict(name="saves_load", max_frames=10000, steps=steps[reset + 1:loaded + 1], **common)
    return save, load


def cut_anywhere(script):
    """global_save.json's steps to `saved_anywhere` and back to the field (the next wait_stage), as the save cut."""
    steps = script["steps"]
    saved = next(i for i, s in enumerate(steps) if s["type"] == "checkpoint" and s.get("name") == ANYWHERE_END)
    end = next(i for i in range(saved + 1, len(steps))
               if steps[i].get("until", {}).get("type") == "wait_stage" or steps[i]["type"] == "wait_stage")
    return dict(name="saves_anywhere", max_frames=40000, steps=steps[:end + 1],
                default_timeout=script.get("default_timeout", 3000))


def slot_bytes(dump):
    """A gamestate dump's slot (bytes 4..0x26C4) with the volatile ranges zeroed."""
    stable = bytearray(dump)
    for lo, hi in replay.VOLATILE_RANGES:
        stable[lo:hi] = bytes(hi - lo)
    return bytes(stable[4:0x26C4])


def stable_sha1(data):
    stable = bytearray(data)
    for lo, hi in replay.VOLATILE_RANGES:
        stable[lo:hi] = bytes(hi - lo)
    return hashlib.sha1(stable).hexdigest()


def run_emulator(script, card, out_dir):
    """The script in PCSX-Redux with `card` in slot 1 (created by the emulator if missing) and a new card in slot 2;
    returns {checkpoint: stable hash} and the frame count. replay.run_once always starts from new cards, so this is
    its command with the card given."""
    out_dir.mkdir(parents=True, exist_ok=True)
    lua_script = out_dir / "script.lua"
    lua_script.write_text("return " + replay.lua_literal(replay.parse_ints(script)) + "\n")
    env = dict(os.environ, DW3_REPLAY_SCRIPT=str(lua_script), DW3_REPLAY_OUT=str(out_dir), DW3_REPLAY_SPEED="0")
    card2 = out_dir / "memcard2.mcd"
    card2.unlink(missing_ok=True)
    cmd = [str(replay.REDUX), "-no-ui", "-stdout", "-testmode", "-run", "-iso", str(DISC), "-bios", str(replay.OPENBIOS),
           "-memcard1", str(card.resolve()), "-memcard2", str(card2), "-dofile", str(replay.RUN_LUA)]
    log = out_dir / "emulator.log"
    with open(log, "w") as f:
        proc = subprocess.run(cmd, env=env, stdout=f, stderr=subprocess.STDOUT, timeout=script["max_frames"] / 25 + 120)
    result_path = out_dir / "result.json"
    if not result_path.exists():
        tail = "\n    ".join(log.read_text(errors="replace").splitlines()[-10:])
        raise RuntimeError(f"emulator exited {proc.returncode} without result.json ({log}):\n    {tail}")
    result = json.loads(result_path.read_text())
    if result.get("status") != "ok" or proc.returncode != 0:
        raise RuntimeError(f"emulator: exit {proc.returncode}: {result.get('message')} ({log})")
    hashes = {cp["name"]: stable_sha1((out_dir / cp["gamestate_file"]).read_bytes()) for cp in result["checkpoints"]}
    for cp in result["checkpoints"]:  # the dumps by name, for the slot comparison
        shutil.copyfile(out_dir / cp["gamestate_file"], out_dir / f"gamestate_{cp['name']}.bin")
    return hashes, result["frames"]


def run_port(binary, script, card, out_dir, mods=None):
    """The script in the port with `card` in slot 1 (created if missing); returns {checkpoint: stable hash} and the
    frame count. `mods`: a settings `mods` object to run with (and --script-mods); None: the bare binary."""
    out_dir.mkdir(parents=True, exist_ok=True)
    script_path = out_dir / "script.json"
    script_path.write_text(json.dumps(script, indent=1) + "\n")
    record, log, err = out_dir / "record.json", out_dir / "port.log", out_dir / "port.stderr"
    cmd = [str(binary), "--disc", str(DISC), "--script", str(script_path), "--memcard1", str(card.resolve()),
           "--log", str(log), "--record", str(record)]
    if mods is not None:
        cfg = out_dir / "settings.json"
        cfg.write_text(json.dumps({"schema": 1, "disc": {"path": str(DISC)}, "video": {"window": False}, "mods": mods}))
        cmd += ["--config", str(cfg), "--script-mods"]
    dumps = out_dir / "checkpoints"  # the images the checkpoints hash, named as run.lua names its dumps
    dumps.mkdir(exist_ok=True)
    env = dict(os.environ, DW3_PORT_CHECKPOINT_DIR=str(dumps))
    with open(err, "w") as f:
        proc = subprocess.run(cmd, cwd=ROOT, env=env, stdout=f, stderr=subprocess.STDOUT, timeout=RUN_TIMEOUT)
    if not record.exists() or record.stat().st_size == 0:
        tail = "\n    ".join(err.read_text(errors="replace").splitlines()[-10:])
        raise RuntimeError(f"port: exit {proc.returncode} without a record ({err}):\n    {tail}")
    rec = json.loads(record.read_text())
    if proc.returncode != 0 or rec.get("status") != 0:
        raise RuntimeError(f"port: exit {proc.returncode}, status {rec.get('status')}: {rec.get('reason')} ({err})")
    for f in dumps.glob("cp*_*.bin"):  # the dumps by name, as run_emulator's
        shutil.copyfile(f, out_dir / f"gamestate_{f.name.split('_', 1)[1]}")
    return {cp["name"]: cp["gamestate_sha1_stable"] for cp in rec["checkpoints"]}, rec["frames"]


def check_hash(failures, who, result, name, expected):
    hashes, frames = result
    got = hashes.get(name)
    if got == expected[name]:
        print(f"  {who:<31} {name:<7} {got[:12]} ok ({frames} frames)")
    else:
        print(f"  {who:<31} {name:<7} {(got or 'missing')[:12]} DIFFERS (expected {expected[name][:12]}; {frames} frames)")
        failures.append(f"{who}: checkpoint {name}: stable hash {got}, expected {expected[name]} "
                        f"({EXPECTED.relative_to(ROOT)})")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", help="output directory (default build/saves-test/)")
    ap.add_argument("--keep", action="store_true", help="keep the run directories on a pass")
    ap.add_argument("--card-port", help="use this card as the port's save (skips the port's save run)")
    ap.add_argument("--card-emulator", help="use this card as the emulator's save (skips the emulator's save run)")
    ap.add_argument("-j", "--jobs", type=int, default=int(os.environ.get("DW3_JOBS", "0")) or None,
                    help="build jobs (default: $DW3_JOBS, else Ninja's)")
    args = ap.parse_args()

    missing = [str(p.relative_to(ROOT)) for p in (DISC, SOURCE, EXPECTED, ANYWHERE, replay.RUN_LUA) if not p.exists()]
    missing += [] if replay.REDUX.exists() else ["tools/redux/pcsx-redux (scripts/setup.sh redux)"]
    try:
        env = port_test.tool_env()
    except port_test.Missing as e:
        missing.append(str(e))
    if missing:
        print("saves: missing " + ", ".join(missing), file=sys.stderr)
        return 2
    source = json.loads(SOURCE.read_text())
    expected_rec = json.loads(EXPECTED.read_text())
    if expected_rec.get("script_sha1") != replay.sha1_file(SOURCE):
        print(f"saves: {SOURCE.relative_to(ROOT)} changed since {EXPECTED.relative_to(ROOT)} was recorded",
              file=sys.stderr)
        return 1
    expected = {cp["name"]: cp["gamestate_sha1_stable"] for cp in expected_rec["checkpoints"]}
    try:
        save, load = cut_scripts(source)
        anywhere = cut_anywhere(json.loads(ANYWHERE.read_text()))
    except (RuntimeError, ValueError, StopIteration) as e:
        print(f"saves: FAIL: cannot cut {SOURCE.relative_to(ROOT)} or {ANYWHERE.relative_to(ROOT)}: "
              f"{e or 'a step is missing'}")
        return 1
    given = {"port": Path(args.card_port).read_bytes() if args.card_port else None,
             "emulator": Path(args.card_emulator).read_bytes() if args.card_emulator else None}  # before out is cleared
    out = (Path(args.out) if args.out else ROOT / "build/saves-test").resolve()
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    try:
        binary = port_test.build("build/port", [], args.jobs, env)
    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(f"saves: FAIL: {e}")
        return 1
    print(f"saves: cuts of {SOURCE.relative_to(ROOT)}: save {len(save['steps'])} steps (to `{SAVE_END}` and back "
          f"to the field), load {len(load['steps'])} steps (after `{LOAD_FROM}`, to `{LOAD_END}`); of "
          f"{ANYWHERE.relative_to(ROOT)}: {len(anywhere['steps'])} steps (to `{ANYWHERE_END}` and back to the field)")

    failures = []
    t0 = time.time()
    port_card, emu_card, any_card = out / "port.mcd", out / "emulator.mcd", out / "anywhere.mcd"
    jobs = {}
    with ThreadPoolExecutor(max_workers=3) as pool:
        any_job = pool.submit(run_port, binary, anywhere, any_card, out / "anywhere_save",
                              {"global_save": {"enabled": True}})
        if given["port"] is not None:
            port_card.write_bytes(given["port"])
        else:
            jobs["port saves"] = pool.submit(run_port, binary, save, port_card, out / "port_save")
        if given["emulator"] is not None:
            emu_card.write_bytes(given["emulator"])
        else:
            jobs["emulator saves"] = pool.submit(run_emulator, save, emu_card, out / "emulator_save")
        for who, job in jobs.items():
            try:
                check_hash(failures, who, job.result(), SAVE_END, expected)
            except (RuntimeError, subprocess.TimeoutExpired) as e:
                failures.append(f"{who}: {e}")
        try:
            any_hashes, any_frames = any_job.result()
            any_slot = slot_bytes((out / "anywhere_save" / f"gamestate_{ANYWHERE_END}.bin").read_bytes())
            print(f"  {'port saves off an inn (mod)':<31} {ANYWHERE_END:<7} {any_hashes[ANYWHERE_END][:12]} "
                  f"({any_frames} frames)")
        except (RuntimeError, subprocess.TimeoutExpired, OSError, KeyError) as e:
            failures.append(f"port saves off an inn (global_save): {e!r}")
        if failures:
            return report(failures, out, args.keep, t0)
        # Each side loads a copy of the other's card: the emulator's BIOS writes frame 63 even on a load (cards.py
        # masks()), and the format checks compare the cards as the saves left them.
        shutil.copyfile(port_card, out / "port_for_emulator.mcd")
        shutil.copyfile(emu_card, out / "emulator_for_port.mcd")
        jobs = {"emulator loads the port's card": pool.submit(run_emulator, load, out / "port_for_emulator.mcd",
                                                              out / "emulator_load"),
                "port loads the emulator's card": pool.submit(run_port, binary, load, out / "emulator_for_port.mcd",
                                                              out / "port_load")}
        shutil.copyfile(any_card, out / "anywhere_for_emulator.mcd")
        any_load = pool.submit(run_emulator, load, out / "anywhere_for_emulator.mcd", out / "anywhere_load")
        for who, job in jobs.items():
            try:
                check_hash(failures, who, job.result(), LOAD_END, expected)
            except (RuntimeError, subprocess.TimeoutExpired) as e:
                failures.append(f"{who}: {e}")
        who = "emulator loads the mod's card"
        try:
            _, frames = any_load.result()
            dump = (out / "anywhere_load" / f"gamestate_{LOAD_END}.bin").read_bytes()
            loaded_map = int.from_bytes(dump[0x26C4:0x26C8], "little")
            if loaded_map == 0x206 and slot_bytes(dump) == any_slot:
                print(f"  {who:<31} {LOAD_END:<7} map 0x206, the slot as saved ok ({frames} frames)")
            else:
                failures.append(f"{who}: checkpoint {LOAD_END}: map {loaded_map:#x} (expected 0x206), the slot "
                                f"{'as saved' if slot_bytes(dump) == any_slot else 'DIFFERS from the save'}")
        except (RuntimeError, subprocess.TimeoutExpired, OSError) as e:
            failures.append(f"{who}: {e!r}")

    print("  card format (tests/saves/cards.py): anywhere.mcd")
    failures += cards.check_card(any_card.read_bytes(), "anywhere.mcd")[0]
    print("  card formats (tests/saves/cards.py): port.mcd vs emulator.mcd")
    failures += cards.check_pair(port_card.read_bytes(), emu_card.read_bytes(), "port.mcd", "emulator.mcd",
                                 indent="    ")
    # Both sides are deterministic, so two runs print the same SHA-1s (the cards are not committed: game data).
    print(f"    SHA-1: port.mcd {replay.sha1_file(port_card)}, emulator.mcd {replay.sha1_file(emu_card)}")
    return report(failures, out, args.keep, t0)


def report(failures, out, keep, t0):
    elapsed = time.time() - t0
    if failures:
        print("saves: FAIL\n  " + "\n  ".join(failures) + f"\n  outputs: {out}")
        return 1
    if not keep:
        for d in out.iterdir():
            if d.is_dir():
                shutil.rmtree(d)
    print(f"saves: pass (port -> emulator, emulator -> port, the mod's card -> emulator, card formats; {elapsed:.0f} s; "
          f"cards in {out})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
