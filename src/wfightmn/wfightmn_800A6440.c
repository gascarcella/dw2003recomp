#define WFIGHTMN_CAP_DAMAGE_DEFINED /* its definition takes an s32 count (include/wfightmn.h) */
#include "common.h"
#include "object.h"
#include "heap.h"
#include "gfx.h"
#include "sound.h"
#include "pad.h"
#include "gamestate.h"
#include "records.h"
#include "overlay_common.h"
#include "fightstg.h"

/* WFIGHTMN: the battle overlay FIGHTSTG loads (file 0x208) for a normal battle. Its main object
 * (wfightmn_main_update) sets the battle up, then runs FIGHTSTG's event queue (fightstg_events): one
 * handler per event type (wfightmn_handlers). */

/* FIGHTSTG's command menu (fightstg_command_create; fightstg_8008D3B4.c's FightstgCommand), the part used here. */
typedef struct FightstgCommand {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 last;
    /* 0x54 */ s32 member_sel;
    /* 0x58 */ s32 choice;
    /* 0x5C */ s32 command; /* the command chosen: 0..6 */
    /* 0x60 */ s32 arg; /* its member */
    /* 0x64 */ s32 arg2; /* its target, item or Digimon */
    /* 0x68 */ s32 team_tech;
} FightstgCommand; /* size 0x6C */

/* FIGHTSTG's message box (fightstg_message_create; fightstg_8008D3B4.c's FightstgMessage), the part used here. */
typedef struct FightstgMessage {
    /* 0x00 */ Object base;
    /* 0x50 */ u8 unk_50[0x5C];
    /* 0xAC */ void (*show)(struct FightstgMessage *obj, s32 count, s32 *args); /* fightstg_message_show: shows a message */
    /* 0xB0 */ void (*close)(struct FightstgMessage *obj); /* fightstg_message_close */
} FightstgMessage; /* size 0xB4 */

/* FIGHTSTG's model scene (fightstg_entrance_create; fightstg_80086A00.c's FightstgEntrance), the part used here. */
typedef struct FightstgEntrance {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 done; /* done */
} FightstgEntrance;

/* What the main object waits for (WfightmnMainData.wait): a message box, a scene, a script or a fade. */
typedef union WfightmnWait {
    Object *obj;
    FightstgMessage *msg;
    FightstgEntrance *scene;
    FightstgScript *script;
    Fade *fade;
} WfightmnWait;

/* wfightmn_main_update's object (on the object list as 0xC): base.step is the state (0: take the next event,
 * 1: pick its handler, 2: wait for data->wait, 3..26: the handlers in wfightmn_handlers), base.substep a handler's step. */
typedef struct WfightmnMain {
    /* 0x00 */ Object base;
    /* 0x50 */ s32 msg_args[8]; /* a message's arguments (wait.msg->show) */
    /* 0x70 */ s32 status_sound; /* the status sound is playing */
} WfightmnMain; /* size 0x74 */

/* Its data block (base.children): FIGHTSTG's objects it creates. */
typedef struct WfightmnMainData {
    /* 0x00 */ Object *loader; /* the loader (wfightmn_loader_create) */
    /* 0x04 */ FightstgCommand *command; /* the command menu */
    /* 0x08 */ FightstgCamera *camera; /* the camera */
    /* 0x0C */ FightstgLighting *lighting; /* the lighting */
    /* 0x10 */ FightstgStage *stage;  /* the stage */
    /* 0x14 */ FightstgSlots *slots;  /* the models */
    /* 0x18 */ Object *intro_camera; /* the camera sequence (fightstg_intro_camera_create) */
    /* 0x1C */ WfightmnWait wait;
} WfightmnMainData; /* size 0x20 */

/* FIGHTSTG functions this overlay calls. FIGHTSTG's own files define several as void: they leave their object in
 * v0, which this overlay keeps (on the host they return it: OBJECT_V0, FINDINGS 8). */
FightstgCommand *fightstg_command_create();
FightstgCamera *fightstg_camera_create(s32 layer);
FightstgStage *fightstg_stage_create(s32 stage, s32 frames);
FightstgSlots *fightstg_slots_create(void);
FightstgLighting *fightstg_lights_create(s32 layer);
Object *fightstg_intro_camera_create(void);
void fightstg_events_add_party_turn(s32 delay);
void fightstg_events_start_regen(u8 side, s32 member, s32 arg2);
void fightstg_command_set_phase(s32 phase);
s32 fightstg_command_is_open();
FightstgEntrance *fightstg_entrance_create(s32 id, s32 enemy, s32 weak);
Object *fightstg_attack_create(s32 side);
Object *fightstg_tech_create(s32 side, s32 action);
Object *fightstg_item_create(s32 member);
void fightstg_events_add_escape(u8 side);
Object *fightstg_digivolve_create(s32 id, s32 blast);
void fightstg_events_add_boost_end(void);
Fade *fightstg_fade_create(void);
Object *fightstg_player_reaction_create(s32 arg0, s32 arg1);
void fightstg_events_start_blast(s32 id);
Object *fightstg_scripted_turn_create(void);
Object *fightstg_boss_turn_create(s32 arg0, s32 arg1);
Object *fightstg_enemy_turn_create(void);
void fightstg_events_add_gauge_full(void);
void fightstg_events_start_final_phase(void);
Object *fightstg_defeat_camera_create(void);

void wfightmn_main_update();
Object *wfightmn_loader_create(void);
void wfightmn_run_events(WfightmnMain *obj);

extern RECT wfightmn_screen_rect; /* the screen: 0, 0, 320, 240 */

/* Per technique kind (RecordsTechnique.kind): the two effect values wfightmn_boss_enters copies; -1 ends. */
typedef struct WfightmnKindEffect {
    /* 0x0 */ s32 kind;
    /* 0x4 */ s32 effect;
    /* 0x8 */ s32 hit_sound;
} WfightmnKindEffect; /* size 0xC */

/* Per technique defense_stat: the three effect values wfightmn_boss_enters copies. */
typedef struct WfightmnStatEffect {
    /* 0x0 */ s32 stat;
    /* 0x4 */ s32 effect;
    /* 0x8 */ s32 hit_sound;
    /* 0xC */ s32 stage;
} WfightmnStatEffect; /* size 0x10 */

extern WfightmnKindEffect wfightmn_kind_effects[7];
extern WfightmnStatEffect wfightmn_stat_effects[8];

/* The main object's states 3..26: the event handlers (0..2 unused). */
extern void (*wfightmn_handlers[27])(WfightmnMain *obj, WfightmnMainData *data);

/* Sets up the display and the battle's seven layers (0x1000-0x1006). */
void wfightmn_init_layers(void) {
    GfxLayer *layer;

    gfx_module.reset();
    gfx_module.alloc_packet_buffers(0x19000);
    gfx_module.funcs.init_display(320, 240, 0, 0);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 1, 0x1000);
    layer->set_draw_offset(layer, 160, 120);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 1, 0x1001);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 8, 0x1002);
    layer->set_draw_offset(layer, wfightmn_screen_rect.w / 2, wfightmn_screen_rect.h / 2);
    layer->alloc_callbacks(layer, 40);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 1, 0x1003);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 12, 0x1004);
    layer->set_draw_offset(layer, 160, 120);
    layer->alloc_callbacks(layer, 100);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 1, 0x1005);
    layer->set_draw_offset(layer, 0, 0);
    layer = gfx_module.funcs.create_layer(&wfightmn_screen_rect, 1, 0x1006);
    layer->set_draw_offset(layer, 0, 0);
    layer->alloc_callbacks(layer, 10);
}

/* Fills the members' battle state: the party's from the game state (`leader`: the Digimon the first member
 * fights as), the enemies' from the battle (records_state). */
