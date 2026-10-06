#include "selftest.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>

#include <SDL3/SDL.h>

#include "app.h"
#include "disc.h"
#include "game.h"
#include "input.h"
#include "json_value.h"
#include "mods.h"
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

// The settled example of the file (LAUNCHER_MODS_PLAN 4.3, schema 1), with other values, a chord, no card 2 and a
// member no one knows yet.
static const char SAMPLE_SETTINGS[] = R"({
  "schema": 1,
  "disc": { "path": "/games/dw2003.cue", "sha1": "457cb233349ba841e03b33d8060f8fbcadd45cb3" },
  "video": { "window": true, "scale": 4, "fullscreen": true, "refresh": 60 },
  "audio": { "mute": true },
  "memcard1": "cards/card1.mcd",
  "memcard2": null,
  "watchdog": 0,
  "input": {
    "keyboard": { "cross": "X", "start": ["Return", "Keypad Enter"] },
    "gamepad":  { "cross": "south", "up": ["dpup", "lefty-"] },
    "hotkeys":  { "pause": "P", "fullscreen": "F11" }
  },
  "mods": {
    "fast_forward":   { "enabled": true, "hold": [["pad:guide", "pad:south"], "Tab"] },
    "skip_dialogues": { "enabled": false, "toggle": "F2", "fast_forward_waits": false }
  },
  "launcher": { "last_dir": "/games" },
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
    check(f.state() == SettingsFile::State::New && f.values.scale == 2 && f.values.memcard[0].path == "card1.mcd" &&
              f.values.memcard[1].path == "card2.mcd" && f.values.memcard[1].present,
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
              v.memcard[0].present && v.memcard[0].path == "cards/card1.mcd" && !v.memcard[1].present,
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
          R"({"schema":1,"video":{"scale":99,"refresh":55,"fullscreen":"yes"},"audio":3,"memcard1":7,"memcard2":""})");
    SettingsFile b;
    b.load(bad);
    check(b.state() == SettingsFile::State::Loaded && b.messages().size() == 6, "six warnings for six bad values");
    check(b.values.scale == 2 && b.values.refresh == 50 && !b.values.fullscreen &&
              b.values.memcard[0].path == "card1.mcd" && b.values.memcard[1].path == "card2.mcd",
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

// ---- the controls

static SDL_Event key_event(SDL_Scancode key, bool down) {
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.scancode = key;
    e.key.down = down;
    return e;
}

static SDL_Event pad_button_event(SDL_GamepadButton b, bool down) {
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
    e.gbutton.button = (Uint8)b;
    e.gbutton.down = down;
    return e;
}

static SDL_Event pad_axis_event(SDL_GamepadAxis a, int v) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    e.gaxis.axis = (Uint8)a;
    e.gaxis.value = (Sint16)v;
    return e;
}

