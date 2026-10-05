#!/usr/bin/env python3
"""Function-level execution coverage of the emulator test runs (layer-1 oracle boot, layer-2 replay scripts).

Usage:
  tools/coverage.py run [--oracle] [--replay NAME ...] [--out DIR]   # record (default: the oracle and every replay script)
  tools/coverage.py report [--module NAME] [--report PATH]           # summarise the recorded runs

`run` boots the disc once per source under PCSX-Redux with -debugger -interpreter and tools/coverage.lua loaded before
the runner (tests/golden/oracle.lua or tests/replay/run.lua): one exec breakpoint per function start, deleted at its
first hit, re-armed per overlay slot for the file resident there (the slots' files share addresses; the resident file
is the one whose .text from the matching build equals RAM). Each source's hits go to build/coverage/<source>.json;
the oracle's goldens and the replays' records are compared with the committed files on the way (coverage must not
change them; a replay under the interpreter is not guaranteed to reproduce the dynarec's frames, so that is reported,
not required). `report` merges those files into build/coverage/report.md and build/coverage/coverage.json:
covered/total functions per module (EXE units, each tier-1 overlay and its C files, WFIGHTMN/WFIGHTTS, WSTAG aggregated)
per source and for the union, Psy-Q (sdk) functions apart, and which NON_MATCHING holdouts and FAKE matches ran.
`--module NAME` (a module, a C unit stem, or a name prefix ending in `_`) lists every function of it with its sources.

Function addresses and sizes come from the build's ELF symbol tables (every C function, static ones included, and the
splat labels of the asm units), and their C file from the link map; so the build must exist (scripts/build.sh).
Not part of scripts/test.sh.
"""
import argparse
import importlib.util
import json
import os
import re
import subprocess
import sys
import time
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
OUT = BUILD / "coverage"
READELF = ROOT / "tools/binutils/bin/mipsel-linux-gnu-readelf"
OBJDUMP = ROOT / "tools/binutils/bin/mipsel-linux-gnu-objdump"
COVERAGE_LUA = ROOT / "tools/coverage.lua"
ORACLE_PY = ROOT / "tests/golden/oracle.py"
REPLAY_PY = ROOT / "tests/replay/replay.py"
EMU_ARGS = ["-debugger", "-interpreter"]
SLOTS = {0x80082CB0: "t1", 0x800A5DE0: "t2"}
EXE = "SLES_039.36"

# Modules whose rules the port must reproduce exactly (pure logic, tables): the summary lists them first.
# (label, predicate on a function dict)
FOCUS = [
    ("gamestate", lambda f: f["unit"] == "src/main/gamestate.c"),
    ("records", lambda f: f["unit"] == "src/main/records.c"),
    ("card", lambda f: f["unit"] == "src/main/card.c"),
    ("memcard", lambda f: f["unit"] == "src/main/memcard.c"),
    ("pad_random", lambda f: f["name"].startswith("pad_random_")),
    ("fightstg_rules", lambda f: f["name"].startswith("fightstg_rules_")),
    ("cardgame_cpu", lambda f: f["unit"] == "src/cardgame/cardgame_cpu.c"),
    ("stfgtrep", lambda f: f["target"] == "stfgtrep"),
    ("stgtrain", lambda f: f["target"] == "stgtrain"),
    ("stgdglab", lambda f: f["target"] == "stgdglab"),
    ("ststatus", lambda f: f["target"] == "ststatus"),
]


# ---------------------------------------------------------------------------------------------------------------- inventory

def parse_map(path):
    """The .text input sections of a link map: [(start, size, object path)]."""
    out = []
    lines = path.read_text().splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        m = re.match(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)", line)
        if not m and line.strip() == ".text" and i + 1 < len(lines):
            m = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)", lines[i + 1])
        if m:
            start, size = int(m.group(1), 16), int(m.group(2), 16)
            if size:
                out.append((start, size, m.group(3)))
        i += 1
    return sorted(out)


def elf_funcs(elf):
    """{addr: (name, size)} of the FUNC symbols of an ELF (aliases: the plain name wins)."""
    out = {}
    text = subprocess.run([str(READELF), "-sW", str(elf)], check=True, capture_output=True, text=True).stdout
    for line in text.splitlines():
        p = line.split()
        if len(p) < 8 or p[3] != "FUNC" or p[6] in ("ABS", "UND"):
            continue
        addr, size, name = int(p[1], 16), int(p[2]), p[7]
        if size == 0:
            continue
        old = out.get(addr)
        if old is None or ("." in old[0] and "." not in name):
            out[addr] = (name, size)
    return out


