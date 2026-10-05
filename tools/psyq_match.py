#!/usr/bin/env python3
"""Find Psy-Q library objects in a PS1 executable or overlay.

Uses the masked .text signatures from lab313ru/psx_psyq_signatures (installed by
`scripts/setup.sh ext` into tools/ext/psyq-sigs). Each signature is one library object
(e.g. LIBGPU.LIB/FONT.OBJ) with `??` on relocated bytes and the offsets of its labels.

Identical signatures across Psy-Q versions are merged, so every hit carries the set of
versions it is consistent with; the intersection over all distinctive hits narrows the
SDK version. Overlapping hits are resolved by keeping the longest signature.

Usage:
  psyq_match.py SLES_039.36                       # PS-X EXE (header skipped)
  psyq_match.py --raw --base 0x80082CB0 FOO.PRO   # headerless overlay
  options: --symbols FILE   write symbol_addrs.txt lines for named labels
           --min-size N     ignore signatures shorter than N bytes (default 16)
           --min-cluster N  ignore runs of adjacent hits under N bytes (default 128)
"""
import argparse
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SIGDIR = ROOT / "tools/ext/psyq-sigs"
# Directory name -> version string, in release order.
VERSIONS = ["260", "300", "330", "340", "350", "3610", "3611", "370",
            "400", "410", "420", "430", "440", "450", "460", "470"]


def vname(v):
    return {"3610": "3.6.10", "3611": "3.6.11"}.get(v, f"{v[0]}.{v[1]}")


def load_sigs():
    """-> {sig string: {"lib", "obj", "labels", "versions": [..]}} (one entry per distinct sig)."""
    sigs = {}
    for v in VERSIONS:
        for f in sorted((SIGDIR / v).glob("*.json")):
            lib = f.name[:-len(".json")]
            for o in json.loads(f.read_text()):
                if not o.get("sig", "").strip():
                    continue  # bss-only object (no .text)
                key = (lib, o["name"], o["sig"].strip())
                e = sigs.get(key)
                if e is None:
                    e = sigs[key] = {"lib": lib, "obj": o["name"], "sig": o["sig"].strip(),
                                     "labels": {}, "versions": []}
                e["versions"].append(v)
                e["labels"][v] = o.get("labels", [])  # names can change between versions
    return list(sigs.values())


def to_regex(sig):
    parts = []
    for tok in sig.split():
        parts.append(b"." if tok == "??" else re.escape(bytes([int(tok, 16)])))
    return re.compile(b"".join(parts), re.DOTALL)