static void test_input(const std::string &root) {
    std::string err;
    // The binding grammar, both ways (the shortest form written).
    for (const char *text : { R"("F2")", R"("")", R"(["F2", "pad:guide"])", R"([["pad:guide", "pad:south"], "Tab"])",
                              R"("Keypad Enter")", R"("pad:lefty-")" }) {
        Json j = Json::parse(text, nullptr);
        Binding b;
        check(binding_from_json(j, &b, &err) && binding_to_json(b) == j, std::string("a binding round trip: ") + text +
                                                                             " " + err);
    }
    Binding b;
    check(binding_from_json(Json::parse("[]", nullptr), &b, &err) && b.empty() && binding_to_json(b) == Json::string(""),
          "[] is unbound, written \"\"");
    for (const char *bad : { "3", R"(["F2", 3])", R"("pad:nope")", R"("NotAKey")", R"([[]])", R"([["F2", 1]])",
                             R"([["A", "B", "C", "D", "E"]])", R"(["A", "B", "C", "D", "E", "F", "G", "H", "I"])" }) {
        check(!binding_from_json(Json::parse(bad, nullptr), &b, &err) && !err.empty(),
              std::string("a bad binding is refused: ") + bad);
    }
    check(binding_label({ { "F2" }, { "pad:guide", "pad:south" } }) == "F2 or Pad Guide + Pad South", "binding_label");
    std::vector<std::string> names;
    check(names_from_json(Json::parse(R"(["Return", "Keypad Enter"])", nullptr), false, &names, &err) &&
              names.size() == 2 && names_to_json(names) == Json::parse(R"(["Return", "Keypad Enter"])", nullptr),
          "key names: a list");
    check(!names_from_json(Json::string("south"), false, &names, &err) &&
              names_from_json(Json::string("south"), true, &names, &err),
          "a gamepad name is not a key name");
    // The game's defaults (port/src/input.c) are what an empty file means.
    Settings d;
    check(d.keys_for("start") == std::vector<std::string>({ "Return", "Keypad Enter" }) &&
              d.pad_for("up") == std::vector<std::string>({ "dpup", "lefty-" }) &&
              d.hotkey_for("fullscreen") == Binding({ { "F11" } }),
          "the defaults");
    for (const PadButton &pb : pad_buttons()) {
        for (const std::string &k : d.keys_for(pb.id)) {
            check(is_key_name(k), "a default key is an SDL scancode name: " + k);
        }
    }

    // Only changes are written; a reset removes them, and the objects the launcher emptied.
    const std::string dir = path_join(root, "input");
    SettingsFile f;
    f.load(dir);
    f.values.keyboard["cross"] = { "X", "V" };
    f.values.hotkeys["pause"] = { { "pad:guide", "pad:start" } };
    check(f.save(&err), err);
    Json doc = Json::parse(read(f.path()), nullptr);
    check(doc.find("input") != nullptr &&
              *doc.find("input") == Json::parse(R"({"keyboard": {"cross": ["X", "V"]}, "hotkeys": {"pause": [["pad:guide", "pad:start"]]}})",
                                                nullptr),
          "only the changed bindings are written");
    SettingsFile g;
    g.load(dir);
    check(g.values.keys_for("cross") == std::vector<std::string>({ "X", "V" }) &&
              g.values.hotkey_for("pause") == Binding({ { "pad:guide", "pad:start" } }) && g.messages().empty(),
          "the bindings read back");
    g.values.keyboard.clear();
    g.values.hotkeys.clear();
    check(g.save(&err) && Json::parse(read(g.path()), nullptr).find("input") == nullptr,
          "a reset removes the input object the launcher made");
    // A bad binding in the file: a warning, the default.
    write(g.path(), R"({"schema": 1, "input": {"keyboard": {"cross": "Nope", "circle": "V"}, "future": 1}})");
    SettingsFile h;
    h.load(dir);
    check(h.messages().size() == 1 && h.values.keys_for("cross") == std::vector<std::string>({ "X" }) &&
              h.values.keys_for("circle") == std::vector<std::string>({ "V" }),
          "a bad key name: a warning and the default");
    h.values.keyboard["circle"] = { "B" };
    check(h.save(&err) && Json::parse(read(h.path()), nullptr).find("input")->find("future") != nullptr,
          "unknown members of input are kept");

    // The capture, fed events.
    InputCapture c;
    Trigger t;
    c.begin(InputCapture::Kind::Key);
    c.feed(pad_button_event(SDL_GAMEPAD_BUTTON_SOUTH, true));
    check(c.active(), "a key capture ignores the gamepad");
    c.feed(key_event(SDL_SCANCODE_V, true));
    check(c.take_done(&t) && t == Trigger({ "V" }) && !c.active(), "a key capture: the first key");
    c.begin(InputCapture::Kind::Pad);
    c.feed(pad_axis_event(SDL_GAMEPAD_AXIS_LEFTY, -30000));
    check(c.take_done(&t) && t == Trigger({ "lefty-" }), "a gamepad capture: a stick direction");
    c.begin(InputCapture::Kind::Pad);
    c.feed(pad_axis_event(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 20000));
    check(c.take_done(&t) && t == Trigger({ "righttrigger" }), "a gamepad capture: a trigger");
    c.begin(InputCapture::Kind::Binding);
    c.feed(pad_button_event(SDL_GAMEPAD_BUTTON_GUIDE, true));
    c.feed(pad_button_event(SDL_GAMEPAD_BUTTON_SOUTH, true));
    check(!c.take_done(&t) && c.held() == Trigger({ "pad:guide", "pad:south" }), "a chord builds while held");
    c.feed(pad_button_event(SDL_GAMEPAD_BUTTON_SOUTH, false));
    c.feed(pad_button_event(SDL_GAMEPAD_BUTTON_GUIDE, false));
    check(c.take_done(&t) && t == Trigger({ "pad:guide", "pad:south" }), "a chord: everything held, once released");
    c.begin(InputCapture::Kind::Binding);
    c.feed(key_event(SDL_SCANCODE_F2, true));
    c.feed(key_event(SDL_SCANCODE_F2, false));
    check(c.take_done(&t) && t == Trigger({ "F2" }), "a binding capture: one key");
    c.begin(InputCapture::Kind::Binding);
    for (SDL_Scancode k : { SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E }) {
        c.feed(key_event(k, true));
    }
    for (SDL_Scancode k : { SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E }) {
        c.feed(key_event(k, false));
    }
    check(c.take_done(&t) && t == Trigger({ "A", "B", "C", "D" }), "a chord stops at the game's 4 inputs");
    c.begin(InputCapture::Kind::Binding);
    c.feed(key_event(SDL_SCANCODE_ESCAPE, true));
    c.feed(key_event(SDL_SCANCODE_ESCAPE, false));
    check(!c.active() && !c.take_done(&t), "Escape cancels");
}

// ---- the mods

