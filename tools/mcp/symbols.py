"""Symbol lookup for the debug tools: host addresses from the port's ELF (`nm -S`), PS1 addresses from
config/symbol_addrs.txt (the EXE) and config/<overlay>.symbols.txt (the overlays, which share addresses).

Target grammar (`resolve`): "name", "name+0x10" (host address of a global in build/port/dw2003), "0x7f..."/"1234"
(a host address), "ps1:0x8008xxxx" (a PS1 address), "ps1:name", "ps1:name+8" (the PS1 address of an EXE symbol).
"""

from __future__ import annotations

import os
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path

_PS1_LINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(?:\s*//\s*(.*))?")
_NAME_OFF = re.compile(r"^([A-Za-z_]\w*)\s*(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?$")


@dataclass
class HostSymbol:
    name: str
    addr: int
    size: int | None
    kind: str  # nm's letter


@dataclass
class Ps1Symbol:
    name: str
    addr: int
    size: int | None
    type: str | None
    source: str  # "exe" or the overlay name


@dataclass
class Target:
    text: str
    ps1: bool
    addr: int
    name: str | None = None
    offset: int = 0
    size: int | None = None

    def as_dict(self) -> dict:
        d = {"target": self.text, "space": "ps1" if self.ps1 else "host", "addr": hex(self.addr)}
        if self.name:
            d["symbol"] = self.name
            if self.offset:
                d["offset"] = self.offset
            if self.size is not None:
                d["symbol_size"] = self.size
        return d


def parse_int(text: str) -> int:
    text = text.strip()
    return int(text, 16) if text.lower().startswith("0x") else int(text, 10)


class Symbols:
    def __init__(self, root: Path, elf: Path | None = None):
        self.root = Path(root)
        self.elf = Path(elf) if elf else self.root / "build" / "port" / "dw2003"
        self._host: dict[str, HostSymbol] = {}
        self._host_key = None
        self._ps1: dict[str, Ps1Symbol] = {}
        self._ps1_overlays: dict[str, list[Ps1Symbol]] = {}
        self._ps1_key = None

    # ---- host (nm) ----

    def host_table(self) -> dict[str, HostSymbol]:
        """nm -S --defined-only of the ELF, cached on its mtime and size (empty when there is no ELF)."""
        try:
            st = self.elf.stat()
            key = (st.st_mtime_ns, st.st_size, str(self.elf))
        except FileNotFoundError:
            self._host, self._host_key = {}, None
            return self._host
        if key == self._host_key:
            return self._host
        out = subprocess.run(["nm", "-S", "--defined-only", str(self.elf)], capture_output=True, text=True,
                             check=True).stdout
        table: dict[str, HostSymbol] = {}
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 4:
                addr, size, kind, name = parts
                size = int(size, 16)
            elif len(parts) == 3:
                addr, kind, name = parts
                size = None
            else:
                continue
            sym = HostSymbol(name, int(addr, 16), size, kind)
            # Prefer data symbols over a same-named local of another kind; first definition otherwise.
            if name not in table or (table[name].kind.islower() and kind.isupper()):
                table[name] = sym
        self._host, self._host_key = table, key
        return table

    def host(self, name: str) -> HostSymbol | None:
        return self.host_table().get(name)

    # ---- PS1 (config) ----

    def ps1_table(self) -> dict[str, Ps1Symbol]:
        files = sorted(self.root.glob("config/*.symbols.txt"))
        main = self.root / "config" / "symbol_addrs.txt"
        key = tuple((str(p), p.stat().st_mtime_ns) for p in [main] + files if p.exists())
        if key == self._ps1_key:
            return self._ps1
        self._ps1 = self._parse_ps1(main, "exe") if main.exists() else {}
        self._ps1_overlays = {}
        for p in files:
            ov = p.name[: -len(".symbols.txt")]
            for sym in self._parse_ps1(p, ov).values():
                self._ps1_overlays.setdefault(sym.name, []).append(sym)
        self._ps1_key = key
        return self._ps1

    @staticmethod
    def _parse_ps1(path: Path, source: str) -> dict[str, Ps1Symbol]:
        table: dict[str, Ps1Symbol] = {}
        with open(path, encoding="utf-8") as f:
            for line in f:
                m = _PS1_LINE.match(line)
                if not m:
                    continue
                name, addr, comment = m.group(1), int(m.group(2), 16), m.group(3) or ""
                attrs = dict(kv.split(":", 1) for kv in comment.split() if ":" in kv)
                size = int(attrs["size"], 0) if "size" in attrs else None
                table.setdefault(name, Ps1Symbol(name, addr, size, attrs.get("type"), source))
        return table

    def ps1(self, name: str) -> Ps1Symbol | None:
        return self.ps1_table().get(name)

    def ps1_overlay(self, name: str) -> list[Ps1Symbol]:
        self.ps1_table()
        return self._ps1_overlays.get(name, [])

    # ---- combined ----

    def lookup(self, name: str) -> dict:
        """Everything known about `name`: host addr/size (if the ELF defines it), the EXE's PS1 addr/size/type, and
        the overlays that define it."""
        d: dict = {"name": name}
        h = self.host(name)
        if h:
            d["host"] = {"addr": hex(h.addr), "size": h.size, "kind": h.kind}
        p = self.ps1(name)
        if p:
            d["ps1"] = {"addr": hex(p.addr), "size": p.size, "type": p.type}
        ovs = self.ps1_overlay(name)
        if ovs:
            d["ps1_overlays"] = [{"overlay": o.source, "addr": hex(o.addr), "size": o.size, "type": o.type} for o in ovs]
        if not h and not p and not ovs:
            d["found"] = False
        return d

    def search(self, pattern: str, limit: int = 50) -> list[dict]:
        """Names matching a regex (case-insensitive; a plain substring works), host and PS1 tables merged."""
        rx = re.compile(pattern, re.IGNORECASE)
        names = set(n for n in self.host_table() if rx.search(n)) | set(n for n in self.ps1_table() if rx.search(n))
        self.ps1_table()
        names |= set(n for n in self._ps1_overlays if rx.search(n))
        out = []
        for n in sorted(names):
            e = {"name": n}
            h, p = self.host(n), self.ps1(n)
            if h:
                e["host"] = hex(h.addr)
                if h.size is not None:
                    e["size"] = h.size
            if p:
                e["ps1"] = hex(p.addr)
            ovs = self.ps1_overlay(n)
            if ovs:
                e["overlays"] = sorted(set(o.source for o in ovs))
            out.append(e)
            if len(out) >= limit:
                break
        return out

    def resolve(self, target: str) -> Target:
        """The target grammar (module docstring) -> a Target with a host or a PS1 address."""
        text = str(target).strip()
        ps1 = False
        body = text
        if body.lower().startswith("ps1:"):
            ps1, body = True, body[4:].strip()
        try:
            return Target(text, ps1, parse_int(body))
        except ValueError:
            pass
        m = _NAME_OFF.match(body)
        if not m:
            raise ValueError(f"bad target {text!r}: use name, name+0x10, 0x..., or ps1:0x... / ps1:name")
        name, off = m.group(1), parse_int(m.group(2)) if m.group(2) else 0
        if ps1:
            p = self.ps1(name)
            if p is None:
                ovs = self.ps1_overlay(name)
                if len(ovs) == 1:
                    p = ovs[0]
                elif ovs:
                    raise ValueError(f"{name} is defined in several overlays ({', '.join(o.source for o in ovs)}): "
                                     "give the PS1 address")
                else:
                    raise ValueError(f"no PS1 symbol {name!r} in config/symbol_addrs.txt")
            return Target(text, True, p.addr + off, name, off, p.size)
        h = self.host(name)
        if h is None:
            if not self.elf.exists():
                raise ValueError(f"no ELF at {self.elf}: build the port first (game_start build=True)")
            raise ValueError(f"no symbol {name!r} in {self.elf} (symbol_search finds names)")
        return Target(text, False, h.addr + off, name, off, h.size)
