#include "settings.h"

#include <SDL3/SDL.h>

#include "paths.h"

namespace dw3 {

const char *dir_source_name(DirSource s) {
    switch (s) {
    case DirSource::Argument:
        return "--config-dir";
    case DirSource::Environment:
        return SETTINGS_DIR_ENV;
    case DirSource::Portable:
        return "portable mode (portable.txt beside the launcher)";
    case DirSource::CurrentDir:
        return "the current directory (it has a settings.json)";
    case DirSource::User:
        return "the per-user directory";
    }
    return "?";
}

DirLookup dir_lookup_from_system(const std::string &arg) {
    DirLookup in;
    in.arg = arg;
    if (const char *env = SDL_getenv(SETTINGS_DIR_ENV)) {
        in.env = env;
    }
    if (const char *base = SDL_GetBasePath()) { // owned by SDL
        in.exe_dir = path_strip_slash(base);
    }
    if (char *cwd = SDL_GetCurrentDirectory()) {
        in.cwd = path_strip_slash(cwd);
        SDL_free(cwd);
    }
    in.user_dir = [] {
        std::string dir;
        if (char *pref = SDL_GetPrefPath(SETTINGS_PREF_ORG, SETTINGS_PREF_APP)) { // creates it
            dir = path_strip_slash(pref);
            SDL_free(pref);
        }
        return dir;
    };
    return in;
}

SettingsDir settings_dir_choose(const DirLookup &in) {
    SettingsDir out;
    if (!in.arg.empty() || !in.env.empty()) {
        out.source = !in.arg.empty() ? DirSource::Argument : DirSource::Environment;
        const std::string &d = !in.arg.empty() ? in.arg : in.env;
        out.dir = path_strip_slash(path_resolve(in.cwd, d));
        return out;
    }
    if (!in.exe_dir.empty() && path_is_file(path_join(in.exe_dir, SETTINGS_PORTABLE_FILE))) {
        out.source = DirSource::Portable;
        out.dir = in.exe_dir;
        return out;
    }
    if (!in.cwd.empty() && path_is_file(path_join(in.cwd, SETTINGS_FILE))) {
        out.source = DirSource::CurrentDir;
        out.dir = in.cwd;
        return out;
    }
    out.source = DirSource::User;
    out.dir = in.user_dir ? in.user_dir() : "";
    if (out.dir.empty()) {
        out.error = std::string("no per-user directory: ") + SDL_GetError();
    }
    return out;
}

// ---- the values

static void warn(std::vector<std::string> *w, const std::string &msg) {
    if (w != nullptr) {
        w->push_back(msg);
    }
}

// A member that is present but has the wrong type or range: a warning, and the default stays.
static void read_int(const Json *obj, const char *where, const char *key, int lo, int hi, int *v,
                     std::vector<std::string> *w) {
    const Json *m = obj != nullptr ? obj->find(key) : nullptr;
    if (m == nullptr) {
        return;
    }
    int x = m->as_int(lo - 1, lo, hi);
    if (x < lo) {
        warn(w, std::string(where) + "." + key + ": expected a whole number from " + std::to_string(lo) + " to " +
                    std::to_string(hi) + "; using " + std::to_string(*v));
        return;
    }
    *v = x;
}

static void read_bool(const Json *obj, const char *where, const char *key, bool *v, std::vector<std::string> *w) {
    const Json *m = obj != nullptr ? obj->find(key) : nullptr;
    if (m == nullptr) {
        return;
    }
    if (!m->is_bool()) {
        warn(w, std::string(where) + "." + key + ": expected true or false; using " + (*v ? "true" : "false"));
        return;
    }
    *v = m->as_bool(*v);
}

static void read_string(const Json *obj, const char *where, const char *key, std::string *v,
                        std::vector<std::string> *w) {
    const Json *m = obj != nullptr ? obj->find(key) : nullptr;
    if (m == nullptr) {
        return;
    }
    if (!m->is_string()) {
        warn(w, std::string(where) + (*where != '\0' ? "." : "") + key + ": expected a string; using \"" + *v + "\"");
        return;
    }
    *v = m->as_string();
}

static const Json *section(const Json &doc, const char *key, std::vector<std::string> *w) {
    const Json *s = doc.find(key);
    if (s != nullptr && !s->is_object()) {
        warn(w, std::string(key) + ": expected an object; using the defaults");
        return nullptr;
    }
    return s;
}

std::vector<std::string> Settings::keys_for(const std::string &button) const {
    auto it = keyboard.find(button);
    return it != keyboard.end() ? it->second : default_keys(button);
}

std::vector<std::string> Settings::pad_for(const std::string &button) const {
    auto it = gamepad.find(button);
    return it != gamepad.end() ? it->second : default_pad_inputs(button);
}

Binding Settings::hotkey_for(const std::string &action) const {
    auto it = hotkeys.find(action);
    return it != hotkeys.end() ? it->second : default_hotkey(action);
}

static void read_input(const Json &doc, Settings *s, std::vector<std::string> *w) {
    const Json *input = section(doc, "input", w);
    if (input == nullptr) {
        return;
    }
    for (int pad = 0; pad < 2; pad++) {
        const char *key = pad ? "gamepad" : "keyboard";
        const Json *map = input->find(key);
        if (map == nullptr) {
            continue;
        }
        if (!map->is_object()) {
            warn(w, std::string("input.") + key + ": expected an object; using the defaults");
            continue;
        }
        for (const PadButton &b : pad_buttons()) {
            const Json *v = map->find(b.id);
            std::vector<std::string> names;
            std::string err;
            if (v == nullptr) {
                continue;
            }
            if (!names_from_json(*v, pad != 0, &names, &err)) {
                warn(w, std::string("input.") + key + "." + b.id + ": " + err + "; using the default");
                continue;
            }
            (pad ? s->gamepad : s->keyboard)[b.id] = names;
        }
    }
    if (const Json *hotkeys = input->find("hotkeys")) {
        if (!hotkeys->is_object()) {
            warn(w, "input.hotkeys: expected an object; using the defaults");
            return;
        }
        for (const HotkeyAction &a : hotkey_actions()) {
            const Json *v = hotkeys->find(a.id);
            Binding b;
            std::string err;
            if (v == nullptr) {
                continue;
            }
            if (!binding_from_json(*v, &b, &err)) {
                warn(w, std::string("input.hotkeys.") + a.id + ": " + err + "; using the default");
                continue;
            }
            s->hotkeys[a.id] = b;
        }
    }
}

// The file's input: the known entries set or removed, everything else kept; an object left empty by the launcher is
// removed (a file never rebound has no input at all).
static void write_input(const Settings &s, Json *doc) {
    Json *input = doc->find("input");
    if (input == nullptr && s.keyboard.empty() && s.gamepad.empty() && s.hotkeys.empty()) {
        return;
    }
    if (input == nullptr || !input->is_object()) {
        doc->set("input", Json::object());
        input = doc->find("input");
    }
    for (int pad = 0; pad < 2; pad++) {
        const char *key = pad ? "gamepad" : "keyboard";
        const auto &values = pad ? s.gamepad : s.keyboard;
        Json *map = input->find(key);
        if (map == nullptr && values.empty()) {
            continue;
        }
        if (map == nullptr || !map->is_object()) {
            input->set(key, Json::object());
            map = input->find(key);
        }
        for (const PadButton &b : pad_buttons()) {
            auto it = values.find(b.id);
            if (it != values.end()) {
                map->set(b.id, names_to_json(it->second));
            } else {
                map->erase(b.id);
            }
        }
        if (map->members().empty()) {
            input->erase(key);
        }
    }
    Json *hotkeys = input->find("hotkeys");
    if (hotkeys != nullptr || !s.hotkeys.empty()) {
        if (hotkeys == nullptr || !hotkeys->is_object()) {
            input->set("hotkeys", Json::object());
            hotkeys = input->find("hotkeys");
        }
        for (const HotkeyAction &a : hotkey_actions()) {
            auto it = s.hotkeys.find(a.id);
            if (it != s.hotkeys.end()) {
                hotkeys->set(a.id, binding_to_json(it->second));
            } else {
                hotkeys->erase(a.id);
            }
        }
        if (hotkeys->members().empty()) {
            input->erase("hotkeys");
        }
    }
    if (input->members().empty()) {
        doc->erase("input");
    }
}

Settings settings_from_json(const Json &doc, std::vector<std::string> *w) {
    Settings s;
    if (const Json *disc = section(doc, "disc", w)) {
        read_string(disc, "disc", "path", &s.disc_path, w);
        read_string(disc, "disc", "sha1", &s.disc_sha1, w);
    }
    if (const Json *video = section(doc, "video", w)) {
        read_int(video, "video", "scale", 1, 16, &s.scale, w);
        read_bool(video, "video", "fullscreen", &s.fullscreen, w);
        read_int(video, "video", "refresh", 50, 60, &s.refresh, w);
        if (s.refresh != 50 && s.refresh != 60) {
            warn(w, "video.refresh: expected 50 or 60; using 50");
            s.refresh = 50;
        }
    }
    if (const Json *audio = section(doc, "audio", w)) {
        read_bool(audio, "audio", "mute", &s.mute, w);
    }
    read_input(doc, &s, w);
    if (const Json *launcher = section(doc, "launcher", w)) {
        read_string(launcher, "launcher", "last_dir", &s.last_dir, w);
    }
    for (int i = 0; i < 2; i++) {
        const char *key = i == 0 ? "memcard1" : "memcard2";
        const Json *m = doc.find(key);
        if (m == nullptr) {
            continue;
        }
        if (m->is_null()) {
            s.memcard[i].present = false;
        } else if (m->is_string() && !m->as_string().empty()) {
            s.memcard[i].path = m->as_string();
        } else {
            warn(w, std::string(key) + ": expected a file name or null; using \"" + s.memcard[i].path + "\"");
        }
    }
    return s;
}

void settings_to_json(const Settings &s, Json *doc) {
    if (!doc->is_object()) {
        *doc = Json::object();
    }
    doc->set("schema", Json::number(SETTINGS_SCHEMA));
    Json &disc = doc->member("disc");
    disc.set("path", Json::string(s.disc_path));
    disc.set("sha1", Json::string(s.disc_sha1));
    Json &video = doc->member("video");
    video.set("scale", Json::number(s.scale));
    video.set("fullscreen", Json::boolean(s.fullscreen));
    video.set("refresh", Json::number(s.refresh));
    doc->member("audio").set("mute", Json::boolean(s.mute));
    for (int i = 0; i < 2; i++) {
        const MemoryCard &c = s.memcard[i];
        doc->set(i == 0 ? "memcard1" : "memcard2", c.present ? Json::string(c.path) : Json());
    }
    write_input(s, doc);
    if (!s.last_dir.empty() || doc->find("launcher") != nullptr) {
        doc->member("launcher").set("last_dir", Json::string(s.last_dir));
    }
}

// ---- the file

void SettingsFile::load(const std::string &dir) {
    dir_ = dir;
    path_ = path_join(dir, SETTINGS_FILE);
    messages_.clear();
    written_.clear();
    doc = Json::object();
    values = Settings();
    state_ = State::New;
    if (!path_is_file(path_)) {
        return;
    }
    std::string text, err;
    if (!file_read(path_, &text, &err)) {
        state_ = State::Broken;
        messages_.push_back(err);
        return;
    }
    Json j = Json::parse(text, &err);
    if (!j.is_object()) {
        state_ = State::Broken;
        messages_.push_back(path_ + ": " + (err.empty() ? std::string("not a JSON object") : err) +
                            "; the defaults are used, and the first save keeps the old file as " + SETTINGS_FILE +
                            ".broken");
        return;
    }
    const Json *schema = j.find("schema");
    int version = schema != nullptr ? schema->as_int(-1, 0, 1 << 30) : -1;
    if (version < 0) {
        messages_.push_back(path_ + ": no valid \"schema\" number; read as schema 1");
    } else if (version > SETTINGS_SCHEMA) {
        state_ = State::Newer;
        messages_.push_back(path_ + ": schema " + std::to_string(version) + " is newer than this launcher's (" +
                            std::to_string(SETTINGS_SCHEMA) + "): it is read but never written");
    }
    doc = std::move(j);
    values = settings_from_json(doc, &messages_);
    if (state_ != State::Newer) {
        state_ = State::Loaded;
        written_ = text;
    }
}

bool SettingsFile::save(std::string *err) {
    if (state_ == State::Newer) {
        if (err != nullptr) {
            *err = path_ + " has a newer schema: not written";
        }
        return false;
    }
    settings_to_json(values, &doc);
    std::string text = doc.dump();
    if (text == written_) {
        return true;
    }
    if (!path_is_dir(dir_) && !path_make_dir(dir_, err)) {
        return false;
    }
    if (state_ == State::Broken && path_is_file(path_)) {
        std::string keep = path_ + ".broken";
        if (!SDL_RenamePath(path_.c_str(), keep.c_str())) {
            if (err != nullptr) {
                *err = "cannot keep the unreadable " + path_ + " as " + keep + ": " + SDL_GetError();
            }
            return false;
        }
    }
    if (!file_write_atomic(path_, text, err)) {
        return false;
    }
    written_ = text;
    state_ = State::Loaded;
    return true;
}

std::string SettingsFile::resolve(const std::string &p) const {
    return path_resolve(dir_, p);
}

} // namespace dw3
