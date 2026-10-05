#!/usr/bin/env python3
"""Host-side replay of the layer-1 goldens (B5): the same C compiled for this machine with the port's flags, driven
through the golden cases (tests/golden/<family>.json), every return value and read compared with what the original did
in the emulator. A mismatch is a finding about the C on a 64-bit host (undefined behaviour, a layout difference, a
missing fixture), never a reason to change src/ without proof.

  tests/host/replay.py [families] [--out DIR] [-v]        build (if needed) and replay; exit 1 on any mismatch
  tests/host/replay.py --findings                        print the mismatches as a table and exit 0

What the host cannot replay is skipped and counted: writes to raw addresses (the overlay slot, data files: the code is
linked natively and the files come from extracted/ through cdload entries), and the cdload entry writes themselves,
which become file registrations. A fixture buffer of a struct that holds pointers (CardgameGame, StgtrainSession, ...) is
remapped with the host offsets layout.c reports, and a pointer field that holds another buffer's address gets that
buffer's host address."""
import argparse
import hashlib
import json
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GOLDEN_DIR = ROOT / "tests/golden"
KNOWN = ROOT / "tests/host/known_mismatches.json"   # [{"where": "<family>/<case prefix>", "reason": ...}]: documented findings
OUT_DEFAULT = ROOT / "build/host"
# PS1 struct facts the remap needs: buffer name -> (host layout key of its first field after the pointer-bearing
# prefix, that field's PS1 offset, the other fields to verify shift by the same amount[, pointer fields]). Pointer fields
# ({PS1 offset: (host layout key, target buffer)}) hold the PS1 scratch address of another buffer of the case; the host
# stores that buffer's host address there instead (P command). A target "@<function>" is a method: the PS1 word is 0
# (left 0) or that function's address (config/*.symbols.txt), and the host stores its own function; such a field may
# also lie past the prefix (CardgameGame.get_score).
BUFFER_LAYOUTS = {"game": ("CardgameGame.card_ids", 0x50,
                           {"CardgameGame.cpu_cards": 0x35C, "CardgameGame.selectable": 0x446, "CardgameGame.players": 0x59C,
                            "CardgameGame.slots": 0x72C, "CardgameGame.marked": 0x46F, "CardgameGame.display": 0x498,
                            "CardgameGame.turns": 0x580},
                           {0x820: ("CardgameGame.get_score", "@cardgame_cpu_get_score")}),
                  "train_session": ("StgtrainSession.layer", 0x54, {"StgtrainSession.digimon": 0x5C, "StgtrainSession.bonus": 0xD8},
                                    {0x50: ("StgtrainSession.main", "train_main"), 0x24: ("Object.children", "session_data")}),
                  "train_main": ("StgtrainMain.layer", 0x50, {"StgtrainMain.level": 0x7C}),
                  "rep_member": ("StfgtrepMember.layer_id", 0x54, {"StfgtrepMember.bonus_applied": 0x64}),
                  "items_page": ("StstatusItemsPage.parent", 0x50, {"StstatusItemsPage.item": 0x78, "StstatusItemsPage.member": 0x3B0}),
                  "tech_page": ("StstatusTechPage.parent", 0x50, {"StstatusTechPage.user": 0x7C, "StstatusTechPage.members": 0x8C}),
                  "enemy_turn": ("FightstgEnemyTurn.args", 0x50, {"FightstgEnemyTurn.action": 0x78}),
                  "dglab_main": ("StgdglabMain.layer_id", 0x50, {"StgdglabMain.member_count": 0x60}),
                  "sprite": ("Sprite.vram_x", 0x08, {"Sprite.scale": 0x38, "Sprite.matrix": 0x50},
                             {0x04: ("Sprite.ot_entry", "ot")})}
