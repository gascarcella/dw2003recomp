#!/usr/bin/env python3
"""Which link unit owns each .data/.sdata/.sbss/.bss symbol of the EXE (the .data/.bss split plan).

  tools/venv/bin/python tools/data_owners.py            # table + proposed boundaries + conflicts
  tools/venv/bin/python tools/data_owners.py --yaml     # only the proposed subsegment lines
  tools/venv/bin/python tools/data_owners.py --region sbss

Needs a split and built tree (scripts/build.sh): symbols and sizes come from the linked ELF
(build/main/SLES_039.36.elf), references from splat's asm (asm/main/*.s for every code unit, the
full disassembly of C units included, and the data asm under asm/main/data/).

Evidence per symbol (strongest first; DECISIONS "Data in C, split per object"):
  gp:U   unit U reaches it through $gp. ASPSX only does that for a variable defined (or COMMON) in the
         same file, so U owns it (or U declares it COMMON).
  fp:U   it holds pointers to functions of U (a module's function table lives with the module).
  ref:U  code of U references it with %hi/%lo (weak when several units do).
  via:S  it points at, or is pointed at by, data symbol S, so it probably shares S's owner.
Owners are then chosen so that each region is in link order (every section is concatenated in the
link order of the objects): a dynamic program picks, per region, the non-decreasing assignment of
units that agrees with the most evidence (weighted gp > fp > ref > via). Symbols without evidence
fall to the unit before them; the boundary between two units is uncertain anywhere in that run,
which the "range" comment shows. Proposed starts are snapped down to 4 bytes in .data and to 8 in
.sdata/.sbss/.bss, where every object starts 8-aligned (DECISIONS "Data in C, split per object").
Overlay uses of EXE data are listed (ovl:N) but are no evidence: overlays link after the EXE.
The output is a proposal; config/main.yaml holds the decisions, with comments where they differ.

  tools/venv/bin/python tools/data_owners.py -t stdgname [--all]   # a tier-1 overlay

Overlays (-G0: no $gp) have .data then .bss (zeros in the file), each in link order. Evidence:
ref:U (code of U), fp:U (pointers to U's functions), ro:U (pointers into U's .rodata: the strings
of an initializer are in the same object), via:S. The .bss start is chosen with the owners: a
second non-decreasing run that may only start where every symbol from there on is zero; with a
tie (no ordering evidence), the earliest start is taken and the tie is printed. Symbols, sizes
and code from build/<t>/<T>.PRO.elf and asm/<t>/; zero test on the original file.
"""
import argparse
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent
ASM = ROOT / "asm/main"
ELF = ROOT / "build/main/SLES_039.36.elf"
NM = ROOT / "tools/binutils/bin/mipsel-linux-gnu-nm"
VRAM, ROM = 0x80010000, 0x800

# Regions, from crt0 and the plan (docs/DISC_LAYOUT.md): .data, .sdata from _gp, .sbss, .bss.
GP = 0x8005CB50
REGIONS = [  # name, start, end
    ("data", 0x8003EDCC, GP),
    ("sdata", GP, 0x8005CCE8),
    ("sbss", 0x8005CCE8, 0x8005CD28),
    ("bss", 0x8005CD28, 0x80082CB0),
]
# Linker-script symbols and splat's markers, not variables.
LINKER_SYM = re.compile(r"(_(ROM|VRAM|DATA|BSS|TEXT|RODATA)_(START|END|SIZE)|_VRAM(_END)?|\.NON_MATCHING)$|^_gp$")
WEIGHT = {"gp": 1000, "fp": 50, "ref": 10, "via": 3}

INS = re.compile(r"^\s*/\* \w+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+(\S+)\s*(.*)")
SYMREF = re.compile(r"%(hi|lo|gp_rel)\(([A-Za-z_]\w*)(?: \+ (0x[0-9A-Fa-f]+))?\)")
WORD = re.compile(r"^\s*/\* \w+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+\.word ([A-Za-z_]\w*)(?: \+ (0x[0-9A-Fa-f]+))?")


def code_units():
    """[(unit, text start vram)] in link order (the order of the code subsegments)."""
    cfg = yaml.safe_load((ROOT / "config/main.yaml").read_text())
    main = next(s for s in cfg["segments"] if isinstance(s, dict) and s.get("name") == "main")
    units = []
    for sub in main["subsegments"]:
        if isinstance(sub, list) and len(sub) >= 3 and sub[1] in ("c", "asm"):
            units.append((sub[2], VRAM + sub[0] - ROM))
    return units


