#!/usr/bin/env python3
"""Layer-1 goldens: call the game's own functions in the emulator (tests/golden/oracle.lua) and record the results.

Usage:
  tests/golden/oracle.py gen [family ...]      # (re)generate tests/golden/<family>.json (default: every family)
  tests/golden/oracle.py check [family ...]    # regenerate into memory and compare with the committed files
  tests/golden/oracle.py list

A family is a module in tests/golden/families/ that returns Case objects: a fixture (bytes written into the game's
globals, by symbol and offset), scratch buffers (placed in borrowed RAM), and calls (function, arguments, return type,
memory to read back). Every case runs as one oracle job: the overwritten bytes are restored afterwards, so cases are
independent. All families run in one emulator boot (~20 s).

Golden files are JSON: {"family", "comment", "oracle": {method, emulator, bios, tree_commit, resident}, "fixtures": {name: [...]},
"cases": [...]} (a case names its fixture in the shared table, since most cases of a family share one);
each case has its fixture, buffers and calls with "ret" (decoded by ret_type), the raw "v0" and the reads (hex, or a
SHA-1 for large regions). Exit codes: 0 ok, 1 mismatch or emulator failure, 2 usage / missing tool.
"""
import argparse
import hashlib
import importlib
import json
import os
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REDUX = ROOT / "tools/redux/pcsx-redux"
REDUX_VERSION = ROOT / "tools/redux/app/usr/share/pcsx-redux/resources/version.json"
OPENBIOS = ROOT / "tools/redux/app/usr/share/pcsx-redux/resources/openbios.bin"


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
ISO = ROOT / "iso/dw2003.cue"
ORACLE_LUA = ROOT / "tests/golden/oracle.lua"
GOLDEN_DIR = ROOT / "tests/golden"
FAMILIES_DIR = GOLDEN_DIR / "families"
SYMBOLS = ROOT / "config/symbol_addrs.txt"
# Scratch RAM for buffers: heap area far above the overlay slot; the oracle restores every byte it overwrites, and the
# game is stopped in its main loop while a job runs.
SCRATCH_BASE = 0x80180000
SCRATCH_SIZE = 0x8000
LARGE_READ = 256  # reads of more than this many bytes are recorded as a SHA-1
JOBS_PER_CHUNK = 100  # jobs per Lua chunk file (a chunk holds at most 65,536 constants)


@dataclass
class Write:
    symbol: str               # symbol name, or a raw address (int or "0x...")
    offset: int = 0
    data: bytes = b""
    field: str = ""
    file: str = None          # instead of data: a file under the repo root, copied whole (an overlay, a data file)


@dataclass
class Read:
    symbol: str               # symbol name, a raw address, or "buf:<name>": one of the case's scratch buffers
    offset: int
    size: int
    field: str = ""


@dataclass
class Call:
    func: str                 # symbol name
    args: list = field(default_factory=list)   # ints, or ("buf", name), or ("sym", name, offset)
    ret_type: str = "s32"     # s32, u32, u16, s16, u8, s8, void
    reads: list = field(default_factory=list)
    comment: str = ""
    writes: list = field(default_factory=list)  # Write: applied just before this call (restored with the case)


@dataclass
class Case:
    name: str
    calls: list
    fixture: list = field(default_factory=list)    # Write
    buffers: dict = field(default_factory=dict)    # name -> bytes
    saves: list = field(default_factory=list)      # (symbol, size): regions restored after the case
    comment: str = ""
    keep: bool = False        # a setup case: its writes stay (an overlay in the slot, a data file), nothing is restored


class Symbols:
    """config/symbol_addrs.txt (the EXE) plus every config/<overlay>.symbols.txt (the overlays share the slot, so a name
    defined by two overlays at different addresses is ambiguous and refused when looked up)."""

    def __init__(self, path=SYMBOLS):
        self.addr, self.size, self.ambiguous = {}, {}, set()
        for file in [path] + sorted(path.parent.glob("*.symbols.txt")):
            for line in file.read_text().splitlines():
                m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);(.*)", line)
                if m:
                    name, addr = m.group(1), int(m.group(2), 16)
                    if name in self.addr and self.addr[name] != addr:
                        self.ambiguous.add(name)
                    self.addr.setdefault(name, addr)
                    s = re.search(r"size:0x([0-9A-Fa-f]+)", m.group(3))
                    if s:
                        self.size.setdefault(name, int(s.group(1), 16))

    def __getitem__(self, name):
        """A symbol's address; a raw address (int, or "0x..." string) passes through."""
        if isinstance(name, int):
            return name
        if name.startswith("0x"):
            return int(name, 16)
        if name in self.ambiguous:
            raise KeyError(f"{name} is defined at different addresses by several overlays")
        return self.addr[name]