// Path B's manifest (port/mods/fast_forward/mod.json, PR #11) as it was, the plan's example (4.4), one with every
// option type, and three that cannot be used.
static const char FAST_FORWARD_MANIFEST[] = R"({
  "schema": 1,
  "id": "fast_forward",
  "name": "Fast-forward",
  "version": "0.1",
  "kind": "builtin",
  "requires_port": 1,
  "description": "Runs the game faster while a key is held, or until it is pressed again. The game itself is unchanged: only the wall clock is faster.",
  "options": [
    { "id": "hold", "name": "Hold", "description": "Fast-forward while this is held.", "type": "binding",
      "default": "Tab", "group": "Controls", "applies": "live" },
    { "id": "toggle", "name": "Toggle", "description": "Fast-forward on or off.", "type": "binding",
      "default": "", "group": "Controls", "applies": "live" },
    { "id": "speed", "name": "Speed", "description": "How fast, as a multiple of the normal speed (50 frames a second, or 60).",
      "type": "enum", "default": "4x", "applies": "live",
      "values": [
        { "id": "2x", "label": "2x" }, { "id": "3x", "label": "3x" }, { "id": "4x", "label": "4x" },
        { "id": "6x", "label": "6x" }, { "id": "8x", "label": "8x" }, { "id": "unlimited", "label": "Unlimited" }
      ] },
    { "id": "mute", "name": "Mute", "description": "No sound while fast-forwarding.", "type": "bool",
      "default": true, "applies": "live" }
  ]
}
)";

static const char SKIP_DIALOGUES_MANIFEST[] = R"({
  "schema": 1, "id": "skip_dialogues", "name": "Skip dialogues", "version": "1.0", "kind": "builtin",
  "description": "Text appears at once and advances by itself. Choices still wait for you.",
  "options": [
    { "id": "toggle", "name": "Toggle", "type": "binding", "default": "F2" },
    { "id": "hold",   "name": "Hold",   "type": "binding", "default": "" },
    { "id": "fast_forward_waits", "name": "Also fast-forward cutscene waits", "type": "bool", "default": false,
      "applies": "live" }
  ]
}
)";

static const char EVERY_TYPE_MANIFEST[] = R"({
  "schema": 1, "id": "every_type", "name": "Every option type", "version": "2.3", "kind": "builtin",
  "requires_port": 1, "description": "A test manifest: one option of each type.",
  "options": [
    { "id": "count", "name": "Count", "type": "int", "min": 1, "max": 9, "default": 3, "group": "Numbers" },
    { "id": "free", "name": "Unbounded", "type": "int", "step": 5, "default": 10, "group": "Numbers" },
    { "id": "ratio", "name": "Ratio", "type": "float", "min": 0, "max": 1, "step": 0.05, "default": 0.5,
      "group": "Numbers", "applies": "restart" },
    { "id": "mode", "name": "Mode", "type": "enum", "values": [ { "id": "a", "label": "First" }, { "id": "b" } ],
      "default": "b" },
    { "id": "flag", "name": "Flag", "type": "bool", "default": true, "description": "A checkbox." },
    { "id": "chord", "name": "Chord", "type": "binding", "default": [["pad:guide", "pad:south"], "F9"],
      "group": "Controls" }
  ]
}
)";

static void write_mod(const std::string &mods, const std::string &id, const std::string &text) {
    path_make_dir(path_join(mods, id), nullptr);
    write(path_join(path_join(mods, id), "mod.json"), text);
}

// A game directory with a stand-in game file and the mods above; returns the game's path.
static std::string make_game_dir(const std::string &root) {
    const std::string dir = path_join(root, "gamedir"), mods = path_join(dir, "mods");
    path_make_dir(mods, nullptr);
    write(path_join(dir, "dw2003"), "");
    write_mod(mods, "fast_forward", FAST_FORWARD_MANIFEST);
    write_mod(mods, "skip_dialogues", SKIP_DIALOGUES_MANIFEST);
    write_mod(mods, "every_type", EVERY_TYPE_MANIFEST);
    write_mod(mods, "broken", "{ \"schema\": 1, ");
    write_mod(mods, "elsewhere", R"({"schema": 1, "id": "other", "name": "Wrong id", "kind": "builtin"})");
    write_mod(mods, "textures", R"({"schema": 1, "id": "textures", "name": "Texture pack", "kind": "data"})");
    return path_join(dir, "dw2003");
}

