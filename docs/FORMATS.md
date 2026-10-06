# File formats

Status: **read off the matching C and checked against the disc** (2026-10-04, session 7). Each section names the C
functions that read the format. "Verified" means a script parsed every file of that kind on the user's disc the way
the C does, with no mismatch. The scripts are listed in the last section. Field names in the tables are ours and
describe the C's use; where the C still says `unk_XX`, the table gives the source name as well.

The overlays' code and the memory map are in `docs/DISC_LAYOUT.md`. This file covers the data the code reads.

## Conventions
- Little-endian. A sector is 2048 bytes (`.STR`: 2336). Offsets are in bytes from the start of the enclosing block.
- **File ID**: the game names no file by its path. It uses an index into the EXE's file table (below).
  `tools/disc_files.py` prints the ID → path map.
- **Sub-file ID**: `(file ID << 16) | index`. `cdload_get_subfile_by_id(id)` returns sub-file `index` of the
  loaded file `id >> 16` (see Containers). Many tables in code and data store sub-file IDs. Some store only the
  index, and the code ORs in the file's high half (`fightstg_model_new`, `fieldstg_actor_update_sprite`).
- **Localised file ID**: the text files exist in 7 languages, each language at consecutive IDs. Code adds
  `records_language` to the Japanese file's ID: 0 JPN (`M` prefix), 1 USA (`U`), 2 ENG (`E`, the EU default set
  in `records.c`), 3 FRA (`F`), 4 ITA (`I`), 5 GER (`D`), 6 SPN (`S`). Sub-file IDs add `records_language << 16`.

## Disc access: the file table and cdload
No archive file exists. Every file under `AAA/` (2,382 of them, from `AAA/DAT/NONE____.BIN` = ID 0) is a separate
ISO 9660 file. The directory records hide it, so only the path table reaches it (`docs/DISC_LAYOUT.md`). The
game reads raw sectors by LBA from its own table:

| EXE symbol | Type | Contents | Read by |
|---|---|---|---|
| `filetable_lba` (`0x80044F6C`) | `u32[2382]` | first sector (LBA) of each file | `filetable_get_lba`, `filetable_get_cdloc` |
| `filetable_sectors` (`0x800474A4`) | `u16[2382]` | size in sectors, rounded up | `filetable_get_sectors` |
| `records_file_count` (`0x8005CCA4`) | `s32` | 2382, the number of files | (no reference found) |

The IDs follow the original build's file list, not the disc's directory order. Files of one kind are mostly
consecutive: IDs 1–0x15E are the 50 text files × 7 languages, and the overlays, sounds and data come after them.
All 2,382 LBAs match the ISO directories, each `AAA/` file once, with sectors = size rounded up and zero bytes
after each file's end (`tools/disc_files.py --check`, layer 3 of the tests).

**Reading (cdload.c).** `cdload_read(id, offset, sectors, buf, done)` reads `sectors` sectors (0 = the whole file),
starting `offset` sectors into the file. It reads with CdlSetmode `0xA0` (double speed, 2340-byte sectors), and
`cdload_check_sector` compares each sector's header with the expected sector number before
`cdload_ready_callback` copies its 2048 data bytes. A file is therefore loaded whole, in whole sectors: the buffer also holds the
rest of the last sector (zeros on this disc). The cache above it (`cdload_module`, 64 `CdloadEntry`) keeps whole files in
heap buffers:

| `CdloadEntry` | Size | Meaning |
|---|---|---|
| `0x0` `state` | s16 | state: 0 free, 1 queued (`cdload_queue_file`), 2 reading (`cdload_update`), 3 loaded |
| `0x2` `marked` | s16 | marked by `cdload_mark_loaded`; `cdload_age_marked` makes marked entries the oldest |
| `0x4` `id` | s32 | file ID (0: entry free) |
| `0x8` `last_use` | s32 | last use (vsync count, `gfx_module.funcs.get_time`); `cdload_find_oldest_entry` frees the least recent, the largest on a tie |
| `0xC` `buffer` | ptr | heap buffer, `sectors << 11` bytes |

`cdload_get_file(id)` returns the buffer, and loads the file first (blocking) if it isn't there. Streaming code
(the field background, below) calls `cdload_reader.read = cdload_read` itself, with sector offsets.

## Containers
Almost every data file, and many sub-files, is one of these four shapes.

**1. Offset table** (`cdload_get_subfile(index, data)`: returns `data + data[index]`). The block starts with
`n` s32 offsets, relative to the block's start. The first offset is the header size, so `n = data[0] / 4`. Nothing
stores `n`, and the code never checks an index against it. An offset of 0 means an empty slot (it points back at
the table). Sub-files are often containers themselves: a model's mesh is a sub-file of a sub-file.

**2. Zero-terminated offset list** (`tim_load_all`): the same layout, but read until an offset word of 0
(`entry == data`). Used for lists of TIM images. A list whose first offset is its header size can be read as
either shape.

**3. Counted string table** (`font_get_entry(data, i)`, the `Font.get_entry` entry): s32 `count`, then `count` s32
offsets relative to the table, then the strings. `font_get_entry` returns `data + offsets[i + 1]` for
`0 <= i <= count`. It accepts `i == count`, one past the last entry, which reads the first string's bytes as an
offset. Entry 0 is the empty string in all 3,005 text tables on the disc.

**4. RLEN compression** (`tim_load_all`, `message_unpack_rlen`): a header `"RLEN"`, then an s32 unpacked size, then
a byte stream. A byte `n` below 0x80 is followed by `n` literal bytes. A byte `n` of 0x80 or more is followed by
one byte, repeated `n & 0x7F` times. A 0 byte ends the stream. `tim_load_all` unpacks into a buffer of
`Tim.buffer_size` bytes (0xA800 by default). `MessageRlen` (`message_rlen_unpack_start`) unpacks in steps of
`unk_70` bytes per frame. Where the data has no `"RLEN"` magic, `message_rlen_get_data` returns it as is.