# Buffers made only of 32-bit pointers: the UI objects a function calls through, stubbed by the goldens (ststatus_items):
# every word holds another buffer's PS1 address or HEAP_NOP's, and the host builds the same words as host pointers (a
# buffer's host address, or a host no-op function). "window" stands for a MessageWindow whose methods all do nothing.
POINTER_BUFFERS = {"window": "sizeof(MessageWindow)", "windows": None, "board": "sizeof(CardgameBoard)", "session_data": None}
HEAP_NOP = 0x80017BDC        # heap_nop (config/symbol_addrs.txt): an empty function in the EXE
# Global arrays of a struct with pointers: symbol -> (PS1 stride, host layout key of the stride, {PS1 field offset: host
# layout key}); a read of one of those fields is moved to the host's element and field, a save covers whole elements.
SYMBOL_ARRAYS = {"stcrdshp_shops": (0xC, "sizeof(StcrdshpShop)", {4: "StcrdshpShop.count"})}
# Buffers named object_*: an Object header (include/object.h) whose words 0x24..0x4F (children, the methods) are pointers:
# 0, another buffer of the case, or an EXE function (by its config/symbol_addrs.txt name: the host's own function).
OBJECT_PREFIX, OBJECT_POINTERS = "object_", (0x24, 0x50)
# Globals with pointer fields that fixtures write (heap_objects.objects, sprite_current, gfx_module.packet): symbol ->
# ({PS1 offset: (host layout key of the field or None for offset 0, number of pointers)}, host size key for a save,
# {PS1 offset: host layout key} for reads of pointer-free fields after a pointer-bearing part). A write to a pointer
# field is split into its words: 0, or the PS1 address of one of the case's buffers (G command: that buffer's host address).
POINTER_GLOBALS = {"heap_objects": ({0: ("HeapObjects.objects", 100)}, "sizeof(HeapObjects)", {0x190: "HeapObjects.filter"}),
                   "sprite_current": ({0: (None, 1)}, "sizeof(pointer)", {}),
                   "gfx_module": ({0x20: ("GfxModule.packet", 1)}, "sizeof(GfxModule)", {})}
# Globals whose tail follows a pointer (pointer-free fields only are written and read there): symbol -> (PS1 offset of the
# first field after the pointer, its host layout key, host size key for a save). A write or read at or past that offset
# moves by the host's shift; the save covers the whole host struct. records_state.gauges follows clear_gauges.
SHIFTED_TAILS = {"records_state": (0x58, "RecordsState.gauges", "sizeof(RecordsState)")}
# Globals the host does not take from a fixture: symbol -> why. heap_funcs.first/end point the oracle's heap at a scratch
# arena (wfightmn_spoils: the fade object wfightmn_battle_end creates); the host's heap is libc's (shims.c).
HOST_SKIPPED_GLOBALS = {"heap_funcs": "heap_funcs.first/end (the oracle's scratch heap arena; the host allocates with libc)"}
SYMBOL_FILE = ROOT / "config/symbol_addrs.txt"
SCRATCH_BASE = 0x80180000    # tests/golden/oracle.py: where the oracle placed the case's buffers, in order, 16-aligned
CDLOAD_ENTRIES = 4           # CdloadModule.entries
CDLOAD_ENTRY_SIZE = 0x10


_FUNCS = {}


_ADDRS = {}


def symbol_address(name):
    """A name's address in config/symbol_addrs.txt or any config/<overlay>.symbols.txt."""
    if not _ADDRS:
        for f in [SYMBOL_FILE] + sorted(SYMBOL_FILE.parent.glob("*.symbols.txt")):
            for line in f.read_text().splitlines():
                m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
                if m:
                    _ADDRS.setdefault(m.group(1), int(m.group(2), 16))
    return _ADDRS[name]


def exe_function(addr):
    """The config/symbol_addrs.txt name of the EXE function at addr, or None."""
    if not _FUNCS:
        for line in SYMBOL_FILE.read_text().splitlines():
            m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);.*type:func", line)
            if m:
                _FUNCS.setdefault(int(m.group(2), 16), m.group(1))
    return _FUNCS.get(addr)


def object_words(data):
    lo, hi = OBJECT_POINTERS
    return struct.unpack_from(f"<{(hi - lo) // 4}I", data, lo)


