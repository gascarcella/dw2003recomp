#include "selftest.h"

#include <cstdio>
#include <string>

#include <SDL3/SDL.h>

#include "app.h"
#include "json_value.h"
#include "paths.h"
#include "settings.h"

namespace dw3 {

static int checks, failures;

static void check(bool ok, const std::string &what) {
    checks++;
    if (!ok) {
        failures++;
        std::fprintf(stderr, "self-test: FAILED: %s\n", what.c_str());
    }
}

static void write(const std::string &path, const std::string &text) {
    std::string err;
    check(SDL_SaveFile(path.c_str(), text.data(), text.size()), "write " + path + ": " + SDL_GetError());
}

static std::string read(const std::string &path) {
    std::string text, err;
    file_read(path, &text, &err);
    return text;
}

// Deletes `path` and everything under it (the test's own directory only).
static SDL_EnumerationResult remove_entry(void *, const char *dir, const char *name) {
    std::string p = path_join(dir, name);
    SDL_PathInfo info;
    if (SDL_GetPathInfo(p.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY) {
        SDL_EnumerateDirectory(p.c_str(), remove_entry, nullptr);
    }
    SDL_RemovePath(p.c_str());
    return SDL_ENUM_CONTINUE;
}

static void remove_tree(const std::string &path) {
    if (path_is_dir(path)) {
        SDL_EnumerateDirectory(path.c_str(), remove_entry, nullptr);
    }
    SDL_RemovePath(path.c_str());
}

// The plan's sketch of the file (LAUNCHER_MODS_PLAN 4.3), plus a member no one knows yet.
static const char SAMPLE_SETTINGS[] = R"({
  "schema": 1,
  "disc": { "path": "/games/dw2003.cue", "sha1": "457cb233349ba841e03b33d8060f8fbcadd45cb3" },
  "video": { "scale": 4, "fullscreen": true, "refresh": 60 },
  "audio": { "mute": true },
  "memcard1": "cards/card1.mcd",
  "input": {
    "keyboard": { "cross": "X", "circle": "C", "start": "Return" },
    "gamepad":  { "cross": "south", "circle": "east" },
    "hotkeys":  { "fast_forward.hold": "Tab", "fast_forward.toggle": "F1", "skip_dialogues.toggle": "F2" }
  },
  "mods": {
    "fast_forward":      { "enabled": true,  "speed": 4, "mute": true },
    "skip_dialogues":    { "enabled": false, "fast_forward_waits": false },
    "battle_animations": { "enabled": false, "hit_reaction": true }
  },
  "future": [1, 2.5, "x\ny", null, {}]
}
)";

static void test_paths() {
    check(path_is_absolute("/a") && path_is_absolute("C:\\a") && path_is_absolute("c:/a") && path_is_absolute("\\a"),
          "absolute paths");
    check(!path_is_absolute("a/b") && !path_is_absolute("C:a") && !path_is_absolute(""), "relative paths");
    check(path_join("/a", "b") == "/a/b" && path_join("/a/", "b") == "/a/b" && path_join("C:\\a\\", "b") == "C:\\a\\b",
          "path_join");
    check(path_join("/a", "/b") == "/b" && path_join("", "b") == "b", "path_join with an absolute name");
    check(path_dir("/a/b") == "/a" && path_dir("/b") == "/" && path_dir("C:\\b") == "C:\\" && path_dir("b") == "",
          "path_dir");
    check(path_base("/a/b.cue") == "b.cue" && path_base("C:\\x\\y.bin") == "y.bin", "path_base");
    check(path_strip_slash("/a/") == "/a" && path_strip_slash("/") == "/" && path_strip_slash("C:\\") == "C:\\",
          "path_strip_slash");
    check(path_to_url("/home/a b/x") == "file:///home/a%20b/x" && path_to_url("C:\\D W\\") == "file:///C:/D%20W/",
          "path_to_url");
}