## Code overlays (`AAA/PRO/*.PRO`)
The overlays are raw code and data with no header. The full layout is in `docs/DISC_LAYOUT.md`.
- **Tier 1** (19 files, at `0x80082CB0`): `overlay_load_stage` copies the stage's file (`overlay_files[stage]`,
  indexed by `gamestate_data.funcs.get_map() >> 8`) to `0x80082CB0`. `overlay_run_object` then calls
  `overlay_entries[stage]`, a fixed address inside the overlay.
- **Tier 2** (`WFIGHTMN`, `WFIGHTTS`, 293 `WSTAG###`, at `0x800A5DE0`): `overlay_load_file(id)` copies a file
  already loaded by cdload. The parent overlay calls its entry: a fixed address for WFIGHT*, or the entry of
  FIELDSTG's stage tables for WSTAG (`fieldstg_find_stage`; `config/wstag.txt`).
- **Data-only**: `SFSTDATA` (ID `0x1CB`, battle stages), `SMDLDATA` (`0x1CC`, battle models), and `SDIGIEDT`
  (`0x1CF`, enemy Digimon) are plain data that FIGHTSTG reads with `cdload_get_file` (see Battle). `WSTAG260` is a
  WSTAG with no code that no table names.

## Images
### TIM (`tim.c`)
Sony's TIM format: magic `0x10`, flags (bits 0–2 pixel mode, bit 3 CLUT present), then the optional CLUT block and
the image block. Each block is `{u32 size, u16 x, y, w, h, data}`. `tim_load` uploads the CLUT only for 4- and
8-bit images, and only with a CLUT. It ignores both blocks' own VRAM coordinates and uses the `Tim` object's
positions instead (`tim_set_image_pos` → `image_x/y`, `tim_set_clut_pos` → `clut_x/y`). It records the image
block's `w, h` in `width/height`, in VRAM halfwords, so a 4-bit image is `w * 4` pixels wide (`frame_w` in FIELDSTG).
`tim_load_all(list)` loads every TIM of a zero-terminated list (shape 2, entries plain or RLEN). Each image goes
`0x40` halfwords right of the last, and all share the one CLUT position. The callers choose the positions, for
example the font at `(0x140, 0)` (`message_load_font`).

Every file that starts with the TIM magic turned out to be something else. `TRAINING/STTRNGCS.BIN`'s first word
`0x10` is an offset table's header size (4 sub-files, two of them empty). The only standalone TIM is the
320×480 16-bit picture inside the EXE (`0x800A5DE0`).

### Sprite banks (`sprite_draw`)
A sprite bank holds frames for `sprite_draw(bank, id, x, y)`. It is usually sub-file 0 of a `*CS.BIN` file, which
pairs with a `*TM.BIN` holding its textures (a TIM list).

| Offset | Type | Meaning |
|---|---|---|
| `0x0` | s32 | offset of the cell table |
| `0x4` | s32 | offset of the frame-ID table (u8 per frame; `sprite_draw` searches it for `id` with no end check) |
| `0x8 + 4i` | s32 | offset of frame `i` |

A **cell** (`SpriteCell`, 0xE bytes) is `{s16 u; u8 v; u8 pad; s16 w, h; s16 clut_x, clut_y; s16 is_8bit}`.
`u` is in texture pixels. The texture page is `sprite_current.vram_x + u / 4` (4-bit) or `+ u / 2` (8-bit), and
the CLUT offset is added to the sprite object's CLUT position. A **frame** is `{s16 count; s16 clut_y; s16 abr}`
(blend mode, -1 opaque) followed by `count` × `{s16 cell, x, y}`. The cells are drawn last to first: as SPRTs, or
as POLY_FT4s through the object's rotation/scale matrix. The SPRT path's packets (the frame search by list
position, `u & 0x7F` for 8-bit cells, the 8-bit CLUT base, a DR_TPAGE at each page change and one at the end) are
pinned by the layer-1 family `sprite_draw` (`tests/golden/families/sprite_draw.py`).

Checked structurally (frame lists and cell indices): 17 banks in 12 `*CS` files, FIELDCOM's two and 269 of the
276 non-empty stage `TMPK` sub-files 0 parse this way. The single banks of `CMNBGDCS`, `STCDSPCS` and `STMCRDCS`
and `STTRNGCS`'s sub-file 3 fail the check, and the reason is not known yet.

### Font (`font.c`, `message.c`)
- Glyph images: `COUNTRY/CMFONTTM.BIN` (ID `0x287`). Its sub-file 0 is a TIM list (2 RLEN-packed TIMs) that
  `message_load_font` loads at `(0x140, 0)`. `COUNTRY/CMFONTCS.BIN` (`0x286`) is the message windows' sprite bank.
- Glyph metrics are in the EXE, not on disc: three fonts (`message_fonts[1..3]`, heights 14/11/9), each with 230
  glyphs for codes 4–0xE9 and 114 extra glyphs (`FontGlyph`, 0xB bytes: tpage x/64, u, v, CLUT x, CLUT y, w, h,
  x offset, y offset, advance, ?).
- Character tables in the EXE: `font_chars` (`0x8004E010`) and `font_chars_ext` (`0x8004E3BC`), `{u16 sjis; u8
  glyph; u8 pad}`, ended by `0xFFFF`. They map the game's glyph codes to Shift-JIS (next section).

## Text (`COUNTRY/<LANG>/?S*.BIN`)
**Files.** There are 50 files per language. 39 are counted string tables (shape 3). The 11 `?SDMG###` files are
offset tables (shape 1) of counted string tables: the event texts, addressed by sub-file ID. The USA copies of
`TALK08`, `TALK09` and `DMG900` are 512 zero bytes: the US release has no post-game. "Users" lists code that names
the file as `records_language + ID` (`CARDGAME` uses decimal IDs 15/22).

