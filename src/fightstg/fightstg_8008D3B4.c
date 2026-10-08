#include "common.h"

#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "sound.h"
#include "gamestate.h"
#include "cdload.h"
#include "message.h"
#include "records.h"
#include "pad.h"
#include "fightstg.h"
#include "psyq/gtemac.h"

/* .bss, in address order. GCC 2.8 emits uninitialized definitions in the order of their first declaration
 * (fightstg_camera_setting's is in fightstg.h), so they are defined here, before the code's own externs. */
FightstgCameraSetting fightstg_camera_setting;
s32 fightstg_unused_2; /* unreferenced */
RECT fightstg_portrait_rect;
DR_MOVE fightstg_cursor_moves[4];
u32 fightstg_cursor_ot[2];
FightstgNewEvent fightstg_new_event;

/* Per event type: 0 = keep it queued and return 0, -1 = keep it queued, else take it off. */
extern s32 fightstg_events_take_modes[];
extern struct FightstgCursorParams fightstg_command_menu_cursor;
/* Event types, indexed by fightstg_events_start_modifier's arg2. */
extern s32 fightstg_modifier_event_types[];

/* File 0x1CC's header (fightstg_models_get's records: FightstgModelRecordA/_B). */
typedef struct FightstgModelFile {
    /* 0x0 */ s32 base;
    /* 0x4 */ s32 entries; /* offset of the entries: {s16 ID, u8 record index, u8 kind}, 0-terminated */
    /* 0x8 */ s32 records_a; /* offset of the type A records */
    /* 0xC */ s32 records_b; /* offset of the type B records */
} FightstgModelFile;

/* File 0x1CC's entries (at `entries`), 0-terminated. */
typedef struct FightstgModelEntry {
    /* 0x0 */ s16 id;    /* ID */
    /* 0x2 */ u8 index;  /* record index */
    /* 0x3 */ u8 kind;   /* kind: below 0x3A a type A record, else type B */
} FightstgModelEntry; /* size 0x4 */

/* Objects (object_new / object_create), named after the function that creates them. */
/* The party's action (fightstg_item_update). */
typedef struct FightstgItem {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x5C */ u8 unk_5C[0x14];
    /* 0x70 */ s32 item; /* action */
    /* 0x74 */ s32 field_element;
    /* 0x78 */ s32 amount; /* amount */
    /* 0x7C */ s32 lowered;
} FightstgItem; /* size 0x80 */

/* A technique or item used by side `side` (0 = the party, 0x10 = the enemy). */
typedef struct FightstgCounter {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x5C */ u8 unk_5C[0x14];
    /* 0x70 */ u8 side;
    /* 0x71 */ u8 pad_71[0x3];
    /* 0x74 */ s32 damage_taken;
    /* 0x78 */ s32 tech; /* record of records_techniques (from 1) */
    /* 0x7C */ s32 hit; /* it hits */
    /* 0x80 */ s32 damage;
    /* 0x84 */ s32 no_knockout;
    /* 0x88 */ u8 unk_88[0x4];
} FightstgCounter; /* size 0x8C */

/* A technique used by side `side` (fightstg_tech_update). */
typedef struct FightstgTech {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 side; /* side */
    /* 0x51 */ u8 pad_51[0x3];
    /* 0x54 */ s32 tech; /* technique: record of records_techniques (from 1) */
    /* 0x58 */ s32 damage;
    /* 0x5C */ s32 healed; /* amount healed */
    /* 0x60 */ s32 target_asleep; /* the target counters */
    /* 0x64 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x70 */ u8 unk_70[0x14];
} FightstgTech; /* size 0x84 */

/* An enemy attack (counter = 0) or counter (1): numbers and effects through a child object. */
typedef struct FightstgBossTurn {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x5C */ u8 unk_5C[0x4];
    /* 0x60 */ s32 restore_idle; /* the boss's weakened phase ended: reset its idle animation after the attack */
    /* 0x64 */ u8 unk_64[0xC];
    /* 0x70 */ s32 tech; /* the technique used (record of records_techniques, from 1) */
    /* 0x74 */ s32 counter; /* fightstg_boss_turn_create's arg0: 0 the boss's turn (state 1), 1 its counter (state 2) */
    /* 0x78 */ s32 no_knockout; /* the counter doesn't knock out (fightstg_boss_turn_create's arg1) */
    /* 0x7C */ s32 target_asleep; /* the party's member is asleep: it may wake (fightstg_rules.roll_wake) */
} FightstgBossTurn; /* size 0x80 */

/* Applies the status effects of an action (fightstg_action.effects), one per step. */
typedef struct FightstgResults {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 side; /* side acting: 0 or 0x10 */
    /* 0x51 */ u8 pad_51[0x3];
    /* 0x54 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args) */
    /* 0x60 */ u8 unk_60[0x14];
} FightstgResults; /* size 0x74 */

/* A view of a model on layer 0x1009 (fightstg_portrait_update's window). */
typedef struct FightstgModelView {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 model_id; /* model ID (fightstg_models.get) */
    /* 0x54 */ FightstgModelParams *params;
    /* 0x58 */ GsRVIEW2 view;
    /* 0x78 */ s32 projection; /* the layer's projection */
    /* 0x7C */ GsCOORDINATE2 coord;
    /* 0xCC */ SVECTOR rot; /* rotation of coord */
    /* 0xD4 */ VECTOR trans;  /* translation of coord */
    /* 0xE4 */ s32 frames;     /* frames left to update */
} FightstgModelView; /* size 0xE8 */

/* A fade: level goes 0 -> 0xFF by step per frame (state 1), then back to 0 (state 2). */
typedef struct FightstgFlash {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 step; /* step: 0xFF / frames */
    /* 0x54 */ s32 level; /* level, 0..0xFF */
} FightstgFlash; /* size 0x58 */

typedef struct FightstgCommandMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 sel;
    /* 0x54 */ s32 *result; /* the caller's slot: set to -1 */
} FightstgCommandMenu; /* size 0x58 */

/* What fightstg_cursor_create copies into its object (e.g. fightstg_command_menu_cursor). */
typedef struct FightstgCursorParams {
    /* 0x00 */ s32 count; /* entries */
    /* 0x04 */ s32 vram_x; /* VRAM x the highlight is copied from */
    /* 0x08 */ s32 vram_y; /* VRAM y of entry 0 */
    /* 0x0C */ s32 vram_step; /* VRAM y step */
    /* 0x10 */ s32 sprite; /* the entries' sprite (-1: none) */
    /* 0x14 */ s32 x;
    /* 0x18 */ s32 y; /* y of entry 0 */
    /* 0x1C */ s32 y_step;
} FightstgCursorParams; /* size 0x20 */

/* The object fightstg_cursor_create creates (a cursor/selection). */
typedef struct FightstgCursor {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 sel; /* selected entry */
    /* 0x54 */ s32 locked;
    /* 0x58 */ FightstgCursorParams params;
    /* 0x78 */ s32 drawn_sel; /* sel when the highlight was last drawn */
    /* 0x7C */ s32 highlight; /* highlight frame, 0..11 */
    /* 0x80 */ s32 times[10]; /* per entry: animation time */
} FightstgCursor; /* size 0xA8 */

/* An item menu: the items of kind 2 (records_funcs.get_item), seven per page. */
typedef struct FightstgItemMenu {
    /* 0x000 */ Object base;
    /* 0x050 */ s32 *result; /* the caller's slot: set to -1, then to the item chosen (-2: cancelled) */
    /* 0x054 */ s32 shown;   /* entry shown */
    /* 0x058 */ s16 items[0x194];  /* items (records_funcs.list_items), 0-terminated */
    /* 0x380 */ s16 *list;    /* the items listed */
    /* 0x384 */ s32 count;    /* their number */
    /* 0x388 */ s32 page;     /* page */
    /* 0x38C */ s32 pages;    /* pages */
} FightstgItemMenu; /* size 0x390 */

/* A technique menu: six per page (fightstg_tech_menu_draw, 80096048, 80096314 show it). */
typedef struct FightstgTechMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *result; /* the caller's slot: set to -1 */
    /* 0x54 */ s32 shown;  /* the cursor's entry, as last shown */
    /* 0x58 */ s16 forms[3];  /* the acting member's chosen forms (gamestate_data.funcs.get_chosen_forms) */
    /* 0x5E */ u8 pad_5E[0x2];
    /* 0x60 */ GamestateForm form_records[3]; /* the forms' records (gamestate_data.funcs.get_form) */
    /* 0x9C */ s32 techniques[12]; /* technique IDs, 0x8000/0x4000 flags */
    /* 0xCC */ s32 count;  /* their number */
    /* 0xD0 */ s32 page;   /* page */
    /* 0xD4 */ s32 pages;  /* pages */
} FightstgTechMenu; /* size 0xD8 */

/* fightstg_command_menu_update's data block (object_new's third argument). */
typedef struct FightstgCommandMenuData {
    /* 0x00 */ FightstgCursor *cursor; /* fightstg_cursor_create(&fightstg_command_menu_cursor) */
    /* 0x04 */ MessageWindow *windows[6];
} FightstgCommandMenuData; /* size 0x1C */

/* A Digimon choice for a party member (its own and its other forms), with a status screen
 * (fightstg_status_create) of the one under the cursor. */
typedef struct FightstgDigivolveMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *result; /* the caller's slot: its value is taken, then it is set to the choice (-2: cancelled) */
    /* 0x54 */ s32 member; /* party member */
    /* 0x58 */ s32 status_page; /* status page */
    /* 0x5C */ s32 cursor; /* entry under the cursor */
    /* 0x60 */ s16 digimon[4]; /* the Digimon listed */
    /* 0x68 */ s32 count;  /* their number */
    /* 0x6C */ s16 forms[3];  /* gamestate_data.funcs.get_chosen_forms */
    /* 0x72 */ u8 pad_72[0x2];
} FightstgDigivolveMenu; /* size 0x74 */

/* A party member's status screen (stats, techniques). */
typedef struct FightstgStatus {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 unk_50[0x4];
    /* 0x54 */ s32 member; /* party member (gamestate_data.funcs) */
    /* 0x58 */ s32 page;   /* page: 0 = stats, 1-2 */
    /* 0x5C */ s32 form;   /* entry of forms, from 1; 0 = none */
    /* 0x60 */ s32 form_level;
    /* 0x64 */ GamestateStats stats;  /* stats (gamestate_data.funcs.get_stats) */
    /* 0x90 */ s16 forms[3];  /* chosen forms (gamestate_data.funcs.get_chosen_forms) */
    /* 0x96 */ u8 pad_96[0x2];
    /* 0x98 */ GamestateForm form_records[3]; /* their records (gamestate_data.funcs.get_form) */
    /* 0xD4 */ s32 techniques[6]; /* technique IDs, 0x8000/0x4000 flags */
} FightstgStatus; /* size 0xEC */

/* fightstg_status_update's data block: its message windows. */
typedef struct FightstgStatusData {
    /* 0x00 */ u8 unk_00[0x4];
    /* 0x04 */ MessageWindow *l1_window;
    /* 0x08 */ MessageWindow *r1_window;
    /* 0x0C */ MessageWindow *skill_label;
    /* 0x10 */ MessageWindow *skill_level;
    /* 0x14 */ MessageWindow *stat_windows[13]; /* the stats (fightstg_status_stat_windows) */
    /* 0x48 */ MessageWindow *tech_windows[6]; /* the techniques */
} FightstgStatusData; /* size 0x60 */

/* fightstg_member_menu_create's object: picks a party member (not the acting one). */
typedef struct FightstgMemberMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *result; /* the caller's slot: set to -1, then the member picked (-2: cancelled) */
    /* 0x54 */ s32 *cursor; /* the cursor, kept by the caller */
    /* 0x58 */ s32 check_items; /* items: also check the members' items (fightstg_member_menu_get_team_item) */
    /* 0x5C */ s32 members[2]; /* the members shown */
    /* 0x64 */ s32 count;   /* how many */
    /* 0x68 */ s32 cursor_started;
    /* 0x6C */ s32 cursor_time; /* time of the last cursor frame */
    /* 0x70 */ s32 cursor_frame; /* blinking cursor frame, 0..4 */
    /* 0x74 */ s32 team_items[2]; /* per member shown: fightstg_member_menu_get_team_item */
    /* 0x7C */ s16 forms[3];  /* the acting member's chosen forms (gamestate_data.funcs.get_chosen_forms) */
    /* 0x82 */ u8 unk_82[0x6];
} FightstgMemberMenu; /* size 0x88 */

/* Its data block (base.children): message windows. */
typedef struct FightstgMemberMenuData {
    /* 0x00 */ FightstgCursor *cursor;
    /* 0x04 */ MessageWindow *hp_labels[2];
    /* 0x0C */ MessageWindow *hp_values[2]; /* value of hp_labels */
    /* 0x14 */ MessageWindow *hp_slashes[2];
    /* 0x1C */ MessageWindow *max_hp_values[2]; /* value of hp_slashes */
    /* 0x24 */ MessageWindow *mp_labels[2];
    /* 0x2C */ MessageWindow *mp_values[2]; /* value of mp_labels */
    /* 0x34 */ MessageWindow *mp_slashes[2];
    /* 0x3C */ MessageWindow *max_mp_values[2]; /* value of mp_slashes */
    /* 0x44 */ MessageWindow *names[2];  /* name */
    /* 0x4C */ MessageWindow *none_window; /* shown when no one can be picked */
} FightstgMemberMenuData; /* size 0x50 */

/* fightstg_switch_menu_create's object: picks a partner for member `member` (and with fightstg_switch_menu_create_team an
 * action with a cost). */
typedef struct FightstgSwitchMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 *result; /* the caller's slot: its value is taken, then it is set to -1; result */
    /* 0x54 */ s32 page;    /* page, 0..2 */
    /* 0x58 */ s32 page_shown; /* page as last shown */
    /* 0x5C */ s32 member;  /* member */
    /* 0x60 */ s32 member_id; /* gamestate_data.funcs.get_party_member(member) */
    /* 0x64 */ s32 picked;  /* entry picked */
    /* 0x68 */ s16 entries[4]; /* the entries: Digimon IDs */
    /* 0x70 */ s32 count;   /* entries */
    /* 0x74 */ s16 forms[3];  /* the member's chosen forms (gamestate_data.funcs.get_chosen_forms) */
    /* 0x7A */ u8 pad_7A[0x2];
    /* 0x7C */ s32 action;  /* the action (record of records_techniques, from 1), 0: none */
    /* 0x80 */ s32 *action_result; /* the caller's second slot: action */
} FightstgSwitchMenu; /* size 0x84 */

/* Its data block (base.children). */
typedef struct FightstgSwitchMenuData {
    /* 0x00 */ FightstgCursor *cursor; /* the list's cursor */
    /* 0x04 */ FightstgCursor *cursor_2; /* the second step's cursor */
    /* 0x08 */ FightstgStatus *status; /* fightstg_status_create's objects (two, swapped) */
    /* 0x0C */ FightstgStatus *status_2;
    /* 0x10 */ MessageWindow *names[4];  /* the entries' names */
    /* 0x20 */ MessageWindow *action_windows[5];
    /* 0x34 */ MessageWindow *mp_windows[4];
} FightstgSwitchMenuData; /* size 0x44 */

/* fightstg_message_create's object: the battle message box (two windows, shown one after the other). */
typedef struct FightstgMessage {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 queue[9]; /* queued messages: a message ID and its arguments, 0 ends */
    /* 0x74 */ s32 ready; /* set by fightstg_message_show: a message is ready */
    /* 0x78 */ s32 window; /* window being shown */
    /* 0x7C */ s32 window_count; /* windows used */
    /* 0x80 */ s32 window_frames; /* frames per window */
    /* 0x84 */ s32 window_time; /* time the window was shown */
    /* 0x88 */ s32 queue_index; /* index into queue */
    /* 0x8C */ u8 unk_8C[0x4];
    /* 0x90 */ s32 arrow_frame; /* "next" arrow animation frame, 0..4 */
    /* 0x94 */ s32 arrow_time; /* its last time */
    /* 0x98 */ s32 waiting; /* waiting for the button: the arrow is shown */
    /* 0x9C */ s32 members[3]; /* members listed (fightstg_message_list_members) */
    /* 0xA8 */ s32 member_count; /* their count */
    /* 0xAC */ void (*show)(); /* fightstg_message_show */
    /* 0xB0 */ void (*close)(); /* fightstg_message_close */
} FightstgMessage; /* size 0xB4 */

/* Its data block (base.children). */
typedef struct FightstgMessageData {
    /* 0x0 */ MessageWindow *windows[2]; /* the two windows */
} FightstgMessageData; /* size 0x8 */

typedef struct FightstgConfusedMenu {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 sel;
    /* 0x54 */ s32 selected; /* selected entry */
    /* 0x58 */ s32 *result; /* the caller's slot: set to -1 */
    /* 0x5C */ Object *portrait;
    /* 0x60 */ Object *idle_camera;
    /* 0x64 */ s32 queue[9]; /* 0-terminated, shown one by one (show) */
    /* 0x88 */ s32 ready;
    /* 0x8C */ s32 window; /* window being shown */
    /* 0x90 */ s32 windows; /* windows */
    /* 0x94 */ s32 frames_per_window; /* frames per window */
    /* 0x98 */ s32 window_time; /* time the window was shown */
    /* 0x9C */ s32 entry; /* entry of queue */
    /* 0xA0 */ u8 unk_A0[0x4];
    /* 0xA4 */ s32 cursor_frame; /* blinking cursor frame, 0..4 */
    /* 0xA8 */ s32 cursor_time; /* time of the last cursor frame */
    /* 0xAC */ s32 waiting; /* waiting for a button */
    /* 0xB0 */ u8 unk_B0[0x10];
    /* 0xC0 */ void (*show)(struct FightstgConfusedMenu *, s32, s32);
} FightstgConfusedMenu; /* size 0xC4 */

extern s32 fightstg_rules_opposite_elements[];

/* Entries of fightstg_jump_params (6 of them), indexed by fightstg_jump_create's arg1 - 1. */
typedef struct FightstgJumpParams {
    /* 0x0 */ s32 height;
    /* 0x4 */ s32 speed;
} FightstgJumpParams; /* size 0x8 */

extern FightstgJumpParams fightstg_jump_params[];

typedef struct FightstgJump {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 kind; /* kind, 1..6 */
    /* 0x54 */ s32 distance;
    /* 0x58 */ s32 z_offset;
    /* 0x5C */ FightstgModelParams *params; /* the model moved (its pos) */
    /* 0x60 */ s32 height; /* fightstg_jump_params[kind - 1].height */
    /* 0x64 */ s32 start_y; /* params->pos.y, at the start */
    /* 0x68 */ s32 speed; /* fightstg_jump_params[kind - 1].speed */
    /* 0x6C */ s32 time;
} FightstgJump; /* size 0x70 */

typedef struct FightstgSound {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 sound;
    /* 0x54 */ s32 key; /* voice (sound_play) */
    /* 0x58 */ s32 timer; /* timer, counted down by fightstg_battle.frames */
} FightstgSound; /* size 0x5C */

void fightstg_item_update();
void fightstg_counter_update();
void fightstg_tech_update();
void fightstg_scripted_turn_update();
void fightstg_boss_turn_update();
void fightstg_results_update();
void fightstg_model_view_update();
void fightstg_camera_update();
void fightstg_camera_set(FightstgCamera *obj, FightstgCameraSetting *params);
void fightstg_camera_move();
struct FightstgCameraSetting *fightstg_camera_get_preset();
struct FightstgCameraSetting *fightstg_camera_get_default();
void fightstg_command_update();
void fightstg_flash_update();
void fightstg_flash_draw(FightstgFlash *obj);
void fightstg_hud_update();
void fightstg_command_menu_update();
void fightstg_portrait_update();
void fightstg_status_update();
void fightstg_digivolve_menu_update();
void fightstg_member_menu_update();
void fightstg_switch_menu_update();
void fightstg_message_show();
void fightstg_message_close();
void fightstg_confused_menu_update();
void fightstg_jump_update();
void fightstg_message_draw(struct FightstgMessage *obj);
void fightstg_message_step(struct FightstgMessage *obj, struct FightstgMessageData *data);
void fightstg_events_add(FightstgNewEvent *event);
void fightstg_events_add_first(FightstgNewEvent *event);
s32 fightstg_events_get_delay(u8 side, s32 kind);
s32 fightstg_events_find_from(s32 i);
s32 fightstg_events_find_member(s32 type, u8 side, s32 member);
struct FightstgCursor *fightstg_cursor_create(struct FightstgCursorParams *params);
void fightstg_events_start_modifier(u8 side, s32 member, s32 kind, s32 tech);
void fightstg_command_menu_open(struct FightstgCommandMenu *obj);
void fightstg_battle_update_speed(void);
void fightstg_item_menu_update();
void fightstg_tech_menu_update();
void fightstg_cursor_update();
void fightstg_battle_draw_quad(s32 layer_id, s32 depth, DVECTOR *pos, CVECTOR *colors, s32 semi);
void *fightstg_models_get(s32 id);

/* An action's script parameters (fightstg_item_run_script's default case): -1 ends the table. */
typedef struct FightstgItemScript {
    /* 0x0 */ s16 action; /* action */
    /* 0x2 */ s16 effect;
    /* 0x4 */ s16 hit_sound;
} FightstgItemScript; /* size 0x6 */

extern FightstgItemScript fightstg_item_scripts[];

/* One step of the party's action `item` (fightstg_item_update): starts the battle script for it
 * (fightstg_script_create) and applies its effect. Returns 1 when done. */
s32 fightstg_item_run_script(FightstgItem *obj, FightstgScript **data) {
    FightstgMember *member;
    FightstgStats *p;
    s32 a;
    s32 b;
    s32 lim;
    s32 i;
    FightstgMember *target;
    RecordsTechnique *rec;

    switch (obj->base.substep) {
    case 0:
    default:
        *data = fightstg_script_create();
        (*data)->side = 0;
        switch (obj->item) {
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
            (*data)->script = 0xF;
            (*data)->stage = (obj->item - 0x4D) * 3 + 0x22;
            (*data)->effect = 2;
            (*data)->hit_sound = 0x39;
            break;
        case 0x54:
            obj->field_element = pad_random.next() % 7 + 2;
            (*data)->script = 0xF;
            (*data)->stage = (obj->field_element - 2) * 3 + 0x22;
            (*data)->effect = 2;
            (*data)->hit_sound = 0x39;
            break;
        case 0x56:
            (*data)->script = 0x11;
            (*data)->stage = -1;
            (*data)->effect = 0x15;
            (*data)->hit_sound = 0x1B;
            break;
        case 0x57:
            (*data)->script = 0x11;
            (*data)->stage = -1;
            (*data)->effect = 0x27;
            (*data)->hit_sound = 0x31;
            break;
        case 0x59:
            (*data)->script = 0x11;
            (*data)->stage = -1;
            (*data)->effect = 0x29;
            (*data)->hit_sound = 0x31;
            break;
        case 0x58:
            member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
            p = fightstg_rules.get_stats(0x10, 0, fightstg_battle.state.current[1]);
            if (p->stats[0] > p->stats[1]) {
                if (records_state.blocked[8] == 0) {
                    obj->lowered = 1;
                    a = p->stats[0];
                    b = p->stats[1];
                    if (member->modifiers[0] != 0) {
                        p->stats[0] -= member->modifiers[0];
                    }
                    lim = -(p->stats[0] / 2);
                    member->modifiers[0] -= a - b;
                    if (member->modifiers[0] < lim) {
                        member->modifiers[0] = lim;
                    }
                }
            } else if (p->stats[0] < p->stats[1] && records_state.blocked[9] == 0) {
                obj->lowered = 2;
                a = p->stats[0];
                b = p->stats[1];
                if (member->modifiers[1] != 0) {
                    p->stats[1] -= member->modifiers[1];
                }
                lim = -(p->stats[1] / 2);
                member->modifiers[1] -= b - a;
                if (member->modifiers[1] < lim) {
                    member->modifiers[1] = lim;
                }
            }
            (*data)->script = 0x11;
            (*data)->stage = -1;
            if (obj->lowered == 1) {
                (*data)->effect = 0x25;
                (*data)->hit_sound = 0x31;
            } else if (obj->lowered == 2) {
                (*data)->effect = 0x27;
                (*data)->hit_sound = 0x31;
            } else {
                (*data)->effect = 0x2E;
                (*data)->hit_sound = 0x1E;
            }
            break;
        case 0x55:
            (*data)->script = 0xE;
            (*data)->stage = -1;
            (*data)->effect = 0x1C;
            (*data)->hit_sound = 0x27;
            if (records_state.blocked[5] == 0) {
                FightstgMember *enemy = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];

                obj->amount = enemy->max_hp;
                obj->amount /= 5;
                if (enemy->hp - obj->amount <= 0) {
                    (*data)->results[3] = 2;
                    obj->base.timer = 1;
                } else {
                    (*data)->results[3] = 1;
                    wfightmn_update_idle_anim(0x10, obj->amount);
                    wfightmn_update_idle_anim(0, -obj->amount);
                }
            } else {
                (*data)->results[3] = 3;
            }
            break;
        case 0x5A:
            (*data)->script = 0xE;
            rec = &records_techniques[0x88];
            (*data)->stage = rec->anim_stage;
            (*data)->effect = rec->anim_effect;
            (*data)->hit_sound = rec->hit_sound;
            obj->amount = wfightmn_cap_damage(0, fightstg_rules.get_special_damage(0, 0x89), 0);
            target = fightstg_battle.state.members[1];
            if (obj->amount > 0) {
                if (target[fightstg_battle.state.current[1]].hp - obj->amount <= 0) {
                    (*data)->results[3] = 2;
                } else {
                    (*data)->results[3] = 1;
                    wfightmn_update_idle_anim(0x10, obj->amount);
                }
            } else {
                (*data)->results[3] = 3;
            }
            break;
        default:
            (*data)->script = 0xA;
            (*data)->stage = -1;
            for (i = 0; fightstg_item_scripts[i].action != -1; i++) {
                if (fightstg_item_scripts[i].action == obj->item) {
                    (*data)->effect = fightstg_item_scripts[i].effect;
                    (*data)->hit_sound = fightstg_item_scripts[i].hit_sound;
                    break;
                }
            }
            if (obj->item >= 0x2B && obj->item < 0x2F) {
                wfightmn_update_idle_anim(0, -((RecordsUsable *)records_funcs.get_item(obj->item)->data)->amount);
            } else if (obj->item == 0x47) {
                wfightmn_update_idle_anim(0, -((fightstg_battle.state.members[0] + fightstg_battle.state.current[0])->max_hp >> 1));
            }
            break;
        }
        obj->base.substep++;
        if (fightstg_battle.state.type == 6) {
            if (obj->item == 0x5A) {
                wfightmn_count_boss_hits(0, obj->amount);
            } else {
                wfightmn_check_final_phase_end(0);
            }
        }
        return 0;
    case 1:
        if (*data == NULL) {
            if (obj->item == 0x55 && obj->base.timer == 0 && records_state.blocked[5] == 0) {
                *data = fightstg_script_create();
                (*data)->side = 0;
                (*data)->script = 0xA;
                (*data)->effect = 0x21;
                (*data)->hit_sound = 0x1F;
                obj->base.substep++;
                break;
            }
            return 1;
        }
        break;
    case 2:
        if (*data == NULL) {
            return 1;
        }
        break;
    }
    return 0;
}

/* What a status-cure item (0x42-0x45) clears. */
typedef struct FightstgItemCure {
    /* 0x0 */ s16 message; /* message */
    /* 0x2 */ s16 status; /* FightstgMember.status flag */
    /* 0x4 */ s32 cure_id;
} FightstgItemCure; /* size 0x8 */

extern FightstgItemCure fightstg_item_cure_poison;
extern FightstgItemCure fightstg_item_cure_paralysis;
extern FightstgItemCure fightstg_item_cure_confusion;
extern FightstgItemCure fightstg_item_cure_all;

void fightstg_events_set_field_end(s32 delay);
void *fightstg_boss_turn_create(s32 arg0, s32 arg1);

/* The party uses item `item` in battle: its script (fightstg_item_run_script), then its effect and message, then
 * one fewer in the bag (gamestate_data.items). */
