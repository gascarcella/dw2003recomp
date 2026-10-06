#include "app.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <algorithm>
#include <cstdio>

#include "paths.h"

namespace dw3 {

static const char *const SCREEN_NAMES[] = { "Play", "Disc", "Settings", "Controls", "Mods" };
static_assert(sizeof(SCREEN_NAMES) / sizeof(SCREEN_NAMES[0]) == (size_t)Screen::Count, "one name per screen");

const char *screen_name(Screen s) {
    return (int)s >= 0 && s < Screen::Count ? SCREEN_NAMES[(int)s] : "?";
}

// ---- the look: dark panels, one warm accent.

static ImVec4 rgb(unsigned hex, float a = 1.0f) {
    return ImVec4(((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f, a);
}

static const unsigned ACCENT = 0xE8873A;
static const unsigned ERROR_RED = 0xE5534B;

// A two-column list of labels and values (values wrap).
static bool fields_begin(const char *id) {
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit)) {
        return false;
    }
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

static void field(const char *label, const std::string &value, const ImVec4 *color = nullptr) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", label);
    ImGui::TableNextColumn();
    if (color != nullptr) {
        ImGui::PushStyleColor(ImGuiCol_Text, *color);
    }
    ImGui::TextWrapped("%s", value.c_str());
    if (color != nullptr) {
        ImGui::PopStyleColor();
    }
}

// `prefix` + `text`, the text shortened from its start ("...") to fit the remaining width.
static void text_fit_left(const char *prefix, const std::string &text) {
    const float avail = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(prefix).x;
    size_t start = 0;
    while (start < text.size() && ImGui::CalcTextSize(("..." + text.substr(start)).c_str()).x > avail &&
           ImGui::CalcTextSize(text.c_str()).x > avail) {
        start++;
    }
    std::string shown = start == 0 ? text : "..." + text.substr(start);
    ImGui::TextDisabled("%s%s", prefix, shown.c_str());
}

static void gui_style(float scale) {
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);
    style.WindowRounding = 0;
    style.ChildRounding = 6;
    style.FrameRounding = 5;
    style.GrabRounding = 5;
    style.PopupRounding = 6;
    style.WindowPadding = ImVec2(16, 14);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.WindowBorderSize = 0;
    style.ChildBorderSize = 0;
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
    ImVec4 *c = style.Colors;
    c[ImGuiCol_Text] = rgb(0xE8E6E3);
    c[ImGuiCol_TextDisabled] = rgb(0x8A8F98);
    c[ImGuiCol_WindowBg] = rgb(0x15171C);
    c[ImGuiCol_ChildBg] = rgb(0x1C1F26);
    c[ImGuiCol_PopupBg] = rgb(0x22262E);
    c[ImGuiCol_Border] = rgb(0x2E333D);
    c[ImGuiCol_FrameBg] = rgb(0x262A33);
    c[ImGuiCol_FrameBgHovered] = rgb(0x2F3440);
    c[ImGuiCol_FrameBgActive] = rgb(0x363C4A);
    c[ImGuiCol_Button] = rgb(0x2A2F39);
    c[ImGuiCol_ButtonHovered] = rgb(0x353B47);
    c[ImGuiCol_ButtonActive] = rgb(ACCENT, 0.85f);
    c[ImGuiCol_Header] = rgb(ACCENT, 0.22f);
    c[ImGuiCol_HeaderHovered] = rgb(ACCENT, 0.32f);
    c[ImGuiCol_HeaderActive] = rgb(ACCENT, 0.45f);
    c[ImGuiCol_CheckMark] = rgb(ACCENT);
    c[ImGuiCol_SliderGrab] = rgb(ACCENT, 0.85f);
    c[ImGuiCol_SliderGrabActive] = rgb(ACCENT);
    c[ImGuiCol_Separator] = rgb(0x2E333D);
    c[ImGuiCol_NavCursor] = rgb(ACCENT);
    c[ImGuiCol_TextSelectedBg] = rgb(ACCENT, 0.35f);
    c[ImGuiCol_Tab] = rgb(0x262A33);
    c[ImGuiCol_TabHovered] = rgb(ACCENT, 0.45f);
    c[ImGuiCol_TabSelected] = rgb(ACCENT, 0.30f);
    c[ImGuiCol_TabSelectedOverline] = rgb(ACCENT);
    c[ImGuiCol_TabDimmed] = rgb(0x262A33);
    c[ImGuiCol_TabDimmedSelected] = rgb(ACCENT, 0.22f);
    c[ImGuiCol_TitleBg] = rgb(0x22262E);
    c[ImGuiCol_TitleBgActive] = rgb(0x2A2F39);
    c[ImGuiCol_ScrollbarGrab] = rgb(0x353B47);
    c[ImGuiCol_ScrollbarBg] = rgb(0x1C1F26);
    c[ImGuiCol_ModalWindowDimBg] = rgb(0x000000, 0.45f);
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    style.FontSizeBase = 17.0f;
}