| File | JPN ID | Shape | Entries (ENG) | Users |
|---|---|---|---|---|
| `AMTMAP`, `ASKMAP` | `0x001`, `0x008` | table | 47, 47 | (not found) |
| `CARDGM` | `0x00F` | table | 71 | CARDGAME |
| `CARDNM` | `0x016` | table | 317 (card names, 1-based) | STCRDABM, STCRDDEK, STCRDSHP, CARDGAME |
| `CARDST` | `0x01D` | table | 242 | STCRD* |
| `CRDABM`, `CRDDEK`, `CRDSHP` | `0x024`, `0x02B`, `0x032` | table | 9, 86, 32 | the card screens, gamestate |
| `DGLABO`, `DGNMET` | `0x039`, `0x040` | table | 45, 4 | STGDGLAB, STDGNAME |
| `DIGINF`, `DIGNAM` | `0x047`, `0x04E` | table | 53, 251 (Digimon names) | STGDGLAB, STSTATUS, FIGHTSTG, WFIGHT*, STFGTREP, ... |
| `FGTRPT`, `HTLNAM` | `0x055`, `0x05C` | table | 12, 22 | STFGTREP; FIELDSTG, inn |
| `ITMINF`, `ITMNAM` | `0x063`, `0x06A` | table | 403, 403 (as `records_items`) | STITSHOP, STSTATUS, FIGHTSTG, WFIGHTMN, ... |
| `ITSHOP`, `MEMCRD` | `0x071`, `0x078` | table | 43, 41 | STITSHOP; STGMCARD (save title = entry 0x27) |
| `MFIGHT` | `0x07F` | table | 145 | FIGHTSTG, WFIGHTMN |
| `NAMEDT`, `NAMEET`, `SHPNAM` | `0x086`, `0x08D`, `0x094` | table | 19, 31, 68 | name entry, shops |
| `SKLINF`, `SKLNAM` | `0x09B`, `0x0A2` | table | 220, 444 (techniques, 1-based) | FIGHTSTG, STGDGLAB, STSTATUS, ... |
| `STAREA`, `STATUS`, `STNAME` | `0x0A9`, `0x0B0`, `0x0B7` | table | 22, 108, 136 | FIELDSTG (map titles), fieldmenu, STSTATUS |
| `SYSTEM` | `0x0BE` | table | 11 | (not found) |
| `TALK00`–`TALK09` | `0x0C5` + 7k | table | 138–1194 | `fieldstg_stage.talk_file` (set by each WSTAG setup) |
| `TRAINI` | `0x10B` | table | 118 | STGTRAIN |
| `DMG200` … `DMG900` | `0x112` + 7k | container | 7–66 tables | `FieldstgEventDef.text` sub-file IDs (WSTAG data) |

**Encoding.** File text is the game's 1-byte glyph code. `message_set_line_text` clears `MessageLine.is_sjis`, and
`message_decode_char` reads the text in that mode:

| Bytes | Meaning |
|---|---|
| `00` | end of the string |
| `01 xx` | extra glyph `xx` (1–0x72): `font_chars_ext`, the font's second glyph array |
| `02 c …` | control code `c`; total length `message_code_lengths[c]` = {1, 2, 3, 2, 5, 3, 3, 2, 3, 2} |
| `03 xx` | decoded as type 3 (drawn as glyph 4); never occurs in the files |
| `04`–`E9` | a glyph: `font_chars` maps it to Shift-JIS. 4–0xD are `０`–`９`, 0xE–0x27 `Ａ`–`Ｚ`, 0x28–0x41 `ａ`–`ｚ`, then hiragana, katakana, `・？！ー～` |
| `EA`–`FF` | out of range: type 3 |

Control codes (`message_control_handlers`; the counts are from all text files of all languages):

| Code | Length | Handler | Meaning | Seen |
|---|---|---|---|---|
| 0 | 1 | `message_code_end` | end of the page | never |
| 1 | 2 | `message_code_newline` | new line; the page ends after `page_lines` lines | yes |
| 2 | 3 | `message_code_wait_button` | wait for a button; the argument (1–4) selects `message_wait_buttons` | yes |
| 3 | 2 | `message_code_instant` | show the rest of the page at once | yes |
| 4 | 5 | `message_code_nop` | nothing (3 argument bytes) | never |
| 5 | 3 | `message_code_insert_line` | insert text line `n` of the window (`message_set_ext_line_text`) | yes |
| 6 | 3 | `message_code_pause` | pause `n` frames | never |
| 7 | 2 | `message_code_end` | page break: the next page starts after it | yes |
| 8 | 3 | `message_code_player_name` | put the player's name (`gamestate_data.name`) in line `n` | yes |
| 9 | 2 | `message_code_end` | end of the page | yes |

Text built in code (`message_copy_text` with literals, names typed by the player) is plain Shift-JIS, decoded with
`is_sjis` set: two bytes per character, lead byte first, `\n` as new line. The character tables hold each code as a
u16 number (`0x824F` for `０`), so in memory the bytes are swapped relative to text order. This is the
"byte-swapped Shift-JIS": `font_convert_text` writes `SWAP16(code)` as a u16 so that the bytes come out in text
order, and `message_decode_char` builds `text[0] << 8 | text[1]` before comparing. `font_convert_text` converts in
both directions (mode 0 glyph codes → Shift-JIS, mode 1 back). STGMCARD uses it for the save title.