def is_func_label(name):
    return not re.match(r"^(loc|text|data|bss|rdata|sdata|sbss)_[0-9A-Fa-f]+$", name)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary")
    ap.add_argument("--raw", action="store_true", help="headerless binary (overlay)")
    ap.add_argument("--base", type=lambda x: int(x, 0), help="load address for --raw")
    ap.add_argument("--min-size", type=int, default=16)
    ap.add_argument("--min-cluster", type=int, default=128,
                    help="ignore runs of adjacent hits smaller than this (default 128 B)")
    ap.add_argument("--symbols", help="write symbol_addrs.txt-style lines here")
    ap.add_argument("--prefix", default="", help="prefix for emitted symbol names (overlays)")
    args = ap.parse_args()

    data = Path(args.binary).read_bytes()
    if args.raw:
        if args.base is None:
            sys.exit("--raw needs --base")
        base, code = args.base, data
    else:
        assert data[:8] == b"PS-X EXE", "not a PS-X EXE"
        base = struct.unpack_from("<I", data, 0x18)[0]
        code = data[0x800:]

    sigs = [s for s in load_sigs() if len(s["sig"].split()) >= args.min_size]
    hits = []
    for s in sigs:
        size = len(s["sig"].split())
        # Not finditer: hits may overlap, so restart one byte after each hit.
        pos = 0
        rx = to_regex(s["sig"])
        while True:
            m = rx.search(code, pos)
            if not m:
                break
            if m.start() % 4 == 0:
                hits.append({"off": m.start(), "size": size, **s})
            pos = m.start() + 1

    # Group candidates by place: several objects (or versions of one) can match the same bytes.
    groups = defaultdict(lambda: defaultdict(set))  # (off, size) -> (lib, obj) -> versions
    labels = {}  # (key, off, size) -> {version: labels}; names can differ between versions
    for h in hits:
        key = (h["lib"], h["obj"])
        groups[(h["off"], h["size"])][key] |= set(h["versions"])
        labels.setdefault((key, h["off"], h["size"]), {}).update(h["labels"])
    places = [{"off": off, "size": size, "cands": dict(c)} for (off, size), c in groups.items()]

    # Resolve overlaps greedily: longest first.
    places.sort(key=lambda p: (-p["size"], p["off"]))
    taken, kept, overlapped = [], [], 0
    for p in places:
        a, b = p["off"], p["off"] + p["size"]
        if any(a < e and s < b for s, e in taken):
            overlapped += 1
            continue
        taken.append((a, b))
        kept.append(p)
    kept.sort(key=lambda p: p["off"])

    # Library code is linked as one contiguous run. Short hits that are not part of a
    # run of adjacent objects totalling >= --min-cluster bytes are coincidences in game code.
    clusters, cur = [], []
    for p in kept:
        if cur and p["off"] != cur[-1]["off"] + cur[-1]["size"]:
            clusters.append(cur)
            cur = []
        cur.append(p)
    if cur:
        clusters.append(cur)
    kept, isolated = [], []
    for c in clusters:
        (kept if sum(p["size"] for p in c) >= args.min_cluster else isolated).extend(c)

    for p in kept:
        p["versions"] = set().union(*p["cands"].values())
    tally = defaultdict(int)
    for p in kept:
        for v in p["versions"]:
            tally[v] += p["size"]
    total = sum(p["size"] for p in kept)
    best = max(VERSIONS, key=lambda v: (tally[v], VERSIONS.index(v)))

    # Pick one object per place: prefer candidates consistent with the best version, then
    # objects not already claimed by an unambiguous place (each object is linked once).
    for p in kept:
        c = {k: v for k, v in p["cands"].items() if best in v} or p["cands"]
        p["pick"] = sorted(c)
    # Then libraries that are linked for sure (some place has candidates from that library
    # only): tiny functions have byte-identical twins in libraries the game never uses.
    sole = defaultdict(int)
    for p in kept:
        if len(p["pick"]) == 1:
            sole[p["pick"][0]] += 1
    linked_libs = {p["pick"][0][0] for p in kept if len({k[0] for k in p["pick"]}) == 1}
    for p in kept:
        if len(p["pick"]) > 1:
            unclaimed = [k for k in p["pick"] if not sole[k]]
            if unclaimed:
                p["pick"] = unclaimed
        if len(p["pick"]) > 1:
            in_linked = [k for k in p["pick"] if k[0] in linked_libs]
            if in_linked:
                p["pick"] = in_linked

    print(f"{len(sigs)} distinct signatures (>= {args.min_size} B); {len(kept)} objects placed, "
          f"{overlapped} overlapping hits dropped, {len(isolated)} isolated short hits ignored\n")
    print(f"{'address':10} {'end':10} {'size':>6}  {'lib/obj':28} versions")
    for p in kept:
        v = sorted(p["versions"], key=VERSIONS.index)
        vs = "all" if len(v) == len(VERSIONS) else (
            f"{vname(v[0])}–{vname(v[-1])}" if VERSIONS.index(v[-1]) - VERSIONS.index(v[0]) + 1 == len(v)
            else ",".join(vname(x) for x in v))
        name = " | ".join(f"{l}/{o}" for l, o in p["pick"])
        flag = "  AMBIGUOUS" if len(p["pick"]) > 1 else ""
        print(f"{base + p['off']:08X}   {base + p['off'] + p['size']:08X}   {p['size']:6}  {name:28} {vs}{flag}")
    if isolated:
        print("\nisolated short hits (ignored):")
        for p in isolated:
            print(f"  {base + p['off']:08X} {p['size']:4} B  " + " | ".join(f"{l}/{o}" for l, o in sorted(p["cands"])))

    print(f"\nmatched bytes: {total} (0x{total:X})")
    print("bytes consistent with each version:")
    for v in VERSIONS:
        if tally[v]:
            print(f"  {vname(v):7} {tally[v]:7}  {100 * tally[v] / total:5.1f}%")
    common = set(VERSIONS).intersection(*(p["versions"] for p in kept)) if kept else set()
    print("versions consistent with every placed object:",
          ", ".join(vname(v) for v in VERSIONS if v in common) or "none")
    odd = [p for p in kept if best not in p["versions"]]
    print(f"objects NOT consistent with {vname(best)}: {len(odd)}")
    for p in odd:
        print(f"  {base + p['off']:08X} " + " | ".join(f"{l}/{o}" for l, o in p["pick"]) +
              f" ({p['size']} B): {','.join(vname(x) for x in sorted(p['versions'], key=VERSIONS.index))}")
    # Version-distinctive objects: the newest version where each one first appears.
    if kept:
        newest_floor = max((min(p["versions"], key=VERSIONS.index) for p in kept), key=VERSIONS.index)
        print(f"newest 'first version' among placed objects: {vname(newest_floor)}")
        for p in kept:
            if min(p["versions"], key=VERSIONS.index) == newest_floor:
                print(f"  {base + p['off']:08X} " + " | ".join(f"{l}/{o}" for l, o in p["pick"]))

    if kept:
        lo = min(p["off"] for p in kept)
        hi = max(p["off"] + p["size"] for p in kept)
        print(f"\nlibrary span: {base + lo:08X}–{base + hi:08X}")
        gaps, cur = [], lo
        for p in kept:
            if p["off"] > cur:
                gaps.append((cur, p["off"]))
            cur = max(cur, p["off"] + p["size"])
        print(f"unmatched gaps inside the span: {len(gaps)}, {sum(b - a for a, b in gaps)} B")
        for a, b in gaps:
            print(f"  {base + a:08X}–{base + b:08X} ({b - a} B)")

    if args.symbols:
        lines, seen, skipped = [], {}, 0
        def funcs(p, key):
            by_ver = labels[(key, p["off"], p["size"])]
            labs = by_ver.get(best) or by_ver[max(by_ver, key=VERSIONS.index)]
            return [(l["name"], l["offset"]) for l in labs if is_func_label(l["name"])]
        for p in kept:
            # Ambiguous object, but every candidate defines the same functions: name them anyway.
            if len(p["pick"]) > 1 and len({tuple(funcs(p, k)) for k in p["pick"]}) == 1:
                lines.append(f"// {base + p['off']:08X} one of " +
                             " | ".join(f"{l}/{o}" for l, o in p["pick"]) + " (same names)")
                p["pick"] = p["pick"][:1]
                p["same_names"] = True
            if len(p["pick"]) > 1:
                skipped += 1
                lines.append(f"// {base + p['off']:08X} ambiguous, left unnamed: " +
                             " | ".join(f"{l}/{o}" for l, o in p["pick"]))
                continue
            key = p["pick"][0]
            if not p.get("same_names"):
                lines.append(f"// {key[0]}/{key[1]}")
            for lab_name, lab_off in funcs(p, key):
                name = args.prefix + lab_name
                addr = base + p["off"] + lab_off
                if lab_name.startswith("stup"):  # crt0 (2MBYTE.OBJ) internal labels, not functions
                    lines.append(f"// {name} = 0x{addr:08X}; (crt0 label)")
                    continue
                if addr in seen.values():
                    lines.append(f"// {name} = 0x{addr:08X}; (alias of an earlier name)")
                    continue
                if name in seen and seen[name] != addr:
                    print(f"warning: {name} at {seen[name]:08X} and {addr:08X}; keeping the first",
                          file=sys.stderr)
                    continue
                seen[name] = addr
                lines.append(f"{name} = 0x{addr:08X}; // type:func")
        Path(args.symbols).write_text("\n".join(lines) + "\n")
        print(f"\nwrote {len(seen)} symbols to {args.symbols} ({skipped} ambiguous objects left unnamed)")


if __name__ == "__main__":
    main()
