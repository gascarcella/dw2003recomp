#!/usr/bin/env python3
"""Census of the matching workarounds in the game's C, and the gate that keeps the game code all C (issue #57).

  tools/venv/bin/python tools/hacks.py            # the counts, and every rule broken (exit 1 if any)
  tools/venv/bin/python tools/hacks.py --list     # also every marker: file:line, its function, its kind
  tools/venv/bin/python tools/hacks.py --check    # also the counts docs/STATUS.md quotes (CI's `check`, test.sh)
  tools/venv/bin/python tools/hacks.py --update   # rewrite the counts docs/STATUS.md quotes (then review the diff)

Reads only tracked text (src/**/*.c, src/**/*.h, include/**/*.h, config/*.yaml, config/wstag*.txt, docs/STATUS.md):
stdlib only, no build, no disc, well under a second. Comments and strings are told apart from code first, so a marker
named in a note is not a use. The markers it counts (DECISIONS "Match status and forced (FAKE) matches", "LOOP_BLOCK"):
  fake      a `FAKE:` comment (docs/MATCHING.md "Forced (FAKE) matches"); a forced match: a function with one or more
  loop      a use of LOOP_BLOCK(...) or LOOP_BARRIER() (include/common.h), with its evidence class (A1, A2, B, C;
            docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER") or FAKE (a barrier that is part of a forced shape)
  holdout   INCLUDE_ASM in game C: a function still in asm (Psy-Q code is never in src/: config/*.yaml `psyq/` units)
  nonmatch  an #if block on NON_MATCHING (a holdout's WIP C)
  asm       a use of a macro that wraps inline asm (include/port.h's), except Sony's gte_* (include/psyq/gtemac.h)
The rules (each one broken is printed as file:line: problem, and the exit status is 1):
  - INCLUDE_ASM in src/ only for a function in HOLDOUTS below, and a NON_MATCHING block only in a file that has one;
    every HOLDOUTS name still has its INCLUDE_ASM (a matched one leaves the list). INCLUDE_RODATA in src/: never.
  - every `asm`/`hasm` code segment of config/*.yaml is Psy-Q (a `psyq/` unit or PSYQ_ASM), every WSTAG file of
    config/wstag.txt is a C unit (config/wstag_c.txt).
  - a `FAKE:` comment gives its reason (FAKE_MIN_WORDS words at least).
  - a LOOP_BLOCK/LOOP_BARRIER use has, within EVIDENCE_LINES lines above it in its function, a comment naming its
    evidence class (`class A1`, `Class C (...)`) or a `FAKE:` comment.
  - no inline asm (`asm`, `__asm`, `__asm__`) in src/, nor in include/ outside a macro definition; the macros that wrap
    it (include/psyq/ excepted) only in ASM_ALLOWED's functions.
  - no `#if 0` / `#elif 0`.
--check also compares the counts with docs/STATUS.md's bold quotes: `**N forced matches**`, `**N `LOOP_BLOCK`/
`LOOP_BARRIER` uses**` followed by `by evidence class: A1 n, A2 n, ...`, and `**N holdouts**` (must be quoted while
there are any). The idea of a workaround census in CI comes from ReGame-Labs/dw3_decomp's tools/hacks.py (MIT;
docs/THIRD_PARTY.md); this tool is our own.
"""
import argparse
import bisect
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
STATUS = ROOT / "docs/STATUS.md"

# The functions still in asm (INCLUDE_ASM, WIP C under #ifdef NON_MATCHING). Empty since issue #55 matched the last 8:
# any INCLUDE_ASM or NON_MATCHING in game code fails. A function that has to go back to asm is listed here, on purpose.
HOLDOUTS: set[str] = set()
# Functions that may use a macro wrapping inline asm (DECISIONS "Match status and forced (FAKE) matches": accepted).
ASM_ALLOWED = {
    "heap_run_object": "the original's inline asm moves $sp to the scratchpad (include/port.h PORT_SCRATCHPAD_STACK_*)",
}
# Split-asm code segments of config/*.yaml that are Psy-Q without a `psyq/` name.
PSYQ_ASM = {"crt0": "Psy-Q 2MBYTE.OBJ (__main, __SN_ENTRY_POINT)"}
FAKE_MIN_WORDS = 3
EVIDENCE_LINES = 12
CLASSES = ("A1", "A2", "B", "C", "FAKE")

