-- Boot check for scripts/check_emulator.sh, run inside PCSX-Redux (-dofile). Waits until the EXE has loaded the
-- country-select overlay CNTY_SEL (the first screen): overlay_module.stage (0x80055D28, include/overlay.h) == 22
-- (overlay_files[22] = CNTY_SEL.PRO, src/main/overlay.c) and the tier-1 slot at main_overlay_base (0x80082CB0) holds
-- CNTY_SEL's first word (0x80083DC0). Prints "boot check: OK ..." and exits 0, or "boot check: FAIL ..." and exits 1
-- after DW3_BOOT_FRAMES vsyncs (default 3000; the retail BIOS needs ~1100, OpenBIOS ~900).
local ffi = require('ffi')
local max_frames = tonumber(os.getenv('DW3_BOOT_FRAMES') or '') or 3000
local frames = 0
local mem = PCSX.getMemPtr()
local function u32(addr) return ffi.cast('uint32_t*', mem + bit.band(addr, 0x1FFFFF))[0] end
local function s32(addr) return ffi.cast('int32_t*', mem + bit.band(addr, 0x1FFFFF))[0] end

local OVERLAY_MODULE = 0x80055D28   -- OverlayModule { s32 stage; s32 file; ... }
local OVERLAY_SLOT = 0x80082CB0     -- main_overlay_base
local CNTY_SEL_STAGE = 22
local CNTY_SEL_WORD0 = 0x80083DC0

local function state()
    return string.format('frame=%d pc=%08x overlay.stage=%d overlay.file=%d slot[0]=%08x',
        frames, PCSX.getRegisters().pc, s32(OVERLAY_MODULE), s32(OVERLAY_MODULE + 4), u32(OVERLAY_SLOT))
end

PCSX.Events.createEventListener('GPU::Vsync', function()
    frames = frames + 1
    if s32(OVERLAY_MODULE) == CNTY_SEL_STAGE and u32(OVERLAY_SLOT) == CNTY_SEL_WORD0 then
        print('boot check: OK (CNTY_SEL loaded) ' .. state())
        PCSX.quit(0)
    elseif frames % 500 == 0 then
        print('boot check: ' .. state())
    end
    if frames >= max_frames then
        print('boot check: FAIL (CNTY_SEL not loaded after ' .. max_frames .. ' frames) ' .. state())
        PCSX.quit(1)
    end
end)
print('boot check: waiting for CNTY_SEL (up to ' .. max_frames .. ' frames)')
