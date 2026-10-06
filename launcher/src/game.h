// Starting the game (LAUNCHER_MODS_PLAN 4.1): `dw2003 --config <dir>/settings.json` through SDL_CreateProcess, its
// output (stdout and stderr together) read without blocking, the last lines kept for an error report.
#pragma once

#include <deque>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "settings.h"

namespace dw3 {

// The game's executable: `explicit_path` (--game) when given, else $DW3_GAME, else `dw2003` beside the launcher,
// else a development tree's SDL build (build/launcher/../port-sdl/dw2003). "" when none exists; `tried` lists the
// places looked at.
std::string game_find(const std::string &explicit_path, const std::string &exe_dir, std::vector<std::string> *tried);

// What `dw2003 --config FILE --print-settings` said about the settings file.
struct GameProbe {
    enum class Result {
        Valid,       // exit 0: the game reads the file as it is
        Invalid,     // exit 64 naming a key: `message` has the game's words
        NoConfig,    // the game predates --config (its usage, without "--config"): use game_args_interim
        Failed,      // it could not be run, or ended otherwise: `message`
    } result = Result::Failed;
    std::string message;
};
GameProbe game_probe(const std::string &game, const std::string &settings_path);

// The command line: `game --config FILE`.
std::vector<std::string> game_args(const std::string &game, const SettingsFile &settings);

// INTERIM, to delete at integration: once path B's --config is on main (pull request #10), every game build takes the
// file and game_probe never answers NoConfig. Until then the settings are translated into the options the game
// already has. video.refresh 60 is not passed (--fps 60 alone would run everything 20% fast; plan 3.5).
std::vector<std::string> game_args_interim(const std::string &game, const SettingsFile &settings);

// A text form of an exit status (SDL_WaitProcess's: negative = killed by that signal).
std::string game_exit_text(int code);

class GameRun {
public:
    ~GameRun();
    // Starts `args` in `working_dir`; with `echo`, the game's output is copied to the launcher's stderr too. False with
    // the reason in `err`.
    bool start(const std::vector<std::string> &args, const std::string &working_dir, std::string *err,
               bool echo = true);
    // Reads what the game wrote and checks whether it ended; call once a frame. True while it runs.
    bool poll();
    bool running() const { return proc_ != nullptr; }
    int exit_code() const { return exit_code_; }
    // The last lines of its output (at most kept_lines), oldest first.
    const std::deque<std::string> &lines() const { return lines_; }
    std::string command() const { return command_; }

    static constexpr size_t kept_lines = 200;

private:
    void read_output();
    void add_text(const char *data, size_t n);

    SDL_Process *proc_ = nullptr;
    int exit_code_ = 0;
    bool echo_ = true;
    std::string partial_, command_;
    std::deque<std::string> lines_;
};

} // namespace dw3