void fightstg_item_update(FightstgItem *obj, FightstgMessage **data) {
    u8 unused[0x90]; /* never referenced, but the frame has room for it */

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            *data = fightstg_message_create();
            obj->args[0] = 0;
            obj->args[1] = obj->item;
            (*data)->show(*data, 0xE, obj->args);
            obj->base.step++;
            break;
        case 1:
            if (*data == NULL && fightstg_item_run_script(obj, (FightstgScript **)data) != 0) {
                obj->base.next_state(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (*data == NULL) {
                obj->base.step++;
            }
            break;
        case 1:
            switch (obj->item) {
            case 0x2B:
            default: {
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                s32 n;

                if (member->max_hp == member->hp) {
                    *data = fightstg_message_create();
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                    obj->base.next_step(obj);
                } else {
                    if (member->max_hp >= member->hp + item->amount) {
                        n = item->amount;
                        member->hp += n;
                    } else {
                        n = member->max_hp - member->hp;
                        member->hp = member->max_hp;
                    }
                    *data = fightstg_message_create();
                    obj->args[0] = 0;
                    obj->args[1] = n;
                    (*data)->show(*data, 8, obj->args);
                    obj->base.next_step(obj);
                }
                break;
            }
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45: {
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                FightstgItemCure *cure;

                switch (obj->item) {
                case 0x42:
                default:
                    cure = &fightstg_item_cure_poison;
                    break;
                case 0x43:
                    cure = &fightstg_item_cure_paralysis;
                    break;
                case 0x44:
                    cure = &fightstg_item_cure_confusion;
                    break;
                case 0x45:
                    cure = &fightstg_item_cure_all;
                    break;
                }
                *data = fightstg_message_create();
                if (member->status & cure->status) {
                    member->status &= ~cure->status;
                    fightstg_events.cure_status(0, fightstg_battle.state.current[0], cure->cure_id);
                    obj->args[0] = cure->message;
                    obj->args[1] = 0;
                    (*data)->show(*data, 2, obj->args);
                } else {
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                }
                obj->base.next_step(obj);
                break;
            }
            case 0x46:
                switch (obj->base.substep) {
                case 0:
                    *data = fightstg_message_create();
                    obj->args[0] = 0;
                    obj->args[1] = 5;
                    (*data)->show(*data, 9, obj->args);
                    obj->base.next_substep(obj);
                    break;
                case 1:
                    if (*data == NULL) {
                        s32 i;

                        for (i = 0; i < 3; i++) {
                            if (fightstg_battle.state.members[0][i].hp == 0) {
                                fightstg_battle.state.members[0][i].hp = fightstg_battle.state.members[0][i].max_hp;
                                wfightmn_check_regen(i);
                            }
                        }
                        obj->base.next_substep(obj);
                    }
                    break;
                case 2:
                    if (*data == NULL) {
                        gamestate_data.items[obj->item]--;
                        obj->base.state = OBJECT_STATE_END;
                    }
                    break;
                }
                break;
            case 0x47: {
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];

                *data = fightstg_message_create();
                if (member->hp != member->max_hp || member->mp != member->max_mp) {
                    s32 a = member->max_hp / 2;
                    s32 b = member->max_mp / 2;

                    if (member->hp + a >= member->max_hp) {
                        member->hp = member->max_hp;
                    } else {
                        member->hp += a;
                    }
                    if (member->max_mp <= member->mp + b) {
                        member->mp = member->max_mp;
                    } else {
                        member->mp += b;
                    }
                    obj->args[0] = 0x3B;
                    obj->args[1] = 0;
                    (*data)->show(*data, 2, obj->args);
                } else {
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                }
                obj->base.next_step(obj);
                break;
            }
            case 0x48: {
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                FightstgStats *p = fightstg_rules.get_stats(0, 1, fightstg_battle.state.current[0]);
                s32 lim;

                if (member->modifiers[2] != 0) {
                    p->stats[4] -= member->modifiers[2];
                }
                lim = p->stats[4];
                member->modifiers[2] += lim * item->amount / 128;
                if (member->modifiers[2] > lim) {
                    member->modifiers[2] = lim;
                }
                fightstg_events_start_modifier(0, fightstg_battle.state.current[0], 2, 0);
                *data = fightstg_message_create();
                obj->args[0] = 0x32;
                obj->args[1] = 0;
                obj->args[2] = fightstg_battle.state.current[0];
                (*data)->show(*data, 7, obj->args);
                obj->base.next_step(obj);
                break;
            }
            case 0x49: {
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                FightstgStats *p = fightstg_rules.get_stats(0, 1, fightstg_battle.state.current[0]);
                s32 max;
                s32 min;

                if (member->modifiers[0] != 0) {
                    p->stats[0] -= member->modifiers[0];
                }
                if (member->modifiers[1] != 0) {
                    p->stats[1] -= member->modifiers[1];
                }
                max = p->stats[0];
                member->modifiers[0] += max * item->amount / 128;
                if (member->modifiers[0] > max) {
                    member->modifiers[0] = max;
                }
                min = -(p->stats[1] / 2);
                member->modifiers[1] -= p->stats[1] * item->amount / 512;
                if (member->modifiers[1] < min) {
                    member->modifiers[1] = min;
                }
                fightstg_events_start_modifier(0, fightstg_battle.state.current[0], 0, 0);
                fightstg_events_start_modifier(0, fightstg_battle.state.current[0], 1, 0);
                *data = fightstg_message_create();
                obj->args[0] = 0x3C;
                obj->args[1] = 0;
                (*data)->show(*data, 2, obj->args);
                obj->base.next_step(obj);
                break;
            }
            case 0x4A: {
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                FightstgStats *p = fightstg_rules.get_stats(0, 1, fightstg_battle.state.current[0]);
                s32 max;
                s32 min;

                if (member->modifiers[0] != 0) {
                    p->stats[0] -= member->modifiers[0];
                }
                if (member->modifiers[1] != 0) {
                    p->stats[1] -= member->modifiers[1];
                }
                max = p->stats[1];
                member->modifiers[1] += max * item->amount / 128;
                if (member->modifiers[1] > max) {
                    member->modifiers[1] = max;
                }
                min = -(p->stats[0] / 2);
                member->modifiers[0] -= p->stats[0] * item->amount / 512;
                if (member->modifiers[0] < min) {
                    member->modifiers[0] = min;
                }
                fightstg_events_start_modifier(0, fightstg_battle.state.current[0], 0, 0);
                fightstg_events_start_modifier(0, fightstg_battle.state.current[0], 1, 0);
                *data = fightstg_message_create();
                obj->args[0] = 0x3D;
                obj->args[1] = 0;
                (*data)->show(*data, 2, obj->args);
                obj->base.next_step(obj);
                break;
            }
            case 0x4B:
                (fightstg_battle.state.members[0] + fightstg_battle.state.current[0])->boosted = 1;
                *data = fightstg_message_create();
                obj->args[0] = 0x3E;
                obj->args[1] = 0;
                (*data)->show(*data, 2, obj->args);
                obj->base.next_step(obj);
                break;
            case 0x4C: {
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                s32 i = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);

                records_state.gauges[i] += item->amount;
                if (records_state.gauges[i] >= 999) {
                    records_state.gauges[i] = 999;
                }
                *data = fightstg_message_create();
                obj->args[0] = 0x3F;
                obj->args[1] = 0;
                (*data)->show(*data, 2, obj->args);
                obj->base.next_step(obj);
                break;
            }
            case 0x4D:
            case 0x4E:
            case 0x4F:
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
                fightstg_events_set_field_end(fightstg_events.get_delay(0, 8));
                fightstg_battle.state.field.element = obj->item - 0x4B;
                fightstg_battle.state.field.power = 0x40;
                *data = fightstg_message_create();
                obj->args[0] = obj->item + 0x16;
                (*data)->show(*data, 1, obj->args);
                obj->base.next_step(obj);
                break;
            case 0x54:
                fightstg_events_set_field_end(fightstg_events.get_delay(0, 8));
                fightstg_battle.state.field.element = obj->field_element;
                fightstg_battle.state.field.power = 0x7F;
                *data = fightstg_message_create();
                obj->args[0] = obj->field_element + 0x61;
                (*data)->show(*data, 1, obj->args);
                obj->base.next_step(obj);
                break;
            case 0x55:
                if (records_state.blocked[5] == 0) {
                    FightstgMember *member;

                    *data = fightstg_message_create();
                    obj->args[0] = 0x10;
                    obj->args[1] = obj->amount;
                    member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                    if (member->hp - obj->amount <= 0) {
                        (*data)->show(*data, 4, obj->args);
                        member->hp = 0;
                        fightstg_events_add_knockout(0x10);
                    } else {
                        (*data)->show(*data, 0x14, obj->args);
                        member->hp -= obj->amount;
                        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                        member->hp += obj->amount;
                        if (member->hp > member->max_hp) {
                            member->hp = member->max_hp;
                        }
                    }
                } else {
                    *data = fightstg_message_create();
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                }
                obj->base.next_step(obj);
                break;
            case 0x56: {
                RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                FightstgMember *other = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];

                switch (obj->base.substep) {
                case 0:
                default:
                    if ((pad_random.next() & 1) && records_state.blocked[2] == 0) {
                        fightstg_events_start_confusion(0x10, 0, item->amount);
                        *data = fightstg_message_create();
                        obj->args[0] = 0x20;
                        obj->args[1] = 0x10;
                        (*data)->show(*data, 2, obj->args);
                        if ((pad_random.next() & 3) == 0) {
                            obj->base.next_substep(obj);
                        } else {
                            obj->base.next_step(obj);
                        }
                        other->status |= 4;
                    } else {
                        *data = fightstg_message_create();
                        obj->args[0] = 0x2F;
                        (*data)->show(*data, 1, obj->args);
                        obj->base.next_step(obj);
                    }
                    break;
                case 1:
                    if (*data == NULL) {
                        member->status |= 4;
                        fightstg_events_start_confusion(0, 0, item->amount);
                        *data = fightstg_message_create();
                        obj->args[0] = 0x20;
                        obj->args[1] = 0;
                        (*data)->show(*data, 2, obj->args);
                        obj->base.next_step(obj);
                    }
                    break;
                }
                break;
            }
            case 0x57:
                if (records_state.blocked[9] == 0) {
                    FightstgMember *member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                    FightstgStats *p = fightstg_rules.get_stats(0x10, 0, fightstg_battle.state.current[1]);
                    s32 a = p->stats[0];
                    s32 b = p->stats[1];
                    s32 max;

                    if (member->modifiers[0] != 0) {
                        p->stats[0] -= member->modifiers[0];
                    }
                    if (member->modifiers[1] != 0) {
                        p->stats[1] -= member->modifiers[1];
                    }
                    member->modifiers[0] += a * 3 / 10;
                    member->modifiers[1] -= b / 2;
                    max = p->stats[0];
                    if (member->modifiers[0] > max) {
                        member->modifiers[0] = max;
                    }
                    if (member->modifiers[1] < -p->stats[1] / 2) {
                        member->modifiers[1] = -p->stats[1] / 2;
                    }
                    fightstg_events_start_modifier(0x10, fightstg_battle.state.current[1], 0, 0);
                    fightstg_events_start_modifier(0x10, fightstg_battle.state.current[1], 1, 0);
                    *data = fightstg_message_create();
                    obj->args[0] = 0x40;
                    obj->args[1] = 0x10;
                    (*data)->show(*data, 2, obj->args);
                    obj->base.next_step(obj);
                } else {
                    *data = fightstg_message_create();
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                    obj->base.next_step(obj);
                }
                break;
            case 0x58:
                switch (obj->lowered) {
                case 1:
                    fightstg_events_start_modifier(0x10, fightstg_battle.state.current[1], 0, 0);
                    *data = fightstg_message_create();
                    obj->args[0] = 0x33;
                    obj->args[1] = 0x10;
                    obj->args[2] = fightstg_battle.state.current[1];
                    (*data)->show(*data, 2, obj->args);
                    break;
                case 2:
                    fightstg_events_start_modifier(0x10, fightstg_battle.state.current[1], 1, 0);
                    *data = fightstg_message_create();
                    obj->args[0] = 0x34;
                    obj->args[1] = 0x10;
                    obj->args[2] = fightstg_battle.state.current[1];
                    (*data)->show(*data, 2, obj->args);
                    break;
                default:
                    *data = fightstg_message_create();
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                    break;
                }
                obj->base.next_step(obj);
                break;
            case 0x59:
                if (records_state.blocked[10] == 0) {
                    RecordsUsable *item = records_funcs.get_item(obj->item)->data;
                    FightstgMember *member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                    FightstgStats *p = fightstg_rules.get_stats(0x10, 0, fightstg_battle.state.current[1]);
                    s32 lim;

                    if (member->modifiers[2] != 0) {
                        p->stats[4] -= member->modifiers[2];
                    }
                    lim = -(p->stats[4] / 2);
                    member->modifiers[2] -= p->stats[4] * item->amount / 128;
                    if (member->modifiers[2] < lim) {
                        member->modifiers[2] = lim;
                    }
                    fightstg_events_start_modifier(0x10, fightstg_battle.state.current[1], 2, 0);
                    *data = fightstg_message_create();
                    obj->args[0] = 0x35;
                    obj->args[1] = 0x10;
                    obj->args[2] = fightstg_battle.state.current[1];
                    (*data)->show(*data, 2, obj->args);
                    obj->base.next_step(obj);
                } else {
                    *data = fightstg_message_create();
                    obj->args[0] = 0x2F;
                    (*data)->show(*data, 1, obj->args);
                    obj->base.next_step(obj);
                }
                break;
            case 0x5A:
                if (obj->amount > 0) {
                    FightstgMember *member;

                    *data = fightstg_message_create();
                    obj->args[0] = 0x10;
                    obj->args[1] = obj->amount;
                    (*data)->show(*data, 4, obj->args);
                    member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                    if (member->hp - obj->amount <= 0) {
                        member->hp = 0;
                        fightstg_events_add_knockout(0x10);
                    } else {
                        member->hp -= obj->amount;
                    }
                } else {
                    *data = fightstg_message_create();
                    obj->args[0] = 0x1D;
                    obj->args[1] = 0x10;
                    (*data)->show(*data, 2, obj->args);
                }
                obj->base.next_step(obj);
                break;
            }
            break;
        case 2:
            if (*data == NULL) {
                gamestate_data.items[obj->item]--;
                if (obj->item == 0x55 && fightstg_battle.state.type == 6) {
                    *data = fightstg_boss_turn_create(1, 0);
                    obj->base.next_step(obj);
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        case 3:
            if (*data == NULL) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgItem *) fightstg_item_create(s32 item) {
    FightstgItem *obj = object_new(fightstg_item_update, sizeof(FightstgItem), sizeof(FightstgMessage *));

    obj->item = item;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

s32 fightstg_enemy_check_condition(u8 type, s16 value);
s32 fightstg_enemy_get_action(u8 type);

/* The stage is computed as in WFIGHTMN's tech setup (`stage = n * 3 + 0x21`, + 1 for a strong element). wip-14: own
 * variables for case 0's side index and the damage block's target/victim; the enemy's copied tech reads
 * `(members[1] + current[1])->digimon` for the original's operand order (addu base,index). */
void fightstg_counter_update(FightstgCounter *obj, FightstgMessage **data) {
    FightstgMember *member;
    RecordsTechnique *rec;
    FightstgEnemyRecord *digimon;
    FightstgStats *values;
    FightstgMember *members;
    s32 side;
    s32 j;
    s32 n;
    s32 own;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        if (obj->damage_taken == 0) {
            obj->base.state = OBJECT_STATE_DONE;
            break;
        }
        own = obj->side != 0;
        member = &fightstg_battle.state.members[own][fightstg_battle.state.current[own]];
        if ((member->status & 2) && fightstg_rules.roll_paralyzed(obj->side) != 0) {
            obj->base.state = OBJECT_STATE_DONE;
            break;
        }
        if (obj->side == 0) {
            j = fightstg_events.find_member(8, 0, fightstg_battle.state.current[0]);
            if (j >= 0) {
                obj->tech = fightstg_events.events[j].args[2];
                obj->base.step = 0;
                fightstg_events.events[j].type = 0;
            } else {
                if (fightstg_rules.get_stats(0, 1, fightstg_battle.state.current[0])->counter == 0) {
                    obj->base.state = OBJECT_STATE_DONE;
                    break;
                }
                if (fightstg_rules.roll_counter(0, obj->damage_taken) != 0) {
                    obj->tech = records_get_digimon_func(member->digimon)->techniques[0];
                    obj->base.step = 1;
                } else {
                    obj->base.state = OBJECT_STATE_DONE;
                    break;
                }
            }
        } else {
            digimon = fightstg_enemy_records.get(member->digimon);
            if (digimon->counter_condition == 0) {
                obj->base.state = OBJECT_STATE_DONE;
                break;
            }
            if (fightstg_enemy_check_condition(digimon->counter_condition, digimon->counter_value) == 0) {
                obj->base.state = OBJECT_STATE_DONE;
                break;
            }
            n = fightstg_enemy_get_action(digimon->counter_action);
            obj->tech = n;
            if (n == 1) {
                obj->tech = fightstg_enemy_records.get((fightstg_battle.state.members[1] + fightstg_battle.state.current[1])->digimon)->tech;
                obj->base.step = n;
            }
        }
        *data = fightstg_message_create();
        obj->args[0] = obj->side;
        obj->args[1] = obj->tech;
        (*data)->show(*data, obj->base.step + 5, obj->args);
        obj->hit = fightstg_rules.roll_hit(obj->side, obj->tech);
        if (obj->hit != 0) {
            obj->damage = fightstg_rules.get_counter_damage(obj->side, obj->tech, obj->damage_taken);
            if (fightstg_battle.state.type != 0) {
                obj->damage = wfightmn_cap_damage(obj->side, obj->damage, 0);
            }
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (*data == NULL) {
                rec = &records_techniques[obj->tech - 1];
                side = obj->side != 0;
                *data = (FightstgMessage *)fightstg_script_create();
                ((FightstgScript *)*data)->side = obj->side;
                if (obj->side == 0 && rec->anim_script == 5) {
                    ((FightstgScript *)*data)->script = 6;
                } else {
                    ((FightstgScript *)*data)->script = rec->anim_script;
                }
                if (rec->hit_sound != 0) {
                    ((FightstgScript *)*data)->hit_sound = rec->hit_sound;
                }
                values = fightstg_rules.get_stats(obj->side, 1, fightstg_battle.state.current[side]);
                if (rec->defense_stat >= 2 || values->attack_element >= 2) {
                    s32 level;
                    s32 stage;

                    if (rec->defense_stat >= 2) {
                        level = rec->defense_stat - 2;
                    } else {
                        level = values->attack_element - 2;
                    }
                    stage = level * 3 + 0x21;
                    if (values->attack_element_power >= 0x40) {
                        ((FightstgScript *)*data)->stage = stage + 1;
                    } else {
                        ((FightstgScript *)*data)->stage = stage;
                    }
                } else {
                    ((FightstgScript *)*data)->stage = -1;
                }
                if (rec->anim_effect != 0) {
                    ((FightstgScript *)*data)->effect = rec->anim_effect;
                } else if (rec->anim_script == 5) {
                    ((FightstgScript *)*data)->effect = 0x2E;
                    ((FightstgScript *)*data)->hit_sound = 0x1E;
                }
                members = fightstg_battle.state.members[1 - side];
                if (obj->hit != 0) {
                    if (members[fightstg_battle.state.current[1 - side]].hp - obj->damage <= 0) {
                        ((FightstgScript *)*data)->results[3] = 2;
                    } else {
                        ((FightstgScript *)*data)->results[3] = 1;
                        wfightmn_count_boss_hits(obj->side, obj->damage);
                        wfightmn_update_idle_anim((u8)(0x10 - obj->side), obj->damage);
                    }
                } else {
                    ((FightstgScript *)*data)->results[3] = 3;
                }
                members = obj->side != 0 ? fightstg_battle.state.members[1] : fightstg_battle.state.members[0];
                members[fightstg_battle.state.current[obj->side != 0]].power_up = 0;
                obj->base.step++;
            }
            break;
        case 1:
            if (*data == NULL) {
                *data = fightstg_message_create();
                if (obj->hit != 0) {
                    obj->args[0] = (obj->side == 0) << 4;
                    obj->args[1] = obj->damage;
                    (*data)->show(*data, 4, obj->args);
                } else {
                    obj->args[0] = 0x1D;
                    obj->args[1] = (obj->side == 0) << 4;
                    (*data)->show(*data, 2, obj->args);
                }
                if (obj->damage != 0) {
                    s32 target;
                    FightstgMember *victim;

                    target = obj->side == 0;
                    victim = &fightstg_battle.state.members[target][fightstg_battle.state.current[target]];
                    victim->hp -= obj->damage;
                    if (victim->hp <= 0) {
                        victim->hp = 0;
                        if (obj->no_knockout == 0) {
                            fightstg_events_add_knockout(target << 4);
                        }
                    }
                }
                obj->base.step++;
            }
            break;
        case 2:
            if (*data == NULL) {
                wfightmn_add_gauge(obj->side, obj->damage_taken);
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

void *fightstg_counter_create(s8 side, s32 damage_taken, s32 no_knockout) {
    FightstgCounter *obj = object_new(fightstg_counter_update, sizeof(FightstgCounter), sizeof(FightstgMessage *));

    obj->side = side;
    obj->damage_taken = damage_taken;
    obj->no_knockout = no_knockout;
    return obj;
}

/* Event types taken off the queue when a party member is revived. */
extern s32 fightstg_revive_event_types[6];

/* Revives party member `i` with item `rec` (a record of records_techniques, from 1). */
void fightstg_revive_member(s32 rec, s32 i) {
    FightstgMember *party = fightstg_battle.state.members[0];
    RecordsTechnique *item = &records_techniques[rec - 1];
    s32 k;
    s32 j;

    if (party[i].digimon != 0) {
        for (k = 0; k < 6; k++) {
            j = fightstg_events.find_member(fightstg_revive_event_types[k], 0, i);
            if (j >= 0) {
                fightstg_events.events[j].type = 0;
            }
        }
        if (party[i].hp == 0) {
            wfightmn_check_regen(i);
        }
        party[i].status = 0;
        party[i].hp = party[i].max_hp;
        fightstg_rules.change_modifier(0, i, 1, item->effect_power);
        fightstg_events_start_modifier(0, i, 1, rec);
    }
}

/* A status technique's effect (fightstg_tech_update): technique `technique` changes stat `stat` by `factor` times its
 * power, with message `message`; -1 ends the table. */
typedef struct FightstgTechStatEffect {
    /* 0x0 */ s16 technique; /* technique */
    /* 0x2 */ s16 factor; /* <= 0: on the other side */
    /* 0x4 */ s16 stat;  /* stat */
    /* 0x6 */ s16 message; /* message */
} FightstgTechStatEffect; /* size 0x8 */

extern FightstgTechStatEffect fightstg_tech_stat_effects[];
extern FightstgTechStatEffect fightstg_tech_party_stat_effects[];
extern u8 fightstg_tech_cure_flags[];
extern s32 fightstg_tech_cure_kinds[];
extern s32 fightstg_tech_cure_messages[];

void fightstg_events_add_regen_end(u8 side);
void fightstg_events_start_regen(u8 side, s32 arg1, s32 arg2);
void fightstg_events_add_counter_stance(s32 arg0);
void fightstg_events_add_revert(void);

void fightstg_tech_update(FightstgTech *obj, FightstgMessage **data) {
    RecordsTechnique *rec;
    s32 side;
    FightstgMember *member;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        rec = &records_techniques[obj->tech - 1];
        if (rec->element == 2 || rec->element == 3) {
            fightstg_action.run(obj->side, obj->tech);
        } else {
            heap_funcs.bzero(&fightstg_action, 0x68); /* PC_PORT: a byte count: the state before `run` (pointer-free) */
        }
        *data = fightstg_message_create();
        if (rec->anim_script == 0xC) {
            obj->args[0] = 0x3A;
            obj->args[1] = obj->side;
            (*data)->show(*data, 2, obj->args);
        } else {
            obj->args[0] = obj->side;
            obj->args[1] = obj->tech;
            (*data)->show(*data, 3, obj->args);
        }
        side = 1 - (obj->side >> 4);
        member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
        if (member->status & 8) {
            obj->target_asleep = 1;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (*data == NULL) {
                if (fightstg_action.effects[0xA] == 0) {
                    FightstgMember *members;

                    *data = wfightmn_tech_script_create(obj->side, obj->tech);
                    members = obj->side != 0 ? fightstg_battle.state.members[1] : fightstg_battle.state.members[0];
                    members[fightstg_battle.state.current[obj->side != 0]].power_up = 0;
                } else {
                    wfightmn_check_final_phase_end(obj->side);
                }
                obj->base.step++;
            }
            break;
        case 1:
            if (*data == NULL) {
                RecordsTechnique *rec = &records_techniques[obj->tech - 1];
                switch (rec->element) {
                case 2:
                case 3: {
                    s32 side;

                    *data = fightstg_message_create();
                    side = obj->side == 0;
                    if (fightstg_action.effects[0xA] != 0) {
                        fightstg_events_add_counter_stance(obj->tech);
                        obj->base.state = OBJECT_STATE_END;
                        break;
                    }
                    if (fightstg_action.effects[9] != 0) {
                        if (records_techniques[obj->tech - 1].kind == 0x1F) {
                            obj->damage = fightstg_action.damages[0] + fightstg_action.damages[1];
                            obj->args[0] = side << 4;
                            obj->args[1] = obj->damage;
                            (*data)->show(*data, 4, obj->args);
                        } else {
                            obj->args[0] = side << 4;
                            obj->args[1] = fightstg_action.damage;
                            obj->args[2] = (s16)fightstg_action.hit_count;
                            (*data)->show(*data, 0x10, obj->args);
                            obj->damage = fightstg_action.damage * (s16)fightstg_action.hit_count;
                            if (obj->damage >= 10000) {
                                obj->damage = 9999;
                            }
                        }
                    } else if (fightstg_action.effects[6] != 0) {
                        FightstgMember *p = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];

                        p->hp = 0;
                        fightstg_events_add_knockout(side << 4);
                        (*data)->base.state = OBJECT_STATE_END;
                    } else if (fightstg_action.hits[0] != 0) {
                        obj->args[0] = side << 4;
                        obj->args[1] = fightstg_action.damage;
                        (*data)->show(*data, 4, obj->args);
                        obj->damage = fightstg_action.damage;
                    } else {
                        obj->args[0] = 0x1D;
                        obj->args[1] = side << 4;
                        (*data)->show(*data, 2, obj->args);
                    }
                    if (obj->damage != 0) {
                        FightstgMember *member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];

                        member->hp -= obj->damage;
                        if (member->hp <= 0) {
                            member->hp = 0;
                            fightstg_events_add_knockout(side << 4);
                            obj->base.substep = 1;
                        }
                    }
                    obj->base.step++;
                    break;
                }
                case 4:
                    obj->args[0] = obj->side;
                    switch (obj->tech) {
                    case 0xBD:
                        fightstg_events_add_regen_end(obj->side);
                        fightstg_events_start_regen(obj->side, fightstg_battle.state.current[obj->side != 0], obj->tech);
                        *data = fightstg_message_create();
                        obj->args[0] = 0x27;
                        obj->args[1] = obj->side;
                        (*data)->show(*data, 2, obj->args);
                        obj->base.next_step(obj);
                        break;
                    case 0xBE:
                    case 0xBF:
                    case 0xC0:
                    case 0xC1:
                    case 0xC2:
                    case 0xC3:
                    case 0xC4:
                    case 0xC5: {
                        s32 side = obj->side != 0;
                        s32 k;
                        FightstgMember *members;

                        switch (obj->tech) {
                        case 0xBE:
                        case 0xBF:
                        default:
                            k = 0;
                            break;
                        case 0xC0:
                        case 0xC1:
                            k = 1;
                            break;
                        case 0xC2:
                        case 0xC3:
                            k = 2;
                            break;
                        case 0xC4:
                        case 0xC5:
                            k = 3;
                            break;
                        }
                        members = fightstg_battle.state.members[side];
                        switch (obj->base.substep) {
                        case 0:
                        default:
                            *data = fightstg_message_create();
                            if (obj->tech & 1) {
                                obj->args[0] = obj->side;
                                obj->args[1] = fightstg_tech_cure_kinds[k];
                                (*data)->show(*data, 9, obj->args);
                                obj->base.substep = 1;
                                obj->base.timer = 3;
                                return;
                            }
                            if (members[fightstg_battle.state.current[side]].status & fightstg_tech_cure_flags[k]) {
                                obj->args[0] = fightstg_tech_cure_messages[k];
                                obj->args[1] = obj->side;
                                obj->args[2] = fightstg_battle.state.current[0];
                                (*data)->show(*data, 2, obj->args);
                                obj->base.substep = 1;
                                obj->base.timer = 1;
                                return;
                            }
                            obj->args[0] = 0x2F;
                            (*data)->show(*data, 1, obj->args);
                            obj->base.next_step(obj);
                            break;
                        case 1: {
                            s32 i;

                            for (i = 0; i < obj->base.timer; i++) {
                                fightstg_events.cure_status(obj->side, i, obj->tech);
                            }
                            obj->base.next_step(obj);
                            break;
                        }
                        }
                        break;
                    }
                    case 0x64: {
                        FightstgMember *members = fightstg_battle.state.members[0];

                        switch (obj->base.substep) {
                        case 0:
                        default:
                            *data = fightstg_message_create();
                            obj->args[0] = 0;
                            obj->args[1] = 5;
                            (*data)->show(*data, 9, obj->args);
                            obj->base.substep++;
                            break;
                        case 1: {
                            s32 i;

                            for (i = 0; i < 3; i++) {
                                if (members[i].digimon != 0 && members[i].hp == 0) {
                                    members[i].hp = members[i].max_hp;
                                    wfightmn_check_regen(i);
                                }
                            }
                            obj->base.next_step(obj);
                            break;
                        }
                        }
                        break;
                    }
                    case 0x177:
                        switch (obj->base.substep) {
                        case 0:
                        default:
                            *data = fightstg_message_create();
                            obj->args[0] = 0;
                            obj->args[1] = 6;
                            (*data)->show(*data, 9, obj->args);
                            obj->base.substep++;
                            break;
                        case 1: {
                            s32 i;

                            for (i = 0; i < 3; i++) {
                                fightstg_revive_member(0x177, i);
                            }
                            obj->base.next_step(obj);
                            break;
                        }
                        }
                        break;
                    default: {
                        FightstgMember *members = obj->side != 0 ? fightstg_battle.state.members[1] : fightstg_battle.state.members[0];

                        switch (obj->base.substep) {
                        case 0:
                        default: {
                            s32 msg;
                            s32 n;
                            s32 j;
                            s32 i;

                            msg = -1;
                            obj->healed = fightstg_rules.get_heal(obj->side, obj->tech);
                            n = 0;
                            if (obj->tech >= 0xBB) {
                                for (i = 0; i < 3; i++) {
                                    if (members[i].digimon != 0 && members[i].hp != 0) {
                                        if (n < members[i].max_hp - members[i].hp) {
                                            n = members[i].max_hp - members[i].hp;
                                        }
                                    }
                                }
                                if (n != 0) {
                                    msg = 9;
                                    if (obj->healed < n) {
                                        n = obj->healed;
                                    }
                                    obj->args[1] = 0;
                                    obj->args[2] = n;
                                }
                            } else {
                                i = fightstg_battle.state.current[obj->side != 0];
                                if (n < members[i].max_hp - members[i].hp) {
                                    n = members[i].max_hp - members[i].hp;
                                }
                                if (n != 0) {
                                    msg = 8;
                                    if (obj->healed < n) {
                                        n = obj->healed;
                                    }
                                    obj->healed = n;
                                    obj->args[1] = n;
                                }
                            }
                            *data = fightstg_message_create();
                            obj->args[0] = obj->side;
                            if (msg != -1) {
                                (*data)->show(*data, msg, obj->args);
                                if (obj->tech >= 0xBB) {
                                    for (j = 0; j < 3; j++) {
                                        if (members[j].digimon != 0 && members[j].hp != 0) {
                                            if (members[j].hp + obj->healed > members[j].max_hp) {
                                                members[j].hp = members[j].max_hp;
                                            } else {
                                                members[j].hp += obj->healed;
                                            }
                                        }
                                    }
                                } else {
                                    j = fightstg_battle.state.current[obj->side != 0];
                                    members[j].hp = members[j].hp + obj->healed > members[j].max_hp ? members[j].max_hp : members[j].hp + obj->healed;
                                }
                            } else {
                                obj->args[0] = 0x2F;
                                obj->args[1] = obj->side;
                                (*data)->show(*data, 2, obj->args);
                            }
                            obj->base.next_step(obj);
                            break;
                        }
                        case 1:
                            break;
                        }
                        break;
                    }
                    }
                    break;
                case 5:
                    *data = fightstg_message_create();
                    switch (obj->tech) {
                    case 0xC6:
                    case 0xC8:
                    case 0xCA:
                    case 0xCC:
                    case 0xCD:
                    case 0xCE:
                    case 0xCF:
                    case 0xD0: {
                        FightstgTechStatEffect *e;
                        s32 ok;
                        u32 side;

                        for (e = fightstg_tech_stat_effects; e->technique != -1; e++) {
                            if (e->technique == obj->tech) {
                                break;
                            }
                        }
                        ok = 1;
                        side = obj->side >> 4;
                        if (e->factor <= 0) {
                            side ^= 1;
                            if (obj->side == 0 && records_state.blocked[e->stat + 8] != 0) {
                                obj->args[0] = 0x2F;
                                (*data)->show(*data, 1, obj->args);
                                ok = 0;
                            }
                        }
                        if (ok) {
                            fightstg_rules.change_modifier(side << 4, fightstg_battle.state.current[side], e->stat, e->factor * rec->effect_power);
                            fightstg_events_start_modifier(side << 4, fightstg_battle.state.current[side], e->stat, obj->tech);
                            obj->args[0] = e->message;
                            obj->args[1] = side << 4;
                            obj->args[2] = fightstg_battle.state.current[side];
                            (*data)->show(*data, 2, obj->args);
                        }
                        break;
                    }
                    case 0xC7:
                    case 0xC9:
                    case 0xCB: {
                        FightstgTechStatEffect *e;
                        s32 i;

                        for (e = fightstg_tech_party_stat_effects; e->technique != -1; e++) {
                            if (e->technique == obj->tech) {
                                break;
                            }
                        }
                        for (i = 0; i < 3; i++) {
                            if (fightstg_battle.state.members[0][(obj->side >> 4) * 3 + i].digimon != 0 &&
                                fightstg_battle.state.members[0][(obj->side >> 4) * 3 + i].hp > 0) {
                                fightstg_rules.change_modifier(obj->side, i, e->stat, e->factor * rec->effect_power);
                                fightstg_events_start_modifier(obj->side, i, e->stat, obj->tech);
                            }
                        }
                        obj->args[0] = obj->side;
                        switch (e->stat) {
                        case 0:
                            obj->args[1] = 7;
                            break;
                        case 1:
                            obj->args[1] = 8;
                            break;
                        case 2:
                            obj->args[1] = 9;
                            break;
                        }
                        (*data)->show(*data, 9, obj->args);
                        break;
                    }
                    case 0xD1:
                    case 0xD2: {
                        s32 s = obj->side >> 4;
                        FightstgMember *p = &fightstg_battle.state.members[s][fightstg_battle.state.current[s]];

                        p->power_up = rec->effect_power;
                        obj->args[0] = 0x36;
                        obj->args[1] = obj->side;
                        (*data)->show(*data, 2, obj->args);
                        break;
                    }
                    case 0xD3:
                        if (fightstg_rules.roll_switch_seal(0x10, 0xD3) != 0) {
                            fightstg_events_start_seal(0xD3);
                            obj->args[0] = 0x44;
                            obj->args[1] = 0;
                            (*data)->show(*data, 2, obj->args);
                        } else {
                            obj->args[0] = 0x2F;
                            (*data)->show(*data, 1, obj->args);
                        }
                        break;
                    case 0xD4:
                        if (fightstg_rules.roll_digivolve_seal(0x10, 0xD4) != 0) {
                            fightstg_events_start_seal(0xD4);
                            obj->args[0] = 0x45;
                            obj->args[1] = 0;
                            (*data)->show(*data, 2, obj->args);
                        } else {
                            obj->args[0] = 0x2F;
                            (*data)->show(*data, 1, obj->args);
                        }
                        break;
                    case 0x187:
                        if (fightstg_rules.roll_revert(0x10, 0x187) != 0) {
                            fightstg_events_add_revert();
                            (*data)->base.state = OBJECT_STATE_END;
                        } else {
                            obj->args[0] = 0x2F;
                            (*data)->show(*data, 1, obj->args);
                        }
                        break;
                    case 0x188: {
                        FightstgMember *other = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                        FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                        s32 n;

                        if (member->mp != 0) {
                            n = member->max_mp * rec->effect_power / 128;
                            if (n > member->mp) {
                                n = member->mp;
                            }
                            other->mp += n;
                            member->mp -= n;
                            if (other->mp > other->max_mp) {
                                other->mp = other->max_mp;
                            }
                            if (member->mp < 0) {
                                member->mp = 0;
                            }
                            obj->args[0] = 0x10;
                            obj->args[1] = n;
                            obj->args[2] = 1;
                            (*data)->show(*data, 0x12, obj->args);
                        } else {
                            obj->args[0] = 0x2F;
                            obj->args[1] = 0x10;
                            (*data)->show(*data, 2, obj->args);
                        }
                        break;
                    }
                    case 0x190: {
                        FightstgMember *member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];

                        obj->healed = fightstg_rules.get_heal(obj->side, 0x190);
                        member->hp += obj->healed;
                        if (member->hp > member->max_hp) {
                            member->hp = member->max_hp;
                        }
                        member->power_up = 0x20;
                        obj->args[0] = 0x46;
                        obj->args[1] = 0x10;
                        (*data)->show(*data, 2, obj->args);
                        break;
                    }
                    }
                    obj->base.next_step(obj);
                    break;
                case 6: {
                    FightstgStats *p;
                    s32 n;

                    *data = fightstg_message_create();
                    p = fightstg_rules.get_stats(obj->side, 1, fightstg_battle.state.current[obj->side == 0x10]);
                    fightstg_battle.state.field.element = rec->element;
                    n = (s16)(p->stats[2] / 10) + rec->effect_power;
                    if (n >= 0x80) {
                        n = 0x7F;
                    }
                    fightstg_battle.state.field.power = n;
                    fightstg_events_set_field_end(p->stats[2] * 12 + 1000);
                    obj->args[0] = rec->defense_stat + 0x61;
                    (*data)->show(*data, 1, obj->args);
                    obj->base.next_step(obj);
                    break;
                }
                }
            }
            break;
        case 2:
            if (*data == NULL) {
                RecordsTechnique *rec = &records_techniques[obj->tech - 1];
                if ((rec->element == 2 || rec->element == 3) && obj->base.substep == 0) {
                    if (obj->damage == 0) {
                        if (obj->side == 0 && fightstg_battle.state.type == 6) {
                            obj->base.set_step(obj, 7);
                            break;
                        }
                    } else {
                        *data = (FightstgMessage *)fightstg_results_create(obj->side);
                        obj->base.next_step(obj);
                        break;
                    }
                }
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 3:
            if (*data == NULL) {
                obj->base.step++;
            }
            break;
        case 4: {
            s32 side = obj->side == 0;
            FightstgMember *member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
            RecordsTechnique *rec;

            rec = &records_techniques[obj->tech - 1];
            if (member->status & 8) {
                if (obj->target_asleep != 0 && fightstg_rules.roll_wake(side << 4, obj->damage) != 0) {
                    s32 j;

                    obj->args[0] = 0x2B;
                    obj->args[1] = 0x10 - obj->side;
                    obj->args[2] = fightstg_battle.state.current[side];
                    *data = fightstg_message_create();
                    (*data)->show(*data, 7, obj->args);
                    member->status &= ~8;
                    j = fightstg_events.find_member(0xC, 0x10 - obj->side, fightstg_battle.state.current[side]);
                    if (j >= 0) {
                        fightstg_events.events[j].type = 0;
                    }
                    obj->base.step = 9;
                    break;
                }
                obj->base.state = OBJECT_STATE_END;
                break;
            }
            if (rec->element == 2) {
                *data = fightstg_counter_create((obj->side == 0) << 4, obj->damage, rec->anim_script == 0xC);
                obj->base.step = 6;
            } else {
                obj->base.step = 5;
            }
            break;
        }
        case 6:
            if (*data == NULL) {
                obj->base.state = OBJECT_STATE_END;
                break;
            }
            if ((*data)->base.state != OBJECT_STATE_DONE) {
                break;
            }
        case 5:
            wfightmn_add_gauge(obj->side, obj->damage);
            obj->base.state = OBJECT_STATE_END;
            break;
        case 7:
            if (*data == NULL) {
                RecordsTechnique *rec = &records_techniques[obj->tech - 1];
                *data = fightstg_boss_turn_create(1, rec->anim_script == 0xC);
                obj->base.step++;
            }
            break;
        case 8:
            if (*data == NULL) {
                obj->base.set_state(obj, OBJECT_STATE_END);
            }
            break;
        case 9:
            if (*data == NULL) {
                RecordsTechnique *rec = &records_techniques[obj->tech - 1];
                if (rec->element == 2) {
                    *data = fightstg_counter_create((obj->side == 0) << 4, obj->damage, rec->anim_script == 0xC);
                    obj->base.step = 6;
                } else {
                    obj->base.step = 5;
                }
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgTech *) fightstg_tech_create(s8 side, s32 tech) {
    FightstgTech *obj = object_new(fightstg_tech_update, sizeof(FightstgTech), sizeof(FightstgMessage *));

    obj->side = side;
    obj->tech = tech;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* The scripted battle's enemy turn (battle type 5): the enemy uses its tech_2, which leaves the party's member at
 * 1 HP, then the boss enters (fightstg_events_add_boss_entrance). */
typedef struct FightstgScriptedTurn {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 args[3]; /* the message's arguments (fightstg_message_show's args); [1] is also the technique used */
    /* 0x5C */ u8 unk_5C[0x14];
} FightstgScriptedTurn; /* size 0x70 */

void fightstg_scripted_turn_update(FightstgScriptedTurn *obj, FightstgMessage **data) {
    FightstgEnemyRecord *digimon;
    FightstgMember *member;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        digimon = fightstg_enemy_records.get(fightstg_battle.state.members[1][fightstg_battle.state.current[1]].digimon);
        *data = fightstg_message_create();
        obj->args[0] = 0x10;
        obj->args[1] = digimon->tech_2;
        (*data)->show(*data, 3, obj->args);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            if (*data == NULL) {
                *data = wfightmn_tech_script_create(0x10, obj->args[1]);
                obj->base.step++;
            }
            break;
        case 1:
            if (*data == NULL) {
                member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                *data = fightstg_message_create();
                obj->args[0] = 0;
                obj->args[1] = member->hp - 1;
                (*data)->show(*data, 4, obj->args);
                member->hp = 1;
                obj->base.step++;
            }
            break;
        case 2:
            if (*data == NULL) {
                fightstg_events_add_enemy_turn(fightstg_events.get_delay(0x10, 0));
                fightstg_events_add_boss_entrance();
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgScriptedTurn *) fightstg_scripted_turn_create(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_new(fightstg_scripted_turn_update, sizeof(FightstgScriptedTurn), sizeof(FightstgMessage *)))
}

void fightstg_boss_turn_update(FightstgBossTurn *obj, FightstgMessage **data) {
    FightstgEnemyRecord *digimon = fightstg_enemy_records.get(fightstg_battle.state.members[1][fightstg_battle.state.current[1]].digimon);
    s32 j;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.set_state(obj, obj->counter + 1);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            fightstg_events_add_enemy_turn(fightstg_events.get_delay(0x10, 0));
            if (fightstg_battle.state.final_phase != 0) {
                if (fightstg_events.find(0x18) >= 0) {
                    obj->base.set_state(obj, OBJECT_STATE_END);
                } else {
                    *data = fightstg_message_create();
                    obj->args[0] = 8;
                    obj->args[1] = 0x10;
                    obj->tech = digimon->tech_3;
                    (*data)->show(*data, 2, obj->args);
                    fightstg_battle.state.final_phase = 0;
                    fightstg_battle.state.hits = 0;
                    obj->restore_idle = 1;
                }
            } else {
                *data = fightstg_message_create();
                obj->args[0] = 0x10;
                obj->args[1] = fightstg_battle.state.copied_tech != 0 ? fightstg_battle.state.copied_tech : digimon->tech;
                (*data)->show(*data, 3, obj->args);
                obj->tech = digimon->tech;
            }
            if ((fightstg_battle.state.members[0] + fightstg_battle.state.current[0])->status & 8) {
                obj->target_asleep = 1;
            }
            obj->base.step++;
            break;
        case 1:
            if (*data == NULL) {
                fightstg_action.run(0x10, obj->tech);
                *data = wfightmn_tech_script_create(0x10, obj->tech);
                if (obj->restore_idle != 0) {
                    wfightmn_update_idle_anim(0x10, 0);
                    obj->restore_idle = 0;
                }
                obj->base.step++;
            }
            break;
        case 2:
            if (*data == NULL) {
                *data = fightstg_message_create();
                if (fightstg_action.hits[0] != 0) {
                    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];

                    if (fightstg_action.effects[6] != 0) {
                        member->hp = 0;
                        (*data)->base.state = OBJECT_STATE_END;
                    } else {
                        obj->args[0] = 0;
                        obj->args[1] = fightstg_action.damage;
                        (*data)->show(*data, 4, obj->args);
                        member->hp -= fightstg_action.damage;
                        if (member->hp <= 0) {
                            member->hp = 0;
                        }
                    }
                    obj->base.step++;
                } else {
                    obj->args[0] = 0x1D;
                    obj->args[1] = 0;
                    (*data)->show(*data, 2, obj->args);
                    obj->base.step = 5;
                }
            }
            break;
        case 3:
            if (*data == NULL) {
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];

                if (fightstg_action.effects[6] != 0) {
                    member->hp = 0;
                    fightstg_events_add_knockout(0);
                    obj->base.state = OBJECT_STATE_END;
                } else if (member->hp <= 0) {
                    member->hp = 0;
                    fightstg_events_add_knockout(0);
                    obj->base.state = OBJECT_STATE_END;
                } else {
                    *data = (FightstgMessage *)fightstg_results_create(0x10);
                    obj->base.step++;
                }
            }
            break;
        case 4:
            if (*data == NULL) {
                FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];

                if (!(member->status & 8)) {
                    obj->base.step = 5;
                } else if (obj->target_asleep != 0 && fightstg_rules.roll_wake(0, fightstg_action.damage) != 0) {
                    obj->args[0] = 0x2B;
                    obj->args[1] = 0;
                    obj->args[2] = fightstg_battle.state.current[0];
                    *data = fightstg_message_create();
                    (*data)->show(*data, 7, obj->args);
                    member->status &= ~8;
                    j = fightstg_events.find_member(0xC, 0, fightstg_battle.state.current[0]);
                    if (j >= 0) {
                        fightstg_events.events[j].type = 0;
                    }
                    obj->base.step = 5;
                } else {
                    obj->base.state = OBJECT_STATE_END;
                }
            }
            break;
        case 5:
            if (*data == NULL) {
                wfightmn_add_gauge(0x10, fightstg_action.damage);
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        if (*data == NULL) {
            switch (obj->base.step) {
            case 0:
            default:
                *data = fightstg_message_create();
                obj->args[0] = 0x10;
                (*data)->show(*data, 6, obj->args);
                obj->base.step++;
                break;
            case 1:
                fightstg_action.run(0x10, digimon->tech_2);
                *data = wfightmn_tech_script_create(0x10, digimon->tech_2);
                obj->base.step++;
                break;
            case 2:
                *data = fightstg_message_create();
                if (fightstg_action.hits[0] != 0) {
                    FightstgMember *member;

                    obj->args[0] = 0;
                    obj->args[1] = fightstg_action.damage;
                    (*data)->show(*data, 4, obj->args);
                    member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                    member->hp -= fightstg_action.damage;
                    if (member->hp <= 0) {
                        member->hp = 0;
                        if (obj->no_knockout == 0) {
                            fightstg_events_add_knockout(0);
                        }
                    } else {
                        wfightmn_add_gauge(0x10, fightstg_action.damage);
                    }
                } else {
                    obj->args[0] = 0x1D;
                    obj->args[1] = 0;
                    (*data)->show(*data, 2, obj->args);
                }
                obj->base.step++;
                break;
            case 3:
                fightstg_events_end_final_phase();
                obj->base.state = OBJECT_STATE_END;
                break;
            }
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

void *fightstg_boss_turn_create(s32 arg0, s32 arg1) {
    FightstgBossTurn *obj = object_new(fightstg_boss_turn_update, sizeof(FightstgBossTurn), sizeof(FightstgMessage *));

    obj->counter = arg0;
    obj->no_knockout = arg1;
    return obj;
}

/* Each branch clears its effect itself (the copies are cross-jumped into one tail, except 0x21's, which keeps it):
 * the per-branch copies make the loop long enough (396 insns) that loop.c leaves the seven 0x10 constants in place
 * (last-fight; the one shared clear gave 376 insns and hoisted 0x10 into s6). The original's jump table starts at
 * case 2 (hence the case 2 label); `enemy = obj->side != 0` is its own variable (wip-9: written inline GCC folds
 * `1 - (side != 0)` to `side == 0`). */
void fightstg_results_update(FightstgResults *obj, FightstgMessage **data) {
    s32 i;
    s32 side;
    s32 enemy;
    FightstgMember *member;
    FightstgMember *target;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        fightstg_action.effects[9] = 0;
        fightstg_action.effects[10] = 0;
        fightstg_action.effects[11] = 0;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            for (i = 0; i < 0x25; i++) {
                if (fightstg_action.effects[i] != 0) {
                    obj->base.substep = i;
                    side = obj->side >> 4;
                    obj->base.step++;
                    member = &fightstg_battle.state.members[1 - side][fightstg_battle.state.current[1 - side]];
                    if (i == 5 && (member->status & 8)) {
                        fightstg_events_start_sleep(0x10 - obj->side, fightstg_action.tech, fightstg_action.effects[5]);
                        fightstg_action.effects[obj->base.substep] = 0;
                        break;
                    } else {
                        *data = fightstg_message_create();
                        if (i == 12) {
                            target = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                            obj->args[0] = target->item;
                            (*data)->show(*data, 0x11, obj->args);
                            gamestate_data.items[target->item]++;
                            if (gamestate_data.items[target->item] >= 100) {
                                gamestate_data.items[target->item] = 99;
                            }
                            target->item = -1;
                            fightstg_action.effects[obj->base.substep] = 0;
                        } else if (i == 8) {
                            member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
                            obj->args[0] = obj->side;
                            if (member->max_hp < member->hp + fightstg_action.drained) {
                                obj->args[1] = member->max_hp - member->hp;
                            } else {
                                obj->args[1] = fightstg_action.drained;
                            }
                            obj->args[2] = 0;
                            (*data)->show(*data, 0x12, obj->args);
                            member->hp += obj->args[1];
                            fightstg_action.effects[obj->base.substep] = 0;
                        } else if (i == 0x1D) {
                            member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                            target = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
                            obj->args[0] = 0x10;
                            if (member->mp < fightstg_action.drained) {
                                obj->args[1] = member->mp;
                            } else {
                                obj->args[1] = fightstg_action.drained;
                            }
                            obj->args[2] = 1;
                            (*data)->show(*data, 0x12, obj->args);
                            target->mp += obj->args[1];
                            if (target->max_mp < target->mp) {
                                target->mp = target->max_mp;
                            }
                            member->mp -= obj->args[1];
                            fightstg_action.effects[obj->base.substep] = 0;
                        } else if (i == 0x20) {
                            obj->args[0] = 0x10 - obj->side;
                            if (fightstg_action.effects[0x20] & 1) {
                                obj->args[1] = 0;
                            } else if (fightstg_action.effects[0x20] & 2) {
                                obj->args[1] = 1;
                            } else if (fightstg_action.effects[0x20] & 4) {
                                obj->args[1] = 2;
                            }
                            (*data)->show(*data, 0x13, obj->args);
                            fightstg_action.effects[obj->base.substep] = 0;
                        } else if (i == 0x21) {
                            obj->args[0] = 0x10 - obj->side;
                            if (fightstg_action.effects[0x21] & 1) {
                                obj->args[1] = 0;
                                fightstg_action.effects[obj->base.substep] &= ~1;
                            } else if (fightstg_action.effects[0x21] & 2) {
                                obj->args[1] = 1;
                                fightstg_action.effects[obj->base.substep] &= ~2;
                            } else if (fightstg_action.effects[0x21] & 4) {
                                obj->args[1] = 2;
                                fightstg_action.effects[obj->base.substep] &= ~4;
                            }
                            (*data)->show(*data, 0x13, obj->args);
                            break;
                        } else {
                            enemy = obj->side != 0;
                            switch (i) {
                            case 2:
                            default:
                                fightstg_events_start_poison(0x10 - obj->side, fightstg_battle.state.current[1 - enemy],
                                                       fightstg_action.effects[i]);
                                obj->args[0] = 0x1E;
                                break;
                            case 3:
                                fightstg_events_start_paralysis(0x10 - obj->side, fightstg_action.tech, fightstg_action.effects[i]);
                                obj->args[0] = 0x1F;
                                break;
                            case 4:
                                fightstg_events_start_confusion(0x10 - obj->side, fightstg_action.tech, fightstg_action.effects[i]);
                                obj->args[0] = 0x20;
                                break;
                            case 5:
                                fightstg_events_start_sleep(0x10 - obj->side, fightstg_action.tech, fightstg_action.effects[i]);
                                obj->args[0] = 0x21;
                                break;
                            case 13:
                                fightstg_events_start_seal(0xD3);
                                obj->args[0] = 0x44;
                                break;
                            case 26:
                                obj->args[0] = 0x33;
                                break;
                            case 27:
                                obj->args[0] = 0x34;
                                break;
                            case 34:
                                obj->args[0] = 0x4C;
                                break;
                            case 35:
                                obj->args[0] = 0x48;
                                fightstg_events_end_battle(0);
                                break;
                            }
                            obj->args[1] = (obj->side == 0) << 4;
                            (*data)->show(*data, 2, obj->args);
                            fightstg_action.effects[obj->base.substep] = 0;
                        }
                    }
                    break;
                }
            }
            if (obj->base.substep == 0) {
                obj->base.state = OBJECT_STATE_END;
            }
            break;
        case 1:
            if (*data == NULL) {
                obj->base.set_step(obj, 0);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgResults *fightstg_results_create(u8 side) {
    FightstgResults *obj = object_new(fightstg_results_update, sizeof(FightstgResults), sizeof(FightstgMessage *));

    obj->side = side;
    return obj;
}

void fightstg_model_view_set(FightstgModelView *obj) {
    GfxLayer *layer;

    RotMatrixYXZ_gte(&obj->rot, &obj->coord.coord);
    obj->coord.flg = 0;
    obj->view.super = &obj->coord;
    obj->coord.coord.t[0] = obj->trans.vx;
    obj->coord.coord.t[1] = obj->trans.vy;
    obj->coord.coord.t[2] = obj->trans.vz;
    GsSetRefView2(&obj->view);
    layer = gfx_module.funcs.get_layer(0x1009);
    layer->save_camera(layer, 1, obj->projection);
    obj->frames--;
}

void fightstg_model_view_reset(FightstgModelView *obj) {
    FightstgModelRecordA *rec = fightstg_models.get(obj->model_id);
    s32 i = 6;

    if (obj->params->idle_anim != 0) {
        i = 7;
    }
    obj->view.vpx = rec->camera_eyes[i][0];
    obj->view.vpy = -rec->camera_eyes[i][1];
    obj->view.vpz = -rec->camera_eyes[i][2];
    obj->view.vrx = rec->camera_targets[i][0];
    obj->view.vry = -rec->camera_targets[i][1];
    obj->view.vrz = -rec->camera_targets[i][2];
    obj->projection = rec->camera_projections[i];
    obj->rot.vx = 0;
    obj->rot.vy = 0;
    obj->rot.vz = 0;
    obj->trans.vx = 0;
    obj->trans.vy = 0;
    obj->trans.vz = 0;
}

void fightstg_model_view_update(FightstgModelView *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.substep) {
        case 0:
        default:
            if (gfx_module.funcs.get_layer(0x1009) != NULL) {
                fightstg_model_view_reset(obj);
                obj->frames = 2;
                obj->base.next_substep(obj);
            }
        case 1:
            break;
        }
        if (obj->frames != 0) {
            fightstg_model_view_set(obj);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgModelView *fightstg_model_view_create(s32 id, FightstgModelParams *params) {
    FightstgModelView *obj = object_new(fightstg_model_view_update, sizeof(FightstgModelView), 0);

    obj->model_id = id;
    obj->params = params;
    return obj;
}

/* The camera: moves setting from move_from to move_to (move) and sets the view of layer layer_id. */
void fightstg_camera_update(FightstgCamera *obj) {
    SVECTOR a;
    SVECTOR b;
    SVECTOR out;
    GsCOORDINATE2 coord;
    GsRVIEW2 view;
    GfxLayer *layer;
    s32 t;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->move_pos = 0x1000;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_DONE:
        obj->base.set_state(obj, OBJECT_STATE_RUN);
        obj->base.set_step(obj, 1);
        /* fallthrough */
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 1:
        case 2:
            if (obj->move_pos != 0x1000) {
                t = obj->move_pos + (obj->move_step >> 8) * gfx_module.funcs.get_frame_ticks();
                obj->move_pos = t;
                if (t < 0x1000) {
                    a.vx = obj->move_from.eye[0];
                    a.vy = obj->move_from.eye[1];
                    a.vz = obj->move_from.eye[2];
                    b.vx = obj->move_to.eye[0];
                    b.vy = obj->move_to.eye[1];
                    b.vz = obj->move_to.eye[2];
                    fightstg_math.lerp(&a, &b, t, &out);
                    obj->setting.eye[0] = out.vx;
                    obj->setting.eye[1] = out.vy;
                    obj->setting.eye[2] = out.vz;
                    a.vx = obj->move_from.target[0];
                    a.vy = obj->move_from.target[1];
                    a.vz = obj->move_from.target[2];
                    b.vx = obj->move_to.target[0];
                    b.vy = obj->move_to.target[1];
                    b.vz = obj->move_to.target[2];
                    fightstg_math.lerp(&a, &b, t, &out);
                    obj->setting.target[0] = out.vx;
                    obj->setting.target[1] = out.vy;
                    obj->setting.target[2] = out.vz;
                    a.vx = obj->move_from.trans[0];
                    a.vy = obj->move_from.trans[1];
                    a.vz = obj->move_from.trans[2];
                    b.vx = obj->move_to.trans[0];
                    b.vy = obj->move_to.trans[1];
                    b.vz = obj->move_to.trans[2];
                    fightstg_math.lerp(&a, &b, t, &out);
                    obj->setting.trans[0] = out.vx;
                    obj->setting.trans[1] = out.vy;
                    obj->setting.trans[2] = out.vz;
                    fightstg_math.lerp((SVECTOR *)obj->move_from.rot, (SVECTOR *)obj->move_to.rot, t, &out);
                    obj->setting.rot[0] = out.vx;
                    obj->setting.rot[1] = out.vy;
                    obj->setting.rot[2] = out.vz;
                    a.vx = obj->move_from.roll;
                    a.vy = obj->move_from.projection;
                    a.vz = 0;
                    b.vx = obj->move_to.roll;
                    b.vy = obj->move_to.projection;
                    b.vz = 0;
                    fightstg_math.lerp(&a, &b, t, &out);
                    obj->setting.roll = out.vx;
                    obj->setting.projection = out.vy;
                } else {
                    obj->move_pos = 0x1000;
                    obj->setting = obj->move_to;
                }
            }
            RotMatrixYXZ_gte((SVECTOR *)obj->setting.rot, &coord.coord);
            coord.coord.t[0] = obj->setting.trans[0];
            coord.coord.t[1] = obj->setting.trans[1];
            coord.coord.t[2] = obj->setting.trans[2];
            coord.flg = 0;
            coord.param = NULL;
            coord.super = NULL;
            coord.sub = NULL;
            view.vpx = obj->setting.eye[0];
            view.vpy = obj->setting.eye[1];
            view.vpz = obj->setting.eye[2];
            view.vrx = obj->setting.target[0];
            view.vry = obj->setting.target[1];
            view.vrz = obj->setting.target[2];
            view.rz = obj->setting.roll << 12;
            view.super = &coord;
            GsSetRefView2(&view);
            layer = gfx_module.funcs.get_layer(obj->layer_id);
            layer->save_camera(layer, 1, obj->setting.projection);
            if (obj->move_pos == 0x1000) {
                obj->base.next_step(obj);
            }
            break;
        }
        break;
    case OBJECT_STATE_END:
        layer = gfx_module.funcs.get_layer(obj->layer_id);
        layer->save_camera(layer, 0, 0);
        break;
    }
}

void fightstg_camera_set(FightstgCamera *obj, FightstgCameraSetting *params) {
    obj->setting = *params;
    obj->move_pos = 0x1000;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

void fightstg_camera_move(FightstgCamera *obj, FightstgCameraSetting *from, FightstgCameraSetting *to, s32 frames) {
    if (from != NULL) {
        obj->move_from = *from;
    } else {
        obj->move_from = obj->setting;
    }
    obj->move_to = *to;
    obj->move_step = 0x100000 / frames;
    obj->move_pos = 0;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

FightstgCameraSetting *fightstg_camera_get_preset(FightstgCamera *obj, s32 slot, s32 preset) {
    FightstgSlots *list;
    s32 id;
    FightstgModelRecordA *a;
    FightstgModelRecordB *b;

    /* Evidence (class A2, sched2 barrier; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original finishes every register save and
     * argument copy before the first call's arguments. */
    LOOP_BARRIER();
    list = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
    if (list != NULL) {
        id = list->get_model_id(list, slot);
        if (id != 0) {
            if (!(slot & 0xF0)) {
                a = fightstg_models.get(id);
                fightstg_camera_setting.eye[0] = a->camera_eyes[preset][0];
                fightstg_camera_setting.eye[1] = -a->camera_eyes[preset][1];
                fightstg_camera_setting.eye[2] = -a->camera_eyes[preset][2];
                fightstg_camera_setting.target[0] = a->camera_targets[preset][0];
                fightstg_camera_setting.target[1] = -a->camera_targets[preset][1];
                fightstg_camera_setting.target[2] = -a->camera_targets[preset][2];
                fightstg_camera_setting.projection = a->camera_projections[preset];
            } else {
                b = fightstg_models.get(id);
                fightstg_camera_setting.eye[0] = b->camera_eyes[preset][0];
                fightstg_camera_setting.eye[1] = -b->camera_eyes[preset][1];
                fightstg_camera_setting.eye[2] = -b->camera_eyes[preset][2];
                fightstg_camera_setting.target[0] = b->camera_targets[preset][0];
                fightstg_camera_setting.target[1] = -b->camera_targets[preset][1];
                fightstg_camera_setting.target[2] = -b->camera_targets[preset][2];
                fightstg_camera_setting.projection = b->camera_projections[preset];
            }
            fightstg_camera_setting.rot[0] = 0;
            fightstg_camera_setting.rot[1] = 0;
            fightstg_camera_setting.rot[2] = 0;
            fightstg_camera_setting.trans[0] = 0;
            fightstg_camera_setting.trans[1] = 0;
            fightstg_camera_setting.trans[2] = 0;
            fightstg_camera_setting.roll = 0;
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
    }
    return &fightstg_camera_setting;
}

FightstgCameraSetting *fightstg_camera_get_default(FightstgCamera *obj) {
    FightstgSlots *list;
    s32 id;

    /* Evidence (class A2, sched2 barrier; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original finishes every register save (s0
     * after ra) before the first call's arguments. */
    LOOP_BARRIER();
    list = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
    if (list != NULL) {
        id = list->get_model_id(list, 0x10);
        if (id != 0) {
            fightstg_models.get(id);
            fightstg_camera_get_preset(obj, 0, fightstg_models.record_b->default_camera - 1);
        }
    }
    return &fightstg_camera_setting;
}

OBJECT_V0(FightstgCamera *) fightstg_camera_create(s32 layer_id) {
    FightstgCamera *obj = object_create(fightstg_camera_update, sizeof(FightstgCamera), 0, 0x12);

    obj->set = fightstg_camera_set;
    obj->move = fightstg_camera_move;
    obj->get_default = fightstg_camera_get_default;
    obj->layer_id = layer_id;
    obj->get_preset = fightstg_camera_get_preset;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* The party's command menu (on the object list as 0xE): base.step = phase, base.substep = the command
 * chosen, base.timer = its step. The menus write their choice to `choice` (-1 = none yet, -2 = back). */
typedef struct FightstgCommand {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 last; /* last command chosen */
    /* 0x54 */ s32 member_sel;
    /* 0x58 */ s32 choice; /* the menus' choice */
    /* 0x5C */ s32 command; /* 0 attack, 1 escape, 2 digivolve, 3 item, 4 technique, 5 switch, 6 team technique, -1 confused */
    /* 0x60 */ s32 arg;
    /* 0x64 */ s32 arg2;
    /* 0x68 */ s32 team_tech;
} FightstgCommand; /* size 0x6C */

/* fightstg_command_update's data block: its child objects. */
typedef struct FightstgCommandData {
    /* 0x00 */ void *children[7]; /* 0: HUD, 1: command menu, 2: model view, 3: sub-menu, 4: target,
                               * 6: fightstg_idle_camera_create's */
} FightstgCommandData; /* size 0x1C */

struct FightstgHud *fightstg_hud_create(void);
struct FightstgCommandMenu *fightstg_command_menu_create(s32 sel, s32 *result);
struct FightstgPortrait *fightstg_portrait_create(void);
struct FightstgDigivolveMenu *fightstg_digivolve_menu_create(s32 *result);
struct FightstgItemMenu *fightstg_item_menu_create(s32 *result);
struct FightstgTechMenu *fightstg_tech_menu_create(s32 *result);
struct FightstgMemberMenu *fightstg_member_menu_create(s32 *result, s32 *cursor, s32 team);
struct FightstgSwitchMenu *fightstg_switch_menu_create(s32 *result);
struct FightstgSwitchMenu *fightstg_switch_menu_create_team(s32 *result, s32 *tech);
struct FightstgConfusedMenu *fightstg_confused_menu_create(s32 *result, Object *portrait, Object *idle_camera);
/* fightstg_80086A00.c defines it as void; it leaves its object in v0, which this file keeps (OBJECT_V0). */
void *fightstg_idle_camera_create(void);

void fightstg_command_update(FightstgCommand *obj, FightstgCommandData *data) {
    s32 i;
    Object *child;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        return;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            switch (obj->base.substep) {
            case 0:
            default:
                obj->last = 0;
                for (i = 1; i < 7; i++) {
                    child = data->children[i];
                    if (child != NULL) {
                        child->set_state(child, OBJECT_STATE_END);
                    }
                }
                obj->base.next_substep(obj);
                return;
            case 1:
                return;
            }
            return;
        case 1:
            if (data->children[0] == NULL) {
                data->children[0] = fightstg_hud_create();
            }
            obj->base.set_step(obj, 0);
            return;
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                switch (obj->base.timer) {
                case 0:
                default:
                    if (data->children[0] == NULL) {
                        data->children[0] = fightstg_hud_create();
                    }
                    if (data->children[1] == NULL) {
                        data->children[1] = fightstg_command_menu_create(obj->last, &obj->choice);
                    }
                    if (data->children[2] == NULL) {
                        data->children[2] = fightstg_portrait_create();
                    }
                    if (data->children[6] == NULL) {
                        data->children[6] = fightstg_idle_camera_create();
                    }
                    obj->choice = -1;
                    obj->base.next_timer(obj);
                case 1:
                    if (obj->choice != -1) {
                        obj->last = obj->choice;
                        switch (obj->choice) {
                        case 1:
                            obj->base.set_substep(obj, 3);
                            return;
                        case 2:
                            obj->base.set_substep(obj, 1);
                            return;
                        case 3:
                            obj->base.set_substep(obj, 4);
                            obj->member_sel = 0;
                            return;
                        case 4:
                            obj->base.set_substep(obj, 2);
                            return;
                        case 5:
                            obj->command = 1;
                            obj->base.set_step(obj, 0);
                            return;
                        case 0:
                            obj->command = 0;
                            obj->base.set_step(obj, 0);
                            return;
                        }
                    }
                    return;
                }
                return;
            case 1:
                switch (obj->base.timer) {
                case 0:
                default:
                    obj->choice = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
                    data->children[3] = fightstg_digivolve_menu_create(&obj->choice);
                    obj->base.next_timer(obj);
                case 1:
                    if (obj->choice == -2) {
                        obj->base.set_step(obj, 2);
                    } else if (obj->choice != -1) {
                        obj->command = 2;
                        obj->arg = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
                        obj->arg2 = obj->choice;
                        obj->base.set_step(obj, 0);
                    }
                    return;
                }
                return;
            case 2:
                switch (obj->base.timer) {
                case 0:
                default:
                    data->children[3] = fightstg_item_menu_create(&obj->choice);
                    obj->base.next_timer(obj);
                case 1:
                    if (obj->choice == -2) {
                        obj->base.set_step(obj, 2);
                    } else if (obj->choice != -1) {
                        obj->command = 3;
                        obj->arg = obj->choice;
                        obj->base.set_step(obj, 0);
                    }
                    return;
                }
                return;
            case 3:
                switch (obj->base.timer) {
                case 0:
                default:
                    data->children[3] = fightstg_tech_menu_create(&obj->choice);
                    obj->base.next_timer(obj);
                case 1:
                    if (obj->choice == -2) {
                        obj->base.set_step(obj, 2);
                    } else if (obj->choice != -1) {
                        obj->command = 4;
                        obj->arg = obj->choice;
                        obj->base.set_step(obj, 0);
                    }
                    return;
                }
                return;
            case 4:
                switch (obj->base.timer) {
                case 0:
                default:
                    data->children[3] = fightstg_member_menu_create(&obj->choice, &obj->member_sel, 1);
                    obj->base.next_timer(obj);
                case 1:
                    switch (obj->choice) {
                    case -1:
                        break;
                    case -2:
                        obj->base.set_step(obj, 2);
                        break;
                    default:
                        obj->arg = obj->choice;
                        data->children[4] = fightstg_switch_menu_create_team(&obj->choice, &obj->team_tech);
                        obj->base.next_timer(obj);
                        break;
                    }
                    return;
                case 2:
                    if (obj->choice == -2) {
                        obj->base.set_timer(obj, 0);
                    } else if (obj->choice != -1) {
                        if (obj->choice & 0xF) {
                            obj->command = 6;
                            obj->arg2 = obj->choice >> 4;
                            obj->base.set_step(obj, 0);
                            obj->member_sel = 0;
                        } else {
                            obj->command = 5;
                            obj->arg2 = obj->choice >> 4;
                            obj->base.set_step(obj, 0);
                            obj->member_sel = 0;
                        }
                    }
                    return;
                }
                return;
            }
            return;
        case 3:
            switch (obj->base.timer) {
            case 0:
            default:
                if (data->children[2] == NULL) {
                    data->children[2] = fightstg_portrait_create();
                }
                if (data->children[6] == NULL) {
                    data->children[6] = fightstg_idle_camera_create();
                }
                if (data->children[1] == NULL) {
                    data->children[1] = fightstg_confused_menu_create(&obj->choice, data->children[2], data->children[6]);
                }
                obj->choice = -1;
                obj->base.next_timer(obj);
            case 1:
                if (obj->choice != -1) {
                    if (obj->choice == 0) {
                        obj->command = 0;
                        obj->base.set_step(obj, 0);
                    } else {
                        obj->command = -1;
                        obj->base.set_step(obj, 0);
                    }
                }
                return;
            }
            return;
        case 4:
            switch (obj->base.timer) {
            case 0:
            default:
                if (data->children[6] == NULL) {
                    data->children[6] = fightstg_idle_camera_create();
                }
                if (data->children[3] == NULL) {
                    data->children[3] = fightstg_member_menu_create(&obj->choice, &obj->member_sel, 0);
                }
                obj->base.next_timer(obj);
            case 1:
                if (obj->choice != -1) {
                    obj->arg = obj->choice;
                    data->children[4] = fightstg_switch_menu_create(&obj->choice);
                    obj->base.next_timer(obj);
                }
                return;
            case 2:
                switch (obj->choice) {
                case -1:
                    break;
                case -2:
                    obj->base.set_timer(obj, 0);
                    break;
                default:
                    if (!(obj->choice & 0xF)) {
                        obj->command = 5;
                        obj->arg2 = obj->choice >> 4;
                        obj->base.set_step(obj, 0);
                        obj->member_sel = 0;
                    }
                    break;
                }
                return;
            }
            return;
        }
        return;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        return;
    }
}

OBJECT_V0(FightstgCommand *) fightstg_command_create(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_create(fightstg_command_update, sizeof(FightstgCommand), sizeof(FightstgCommandData), 0xE))
}

void fightstg_command_set_phase(s32 phase) {
    Object *obj = heap_objects.find(0xE, -1, -1);

    if (obj != NULL && obj->state == OBJECT_STATE_RUN) {
        obj->set_step(obj, phase);
    }
}

s32 fightstg_command_is_open(void) {
    return heap_objects.find(0xE, -1, -1)->step != 0;
}

/* Draws the fade: a full-screen (320x240) quad, additive (abr 1), each channel `level`. */
void fightstg_flash_draw(FightstgFlash *obj) {
    POLY_F4 *poly;
    DR_TPAGE *tpage;
    u32 *ot;
    GfxLayer *layer;

    layer = gfx_module.funcs.get_layer(0x1006);
    ot = layer->get_ot_entry(layer, 0);
    poly = gfx_module.funcs.get_packet();
    tpage = (DR_TPAGE *)(poly + 1);
    setRGB0(poly, obj->level, obj->level, obj->level);
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->x3 = poly->x1 = 320;
    poly->x2 = poly->x0 = 0;
    poly->y1 = poly->y0 = 0;
    poly->y3 = poly->y2 = 240;
    addPrim(ot, poly);
    SetDrawTPage(tpage, 0, 1, getTPage(1, 1, 0, 0));
    addPrim(ot, tpage);
    tpage++;
    gfx_module.funcs.set_packet(tpage);
}

void fightstg_flash_update(FightstgFlash *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        obj->level = 0;
        /* fallthrough */
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            obj->level += obj->step * gfx_module.funcs.get_frame_ticks();
            if (obj->level >= 0xFF) {
                obj->level = 0xFF;
                obj->base.next_step(obj);
            }
        }
        break;
    case OBJECT_STATE_DONE:
        obj->level -= obj->step * gfx_module.funcs.get_frame_ticks();
        if (obj->level < 0) {
            obj->level = 0;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
    fightstg_flash_draw(obj);
}

void fightstg_flash_end(FightstgFlash *obj, s32 frames) {
    obj->step = 0xFF / frames;
    obj->base.set_state(obj, OBJECT_STATE_DONE);
}

FightstgFlash *fightstg_flash_create(s32 frames) {
    FightstgFlash *obj = object_new(fightstg_flash_update, sizeof(FightstgFlash), 0);

    obj->step = 0xFF / frames;
    return obj;
}

/* An HP bar: moves from hp_from to hp_to in `duration` frames along a sine curve. */
typedef struct FightstgHudBar {
    /* 0x00 */ s32 hp_from; /* HP moved from */
    /* 0x04 */ s32 hp_to; /* HP moved to */
    /* 0x08 */ s32 hp; /* HP shown */
    /* 0x0C */ s16 moving;
    /* 0x0E */ s16 member; /* member shown (fightstg_battle.state) */
    /* 0x10 */ s16 frames;
    /* 0x12 */ s16 duration;
} FightstgHudBar; /* size 0x14 */

/* The battle HUD (fightstg_hud_update): the acting members' names and HP bars. */
typedef struct FightstgHud {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 named[2]; /* per side: the member whose name is shown */
    /* 0x58 */ FightstgHudBar bars[2]; /* per side */
    /* 0x80 */ s32 update_time; /* frames since the last HP update */
    /* 0x84 */ s32 update_frames; /* frames between HP updates */
} FightstgHud; /* size 0x88 */

/* fightstg_hud_update's data block: its message windows. */
typedef struct FightstgHudData {
    /* 0x00 */ MessageWindow *party_name; /* the party member's name */
    /* 0x04 */ MessageWindow *enemy_name; /* the enemy's name */
    /* 0x08 */ MessageWindow *slash;
    /* 0x0C */ MessageWindow *max_hp; /* max HP */
    /* 0x10 */ MessageWindow *hp;
} FightstgHudData; /* size 0x14 */

void fightstg_hud_update_hp(FightstgHud *obj, FightstgHudData *data);

void fightstg_hud_update_hp(FightstgHud *obj, FightstgHudData *data) {
    FightstgMember *row;
    s32 i;
    s32 j;
    s32 hp;

    obj->update_time += gfx_module.funcs.get_frame_ticks();
    for (i = 0; i < 2; i++) {
        row = fightstg_battle.state.members[i];
        if (obj->bars[i].member != fightstg_battle.state.current[i]) {
            obj->bars[i].hp_from = row[fightstg_battle.state.current[i]].hp;
            obj->bars[i].hp_to = row[fightstg_battle.state.current[i]].hp;
            obj->bars[i].hp = row[fightstg_battle.state.current[i]].hp;
            obj->bars[i].moving = 0;
            obj->bars[i].member = fightstg_battle.state.current[i];
        }
    }
    if (obj->update_time > obj->update_frames) {
        obj->update_time -= obj->update_frames;
        for (j = 0; j < 2; j++) {
            row = fightstg_battle.state.members[j];
            hp = row[fightstg_battle.state.current[j]].hp;
            if (hp <= 0) {
                hp = 0;
            }
            if (row[fightstg_battle.state.current[j]].max_hp < hp) {
                hp = row[fightstg_battle.state.current[j]].max_hp;
            }
            if (hp != obj->bars[j].hp_to) {
                obj->bars[j].hp_to = hp;
                obj->bars[j].moving = 1;
                obj->bars[j].frames = 0;
                obj->bars[j].duration = 30;
                obj->bars[j].hp_from = obj->bars[j].hp;
            }
        }
    }
}

void fightstg_hud_move_bar(FightstgHudBar *bar) {
    s32 t;

    if (bar->moving != 0) {
        bar->frames += gfx_module.funcs.get_frame_ticks();
        if (bar->frames >= bar->duration) {
            bar->moving = 0;
            bar->hp_from = bar->hp = bar->hp_to;
        } else {
            t = rsin((bar->frames << 10) / bar->duration) * bar->duration / 4096;
            bar->hp = bar->hp_from + (bar->hp_to - bar->hp_from) * t / bar->duration;
        }
    }
}

void fightstg_hud_update_names(FightstgHud *obj, FightstgHudData *data) {
    FightstgEnemyRecord *digimon;
    s32 i;

    if (fightstg_battle.state.current[0] != obj->named[0]) {
        if (data->party_name == NULL) {
            data->party_name = message_create_window(0x1005, 1, 0xAE, 0x15);
        }
        data->party_name->set_text(data->party_name,
            gamestate_data.digimon[gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0])].record.name, -1);
    }
    if (fightstg_battle.state.current[1] != obj->named[1]) {
        digimon = fightstg_enemy_records.get(records_state.enemies[fightstg_battle.state.current[1]].digimon);
        if (data->enemy_name == NULL) {
            data->enemy_name = message_create_window(0x1005, 1, 0x11, 0x15);
        }
        if (digimon != NULL) {
            data->enemy_name->set_text(data->enemy_name, cdload_module.files.get_file(records_language + 0x4E), digimon->name);
        }
    }
    for (i = 0; i < 2; i++) {
        obj->named[i] = fightstg_battle.state.current[i];
    }
}

/* The HUD's HP bars (quads drawn by fightstg_battle.draw_quad) and their colours. */
extern DVECTOR fightstg_hud_bars[2][4]; /* per side; the bar's right (party) or left end moves */
extern CVECTOR fightstg_hud_bar_colors[2][2]; /* above a quarter of max HP, else below */
extern DVECTOR fightstg_hud_gauge[4];
extern CVECTOR fightstg_hud_gauge_colors[4];
extern s16 fightstg_hud_icon_x[2][3]; /* per side and member: x of its icon */

void fightstg_hud_draw(FightstgHud *obj, FightstgHudData *data) {
    Sprite sprite;
    CVECTOR colors[4];
    s32 *bank;
    FightstgMember *member;
    s32 side;
    s32 i;
    s32 j;
    s32 w;
    s32 frame;
    s32 k;
    s32 idx;
    s32 x;

    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0, 8, 0xF);
    sprite.draw(bank, 1, 0xA1, 0xF);
    fightstg_hud_update_names(obj, data);
    member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    if (data->hp == NULL) {
        data->hp = message_create_window(0x1005, 3, 0x10B, 0x1A);
    }
    data->hp->set_line_number(data->hp, 0, obj->bars[0].hp);
    data->hp->measure(data->hp, 1);
    if (data->max_hp == NULL) {
        data->max_hp = message_create_window(0x1005, 3, 0x12E, 0x1A);
    }
    data->max_hp->set_line_number(data->max_hp, 0, member->max_hp);
    data->max_hp->measure(data->max_hp, 1);
    for (side = 0; side < 2; side++) {
        member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
        w = (obj->bars[side].hp << 7) / member->max_hp;
        if (obj->bars[side].hp > 0 && w < 4) {
            w = 3;
        }
        if (side == 0) {
            fightstg_hud_bars[0][1].vx = fightstg_hud_bars[0][0].vx + w;
            fightstg_hud_bars[0][3].vx = fightstg_hud_bars[0][2].vx + w;
        } else {
            fightstg_hud_bars[side][1].vx = fightstg_hud_bars[side][0].vx - w;
            fightstg_hud_bars[side][3].vx = fightstg_hud_bars[side][2].vx - w;
        }
        if ((member->max_hp >> 2) < obj->bars[side].hp) {
            colors[0] = colors[2] = fightstg_hud_bar_colors[0][0];
            colors[1] = colors[3] = fightstg_hud_bar_colors[0][1];
        } else {
            colors[0] = colors[2] = fightstg_hud_bar_colors[1][0];
            colors[1] = colors[3] = fightstg_hud_bar_colors[1][1];
        }
        fightstg_battle.draw_quad(0x1005, 1, fightstg_hud_bars[side], colors);
    }
    frame = (gfx_module.funcs.get_time() >> 2) & 3;
    idx = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
    sprite.draw(bank, 0x1E, 0x104, 0x3D);
    if (records_state.gauges[idx] < 1000) {
        fightstg_hud_gauge[2].vx = fightstg_hud_gauge[0].vx = records_state.gauges[idx] / 25 + 0x109;
        fightstg_battle.draw_quad(0x1005, 1, fightstg_hud_gauge, fightstg_hud_gauge_colors);
    }
    if (records_state.gauges[idx] < 1000) {
        sprite.draw(bank, frame + 0x33, 0x109, 0x3E);
    } else {
        sprite.draw(bank, frame + 0x37, 0x109, 0x3E);
    }
    sprite.draw(bank, 0xB, 0x104, 0x3D);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            if (fightstg_battle.state.type >= 4 && j > 0 && i > 0) {
                return;
            }
            member = &fightstg_battle.state.members[i][j];
            if (member->digimon != 0) {
                if (member->status != 0) {
                    k = 3;
                } else if (member->hp == 0) {
                    k = 2;
                } else {
                    k = member->hp != member->max_hp;
                }
                sprite.draw(bank, (fightstg_battle.state.current[i] == j ? 0xC : 0x3C) + k,
                              fightstg_hud_icon_x[i][j], 0x30);
            }
        }
    }
}

void fightstg_hud_update(FightstgHud *obj, FightstgHudData *data) {
    FightstgMember (*members)[3];
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->named[1] = -1;
        obj->named[0] = -1;
        fightstg_hud_update_names(obj, data);
        data->slash = message_create_window(0x1005, 3, 0x10C, 0x1A);
        data->slash->set_text(data->slash, cdload_module.files.get_file(records_language + 0x7F), 0x10);
        members = fightstg_battle.state.members;
        data->hp = message_create_window(0x1005, 3, 0x10B, 0x1A);
        data->hp->set_line_number(data->hp, 0, members[0][fightstg_battle.state.current[0]].hp);
        data->hp->measure(data->hp, 1);
        data->max_hp = message_create_window(0x1005, 3, 0x12E, 0x1A);
        data->max_hp->set_line_number(data->max_hp, 0, members[0][fightstg_battle.state.current[0]].max_hp);
        data->max_hp->measure(data->max_hp, 1);
        for (i = 0; i < 2; i++) {
            obj->bars[i].hp_from = members[i][fightstg_battle.state.current[i]].hp;
            obj->bars[i].hp_to = members[i][fightstg_battle.state.current[i]].hp;
            obj->bars[i].hp = members[i][fightstg_battle.state.current[i]].hp;
            obj->bars[i].moving = 0;
            obj->bars[i].member = fightstg_battle.state.current[i];
            obj->bars[i].frames = 0;
            obj->bars[i].duration = 0;
        }
        obj->update_time = 0;
        obj->update_frames = 8;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        fightstg_hud_update_hp(obj, data);
        fightstg_hud_move_bar(&obj->bars[0]);
        fightstg_hud_move_bar(&obj->bars[1]);
        fightstg_hud_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FightstgHud *fightstg_hud_create(void) {
    return object_new(fightstg_hud_update, sizeof(FightstgHud), sizeof(FightstgHudData));
}

void fightstg_command_menu_open(FightstgCommandMenu *obj) {
    FightstgCommandMenuData *data = (FightstgCommandMenuData *)obj->base.children;
    FightstgMember *member;
    u8 *text;
    s32 i;

    if (data->windows[0] == NULL) {
        text = cdload_module.files.get_file(records_language + 0x7F);
        for (i = 0; i < 6; i++) {
            data->windows[i] = message_create_window(0x1005, 1, 0x24, i * 19 + 0x6D);
            data->windows[i]->set_text(data->windows[i], text, i + 1);
        }
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        if (member->status & 8) {
            for (i = 0; i < 3; i++) {
                data->windows[0]->set_palette(data->windows[i], 7);
            }
        }
        if (member->status & 0x20) {
            data->windows[0]->set_palette(data->windows[2], 7);
        }
    }
}

void fightstg_command_menu_update(FightstgCommandMenu *obj, FightstgCommandMenuData *data) {
    FightstgMember *member;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->cursor = fightstg_cursor_create(&fightstg_command_menu_cursor);
        data->cursor->sel = obj->sel;
        fightstg_command_menu_open(obj);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class B, register priority only; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original keeps the state in s2
         * through case 1 (its 1 is stored to unk_54); without the block only registers differ, same layout. */
        LOOP_BLOCK(
                if (pad_state.get_pressed(0) & 0x2000) {
                    sound_module.play(0x4001C);
                    member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                    if ((member->status & 8) && data->cursor->sel < 3) {
                        break;
                    }
                    if ((member->status & 0x20) && data->cursor->sel == 2) {
                        break;
                    }
                    *obj->result = data->cursor->sel;
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    data->cursor->locked = 1;
                }
        );
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgCommandMenu *fightstg_command_menu_create(s32 sel, s32 *result) {
    FightstgCommandMenu *obj = object_new(fightstg_command_menu_update, sizeof(FightstgCommandMenu), sizeof(FightstgCommandMenuData));

    obj->result = result;
    *result = -1;
    obj->sel = sel;
    return obj;
}

/* A window showing the party's model (fightstg_model_view_create's views) on layer 0x1009. */
typedef struct FightstgPortrait {
    /* 0x00 */ Object base;
    /* 0x50 */ GfxLayer *layer;  /* layer 0x1009 */
    /* 0x54 */ u8 unk_54[0x4];
} FightstgPortrait; /* size 0x58 */

/* fightstg_portrait_update's data block: the view shown and the one replaced. */
typedef struct FightstgPortraitData {
    /* 0x0 */ FightstgModelView *view;
    /* 0x4 */ FightstgModelView *view_2;
} FightstgPortraitData; /* size 0x8 */

extern RECT fightstg_portrait_rect;

void fightstg_portrait_draw(FightstgPortrait *obj, FightstgPortraitData *data) {
    Sprite sprite;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 0);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(cdload_module.get_subfile_by_id(0x4550000), 0xA, 0xF6, 0x4A);
}

void fightstg_portrait_update(FightstgPortrait *obj, FightstgPortraitData *data) {
    FightstgSlots *slots;
    FightstgModelParams *params;
    FightstgModelView *old;
    s32 id;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        fightstg_portrait_rect.x = 0xD0;
        fightstg_portrait_rect.y = 0x4C;
        fightstg_portrait_rect.w = 0x64;
        fightstg_portrait_rect.h = 0x3C;
        obj->layer = gfx_module.funcs.create_layer(&fightstg_portrait_rect, 0xC, 0x1009);
        obj->layer->set_draw_offset(obj->layer, 0x102, 0x6A);
        obj->layer->alloc_callbacks(obj->layer, 0x32);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        if (obj->base.step == 0) {
            slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            if (slots != NULL) {
                params = slots->get_params(slots, 0);
                params->layers[1].shown = 1;
                params->layers[1].edges = 0;
                params->layers[1].layer_id = 0x1009;
                id = params->model_id;
                if (data->view == NULL) {
                    data->view = fightstg_model_view_create(id, params);
                    old = data->view_2;
                } else {
                    data->view_2 = fightstg_model_view_create(id, params);
                    old = data->view;
                }
                if (old != NULL) {
                    old->base.set_state(old, OBJECT_STATE_END);
                }
                obj->base.next_step(obj);
            }
        }
        fightstg_portrait_draw(obj, data);
        break;
    case OBJECT_STATE_DONE:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_END:
        if (obj->layer != NULL) {
            slots = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
            slots->get_params(slots, 0)->layers[1].shown = 0;
            gfx_module.funcs.delete_layer(0x1009);
        }
        break;
    }
}

FightstgPortrait *fightstg_portrait_create(void) {
    return object_new(fightstg_portrait_update, sizeof(FightstgPortrait), sizeof(FightstgPortraitData));
}

/* fightstg_digivolve_menu_update's data block. */
typedef struct FightstgDigivolveMenuData {
    /* 0x00 */ FightstgCursor *cursor;
    /* 0x04 */ MessageWindow *names[4];  /* the Digimon's names */
    /* 0x14 */ FightstgStatus *status; /* status screens: one shown, the other closing */
    /* 0x18 */ FightstgStatus *status_2;
} FightstgDigivolveMenuData; /* size 0x1C */

extern FightstgCursorParams fightstg_digivolve_menu_cursor;

FightstgStatus *fightstg_status_create(s32 member, s32 page, s32 entry);


void fightstg_digivolve_menu_show_names(FightstgDigivolveMenu *obj) {
    FightstgDigivolveMenuData *data = (FightstgDigivolveMenuData *)obj->base.children;
    s32 slot;
    s32 i;
    FightstgMember *member;
    s32 cur;
    RecordsDigimon *digimon;

    for (i = 0, slot = -1; i < 3; i++) {
        if (obj->member == gamestate_data.funcs.get_party_member(i)) {
            slot = i;
            break;
        }
    }
    if (slot != -1) {
        member = &fightstg_battle.state.members[0][slot];
        if (member->blasted != 0) {
            cur = member->base_digimon;
        } else {
            cur = member->digimon;
        }
        for (i = 0; i < obj->count; i++) {
            if (data->names[i] == NULL) {
                data->names[i] = message_create_window(0x1005, 1, 0xBB, i * 19 + 0x92);
            }
            digimon = records_get_digimon_func(obj->digimon[i]);
            if (digimon != NULL) {
                data->names[i]->set_text(data->names[i], cdload_module.files.get_file(records_language + 0x4E), digimon->name_id);
                if (cur == obj->digimon[i]) {
                    data->names[i]->set_palette(data->names[i], 7);
                } else {
                    data->names[i]->set_palette(data->names[i], 0);
                }
            }
        }
    }
}

/* The data block comes in untyped and is read through a typed local (the US decomp's shape): with a typed parameter,
 * data takes the argument's slot, its longer life puts it after `changed` in global-alloc (s3/s2, 99.5%). */
void fightstg_digivolve_menu_update(FightstgDigivolveMenu *obj, void *arg) {
    FightstgDigivolveMenuData *data = arg;
    s32 member;
    s32 i;
    s32 n;
    s32 changed;
    s32 open;
    s32 pad;
    FightstgMember *m;
    s32 cur;
    s32 sel;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        member = obj->member;
        gamestate_data.funcs.get_chosen_forms(member, obj->forms);
        n = 1;
        obj->digimon[0] = records_digimon[member].id;
        for (i = 0; i < 3; i++) {
            if (obj->forms[i] >= 3) {
                obj->digimon[n++] = obj->forms[i];
            }
        }
        obj->count = n;
        fightstg_digivolve_menu_cursor.count = n;
        data->cursor = fightstg_cursor_create(&fightstg_digivolve_menu_cursor);
        data->status = fightstg_status_create(obj->member, obj->status_page, data->cursor->sel);
        fightstg_digivolve_menu_show_names(obj);
        obj->cursor = -1;
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        changed = 0;
        open = 1;
        /* Class C (block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the confirm branch goes out of line. */
        LOOP_BLOCK(
        pad = pad_state.get_pressed(0);
        if (pad & 0x800) {
            if (++obj->status_page == 3) {
                obj->status_page = 0;
            }
            changed = 1;
            sound_module.play(0x4001B);
        } else if (pad & 0x400) {
            if (--obj->status_page < 0) {
                obj->status_page = 2;
            }
            changed = 1;
            sound_module.play(0x4001B);
        } else if (pad & 0x2000) {
            sound_module.play(0x4001C);
            m = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
            if (m->blasted != 0) {
                cur = m->base_digimon;
            } else {
                cur = m->digimon;
            }
            sel = obj->digimon[data->cursor->sel];
            if (cur != sel) {
                *obj->result = sel;
                obj->base.set_state(obj, OBJECT_STATE_END);
                data->cursor->locked = 1;
            } else if (pad & 0x4000) {
                *obj->result = -2;
                obj->base.set_state(obj, OBJECT_STATE_END);
                sound_module.play(0x800450BD);
            }
        } else if (pad & 0x4000) {
            *obj->result = -2;
            obj->base.set_state(obj, OBJECT_STATE_END);
            sound_module.play(0x800450BD);
        }
        );
        if (data->cursor->sel != obj->cursor) {
            obj->cursor = data->cursor->sel;
            changed = 1;
        }
        if (changed) {
            if (open) {
                if (data->status != NULL) {
                    data->status->base.set_state(data->status, OBJECT_STATE_DONE);
                    data->status_2 = fightstg_status_create(obj->member, obj->status_page, data->cursor->sel);
                } else {
                    if (data->status_2 != NULL) {
                        data->status_2->base.set_state(data->status_2, OBJECT_STATE_DONE);
                    }
                    data->status = fightstg_status_create(obj->member, obj->status_page, data->cursor->sel);
                }
            } else {
                if (data->status != NULL) {
                    data->status->base.set_state(data->status, OBJECT_STATE_DONE);
                }
                if (data->status_2 != NULL) {
                    data->status_2->base.set_state(data->status_2, OBJECT_STATE_DONE);
                }
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgDigivolveMenu *fightstg_digivolve_menu_create(s32 *result) {
    FightstgDigivolveMenu *obj = object_new(fightstg_digivolve_menu_update, sizeof(FightstgDigivolveMenu), sizeof(FightstgDigivolveMenuData));

    obj->result = result;
    obj->member = *result;
    *result = -1;
    return obj;
}

/* Where fightstg_status_show_stats shows each stat. */
typedef struct FightstgStatWindow {
    /* 0x0 */ u8 x;     /* x */
    /* 0x1 */ u8 y;     /* y */
    /* 0x2 */ u8 stat;  /* stat (GamestateStats.values) */
} FightstgStatWindow; /* size 0x3 */

extern FightstgStatWindow fightstg_status_stat_windows[13];


void fightstg_status_draw_arrows(FightstgStatus *obj) {
    Sprite sprite;
    s32 *bank = cdload_module.get_subfile_by_id(0x4550000);

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x200, 0);
    if (gfx_module.funcs.get_time() & 0x10) {
        sprite.draw(bank, 0x1F, 0x18, 0xAB);
        sprite.draw(bank, 0x20, 0x48, 0xAB);
    }
}

void fightstg_status_open(FightstgStatus *obj, FightstgStatusData *data) {
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);

    data->l1_window = message_create_window(0x1005, 3, 0x22, 0xAB);
    data->l1_window->set_text(data->l1_window, text, 0x11);
    data->l1_window->set_palette(data->l1_window, 2);
    data->r1_window = message_create_window(0x1005, 3, 0x38, 0xAB);
    data->r1_window->set_text(data->r1_window, text, 0x12);
    data->r1_window->set_palette(data->r1_window, 2);
}

void fightstg_status_draw_stats(FightstgStatus *obj) {
    Sprite sprite;
    s32 *bank;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    bank = cdload_module.get_subfile_by_id(0x2860000);
    sprite.set_vram_pos(0x140, 0);
    sprite.draw(bank, 0x23, 0x18, 0x51);
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0x25, 0x10, 0x4A);
}

void fightstg_status_show_stats(FightstgStatus *obj, FightstgStatusData *data) {
    s32 i;
    s32 value;
    FightstgStatWindow *e;

    for (i = 0; i < 13; i++) {
        e = &fightstg_status_stat_windows[i];
        data->stat_windows[i] = message_create_window(0x1005, 1, e->x, e->y);
        value = ((s16 *)&obj->stats)[e->stat];
        if (value >= 1000) {
            value = 999;
        }
        data->stat_windows[i]->set_line_number(data->stat_windows[i], 0, value);
        data->stat_windows[i]->measure(data->stat_windows[i], 1);
    }
    if (obj->stats.penalties[0] != 0) {
        data->stat_windows[0]->set_palette(data->stat_windows[0], 6);
    }
    if (obj->stats.penalties[1] != 0) {
        data->stat_windows[1]->set_palette(data->stat_windows[1], 6);
    }
    if (obj->stats.penalties[2] != 0) {
        data->stat_windows[4]->set_palette(data->stat_windows[4], 6);
    }
}

void fightstg_status_draw_techs(FightstgStatus *obj) {
    Sprite sprite;
    s32 *bank;
    s32 i;
    s32 id;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    bank = cdload_module.get_subfile_by_id(0x2860000);
    sprite.set_vram_pos(0x140, 0);
    for (i = 0; i < 6; i++) {
        id = obj->techniques[i] & 0x1FFF;
        if (id != 0) {
            sprite.draw(bank, records_techniques[id - 1].element + 0x37, 0x18, i * 14 + 0x51);
        }
    }
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0x26, 0x10, 0x4A);
}

void fightstg_status_show_techs(FightstgStatus *obj, FightstgStatusData *data) {
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);
    s32 i;
    s32 id;

    data->skill_label = message_create_window(0x1005, 1, 0x5A, 0xA9);
    data->skill_label->set_text(data->skill_label, text, 0x19);
    data->skill_level = message_create_window(0x1005, 1, 0x93, 0xA9);
    if (obj->form_level >= 0) {
        data->skill_level->set_line_number(data->skill_level, 0, obj->form_level);
    } else {
        data->skill_level->set_text(data->skill_level, text, 0x1A);
    }
    data->skill_level->measure(data->skill_level, 1);
    for (i = 0; i < 6; i++) {
        data->tech_windows[i] = message_create_window(0x1005, 1, 0x26, i * 14 + 0x51);
        id = obj->techniques[i];
        if (id != 0) {
            data->tech_windows[i]->set_text(data->tech_windows[i], cdload_module.files.get_file(records_language + 0xA2), id & 0x1FFF);
            if (id & 0x8000) {
                data->tech_windows[i]->set_palette(data->tech_windows[i], 3);
            } else if (id & 0x4000) {
                data->tech_windows[i]->set_palette(data->tech_windows[i], 4);
            }
        }
    }
}

