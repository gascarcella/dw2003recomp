// The launcher's window: Dear ImGui over SDL3 + SDL_Renderer (docs/LAUNCHER.md "Launcher"). The screens are drawn into
// one full-window ImGui window, so they could later be drawn inside the game's window too.
#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "disc.h"
#include "game.h"
#include "mods.h"
#include "settings.h"

namespace dw3 {

enum class Screen { Play, Disc, Settings, Controls, Mods, Count };
const char *screen_name(Screen s);

// SDL video, the window, the renderer and ImGui's context and backends. False with the reason in `err`.
bool gui_open(SDL_Window **window, SDL_Renderer **renderer, std::string *err);
void gui_close(SDL_Window *window, SDL_Renderer *renderer);

// The disc in the settings, as the launcher sees it.
enum class DiscStatus {
    Unset,      // no disc.path
    Missing,    // the file (or the .cue's BIN) is not there
    Unverified, // there, but no matching disc.sha1, or the BIN's size changed: to check
    Checking,   // the check is running
    Verified,   // disc.sha1 is the EU disc's and the BIN has its size
};

struct AppOptions {
    std::string game;       // --game: the game's executable ("" = game_find's lookup)
    bool echo_game = true;  // copy the game's output to the launcher's stderr
};

class App {
public:
    // A binding somewhere in the settings (a hotkey, a mod's binding option): the conflict marks look at all of them.
    struct BindingUse {
        std::string key;   // "hotkey:<action>", "mod:<mod>.<option>"
        std::string label; // "the hotkey Pause", "Fast-forward: Hold"
        Binding binding;
    };

    App(SDL_Window *window, SDL_Renderer *renderer, const SettingsDir &location, const AppOptions &options = {});

    // Every SDL event goes through here (ImGui's backend first).
    void handle_event(const SDL_Event &e);
    // One frame: the UI, the render and the present. With `screenshot`, the frame is also saved there as a PNG
    // before the present (the self-test's pictures).
    void frame(const char *screenshot = nullptr);
    // Saves the settings now if they changed.
    void flush();

    bool quit_requested() const { return quit_; }
    Screen screen() const { return screen_; }
    void set_screen(Screen s) { screen_ = s; }
    SettingsFile &settings() { return settings_; }
    const std::string &last_error() const { return error_; }
    // Frames are wanted soon (a check or the game running): the main loop then waits less.
    bool busy() const { return check_.state() == DiscCheck::State::Running || run_.running(); }

    // The disc screen's actions (the self-test calls them too).
    void choose_disc(const std::string &path);
    DiscStatus disc_status() const;
    DiscCheck &disc_check() { return check_; }
    const std::string &disc_message() const { return disc_message_; }
    // The play button: settings saved, checked by the game, the game started (the launcher's window hidden).
    void play();
    const GameRun &game_run() const { return run_; }
    const std::string &play_error() const { return play_error_; }
    const std::string &game_path() const { return game_; }
    // The controls' prompt (the self-test drives it with events).
    InputCapture &capture() { return capture_; }
    void capture_for(int section, const std::string &id); // 0 keyboard, 1 gamepad (button ids), 2 hotkeys (actions)
    void capture_for_mod(const std::string &mod, const std::string &option);
    // The mods found beside the game.
    const std::vector<ModManifest> &mods() const { return mods_; }
    const std::string &mods_dir() const { return mods_dir_; }
    void select_mod(const std::string &id);

private:
    void draw();
    void draw_nav();
    void draw_status_bar();
    void draw_play();
    void draw_disc();
    void draw_settings();
    void draw_controls();
    void draw_names_table(bool pad);
    void draw_hotkeys();
    void draw_capture_popup();
    void draw_mods();
    void draw_mod(const ModManifest &m);
    bool draw_mod_option(const ModManifest &m, const ModOption &o, ModValues &values,
                         const std::vector<BindingUse> &uses);
    std::vector<BindingUse> binding_uses() const;
    // A binding's chips (x removes one) and its + button (want_capture_ set when pressed). True when changed.
    bool binding_editor(const std::string &key, Binding *b, const std::vector<BindingUse> &uses);
    const ModManifest *find_mod(const std::string &id) const;
    void begin_capture(InputCapture::Kind kind, int section, const std::string &id, const std::string &label);
    void finish_capture();
    void draw_play_error();
    void step_screen(int delta);
    void update_disc_check();
    void update_game();
    void open_file_dialog();
    static void SDLCALL file_dialog_done(void *self, const char *const *files, int filter);

    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SettingsDir location_;
    SettingsFile settings_;
    Screen screen_ = Screen::Play;
    bool dirty_ = false;
    bool quit_ = false;
    std::string error_; // the last save error, shown in the status bar

    // The disc: the check, its outcome, the typed path, the file dialog's answer (it may come from another thread).
    DiscCheck check_;
    std::string disc_message_;
    bool disc_message_ok_ = false;
    char disc_input_[1024] = "";
    std::mutex dialog_mutex_;
    bool dialog_open_ = false, dialog_done_ = false;
    std::string dialog_file_, dialog_error_;

    // The controls: the "press a key" prompt and what it is for (section 0 keyboard, 1 gamepad, 2 hotkeys).
    InputCapture capture_;
    int capture_section_ = 0;
    std::string capture_id_, capture_label_;
    bool capture_popup_ = false;  // the prompt is to be opened this frame
    bool pad_nav_hold_ = false;   // a gamepad press just went to a capture: no gamepad navigation until released
    bool want_capture_ = false;   // binding_editor's + was pressed
    std::string capture_option_;  // section 3 (a mod's binding option): capture_id_ is the mod

    // The mods: the manifests beside the game, the one shown.
    std::string mods_dir_;
    std::vector<ModManifest> mods_;
    std::string mod_selected_;

    // The game.
    std::string game_;                    // its executable ("" = not found)
    bool echo_game_ = true;
    std::vector<std::string> game_tried_; // where game_find looked
    GameRun run_;
    std::string play_error_; // why the last start failed, or how the game ended
    std::vector<std::string> play_log_; // the game's last lines when it ended with an error
};

} // namespace dw3