def decode_ret(v0, ret_type):
    if ret_type == "void":
        return None
    if ret_type == "u8":
        return v0 & 0xFF
    if ret_type == "s8":
        return ((v0 & 0xFF) ^ 0x80) - 0x80
    if ret_type == "u16":
        return v0 & 0xFFFF
    if ret_type == "s16":
        return ((v0 & 0xFFFF) ^ 0x8000) - 0x8000
    if ret_type == "s32":
        return ((v0 & 0xFFFFFFFF) ^ 0x80000000) - 0x80000000
    return v0 & 0xFFFFFFFF


def lua_literal(v, indent=""):
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, int):
        return str(v)
    if isinstance(v, str):
        return '"' + v.replace("\\", "\\\\").replace('"', '\\"') + '"'
    inner = indent + "  "
    if isinstance(v, list):
        return "{\n" + ",\n".join(inner + lua_literal(x, inner) for x in v) + "\n" + indent + "}"
    if isinstance(v, dict):
        return "{\n" + ",\n".join(f"{inner}[{lua_literal(k)}] = {lua_literal(x, inner)}" for k, x in v.items()) + "\n" + indent + "}"
    raise TypeError(type(v))


def layout_buffers(case):
    """Places a case's buffers in scratch RAM (16-byte aligned); returns name -> address."""
    addr, places = SCRATCH_BASE, {}
    for name, data in case.buffers.items():
        places[name] = addr
        addr += (max(len(data), 1) + 15) & ~15
    if addr - SCRATCH_BASE > SCRATCH_SIZE:
        raise ValueError(f"{case.name}: buffers exceed the scratch area")
    return places


def resolve_arg(sym, places, arg):
    if isinstance(arg, int):
        return arg & 0xFFFFFFFF
    if arg[0] == "buf":
        return places[arg[1]]
    if arg[0] == "sym":
        return (sym[arg[1]] + (arg[2] if len(arg) > 2 else 0)) & 0xFFFFFFFF
    raise ValueError(arg)


def job_for(sym, family, case):
    places = layout_buffers(case)
    writes = [{"addr": places[n], "hex": d.hex()} for n, d in case.buffers.items() if d]
    writes += [write_job(sym, w) for w in case.fixture]
    saves = [{"addr": sym[s], "size": n} for s, n in case.saves]
    calls = []
    for c in case.calls:
        calls.append({
            "name": c.func, "func": sym[c.func],
            "args": [resolve_arg(sym, places, a) for a in c.args],
            "reads": [{"addr": read_addr(sym, places, r), "size": r.size} for r in c.reads],
            "writes": [write_job(sym, w) for w in c.writes],
        })
    job = {"name": f"{family}/{case.name}", "saves": saves, "writes": writes, "calls": calls}
    if case.keep:
        job["keep"] = True
    return job


def read_addr(sym, places, r):
    if r.symbol.startswith("buf:"):
        return places[r.symbol[4:]] + r.offset
    return sym[r.symbol] + r.offset


def write_job(sym, w):
    if w.file:
        return {"addr": sym[w.symbol] + w.offset, "file": str(ROOT / w.file)}
    return {"addr": sym[w.symbol] + w.offset, "hex": w.data.hex()}


def arg_json(arg):
    if isinstance(arg, int):
        return arg
    if arg[0] == "buf":
        return {"buf": arg[1]}
    return {"symbol": arg[1], "offset": arg[2] if len(arg) > 2 else 0}


def fixture_json(fixture):
    out = []
    for w in fixture:
        name = f"0x{w.symbol:08X}" if isinstance(w.symbol, int) else w.symbol
        entry = {"symbol": name, "offset": w.offset, "field": w.field}
        if w.file:
            data = (ROOT / w.file).read_bytes()
            entry.update(file=w.file, size=len(data), sha1=hashlib.sha1(data).hexdigest())
        else:
            entry["hex"] = w.data.hex()
        out.append(entry)
    return out