void fightstg_status_update(FightstgStatus *obj, FightstgStatusData *data) {
    s32 found[10];
    s32 member;
    RecordsDigimon *digimon;
    s32 i;
    s32 j;
    s32 n;
    s32 count;
    s32 id;
    s32 k;
    s32 m;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        member = obj->member;
        switch (obj->page) {
        case 0:
            gamestate_data.funcs.get_stats(member, &obj->stats);
            if (obj->form != 0) {
                gamestate_data.funcs.get_chosen_forms(member, obj->forms);
                digimon = records_get_digimon_func(obj->forms[obj->form - 1]);
                for (k = 0; k < 6; k++) {
                    (&obj->stats.values[6])[k] += digimon->stats[k];
                }
                for (k = 0; k < 7; k++) {
                    (&obj->stats.values[12])[k] += digimon->resists[k];
                }
            }
            break;
        case 1:
            if (obj->form == 0) {
                digimon = &records_digimon[member];
                obj->techniques[0] = digimon->techniques[6] | 0x8000;
                obj->form_level = -1;
            } else {
                gamestate_data.funcs.get_chosen_forms(member, obj->forms);
                gamestate_data.funcs.get_form(member, obj->forms[obj->form - 1], &obj->form_records[0]);
                obj->form_level = obj->form_records[0].level;
                for (k = 0, n = 0; k < 6; k++) {
                    if (obj->form_records[0].techniques[k] != 0) {
                        obj->techniques[n++] = obj->form_records[0].techniques[k];
                    }
                }
            }
            break;
        case 2:
            if (obj->form == 0) {
                obj->form_level = -1;
            } else {
                for (i = 9; i >= 0; i--) {
                    found[i] = 0;
                }
                i = 0;
                count = gamestate_data.funcs.get_chosen_forms(member, obj->forms);
                for (j = 0; j < count; j++) {
                    if (gamestate_data.funcs.get_form(member, obj->forms[j], &obj->form_records[j]) >= 0
                        && j != obj->form - 1) {
                        for (k = 0; k < 6; k++) {
                            id = obj->form_records[j].techniques[k];
                            if (id != 0 && (id & 0x4000)) {
                                found[i++] = id & 0x1FFF;
                            }
                        }
                    }
                }
                n = 0;
                for (m = 0; m < 10; m++) {
                    id = found[m];
                    for (i = 0; i < 6; i++) {
                        if (obj->techniques[i] == found[m]) {
                            id = 0;
                            break;
                        }
                    }
                    if (id != 0) {
                        obj->techniques[n++] = id;
                    }
                }
                obj->form_level = obj->form_records[obj->form - 1].level;
            }
            break;
        }
        fightstg_status_open(obj, data);
        switch (obj->page) {
        case 0:
            fightstg_status_show_stats(obj, data);
            break;
        case 1:
        case 2:
            fightstg_status_show_techs(obj, data);
            break;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        fightstg_status_draw_arrows(obj);
        switch (obj->page) {
        case 0:
            fightstg_status_draw_stats(obj);
            break;
        case 1:
        case 2:
            fightstg_status_draw_techs(obj);
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        fightstg_status_draw_arrows(obj);
        switch (obj->page) {
        case 0:
            fightstg_status_draw_stats(obj);
            break;
        case 1:
        case 2:
            fightstg_status_draw_techs(obj);
            break;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FightstgStatus *fightstg_status_create(s32 member, s32 page, s32 entry) {
    FightstgStatus *obj = object_new(fightstg_status_update, sizeof(FightstgStatus), sizeof(FightstgStatusData));

    obj->member = member;
    obj->page = page;
    obj->form = entry;
    return obj;
}

/* fightstg_item_menu_update's data block: the cursor and the message windows. */
typedef struct FightstgItemMenuData {
    /* 0x00 */ FightstgCursor *cursor;
    /* 0x04 */ MessageWindow *l1_window;
    /* 0x08 */ MessageWindow *r1_window;
    /* 0x0C */ MessageWindow *times_label;
    /* 0x10 */ MessageWindow *count_window; /* how many of the item */
    /* 0x14 */ MessageWindow *description; /* the item's description */
    /* 0x18 */ MessageWindow *item_windows[7]; /* the page's items */
} FightstgItemMenuData; /* size 0x34 */

extern FightstgCursorParams fightstg_item_menu_cursor;

void fightstg_item_menu_draw(FightstgItemMenu *obj) {
    Sprite sprite;
    s32 *bank;
    s32 i;
    s32 k;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x140, 0);
    bank = cdload_module.get_subfile_by_id(0x2860000);
    for (i = 0; i < 7; i++) {
        k = obj->page * 7 + i;
        if (obj->count - 1 < k) {
            break;
        }
        sprite.draw(bank, records_funcs.get_item_icon(obj->list[k]), 0x1D, i * 14 + 0x45);
    }
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    if (gfx_module.funcs.get_time() & 0x10) {
        if (obj->page > 0) {
            sprite.draw(bank, 0x1F, 0x10, 0xA9);
        }
        if (obj->page < obj->pages - 1) {
            sprite.draw(bank, 0x20, 0x90, 0xA9);
        }
    }
    sprite.draw(bank, 0x27, 8, 0x3E);
    sprite.draw(bank, 0x28, 0xA6, 0xA0);
    sprite.draw(bank, 0x31, 0xB, 0xBC);
}

void fightstg_item_menu_open(FightstgItemMenu *obj, FightstgItemMenuData *data) {
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);
    s32 i;

    data->l1_window = message_create_window(0x1005, 3, 0x1A, 0xA9);
    data->l1_window->set_text(data->l1_window, text, 0x11);
    data->l1_window->set_palette(data->l1_window, 2);
    data->r1_window = message_create_window(0x1005, 3, 0x80, 0xA9);
    data->r1_window->set_text(data->r1_window, text, 0x12);
    data->r1_window->set_palette(data->r1_window, 2);
    data->times_label = message_create_window(0x1005, 1, 0xAC, 0xA5);
    data->times_label->set_text(data->times_label, text, 0xC);
    for (i = 0; i < 7; i++) {
        data->item_windows[i] = message_create_window(0x1005, 1, 0x2A, i * 14 + 0x45);
    }
    data->description = message_create_window(0x1005, 1, 0x14, 0xC2);
    data->count_window = message_create_window(0x1005, 1, 0xC6, 0xA5);
}

void fightstg_item_menu_show_page(FightstgItemMenu *obj, FightstgItemMenuData *data) {
    s32 i;
    s32 k;
    s32 id;

    if (obj->count != 0) {
        for (i = 0; i < 7; i++) {
            k = obj->page * 7 + i;
            if (obj->count - 1 < k) {
                data->item_windows[i]->set_visible(data->item_windows[i], 0);
            } else {
                id = obj->list[k];
                if (id != 0) {
                    data->item_windows[i]->set_text(data->item_windows[i], cdload_module.files.get_file(records_language + 0x6A), id);
                }
            }
        }
        id = obj->list[obj->page * 7 + data->cursor->sel];
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x63), id);
        data->count_window->set_line_number(data->count_window, 0, gamestate_data.items[id]);
    } else {
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x7F), 0x15);
        data->count_window->set_text(data->count_window, cdload_module.files.get_file(records_language + 0x7F), 0x1A);
    }
    data->count_window->measure(data->count_window, 1);
}