bool gui_open(SDL_Window **window, SDL_Renderer **renderer, std::string *err) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        *err = std::string("SDL_Init: ") + SDL_GetError();
        return false;
    }
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (scale <= 0) {
        scale = 1;
    }
    *window = SDL_CreateWindow("Digimon World 2003 launcher", (int)(960 * scale), (int)(620 * scale),
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (*window == nullptr) {
        *err = std::string("SDL_CreateWindow: ") + SDL_GetError();
        return false;
    }
    *renderer = SDL_CreateRenderer(*window, nullptr);
    if (*renderer == nullptr) {
        *err = std::string("SDL_CreateRenderer: ") + SDL_GetError();
        SDL_DestroyWindow(*window);
        return false;
    }
    SDL_SetRenderVSync(*renderer, 1); // may fail (the offscreen driver): then the main loop's wait paces it
    SDL_SetWindowMinimumSize(*window, (int)(640 * scale), (int)(420 * scale));
    SDL_SetWindowPosition(*window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(*window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // no imgui.ini: the layout is fixed
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    gui_style(scale);
    io.Fonts->AddFontDefaultVector();
    ImGui_ImplSDL3_InitForSDLRenderer(*window, *renderer);
    ImGui_ImplSDLRenderer3_Init(*renderer);
    return true;
}

void gui_close(SDL_Window *window, SDL_Renderer *renderer) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// True while any button of an open gamepad is held or an axis is past a third of its range.
static bool any_gamepad_input_held() {
    int n = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&n);
    bool held = false;
    for (int i = 0; i < n && !held; i++) {
        SDL_Gamepad *g = SDL_GetGamepadFromID(ids[i]);
        if (g == nullptr) {
            continue;
        }
        for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT && !held; b++) {
            held = SDL_GetGamepadButton(g, (SDL_GamepadButton)b);
        }
        for (int a = 0; a < SDL_GAMEPAD_AXIS_COUNT && !held; a++) {
            int v = SDL_GetGamepadAxis(g, (SDL_GamepadAxis)a);
            held = a >= SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? v > 10000 : (v > 10000 || v < -10000);
        }
    }
    SDL_free(ids);
    return held;
}

// ---- the app

App::App(SDL_Window *window, SDL_Renderer *renderer, const SettingsDir &location, const AppOptions &options)
    : window_(window), renderer_(renderer), location_(location) {
    if (!location_.dir.empty()) {
        settings_.load(location_.dir);
    }
    std::string exe_dir;
    if (const char *base = SDL_GetBasePath()) {
        exe_dir = path_strip_slash(base);
    }
    game_ = game_find(options.game, exe_dir, &game_tried_);
    echo_game_ = options.echo_game;
    SDL_strlcpy(disc_input_, settings_.values.disc_path.c_str(), sizeof(disc_input_));
    // First run, or the disc went away: start on the disc screen (LAUNCHER_MODS_PLAN 1). A disc set by hand (no
    // verified SHA-1, or its size changed) is checked right away.
    switch (disc_status()) {
    case DiscStatus::Unset:
    case DiscStatus::Missing:
        screen_ = Screen::Disc;
        break;
    case DiscStatus::Unverified:
        screen_ = Screen::Disc;
        check_.start(settings_.resolve(settings_.values.disc_path));
        break;
    case DiscStatus::Checking:
    case DiscStatus::Verified:
        break;
    }
}

void App::handle_event(const SDL_Event &e) {
    // The controls' prompt takes its keys and gamepad inputs before ImGui (no Enter or Escape reaches a widget).
    if (capture_.active() && capture_.feed(e)) {
        if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
            pad_nav_hold_ = true;
        }
        return;
    }
    ImGui_ImplSDL3_ProcessEvent(&e);
    if (e.type == SDL_EVENT_QUIT ||
        (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(window_))) {
        quit_ = true;
    }
    // A file dropped on the window: a disc image (the fallback when the file dialog is not available).
    if (e.type == SDL_EVENT_DROP_FILE && e.drop.data != nullptr && !run_.running()) {
        screen_ = Screen::Disc;
        choose_disc(e.drop.data);
    }
}

// ---- the disc

DiscStatus App::disc_status() const {
    const Settings &s = settings_.values;
    if (check_.state() == DiscCheck::State::Running) {
        return DiscStatus::Checking;
    }
    if (s.disc_path.empty()) {
        return DiscStatus::Unset;
    }
    std::string bin, err;
    if (!disc_bin_path(settings_.resolve(s.disc_path), &bin, &err) || disc_file_size(bin) < 0) {
        return DiscStatus::Missing;
    }
    if (s.disc_sha1 != DISC_SHA1 || disc_file_size(bin) != (int64_t)DISC_BIN_SIZE) {
        return DiscStatus::Unverified;
    }
    return DiscStatus::Verified;
}