def case_json(case, result, fixtures):
    """fixtures: shared table name -> fixture JSON; a case stores the name of its (deduplicated) fixture."""
    out = {"name": case.name}
    if case.comment:
        out["comment"] = case.comment
    fj = fixture_json(case.fixture)
    for fname, f in fixtures.items():
        if f == fj:
            break
    else:
        fname = f"fixture_{len(fixtures)}"
        fixtures[fname] = fj
    out["fixture"] = fname
    out["buffers"] = {n: d.hex() for n, d in case.buffers.items()}
    out["saves"] = [[sname, size] for sname, size in case.saves]
    out["calls"] = []
    for c, r in zip(case.calls, result["calls"]):
        cj = {"func": c.func, "args": [arg_json(a) for a in c.args], "ret_type": c.ret_type,
              "ret": decode_ret(r["v0"], c.ret_type), "v0": r["v0"]}
        if c.comment:
            cj["comment"] = c.comment
        if c.writes:
            cj["writes"] = fixture_json(c.writes)
        reads = []
        for rd, hx in zip(c.reads, r["reads"]):
            entry = {"symbol": rd.symbol, "offset": rd.offset, "size": rd.size, "field": rd.field}
            if rd.size > LARGE_READ:
                entry["sha1"] = hashlib.sha1(bytes.fromhex(hx)).hexdigest()
            else:
                entry["hex"] = hx
            reads.append(entry)
        if reads:
            cj["reads"] = reads
        out["calls"].append(cj)
    return out


def tree_commit():
    try:
        return subprocess.run(["git", "-C", str(ROOT), "log", "-1", "--format=%H", "--", "src", "include", "config"],
                              check=True, capture_output=True, text=True).stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown"


def bios_path(name):
    if name == "openbios":
        return OPENBIOS
    if name == "retail":
        return RETAIL_BIOS
    return Path(name)


def oracle_meta(resident, bios=OPENBIOS):
    info = json.loads(REDUX_VERSION.read_text())
    return {
        "method": "lua-calls: tests/golden/oracle.lua hijacks the game at pad_update (-debugger -interpreter)",
        "emulator": {"name": "pcsx-redux", "version": info["version"], "build_id": info["buildId"],
                     "changeset": info["changeset"]},
        "bios": {"name": "openbios" if bios == OPENBIOS else ("retail" if bios == RETAIL_BIOS else bios.name),
                 "sha1": hashlib.sha1(bios.read_bytes()).hexdigest()},
        "tree_commit": tree_commit(),
        "resident": resident,
    }


def load_family(name):
    sys.path.insert(0, str(FAMILIES_DIR))
    return importlib.import_module(name)


def all_families():
    return sorted(p.stem for p in FAMILIES_DIR.glob("*.py") if not p.stem.startswith("_"))


def run_oracle(jobs, out_dir, overlays=(), resident=None, verbose=False, bios=OPENBIOS):
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    # A Lua chunk may hold at most 65,536 constants, and every job's hex strings and numbers are constants: the jobs go in
    # chunk files of JOBS_PER_CHUNK jobs each, and jobs.lua gathers them.
    spec = {"overlays": list(overlays)}
    if resident:
        spec["resident"] = resident
    chunks = [jobs[i:i + JOBS_PER_CHUNK] for i in range(0, len(jobs), JOBS_PER_CHUNK)]
    for n, chunk in enumerate(chunks):
        (out_dir / f"jobs_{n}.lua").write_text("return " + lua_literal(chunk) + "\n")
    main = ["local spec = " + lua_literal(spec), "spec.jobs = {}"]
    for n in range(len(chunks)):
        main.append(f'for _, j in ipairs(dofile("{out_dir / f"jobs_{n}.lua"}")) do spec.jobs[#spec.jobs + 1] = j end')
    main.append("return spec")
    (out_dir / "jobs.lua").write_text("\n".join(main) + "\n")
    env = dict(os.environ, DW3_ORACLE_JOBS=str(out_dir / "jobs.lua"), DW3_ORACLE_OUT=str(out_dir))
    if verbose:
        env["DW3_ORACLE_VERBOSE"] = "1"
    mcd = [str(out_dir / "memcard1.mcd"), str(out_dir / "memcard2.mcd")]
    cmd = [str(REDUX), "-no-ui", "-stdout", "-testmode", "-run", "-debugger", "-interpreter", "-iso", str(ISO),
           "-bios", str(bios), "-memcard1", mcd[0], "-memcard2", mcd[1], "-dofile", str(ORACLE_LUA)]
    with open(out_dir / "emulator.log", "w") as log:
        proc = subprocess.run(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=600)
    result_path = out_dir / "results.json"
    if not result_path.exists():
        tail = "\n".join((out_dir / "emulator.log").read_text(errors="replace").splitlines()[-10:])
        raise RuntimeError(f"emulator exited {proc.returncode} without results.json:\n{tail}")
    results = json.loads(result_path.read_text())
    if results.get("status") != "ok":
        raise RuntimeError(f"oracle failed: {results.get('message')} (log {out_dir / 'emulator.log'})")
    return {j["name"]: j for j in results["jobs"]}