void fightstg_item_menu_update(FightstgItemMenu *obj, FightstgItemMenuData *data) {
    s32 count;
    s32 i;
    s32 n;
    s32 page;
    s32 pad;
    s32 shown;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        count = records_funcs.list_items(1, (u16 *)obj->items);
        obj->count = 0;
        for (i = 0; i < count; i++) {
            if (obj->items[i] == 0) {
                break;
            }
            if (((RecordsUsable *)records_funcs.get_item(obj->items[i])->data)->flags & 2) {
                obj->count++;
            }
        }
        if (obj->count != 0) {
            obj->list = heap_funcs.alloc(obj->count * sizeof(s16), 2);
            for (i = 0, n = 0; i < count; i++) {
                if (obj->items[i] == 0) {
                    break;
                }
                if (((RecordsUsable *)records_funcs.get_item(obj->items[i])->data)->flags & 2) {
                    obj->list[n++] = obj->items[i];
                }
            }
            if (obj->count % 7 != 0) {
                obj->pages = obj->count / 7 + 1;
            } else {
                obj->pages = obj->count / 7;
            }
            if (obj->count != 0) {
                if (obj->count >= 8) {
                    fightstg_item_menu_cursor.count = 7;
                } else {
                    fightstg_item_menu_cursor.count = obj->count;
                }
            }
        }
        data->cursor = fightstg_cursor_create(&fightstg_item_menu_cursor);
        fightstg_item_menu_open(obj, data);
        fightstg_item_menu_show_page(obj, data);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class C, block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original places the cancel block out
         * of line, after the unk_54 store. */
        LOOP_BLOCK(
            fightstg_item_menu_draw(obj);
            pad = pad_state.get_pressed(0);
            page = obj->page;
            if (obj->pages != 0) {
                if (pad & 0x400) {
                    obj->page = page - 1;
                    if (obj->page < 0) {
                        obj->page = 0;
                    }
                } else if (pad & 0x800) {
                    obj->page = page + 1;
                    if (obj->pages - 1 < obj->page) {
                        obj->page = obj->pages - 1;
                    }
                }
            }
            if (page != obj->page) {
                data->cursor->sel = 0;
                shown = obj->count - obj->page * 7;
                if (shown >= 8) {
                    shown = 7;
                }
                data->cursor->params.count = shown;
                fightstg_item_menu_show_page(obj, data);
                sound_module.play(0x4001B);
            } else if (obj->shown != data->cursor->sel) {
                fightstg_item_menu_show_page(obj, data);
            } else if (pad & 0x2000) {
                sound_module.play(0x4001C);
                if (obj->count != 0) {
                    *obj->result = obj->list[obj->page * 7 + data->cursor->sel];
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    data->cursor->locked = 1;
                    break;
                }
            } else if (pad & 0x4000) {
                sound_module.play(0x800450BD);
                *obj->result = -2;
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            obj->shown = data->cursor->sel;
        );
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        if (obj->list != NULL) {
            heap_funcs.free(obj->list);
        }
        break;
    }
}

FightstgItemMenu *fightstg_item_menu_create(s32 *result) {
    FightstgItemMenu *obj = object_new(fightstg_item_menu_update, sizeof(FightstgItemMenu), sizeof(FightstgItemMenuData));

    obj->result = result;
    *result = -1;
    return obj;
}

/* fightstg_tech_menu_update's data block: the cursor and the message windows. */
typedef struct FightstgTechMenuData {
    /* 0x00 */ FightstgCursor *cursor;
    /* 0x04 */ MessageWindow *l1_window;
    /* 0x08 */ MessageWindow *r1_window;
    /* 0x0C */ MessageWindow *mp_label;
    /* 0x10 */ MessageWindow *max_mp;
    /* 0x14 */ MessageWindow *slash;
    /* 0x18 */ MessageWindow *mp;
    /* 0x1C */ MessageWindow *tech_windows[6]; /* the page's techniques */
    /* 0x34 */ MessageWindow *description; /* the technique's description */
    /* 0x38 */ MessageWindow *cost_label;
    /* 0x3C */ MessageWindow *cost;      /* its cost */
} FightstgTechMenuData; /* size 0x40 */