**European letters.** The extra glyphs `01 3B`–`01 72` are kanji in the tables (`亜`, `唖`, …, the first JIS
level-1 kanji). The European fonts draw accented letters on them. From the words they occur in: `3B ¡`, `3F ¿`,
`41 Ä`, `45 Ñ`, `47 Ö`, `49 Ü`, `4A ß`, `4B à`, `4C â`, `4E ä`, `4F ç`, `50 è`, `51 é`, `52 ê`, `54 ì`, `55 î`,
`56 ï`, `57 ñ`, `58 ò`, `59 ô`, `5B ö`, `5C ù`, `5D û`, `5E ü`, `5F á`, `60 í`, `61 ó`, `62 ú`, `69 É`, `6A Í`, `6B Ó`,
`6C Ú`, `72 º`. This is a reading from context, not yet checked against the font image. `6D`–`6F` and a few others
occur once or twice and are unassigned. `tools/dump_text.py` uses this table.

Verified: all 91,059 entries of all 350 text files decode with no unknown byte (`tools/dump_text.py --check`).

## Sound (`sound.c`; `SOUND/<NAME>/MP<NAME>.BIN`, `MV<NAME>.BIN`)
A **sound bank** is a pair of files. `sound_banks[id]` (72 entries, 0 = none) points to a `SoundBank` in the EXE:

| `SoundBank` | Type | Meaning |
|---|---|---|
| `0x00` `body_file` | s32 | file ID of the body file `MV*` |
| `0x04` `header_file` | s32 | file ID of the header file `MP*` |
| `0x08` `vab_header` | s32 | sub-file ID: the VAB header (in `MP*`) |
| `0x0C` `vab_body` | s32 | sub-file ID: the VAB body (in `MV*`) |
| `0x10` `seps` | s32[] | sub-file IDs of the SEPs (in `MP*`), 0-terminated (bank 1 has two) |

- `MP*.BIN`: an offset table. Usually sub-file 0 is the VAB header (`pBAV`, version 7) and sub-file 1 a SEP (`pQES`).
  `TTLBGM` has them the other way round, and `COMMON` has two SEPs. `sound_update_loading` copies the whole file
  into the entry's buffer (`sound_buffers`: 0xE000 bytes for entry 0, 0xA000 for entries 1 and 2), then calls
  `SsVabOpenHeadSticky` with the SPU address `sound_spu_addrs[entry]` and `SsSepOpen` per SEP.
- `MV*.BIN`: an offset table with one sub-file, the VAB body (the VAG samples, `SsVabTransBody`).
- Three banks can be loaded at once. Entry 0 holds bank 1 (`COMMON`, loaded by `sound_init`). Entries 1 and 2
  alternate (`sound_load_extra_bank`).
- A **sound key** (`sound_play`) packs: bits 18–24 the bank ID, bit 31 a note (`SsUtKeyOn`: prog bits 11–17,
  tone 7–10, note 0–6), otherwise a SEP sequence (SEP index bits 8–15, sequence 0–7). Bit 30 makes it the
  "current" sound, which replaces the previous one.