def elf_base(elf):
    text = subprocess.run([str(READELF), "-SW", str(elf)], check=True, capture_output=True, text=True).stdout
    for line in text.splitlines():
        m = re.search(r"\]\s+\S+\s+PROGBITS\s+([0-9a-f]{8})", line)
        if m:
            return int(m.group(1), 16)
    raise RuntimeError(f"no PROGBITS section in {elf}")


def inventory():
    """Every function of every built target: a list of dicts {target, file, slot, unit, module, sub, psyq, name, addr,
    size}, plus {file: (base, text_lo, text_hi, path)} for the slot candidates."""
    funcs, files = [], {}
    for d in sorted(BUILD.iterdir()):
        elfs = sorted(d.glob("*.elf")) if d.is_dir() else []
        if not elfs or d.name in ("coverage", "host"):
            continue
        elf = elfs[0]
        fname = elf.name[:-len(".elf")]
        target = d.name
        objs = parse_map(elf.with_suffix(".map"))
        if not objs:
            continue
        base = elf_base(elf)
        slot = "exe" if fname == EXE else SLOTS.get(base)
        if slot is None:
            raise RuntimeError(f"{elf}: unknown load address {base:08x}")
        if slot != "exe":
            files[fname] = (base, objs[0][0], objs[-1][0] + objs[-1][1], str(BUILD / fname))
        starts = [o[0] for o in objs]
        for addr, (name, size) in sorted(elf_funcs(elf).items()):
            i = max(i for i, s in enumerate(starts) if s <= addr) if starts[0] <= addr else None
            if i is None or addr >= objs[i][0] + objs[i][1]:
                continue  # not in a .text input section (a data label typed as a function)
            obj = objs[i][2]
            rel = obj.split(f"build/{target}/", 1)[-1]
            unit = re.sub(r"\.o$", "", rel)
            psyq = "/psyq/" in unit
            if target.startswith("wstag"):
                module, sub = "wstag", target
            elif target == "main":
                module = "psyq" if psyq else "main"
                sub = (re.sub(r"^asm/main/psyq/", "", unit).split("/")[0] if psyq
                       else Path(unit).name.split(".")[0])
            else:
                module = target
                sub = "psyq" if psyq else Path(unit).name.split(".")[0]
            funcs.append({"target": target, "file": fname, "slot": slot, "unit": unit, "module": module, "sub": sub,
                          "psyq": psyq, "name": name, "addr": addr, "size": size})
    return funcs, files


def sync_points():
    """The instructions right after the memcpy in overlay_load_stage and overlay_load_file (the game's two overlay
    copies): residency is re-checked there before the new code can run."""
    elf = BUILD / "main" / f"{EXE}.elf"
    syms = {name: (addr, size) for addr, (name, size) in elf_funcs(elf).items()}
    out = []
    for fn in ("overlay_load_stage", "overlay_load_file"):
        addr, size = syms[fn]
        dis = subprocess.run([str(OBJDUMP), "-d", f"--start-address={addr:#x}", f"--stop-address={addr + size:#x}",
                              str(elf)], check=True, capture_output=True, text=True).stdout
        jals = [int(m.group(1), 16) for m in re.finditer(r"^([0-9a-f]{8}):\s+\S+\s+jal\s+\S+ <memcpy>", dis, re.M)]
        if len(jals) != 1:
            raise RuntimeError(f"{fn}: expected one jal memcpy, found {len(jals)}")
        out.append(jals[0] + 8)
    return out


def lua_literal(v):
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, int):
        return str(v)
    if isinstance(v, str):
        return json.dumps(v)
    if isinstance(v, (list, tuple)):
        return "{" + ",".join(lua_literal(x) for x in v) + "}"
    if isinstance(v, dict):
        return "{" + ",".join(f"{k}={lua_literal(x)}" for k, x in v.items()) + "}"
    raise TypeError(type(v))