# Strings, character constants and comments in source order (so that a quote in a comment or a /* in a string is not
# taken for the other).
TOKEN = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|/\*.*?\*/|//[^\n]*', re.S)
LOOP_USE = re.compile(r"\bLOOP_(BLOCK|BARRIER)\s*\(")
INCLUDE_ASM = re.compile(r"\bINCLUDE_ASM\s*\(\s*[^,()]*,\s*(\w+)\s*\)")
INCLUDE_RODATA = re.compile(r"\bINCLUDE_RODATA\s*\(")
ASM_KEYWORD = re.compile(r"\b(?:__asm__|__asm|asm)\b")
DIRECTIVE = re.compile(r"^[ \t]*#[ \t]*(\w+)(.*)$")
EVIDENCE = re.compile(r"\bclass\s+(A1|A2|B|C)\b", re.I)
FAKE = re.compile(r"\bFAKE:")
NON_MATCHING = re.compile(r"\bNON_?MATCHING\b")
IF_ZERO = re.compile(r"^(?:el)?if$")
YAML_SEGMENT = re.compile(r"^\s*-\s*\[\s*0x[0-9A-Fa-f]+\s*,\s*(asm|hasm)\s*,\s*([\w/.]+)", re.M)
# The counts docs/STATUS.md quotes: kind -> the bold phrase (N for the number).
DOC = {
    "forced": "**N forced matches**",
    "loop": "**N `LOOP_BLOCK`/`LOOP_BARRIER` uses**",
    "holdout": "**N holdouts**",
}
DOC_RX = {k: re.compile(re.escape(v).replace("N", r"(\d+)", 1)) for k, v in DOC.items()}
DOC_CLASSES = re.compile(r"by evidence class: ((?:[A-Z][A-Z0-9]* \d+(?:, )?)+)")



def tracked(*dirs):
    """The tracked files under dirs: configure.py's generated headers (include/asm_generated/) and other build output
    are not the game's source. Without git (an exported tree), every file but include/asm_generated/."""
    try:
        out = subprocess.run(["git", "-C", str(ROOT), "ls-files", "-z", "--", *dirs], capture_output=True, check=True)
        return [ROOT / f for f in out.stdout.decode().split("\0") if f]
    except (OSError, subprocess.CalledProcessError):
        return [p for d in dirs for p in (ROOT / d).rglob("*") if p.is_file() and "asm_generated" not in p.parts]

class Source:
    """One C file: its code with comments and strings blanked (offsets and lines kept), its comments, its functions."""

    def __init__(self, path):
        self.path = path
        self.rel = path.relative_to(ROOT).as_posix()
        self.text = path.read_text(errors="replace")
        self.newlines = [m.start() for m in re.finditer("\n", self.text)]
        self.comments = []  # (first line, last line, text)
        code = []
        last = 0
        for m in TOKEN.finditer(self.text):
            code.append(self.text[last:m.start()])
            tok = m.group()
            if tok[0] in "/":
                self.comments.append((self.line(m.start()), self.line(m.end() - 1), tok))
            # a string keeps its quotes, a comment becomes spaces; newlines stay
            code.append(tok[0] + re.sub(r"[^\n]", " ", tok[1:-1]) + tok[-1] if tok[0] in "\"'"
                        else re.sub(r"[^\n]", " ", tok))
            last = m.end()
        code.append(self.text[last:])
        self.code = "".join(code)
        self.lines = self.code.split("\n")
        self._funcs = None
        self._directives = None
        self.wip = []  # (first, last line) of the WIP C of NON_MATCHING blocks

    def line(self, offset):
        """The 1-based line of a character offset (code and text share offsets)."""
        return bisect.bisect_left(self.newlines, offset) + 1

    def directives(self):
        """(line, name, rest) of every preprocessor line (continuations joined)."""
        if self._directives is not None:
            return self._directives
        out, i = [], 0
        while i < len(self.lines):
            m = DIRECTIVE.match(self.lines[i])
            first = i
            if m:
                rest = m.group(2)
                while self.lines[i].endswith("\\") and i + 1 < len(self.lines):
                    i += 1
                    rest += " " + self.lines[i]
                out.append((first + 1, m.group(1), rest))
            i += 1
        self._directives = out
        return out

    def functions(self):
        """(first line, last line, name) of every function definition, from the braces at file scope."""
        if self._funcs is None:
            code = "\n".join("" if DIRECTIVE.match(ln) else ln for ln in self.lines)  # same line numbers
            self._funcs, depth, start = [], 0, None
            for m in re.finditer(r"[{}]", code):
                if m.group() == "{":
                    if depth == 0:
                        start = (m.start(), func_name(code, m.start()))
                    depth += 1
                else:
                    depth -= 1
                    if depth == 0 and start[1]:
                        self._funcs.append((self.line(start[0]), self.line(m.start()), start[1]))
                    if depth < 0:
                        raise SystemExit(f"{self.rel}:{self.line(m.start())}: unbalanced braces")
        return self._funcs

    def function_at(self, line):
        for first, last, name in self.functions():
            if first <= line <= last:
                return name, first
        return None, None