def elf_symbols():
    """{name: (addr, size)} for every defined symbol of the linked EXE."""
    out = subprocess.run([str(NM), "-S", "-n", str(ELF)], capture_output=True, text=True, check=True).stdout
    syms = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) == 4:
            syms[p[3]] = (int(p[0], 16), int(p[1], 16))
        elif len(p) == 3:
            syms.setdefault(p[2], (int(p[0], 16), 0))
    return syms


def region_of(a):
    for name, s, e in REGIONS:
        if s <= a < e:
            return name
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--region", choices=[r[0] for r in REGIONS], help="only this region")
    ap.add_argument("--yaml", action="store_true", help="only print the proposed subsegment lines")
    ap.add_argument("--all", action="store_true", help="list every symbol, not only boundaries/conflicts")
    ap.add_argument("-t", "--target", default="main", help="a tier-1 overlay (e.g. stdgname) instead of the EXE")
    args = ap.parse_args()
    if args.target != "main":
        return overlay_main(args)
    if not ELF.exists() or not ASM.is_dir():
        sys.exit("build/main or asm/main missing: run scripts/build.sh first")

    units = code_units()
    order = {u: i for i, (u, _) in enumerate(units)}
    syms = elf_symbols()

    # Data symbols in the four regions, with extents (size 0 -> up to the next symbol).
    data = sorted((a, n) for n, (a, s) in syms.items() if region_of(a) and not LINKER_SYM.search(n))
    dsyms = []  # [addr, end, name]
    for i, (a, n) in enumerate(data):
        if dsyms and dsyms[-1][0] == a:
            continue  # alias
        size = syms[n][1]
        nxt = data[i + 1][0] if i + 1 < len(data) else REGIONS[-1][2]
        dsyms.append([a, a + size if size else nxt, n])
    # Cap at the next symbol and at the region end.
    for i, d in enumerate(dsyms):
        nxt = dsyms[i + 1][0] if i + 1 < len(dsyms) else REGIONS[-1][2]
        d[1] = min(max(d[1], d[0] + 1), nxt) if d[1] > d[0] else nxt
    starts = [d[0] for d in dsyms]

    def owner_sym(addr):
        import bisect
        i = bisect.bisect_right(starts, addr) - 1
        if i >= 0 and dsyms[i][0] <= addr < dsyms[i][1]:
            return dsyms[i][2]
        return None

    # Functions -> unit (by address, from the code unit starts).
    ustarts = [s for _, s in units]

    def unit_of_code(addr):
        import bisect
        i = bisect.bisect_right(ustarts, addr) - 1
        return units[i][0] if i >= 0 else None

    ev = defaultdict(lambda: defaultdict(set))  # sym -> kind -> {unit or sym}
    files = [p for p in ASM.rglob("*.s") if "nonmatchings" not in p.parts and p.parts[len(ASM.parts)] != "data"]
    for f in files:
        unit = str(f.relative_to(ASM).with_suffix(""))
        if unit not in order:
            continue
        for line in f.read_text().splitlines():
            m = INS.match(line)
            if not m:
                continue
            for kind, name, off in SYMREF.findall(m.group(3)):
                if name not in syms or LINKER_SYM.search(name):
                    continue
                s = owner_sym(syms[name][0] + int(off or "0", 16))
                if s:
                    ev[s]["gp" if kind == "gp_rel" else "ref"].add(unit)

    # Overlays (asm/<overlay>/) that use EXE data: shown, not used as evidence (an overlay is
    # linked after the EXE, so it says nothing about which EXE file defines the variable).
    for d in sorted(p for p in (ROOT / "asm").iterdir() if p.is_dir() and p.name != "main"):
        for f in d.rglob("*.s"):
            for name, off in re.findall(r"%(?:hi|lo|gp_rel)\((D_8[0-9A-F]{7})(?: \+ (0x[0-9A-Fa-f]+))?\)", f.read_text()):
                if name in syms and (s := owner_sym(syms[name][0] + int(off or "0", 16))):
                    ev[s]["ovl"].add(d.name)

    # Pointers stored in data (to functions -> fp, to data -> via both ways): the data asm, and the
    # full disassembly of C units, which holds their data once it is in C.
    for f in ASM.rglob("*.s"):
        if "nonmatchings" in f.parts:
            continue
        for line in f.read_text().splitlines():
            m = WORD.match(line)
            if not m or m.group(2) not in syms:
                continue
            here = owner_sym(int(m.group(1), 16))
            target = syms[m.group(2)][0] + int(m.group(3) or "0", 16)
            if not here:
                continue
            if region_of(target):
                t = owner_sym(target)
                if t and t != here:
                    ev[here]["via"].add(t)
                    ev[t]["via"].add(here)
            elif (u := unit_of_code(target)) and 0x80010E88 <= target < 0x8003EDCC:
                ev[here]["fp"].add(u)

    # Per region: DP over (symbol, unit), non-decreasing units, maximizing agreeing evidence.
    result = {}  # sym -> owner
    report = []
    for rname, rs, re_ in REGIONS:
        rsyms = [d for d in dsyms if rs <= d[0] < re_]
        n, U = len(rsyms), len(units)

        def score(sym, ui):
            e = ev[sym[2]]
            u = units[ui][0]
            return sum(WEIGHT[k] for k in ("gp", "fp", "ref") if u in e[k])

        # Iterate twice so "via" evidence can use the owners of the first pass.
        owners = [None] * n
        for _ in range(2):
            via_owner = {}
            for i, d in enumerate(rsyms):
                via_owner[d[2]] = units[owners[i]][0] if owners[i] is not None else None
            best = [[0] * U for _ in range(n + 1)]
            back = [[0] * U for _ in range(n + 1)]
            for i, d in enumerate(rsyms):
                vias = {result.get(v) or via_owner.get(v) for v in ev[d[2]]["via"]}
                run_best, run_arg = -1, 0
                for ui in range(U):
                    if best[i][ui] > run_best:
                        run_best, run_arg = best[i][ui], ui
                    s = score(d, ui) + (WEIGHT["via"] if units[ui][0] in vias else 0)
                    best[i + 1][ui] = run_best + s
                    back[i + 1][ui] = run_arg
            # Prefer the earliest unit that reaches the maximum (unknowns fall to the previous unit).
            ui = max(range(U), key=lambda k: (best[n][k], -k))
            for i in range(n, 0, -1):
                owners[i - 1] = ui
                ui = back[i][ui]
        for i, d in enumerate(rsyms):
            result[d[2]] = units[owners[i]][0]

        # Boundaries: each unit's first symbol, and the range in which the boundary could move
        # (the run of symbols without evidence for either neighbour).
        def has_ev(d, u):
            e = ev[d[2]]
            return any(u in e[k] for k in ("gp", "fp", "ref")) or any(result.get(v) == u for v in e["via"])

        prev = None
        for i, d in enumerate(rsyms):
            u = result[d[2]]
            if u != prev:
                j = i
                while j > 0 and not has_ev(rsyms[j - 1], prev) and not has_ev(rsyms[j - 1], u):
                    j -= 1
                k = i
                while k < n - 1 and not has_ev(rsyms[k], u) and result[rsyms[k + 1][2]] == u:
                    k += 1
                # Objects' .data start 4-aligned (an unaligned first label, `_ctype_ + 1`, means the
                # object starts at the word before it); their .sdata/.sbss/.bss start 8-aligned.
                mask = ~3 if rname == "data" else ~7
                if report and report[-1][0] == rname and report[-1][1] == d[0] & mask:
                    report.pop()  # the previous unit only had padding
                report.append((rname, d[0] & mask, u, rsyms[j][0] & mask, rsyms[k][0] & mask))
                prev = u
        if not args.yaml and args.region in (None, rname):
            print(f"\n== .{rname} 0x{rs:08X}-0x{re_:08X}: {n} symbols ==")
            print(f"{'addr':8} {'size':>6} {'owner':24} {'symbol':28} evidence")
            prev = None
            for i, d in enumerate(rsyms):
                e = ev[d[2]]
                u = result[d[2]]
                cand = set().union(e["gp"], e["fp"], e["ref"])
                conflict = cand and u not in cand
                strong_conflict = e["gp"] and u not in e["gp"]
                show = args.all or u != prev or conflict or (i + 1 < n and result[rsyms[i + 1][2]] != u)
                prev = u
                if not show:
                    continue
                parts = [f"{k}:{','.join(sorted(e[k]))}" for k in ("gp", "fp", "ref") if e[k]]
                if e["via"]:
                    parts.append("via:" + ",".join(sorted(e["via"]))[:60])
                if e["ovl"]:
                    parts.append(f"ovl:{len(e['ovl'])}")
                flag = " !!CONFLICT(gp)" if strong_conflict else (" !conflict" if conflict else "")
                print(f"{d[0]:08X} {d[1] - d[0]:6X} {u:24} {d[2]:28} {' '.join(parts)}{flag}")

    print("\n# Proposed subsegments (vram, unit, boundary range where it could move):")
    for rname, a, u, lo, hi in report:
        if args.region not in (None, rname):
            continue
        rng = "" if lo == hi == a else f"  # boundary in 0x{lo:08X}-0x{hi:08X}"
        typ = rname
        print(f"      - [0x{a - VRAM + ROM:X}, {typ}, {u}]  # 0x{a:08X}{rng}")