void wfightmn_init_members(s32 leader) {
    FightstgMember *members = fightstg_battle.state.members[0]; /* the party's, then the enemies' */
    s32 i;
    s32 id;
    GamestateRecord *rec;
    FightstgEnemyRecord *digimon;

    for (i = 0; i < 3; i++) {
        id = gamestate_data.funcs.get_party_member(i);
        if (id >= 0) {
            rec = gamestate_data.funcs.get_record(id);
            if (i == 0) {
                members[0].digimon = leader;
            } else {
                members[i].digimon = records_digimon[id].id;
            }
            members[i].hp = rec->stats.values[2];
            members[i].max_hp = rec->stats.values[3];
            members[i].mp = rec->stats.values[4];
            members[i].max_mp = rec->stats.values[5];
        }
    }
    members = fightstg_battle.state.members[1];
    for (i = 0; i < 3; i++) {
        members[i].digimon = records_state.enemies[i].digimon;
        if (records_state.enemies[i].digimon != 0) {
            members[i].hp = members[i].max_hp = records_state.enemies[i].hp;
            members[i].mp = members[i].max_mp = records_state.enemies[i].mp;
            digimon = fightstg_enemy_records.get(members[i].digimon);
            if (digimon->item != 0) {
                members[i].item = digimon->item;
            }
        }
    }
}

/* Whether the party strikes first: never without the battle's chance (records_state.first_strike_chance), which shrinks with
 * the first member's level and the first enemy's. */
s32 wfightmn_roll_first_strike(void) {
    s32 levels;
    s32 chance;
    s32 r;

    if (records_state.first_strike_chance == 0) {
        return 0;
    }
    levels = 32 - gamestate_data.digimon[gamestate_data.funcs.get_party_member(0)].record.stats.values[0];
    chance = records_state.first_strike_chance * (levels - records_state.enemies[0].level) / 32;
    r = pad_random.next() % 128;
    if (r == 0) {
        return 1;
    }
    return r < chance;
}

/* Party member `member`: if it has item 0x140 in equipment slot 4 or 5, fightstg_events_start_regen(0, member, 0). */
void wfightmn_check_regen(s32 member) {
    s32 id = gamestate_data.funcs.get_party_member(member);
    s32 i;
    s16 *equip;

    if (id >= 0) {
        equip = &gamestate_data.funcs.get_record(id)->equipment[4];
        for (i = 0; i < 2; i++) {
            if (equip[i] == 0x140) {
                fightstg_events_start_regen(0, member, 0);
                return;
            }
        }
    }
}

/* Calls wfightmn_check_regen for each of the three party members. */
void wfightmn_check_regen_all(void) {
    s32 i;

    for (i = 0; i < 3; i++) {
        wfightmn_check_regen(i);
    }
}

/* The acting party member took part (records_battle_results); if it fights as one of its other forms, also that form's
 * slot. */
void wfightmn_mark_took_part(void) {
    s16 forms[4];
    s32 id;
    FightstgMember *member;
    s32 i;
    RecordsDigimon *digimon;

    records_battle_results.members[fightstg_battle.state.current[0]].took_part = 1;
    id = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
    member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    digimon = &records_digimon[id];
    if (digimon->id != member->digimon && gamestate_data.funcs.get_chosen_forms(id, forms) > 0) {
        for (i = 0; i < 3; i++) {
            if (forms[i] == member->digimon) {
                records_battle_results.members[fightstg_battle.state.current[0]].forms[i] = 1;
                return;
            }
        }
    }
}

/* As wfightmn_mark_took_part, for the member and form the command menu chose. */
void wfightmn_mark_chosen_took_part(WfightmnMain *obj, WfightmnMainData *data) {
    s16 forms[4];
    s32 i = 0;
    s32 member = -1;
    s32 id = gamestate_data.funcs.get_party_member(data->command->arg);

    for (; i < 3; i++) {
        if (gamestate_data.funcs.get_party_member(i) == id) {
            member = i;
            break;
        }
    }
    records_battle_results.members[member].took_part = 1;
    if (gamestate_data.funcs.get_chosen_forms(gamestate_data.funcs.get_party_member(member), forms) > 0) {
        for (i = 0; i < 3; i++) {
            if (forms[i] == data->command->arg2) {
                records_battle_results.members[member].forms[i] = 1;
                return;
            }
        }
    }
}

/* The battle's main object (the overlay's entry creates it): sets up the screen, the music, FIGHTSTG's objects and
 * the members, decides who strikes first, then runs the events (wfightmn_run_events). */
void wfightmn_main_update(WfightmnMain *obj, WfightmnMainData *data) {
    s16 forms[4];
    s32 id;
    s32 leader;
    FightstgMember *member;
    FightstgStats *stats;
    s32 chance;
    s32 i;
    s32 found;
    s32 map;
    FightstgMember *party;

    switch (obj->base.state) {
    case OBJECT_STATE_INIT:
    default:
        switch (obj->base.step) {
        case 0:
        default:
            wfightmn_init_layers();
            obj->base.next_step(obj);
            /* fallthrough */
        case 1:
            switch (obj->base.substep) {
            case 0:
            default:
                sound_module.load_extra_bank((records_state.music >> 18) & 0x7F);
                obj->base.next_substep(obj);
                /* fallthrough */
            case 1:
                if (sound_module.is_loading() == 0) {
                    sound_module.play(records_state.music);
                    obj->base.next_step(obj);
                }
                break;
            }
            break;
        case 2:
            switch (obj->base.substep) {
            case 0:
            default:
                data->command = fightstg_command_create();
                data->camera = fightstg_camera_create(0x1001);
                data->stage = fightstg_stage_create(records_state.stage, 60);
                data->slots = fightstg_slots_create();
                data->lighting = fightstg_lights_create(0x1001);
                obj->base.next_substep(obj);
                break;
            case 1:
                map = gamestate_data.funcs.get_prev_map();
                if (map == 0x22D && records_state.battle == 0x143) {
                    fightstg_battle.state.type = 1;
                } else if (map == 0x23A && records_state.battle == 0xB) {
                    fightstg_battle.state.type = 2;
                } else if (map == 0x272 && records_state.battle == 0x1E) {
                    fightstg_battle.state.type = 3;
                } else if (records_state.battle == 0x144) {
                    fightstg_battle.state.type = 4;
                    fightstg_battle.state.final_phase = 0;
                    fightstg_battle.state.hits = 0;
                    fightstg_battle.state.copied_tech = 0;
                } else {
                    fightstg_battle.state.type = 0;
                }
                id = gamestate_data.funcs.get_party_member(0);
                if (wfightmn_roll_first_strike() != 0) {
                    leader = records_digimon[id].id;
                    obj->base.timer = 1;
                } else if (gamestate_data.funcs.get_chosen_forms(id, forms) > 0 && gamestate_data.digimon[id].shown_form != 0) {
                    leader = gamestate_data.digimon[id].shown_form;
                } else {
                    leader = records_digimon[id].id;
                }
                data->slots->add(data->slots, 0, leader, 1);
                data->slots->reset_pos(data->slots, 0);
                data->slots->add(data->slots, 0x10, records_state.enemies[0].digimon, 1);
                data->slots->reset_pos(data->slots, 0x10);
                wfightmn_init_members(leader);
                member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
                if (member->max_hp / 4 >= member->hp) {
                    data->slots->set_idle_anim(data->slots, 0, 1);
                }
                gfx_module.frame_ticks = 1;
                data->intro_camera = fightstg_intro_camera_create();
                obj->base.substep++;
                break;
            case 2:
                gfx_module.frame_ticks = 1;
                data->loader = wfightmn_loader_create();
                obj->base.substep++;
                break;
            case 3:
                if (data->loader != NULL) {
                    break;
                }
                obj->base.substep++;
                break;
            case 4:
                wfightmn_mark_took_part();
                obj->base.substep++;
                break;
            case 5:
                if (obj->base.timer != 0) {
                    fightstg_events_add_enemy_turn(0);
                    fightstg_events_add_party_turn(fightstg_events.get_delay(0, 0) / 2);
                    data->wait.msg = fightstg_message_create();
                    obj->msg_args[0] = 7;
                    data->wait.msg->show(data->wait.msg, 1, obj->msg_args);
                    obj->base.substep++;
                    fightstg_command_set_phase(1);
                } else {
                    stats = fightstg_rules.get_stats(0, 1, fightstg_battle.state.current[0]);
                    chance = fightstg_rules.get_stats(0x10, 0, fightstg_battle.state.current[1])->stats[4] * 8 /
                             stats->stats[4];
                    if (pad_random.next() % 128 < chance) {
                        fightstg_events_add_enemy_turn(0);
                        fightstg_events_add_party_turn(fightstg_events.get_delay(0, 0) / 2);
                        fightstg_command_set_phase(1);
                    } else {
                        fightstg_events_add_party_turn(0);
                        fightstg_events_add_enemy_turn(fightstg_events.get_delay(0, 0) / 2);
                    }
                    obj->base.next_state(obj);
                }
                wfightmn_check_regen_all();
                break;
            case 6:
                if (data->wait.obj == NULL) {
                    obj->base.next_state(obj);
                }
                break;
            }
            break;
        }
        break;
    case OBJECT_STATE_RUN:
        wfightmn_run_events(obj);
        party = fightstg_battle.state.members[0];
        if (obj->status_sound == 0) {
            for (i = 0; i < 3; i++) {
                if (party[i].status & 4) {
                    sound_module.play(0x60040000);
                    obj->status_sound = 1;
                    break;
                }
            }
        } else {
            found = 0;
            for (i = 0; i < 3; i++) {
                if (party[i].status & 4) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                sound_module.play(records_state.music);
                obj->status_sound = 0;
            }
        }
        break;
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    }
}