static void test_json() {
    std::string err;
    Json j = Json::parse(SAMPLE_SETTINGS, &err);
    check(j.is_object() && err.empty(), "the sample settings parse: " + err);
    Json again = Json::parse(j.dump(), &err);
    check(again == j, "dump() reads back equal");
    check(again.dump() == j.dump(), "dump() is stable");
    check(j.members().front().first == "schema" && j.members().back().first == "future", "member order kept");
    check(Json::parse("{\"a\":1,}", &err).is_null() && !err.empty(), "a trailing comma is an error");
    Json n = Json::number(0.1);
    check(Json::parse(n.dump(), nullptr) == n && n.dump() == "0.1\n", "numbers: the shortest text that reads back");
    check(Json::number(3).dump() == "3\n", "whole numbers without a fraction");
    check(Json::string("a\"\\\x01é").dump() == "\"a\\\"\\\\\\u0001é\"\n", "string escapes");
}

static void test_lookup(const std::string &root) {
    const std::string exe = path_join(root, "exe"), cwd = path_join(root, "cwd"), user = path_join(root, "user");
    std::string err;
    for (const std::string &d : { exe, cwd }) {
        check(path_make_dir(d, &err), err);
    }
    int user_calls = 0;
    DirLookup in;
    in.exe_dir = exe;
    in.cwd = cwd;
    in.user_dir = [&] {
        user_calls++;
        path_make_dir(user, nullptr);
        return user;
    };
    // Nothing there: the per-user directory.
    SettingsDir d = settings_dir_choose(in);
    check(d.source == DirSource::User && d.dir == user && user_calls == 1, "lookup: the per-user directory last");
    // A settings.json in the current directory.
    write(path_join(cwd, SETTINGS_FILE), "{}");
    d = settings_dir_choose(in);
    check(d.source == DirSource::CurrentDir && d.dir == cwd, "lookup: a settings.json in the current directory");
    // portable.txt beside the executable wins over it.
    write(path_join(exe, SETTINGS_PORTABLE_FILE), "");
    d = settings_dir_choose(in);
    check(d.source == DirSource::Portable && d.dir == exe, "lookup: portable.txt beside the executable");
    // The environment wins over portable mode; relative to the current directory.
    in.env = "from-env";
    d = settings_dir_choose(in);
    check(d.source == DirSource::Environment && d.dir == path_join(cwd, "from-env"), "lookup: the environment");
    // --config-dir wins over everything; its trailing separator dropped.
    in.arg = path_join(root, "arg") + "/";
    d = settings_dir_choose(in);
    check(d.source == DirSource::Argument && d.dir == path_join(root, "arg"), "lookup: --config-dir first");
    check(user_calls == 1, "lookup: the per-user directory is only made when it is chosen");
}