def func_name(code, brace):
    """The name of the function whose body opens at `brace`, or None (an initializer, a struct)."""
    i = brace - 1
    while i >= 0 and code[i].isspace():
        i -= 1
    if i < 0 or code[i] != ")":
        return None
    depth = 0
    while i >= 0:
        depth += (code[i] == ")") - (code[i] == "(")
        if depth == 0:
            break
        i -= 1
    m = re.search(r"(\w+)\s*$", code[max(0, i - 200):i])
    return m.group(1) if m and m.group(1) not in ("if", "while", "for", "switch") else None


class Census:
    def __init__(self):
        self.markers = []  # (kind, file, line, function, detail)
        self.problems = []  # (file, line, message)
        self.nonmatch_lines = set()  # (file, line) of each NON_MATCHING #if
        self.holdout_files = set()

    def mark(self, kind, src, line, detail="", func=None):
        if func is None:
            func = src.function_at(line)[0] or "(file scope)"
        if any(a <= line <= b for a, b in src.wip) and kind != "nonmatch":
            detail = f"{detail}, in WIP C" if detail else "in WIP C"
        self.markers.append((kind, src.rel, line, func, detail))

    def problem(self, rel, line, message):
        self.problems.append((rel, line, message))

    def counts(self):
        c = Counter(k for k, *_ in self.markers)
        c["forced"] = len({(f, fn) for k, f, _, fn, _ in self.markers if k == "fake"})
        for k, _, _, _, d in self.markers:
            if k == "loop":
                c["loop " + d.split(",")[0]] += 1  # the class (a WIP use's detail says so after it)
        return c

    # -- the scan --
    def scan(self):
        files = sorted(p for p in tracked("src", "include") if p.suffix in (".c", ".h"))
        asm_macros = {}
        sources = [Source(p) for p in files]
        for s in sources:
            if s.rel.startswith("include/"):
                self.scan_header_asm(s, asm_macros)
        for s in sources:
            self.scan_source(s, asm_macros)
        for name in sorted(HOLDOUTS - {fn for k, _, _, fn, _ in self.markers if k == "holdout"}):
            self.problem("tools/hacks.py", 0, f"HOLDOUTS lists {name}, which has no INCLUDE_ASM: remove it")
        for rel, line in sorted(self.nonmatch_lines):
            if rel not in self.holdout_files:
                self.problem(rel, line, "a NON_MATCHING block in game code (no holdout of HOLDOUTS in this file)")
        self.scan_config()

    def scan_header_asm(self, s, asm_macros):
        """Macros of include/ whose definition holds inline asm; asm outside a #define is a problem."""
        in_define = set()
        for line, name, rest in s.directives():
            if name == "define":
                n = line
                while s.lines[n - 1].endswith("\\"):
                    in_define.add(n)
                    n += 1
                in_define.add(n)
                if ASM_KEYWORD.search(rest):
                    macro = re.match(r"\s*(\w+)", rest).group(1)
                    if not s.rel.startswith("include/psyq/"):  # Sony's gte_* macros: the original's own
                        asm_macros[macro] = s.rel
        for n, text in enumerate(s.lines, 1):
            if n not in in_define and ASM_KEYWORD.search(text):
                self.problem(s.rel, n, "inline asm in a header outside a macro definition")

    def scan_source(self, s, asm_macros):
        game = s.rel.startswith("src/")
        # preprocessor: NON_MATCHING blocks, #if 0
        stack = []  # per open #if: the line where its WIP branch starts, or None (not a NON_MATCHING #ifdef)
        for line, name, rest in s.directives():
            if name in ("if", "ifdef", "ifndef"):
                nm = bool(NON_MATCHING.search(rest))
                stack.append(line if nm and name != "ifndef" else None)
                if nm:
                    self.mark("nonmatch", s, line, f"#{name}{rest}".strip(), self.function_after(s, line))
                    self.nonmatch_lines.add((s.rel, line))
            elif name in ("else", "elif", "endif") and stack:
                if stack[-1] is not None:
                    s.wip.append((stack[-1], line))
                    stack[-1] = None
                if name == "endif":
                    stack.pop()
            if IF_ZERO.match(name) and re.match(r"\s*0\s*$", rest):
                self.problem(s.rel, line, f"#{name} 0: dead code does not belong in the C")
        directive_lines = {ln for ln, *_ in s.directives()}
        # holdouts
        for m in INCLUDE_ASM.finditer(s.code) if "INCLUDE_ASM" in s.code else ():
            line = s.line(m.start())
            if line in directive_lines:
                continue
            func = m.group(1)
            self.mark("holdout", s, line, "INCLUDE_ASM", func)
            self.holdout_files.add(s.rel)
            if not game:
                self.problem(s.rel, line, "INCLUDE_ASM outside src/")
            elif func not in HOLDOUTS:
                self.problem(s.rel, line, f"INCLUDE_ASM of {func}: game code must be C (HOLDOUTS lists the last ones)")
        for m in INCLUDE_RODATA.finditer(s.code) if "INCLUDE_RODATA" in s.code else ():
            line = s.line(m.start())
            if line not in directive_lines:
                self.problem(s.rel, line, "INCLUDE_RODATA: game data must be C")
        # FAKE: comments
        for first, _, text in s.comments:
            m = FAKE.search(text)
            if not m:
                continue
            line = first + text.count("\n", 0, m.start())
            reason = re.sub(r"[\s*/]+", " ", text[m.end():]).strip()
            self.mark("fake", s, line, reason[:60] + ("..." if len(reason) > 60 else ""))
            if len(reason.split()) < FAKE_MIN_WORDS:
                self.problem(s.rel, line, f"FAKE: without its reason (what is forced and why; {FAKE_MIN_WORDS}+ words)")
        # LOOP_BLOCK / LOOP_BARRIER
        for m in LOOP_USE.finditer(s.code) if "LOOP_" in s.code else ():
            line = s.line(m.start())
            if line in directive_lines:
                continue
            func, func_first = s.function_at(line)
            cls = self.evidence(s, line, func_first or 1)
            self.mark("loop", s, line, cls or "none")
            if not cls:
                self.problem(s.rel, line, f"LOOP_{m.group(1)} without an evidence comment naming its class "
                             f"(A1, A2, B, C; docs/MATCHING.md) within {EVIDENCE_LINES} lines above")
        # inline asm
        if game and "asm" in s.code:
            for m in ASM_KEYWORD.finditer(s.code):
                self.problem(s.rel, s.line(m.start()), "inline asm in game code")
        for macro in (m for m in asm_macros if m in s.code):
            for m in re.finditer(r"\b%s\b" % re.escape(macro), s.code):
                line = s.line(m.start())
                if line in directive_lines:
                    continue
                func = s.function_at(line)[0]
                self.mark("asm", s, line, macro)
                if func not in ASM_ALLOWED:
                    self.problem(s.rel, line, f"{macro} (inline asm) outside ASM_ALLOWED's functions")

    def function_after(self, s, line):
        """The function a NON_MATCHING block holds: the first one that starts after its #if."""
        for first, _, name in s.functions():
            if first >= line:
                return name
        return "(file scope)"

    def evidence(self, s, line, func_first):
        best = None
        for first, last, text in s.comments:
            if max(func_first, line - EVIDENCE_LINES) <= last <= line and (best is None or last >= best[0]):
                m = EVIDENCE.search(text)
                if m:
                    best = (last, m.group(1).upper())
                elif FAKE.search(text):
                    best = (last, "FAKE")
        return best[1] if best else None

    def scan_config(self):
        for y in sorted((ROOT / "config").glob("*.yaml")):
            text = y.read_text()
            for m in YAML_SEGMENT.finditer(text):
                name = m.group(2)
                if not name.startswith("psyq/") and name not in PSYQ_ASM:
                    self.problem(y.relative_to(ROOT).as_posix(), text.count("\n", 0, m.start()) + 1,
                                 f"split-asm code segment {name}: game code must be C (Psy-Q: psyq/ or PSYQ_ASM)")
        table = ROOT / "config/wstag.txt"
        c_list = ROOT / "config/wstag_c.txt"
        if table.exists():
            names = [ln.split()[0] for ln in table.read_text().splitlines() if ln.strip() and not ln.startswith("#")]
            c_units = {ln.split("#")[0].split()[0] for ln in c_list.read_text().splitlines()
                       if ln.split("#")[0].strip()} if c_list.exists() else set()
            for n in names:
                if n not in c_units:
                    self.problem("config/wstag_c.txt", 0, f"{n} is not a C unit: game code must be C")


