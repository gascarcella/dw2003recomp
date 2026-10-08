-- This game's probes for psxstack's replay runner (psxstack/tools/replay/run.lua and boot_check.lua; GAME_CONTRACT.md
-- "6. Tests"): the same state the port's adapter reports (port/game/state.c: game_state_stage/file/map/random_index/
-- player_pos, the checkpoint image), read from the emulated RAM. Loaded with PSXSTACK_REPLAY, the runner's memory
-- accessors, in scope.
--
-- Memory (config/symbol_addrs.txt, include/gamestate.h, include/overlay.h, include/pad.h):
local GAMESTATE_DATA = 0x80048D34       -- gamestate_data: the pointer-free save struct
local GAMESTATE_SIZE = 0x275C
local GAMESTATE_MAP = GAMESTATE_DATA + 0x26C4  -- .map (s32); the high byte picks the overlay (overlay_files)
local OVERLAY_MODULE = 0x80055D28       -- OverlayModule { s32 stage; s32 file; ... }
local OVERLAY_SLOT = 0x80082CB0         -- main_overlay_base: where the tier-1 overlays load (game.json's slot 1)
local PAD_RANDOM_INDEX = 0x8004DC04     -- pad_random.index (s32)
local PAD_HELD = 0x8004B7D0 + 0x48 + 6  -- pad_state.slots[0][0].held (u16, active high): what the game saw
local PAD_PRESSED = 0x8004B7D0 + 0x48   -- pad_state.slots[0][0].pressed (u16): the edges the game saw this frame
local HEAP_OBJECTS = 0x8004B610         -- heap_objects.objects[100] (include/heap.h): the field's player actor is the
                                        -- object of kind 5 and key2 0 (heap_objects.find(5, -1, 0) in FIELDSTG)
local ACTOR_POS = 0x50                  -- FieldstgActor.pos (GamestatePos, 24.8 fixed point; include/fieldstg.h)
local CNTY_SEL_STAGE = 22               -- overlay_files[22] = CNTY_SEL.PRO (src/main/overlay.c): the first screen
local CNTY_SEL_WORD0 = 0x80083DC0       -- its first word in the slot

local R = PSXSTACK_REPLAY
local u16, u32, s32 = R.u16, R.u32, R.s32

return {
    image_addr = GAMESTATE_DATA,
    image_size = GAMESTATE_SIZE,
    stage = function() return s32(OVERLAY_MODULE) end,
    file = function() return s32(OVERLAY_MODULE + 4) end,
    map = function() return u32(GAMESTATE_MAP) end,
    random_index = function() return s32(PAD_RANDOM_INDEX) end,
    pad_held = function() return u16(PAD_HELD) end,
    pad_pressed = function() return u16(PAD_PRESSED) end,
    -- The field player's position in pixels (x, y), or nil outside the field / before the actor exists.
    player_pos = function()
        for i = 0, 99 do
            local obj = u32(HEAP_OBJECTS + i * 4)
            if obj ~= 0 and s32(obj) == 5 and s32(obj + 8) == 0 then
                return s32(obj + ACTOR_POS) / 256, s32(obj + ACTOR_POS + 4) / 256
            end
        end
        return nil
    end,
    -- The boot check (scripts/check_emulator.sh): the EXE has loaded CNTY_SEL, the country-select overlay (the
    -- retail BIOS needs ~1100 frames, OpenBIOS ~900).
    booted = function() return s32(OVERLAY_MODULE) == CNTY_SEL_STAGE and u32(OVERLAY_SLOT) == CNTY_SEL_WORD0 end,
}