void App::choose_disc(const std::string &typed) {
    // A typed or pasted path may come with quotes or spaces around it; a relative one is taken from the current
    // directory (it is stored absolute).
    std::string path = typed;
    while (!path.empty() && (path.back() == ' ' || path.back() == '\t' || path.back() == '\n' || path.back() == '\r')) {
        path.pop_back();
    }
    size_t first = path.find_first_not_of(" \t");
    path = first == std::string::npos ? "" : path.substr(first);
    if (path.size() >= 2 && (path[0] == '"' || path[0] == '\'') && path.back() == path[0]) {
        path = path.substr(1, path.size() - 2);
    }
    if (path.empty()) {
        return;
    }
    if (!path_is_absolute(path)) {
        if (char *cwd = SDL_GetCurrentDirectory()) {
            path = path_join(cwd, path);
            SDL_free(cwd);
        }
    }
    SDL_strlcpy(disc_input_, path.c_str(), sizeof(disc_input_));
    disc_message_.clear();
    std::string bin, err;
    if (!disc_bin_path(path, &bin, &err)) {
        disc_message_ = err;
        disc_message_ok_ = false;
        return;
    }
    if (!path_is_file(bin)) {
        disc_message_ = "Not found: " + bin;
        disc_message_ok_ = false;
        return;
    }
    check_.start(path);
}

void App::update_disc_check() {
    DiscCheck::State before = check_.state();
    DiscCheck::State now = check_.poll();
    if (before != DiscCheck::State::Running || now == DiscCheck::State::Running) {
        return;
    }
    Settings &s = settings_.values;
    if (now == DiscCheck::State::Passed) {
        if (settings_.resolve(s.disc_path) != check_.path()) {
            s.disc_path = check_.path(); // a new disc: absolute (a relative one written by hand stays as it is)
        }
        s.disc_sha1 = check_.sha1();
        s.last_dir = path_dir(check_.path());
        dirty_ = true;
        disc_message_ = "This is the European disc (SLES-03936).";
        disc_message_ok_ = true;
    } else if (now == DiscCheck::State::Failed) {
        disc_message_ = check_.message();
        disc_message_ok_ = false;
        // The disc in the settings failed its check (changed on disk): forget its SHA-1, keep the path to show.
        if (settings_.resolve(s.disc_path) == check_.path() && !s.disc_sha1.empty()) {
            s.disc_sha1.clear();
            dirty_ = true;
        }
    } else {
        disc_message_ = "Check cancelled.";
        disc_message_ok_ = false;
    }
}

void SDLCALL App::file_dialog_done(void *self, const char *const *files, int) {
    App *app = static_cast<App *>(self);
    std::lock_guard<std::mutex> lock(app->dialog_mutex_);
    app->dialog_done_ = true;
    if (files == nullptr) {
        app->dialog_error_ = SDL_GetError();
    } else if (files[0] != nullptr) {
        app->dialog_file_ = files[0];
    }
}

void App::open_file_dialog() {
    static const SDL_DialogFileFilter filters[] = {
        { "Disc image (.cue, .bin)", "cue;bin" },
        { "All files", "*" },
    };
    const Settings &s = settings_.values;
    std::string where = !s.last_dir.empty() ? s.last_dir
                        : !s.disc_path.empty() ? path_dir(settings_.resolve(s.disc_path))
                                               : std::string();
    {
        std::lock_guard<std::mutex> lock(dialog_mutex_);
        dialog_open_ = true;
        dialog_done_ = false;
        dialog_file_.clear();
        dialog_error_.clear();
    }
    SDL_ShowOpenFileDialog(file_dialog_done, this, window_, filters, 2, where.empty() ? nullptr : where.c_str(),
                           false);
}

// ---- the game

void App::play() {
    play_error_.clear();
    play_log_.clear();
    if (run_.running()) {
        return;
    }
    if (game_.empty()) {
        play_error_ = "The game was not found.";
        return;
    }
    dirty_ = true;
    flush();
    if (!error_.empty()) {
        play_error_ = "The settings could not be saved: " + error_;
        return;
    }
    GameProbe probe = game_probe(game_, settings_.path());
    std::vector<std::string> args;
    switch (probe.result) {
    case GameProbe::Result::Valid:
        args = game_args(game_, settings_);
        interim_ = false;
        break;
    case GameProbe::Result::NoConfig:
        args = game_args_interim(game_, settings_);
        interim_ = true;
        break;
    case GameProbe::Result::Invalid:
        play_error_ = "The game does not accept the settings file:\n" + probe.message;
        return;
    case GameProbe::Result::Failed:
        play_error_ = "The game could not be run: " + probe.message;
        return;
    }
    std::string err;
    if (!run_.start(args, settings_.dir(), &err, echo_game_)) {
        play_error_ = err;
        return;
    }
    std::fprintf(stderr, "launcher: started %s%s\n", run_.command().c_str(),
                 interim_ ? " (interim options: this game build has no --config)" : "");
    SDL_HideWindow(window_);
}