def generate(families, out_dir, verbose=False, bios=OPENBIOS):
    """Runs every family's cases in one boot; returns {family: golden dict}."""
    sym = Symbols()
    mods = {f: load_family(f) for f in families}
    cases = {f: m.cases(sym) for f, m in mods.items()}
    resident = None
    jobs = []
    for f, cs in cases.items():
        jobs += [job_for(sym, f, c) for c in cs]
    results = run_oracle(jobs, out_dir, resident=resident, verbose=verbose, bios=bios)
    goldens = {}
    for f, cs in cases.items():
        fixtures = {}
        case_list = [case_json(c, results[f"{f}/{c.name}"], fixtures) for c in cs]
        goldens[f] = {
            "family": f,
            "comment": getattr(mods[f], "COMMENT", ""),
            "generator": f"tests/golden/families/{f}.py",
            "oracle": oracle_meta(getattr(mods[f], "RESIDENT", "CNTY_SEL loaded: the EXE's code and data are resident"), bios),
            "fixtures": fixtures,
            "cases": case_list,
        }
    return goldens


def check_tools():
    missing = [str(p) for p in (REDUX, ISO, ORACLE_LUA, SYMBOLS) if not p.exists()]
    if missing:
        print("oracle: missing " + ", ".join(missing), file=sys.stderr)
        sys.exit(2)


def comparable(golden):
    """A golden without the provenance that may differ between a valid regeneration and the file: the matching tree's
    commit (a doc-only commit changes nothing) and the BIOS (a retail-BIOS run must reproduce the OpenBIOS goldens)."""
    g = dict(golden)
    g["oracle"] = {k: v for k, v in golden["oracle"].items() if k not in ("tree_commit", "bios")}
    return g


def diff_goldens(expected, actual, path=""):
    """First few differences between two JSON values."""
    diffs = []
    if isinstance(expected, dict) and isinstance(actual, dict):
        for k in sorted(set(expected) | set(actual)):
            if k not in expected or k not in actual:
                diffs.append(f"{path}/{k}: only in {'expected' if k in expected else 'actual'}")
            else:
                diffs += diff_goldens(expected[k], actual[k], f"{path}/{k}")
    elif isinstance(expected, list) and isinstance(actual, list):
        if len(expected) != len(actual):
            diffs.append(f"{path}: {len(expected)} vs {len(actual)} entries")
        for i, (e, a) in enumerate(zip(expected, actual)):
            name = e.get("name") if isinstance(e, dict) else None
            diffs += diff_goldens(e, a, f"{path}[{name or i}]")
    elif expected != actual:
        diffs.append(f"{path}: expected {json.dumps(expected)[:80]}, got {json.dumps(actual)[:80]}")
    return diffs[:20]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("gen", "check"):
        p = sub.add_parser(name)
        p.add_argument("families", nargs="*")
        p.add_argument("--out", help="keep the emulator run's files here")
        p.add_argument("-v", "--verbose", action="store_true")
        p.add_argument("--bios", default="openbios", help="openbios (default), retail (cross-check only), or a BIOS file")
    sub.add_parser("list")
    args = ap.parse_args()
    if args.cmd == "list":
        print("\n".join(all_families()))
        return 0
    check_tools()
    families = args.families or all_families()
    out = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="dw3_oracle_"))
    print(f"oracle: {args.cmd} {' '.join(families)} (one emulator boot; run files in {out})")
    bios = bios_path(args.bios)
    if not bios.exists():
        print(f"oracle: BIOS {bios} not found", file=sys.stderr)
        return 2
    if args.cmd == "gen" and bios != OPENBIOS:
        print("oracle: committed goldens come from OpenBIOS; use another BIOS only with check", file=sys.stderr)
        return 2
    goldens = generate(families, out, verbose=args.verbose, bios=bios)
    status = 0
    for f, g in goldens.items():
        path = GOLDEN_DIR / f"{f}.json"
        n = sum(len(c["calls"]) for c in g["cases"])
        if args.cmd == "gen":
            path.write_text(json.dumps(g, indent=1) + "\n")
            print(f"  wrote {path.relative_to(ROOT)}: {len(g['cases'])} cases, {n} calls")
        else:
            if not path.exists():
                print(f"  {f}: no golden file ({path.relative_to(ROOT)}); run gen")
                status = 1
                continue
            diffs = diff_goldens(comparable(json.loads(path.read_text())), comparable(g))
            if diffs:
                status = 1
                print(f"  {f}: MISMATCH\n    " + "\n    ".join(diffs))
            else:
                print(f"  {f}: reproduced exactly ({len(g['cases'])} cases, {n} calls)")
    return status


if __name__ == "__main__":
    sys.exit(main())