void fightstg_tech_menu_draw(FightstgTechMenu *obj) {
    Sprite sprite;
    s32 *bank;
    s32 i;
    s32 k;
    s32 id;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x140, 0);
    bank = cdload_module.get_subfile_by_id(0x2860000);
    for (i = 0; i < 6; i++) {
        k = obj->page * 6 + i;
        if (k < obj->count) {
            id = obj->techniques[k] & 0x1FFF;
            if (id == 0) {
                break;
            }
            sprite.draw(bank, records_techniques[id - 1].element + 0x37, 0x1D, i * 14 + 0x45);
        }
    }
    sprite.set_vram_pos(0x200, 0);
    bank = cdload_module.get_subfile_by_id(0x4550000);
    if (gfx_module.funcs.get_time() & 0x10) {
        if (obj->page > 0) {
            sprite.draw(bank, 0x1F, 0x10, 0x9F);
        }
        if (obj->page < obj->pages - 1) {
            sprite.draw(bank, 0x20, 0x90, 0x9F);
        }
    }
    sprite.draw(bank, 0x2A, 8, 0x3E);
    sprite.draw(bank, 0x29, 0xA3, 0x21);
    sprite.draw(bank, 0x31, 0xB, 0xBC);
}

void fightstg_tech_menu_open(FightstgTechMenu *obj, FightstgTechMenuData *data) {
    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);
    s32 i;

    data->l1_window = message_create_window(0x1005, 3, 0x1A, 0x9F);
    data->l1_window->set_text(data->l1_window, text, 0x11);
    data->l1_window->set_palette(data->l1_window, 2);
    data->r1_window = message_create_window(0x1005, 3, 0x80, 0x9F);
    data->r1_window->set_text(data->r1_window, text, 0x12);
    data->r1_window->set_palette(data->r1_window, 2);
    data->mp_label = message_create_window(0x1005, 3, 0xAC, 0x3A);
    data->mp_label->set_text(data->mp_label, text, 0xD);
    data->mp = message_create_window(0x1005, 3, 0xD8, 0x3A);
    if (member->blasted != 0) {
        if (member->mp < 100) {
            data->mp->set_text(data->mp, text, 0x1A);
        } else if (member->mp < 1000) {
            data->mp->set_text(data->mp, text, 0xF);
        } else {
            data->mp->set_text(data->mp, text, 0x24);
        }
    } else {
        data->mp->set_line_number(data->mp, 0, member->mp);
    }
    data->mp->measure(data->mp, 1);
    data->slash = message_create_window(0x1005, 3, 0xD9, 0x3A);
    data->slash->set_text(data->slash, text, 0x10);
    data->max_mp = message_create_window(0x1005, 3, 0xFB, 0x3A);
    data->max_mp->set_line_number(data->max_mp, 0, member->max_mp);
    data->max_mp->measure(data->max_mp, 1);
    for (i = 0; i < 6; i++) {
        data->tech_windows[i] = message_create_window(0x1005, 1, 0x2A, i * 14 + 0x45);
    }
    data->description = message_create_window(0x1005, 1, 0x14, 0xC2);
    data->cost_label = message_create_window(0x1005, 1, 0x100, 0xD0);
    data->cost = message_create_window(0x1005, 1, 0x12B, 0xD0);
}

void fightstg_tech_menu_show_page(FightstgTechMenu *obj, FightstgTechMenuData *data) {
    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    s32 i;
    s32 k;
    s32 id;
    s32 cost;

    if (obj->count != 0) {
        for (i = 0; i < 6; i++) {
            k = obj->page * 6 + i;
            if (obj->count - 1 < k) {
                data->tech_windows[i]->set_visible(data->tech_windows[i], 0);
            } else {
                id = obj->techniques[k];
                data->tech_windows[i]->set_text(data->tech_windows[i], cdload_module.files.get_file(records_language + 0xA2), id & 0x1FFF);
                cost = fightstg_rules.get_tech_cost(0, id);
                if (member->blasted != 0) {
                    if (id & 0x8000) {
                        data->tech_windows[i]->set_palette(data->tech_windows[i], 3);
                    } else if (id & 0x4000) {
                        data->tech_windows[i]->set_palette(data->tech_windows[i], 4);
                    } else {
                        data->tech_windows[i]->set_palette(data->tech_windows[i], 0);
                    }
                } else if (member->mp < cost) {
                    data->tech_windows[i]->set_palette(data->tech_windows[i], 7);
                } else if (id & 0x8000) {
                    data->tech_windows[i]->set_palette(data->tech_windows[i], 3);
                } else if (id & 0x4000) {
                    data->tech_windows[i]->set_palette(data->tech_windows[i], 4);
                } else {
                    data->tech_windows[i]->set_palette(data->tech_windows[i], 0);
                }
            }
        }
        cost = fightstg_rules.get_tech_cost(0, obj->techniques[obj->page * 6 + data->cursor->sel]);
        id = obj->techniques[obj->page * 6 + data->cursor->sel];
        if (member->blasted == 0 && member->mp < cost) {
            data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x7F), 0x54);
            data->cost_label->set_text(data->cost_label, cdload_module.files.get_file(records_language + 0x7F), 0xD);
            data->cost_label->set_palette(data->cost_label, 7);
            data->cost->set_line_number(data->cost, 0, cost);
            data->cost->measure(data->cost, 1);
            data->cost->set_palette(data->cost, 7);
        } else {
            data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x9B), id & 0x1FFF);
            data->cost_label->set_text(data->cost_label, cdload_module.files.get_file(records_language + 0x7F), 0xD);
            data->cost->set_line_number(data->cost, 0, cost);
            data->cost->measure(data->cost, 1);
            if (id & 0x4000) {
                data->cost_label->set_palette(data->cost_label, 4);
                data->cost->set_palette(data->cost, 4);
            } else {
                data->cost_label->set_palette(data->cost_label, 0);
                data->cost->set_palette(data->cost, 0);
            }
        }
    } else {
        data->description->set_text(data->description, cdload_module.files.get_file(records_language + 0x7F), 0x13);
    }
}

extern FightstgCursorParams fightstg_tech_menu_cursor;

/* The technique menu's update: lists the acting member's techniques (its own form's technique, those of
 * its items' records for its form, and the 0x4000 ones of the other records), then lets one be picked
 * (cost paid unless unk_1A). */
void fightstg_tech_menu_update(FightstgTechMenu *obj, FightstgTechMenuData *data) {
    s32 list[10];
    FightstgMember *m;
    FightstgMember *member;
    RecordsDigimon *rec;
    RecordsDigimon *own;
    s32 form;
    s32 n;
    s32 id;
    s32 i;
    s32 j;
    s32 count;
    s32 found;
    s32 tech;
    s32 shown;
    s32 pad;
    s32 page;
    s32 cost;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        id = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
        m = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        form = m->digimon;
        rec = &records_digimon[id];
        if (form == rec->id) {
            obj->techniques[0] = rec->techniques[6] | 0x8000;
            obj->count = 1;
        } else {
            n = gamestate_data.funcs.get_chosen_forms(id, obj->forms);
            obj->count = 0;
            for (i = 0; i < n; i++) {
                gamestate_data.funcs.get_form(id, obj->forms[i], &obj->form_records[i]);
                if (form == obj->form_records[i].id) {
                    for (j = 0; j < 6; j++) {
                        if (obj->form_records[i].techniques[j] != 0) {
                            obj->techniques[obj->count++] = (s16)(obj->form_records[i].techniques[j] & ~0x4000);
                        }
                    }
                }
            }
            if (m->blasted != 0) {
                own = records_get_digimon_func(m->digimon);
                found = 0;
                for (j = 0; j < obj->count; j++) {
                    if ((obj->techniques[j] & 0x1FFF) == own->techniques[6]) {
                        obj->techniques[j] = (obj->techniques[j] & 0x1FFF) | 0x8000;
                        found = -1;
                        break;
                    }
                }
                if (found != -1) {
                    obj->techniques[obj->count++] = own->techniques[6] | 0x8000;
                }
            }
            for (j = 0; j < 10; j++) {
                list[j] = 0;
            }
            count = 0;
            for (i = 0; i < n; i++) {
                if (form != obj->form_records[i].id) {
                    for (j = 0; j < 6; j++) {
                        if (obj->form_records[i].techniques[j] & 0x4000) {
                            list[count++] = obj->form_records[i].techniques[j];
                        }
                    }
                }
            }
            for (j = 0; j < count; j++) {
                tech = list[j] & 0x1FFF;
                for (i = 0; i < obj->count; i++) {
                    if (tech == (obj->techniques[i] & 0x1FFF)) {
                        tech = 0;
                        break;
                    }
                }
                if (tech != 0) {
                    obj->techniques[obj->count++] = list[j];
                }
            }
        }
        if (obj->count != 0) {
            if (obj->count % 6 != 0) {
                obj->pages = obj->count / 6 + 1;
            } else {
                obj->pages = obj->count / 6;
            }
        }
        if (obj->count != 0) {
            if (obj->count >= 7) {
                fightstg_tech_menu_cursor.count = 6;
            } else {
                fightstg_tech_menu_cursor.count = obj->count;
            }
        } else {
            fightstg_tech_menu_cursor.count = 1;
        }
        data->cursor = fightstg_cursor_create(&fightstg_tech_menu_cursor);
        fightstg_tech_menu_open(obj, data);
        fightstg_tech_menu_show_page(obj, data);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        /* Evidence (class C, block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original places the cancel block and
         * the confirm stub (a jump into the confirm tail) out of line before case 1. */
        LOOP_BLOCK(
            fightstg_tech_menu_draw(obj);
            pad = pad_state.get_pressed(0);
            page = obj->page;
            if (obj->pages != 0) {
                if (pad & 0x400) {
                    if (--obj->page < 0) {
                        obj->page = 0;
                    }
                } else if (pad & 0x800) {
                    if (++obj->page > obj->pages - 1) {
                        obj->page = obj->pages - 1;
                    }
                }
            }
            if (page != obj->page) {
                data->cursor->sel = 0;
                shown = obj->count - obj->page * 6;
                if (shown >= 7) {
                    shown = 6;
                }
                data->cursor->params.count = shown;
                fightstg_tech_menu_show_page(obj, data);
                sound_module.play(0x4001B);
            } else if (obj->shown != data->cursor->sel) {
                fightstg_tech_menu_show_page(obj, data);
            } else if (pad & 0x2000) {
                sound_module.play(0x4001C);
                if (obj->count != 0) {
                    member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                    if (member->blasted != 0) {
                        *obj->result = obj->techniques[obj->page * 6 + data->cursor->sel] & 0x1FFF;
                        obj->base.set_state(obj, OBJECT_STATE_END);
                        data->cursor->locked = 1;
                        break;
                    }
                    cost = fightstg_rules.get_tech_cost(0, obj->techniques[obj->page * 6 + data->cursor->sel]);
                    if (member->mp >= cost) {
                        member->mp -= cost;
                        *obj->result = obj->techniques[obj->page * 6 + data->cursor->sel] & 0x1FFF;
                        obj->base.set_state(obj, OBJECT_STATE_END);
                        data->cursor->locked = 1;
                        break;
                    }
                }
            } else if (pad & 0x4000) {
                sound_module.play(0x800450BD);
                *obj->result = -2;
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            obj->shown = data->cursor->sel;
        );
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgTechMenu *fightstg_tech_menu_create(s32 *result) {
    FightstgTechMenu *obj = object_new(fightstg_tech_menu_update, sizeof(FightstgTechMenu), sizeof(FightstgTechMenuData));

    obj->result = result;
    *result = -1;
    return obj;
}

/* The acting member's item (its record's unk_3D) if member `target` holds it, else 0. */
s32 fightstg_member_menu_get_team_item(FightstgMemberMenu *obj, s32 member, s32 target) {
    s32 id;
    s32 item;
    s32 n;
    s32 i;
    FightstgMember *m;

    gamestate_data.funcs.get_party_member(member);
    id = gamestate_data.funcs.get_party_member(target);
    m = &fightstg_battle.state.members[0][member];
    item = records_get_digimon_func(m->digimon)->level_thresholds[6];
    n = gamestate_data.funcs.get_chosen_forms(id, obj->forms);
    if (n <= 0 || item == 0) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (obj->forms[i] == records_digimon[item - 1].id) {
            return item;
        }
    }
    return 0;
}

void fightstg_member_menu_draw(FightstgMemberMenu *obj) {
    Sprite sprite;
    s32 *bank;
    s32 i;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    for (i = 0; i < obj->count; i++) {
        if (obj->team_items[i] != 0) {
            sprite.draw(bank, 0x30, 0x5A, i * 0x1F + 0x45);
        }
    }
}

void fightstg_member_menu_open(FightstgMemberMenu *obj, FightstgMemberMenuData *data) {
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);
    s32 i;

    for (i = 0; i < obj->count; i++) {
        data->hp_labels[i] = message_create_window(0x1005, 3, 0x6C, i * 0x20 + 0x4E);
        data->hp_labels[i]->set_text(data->hp_labels[i], text, 0xE);
        data->hp_slashes[i] = message_create_window(0x1005, 3, 0x99, i * 0x20 + 0x4E);
        data->hp_slashes[i]->set_text(data->hp_slashes[i], text, 0x10);
        data->mp_labels[i] = message_create_window(0x1005, 3, 0x6C, i * 0x20 + 0x5C);
        data->mp_labels[i]->set_text(data->mp_labels[i], text, 0xD);
        data->mp_slashes[i] = message_create_window(0x1005, 3, 0x99, i * 0x20 + 0x5C);
        data->mp_slashes[i]->set_text(data->mp_slashes[i], text, 0x10);
        data->hp_values[i] = message_create_window(0x1005, 3, 0x98, i * 0x20 + 0x4E);
        data->max_hp_values[i] = message_create_window(0x1005, 3, 0xBB, i * 0x20 + 0x4E);
        data->mp_values[i] = message_create_window(0x1005, 3, 0x98, i * 0x20 + 0x5C);
        data->max_mp_values[i] = message_create_window(0x1005, 3, 0xBB, i * 0x20 + 0x5C);
        data->names[i] = message_create_window(0x1005, 1, 0x24, i * 0x20 + 0x4C);
    }
}

void fightstg_member_menu_show(FightstgMemberMenu *obj, FightstgMemberMenuData *data) {
    FightstgMember *m;
    s32 i;

    for (i = 0; i < obj->count; i++) {
        m = &fightstg_battle.state.members[0][obj->members[i]];
        data->hp_values[i]->set_line_number(data->hp_values[i], 0, m->hp);
        data->hp_values[i]->measure(data->hp_values[i], 1);
        data->max_hp_values[i]->set_line_number(data->max_hp_values[i], 0, m->max_hp);
        data->max_hp_values[i]->measure(data->max_hp_values[i], 1);
        data->mp_values[i]->set_line_number(data->mp_values[i], 0, m->mp);
        data->mp_values[i]->measure(data->mp_values[i], 1);
        data->max_mp_values[i]->set_line_number(data->max_mp_values[i], 0, m->max_mp);
        data->max_mp_values[i]->measure(data->max_mp_values[i], 1);
        data->names[i]->set_text(data->names[i],
                                 gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(obj->members[i]))->name, -1);
        if (m->hp == 0) {
            data->names[i]->set_palette(data->names[i], 7);
        } else {
            data->names[i]->set_palette(data->names[i], 0);
        }
    }
}

/* Shape from the US decomp (func_800967A4; DECISIONS "Independent EU-only project on the unpatched disc"): case 0 reads the current member
 * into a local and the loop re-reads the global, case 1 keeps the cursor's sel in a local. */
extern FightstgCursorParams fightstg_member_menu_cursor;

void fightstg_member_menu_update(FightstgMemberMenu *obj, FightstgMemberMenuData *data) {
    Sprite sprite;
    FightstgMember *members;
    FightstgMember *m;
    s32 pad;
    s32 sel;
    s32 current;
    s32 flag;
    s32 i;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        current = fightstg_battle.state.current[0];
        members = fightstg_battle.state.members[0];
        flag = members[current].blasted;
        for (i = 0; i < 3; i++) {
            if (members[i].digimon != 0 && i != fightstg_battle.state.current[0]) {
                obj->members[obj->count] = i;
                if (obj->check_items != 0 && flag == 0) {
                    obj->team_items[obj->count] = fightstg_member_menu_get_team_item(obj, fightstg_battle.state.current[0], i);
                }
                obj->count++;
            }
        }
        if (obj->count > 0) {
            fightstg_member_menu_cursor.count = obj->count;
            data->cursor = fightstg_cursor_create(&fightstg_member_menu_cursor);
            data->cursor->sel = *obj->cursor;
            fightstg_member_menu_open(obj, data);
            fightstg_member_menu_show(obj, data);
            obj->base.next_state(obj);
        } else {
            data->none_window = message_create_window(0x1005, 1, 0x14, 0xC2);
            data->none_window->set_visible(data->none_window, 0);
            obj->base.set_state(obj, OBJECT_STATE_DONE);
        }
        break;
    case OBJECT_STATE_RUN:
        fightstg_member_menu_draw(obj);
        /* Class C (block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the cancel block goes out of line. */
        LOOP_BLOCK(
            pad = pad_state.get_pressed(0);
            if (obj->check_items != 0 && (pad & 0x4000)) {
                sound_module.play(0x800450BD);
                *obj->result = -2;
                obj->base.set_state(obj, OBJECT_STATE_END);
                break;
            }
            if (pad & 0x2000) {
                sound_module.play(0x4001C);
                sel = data->cursor->sel;
                m = &fightstg_battle.state.members[0][obj->members[sel]];
                if (m->hp != 0) {
                    *obj->cursor = sel;
                    *obj->result = obj->members[data->cursor->sel];
                    obj->base.set_state(obj, OBJECT_STATE_END);
                    data->cursor->locked = 1;
                }
            }
        );
        break;
    case OBJECT_STATE_DONE:
        if (pad_state.get_pressed(0) & 0x2000) {
            sound_module.play(0x4001C);
            *obj->result = -2;
            obj->base.set_state(obj, OBJECT_STATE_END);
            data->none_window->set_visible(data->none_window, 0);
            break;
        }
        sprite_init(&sprite);
        sprite.set_layer_id(0x1005, 1);
        if (obj->cursor_started != 0) {
            if (gfx_module.funcs.get_time() - obj->cursor_time >= 4) {
                obj->cursor_time = gfx_module.funcs.get_time();
                if (++obj->cursor_frame >= 5) {
                    obj->cursor_frame = 0;
                }
            }
            sprite.set_vram_pos(0x140, 0);
            sprite.set_palette(obj->cursor_frame);
            sprite.draw(cdload_module.get_subfile_by_id(0x2860000), 0xA, 0x123, 0xD0);
            sprite.set_palette(0);
        } else {
            obj->cursor_started = 1;
        }
        sprite.set_vram_pos(0x200, 0);
        sprite.draw(cdload_module.get_subfile_by_id(0x4550000), 0x31, 0xB, 0xBC);
        if (data->none_window->is_visible(data->none_window) == 0) {
            data->none_window->set_text(data->none_window, cdload_module.files.get_file(records_language + 0x7F), 0x14);
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FightstgMemberMenu *fightstg_member_menu_create(s32 *result, s32 *cursor, s32 team) {
    FightstgMemberMenu *obj = object_new(fightstg_member_menu_update, sizeof(FightstgMemberMenu), sizeof(FightstgMemberMenuData));

    obj->result = result;
    *result = -1;
    obj->cursor = cursor;
    obj->check_items = team;
    return obj;
}

void fightstg_switch_menu_draw(FightstgSwitchMenu *obj) {
    Sprite sprite;
    s32 *bank = cdload_module.get_subfile_by_id(0x4550000);

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0x2B, 0xA7, 0x8F);
    if (obj->action != 0) {
        sprite.draw(bank, 2, 0xA7, 0xB8);
        sprite.draw(bank, 0x29, 0xA3, 0x21);
    }
}

/* x of the windows mp_windows[] (fightstg_switch_menu_open). */
extern DVECTOR fightstg_switch_menu_window_x[4];

void fightstg_switch_menu_open(FightstgSwitchMenu *obj, FightstgSwitchMenuData *data) {
    s32 i;

    for (i = 0; i < 4; i++) {
        data->names[i] = message_create_window(0x1005, 1, 0xBB, i * 0x13 + 0x92);
    }
    data->action_windows[4] = message_create_window(0x1005, 1, 0xAB, 0x92);
    data->action_windows[0] = message_create_window(0x1005, 1, 0xBB, 0xA5);
    data->action_windows[1] = message_create_window(0x1005, 1, 0xBB, 0xB8);
    data->action_windows[2] = message_create_window(0x1005, 3, 0x10E, 0xBA);
    data->action_windows[3] = message_create_window(0x1005, 3, 0x133, 0xBA);
    for (i = 0; i < 4; i++) {
        data->mp_windows[i] = message_create_window(0x1005, 3, fightstg_switch_menu_window_x[i].vx, 0x39);
    }
}

void fightstg_switch_menu_show_forms(FightstgSwitchMenu *obj, FightstgSwitchMenuData *data, s32 show) {
    RecordsDigimon *rec;
    s32 i;

    if (show) {
        for (i = 0; i < obj->count; i++) {
            rec = records_get_digimon_func(obj->entries[i]);
            if (rec != NULL) {
                data->names[i]->set_text(data->names[i], cdload_module.files.get_file(records_language + 0x4E), rec->name_id);
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            if (data->names[i] != NULL) {
                data->names[i]->set_visible(data->names[i], show);
            }
        }
    }
}

void fightstg_switch_menu_show_actions(FightstgSwitchMenu *obj, FightstgSwitchMenuData *data, s32 show) {
    u8 *text = cdload_module.files.get_file(records_language + 0x7F);
    FightstgMember *self;
    FightstgMember *partner;
    RecordsDigimon *rec;
    s32 i;

    if (show) {
        self = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        partner = &fightstg_battle.state.members[0][obj->member];
        rec = records_get_digimon_func(obj->entries[obj->picked]);
        data->action_windows[4]->set_text(data->action_windows[4], cdload_module.files.get_file(records_language + 0x4E), rec->name_id);
        data->action_windows[0]->set_text(data->action_windows[0], text, 0x1B);
        if ((self->status & 0x10) || (partner->status & 0x10)) {
            data->action_windows[0]->set_palette(data->action_windows[0], 7);
        } else {
            data->action_windows[0]->set_palette(data->action_windows[0], 0);
        }
        if (obj->action != 0) {
            data->action_windows[1]->set_text(data->action_windows[1], text, 0x1C);
            data->action_windows[2]->set_text(data->action_windows[2], text, 0xD);
            data->action_windows[3]->set_line_number(data->action_windows[3], 0, records_techniques[obj->action - 1].mp_cost);
            data->action_windows[3]->measure(data->action_windows[3], 1);
            if (self->mp < records_techniques[obj->action - 1].mp_cost || partner->mp < records_techniques[obj->action - 1].mp_cost) {
                data->action_windows[1]->set_palette(data->action_windows[1], 7);
            } else if ((self->status & 0x20) || (partner->status & 0x20) || (self->status & 8)) {
                data->action_windows[1]->set_palette(data->action_windows[1], 7);
            } else if (self->hp <= 0 || partner->hp <= 0) {
                data->action_windows[1]->set_palette(data->action_windows[1], 7);
            } else {
                data->action_windows[1]->set_palette(data->action_windows[1], 0);
            }
            data->mp_windows[0]->set_text(data->mp_windows[0], text, 0xD);
            data->mp_windows[1]->set_line_number(data->mp_windows[1], 0, self->mp);
            data->mp_windows[1]->measure(data->mp_windows[1], 1);
            data->mp_windows[2]->set_text(data->mp_windows[2], text, 0x10);
            data->mp_windows[3]->set_line_number(data->mp_windows[3], 0, self->max_mp);
            data->mp_windows[3]->measure(data->mp_windows[3], 1);
        }
    } else {
        for (i = 0; i < 5; i++) {
            if (data->action_windows[i] != NULL) {
                data->action_windows[i]->set_visible(data->action_windows[i], show);
            }
        }
        for (i = 0; i < 4; i++) {
            if (data->mp_windows[i] != NULL) {
                data->mp_windows[i]->set_visible(data->mp_windows[i], show);
            }
        }
    }
}

/* The list's and the second step's parameters (fightstg_switch_menu_update). */
extern FightstgCursorParams fightstg_switch_menu_cursors[2];

void fightstg_switch_menu_update(FightstgSwitchMenu *obj, FightstgSwitchMenuData *data) {
    FightstgMember *self;
    FightstgMember *partner;
    RecordsDigimon *own;
    RecordsDigimon *rec;
    s32 id;
    s32 count;
    s32 i;
    s32 pad;
    s32 changed;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        id = obj->member_id;
        gamestate_data.funcs.get_chosen_forms(id, obj->forms);
        count = 1;
        obj->entries[0] = records_digimon[id].id;
        for (i = 0; i < 3; i++) {
            if (obj->forms[i] >= 3) {
                obj->entries[count++] = obj->forms[i];
            }
        }
        obj->count = count;
        fightstg_switch_menu_cursors[0].count = count;
        fightstg_switch_menu_open(obj, data);
        obj->page_shown = -1;
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
        default:
            switch (obj->base.substep) {
            case 0:
            default:
                data->cursor = fightstg_cursor_create(&fightstg_switch_menu_cursors[0]);
                data->cursor->sel = obj->picked;
                if (data->cursor_2 != NULL) {
                    data->cursor_2->base.set_state(data->cursor_2, OBJECT_STATE_END);
                }
                obj->base.next_substep(obj);
                break;
            case 1:
                /* Evidence (class C, block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original places the cancel and
                 * confirm blocks out of line (between cases 1/1/0 and 1/1/1). */
                LOOP_BLOCK(
                    pad = pad_state.get_pressed(0);
                    if (pad & 0x4000) {
                        sound_module.play(0x800450BD);
                        *obj->result = -2;
                        obj->base.set_state(obj, OBJECT_STATE_END);
                        break;
                    }
                    if (pad & 0x2000) {
                        sound_module.play(0x4001C);
                        obj->base.next_step(obj);
                        fightstg_switch_menu_show_forms(obj, data, 0);
                        data->cursor->locked = 1;
                        break;
                    }
                    if (pad & 0x800) {
                        if (++obj->page == 3) {
                            obj->page = 0;
                        }
                        sound_module.play(0x4001B);
                    } else if (pad & 0x400) {
                        if (--obj->page < 0) {
                            obj->page = 2;
                        }
                        sound_module.play(0x4001B);
                    } else {
                        fightstg_switch_menu_show_forms(obj, data, 1);
                        fightstg_switch_menu_show_actions(obj, data, 0);
                    }
                );
                break;
            }
            break;
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                if (fightstg_battle.state.members[0][fightstg_battle.state.current[0]].blasted == 0) {
                    own = records_get_digimon_func(fightstg_battle.state.members[0][fightstg_battle.state.current[0]].digimon);
                    rec = records_get_digimon_func(obj->entries[obj->picked]);
                    if (own->level_thresholds[6] != 0 && own->level_thresholds[6] == rec->name_id) {
                        obj->action = own->pair_technique;
                    } else {
                        obj->action = 0;
                    }
                }
                fightstg_switch_menu_cursors[1].count = obj->action != 0 ? 2 : 1;
                data->cursor_2 = fightstg_cursor_create(&fightstg_switch_menu_cursors[1]);
                if (data->cursor != NULL) {
                    data->cursor->base.set_state(data->cursor, OBJECT_STATE_END);
                }
                obj->base.next_substep(obj);
                break;
            case 1:
                /* Evidence (class C, block placement; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original places the cancel
                 * block out of line (before this case's code). */
                LOOP_BLOCK(
                    pad = pad_state.get_pressed(0);
                    if (pad & 0x2000) {
                        self = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                        partner = &fightstg_battle.state.members[0][obj->member];
                        sound_module.play(0x4001C);
                        if (data->cursor_2->sel == 0) {
                            if ((self->status & 0x10) || (partner->status & 0x10)) {
                                break;
                            }
                            *obj->result = 0;
                            *obj->result |= obj->entries[obj->picked] << 4;
                            obj->base.set_step(obj, 0);
                            fightstg_switch_menu_show_actions(obj, data, 0);
                            data->cursor_2->locked = 1;
                        } else {
                            if (self->mp < records_techniques[obj->action - 1].mp_cost ||
                                partner->mp < records_techniques[obj->action - 1].mp_cost) {
                                break;
                            }
                            if ((self->status & 0x20) || (partner->status & 0x20) || (self->status & 8)) {
                                break;
                            }
                            if (self->hp <= 0 || partner->hp <= 0) {
                                break;
                            }
                            self->mp -= records_techniques[obj->action - 1].mp_cost;
                            partner->mp -= records_techniques[obj->action - 1].mp_cost;
                            *obj->result = data->cursor_2->sel;
                            *obj->result |= obj->entries[obj->picked] << 4;
                            *obj->action_result = obj->action;
                            obj->base.set_step(obj, 0);
                            fightstg_switch_menu_show_actions(obj, data, 0);
                            data->cursor_2->locked = 1;
                        }
                    } else if (pad & 0x4000) {
                        sound_module.play(0x800450BD);
                        obj->base.set_step(obj, 0);
                        fightstg_switch_menu_show_actions(obj, data, 0);
                        break;
                    } else if (pad & 0x800) {
                        if (++obj->page == 3) {
                            obj->page = 0;
                        }
                        sound_module.play(0x4001B);
                    } else if (pad & 0x400) {
                        if (--obj->page < 0) {
                            obj->page = 2;
                        }
                        sound_module.play(0x4001B);
                    } else {
                        fightstg_switch_menu_show_forms(obj, data, 0);
                        fightstg_switch_menu_show_actions(obj, data, 1);
                    }
                );
                break;
            }
            fightstg_switch_menu_draw(obj);
            break;
        }
        if (obj->page_shown != obj->page) {
            changed = 1;
        } else if (data->cursor == NULL) {
            changed = 0;
        } else if (obj->picked != data->cursor->sel) {
            obj->picked = data->cursor->sel;
            changed = 1;
        } else {
            changed = 0;
        }
        if (changed) {
            obj->page_shown = obj->page;
            if (data->status != NULL) {
                data->status->base.set_state(data->status, OBJECT_STATE_DONE);
                data->status_2 = fightstg_status_create(obj->member_id, obj->page, obj->picked);
            } else {
                if (data->status_2 != NULL) {
                    data->status_2->base.set_state(data->status_2, OBJECT_STATE_DONE);
                }
                data->status = fightstg_status_create(obj->member_id, obj->page, obj->picked);
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgSwitchMenu *fightstg_switch_menu_create(s32 *result) {
    FightstgSwitchMenu *obj = object_new(fightstg_switch_menu_update, sizeof(FightstgSwitchMenu), sizeof(FightstgSwitchMenuData));

    obj->result = result;
    obj->member = *result;
    obj->member_id = gamestate_data.funcs.get_party_member(*result);
    *result = -1;
    return obj;
}

FightstgSwitchMenu *fightstg_switch_menu_create_team(s32 *result, s32 *tech) {
    FightstgSwitchMenu *obj = fightstg_switch_menu_create(result);

    obj->action_result = tech;
    return obj;
}

void fightstg_message_draw(FightstgMessage *obj) {
    Sprite sprite;
    s32 *bank;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    if (obj->waiting != 0) {
        if (gfx_module.funcs.get_time() - obj->arrow_time >= 4) {
            obj->arrow_time = gfx_module.funcs.get_time();
            if (++obj->arrow_frame >= 5) {
                obj->arrow_frame = 0;
            }
        }
        sprite.set_vram_pos(0x140, 0);
        sprite.set_palette(obj->arrow_frame);
        sprite.draw(cdload_module.get_subfile_by_id(0x2860000), 0xA, 0x123, 0xD0);
        sprite.set_palette(0);
    }
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0x31, 0xB, 0xBC);
}

void fightstg_message_step(FightstgMessage *obj, FightstgMessageData *data) {
    s32 msg;

    switch (obj->base.step) {
    case 0:
    default:
        if (obj->ready != 0) {
            obj->base.step++;
        }
        break;
    case 1:
        obj->window_time = gfx_module.funcs.get_time();
        obj->window_frames = 3;
        data->windows[obj->window]->set_visible(data->windows[obj->window], 1);
        obj->base.step++;
        break;
    case 2:
        if (obj->window_frames < gfx_module.funcs.get_time() - obj->window_time) {
            if (++obj->window >= obj->window_count) {
                obj->waiting = 1;
                obj->base.step++;
            } else {
                obj->base.step = 1;
            }
        }
        break;
    case 3:
#ifdef PC_PORT
        if (port_mod_skip_dialogues != 0) {
            /* skip_dialogues: the battle's message goes on by itself (the press below, without its sound) */
            msg = obj->queue[++obj->queue_index];
            if (msg == 0) {
                obj->base.state = OBJECT_STATE_END;
            } else {
                obj->show(obj, msg, NULL);
                obj->base.set_step(obj, 0);
            }
            obj->waiting = 0;
            break;
        }
#endif
        if (pad_state.get_pressed(0) & 0x2000) {
            sound_module.play(0x4001C);
            msg = obj->queue[++obj->queue_index];
            if (msg == 0) {
                obj->base.state = OBJECT_STATE_END;
            } else {
                obj->show(obj, msg, NULL);
                obj->base.set_step(obj, 0);
            }
            obj->waiting = 0;
        }
        break;
    case 4:
        if (gfx_module.funcs.get_time() - obj->base.substep >= 0x15 || (pad_state.get_pressed(0) & 0x2000)) {
            obj->base.state = OBJECT_STATE_END;
        }
        break;
    }
}

void fightstg_message_update(FightstgMessage *obj, FightstgMessageData *data) {
    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        fightstg_message_draw(obj);
        fightstg_message_step(obj, data);
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Puts a member's name on line 1 of the first window: side 0 = party (member), else the enemy (member). */
void fightstg_message_set_name(FightstgMessage *obj, FightstgMessageData *data, s32 side, s32 member) {
    FightstgEnemyRecord *rec;
    FightstgMember *m;

    if (side == 0) {
        data->windows[0]->set_line_text(data->windows[0],
                                gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(member))->name, -1, 1);
    } else {
        m = &fightstg_battle.state.members[1][member];
        rec = fightstg_enemy_records.get(m->digimon);
        data->windows[0]->set_line_text(data->windows[0], cdload_module.files.get_file(records_language + 0x4E), rec->name, 1);
    }
}

/* Lists in `members` the members of side args[0] that message args[1] is about (e.g. 0: hurt, 1..4: with a status, 5: down). */
void fightstg_message_list_members(FightstgMessage *obj, s32 *args) {
    FightstgMember *m;
    s32 i;
    s32 side;

    side = args[0] != 0;
    obj->member_count = 0;
    m = fightstg_battle.state.members[side];
    switch (args[1]) {
    case 0:
    default:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0 && m[i].hp < m[i].max_hp) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 1:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0 && (m[i].status & 1)) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 2:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0 && (m[i].status & 2)) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 3:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0 && (m[i].status & 4)) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 4:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0 && m[i].status != 0) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 5:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp == 0) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 6:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    case 7:
    case 8:
    case 9:
        for (i = 0; i < 3; i++) {
            if (m[i].digimon != 0 && m[i].hp != 0) {
                obj->members[obj->member_count++] = i;
            }
        }
        break;
    }
}