def golden_symbols(goldens):
    names = {t[1:] for layout in BUFFER_LAYOUTS.values() if len(layout) > 3 for _, t in layout[3].values() if t.startswith("@")}
    for g in goldens:
        for f in g["fixtures"].values():
            names |= {w["symbol"] for w in f if not w["symbol"].startswith("0x")}
        for c in g["cases"]:
            names |= {s for s, _ in c["saves"]}
            for bname, hx in c["buffers"].items():
                if bname.startswith(OBJECT_PREFIX):
                    names |= {exe_function(w) for w in object_words(bytes.fromhex(hx)) if exe_function(w)}
            for call in c["calls"]:
                names.add(call["func"])
                names |= {w["symbol"] for w in call.get("writes", []) if not w["symbol"].startswith("0x")}
                names |= {r["symbol"] for r in call.get("reads", []) if not r["symbol"].startswith("buf:")}
                names |= {a["symbol"] for a in call["args"] if isinstance(a, dict) and "symbol" in a}
    return sorted(names)


def write_symtab(names, path):
    lines = ["/* generated by tests/host/replay.py from the goldens' symbol names */",
             "struct host_symbol { const char *name; void *addr; };"]
    lines += [f"extern char {n}[];" for n in names]
    lines.append("struct host_symbol host_symbols[] = {")
    lines += [f'    {{ "{n}", {n} }},' for n in names]
    lines += ["    { 0, 0 },", "};", ""]
    path.write_text("\n".join(lines))


class Host:
    def __init__(self, binary, verbose=False):
        self.p = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
        self.verbose = verbose
        self.send("L")
        self.offsets = {}
        while True:
            line = self.p.stdout.readline().strip()
            if line == ".":
                break
            _, name, value = line.split(" ", 2)
            self.offsets[name] = int(value)

    def send(self, line):
        if self.verbose:
            print("  >", line[:120])
        self.p.stdin.write(line + "\n")

    def ask(self, line):
        self.send(line)
        ans = self.p.stdout.readline().strip()
        if ans.startswith("E"):
            raise RuntimeError(ans)
        return ans.split(" ", 1)[1] if " " in ans else ""

    def close(self):
        self.send("Q")
        self.p.stdin.close()
        self.p.wait()


def decode(rax, ret_type):
    v = int(rax, 16)
    if ret_type == "void":
        return None
    if ret_type in ("u8", "s8"):
        v &= 0xFF
        return v - 0x100 if ret_type == "s8" and v & 0x80 else v
    if ret_type in ("u16", "s16"):
        v &= 0xFFFF
        return v - 0x10000 if ret_type == "s16" and v & 0x8000 else v
    v &= 0xFFFFFFFF
    return v - (1 << 32) if ret_type == "s32" and v & 0x80000000 else v


