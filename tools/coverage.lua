-- Function-level execution coverage recorder, loaded into PCSX-Redux before a test runner (tools/coverage.py writes a
-- wrapper chunk that does dofile(this) and then dofile(the runner: tests/golden/oracle.lua or psxstack/tools/replay/run.lua)).
-- Needs -debugger -interpreter: exec breakpoints never fire under the dynarec (DECISIONS "Layer-1 goldens: calls on the running game").
--
-- Input (DW3_COVERAGE_SPEC, a Lua chunk tools/coverage.py writes):
--   return { exe = { 0x80010f4c, ... },                       -- every EXE function start
--            slots = { { name = 't1', cands = { { key = 'FIELDSTG', path = '/.../build/FIELDSTG.PRO', base = 0x80082CB0,
--                                                 lo = 0x80083784, hi = 0x80092DFC, funcs = { ... } }, ... } }, ... },
--            sync = { 0x80020CFC, 0x80020D78 },               -- right after overlay_load_stage/_file's memcpy
--            copy = { 0x8002514C, 0x80025180 } }              -- memcpy itself
-- Output (DW3_COVERAGE_OUT, written when the runner calls PCSX.quit): one line per first hit,
--   "hit <key> <addr hex> <frame> <stage> <ctx>", plus "res <slot> <key|-> <frame> <how>" at every residency change.
--
-- Mechanism: one exec breakpoint per function start that has not been hit yet; the shared invoker records the hit and
-- returns false, which makes Redux delete the breakpoint, so a function costs one Lua call per run. The two overlay slots
-- (tier 1 at 0x80082CB0, tier 2 at 0x800A5DE0) hold different files over time and the files share addresses: a slot's
-- breakpoints are those of its *resident* file only. The resident file is the candidate whose .text bytes (from the
-- matching build, byte-identical to the disc) equal RAM, longest .text first (a file whose code is a prefix of another's
-- must not win). Residency is re-checked right after the game's two overlay copies (the `sync` breakpoints), at every
-- vsync (a fallback, counted as "late" when it finds a change outside memcpy), and when the oracle applies a job's writes
-- (DW3_COVERAGE_SYNC, called by tests/golden/oracle.lua when it is defined; it also gets the function the oracle is
-- about to call, since a pc set from Lua may not pass the breakpoint check). At the oracle's first job every breakpoint
-- is armed again, so the hits with a job's context are exactly what the golden calls ran (the boot's have context '-').
local ffi = require('ffi')
pcall(ffi.cdef, 'int memcmp(const void *a, const void *b, size_t n);')
local C = ffi.load('PCSX')
local mem = PCSX.getMemPtr()
local function ptr(addr) return mem + bit.band(addr, 0x1FFFFF) end
local OVERLAY_MODULE = 0x80055D28

local spec = dofile(assert(os.getenv('DW3_COVERAGE_SPEC'), 'DW3_COVERAGE_SPEC not set'))
local out_path = assert(os.getenv('DW3_COVERAGE_OUT'), 'DW3_COVERAGE_OUT not set')

local frame = 0
local ctx = '-'
local lines = {}
local armed = {}      -- addr -> { key = candidate key, bp = Breakpoint* }
local hit = {}        -- key -> { [addr] = true }
local nhits, ncalls = 0, 0

local function stage() return ffi.cast('int32_t*', ptr(OVERLAY_MODULE))[0] end

