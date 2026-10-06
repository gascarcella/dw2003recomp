// A JSON document the launcher can edit and write back (LAUNCHER_MODS_PLAN 4.3, 4.4). Parsing goes through the
// port's strict reader (port/src/json.c, shared, not modified); this file adds an editable tree that keeps the
// members' order and a writer. Unknown members survive a load and a save: the game's side (path B) may add keys the
// launcher does not know yet.
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace dw3 {

class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Json() = default;
    static Json boolean(bool v);
    static Json number(double v);
    static Json string(std::string v);
    static Json array();
    static Json object();

    // Parses `text`; on an error returns a null value and sets `err` ("line L column C: ...").
    static Json parse(const std::string &text, std::string *err);
    // Two-space indented, members in their order, a newline at the end.
    std::string dump() const;

    Type type() const { return type_; }
    bool is_null() const { return type_ == Type::Null; }
    bool is_bool() const { return type_ == Type::Bool; }
    bool is_number() const { return type_ == Type::Number; }
    bool is_string() const { return type_ == Type::String; }
    bool is_array() const { return type_ == Type::Array; }
    bool is_object() const { return type_ == Type::Object; }

    bool as_bool(bool fallback) const { return type_ == Type::Bool ? bool_ : fallback; }
    double as_number(double fallback) const { return type_ == Type::Number ? number_ : fallback; }
    // An integral number in [lo, hi], else `fallback`.
    int as_int(int fallback, int lo, int hi) const;
    const std::string &as_string() const { return string_; } // empty unless a string
    std::string as_string(const std::string &fallback) const { return type_ == Type::String ? string_ : fallback; }

    // Object members: find returns nullptr when absent (or when this is not an object); set replaces or appends
    // (turning a non-object into an empty object first); member returns the member, created as null when absent.
    const Json *find(const std::string &key) const;
    Json *find(const std::string &key);
    Json &member(const std::string &key);
    void set(const std::string &key, Json value);
    bool erase(const std::string &key);
    const std::vector<std::pair<std::string, Json>> &members() const { return members_; }

    // Array items.
    const std::vector<Json> &items() const { return items_; }
    void push(Json value);

    bool operator==(const Json &o) const;
    bool operator!=(const Json &o) const { return !(*this == o); }

private:
    void dump_to(std::string &out, int depth) const;

    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0;
    std::string string_;
    std::vector<Json> items_;
    std::vector<std::pair<std::string, Json>> members_;
};

} // namespace dw3