def write_spec(path, funcs, files):
    """The Lua spec for tools/coverage.lua. Candidates with identical .text are merged into one key "A+B" (a hit is
    then credited to both: their code is the same bytes at the same addresses)."""
    by_file = defaultdict(list)
    for f in funcs:
        by_file[f["file"]].append(f["addr"])
    slots = {"t1": {}, "t2": {}}
    for fname, (base, lo, hi, fpath) in sorted(files.items()):
        if not by_file.get(fname):
            continue
        data = Path(fpath).read_bytes()[lo - base:hi - base]
        slot = slots[SLOTS[base]]
        key = (lo, hi, data)
        if key in slot:
            slot[key]["key"] += "+" + fname
            assert slot[key]["funcs"] == sorted(by_file[fname]), fname
        else:
            slot[key] = {"key": fname, "path": fpath, "base": base, "lo": lo, "hi": hi, "funcs": sorted(by_file[fname])}
    memcpy = next((f["addr"], f["size"]) for f in funcs if f["file"] == EXE and f["name"] == "memcpy")
    spec = {
        "exe": sorted(by_file[EXE]),
        "slots": [{"name": n, "cands": list(s.values())} for n, s in slots.items()],
        "sync": sync_points(),
        "copy": [memcpy[0], memcpy[0] + memcpy[1]],
    }
    path.write_text("return " + lua_literal(spec) + "\n")


# ---------------------------------------------------------------------------------------------------------------- runs

def load_module(name, path):
    """Imports a test driver by path under its own name (the golden families import `oracle`)."""
    if name in sys.modules:
        return sys.modules[name]
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


def parse_hits(path, funcs):
    """The recorder's output: (stats dict, hits [{file, addr, name, frame, stage, ctx}], residency changes)."""
    index = {(f["file"], f["addr"]): f for f in funcs}
    stats, hits, residency = {}, [], []
    if not path.exists():
        return stats, hits, residency
    for line in path.read_text().splitlines():
        p = line.split()
        if not p:
            continue
        if p[0] == "stats":
            stats = {k: int(v) for k, v in (x.split("=") for x in p[1:])}
        elif p[0] == "res":
            residency.append({"slot": p[1], "file": p[2], "frame": int(p[3]), "how": p[4]})
        elif p[0] == "hit":
            addr = int(p[2], 16)
            for fname in p[1].split("+"):
                f = index.get((fname, addr))
                hits.append({"file": fname, "addr": addr, "name": f["name"] if f else "?", "frame": int(p[3]),
                             "stage": int(p[4]), "ctx": " ".join(p[5:])})
    return stats, hits, residency


def run_source(source, out_dir, spec_path, runner_lua, call, funcs):
    """Runs one source with the recorder; returns its record."""
    out_dir.mkdir(parents=True, exist_ok=True)
    cov_out = out_dir / "coverage.txt"
    if cov_out.exists():
        cov_out.unlink()
    wrapper = out_dir / "coverage_wrapper.lua"
    wrapper.write_text(f"dofile({json.dumps(str(COVERAGE_LUA))})\ndofile({json.dumps(str(runner_lua))})\n")
    os.environ["DW3_COVERAGE_SPEC"] = str(spec_path)
    os.environ["DW3_COVERAGE_OUT"] = str(cov_out)
    t0 = time.time()
    status, note = "ok", ""
    try:
        note = call(wrapper)
    except Exception as e:  # the hits up to the failure are still worth keeping
        status, note = "fail", f"{type(e).__name__}: {e}"
    finally:
        del os.environ["DW3_COVERAGE_SPEC"], os.environ["DW3_COVERAGE_OUT"]
    wall = time.time() - t0
    stats, hits, residency = parse_hits(cov_out, funcs)
    rec = {"source": source, "status": status, "note": note, "wall_s": round(wall, 1), "stats": stats,
           "late_syncs": stats.get("late", 0), "residency": residency, "hits": hits}
    late = [r for r in residency if r["how"] == "vsync"]  # a slot changed outside the game's copies: hits may be missed
    print(f"  {source}: {status}, {len({(h['file'], h['addr']) for h in hits})} functions, {stats.get('frames', 0)} frames, {wall:.0f} s wall"
          + (f"; {len(late)} residency changes found only at a vsync" if late else "") + (f"; {note}" if note else ""))
    return rec


