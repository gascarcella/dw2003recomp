"""records_* (src/main/records.c): the Digimon, item and technique tables and their pure accessors.
docs/MECHANICS.md sections 8 and 9.

The tables are the EXE's .data: a regression golden records their SHA-1 as the game holds them in RAM
(records_items is left out: it holds pointers, so a host build lays it out differently). The accessors:
records_find_digimon(id) for every id in the table and some that are not, records_get_item_icon(id) for every item (the
icon is the item's type), records_is_item_category(id, category) over a sample of items x every category.
"""
import re
from pathlib import Path

from oracle import Call, Case, Read

COMMENT = ("SHA-1 of records_digimon (55 x 0x58), records_techniques (443 x 0x12) and records_item_icons (32 bytes) in RAM; "
           "records_find_digimon for the 55 ids of the table and 6 unknown ids; records_get_item_icon for items 1..402; "
           "records_is_item_category for 25 items x categories 0..6.")

DIGIMON_SIZE, DIGIMON_COUNT = 0x58, 55
TECHNIQUES_SIZE = 0x1F26
ICONS_SIZE = 32
ITEM_COUNT = 402
UNKNOWN_IDS = [0, 1, 2, 0x1234, 0xFFFF, 0x7FFFFFFF]
SAMPLE_ITEMS = [1, 6, 42, 43, 47, 91, 92, 93, 150, 176, 200, 214, 215, 217, 250, 291, 292, 293, 300, 330, 350, 380, 401, 402, 7]
CATEGORIES = [0, 1, 2, 3, 4, 5, 6]


def digimon_ids():
    """The id of every records_digimon row, parsed from src/main/records.c."""
    src = (Path(__file__).resolve().parents[3] / "src/main/records.c").read_text()
    body = src[src.index("records_digimon[] = {"):]
    body = body[:body.index("\n};")]
    ids = [int(m, 16) for m in re.findall(r"^\s*\{ (0x[0-9A-Fa-f]+),", body, re.M)]
    assert len(ids) == DIGIMON_COUNT, len(ids)
    return ids


def cases(sym):
    out = [Case("tables", [Call("records_find_digimon", [0x17F], "s32", [
        Read("records_digimon", 0, DIGIMON_COUNT * DIGIMON_SIZE, "records_digimon[55] (SHA-1)"),
        Read("records_techniques", 0, TECHNIQUES_SIZE, "records_techniques[443] (SHA-1)"),
        Read("records_item_icons", 0, ICONS_SIZE, "records_item_icons[32]")],
        comment="Kotemon is row 0; the reads are the tables themselves")],
        comment="the pointer-free tables as the game holds them in RAM (records_items holds pointers: not recorded)")]
    ids = digimon_ids()
    out.append(Case("find_digimon", [Call("records_find_digimon", [i], "s32", comment=f"id {i:#x}") for i in ids],
                    comment="every id of the table, in table order: returns the row"))
    out.append(Case("find_digimon_unknown", [Call("records_find_digimon", [i], "s32", comment=f"id {i:#x}") for i in UNKNOWN_IDS],
                    comment="ids not in the table: -1 (the compare is on a u16 field, so 0x7FFFFFFF matches nothing)"))
    out.append(Case("get_item_icon", [Call("records_get_item_icon", [i], "s32", comment=f"item {i}") for i in range(1, ITEM_COUNT + 1)],
                    comment="records_item_icons[records_items[id - 1].type] for every item: the type of each item"))
    for item in SAMPLE_ITEMS:
        out.append(Case(f"is_item_category_{item}", [Call("records_is_item_category", [item, c], "s32", comment=f"category {c}") for c in CATEGORIES],
                        comment="1 key, 2 usable, 3 weapon, 4 armour, 5 accessory"))
    return out
