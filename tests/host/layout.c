/* Host struct layout facts the driver needs: on an LP64 host, structs with pointers are laid out differently from the
 * PS1 (ILP32), so a fixture buffer of such a struct is remapped by these offsets (tests/host/replay.py). */
#include <stddef.h>
#include "common.h"
#include "cardgame.h"
#include "fightstg.h"
#include "gamestate.h"
#include "gfx.h"
#include "heap.h"
#include "object.h"
#include "stgtrain.h"
#include "stgdglab.h"
#include "message.h"
#include "stcrdshp.h"
#include "records.h"

/* StfgtrepMember is local to src/stfgtrep/stfgtrep_80082E70.c: its head, as declared there (Object, a pointer, then s32s). */
struct host_stfgtrep_member_head {
    Object base;
    void *main;
    s32 layer_id, ot_depth, slot, exp_gained, bonus_applied;
};

/* StstatusItemsPage and StstatusTechPage are local to src/ststatus/ststatus_8008E94C.c and ststatus_800937A4.c: their
 * heads, as declared there (Object, then s32s and s16 arrays: one shift for every field after the Object). */
struct host_ststatus_items_page_head {
    Object base;
    s32 parent, layer_id, ot_depth, member_count, frames[3], frame_time, category_cursor, unk_74, item, list_result,
        counts_shown;
    s16 items[0x194];
    s32 item_count, member;
};
struct host_ststatus_tech_page_head {
    Object base;
    s32 parent, layer_id, ot_depth, member_count, frames[3], frame_time, member_cursor_frame, member_cursor_time,
        member_cursor_shown, user, target_cursor_shown, target, technique_cursor;
    s16 members[1];
};

/* FightstgEnemyTurn is local to src/fightstg/fightstg_80086A00.c: its head, as declared there (Object, then s32s and a
 * byte array: one shift for every field after the Object). */
struct host_fightstg_enemy_turn_head {
    Object base;
    s32 args[3];
    u8 unk_5C[0x1C];
    s32 action;
};

struct host_offset { const char *name; long value; };
struct host_offset host_offsets[] = {
    { "sizeof(Object)", sizeof(Object) },
    { "Object.children", offsetof(Object, children) },
    { "sizeof(HeapObjects)", sizeof(HeapObjects) },
    { "HeapObjects.objects", offsetof(HeapObjects, objects) },
    { "HeapObjects.filter", offsetof(HeapObjects, filter) },
    { "sizeof(GfxModule)", sizeof(GfxModule) },
    { "GfxModule.packet", offsetof(GfxModule, packet) },
    { "Sprite.ot_entry", offsetof(Sprite, ot_entry) },
    { "Sprite.vram_x", offsetof(Sprite, vram_x) },
    { "Sprite.scale", offsetof(Sprite, scale) },
    { "Sprite.matrix", offsetof(Sprite, matrix) },
    { "sizeof(pointer)", sizeof(void *) },
    { "CardgameGame.card_ids", offsetof(CardgameGame, card_ids) },
    { "CardgameGame.cpu_cards", offsetof(CardgameGame, cpu_cards) },
    { "CardgameGame.selectable", offsetof(CardgameGame, selectable) },
    { "CardgameGame.players", offsetof(CardgameGame, players) },
    { "CardgameGame.slots", offsetof(CardgameGame, slots) },
    { "CardgameGame.shuffle_deck", offsetof(CardgameGame, shuffle_deck) },
    { "CardgameGame.marked", offsetof(CardgameGame, marked) },
    { "CardgameGame.display", offsetof(CardgameGame, display) },
    { "CardgameGame.turns", offsetof(CardgameGame, turns) },
    { "CardgameGame.get_score", offsetof(CardgameGame, get_score) },
    { "sizeof(CardgameBoard)", sizeof(CardgameBoard) },
    { "FightstgBattle.state", offsetof(FightstgBattle, state) },
    { "RecordsState.gauges", offsetof(RecordsState, gauges) },
    { "sizeof(RecordsState)", sizeof(RecordsState) },
    { "GamestateData.funcs", offsetof(GamestateData, funcs) },
    { "GamestateData.digimon", offsetof(GamestateData, digimon) },
    { "StgtrainSession.main", offsetof(StgtrainSession, main) },
    { "StgtrainSession.layer", offsetof(StgtrainSession, layer) },
    { "StgtrainSession.digimon", offsetof(StgtrainSession, digimon) },
    { "StgtrainSession.bonus", offsetof(StgtrainSession, bonus) },
    { "StgtrainMain.layer", offsetof(StgtrainMain, layer) },
    { "StgtrainMain.level", offsetof(StgtrainMain, level) },
    { "FightstgEnemyTurn.args", offsetof(struct host_fightstg_enemy_turn_head, args) },
    { "FightstgEnemyTurn.action", offsetof(struct host_fightstg_enemy_turn_head, action) },
    { "StfgtrepMember.layer_id", offsetof(struct host_stfgtrep_member_head, layer_id) },
    { "StfgtrepMember.bonus_applied", offsetof(struct host_stfgtrep_member_head, bonus_applied) },
    { "StstatusItemsPage.parent", offsetof(struct host_ststatus_items_page_head, parent) },
    { "StstatusItemsPage.item", offsetof(struct host_ststatus_items_page_head, item) },
    { "StstatusItemsPage.member", offsetof(struct host_ststatus_items_page_head, member) },
    { "StstatusTechPage.parent", offsetof(struct host_ststatus_tech_page_head, parent) },
    { "StstatusTechPage.user", offsetof(struct host_ststatus_tech_page_head, user) },
    { "StstatusTechPage.members", offsetof(struct host_ststatus_tech_page_head, members) },
    { "StgdglabMain.layer_id", offsetof(StgdglabMain, layer_id) },
    { "StgdglabMain.member_count", offsetof(StgdglabMain, member_count) },
    { "sizeof(MessageWindow)", sizeof(MessageWindow) },
    { "sizeof(StcrdshpShop)", sizeof(StcrdshpShop) },
    { "StcrdshpShop.count", offsetof(StcrdshpShop, count) },
    { 0, 0 },
};
