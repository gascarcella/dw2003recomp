// The mods (docs/LAUNCHER.md "Mod manifest"): their manifests (`mod.json`, installed beside the game in mods/<id>/) and the
// user's values in settings.json (`mods.<id>`: { "enabled": bool, "<option id>": value }). The launcher renders every
// mod's screen from its manifest; the values follow "Mods section": a mod that is absent, or has no `enabled`, is off; an absent
// option keeps the manifest's default. As for the bindings, only what the user changed is written.
#pragma once

#include <string>
#include <vector>

#include "input.h"
#include "json_value.h"

namespace dw3 {

constexpr int MOD_MANIFEST_SCHEMA = 1;

struct ModOption {
    enum class Type { Bool, Int, Float, Enum, Binding };
    struct Value {
        std::string id, label;
    };

    std::string id, name, description, group;
    Type type = Type::Bool;
    bool restart = false; // applies: "restart" (else "live")
    Json def;             // the default, as the manifest writes it
    bool has_min = false, has_max = false;
    double min = 0, max = 0, step = 0; // int and float
    // slider_max and input_toggle (int and float, together): a slider from min to slider_max while the bool option
    // `input_toggle` is off (a value above it counts as slider_max), a typed number from min to max while it is on.
    bool has_slider_max = false;
    double slider_max = 0;
    std::string input_toggle;
    std::vector<Value> values;         // enum

    // Whether `v` is a valid value of this option (a binding in the settings' grammar, an enum id, a number in range...).
    bool valid(const Json &v, std::string *err) const;
};

// A preset (the manifest's `presets`): a button that sets some options' values at once. Only the launcher knows them.
struct ModPreset {
    std::string id, name, description;
    std::vector<std::pair<std::string, Json>> values; // option id -> value, in the manifest's order
};

struct ModManifest {
    std::string dir;   // its directory
    std::string id, name, version, description, kind;
    int requires_port = 0;
    std::vector<ModOption> options;
    std::vector<ModPreset> presets;
    std::string error; // why it cannot be used ("" = fine); the mod is listed, not editable

    const ModOption *option(const std::string &id) const;
};

// Reads <dir>/mod.json. Never fails: problems go to `error`.
ModManifest mod_manifest_load(const std::string &dir);
// Every <mods_dir>/<id>/mod.json, sorted by name.
std::vector<ModManifest> mods_scan(const std::string &mods_dir);

// The user's values in the settings document (`doc` = the whole settings file).
class ModValues {
public:
    explicit ModValues(Json *doc) : doc_(doc) {}

    bool enabled(const std::string &mod) const;
    void set_enabled(const std::string &mod, bool on);
    // The value in the file, or nullptr (absent, or invalid for the option: then `err` says why).
    const Json *stored(const ModManifest &m, const ModOption &o, std::string *err) const;
    // The effective value: the stored one when valid, else the default.
    Json value(const ModManifest &m, const ModOption &o) const;
    // Sets a value; equal to the default it is removed instead (only changes are written).
    void set(const ModManifest &m, const ModOption &o, const Json &v);
    void reset(const std::string &mod, const std::string &option);
    bool is_set(const std::string &mod, const std::string &option) const;
    // Whether the option's input_toggle is on (false without one).
    bool typed(const ModManifest &m, const ModOption &o) const;
    // The value the game uses: value(), capped at slider_max while the input_toggle is off.
    Json effective(const ModManifest &m, const ModOption &o) const;
    // A preset's values: whether every one is the effective value, and setting them all.
    bool preset_active(const ModManifest &m, const ModPreset &p) const;
    void apply_preset(const ModManifest &m, const ModPreset &p);

private:
    const Json *mod(const std::string &id) const;
    Json *mod_for_write(const std::string &id);
    void tidy(const std::string &id); // removes an emptied mod and an emptied mods object

    Json *doc_;
};

// A binding option's value as a Binding (the default when it does not parse).
Binding mod_binding(const ModValues &v, const ModManifest &m, const ModOption &o);

} // namespace dw3
