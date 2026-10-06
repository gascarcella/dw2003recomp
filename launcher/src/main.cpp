// dw2003-launcher: finds the settings directory, edits settings.json and starts the game (LAUNCHER_MODS_PLAN 4.1,
// 4.6; launcher/README.md). Exit status: 0 closed (or the self-test passed), 1 the window could not be opened (or
// the self-test failed), 64 a usage error.
#include <cstdio>
#include <cstring>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "app.h"
#include "selftest.h"
#include "settings.h"

static void usage(const char *argv0) {
    std::fprintf(stderr,
                 "usage: %s [--config-dir DIR] [--game PATH] [--self-test DIR]\n"
                 "  --config-dir DIR  the settings directory (else $%s, portable.txt beside the launcher, a\n"
                 "                    settings.json in the current directory, the per-user directory)\n"
                 "  --game PATH       the game's executable (else $DW3_GAME, dw2003 beside the launcher,\n"
                 "                    ../port-sdl/dw2003 from the launcher's directory)\n"
                 "  --self-test DIR   run the self-test (it writes into DIR/launcher-self-test/, replacing it),\n"
                 "                    on any video driver (CI: SDL_VIDEO_DRIVER=offscreen); exit 0 = passed\n",
                 argv0, dw3::SETTINGS_DIR_ENV);
}

int main(int argc, char **argv) {
    // The self-test starts this executable as a stand-in for the game (selftest.cpp).
    if (const char *mode = SDL_getenv(dw3::SELF_TEST_GAME_ENV)) {
        return dw3::self_test_fake_game(mode, argc, argv);
    }
    std::string config_dir, self_test;
    dw3::AppOptions options;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--config-dir") == 0 && i + 1 < argc) {
            config_dir = argv[++i];
        } else if (std::strcmp(argv[i], "--game") == 0 && i + 1 < argc) {
            options.game = argv[++i];
        } else if (std::strcmp(argv[i], "--self-test") == 0 && i + 1 < argc) {
            self_test = argv[++i];
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 64;
        }
    }
    if (!self_test.empty()) {
        return dw3::self_test_run(self_test) ? 0 : 1;
    }

    dw3::SettingsDir location = dw3::settings_dir_choose(dw3::dir_lookup_from_system(config_dir));
    if (location.dir.empty()) {
        std::fprintf(stderr, "launcher: %s\n", location.error.c_str());
    } else {
        std::fprintf(stderr, "launcher: settings in %s (%s)\n", location.dir.c_str(),
                     dw3::dir_source_name(location.source));
    }
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::string err;
    if (!dw3::gui_open(&window, &renderer, &err)) {
        std::fprintf(stderr, "launcher: %s\n", err.c_str());
        return 1;
    }
    {
        dw3::App app(window, renderer, location, options);
        int busy = 0; // frames still drawn without waiting after an event (ImGui spreads a press and its release)
        while (!app.quit_requested()) {
            SDL_Event e;
            // Idle: wait for an event (up to a quarter of a second, so the frames keep coming for ImGui's timers; a
            // tenth while the disc check or the game runs, for the progress bar and the game's output).
            if (busy > 0 ? SDL_PollEvent(&e) : SDL_WaitEventTimeout(&e, app.busy() ? 100 : 250)) {
                busy = 4;
                do {
                    app.handle_event(e);
                } while (SDL_PollEvent(&e));
            } else if (busy > 0) {
                busy--;
            }
            app.frame();
        }
        app.flush();
    }
    dw3::gui_close(window, renderer);
    return 0;
}