def class_text(counts):
    return ", ".join(f"{c} {counts['loop ' + c]}" for c in CLASSES if counts["loop " + c])


def check_docs(counts):
    """What docs/STATUS.md quotes that differs from the counts (a missing quote too)."""
    text = STATUS.read_text()
    bad = []
    for kind, phrase in DOC.items():
        quoted = [int(m.group(1)) for m in DOC_RX[kind].finditer(text)]
        if not quoted and (kind != "holdout" or counts[kind]):  # no holdouts left: the phrase may go
            bad.append(f"docs/STATUS.md: no `{phrase}` (N = {counts[kind]})")
        bad += [f"docs/STATUS.md: `{phrase.replace('N', str(q), 1)}`, the count is {counts[kind]}"
                for q in quoted if q != counts[kind]]
    m = DOC_CLASSES.search(text)
    if not m and counts["loop"]:
        bad.append(f"docs/STATUS.md: no `by evidence class: ...` ({class_text(counts)})")
    elif m and m.group(1) != class_text(counts):
        bad.append(f"docs/STATUS.md: `by evidence class: {m.group(1)}`, the count is {class_text(counts)}")
    return bad


def update_docs(counts):
    """Rewrite the quoted counts in docs/STATUS.md; True if it changed."""
    text = STATUS.read_text()
    new = text
    for kind, phrase in DOC.items():
        new = DOC_RX[kind].sub(lambda m, p=phrase.replace("N", str(counts[kind]), 1): p, new)
    new = DOC_CLASSES.sub(lambda m: f"by evidence class: {class_text(counts)}", new)
    if new != text:
        STATUS.write_text(new)
    return new != text


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--list", action="store_true", help="every marker with file:line and function")
    ap.add_argument("--check", action="store_true", help="also compare the counts docs/STATUS.md quotes")
    ap.add_argument("--update", action="store_true", help="rewrite the counts docs/STATUS.md quotes")
    args = ap.parse_args()

    c = Census()
    c.scan()
    counts = c.counts()
    if args.list:
        for kind, rel, line, func, detail in sorted(c.markers, key=lambda m: (m[0], m[1], m[2])):
            print(f"{kind:9} {rel}:{line}: {func}" + (f"  [{detail}]" if detail else ""))
        print()
    print(f"forced matches {counts['forced']} ({counts['fake']} FAKE: comments); "
          f"LOOP_BLOCK/LOOP_BARRIER uses {counts['loop']} ({class_text(counts) or 'none'}); "
          f"holdouts {counts['holdout']} (INCLUDE_ASM), NON_MATCHING blocks {counts['nonmatch']}; "
          f"asm macro uses {counts['asm']}")
    bad = [f"{rel}:{line}: {msg}" if line else f"{rel}: {msg}" for rel, line, msg in c.problems]
    if args.update:
        print("docs/STATUS.md:", "counts updated" if update_docs(counts) else "already up to date")
    if args.check:
        bad += check_docs(counts)
    sys.stdout.flush()
    for b in bad:
        print(b, file=sys.stderr)
    if bad:
        print(f"hacks: {len(bad)} problem(s)", file=sys.stderr)
        return 1
    print("hacks: ok" + (" (docs/STATUS.md agrees)" if args.check else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