void App::update_game() {
    if (!run_.running() || run_.poll()) {
        return;
    }
    // It ended: the launcher comes back.
    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);
    int code = run_.exit_code();
    std::fprintf(stderr, "launcher: the game %s\n", game_exit_text(code).c_str());
    if (code != 0) {
        play_error_ = "The game " + game_exit_text(code) + ".";
        const auto &lines = run_.lines();
        size_t from = lines.size() > 40 ? lines.size() - 40 : 0;
        play_log_.assign(lines.begin() + (long)from, lines.end());
        screen_ = Screen::Play;
    }
}

void App::flush() {
    if (!dirty_ || !settings_.writable() || location_.dir.empty()) {
        return;
    }
    std::string err;
    if (settings_.save(&err)) {
        error_.clear();
    } else {
        error_ = err;
    }
    dirty_ = false;
}

void App::step_screen(int delta) {
    int n = (int)Screen::Count;
    screen_ = (Screen)(((int)screen_ + delta + n) % n);
}

void App::frame(const char *screenshot) {
    update_disc_check();
    update_game();
    {
        std::lock_guard<std::mutex> lock(dialog_mutex_);
        if (dialog_done_) {
            dialog_open_ = dialog_done_ = false;
            if (!dialog_error_.empty()) {
                disc_message_ = "The file dialog is not available (" + dialog_error_ +
                                "). Drag the .cue or .bin onto this window, or type its path.";
                disc_message_ok_ = false;
            } else if (!dialog_file_.empty()) {
                choose_disc(dialog_file_);
            }
        }
    }
    finish_capture();
    // ImGui reads the gamepads' state itself: after a gamepad press that went to the prompt, its navigation waits
    // until every button is released (else the same press would also activate the focused widget).
    ImGuiIO &nav_io = ImGui::GetIO();
    if (pad_nav_hold_ && !capture_.active() && !any_gamepad_input_held()) {
        pad_nav_hold_ = false;
    }
    if (pad_nav_hold_ || capture_.active()) {
        nav_io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    } else {
        nav_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    }
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    draw();
    ImGui::Render();
    // Saved once no widget is being dragged or typed into: one write per change, not one per frame.
    if (dirty_ && !ImGui::IsAnyItemActive()) {
        flush();
    }
    ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    SDL_SetRenderDrawColorFloat(renderer_, bg.x, bg.y, bg.z, 1.0f);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    if (screenshot != nullptr) {
        SDL_Surface *s = SDL_RenderReadPixels(renderer_, nullptr);
        if (s == nullptr || !SDL_SavePNG(s, screenshot)) {
            error_ = std::string("screenshot ") + screenshot + ": " + SDL_GetError();
        }
        SDL_DestroySurface(s);
    }
    SDL_RenderPresent(renderer_);
}

void App::draw() {
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("##launcher", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    // The screens in turn: Ctrl+PageUp/PageDown, the gamepad's shoulder buttons (not while a widget is in use: the
    // shoulders also tweak a slider's speed).
    ImGuiIO &io = ImGui::GetIO();
    if (!ImGui::IsAnyItemActive()) {
        if ((io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_PageDown, false)) ||
            ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false)) {
            step_screen(1);
        } else if ((io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_PageUp, false)) ||
                   ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) {
            step_screen(-1);
        }
    }

    const float status_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    const float nav_w = 190 * ImGui::GetStyle().FontScaleDpi;
    ImGui::BeginChild("nav", ImVec2(nav_w, -status_h), ImGuiChildFlags_None);
    draw_nav();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("page", ImVec2(0, -status_h), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 1.45f);
    ImGui::TextUnformatted(screen_name(screen_));
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();
    switch (screen_) {
    case Screen::Play:
        draw_play();
        break;
    case Screen::Disc:
        draw_disc();
        break;
    case Screen::Settings:
        draw_settings();
        break;
    case Screen::Controls:
        draw_controls();
        break;
    case Screen::Mods:
        draw_placeholder("The mods with their switches and each mod's options (phase 4).");
        break;
    case Screen::Count:
        break;
    }
    ImGui::EndChild();
    draw_status_bar();
    ImGui::End();
}

void App::draw_nav() {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, rgb(ACCENT));
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 1.25f);
    ImGui::TextUnformatted("  DW2003");
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::TextDisabled("  PC port launcher");
    ImGui::Spacing();
    ImGui::Spacing();
    const float h = ImGui::GetFrameHeight() * 1.35f;
    for (int i = 0; i < (int)Screen::Count; i++) {
        char label[64];
        SDL_snprintf(label, sizeof(label), "  %s", SCREEN_NAMES[i]);
        if (ImGui::Selectable(label, (int)screen_ == i, ImGuiSelectableFlags_None, ImVec2(0, h))) {
            screen_ = (Screen)i;
        }
    }
}

