#include "mods.h"

#include <algorithm>
#include <cmath>

#include <SDL3/SDL.h>

#include "paths.h"

namespace dw3 {

bool ModOption::valid(const Json &v, std::string *err) const {
    switch (type) {
    case Type::Bool:
        if (!v.is_bool()) {
            *err = "expected true or false";
            return false;
        }
        return true;
    case Type::Int:
    case Type::Float: {
        if (!v.is_number() || (type == Type::Int && v.as_number(0) != std::floor(v.as_number(0)))) {
            *err = type == Type::Int ? "expected a whole number" : "expected a number";
            return false;
        }
        double x = v.as_number(0);
        if ((has_min && x < min) || (has_max && x > max)) {
            *err = "out of range";
            return false;
        }
        return true;
    }
    case Type::Enum:
        for (const Value &e : values) {
            if (v.is_string() && v.as_string() == e.id) {
                return true;
            }
        }
        *err = "not one of the option's values";
        return false;
    case Type::Binding: {
        Binding b;
        return binding_from_json(v, &b, err);
    }
    }
    return false;
}

const ModOption *ModManifest::option(const std::string &oid) const {
    for (const ModOption &o : options) {
        if (o.id == oid) {
            return &o;
        }
    }
    return nullptr;
}

// ---- reading a manifest

static bool get_string(const Json &obj, const char *key, bool required, std::string *out, std::string *err,
                       const std::string &where) {
    const Json *v = obj.find(key);
    if (v == nullptr) {
        if (required) {
            *err = where + key + ": missing";
        }
        return !required;
    }
    if (!v->is_string()) {
        *err = where + key + ": expected a string";
        return false;
    }
    *out = v->as_string();
    return true;
}

static bool parse_option(const Json &j, ModOption *o, std::string *err) {
    if (!j.is_object()) {
        *err = "options: an entry is not an object";
        return false;
    }
    std::string where = "options[" + (j.find("id") && j.find("id")->is_string() ? j.find("id")->as_string() : "?") +
                        "].";
    std::string type, applies;
    if (!get_string(j, "id", true, &o->id, err, where) || !get_string(j, "name", true, &o->name, err, where) ||
        !get_string(j, "description", false, &o->description, err, where) ||
        !get_string(j, "group", false, &o->group, err, where) || !get_string(j, "type", true, &type, err, where) ||
        !get_string(j, "applies", false, &applies, err, where)) {
        return false;
    }
    static const std::pair<const char *, ModOption::Type> types[] = {
        { "bool", ModOption::Type::Bool },   { "int", ModOption::Type::Int },
        { "float", ModOption::Type::Float }, { "enum", ModOption::Type::Enum },
        { "binding", ModOption::Type::Binding },
    };
    bool known = false;
    for (const auto &t : types) {
        if (type == t.first) {
            o->type = t.second;
            known = true;
        }
    }
    if (!known) {
        *err = where + "type: \"" + type + "\" is not bool, int, float, enum or binding";
        return false;
    }
    if (!applies.empty() && applies != "live" && applies != "restart") {
        *err = where + "applies: live or restart";
        return false;
    }
    o->restart = applies == "restart";
    if (o->type == ModOption::Type::Int || o->type == ModOption::Type::Float) {
        for (const char *k : { "min", "max", "step", "slider_max" }) {
            const Json *v = j.find(k);
            if (v == nullptr) {
                continue;
            }
            if (!v->is_number()) {
                *err = where + k + ": expected a number";
                return false;
            }
            double x = v->as_number(0);
            if (k[0] == 's' && k[1] == 'l') {
                o->slider_max = x, o->has_slider_max = true;
            } else if (k[1] == 'i') {
                o->min = x, o->has_min = true;
            } else if (k[1] == 'a') {
                o->max = x, o->has_max = true;
            } else {
                o->step = x;
            }
        }
        if (o->has_min && o->has_max && o->min > o->max) {
            *err = where + "min is above max";
            return false;
        }
        if (!get_string(j, "input_toggle", false, &o->input_toggle, err, where)) {
            return false;
        }
        if (o->has_slider_max != !o->input_toggle.empty()) {
            *err = where + "slider_max and input_toggle go together";
            return false;
        }
        if (o->has_slider_max && (!o->has_min || !o->has_max || o->slider_max <= o->min || o->slider_max > o->max)) {
            *err = where + "slider_max: needs min and max, above min and at most max";
            return false;
        }
    }
    if (o->type == ModOption::Type::Enum) {
        const Json *values = j.find("values");
        if (values == nullptr || !values->is_array() || values->items().empty()) {
            *err = where + "values: an enum needs a list of {id, label}";
            return false;
        }
        for (const Json &v : values->items()) {
            ModOption::Value e;
            if (!v.is_object() || !get_string(v, "id", true, &e.id, err, where + "values.") ||
                !get_string(v, "label", false, &e.label, err, where + "values.")) {
                if (err->empty()) {
                    *err = where + "values: an entry is not an object";
                }
                return false;
            }
            if (e.label.empty()) {
                e.label = e.id;
            }
            o->values.push_back(e);
        }
    }
    const Json *def = j.find("default");
    if (def == nullptr) {
        *err = where + "default: missing";
        return false;
    }
    std::string why;
    if (!o->valid(*def, &why)) {
        *err = where + "default: " + why;
        return false;
    }
    o->def = *def;
    return true;
}

ModManifest mod_manifest_load(const std::string &dir) {
    ModManifest m;
    m.dir = dir;
    m.id = path_base(dir);
    m.name = m.id;
    std::string text, err;
    const std::string path = path_join(dir, "mod.json");
    if (!file_read(path, &text, &err)) {
        m.error = err;
        return m;
    }
    Json j = Json::parse(text, &err);
    if (!j.is_object()) {
        m.error = "mod.json: " + (err.empty() ? std::string("not a JSON object") : err);
        return m;
    }
    const Json *schema = j.find("schema");
    if (schema == nullptr || schema->as_int(-1, 0, 1 << 30) != MOD_MANIFEST_SCHEMA) {
        m.error = "mod.json: schema " + (schema != nullptr ? schema->dump().substr(0, 8) : std::string("missing")) +
                  " (this launcher reads schema 1)";
        return m;
    }
    std::string id;
    if (!get_string(j, "id", true, &id, &err, "") || !get_string(j, "name", true, &m.name, &err, "") ||
        !get_string(j, "version", false, &m.version, &err, "") ||
        !get_string(j, "description", false, &m.description, &err, "") ||
        !get_string(j, "kind", true, &m.kind, &err, "")) {
        m.error = "mod.json: " + err;
        return m;
    }
    if (id != m.id) {
        m.error = "mod.json: id \"" + id + "\" is not its directory's name (" + m.id + ")";
        return m;
    }
    const Json *rp = j.find("requires_port");
    if (rp != nullptr) {
        m.requires_port = rp->as_int(-1, 0, 1 << 30);
        if (m.requires_port < 0) {
            m.error = "mod.json: requires_port: expected a whole number";
            return m;
        }
    }
    const Json *options = j.find("options");
    if (options != nullptr && !options->is_array()) {
        m.error = "mod.json: options: expected a list";
        return m;
    }
    if (options != nullptr) {
        for (const Json &oj : options->items()) {
            ModOption o;
            if (!parse_option(oj, &o, &err)) {
                m.error = "mod.json: " + err;
                return m;
            }
            if (o.id == "enabled" || m.option(o.id) != nullptr) {
                m.error = "mod.json: option id \"" + o.id + "\" is used twice (or is \"enabled\")";
                return m;
            }
            m.options.push_back(o);
        }
    }
    for (const ModOption &o : m.options) {
        const ModOption *t = o.input_toggle.empty() ? nullptr : m.option(o.input_toggle);
        if (!o.input_toggle.empty() && (t == nullptr || t->type != ModOption::Type::Bool)) {
            m.error = "mod.json: options[" + o.id + "].input_toggle: \"" + o.input_toggle + "\" is not a bool option";
            return m;
        }
    }
    const Json *presets = j.find("presets");
    if (presets != nullptr && !presets->is_array()) {
        m.error = "mod.json: presets: expected a list";
        return m;
    }
    static const std::vector<Json> no_presets;
    for (const Json &pj : presets != nullptr ? presets->items() : no_presets) {
        ModPreset p;
        if (!pj.is_object() || !get_string(pj, "id", true, &p.id, &err, "presets[].") ||
            !get_string(pj, "name", true, &p.name, &err, "presets[].") ||
            !get_string(pj, "description", false, &p.description, &err, "presets[].")) {
            m.error = "mod.json: " + (err.empty() ? std::string("presets: an entry is not an object") : err);
            return m;
        }
        const std::string where = "mod.json: presets[" + p.id + "].";
        const Json *vals = pj.find("values");
        if (vals == nullptr || !vals->is_object() || vals->members().empty()) {
            m.error = where + "values: expected an object of option values";
            return m;
        }
        for (const ModPreset &q : m.presets) {
            if (q.id == p.id) {
                m.error = where + "id: used twice";
                return m;
            }
        }
        for (const auto &kv : vals->members()) {
            const ModOption *o = m.option(kv.first);
            std::string why;
            if (o == nullptr || o->type == ModOption::Type::Binding) {
                m.error = where + "values." + kv.first + ": not an option (bindings cannot be preset)";
                return m;
            }
            if (!o->valid(kv.second, &why)) {
                m.error = where + "values." + kv.first + ": " + why;
                return m;
            }
            p.values.emplace_back(kv.first, kv.second);
        }
        // A value above a slider's top only with its toggle switched on by the same preset.
        for (const auto &kv : p.values) {
            const ModOption *o = m.option(kv.first);
            if (!o->has_slider_max || kv.second.as_number(0) <= o->slider_max) {
                continue;
            }
            bool on = false;
            for (const auto &t : p.values) {
                on |= t.first == o->input_toggle && t.second.as_bool(false);
            }
            if (!on) {
                m.error = where + "values." + kv.first + ": above slider_max without " + o->input_toggle + ": true";
                return m;
            }
        }
        m.presets.push_back(p);
    }
    if (m.kind != "builtin") {
        m.error = "kind \"" + m.kind + "\": only built-in mods are supported for now";
    }
    return m;
}

static SDL_EnumerationResult SDLCALL collect(void *list, const char *dir, const char *name) {
    std::string p = path_join(dir, name);
    if (path_is_file(path_join(p, "mod.json"))) {
        static_cast<std::vector<std::string> *>(list)->push_back(p);
    }
    return SDL_ENUM_CONTINUE;
}

std::vector<ModManifest> mods_scan(const std::string &mods_dir) {
    std::vector<std::string> dirs;
    std::vector<ModManifest> mods;
    if (!path_is_dir(mods_dir)) {
        return mods;
    }
    SDL_EnumerateDirectory(mods_dir.c_str(), collect, &dirs);
    for (const std::string &d : dirs) {
        mods.push_back(mod_manifest_load(d));
    }
    std::sort(mods.begin(), mods.end(), [](const ModManifest &a, const ModManifest &b) {
        return SDL_strcasecmp(a.name.c_str(), b.name.c_str()) < 0;
    });
    return mods;
}

// ---- the values

const Json *ModValues::mod(const std::string &id) const {
    const Json *mods = doc_->find("mods");
    const Json *m = mods != nullptr ? mods->find(id) : nullptr;
    return m != nullptr && m->is_object() ? m : nullptr;
}

Json *ModValues::mod_for_write(const std::string &id) {
    Json &mods = doc_->member("mods");
    if (!mods.is_object()) {
        mods = Json::object();
    }
    Json &m = mods.member(id);
    if (!m.is_object()) {
        m = Json::object();
    }
    return &m;
}

void ModValues::tidy(const std::string &id) {
    Json *mods = doc_->find("mods");
    if (mods == nullptr) {
        return;
    }
    Json *m = mods->find(id);
    if (m != nullptr && m->is_object() && m->members().empty()) {
        mods->erase(id);
    }
    if (mods->is_object() && mods->members().empty()) {
        doc_->erase("mods");
    }
}

bool ModValues::enabled(const std::string &id) const {
    const Json *m = mod(id);
    const Json *e = m != nullptr ? m->find("enabled") : nullptr;
    return e != nullptr && e->as_bool(false);
}

void ModValues::set_enabled(const std::string &id, bool on) {
    if (on) {
        mod_for_write(id)->set("enabled", Json::boolean(true));
        return;
    }
    // Off is the default: written only while the mod has other values (so the file shows it was turned off).
    if (Json *m = mod_for_write(id)) {
        m->erase("enabled");
        if (!m->members().empty()) {
            m->set("enabled", Json::boolean(false));
        }
    }
    tidy(id);
}

const Json *ModValues::stored(const ModManifest &m, const ModOption &o, std::string *err) const {
    const Json *mj = mod(m.id);
    const Json *v = mj != nullptr ? mj->find(o.id) : nullptr;
    std::string why;
    if (v == nullptr) {
        return nullptr;
    }
    if (!o.valid(*v, &why)) {
        if (err != nullptr) {
            *err = "mods." + m.id + "." + o.id + ": " + why + "; the default is shown";
        }
        return nullptr;
    }
    return v;
}

Json ModValues::value(const ModManifest &m, const ModOption &o) const {
    const Json *v = stored(m, o, nullptr);
    return v != nullptr ? *v : o.def;
}

void ModValues::set(const ModManifest &m, const ModOption &o, const Json &v) {
    if (v == o.def) {
        reset(m.id, o.id);
        return;
    }
    mod_for_write(m.id)->set(o.id, v);
}

void ModValues::reset(const std::string &mod_id, const std::string &option) {
    Json *mods = doc_->find("mods");
    Json *mj = mods != nullptr ? mods->find(mod_id) : nullptr;
    if (mj == nullptr || !mj->is_object()) {
        return;
    }
    mj->erase(option);
    // A mod left with only "enabled": false is the default: removed.
    const Json *e = mj->find("enabled");
    if (mj->members().size() == 1 && e != nullptr && !e->as_bool(true)) {
        mj->erase("enabled");
    }
    tidy(mod_id);
}

bool ModValues::is_set(const std::string &mod_id, const std::string &option) const {
    const Json *mj = mod(mod_id);
    return mj != nullptr && mj->find(option) != nullptr;
}

bool ModValues::typed(const ModManifest &m, const ModOption &o) const {
    const ModOption *t = o.input_toggle.empty() ? nullptr : m.option(o.input_toggle);
    return t != nullptr && value(m, *t).as_bool(false);
}

Json ModValues::effective(const ModManifest &m, const ModOption &o) const {
    Json v = value(m, o);
    if (o.has_slider_max && !typed(m, o) && v.as_number(0) > o.slider_max) {
        return Json::number(o.slider_max);
    }
    return v;
}

bool ModValues::preset_active(const ModManifest &m, const ModPreset &p) const {
    for (const auto &kv : p.values) {
        const ModOption *o = m.option(kv.first);
        if (o == nullptr || !(effective(m, *o) == kv.second)) {
            return false;
        }
    }
    return true;
}

void ModValues::apply_preset(const ModManifest &m, const ModPreset &p) {
    for (const auto &kv : p.values) {
        if (const ModOption *o = m.option(kv.first)) {
            set(m, *o, kv.second);
        }
    }
}

Binding mod_binding(const ModValues &v, const ModManifest &m, const ModOption &o) {
    Binding b;
    std::string err;
    if (!binding_from_json(v.value(m, o), &b, &err)) {
        binding_from_json(o.def, &b, &err);
    }
    return b;
}

} // namespace dw3