/* Ends the acting party member's form change (blasted) and its pending events of types 0x13 and 8. */
void wfightmn_end_blast(void) {
    s32 event = fightstg_events.find_member(0x13, 0, fightstg_battle.state.current[0]);
    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    s32 id = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);

    if (member->blasted != 0) {
        if (event >= 0) {
            fightstg_events.events[event].type = 0;
        }
        records_state.gauges[id] = 0;
        member->blasted = 0;
        event = fightstg_events.find_member(8, 0, fightstg_battle.state.current[0]);
        if (event >= 0) {
            fightstg_events.events[event].type = 0;
        }
    }
}

void wfightmn_party_turn(WfightmnMain *obj, WfightmnMainData *data) {
    s32 event;
    FightstgMember *member;

    switch (obj->base.substep) {
    case 0:
    default:
        event = fightstg_events.find_member(8, 0, fightstg_battle.state.current[0]);
        if (event >= 0) {
            fightstg_events.events[event].type = 0;
        }
        event = fightstg_events.find_member(4, 0, fightstg_battle.state.current[0]);
        if (event >= 0) {
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = 0x42;
            obj->msg_args[1] = 0;
            data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
            fightstg_events.events[event].type = 0;
        }
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
            if ((member->status & 2) && fightstg_rules.roll_paralyzed(0) != 0) {
                data->wait.msg = fightstg_message_create();
                obj->msg_args[0] = 0x56;
                obj->msg_args[1] = 0;
                data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
                fightstg_events_add_party_turn(fightstg_events.get_delay(0, 0));
                obj->base.set_step(obj, 2);
            } else if (member->status & 4) {
                obj->base.set_step(obj, 0x11);
            } else {
                fightstg_command_set_phase(2);
                obj->base.substep = 2;
            }
        }
        break;
    case 2:
        if (fightstg_command_is_open() == 0) {
            switch (data->command->command) {
            case 1:
                fightstg_battle.state.escapes++;
                fightstg_events_add_escape(0);
                data->wait.msg = fightstg_message_create();
                obj->msg_args[0] = 0x41;
                obj->msg_args[1] = 0;
                data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
                obj->base.set_step(obj, 2);
                wfightmn_check_final_phase_end(0);
                break;
            case 0:
                data->wait.obj = fightstg_attack_create(0);
                obj->base.set_step(obj, 2);
                break;
            case 4:
                data->wait.obj = fightstg_tech_create(0, data->command->arg);
                obj->base.set_step(obj, 2);
                break;
            case 2:
                obj->base.set_step(obj, 4);
                wfightmn_check_final_phase_end(0);
                break;
            case 5:
                obj->base.set_step(obj, 5);
                wfightmn_check_final_phase_end(0);
                break;
            case 6:
                obj->base.set_step(obj, 6);
                break;
            case 3:
                data->wait.obj = fightstg_item_create(data->command->arg);
                obj->base.set_step(obj, 2);
                break;
            default:
                obj->base.step = -1;
                break;
            }
            fightstg_events_add_party_turn(fightstg_events.get_delay(0, 0));
        }
        break;
    }
}

void wfightmn_digivolve(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgMember *member;
    RecordsDigimon *digimon;

    switch (obj->base.substep) {
    case 0:
    default:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        member->digimon = data->command->arg2;
        wfightmn_mark_took_part();
        wfightmn_end_blast();
        data->wait.obj = fightstg_digivolve_create(member->digimon, 0);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            digimon = records_get_digimon_func((fightstg_battle.state.members[0] + fightstg_battle.state.current[0])->digimon);
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = 0;
            obj->msg_args[1] = digimon->name_id;
            data->wait.msg->show(data->wait.msg, 12, obj->msg_args);
            obj->base.set_step(obj, 2);
        }
        break;
    }
}

void wfightmn_switch_member(WfightmnMain *obj, WfightmnMainData *data) {
    s32 i;
    s32 member;
    s32 id;
    FightstgMember *m;
    s32 next;

    switch (obj->base.substep) {
    case 0:
    default:
        data->wait.msg = fightstg_message_create();
        obj->msg_args[0] = 0x38;
        obj->msg_args[1] = 0;
        data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            i = 0;
            member = -1;
            id = gamestate_data.funcs.get_party_member(data->command->arg);
            for (; i < 3; i++) {
                if (gamestate_data.funcs.get_party_member(i) == id) {
                    member = i;
                    break;
                }
            }
            obj->msg_args[0] = member;
            obj->msg_args[1] = fightstg_battle.state.current[0];
            fightstg_battle.state.current[0] = obj->msg_args[0];
            data->wait.scene = fightstg_entrance_create(data->command->arg2, 0, wfightmn_update_idle_anim(0, 0));
            fightstg_battle.state.current[0] = obj->msg_args[1];
            obj->base.substep++;
        }
        break;
    case 2:
        if (data->wait.scene->done != 0) {
            next = obj->msg_args[0];
            m = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
            if (m->boosted != 0) {
                m->boosted = 0;
                fightstg_events_add_boost_end();
            }
            wfightmn_end_blast();
            fightstg_battle.state.current[0] = next;
            m = &fightstg_battle.state.members[0][next];
            m->digimon = data->command->arg2;
            wfightmn_mark_took_part();
            obj->base.substep++;
        }
        break;
    case 3:
        if (data->wait.obj == NULL) {
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = 0x39;
            obj->msg_args[1] = 0;
            data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
            obj->base.set_step(obj, 2);
        }
        break;
    }
}