void App::draw_status_bar() {
    ImGui::Spacing();
    if (!error_.empty()) {
        ImGui::TextColored(rgb(ERROR_RED), "%s", error_.c_str());
        return;
    }
    if (location_.dir.empty()) {
        ImGui::TextColored(rgb(ERROR_RED), "No settings directory: %s", location_.error.c_str());
        return;
    }
    text_fit_left("Settings: ", settings_.path());
}

static const char *disc_status_text(DiscStatus d) {
    switch (d) {
    case DiscStatus::Unset:
        return "not chosen yet";
    case DiscStatus::Missing:
        return "not found";
    case DiscStatus::Unverified:
        return "not checked";
    case DiscStatus::Checking:
        return "checking...";
    case DiscStatus::Verified:
        return "the European disc (SLES-03936), checked";
    }
    return "?";
}

void App::draw_play() {
    const Settings &s = settings_.values;
    const DiscStatus disc = disc_status();
    ImGui::TextWrapped("Digimon World 2003 (Europe, SLES-03936) on the PC port.");
    ImGui::Spacing();
    if (fields_begin("play")) {
        const ImVec4 red = rgb(ERROR_RED);
        field("Disc", s.disc_path.empty() ? "not chosen yet" : settings_.resolve(s.disc_path));
        field("", disc_status_text(disc), disc == DiscStatus::Verified ? nullptr : &red);
        for (int i = 0; i < 2; i++) {
            const MemoryCard &c = s.memcard[i];
            field(i == 0 ? "Memory card 1" : "Memory card 2", c.present ? settings_.resolve(c.path) : "none");
        }
        field("Game", game_.empty() ? "not found" : game_, game_.empty() ? &red : nullptr);
        ImGui::EndTable();
    }
    ImGui::Spacing();
    const bool ready = disc == DiscStatus::Verified && !game_.empty() && !run_.running() && settings_.writable() &&
                       !location_.dir.empty();
    ImGui::BeginDisabled(!ready);
    ImGui::PushStyleColor(ImGuiCol_Button, rgb(ACCENT, ready ? 0.85f : 0.35f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, rgb(ACCENT));
    ImGui::PushStyleColor(ImGuiCol_Text, rgb(0x15171C));
    if (ImGui::Button("Play", ImVec2(200 * ImGui::GetStyle().FontScaleDpi, ImGui::GetFrameHeight() * 1.6f))) {
        play();
    }
    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();
    if (!ready) {
        if (run_.running()) {
            ImGui::TextDisabled("The game is running.");
        } else if (disc != DiscStatus::Verified) {
            ImGui::TextDisabled("Choose and check the disc first (the Disc screen).");
        } else if (game_.empty()) {
            ImGui::TextWrapped("The game's executable (dw2003) was not found. Build it with "
                               "cmake -S port -B build/port-sdl -G Ninja -DDW3_PORT_SDL=ON, put it beside the "
                               "launcher, or start the launcher with --game PATH. Looked at:");
            for (const std::string &t : game_tried_) {
                ImGui::BulletText("%s", t.c_str());
            }
        } else if (location_.dir.empty()) {
            ImGui::TextDisabled("There is no settings directory (the status bar says why).");
        } else {
            ImGui::TextDisabled("The settings file is from a newer launcher: this one does not start the game.");
        }
    }
    if (interim_ && play_error_.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("This game build has no --config: it was started with the equivalent options "
                            "(60 Hz is not passed).");
    }
    draw_play_error();
}

void App::draw_play_error() {
    if (play_error_.empty()) {
        return;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, rgb(ERROR_RED));
    ImGui::TextWrapped("%s", play_error_.c_str());
    ImGui::PopStyleColor();
    if (play_log_.empty()) {
        return;
    }
    ImGui::TextDisabled("Its last lines:");
    ImGui::SameLine();
    if (ImGui::SmallButton("Copy")) {
        std::string text = run_.command() + "\n";
        for (const std::string &l : play_log_) {
            text += l + "\n";
        }
        ImGui::SetClipboardText(text.c_str());
    }
    ImGui::BeginChild("log", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    for (const std::string &l : play_log_) {
        ImGui::TextUnformatted(l.c_str());
    }
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}

void App::draw_disc() {
    const Settings &s = settings_.values;
    const DiscStatus disc = disc_status();
    ImGui::TextWrapped("The game needs your own copy of the European disc (SLES-03936), as a .cue with its .bin "
                       "(one MODE2/2352 track, as Redump has it). Nothing is unpacked or copied.");
    ImGui::Spacing();
    if (fields_begin("disc")) {
        const ImVec4 red = rgb(ERROR_RED);
        field("Disc", s.disc_path.empty() ? "not chosen yet" : settings_.resolve(s.disc_path));
        field("Status", disc_status_text(disc),
              disc == DiscStatus::Verified || disc == DiscStatus::Checking ? nullptr : &red);
        ImGui::EndTable();
    }
    ImGui::Spacing();

    if (check_.state() == DiscCheck::State::Running) {
        char label[64];
        SDL_snprintf(label, sizeof(label), "SHA-1 %.0f%%", check_.progress() * 100.0f);
        ImGui::TextDisabled("Checking %s", check_.path().c_str());
        ImGui::ProgressBar(check_.progress(), ImVec2(-1, 0), label);
        if (ImGui::Button("Cancel")) {
            check_.cancel();
        }
        return;
    }

    bool open_dialog;
    {
        std::lock_guard<std::mutex> lock(dialog_mutex_);
        open_dialog = dialog_open_;
    }
    ImGui::BeginDisabled(open_dialog);
    if (ImGui::Button("Choose a file...")) {
        open_file_dialog();
    }
    ImGui::EndDisabled();
    if (disc == DiscStatus::Unverified || (disc == DiscStatus::Verified && !s.disc_path.empty())) {
        ImGui::SameLine();
        if (ImGui::Button(disc == DiscStatus::Verified ? "Check again" : "Check")) {
            choose_disc(settings_.resolve(s.disc_path));
        }
    }
    ImGui::Spacing();
    ImGui::TextDisabled("Or drag the .cue (or the .bin) onto this window, or type its path:");
    ImGui::SetNextItemWidth(-ImGui::CalcTextSize("Use this path").x - ImGui::GetStyle().FramePadding.x * 2 -
                            ImGui::GetStyle().ItemSpacing.x);
    bool enter = ImGui::InputTextWithHint("##path", "/path/to/Digimon World 2003 (Europe).cue", disc_input_,
                                          sizeof(disc_input_), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Use this path") || enter) {
        choose_disc(disc_input_);
    }
    if (!disc_message_.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, disc_message_ok_ ? rgb(0x6CC070) : rgb(ERROR_RED));
        ImGui::TextWrapped("%s", disc_message_.c_str());
        ImGui::PopStyleColor();
    }
}

static void section_title(const char *title) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, rgb(ACCENT));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
}

