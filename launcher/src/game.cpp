#include "game.h"

#include "paths.h"

namespace dw3 {

#ifdef SDL_PLATFORM_WINDOWS
static const char GAME_EXE[] = "dw2003.exe";
#else
static const char GAME_EXE[] = "dw2003";
#endif

std::string game_find(const std::string &explicit_path, const std::string &exe_dir, std::vector<std::string> *tried) {
    std::vector<std::string> candidates;
    if (!explicit_path.empty()) {
        candidates.push_back(explicit_path); // --game: only that one
    } else {
        if (const char *env = SDL_getenv("DW3_GAME")) {
            if (*env != '\0') {
                candidates.push_back(env);
            }
        }
        candidates.push_back(path_join(exe_dir, GAME_EXE));
        candidates.push_back(path_join(path_join(path_dir(exe_dir), "port-sdl"), GAME_EXE));
    }
    for (const std::string &c : candidates) {
        if (tried != nullptr) {
            tried->push_back(c);
        }
        if (path_is_file(c)) {
            return c;
        }
    }
    return "";
}

// Runs `args` to its end; its output (stdout and stderr) in `out`. False when it could not be started.
static bool run_to_end(const std::vector<std::string> &args, std::string *out, int *code, std::string *err) {
    std::vector<const char *> argv;
    for (const std::string &a : args) {
        argv.push_back(a.c_str());
    }
    argv.push_back(nullptr);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)argv.data());
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER, SDL_PROCESS_STDIO_NULL);
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
    SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_TO_STDOUT_BOOLEAN, true);
    SDL_Process *p = SDL_CreateProcessWithProperties(props);
    SDL_DestroyProperties(props);
    if (p == nullptr) {
        *err = std::string("cannot run ") + args[0] + ": " + SDL_GetError();
        return false;
    }
    size_t n = 0;
    void *data = SDL_ReadProcess(p, &n, code); // reads to the end, then waits for the exit
    if (data != nullptr) {
        out->assign((const char *)data, n);
        SDL_free(data);
    }
    SDL_DestroyProcess(p);
    return true;
}

GameProbe game_probe(const std::string &game, const std::string &settings_path) {
    GameProbe probe;
    std::string out;
    int code = -255;
    if (!run_to_end({ game, "--config", settings_path, "--print-settings" }, &out, &code, &probe.message)) {
        probe.result = GameProbe::Result::Failed;
        return probe;
    }
    if (code == 0) {
        probe.result = GameProbe::Result::Valid;
    } else if (code == 64 && out.compare(0, 6, "usage:") == 0 && out.find("--config") == std::string::npos) {
        probe.result = GameProbe::Result::NoConfig;
    } else if (code == 64) {
        probe.result = GameProbe::Result::Invalid;
        probe.message = out;
    } else {
        probe.result = GameProbe::Result::Failed;
        probe.message = "the settings check (--print-settings) " + game_exit_text(code) + "\n" + out;
    }
    while (!probe.message.empty() && (probe.message.back() == '\n' || probe.message.back() == '\r')) {
        probe.message.pop_back();
    }
    return probe;
}

std::vector<std::string> game_args(const std::string &game, const SettingsFile &settings) {
    return { game, "--config", settings.path() };
}

std::vector<std::string> game_args_interim(const std::string &game, const SettingsFile &settings) {
    const Settings &s = settings.values;
    std::vector<std::string> args = { game, "--window", "--scale", std::to_string(s.scale), "--watchdog", "0" };
    if (!s.disc_path.empty()) {
        args.insert(args.end(), { "--disc", settings.resolve(s.disc_path) });
    }
    if (s.fullscreen) {
        args.push_back("--fullscreen");
    }
    if (s.mute) {
        args.push_back("--mute");
    }
    for (int i = 0; i < 2; i++) {
        const MemoryCard &c = s.memcard[i];
        args.push_back(i == 0 ? "--memcard1" : "--memcard2");
        args.push_back(c.present ? settings.resolve(c.path) : "none");
    }
    return args;
}

std::string game_exit_text(int code) {
    switch (code) {
    case 0:
        return "ended normally";
    case 1:
        return "stopped on a fatal error (status 1)";
    case 2:
        return "halted (status 2: the game stopped itself)";
    case 3:
        return "stopped at an unimplemented part of the port (status 3)";
    case 4:
        return "was stopped by the watchdog (status 4: no frame for too long)";
    case 64:
        return "rejected its options or settings (status 64)";
    default:
        break;
    }
    if (code < 0 && code != -255) {
        const char *name = code == -11 ? " (a crash: SIGSEGV)" : code == -6 ? " (SIGABRT)" : code == -9 ? " (SIGKILL)" : "";
        return "was killed by signal " + std::to_string(-code) + name;
    }
    return "ended with status " + std::to_string(code);
}

// ---- the running game

GameRun::~GameRun() {
    if (proc_ != nullptr) {
        SDL_DestroyProcess(proc_); // leaves the game running: closing the launcher does not end it
    }
}

bool GameRun::start(const std::vector<std::string> &args, const std::string &working_dir, std::string *err,
                    bool echo) {
    if (proc_ != nullptr) {
        *err = "the game is already running";
        return false;
    }
    std::vector<const char *> argv;
    command_.clear();
    for (const std::string &a : args) {
        argv.push_back(a.c_str());
        command_ += (command_.empty() ? "" : " ") + a;
    }
    argv.push_back(nullptr);
    lines_.clear();
    partial_.clear();
    echo_ = echo;
    exit_code_ = 0;
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)argv.data());
    if (!working_dir.empty()) {
        SDL_SetStringProperty(props, SDL_PROP_PROCESS_CREATE_WORKING_DIRECTORY_STRING, working_dir.c_str());
    }
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER, SDL_PROCESS_STDIO_NULL);
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
    SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_TO_STDOUT_BOOLEAN, true);
    proc_ = SDL_CreateProcessWithProperties(props);
    SDL_DestroyProperties(props);
    if (proc_ == nullptr) {
        *err = std::string("cannot start ") + args[0] + ": " + SDL_GetError();
        return false;
    }
    return true;
}

void GameRun::add_text(const char *data, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (data[i] == '\n') {
            lines_.push_back(partial_);
            partial_.clear();
            if (lines_.size() > kept_lines) {
                lines_.pop_front();
            }
        } else if (data[i] != '\r') {
            partial_ += data[i];
        }
    }
}

void GameRun::read_output() {
    SDL_IOStream *out = SDL_GetProcessOutput(proc_);
    if (out == nullptr) {
        return;
    }
    char buf[4096];
    size_t n;
    // The pipe is non-blocking: a read returns 0 when nothing is waiting. Draining it keeps the game from blocking
    // on a full pipe.
    while ((n = SDL_ReadIO(out, buf, sizeof(buf))) > 0) {
        add_text(buf, n);
        if (echo_) {
            fwrite(buf, 1, n, stderr); // the game's log also reaches the launcher's terminal
        }
    }
}

bool GameRun::poll() {
    if (proc_ == nullptr) {
        return false;
    }
    read_output();
    int code = 0;
    if (!SDL_WaitProcess(proc_, false, &code)) {
        return true;
    }
    read_output(); // what it wrote before it ended
    if (!partial_.empty()) {
        add_text("\n", 1);
    }
    exit_code_ = code;
    SDL_DestroyProcess(proc_);
    proc_ = nullptr;
    return false;
}

} // namespace dw3
