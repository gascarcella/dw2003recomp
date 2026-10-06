# tests/saves: save round trips between the port and the emulator

Layer 3's save check (`tests/README.md`; `docs/PC_PORT_PLAN.md` M4): a save the PC port writes must load in the emulator,
a save the emulator writes must load in the port, and the two cards must be the same bytes but for what timing and stale
RAM put there.

```sh
tests/saves/run.py                     # ~1 min: two saves in parallel, then two loads in parallel, then the cards
tests/saves/run.py --keep              # keep each run's log, record and checkpoint dumps (build/saves-test/)
tests/saves/run.py --card-emulator X.mcd [--card-port Y.mcd]   # use a given card instead of that side's save run
tests/saves/cards.py A.mcd [B.mcd]     # the format checks of one card, or of two against each other
scripts/test.sh --layer 3              # the formats, then this
```
It needs the disc, PCSX-Redux (`scripts/setup.sh redux`), and what the port needs (host gcc, CMake, Ninja: `tests/port`);
exit 2 when one is missing (`test.sh` skips it then). Nothing is committed but the code: the cards are made by the run
(`build/saves-test/port.mcd`, `emulator.mcd`) and stay there.

## The runs
No script of its own: `run.py` cuts layer 2's `tests/replay/scripts/first_battle_save.json` at run time, by checkpoint
and step type, so a step appended to that script does not move the cuts.
- **save**: from boot to the `saved` checkpoint (the registration, the first battle, the walk to the Asuka Inn, STGMCARD's
  save to card 1, slot 1) and on to the next wait for FIELDSTG (the "Saved." window closed, the card complete);
  ~27,000 frames in the emulator, ~24,500 in the port (16 s alone); the whole test takes ~52 s, the emulator's save
  most of it.
- **load**: the steps after the script's `reset` up to `loaded` (the boot, the title, Continue, STGMCARD's load, the
  inn), started from a fresh boot with the card in slot 1; ~3,600 frames.

| Run | Card in slot 1 | Must reach |
|---|---|---|
| the port saves (`--memcard1 port.mcd`, a new card) | the port's new formatted card | `saved` with `first_battle_save`'s expected stable hash |
| the emulator saves (`-memcard1 emulator.mcd`, OpenBIOS) | PCSX-Redux's new card | the same |
| the emulator loads | a copy of `port.mcd` | `loaded` with `first_battle_save`'s expected stable hash |
| the port loads | a copy of `emulator.mcd` | the same |

The reference hashes are `tests/replay/expected/first_battle_save.json`'s (`saved`, `loaded`): there the emulator saves,
resets and loads its own card in one run, so the round trips must end in the very state the emulator's own round trip
ends in (the stable hash zeroes the play time, the encounter timer and the slot checksum: tests/README.md "The stable
hash and the RNG"). The emulator runs through `replay.py`'s step table and `run.lua`, with the card given instead of
`run_once`'s fresh one; the port runs as `tests/port/run.py` builds it (`build/port/dw2003`).
Each load gets a copy: PCSX-Redux's BIOS writes directory frame 63 even when the game only reads (the port does not), and
the format checks compare the cards as the saves left them.

## The card checks (`cards.py`)
Each card: 128 KB; directory frame 0 starts `MC`; every directory frame 0..62 has its XOR checksum (byte 0x7F); the
directory holds exactly one file, `BESLES-03936DMW3-EUR`, 0x8000 bytes in four contiguous blocks with states
`51 52 52 53` and the next-block chain, every other entry free (`A0`); the file's Sony header is `SC`, type 0x13 (three
icon frames), 4 blocks; the game's header has version 4, `DMW3` and the slot saved last; that slot has version 4; both
of the game's checksums are exact (`memcard_get_checksum`, the XOR of bytes 4..end: STGMCARD's own check is lenient,
`sum & ~stored == 0`, so a bit cleared in a slot still loads; the test does not rely on it).

The pair: every byte of the two cards equal (directory, Sony header, icons, the game's header and slot, the two unused
slots and the file's unused end, blocks 5-15) except these, each masked by name (`cards.py` `masks()`):

| Field | Card bytes (file at block 1, slot 0) | Why it may differ |
|---|---|---|
| directory frame 63 | `0x1F80..0x1FFF` | the emulator's BIOS writes a buffer of its own there when it clears a new card's flag (OpenBIOS: no valid checksum); the port leaves the formatted card's copy of frame 0 |
| header checksum | `0x2200` | the XOR of the header, which holds the play time |
| header `slots[0].playtime` | `0x222C..0x2237` | the play time at the save (`StgmcardPlayInfo`: frames since boot, which follow the CD and movie timing) |
| header tail | `0x22D4..0x22FF` | the header is written in 0x80-byte calls from a 0x180-byte heap buffer that is never cleared: stale RAM, never read back |
| slot checksum | `0x2300` | the XOR of the slot, which holds the two timers below |
| slot `encounter_timer` | `0x2330..0x2333` | steps every 8th moving frame (`replay.py` `VOLATILE_RANGES`) |
| slot `playtime_frames`, `playtime[4]` | `0x2348..0x2353` | the play time (`VOLATILE_RANGES`) |
| slot tail | `0x49C4..0x49FF` | written from a 0x2780-byte heap buffer, never cleared: stale RAM |

The slot's masks are `VOLATILE_RANGES` inside the saved 0x26C4 bytes (an assertion keeps them in step). In the session-16
run every masked field differs (frame 63 in 5 bytes, the play time in 3, the encounter timer in 1, the tails in 15 and
60) and nothing else does. The tails are host memory in the port: its ASan build's card differs from the plain build's
in the header tail alone (15 bytes), so the port's card is byte-identical run to run, not build to build.

## What a failure looks like
A corrupted byte fails with the field it is in (scratch copies of the cards):
- a money bit flipped in the emulator's slot (`+0x6C`): the slot checksum is wrong, the pair differs at "slot 0 +0x6c",
  and the port's load times out (STGMCARD refuses the slot: "step 8 (press until wait_stage) timed out");
- bit 0 of a slot byte set in the port's card: the checksum is wrong, the pair differs, and the emulator *loads* it
  (the lenient check) into a state with another stable hash: "checkpoint loaded: stable hash 6a47ca8b..., expected
  b2168cbd...";
- a directory byte (the file size): frame 1's checksum, the size, and the pair.