void App::draw_settings() {
    Settings &s = settings_.values;
    const float label_w = 150 * ImGui::GetStyle().FontScaleDpi;
    ImGui::BeginDisabled(!settings_.writable());

    section_title("Video");
    ImGui::TextDisabled("Window size");
    ImGui::SameLine(label_w);
    ImGui::SetNextItemWidth(260 * ImGui::GetStyle().FontScaleDpi);
    char size[64];
    SDL_snprintf(size, sizeof(size), "x%d  (%d x %d)", s.scale, 320 * s.scale, 240 * s.scale);
    dirty_ |= ImGui::SliderInt("##scale", &s.scale, 1, 16, size, ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextDisabled("Fullscreen");
    ImGui::SameLine(label_w);
    dirty_ |= ImGui::Checkbox("##fullscreen", &s.fullscreen);
    ImGui::SameLine();
    ImGui::TextDisabled("(F11 in the game, by default)");
    ImGui::TextDisabled("Refresh");
    ImGui::SameLine(label_w);
    if (ImGui::RadioButton("50 Hz (PAL)", s.refresh == 50)) {
        s.refresh = 50;
        dirty_ = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("60 Hz", s.refresh == 60)) {
        s.refresh = 60;
        dirty_ = true;
    }
    if (s.refresh == 60) {
        ImGui::Indent(label_w);
        ImGui::PushTextWrapPos(0);
        ImGui::TextDisabled("The game's own 60 Hz mode, as the NTSC patch gives it: time, walking and music keep their "
                            "speed, other things run 20%% faster. Until the game implements it, it logs this and "
                            "runs at 50 Hz.");
        ImGui::PopTextWrapPos();
        ImGui::Unindent(label_w);
    }

    section_title("Audio");
    ImGui::TextDisabled("Mute");
    ImGui::SameLine(label_w);
    dirty_ |= ImGui::Checkbox("##mute", &s.mute);

    section_title("Memory cards");
    for (int i = 0; i < 2; i++) {
        MemoryCard &c = s.memcard[i];
        ImGui::PushID(i);
        ImGui::TextDisabled("Slot %d", i + 1);
        ImGui::SameLine(label_w);
        dirty_ |= ImGui::Checkbox("##in", &c.present);
        ImGui::SameLine();
        ImGui::BeginDisabled(!c.present);
        char name[512];
        SDL_strlcpy(name, c.path.c_str(), sizeof(name));
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##file", name, sizeof(name))) {
            if (name[0] != '\0') {
                c.path = name;
                dirty_ = true;
            }
        }
        ImGui::EndDisabled();
        ImGui::PopID();
    }
    ImGui::Indent(label_w);
    ImGui::PushTextWrapPos(0);
    ImGui::TextDisabled("A .mcd card image, relative to the settings directory; the game creates a formatted card when "
                        "the file is missing. Unticked: no card in the slot.");
    ImGui::PopTextWrapPos();
    ImGui::Unindent(label_w);
    ImGui::EndDisabled();

    section_title("Settings file");
    if (fields_begin("where")) {
        const ImVec4 red = rgb(ERROR_RED);
        field("Directory", location_.dir.empty() ? "(none)" : location_.dir);
        field("Chosen by", dir_source_name(location_.source));
        switch (settings_.state()) {
        case SettingsFile::State::New:
            field("File", "not written yet (the defaults)");
            break;
        case SettingsFile::State::Loaded:
            field("File", "read");
            break;
        case SettingsFile::State::Broken:
            field("File", "unreadable: the defaults are used", &red);
            break;
        case SettingsFile::State::Newer:
            field("File", "from a newer launcher: read only", &red);
            break;
        }
        ImGui::EndTable();
    }
    for (const std::string &m : settings_.messages()) {
        ImGui::Bullet();
        ImGui::TextWrapped("%s", m.c_str());
    }
    if (!location_.dir.empty() && ImGui::Button("Open the directory")) {
        SDL_OpenURL(path_to_url(location_.dir).c_str());
    }
}