static void test_settings_file(const std::string &root) {
    std::string err;
    // A new directory (missing, two levels): the defaults; the first save makes the directory and the file.
    const std::string fresh = path_join(root, "new/settings");
    SettingsFile f;
    f.load(fresh);
    check(f.state() == SettingsFile::State::New && f.values.scale == 3 && f.values.memcard1 == "card1.mcd",
          "a missing file gives the defaults");
    f.values.disc_path = "../disc/dw2003.cue";
    check(f.save(&err), "the first save: " + err);
    check(path_is_file(path_join(fresh, SETTINGS_FILE)), "the first save writes settings.json");
    check(f.resolve("card1.mcd") == path_join(fresh, "card1.mcd"), "paths resolve against the file's directory");
    check(f.resolve("/abs/x.mcd") == "/abs/x.mcd", "absolute paths stay");
    SettingsFile g;
    g.load(fresh);
    check(g.state() == SettingsFile::State::Loaded && g.values.disc_path == "../disc/dw2003.cue" &&
              g.messages().empty(),
          "the saved file reads back");
    Json doc = Json::parse(read(g.path()), nullptr);
    check(doc.find("schema") != nullptr && doc.find("schema")->as_int(0, 0, 99) == SETTINGS_SCHEMA,
          "the file has its schema number");

    // The sample: every value read, every unknown member kept through a save.
    const std::string sample = path_join(root, "sample");
    path_make_dir(sample, nullptr);
    write(path_join(sample, SETTINGS_FILE), SAMPLE_SETTINGS);
    SettingsFile s;
    s.load(sample);
    const Settings &v = s.values;
    check(s.state() == SettingsFile::State::Loaded && s.messages().empty(), "the sample loads without a warning");
    check(v.disc_path == "/games/dw2003.cue" && v.scale == 4 && v.fullscreen && v.refresh == 60 && v.mute &&
              v.memcard1 == "cards/card1.mcd",
          "the sample's values");
    s.values.scale = 2;
    check(s.save(&err), "saving the sample: " + err);
    Json before = Json::parse(SAMPLE_SETTINGS, nullptr), after = Json::parse(read(s.path()), nullptr);
    check(after.find("video") != nullptr && after.find("video")->find("scale")->as_int(0, 0, 99) == 2,
          "the change is written");
    after.member("video").set("scale", Json::number(4));
    check(after == before, "everything else is kept as it was (input, mods, an unknown member, the order)");

    // Invalid values: a warning each, the defaults used.
    const std::string bad = path_join(root, "bad");
    path_make_dir(bad, nullptr);
    write(path_join(bad, SETTINGS_FILE),
          R"({"schema":1,"video":{"scale":99,"refresh":55,"fullscreen":"yes"},"audio":3,"memcard1":7})");
    SettingsFile b;
    b.load(bad);
    check(b.state() == SettingsFile::State::Loaded && b.messages().size() == 5, "five warnings for five bad values");
    check(b.values.scale == 3 && b.values.refresh == 50 && !b.values.fullscreen && b.values.memcard1 == "card1.mcd",
          "bad values fall back to the defaults");

    // Not JSON: the defaults; the first save keeps the old file aside.
    const std::string broken = path_join(root, "broken");
    path_make_dir(broken, nullptr);
    write(path_join(broken, SETTINGS_FILE), "{ \"schema\": 1, oops");
    SettingsFile k;
    k.load(broken);
    check(k.state() == SettingsFile::State::Broken && k.messages().size() == 1, "a broken file is reported");
    check(k.save(&err), "saving over a broken file: " + err);
    check(read(path_join(broken, std::string(SETTINGS_FILE) + ".broken")) == "{ \"schema\": 1, oops",
          "the broken file is kept as settings.json.broken");
    check(Json::parse(read(k.path()), nullptr).is_object(), "a valid file replaces it");

    // A newer schema: read, never written.
    const std::string newer = path_join(root, "newer");
    path_make_dir(newer, nullptr);
    const std::string newer_text = R"({"schema":2,"video":{"scale":5}})";
    write(path_join(newer, SETTINGS_FILE), newer_text);
    SettingsFile n;
    n.load(newer);
    check(n.state() == SettingsFile::State::Newer && !n.writable() && n.values.scale == 5, "a newer schema is read");
    n.values.scale = 1;
    check(!n.save(&err) && read(n.path()) == newer_text, "a newer schema is never written");
}

// ---- the window

static void push_key(SDL_Window *w, SDL_Scancode key, SDL_Keymod mod, bool down) {
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(w);
    e.key.scancode = key;
    e.key.key = SDL_GetKeyFromScancode(key, SDL_KMOD_NONE, false);
    e.key.mod = mod;
    e.key.down = down;
    check(SDL_PushEvent(&e), std::string("SDL_PushEvent: ") + SDL_GetError());
}

static void pump(App &app, int frames) {
    for (int i = 0; i < frames; i++) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            app.handle_event(e);
        }
        app.frame();
    }
}