local function record(key, addr)
    hit[key] = hit[key] or {}
    if hit[key][addr] then return end
    hit[key][addr] = true
    nhits = nhits + 1
    lines[#lines + 1] = string.format('hit %s %08x %d %d %s', key, addr, frame, stage(), ctx)
end

-- The invoker shared by every function breakpoint (one ffi callback: LuaJIT has a limited number of callback slots).
DW3_COVERAGE_CB = ffi.cast('bool (*)(uint32_t, unsigned, const char *)', function(addr, width, cause)
    ncalls = ncalls + 1
    local a = armed[addr]
    if a then
        armed[addr] = nil
        record(a.key, addr)
    end
    return false  -- Redux deletes the breakpoint
end)

-- Insertion order matters for Redux's breakpoint tree: sorted insertion measured 1.6x slower than shuffled.
local function shuffled(t)
    local s = {}
    for i, v in ipairs(t) do s[i] = v end
    local x = 12345
    for i = #s, 2, -1 do
        x = (x * 1103515245 + 12345) % 2147483648
        local j = x % i + 1
        s[i], s[j] = s[j], s[i]
    end
    return s
end

local function arm(key, funcs)
    local done = hit[key] or {}
    for _, addr in ipairs(funcs) do
        if not done[addr] and not armed[addr] then
            armed[addr] = { key = key, bp = C.addBreakpoint(addr, 'Exec', 4, 'cov', DW3_COVERAGE_CB, '') }
        end
    end
end

local function disarm(key, funcs)
    for _, addr in ipairs(funcs) do
        local a = armed[addr]
        if a and a.key == key then
            C.removeBreakpoint(a.bp)
            armed[addr] = nil
        end
    end
end

-- Candidate .text bytes, read once.
for _, slot in ipairs(spec.slots) do
    for _, c in ipairs(slot.cands) do
        local f = assert(io.open(c.path, 'rb'), 'cannot open ' .. c.path)
        local data = f:read('*a')
        f:close()
        c.text = data:sub(c.lo - c.base + 1, c.hi - c.base)
        assert(#c.text == c.hi - c.lo, 'short file ' .. c.path)
        c.funcs = shuffled(c.funcs)
    end
    table.sort(slot.cands, function(a, b) return #a.text > #b.text end)
end

local nsync, nlate = 0, 0
local function sync(how)
    nsync = nsync + 1
    for _, slot in ipairs(spec.slots) do
        local found = nil
        for _, c in ipairs(slot.cands) do
            if ffi.C.memcmp(ptr(c.lo), c.text, #c.text) == 0 then found = c break end
        end
        if found ~= slot.resident then
            if slot.resident then disarm(slot.resident.key, slot.resident.funcs) end
            slot.resident = found
            if found then arm(found.key, found.funcs) end
            if how == 'vsync' then
                -- A vblank inside the game's memcpy (the .text already copied, the rest still going) is not late:
                -- no overlay code runs during the copy.
                local pc = PCSX.getRegisters().pc
                if spec.copy and pc >= spec.copy[1] and pc < spec.copy[2] then how = 'copy' else nlate = nlate + 1 end
            end
            lines[#lines + 1] = string.format('res %s %s %d %s', slot.name, found and found.key or '-', frame, how)
        end
    end
end

-- Called by tests/golden/oracle.lua after a call's writes, before it sets pc: the job's name becomes the context of
-- later hits, and the called function is recorded here (the breakpoint check may not see a pc set from Lua).
local exe_funcs
function DW3_COVERAGE_SYNC(name, func)
    if ctx == '-' then
        -- The first job: what the boot ran is recorded (context '-'); from here on every function counts again, so
        -- the hits with a job's context are exactly what the jobs ran ("oracle_calls" in the report).
        hit = {}
        arm('SLES_039.36', exe_funcs)
        for _, slot in ipairs(spec.slots) do
            if slot.resident then arm(slot.resident.key, slot.resident.funcs) end
        end
    end
    ctx = name or ctx
    sync('oracle')
    if func then
        local a = armed[func]
        if a then
            C.removeBreakpoint(a.bp)
            armed[func] = nil
            record(a.key, func)
        end
    end
end

exe_funcs = shuffled(spec.exe)
arm('SLES_039.36', exe_funcs)
local debug = (os.getenv('DW3_COVERAGE_DEBUG') or '') ~= ''
DW3_COVERAGE_SYNC_CB = ffi.cast('bool (*)(uint32_t, unsigned, const char *)', function(addr)
    if debug then
        local r = PCSX.getRegisters()
        print(string.format('coverage: load sync at %08x frame %d stage %d a0=%08x slot word %08x', addr, frame, stage(),
            r.GPR.n.a0, ffi.cast('uint32_t*', ptr(0x80082CB0))[0]))
    end
    sync('load')
    return true
end)
for _, addr in ipairs(spec.sync or {}) do C.addBreakpoint(addr, 'Exec', 4, 'cov sync', DW3_COVERAGE_SYNC_CB, '') end

DW3_COVERAGE_LISTENER = PCSX.Events.createEventListener('GPU::Vsync', function()
    frame = frame + 1
    sync('vsync')
end)

local function write_out()
    local f = io.open(out_path, 'wb')
    if not f then return end
    f:write(string.format('stats frames=%d hits=%d invocations=%d syncs=%d late=%d\n', frame, nhits, ncalls, nsync, nlate))
    f:write(table.concat(lines, '\n'), '\n')
    f:close()
end

-- The runners end with PCSX.quit: write the hits first.
local quit = PCSX.quit
PCSX.quit = function(code)
    write_out()
    quit(code)
end
print(string.format('coverage: %d EXE functions armed, %d slot candidates', #spec.exe,
    (function() local n = 0 for _, s in ipairs(spec.slots) do n = n + #s.cands end return n end)()))