static void test_mods(const std::string &root) {
    const std::string game = make_game_dir(root);
    std::vector<ModManifest> mods = mods_scan(path_join(path_dir(game), "mods"));
    check(mods.size() == 6, "six manifests found: " + std::to_string(mods.size()));
    auto find = [&](const std::string &id) -> const ModManifest * {
        for (const ModManifest &m : mods) {
            if (m.id == id) {
                return &m;
            }
        }
        return nullptr;
    };
    const ModManifest *ff = find("fast_forward"), *every = find("every_type");
    check(ff != nullptr && ff->error.empty() && ff->options.size() == 4 && ff->requires_port == 1 &&
              ff->option("speed")->type == ModOption::Type::Enum && ff->option("speed")->values.size() == 6 &&
              ff->option("hold")->group == "Controls",
          "path B's fast_forward manifest reads: " + (ff != nullptr ? ff->error : std::string("missing")));
    check(find("skip_dialogues") != nullptr && find("skip_dialogues")->error.empty(), "the plan's example reads");
    check(every != nullptr && every->error.empty() && every->option("ratio")->restart &&
              every->option("ratio")->step == 0.05 && every->option("mode")->values[1].label == "b",
          "every option type reads: " + (every != nullptr ? every->error : std::string()));
    check(find("broken") != nullptr && !find("broken")->error.empty(), "a manifest that is not JSON is listed, unusable");
    check(find("elsewhere") != nullptr && find("elsewhere")->error.find("directory") != std::string::npos,
          "an id that is not its directory's name");
    check(find("textures") != nullptr && find("textures")->error.find("built-in") != std::string::npos,
          "a data mod: not yet");
    std::string err;
    check(!mod_manifest_load(path_join(root, "nowhere")).error.empty(), "a missing manifest");

    // The values: only changes written; "enabled" false is the default.
    Json doc = Json::object();
    ModValues v(&doc);
    check(!v.enabled("fast_forward") && v.value(*ff, *ff->option("speed")) == Json::string("4x"), "defaults");
    v.set_enabled("fast_forward", true);
    v.set(*ff, *ff->option("speed"), Json::string("8x"));
    check(doc == Json::parse(R"({"mods": {"fast_forward": {"enabled": true, "speed": "8x"}}})", nullptr),
          "enabled and a changed option are written: " + doc.dump());
    v.set(*ff, *ff->option("speed"), Json::string("4x"));
    check(doc == Json::parse(R"({"mods": {"fast_forward": {"enabled": true}}})", nullptr),
          "an option set back to its default is removed");
    v.set(*ff, *ff->option("mute"), Json::boolean(false));
    v.set_enabled("fast_forward", false);
    check(doc == Json::parse(R"({"mods": {"fast_forward": {"mute": false, "enabled": false}}})", nullptr),
          "a mod turned off keeps its options, and says it is off: " + doc.dump());
    v.reset("fast_forward", "mute");
    check(doc == Json::object(), "nothing left: the mods object goes");
    // A value the manifest rejects: reported, the default used; unknown mods and options are kept.
    doc = Json::parse(R"({"mods": {"fast_forward": {"enabled": true, "speed": "9x", "future": 1}, "other": {"enabled": true}}})",
                      nullptr);
    check(v.stored(*ff, *ff->option("speed"), &err) == nullptr && err.find("speed") != std::string::npos &&
              v.value(*ff, *ff->option("speed")) == Json::string("4x"),
          "an invalid stored value: the default, and why");
    v.set(*ff, *ff->option("speed"), Json::string("2x"));
    check(doc.find("mods")->find("other") != nullptr && doc.find("mods")->find("fast_forward")->find("future") != nullptr,
          "unknown mods and options are kept");
    // The option types' checks.
    const ModOption &count = *every->option("count");
    check(count.valid(Json::number(9), &err) && !count.valid(Json::number(10), &err) &&
              !count.valid(Json::number(2.5), &err) && !count.valid(Json::string("3"), &err),
          "int: whole, in range");
    check(every->option("chord")->valid(Json::parse(R"([["pad:guide", "pad:south"], "F9"])", nullptr), &err) &&
              mod_binding(v, *every, *every->option("chord")).size() == 2,
          "a binding option's default chord");
}

// ---- the disc check

static DiscCheck::State wait_check(DiscCheck &c) {
    for (int i = 0; i < 120000 / 5 && c.poll() == DiscCheck::State::Running; i++) {
        SDL_Delay(5);
    }
    return c.state();
}

// A file of the EU BIN's exact size without its data (sparse where the filesystem allows): with the right disc.sha1 in
// the settings the launcher counts it as verified (it never hashes a verified disc again), so the play path can be
// tested without the disc.
static bool write_sized_bin(const std::string &path) {
    SDL_IOStream *io = SDL_IOFromFile(path.c_str(), "wb");
    if (io == nullptr) {
        return false;
    }
    bool ok = SDL_SeekIO(io, (Sint64)DISC_BIN_SIZE - 1, SDL_IO_SEEK_SET) >= 0 && SDL_WriteU8(io, 0);
    return SDL_CloseIO(io) && ok;
}

