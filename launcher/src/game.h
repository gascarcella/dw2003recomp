// Starting the game (docs/LAUNCHER.md "Contract"): `dw2003 --config <dir>/settings.json` through SDL_CreateProcess, its
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
        NoConfig,    // the game predates --config (its usage, without "--config"): too old for the launcher
        Failed,      // it could not be run, or ended otherwise: `message`
    } result = Result::Failed;
    std::string message;
};
GameProbe game_probe(const std::string &game, const std::string &settings_path);

// The command line: `game --config FILE --crash-dir <dir>/crashes`.
std::vector<std::string> game_args(const std::string &game, const SettingsFile &settings);
// The game's crash directory and the launcher's log directory under the settings directory (docs/LAUNCHER.md
// "Crash report").
std::string game_crash_dir(const std::string &settings_dir);
std::string game_log_dir(const std::string &settings_dir);
constexpr const char *GAME_LOG_FILE = "last-run.log";
constexpr const char *GAME_LOG_FILE_PREVIOUS = "last-run.1.log";
// The line the game prints last when it wrote a crash report: "port: crash report: PATH".
constexpr const char *GAME_REPORT_LINE = "port: crash report: ";

// A text form of an exit status (SDL_WaitProcess's: negative = killed by that signal).
std::string game_exit_text(int code);

class GameRun {
public:
    ~GameRun();
    // Starts `args` in `working_dir`; with `echo`, the game's output is copied to the launcher's stderr too; with
    // `log_path`, the whole output is streamed to that file as well (truncated first). False with the reason in `err`.
    bool start(const std::vector<std::string> &args, const std::string &working_dir, std::string *err,
               bool echo = true, const std::string &log_path = "");
    // Reads what the game wrote and checks whether it ended; call once a frame. True while it runs.
    bool poll();
    bool running() const { return proc_ != nullptr; }
    int exit_code() const { return exit_code_; }
    // The last lines of its output (at most kept_lines), oldest first.
    const std::deque<std::string> &lines() const { return lines_; }
    std::string command() const { return command_; }
    // The crash report the game named in its output (GAME_REPORT_LINE), "" when none.
    const std::string &report_path() const { return report_path_; }
    // The game's version line ("port: version ..."), "" when it printed none.
    const std::string &version() const { return version_; }
    const std::string &log_path() const { return log_path_; }

    static constexpr size_t kept_lines = 200;

private:
    void read_output();
    void add_text(const char *data, size_t n);

    SDL_Process *proc_ = nullptr;
    int exit_code_ = 0;
    bool echo_ = true;
    std::string partial_, command_, report_path_, version_, log_path_;
    SDL_IOStream *log_ = nullptr;
    std::deque<std::string> lines_;
};

} // namespace dw3