def cmd_run(args):
    for p in (READELF, OBJDUMP, BUILD / "main" / f"{EXE}.elf"):
        if not p.exists():
            print(f"coverage: missing {p} (scripts/build.sh)", file=sys.stderr)
            return 2
    funcs, files = inventory()
    OUT.mkdir(parents=True, exist_ok=True)
    work = Path(args.out) if args.out else OUT / "runs"
    work.mkdir(parents=True, exist_ok=True)
    spec_path = work / "spec.lua"
    write_spec(spec_path, funcs, files)
    print(f"coverage: {len(funcs)} functions in {len({f['file'] for f in funcs})} files; runs in {work}")
    sources = []
    do_all = not args.oracle and args.replay is None
    if args.oracle or do_all:
        sources.append("oracle")
    replay = load_module("replay", REPLAY_PY)
    scripts = sorted(replay.SCRIPTS.glob("*.json"))
    if args.replay is not None or do_all:
        script_paths = {}
        for n in args.replay or [s.stem for s in scripts]:
            path = Path(n) if n.endswith(".json") else replay.SCRIPTS / f"{n}.json"
            if not path.exists():
                print(f"coverage: no replay script {n}", file=sys.stderr)
                return 2
            sources.append(f"replay_{path.stem}")
            script_paths[f"replay_{path.stem}"] = path.resolve()
    status = 0
    for source in sources:
        if source == "oracle":
            oracle = load_module("oracle", ORACLE_PY)

            def call(wrapper, oracle=oracle, out=work / "oracle"):
                oracle.ORACLE_LUA = wrapper
                families = oracle.all_families()
                goldens = oracle.generate(families, out)
                bad = [f for f, g in goldens.items()
                       if oracle.diff_goldens(oracle.comparable(json.loads((oracle.GOLDEN_DIR / f"{f}.json").read_text())),
                                              oracle.comparable(g))]
                if bad:
                    raise RuntimeError("goldens differ under coverage: " + ", ".join(bad))
                return f"{len(families)} families, every golden reproduced"
            rec = run_source(source, work / "oracle", spec_path, oracle.ORACLE_LUA, call, funcs)
        else:
            path = script_paths[source]

            def call(wrapper, path=path, out=work / source):
                script = replay.load_script(path)
                expected_path = replay.EXPECTED / f"{script['name']}.json"
                bios = replay.OPENBIOS
                if expected_path.exists():
                    bios = replay.bios_path(json.loads(expected_path.read_text())["bios"]["name"])
                record = replay.run_once(path, script, bios, out, lua=wrapper, emu_args=EMU_ARGS)
                if not expected_path.exists():
                    return "no expected file"
                diffs = replay.compare(json.loads(expected_path.read_text()), record)
                return ("reproduces its expected file under the interpreter" if not diffs
                        else f"differs from its expected file under the interpreter ({len(diffs)} fields; "
                             f"first: {diffs[0][:120]})")
            rec = run_source(source, work / source, spec_path, replay.RUN_LUA, call, funcs)
        if rec["status"] != "ok":
            status = 1
        (OUT / f"{source}.json").write_text(json.dumps(rec, indent=0) + "\n")
    report(funcs, None, OUT / "report.md")
    return status


# ---------------------------------------------------------------------------------------------------------------- report

def stage_names():
    """{stage: overlay name} from overlay_files' comments in src/main/overlay.c."""
    text = (ROOT / "src/main/overlay.c").read_text()
    return {int(m.group(1)): m.group(2) for m in re.finditer(r"0x[0-9A-Fa-f]+,\s*/\*\s*(\d+) (\w+)\.PRO", text)}


def holdouts_and_fakes():
    """[(kind, target, function name, where)] for the NON_MATCHING holdouts and the FAKE matches in src/."""
    out = []
    for c in sorted((ROOT / "src").rglob("*.c")):
        lines = c.read_text(errors="replace").splitlines()
        target = c.parent.name if c.parent.name != "wstag" else c.stem.split("_")[0]
        rel = c.relative_to(ROOT)
        for i, line in enumerate(lines):
            if line.startswith("#ifdef NON_MATCHING"):
                for j in range(i + 1, len(lines)):
                    m = re.match(r'INCLUDE_ASM\("[^"]*",\s*(\w+)\)', lines[j])
                    if m:
                        out.append(("holdout", target, m.group(1), f"{rel}:{i + 1}"))
                        break
            if "FAKE:" in line:
                for j in range(i, -1, -1):
                    m = re.match(r"^(?:static\s+)?[A-Za-z_][\w\s\*]*?\b(\w+)\s*\([^;]*$", lines[j])
                    if m and not lines[j].startswith(("#", " ", "\t")):
                        out.append(("FAKE", target, m.group(1), f"{rel}:{i + 1}"))
                        break
    return out