static void test_disc(const std::string &root) {
    std::string bin, err;
    path_make_dir(path_join(root, "sub dir"), nullptr);
    write(path_join(root, "a.cue"), "FILE \"sub dir/Game (Europe).bin\" BINARY\r\n  TRACK 01 MODE2/2352\r\n");
    check(disc_bin_path(path_join(root, "a.cue"), &bin, &err) && bin == path_join(root, "sub dir/Game (Europe).bin"),
          "a .cue: a quoted relative FILE, CRLF lines: " + bin + err);
    write(path_join(root, "b.CUE"), "REM x\n\tFILE b.bin BINARY\n");
    check(disc_bin_path(path_join(root, "b.CUE"), &bin, &err) && bin == path_join(root, "b.bin"),
          "a .CUE: an unquoted FILE after a tab");
    write(path_join(root, "c.cue"), "FILE \"/abs/c.bin\" BINARY\n");
    check(disc_bin_path(path_join(root, "c.cue"), &bin, &err) && bin == "/abs/c.bin", "a .cue: an absolute FILE");
    write(path_join(root, "d.cue"), "TRACK 01 MODE2/2352\n");
    check(!disc_bin_path(path_join(root, "d.cue"), &bin, &err) && err.find("no FILE") != std::string::npos,
          "a .cue without a FILE line");
    check(disc_bin_path(path_join(root, "x.bin"), &bin, &err) && bin == path_join(root, "x.bin"), "a .bin is itself");

    // The check over a small file: the SHA-1 of "abc" (FIPS 180-4's example), and the wrong disc.
    write(path_join(root, "abc.bin"), "abc");
    DiscCheck c;
    c.start(path_join(root, "abc.bin"));
    check(wait_check(c) == DiscCheck::State::Failed && c.sha1() == "a9993e364706816aba3e25717850c26c9cd0d89d" &&
              c.message().find("not the unpatched European disc") != std::string::npos,
          "the check of a wrong file: its SHA-1 and the message");
    c.start(path_join(root, "a.cue"));
    check(wait_check(c) == DiscCheck::State::Failed && c.message().find("cannot open") != std::string::npos &&
              c.sha1().empty(),
          "the check of a .cue whose BIN is missing");
    // The real disc, when the environment names one (DW3_SELFTEST_DISC: a .cue or .bin of the EU disc).
    if (const char *disc = SDL_getenv("DW3_SELFTEST_DISC")) {
        Uint64 t0 = SDL_GetTicks();
        c.start(disc);
        check(wait_check(c) == DiscCheck::State::Passed && c.sha1() == DISC_SHA1, "the real disc passes: " + c.message());
        std::fprintf(stderr, "self-test: the real disc checked in %.1f s\n", (SDL_GetTicks() - t0) / 1000.0);
    }
}

// ---- the game's stand-in and the launch path

int self_test_fake_game(const char *mode, int argc, char **argv) {
    bool config = false, print = false;
    for (int i = 1; i < argc; i++) {
        config |= std::strcmp(argv[i], "--config") == 0;
        print |= std::strcmp(argv[i], "--print-settings") == 0;
    }
    if (std::strcmp(mode, "old") == 0) { // a game before --config: rejects it like any unknown option
        if (config) {
            std::fprintf(stderr, "usage: %s [--disc CUE|BIN] [--no-disc-check] ...\n", argv[0]);
            return 64;
        }
        for (int i = 1; i < argc; i++) {
            std::printf("arg %s\n", argv[i]);
        }
        return 0;
    }
    if (std::strcmp(mode, "invalid") == 0 && print) {
        std::fprintf(stderr, "port: settings %s: video.scale: an integer from 1 to 16, not 17\n", argv[2]);
        return 64;
    }
    if (std::strcmp(mode, "abort") == 0) {
        std::abort();
    }
    if (print) {
        std::puts("{}");
        return 0;
    }
    if (std::strcmp(mode, "fail") == 0) { // more lines than the launcher keeps, then a fatal error
        for (int i = 0; i < 250; i++) {
            std::printf("line %d\n", i);
        }
        std::fflush(stdout);
        std::fprintf(stderr, "port: fatal: the stand-in's error");
        return 1;
    }
    std::puts("port: start");
    return 0;
}

static std::string self_exe() {
    const char *base = SDL_GetBasePath();
#ifdef SDL_PLATFORM_WINDOWS
    return path_join(base != nullptr ? base : "", "dw2003-launcher.exe");
#else
    return path_join(base != nullptr ? base : "", "dw2003-launcher");
#endif
}

static void set_fake_mode(const char *mode) {
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(), SELF_TEST_GAME_ENV, mode, true);
}

