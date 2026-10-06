-- Layer-1 oracle, run inside PCSX-Redux (-no-ui -debugger -interpreter, driven by tests/golden/oracle.py). It boots the
-- real game until the code under test is resident, then calls the game's own functions with fixtures written into RAM
-- and records what they return and write. That is the golden: the original code, compiled by the original compiler,
-- running on the emulated R3000, resolving every piece of UB the way the console did.
--
-- How a call works (DECISIONS "Layer-1 goldens: calls on the running game"): an exec breakpoint on a hook function the main loop calls every
-- frame (pad_update) stops the game at a known point outside any interrupt handler. There the registers are saved,
-- the job's fixture bytes are written (the overwritten bytes are kept for the restore), a0..a3 are set, ra is pointed
-- at a sentinel address that is never executed (0x80010000, inn's .rodata) and pc at the function. The CPU runs the
-- function on the game's own stack; the exec breakpoint on the sentinel fires when it returns, and v0/v1 plus the
-- requested memory are read. Calls chain from the sentinel; after a job's calls its saved regions are restored, so
-- jobs are independent. The breakpoints need the interpreter: the dynarec does not check them.
--
-- Input (DW3_ORACLE_JOBS, a Lua chunk the driver writes):
--   return { hook = 0x8001851C, sentinel = 0x80010000, resident = { stage = 22, word0 = 0x80083DC0 },
--            overlays = { { addr = 0x80082CB0, file = '/path/FIGHTSTG.PRO', bss = {addr, size} } },  -- optional
--            jobs = { { name = 'x', saves = {{addr=, size=}}, writes = {{addr=, hex=} | {addr=, file=}}, keep = false,
--                       calls = {{ func =, args = {a0, a1, a2, a3}, reads = {{addr=, size=}} }} } } }
-- Output: DW3_ORACLE_OUT/results.json: per job, per call: v0, v1 (u32) and the reads (hex).
local ffi = require('ffi')
io.stdout:setvbuf('line')
local mem = PCSX.getMemPtr()
local function ptr(addr) return mem + bit.band(addr, 0x1FFFFF) end
local function u32(addr) return tonumber(ffi.cast('uint32_t*', ptr(addr))[0]) end
local function s32(addr) return ffi.cast('int32_t*', ptr(addr))[0] end
local function read_hex(addr, size)
    local t = {}
    for i = 0, size - 1 do t[#t + 1] = string.format('%02x', ffi.cast('uint8_t*', ptr(addr + i))[0]) end
    return table.concat(t)
end
local function write_hex(addr, hex)
    for i = 0, #hex / 2 - 1 do
        ffi.cast('uint8_t*', ptr(addr + i))[0] = tonumber(hex:sub(2 * i + 1, 2 * i + 2), 16)
    end
end

local out_dir = assert(os.getenv('DW3_ORACLE_OUT'), 'DW3_ORACLE_OUT not set')
local spec = dofile(assert(os.getenv('DW3_ORACLE_JOBS'), 'DW3_ORACLE_JOBS not set'))
local verbose = (os.getenv('DW3_ORACLE_VERBOSE') or '') ~= ''
local HOOK = spec.hook or 0x8001851C          -- pad_update: called once per main-loop iteration (src/main/main.c)
local SENTINEL = spec.sentinel or 0x80010000  -- inn's .rodata: never executed
local OVERLAY_MODULE = 0x80055D28
local OVERLAY_SLOT = 0x80082CB0
local max_boot_frames = spec.max_boot_frames or 3000
local call_timeout_frames = spec.call_timeout_frames or 600

-- Minimal JSON writer (numbers, strings, booleans, arrays, string-keyed tables).
local function is_array(t) local n = 0 for _ in pairs(t) do n = n + 1 end return n == #t end
local function json(v, indent)
    indent = indent or ''
    local t = type(v)
    if t == 'number' then return string.format(math.floor(v) == v and '%d' or '%.17g', v)
    elseif t == 'string' then return '"' .. v:gsub('[%c"\\]', function(c) return string.format('\\u%04x', c:byte()) end) .. '"'
    elseif t == 'boolean' then return tostring(v)
    elseif t == 'nil' then return 'null'
    elseif t == 'table' then
        local inner, parts = indent .. '  ', {}
        if is_array(v) then
            if #v == 0 then return '[]' end
            for _, x in ipairs(v) do parts[#parts + 1] = inner .. json(x, inner) end
            return '[\n' .. table.concat(parts, ',\n') .. '\n' .. indent .. ']'
        end
        local keys = {}
        for k in pairs(v) do keys[#keys + 1] = tostring(k) end
        table.sort(keys)
        for _, k in ipairs(keys) do parts[#parts + 1] = inner .. json(k) .. ': ' .. json(v[k], inner) end
        return '{\n' .. table.concat(parts, ',\n') .. '\n' .. indent .. '}'
    end
    error('cannot serialise a ' .. t)
end

local results = { status = 'running', jobs = {} }
local frame = 0
local function finish(status, message)
    results.status = status
    results.message = message
    results.frames = frame
    local f = assert(io.open(out_dir .. '/results.json', 'wb'))
    f:write(json(results), '\n')
    f:close()
    print('oracle: ' .. status .. ' ' .. (message or ''))
    PCSX.quit(status == 'ok' and 0 or 1)
end

-- Job state machine, driven by the hook and sentinel breakpoints.
local job_index, call_index = 0, 0
local saved = {}            -- {addr, hex} regions to restore at the end of the current job
local call_started_frame = nil
local hook_bp, sentinel_bp

local function restore_saved()
    for i = #saved, 1, -1 do write_hex(saved[i].addr, saved[i].hex) end
    saved = {}
end

-- Applies one write ({addr, hex} or {addr, file}: the file's bytes), keeping the overwritten bytes for the restore
-- unless the job is kept.
local function apply_write(w, keep)
    local data
    if w.file then
        local f = assert(io.open(w.file, 'rb'), 'cannot open ' .. w.file)
        data = f:read('*a')
        f:close()
        -- Whole sectors, as the game loads them (cdload reads sectors, overlay_load_stage copies get_sectors << 11
        -- bytes): the rest of the last sector is zeros on the disc, so a table at a file's end is followed by zeros,
        -- not by whatever the previous overlay left in the slot (stfgtrep_resist_gains, tests/host/FINDINGS.md 2).
        data = data .. string.rep('\0', (-#data) % 2048)
    end
    local size = data and #data or #w.hex / 2
    if not keep then saved[#saved + 1] = { addr = w.addr, hex = read_hex(w.addr, size) } end
    if data then
        ffi.copy(ptr(w.addr), data, #data)
        -- A file write is code (an overlay into the slot): the emulated instruction cache may still hold the previous
        -- overlay's instructions at the same addresses (the game itself flushes it after a load), so drop it.
        PCSX.invalidateCache()
    else
        write_hex(w.addr, w.hex)
    end
end

local function start_call(call)
    local job = spec.jobs[job_index]
    for _, w in ipairs(call.writes or {}) do apply_write(w, job.keep) end
    local regs = PCSX.getRegisters()
    local args = call.args or {}
    regs.GPR.n.a0 = args[1] or 0
    regs.GPR.n.a1 = args[2] or 0
    regs.GPR.n.a2 = args[3] or 0
    regs.GPR.n.a3 = args[4] or 0
    regs.GPR.n.ra = SENTINEL
    regs.pc = call.func
    -- tools/coverage.lua (only loaded by tools/coverage.py) re-checks the overlay slots after the writes
    if DW3_COVERAGE_SYNC then DW3_COVERAGE_SYNC(job.name, call.func) end
    call_started_frame = frame
    if verbose then print(string.format('oracle: call %s(%08x, %08x, %08x, %08x)', call.name or string.format('%08x', call.func),
        regs.GPR.n.a0, regs.GPR.n.a1, regs.GPR.n.a2, regs.GPR.n.a3)) end
end

local function start_job(job)
    results.jobs[#results.jobs + 1] = { name = job.name, calls = {} }
    if not job.keep then
        for _, s in ipairs(job.saves or {}) do saved[#saved + 1] = { addr = s.addr, hex = read_hex(s.addr, s.size) } end
    end
    for _, w in ipairs(job.writes or {}) do apply_write(w, job.keep) end
    if verbose then print('oracle: job ' .. job_index .. ' ' .. (job.name or '?') .. ' (' .. #(job.calls or {}) .. ' calls)') end
end

-- Advances to the next call (or the next job) and starts it; finishes when no job is left.
local function next_call()
    while true do
        local job = spec.jobs[job_index]
        if job and call_index < #(job.calls or {}) then
            call_index = call_index + 1
            start_call(job.calls[call_index])
            return
        end
        if job then
            if job.keep then saved = {} else restore_saved() end   -- a kept job (a setup: an overlay, a data file) stays
        end
        job_index = job_index + 1
        call_index = 0
        if job_index > #spec.jobs then
            finish('ok', #spec.jobs .. ' jobs')
            return
        end
        start_job(spec.jobs[job_index])
    end
end

local function on_sentinel()
    local regs = PCSX.getRegisters()
    local job = spec.jobs[job_index]
    local call = job.calls[call_index]
    local rec = { v0 = tonumber(regs.GPR.n.v0), v1 = tonumber(regs.GPR.n.v1), reads = {} }
    for _, r in ipairs(call.reads or {}) do rec.reads[#rec.reads + 1] = read_hex(r.addr, r.size) end
    local jr = results.jobs[#results.jobs]
    jr.calls[#jr.calls + 1] = rec
    if verbose then print(string.format('oracle:   -> v0=%08x v1=%08x', rec.v0, rec.v1)) end
    next_call()
    return true
end

local function on_hook()
    hook_bp:disable()
    local regs = PCSX.getRegisters()
    if verbose then print(string.format('oracle: hooked at %08x (frame %d, sp=%08x, gp=%08x)', regs.pc, frame, regs.GPR.n.sp, regs.GPR.n.gp)) end
    sentinel_bp = PCSX.addBreakpoint(SENTINEL, 'Exec', 4, 'oracle sentinel', on_sentinel)
    next_call()
    return true
end

local function load_overlays()
    for _, ov in ipairs(spec.overlays or {}) do
        local f = assert(io.open(ov.file, 'rb'))
        local data = f:read('*a')
        f:close()
        ffi.copy(ptr(ov.addr), data, #data)
        if ov.bss then ffi.fill(ptr(ov.bss.addr), ov.bss.size, 0) end
        if verbose then print(string.format('oracle: loaded %s (%d bytes) at %08x', ov.file, #data, ov.addr)) end
    end
end

local resident = false
DW3_ORACLE_LISTENER = PCSX.Events.createEventListener('GPU::Vsync', function()
    frame = frame + 1
    if not resident then
        local r = spec.resident or { stage = 22, word0 = 0x80083DC0 }
        if s32(OVERLAY_MODULE) == r.stage and (r.word0 == nil or u32(OVERLAY_SLOT) == r.word0) then
            resident = true
            results.resident_frame = frame
            load_overlays()
            hook_bp = PCSX.addBreakpoint(HOOK, 'Exec', 4, 'oracle hook', on_hook)
            if verbose then print('oracle: resident at frame ' .. frame .. ', hook set') end
        elseif frame > max_boot_frames then
            finish('fail', 'code not resident after ' .. frame .. ' frames')
        end
    elseif call_started_frame and frame - call_started_frame > call_timeout_frames then
        local job = spec.jobs[job_index]
        finish('fail', string.format('job %s call %d did not return within %d frames (pc=%08x)', job and job.name or '?',
            call_index, call_timeout_frames, PCSX.getRegisters().pc))
    end
end)
print('oracle: start (' .. #spec.jobs .. ' jobs)')