# ---- Tier-1 overlays (--target): .data then .bss, each in link order; no $gp (overlays are -G0) ----
OVL_WEIGHT = {"fp": 50, "ro": 50, "ref": 10, "via": 3}


def overlay_layout(t):
    """The overlay's config: (units [(name, text start, text end)], rodata [(unit, start, end)],
    data start vram, file end vram, vram base, original file path, elf path, asm path)."""
    cfg = yaml.safe_load((ROOT / f"config/{t}.yaml").read_text())
    opts = cfg["options"]
    seg = next(s for s in cfg["segments"] if isinstance(s, dict) and s.get("type") == "code")
    base, rom0 = seg["vram"], seg["start"]
    end = base + cfg["segments"][-1][0] - rom0
    subs = [s for s in seg["subsegments"] if isinstance(s, list)] + [[cfg["segments"][-1][0], "end"]]
    units, rodata, data_start = [], [], None
    for cur, nxt in zip(subs, subs[1:]):
        lo, hi = base + cur[0] - rom0, base + nxt[0] - rom0
        kind = cur[1].lstrip(".")
        if kind in ("c", "asm"):
            units.append((cur[2], lo, hi))
        elif kind == "rodata":
            rodata.append((cur[2], lo, hi))
        elif kind in ("data", "bss", "bin") and data_start is None and units:
            data_start = lo
    target = (ROOT / "config" / opts["base_path"] / opts["target_path"]).resolve()
    return units, rodata, data_start, end, base, target, ROOT / opts["elf_path"], ROOT / opts["asm_path"]