/* Message 0xD (fightstg_message_show): the members listed by fightstg_message_list_members in the first window
 * (side args[0]), and in the second the text for args[1] (0: a value, args[2]). */
void fightstg_message_show_members(FightstgMessage *obj, FightstgMessageData *data, s32 *args) {
    FightstgEnemyRecord *rec;
    FightstgMember *m;
    u8 side;
    u32 kind;
    s32 id;
    s32 i;

    if (obj->member_count != 0) {
        side = args[0];
        kind = args[1];
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), obj->member_count + 0x15);
        if (side == 0) {
            for (i = 0; i < obj->member_count; i++) {
                id = gamestate_data.funcs.get_party_member(obj->members[i]);
                if (id >= 0) {
                    data->windows[0]->set_line_text(data->windows[0], gamestate_data.funcs.get_record(id)->name, -1, i + 1);
                }
            }
        } else {
            for (i = 0; i < obj->member_count; i++) {
                m = &fightstg_battle.state.members[1][obj->members[i]];
                if (m->digimon != 0) {
                    rec = fightstg_enemy_records.get(m->digimon);
                    data->windows[0]->set_line_text(data->windows[0], cdload_module.files.get_file(records_language + 0x4E), rec->name,
                                            i + 1);
                }
            }
        }
        switch (kind) {
        case 0:
        default:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x22);
            data->windows[1]->set_line_number(data->windows[1], 1, args[2]);
            break;
        case 1:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x28);
            break;
        case 2:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x29);
            break;
        case 3:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x2A);
            break;
        case 4:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x2C);
            break;
        case 5:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x2D);
            break;
        case 6:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x2E);
            break;
        case 7:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x30);
            break;
        case 8:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x31);
            break;
        case 9:
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x32);
            break;
        }
        obj->window_count = 2;
    } else {
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x2F);
        obj->window_count = 1;
    }
}

/* Shows battle message msg (1..22) with its arguments in the message box's two windows: the first window
 * usually names the actor of side args[0] (fightstg_message_set_name), the second has the message itself. Some
 * messages queue a follow-up in `queue`. */
void fightstg_message_show(FightstgMessage *obj, s32 msg, s32 *args) {
    FightstgMessageData *data = (FightstgMessageData *)obj->base.children;
    s32 side;
    s32 total;

    obj->ready = 1;
    if (data->windows[0] == NULL) {
        data->windows[0] = message_create_window(0x1005, 1, 0x14, 0xC2);
    }
    if (data->windows[1] == NULL) {
        data->windows[1] = message_create_window(0x1005, 1, 0x14, 0xD0);
    }
    obj->window = 0;
    switch (msg) {
    case 1:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), args[0]);
        obj->window_count = 1;
        break;
    case 2:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), args[0]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[1], fightstg_battle.state.current[args[1] != 0]);
        break;
    case 3:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 9);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0xA2), args[1], 1);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 4:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0xB);
        data->windows[1]->set_line_number(data->windows[1], 1, args[1]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 5:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0xA);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0xA2), args[1], 1);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 6:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x25);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 7:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), args[0]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[1], args[2]);
        break;
    case 8:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x22);
        data->windows[1]->set_line_number(data->windows[1], 1, args[1]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 9:
        fightstg_message_list_members(obj, args);
        fightstg_message_show_members(obj, data, args);
        break;
    case 10:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x27);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], args[1]);
        obj->queue[1] = 11;
        obj->queue[2] = args[0];
        obj->queue[3] = args[1];
        obj->queue[4] = args[2];
        break;
    case 11:
        args = &obj->queue[obj->queue_index + 1];
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x22);
        data->windows[1]->set_line_number(data->windows[1], 1, args[2]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], args[1]);
        obj->queue_index += 3;
        break;
    case 12:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x37);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0x4E), args[1], 1);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 13:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x4E);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x4F);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0x4E), args[0], 1);
        obj->window_count = 2;
        break;
    case 14:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 9);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0x6A), args[1], 1);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 15:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x55);
        data->windows[1]->set_line_number(data->windows[1], 1, args[2]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], args[1]);
        break;
    case 16:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x23);
        total = args[1] * args[2];
        data->windows[1]->set_line_number(data->windows[1], 1, args[1]);
        data->windows[1]->set_line_number(data->windows[1], 2, args[2]);
        data->windows[1]->set_line_number(data->windows[1], 3, total);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 17:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x26);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 9);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0x6A), args[0], 1);
        obj->window_count = 2;
        break;
    case 18:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        if (args[2] == 0) {
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x22);
        } else {
            data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x8F);
        }
        data->windows[1]->set_line_number(data->windows[1], 1, args[1]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 19:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), args[1] + 0x49);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, args[0], fightstg_battle.state.current[args[0] != 0]);
        break;
    case 20:
        fightstg_message_show(obj, 4, args);
        obj->queue[1] = 21;
        obj->queue[2] = (args[0] == 0) << 4;
        obj->queue[3] = args[1];
        break;
    case 21:
        side = obj->queue[obj->queue_index + 1];
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 0x22);
        data->windows[1]->set_line_number(data->windows[1], 1, obj->queue[obj->queue_index + 2]);
        obj->window_count = 2;
        fightstg_message_set_name(obj, data, side, fightstg_battle.state.current[side != 0]);
        obj->queue_index += 2;
        break;
    case 22:
        data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x8C);
        data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), 9);
        data->windows[1]->set_line_text(data->windows[1], cdload_module.files.get_file(records_language + 0xA2), args[0], 1);
        obj->window_count = 2;
        break;
    }
    data->windows[0]->set_visible(data->windows[0], 0);
    data->windows[1]->set_visible(data->windows[1], 0);
}

void fightstg_message_close(FightstgMessage *obj) {
    obj->base.step = 4;
    obj->waiting = 0;
    obj->base.substep = gfx_module.funcs.get_time();
}

FightstgMessage *fightstg_message_create(void) {
    FightstgMessage *obj = object_new(fightstg_message_update, sizeof(FightstgMessage), sizeof(FightstgMessageData));

    obj->show = fightstg_message_show;
    obj->close = fightstg_message_close;
    return obj;
}

/* Pairs {queue entry (an ID), text line}, indexed by fightstg_confused_menu_show's arg1. */
typedef struct FightstgConfusedMessage {
    /* 0x0 */ s16 id;
    /* 0x2 */ s16 text;
} FightstgConfusedMessage; /* size 0x4 */

extern FightstgConfusedMessage fightstg_confused_menu_messages[];
extern s16 fightstg_confused_menu_order[8]; /* shuffled by fightstg_confused_menu_update */
extern FightstgCursorParams fightstg_confused_menu_cursor;

void fightstg_confused_menu_open(FightstgConfusedMenu *obj) {
    FightstgCommandMenuData *data = (FightstgCommandMenuData *)obj->base.children;
    u8 *text;
    s32 i;

    if (data->windows[0] == NULL) {
        text = cdload_module.files.get_file(records_language + 0x7F);
        for (i = 0; i < 6; i++) {
            data->windows[i] = message_create_window(0x1005, 1, 0x24, i * 0x13 + 0x6D);
            data->windows[i]->set_text(data->windows[i], text, fightstg_confused_menu_order[i] + 0x6B);
        }
    }
}

void fightstg_confused_menu_draw(FightstgConfusedMenu *obj) {
    Sprite sprite;
    s32 *bank;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    if (obj->waiting != 0) {
        if (gfx_module.funcs.get_time() - obj->cursor_time >= 4) {
            obj->cursor_time = gfx_module.funcs.get_time();
            if (++obj->cursor_frame >= 5) {
                obj->cursor_frame = 0;
            }
        }
        sprite.set_vram_pos(0x140, 0);
        sprite.set_palette(obj->cursor_frame);
        sprite.draw(cdload_module.get_subfile_by_id(0x2860000), 0xA, 0x123, 0xD0);
        sprite.set_palette(0);
    }
    bank = cdload_module.get_subfile_by_id(0x4550000);
    sprite.set_vram_pos(0x200, 0);
    sprite.draw(bank, 0x31, 0xB, 0xBC);
}

void fightstg_confused_menu_step(FightstgConfusedMenu *obj, FightstgCommandMenuData *data) {
    s32 pad = pad_state.get_pressed(0);

    switch (obj->base.step) {
    case 0:
    default:
        if (obj->ready != 0) {
            obj->base.step++;
        }
        break;
    case 1:
        obj->window_time = gfx_module.funcs.get_time();
        obj->frames_per_window = 3;
        data->windows[obj->window]->set_visible(data->windows[obj->window], 1);
        obj->base.step++;
        break;
    case 2:
        if (obj->frames_per_window < gfx_module.funcs.get_time() - obj->window_time) {
            if (++obj->window >= obj->windows) {
                obj->waiting = 1;
                if (pad & 0x2000) {
                    sound_module.play(0x4001C);
                    if (obj->queue[++obj->entry] == 0) {
                        obj->base.state = OBJECT_STATE_END;
                    } else {
                        obj->show(obj, obj->queue[obj->entry], 0);
                        obj->base.set_step(obj, 0);
                    }
                    obj->waiting = 0;
                }
            } else {
                obj->base.step = 1;
            }
        }
        break;
    }
}

void fightstg_confused_menu_set_name(FightstgConfusedMenu *obj, FightstgCommandMenuData *data, s32 arg2) {
    if (arg2 == 0) {
        data->windows[0]->set_line_text(data->windows[0],
                                 gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]))->name,
                                 -1, 1);
    }
}

void fightstg_confused_menu_show(FightstgConfusedMenu *obj, s32 arg1, s32 arg2) {
    FightstgCommandMenuData *data = (FightstgCommandMenuData *)obj->base.children;

    obj->ready = 1;
    if (data->windows[0] == NULL) {
        data->windows[0] = message_create_window(0x1005, 1, 0x14, 0xC2);
    }
    if (data->windows[1] == NULL) {
        data->windows[1] = message_create_window(0x1005, 1, 0x14, 0xD0);
    }
    obj->queue[0] = fightstg_confused_menu_messages[arg1].id;
    data->windows[0]->set_text(data->windows[0], cdload_module.files.get_file(records_language + 0x7F), 0x16);
    data->windows[1]->set_text(data->windows[1], cdload_module.files.get_file(records_language + 0x7F), fightstg_confused_menu_messages[arg1].text);
    obj->windows = 2;
    fightstg_confused_menu_set_name(obj, data, 0);
    data->windows[0]->set_visible(data->windows[0], 0);
    data->windows[1]->set_visible(data->windows[1], 0);
}

void fightstg_confused_menu_update(FightstgConfusedMenu *obj, FightstgCommandMenuData *data) {
    s32 i;
    s32 j;
    s16 tmp;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        data->cursor = fightstg_cursor_create(&fightstg_confused_menu_cursor);
        data->cursor->sel = obj->sel;
        for (i = 0; i < 8; i++) {
            j = pad_random.next() % 8;
            tmp = fightstg_confused_menu_order[i];
            fightstg_confused_menu_order[i] = fightstg_confused_menu_order[j];
            fightstg_confused_menu_order[j] = tmp;
        }
        fightstg_confused_menu_open(obj);
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 0:
            if (pad_state.get_pressed(0) & 0x2000) {
                sound_module.play(0x4001C);
                if (fightstg_rules.roll_confused(0) == 0) {
                    *obj->result = 0;
                    obj->base.set_state(obj, OBJECT_STATE_END);
                } else {
                    obj->selected = data->cursor->sel;
                    for (i = 0; i < 6; i++) {
                        data->windows[i]->base.set_state(data->windows[i], OBJECT_STATE_END);
                    }
                    data->cursor->base.set_state(data->cursor, OBJECT_STATE_END);
                    obj->portrait->state = OBJECT_STATE_END;
                    obj->idle_camera->state = OBJECT_STATE_END;
                    obj->base.next_step(obj);
                }
                data->cursor->locked = 1;
            }
            break;
        case 1:
            if (data->windows[0] == NULL && data->windows[1] == NULL) {
                fightstg_confused_menu_show(obj, fightstg_confused_menu_order[obj->selected] * 2 | (pad_random.next() & 1), 0);
                obj->base.set_state(obj, OBJECT_STATE_DONE);
            }
            break;
        }
        break;
    case OBJECT_STATE_DONE:
        fightstg_confused_menu_draw(obj);
        fightstg_confused_menu_step(obj, data);
        if (obj->base.state == OBJECT_STATE_END) {
            *obj->result = 1;
        }
        break;
    case OBJECT_STATE_END:
        break;
    }
}

FightstgConfusedMenu *fightstg_confused_menu_create(s32 *result, Object *portrait, Object *idle_camera) {
    FightstgConfusedMenu *obj = object_new(fightstg_confused_menu_update, sizeof(FightstgConfusedMenu), sizeof(FightstgCommandMenuData));

    obj->result = result;
    *result = -1;
    obj->sel = 0;
    obj->portrait = portrait;
    obj->idle_camera = idle_camera;
    return obj;
}

extern RECT fightstg_cursor_rect;
extern DR_MOVE fightstg_cursor_moves[4];
extern u32 fightstg_cursor_ot[2];

/* Vsync callback of fightstg_cursor_update: copies the highlight into VRAM (cursor entry sel, and
 * restores entry drawn_sel). */
/* `arg` is gfx_module.vsync_arg, the cursor object as an s32 (PTR_TO_S32 in fightstg_cursor_update). */
void fightstg_cursor_copy_highlight(s32 arg) {
    FightstgCursor *obj = S32_TO_PTR(FightstgCursor *, arg);
    u32 *cont;
    s32 i;
    s32 y;

    cont = BreakDraw();
    if (cont != (u32 *)-1) {
        if (++obj->highlight == 12) {
            obj->highlight = 0;
        }
        ClearOTag(fightstg_cursor_ot, 2);
        for (i = 0; i < 4; i += 2) {
            if (i == 2 && obj->drawn_sel == obj->sel) {
                break;
            }
            /* Evidence (class B, register priority only; docs/MATCHING.md "LOOP_BLOCK and LOOP_BARRIER"): the original keeps obj in s1;
             * without the block only registers differ, also with scheduling off (one block, in either branch). */
            if (i < 2) {
                fightstg_cursor_rect.x = obj->highlight * 12;
                LOOP_BLOCK(y = obj->params.vram_y + obj->sel * obj->params.vram_step;);
            } else {
                fightstg_cursor_rect.x = 0x90;
                y = obj->params.vram_y + obj->drawn_sel * obj->params.vram_step;
            }
            SetDrawMove(&fightstg_cursor_moves[i], &fightstg_cursor_rect, obj->params.vram_x, y);
            addPrim(fightstg_cursor_ot, &fightstg_cursor_moves[i]);
            SetDrawMove(&fightstg_cursor_moves[i + 1], &fightstg_cursor_rect, obj->params.vram_x, y + 0x100);
            addPrim(fightstg_cursor_ot, &fightstg_cursor_moves[i + 1]);
        }
        while (IsIdleGPU(0) != 0) {
            PLATFORM_WAIT();
        }
        ContinueDraw(fightstg_cursor_ot, cont);
    }
    obj->drawn_sel = obj->sel;
}

void fightstg_cursor_draw(FightstgCursor *obj) {
    Sprite sprite;
    s32 *bank;
    s32 i;
    s32 y;
    s32 frame;

    sprite_init(&sprite);
    sprite.set_layer_id(0x1005, 1);
    sprite.set_vram_pos(0x200, 0);
    bank = cdload_module.get_subfile_by_id(0x4550000);
    y = obj->params.y;
    for (i = 0; i < obj->params.count; i++) {
        if (obj->times[i] != 0 || obj->sel == i) {
            obj->times[i] += gfx_module.funcs.get_frame_ticks();
            if (obj->times[i] >= 20) {
                if (obj->sel != i) {
                    obj->times[i] = 0;
                } else {
                    obj->times[i] -= 20;
                }
            }
        }
        frame = obj->times[i] >> 2;
        if (frame >= 0 && frame < 5) {
            sprite.set_palette(frame);
        } else {
            sprite.set_palette(0);
        }
        sprite.draw(bank, obj->params.sprite, obj->params.x, y);
        y += obj->params.y_step;
    }
}

void fightstg_cursor_copy_highlight(s32 arg);

void fightstg_cursor_update(FightstgCursor *obj) {
    s32 pad;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_RUN:
        switch (obj->base.step) {
        case 2:
        default:
            gfx_module.vsync_callback = fightstg_cursor_copy_highlight;
            gfx_module.vsync_arg = PTR_TO_S32(obj);
        case 0:
        case 1:
            obj->base.next_step(obj);
            break;
        case 3:
            break;
        }
        if (obj->params.sprite != -1) {
            fightstg_cursor_draw(obj);
        }
        if (obj->locked == 0) {
            pad = pad_state.get_repeat(0) | pad_state.get_pressed(0);
            if (pad & 0x10) {
                if (obj->sel != 0) {
                    obj->sel--;
                    sound_module.play(0x4001B);
                }
            } else if (pad & 0x40) {
                if (obj->sel != obj->params.count - 1) {
                    obj->sel++;
                    sound_module.play(0x4001B);
                }
            }
        }
        break;
    case OBJECT_STATE_DONE:
        break;
    case OBJECT_STATE_END:
        gfx_module.vsync_callback = NULL;
        break;
    }
}

FightstgCursor *fightstg_cursor_create(FightstgCursorParams *params) {
    FightstgCursor *obj = object_new(fightstg_cursor_update, sizeof(FightstgCursor), 0);

    obj->params = *params;
    return obj;
}

void fightstg_jump_update(FightstgJump *obj) {
    s32 y;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->z_offset = 0;
        switch (obj->kind) {
        case 4:
            obj->z_offset = obj->distance + 0x2800;
            break;
        case 5:
            obj->z_offset = obj->params->pos.z - obj->params->start_pos.z;
            if (obj->z_offset < 0) {
                obj->z_offset = -obj->z_offset;
            }
            break;
        }
        if (obj->params->id == 0x10) {
            obj->z_offset = -obj->z_offset;
        }
        obj->time = fightstg_battle.frames * obj->speed;
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        switch (obj->kind) {
        default:
            y = fightstg_math.wave(2, obj->time, obj->height);
            obj->params->pos.y = fightstg_math.wave(1, obj->time, obj->start_y) - y;
            break;
        case 4:
        case 5:
            obj->params->pos.y = obj->params->start_pos.y - fightstg_math.wave(2, obj->time, obj->height);
            break;
        case 6:
            obj->params->pos.y = fightstg_math.wave(0, obj->time, obj->params->start_pos.y);
            break;
        }
        if (obj->kind == 4) {
            obj->params->pos.z = obj->params->start_pos.z + fightstg_math.wave(0, obj->time, obj->z_offset);
        }
        if (obj->kind == 5) {
            obj->params->pos.z = obj->params->start_pos.z + obj->z_offset - fightstg_math.wave(0, obj->time, obj->z_offset);
        }
        obj->time += fightstg_battle.frames * obj->speed;
        if (obj->time >= 0x1000) {
            obj->base.next_state(obj);
        }
        break;
    case OBJECT_STATE_DONE:
        switch (obj->kind) {
        default:
            obj->params->pos.y = 0;
            break;
        case 4:
        case 5:
        case 6:
            obj->params->pos.y = obj->params->start_pos.y;
            break;
        }
        if (obj->kind == 4) {
            obj->params->pos.z = obj->params->start_pos.z + obj->z_offset;
        }
        if (obj->kind == 5) {
            obj->params->pos.z = obj->params->start_pos.z;
        }
        obj->base.next_state(obj);
        break;
    case OBJECT_STATE_END:
        break;
    }
}

OBJECT_V0(FightstgJump *) fightstg_jump_create(FightstgModelParams *target, s32 kind, s32 distance) {
    FightstgJump *obj = object_new(fightstg_jump_update, sizeof(FightstgJump), 0);

    obj->kind = kind;
    obj->params = target;
    obj->distance = distance;
    obj->start_y = target->pos.y;
    obj->height = fightstg_jump_params[kind - 1].height;
    obj->speed = fightstg_jump_params[kind - 1].speed;
    OBJECT_V0_RETURN(obj) /* PC_PORT: FINDINGS 8: callers use the object (v0) */
}

/* fightstg_move_create's object: moves a model (its parameters' pos) from `from` to `to`. */
typedef struct FightstgMove {
    /* 0x00 */ Object base;
    /* 0x50 */ FightstgModelParams *params;
    /* 0x54 */ FightstgVec from;
    /* 0x5A */ FightstgVec to;
    /* 0x60 */ s32 pos; /* position (0x1000 = done) */
    /* 0x64 */ s32 step; /* step: 0x1000 / frames */
} FightstgMove; /* size 0x68 */

void fightstg_move_update(FightstgMove *obj) {
    SVECTOR from;
    SVECTOR to;
    SVECTOR pos;
    s32 t;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        obj->pos = fightstg_battle.frames * obj->step;
        obj->base.next_state(obj);
    case OBJECT_STATE_RUN:
        t = obj->pos;
        from.vx = obj->from.x;
        from.vy = obj->from.y;
        from.vz = obj->from.z;
        to.vx = obj->to.x;
        to.vy = obj->to.y;
        to.vz = obj->to.z;
        fightstg_math.lerp(&from, &to, t, &pos);
        obj->params->pos.x = pos.vx;
        obj->params->pos.y = pos.vy;
        obj->params->pos.z = pos.vz;
        obj->pos += fightstg_battle.frames * obj->step;
        if (obj->pos >= 0x1000) {
            obj->params->pos.x = obj->to.x;
            obj->params->pos.y = obj->to.y;
            obj->params->pos.z = obj->to.z;
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

FightstgMove *fightstg_move_create(FightstgModelParams *params, FightstgVec *to, s32 frames) {
    FightstgMove *obj = object_new(fightstg_move_update, sizeof(FightstgMove), 0);

    obj->params = params;
    obj->to = *to;
    obj->from = params->pos;
    obj->pos = 0;
    obj->step = 0x1000 / frames;
    return obj;
}

void fightstg_sound_update(FightstgSound *obj) {
    switch (obj->base.state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    case OBJECT_STATE_RUN:
    default:
        obj->timer -= fightstg_battle.frames;
        if (obj->timer <= 0) {
            sound_module.key_off(obj->sound, obj->key);
            obj->base.set_state(obj, OBJECT_STATE_END);
        }
        break;
    }
}

/* Sound IDs, indexed by fightstg_sound_play's id. */
extern s32 fightstg_sounds[];

void *fightstg_sound_play(s32 id, s32 delay) {
    s32 sound;
    FightstgSound *obj;

    switch (id) {
    default:
        sound = fightstg_sounds[id];
        break;
    case 0x5A:
        sound = records_state.music;
        break;
    case 0x5B:
        sound_module.stop(0x20040006);
        return NULL;
    }
    if (delay == 0) {
        sound_module.play(sound);
        return NULL;
    }
    obj = object_new(fightstg_sound_update, sizeof(FightstgSound), 0);
    obj->sound = sound;
    obj->key = (s16)sound_module.play(sound);
    obj->timer = delay;
    return obj;
}

s32 fightstg_enemy_find_record(s32 id) {
    FightstgEnemyRecord *recs = (FightstgEnemyRecord *)cdload_module.files.get_file(0x1CF);
    s32 i;

    for (i = 0; recs[i].id != 0; i++) {
        if (recs[i].id == id) {
            return i;
        }
    }
    return -1;
}

FightstgEnemyRecord *fightstg_enemy_get_record(s32 id) {
    FightstgEnemyRecord *recs = (FightstgEnemyRecord *)cdload_module.files.get_file(0x1CF);
    s32 i;

    if ((i = fightstg_enemy_find_record(id)) >= 0) {
        return &recs[i];
    }
    return NULL;
}

void fightstg_events_add(FightstgNewEvent *event) {
    s32 i;
    s32 slot;

    for (i = 0, slot = -1; i < 99; i++) {
        if (fightstg_events.events[i].type == 0) {
            slot = i;
            break;
        }
    }
    if (slot != -1) {
        fightstg_events.events[slot].type = event->type;
        fightstg_events.events[slot].delay = event->delay;
        for (i = 0; i < 6; i++) {
            fightstg_events.events[slot].args[i] = event->args[i];
        }
    }
}

void fightstg_events_add_first(FightstgNewEvent *event) {
    s32 i;

    if (event->delay < 2) {
        event->delay = 2;
    }
    for (i = 0; i < 99; i++) {
        if (fightstg_events.events[i].type != 0) {
            fightstg_events.events[i].delay += event->delay;
        }
    }
    event->delay = 1;
    fightstg_events_add(event);
}

s32 fightstg_events_take_next(void) {
    FightstgEvent *event;
    s32 i;
    s32 min = 0x7FFF;
    s32 next = -1;
    s32 delay;
    s32 type;

    for (i = 0; i < 99; i++) {
        if (fightstg_events.events[i].type != 0) {
            if (next != -1 && fightstg_events.events[i].delay <= fightstg_events.events[next].delay
                && (fightstg_events.events[next].type == 2 || fightstg_events.events[next].type == 3)) {
                next = i;
                min = fightstg_events.events[i].delay;
            }
            if (fightstg_events.events[i].delay < min) {
                next = i;
                min = fightstg_events.events[i].delay;
            }
        }
    }
    if (next == -1) {
        return 0;
    }
    event = &fightstg_events.events[next];
    delay = event->delay;
    for (i = 0; i < 99; i++) {
        if (fightstg_events.events[i].type != 0) {
            fightstg_events.events[i].delay -= delay;
        }
    }
    fightstg_events.taken_type = event->type;
    switch (fightstg_events_take_modes[fightstg_events.taken_type]) {
    case 0:
        return 0;
    case -1:
        type = event->type;
        fightstg_events.taken = next;
        return type;
    default:
        type = event->type;
        event->type = 0;
        fightstg_events.taken = next;
        return type;
    }
}

s32 fightstg_events_find_from(s32 i) {
    s32 type = fightstg_events.find_type;

    if (type < 1 || type > 24) {
        return -1;
    }
    for (; i < 99; i++) {
        if (fightstg_events.events[i].type == type) {
            return i;
        }
    }
    return -1;
}

s32 fightstg_events_find(s32 type) {
    fightstg_events.find_type = type;
    return fightstg_events.found = fightstg_events_find_from(0);
}

s32 fightstg_events_find_next(void) {
    return fightstg_events.found = fightstg_events_find_from(fightstg_events.found + 1);
}

s32 fightstg_events_find_member(s32 type, u8 side, s32 member) {
    fightstg_events.find_type = type;
    fightstg_events.found = fightstg_events_find_from(0);
    while (fightstg_events.found >= 0) {
        if (fightstg_events.events[fightstg_events.found].args[0] == side
            && fightstg_events.events[fightstg_events.found].args[1] == member) {
            break;
        }
        fightstg_events.found = fightstg_events_find_from(fightstg_events.found + 1);
    }
    return fightstg_events.found;
}

void fightstg_events_remove_member(s32 *args) {
    s32 i;
    u8 kind = args[0];
    s32 id = args[1];

    for (i = 0; i < 99; i++) {
        if (fightstg_events.events[i].type != 0 && fightstg_events.events[i].args[0] == kind
            && fightstg_events.events[i].args[1] == id) {
            fightstg_events.events[i].type = 0;
        }
    }
}

/* Per formula of fightstg_events_get_delay: a base and the result's bounds (0: none). */
typedef struct FightstgDelayFormula {
    /* 0x0 */ s16 base;  /* base (random range, or factor) */
    /* 0x2 */ s16 min;   /* minimum */
    /* 0x4 */ s16 max;   /* maximum */
} FightstgDelayFormula; /* size 0x6 */

extern FightstgDelayFormula fightstg_events_delay_formulas[];

/* A value by formula kind for side's acting member (and the other side's): mostly random, from the stats
 * blocks of fightstg_rules_get_stats, bounded by fightstg_events_delay_formulas[kind]. */
s32 fightstg_events_get_delay(u8 side, s32 kind) {
    s32 value;
    s32 attack;
    s32 defense;

    switch (kind) {
    case 2: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);

        value = pad_random.next() % fightstg_events_delay_formulas[kind].base + own->stats[2] * 10;
        break;
    }
    case 8:
        value = pad_random.next() % 8001;
        break;
    case 9: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);
        FightstgStats *other =
            fightstg_rules.get_stats((side == 0) << 4, 0, fightstg_battle.state.current[1 - enemy]);

        value = pad_random.next() % fightstg_events_delay_formulas[kind].base;
        attack = (own->stats[2] + fightstg_events.status_power) * 8 + 3000;
        defense = (other->resists[8] + other->resists[4]) * 8;
        value += attack;
        value -= defense;
        break;
    }
    case 10: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);
        FightstgStats *other =
            fightstg_rules.get_stats((side == 0) << 4, 0, fightstg_battle.state.current[1 - enemy]);

        value = pad_random.next() % fightstg_events_delay_formulas[kind].base;
        attack = (own->stats[2] + fightstg_events.status_power) * 8 + 3000;
        defense = (other->resists[9] + other->resists[3]) * 8;
        value += attack;
        value -= defense;
        break;
    }
    case 11: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);
        FightstgStats *other =
            fightstg_rules.get_stats((side == 0) << 4, 0, fightstg_battle.state.current[1 - enemy]);

        value = pad_random.next() % fightstg_events_delay_formulas[kind].base;
        attack = (own->stats[2] + fightstg_events.status_power) * 8 + 1000;
        defense = (other->resists[10] + other->resists[2]) * 8;
        value += attack;
        value -= defense;
        break;
    }
    case 12: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);

        value = pad_random.next() % fightstg_events_delay_formulas[kind].base + 2000 + own->stats[2] * 10;
        break;
    }
    default: {
        s32 enemy = side != 0;
        FightstgStats *own = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[enemy]);
        FightstgStats *other = fightstg_rules.get_stats(0x10 - side, 0, fightstg_battle.state.current[1 - enemy]);
        s32 root = 999;
        s32 product = own->stats[4] * other->stats[4];
        s32 i;

        for (i = 0; i < 10; i++) {
            root = (root + product / root) / 2;
        }
        value = fightstg_events_delay_formulas[kind].base * other->stats[4] / root;
        break;
    }
    }
    if (fightstg_events_delay_formulas[kind].min != 0 && value < fightstg_events_delay_formulas[kind].min) {
        value = fightstg_events_delay_formulas[kind].min;
    }
    if (fightstg_events_delay_formulas[kind].max != 0 && value > fightstg_events_delay_formulas[kind].max) {
        value = fightstg_events_delay_formulas[kind].max;
    }
    return value;
}

/* Event types of the status effects 0..2 (the last three bytes: for "all", with them). */
extern u8 fightstg_status_event_types[8];
/* Flags (FightstgMember.status) of the status effects 0..3. */
extern u8 fightstg_status_flags[4];

/* Cures a member's status effect, by the item / action `id` (0xBE..0xC5). One case per id (cross-jumped later;
 * their references decide effect's register). */
void fightstg_events_cure_status(u8 side, s32 member, s32 id) {
    FightstgMember *m;
    s32 effect;
    s32 i;
    s32 j;
    s32 enemy;

    switch (id) {
    case 0xBE:
    default:
        effect = 0;
        break;
    case 0xBF:
        effect = 0;
        break;
    case 0xC0:
        effect = 1;
        break;
    case 0xC1:
        effect = 1;
        break;
    case 0xC2:
        effect = 2;
        break;
    case 0xC3:
        effect = 2;
        break;
    case 0xC4:
        effect = 3;
        break;
    case 0xC5:
        effect = 3;
        break;
    }
    enemy = side != 0;
    m = fightstg_battle.state.members[enemy];
    if (m[member].digimon != 0) {
        m[member].status &= ~fightstg_status_flags[effect];
        if (effect != 3) {
            j = fightstg_events.find_member(fightstg_status_event_types[effect], side, member);
            if (j >= 0) {
                fightstg_events.events[j].type = 0;
            }
        } else {
            for (i = 0; i < 6; i++) {
                j = fightstg_events.find_member(fightstg_status_event_types[i], side, member);
                if (j >= 0) {
                    fightstg_events.events[j].type = 0;
                }
            }
        }
    }
}

