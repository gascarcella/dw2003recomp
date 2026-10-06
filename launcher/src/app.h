// The launcher's window: Dear ImGui over SDL3 + SDL_Renderer (LAUNCHER_MODS_PLAN 4.6). The screens are drawn into
// one full-window ImGui window, so they could later be drawn inside the game's window too (4.1).
#pragma once

#include <string>

#include <SDL3/SDL.h>

#include "settings.h"

namespace dw3 {

enum class Screen { Play, Disc, Settings, Controls, Mods, Count };
const char *screen_name(Screen s);

// SDL video, the window, the renderer and ImGui's context and backends. False with the reason in `err`.
bool gui_open(SDL_Window **window, SDL_Renderer **renderer, std::string *err);
void gui_close(SDL_Window *window, SDL_Renderer *renderer);

class App {
public:
    App(SDL_Window *window, SDL_Renderer *renderer, const SettingsDir &location);

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

private:
    void draw();
    void draw_nav();
    void draw_status_bar();
    void draw_play();
    void draw_disc();
    void draw_settings();
    void draw_placeholder(const char *what);
    void step_screen(int delta);

    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SettingsDir location_;
    SettingsFile settings_;
    Screen screen_ = Screen::Play;
    bool dirty_ = false;
    bool quit_ = false;
    std::string error_; // the last save error, shown in the status bar
};

} // namespace dw3
