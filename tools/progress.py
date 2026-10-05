#!/usr/bin/env python3
"""Progress toward a fully decompiled build, as a Markdown table with text progress bars.

Every output file already rebuilds byte-identical from split assembly; the percentages here are the share of
game code that is compiled from matching C (objdiff's matched code, in bytes). Psy-Q SDK code is excluded.

  tools/venv/bin/python tools/progress.py            # print the table (runs objdiff-cli report first)
  tools/venv/bin/python tools/progress.py --readme   # also rewrite README.md between the progress markers
  options: --no-report  use the existing build/report.json
"""
import argparse
import datetime
import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REPORT = ROOT / "build/report.json"
README = ROOT / "README.md"
START, END = "<!-- progress:start -->", "<!-- progress:end -->"
WIDTH = 20

# Display names and order of the targets (objdiff unit names are "<target>/<unit>").
TIER1 = ["fieldstg", "fightstg", "cardgame", "cnty_sel", "shocktst", "soundtst", "stagslct", "stcrdabm", "stcrddek",
         "stcrdshp", "stdgname", "stdwtitl", "stfgtrep", "stgdglab", "stgmcard", "stgtrain", "stitshop", "stplnmet",
         "ststatus"]
TIER2 = ["wfightmn", "wfightts"]


def bar(frac: float) -> str:
    full = int(frac * WIDTH)  # floor: a full bar means 100%
    return "█" * full + "░" * (WIDTH - full)


def row(name: str, matched: int, total: int, fmatched: int, ftotal: int) -> str:
    frac = matched / total if total else 0.0
    return f"| {name} | `{bar(frac)}` | {100 * frac:5.1f}% | {fmatched} / {ftotal} | {total:,} |"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--readme", action="store_true")
    ap.add_argument("--no-report", action="store_true")
    args = ap.parse_args()

    if not args.no_report:
        subprocess.run([str(ROOT / "tools/bin/objdiff-cli"), "report", "generate", "-p", ".", "-o", str(REPORT)],
                       cwd=ROOT, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    rep = json.loads(REPORT.read_text())

    per_target = defaultdict(lambda: [0, 0, 0, 0])  # matched code, total code, matched funcs, total funcs
    for u in rep["units"]:
        cats = (u.get("metadata") or {}).get("progress_categories") or []
        if "sdk" in cats:
            continue
        m = u["measures"]
        t = per_target[u["name"].split("/")[0]]
        t[0] += int(m.get("matched_code", 0) or 0)
        t[1] += int(m.get("total_code", 0) or 0)
        t[2] += int(m.get("matched_functions", 0) or 0)
        t[3] += int(m.get("total_functions", 0) or 0)

    def total(names):
        acc = [0, 0, 0, 0]
        for n in names:
            for i, v in enumerate(per_target.get(n, [0, 0, 0, 0])):
                acc[i] += v
        return acc

    stage_targets = sorted(n for n in per_target if n.startswith("wstag"))
    groups = [("**All game code**", list(per_target)), ("EXE (`SLES_039.36`)", ["main"]),
              ("Tier-1 overlays (19)", TIER1), ("Tier-2 battle overlays", TIER2),
              (f"WSTAG stage overlays ({len(stage_targets)})", stage_targets)]
    head = ["| Part | Progress | Matched | Functions | Code bytes |", "|---|---|---:|---:|---:|"]
    lines = head + [row(name, *total(names)) for name, names in groups]
    lines += ["", "<details><summary>Per overlay</summary>", ""] + head
    lines += [row(f"`{n.upper()}`", *per_target[n]) for n in TIER1 + TIER2 if n in per_target]
    lines += ["", "</details>"]

    date = datetime.date.today().isoformat()
    block = "\n".join([START, f"_Code compiled from matching C (objdiff, Psy-Q SDK excluded); updated {date}. "
                       "Every file already rebuilds byte-identical from split assembly._", ""] + lines + [END])
    print(block)
    if args.readme:
        text = README.read_text()
        if START in text:
            text = re.sub(re.escape(START) + ".*?" + re.escape(END), lambda _: block, text, flags=re.S)
        else:
            sys.exit("README.md has no progress markers")
        README.write_text(text)


if __name__ == "__main__":
    main()
