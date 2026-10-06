#include "app.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

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

// ---- the app

App::App(SDL_Window *window, SDL_Renderer *renderer, const SettingsDir &location)
    : window_(window), renderer_(renderer), location_(location) {
    if (!location_.dir.empty()) {
        settings_.load(location_.dir);
    }
    // First run, or the disc went away: start on the disc screen (LAUNCHER_MODS_PLAN 1).
    const std::string &disc = settings_.values.disc_path;
    if (disc.empty() || !path_is_file(settings_.resolve(disc))) {
        screen_ = Screen::Disc;
    }
}

void App::handle_event(const SDL_Event &e) {
    ImGui_ImplSDL3_ProcessEvent(&e);
    if (e.type == SDL_EVENT_QUIT ||
        (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(window_))) {
        quit_ = true;
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
        draw_placeholder("Keyboard and gamepad bindings and the hotkeys (phase 3).");
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

void App::draw_play() {
    const Settings &s = settings_.values;
    ImGui::TextWrapped("Digimon World 2003 (Europe, SLES-03936) on the PC port.");
    ImGui::Spacing();
    if (fields_begin("play")) {
        field("Disc", s.disc_path.empty() ? "not chosen yet" : settings_.resolve(s.disc_path));
        field("Memory card", settings_.resolve(s.memcard1));
        ImGui::EndTable();
    }
    ImGui::Spacing();
    ImGui::BeginDisabled(true);
    ImGui::Button("Play", ImVec2(180, 0));
    ImGui::EndDisabled();
    ImGui::TextDisabled("Starting the game comes in phase 2.");
}

void App::draw_disc() {
    const Settings &s = settings_.values;
    if (s.disc_path.empty()) {
        ImGui::TextWrapped("No disc image chosen yet. The game needs your own copy of the European disc "
                           "(a .cue or .bin dump of SLES-03936).");
    } else {
        ImGui::TextWrapped("%s", settings_.resolve(s.disc_path).c_str());
    }
    ImGui::Spacing();
    ImGui::TextDisabled("Choosing and checking the disc comes in phase 2.");
}

void App::draw_settings() {
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
    ImGui::Spacing();
    if (!location_.dir.empty() && ImGui::Button("Open the directory")) {
        SDL_OpenURL(path_to_url(location_.dir).c_str());
    }
    ImGui::Spacing();
    ImGui::TextDisabled("Video, 50/60 Hz and audio come in phase 3.");
}

void App::draw_placeholder(const char *what) {
    ImGui::TextDisabled("%s", what);
}

} // namespace dw3