// ---- the controls

void App::capture_for(int section, const std::string &id) {
    std::string label = id;
    for (const PadButton &b : pad_buttons()) {
        if (id == b.id) {
            label = b.label;
        }
    }
    for (const HotkeyAction &a : hotkey_actions()) {
        if (id == a.id) {
            label = a.label;
        }
    }
    begin_capture(section == 0 ? InputCapture::Kind::Key
                  : section == 1 ? InputCapture::Kind::Pad
                                 : InputCapture::Kind::Binding,
                  section, id, label);
}

void App::begin_capture(InputCapture::Kind kind, int section, const std::string &id, const std::string &label) {
    capture_.begin(kind);
    capture_section_ = section;
    capture_id_ = id;
    capture_label_ = label;
    capture_popup_ = true;
}

void App::finish_capture() {
    Trigger t;
    if (!capture_.take_done(&t) || t.empty()) {
        return;
    }
    Settings &s = settings_.values;
    if (capture_section_ == 2) {
        Binding b = s.hotkey_for(capture_id_);
        if (std::find(b.begin(), b.end(), t) == b.end()) {
            b.push_back(t);
        }
        s.hotkeys[capture_id_] = b;
    } else {
        std::vector<std::string> names = capture_section_ == 0 ? s.keys_for(capture_id_) : s.pad_for(capture_id_);
        if (std::find(names.begin(), names.end(), t[0]) == names.end()) {
            names.push_back(t[0]);
        }
        (capture_section_ == 0 ? s.keyboard : s.gamepad)[capture_id_] = names;
    }
    dirty_ = true;
}

// The other buttons (or hotkeys) using `input`, for the conflict marks.
static std::string uses_of(const Settings &s, bool pad, const std::string &input, const std::string &except) {
    std::string out;
    for (const PadButton &b : pad_buttons()) {
        std::vector<std::string> names = pad ? s.pad_for(b.id) : s.keys_for(b.id);
        if (b.id != except && std::find(names.begin(), names.end(), input) != names.end()) {
            out += (out.empty() ? "" : ", ") + std::string(b.label);
        }
    }
    const std::string full = pad ? "pad:" + input : input;
    for (const HotkeyAction &a : hotkey_actions()) {
        for (const Trigger &t : s.hotkey_for(a.id)) {
            if (a.id != except && t.size() == 1 && t[0] == full) {
                out += (out.empty() ? "" : ", ") + std::string("the hotkey ") + a.label;
            }
        }
    }
    return out;
}

// One input as a chip; true when its remove button was pressed.
static bool chip(const std::string &text, const std::string &conflict, const char *id) {
    ImGui::PushID(id);
    if (!conflict.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Button, rgb(0x7A3B2E));
    }
    ImGui::SmallButton(text.c_str());
    if (!conflict.empty()) {
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("Also bound to %s", conflict.c_str());
    }
    ImGui::SameLine(0, 2);
    bool removed = ImGui::SmallButton("x");
    ImGui::SetItemTooltip("Remove %s", text.c_str());
    ImGui::PopID();
    ImGui::SameLine();
    return removed;
}

