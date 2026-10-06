// The settings directory and the settings file (docs/LAUNCHER.md "Settings directory", "Settings file"): the contract with the game, which
// reads the same file through `dw2003 --config <dir>/settings.json`. Paths inside the file are relative to its
// directory (or absolute).
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "input.h"
#include "json_value.h"

namespace dw3 {

constexpr int SETTINGS_SCHEMA = 1;
constexpr const char *SETTINGS_FILE = "settings.json";
constexpr const char *SETTINGS_PORTABLE_FILE = "portable.txt";
constexpr const char *SETTINGS_DIR_ENV = "DW3_CONFIG_DIR";
// SDL_GetPrefPath's names: "" (no organisation level) and the application's directory name, so the per-user
// directory is ~/.local/share/<app>/ on Linux and %APPDATA%\<app>\ on Windows.
constexpr const char *SETTINGS_PREF_ORG = "";
constexpr const char *SETTINGS_PREF_APP = "dw2003";

// ---- Where the settings live, in this order.
enum class DirSource {
    Argument,    // 1. --config-dir DIR
    Environment, // 1. $DW3_CONFIG_DIR
    Portable,    // 2. an (empty) portable.txt beside the executable: the executable's directory
    CurrentDir,  // 3. a settings.json that already exists in the current directory (never created there implicitly)
    User,        // 4. the per-user directory (SDL_GetPrefPath)
};
const char *dir_source_name(DirSource s);

struct DirLookup {
    std::string arg;     // --config-dir, or ""
    std::string env;     // $DW3_CONFIG_DIR, or ""
    std::string exe_dir; // SDL_GetBasePath()
    std::string cwd;     // SDL_GetCurrentDirectory()
    std::function<std::string()> user_dir; // called only when the first three do not apply (it creates the dir)
};

struct SettingsDir {
    std::string dir; // no trailing separator; "" when none could be found
    DirSource source = DirSource::User;
    std::string error;
};

// The real inputs: SDL's base path, the current directory, the environment, SDL_GetPrefPath.
DirLookup dir_lookup_from_system(const std::string &arg);
SettingsDir settings_dir_choose(const DirLookup &in);

// ---- The settings file (schema 1, settled by the game's side): the values the launcher edits. The rest of the file is kept as it
// was; the defaults are the game's (port/src/settings.c), so a missing member means the same on both sides.
struct MemoryCard {
    bool present = true; // false: null in the file, no card in the slot
    std::string path;    // a .mcd image, relative to the settings directory (created formatted by the game)
};

struct Settings {
    std::string disc_path; // "" = unset
    std::string disc_sha1; // the SHA-1 the launcher verified for disc_path ("" = not verified)
    int scale = 2;         // the window: 320*scale x 240*scale (1..16)
    bool fullscreen = false;
    int refresh = 50; // 50 (PAL) or 60 (the game's own 60 Hz mode; docs/LAUNCHER.md "50/60 Hz")
    bool mute = false;
    MemoryCard memcard[2] = { { true, "card1.mcd" }, { true, "card2.mcd" } };
    std::string last_dir; // launcher.last_dir: where the file dialog opens (the launcher's own state)
    // input: only what the file sets; a button or action that is absent keeps the game's default (input.h), so a
    // changed default in a later game reaches every user who never rebound it.
    std::map<std::string, std::vector<std::string>> keyboard; // input.keyboard: button -> key names
    std::map<std::string, std::vector<std::string>> gamepad;  // input.gamepad: button -> gamepad input names
    std::map<std::string, Binding> hotkeys;                   // input.hotkeys: action -> binding

    // The effective values (the file's, else the defaults).
    std::vector<std::string> keys_for(const std::string &button) const;
    std::vector<std::string> pad_for(const std::string &button) const;
    Binding hotkey_for(const std::string &action) const;
};

// Reads the known members of `doc` into a Settings (defaults for missing ones; a warning for each invalid one).
Settings settings_from_json(const Json &doc, std::vector<std::string> *warnings);
// Writes `s` into `doc`, keeping every other member and the members' order.
void settings_to_json(const Settings &s, Json *doc);

class SettingsFile {
public:
    enum class State {
        New,    // no file yet: defaults, written at the first save
        Loaded, // read
        Broken, // not JSON (or not an object): defaults; the first save keeps the old file as settings.json.broken
        Newer,  // a newer schema than this launcher's: shown, never written
    };

    // Reads <dir>/settings.json. Never fails: problems end up in state() and messages().
    void load(const std::string &dir);
    // Writes the file when its text changed (creating the directory). False with the reason in `err`.
    bool save(std::string *err);

    const std::string &dir() const { return dir_; }
    const std::string &path() const { return path_; }
    State state() const { return state_; }
    bool writable() const { return state_ != State::Newer; }
    const std::vector<std::string> &messages() const { return messages_; }
    // A path from the file (relative to its directory) as an absolute one.
    std::string resolve(const std::string &p) const;

    Settings values;
    Json doc; // the file as read; `values` are written over it at save

private:
    std::string dir_, path_, written_;
    State state_ = State::New;
    std::vector<std::string> messages_;
};

} // namespace dw3