def overlay_main(args):
    import bisect
    t = args.target.lower()
    units, rodata, dstart, dend, base, binpath, elf, asm = overlay_layout(t)
    if not elf.exists() or not asm.is_dir():
        sys.exit(f"{elf} or {asm} missing: run scripts/build.sh first")
    names = [u for u, _, _ in units]
    orig = binpath.read_bytes()
    out = subprocess.run([str(NM), "-S", "-n", str(elf)], capture_output=True, text=True, check=True).stdout
    syms = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) >= 3 and not p[-1].endswith(".NON_MATCHING") and not LINKER_SYM.search(p[-1]):
            syms.setdefault(p[-1], (int(p[0], 16), int(p[1], 16) if len(p) == 4 else 0))
    data = sorted((a, n) for n, (a, s) in syms.items() if dstart <= a < dend and not n.startswith("."))
    dsyms = []  # [addr, end, name]
    for a, n in data:
        if dsyms and dsyms[-1][0] == a:
            continue
        dsyms.append([a, a + syms[n][1], n])
    for i, d in enumerate(dsyms):  # extents: own size, capped at the next symbol; size 0 -> next symbol
        nxt = dsyms[i + 1][0] if i + 1 < len(dsyms) else dend
        d[1] = min(d[1], nxt) if d[1] > d[0] else nxt
    if dsyms and dsyms[0][0] != dstart:
        dsyms.insert(0, [dstart, dsyms[0][0], f"(0x{dstart:08X})"])
    starts = [d[0] for d in dsyms]

    def owner_sym(addr):
        i = bisect.bisect_right(starts, addr) - 1
        return dsyms[i][2] if i >= 0 and dsyms[i][0] <= addr < dsyms[i][1] else None

    def unit_in(ranges, addr):
        return next((u for u, lo, hi in ranges if lo <= addr < hi), None)

    def zero(d):
        return not any(orig[d[0] - base:d[1] - base])

    ev = defaultdict(lambda: defaultdict(set))
    for u in names:
        f = asm / f"{u}.s"
        if not f.exists():
            continue
        for line in f.read_text().splitlines():
            m = INS.match(line)
            if not m:
                continue
            for kind, name, off in SYMREF.findall(m.group(3)):
                if name in syms and (s := owner_sym(syms[name][0] + int(off or "0", 16))):
                    ev[s]["ref"].add(u)
    for f in asm.rglob("*.s"):
        if "nonmatchings" in f.parts:
            continue
        for line in f.read_text().splitlines():
            m = WORD.match(line)
            if not m or m.group(2) not in syms or not (here := owner_sym(int(m.group(1), 16))):
                continue
            target = syms[m.group(2)][0] + int(m.group(3) or "0", 16)
            if dstart <= target < dend:
                if (s := owner_sym(target)) and s != here:
                    ev[here]["via"].add(s)
                    ev[s]["via"].add(here)
            elif u := unit_in(units, target):
                ev[here]["fp"].add(u)
            elif u := unit_in(rodata, target):
                ev[here]["ro"].add(u)

    U, n = len(names), len(dsyms)

    def assign(lo, hi, via_owner):
        """Best non-decreasing owners of dsyms[lo:hi]: (score, [unit index]); unknowns join the unit before."""
        def score(i, u):
            e = ev[dsyms[i][2]]
            s = sum(OVL_WEIGHT[k] for k in ("fp", "ro", "ref") if names[u] in e[k])
            return s + (OVL_WEIGHT["via"] if names[u] in {via_owner.get(v) for v in e["via"]} else 0)
        S = [[0] * U for _ in range(hi - lo + 1)]  # S[j][u]: symbols lo+j.., the first one's unit >= u
        for j in range(hi - lo - 1, -1, -1):
            run = -1
            for u in range(U - 1, -1, -1):
                run = max(run, S[j + 1][u])
                S[j][u] = max(score(lo + j, u) + run, S[j][u + 1] if u + 1 < U else -1)
        owners, prev = [], 0
        for j in range(hi - lo):
            # the smallest unit >= prev that keeps the optimum (unknowns join the unit before them)
            target_score = S[j][prev]
            u = prev
            while score(lo + j, u) + max(S[j + 1][u:]) != target_score:
                u += 1
            owners.append(u)
            prev = u
        return (S[0][0] if hi > lo else 0), owners

    # .bss can only start where every symbol from there on is zero.
    kmin = n
    while kmin > 0 and zero(dsyms[kmin - 1]):
        kmin -= 1
    via_owner = {}
    for _ in range(2):
        cands = []
        for k in range(kmin, n + 1):
            sd, od = assign(0, k, via_owner)
            sb, ob = assign(k, n, via_owner)
            cands.append((sd + sb, k, od + ob))
        best = max(c[0] for c in cands)
        ks = [c[1] for c in cands if c[0] == best]
        _, k, owners = next(c for c in cands if c[0] == best)  # the earliest: the most .bss
        via_owner = {d[2]: names[owners[i]] for i, d in enumerate(dsyms)}

    def at(i):
        return f"0x{dsyms[i][0]:08X}" if i < n else "end"

    print(f"== {t}: .data/.bss 0x{dstart:08X}-0x{dend:08X}, {n} symbols, units: {' '.join(names)}")
    print(f"   all zero from {at(kmin)}; best score {best} with .bss at {', '.join(at(i) for i in ks)}"
          f" (chosen: {at(k)})")
    print(f"{'addr':8} {'size':>6} {'sec':4} {'owner':28} {'symbol':26} evidence")
    report = []
    for i, d in enumerate(dsyms):
        sec = "data" if i < k else "bss"
        u = names[owners[i]]
        e = ev[d[2]]
        cand = set().union(e["fp"], e["ro"], e["ref"])
        conflict = cand and u not in cand
        newb = i == 0 or i == k or owners[i] != owners[i - 1]
        if newb:
            report.append((sec, d[0], u, i))
        last = i + 1 == n or i + 1 == k or owners[i + 1] != owners[i]
        if args.all or newb or conflict or last:
            parts = [f"{kk}:{','.join(sorted(e[kk]))}" for kk in ("fp", "ro", "ref") if e[kk]]
            if e["via"]:
                parts.append("via:" + ",".join(sorted(e["via"]))[:50])
            z = " zero" if zero(d) else ""
            print(f"{d[0]:08X} {d[1] - d[0]:6X} {sec:4} {u:28} {d[2]:26} {' '.join(parts)}{z}"
                  + (" !conflict" if conflict else ""))

    def has_ev(i, u):
        e = ev[dsyms[i][2]]
        return any(u in e[kk] for kk in ("fp", "ro", "ref")) or any(via_owner.get(v) == u for v in e["via"])

    print("\n# Proposed subsegments (rom offset, type, unit; range: where the boundary could move):")
    for sec, a, u, i in report:
        j = i  # move the start back over symbols with no evidence for either side (same section)
        prev = names[owners[i - 1]] if i else None
        lo_lim = k if sec == "bss" else 0
        if i != k:
            while j > lo_lim and not has_ev(j - 1, prev) and not has_ev(j - 1, u):
                j -= 1
        rng = f"  # boundary in 0x{dsyms[j][0]:08X}-0x{a:08X}" if j != i else ""
        print(f"      - [0x{a - base:X}, {sec}, {u}]  # 0x{a:08X} (mod 8 = {a % 8}){rng}")


if __name__ == "__main__":
    main()