void App::draw_controls() {
    ImGui::BeginDisabled(!settings_.writable());
    if (ImGui::BeginTabBar("controls")) {
        if (ImGui::BeginTabItem("Keyboard")) {
            draw_names_table(false);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Gamepad")) {
            draw_names_table(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Hotkeys")) {
            draw_hotkeys();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndDisabled();
    draw_capture_popup();
}

void App::draw_names_table(bool pad) {
    Settings &s = settings_.values;
    auto &map = pad ? s.gamepad : s.keyboard;
    ImGui::TextDisabled(pad ? "Every gamepad drives the PS1 pad (positions: south is the PlayStation cross)."
                            : "Keys by their place on the keyboard, whatever its layout.");
    ImGui::SameLine();
    ImGui::BeginDisabled(map.empty());
    if (ImGui::SmallButton(pad ? "Reset the gamepad" : "Reset the keyboard")) {
        map.clear();
        dirty_ = true;
    }
    ImGui::EndDisabled();
    if (!ImGui::BeginTable(pad ? "pad" : "keys", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
        return;
    }
    ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Inputs", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Default", ImGuiTableColumnFlags_WidthFixed);
    for (const PadButton &b : pad_buttons()) {
        ImGui::PushID(b.id);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(b.label);
        ImGui::TableNextColumn();
        std::vector<std::string> names = pad ? s.pad_for(b.id) : s.keys_for(b.id);
        for (size_t i = 0; i < names.size(); i++) {
            const std::string label = pad ? pad_input_label(names[i]) : names[i];
            if (chip(label, uses_of(s, pad, names[i], b.id), std::to_string(i).c_str())) {
                names.erase(names.begin() + (long)i);
                map[b.id] = names;
                dirty_ = true;
                break;
            }
        }
        if (names.empty()) {
            ImGui::TextDisabled("unbound");
            ImGui::SameLine();
        }
        if (ImGui::SmallButton("+")) {
            capture_for(pad ? 1 : 0, b.id);
        }
        ImGui::SetItemTooltip(pad ? "Add a gamepad input" : "Add a key");
        ImGui::TableNextColumn();
        if (map.count(b.id) != 0) {
            if (ImGui::SmallButton("Default")) {
                map.erase(b.id);
                dirty_ = true;
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void App::draw_hotkeys() {
    Settings &s = settings_.values;
    ImGui::TextWrapped("The port's own actions. A hotkey never reaches the game's pad. A chord: hold its inputs "
                       "together, then let go. The mods' bindings are on the Mods screen.");
    if (!ImGui::BeginTable("hotkeys", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
        return;
    }
    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Binding", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Default", ImGuiTableColumnFlags_WidthFixed);
    for (const HotkeyAction &a : hotkey_actions()) {
        ImGui::PushID(a.id);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(a.label);
        ImGui::SetItemTooltip("%s", a.description);
        ImGui::TableNextColumn();
        Binding b = s.hotkey_for(a.id);
        for (size_t i = 0; i < b.size(); i++) {
            std::string conflict;
            if (b[i].size() == 1) {
                const std::string &in = b[i][0];
                bool pad = in.compare(0, 4, "pad:") == 0;
                conflict = uses_of(s, pad, pad ? in.substr(4) : in, a.id);
            }
            if (chip(binding_label({ b[i] }), conflict, std::to_string(i).c_str())) {
                b.erase(b.begin() + (long)i);
                s.hotkeys[a.id] = b;
                dirty_ = true;
                break;
            }
        }
        if (b.empty()) {
            ImGui::TextDisabled("unbound");
            ImGui::SameLine();
        }
        ImGui::BeginDisabled(b.size() >= BINDING_MAX_TRIGGERS);
        if (ImGui::SmallButton("+")) {
            capture_for(2, a.id);
        }
        ImGui::EndDisabled();
        ImGui::SetItemTooltip("Add a key, a gamepad input or a chord (up to %d inputs held together)",
                              (int)CHORD_MAX_INPUTS);
        ImGui::TableNextColumn();
        if (s.hotkeys.count(a.id) != 0) {
            if (ImGui::SmallButton("Default")) {
                s.hotkeys.erase(a.id);
                dirty_ = true;
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void App::draw_capture_popup() {
    if (capture_popup_) {
        ImGui::OpenPopup("Press an input");
        capture_popup_ = false;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("Press an input", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }
    if (!capture_.active()) {
        ImGui::CloseCurrentPopup();
    }
    const char *what = capture_section_ == 0   ? "a key"
                       : capture_section_ == 1 ? "a gamepad button, trigger or stick direction"
                                               : "a key, a gamepad input, or several together (a chord)";
    ImGui::Text("%s: press %s.", capture_label_.c_str(), what);
    if (!capture_.held().empty()) {
        ImGui::TextColored(rgb(ACCENT), "%s", binding_label({ capture_.held() }).c_str());
    }
    ImGui::TextDisabled("Escape cancels.");
    if (ImGui::Button("Cancel")) {
        capture_.cancel();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void App::draw_placeholder(const char *what) {
    ImGui::TextDisabled("%s", what);
}

} // namespace dw3