Verified: all 71 banks. Each VAB header's size field equals `0x20 + 0x800 + programs × 0x200 + 0x200 + Σ VAG
sizes × 8`, each body file holds that many sample bytes (padded to the sector), and every SEP sub-file has the
`pQES` magic. The 71 banks are exactly the 71 `SOUND/` directories.

## Game records in the EXE (`records.c`, `gamestate.c`)
The Digimon, item and technique tables are EXE `.data`, not disc files, and are in C in `src/main/records.c`.
Their names are in the text files above.

| Table | Address | Record | Count | Read by |
|---|---|---|---|---|
| `records_digimon` | `0x8003EF5C` | `RecordsDigimon`, 0x58 | 55; `records_find_digimon` searches the first 52 by `id`; the last 3 are placeholders | `records_get_digimon`, `gamestate_init_records`, STFGTREP, FIGHTSTG |
| `records_items` | `0x80041844` | `RecordsItem`, 0xC | 402, from item ID 1 (`records_items[id - 1]`, folded to `records_items - 0xC`; `ITMNAM`) | `records_get_item`, `gamestate_get_stats`, STITSHOP |
| item data | `.data`/`.sdata` | `RecordsWeapon`/`RecordsArmor` 0x14, `RecordsAccessory` 0xC, `RecordsUsable` 4, by item type | 351 | `gamestate_get_stats`, `gamestate_unequip_item`, STITSHOP, STSTATUS, FIGHTSTG |
| `records_item_lists` | `0x80042BC8` | 5 pointers to 0-terminated u16 item-ID lists | 5 | `records_list_items` |
| `records_techniques` | `0x80042BDC` | `RecordsTechnique`, 0x12 | 443 (`records_techniques[id - 1]`; `SKLNAM`) | FIGHTSTG, STSTATUS, STGDGLAB |

`RecordsDigimon` (fields as `include/records.h`): `0x00` ID; `0x02` 6 × u16 and `0x0E` 7 × u16 base stats
(copied to `GamestateStats.stats`/`resists`); `0x1C` 7 × u16 techniques (1–6 learnt, at the levels in `0x31`);
`0x2C`–`0x30` the status resistances (`FightstgStats.resists[7..11]`); `0x31` technique levels; `0x37` level thresholds;
`0x3E` experience curve; `0x3F`/`0x40` HP and MP at a new game; `0x41` HP/MP gain base; `0x43`/`0x49` per-stat gain
classes; `0x50` 5 blast forms by level band (`records_digimon` index + 1, WFIGHTMN); `0x55` its `?SDIGNAM` entry;
`0x56` its battle type (`FightstgStats.type`); `0x2A`, `0x57` unknown. `RecordsItem`: `0x00` pointer to the type's data, `0x04` price, `0x06` selling price,
`0x08` category (`RecordsItemCategory`: 1 key, 2 usable, 3 weapon, 4 armour, 5 accessory; `records_is_item_category`),
`0x09` type (2–14 weapons, 15–20 armour, 21–24 accessories, 25–28 usable, 29 key items; it selects the data layout;
`records_item_icons[type]` is its icon frame in `CMFONTCS`, `records_get_item_icon`). `RecordsTechnique`: `0x00` MP cost, `0x02` power, `0x04` element, `0x05` target,
`0x06` hit chance /128, `0x07` defending stat, `0x0A` kind, `0x0B` status-effect chance /128 (poison, paralysis, ...,
seals), `0x0C` effect power (poison damage, healing),
`0x08`/`0x09`/`0x0D`–`0x11` unknown.

The battle's own records are on disc, in the data-only overlays (see Battle).

## Save data (`memcard.c`, STGMCARD)
One memory-card file per game: `BESLES-03936DMW3-EUR` (`memcard_set_file_name`; the JP and US names are chosen by
`records_language` 0 and 1), created with 4 blocks (`memcard_create_file`, 0x8000 bytes).

| File offset | Size | Contents | Written by |
|---|---|---|---|
| `0x000` | 0x80 | Sony header (`MemcardHeader`): `"SC"`, type `0x13` (0x10 + 3 icon frames), 4 blocks, Shift-JIS title (`?SMEMCRD` entry 0x27 through `font_convert_text`), CLUT at `0x60` | `memcard_set_header`, STGMCARD |
| `0x080` | 3 × 0x80 | icon frames (16×16, 4-bit) | STGMCARD (`stgmcard_icon`) |
| `0x200` | 0x100 (0xD4 used) | part 1: `StgmcardSaveHeader`, what the load screen shows | `memcard_write(…, part 1)` |
| `0x300` | 0x2700 (0x26C4 used) | part 2: slot 1 | `memcard_write(…, part slot + 2)` |
| `0x2A00` | 0x2700 | part 3: slot 2 | |
| `0x5100` | 0x2700 | part 4: slot 3; the file's last 0x800 bytes are unused | |

The part offsets come from `memcard_read`/`memcard_write`: `frames × 0x80 + 0x80`, then `memcard_state.part1_size` (0x100), then
`slot_size` (0x2700) per slot. Both functions move 0x80 bytes per call.

`StgmcardSaveHeader` (0xD4): `0x00` checksum, `0x01` slot saved last, `0x02` version (4), `0x04` `"DMW3"`
(`0x33574D44`), `0x08` 3 × 0x44 slot summaries (`StgmcardSlotSummary`: name, play time, money, party sprites).

**Slot**: the first 0x26C4 bytes of `gamestate_data`, copied whole (`StgmcardSaveData`). byte `0x00` holds the
checksum, byte `0x02` the version (4) (`GamestateData.checksum`/`version`; STGMCARD reads both with `lbu`), `0x54` the player's name, `0x6C` money, `0x075C` the 8 Digimon records (0x3DC
each), `0x263C` story progress, `0x2644` flag bit arrays. See `include/gamestate.h`. Loading rejects a slot whose
version isn't 4 (for the title screen's load).

**Checksum**: the XOR of every byte after the first word (`memcard_get_checksum`: header bytes 4–0xD3, slot bytes
4–0x26C3), stored in byte 0 (header and slot). The check is lenient: `checksum & ~stored` must be 0
(STGMCARD), not `checksum == stored`. `memcard_check_checksum` (exact) exists but STGMCARD doesn't call it.

**Unused tails**: both parts are written in 0x80-byte calls, so the bytes after the used part (header `0xD4`–`0xFF`,
slot `0x26C4`–`0x26FF`) are whatever follows the game's buffers (`stgmcard_module.header`, 0x180 bytes, and `.save`,
0x2780, from the heap, not cleared): stale RAM, never read back as data.

### The card image (`.mcd`, the PC port's and PCSX-Redux's memory card files)
A raw 128 KB dump of the card (the layout of psx-spx "Memory Card Data Format"): 16 blocks of 0x2000 bytes, each
64 frames of 0x80. Block 0 is the directory: frame 0 `"MC"` (byte 0x7F: the XOR of bytes 0–0x7E, as in every
directory frame), frames 1–15 one entry per data block (`0x00` allocation state: `0x51` first block of a file,
`0x52` middle, `0x53` last, `0xA0` free; `0x04` file size in bytes, in the first block's entry only; `0x08` the next
block − 1, `0xFFFF` at the end; `0x0A` the name, NUL-terminated, in the first block's entry only), frames 16–35 the
broken-frame list (empty: `FFFFFFFF`, next `FFFF`), frame 63 a copy of frame 0. A new card from PCSX-Redux is exactly
this with nothing else set (the port's `psyq_mcrd_format_image` writes the same bytes). After the save of
`first_battle_save`: entries 1–4 are `51 / 0x8000 / next 1 / BESLES-03936DMW3-EUR`, `52 / next 2`, `52 / next 3`,
`53 / FFFF`, and the file is blocks 1–4 (the table above, at 0x2000). The emulator's frame 63 then holds what its BIOS
wrote to clear the card's new-card flag (OpenBIOS: a buffer of its own); the port leaves it as it was.

## Card game (`card.c`, CARDGAME, STCRD*; `AAA/DAT/CARD/`)
| File (ID) | Layout | Read by |
|---|---|---|
| `CARDPAK0`–`CARDPAK4` (`0x7F6`–`0x7FA`, `card_files`) | arrays of 0x62C-byte card records: 64, 64, 64, 64, 60 = 316 cards | `card_select(n)`: record `(n - 1) & 63` of file `card_files[(n - 1) >> 6]` |
| `CARD_NPC` (`0x7A4`) | 170 × 0xD0 `CardgameOpponent` | `cardgame_game_load_opponent` |
| `CARDDATA` (`0x25D`) | offset table: 4 sub-files (TIM lists, sprite banks); sub-file 0 is loaded at `(0x280, 0)` | `cardgame_game_update` |
| `CARDPACK` (`0x25F`) | 316 × 0x62C like the CARDPAKs, but different bytes (an older build?) | no reference found |

**Card record** (0x62C): bytes `0x0C`–`0x62B` are a TIM: 32×32, 8-bit, CLUT, 0x620 bytes. `card_load_image`
loads it into the cell that `card_set_cell` picks. The first 12 bytes:

| Offset | Type | Meaning | Read by |
|---|---|---|---|
| `0x0` | u8 | colour/type 1–6 (deck counts per type, the type icon) | STCRDDEK, STCRDABM, STCRDSHP |
| `0x1` | u8 | first value | STCRDDEK, STCRDSHP (`unk_118`) |
| `0x2` | u8 | second value | same |
| `0x3` | u8 | kind: 16 for the 258 Digimon cards (43 per type), other values for option cards; `card_classes[kind]` → `card_get_class` (0 a Digimon card, 1 an option card, 2 one that can also be played in phase 5: `cardgame_is_card_playable`) | `card_get_class` |
| `0x5` | u8 | level | STCRDDEK, STCRDSHP |
| `0x4`, `0x6`–`0xB` | | unknown (`0x8` is a small s32) | |

**Opponent** (`CardgameOpponent`, 0xD0): `0x00` 40 × `{s16 card (bits 0–11 card + 1, bit 15 a flag), u8 stage (the
CPU may draw it from match stage round × 2 + 1/2 on; 7: kept for the deck's end), u8 kind (the CPU's play class)}`,
`0xA0` 20 × s16 0-terminated `counter_ids` (card IDs + 1 the CPU answers with a kind-4 card), `0xC8` u8 `level` (shown
as "LV"), `0xCC` s32 index into `cardgame_prize_items`. The rules data for
the card effects (`cardgame_card_data`, 60 scripts) is in the CARDGAME overlay, not on disc.

Verified: record counts by file size, the TIM header at `0x0C` in all 316 records, 170 × 0xD0 = `CARD_NPC`'s size.

## Field (FIELDSTG, WSTAG###)
A field stage is one `WSTAG###` overlay plus up to four data files. Its setup function fills `fieldstg_stage` (`FieldstgStageState`, `include/fieldstg.h`) with file IDs and pointers into its
own `.data`. `fieldstg_loader_run` then loads the files.