static void test_window(const std::string &root) {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::string err;
    if (!gui_open(&window, &renderer, &err)) {
        check(false, err);
        return;
    }
    std::fprintf(stderr, "self-test: video driver %s, renderer %s\n", SDL_GetCurrentVideoDriver(),
                 SDL_GetRendererName(renderer));
    const std::string shots = path_join(root, "screens");
    path_make_dir(shots, nullptr);
    {
        SettingsDir location;
        location.dir = path_join(root, "ui");
        location.source = DirSource::Argument;
        App app(window, renderer, location);
        check(app.screen() == Screen::Disc, "a first run starts on the disc screen");
        pump(app, 3);

        // Ctrl+PageDown through every screen (a picture of each), back to the start; Ctrl+PageUp one back.
        const Screen first = app.screen();
        const int n = (int)Screen::Count;
        for (int i = 0; i < n; i++) {
            Screen want = (Screen)(((int)first + i) % n);
            check(app.screen() == want, std::string("Ctrl+PageDown reaches ") + screen_name(want));
            pump(app, 2);
            std::string png = path_join(shots, std::to_string((int)app.screen()) + "-" + screen_name(app.screen()) +
                                                   ".png");
            app.frame(png.c_str());
            check(path_is_file(png), "a picture of " + std::string(screen_name(app.screen())));
            push_key(window, SDL_SCANCODE_PAGEDOWN, SDL_KMOD_LCTRL, true);
            pump(app, 1);
            push_key(window, SDL_SCANCODE_PAGEDOWN, SDL_KMOD_LCTRL, false);
            pump(app, 2);
        }
        check(app.screen() == first, "Ctrl+PageDown wraps around");
        push_key(window, SDL_SCANCODE_PAGEUP, SDL_KMOD_LCTRL, true);
        pump(app, 1);
        push_key(window, SDL_SCANCODE_PAGEUP, SDL_KMOD_LCTRL, false);
        pump(app, 2);
        check(app.screen() == (Screen)(((int)first + n - 1) % n), "Ctrl+PageUp goes back");
        // PageDown without Ctrl does not change the screen.
        Screen here = app.screen();
        push_key(window, SDL_SCANCODE_PAGEDOWN, SDL_KMOD_NONE, true);
        pump(app, 1);
        push_key(window, SDL_SCANCODE_PAGEDOWN, SDL_KMOD_NONE, false);
        pump(app, 2);
        check(app.screen() == here, "PageDown alone keeps the screen");

        // A virtual gamepad: R1 next, L1 back.
        SDL_VirtualJoystickDesc desc;
        SDL_INIT_INTERFACE(&desc);
        desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
        desc.nbuttons = SDL_GAMEPAD_BUTTON_DPAD_RIGHT + 1;
        desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
        desc.name = "dw2003 launcher self-test";
        SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
        SDL_Joystick *joy = id != 0 ? SDL_OpenJoystick(id) : nullptr;
        check(joy != nullptr, std::string("a virtual gamepad: ") + SDL_GetError());
        if (joy != nullptr) {
            pump(app, 4); // the backend opens it on SDL_EVENT_GAMEPAD_ADDED
            here = app.screen();
            SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, true);
            pump(app, 2);
            SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, false);
            pump(app, 2);
            check(app.screen() == (Screen)(((int)here + 1) % n), "the gamepad's R1 goes to the next screen");
            SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, true);
            pump(app, 2);
            SDL_SetJoystickVirtualButton(joy, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, false);
            pump(app, 2);
            check(app.screen() == here, "the gamepad's L1 goes back");
            SDL_CloseJoystick(joy);
            SDL_DetachVirtualJoystick(id);
            pump(app, 2);
        }

        // The window's close button ends the loop; nothing was changed, so nothing was written.
        SDL_Event e;
        SDL_zero(e);
        e.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
        e.window.windowID = SDL_GetWindowID(window);
        SDL_PushEvent(&e);
        pump(app, 1);
        check(app.quit_requested(), "closing the window quits");
        app.flush();
        check(!path_is_file(path_join(location.dir, SETTINGS_FILE)), "an unchanged first run writes no file");
        check(app.last_error().empty(), "no error in the status bar: " + app.last_error());
    }
    gui_close(window, renderer);
}

bool self_test_run(const std::string &dir) {
    std::string err;
    const std::string root = path_join(path_strip_slash(dir), "launcher-self-test");
    remove_tree(root);
    if (!path_make_dir(root, &err)) {
        std::fprintf(stderr, "self-test: %s\n", err.c_str());
        return false;
    }
    test_paths();
    test_json();
    test_lookup(path_join(root, "lookup"));
    test_settings_file(path_join(root, "files"));
    test_window(root);
    std::fprintf(stderr, "self-test: %d of %d checks passed; pictures in %s\n", checks - failures, checks,
                 path_join(root, "screens").c_str());
    return failures == 0;
}

} // namespace dw3
