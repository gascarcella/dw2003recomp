-- The NTSC/60 Hz patch of the EU disc in the emulator, without a patched image (docs/LAUNCHER.md "50/60 Hz"): the
-- patch is two data words of SLES_039.36 (DECISIONS session 1): records_60hz at 0x8005CCAC, 0 -> 1 (SetVideoMode(NTSC),
-- the time step, LIBSND's tick, actor speeds), and main_screen_pos at 0x8005CCB0, 1 -> 0 (the NTSC screen offset and the
-- card game's NTSC layout). They are written at main's first instruction (an exec breakpoint, after the EXE is loaded
-- and before main reads them), at every boot (a script's reset reloads the EXE).
-- Loaded before tests/replay/run.lua by `replay.py run --prelude` or `spu_trace.py run --prelude` (-debugger
-- -interpreter). DW3_NTSC_SCREEN_POS=1 keeps main_screen_pos at 1, as the port's 60 Hz setting does.
local ffi = require('ffi')
local C = ffi.load('PCSX')
local mem = PCSX.getMemPtr()

local MAIN = 0x80014524               -- main (config/symbol_addrs.txt)
local RECORDS_60HZ, SCREEN_POS = 0x8005CCAC, 0x8005CCB0
local screen_pos = tonumber(os.getenv('DW3_NTSC_SCREEN_POS') or '0')

local function word(addr) return ffi.cast('uint32_t*', mem + bit.band(addr, 0x1FFFFC)) end

DW3_NTSC_PATCH = ffi.cast('bool (*)(uint32_t, unsigned, const char *)', function(address, width, cause)
    -- only the game's own main: the EXE's two words still hold their unpatched values
    local v60, pos = word(RECORDS_60HZ)[0], word(SCREEN_POS)[0]
    if v60 == 0 and pos == 1 then
        word(RECORDS_60HZ)[0] = 1
        word(SCREEN_POS)[0] = screen_pos
        print(string.format('ntsc_patch: records_60hz = 1, main_screen_pos = %d', screen_pos))
    end
    return true
end)
DW3_NTSC_PATCH_BP = C.addBreakpoint(MAIN, 'Exec', 4, 'ntsc_patch', DW3_NTSC_PATCH, 'ntsc_patch')