| `fieldstg_stage` field | Typical value | File | Format |
|---|---|---|---|
| `background_file` | `S###PACK` (`Z_STAGE/`) | streamed background | below |
| `sprite_file` | `S###TMPK` sub-file 0 | the stage's sprite bank | sprite bank |
| `mask_file` | `S###MASK` (`FIELD/STAGE/`) | loaded whole and freed after the upload | TIM list of 1–3 RLEN-packed TIMs (8-bit with CLUT or 16-bit), at `(0x140, 0x100)`, CLUT `(0, 0x1F0)` |
| `mask_subfile` | always 0 (nothing sets it) | | would be a TIM list sub-file ID, same place |
| `fieldstg_attr.set_file(i, id)` | `S###TMPK` sub-files 1, 2, … | attribute (collision) maps, layers 0–7 | below |
| `talk_file` | `records_language + ?STALK0n` | NPC lines | text table |
| `music`, `sound` | bank ID, sound key | | Sound |

Shared files: `FIELD/FIELDCOM.BIN` (`0x160`): sub-files 0–1 are the field's common sprite banks, and 2–3 TIM
lists loaded at `(0x200, 0x100)` and `(0x240, 0x100)`. `P002PLYD/F/K/L` (`0x3C9`–`0x3CC`) are loaded when the map
events need them (`fieldstg_loader_load_action_anims`). `SDIGDEMO`/`SSUBDEMO` (`0x88D`/`0x88C`) hold sprites and
TIMs for a demo object (`fieldstg_80085590.c`).

### Background (`Z_STAGE/S###PACK.BIN`; `fieldstg_background_update`, `fieldstg_tile_*`)
The map is made of 128×128 tiles. Only the tiles around the view are in memory (30 tile buffers, 12 VRAM slots).

| Offset | Type | Meaning |
|---|---|---|
| `0x0` | s32 | number of non-empty tiles (`tile_count`; the code ignores it; true in all 237 files) |
| `0x4` | s32 | width in tiles |
| `0x8` | s32 | height in tiles |
| `0xC` | s32 | bytes per tile slot (a sector multiple: 0x3800–0x5800) |
| `0x10` | u16[w×h] | per tile: its size in bytes, 0 = no tile |
| sector `1 + n × slot / 0x800` | | tile `n` (`fieldstg_tile_load` reads it with `cdload_read`) |

A tile is RLEN-packed (or raw). `fieldstg_tile_unpack` unpacks 0x2800 bytes per frame. Unpacked, it is an offset
table whose sub-file 0 is an 8-bit TIM, loaded into the tile's VRAM slot with CLUT row `0xF0 + slot`. Words 1… hold
three layers (OT depths `fieldstg_tile_ot_depths`), each `{s32 count; count × {s16 x, y, u, v, w, h}}`, one SPRT
per entry (`fieldstg_tile_upload`). The tile object keeps 5 SPRTs per layer. In 5 files (`S202`, `S203`, `S370`,
`S371`, `S530`), one or two tiles have 6 in layer 0. The sixth overwrites layer 1's first slot and is lost when
layer 1 is filled in.

Verified: all 237 files (11,745 tiles; 4,403 stored raw). The header's tile count matches, every tile's TIM is
8-bit with a CLUT, and the layer lists end before the TIM.

### Attribute maps (`S###TMPK` sub-files 1+; `fieldstg_attr_load_layer`, `fieldstg_attr_get`)
A 6-level lookup that shares identical blocks. The sub-file is an offset table of 6 parts:

| Part | Entry | Meaning |
|---|---|---|
| 0 | u8 `w`, u8 `h`, then u8[w×h] | per 128-pixel block: an index into part 1 |
| 1 | u8 × 4 per entry | 64-pixel quarters (index × 4 + (y & 64 ? 2 : 0) + (x & 64 ? 1 : 0)) → part 2 |
| 2 | s16 × 4 | 32-pixel quarters → part 3 |
| 3 | s16 × 4 | 16-pixel quarters → part 4 |
| 4 | s16 × 4 | 8-pixel quarters → part 5 |
| 5 | u8[64] per entry | the 8×8 attribute bytes |