def load_runs():
    runs = {}
    for p in sorted(OUT.glob("*.json")):
        if p.name == "coverage.json":
            continue
        rec = json.loads(p.read_text())
        runs[rec["source"]] = rec
        if rec["source"] == "oracle":
            # The recorder starts over at the oracle's first job: hits with a job's context are what the golden calls
            # ran, the rest is the boot to CNTY_SEL.
            calls = [h for h in rec["hits"] if h["ctx"] != "-"]
            runs["oracle_calls"] = dict(rec, source="oracle_calls", hits=calls,
                                        note="the oracle's jobs only (the boot to CNTY_SEL excluded)")
    return runs


def module_rows(funcs, covered):
    """[(module label, [functions])] in report order: EXE units, Psy-Q, each tier-1 overlay and its files, tier 2, WSTAG."""
    groups = defaultdict(list)
    for f in funcs:
        groups[(f["module"], f["sub"])].append(f)
    rows = []
    modules = sorted({f["module"] for f in funcs}, key=lambda m: (m != "main", m == "psyq", m.startswith("w"), m))
    for m in modules:
        subs = sorted(s for (mm, s) in groups if mm == m)
        allf = [f for s in subs for f in groups[(m, s)]]
        if m == "main":
            for s in subs:
                rows.append((f"main/{s}", groups[(m, s)]))
        else:
            rows.append((m, [f for f in allf if not f["psyq"]] if m != "psyq" else allf))
            if m not in ("psyq", "wstag") and len(subs) > 1:
                for s in subs:
                    rows.append((f"  {m}/{s}", groups[(m, s)]))
    return rows