void fightstg_events_add_party_turn(s32 delay) {
    fightstg_new_event.type = 2;
    fightstg_new_event.delay = delay;
    fightstg_new_event.args[0] = -1;
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_add_enemy_turn(s32 delay) {
    fightstg_new_event.type = 3;
    fightstg_new_event.delay = delay;
    fightstg_new_event.args[0] = -1;
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_end_battle(s8 result) {
    fightstg_new_event.type = 1;
    fightstg_new_event.delay = 1;
    fightstg_new_event.args[0] = -1;
    fightstg_events.result = result;
    fightstg_events_add_first(&fightstg_new_event);
}

void fightstg_events_add_escape(u8 side) {
    fightstg_new_event.type = 4;
    fightstg_new_event.delay = fightstg_events_get_delay(side, 1);
    fightstg_new_event.args[0] = side;
    fightstg_new_event.args[1] = fightstg_battle.state.current[side != 0];
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_add_regen_end(u8 side) {
    s32 enemy = side != 0;
    s32 i = fightstg_events_find_member(5, side, enemy);
    s32 delay = fightstg_events_get_delay(side, 2);
    FightstgEvent *event;

    if (i >= 0) {
        event = &fightstg_events.events[i];
        event->delay = delay;
    } else {
        fightstg_new_event.type = 5;
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = fightstg_battle.state.current[enemy];
        fightstg_new_event.args[2] = 0xBD;
        fightstg_events_add(&fightstg_new_event);
    }
}

void fightstg_events_start_regen(u8 side, s32 arg1, s32 arg2) {
    s32 i = fightstg_events_find_member(6, side, arg1);
    FightstgEvent *event;

    if (i >= 0) {
        event = &fightstg_events.events[i];
        event->args[2] = 0xBD;
    } else {
        fightstg_new_event.type = 6;
        fightstg_new_event.delay = 1000;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = arg1;
        fightstg_new_event.args[2] = arg2;
        fightstg_events_add(&fightstg_new_event);
    }
}

void fightstg_events_set_field_end(s32 delay) {
    s32 i;

    fightstg_events.find_type = 7;
    i = fightstg_events_find_from(0);
    if (delay > 0x7FFF) {
        delay = 0x7FFF;
    }
    if (i >= 0) {
        fightstg_events.events[i].delay = delay;
    } else {
        fightstg_new_event.type = 7;
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = -1;
        fightstg_events_add(&fightstg_new_event);
    }
}

void fightstg_events_start_poison(u8 side, s32 member, s32 power) {
    s32 i = fightstg_events_find_member(9, side, member);
    u8 enemy;
    FightstgMember *m;
    FightstgEvent *event;

    if (i >= 0) {
        event = &fightstg_events.events[i];
        event->args[2] = power;
    } else {
        fightstg_new_event.type = 9;
        fightstg_new_event.delay = 1000;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = member;
        fightstg_new_event.args[2] = power;
        fightstg_events_add(&fightstg_new_event);
    }
    enemy = side != 0;
    m = &fightstg_battle.state.members[enemy][member];
    m->status |= 1;
}

void fightstg_events_start_paralysis(u8 side, s32 arg1, u8 power) {
    s32 enemy = side != 0;
    s32 i = fightstg_events_find_member(10, side, fightstg_battle.state.current[enemy]);
    s32 delay;
    FightstgMember *member;

    fightstg_events.status_power = power;
    delay = fightstg_events_get_delay(side, 9);
    if (i >= 0) {
        fightstg_events.events[i].delay = delay;
    } else {
        fightstg_new_event.type = 10;
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = fightstg_battle.state.current[enemy];
        fightstg_events_add(&fightstg_new_event);
    }
    member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    member->paralysis_power = power;
    member->status |= 2;
}

void fightstg_events_start_confusion(u8 side, s32 arg1, u8 power) {
    s32 enemy = side != 0;
    s32 i = fightstg_events_find_member(11, side, fightstg_battle.state.current[enemy]);
    s32 delay;
    FightstgMember *member;

    if (arg1 == 0) {
        delay = fightstg_events_get_delay(side, 8);
    } else {
        fightstg_events.status_power = power;
        delay = fightstg_events_get_delay(side, 10);
    }
    if (i >= 0) {
        fightstg_events.events[i].delay = delay;
    } else {
        fightstg_new_event.type = 11;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[1] = fightstg_battle.state.current[enemy];
        fightstg_events_add(&fightstg_new_event);
    }
    member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    member->confusion_power = power;
    member->status |= 4;
}

void fightstg_events_start_sleep(u8 side, s32 arg1, u8 power) {
    s32 enemy = side != 0;
    s32 i = fightstg_events_find_member(12, side, fightstg_battle.state.current[enemy]);
    s32 delay;
    FightstgMember *member;

    fightstg_events.status_power = power;
    delay = fightstg_events_get_delay(side, 11);
    if (i >= 0) {
        fightstg_events.events[i].delay = delay;
    } else {
        fightstg_new_event.type = 12;
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = fightstg_battle.state.current[enemy];
        fightstg_events_add(&fightstg_new_event);
    }
    member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    member->sleep_power = power;
    member->status |= 8;
}

void fightstg_events_start_modifier(u8 side, s32 member, s32 kind, s32 tech) {
    s32 i = fightstg_events_find_member(fightstg_modifier_event_types[kind], side, member);
    s32 delay;

    if (tech != 0) {
        delay = fightstg_events_get_delay(side, 12);
    } else {
        delay = fightstg_events_get_delay(side, 8);
    }

    if (i >= 0) {
        fightstg_events.events[i].delay = delay;
    } else {
        fightstg_new_event.type = fightstg_modifier_event_types[kind];
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = side;
        fightstg_new_event.args[1] = member;
        fightstg_new_event.args[2] = kind;
        fightstg_events_add(&fightstg_new_event);
    }
}

/* Event types, for kinds 0xD and 0xE (fightstg_events_start_seal). */
extern s32 fightstg_seal_event_types[2];

void fightstg_events_start_seal(s32 id) {
    RecordsTechnique *rec = &records_techniques[id - 1];
    s32 kind = 0;
    s32 member;
    s32 i;
    s32 delay;
    FightstgMember *m;
    FightstgEvent *event;

    if (rec->kind != 0xD) {
        kind = rec->kind == 0xE;
    }
    member = fightstg_battle.state.current[0];
    i = fightstg_events_find_member(fightstg_seal_event_types[kind], 0, member);
    delay = (pad_random.next() % 101 + 100) * rec->effect_power;
    if (i >= 0) {
        event = &fightstg_events.events[i];
        event->delay = delay;
    } else {
        fightstg_new_event.type = fightstg_seal_event_types[kind];
        fightstg_new_event.delay = delay;
        fightstg_new_event.args[0] = 0;
        fightstg_new_event.args[1] = member;
        fightstg_new_event.args[2] = kind;
        fightstg_events_add(&fightstg_new_event);
    }
    m = &fightstg_battle.state.members[0][member];
    if (kind == 0) {
        m->status |= 0x10;
    } else {
        m->status |= 0x20;
    }
}

void fightstg_events_add_counter_stance(s32 arg0) {
    fightstg_new_event.type = 8;
    fightstg_new_event.delay = 0x7FFF;
    fightstg_new_event.args[0] = 0;
    fightstg_new_event.args[1] = fightstg_battle.state.current[0];
    fightstg_new_event.args[2] = arg0;
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_add_gauge_full(void) {
    fightstg_new_event.type = 0x12;
    fightstg_new_event.delay = 0;
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_start_blast(s32 arg0) {
    fightstg_new_event.type = 0x13;
    fightstg_new_event.delay = fightstg_events_get_delay(0, arg0 + 3);
    fightstg_new_event.args[0] = 0;
    fightstg_new_event.args[1] = fightstg_battle.state.current[0];
    fightstg_events_add(&fightstg_new_event);
}

void fightstg_events_add_knockout(u8 side) {
    fightstg_new_event.type = 0x14;
    fightstg_new_event.delay = 1;
    fightstg_new_event.args[0] = side;
    fightstg_new_event.args[1] = fightstg_battle.state.current[side != 0];
    fightstg_events_add_first(&fightstg_new_event);
}

void fightstg_events_add_boost_end(void) {
    fightstg_new_event.type = 0x15;
    fightstg_new_event.delay = 1;
    fightstg_new_event.args[0] = 0;
    fightstg_new_event.args[1] = fightstg_battle.state.current[0];
    fightstg_events_add_first(&fightstg_new_event);
}

void fightstg_events_add_revert(void) {
    fightstg_new_event.type = 0x16;
    fightstg_new_event.delay = 1;
    fightstg_new_event.args[0] = 0;
    fightstg_new_event.args[1] = fightstg_battle.state.current[0];
    fightstg_events_add_first(&fightstg_new_event);
}

void fightstg_events_add_boss_entrance(void) {
    fightstg_new_event.type = 0x17;
    fightstg_new_event.delay = 1;
    fightstg_events_add_first(&fightstg_new_event);
}

void fightstg_events_start_final_phase(void) {
    FightstgEnemyRecord *rec;

    fightstg_new_event.type = 0x18;
    fightstg_new_event.delay = 3000;
    fightstg_events_add(&fightstg_new_event);
    rec = fightstg_enemy_records.get(0x1D3);
    fightstg_battle.state.final_phase = 1;
    fightstg_battle.state.members[1][0].modifiers[1] = -rec->stats[1] >> 1;
    fightstg_battle.state.members[1][0].modifiers[3] = -rec->stats[2] >> 1;
}

void fightstg_events_end_final_phase(void) {
    s32 i = fightstg_events.find(0x18);

    if (i >= 0) {
        fightstg_events.events[i].delay = 1;
    }
}

void fightstg_action_try_poison(void) {
    s32 enemy = fightstg_action.side != 0;
    FightstgMember *member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    s32 value = fightstg_rules.roll_poison(fightstg_action.side, fightstg_action.tech);

    if (value != 0) {
        if (member->boosted != 0) {
            value *= 2;
        }
        fightstg_action.effects[2] = value;
    }
}

void fightstg_action_try_paralysis(void) {
    FightstgAction *state = &fightstg_action;
    FightstgRules *rules = &fightstg_rules;
    s32 enemy = state->side != 0;
    FightstgMember *member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    RecordsTechnique *rec;
    s32 value;

    if (rules->roll_paralysis(state->side, state->tech) != 0) {
        rec = &records_techniques[state->tech - 1];
        if (rec->kind < 2) {
            value = rules->stats[0].paralysis_power;
        } else {
            value = rec->effect_power;
        }
        if (member->boosted != 0) {
            value *= 2;
        }
        fightstg_action.effects[3] = value;
    }
}

void fightstg_action_try_confusion(void) {
    FightstgAction *state = &fightstg_action;
    FightstgRules *rules = &fightstg_rules;
    s32 enemy = state->side != 0;
    FightstgMember *member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    RecordsTechnique *rec;
    s32 value;

    if (rules->roll_confusion(state->side, state->tech) != 0) {
        rec = &records_techniques[state->tech - 1];
        if (rec->kind < 2) {
            value = rules->stats[0].confusion_power;
        } else {
            value = rec->effect_power;
        }
        if (member->boosted != 0) {
            value *= 2;
        }
        fightstg_action.effects[4] = value;
    }
}

void fightstg_action_try_sleep(void) {
    s32 enemy = fightstg_action.side != 0;
    FightstgMember *member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    RecordsTechnique *rec;
    s32 value;

    if (fightstg_rules.roll_sleep(fightstg_action.side, fightstg_action.tech) != 0) {
        rec = &records_techniques[fightstg_action.tech - 1];
        value = rec->effect_power;
        if (member->boosted != 0) {
            value *= 2;
        }
        fightstg_action.effects[rec->kind] = value;
    }
}

void fightstg_action_try_knockout(void) {
    FightstgAction *state = &fightstg_action;

    if (fightstg_rules.roll_knockout(state->side, state->tech) != 0) {
        state->effects[6] = 1;
    }
}

void fightstg_action_hit_again(void) {
    RecordsTechnique *rec;
    s32 i;
    s32 count = 3;
    s32 side;
    FightstgBattle *battle = &fightstg_battle;

    rec = &records_techniques[fightstg_action.tech - 1];
    side = fightstg_action.side;
    if (rec->kind >= 2) {
        count = rec->hit_count;
    }
    fightstg_action.strikes = 1;
    fightstg_action.hit_count = fightstg_action.hits[0];
    if (side == 0 && battle->state.type == 6 && fightstg_action.hits[0] == 0) {
        fightstg_action.strikes = count;
    } else {
        for (i = 1; i < count; i++) {
            if (fightstg_rules.roll_hit(fightstg_action.side, fightstg_action.tech) != 0) {
                fightstg_action.hits[fightstg_action.strikes++] = 1;
                fightstg_action.hit_count++;
            } else {
                fightstg_action.hits[fightstg_action.strikes++] = 0;
            }
        }
    }
    fightstg_action.effects[9] = 1;
}

void fightstg_action_try_drain(void) {
    FightstgAction *state = &fightstg_action;
    FightstgRules *rules = &fightstg_rules;
    s32 enemy = state->side != 0;
    FightstgMember *member = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
    RecordsTechnique *rec;
    s32 value;

    if (rules->roll_drain(state->side, state->tech) != 0) {
        rec = &records_techniques[state->tech - 1];
        if (rec->kind == 8) {
            value = rec->effect_power;
            if (member->boosted != 0) {
                value *= 2;
            }
            state->drained = state->damage * value / 128;
        } else {
            value = rules->stats[0].drain_power;
            if (member->boosted != 0) {
                value *= 2;
            }
            state->drained = state->damage * value / 128;
        }
        fightstg_action.effects[8] = 1;
    }
}

void fightstg_action_mark_counter_tech(void) {
    RecordsTechnique *rec = &records_techniques[fightstg_action.tech - 1];

    fightstg_action.effects[rec->kind] = rec->kind;
}

void fightstg_action_mark_critical_tech(void) {
    RecordsTechnique *rec = &records_techniques[fightstg_action.tech - 1];

    fightstg_action.effects[rec->kind] = rec->kind;
}

/* rules is a const pointer (the US decomp's shape): the front end substitutes its value into the call, so the original's
 * &fightstg_rules stays in a register and roll_steal is loaded from it after the branch (lw 0xB8(v0)). A plain pointer
 * local or the direct call folds the address into the load (97.4%). Only this function reads roll_steal. */
void fightstg_action_try_steal(void) {
    FightstgRules *const rules = &fightstg_rules;
    FightstgMember *members = fightstg_battle.state.members[1];
    RecordsTechnique *rec;

    if (members[fightstg_battle.state.current[1]].item > 0 && rules->roll_steal(0, fightstg_action.tech) != 0) {
        rec = &records_techniques[fightstg_action.tech - 1];
        fightstg_action.effects[rec->kind] = 1;
        cdload_module.queue_file(records_language + 0x6A);
    }
}

void fightstg_action_lower_attack(void) {
    FightstgAction *state = &fightstg_action;
    s16 enemy = state->side != 0;
    RecordsTechnique *rec;

    if (enemy || records_state.blocked[8] == 0) {
        rec = &records_techniques[state->tech - 1];
        fightstg_rules.change_modifier(0x10 - state->side, fightstg_battle.state.current[1 - enemy], 0, -rec->effect_power);
        fightstg_events_start_modifier(0x10 - state->side, fightstg_battle.state.current[1 - enemy], 0, state->tech);
        state->effects[rec->kind] = rec->effect_power;
    }
}

void fightstg_action_lower_defense(void) {
    FightstgAction *state = &fightstg_action;
    s32 other = state->side == 0;
    RecordsTechnique *rec = &records_techniques[state->tech - 1];

    fightstg_rules.change_modifier(0x10 - state->side, fightstg_battle.state.current[other], 1, -rec->effect_power);
    fightstg_events_start_modifier(0x10 - state->side, fightstg_battle.state.current[other], 1, state->tech);
    state->effects[rec->kind] = rec->effect_power;
}

void fightstg_action_drain_mp(void) {
    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    FightstgAction *state = &fightstg_action;
    RecordsTechnique *rec = &records_techniques[state->tech - 1];

    if (member->mp != 0) {
        state->drained = member->max_mp * rec->effect_power / 128;
        if (member->mp < state->drained) {
            state->drained = member->mp;
        }
        state->effects[rec->kind] = state->drained;
    }
}

void fightstg_action_try_lower_stat(void) {
    s32 i = pad_random.next() % 2;
    FightstgAction *state = &fightstg_action;
    RecordsTechnique *rec;
    GamestateRecord *digimon;

    if (fightstg_rules.roll_lower_stat(state->side, state->tech) != 0) {
        rec = &records_techniques[state->tech - 1];
        digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]));
        digimon->stats.penalties[i] += rec->effect_power;
        state->effects[rec->kind] = 1 << i;
    }
}

void fightstg_action_try_lower_stats(void) {
    RecordsTechnique *rec = &records_techniques[fightstg_action.tech - 1];
    GamestateRecord *digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]));
    s32 i;

    for (i = 0; i < 3; i++) {
        if (fightstg_rules.roll_lower_stat(fightstg_action.side, fightstg_action.tech) != 0) {
            digimon->stats.penalties[i] += rec->effect_power;
            fightstg_action.effects[rec->kind] |= 1 << i;
        }
    }
}

void fightstg_action_try_lower_all_stats(void) {
    RecordsTechnique *rec = &records_techniques[fightstg_action.tech - 1];
    GamestateRecord *digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]));
    s32 i;

    if (fightstg_rules.roll_lower_stat(fightstg_action.side, fightstg_action.tech) != 0) {
        for (i = 0; i < 3; i++) {
            digimon->stats.penalties[i] += rec->effect_power;
        }
        fightstg_action.effects[rec->kind] = 7;
    }
}

void fightstg_action_try_seal(void) {
    RecordsTechnique *rec;

    if (fightstg_rules.roll_switch_seal(fightstg_action.side, fightstg_action.tech) != 0) {
        rec = &records_techniques[fightstg_action.tech - 1];
        fightstg_action.effects[rec->kind] = 1;
    }
}

void fightstg_action_double_strike(void) {
    s32 ids[2] = { 0x1B9, 0x1BA };
    s32 i;

    fightstg_action.hit_count = 0;
    fightstg_action.strikes = 2;
    for (i = 0; i < 2; i++) {
        if (fightstg_rules.roll_special_hit(fightstg_action.side, fightstg_action.tech) != 0) {
            fightstg_action.hits[i] = 1;
            fightstg_action.damages[i] = fightstg_rules.get_special_damage(fightstg_action.side, ids[i]);
            fightstg_action.hit_count++;
        }
    }
    fightstg_action.effects[9] = 1;
}

void fightstg_action_cut_hp(void) {
    RecordsTechnique *rec;

    if (fightstg_battle.state.type == 2) {
        rec = &records_techniques[fightstg_action.tech - 1];
        fightstg_action.effects[rec->kind] = 1;
    } else {
        fightstg_action.hits[0] = fightstg_rules.roll_hit(fightstg_action.side, fightstg_action.tech);
        fightstg_action.strikes++;
        fightstg_action.damage = fightstg_rules.get_damage(fightstg_action.side, fightstg_action.tech);
    }
}

/* Applies the action's effect by its kind (records_techniques's kind). */
void fightstg_action_apply_effect(void) {
    RecordsTechnique *rec = &records_techniques[fightstg_action.tech - 1];

    switch (rec->kind) {
    case 2:
        fightstg_action_try_poison();
        break;
    case 3:
        fightstg_action_try_paralysis();
        break;
    case 4:
        fightstg_action_try_confusion();
        break;
    case 5:
        fightstg_action_try_sleep();
        break;
    case 6:
        fightstg_action_try_knockout();
        break;
    case 8:
        fightstg_action_try_drain();
        break;
    case 12:
        fightstg_action_try_steal();
        break;
    case 26:
        fightstg_action_lower_attack();
        break;
    case 11:
        fightstg_action_mark_critical_tech();
        break;
    case 27:
        fightstg_action_lower_defense();
        break;
    case 29:
        fightstg_action_drain_mp();
        break;
    case 32:
        fightstg_action_try_lower_stat();
        break;
    case 33:
        fightstg_action_try_lower_stats();
        break;
    case 34:
        fightstg_action_try_lower_all_stats();
        break;
    case 13:
        fightstg_action_try_seal();
        break;
    }
}

/* Runs action `id` (record of records_techniques) for `side`: the hit, then its effects (fightstg_action). */
void fightstg_action_run(u8 side, s32 id) {
    RecordsTechnique *rec;

    heap_funcs.bzero(&fightstg_action, 0x68); /* PC_PORT: a byte count: the state before `run` (pointer-free) */
    rec = &records_techniques[id - 1];
    fightstg_action.side = side;
    fightstg_action.tech = id;
    if (rec->kind == 31) {
        fightstg_action_double_strike();
    } else if (rec->kind == 35) {
        fightstg_action_cut_hp();
    } else {
        if (rec->kind == 10 &&
            (side == 0 ||
             fightstg_enemy_records.get((fightstg_battle.state.members[1] + fightstg_battle.state.current[1])->digimon)->tech != id)) {
            fightstg_action_mark_counter_tech();
            return;
        }
        switch (rec->element) {
        case 2:
            fightstg_action.hits[0] = fightstg_rules.roll_hit(side, id);
            fightstg_action.strikes++;
            fightstg_action.damage = fightstg_rules.get_damage(side, id);
            if (side == 0) {
                if (rec->kind < 2) {
                    if (rec->anim_script != 0xB && rec->anim_script != 0xC) {
                        if (fightstg_rules.stats[0].multi_hit != 0) {
                            fightstg_action_hit_again();
                        } else if (fightstg_action.hits[0] != 0) {
                            if (fightstg_rules.stats[0].poison_chance != 0) {
                                fightstg_action_try_poison();
                            }
                            if (fightstg_rules.stats[0].paralysis_chance != 0) {
                                fightstg_action_try_paralysis();
                            }
                            if (fightstg_rules.stats[0].confusion_chance != 0) {
                                fightstg_action_try_confusion();
                            }
                            if (fightstg_rules.stats[0].knockout_chance != 0) {
                                fightstg_action_try_knockout();
                            }
                            if (fightstg_rules.stats[0].drain_chance != 0) {
                                fightstg_action_try_drain();
                            }
                        }
                    }
                } else if (rec->kind == 9) {
                    fightstg_action_hit_again();
                } else if (fightstg_action.hits[0] != 0) {
                    fightstg_action_apply_effect();
                }
            } else if (rec->kind >= 2) {
                if (rec->kind == 9) {
                    fightstg_action_hit_again();
                } else if (fightstg_action.hits[0] != 0) {
                    fightstg_action_apply_effect();
                }
            }
            break;
        case 3:
            fightstg_action.hits[0] = fightstg_rules.roll_special_hit(side, id);
            fightstg_action.strikes++;
            fightstg_action.damage = fightstg_rules.get_special_damage(side, id);
            if (rec->kind >= 2) {
                if (rec->kind == 9) {
                    fightstg_action_hit_again();
                } else if (fightstg_action.hits[0] != 0) {
                    fightstg_action_apply_effect();
                }
            }
            break;
        }
    }
    if (fightstg_battle.state.type != 0) {
        fightstg_action.damage = wfightmn_cap_damage(side, fightstg_action.damage,
                                                   fightstg_action.effects[9] ? fightstg_action.hit_count : 0);
    }
}

/* Sets the frames of this tick (frames) by the battle speed. */
void fightstg_battle_update_speed(void) {
    FightstgSpeed *speed = &fightstg_battle.speed;

    switch (speed->mode) {
    case 0:
    default:
        fightstg_battle.frames = gfx_module.funcs.get_frame_ticks();
        break;
    case 1:
        fightstg_battle.frames = 0;
        break;
    case 2:
        speed->frames += gfx_module.funcs.get_frame_ticks();
        fightstg_battle.frames = 0;
        while (speed->frames >= 5) {
            fightstg_battle.frames++;
            speed->frames -= 4;
        }
        break;
    case 3:
        fightstg_battle.frames = gfx_module.funcs.get_frame_ticks() * 2;
        break;
    }
}

void fightstg_battle_set_speed(s32 mode) {
    fightstg_battle.speed.mode = mode;
    fightstg_battle.speed.frames = 0;
    fightstg_battle_update_speed();
}

/* Projects `in` with the camera: out = screen x, y and the OT depth (z / 4) >> (16 - the layer's depth bits). */
void fightstg_battle_to_screen(GfxLayer *layer, SVECTOR *in, SVECTOR *out) {
    SVECTOR sxy;
    MATRIX m;
    s32 z;
    s32 shift = 0x10 - layer->get_ot_bits(layer);

    gte_MulMatrix0(&GsWSMATRIX, &message_identity_matrix, &m);
    gte_SetTransMatrix(&GsWSMATRIX);
    gte_ldlv0(message_identity_matrix.t);
    gte_rt();
    gte_stlvnl(m.t);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    gte_ldv0_u(in);
    gte_rtps();
    gte_stsxy(&sxy);
    gte_stszotz(&z);
    out->vx = sxy.vx;
    out->vy = sxy.vy;
    out->vz = z >> shift;
}

/* Draws a gouraud quad on layer `layer_id` at OT entry `depth`; semi: semi-transparent (with a draw mode
 * packet after it). */
void fightstg_battle_draw_quad(s32 layer_id, s32 depth, DVECTOR *pos, CVECTOR *colors, s32 semi) {
    GfxLayer *layer = gfx_module.funcs.get_layer(layer_id);
    u32 *ot = layer->get_ot_entry(layer, depth);
    POLY_G4 *poly = gfx_module.funcs.get_packet();
    DR_TPAGE *tpage;

    *(CVECTOR *)&poly->r0 = colors[0];
    *(CVECTOR *)&poly->r1 = colors[1];
    *(CVECTOR *)&poly->r2 = colors[2];
    *(CVECTOR *)&poly->r3 = colors[3];
    setPolyG4(poly);
    if (semi) {
        setSemiTrans(poly, 1);
    }
    poly->x0 = pos[0].vx;
    poly->x1 = pos[1].vx;
    poly->x2 = pos[2].vx;
    poly->x3 = pos[3].vx;
    poly->y0 = pos[0].vy;
    poly->y1 = pos[1].vy;
    poly->y2 = pos[2].vy;
    poly->y3 = pos[3].vy;
    addPrim(ot, poly);
    poly++;
    if (semi) {
        tpage = (DR_TPAGE *)poly;
        setlen(tpage, 1);
        tpage->code[0] = 0xE1000245;
        addPrim(ot, tpage);
        poly = (POLY_G4 *)(tpage + 1);
    }
    gfx_module.funcs.set_packet(poly);
}

void fightstg_battle_draw_quad_opaque(s32 layer_id, s32 depth, void *pos, void *colors) {
    fightstg_battle_draw_quad(layer_id, depth, pos, colors, 0);
}

void fightstg_battle_draw_quad_semi(s32 layer_id, s32 depth, void *pos, void *colors) {
    fightstg_battle_draw_quad(layer_id, depth, pos, colors, 1);
}

void *fightstg_models_get(s32 id) {
    FightstgModels *cache = &fightstg_models;
    FightstgModelFile *file;
    FightstgModelEntry *entry;
    FightstgModelRecordA *recs_a;
    FightstgModelRecordB *recs_b;
    void *rec;
    s32 index;

    if (id == fightstg_models.id) {
        if (cache->is_b != 0) {
            return cache->record_b;
        }
        return cache->record;
    }
    file = (FightstgModelFile *)cdload_module.files.get_file(0x1CC);
    entry = (FightstgModelEntry *)((u8 *)file + file->entries);
    recs_a = (FightstgModelRecordA *)((u8 *)file + file->records_a);
    recs_b = (FightstgModelRecordB *)((u8 *)file + file->records_b);
    for (; entry->id != 0; entry++) {
        if (entry->id == id) {
            fightstg_models.id = id;
            fightstg_models.index = index = entry->index;
            fightstg_models.kind = entry->kind;
            fightstg_models.is_b = entry->kind >= 0x3A;
            if (fightstg_models.is_b != 0) {
                rec = &recs_b[index];
                fightstg_models.record = rec;
                fightstg_models.record_b = rec;
                return rec;
            }
            rec = &recs_a[index];
            fightstg_models.record = rec;
            fightstg_models.record_b = rec;
            return rec;
        }
    }
    return NULL;
}

void fightstg_models_select(s32 i) {
    FightstgModelFile *file = (FightstgModelFile *)cdload_module.files.get_file(0x1CC);
    FightstgModelEntry *entry = (FightstgModelEntry *)((u8 *)file + file->entries) + i;
    FightstgModelRecordA *recs_a = (FightstgModelRecordA *)((u8 *)file + file->records_a);
    FightstgModelRecordB *recs_b = (FightstgModelRecordB *)((u8 *)file + file->records_b);
    FightstgModels *cache = &fightstg_models;
    void *rec;
    s32 index;

    cache->id = entry->id;
    cache->index = index = entry->index;
    cache->kind = entry->kind;
    cache->is_b = entry->kind >= 0x3A;
    if (cache->is_b == 0) {
        rec = &recs_a[index];
    } else {
        rec = &recs_b[index];
    }
    cache->record = rec;
    cache->record_b = rec;
}

u8 *fightstg_models_get_texture_anim(s32 id) {
    FightstgModelFile *file = (FightstgModelFile *)cdload_module.files.get_file(0x1CC);

    return PTR_ADD(u8 *, ((FightstgModelRecordA *)fightstg_models_get(id))->texture_anims - file->base, file);
}

/* The first and last entry of file 0x1CC whose record is of type `type` (0: A, 1: B). */
void fightstg_models_get_range(s32 type, s32 *min, s32 *max) {
    FightstgModelFile *file = (FightstgModelFile *)cdload_module.files.get_file(0x1CC);
    FightstgModelEntry *entry = (FightstgModelEntry *)((u8 *)file + file->entries);
    s32 first = 0xFF;
    s32 last = 0;
    s32 i;
    s32 is_b;

    for (i = 0; entry->id != 0; i++, entry++) {
        is_b = entry->kind >= 0x3A;
        if (is_b == type) {
            if (i < first) {
                first = i;
            }
            if (last < i) {
                last = i;
            }
        }
    }
    *min = first;
    *max = last;
}

/* What records_funcs.get_item(item)->data points to, as fightstg_rules_get_stats reads it: one view of
 * RecordsWeapon, RecordsArmor and RecordsAccessory (include/records.h), as the function reads them all through one
 * pointer. */
typedef struct FightstgItemBonus {
    /* 0x00 */ u8 unk_00[0x6];
    /* 0x06 */ u16 value;  /* value */
    /* 0x08 */ u8 kind;   /* kind (0x11..0x15: FightstgStats.resists[7..11]) */
    /* 0x09 */ u8 unk_09[0x5];
    /* 0x0E */ u8 accuracy;
    /* 0x0F */ u8 unk_0F;
    /* 0x10 */ u8 status_chance;
    /* 0x11 */ u8 status_power;
    /* 0x12 */ u8 strong_type;
} FightstgItemBonus;

/* An enemy's stats: its record's, scaled by the fight's strength for it, plus the member's modifiers. */
static inline void fightstg_rules_get_enemy_stats(FightstgStats *out, FightstgMember *m, s32 member) {
    FightstgEnemyRecord *enemy;
    s32 j;

    enemy = fightstg_enemy_records.get(m->digimon);
    out->level = records_state.enemies[member].level;
    for (j = 0; j < 5; j++) {
        out->stats[j] = enemy->stats[j] * records_state.enemies[member].stat_scale / 16;
    }
    for (j = 0; j < 12; j++) {
        out->resists[j] = enemy->resists[j];
    }
    out->status = m->status;
    if (m->modifiers[0] != 0) {
        out->stats[0] += m->modifiers[0];
    }
    if (m->modifiers[1] != 0) {
        out->stats[1] += m->modifiers[1];
    }
    if (m->modifiers[2] != 0) {
        out->stats[4] += m->modifiers[2];
    }
    if (m->modifiers[3] != 0) {
        out->stats[2] += m->modifiers[3];
    }
    out->type = enemy->type;
    out->power_up = m->power_up;
}

/* Fills the stats block of a combatant (first: fightstg_rules.stats[0], else [1]): a party member's
 * (side 0) from its Digimon, its current form and its items; an enemy's from its record. */
FightstgStats *fightstg_rules_get_stats(u8 side, s32 first, s32 member) {
    GamestateStats stats;
    FightstgStats *out;
    FightstgMember *m;
    RecordsDigimon *rec;
    RecordsDigimon *form;
    GamestateRecord *digimon;
    RecordsItem *entry;
    FightstgItemBonus *item;
    s32 id;
    s16 *p;
    s32 n;
    s32 found;
    s32 i;

    if (first) {
        out = &fightstg_rules.stats[0];
    } else {
        out = &fightstg_rules.stats[1];
    }
    heap_funcs.bzero(out, sizeof(FightstgStats));
    if (side == 0) {
        m = &fightstg_battle.state.members[0][member];
        id = gamestate_data.funcs.get_party_member(member);
        rec = &records_digimon[id];
        gamestate_data.funcs.get_stats(id, &stats);
        for (i = 7; i < 12; i++) {
            out->resists[i] = rec->status_resists[i - 7];
        }
        if (rec->id != m->digimon) {
            form = records_get_digimon_func(m->digimon);
            for (i = 0; i < 5; i++) {
                (&stats.values[6])[i] += form->stats[i];
            }
            for (i = 0; i < 7; i++) {
                (&stats.values[12])[i] += form->resists[i];
            }
            for (i = 7; i < 12; i++) {
                out->resists[i] += form->status_resists[i - 7];
            }
        }
        if (m->modifiers[0] != 0) {
            stats.values[6] += m->modifiers[0];
        }
        if (m->modifiers[1] != 0) {
            stats.values[7] += m->modifiers[1];
        }
        if (m->modifiers[2] != 0) {
            stats.values[10] += m->modifiers[2];
        }
        out->level = stats.values[0];
        /* FAKE: p walks the stat totals here and the equipment below (the shape of the US decomp's
         * FIGHTSTG_computeStats): the extra references put p ahead of found in gcc's register allocation, so p gets
         * s0 and found s3, as in the original; a pointer of its own for the totals leaves them swapped (99.1%). */
        p = &stats.values[6];
        for (i = 0; i < 5; i++) {
            if (p[i] > 0) {
                out->stats[i] = p[i];
            } else {
                out->stats[i] = 1;
            }
        }
        p = &stats.values[12];
        for (i = 0; i < 7; i++) {
            if (p[i] > 0) {
                out->resists[i] = p[i];
            } else {
                out->resists[i] = 1;
            }
        }
        out->status = m->status;
        digimon = gamestate_data.funcs.get_record(id);
        p = &digimon->equipment[4];
        for (i = 0; i < 2; i++) {
            if (p[i] != 0) {
                item = records_funcs.get_item(p[i])->data;
                if (item->kind == 0x11) {
                    out->resists[7] = item->value;
                } else if (item->kind == 0x12) {
                    out->resists[8] = item->value;
                } else if (item->kind == 0x13) {
                    out->resists[9] = item->value;
                } else if (item->kind == 0x14) {
                    out->resists[10] = item->value;
                } else if (item->kind == 0x15) {
                    out->resists[11] = item->value;
                }
            }
        }
        p = digimon->equipment;
        out->type = records_get_digimon_func(m->digimon)->type;
        out->power_up = m->power_up;
        n = 0;
        for (i = 0; i < 4; i++) {
            if (p[i] > 0) {
                entry = records_funcs.get_item(p[i]);
                if (entry->type >= 2 && entry->type <= 14) {
                    item = entry->data;
                    out->accuracy += item->accuracy;
                    if (item->strong_type >= 2) {
                        out->strong_types[n++] = item->strong_type;
                    }
                } else {
                    item = entry->data;
                    if (item->status_chance != 0) {
                        out->evasion += item->status_chance;
                    }
                }
            }
        }
        found = 0;
        for (i = 0; i < 4; i++) {
            if (i != 1 && p[i] > 0) {
                item = records_funcs.get_item(p[i])->data;
                if (p[i] == 0x97) {
                    out->poison_chance = item->status_chance;
                    out->poison_power = item->status_power;
                    found = 1;
                } else if (p[i] == 0xD2) {
                    out->paralysis_chance = item->status_chance;
                    out->paralysis_power = item->status_power;
                    found = 1;
                } else if (p[i] == 0xB4 || p[i] == 0xC2) {
                    out->confusion_chance = item->status_chance;
                    out->confusion_power = item->status_power;
                    found = 1;
                } else if (p[i] == 0x6D || p[i] == 0xBA) {
                    out->knockout_chance = item->status_chance;
                    out->knockout_power = item->status_power;
                    found = 1;
                } else if (p[i] == 0x5E || p[i] == 0x93 || p[i] == 0xAD) {
                    out->drain_chance = item->status_chance;
                    out->drain_power = item->status_power;
                    found = 1;
                } else if (p[i] == 0x96 || p[i] == 0xBF) {
                    out->critical = item->status_power;
                    found = 1;
                }
            }
        }
        if (!found) {
            p = &digimon->equipment[4];
            for (i = 0; i < 2; i++) {
                if (p[i] == 0x13C) {
                    out->multi_hit = 1;
                } else if (p[i] == 0x13D) {
                    item = records_funcs.get_item(0x13D)->data;
                    out->critical = item->value;
                } else if (p[i] == 0x13E) {
                    item = records_funcs.get_item(0x13E)->data;
                    out->counter = item->value;
                }
            }
        }
        p = &digimon->equipment[4];
        for (i = 0; i < 2; i++) {
            if (p[i] >= 0x153 && p[i] <= 0x167) {
                item = records_funcs.get_item(p[i])->data;
                if (p[i] < 0x156) {
                    out->attack_element = 2;
                } else if (p[i] < 0x159) {
                    out->attack_element = 3;
                } else if (p[i] < 0x15C) {
                    out->attack_element = 4;
                } else if (p[i] < 0x15F) {
                    out->attack_element = 5;
                } else if (p[i] < 0x162) {
                    out->attack_element = 6;
                } else if (p[i] < 0x165) {
                    out->attack_element = 7;
                } else if (p[i] < 0x168) {
                    out->attack_element = 8;
                }
                out->attack_element_power = item->value;
            } else if (p[i] >= 0x145 && p[i] <= 0x146) {
                item = records_funcs.get_item(p[i])->data;
                out->guard = item->value;
            } else if (p[i] >= 0x14B && p[i] <= 0x14C) {
                item = records_funcs.get_item(p[i])->data;
                out->accuracy += item->value;
            } else if (p[i] >= 0x14D && p[i] <= 0x14E) {
                item = records_funcs.get_item(p[i])->data;
                out->evasion += item->value;
            } else if (p[i] >= 0x14F && p[i] <= 0x150) {
                item = records_funcs.get_item(p[i])->data;
                out->escape = item->value;
            } else if (p[i] == 0x13F) {
                item = records_funcs.get_item(p[i])->data;
                out->no_escape = 1;
            } else if (p[i] >= 0x147 && p[i] <= 0x148) {
                item = records_funcs.get_item(p[i])->data;
                out->steal = item->value;
            }
        }
    } else {
        m = &fightstg_battle.state.members[1][member];
        fightstg_rules_get_enemy_stats(out, m, member);
    }
    if (first) {
        return &fightstg_rules.stats[0];
    }
    return &fightstg_rules.stats[1];
}