In an attribute byte, bits 0–3 are a slope class and bit 4 mirrors it (`fieldstg_attr_get_step`: the step per
direction, `fieldstg_slope_steps`). The higher bits are used elsewhere (values up to 0x29 and beyond occur) and
are not decoded yet. Without a map, `fieldstg_attr_get` returns 1. Verified: 18 of the 294 TMPK files are all zero.
In 263 of the other 276, sub-file 0 is the sprite bank and every later sub-file parses as a map with every index in
range. The other 13 have a sprite bank or an unknown sub-file at another position.

### Actor sprites (`FIELD/SPRT/P###xxxx.BIN`; `fieldstg_actor_update_sprite`)
`fieldstg_actor_sprite_files[406]` gives each actor ID a sub-file ID: the animation table of a P-file (an offset
table).
- **Animation table**: 12-byte records `{s16 anim; s16 script[5]}`, ended by `anim == 0`. Each script is a
  sub-file index for directions 0–4. Directions 5–7 use 3–1 mirrored. An unknown anim falls back to the first
  record.
- **Script**: s32 words, 4 per frame: `{time, frame sub-file index, x offset, y offset}`. A `time` of 0 loops to
  the start, and -1 ends the animation (`anim_done`).
- **Frame**: a 4-bit TIM with a CLUT, loaded into the actor's `FieldstgVramPlace` when the frame changes.

Verified: all 170 sprite sets that the table names. Every script frame is a TIM with flags 8 (11,230 frames).

### Stage data in the WSTAG overlay (`.data`)
These structures are in each `WSTAG###.PRO` (`include/fieldstg.h`). Pointers are absolute RAM addresses
(tier-2 base `0x800A5DE0`), so this data is code-linked, not a file format. A PC port has to relocate it or
rebuild it from the C.

| Structure | Size | End | Contents |
|---|---|---|---|
| `FieldstgSprite` (`fieldstg_stage.sprites`) | 0x12 | `present == 0` | shown, type (0xFF: common bank), sprite, animation mode, frames, x, y, priority |
| `FieldstgMapEvent` (`map_events`) | 0x18 | `type == 0` | 2 flag conditions (0xFFFF: none), type 1–14, parameter, x, y, … |
| `FieldstgPlacedActor *[]` (`actors`) | 0x14 each | NULL | flag condition list, talk list, actor ID, VRAM place, x, y, direction |
| `FieldstgTalk` | 0xC | | flags required, flags set afterwards, message index (`talk_file`) |
| `FieldstgVramPlace[]` (`vram_places`) | 0x10 | | tpage, image and CLUT positions: [0] shadow, [1] background, [2+] actors |
| `FieldstgEventDef[]` (`events`) | 0x14 | ID -1 | ID, script pointer, text sub-file ID (`?SDMG###` JPN + language), poll and end callbacks |
| `FieldstgBattleLists` | 0x1C | | encounter lists per area (`encounters`) and the battles events start (`scripted`); `FieldstgBattleList`: a rate index and 8 × `FieldstgListedBattle` {battle, stage, music} |

**Event scripts** (`fieldstg_event_update`): s16 words. The high byte of the first word is the command and the
low byte its variant:

| Word | Arguments | Action |
|---|---|---|
| `0x00xx` (or unknown) | | end of the script |
| `0x0100` | actor, x, y | place the actor (pixels) |
| `0x0101` | id, a, b | actor id < 0x320: `play_anim(a, b)`; else message a script object (started if needed, `fieldstg_start_script_object`) |
| `0x0102` | actor, x, y, dir | walk to (x, y), then face dir |
| `0x02xx` | slot, message, actor, mode | open dialog `slot` (0–2) with a message of the event's text; mode 4: no speaker |
| `0x0300` | n | wait n frames |
| `0x0301` | | wait for dialog 0 to close |
| `0x0302` | actor | wait until it stops walking |
| `0x0303` | actor | wait for its animation |
| `0x0304` | map, x, y, dir | go to another map (`fieldstg_goto_map`) |
| `0x0600` | snap, actor | camera follows the actor (`fieldstg_camera_follow`) |
| `0x0601` | snap, x, y | camera moves to (x, y) (`fieldstg_camera_move_to`) |

After the script, the event's poll callback runs until it returns 0, then the end callback. Event IDs
8000–8999 don't stop the player. In C the scripts are written with the `FIELDSTG_EVENT_*` macros (`include/fieldstg.h`),
one command per line; every script of the 293 WSTAG files parses into these commands and ends with `0x0000`
(29 of them have a non-zero padding word after the end).

## Battle (FIGHTSTG, WFIGHTMN)
| File (ID) | Layout | Read by |
|---|---|---|
| `SMDLDATA.PRO` (`0x1CC`) | header `{?, entries, A records, B records}` (offsets); entries `{s16 ID, u8 index, u8 kind}` 0-terminated; kind < 0x3A → `FightstgModelRecordA` (0xC4: 12 camera set-ups), else `FightstgModelRecordB` (0x48: 3) | `fightstg_models_get` |
| `SDIGIEDT.PRO` (`0x1CF`) | `FightstgEnemyRecord` × 193 (0x46 each) until ID 0: item, drop rate, name, 3 techniques, 5 stats, 12 resistances, type, 3 conditional actions, counter | `fightstg_enemy_find_record` |
| `SFSTDATA.PRO` (`0x1CB`) | `FightstgStageRecord` × 57 (0x58 each): model and animation sub-file IDs (in `FIGHT/EFFECT/MEFT####`), sound, background colour, unclipped parts, lights | `fightstg_stage_*` |
| `FIGHT/MODEL/M###xxxx.BIN` | model containers, below | `fightstg_model_new` |
| `FIGHT/EFFECT/E###xxxx`, `MEFT####` | effect and stage containers (TIM lists, sprite banks, models) | `fightstg_effect_get_files` and the effect code (not traced) |
| `FIGHT/F000COM1–4` | TIM lists of RLEN TIMs | not traced |