static void test_game(const std::string &root) {
    const std::string exe = self_exe();
    const std::string settings = path_join(root, SETTINGS_FILE);
    path_make_dir(root, nullptr);
    write(settings, "{\"schema\": 1}\n");
    check(path_is_file(exe), "the launcher's own executable: " + exe);

    std::vector<std::string> tried;
    check(game_find(exe, "/nowhere", &tried) == exe && tried.size() == 1, "--game is the only place looked at");
    tried.clear();
    check(game_find("", "/nowhere/launcher", &tried).empty() && tried.back() == "/nowhere/port-sdl/dw2003",
          "the development tree's SDL build is looked at last");

    set_fake_mode("new");
    check(game_probe(exe, settings).result == GameProbe::Result::Valid, "the probe: a game with --config");
    set_fake_mode("old");
    check(game_probe(exe, settings).result == GameProbe::Result::NoConfig, "the probe: a game without --config");
    set_fake_mode("invalid");
    GameProbe p = game_probe(exe, settings);
    check(p.result == GameProbe::Result::Invalid && p.message.find("video.scale") != std::string::npos,
          "the probe: a file the game rejects, with its message");
    set_fake_mode("abort");
    p = game_probe(exe, settings);
    check(p.result == GameProbe::Result::Failed && p.message.find("signal 6") != std::string::npos,
          "the probe: a game that crashes: " + p.message);

    // A run that writes 251 lines and fails: the status, the last lines kept, the line without a newline.
    set_fake_mode("fail");
    GameRun run;
    std::string err;
    check(run.start({ exe, "--config", settings }, root, &err, false), "the stand-in starts: " + err);
    for (int i = 0; i < 2000 && run.poll(); i++) {
        SDL_Delay(5);
    }
    check(!run.running() && run.exit_code() == 1, "the run's exit status");
    check(run.lines().size() == GameRun::kept_lines && run.lines().back() == "port: fatal: the stand-in's error" &&
              run.lines().front() == "line 51",
          "the last 200 lines are kept, the unterminated last one too");

    SettingsFile f;
    f.load(path_join(root, "command"));
    check(game_args("g", f) == std::vector<std::string>({ "g", "--config", f.path() }), "the --config command");
    SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), SELF_TEST_GAME_ENV);

    // The real game, when the environment names it (DW3_SELFTEST_GAME: an SDL build of dw2003): the probe must accept
    // the file; with DW3_SELFTEST_DISC too, the game runs 300 frames from the launcher's
    // command (offscreen video, no audio device, unthrottled) and must end normally with the disc checked.
    const char *real = SDL_getenv("DW3_SELFTEST_GAME");
    if (real == nullptr) {
        return;
    }
    const std::string real_dir = path_join(root, "real");
    SettingsFile r;
    r.load(real_dir);
    if (const char *disc = SDL_getenv("DW3_SELFTEST_DISC")) {
        r.values.disc_path = disc;
        r.values.disc_sha1 = DISC_SHA1;
    }
    // What the controls screen writes, for the game's own parser: rebinds, a chord, an unbound hotkey.
    r.values.keyboard["cross"] = { "X", "V" };
    r.values.gamepad["circle"] = { "east", "rightx+" };
    r.values.gamepad["select"] = {};
    r.values.hotkeys["pause"] = { { "pad:guide", "pad:start" }, { "P" } };
    r.values.hotkeys["fullscreen"] = {};
    // Every mod beside the game turned on, every option off its default (the game's own checks of mods.<id>).
    int mods_set = 0;
    ModValues mv(&r.doc);
    for (const ModManifest &m : mods_scan(path_join(path_dir(real), "mods"))) {
        check(m.error.empty(), "the game's manifest " + m.id + ": " + m.error);
        if (!m.error.empty()) {
            continue;
        }
        mv.set_enabled(m.id, true);
        for (const ModOption &o : m.options) {
            Json v;
            switch (o.type) {
            case ModOption::Type::Bool:
                v = Json::boolean(!o.def.as_bool(false));
                break;
            case ModOption::Type::Int:
            case ModOption::Type::Float:
                v = Json::number(o.has_max ? o.max : o.has_min ? o.min : o.def.as_number(0) + 1);
                break;
            case ModOption::Type::Enum:
                v = Json::string(o.values.back().id == o.def.as_string("") ? o.values.front().id
                                                                             : o.values.back().id);
                break;
            case ModOption::Type::Binding:
                v = Json::parse(R"([["pad:guide", "pad:north"], "F9"])", nullptr);
                break;
            }
            mv.set(m, o, v);
            mods_set++;
        }
    }
    check(r.save(&err), "the real game's settings: " + err);
    p = game_probe(real, r.path());
    check(p.result == GameProbe::Result::Valid, "the real game accepts the launcher's file: " + p.message);
    std::fprintf(stderr, "self-test: the real game accepts the file (%d mod options set)\n", mods_set);
    if (r.values.disc_path.empty() || p.result != GameProbe::Result::Valid) {
        return;
    }
    std::vector<std::string> args = game_args(real, r);
    args.insert(args.end(), { "--max-frames", "300", "--fps", "0" });
    SDL_Environment *env = SDL_GetEnvironment();
    SDL_SetEnvironmentVariable(env, "SDL_VIDEO_DRIVER", "offscreen", true);
    SDL_SetEnvironmentVariable(env, "SDL_AUDIO_DRIVER", "dummy", true);
    GameRun game;
    check(game.start(args, r.dir(), &err, false), "the real game starts: " + err);
    for (int i = 0; i < 120000 / 10 && game.poll(); i++) {
        SDL_Delay(10);
    }
    bool disc_ok = false;
    for (const std::string &l : game.lines()) {
        disc_ok |= l.find("(the EU disc)") != std::string::npos;
    }
    check(!game.running() && game.exit_code() == 0 && disc_ok,
          "the real game runs 300 frames from the launcher's command: it " + game_exit_text(game.exit_code()) +
              (game.lines().empty() ? std::string() : "; last line: " + game.lines().back()));
    check(path_is_file(path_join(real_dir, "card1.mcd")), "the game made memory card 1 in the settings directory");
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

