// The controls (docs/LAUNCHER.md "Input bindings"): the PS1 pad's buttons, the game's default keys
// and gamepad inputs (port/src/input.c), the port's hotkey actions, and the binding grammar shared with the mods'
// `binding` options:
//   a binding = a string or a list; each element is one trigger, any of which fires; "" or [] = unbound
//   a trigger = one input (a string) or a chord (a list of inputs, all held)
//   an input  = an SDL3 scancode name ("X", "Keypad Enter") or "pad:" + a gamepad input name ("pad:south")
// input.keyboard and input.gamepad map a button to one name or a list of names, without "pad:" and without chords.
#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "json_value.h"

namespace dw3 {

// The game's limits (port/src/input.c INPUT_CHORD, INPUT_MAX_TRIGGERS): a longer chord or binding is refused.
constexpr size_t CHORD_MAX_INPUTS = 4;
constexpr size_t BINDING_MAX_TRIGGERS = 8;

using Trigger = std::vector<std::string>; // one input, or a chord (all held)
using Binding = std::vector<Trigger>;     // any trigger fires; empty = unbound

struct PadButton {
    const char *id;    // the settings file's name: up down left right cross circle square triangle start select l1...
    const char *label; // shown
};
// The 14 buttons in the order the screen shows them.
const std::vector<PadButton> &pad_buttons();
// The game's defaults for a button (port/src/input.c's tables): key names, gamepad input names.
std::vector<std::string> default_keys(const std::string &button);
std::vector<std::string> default_pad_inputs(const std::string &button);

struct HotkeyAction {
    const char *id;
    const char *label;
    const char *description;
};
// The port's own actions (input.hotkeys): pause, fullscreen. A mod's bindings are its own options.
const std::vector<HotkeyAction> &hotkey_actions();
Binding default_hotkey(const std::string &action);

// The gamepad input names (ours: SDL's positional buttons, and the axes past half way).
const std::vector<std::string> &pad_input_names();
bool is_pad_input_name(const std::string &name);
bool is_key_name(const std::string &name); // an SDL3 scancode name
// A readable form: "X", "Keypad Enter", "Pad South", "Pad Left stick up".
std::string input_label(const std::string &input); // "pad:..." or a key name
std::string pad_input_label(const std::string &name);
std::string binding_label(const Binding &b); // "F2 or Pad Guide + Pad South"; "unbound"

// A list of names (input.keyboard/gamepad): a string or a list of strings. False (and `err`) when it is neither or a
// name is unknown (`pad` selects the gamepad's names).
bool names_from_json(const Json &j, bool pad, std::vector<std::string> *out, std::string *err);
Json names_to_json(const std::vector<std::string> &names); // one name as a string, else a list ("" when empty)

// A binding from the file; false (and `err`) when it does not follow the grammar or names an unknown input.
bool binding_from_json(const Json &j, Binding *out, std::string *err);
Json binding_to_json(const Binding &b); // the shortest form: "" / "F2" / ["F2", ["pad:guide", "pad:south"]]

// ---- capturing an input from SDL's events (the "press a key" prompt)
class InputCapture {
public:
    enum class Kind { Key, Pad, Binding }; // a key name only, a gamepad name only, or a trigger of either (chords)

    void begin(Kind kind);
    void cancel() { active_ = false; }
    bool active() const { return active_; }
    // Feeds one event; true when the event belonged to the capture (it then does not reach ImGui). When the capture
    // completes, done() turns true once and result() holds the trigger (one input for Key and Pad; "pad:" names
    // only for Binding). Escape alone cancels.
    bool feed(const SDL_Event &e);
    bool take_done(Trigger *result);
    const Trigger &held() const { return order_; } // what is held so far (for the prompt)

private:
    void press(const std::string &input);
    void release(const std::string &input);

    bool active_ = false, done_ = false;
    Kind kind_ = Kind::Key;
    Trigger order_;          // every input pressed during this capture, in order
    std::vector<std::string> down_; // held now
    Trigger result_;
};

} // namespace dw3