void wfightmn_team_tech(WfightmnMain *obj, WfightmnMainData *data) {
    s32 i;
    FightstgMember *enemies;

    switch (obj->base.substep) {
    case 0:
    default:
        data->wait.obj = fightstg_tech_create(0, data->command->team_tech);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            enemies = fightstg_battle.state.members[1];
            for (i = 0; i < 3; i++) {
                if (enemies[i].digimon != 0 && enemies[i].hp != 0) {
                    obj->base.set_step(obj, 5);
                    obj->base.substep = 1;
                    return;
                }
            }
            wfightmn_mark_chosen_took_part(obj, data);
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_battle_end(WfightmnMain *obj, WfightmnMainData *data) {
    s32 count;
    s32 i;
    s32 k;
    s32 pick;
    s32 chance;
    s32 id;
    FightstgMember *enemies;
    FightstgMember *party;
    FightstgEnemyRecord *digimon;
    GfxLayer *layer;

    switch (obj->base.substep) {
    case 0:
    default:
        if (data->intro_camera == NULL || data->intro_camera->state == OBJECT_STATE_DONE) {
            count = 0;
            if (fightstg_events.result != 0) {
                records_battle_results.battle = records_state.battle;
                records_battle_results.member = fightstg_battle.state.current[0];
                enemies = fightstg_battle.state.members[1];
                for (i = 0; i < 3; i++) {
                    if (enemies[i].digimon != 0 && enemies[i].item != 0) {
                        count++;
                    }
                }
#ifndef PC_PORT
                pick = pad_random.next() % count;
#else
                /* PC_PORT: FINDINGS 7: count is 0 when no enemy holds an item. The R3000A's div by 0 leaves the
                 * dividend in HI (x % 0 == x, no trap) where x86 traps; the draw is taken either way. */
                pick = pad_random.next();
                if (count != 0) {
                    pick %= count;
                }
#endif
                count = 0;
                for (i = 0; i < 3; i++) {
                    if (enemies[i].digimon != 0 && enemies[i].item != 0) {
                        if (pick == count) {
                            break;
                        }
                        count++;
                    }
                }
                digimon = fightstg_enemy_records.get(enemies[count].digimon);
                if (enemies[count].item > 0) {
                    chance = digimon->drop_rate + 1;
                    if (pad_random.next() % 1024 < chance) {
                        records_battle_results.item = enemies[count].item;
                    } else {
                        records_battle_results.item = 0;
                    }
                } else {
                    records_battle_results.item = 0;
                }
                if (records_state.has_prize == 1) {
                    records_battle_results.item = records_state.prize_item;
                }
            }
            party = fightstg_battle.state.members[0];
            for (k = 0; k < 3; k++) {
                id = gamestate_data.funcs.get_party_member(k);
                if (id >= 0) {
                    if (party[k].hp <= 0) {
                        gamestate_data.digimon[id].record.stats.values[2] = 1;
                        records_battle_results.members[k].took_part = 0;
                        records_battle_results.members[k].forms[0] = 0;
                        records_battle_results.members[k].forms[1] = 0;
                        records_battle_results.members[k].forms[2] = 0;
                    } else {
                        gamestate_data.digimon[id].record.stats.values[2] = party[k].hp;
                    }
                    gamestate_data.digimon[id].record.stats.values[4] = party[k].mp;
                }
            }
            data->wait.fade = fightstg_fade_create();
            data->wait.fade->start(data->wait.fade, 0, 10);
            obj->base.substep++;
        }
        break;
    case 1:
        if (data->wait.obj->state == OBJECT_STATE_DONE) {
            layer = gfx_module.funcs.get_layer(0x1000);
            layer->set_bg_color(layer, 0, 0, 0);
            if (data->intro_camera != NULL) {
                data->intro_camera->set_state(data->intro_camera, OBJECT_STATE_END);
            }
            obj->base.substep++;
        }
        break;
    case 2:
        wfightmn_end_blast();
        switch (fightstg_events.result) {
        case 0:
        default:
            gamestate_data.funcs.set_next_map(gamestate_data.field_map, 0);
            break;
        case 1:
            if (fightstg_battle.state.type == 6) {
                gamestate_data.funcs.set_next_map(0xE0B, 0);
            } else {
                gamestate_data.funcs.set_next_map(0x1400, 0);
            }
            break;
        case 2:
            gamestate_data.funcs.set_next_map(0xE00, 0);
            break;
        }
        break;
    }
}

/* The message arguments are set per branch (cross-jumped later; the references decide obj's register). */
void wfightmn_escape(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev;

    switch (obj->base.timer) {
    case 0:
    default:
        ev = &fightstg_events.events[fightstg_events.taken];
        if (fightstg_rules.roll_escape(ev->args[0]) != 0) {
            obj->base.timer = 1;
            ev->type = 0;
            data->wait.msg = fightstg_message_create();
            if (ev->args[0] != 0) {
                obj->msg_args[0] = 0x5F;
                obj->msg_args[1] = ev->args[0];
            } else {
                obj->msg_args[0] = 0x43;
                obj->msg_args[1] = ev->args[0];
            }
            data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
        } else {
            ev->delay = fightstg_events.get_delay(ev->args[0], 1);
            obj->base.set_step(obj, 0);
        }
        break;
    case 1:
        if (data->wait.obj == NULL) {
            fightstg_events_end_battle(0);
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_regen_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    s32 i = fightstg_events.find_member(6, ev->args[0], ev->args[1]);
    FightstgEvent *other;
    GamestateRecord *rec;

    if (i >= 0) {
        other = &fightstg_events.events[i];
        data->wait.msg = fightstg_message_create();
        obj->msg_args[0] = 0x5A;
        obj->msg_args[1] = ev->args[0];
        obj->msg_args[2] = ev->args[1];
        data->wait.msg->show(data->wait.msg, 7, obj->msg_args);
        if (other->args[0] == 0 &&
            ((rec = gamestate_data.funcs.get_record(gamestate_data.funcs.get_party_member(other->args[1])))->equipment[4] == 0x140 ||
             rec->equipment[5] == 0x140)) {
            other->type = 6;
            other->args[2] = 0;
        } else {
            other->type = 0;
        }
    }
    obj->base.set_step(obj, 2);
}

void wfightmn_regen(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    s32 side = ev->args[0] >> 4;
    FightstgMember *member = &fightstg_battle.state.members[side][ev->args[1]];

    switch (obj->base.substep) {
    case 0:
    default:
        ev->delay = 1000;
        obj->base.timer = fightstg_rules.get_regen(ev->args[0], ev->args[1], ev->args[2]);
        if (member->hp < member->max_hp) {
            if (ev->args[1] == fightstg_battle.state.current[side]) {
                data->wait.script = wfightmn_tech_script_create(ev->args[0], 0xBD);
                wfightmn_update_idle_anim(ev->args[0], -obj->base.timer);
            }
            obj->base.substep++;
        } else {
            obj->base.set_step(obj, 0);
        }
        break;
    case 1:
        if (data->wait.obj == NULL) {
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = ev->args[0];
            obj->msg_args[1] = ev->args[1];
            obj->msg_args[2] = obj->base.timer;
            data->wait.msg->show(data->wait.msg, 10, obj->msg_args);
            obj->base.substep++;
        }
        break;
    case 2:
        if (data->wait.obj == NULL) {
            member->hp += obj->base.timer;
            if (member->hp > member->max_hp) {
                member->hp = member->max_hp;
            }
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_field_end(WfightmnMain *obj, WfightmnMainData *data) {
    data->wait.msg = fightstg_message_create();
    obj->msg_args[0] = 0x6A;
    data->wait.msg->show(data->wait.msg, 1, obj->msg_args);
    fightstg_battle.state.field.element = 0;
    fightstg_battle.state.field.power = 0;
    obj->base.set_step(obj, 2);
}

void wfightmn_poison(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev;
    s32 side;
    s32 damage;
    FightstgMember *member;
    FightstgMember *target;

    switch (obj->base.substep) {
    case 0:
    default:
        ev = &fightstg_events.events[fightstg_events.taken];
        side = ev->args[0] >> 4;
        obj->msg_args[0] = ev->args[0];
        obj->msg_args[1] = ev->args[1];
        damage = fightstg_rules.get_poison_damage((FightstgPoisonArgs *)ev->args);
        obj->msg_args[2] = damage;
        if (ev->args[1] == fightstg_battle.state.current[side]) {
            member = &fightstg_battle.state.members[side][ev->args[1]];
            if (ev->args[0] == 0) {
                data->wait.obj = fightstg_player_reaction_create(member->hp - damage <= 0 ? 2 : 1, 1);
            } else {
                data->wait.script = fightstg_script_create();
                data->wait.script->side = 0;
                data->wait.script->script = 0xE;
                data->wait.script->effect = 0x13;
                data->wait.script->hit_sound = 0x1A;
                data->wait.script->stage = -1;
                if (member->hp - obj->msg_args[2] <= 0) {
                    data->wait.script->results[3] = 2;
                } else {
                    data->wait.script->results[3] = 1;
                }
            }
            wfightmn_update_idle_anim(ev->args[0], obj->msg_args[2]);
            obj->base.substep++;
        } else {
            obj->base.set_step(obj, 0);
        }
        ev->delay = 1000;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            target = &fightstg_battle.state.members[obj->msg_args[0] >> 4][obj->msg_args[1]];
            target->hp -= obj->msg_args[2];
            if (target->hp <= 0) {
                target->hp = 0;
                fightstg_events_add_knockout(obj->msg_args[0]);
            }
            data->wait.msg = fightstg_message_create();
            data->wait.msg->show(data->wait.msg, 15, obj->msg_args);
            obj->base.substep++;
        }
        break;
    case 2:
        if (data->wait.obj == NULL) {
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_status_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    FightstgMember *member;

    switch (obj->base.substep) {
    case 0:
    default:
        obj->msg_args[0] = obj->base.step + 0x1C;
        obj->msg_args[1] = ev->args[0];
        obj->msg_args[2] = ev->args[1];
        data->wait.msg = fightstg_message_create();
        data->wait.msg->show(data->wait.msg, 7, obj->msg_args);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            member = &fightstg_battle.state.members[ev->args[0] != 0][ev->args[1]];
            switch (obj->base.step) {
            case 13:
            default:
                member->status &= ~2;
                break;
            case 14:
                member->status &= ~4;
                break;
            case 15:
                member->status &= ~8;
                break;
            }
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_modifier_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    FightstgMember *member;

    switch (obj->base.substep) {
    case 0:
    default:
        data->wait.msg = fightstg_message_create();
        obj->msg_args[0] = ev->args[2] + 0x5B;
        obj->msg_args[1] = ev->args[0];
        obj->msg_args[2] = ev->args[1];
        data->wait.msg->show(data->wait.msg, 7, obj->msg_args);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            member = &fightstg_battle.state.members[ev->args[0] >> 4][ev->args[1]];
            member->modifiers[ev->args[2]] = 0;
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_confused_turn(WfightmnMain *obj, WfightmnMainData *data) {
    switch (obj->base.substep) {
    case 0:
    default:
        fightstg_command_set_phase(3);
        obj->base.substep++;
        break;
    case 1:
        if (fightstg_command_is_open() == 0) {
            if (data->command->command == 0) {
                data->wait.obj = fightstg_attack_create(0);
                obj->base.set_step(obj, 2);
            } else {
                obj->base.set_step(obj, 0);
            }
            fightstg_events_add_party_turn(fightstg_events.get_delay(0, 0));
        }
        break;
    }
}

void wfightmn_seal_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    FightstgMember *member;

    switch (obj->base.substep) {
    case 0:
    default:
        data->wait.msg = fightstg_message_create();
        obj->msg_args[0] = ev->args[2] + 0x57;
        obj->msg_args[1] = ev->args[0];
        obj->msg_args[2] = ev->args[1];
        data->wait.msg->show(data->wait.msg, 7, obj->msg_args);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            member = &fightstg_battle.state.members[ev->args[0] >> 4][ev->args[1]];
            member->status &= ~(1 << (ev->args[2] + 4));
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

void wfightmn_blast(WfightmnMain *obj, WfightmnMainData *data) {
    s32 id;
    s16 level;
    s32 stage;
    FightstgMember *member;
    FightstgMember *m; /* the same member, once the scene is done */
    RecordsDigimon *digimon;
    FightstgSlots *models;
    s32 i;

    switch (obj->base.substep) {
    case 0:
    default:
        data->wait.msg = fightstg_message_create();
        obj->msg_args[0] = 0x61;
        obj->msg_args[1] = 0;
        data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            id = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
            level = gamestate_data.funcs.get_record(id)->stats.values[0];
            if (level < 4) {
                stage = 0;
            } else if (level < 19) {
                stage = 1;
            } else if (level < 39) {
                stage = 2;
            } else if (level < 70) {
                stage = 3;
            } else {
                stage = 4;
            }
            member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
            digimon = records_get_digimon_func(records_digimon[id].id);
            member->base_digimon = member->digimon;
            member->digimon = records_digimon[digimon->blast_forms[stage] - 1].id;
            member->blasted = 1;
            fightstg_events_start_blast(id);
            data->wait.obj = fightstg_digivolve_create(member->digimon, 1);
            obj->base.substep++;
        }
        break;
    case 2:
        models = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
        if (models->get_params(models, 0)->anim == 0xD) {
            m = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
            m->hp = m->max_hp;
            if (m->status & 4) {
                i = fightstg_events.find_member(0xB, 0, fightstg_battle.state.current[0]);
                if (i >= 0) {
                    fightstg_events.events[i].type = 0;
                    fightstg_events.events[i].delay = 0;
                }
                m->status &= ~4;
                obj->status_sound = 0;
            }
            obj->base.set_step(obj, 2);
        }
        break;
    }
}

void wfightmn_blast_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgMember *member;

    switch (obj->base.substep) {
    case 0:
    default:
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        member->digimon = member->base_digimon;
        wfightmn_end_blast();
        data->wait.scene = fightstg_entrance_create(member->digimon, 0, wfightmn_update_idle_anim(0, 0));
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = 0x62;
            obj->msg_args[1] = 0;
            data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
            obj->base.set_step(obj, 2);
        }
        break;
    }
}

void wfightmn_knockout(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev;
    FightstgMember *member;
    FightstgMember *members;
    FightstgMember *enemy;
    FightstgEnemyRecord *digimon;
    FightstgSlots *models;
    s32 side;
    s32 found;
    s32 index;
    s32 i;

    switch (obj->base.substep) {
    case 0:
    default:
        ev = &fightstg_events.events[fightstg_events.taken];
        data->wait.msg = fightstg_message_create();
        if (ev->args[0] != 0 && fightstg_battle.state.type == 4) {
            obj->msg_args[0] = 0x8B;
            data->wait.msg->show(data->wait.msg, 1, obj->msg_args);
            obj->base.set_substep(obj, 5);
            break;
        }
        fightstg_events.remove_member(ev->args);
        side = ev->args[0] >> 4;
        member = &fightstg_battle.state.members[side][fightstg_battle.state.current[side]];
        if (side == 0) {
            if (member->boosted != 0) {
                member->boosted = 0;
                fightstg_events_add_boost_end();
            }
            obj->msg_args[0] = 0x50;
            obj->msg_args[1] = 0;
        } else {
            obj->msg_args[0] = 0x51;
            obj->msg_args[1] = 0x10;
        }
        data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
        member->power_up = 0;
        member->status = 0;
        member->confusion_power = 0;
        member->sleep_power = 0;
        member->paralysis_power = 0;
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            found = -1;
            ev = &fightstg_events.events[fightstg_events.taken];
            members = fightstg_battle.state.members[ev->args[0] >> 4];
            for (i = 0; i < 3; i++) {
                if (members[i].digimon != 0 && members[i].hp != 0) {
                    found = i;
                    break;
                }
            }
            if (found != -1) {
                if (ev->args[0] == 0) {
                    fightstg_command_set_phase(4);
                    obj->base.substep = 2;
                } else {
                    members = fightstg_battle.state.members[1] + found;
                    obj->base.substep = 3;
                    obj->msg_args[0] = found;
                    obj->msg_args[1] = fightstg_battle.state.current[1];
                    fightstg_battle.state.current[1] = found;
                    data->wait.scene = fightstg_entrance_create(members->digimon, 1, wfightmn_update_idle_anim(0x10, 0));
                    fightstg_battle.state.current[1] = obj->msg_args[1];
                }
            } else {
                data->wait.msg = fightstg_message_create();
                if (ev->args[0] == 0) {
                    fightstg_events_end_battle(2);
                    sound_module.play(0x60040008);
                    obj->msg_args[0] = 0x53;
                    obj->base.set_step(obj, 2);
                    data->intro_camera = fightstg_defeat_camera_create();
                } else {
                    fightstg_events_end_battle(1);
                    if (fightstg_battle.state.type != 6) {
                        sound_module.play(0x6004001E);
                    }
                    obj->msg_args[0] = 0x52;
                    obj->base.set_step(obj, 0x18);
                    obj->msg_args[1] = gfx_module.funcs.get_time();
                    obj->msg_args[2] = 100;
                    models = (FightstgSlots *)heap_objects.find(0x14, -1, -1);
                    models->get_params(models, 0)->anim = 0xD;
                }
                data->wait.msg->show(data->wait.msg, 1, obj->msg_args);
            }
        }
        break;
    case 2:
        if (fightstg_command_is_open() == 0) {
            obj->base.set_step(obj, 5);
            obj->base.substep = 1;
        }
        break;
    case 3:
        if (data->wait.scene->done != 0) {
            fightstg_battle.state.current[1] = obj->msg_args[0];
            obj->base.substep++;
        }
        break;
    case 4:
        if (data->wait.obj == NULL) {
            enemy = fightstg_battle.state.members[1];
            digimon = fightstg_enemy_records.get((enemy + fightstg_battle.state.current[1])->digimon);
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = digimon->name;
            data->wait.msg->show(data->wait.msg, 13, obj->msg_args);
            obj->base.set_step(obj, 2);
        }
        break;
    case 5:
        switch (obj->base.timer) {
        case 0:
        default:
            if (data->wait.obj == NULL) {
                fightstg_battle.state.type = 5;
                fightstg_battle.state.current[1] = 1;
                data->wait.scene = fightstg_entrance_create(fightstg_battle.state.members[1][1].digimon, 1, 0);
                fightstg_battle.state.current[1] = 0;
                obj->base.timer++;
            }
            break;
        case 1:
            if (data->wait.scene->done != 0) {
                fightstg_battle.state.members[1][0] = fightstg_battle.state.members[1][1];
                fightstg_battle.state.current[1] = 0;
                fightstg_battle.state.members[1][1].digimon = 0;
                obj->base.timer++;
            }
            break;
        case 2:
            index = fightstg_events.find(3);
            if (index >= 0) {
                fightstg_events.events[index].delay = 0;
            }
            obj->base.set_step(obj, 2);
            break;
        }
        break;
    }
}

void wfightmn_boost_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgEvent *ev = &fightstg_events.events[fightstg_events.taken];
    FightstgMember *member;

    member = &fightstg_battle.state.members[0][ev->args[1]];
    member->boosted = 0;
    data->wait.msg = fightstg_message_create();
    obj->msg_args[0] = 0x59;
    obj->msg_args[1] = ev->args[0];
    obj->msg_args[2] = ev->args[1];
    data->wait.msg->show(data->wait.msg, 7, obj->msg_args);
    obj->base.set_step(obj, 2);
}

void wfightmn_revert_form(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgMember *member;
    s32 i;

    switch (obj->base.substep) {
    case 0:
    default:
        i = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);
        member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
        member->digimon = records_digimon[i].id;
        member->power_up = 0;
        wfightmn_end_blast();
        i = fightstg_events.find_member(8, 0, fightstg_battle.state.current[0]);
        if (i >= 0) {
            fightstg_events.events[i].type = 0;
        }
        data->wait.scene = fightstg_entrance_create(member->digimon, 0, wfightmn_update_idle_anim(0, 0));
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.obj == NULL) {
            data->wait.msg = fightstg_message_create();
            obj->msg_args[0] = 0x47;
            obj->msg_args[1] = 0;
            data->wait.msg->show(data->wait.msg, 2, obj->msg_args);
            obj->base.set_step(obj, 2);
        }
        break;
    }
}

void wfightmn_victory_wait(WfightmnMain *obj, WfightmnMainData *data) {
    switch (obj->base.substep) {
    case 0:
    default:
        if (data->wait.obj != NULL) {
            if (obj->msg_args[2] < gfx_module.funcs.get_time() - obj->msg_args[1]) {
                data->wait.msg->close(data->wait.msg);
                obj->base.substep++;
            }
            break;
        }
        obj->base.set_step(obj, 0);
        break;
    case 1:
        if (data->wait.obj != NULL) {
            data->wait.obj->state = OBJECT_STATE_END;
        }
        obj->base.set_step(obj, 0);
        break;
    }
}

void wfightmn_boss_enters(WfightmnMain *obj, WfightmnMainData *data) {
    RecordsTechnique *rec;
    RecordsTechnique *tech;
    s32 i;

    switch (obj->base.substep) {
    case 0:
    default:
        fightstg_battle.state.type = 6;
        fightstg_battle.state.current[1] = 2;
        data->wait.scene = fightstg_entrance_create(fightstg_battle.state.members[1][2].digimon, 1, 0);
        fightstg_battle.state.current[1] = 0;
        obj->base.substep++;
        break;
    case 1:
        if (data->wait.scene->done != 0) {
            fightstg_battle.state.members[1][0] = fightstg_battle.state.members[1][2];
            fightstg_battle.state.current[1] = 0;
            fightstg_battle.state.members[1][2].digimon = 0;
            obj->base.substep++;
        }
        break;
    case 2:
        if (data->wait.obj == NULL) {
            rec = &records_techniques[0x1B7];
            *rec = records_techniques[0x1BA];
            if (fightstg_battle.state.copied_tech != 0) {
                tech = &records_techniques[fightstg_battle.state.copied_tech - 1];
                if (tech->anim_script != 5 && tech->anim_script != 12) {
                    if (tech->kind >= 2 && (tech->kind < 9 || tech->kind > 10) && tech->kind != 12) {
                        rec->kind = tech->kind;
                        rec->effect_power = tech->effect_power;
                        rec->effect_chance = tech->effect_chance;
                        for (i = 0; wfightmn_kind_effects[i].kind != -1; i++) {
                            if (wfightmn_kind_effects[i].kind == tech->kind) {
                                rec->anim_effect = wfightmn_kind_effects[i].effect;
                                rec->hit_sound = wfightmn_kind_effects[i].hit_sound;
                                rec->anim_stage = 0;
                                break;
                            }
                        }
                    }
                    if (tech->defense_stat >= 2) {
                        rec->defense_stat = tech->defense_stat;
                        rec->element_power = tech->element_power;
                        if (rec->kind < 2) {
                            i = 0;
                            while (1) {
                                if (wfightmn_stat_effects[i].stat == tech->defense_stat) {
                                    rec->anim_effect = wfightmn_stat_effects[i].effect;
                                    rec->hit_sound = wfightmn_stat_effects[i].hit_sound;
                                    rec->anim_stage = wfightmn_stat_effects[i].stage;
                                    break;
                                }
                                i++;
                            }
                        }
                    }
                }
                data->wait.msg = fightstg_message_create();
                obj->msg_args[0] = fightstg_battle.state.copied_tech;
                data->wait.msg->show(data->wait.msg, 22, obj->msg_args);
                obj->base.substep = 3;
            } else {
                obj->base.substep = 4;
            }
        }
        break;
    case 3:
        if (data->wait.obj == NULL) {
            obj->base.substep++;
        }
        break;
    case 4:
        obj->base.set_step(obj, 2);
        break;
    }
}

void wfightmn_final_phase_end(WfightmnMain *obj, WfightmnMainData *data) {
    FightstgNewEvent event;
    s32 i = fightstg_events.find(3);

    if (i >= 0) {
        fightstg_events.events[i].type = 0;
    }
    event.type = 3;
    event.delay = 1;
    event.args[0] = -1;
    fightstg_events.add_first(&event);
    fightstg_battle.state.members[1][0].modifiers[3] = 0;
    fightstg_battle.state.members[1][0].modifiers[1] = 0;
    obj->base.set_step(obj, 2);
}

void wfightmn_run_events(WfightmnMain *obj) {
    WfightmnMainData *data = (WfightmnMainData *)obj->base.children;

    switch (obj->base.step) {
    default:
        if (obj->base.step >= 3 && obj->base.step < 27) {
            wfightmn_handlers[obj->base.step](obj, data);
        } else {
            obj->base.set_step(obj, 1);
            if ((obj->base.substep = fightstg_events.take_next()) == 0) {
                fightstg_events_add_party_turn(0);
                fightstg_events_add_enemy_turn(fightstg_events.get_delay(0, 0) / 2);
                obj->base.set_step(obj, 0);
            }
        }
        break;
    case 0:
        obj->base.set_step(obj, 1);
        obj->base.substep = fightstg_events.take_next();
        break;
    case 1:
        switch (obj->base.substep) {
        default:
            obj->base.set_step(obj, 3);
            break;
        case 1:
            obj->base.set_step(obj, 7);
            break;
        case 3:
            if (fightstg_battle.state.type == 5) {
                data->wait.obj = fightstg_scripted_turn_create();
                obj->base.set_step(obj, 2);
            } else if (fightstg_battle.state.type == 6) {
                data->wait.obj = fightstg_boss_turn_create(0, 0);
                obj->base.set_step(obj, 2);
            } else {
                (fightstg_battle.state.members[1] + fightstg_battle.state.current[1])->turns++;
                data->wait.obj = fightstg_enemy_turn_create();
                obj->base.set_step(obj, 2);
            }
            break;
        case 4:
            obj->base.set_step(obj, 8);
            break;
        case 5:
            obj->base.set_step(obj, 9);
            break;
        case 6:
            obj->base.set_step(obj, 10);
            break;
        case 7:
            obj->base.set_step(obj, 11);
            break;
        case 9:
            obj->base.set_step(obj, 12);
            break;
        case 10:
            obj->base.set_step(obj, 13);
            break;
        case 11:
            obj->base.set_step(obj, 14);
            break;
        case 12:
            obj->base.set_step(obj, 15);
            break;
        case 13:
        case 14:
        case 15:
            obj->base.set_step(obj, 16);
            break;
        case 16:
        case 17:
            obj->base.set_step(obj, 18);
            break;
        case 18:
            obj->base.set_step(obj, 19);
            break;
        case 19:
            obj->base.set_step(obj, 20);
            break;
        case 20:
            obj->base.set_step(obj, 21);
            break;
        case 21:
            obj->base.set_step(obj, 22);
            break;
        case 22:
            obj->base.set_step(obj, 23);
            break;
        case 23:
            obj->base.set_step(obj, 25);
            break;
        case 24:
            obj->base.set_step(obj, 26);
            break;
        }
        break;
    case 2:
        if (data->wait.obj == NULL) {
            obj->base.set_step(obj, 0);
        }
        break;
    }
}

/* The overlay's entry (FIGHTSTG's fightstg_main_update keeps the object it returns): creates the battle's
 * main object (wfightmn_main_update, 0x74 bytes) and clears the battle state and the last battle's results. */
Object *wfightmn_main_create(void) {
    Object *obj = object_create(wfightmn_main_update, sizeof(WfightmnMain), sizeof(WfightmnMainData), 0xC);

    heap_funcs.bzero(&fightstg_battle.state, sizeof(FightstgBattleState));
    heap_funcs.bzero(&records_battle_results, 0x12); /* PC_PORT: a byte count: the fields before unk_12 (pointer-free) */
    return obj;
}

void wfightmn_note_copied_tech(u8 side, s32 id) {
    RecordsTechnique *tech = &records_techniques[id - 1];
    s32 flag;
    u8 kind;

    if (side == 0 && fightstg_battle.state.type == 4) {
        flag = 0;
        if (tech->anim_script != 5 && tech->anim_script != 12) {
            kind = tech->kind;
            if (kind >= 2 && (kind < 9 || kind > 10) && kind != 12) {
                flag = 1;
            }
            if (tech->defense_stat >= 2) {
                flag = 1;
            }
            if (flag) {
                fightstg_battle.state.copied_tech = id;
            }
        }
    }
}

void wfightmn_add_gauge(u8 side, s32 value) {
    FightstgMember *member = &fightstg_battle.state.members[0][fightstg_battle.state.current[0]];
    s32 id = gamestate_data.funcs.get_party_member(fightstg_battle.state.current[0]);

    if (side != 0 && value != 0 && member->hp != 0 && member->blasted == 0) {
        records_state.gauges[id] += fightstg_rules.get_gauge_gain(value);
        if (records_state.gauges[id] >= 1000) {
            records_state.gauges[id] = 1000;
            fightstg_events_add_gauge_full();
        }
    }
}

/* Per kind fightstg_action.effects[2..12]: the script's two values (wfightmn_tech_script_create). */
typedef struct WfightmnActionEffect {
    /* 0x0 */ s32 effect;
    /* 0x4 */ s32 hit_sound;
} WfightmnActionEffect; /* size 0x8 */

extern WfightmnActionEffect wfightmn_action_effects[11];

/* Creates the script object (fightstg_script_create) for `side` using technique `id` on the other side's acting
 * member: which script and stage it plays (script, stage, effect, hit_sound), each hit's outcome (results) and the
 * value it deals, then passes the value on (wfightmn_update_idle_anim and friends). */
void *wfightmn_tech_script_create(u8 side, s32 id) {
    s32 s = side != 0;
    RecordsTechnique *tech = &records_techniques[id - 1];
    FightstgStats *atk = fightstg_rules.get_stats(side, 1, fightstg_battle.state.current[s]);
    FightstgStats *def = fightstg_rules.get_stats(0x10 - side, 0, fightstg_battle.state.current[1 - s]);
    FightstgScript *script = fightstg_script_create();
    FightstgMember *members;
    s32 value;
    s32 i;
    s32 n;
    s32 stage;

    script->side = side;
    if (side == 0) {
        if (tech->anim_script == 5) {
            if (atk->multi_hit != 0) {
                script->script = 8;
                script->effect = tech->anim_effect;
                script->hit_sound = tech->hit_sound;
            } else {
                for (i = 2; i < 13; i++) {
                    if (fightstg_action.effects[i] != 0) {
                        script->script = 6;
                        script->effect = wfightmn_action_effects[i - 2].effect;
                        script->hit_sound = wfightmn_action_effects[i - 2].hit_sound;
                        break;
                    }
                }
                if (script->script == 0) {
                    for (i = 0; i < 3; i++) {
                        if (atk->strong_types[i] >= 2 && atk->strong_types[i] == def->type) {
                            script->script = 6;
                            script->effect = tech->anim_effect;
                            script->hit_sound = tech->hit_sound;
                            break;
                        }
                    }
                }
                if (script->script == 0) {
                    script->script = tech->anim_script;
                    script->effect = tech->anim_effect;
                    script->hit_sound = tech->hit_sound;
                }
            }
        } else {
            if (tech->kind < 2 && tech->element == 2 && tech->anim_script == 6 && atk->multi_hit != 0) {
                script->script = 8;
            } else {
                script->script = tech->anim_script;
            }
            script->effect = tech->anim_effect;
            script->hit_sound = tech->hit_sound;
            if (tech->defense_stat >= 2 || (tech->strong_type >= 2 && tech->strong_type == def->type)) {
                script->stage = tech->anim_stage;
            } else {
                script->stage = -1;
            }
        }
        if (script->stage <= 0) {
            if (atk->attack_element >= 2) {
                if (tech->defense_stat >= 2) {
                    n = tech->defense_stat - 2;
                } else {
                    n = atk->attack_element - 2;
                }
                stage = n * 3 + 0x21;
                if (atk->attack_element_power >= 0x40) {
                    script->stage = stage + 1;
                } else {
                    script->stage = stage;
                }
            } else if (tech->strong_type < 2) {
                for (i = 0; i < 3; i++) {
                    if (atk->strong_types[i] == 2 && def->type == atk->strong_types[i]) {
                        script->stage = 0x35;
                        break;
                    }
                    if (atk->strong_types[i] == 10 && def->type == 10) {
                        script->stage = 0x36;
                        break;
                    }
                }
            }
        }
    } else {
        script->script = tech->anim_script;
        script->effect = tech->anim_effect;
        script->hit_sound = tech->hit_sound;
        if (tech->defense_stat >= 2 || (tech->strong_type >= 2 && tech->strong_type == def->type)) {
            script->stage = tech->anim_stage;
        } else {
            script->stage = -1;
        }
    }

    members = fightstg_battle.state.members[1 - s];
    if (id == 0x1B5) {
        value = 9999;
        script->results[3] = 1;
    } else if (tech->kind == 0x1F) {
        if ((s16)fightstg_action.hit_count != 0) {
            value = fightstg_action.damages[0] + fightstg_action.damages[1];
            if (members[fightstg_battle.state.current[1 - s]].hp - value <= 0) {
                if ((s16)fightstg_action.hit_count == 1) {
                    script->results[0] = 3;
                } else {
                    script->results[0] = 0;
                }
                script->results[3] = 2;
            } else {
                for (i = 0; i < 2; i++) {
                    if (fightstg_action.hits[i] != 0) {
                        script->results[i * 3] = 0;
                    } else {
                        script->results[i * 3] = 3;
                    }
                }
            }
        } else {
            value = 0;
            script->results[0] = 3;
            script->results[3] = 3;
        }
    } else if (fightstg_action.effects[9] != 0) {
        value = (s16)fightstg_action.hit_count * fightstg_action.damage;
        if (members[fightstg_battle.state.current[1 - s]].hp - value <= 0) {
            for (i = 0; i < (s16)fightstg_action.hit_count - 1; i++) {
                if (fightstg_action.hits[i] != 0) {
                    script->results[i] = 0;
                } else {
                    script->results[i] = 3;
                }
            }
            script->results[3] = 2;
        } else {
            for (i = 0; i < fightstg_action.strikes - 1; i++) {
                if (fightstg_action.hits[i] != 0) {
                    script->results[i] = 0;
                } else {
                    script->results[i] = 3;
                }
            }
            if (fightstg_action.hits[i] != 0) {
                script->results[3] = 1;
            } else {
                script->results[3] = 3;
            }
        }
    } else if (fightstg_action.effects[6] != 0) {
        script->results[3] = 2;
        value = 9999;
    } else if (tech->kind == 0x23 && fightstg_action.effects[0x23] != 0) {
        script->results[3] = 1;
        value = fightstg_action.damage;
    } else if (tech->element == 2 || tech->element == 3) {
        if (fightstg_action.hits[0] != 0) {
            if (members[fightstg_battle.state.current[1 - s]].hp - fightstg_action.damage <= 0) {
                script->results[3] = 2;
            } else {
                script->results[3] = 1;
            }
            value = fightstg_action.damage;
        } else {
            script->results[3] = 3;
            value = 0;
        }
    } else {
        switch (id) {
        case 0x64:
        case 0x177:
            value = -9999;
            break;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0x190:
            value = -fightstg_rules.get_heal(side, id);
            break;
        default:
            value = 0;
            break;
        }
    }

    if (tech->element == 2 || tech->element == 3) {
        wfightmn_note_copied_tech(side, id);
        wfightmn_count_boss_hits(side, value);
        wfightmn_update_idle_anim(0x10 - side, value);
        if (fightstg_action.effects[8] != 0) {
            wfightmn_update_idle_anim(side, -fightstg_action.drained);
        }
    } else {
        wfightmn_check_final_phase_end(side);
        wfightmn_update_idle_anim(side, value);
    }
    return script;
}

s32 wfightmn_update_idle_anim(u8 side, s32 amount) {
    WfightmnMainData *data;
    s32 s;
    FightstgMember *member;

    s = side >> 4;
    data = (WfightmnMainData *)heap_objects.find(0xC, -1, -1)->children;

    if (side == 0x10 && fightstg_battle.state.type == 6) {
        data->slots->set_idle_anim(data->slots, side, fightstg_battle.state.final_phase);
    } else {
        member = &fightstg_battle.state.members[s][fightstg_battle.state.current[s]];
        if (member->hp - amount > member->max_hp / 4) {
            data->slots->set_idle_anim(data->slots, side, 0);
            return 0;
        }
        data->slots->set_idle_anim(data->slots, side, 1);
    }
    return 1;
}

void wfightmn_count_boss_hits(u8 side, s32 value) {
    if (fightstg_battle.state.type == 6 && side == 0 && fightstg_battle.state.final_phase == 0 && value != 0) {
        if (++fightstg_battle.state.hits >= 3) {
            fightstg_events_start_final_phase();
            wfightmn_update_idle_anim(0x10, value);
        }
    }
}

void wfightmn_check_final_phase_end(u8 arg0) {
    if (fightstg_battle.state.type == 6 && arg0 == 0 && fightstg_battle.state.final_phase != 0) {
        fightstg_events_end_final_phase();
    }
}

/* Caps the damage `value` (dealt `count` times, or once for 0) dealt to `side` 0 in battle kinds 1 and 2 so the
 * member keeps an eleventh of its max HP (kind 3: none). `count` is an s32 here (include/wfightmn.h). */
s32 wfightmn_cap_damage(u8 side, s32 value, s32 count) {
    s32 other = side == 0;
    FightstgMember *member = &fightstg_battle.state.members[other][fightstg_battle.state.current[other]];
    s32 limit;
    s32 total;

    switch (fightstg_battle.state.type) {
    case 1:
    case 2:
        if (side == 0) {
            limit = (s16)(member->max_hp / 11);
            if (count != 0) {
                total = value * count;
                if (limit < member->hp) {
                    if (member->hp - total < limit) {
                        value = (member->hp - limit) / count;
                    }
                } else {
                    value = 0;
                }
            } else if (limit < member->hp) {
                if (member->hp - value < limit) {
                    value = member->hp - limit;
                }
            } else {
                value = 0;
            }
        }
        break;
    case 3:
        if (side == 0) {
            value = 0;
        }
        break;
    }
    return value;
}

/* The overlay's .data (DECISIONS "Data in C, split per object"): all of it is this file's. */
RECT wfightmn_screen_rect = { 0, 0, 320, 240 };
WfightmnKindEffect wfightmn_kind_effects[7] = {
    { 2, 19, 26 }, { 3, 20, 26 }, { 4, 21, 27 }, { 5, 22, 50 }, { 6, 26, 50 }, { 8, 28, 39 }, { -1, 37, 49 },
};
WfightmnStatEffect wfightmn_stat_effects[8] = {
    { 2, 5, 64, 34 }, { 3, 9, 45, 37 }, { 4, 11, 44, 40 }, { 5, 14, 29, 43 },
    { 6, 16, 44, 46 }, { 7, 65, 64, 49 }, { 8, 7, 64, 52 }, { -1, 0, 0, 0 },
};
void wfightmn_escape(WfightmnMain *obj, WfightmnMainData *data);
void wfightmn_knockout(WfightmnMain *obj, WfightmnMainData *data);
void wfightmn_boss_enters(WfightmnMain *obj, WfightmnMainData *data);
void (*wfightmn_handlers[27])(WfightmnMain *obj, WfightmnMainData *data) = {
    NULL,
    NULL,
    NULL,
    wfightmn_party_turn,
    wfightmn_digivolve,
    wfightmn_switch_member,
    wfightmn_team_tech,
    wfightmn_battle_end,
    wfightmn_escape,
    wfightmn_regen_end,
    wfightmn_regen,
    wfightmn_field_end,
    wfightmn_poison,
    wfightmn_status_end,
    wfightmn_status_end,
    wfightmn_status_end,
    wfightmn_modifier_end,
    wfightmn_confused_turn,
    wfightmn_seal_end,
    wfightmn_blast,
    wfightmn_blast_end,
    wfightmn_knockout,
    wfightmn_boost_end,
    wfightmn_revert_form,
    wfightmn_victory_wait,
    wfightmn_boss_enters,
    wfightmn_final_phase_end,
};
WfightmnActionEffect wfightmn_action_effects[11] = {
    { 19, 26 }, { 20, 26 }, { 21, 27 }, { 22, 50 }, { 26, 50 }, { 0, 0 },
    { 28, 39 }, { 0, 0 }, { 46, 30 }, { 0, 59 }, { 31, 58 },
};