static void pump_until(App &app, const std::function<bool()> &done, int max_ms) {
    for (int t = 0; t < max_ms && !done(); t += 5) {
        pump(app, 1);
        SDL_Delay(5);
    }
}

// `path` must stay valid until the event has been handled (SDL_PushEvent does not copy it).
static void drop_file(SDL_Window *w, const std::string &path) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_DROP_FILE;
    e.drop.windowID = SDL_GetWindowID(w);
    e.drop.data = path.c_str();
    SDL_PushEvent(&e);
}

// The play path with the game's stand-in and a verified disc (a file of the BIN's size and the right disc.sha1).
static void test_play(SDL_Window *window, const std::string &root) {
    const std::string dir = path_join(root, "play"), shots = path_join(root, "screens");
    path_make_dir(path_join(dir, "disc"), nullptr);
    if (!write_sized_bin(path_join(dir, "disc/dw2003.bin"))) {
        check(false, std::string("a file of the BIN's size: ") + SDL_GetError());
        return;
    }
    write(path_join(dir, "disc/dw2003.cue"), "FILE \"dw2003.bin\" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n");
    write(path_join(dir, SETTINGS_FILE), std::string("{\"schema\": 1, \"disc\": {\"path\": \"disc/dw2003.cue\", \"sha1\": \"") +
                                             DISC_SHA1 + "\"}}\n");
    SettingsDir location;
    location.dir = dir;
    location.source = DirSource::Argument;
    AppOptions options;
    options.game = self_exe();
    options.echo_game = false;
    App app(window, SDL_GetRenderer(window), location, options);
    check(app.disc_status() == DiscStatus::Verified && app.screen() == Screen::Play,
          "a verified disc: the launcher opens on Play");
    pump(app, 2);

    // The game ends with an error: the window comes back with the status and the last lines.
    set_fake_mode("fail");
    app.play();
    check(app.game_run().running() || !app.play_error().empty(), "Play starts the game: " + app.play_error());
    check((SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN) != 0, "the launcher hides while the game runs");
    pump_until(app, [&] { return !app.game_run().running(); }, 10000);
    check(app.play_error().find("fatal error (status 1)") != std::string::npos, "the exit status is shown: " +
                                                                                   app.play_error());
    check((SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN) == 0, "the launcher comes back when the game ends");
    pump(app, 2);
    std::string png = path_join(shots, "5-Play-error.png");
    app.frame(png.c_str());

    // A game too old for --config: told so, nothing started.
    set_fake_mode("old");
    app.play();
    check(!app.game_run().running() && app.play_error().find("too old") != std::string::npos,
          "a game without --config is reported: " + app.play_error());

    // The game rejects the file: shown, nothing started.
    set_fake_mode("invalid");
    app.play();
    check(!app.game_run().running() && app.play_error().find("video.scale") != std::string::npos,
          "a file the game rejects is reported: " + app.play_error());
    SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), SELF_TEST_GAME_ENV);

    // The controls: Cross gets V through the prompt (injected key events), the file has it.
    app.set_screen(Screen::Controls);
    app.capture_for(0, "cross");
    pump(app, 2);
    png = path_join(shots, "7-Controls-prompt.png");
    app.frame(png.c_str());
    push_key(window, SDL_SCANCODE_V, SDL_KMOD_NONE, true);
    pump(app, 1);
    push_key(window, SDL_SCANCODE_V, SDL_KMOD_NONE, false);
    pump(app, 3);
    check(!app.capture().active() &&
              app.settings().values.keys_for("cross") == std::vector<std::string>({ "X", "V" }),
          "the prompt adds a key");
    Json written = Json::parse(read(app.settings().path()), nullptr);
    const Json *kb = written.find("input") != nullptr ? written.find("input")->find("keyboard") : nullptr;
    check(kb != nullptr && kb->find("cross") != nullptr && *kb->find("cross") == Json::parse(R"(["X", "V"])", nullptr),
          "the new binding is saved");
    // The same key on Circle is marked as a conflict (a picture), Escape cancels the prompt.
    app.settings().values.keyboard["circle"] = { "C", "V" };
    pump(app, 2);
    png = path_join(shots, "8-Controls-keyboard.png");
    app.frame(png.c_str());
    app.capture_for(2, "pause");
    pump(app, 1);
    push_key(window, SDL_SCANCODE_ESCAPE, SDL_KMOD_NONE, true);
    pump(app, 1);
    push_key(window, SDL_SCANCODE_ESCAPE, SDL_KMOD_NONE, false);
    pump(app, 2);
    check(!app.capture().active() && app.settings().values.hotkeys.count("pause") == 0, "Escape leaves the hotkey");
    app.set_screen(Screen::Settings);
    pump(app, 2);
    png = path_join(shots, "9-Settings.png");
    app.frame(png.c_str());

    // A wrong file dropped on the window: checked, refused, the verified disc stays.
    write(path_join(dir, "wrong.bin"), "abc");
    const std::string wrong = path_join(dir, "wrong.bin");
    drop_file(window, wrong);
    pump(app, 1);
    check(app.screen() == Screen::Disc, "a dropped file opens the disc screen");
    pump_until(app, [&] { return app.disc_check().state() != DiscCheck::State::Running; }, 10000);
    check(app.disc_message().find("not the unpatched European disc") != std::string::npos,
          "a wrong file is refused: " + app.disc_message());
    check(app.settings().values.disc_path == "disc/dw2003.cue" && app.disc_status() == DiscStatus::Verified,
          "the verified disc stays");
    pump(app, 2);
    png = path_join(shots, "6-Disc-wrong-file.png");
    app.frame(png.c_str());
    // "Check again" on the stored disc (here a file of zeros): it fails, the disc loses its SHA-1 and Play its go, the
    // path (relative, as written) stays to be shown.
    app.choose_disc(app.settings().resolve("disc/dw2003.cue"));
    pump_until(app, [&] { return app.disc_check().state() != DiscCheck::State::Running; }, 30000);
    check(app.disc_status() == DiscStatus::Unverified && app.settings().values.disc_sha1.empty() &&
              app.settings().values.disc_path == "disc/dw2003.cue",
          "a stored disc that fails a new check loses its SHA-1");
    // A typed path that does not exist.
    app.choose_disc("  \"" + path_join(dir, "nothing.cue") + "\"  ");
    check(app.disc_message().find("cannot read") != std::string::npos, "a typed path that does not exist: " +
                                                                            app.disc_message());
}