def buffer_addresses(buffers):
    """The PS1 addresses the oracle gave a case's buffers (oracle.layout_buffers)."""
    addr, places = SCRATCH_BASE, {}
    for name, hx in buffers.items():
        places[name] = addr
        addr += (max(len(hx) // 2, 1) + 15) & ~15
    return places


def buffer_shift(host, name):
    """(PS1 offset where the remapped part starts, host shift), or None for a buffer laid out alike on both."""
    if name not in BUFFER_LAYOUTS:
        return None
    key, ps1_off, others = BUFFER_LAYOUTS[name][:3]
    shift = host.offsets[key] - ps1_off
    for k, off in others.items():
        if host.offsets[k] - off != shift:
            raise RuntimeError(f"{name}: {k} shifts by {host.offsets[k] - off}, {key} by {shift}")
    return ps1_off, shift


def remap_buffer(host, name, data, places):
    """A PS1-layout fixture buffer, laid out as the host compiled the struct (pointer-bearing prefix wider); returns the
    bytes and the pointer fields to patch [(host offset, target buffer)]."""
    if name not in BUFFER_LAYOUTS:
        return data, []
    ps1_off, shift = buffer_shift(host, name)
    pointers = BUFFER_LAYOUTS[name][3] if len(BUFFER_LAYOUTS[name]) > 3 else {}
    data = bytearray(data)
    patches = []
    for off, (key, target) in pointers.items():
        if off + 4 > len(data):
            continue
        value = struct.unpack_from("<I", data, off)[0]
        if value == 0:          # a pointer the case leaves null
            continue
        if target.startswith("@"):
            if value != symbol_address(target[1:]):
                raise RuntimeError(f"{name}+{off:#x}: {value:#x} is not the address of {target[1:]}")
        elif value != places.get(target):
            raise RuntimeError(f"{name}+{off:#x}: {value:#x} is not the address of buffer {target}")
        data[off:off + 4] = bytes(4)
        patches.append((host.offsets[key], target))
    if data[:ps1_off] != bytes(ps1_off):
        raise RuntimeError(f"{name}: the pointer-bearing prefix is not zero; cannot remap")
    out = bytes(ps1_off + shift) + bytes(data[ps1_off:])
    end = max([off + host.offsets["sizeof(pointer)"] for off, _ in patches] + [len(out)])
    return out + bytes(end - len(out)), patches


def pointer_buffer(host, name, data, places):
    """A POINTER_BUFFERS buffer: host bytes (zeros) and its pointer patches [(host offset, target buffer or @nop)]."""
    names = {a: n for n, a in places.items()}
    words = struct.unpack(f"<{len(data) // 4}I", data)
    size_key = POINTER_BUFFERS[name]
    if size_key and len(words) * 8 < host.offsets[size_key]:
        raise RuntimeError(f"{name}: {len(words) * 8} host bytes, smaller than {size_key}")
    patches = []
    for i, w in enumerate(words):
        if w != HEAP_NOP and w not in names:
            raise RuntimeError(f"{name}+{i * 4:#x}: {w:#x} is neither heap_nop nor a buffer of the case")
        patches.append((i * 8, "@nop" if w == HEAP_NOP else names[w]))
    return bytes(len(words) * 8), patches


def object_buffer(host, data, places):
    """An object_* buffer: the PS1 header's pointer-free head (0..0x24) as is, its pointer words as patches
    [(host offset, target buffer or @function)] at the host's pointer stride; a tail past the header (the type's own
    fields, pointer-free: FieldstgManager's up to map_entry) follows the host's header (read_offset moves its reads)."""
    lo, hi = OBJECT_POINTERS
    if len(data) < hi:
        raise RuntimeError(f"an Object buffer of {len(data):#x} bytes: shorter than the 0x50-byte header")
    names = {a: n for n, a in places.items()}
    patches = []
    for k, w in enumerate(object_words(data)):
        if w == 0:
            continue
        target = names.get(w) or (f"@{exe_function(w)}" if exe_function(w) else None)
        if target is None:
            raise RuntimeError(f"Object +{lo + 4 * k:#x}: {w:#x} is neither a buffer of the case nor an EXE function")
        patches.append((host.offsets["Object.children"] + k * host.offsets["sizeof(pointer)"], target))
    return data[:lo] + bytes(host.offsets["sizeof(Object)"] - lo) + data[hi:], patches


def global_pointer_writes(host, w, places):
    """A write into a POINTER_GLOBALS pointer field: [(host offset, target or None for 0)]."""
    fields = POINTER_GLOBALS[w["symbol"]][0]
    data = bytes.fromhex(w["hex"])
    names = {a: n for n, a in places.items()}
    for off, (key, count) in fields.items():
        if off <= w["offset"] and w["offset"] + len(data) <= off + 4 * count and len(data) % 4 == 0 and (w["offset"] - off) % 4 == 0:
            base = host.offsets[key] if key else 0
            out = []
            for k, v in enumerate(struct.unpack(f"<{len(data) // 4}I", data)):
                if v and v not in names:
                    raise RuntimeError(f"{w['symbol']}+{w['offset'] + 4 * k:#x}: {v:#x} is not a buffer of the case")
                out.append((base + ((w["offset"] - off) // 4 + k) * host.offsets["sizeof(pointer)"], names.get(v)))
            return out
    raise RuntimeError(f"{w['symbol']}+{w['offset']:#x}: a write outside its pointer fields")


def read_offset(host, symbol, offset):
    """A read of a remapped buffer: the PS1 offset moved as the buffer was."""
    if symbol in SYMBOL_ARRAYS:
        stride, size_key, fields = SYMBOL_ARRAYS[symbol]
        k, field_off = divmod(offset, stride)
        if field_off not in fields:
            raise RuntimeError(f"{symbol}+{offset:#x}: not a field the host can find")
        return k * host.offsets[size_key] + host.offsets[fields[field_off]]
    if symbol in POINTER_GLOBALS:
        starts = [o for o in POINTER_GLOBALS[symbol][2] if o <= offset]
        if starts:
            return host.offsets[POINTER_GLOBALS[symbol][2][max(starts)]] + offset - max(starts)
        raise RuntimeError(f"{symbol}+{offset:#x}: not a field the host can find")
    if symbol in SHIFTED_TAILS and offset >= SHIFTED_TAILS[symbol][0]:
        return host.offsets[SHIFTED_TAILS[symbol][1]] + offset - SHIFTED_TAILS[symbol][0]
    if symbol.startswith("buf:" + OBJECT_PREFIX) and offset >= OBJECT_POINTERS[1]:
        return offset - OBJECT_POINTERS[1] + host.offsets["sizeof(Object)"]
    if symbol.startswith("buf:"):
        s = buffer_shift(host, symbol[4:])
        if s and offset >= s[0]:
            return offset + s[1]
    return offset


def apply_writes(host, writes, files, skipped, places=None, deferred=None):
    """Writes of a fixture or a call. Raw addresses are skipped (overlays, data files); a cdload entry write registers
    the file whose raw-address write it points at. A write into a global's pointer field becomes host pointers (G),
    sent at once, or appended to `deferred` when the case's buffers do not exist yet (the fixture)."""
    for w in writes:
        if w["symbol"] in HOST_SKIPPED_GLOBALS:
            skipped.append(HOST_SKIPPED_GLOBALS[w["symbol"]])
            continue
        if w["symbol"] in POINTER_GLOBALS and "hex" in w:
            for off, target in global_pointer_writes(host, w, places or {}):
                cmd = f"G {w['symbol']} {off} {target}" if target else f"W {w['symbol']} {off} {'00' * host.offsets['sizeof(pointer)']}"
                if deferred is not None and target:
                    deferred.append(cmd)
                else:
                    host.send(cmd)
            continue
        if w["symbol"].startswith("0x"):
            if "file" in w:
                files[int(w["symbol"], 16) + w["offset"]] = w["file"]
            skipped.append(w["field"])
            continue
        if w["symbol"] == "cdload_module" and (w["offset"] - CDLOAD_ENTRIES) % CDLOAD_ENTRY_SIZE == 0 and len(w.get("hex", "")) == 32:
            state, _, fid, _, buf = struct.unpack("<hhiiI", bytes.fromhex(w["hex"]))
            if state == 3 and buf in files:
                host.send(f"F {fid:#x} {ROOT / files[buf]}")
                skipped.append(w["field"] + " (registered as a host file)")
                continue
        if "file" in w:
            skipped.append(w["field"])
            continue
        off = w["offset"]
        if w["symbol"] in SHIFTED_TAILS and off >= SHIFTED_TAILS[w["symbol"]][0]:
            off = host.offsets[SHIFTED_TAILS[w["symbol"]][1]] + off - SHIFTED_TAILS[w["symbol"]][0]
        host.send(f"W {w['symbol']} {off} {w['hex']}")


def replay_family(host, g, verbose=False):
    files, skipped, mismatches, calls = {}, [], [], 0
    for case in g["cases"]:
        for sname, size in case["saves"]:
            if sname in SYMBOL_ARRAYS:   # whole elements, at the host's stride
                stride, size_key, _ = SYMBOL_ARRAYS[sname]
                size = -(-size // stride) * host.offsets[size_key]
            if sname in POINTER_GLOBALS:   # the whole host struct
                size = host.offsets[POINTER_GLOBALS[sname][1]]
            if sname in SHIFTED_TAILS:
                size = max(size, host.offsets[SHIFTED_TAILS[sname][2]])
            host.send(f"S {sname} {size}")
        places, patches, deferred = buffer_addresses(case["buffers"]), [], []
        apply_writes(host, g["fixtures"][case["fixture"]], files, skipped, places, deferred)
        for bname, hx in case["buffers"].items():
            if bname in POINTER_BUFFERS:
                data, ptrs = pointer_buffer(host, bname, bytes.fromhex(hx), places)
            elif bname.startswith(OBJECT_PREFIX):
                data, ptrs = object_buffer(host, bytes.fromhex(hx), places)
            else:
                data, ptrs = remap_buffer(host, bname, bytes.fromhex(hx), places)
            host.send(f"B {bname} {data.hex()}")
            patches += [(bname, off, target) for off, target in ptrs]
        for bname, off, target in patches:
            host.send(f"P {bname} {off} {target}")
        for cmd in deferred:
            host.send(cmd)
        for call in case["calls"]:
            apply_writes(host, call.get("writes", []), files, skipped, places)
            args = []
            for a in call["args"]:
                if isinstance(a, int):
                    args.append(f"i{a}")
                elif "buf" in a:
                    args.append(f"b{a['buf']}")
                else:
                    args.append(f"s{a['symbol']}+{a['offset']}")
            args += ["i0"] * (4 - len(args))
            rax = host.ask(f"C {call['func']} {' '.join(args)}")
            calls += 1
            # "trap": the call raised SIGFPE (an x86 division by zero; replay.c recovers and the reads still run)
            got = "trap (SIGFPE)" if rax == "trap" else decode(rax, call["ret_type"])
            where = f"{g['family']}/{case['name']}: {call['func']}"
            if got != call["ret"]:
                mismatches.append((where, "ret", call["ret"], got, call.get("comment", "")))
            for r in call.get("reads", []):
                hx = host.ask(f"D {r['symbol']} {read_offset(host, r['symbol'], r['offset'])} {r['size']}")
                if "sha1" in r:
                    exp, act = r["sha1"], hashlib.sha1(bytes.fromhex(hx)).hexdigest()
                else:
                    exp, act = r["hex"], hx
                if exp != act:
                    mismatches.append((where, r["field"], exp, act, call.get("comment", "")))
        if case.get("keep"):
            pass
        host.send("X")
    return calls, mismatches, sorted(set(skipped))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("families", nargs="*")
    ap.add_argument("--out", default=str(OUT_DEFAULT))
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--findings", action="store_true", help="print the mismatches as a table and exit 0")
    args = ap.parse_args()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    families = args.families or sorted(p.stem for p in GOLDEN_DIR.glob("*.json"))
    goldens = [json.loads((GOLDEN_DIR / f"{f}.json").read_text()) for f in families]
    symtab = out / "symtab.c"
    write_symtab(golden_symbols(goldens), symtab)
    binary = out / "replay"
    subprocess.run([str(ROOT / "tests/host/build.sh"), str(symtab), str(binary)], check=True)
    host = Host(binary, verbose=args.verbose)
    known = json.loads(KNOWN.read_text()) if KNOWN.exists() else []
    print(f"host replay: {' '.join(families)} (gcc -m64; binary {binary.relative_to(ROOT)})")
    status, all_mismatches = 0, []
    for g in goldens:
        calls, mismatches, skipped = replay_family(host, g, args.verbose)
        all_mismatches += mismatches
        new = [m for m in mismatches if not any(m[0].startswith(k["where"]) for k in known)]
        tag = "matches the original" if not mismatches else (
            f"{len(mismatches)} known mismatch(es) (tests/host/FINDINGS.md)" if not new else f"{len(new)} NEW MISMATCH(ES), {len(mismatches) - len(new)} known")
        print(f"  {g['family']}: {len(g['cases'])} cases, {calls} calls: {tag}"
              + (f"; skipped on the host: {len(skipped)} write kind(s)" if skipped else ""))
        if args.verbose and skipped:
            for s in skipped:
                print(f"    skipped: {s}")
        shown = mismatches if args.findings else new[:10]
        for where, what, exp, got, comment in shown:
            print(f"    {where} [{what}]: original {str(exp)[:40]}, host {str(got)[:40]}" + (f"  ({comment})" if comment else ""))
        if new:
            status = 1
    host.close()
    (out / "findings.json").write_text(json.dumps([dict(zip(("where", "what", "original", "host", "comment"), m)) for m in all_mismatches], indent=1))
    return 0 if args.findings else status


if __name__ == "__main__":
    sys.exit(main())