s32 fightstg_rules_get_field_bonus(s32 value, s32 element) {
    FightstgField *info = &fightstg_battle.state.field;

    if (info->element < 2) {
        return 0;
    }
    if (element == info->element) {
        return value * info->power / 128;
    }
    if (fightstg_rules_opposite_elements[element] == info->element) {
        return -(value * info->power / 256);
    }
    return 0;
}

s32 fightstg_rules_roll_critical(u8 side, s32 id);

/* Damage of action `id` (record of records_techniques) from a base value, with fightstg_rules's stats
 * filled in ([0] the attacker, [1] the target). An old-style definition: fightstg_rules_get_damage passes
 * a fourth argument (the target's stats), fightstg_rules_get_counter_damage three. */
s32 fightstg_rules_scale_damage(side, id, base)
    u8 side;
    s32 id;
    s32 base;
{
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 max;
    s32 i;

    rec = &records_techniques[id - 1];
    value = base;
    stats = &fightstg_rules.stats[0];
    value += fightstg_rules_get_field_bonus(value, rec->defense_stat);
    target = &fightstg_rules.stats[1];
    if (rec->strong_type >= 2) {
        if (rec->strong_type == target->type) {
            value += value / 2;
        }
    } else {
        for (i = 0; i < 3; i++) {
            if (stats->strong_types[i] >= 2 && stats->strong_types[i] == target->type) {
                value += value / 2;
                break;
            }
        }
    }
    if (stats->power_up != 0) {
        value += value * stats->power_up / 64;
    }
    if (rec->defense_stat >= 2) {
        value += value * rec->element_power * 2 / target->resists[rec->defense_stat - 2];
    } else if (rec->anim_script != 0xB && rec->anim_script != 0xC && stats->attack_element != 0) {
        value += value * stats->attack_element_power * 2 / target->resists[stats->attack_element - 2];
    }
    if (rec->kind < 2 && stats->multi_hit != 0 && (rec->anim_script != 0xB && rec->anim_script != 0xC)) {
        value = value * 4 / 10;
    }
    if (fightstg_rules_roll_critical(side, id) != 0) {
        value += value * ((pad_random.next() & 0x3F) + 0x20) / 64;
    }
    if (target->guard != 0) {
        value -= target->guard;
        if (value <= 0) {
            value = 0;
        }
    }
    max = base * 5;
    if (value > max) {
        value = max;
    }
    if (value >= 10000) {
        value = 9999;
    }
    return value;
}

s32 fightstg_rules_get_damage(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 member;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (side == 0) {
        value = rec->power * stats->stats[0] / target->stats[1];
    } else {
        member = fightstg_battle.state.current[1];
        value = rec->power * records_state.enemies[member].stat_scale / 16 * stats->stats[0] / target->stats[1];
    }
    return fightstg_rules_scale_damage(side, id, value, target);
}

s32 fightstg_rules_roll_special_critical(u8 side, s32 id);

s32 fightstg_rules_get_special_damage(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 power;
    s32 base;
    s32 value;
    s32 member;
    s16 defense;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (side == 0) {
        power = rec->power;
    } else {
        member = fightstg_battle.state.current[1];
        power = rec->power * records_state.enemies[member].stat_scale / 16;
    }
    base = power * (stats->stats[2] * 50 / target->stats[2] + 50) / 100;
    if (base > power * 2) {
        base = power * 2;
    }
    if (base < power / 2) {
        base = power / 2;
    }
    value = base;
    value += fightstg_rules_get_field_bonus(value, rec->defense_stat);
    if (rec->defense_stat >= 2) {
        defense = target->resists[rec->defense_stat - 2];
        if (defense < 100) {
            value = value * (400 - defense * 3) / 100;
        } else if (defense >= 300) {
            value = value * (65 - defense / 20) / 100;
        } else {
            value = value * (125 - defense / 4) / 100;
        }
    }
    if (rec->strong_type >= 2 && rec->strong_type == target->type) {
        value += value / 2;
    }
    if (fightstg_rules_roll_special_critical(side, id) != 0) {
        value += value * (pad_random.next() % 65 + 0x20) / 64;
    }
    if (target->guard != 0) {
        value -= target->guard;
        if (value <= 0) {
            value = 0;
        }
    }
    if (value > base * 5) {
        value = base * 5;
    }
    if (value >= 10000) {
        value = 9999;
    }
    return value;
}

s32 fightstg_rules_get_poison_damage(FightstgPoisonArgs *args) {
    FightstgBattle *battle = &fightstg_battle;
    u8 side = args->side;
    s32 member = args->member;
    u8 enemy = side != 0;
    s32 amount = args->amount;
    FightstgMember *m;
    FightstgStats *target;
    s32 value;
    s32 half;

    if (member != battle->state.current[enemy]) {
        return 0;
    }
    fightstg_rules_get_stats(side, 0, member);
    target = &fightstg_rules.stats[1];
    m = &fightstg_battle.state.members[enemy][member];
    value = (amount / 2 - (target->resists[7] + target->resists[1]) / 10) * m->max_hp / 100;
    if (value <= 0) {
        value = 1;
    }
    half = m->max_hp / 2;
    if (half < value) {
        value = half;
    }
    return value;
}

s32 fightstg_rules_get_counter_damage(u8 side, s32 id, s32 amount) {
    FightstgMember *member;
    RecordsTechnique *rec;
    s32 own;
    u8 saved;
    s32 ret;
    s32 value;

    if (side == 0) {
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        own = id == records_get_digimon_func(member->digimon)->techniques[0];
    } else {
        member = &fightstg_battle.state.members[1][fightstg_battle.state.current[1]];
        fightstg_enemy_records.get(member->digimon);
        own = 0;
    }
    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    if (own == 1) {
        saved = fightstg_rules.stats[0].multi_hit;
        fightstg_rules.stats[0].multi_hit = 0;
        ret = fightstg_rules_scale_damage(side, id, amount);
        fightstg_rules.stats[0].multi_hit = saved;
        return ret;
    }
    rec = &records_techniques[id - 1];
    if (member->boosted != 0) {
        value = amount * rec->effect_power / 32;
    } else {
        value = amount * rec->effect_power / 64;
    }
    return fightstg_rules_scale_damage(side, id, value);
}

s32 fightstg_rules_get_heal(u8 side, s32 id) {
    FightstgStats *stats;
    RecordsTechnique *rec;
    s32 value;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
    } else {
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    rec = &records_techniques[id - 1];
    value = (rec->power << 6) + stats->stats[3] * rec->power / 8;
    if (value >= 10000) {
        value = 9999;
    }
    return value;
}

s32 fightstg_rules_get_regen(u8 side, s32 member, s32 arg2) {
    FightstgMember *m;
    s32 value;

    if (side == 0) {
        m = &fightstg_battle.state.members[0][member];
    } else {
        m = &fightstg_battle.state.members[1][member];
    }
    if (arg2 != 0) {
        value = m->max_hp * (pad_random.next() % 9 + 8) / 128;
    } else {
        value = m->max_hp * (pad_random.next() % 5 + 4) / 128;
    }
    if (value >= 10000) {
        value = 9999;
    }
    return value;
}

s32 fightstg_rules_roll_critical(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    FightstgMember *m;
    s32 enemy;
    s32 other;
    s32 chance;
    s32 i;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    chance = 4;
    rec = &records_techniques[id - 1];
    if (rec->kind >= 2) {
        if (rec->kind == 0xB) {
            chance = rec->effect_power + 4;
        }
    } else if (stats->critical != 0) {
        enemy = side != 0;
        m = &fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]];
        if (m->boosted != 0) {
            chance = stats->critical * 2 + 4;
        } else {
            chance = stats->critical + 4;
        }
    }
    if (rec->strong_type >= 2) {
        if (rec->strong_type == target->type) {
            chance += 60;
        }
    } else {
        for (i = 0; i < 3; i++) {
            if (stats->strong_types[i] >= 2 && stats->strong_types[i] == target->type) {
                chance += 16;
                break;
            }
        }
    }
    other = side == 0;
    m = &fightstg_battle.state.members[other][fightstg_battle.state.current[other]];
    if (m->status & 2) {
        chance += m->paralysis_power >> 3;
    }
    if (m->status & 8) {
        chance += m->sleep_power >> 3;
    }
    if (m->status & 4) {
        chance += m->confusion_power >> 1;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_special_critical(u8 side, s32 id) {
    FightstgStats *target;
    RecordsTechnique *rec;
    FightstgMember *m;
    s32 other;
    s32 ratio;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
    }
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    ratio = rec->element_power * 100 / target->resists[rec->defense_stat - 2];
    if (ratio > 64) {
        ratio = 64;
    }
    chance = ratio + 4;
    if (rec->strong_type >= 2 && rec->strong_type == target->type) {
        chance = ratio + 64;
    }
    other = side == 0;
    m = &fightstg_battle.state.members[other][fightstg_battle.state.current[other]];
    if (m->status & 2) {
        chance += m->paralysis_power >> 3;
    }
    if (m->status & 8) {
        chance += m->sleep_power >> 3;
    }
    if (m->status & 4) {
        chance += m->confusion_power >> 1;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_hit(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 diff;
    s32 level;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (side == 0) {
        if (fightstg_battle.state.type == 6) {
            return (pad_random.next() & 0x7F) >= 0x2B;
        }
        diff = stats->stats[4] + stats->accuracy - target->stats[4];
        level = stats->level - target->level;
        if (rec->kind < 2) {
            chance = rec->accuracy + rec->accuracy * (diff / 8 + (level - stats->critical)) / 128;
        } else {
            chance = rec->accuracy + rec->accuracy * (diff / 8 + level) / 128;
        }
    } else {
        level = stats->level - target->level;
        diff = stats->stats[4] - target->stats[4] - target->evasion;
        chance = rec->accuracy + rec->accuracy * (diff / 8 + level) / 128;
        if (chance < 0x20) {
            chance = 0x20;
        }
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_special_hit(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 diff;
    s32 level;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (side == 0 && fightstg_battle.state.type == 6) {
        return (pad_random.next() & 0x7F) >= 0x2B;
    }
    diff = stats->stats[3] - target->stats[3];
    level = stats->level - target->level;
    chance = rec->accuracy + rec->accuracy * (diff / 8 + level) / 128;
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_poison(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 amount;
    s32 value;
    s32 chance;

    if (side == 0) {
        if (records_state.blocked[0] != 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (rec->kind < 2) {
        amount = stats->poison_power;
        value = stats->poison_chance + stats->stats[3] / 8;
    } else {
        amount = rec->effect_power;
        value = rec->effect_chance + stats->stats[3] / 8;
    }
    chance = value - (target->resists[7] + target->resists[1] + target->stats[3] / 2) / 8;
    if (chance <= 0) {
        chance = 1;
    }
    if (chance >= 0x80) {
        chance = 0x7F;
    }
    if ((pad_random.next() & 0x7F) < chance) {
        return amount;
    }
    return 0;
}

s32 fightstg_rules_roll_paralysis(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 chance;

    if (side == 0) {
        if (records_state.blocked[1] != 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (rec->kind < 2) {
        value = stats->paralysis_chance + stats->stats[3] / 8;
    } else {
        value = rec->effect_chance + stats->stats[3] / 8;
    }
    chance = value - (target->resists[8] + target->resists[4] + target->stats[3] / 2) / 8;
    if (chance <= 0) {
        chance = 1;
    }
    if (chance >= 0x80) {
        chance = 0x7F;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_confusion(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 chance;

    if (side == 0) {
        if (records_state.blocked[2] != 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (rec->kind < 2) {
        value = stats->confusion_chance + stats->stats[3] / 8;
    } else {
        value = rec->effect_chance + stats->stats[3] / 8;
    }
    chance = value - (target->resists[9] + target->resists[3] + target->stats[3] / 2) / 8;
    if (chance <= 0) {
        chance = 1;
    }
    if (chance >= 0x80) {
        chance = 0x7F;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_sleep(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 chance;

    if (side == 0) {
        if (records_state.blocked[3] != 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    value = rec->effect_chance + stats->stats[3] / 8;
    chance = value - (target->resists[10] + target->resists[2] + target->stats[3] / 2) / 8;
    if (chance <= 0) {
        chance = 1;
    }
    if (chance >= 0x80) {
        chance = 0x7F;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_knockout(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 value;
    s32 chance;

    if (side == 0) {
        if (records_state.blocked[4] != 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    rec = &records_techniques[id - 1];
    if (rec->kind < 2) {
        value = stats->knockout_chance + stats->stats[3] / 8;
    } else {
        value = rec->effect_chance + stats->stats[3] / 8;
    }
    chance = value - (target->resists[11] + target->resists[6] + target->stats[3] / 2) / 8;
    if (chance <= 0) {
        chance = 1;
    }
    if (chance >= 0x80) {
        chance = 0x7F;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_steal(u8 side, s32 id) {
    FightstgStats *stats;
    FightstgStats *target;
    RecordsTechnique *rec;
    s32 ratio;
    s32 percent;
    s32 chance;
    s32 roll;

    if (side == 0) {
        if (records_state.blocked[7] != 0) {
            return 0;
        }
        if (fightstg_battle.state.members[1][fightstg_battle.state.current[1]].item <= 0) {
            return 0;
        }
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    if (side == 0) {
        stats = &fightstg_rules.stats[0];
        target = &fightstg_rules.stats[1];
        ratio = stats->stats[4] * 100 / target->stats[4];
        rec = &records_techniques[id - 1];
        if (ratio > 200) {
            ratio = 200;
        }
        percent = (rec->effect_chance + stats->steal) * 100 / 64;
        chance = fightstg_enemy_records.get(fightstg_battle.state.members[1][fightstg_battle.state.current[1]].digimon)->drop_rate * ratio * percent / 10000;
        roll = pad_random.next() % 1024;
        if (roll < chance) {
            return 1;
        }
    }
    return 0;
}

s32 fightstg_rules_roll_drain(u8 side, s32 id) {
    FightstgStats *stats;
    RecordsTechnique *rec;
    s32 chance;

    if (side == 0 && records_state.blocked[5] != 0) {
        return 0;
    }
    stats = fightstg_rules_get_stats(side, 1, fightstg_battle.state.current[side >> 4]);
    rec = &records_techniques[id - 1];
    if (rec->kind < 2) {
        chance = stats->drain_chance;
    } else {
        chance = rec->effect_chance;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_revert(u8 side, s32 id) {
    RecordsTechnique *rec;
    s32 i;
    s32 chance;

    for (i = 0; i < 8; i++) {
        if (records_digimon[i].id == fightstg_battle.state.members[0][fightstg_battle.state.current[0]].digimon) {
            return 0;
        }
    }
    rec = &records_techniques[id - 1];
    chance = rec->effect_chance;
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_lower_stat(u8 side, s32 id) {
    RecordsTechnique *rec;
    s32 diff;
    s32 chance;

    fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
    fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    diff = fightstg_rules.stats[0].stats[3] - fightstg_rules.stats[1].stats[3];
    rec = &records_techniques[id - 1];
    chance = rec->effect_chance + diff / 8;
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_switch_seal(u8 side, s32 id) {
    RecordsTechnique *rec = &records_techniques[id - 1];
    s32 chance = rec->effect_chance;

    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_digivolve_seal(u8 side, s32 id) {
    RecordsTechnique *rec = &records_techniques[id - 1];
    s32 chance = rec->effect_chance;

    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_counter(u8 side, s32 damage) {
    s32 chance = (damage << 6) / (fightstg_battle.state.members[0] + fightstg_battle.state.current[0])->max_hp + 0x20;

    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_escape(u8 side) {
    FightstgStats *stats;
    FightstgStats *target;
    FightstgBattleState *battle;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0, 1, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    } else {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
        fightstg_rules_get_stats(0x10, 1, fightstg_battle.state.current[1]);
    }
    battle = &fightstg_battle.state;
    stats = &fightstg_rules.stats[0];
    target = &fightstg_rules.stats[1];
    if (side == 0) {
        if (records_state.blocked[11] != 0) {
            return 0;
        }
        if ((stats->status & 8) || (stats->status & 0x10)) {
            return 0;
        }
        chance = (battle->escapes + 1) * 8;
        if (stats->status & 2) {
            chance >>= 1;
        }
        if (stats->level > target->level) {
            chance += stats->level - target->level;
        }
        if (stats->stats[4] > target->stats[4]) {
            chance += (stats->stats[4] - target->stats[4]) / 10;
        }
        chance += stats->escape;
    } else {
        if (stats->status & 8) {
            return 0;
        }
        chance = 0x40;
        if (target->no_escape != 0) {
            chance = 0x20;
        }
        if (stats->status & 2) {
            chance >>= 1;
        }
        chance += (stats->stats[4] - target->stats[4]) / 10;
    }
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_wake(u8 side, s32 amount) {
    FightstgStats *target;
    s32 enemy;
    s32 i;
    s32 chance;
    s32 wait;
    s32 base;

    if (side == 0) {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
    } else {
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    }
    enemy = side != 0;
    target = &fightstg_rules.stats[1];
    i = fightstg_events.find_member(0xC, side, fightstg_battle.state.current[enemy]);
    base = 64 - fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]].sleep_power / 2;
    wait = fightstg_events.events[i].delay / 100;
    chance = (amount << 7) / target->stats[1] + base - wait;
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_confused(u8 side) {
    FightstgStats *stats;
    s32 enemy;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
    } else {
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[1];
    enemy = side != 0;
    chance = fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]].confusion_power -
             (stats->resists[9] + stats->resists[3]) / 8;
    return (pad_random.next() & 0x7F) < chance;
}

s32 fightstg_rules_roll_paralyzed(u8 side) {
    FightstgStats *stats;
    s32 enemy;
    s32 chance;

    if (side == 0) {
        fightstg_rules_get_stats(0, 0, fightstg_battle.state.current[0]);
    } else {
        fightstg_rules_get_stats(0x10, 0, fightstg_battle.state.current[1]);
    }
    stats = &fightstg_rules.stats[1];
    enemy = side != 0;
    chance = fightstg_battle.state.members[enemy][fightstg_battle.state.current[enemy]].paralysis_power -
             (stats->resists[8] + stats->resists[4]) / 8;
    if (chance < 0x20) {
        chance = 0x20;
    }
    return (pad_random.next() & 0x7F) < chance;
}

/* The stat (index into FightstgStats.stats) each of a member's modifiers changes. */
extern s16 fightstg_rules_modifier_stats[4];

/* Changes modifier `kind` of a member by amount / 128 of the stat, within -1/2 .. 1 times it. */
void fightstg_rules_change_modifier(u8 side, s32 member, s32 kind, s32 amount) {
    FightstgStats *stats;
    FightstgMember *m;
    s32 base;
    s32 min;

    if (side == 0) {
        stats = fightstg_rules_get_stats(0, 0, member);
        m = &fightstg_battle.state.members[0][member];
    } else {
        stats = fightstg_rules_get_stats(0x10, 0, member);
        m = &fightstg_battle.state.members[1][member];
    }
    if (m->digimon != 0 && m->hp != 0) {
        if (m->modifiers[kind] != 0) {
            stats->stats[fightstg_rules_modifier_stats[kind]] -= m->modifiers[kind];
        }
        base = stats->stats[fightstg_rules_modifier_stats[kind]];
        min = -(base / 2);
        m->modifiers[kind] += base * amount / 128;
        if (m->modifiers[kind] < min) {
            m->modifiers[kind] = min;
        }
        if (base < m->modifiers[kind]) {
            m->modifiers[kind] = base;
        }
    }
}

s32 fightstg_rules_get_gauge_gain(s32 amount) {
    GamestateRecord *digimon;
    s32 percent;
    s32 value;
    s32 i;
    s16 *items;
    FightstgMember *m = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];

    percent = amount * 100 / m->max_hp;
    value = percent * percent / 20;
    digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]));
    items = &digimon->equipment[4];
    for (i = 0; i < 2; i++) {
        if (items[i] == 0x149) {
            value += value / 5;
        } else if (items[i] == 0x14A) {
            value += value * 4 / 10;
        }
    }
    if (value > 1000) {
        value = 1000;
    }
    return value;
}

s32 fightstg_rules_get_tech_cost(u8 side, s32 id) {
    GamestateRecord *digimon;
    s32 cost = records_techniques[(id & 0x1FFF) - 1].mp_cost;
    s16 item;

    if (side == 0) {
        digimon = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]));
        item = 0;
        if (digimon->equipment[4] == 0x143 || digimon->equipment[4] == 0x144) {
            item = digimon->equipment[4];
        }
        if (digimon->equipment[5] == 0x143 || digimon->equipment[5] == 0x144) {
            item = digimon->equipment[5];
        }
        if (item != 0) {
            cost -= ((RecordsAccessory *)records_funcs.get_item(item)->data)->bonus_value;
            if (cost <= 0) {
                cost = 1;
            }
        }
        if (id & 0x4000) {
            if (cost / 5 != 0) {
                cost += cost / 5;
            } else {
                cost += 1;
            }
        }
    }
    return cost;
}

void fightstg_math_nop(void) {
}

/* out = a + (b - a) * t (t: 0x1000 = 1), on the GTE. */
void fightstg_math_lerp(SVECTOR *a, SVECTOR *b, s32 t, SVECTOR *out) {
    SVECTOR d;
    SVECTOR r;

    gte_lddp(t);
    d.vx = b->vx - a->vx;
    d.vy = b->vy - a->vy;
    d.vz = b->vz - a->vz;
    gte_ldsv(&d);
    gte_gpf12();
    *out = *a;
    gte_stsv(&r);
    out->vx += r.vx;
    out->vy += r.vy;
    out->vz += r.vz;
}

/* scale * sin (mode 0), cos (1) or sin of the doubled angle (2), with angles in 1/16384 turns. */
s32 fightstg_math_wave(s32 mode, s32 angle, s32 scale) {
    switch (mode) {
    case 0:
    default:
        return rsin(angle >> 2) * scale / 4096;
    case 1:
        return rsin((angle >> 2) + 0x400) * scale / 4096;
    case 2:
        return rsin(angle >> 1) * scale / 4096;
    }
}

/* The tables below point to these WIP functions (asm unless NON_MATCHING). */
void fightstg_events_cure_status(u8 side, s32 member, s32 id);
s32 fightstg_rules_roll_wake(u8 side, s32 amount);

/* .data (address order) */

FightstgItemCure fightstg_item_cure_poison = { 40, 1, 190 };
FightstgItemCure fightstg_item_cure_paralysis = { 41, 2, 192 };
FightstgItemCure fightstg_item_cure_confusion = { 42, 4, 194 };
FightstgItemCure fightstg_item_cure_all = { 44, 63, 196 };

FightstgItemScript fightstg_item_scripts[16] = {
    { 43, 33, 31 }, { 44, 33, 31 }, { 45, 33, 31 }, { 46, 33, 31 }, { 66, 34, 31 }, { 67, 34, 31 }, { 68, 34, 31 },
    { 69, 34, 31 }, { 70, 35, 31 }, { 71, 33, 31 }, { 72, 40, 31 }, { 73, 36, 31 }, { 74, 38, 31 }, { 75, 46, 30 },
    { 76, 46, 30 }, { -1, 0, 0 },
};

FightstgTechStatEffect fightstg_tech_stat_effects[9] = {
    { 198, 1, 0, 48 }, { 200, 1, 1, 49 }, { 202, 1, 2, 50 }, { 204, -1, 0, 51 }, { 205, -1, 1, 52 },
    { 206, -1, 1, 52 }, { 207, -1, 2, 53 }, { 208, -1, 2, 53 }, { -1, 0, 0, 0 },
};

FightstgTechStatEffect fightstg_tech_party_stat_effects[4] = { { 199, 1, 0, 48 }, { 201, 1, 1, 49 }, { 203, 1, 2, 50 }, { -1, 0, 0, 0 } };
u8 fightstg_tech_cure_flags[4] = { 1, 2, 4, 0x3F };
s32 fightstg_revive_event_types[6] = { 9, 10, 11, 12, 16, 17 };
s32 fightstg_tech_cure_kinds[4] = { 1, 2, 3, 4 };
s32 fightstg_tech_cure_messages[4] = { 40, 41, 42, 44 };

DVECTOR fightstg_hud_bars[2][4] = {
    { { 175, 37 }, { 303, 37 }, { 175, 43 }, { 303, 43 } }, { { 143, 37 }, { 15, 37 }, { 143, 43 }, { 15, 43 } },
};

CVECTOR fightstg_hud_bar_colors[2][2] = {
    { { 0, 0x71, 0x28, 0 }, { 0, 0xC8, 0x3E, 0 } }, { { 0xB3, 0x37, 0x16, 0 }, { 0xFF, 0x37, 0xD, 0 } },
};

DVECTOR fightstg_hud_gauge[4] = { { 265, 62 }, { 305, 62 }, { 265, 70 }, { 305, 70 } };
CVECTOR fightstg_hud_gauge_colors[4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
s16 fightstg_hud_icon_x[2][3] = { { 271, 282, 293 }, { 37, 26, 15 } };
struct FightstgCursorParams fightstg_command_menu_cursor = { 6, 17, 110, 19, 34, 16, 109, 19 };
FightstgCursorParams fightstg_digivolve_menu_cursor = { 0, 168, 147, 19, 36, 167, 146, 19 };

FightstgStatWindow fightstg_status_stat_windows[13] = {
    { 0x3C, 0x52, 6 }, { 0x3C, 0x60, 7 }, { 0x3C, 0x6E, 8 }, { 0x3C, 0x7C, 9 }, { 0x3C, 0x8A, 0xA },
    { 0x3C, 0x98, 0xB }, { 0x7E, 0x52, 0xC }, { 0x7E, 0x60, 0xD }, { 0x7E, 0x6E, 0xE }, { 0x7E, 0x7C, 0xF },
    { 0x7E, 0x8A, 0x10 }, { 0x7E, 0x98, 0x11 }, { 0x7E, 0xA6, 0x12 },
};

FightstgCursorParams fightstg_item_menu_cursor = { 1, 14, 69, 14, -1, 0, 0, 0 };
FightstgCursorParams fightstg_tech_menu_cursor = { 1, 14, 69, 14, -1, 0, 0, 0 };
FightstgCursorParams fightstg_member_menu_cursor = { 2, 17, 78, 32, 47, 16, 77, 32 };

FightstgCursorParams fightstg_switch_menu_cursors[2] = {
    { 4, 168, 147, 19, 36, 167, 146, 19 }, { 2, 168, 166, 19, 34, 167, 165, 19 },
};

DVECTOR fightstg_switch_menu_window_x[4] = { { 172, 0 }, { 216, 0 }, { 217, 0 }, { 251, 0 } };

FightstgConfusedMessage fightstg_confused_menu_messages[16] = {
    { 1, 115 }, { 2, 116 }, { 3, 117 }, { 4, 118 }, { 5, 119 }, { 6, 120 }, { 7, 121 }, { 8, 122 }, { 9, 123 },
    { 10, 124 }, { 11, 125 }, { 12, 126 }, { 13, 127 }, { 14, 128 }, { 15, 129 }, { 16, 130 },
};

s16 fightstg_confused_menu_order[8] = { 7, 1, 0, 3, 2, 5, 4, 6 };
FightstgCursorParams fightstg_confused_menu_cursor = { 6, 17, 110, 19, 34, 16, 109, 19 };
RECT fightstg_cursor_rect = { 0, 244, 12, 12 };

FightstgJumpParams fightstg_jump_params[6] = {
    { 640, 136 }, { 1280, 102 }, { 1920, 81 }, { 2560, 64 }, { 1280, 64 }, { 0, 42 },
};

s32 fightstg_sounds[92] = {
    0x6004001E, 0x0004001C, 0x00040004, 0x0004000A, 0x0004000B, 0x0004000C,
    0x4004000D, 0x00040014, 0x0004001A, 0x00040019, 0x00040017, 0x00040011,
    0x0004000F, 0x0004001F, 0x00040016, 0x00040010, 0x0004000E, 0x00040012,
    0x8004103C, 0x800410BD, 0x8004213E, 0x800421BF, 0xA0042240, 0x80042342,
    0x80042444, 0x8004293E, 0x800429BF, 0x80042A40, 0x00040001, 0x80042B42,
    0x80042C44, 0x80042CC5, 0x80042D46, 0x80042DC7, 0x80042E48, 0xA0042F4A,
    0xA0042FCB, 0xA004303C, 0x800430BD, 0x8004313E, 0xA00431BF, 0xA0043240,
    0x800432C1, 0x80043342, 0x800433C3, 0x800434C5, 0x80043546, 0xA00435C7,
    0x80043648, 0x8004374A, 0x800437CB, 0x8004383C, 0x80043A40, 0xA0043BC3,
    0xA0043C44, 0x800440BD, 0x8004413E, 0x800441BF, 0x80044240, 0x800442C1,
    0x80044444, 0x800445C7, 0x80044648, 0x800446C9, 0x8004474A, 0x8004483C,
    0x41180000, 0x8004503C, 0x800450BD, 0x8004513E, 0x800452C6, 0x80045341,
    0x0004001B, 0x800454C4, 0x8004583C, 0x800458BD, 0x8004593E, 0x800459BF,
    0x80045A40, 0x80045AC1, 0x80045B42, 0x80045BC3, 0x80045C44, 0x80045CC5,
    0x80045D46, 0x80045DC7, 0x8004603C, 0x800460BD, 0x8004613E, 0x20040006,
    0, 0,
};

FightstgEnemyRecords fightstg_enemy_records = { fightstg_enemy_get_record };

s32 fightstg_events_take_modes[26] = {
    0, 1, 1, 1, -1, 1, -1, 1,
    1, -1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0,
};

FightstgEvents fightstg_events = {
    .add = fightstg_events_add,
    .add_first = fightstg_events_add_first,
    .take_next = fightstg_events_take_next,
    .find = fightstg_events_find,
    .find_next = fightstg_events_find_next,
    .find_member = fightstg_events_find_member,
    .remove_member = fightstg_events_remove_member,
    .get_delay = fightstg_events_get_delay,
    .cure_status = (void (*)())fightstg_events_cure_status,
};

FightstgDelayFormula fightstg_events_delay_formulas[13] = {
    { 1000, 707, 1414 }, { 250, 176, 353 }, { 2001, 1001, 0 }, { 2000, 0, 0 }, { 2500, 0, 0 }, { 3000, 0, 0 },
    { 3500, 0, 0 }, { 4000, 0, 0 }, { 0, 2000, 6000 }, { 500, 1000, 0 }, { 500, 1000, 0 }, { 500, 500, 0 },
    { 2001, 0, 0 },
};

u8 fightstg_status_event_types[8] = { 9, 0xA, 0xB, 0xC, 0x10, 0x11, 0, 0 };
u8 fightstg_status_flags[4] = { 1, 2, 4, 0x3F };
s32 fightstg_modifier_event_types[3] = { 13, 14, 15 };
s32 fightstg_seal_event_types[2] = { 16, 17 };

FightstgAction fightstg_action = {
    .run = (void (*)())fightstg_action_run,
};

FightstgBattle fightstg_battle = {
    .update_speed = fightstg_battle_update_speed,
    .set_speed = fightstg_battle_set_speed,
    .to_screen = fightstg_battle_to_screen,
    .draw_quad = (void (*)(s32, s32, DVECTOR *, CVECTOR *))fightstg_battle_draw_quad_opaque,
    .draw_quad_semi = (void (*)(s32, s32, DVECTOR *, CVECTOR *))fightstg_battle_draw_quad_semi,
};

FightstgModels fightstg_models = {
    .get = fightstg_models_get,
    .select = fightstg_models_select,
    .get_range = fightstg_models_get_range,
    .get_texture_anim = (void *(*)(s32))fightstg_models_get_texture_anim,
};

FightstgRules fightstg_rules = {
    .get_stats = fightstg_rules_get_stats,
    .get_damage = fightstg_rules_get_damage,
    .get_special_damage = fightstg_rules_get_special_damage,
    .get_poison_damage = fightstg_rules_get_poison_damage,
    .get_counter_damage = fightstg_rules_get_counter_damage,
    .get_heal = fightstg_rules_get_heal,
    .get_regen = fightstg_rules_get_regen,
    .roll_hit = fightstg_rules_roll_hit,
    .roll_special_hit = fightstg_rules_roll_special_hit,
    .roll_poison = fightstg_rules_roll_poison,
    .roll_paralysis = fightstg_rules_roll_paralysis,
    .roll_confusion = fightstg_rules_roll_confusion,
    .roll_sleep = fightstg_rules_roll_sleep,
    .roll_knockout = fightstg_rules_roll_knockout,
    .roll_steal = fightstg_rules_roll_steal,
    .roll_drain = fightstg_rules_roll_drain,
    .roll_revert = fightstg_rules_roll_revert,
    .roll_lower_stat = fightstg_rules_roll_lower_stat,
    .roll_switch_seal = fightstg_rules_roll_switch_seal,
    .roll_digivolve_seal = fightstg_rules_roll_digivolve_seal,
    .roll_counter = fightstg_rules_roll_counter,
    .roll_escape = fightstg_rules_roll_escape,
    .roll_wake = fightstg_rules_roll_wake,
    .roll_confused = fightstg_rules_roll_confused,
    .roll_paralyzed = fightstg_rules_roll_paralyzed,
    .change_modifier = fightstg_rules_change_modifier,
    .get_gauge_gain = fightstg_rules_get_gauge_gain,
    .get_tech_cost = fightstg_rules_get_tech_cost,
};

s32 fightstg_rules_opposite_elements[9] = {
    0, 0, 4, 2, 5, 3, 7, 8,
    6,
};

s16 fightstg_rules_modifier_stats[4] = { 0, 1, 4, 0 };

FightstgMath fightstg_math = {
    .nop = fightstg_math_nop,
    .lerp = fightstg_math_lerp,
    .wave = fightstg_math_wave,
};
