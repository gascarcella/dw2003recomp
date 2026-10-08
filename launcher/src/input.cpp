#include "input.h"

#include <algorithm>

namespace dw3 {

const std::vector<PadButton> &pad_buttons() {
    static const std::vector<PadButton> buttons = {
        { "up", "Up" },         { "down", "Down" },     { "left", "Left" },   { "right", "Right" },
        { "cross", "Cross" },   { "circle", "Circle" }, { "square", "Square" }, { "triangle", "Triangle" },
        { "start", "Start" },   { "select", "Select" }, { "l1", "L1" },       { "r1", "R1" },
        { "l2", "L2" },         { "r2", "R2" },
    };
    return buttons;
}

// port/runtime/input.c's input_keymap and input_padmap (and its triggers and left stick), in the file's names.
struct ButtonDefaults {
    const char *button;
    std::vector<std::string> keys, pad;
};

static const std::vector<ButtonDefaults> &button_defaults() {
    static const std::vector<ButtonDefaults> d = {
        { "up", { "Up" }, { "dpup", "lefty-" } },
        { "down", { "Down" }, { "dpdown", "lefty+" } },
        { "left", { "Left" }, { "dpleft", "leftx-" } },
        { "right", { "Right" }, { "dpright", "leftx+" } },
        { "cross", { "X" }, { "south" } },
        { "circle", { "C" }, { "east" } },
        { "square", { "Z" }, { "west" } },
        { "triangle", { "S" }, { "north" } },
        { "start", { "Return", "Keypad Enter" }, { "start" } },
        { "select", { "Backspace", "Right Shift" }, { "back" } },
        { "l1", { "Q" }, { "leftshoulder" } },
        { "r1", { "E" }, { "rightshoulder" } },
        { "l2", { "1" }, { "lefttrigger" } },
        { "r2", { "3" }, { "righttrigger" } },
    };
    return d;
}

std::vector<std::string> default_keys(const std::string &button) {
    for (const auto &d : button_defaults()) {
        if (button == d.button) {
            return d.keys;
        }
    }
    return {};
}

std::vector<std::string> default_pad_inputs(const std::string &button) {
    for (const auto &d : button_defaults()) {
        if (button == d.button) {
            return d.pad;
        }
    }
    return {};
}

const std::vector<HotkeyAction> &hotkey_actions() {
    static const std::vector<HotkeyAction> actions = {
        { "pause", "Pause", "Stops and resumes the game" },
        { "fullscreen", "Fullscreen", "Switches between the window and the full screen" },
    };
    return actions;
}

Binding default_hotkey(const std::string &action) {
    if (action == "pause") {
        return { { "P" } };
    }
    if (action == "fullscreen") {
        return { { "F11" } };
    }
    return {};
}

// ---- names

struct PadName {
    const char *name;
    const char *label;
};

// The order of SDL_GamepadButton up to the touchpad, then the axes (docs/LAUNCHER.md "Input bindings").
static const PadName PAD_NAMES[] = {
    { "south", "South" },
    { "east", "East" },
    { "west", "West" },
    { "north", "North" },
    { "back", "Back" },
    { "guide", "Guide" },
    { "start", "Start" },
    { "leftstick", "Left stick click" },
    { "rightstick", "Right stick click" },
    { "leftshoulder", "Left shoulder" },
    { "rightshoulder", "Right shoulder" },
    { "dpup", "D-pad up" },
    { "dpdown", "D-pad down" },
    { "dpleft", "D-pad left" },
    { "dpright", "D-pad right" },
    { "misc1", "Misc" },
    { "paddle1", "Paddle 1" },
    { "paddle2", "Paddle 2" },
    { "paddle3", "Paddle 3" },
    { "paddle4", "Paddle 4" },
    { "touchpad", "Touchpad" },
    { "lefttrigger", "Left trigger" },
    { "righttrigger", "Right trigger" },
    { "leftx-", "Left stick left" },
    { "leftx+", "Left stick right" },
    { "lefty-", "Left stick up" },
    { "lefty+", "Left stick down" },
    { "rightx-", "Right stick left" },
    { "rightx+", "Right stick right" },
    { "righty-", "Right stick up" },
    { "righty+", "Right stick down" },
};

const std::vector<std::string> &pad_input_names() {
    static const std::vector<std::string> names = [] {
        std::vector<std::string> v;
        for (const PadName &p : PAD_NAMES) {
            v.push_back(p.name);
        }
        return v;
    }();
    return names;
}

bool is_pad_input_name(const std::string &name) {
    const auto &n = pad_input_names();
    return std::find(n.begin(), n.end(), name) != n.end();
}

bool is_key_name(const std::string &name) {
    return !name.empty() && SDL_GetScancodeFromName(name.c_str()) != SDL_SCANCODE_UNKNOWN;
}

std::string pad_input_label(const std::string &name) {
    for (const PadName &p : PAD_NAMES) {
        if (name == p.name) {
            return std::string("Pad ") + p.label;
        }
    }
    return "Pad " + name;
}

std::string input_label(const std::string &input) {
    return input.compare(0, 4, "pad:") == 0 ? pad_input_label(input.substr(4)) : input;
}

std::string binding_label(const Binding &b) {
    if (b.empty()) {
        return "unbound";
    }
    std::string out;
    for (size_t i = 0; i < b.size(); i++) {
        if (i > 0) {
            out += " or ";
        }
        for (size_t j = 0; j < b[i].size(); j++) {
            out += (j > 0 ? " + " : "") + input_label(b[i][j]);
        }
    }
    return out;
}

// ---- the file's forms

bool names_from_json(const Json &j, bool pad, std::vector<std::string> *out, std::string *err) {
    out->clear();
    std::vector<const Json *> items;
    if (j.is_string()) {
        if (!j.as_string().empty()) {
            items.push_back(&j);
        }
    } else if (j.is_array()) {
        for (const Json &x : j.items()) {
            items.push_back(&x);
        }
    } else {
        *err = "expected a name or a list of names";
        return false;
    }
    for (const Json *x : items) {
        if (!x->is_string()) {
            *err = "expected a name or a list of names";
            return false;
        }
        const std::string &name = x->as_string();
        if (pad ? !is_pad_input_name(name) : !is_key_name(name)) {
            *err = std::string("unknown ") + (pad ? "gamepad input" : "key") + " \"" + name + "\"";
            return false;
        }
        out->push_back(name);
    }
    return true;
}

Json names_to_json(const std::vector<std::string> &names) {
    if (names.size() == 1) {
        return Json::string(names[0]);
    }
    if (names.empty()) {
        return Json::string("");
    }
    Json a = Json::array();
    for (const std::string &n : names) {
        a.push(Json::string(n));
    }
    return a;
}

static bool input_valid(const std::string &input, std::string *err) {
    bool ok = input.compare(0, 4, "pad:") == 0 ? is_pad_input_name(input.substr(4)) : is_key_name(input);
    if (!ok) {
        *err = "unknown input \"" + input + "\"";
    }
    return ok;
}

bool binding_from_json(const Json &j, Binding *out, std::string *err) {
    out->clear();
    if (j.is_string()) {
        if (j.as_string().empty()) {
            return true;
        }
        if (!input_valid(j.as_string(), err)) {
            return false;
        }
        out->push_back({ j.as_string() });
        return true;
    }
    if (!j.is_array()) {
        *err = "expected an input, a chord or a list of them";
        return false;
    }
    if (j.items().size() > BINDING_MAX_TRIGGERS) {
        *err = "at most " + std::to_string(BINDING_MAX_TRIGGERS) + " triggers";
        return false;
    }
    for (const Json &t : j.items()) {
        Trigger trigger;
        if (t.is_string()) {
            trigger.push_back(t.as_string());
        } else if (t.is_array() && !t.items().empty() && t.items().size() <= CHORD_MAX_INPUTS) {
            for (const Json &i : t.items()) {
                if (!i.is_string()) {
                    *err = "a chord is a list of inputs";
                    return false;
                }
                trigger.push_back(i.as_string());
            }
        } else {
            *err = "expected an input or a chord (a list of 1 to " + std::to_string(CHORD_MAX_INPUTS) + " inputs)";
            return false;
        }
        for (const std::string &i : trigger) {
            if (!input_valid(i, err)) {
                return false;
            }
        }
        out->push_back(trigger);
    }
    return true;
}

Json binding_to_json(const Binding &b) {
    if (b.empty()) {
        return Json::string("");
    }
    if (b.size() == 1 && b[0].size() == 1) {
        return Json::string(b[0][0]);
    }
    Json a = Json::array();
    for (const Trigger &t : b) {
        if (t.size() == 1) {
            a.push(Json::string(t[0]));
        } else {
            Json chord = Json::array();
            for (const std::string &i : t) {
                chord.push(Json::string(i));
            }
            a.push(chord);
        }
    }
    return a;
}

// ---- capture

static const char *button_name(Uint8 button) {
    static const char *const names[] = { "south",       "east",          "west",   "north",   "back",    "guide",
                                         "start",       "leftstick",     "rightstick", "leftshoulder", "rightshoulder",
                                         "dpup",        "dpdown",        "dpleft", "dpright", "misc1",   "paddle1",
                                         "paddle2",     "paddle3",       "paddle4", "touchpad" };
    // SDL_GamepadButton: ..., MISC1, RIGHT_PADDLE1, LEFT_PADDLE1, RIGHT_PADDLE2, LEFT_PADDLE2, TOUCHPAD (SDL's own
    // mapping names for them are paddle1..paddle4 in that order).
    return button < sizeof(names) / sizeof(names[0]) ? names[button] : nullptr;
}

void InputCapture::begin(Kind kind) {
    kind_ = kind;
    active_ = true;
    done_ = false;
    order_.clear();
    down_.clear();
    result_.clear();
}

void InputCapture::press(const std::string &input) {
    bool known = std::find(order_.begin(), order_.end(), input) != order_.end();
    if (!known && order_.size() >= CHORD_MAX_INPUTS) {
        return; // the game's longest chord: a fifth input is ignored
    }
    if (std::find(down_.begin(), down_.end(), input) == down_.end()) {
        down_.push_back(input);
    }
    if (std::find(order_.begin(), order_.end(), input) == order_.end()) {
        order_.push_back(input);
    }
    if (kind_ != Kind::Binding) { // one input: the first press decides
        result_ = { input };
        active_ = false;
        done_ = true;
    }
}

void InputCapture::release(const std::string &input) {
    auto it = std::find(down_.begin(), down_.end(), input);
    if (it == down_.end()) {
        return;
    }
    down_.erase(it);
    if (down_.empty() && !order_.empty()) { // everything released: the trigger is all that was held
        result_ = order_;
        active_ = false;
        done_ = true;
    }
}

bool InputCapture::feed(const SDL_Event &e) {
    if (!active_) {
        return false;
    }
    const char *prefix = kind_ == Kind::Binding ? "pad:" : "";
    switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        if (e.key.repeat) {
            return true;
        }
        if (e.key.scancode == SDL_SCANCODE_ESCAPE && order_.empty()) {
            if (!e.key.down) {
                active_ = false; // Escape alone cancels (on its release, so it does not reach ImGui either)
            }
            return true;
        }
        if (kind_ == Kind::Pad) {
            return true;
        }
        const char *name = SDL_GetScancodeName(e.key.scancode);
        if (name == nullptr || *name == '\0') {
            return true;
        }
        e.key.down ? press(name) : release(name);
        return true;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const char *name = button_name(e.gbutton.button);
        if (kind_ == Kind::Key || name == nullptr) {
            return true;
        }
        std::string input = std::string(prefix) + name;
        e.gbutton.down ? press(input) : release(input);
        return true;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        if (kind_ == Kind::Key) {
            return true;
        }
        static const char *const axes[][2] = { { "leftx-", "leftx+" },   { "lefty-", "lefty+" },
                                               { "rightx-", "rightx+" }, { "righty-", "righty+" },
                                               { nullptr, "lefttrigger" }, { nullptr, "righttrigger" } };
        if (e.gaxis.axis >= 6) {
            return true;
        }
        const int v = e.gaxis.value;
        for (int side = 0; side < 2; side++) {
            const char *name = axes[e.gaxis.axis][side];
            if (name == nullptr) {
                continue;
            }
            std::string input = std::string(prefix) + name;
            bool past = side == 0 ? v < -16384 : v > 16384; // half way, as the game reads it
            bool back = side == 0 ? v > -8192 : v < 8192;   // released with some hysteresis
            if (past) {
                press(input);
            } else if (back) {
                release(input);
            }
        }
        return true;
    }
    default:
        return false;
    }
}

bool InputCapture::take_done(Trigger *result) {
    if (!done_) {
        return false;
    }
    done_ = false;
    *result = result_;
    return true;
}

} // namespace dw3