def report(funcs, module, path):
    runs = load_runs()
    if not runs:
        print("coverage: no runs in build/coverage (tools/coverage.py run)", file=sys.stderr)
        return 2
    sources = sorted(runs, key=lambda s: (not s.startswith("oracle"), s))
    key = lambda f: (f["file"], f["addr"])
    hit_by = defaultdict(dict)   # (file, addr) -> {source: first hit}
    for s in sources:
        for h in runs[s]["hits"]:
            hit_by[(h["file"], h["addr"])].setdefault(s, h)
    game = [f for f in funcs if not f["psyq"]]
    cols = sources + ["union"]

    def cov(fs, s):
        n = sum(1 for f in fs if (s == "union" and hit_by.get(key(f))) or s in hit_by.get(key(f), {}))
        return n

    def cell(fs, s):
        n, t = cov(fs, s), len(fs)
        return f"{n}/{t}" if t else "-"

    md = ["# Function coverage of the emulator test runs", "",
          "Made by `tools/coverage.py` (function starts executed at least once; see its docstring). Sources:", ""]
    for s in sources:
        r = runs[s]
        md.append(f"- `{s}`: {r['status']}, {len({(h['file'], h['addr']) for h in r['hits']})} functions, "
                  f"{r['stats'].get('frames', 0)} frames, "
                  f"{r['wall_s']} s wall; {r['note']}")
    md += ["", "Psy-Q (sdk) functions are counted apart (`psyq` rows) and excluded from the totals.", ""]
    total_row = ["**game total (no Psy-Q)**"] + [cell(game, s) for s in cols]
    md += ["| module | " + " | ".join(cols) + " |", "|---" * (len(cols) + 1) + "|", "| " + " | ".join(total_row) + " |"]
    md.append("| " + " | ".join(["**focus: pure logic**"] + [""] * len(cols)) + " |")
    focus_rows = []
    for label, pred in FOCUS:
        fs = [f for f in game if pred(f)]
        focus_rows.append((label, fs))
        md.append("| " + " | ".join([label] + [cell(fs, s) for s in cols]) + " |")
    md.append("| " + " | ".join(["**all modules**"] + [""] * len(cols)) + " |")
    rows = module_rows(funcs, hit_by)
    for label, fs in rows:
        md.append("| " + " | ".join([label.replace("  ", "&nbsp;&nbsp;")] + [cell(fs, s) for s in cols]) + " |")

    md += ["", "## Uncovered functions of the focus modules", ""]
    for label, fs in focus_rows:
        un = [f["name"] for f in fs if not hit_by.get(key(f))]
        md.append(f"- **{label}** ({len(un)} of {len(fs)} not run): " + (", ".join(f"`{n}`" for n in un) or "none"))

    md += ["", "## NON_MATCHING holdouts and FAKE matches", "", "| kind | function | where | run by |", "|---|---|---|---|"]
    byname = defaultdict(list)
    for f in funcs:
        byname[(f["target"], f["name"])].append(f)
    hf = []
    for kind, target, name, where in holdouts_and_fakes():
        fs = byname.get((target, name), [])
        srcs = [s for s in sources if any(s in hit_by.get(key(f), {}) for f in fs)]
        hf.append({"kind": kind, "target": target, "name": name, "where": where, "found": bool(fs), "run_by": srcs})
        md.append(f"| {kind} | `{name}` | {where} | {', '.join(srcs) if srcs else ('not run' if fs else 'NOT FOUND')} |")

    if module:
        stages = stage_names()
        md += ["", f"## Functions of `{module}`", "", "| function | addr | file | run by (first hit: frame, context) |",
               "|---|---|---|---|"]
        sel = [f for f in funcs if module in (f["module"], f["sub"], f["target"], Path(f["unit"]).stem)
               or (module.endswith("_") and f["name"].startswith(module))]
        for f in sorted(sel, key=lambda f: (f["file"], f["addr"])):
            hb = hit_by.get(key(f), {})
            by = "; ".join(f"{s} (frame {h['frame']}, "
                           f"{h['ctx'] if h['ctx'] != '-' else stages.get(h['stage'], 'stage ' + str(h['stage']))})"
                           for s, h in hb.items()) or "-"
            md.append(f"| `{f['name']}` | {f['addr']:08X} | {f['file']} | {by} |")
        if not sel:
            md.append(f"(no function matches `{module}`)")

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(md) + "\n")
    summary = {
        "sources": {s: {k: runs[s][k] for k in ("status", "note", "wall_s", "stats")} for s in sources},
        "union_of": sources,
        "game_total": {s: [cov(game, s), len(game)] for s in cols},
        "psyq_total": {s: [cov([f for f in funcs if f["psyq"]], s), sum(1 for f in funcs if f["psyq"])] for s in cols},
        "focus": {label: {s: [cov(fs, s), len(fs)] for s in cols} for label, fs in focus_rows},
        "modules": {label.strip(): {s: [cov(fs, s), len(fs)] for s in cols} for label, fs in rows},
        "holdouts_fakes": hf,
        "functions": [{"file": f["file"], "name": f["name"], "addr": f"{f['addr']:08X}", "unit": f["unit"],
                       "psyq": f["psyq"], "run_by": [s for s in sources if s in hit_by.get(key(f), {})]} for f in funcs],
    }
    (OUT / "coverage.json").write_text(json.dumps(summary, indent=0) + "\n")
    # Short summary on stdout.
    print("coverage: " + ", ".join(f"{s} {cell(game, s)}" for s in cols) + " game functions (Psy-Q apart)")
    for label, fs in focus_rows:
        print(f"  {label:15s} " + "  ".join(f"{s} {cell(fs, s)}" for s in cols))
    ran = [h for h in hf if h["run_by"]]
    print(f"  holdouts/FAKE run: {len(ran)} of {len(hf)}: " + ", ".join(f"{h['name']} ({'+'.join(h['run_by'])})" for h in ran))
    print(f"  report: {path.relative_to(ROOT) if path.is_relative_to(ROOT) else path}; data: build/coverage/coverage.json")
    return 0


def cmd_report(args):
    funcs, _ = inventory()
    return report(funcs, args.module, Path(args.report) if args.report else OUT / "report.md")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run", help="record coverage (default: the oracle and every replay script)")
    r.add_argument("--oracle", action="store_true", help="the layer-1 oracle boot (all families)")
    r.add_argument("--replay", nargs="*", help="replay scripts by name or .json path (no name: every committed script)")
    r.add_argument("--out", help="emulator run files (default build/coverage/runs)")
    r.set_defaults(func=cmd_run)
    p = sub.add_parser("report", help="summarise build/coverage/*.json")
    p.add_argument("--module", help="list this module's functions (module, unit stem, or name prefix ending in _)")
    p.add_argument("--report", help="markdown path (default build/coverage/report.md)")
    p.set_defaults(func=cmd_report)
    args = ap.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