`SMDLDATA` has 248 entries: 55 type A (the party Digimon) and 193 type B, the same count as `SDIGIEDT`'s records.
Each record starts with `model_file`, `anim_file`, `script_file`, `texture_anims` (sub-file IDs), `depth`, and
`height`.

### Models (`fightstg_model.c`)
Inside the model file, `model_file` names a sub-file, the **model header**:

| Offset | Type | Meaning |
|---|---|---|
| `0x0` | s32 | sub-file index of the texture's TIM list (0: none); loaded at the model's VRAM position |
| `0x4` | s32 | `n`, the number of parts after the root |
| `0x8 + 12i` | s32 × 3 | part `i + 1`: parent part index, mesh sub-file index, key sub-file index |

- **Mesh** (`fightstg_model_mesh_create`): a sub-file that is itself an offset table of 6. Sub-file 0 holds the
  vertices: 6-byte `FightstgVec` `{s16 x, y, z}`, the first one's `x` being the count. Sub-file 1 holds the normals
  in the same shape (lit with `gte_ncs`). Sub-file 2 is the face command stream, and sub-file 5 the bounds (9 6-byte vectors
  tested by `fightstg_model_mesh_is_visible`). No code read so far uses sub-files 3 and 4.
- **Face command stream** (`fightstg_model_mesh_draw`):
  - A byte `0x8n`–`0xEn` sets a state: 8 quad, 9 textured, 0xA ?, 0xB ?, 0xC gouraud, 0xD gouraud polygon (GT),
    0xE blend mode + 1.
  - `01 u_lo u_hi v clut_x clut_y tpage_mode` sets the texture: the UV base, the CLUT relative to the model's VRAM
    position, and the page.
  - `02`–`05 r g b` sets colour 0–3.
  - `00` starts a run of faces. Each face is 3 or 4 vertex indices, then as many normal indices (gouraud), then as
    many u, v pairs (textured). The run continues while the next byte is 0.
  - `FF` ends the stream.
  Back faces (`nclip`) and degenerate faces are skipped.
- **Keys** (`fightstg_model_pose_part`): the part's key sub-file is an offset table of 4. Sub-files 0, 1 and 2 are
  the translation, rotation and scale keys, SVECTORs whose `pad` is the key frame, ascending. The pose between two
  keys is interpolated. Sub-file 3 is unused.
- **Animations** (`fightstg_model_play_anim`): `anim_file` names an offset table of animations (62 in every
  model). Animation `a` (from 1) is sub-file `a - 1`: `FightstgAnimKey` `{s16 index, frames, key, blend_key}`
  until `index == 0x7FFF`. `frames == 0` holds `key` and stops. `blend_key == 0` blends over `frames` frames to the
  next entry's `key` (-1: the idle animation). Otherwise it plays `frames` key frames from `key`.

Verified: all 248 SMDLDATA entries and their files. That covers 5,402 meshes: each is a 6-entry table, every face
command stream walks to `FF` with all vertex indices below the vertex count, every key table has 4 entries, and
all 62 animations of each model end with `0x7FFF`. `SDIGIEDT`'s 193 records end with ID 0. `SFSTDATA` is exactly
57 records.

## Movies (`AAA/STR/MOVIE*.STR`)
14 files of mixed-mode sectors (MDEC video, XA audio), stored by dumpsxiso as 2336-byte sectors. Video sectors
carry the MDEC frame magic `0x80010160`. The player is the title overlay's LIBPRESS (`STDWTITL`). Not traced
further: no game code that reads them has been read for this document.

## Not yet traced
- `TRAINING/TRANIN##.BIN` (12 files, offset tables of 9 sub-files; STGTRAIN), `SCREEN/TLOGO*`, `TTCOMMPK`,
  `END__ALL`, `CNTSELWN`, `FIGHT/PGFGTC00`/`PGFGTW00`. All are offset tables of TIM lists, sprite banks and
  unknown sub-files.
- Battle effect files (`E###`, `MEFT####`) beyond the model and TIM sub-files; `script_file` / `texture_anims` of
  the model records (`fightstg_script_create`, `fightstg_texture_anim_create`).
- `S###BG01`/`BG02` (2 stages each, TIM lists); `S###TMPK` sub-files that are neither sprite banks nor attribute
  maps (in a few stages).
- Field: the attribute byte's high bits; `FieldstgMapEvent` types 2–14 in detail;
  card record bytes 4 and 6–11.
- The EU accent table is inferred from text, not checked against `CMFONTTM`'s images.
- `NONE____.BIN` (ID 0) is a 1-byte file. `CARDPACK.BIN` and the `?SAMTMAP`/`ASKMAP`/`SYSTEM` texts have no code
  reference found by ID.

## Tools and verification scripts
- `tools/disc_files.py`: file ID ↔ path (from the EXE's table and the disc's ISO path table), and `read(id)` /
  `subfile()` helpers for scripts.
- `tools/dump_text.py`: prints any text file in any language (`--check` decodes all of them).
- `tests/formats/run.sh` (layer 3 of `scripts/test.sh`) runs the repeatable checks: `disc_files.py --check`,
  `dump_text.py --check`, `overlay_layout.py --wstag-table` (WSTAG stage table) and `flag_census.py --check`.
- The other checks in this document were one-off scripts (kept out of the repo). Each parses every file of one
  kind the way the cited C does: sound banks, `Z_STAGE` backgrounds, attribute maps, actor sprites, sprite banks,
  battle models, card files.