// The mods screen over the fixtures: a rebind of a mod's key through the prompt, a picture of each kind of page.
static void test_mods_window(SDL_Window *window, const std::string &root) {
    const std::string shots = path_join(root, "screens");
    SettingsDir location;
    location.dir = path_join(root, "modsui");
    location.source = DirSource::Argument;
    AppOptions options;
    options.game = make_game_dir(path_join(root, "modsui-game"));
    App app(window, SDL_GetRenderer(window), location, options);
    check(app.mods().size() == 6 && app.mods_dir() == path_join(path_dir(options.game), "mods"),
          "the launcher finds the mods beside the game");
    app.set_screen(Screen::Mods);
    app.select_mod("fast_forward");
    pump(app, 3);
    std::string png = path_join(shots, "10-Mods-fast_forward.png");
    app.frame(png.c_str());
    // Toggle gets X through the prompt: the file has it, and X is marked (Cross uses it).
    app.capture_for_mod("fast_forward", "toggle");
    pump(app, 1);
    push_key(window, SDL_SCANCODE_X, SDL_KMOD_NONE, true);
    pump(app, 1);
    push_key(window, SDL_SCANCODE_X, SDL_KMOD_NONE, false);
    pump(app, 3);
    Json written = Json::parse(read(path_join(location.dir, SETTINGS_FILE)), nullptr);
    const Json *ff = written.find("mods") != nullptr ? written.find("mods")->find("fast_forward") : nullptr;
    check(ff != nullptr && ff->find("toggle") != nullptr && *ff->find("toggle") == Json::string("X") &&
              ff->find("enabled") == nullptr,
          "a mod's binding through the prompt is saved (and the mod stays off)");
    pump(app, 2);
    png = path_join(shots, "11-Mods-binding-conflict.png");
    app.frame(png.c_str());
    app.select_mod("every_type");
    pump(app, 3);
    png = path_join(shots, "12-Mods-every-type.png");
    app.frame(png.c_str());
    app.select_mod("broken");
    pump(app, 3);
    png = path_join(shots, "13-Mods-broken.png");
    app.frame(png.c_str());
    check(app.last_error().empty(), "no error in the status bar: " + app.last_error());
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
    test_play(window, root);
    test_mods_window(window, root);
    gui_close(window, renderer);
}

bool self_test_run(const std::string &dir) {
    std::string err;
    // Absolute: the lookup resolves a relative --config-dir against the current directory, and the game's stand-in
    // runs in another one.
    std::string base = path_strip_slash(dir);
    if (!path_is_absolute(base)) {
        if (char *cwd = SDL_GetCurrentDirectory()) {
            base = path_join(cwd, base);
            SDL_free(cwd);
        }
    }
    const std::string root = path_join(base, "launcher-self-test");
    remove_tree(root);
    if (!path_make_dir(root, &err)) {
        std::fprintf(stderr, "self-test: %s\n", err.c_str());
        return false;
    }
    test_paths();
    test_json();
    test_lookup(path_join(root, "lookup"));
    test_settings_file(path_join(root, "files"));
    test_input(path_join(root, "controls"));
    test_mods(path_join(root, "mods"));
    test_disc(path_join(root, "disc"));
    test_game(path_join(root, "game"));
    test_window(root);
    std::fprintf(stderr, "self-test: %d of %d checks passed; pictures in %s\n", checks - failures, checks,
                 path_join(root, "screens").c_str());
    return failures == 0;
}

} // namespace dw3
